/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#include "StdAfx.h"

#include "player_session.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/LianHun_define.h"

#include "../common/ServerDefine/log_server_define.h"
#include "map.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "hearSay_helper.h"


// 改变灵气
VOID Role::ChangeLinqi(INT nValue)
{
	if( 0 == nValue ) return;

	m_nAtt[ERA_Injury] += nValue;			

	// 取上下限
	if( m_nAtt[ERA_Injury] < AttRes::GetInstance()->GetAttMin(ERA_Injury) ) m_nAtt[ERA_Injury] = AttRes::GetInstance()->GetAttMin(ERA_Injury);

	INT nMaxlingqi = AttRes::GetInstance()->GetAttMax(ERA_Injury) + GET_VIP_EXTVAL(GetTotalRecharge(), LINGQI_MAX, INT);

	if( m_nAtt[ERA_Injury] > nMaxlingqi ) m_nAtt[ERA_Injury] = nMaxlingqi;

	SendAttChange(ERA_Injury);

}

DWORD Role::buyLingqi()
{
	INT nLestLingqi = AttRes::GetInstance()->GetAttMax(ERA_Injury) + GET_VIP_EXTVAL(GetTotalRecharge(), LINGQI_MAX, INT) - m_nAtt[ERA_Injury];

	if (nLestLingqi < 1000)
		return E_BuyLingqi_Full;
	
	if (GetDayClearData(ERDCT_BUYLINGQI) >= MAXHUENLIANTIME + GET_VIP_EXTVAL(GetTotalRecharge(), LIANHUEN_TIME_MAX, INT))
		return E_LIANHUEN_TIME_FULL;
	
	INT32 nNeedYuanbao = LianHunHelper::GetBuyLingqi(GetDayClearData(ERDCT_BUYLINGQI));
	if (GetCurMgr().GetBaiBaoYuanBao() < nNeedYuanbao)
		return E_BuyLingqi_Not_Sivler;

	ChangeLinqi(1000);
	ModRoleDayClearDate(ERDCT_BUYLINGQI);
	GetCurMgr().DecBaiBaoYuanBao(nNeedYuanbao, elci_buy_lingqi);

	return E_Success;
}

INT Role::get_huenjing_temp_bag_size()
{
	INT nSize = 0;
	for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
	{
		if (st_role_huenjing_data.s_huenjing_bag_temp[i].dw_data_id == 0)
		{
			nSize++;
		}
	}

	return nSize;

}

INT Role::get_huenjing_temp_bag_num()
{
	INT nSize = 0;
	for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
	{
		if (st_role_huenjing_data.s_huenjing_bag_temp[i].dw_data_id != 0)
		{
			nSize++;
		}
	}

	return nSize;

}

INT Role::get_huenjing_bag_size()
{
	INT nSize = 0;
	for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
	{
		if (st_role_huenjing_data.s_huenjing_bag[i].dw_data_id == 0)
		{
			nSize++;
		}
	}

	return nSize;
}

VOID Role::add_huenjing_to_temp_bag(const tagHuenJing& sHuenjing)
{
	for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
	{
		if (st_role_huenjing_data.s_huenjing_bag_temp[i].dw_data_id == 0)
		{
			memcpy(&st_role_huenjing_data.s_huenjing_bag_temp[i], &sHuenjing, sizeof(tagHuenJing));
			send_update_huenJing_data(0, i);
			break;
		}
	}
}

VOID Role::add_huenjing_to_bag(const tagHuenJing& sHuenjing)
{
	for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
	{
		if (st_role_huenjing_data.s_huenjing_bag[i].dw_data_id == 0)
		{
			memcpy(&st_role_huenjing_data.s_huenjing_bag[i], &sHuenjing, sizeof(tagHuenJing));
			send_update_huenJing_data(1, i);
			break;
		}
	}	
}


