/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"
#include "vip_netbar.h"


#include "../common/ServerDefine/vip_netbar_server_define.h"
#include "../common/ServerDefine/vip_netbar_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "world_session.h"
#include "att_res.h"
#include "player_session.h"
#include "db_session.h"
#include "item_creator.h"
#include "item_mgr.h"

VipNerBarMgr g_VipNetBarMgr;

class IPComp
{
public:
	bool operator()(const IpRange& left, const IpRange& right)
	{
		return left.GetIpMin() < right.GetIpMin();
	}
};


// 
BOOL VipNerBarMgr::Init()
{
	m_setNotify.clear();
	m_mapVipNetBars.clear();
	m_vecIpRanges.clear();
	m_mapIp2VNBId.clear();

	// 生成掉落容器
	file_container* p_vip_var = new file_container;

	// 得到配置文件中怪物掉落表的路径
	tstring strVNBTablePath	= World::p_var->get_string(_T("path"),	_T("vip_netbar"));

	// 加载怪物掉落文件
	std::list<tstring> listField;
	if (!p_vip_var->load(NULL, strVNBTablePath.c_str(), "id", &listField))
	{
		p_vip_var->clear();
		SAFE_DELETE(p_vip_var);
		return FALSE;
	}
	;

	// 一个一个的加载怪物掉落文件
	for(std::list<tstring>::iterator it = listField.begin(); it != listField.end(); ++it)
	{
		s_vnb* pVNB = new s_vnb;
		ZeroMemory(pVNB, sizeof(s_vnb));

		pVNB->dw_id = _tcstol(it->c_str(), NULL, 10);
		_tcsncpy(pVNB->sz_name, p_vip_var->get_string(_T("name"), it->c_str(), NULL), X_SHORT_NAME);

		if (VALID_POINT(m_mapVipNetBars.find(pVNB->dw_id)))
			continue;

		m_mapVipNetBars.add(pVNB->dw_id, pVNB);
		
		LPCTSTR tszIpMin = NULL;
		LPCTSTR tszIpMax = NULL;
		DWORD dwIpMin = INVALID_VALUE;
		DWORD dwIpMax = INVALID_VALUE;

		for(INT i = 0; i < MAX_VNB_IP_NUM; ++i)
		{
			tstringstream ss;
			ss << _T("ipmin") << i+1;
			tszIpMin = p_vip_var->get_string(ss.str().c_str(), it->c_str(), 0);
			if (!VALID_POINT(tszIpMin))
				break;
			
			ss.str(_T(""));
			ss << _T("ipmax") << i+1;
			tszIpMax = p_vip_var->get_string(ss.str().c_str(), it->c_str(), 0);
			if (!VALID_POINT(tszIpMax))
				break;

			dwIpMin = TransTSIp2DWORD(tszIpMin);
			GeneralzeIP(dwIpMin);

			dwIpMax = TransTSIp2DWORD(tszIpMax);
			GeneralzeIP(dwIpMax);

			ASSERT(dwIpMin <= dwIpMax);

			m_vecIpRanges.push_back(IpRange(dwIpMin, dwIpMax, pVNB->dw_id));
		}
	}

	std::sort(m_vecIpRanges.begin(), m_vecIpRanges.end(), IPComp());
	p_vip_var->clear();
	SAFE_DELETE(p_vip_var);

	return TRUE;
}

VOID VipNerBarMgr::UpdateDbPlayerLogin(DWORD dw_account_id, DWORD dw_time)
{
	tagDWORDTime dwLoginDate = dw_time;
	dwLoginDate.min = 0;
	dwLoginDate.sec = 0;
	dwLoginDate.hour = 0;
	NET_DB2C_update_vnb_player send;
	send.dw_account_id = dw_account_id;
	send.dw_login_time = dwLoginDate;
	g_dbSession.Send(&send, send.dw_size);
}

