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
	bool bNeedYuanbao = false;//若为true，表明玩家背包中没有抽奖所需道具，只能消耗非绑定元宝进行抽奖
	DWORD retValue = pRole->role_get_free_gamble();//玩家优先进行免费抽奖
	if (INVALID_VALUE == retValue)
	{
		//检查是否超出今日付费抽奖次数
		DWORD ret = pRole->role_get_normal_gamble();
		if (INVALID_VALUE == ret)
		{
			return E_LOTTERY_NOT_GP;//今日玩家付费抽奖次数已用完，无法继续抽奖
		}
		bytype = 1;//付费抽奖
		DWORD dwKeyID = TPYE_B_NEED_ITEM;

		package_list<tagItem*> list_item;
		INT32 n_num = pRole->GetItemMgr().GetBagSameItemList(list_item, dwKeyID, 1);
		//若缺少指定的道具
		if(n_num < 1)
		{
			//再考虑背包中是否有10个以上的非绑定元宝
			if (pRole->GetCurMgr().GetBaiBaoYuanBao() >= 10)
			{
				bNeedYuanbao = true;
			}
			else
			{
				//回滚抽奖次数，失败不累加次数
				if (pRole->m_byDayClear[ERDCT_FinishedGamble_Number] > 0)
				{
					pRole->m_byDayClear[ERDCT_FinishedGamble_Number]--;
				}
				return E_LOTTERY_NOT_KEY;//既没有抽奖道具也没有规定数目的指定元宝
			}
		}
		else
		{
			pKey = *(list_item.begin());
		}
	}
	else
	{
		bytype = 0;//本次抽奖免费
	}
	
	// 背包空间判断
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
	
	//若是免费抽奖
	if (0 == bytype)
	{
		//不需要减少玩家背包的物品或是元宝
	}
	else
	{
		//若是通过元宝抽奖
		if (bNeedYuanbao)
		{
			pRole->GetCurMgr().DecBaiBaoYuanBao(10,elcid_exchange_lottey);
		}
		else
		{
			pRole->GetItemMgr().ItemUsedFromBag(pKey->n64_serial, 1, elcid_exchange_lottey);
		}
	}
	
	// 头奖记录下来
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
	// 发传闻
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

