/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家交易

#include "StdAfx.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/exchange_protocol.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../common/ServerDefine/log_server_define.h"

//------------------------------------------------------------------------
// 请求交易
//------------------------------------------------------------------------
DWORD Role::ProcExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID)
{
	// 是否自己和自己交易
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}
	
	// 获取地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 申请者是否可以发起交易
	if(!CanExchange())
	{
		return E_Exchange_SelfBusy;
	}
	
	//申请者区域检查
	if (!IsInRoleState(ERS_SafeArea))
	{
		return E_Exchange_NotInSafearea;
	}
	// 被申请者检查
	pTarget = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pTarget))
	{
		// 被申请者不在线,或与申请者不在同一张地图中
		return E_Exchange_NotInSame_Map;
	}

	if(pTarget->IsDuel())
	{
		return E_Exchange_Trage_InDuel;
	}

	// 被申请者是否可交易
	if(!pTarget->CanExchange())
	{
		return E_Exchange_RoleBusy;
	}
	
	//被邀请者区域检查
	if (!pTarget->IsInRoleState(ERS_SafeArea))
	{
		return E_Exchange_NotInSafearea;
	}
	// 交易距离
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		return E_Exchange_OutOfRange;
	}
	
	// 设置申请者交易状态
	BeginExchange(dwTgtRoleID);

	// 设置被申请者状态
	pTarget->GetExchMgr().SetTgtRoleID(GetID());

	return E_Success;
}

//------------------------------------------------------------------------
// 玩家对请求交易的反馈
//------------------------------------------------------------------------
DWORD Role::ProcExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code)
{
	// 是否自己和自己交易
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	// 判断交易对象是否正确
	if(GetExchMgr().GetTgtRoleID() != dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	//// 再次判断是否忙
	//if(!CanExchange())
	//{
	//	return E_Exchange_RoleBusy;
	//}

	// 获取地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 交易申请者检查
	pApplicant = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pApplicant))
	{
		// 申请者下线，或者更换地图
		return E_Exchange_NotInSame_Map;
	}

	// 申请者的交易目标是否变化
	ExchangeMgr &exchMgrApp = pApplicant->GetExchMgr();
	if(pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	//被邀请者区域再次检查 gx add
	if (!IsInRoleState(ERS_SafeArea))
	{
		if (pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() == GetID())
		{
			// 结束交易发起者的交易状态
			pApplicant->EndExchange();
		}
		return E_Exchange_NotInSafearea;
	}

	// 检查交易返回码
	if(dw_error_code != E_Exchange_Accept)
	{
		// 结束交易发起者的交易状态
		pApplicant->EndExchange();
		return dw_error_code;
	}

	// 申请者交易状态检查
	if(!pApplicant->IsExchanging())
	{
		exchMgrApp.SetTgtRoleID(INVALID_VALUE);
		//return INVALID_VALUE;
		return E_Exchange_NotAnswer;//gx modify 为了给客户端提示
	}

	// 交易距离
	if(!IsInDistance(*pApplicant, MAX_EXCHANGE_DISTANCE))
	{
		return E_Exchange_OutOfRange;
	}
	// 申请者区域检查
	if (!pApplicant->IsInRoleState(ERS_SafeArea))
	{
		if (pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() == GetID())
		{
			// 结束交易发起者的交易状态
			pApplicant->EndExchange();
		}
		return E_Exchange_NotInSafearea;
	}

	// 设置申请者交易状态
	BeginExchange(dwTgtRoleID);

	return E_Exchange_Accept;
}

//------------------------------------------------------------------------
// 添加交易物品
//------------------------------------------------------------------------
DWORD Role::ProcExchangeAdd(OUT Role* &pTarget, OUT tagItem* &pItem, INT32 &nInsIndex, INT64 n64_serial)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 取得交易物品
	pItem = GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		ASSERT(VALID_POINT(pItem));
		return INVALID_VALUE;
	}

	// 物品是否可别交易
	if(!GetItemMgr().CanExchange(*pItem))
	{	
		return E_Exchange_ItemCanNot_Exchange;
	}

	if(!MIsEquipment(pItem->dw_data_id))
	{
		if(!VALID_POINT(pItem->pProtoType) || pItem->nUseTimes)
		{
			return E_Exchange_ItemCanNot_Exchange;
		}
	}
	//else
	//{
	//	tagEquip* pEquip = (tagEquip*)pItem;
	//	if (pEquip->equipSpec.nCurLevelExp > 0 || pEquip->equipSpec.nLevel > 1)
	//	{
	//		return E_Exchange_ItemCanNot_Exchange;
	//	}
	//}
	// 放入到待交易物品中
	nInsIndex = GetExchMgr().add_item(n64_serial, pItem->n16Num);
	if(INVALID_VALUE == nInsIndex)
	{
		// 已在交易列表中，或交易物品个数已达到上限
		return INVALID_VALUE;
	}

	// 解除目标锁定状态
	pTarget->GetExchMgr().Unlock();

	return E_Success;
}