VOID VipNerBarMgr::InitData(s_db_vnb_players* pInitData)
{
	m_setHistoryAccountID.clear();
	for (INT i=0; i<pInitData->n_his_players; ++i)
	{
		DWORD dwHisPlayerId = pInitData->dw_account_ids[i];
		if (m_setHistoryAccountID.find(dwHisPlayerId) == m_setHistoryAccountID.end())
		{
			m_setHistoryAccountID.insert(dwHisPlayerId);
		}
	}

	m_setTodayAccountID.clear();
	for (INT i=0; i<pInitData->n_todays_players; ++i)
	{
		DWORD dwTodayPlayerId = pInitData->dw_account_ids[i + pInitData->n_his_players];
		if (m_setTodayAccountID.find(dwTodayPlayerId) == m_setTodayAccountID.end())
		{
			m_setTodayAccountID.insert(dwTodayPlayerId);
		}
	}
}


DWORD VipNerBarMgr::FitNetBar(DWORD dwIp)
{
	INT nLeft = 0;
	INT nRight = m_vecIpRanges.size() - 1;
	INT	nMiddle = 0;

	while (nLeft <= nRight)
	{
		nMiddle = (nLeft + nRight) / 2;
		if (m_vecIpRanges[nMiddle].Fit(dwIp))
		{
			return m_vecIpRanges[nMiddle].GetVNBId();
		}
		if (m_vecIpRanges[nMiddle].OnRightOf(dwIp))
		{
			nRight = nMiddle - 1;
		}
		else if (m_vecIpRanges[nMiddle].OnLeftOf(dwIp))
		{
			nLeft = nMiddle + 1;
		}
	}

	return INVALID_VALUE;
}

VOID VipNerBarMgr::PlayerLogin( DWORD dw_account_id, DWORD dw_ip )
{
	GeneralzeIP(dw_ip);
	// 网吧在线人数
	s_vnb* pVNB = GetVipNetBar(dw_ip);
	if (!VALID_POINT(pVNB))
	{
		return;
	}

	pVNB->on_player_login();
	UpdateDbPlayerLogin(dw_account_id, GetCurrentDWORDTime());

	if (m_setTodayAccountID.find(dw_account_id) == m_setTodayAccountID.end())
	{
		const s_vnb_gift_proto* pGiftProto = AttRes::GetInstance()->GetRandVNBGiftProto();
		if (VALID_POINT(pGiftProto))
		{
			tagItem* pItem = ItemCreator::Create(EICM_VipNetBar, INVALID_VALUE, pGiftProto->dw_item_type_id, pGiftProto->n_num);
			if(VALID_POINT(pItem))
			{
				// 存储到item_baibao表中
				ItemMgr::InsertBaiBao2DBEx(pItem, dw_account_id, elcid_vip_netbar);
				ItemMgr::ProcBaiBaoRecord(pItem->dw_data_id, 
					INVALID_VALUE, INVALID_VALUE, EBBRT_VipNetBarGift);
				// 删除物品
				::Destroy(pItem);

				// 今日礼物
				m_setTodayAccountID.insert(dw_account_id);

				m_setNotify.insert(dw_account_id);
			}
			else
			{
				ASSERT(0);
			}
		}
	}
	if (m_setHistoryAccountID.find(dw_account_id) == m_setHistoryAccountID.end())
	{
		std::list<s_vnb_equip_proto*> listEquips;
		AttRes::GetInstance()->GetRandVNBEquipProto(listEquips);
		while (!listEquips.empty())
		{
			const s_vnb_equip_proto* pEquipProto = listEquips.front();
			listEquips.pop_front();

			if (VALID_POINT(pEquipProto))
			{
				tagItem* pEquip = ItemCreator::CreateEx(EICM_VipNetBar, INVALID_VALUE, pEquipProto->dw_equip_type_id, 1, (EItemQuality)pEquipProto->n_quality);
				if (VALID_POINT(pEquip))
				{
					ItemMgr::InsertBaiBao2DBEx(pEquip , dw_account_id, elcid_vip_netbar);
					// 删除物品
					::Destroy(pEquip );

					// 网吧装备
					m_setHistoryAccountID.insert(dw_account_id);			
				}
				else
				{
					ASSERT(0);
				}
			}
		}
	}
}

VOID VipNerBarMgr::PlayerLogout(DWORD dw_ip)
{
	GeneralzeIP(dw_ip);
	// 网吧在线人数
	s_vnb* pVNB = GetVipNetBar(dw_ip);
	if (VALID_POINT(pVNB))
	{
		pVNB->on_player_logout();
	}
}


