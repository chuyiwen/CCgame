
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
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/pet_exchange_protocot.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "pet_pocket.h"
#include "pet_soul.h"
#include "pet_server_define.h"
//------------------------------------------------------------------------
// 请求交易
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID)
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
	if(!CanPetExchange())
	{
		return E_Pet_Exchange_SelfBusy;
	}

	// 被申请者检查
	pTarget = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pTarget))
	{
		// 被申请者不在线,或与申请者不在同一张地图中
		return E_Pet_Exchange_NotInSame_Map;
	}

	// 被申请者是否可交易
	if(!pTarget->CanPetExchange())
	{
		return E_Pet_Exchange_RoleBusy;
	}

	// 交易距离
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		return E_Pet_Exchange_OutOfRange;
	}

	// 设置申请者交易状态
	BeginPetExchange(dwTgtRoleID);

	// 设置被申请者状态
	pTarget->GetPetExchMgr().SetTgtRoleID(GetID());

	return E_Pets_Success;
}

//------------------------------------------------------------------------
// 玩家对请求交易的反馈
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code)
{
	// 是否自己和自己交易
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	// 判断交易对象是否正确
	if(GetPetExchMgr().GetTgtRoleID() != dwTgtRoleID)
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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// 申请者的交易目标是否变化
	PetExchangeMgr &exchMgrApp = pApplicant->GetPetExchMgr();
	if(pApplicant->IsPetExchanging() && exchMgrApp.GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	// 检查交易返回码
	if(dw_error_code != E_Pet_Exchange_Accept)
	{
		// 结束交易发起者的交易状态
		pApplicant->EndPetExchange();
		return dw_error_code;
	}

	// 申请者交易状态检查
	if(!pApplicant->IsPetExchanging())
	{
		exchMgrApp.SetTgtRoleID(INVALID_VALUE);
		return INVALID_VALUE;
	}

	// 交易距离
	if(!IsInDistance(*pApplicant, MAX_EXCHANGE_DISTANCE))
	{
		return E_Pet_Exchange_OutOfRange;
	}

	// 设置申请者交易状态
	BeginPetExchange(dwTgtRoleID);

	return E_Pet_Exchange_Accept;
}

//------------------------------------------------------------------------
// 添加交易物品
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeAdd( OUT Role* &pTarget, DWORD dwPetID )
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 取得交易物品
	PetSoul* pSoul = GetPetPocket()->GetPetSoul(dwPetID);
	if(!VALID_POINT(pSoul))
	{
		ASSERT(VALID_POINT(pSoul));
		return INVALID_VALUE;
	}

	// 物品是否可别交易
	DWORD dwRtv = GetPetPocket()->CanExchange(dwPetID);
	if(dwRtv != E_Success)
	{
		return dwRtv;
	}

	if( pSoul->GetProto()->nRoleLvlLim > pTarget->get_level())
	{	
		return E_Pet_Exchange_RoleLvlNotEnough;
	}

	// 放入到待交易物品中
	dw_error_code = GetPetExchMgr().AddPet(dwPetID);
	if(E_Success != dw_error_code)
	{
		// 已在交易列表中，或交易物品个数已达到上限
		return INVALID_VALUE;
	}

	// 解除目标锁定状态
	pTarget->GetPetExchMgr().Unlock();

	return E_Success;
}

//------------------------------------------------------------------------
// 取消交易物品
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeDec( OUT Role* &pTarget, DWORD dwPetID )
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 解除目标锁定状态
	pTarget->GetPetExchMgr().Unlock();

	return GetPetExchMgr().DecPet(dwPetID);
}

//------------------------------------------------------------------------
// 修改交易金钱
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeMoney(OUT Role* &pTarget, INT64 n64Silver)
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 解除目标锁定状态
	pTarget->GetPetExchMgr().Unlock();

	// 背包中是否有足够的金钱
	CurrencyMgr &CurMgr = GetCurMgr();
	if(n64Silver > CurMgr.GetBagSilver())
	{
		return E_Pet_Exchange_NotEnough_Money;
	}

	GetPetExchMgr().ResetMoney(n64Silver);

	return E_Success;
}

//------------------------------------------------------------------------
// 锁定交易
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeLock(OUT Role* &pTarget)
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	GetPetExchMgr().Lock();

	return E_Success;
}

//------------------------------------------------------------------------
// 玩家取消交易
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeCancel(OUT Role* &pTarget)
{
	// 是否处于交易状态
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// 获得交易管理器和交易对象id
	PetExchangeMgr &exchMgr = GetPetExchMgr();
	DWORD dwTargetRoleID = exchMgr.GetTgtRoleID();

	// 取消交易
	EndPetExchange();

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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	pTarget->EndPetExchange();

	return E_Success;
}