//------------------------------------------------------------------------
// 取消交易物品
//------------------------------------------------------------------------
DWORD Role::ProcExchangeDec(OUT Role* &pTarget, INT64 n64_serial)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 解除目标锁定状态
	pTarget->GetExchMgr().Unlock();
	
	return GetExchMgr().DecItem(n64_serial);
}

//------------------------------------------------------------------------
// 修改交易金钱
//------------------------------------------------------------------------
DWORD Role::ProcExchangeMoney(OUT Role* &pTarget, INT64 n64Silver)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 解除目标锁定状态
	pTarget->GetExchMgr().Unlock();

	// 背包中是否有足够的金钱
	CurrencyMgr &CurMgr = GetCurMgr();
	if(n64Silver > CurMgr.GetBagSilver())
	{
		return E_Exchange_NotEnough_Money;
	}

	GetExchMgr().ResetMoney(n64Silver);

	return E_Success;
}

//------------------------------------------------------------------------
// 锁定交易
//------------------------------------------------------------------------
DWORD Role::ProcExchangeLock(OUT Role* &pTarget)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	GetExchMgr().Lock();

	return E_Success;
}

//------------------------------------------------------------------------
// 玩家取消交易
//------------------------------------------------------------------------
DWORD Role::ProcExchangeCancel(OUT Role* &pTarget)
{
	// 是否处于交易状态
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// 获得交易管理器和交易对象id
	ExchangeMgr &exchMgr = GetExchMgr();
	DWORD dwTargetRoleID = exchMgr.GetTgtRoleID();
	
	// 取消交易
	EndExchange();

	//// 交易数据是否处于锁定状态
	//if(exchMgr.IsLock())
	//{
	//	return INVALID_VALUE;
	//}

	// 获取地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(dwTargetRoleID);
	if(!VALID_POINT(pTarget))
	{
		// 被申请者不在线,或与申请者不在同一张地图中
		return E_Exchange_NotInSame_Map;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	pTarget->EndExchange();

	return E_Success;
}

//------------------------------------------------------------------------
// 确认交易
//------------------------------------------------------------------------
DWORD Role::ProcExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID)
{
	DWORD dw_error_code = E_Success;

	// 获得交易管理器
	ExchangeMgr &exchMgr = GetExchMgr();

	// 获取地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(exchMgr.GetTgtRoleID());
	if(!VALID_POINT(pTarget))
	{
		// 被申请者不在线,或与申请者不在同一张地图中
		dwFailedRoleID	= exchMgr.GetTgtRoleID();
		dw_error_code		= E_Exchange_NotInSame_Map;
	}

	dw_error_code = ProcExchangeLock(pTarget);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_exchange_item_lock send;
	send.dwSrcID = GetID();
	send.dw_error_code = dw_error_code;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	if(dw_error_code != E_Success)
		return INVALID_VALUE;
	
	// 是否处于交易状态
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// 交易数据是否处于锁定状态
	if(!exchMgr.IsLock())
	{
		return INVALID_VALUE;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		//return E_Exchange_ApplicantTgt_Change;
		return INVALID_VALUE;
	}

	// 目标是否处于锁定状态
	if(!pTarget->GetExchMgr().IsLock())
	{
		return INVALID_VALUE;
	}

	exchMgr.Verify();
	pTarget->GetExchMgr().Verify();

	// 交易距离确认
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		dw_error_code = E_Exchange_OutOfRange;
		goto Exit;
	}
	//双方区域再次检查 gx add
	if (!IsInRoleState(ERS_SafeArea) || !pTarget->IsInRoleState(ERS_SafeArea))
	{
		dw_error_code = E_Exchange_NotInSafearea;
		goto Exit;
	}

	// 背包空间检查 -- 背包空间不足时，不清楚交易数据
	BYTE byExTypeNum	= exchMgr.GetItemTypeNum();
	BYTE byTgtExTypeNum	= pTarget->GetExchMgr().GetItemTypeNum();
	if(byExTypeNum > byTgtExTypeNum)
	{
		// 检查对方背包
		if(byExTypeNum - byTgtExTypeNum > pTarget->GetItemMgr().GetBagFreeSize())
		{
			// 解锁
			exchMgr.Unlock();
			pTarget->GetExchMgr().Unlock();

			dwFailedRoleID = pTarget->GetID();
			return E_Exchange_NotEnough_BagSpace;
		}
	}
	else
	{
		// 检查自己背包
		if(byTgtExTypeNum - byExTypeNum > GetItemMgr().GetBagFreeSize())
		{
			// 解锁
			exchMgr.Unlock();
			pTarget->GetExchMgr().Unlock();

			dwFailedRoleID = GetID();
			return E_Exchange_NotEnough_BagSpace;
		}
	}

	/*************************************************
	* 交易物品处理
	*************************************************/

	tagItem *pSrcItem[MAX_EXCHANGE_ITEM];
	tagItem *pDstItem[MAX_EXCHANGE_ITEM];
	ZeroMemory(pSrcItem, sizeof(tagItem*) * MAX_EXCHANGE_ITEM);
	ZeroMemory(pDstItem, sizeof(tagItem*) * MAX_EXCHANGE_ITEM);

	// 交易物品检查
	dw_error_code = VerifyExchangeData(pSrcItem);
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = GetID();
		goto Exit;
	}

	dw_error_code = pTarget->VerifyExchangeData(pDstItem);
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = pTarget->GetID();
		goto Exit;
	}

	// 4.移动物品

	// 4.1 金钱 -- 先减金钱,避免损失
	GetCurMgr().DecBagSilver(exchMgr.GetMoney(), elcid_exchange_verify, pTarget->GetID());
	pTarget->GetCurMgr().DecBagSilver(pTarget->GetExchMgr().GetMoney(), elcid_exchange_verify, GetID());

	GetCurMgr().IncBagSilver(
		min(GetCurMgr().GetCanIncBagSilver(), pTarget->GetExchMgr().GetMoney()), 
		elcid_exchange_verify, pTarget->GetID());

	pTarget->GetCurMgr().IncBagSilver(
		min(pTarget->GetCurMgr().GetCanIncBagSilver(), exchMgr.GetMoney()), 
		elcid_exchange_verify, GetID());

	// 4.2 物品

	// 4.2.1 先从玩家身上删除交易物品 -- 避免可堆叠物品处理出错
	GetItemMgr().RemoveFromBag(exchMgr.GetSerialArray(), MAX_EXCHANGE_ITEM, elcid_exchange_verify, pTarget->GetID());
	pTarget->GetItemMgr().RemoveFromBag(pTarget->GetExchMgr().GetSerialArray(), 
										MAX_EXCHANGE_ITEM, elcid_exchange_verify, GetID());

	// 4.2.2 将交易物品放到玩家身上
	GetItemMgr().Add2Bag(pDstItem, MAX_EXCHANGE_ITEM, elcid_exchange_verify, pTarget->GetID());
	pTarget->GetItemMgr().Add2Bag(pSrcItem, MAX_EXCHANGE_ITEM, elcid_exchange_verify, GetID());


	// 成就
	GetAchievementMgr().UpdateAchievementCriteria(eta_exchange, 1);
	pTarget->GetAchievementMgr().UpdateAchievementCriteria(eta_exchange, 1);