DWORD Role::Huenlian(BYTE byType)
{
	if ( byType != 0 && byType != 1)
		return INVALID_VALUE;
	
	if (getGodLevel() < HUENLIAN_MIN_ROLE_LEVEL)
		return INVALID_VALUE;

	INT nNeedLingqi = LianHunHelper::getLingqi(st_role_huenjing_data.byCurArtisan);
	
	if (GetAttValue(ERA_Injury) < nNeedLingqi)
		return E_LIANHUEN_NOT_LINGQI;

	if (byType == 1)
	{
		if (GetCurMgr().GetBagSilver() < HIGHLEVEL_HUENLIAN_SIEVER)
			return E_LIANHUEN_NOT_SIVLER;
	}
	
	if (get_huenjing_temp_bag_size() <= 0)
		return E_LIANHUEN_NOT_ENOUTH_BAG_TMP;
	
	// 看看得到了什么魂晶
	tagHuenJing sHuengjing;
	sHuengjing.nLevel = 1;
	
	INT nPro = get_tool()->tool_rand() % 10000;
	INT nProPct = 0;
	INT nRes = 0;
	for (int i = 0; i < 6; i++)
	{
		nProPct += LIANGHUEN_PRO[st_role_huenjing_data.byCurArtisan][i];
		if (nPro < nProPct)
		{
			nRes = i;
			break;
		}
	}
	// 灰烬
	if (nRes == 0)
	{
		sHuengjing.dw_data_id = 1000000;
	}
	else
	{
		INT nProAtt = get_tool()->tool_rand() % 10000;
		INT nProAttPct = 0;
		INT nResAtt = 0;

		for (int i = 0; i < 14; i++)
		{
			nProAttPct += JINGHUENATT_PRO[i];
			if (nProAtt < nProAttPct)
			{
				nResAtt = i;
				break;
			}
		}
		
		sHuengjing.dw_data_id = 1000000 + nRes*100 + nResAtt;
	}
	
	const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(sHuengjing.dw_data_id);
	if (!VALID_POINT(pProto))
		return INVALID_VALUE;

	add_huenjing_to_temp_bag(sHuengjing);
	
	if (pProto->byQuality > 2)
	{
		HearSayHelper::SendMessage(EHST_LianHuen,this->GetID(), sHuengjing.dw_data_id,  INVALID_VALUE,   INVALID_VALUE, INVALID_VALUE, NULL);
	}
	
	if (VALID_POINT(GetScript()))
	{
		GetScript()->OnLianhuen(this);
	}
	// 下个工匠
	BOOL bNext = LianHunHelper::canNextArtisan(st_role_huenjing_data.byCurArtisan, get_level());
	if (bNext)
	{
		INT nNextPro = get_tool()->tool_rand() % 100;
		INT nNextProPec = NEXTGONGJIANGPRO[byType][st_role_huenjing_data.byCurArtisan];

		if ( nNextPro < nNextProPec)
		{
			st_role_huenjing_data.byCurArtisan++;
		}
		else
		{
			st_role_huenjing_data.byCurArtisan = EAT_0;
		}
	}
	else
	{
		st_role_huenjing_data.byCurArtisan = EAT_0;
	}
	

	// 扣除灵气和金币
	ChangeLinqi(-nNeedLingqi);
	if (byType == 1)
	{
		GetCurMgr().DecBagSilver(HIGHLEVEL_HUENLIAN_SIEVER, elci_huenlian);
	}
	return E_Success;
}

