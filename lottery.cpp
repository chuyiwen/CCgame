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
#include "lottery.h"
#include "att_res.h"
#include "role.h"
#include "../common/WorldDefine/gp_mall_protocol.h"
#include "hearSay_helper.h"
#include "item_creator.h"
#include "../common/ServerDefine/lottery_server_define.h"

lottery::lottery()
{
	dwMaxNumber[0] = 0;
	dwMaxNumber[1] = 0;

	//memset(dwPrizeItem, 0, sizeof(dwPrizeItem));
	
}


BOOL lottery::Init()
{
	reset(ELT_A);
	reset(ELT_B);

	return TRUE;
}


void lottery::reset(E_LOTTERY_TYPE elt)
{
	dwMaxNumber[elt] = 0;
	package_map<DWORD, tagLotteryProto*> mapLotteryProto = AttRes::GetInstance()->GetLotteryMap();
	
	tagLotteryProto* pLotteryProto = NULL; 
	mapLotteryProto.reset_iterator();
	while(mapLotteryProto.find_next(pLotteryProto))
	{
		if (pLotteryProto->byType != elt)
			continue;

		m_mapLottery[elt][pLotteryProto->dwID] = pLotteryProto;

		dwMaxNumber[elt] += pLotteryProto->dwNumber;

		if (pLotteryProto->b_prize == 1)
		{
			//dwPrizeItem[0][elt] = pLotteryProto->dwItemID;
			m_mapFristPrize.insert(make_pair(pLotteryProto->dwItemID,pLotteryProto));
		}
		if (pLotteryProto->b_prize == 2)
		{
			//dwPrizeItem[1][elt] = pLotteryProto->dwItemID;
			m_mapFristPrize.insert(make_pair(pLotteryProto->dwItemID,pLotteryProto));
		}
		
	}

}