DWORD VipNerBarMgr::GetVNBId(DWORD dwIp)
{
	DWORD dwVNBId = m_mapIp2VNBId.find(dwIp);
	if (!VALID_VALUE(dwVNBId))
	{
		dwVNBId = FitNetBar(dwIp);
		if (VALID_POINT(dwVNBId))
		{
			m_mapIp2VNBId.add(dwIp, dwVNBId);
		}
	}		
	return dwVNBId;
}

VOID VipNerBarMgr::Destroy()
{
	
}

struct RateItem
{
	INT		nMinNum;
	INT		nRate;
};
const INT NUM_VNB_EXP_RATE	= 5;
const INT NUM_VNB_LOOT_RATE	= 5;
RateItem ExpRate[NUM_VNB_EXP_RATE] =
{
	{	1,		500		},
	{	5,		1000	},
	{	10,		1500	},
	{	15,		2000	},
	{	20,		2500	},
};

RateItem LootRate[NUM_VNB_LOOT_RATE] =
{
	{	1,		200		},
	{	5,		400	},
	{	10,		800	},
	{	15,		1200	},
	{	20,		1500	},
};



INT VipNerBarMgr::GetRate( DWORD dw_ip, INT nType)
{
	GeneralzeIP(dw_ip);
	const s_vnb* pVnb = GetVipNetBar(dw_ip);
	if (!VALID_POINT(pVnb))
	{
		return 0;
	}
	
	RateItem* pArr = (nType == 0 ? ExpRate : (nType == 1 ? LootRate : NULL));
	if(!VALID_POINT(pArr))
	{
		return 0;
	}

	for (INT i=NUM_VNB_EXP_RATE - 1; i>=0; --i)
	{
		if (pVnb->n_player_num > pArr[i].nMinNum )
		{
			return pArr[i].nRate;
		}
	}
	
	return 0;
}

LPCTSTR VipNerBarMgr::GetVNBName( DWORD dw_ip )
{
	GeneralzeIP(dw_ip);
	const s_vnb* pVnb = GetVipNetBar(dw_ip);
	if (!VALID_POINT(pVnb))
	{
		return 0;
	}

	return pVnb->sz_name;
}

s_vnb* VipNerBarMgr::GetVipNetBar( DWORD dw_ip )
{
	DWORD dwVNBId = GetVNBId(dw_ip);
	return m_mapVipNetBars.find(dwVNBId);
}

BOOL VipNerBarMgr::DynamicTest(DWORD dwTestNo, DWORD dwArg1, LPCTSTR szArg2)
{
	if (!VALID_POINT(szArg2))
	{
		return FALSE;
	}

	DWORD dwIp = TransTSIp2DWORD(szArg2);
	switch(dwTestNo)
	{
	case 0:
		PlayerLogin(dwArg1, dwIp);
		break;
	case 1:
		PlayerLogout(dwIp);
		break;
	case 2:
		INT nRate0 = GetRate(dwIp, 0);
		INT nRate1 = GetRate(dwIp, 1);
		LPCTSTR szName = GetVNBName(dwIp);
		break;
	}
	return TRUE;
}

VOID VipNerBarMgr::GeneralzeIP( DWORD &dw_ip )
{
	DWORD dwTemp = 0;

	dwTemp |= (dw_ip >> 24) & 0x000000ff;
	dwTemp |= (dw_ip >> 8) & 0x0000ff00;
	dwTemp |= (dw_ip << 8) & 0x00ff0000;
	dwTemp |= (dw_ip << 24) & 0xff000000;

	dw_ip = dwTemp;
}

DWORD VipNerBarMgr::TransTSIp2DWORD( LPCTSTR tszIP )
{
	tool util;
	
	LPSTR szIp = (LPSTR)util.unicode_to_ansi(tszIP);
	DWORD dwIp = trans_ip.stringip_to_ip(szIp);
	
	return dwIp;
}

VOID VipNerBarMgr::PlayerNotify( DWORD dw_account_id )
{
	if (m_setNotify.find(dw_account_id) != m_setNotify.end())
	{
		m_setNotify.erase(dw_account_id);
		
		PlayerSession* pSession = g_worldSession.FindSession(dw_account_id);
		if (VALID_POINT(pSession))
		{
//			tagNS_VipNetBarGift send;
//			pSession->SendMessage(&send, send.dw_size);
		}
	}
}