//------------------------------------------------------------------------
// 确认交易
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID)
{
	DWORD dw_error_code = E_Success;

	// 是否处于交易状态
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// 获得交易管理器
	PetExchangeMgr &exchMgr = GetPetExchMgr();

	// 交易数据是否处于锁定状态
	if(!exchMgr.IsLock())
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
		dwFailedRoleID	= exchMgr.GetTgtRoleID();
		dw_error_code		= E_Pet_Exchange_NotInSame_Map;
		goto Exit;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		//return E_Exchange_ApplicantTgt_Change;
		return INVALID_VALUE;
	}

	// 目标是否处于锁定状态
	if(!pTarget->GetPetExchMgr().IsLock())
	{
		return INVALID_VALUE;
	}

	exchMgr.Verify();
	if(!pTarget->GetPetExchMgr().IsVerify())
	{
		return INVALID_VALUE;
	}

	// 交易距离确认
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		dw_error_code = E_Pet_Exchange_OutOfRange;
		goto Exit;
	}

	// 背包空间检查 -- 背包空间不足时，不清楚交易数据
	BYTE byMyPetNum	= exchMgr.GetPetNum();
	BYTE byTgtPetNum	= pTarget->GetPetExchMgr().GetPetNum();
	if(byMyPetNum > byTgtPetNum)
	{
		// 检查对方背包
		if(byMyPetNum - byTgtPetNum > pTarget->GetPetPocket()->GetFreeSize())
		{
			// 解锁
			exchMgr.Unlock();
			pTarget->GetPetExchMgr().Unlock();

			dwFailedRoleID = pTarget->GetID();
			return E_Pet_Exchange_NotEnough_Space;
		}
	}
	else
	{
		// 检查自己背包
		if(byTgtPetNum - byMyPetNum >GetPetPocket()->GetFreeSize())
		{
			// 解锁
			exchMgr.Unlock();
			pTarget->GetPetExchMgr().Unlock();

			dwFailedRoleID = GetID();
			return E_Pet_Exchange_NotEnough_Space;
		}
	}

	/*************************************************
	* 交易物品处理
	*************************************************/

	PetSoul *pSrcItem[MAX_EXCHANGE_ITEM];
	PetSoul *pDstItem[MAX_EXCHANGE_ITEM];
	ZeroMemory(pSrcItem, sizeof(PetSoul*) * MAX_EXCHANGE_ITEM);
	ZeroMemory(pDstItem, sizeof(PetSoul*) * MAX_EXCHANGE_ITEM);

	// 交易物品检查
	dw_error_code = VerifyPetExchangeData();
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = GetID();
		goto Exit;
	}

	dw_error_code = pTarget->VerifyPetExchangeData();
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = pTarget->GetID();
		goto Exit;
	}
 
 	// 4.移动物品

	// 4.1 金钱 -- 先减金钱,避免损失
	GetCurMgr().DecBagSilver(exchMgr.GetMoney(), elcid_exchange_verify, pTarget->GetID());
	pTarget->GetCurMgr().DecBagSilver(pTarget->GetPetExchMgr().GetMoney(), elcid_exchange_verify, GetID());

	GetCurMgr().IncBagSilver(
		min(GetCurMgr().GetCanIncBagSilver(), pTarget->GetPetExchMgr().GetMoney()), 
		elcid_exchange_verify, pTarget->GetID());

	pTarget->GetCurMgr().IncBagSilver(
		min(pTarget->GetCurMgr().GetCanIncBagSilver(), exchMgr.GetMoney()), 
		elcid_exchange_verify, GetID());

	// 4.2 物品

	// 4.2.1 先从玩家身上删除交易物品 -- 避免可堆叠物品处理出错
	GetPetPocket()->TakeFromPocket(pSrcItem, MAX_EXCHANGE_ITEM, exchMgr.GetPetIDs(), exchMgr.GetPetNum());
	pTarget->GetPetPocket()->TakeFromPocket(pDstItem, MAX_EXCHANGE_ITEM, pTarget->GetPetExchMgr().GetPetIDs(), pTarget->GetPetExchMgr().GetPetNum());

	// 4.2.2 将交易物品放到玩家身上
	GetPetPocket()->PutInPocket(pDstItem, pTarget->GetPetExchMgr().GetPetNum());
	pTarget->GetPetPocket()->PutInPocket(pSrcItem, exchMgr.GetPetNum());

Exit:
	// 释放交易资源
	EndPetExchange();

	if(VALID_POINT(pTarget))
	{
		pTarget->EndPetExchange();
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
DWORD Role::VerifyPetExchangeData()
{
	// 获得交易管理器
	PetExchangeMgr &exchMgr = GetPetExchMgr();

	// 1.金钱是否还够
	if(GetCurMgr().GetBagSilver() < exchMgr.GetMoney())
	{
		return E_Pet_Exchange_NotEnough_Money;
	}

	//// 2.身上是否有足够的空间
	//if(GetItemMgr().GetBagFreeSize() < exchMgr.GetItemTypeNum())
	//{
	//	return E_Exchange_NotEnough_BagSpace;
	//}

	// 3.待交易物品是否还在身上, 若在，得到物品指针
	if(!GetPetPocket()->CheckExistInPocket(exchMgr.GetPetIDs(), MAX_EXCHANGE_ITEM))
	{
		return E_Pet_Exchange_PetCanNot_Find;
	}

	return E_Success;
}

//------------------------------------------------------------------------
// 修改交易数据前的检查，及得到相关对象
//------------------------------------------------------------------------
DWORD Role::ProcPreparePetExchange(OUT Role* &pTarget)
{
	// 是否处于交易状态
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// 获得交易管理器
	PetExchangeMgr &exchMgr = GetPetExchMgr();

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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// 目标没有处于交易状态，或正在和别人交易
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	return E_Success;
}