Exit:
	// 释放交易资源
	EndExchange();

	if(VALID_POINT(pTarget))
	{
		pTarget->EndExchange();
	}
	else
	{
		// 如果交易目标在其他地图，该处也不可直接操作(多线程导致)
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 检查玩家身上的交易物品是否齐全
//------------------------------------------------------------------------
DWORD Role::VerifyExchangeData(OUT tagItem* pItem[])
{
	// 获得交易管理器
	ExchangeMgr &exchMgr = GetExchMgr();
	
	// 1.金钱是否还够
	if(GetCurMgr().GetBagSilver() < exchMgr.GetMoney())
	{
		return E_Exchange_NotEnough_Money;
	}

	//// 2.身上是否有足够的空间
	//if(GetItemMgr().GetBagFreeSize() < exchMgr.GetItemTypeNum())
	//{
	//	return E_Exchange_NotEnough_BagSpace;
	//}

	// 3.待交易物品是否还在身上, 若在，得到物品指针
	if(!GetItemMgr().CheckExistInBag(pItem, exchMgr.GetSerialArray(), exchMgr.GetNumArray(), MAX_EXCHANGE_ITEM))
	{
		return E_Exchange_ItemCanNot_Find;
	}

	return E_Success;
}

//------------------------------------------------------------------------
// 修改交易数据前的检查，及得到相关对象
//------------------------------------------------------------------------
DWORD Role::ProcPrepareExchange(OUT Role* &pTarget)
{
	// 是否处于交易状态
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// 获得交易管理器
	ExchangeMgr &exchMgr = GetExchMgr();

	// 交易数据是否处于锁定状态
	if(exchMgr.IsLock())
	{
		return INVALID_VALUE;
	}

	// 获取地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(exchMgr.GetTgtRoleID());
	if(!VALID_POINT(pTarget))
	{
		// 被申请者不在线,或与申请者不在同一张地图中
		return E_Exchange_NotInSame_Map;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	return E_Success;
}