DWORD	Role::send_huenjing_data()
{
	NET_SIS_get_HunJing_data send;
	memcpy(&send.st_data, &st_role_huenjing_data, sizeof(tagRoleHuenJingData));
	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD Role::send_update_huenJing_data(BYTE byType, BYTE byIndex)
{
	NET_SIS_update_HunJing_data send;
	send.byType = byType;
	send.byIndex = byIndex;
	switch (byType)
	{
	case 0:
		{
			if (byIndex >= 0 &&byIndex < MAX_HUENJING_BAG_SIZE)
			{
				memcpy(&send.s_data, &st_role_huenjing_data.s_huenjing_bag_temp[byIndex], sizeof(tagHuenJing));
			}
		}
		break;
	case 1:
		{
			if (byIndex >= 0 &&byIndex < MAX_HUENJING_BAG_SIZE)
			{
				memcpy(&send.s_data, &st_role_huenjing_data.s_huenjing_bag[byIndex], sizeof(tagHuenJing));
			}
		}
		break;
	case 2:
		{
			if (byIndex >= 0 &&byIndex < MAX_ROLE_HUENJING_SIZE)
			{
				memcpy(&send.s_data, &st_role_huenjing_data.s_huenjing_role_level[byIndex], sizeof(tagHuenJing));
			}
		}
		break;
	case 3:
		{
			if (byIndex >= 0 &&byIndex < MAX_ROLE_HUENJING_SIZE)
			{
				memcpy(&send.s_data, &st_role_huenjing_data.s_huenjing_role_titel[byIndex], sizeof(tagHuenJing));
			}
		}
		break;
	}

	SendMessage(&send, send.dw_size);
	
	return 0;
}

// 魂精操作
DWORD Role::HuenJingOpt(INT nIndex, BYTE byType, BYTE byConType)
{
	// 操作所有
	if (nIndex == -1)
	{
		if (byType == 0)
		{
			if (get_huenjing_bag_size() < get_huenjing_temp_bag_num())
				return E_LIANHUEN_NOT_ENOUTH_BAG;
			
			for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
			{
				if ( st_role_huenjing_data.s_huenjing_bag_temp[i].dw_data_id == 0)
					continue;

				add_huenjing_to_bag(st_role_huenjing_data.s_huenjing_bag_temp[i]);

				memset(&st_role_huenjing_data.s_huenjing_bag_temp[i], 0, sizeof(tagHuenJing));
				send_update_huenJing_data(0, i);

			}

		}
		else if (byType == 1)
		{	
			tagHuenJing* pData = NULL;
			if (byConType == 0)
			{
				pData = st_role_huenjing_data.s_huenjing_bag_temp;	
			}
			else
			{
				pData = st_role_huenjing_data.s_huenjing_bag;
			}
		
			INT nTotalLingqi = 0;
			for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
			{
				if (pData[i].dw_data_id == 0)
					continue;

				const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(pData[i].dw_data_id);
				if (!VALID_POINT(pProto))
					return INVALID_VALUE;
				INT nAddLingqi = HUENJINGLINGHUEN[pProto->byQuality + 1] + LianHunHelper::getHuenJingLevelUpExpTotal(pData[i].nLevel, pProto->byQuality);

				if (pData[i].dw_data_id == LIANHUENG_HUIJING)
				{
					nAddLingqi = HUENJINGLINGHUEN[0];
				}

				nTotalLingqi += nAddLingqi;
				memset(&pData[i], 0, sizeof(tagHuenJing));
				send_update_huenJing_data(byConType, i);
			}

			ModAttValue(ERA_Morality, nTotalLingqi);

		}
		else if (byType == 2)
		{
			tagHuenJing	s_temp[MAX_HUENJING_BAG_SIZE];
			
			memset(s_temp, 0, sizeof(s_temp));

			//std::vector<tagHuenJing> sortVect;
			tagHuenJing* pData = NULL;
			if (byConType == 0)
			{
				pData = st_role_huenjing_data.s_huenjing_bag_temp;	
			}
			else
			{
				pData = st_role_huenjing_data.s_huenjing_bag;
			}


			int nIndex = 0;
			for (int i = 0; i < MAX_HUENJING_BAG_SIZE; i++)
			{
				if (pData[i].dw_data_id != 0)
				{
					//sortVect.push_back(pData[i]);
					s_temp[nIndex].dw_data_id = pData[i].dw_data_id;
					s_temp[nIndex].nLevel = pData[i].nLevel;
					nIndex++;
				}
			}
			//std::sort(sortVect.begin(), sortVect.end(), HuenJingSort());
			//memset(pData, 0, sizeof(tagHuenJing) * MAX_HUENJING_BAG_SIZE);

			//std::vector<tagHuenJing>::iterator it = sortVect.begin();
			//for (int i = 0; it != sortVect.end(); it++, i++)
			//{
			//	pData[i].dw_data_id = it->dw_data_id;
			//	pData[i].nLevel = it->nLevel;
			//}
			//
			memcpy(pData, s_temp, sizeof(s_temp));

			send_huenjing_data();

		}
	}
	else
	{
		if (byType == 0)
		{
			if (byConType == 0)
			{
				if (st_role_huenjing_data.s_huenjing_bag_temp[nIndex].dw_data_id == 0)
					return INVALID_VALUE;

				if (get_huenjing_bag_size() <= 0)
					return E_LIANHUEN_NOT_ENOUTH_BAG;

				add_huenjing_to_bag(st_role_huenjing_data.s_huenjing_bag_temp[nIndex]);

				memset(&st_role_huenjing_data.s_huenjing_bag_temp[nIndex], 0, sizeof(tagHuenJing));
				send_update_huenJing_data(0, nIndex);
			}
			else if(byConType == 1)
			{
				if (st_role_huenjing_data.s_huenjing_bag[nIndex].dw_data_id == 0)
					return INVALID_VALUE;

				if (get_huenjing_temp_bag_size() <= 0)
					return E_LIANHUEN_NOT_ENOUTH_BAG;

				add_huenjing_to_temp_bag(st_role_huenjing_data.s_huenjing_bag[nIndex]);

				memset(&st_role_huenjing_data.s_huenjing_bag[nIndex], 0, sizeof(tagHuenJing));
				send_update_huenJing_data(1, nIndex);
			}
			
		}
		else if (byType == 1)
		{	
			tagHuenJing* pData = NULL;
			if (byConType == 0)
			{
				pData = &st_role_huenjing_data.s_huenjing_bag_temp[nIndex];	
			}
			else
			{
				pData = &st_role_huenjing_data.s_huenjing_bag[nIndex];
			}

			if (pData->dw_data_id == 0)
				return INVALID_VALUE;

			const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(pData->dw_data_id);
			if (!VALID_POINT(pProto))
				return INVALID_VALUE;
			INT nAddLingqi = HUENJINGLINGHUEN[pProto->byQuality + 1] + LianHunHelper::getHuenJingLevelUpExpTotal(pData->nLevel, pProto->byQuality);

			if (pData->dw_data_id == LIANHUENG_HUIJING)
			{
				nAddLingqi = HUENJINGLINGHUEN[0];
			}

			ModAttValue(ERA_Morality, nAddLingqi);
			
			memset(pData, 0, sizeof(tagHuenJing));

			send_update_huenJing_data(byConType, nIndex);	

		}
			
		

	}
	return E_Success;
}

// 魂精升级
DWORD Role::HuenJingLevelUp(INT nIndex)
{
	if (st_role_huenjing_data.s_huenjing_bag[nIndex].dw_data_id == 0)
		return INVALID_VALUE;

	const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(st_role_huenjing_data.s_huenjing_bag[nIndex].dw_data_id);
	if (!VALID_POINT(pProto))
		return INVALID_VALUE;

	INT nNeedLingHuen = LianHunHelper::getHuenjingLevelUpExp(st_role_huenjing_data.s_huenjing_bag[nIndex].nLevel,pProto->byQuality);
	if (GetAttValue(ERA_Morality) < nNeedLingHuen)
		return E_LIANHUEN_NOT_LINGQI;
	
	if (st_role_huenjing_data.s_huenjing_bag[nIndex].nLevel >= get_level())
		return E_LIANHUEN_LEVEL_MAX;

	st_role_huenjing_data.s_huenjing_bag[nIndex].nLevel++;
	
	send_update_huenJing_data(1, nIndex);
	ModAttValue(ERA_Morality, -nNeedLingHuen);
	return E_Success;
}

// 魂精镶嵌
DWORD Role::HuenJingInlay(INT nSrcIndex, INT nDesIndex, BYTE byType, BYTE byOptType)
{
	if (byOptType == 0)
	{
		if (st_role_huenjing_data.s_huenjing_bag[nSrcIndex].dw_data_id == 0)
			return INVALID_VALUE;

		tagHuenJing* pData = NULL;
		if (byType == 0)
		{
			pData = st_role_huenjing_data.s_huenjing_role_level;
		}
		else
		{
			pData = st_role_huenjing_data.s_huenjing_role_titel;
		}

		if (pData[nDesIndex].dw_data_id != 0)
			return INVALID_VALUE;

		const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(st_role_huenjing_data.s_huenjing_bag[nSrcIndex].dw_data_id);

		if (!VALID_POINT(pProto)) return INVALID_VALUE;

		if (IsHuenJingAttHave(byType, pProto->eAtt))
			return E_LIANHUEN_SAME_ATT;

		if ( byType == 0 && get_level() < LEVELCAONEEDLEVEL[nDesIndex])
			return E_LIANHUEN_CAN_THIS_NOT_LEVEL;
		
		if ( byType == 1 && getGodLevel() < GODLEVELCAONEEDLEVEL[nDesIndex])
			return E_LIANHUEN_CAN_THIS_NOT_LEVEL;

		memcpy(&pData[nDesIndex], &st_role_huenjing_data.s_huenjing_bag[nSrcIndex], sizeof(tagHuenJing));
		if (byType==0)
		{
			send_update_huenJing_data(2, nDesIndex);
		}
		else if(byType == 1)
		{
			send_update_huenJing_data(3, nDesIndex);
		}
	

		memset(&st_role_huenjing_data.s_huenjing_bag[nSrcIndex], 0, sizeof(tagHuenJing));
		send_update_huenJing_data(1, nSrcIndex);

		INT nAttVaule = pProto->nBaseAtt + pProto->nGrowAtt * (pData[nDesIndex].nLevel - 1);

		ModAttModValue(pProto->eAtt, nAttVaule);
		RecalAtt();
		return E_Success;
	}
	else
	{
		if (st_role_huenjing_data.s_huenjing_bag[nDesIndex].dw_data_id != 0)
			return INVALID_VALUE;

		tagHuenJing* pData = NULL;
		if (byType == 0)
		{
			pData = st_role_huenjing_data.s_huenjing_role_level;
		}
		else
		{
			pData = st_role_huenjing_data.s_huenjing_role_titel;
		}
		

		if (pData[nSrcIndex].dw_data_id == 0)
			return INVALID_VALUE;
		
		const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(pData[nSrcIndex].dw_data_id);

		if (!VALID_POINT(pProto)) return INVALID_VALUE;
		
		memcpy(&st_role_huenjing_data.s_huenjing_bag[nDesIndex], &pData[nSrcIndex], sizeof(tagHuenJing));
		send_update_huenJing_data(1, nDesIndex);

		memset(&pData[nSrcIndex], 0, sizeof(tagHuenJing));
		if (byType==0)
		{
			send_update_huenJing_data(2, nSrcIndex);
		}
		else if(byType == 1)
		{
			send_update_huenJing_data(3, nSrcIndex);
		}
		

		INT nAttVaule = pProto->nBaseAtt + pProto->nGrowAtt * (st_role_huenjing_data.s_huenjing_bag[nDesIndex].nLevel-1);

		ModAttModValue(pProto->eAtt, -nAttVaule);
		RecalAtt();
		return E_Success;

	}

	return E_Success;
}


VOID  Role::InitHuenJingAtt()
{

	for (int i = 0; i < MAX_ROLE_HUENJING_SIZE; i++)
	{
		const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(st_role_huenjing_data.s_huenjing_role_level[i].dw_data_id);
		if (!VALID_POINT(pProto)) continue;

		INT nAttVaule = pProto->nBaseAtt + pProto->nGrowAtt * (st_role_huenjing_data.s_huenjing_role_level[i].nLevel-1);
		ModAttModValue(pProto->eAtt, nAttVaule);
		RecalAtt();
	}

	for (int i = 0; i < MAX_ROLE_HUENJING_SIZE; i++)
	{
		const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(st_role_huenjing_data.s_huenjing_role_titel[i].dw_data_id);
		if (!VALID_POINT(pProto)) continue;

		INT nAttVaule = pProto->nBaseAtt + pProto->nGrowAtt * (st_role_huenjing_data.s_huenjing_role_titel[i].nLevel-1);
		ModAttModValue(pProto->eAtt, nAttVaule);
		RecalAtt();
	}

}

BOOL  Role::IsHuenJingAttHave(BYTE byType, ERoleAttribute eat)
{
	tagHuenJing* pData = NULL;
	if (byType == 0)
	{
		pData = st_role_huenjing_data.s_huenjing_role_level;
	}
	else
	{
		pData = st_role_huenjing_data.s_huenjing_role_titel;
	}

	for (int i = 0; i < MAX_ROLE_HUENJING_SIZE; i++)
	{
		const tagHunJingProto* pProto = AttRes::GetInstance()->GetHunJingProto(pData[i].dw_data_id);
		if (!VALID_POINT(pProto))
			continue;

		if (eat == pProto->eAtt)
			return true;
	}

	return false;
}