DWORD lottery::getLottery(Role* pRole,BYTE& bytype, DWORD& dwItme,BYTE& byNum)
{
	tagItem* pKey = NULL;
	bool bNeedYuanbao = false;//��Ϊtrue��������ұ�����û�г齱������ߣ�ֻ�����ķǰ�Ԫ�����г齱
	DWORD retValue = pRole->role_get_free_gamble();//������Ƚ�����ѳ齱
	if (INVALID_VALUE == retValue)
	{
		//����Ƿ񳬳����ո��ѳ齱����
		DWORD ret = pRole->role_get_normal_gamble();
		if (INVALID_VALUE == ret)
		{
			return E_LOTTERY_NOT_GP;//������Ҹ��ѳ齱���������꣬�޷������齱
		}
		bytype = 1;//���ѳ齱
		DWORD dwKeyID = TPYE_B_NEED_ITEM;

		package_list<tagItem*> list_item;
		INT32 n_num = pRole->GetItemMgr().GetBagSameItemList(list_item, dwKeyID, 1);
		//��ȱ��ָ���ĵ���
		if(n_num < 1)
		{
			//�ٿ��Ǳ������Ƿ���10�����ϵķǰ�Ԫ��
			if (pRole->GetCurMgr().GetBaiBaoYuanBao() >= 10)
			{
				bNeedYuanbao = true;
			}
			else
			{
				//�ع��齱������ʧ�ܲ��ۼӴ���
				if (pRole->m_byDayClear[ERDCT_FinishedGamble_Number] > 0)
				{
					pRole->m_byDayClear[ERDCT_FinishedGamble_Number]--;
				}
				return E_LOTTERY_NOT_KEY;//��û�г齱����Ҳû�й涨��Ŀ��ָ��Ԫ��
			}
		}
		else
		{
			pKey = *(list_item.begin());
		}
	}
	else
	{
		bytype = 0;//���γ齱���
	}
	
	// �����ռ��ж�
	if (pRole->GetItemMgr().GetBagFreeSize() <= 0)
	{
		if (1 == bytype)
		{
			if (pRole->m_byDayClear[ERDCT_FinishedGamble_Number] > 0)
			{
				pRole->m_byDayClear[ERDCT_FinishedGamble_Number]--;
			}
		}
		else
		{
			pRole->m_byDayClear[ERDCT_FreeGamble_Number] &= 0x0;
		}
		return E_LOTTERY_NOT_BAG;
	}


	DWORD dwGetItem = 0;
	BYTE  bGetNum = 0;
	DWORD dwIndex = 0;

	INT32 nSumPct = 0;
	INT32 nRandPct = get_tool()->tool_rand() % 10000;

	std::map<DWORD,tagLotteryProto*>::iterator it = m_mapLottery[bytype].begin();
	for(; it != m_mapLottery[bytype].end(); it++)
	{
		nSumPct += it->second->dwNumber;
		if(nRandPct < nSumPct)
		{
			dwGetItem = it->second->dwItemID;
			bGetNum = it->second->b_ItemNum;
			dwIndex = it->second->dwID;
			break;
		}
	}
	
	const tagLotteryProto* pProto = AttRes::GetInstance()->GetLotteryProto(dwIndex);
	if (!VALID_POINT(pProto))
		return INVALID_VALUE;

	tagItem* pItem = ItemCreator::CreateEx(EICM_Lottery, pRole->GetID(), dwGetItem, bGetNum, EIQ_Quality3, pProto->b_bind, -1, 0, 0);

	if (!VALID_POINT(pItem))
	{
		if (1 == bytype)
		{
			if (pRole->m_byDayClear[ERDCT_FinishedGamble_Number] > 0)
			{
				pRole->m_byDayClear[ERDCT_FinishedGamble_Number]--;
			}
		}
		else
		{
			pRole->m_byDayClear[ERDCT_FreeGamble_Number] &= 0x0;
		}

		return INVALID_VALUE;
	}

	pRole->GetItemMgr().Add2Bag(pItem, elcid_exchange_lottey, true);

	dwItme = dwIndex;
	byNum = bGetNum;
	//Log(pRole, dwItme, bytype, 0);

	//pRole->GetCurMgr().DecBagSilverEx(NEED_SILVER, elcid_exchange_lottey);
	
	//������ѳ齱
	if (0 == bytype)
	{
		//����Ҫ������ұ�������Ʒ����Ԫ��
	}
	else
	{
		//����ͨ��Ԫ���齱
		if (bNeedYuanbao)
		{
			pRole->GetCurMgr().DecBaiBaoYuanBao(10,elcid_exchange_lottey);
		}
		else
		{
			pRole->GetItemMgr().ItemUsedFromBag(pKey->n64_serial, 1, elcid_exchange_lottey);
		}
	}
	
	// ͷ����¼����
	/*if (dwGetItem == dwPrizeItem[0][bytype] || dwGetItem == dwPrizeItem[1][bytype])
	{
		tagPrizeRole sPrizeRole;
		memset(sPrizeRole.szRoleName, 0, sizeof(sPrizeRole.szRoleName));
		sPrizeRole.dwRoleID = pRole->GetID();
		sPrizeRole.dwTime = g_world.GetWorldTime();
		s_role_info* pRoleInfo = g_roleMgr.get_role_info(pRole->GetID());
		if (VALID_POINT(pRoleInfo))
		{
			_tcsncpy(sPrizeRole.szRoleName, pRoleInfo->sz_role_name, X_SHORT_NAME);
		}
		m_listPrizeRole[bytype].push_back(sPrizeRole);
		if (m_listPrizeRole[bytype].size() > 5)
		{
			m_listPrizeRole[bytype].pop_front();
		}
	}

	
	//if (dwGetItem == dwPrizeItem[0][bytype])
	{
		HearSayHelper::SendMessage(EHST_Lottery, pRole->GetID(), dwGetItem, 0 , bytype, INVALID_VALUE, pItem);
	}*/
	// ������
	std::map<DWORD,tagLotteryProto*>::iterator iterFirst = m_mapFristPrize.find(dwGetItem);
	if (iterFirst != m_mapFristPrize.end())
	{
		HearSayHelper::SendMessage(EHST_Lottery, pRole->GetID(), dwGetItem, 1 , bytype, INVALID_VALUE, pItem);
	}

	if(VALID_POINT(pRole->GetScript()))
		pRole->GetScript()->OnLottery(pRole);




	return E_Success;

}


VOID lottery::Log(Role* pRole, DWORD dwItemID, DWORD dwType, DWORD dwIndex)
{
	NET_DB2C_log_lottery send;
	send.s_log.dw_role_id		= pRole->GetID();
	send.s_log.dw_account_id	= pRole->GetSession()->GetSessionID();
	send.s_log.dw_item_id		= dwItemID;
	send.s_log.dw_type			= dwType;
	send.s_log.dw_index			= dwIndex;

	g_dbSession.Send(&send, send.dw_size);

}

//VOID lottery::ChangeNumber(DWORD dwType, DWORD dwNumber)
//{
//	if (dwType < 0 || dwType > 1)
//		return;
//
//	std::vector<DWORD> vecItem;
//
//	std::map<DWORD,DWORD>::iterator it = m_mapLottery[dwType].begin();
//	for(; it != m_mapLottery[dwType].end(); it++)
//	{
//		if (it->first != dwPrizeItem[0][dwType] && 
//			it->first != dwPrizeItem[1][dwType] &&
//			m_mapLottery[dwType][it->first] > 0)
//		{
//			vecItem.push_back(it->first);
//		}
//	}
//
//
//
//	for (int i = 0; i < dwNumber; i++)
//	{
//		if (vecItem.empty())
//			return;
//
//		int rand = get_tool()->tool_rand() % vecItem.size();
//		
//		m_mapLottery[dwType][vecItem[rand]]--;
//		dwMaxNumber[dwType]--;
//		
//		if (m_mapLottery[dwType][vecItem[rand]] <= 0)
//		{
//			vecItem.erase(vecItem.begin() + rand);
//		}
//	}
//}



lottery	g_lottery;

