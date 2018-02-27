/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//物品容器

#pragma once

#include "StdAfx.h"
#include "Container.h"

#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "att_res.h"
#include "container_restrict.h"
#include "item_creator.h"

//****************************************** ItemContainer ***********************************************

//-------------------------------------------------------------------------------------------------------
// 构造函数
//-------------------------------------------------------------------------------------------------------
ItemContainer::ItemContainer(EItemConType eConType,  INT16 n16CurSize, INT16 n16MaxSize, 
							 DWORD dwOwnerID, DWORD dw_account_id, ContainerRestrict *pRestrict/*=NULL*/)
							 : Container(eConType, n16CurSize, n16MaxSize), m_TimeLimitMgr(ITEM_UPDATE_TIME)
{
	m_n16MinFreeIndex = 0;

	m_dwOwnerID = dwOwnerID;
	m_dwAccountID = dw_account_id;

	m_pRestrict = (pRestrict != NULL ? pRestrict : new ContainerRestrict);
	m_pRestrict->init(this);
}

//-------------------------------------------------------------------------------------------------------
// 析构函数
//-------------------------------------------------------------------------------------------------------
ItemContainer::~ItemContainer()
{
	m_n16MinFreeIndex = INVALID_VALUE;

	SAFE_DELETE(m_pRestrict);
}

//-------------------------------------------------------------------------------------------------------
// 向容器中添加新物品,但不指定添加位置,返回错误码
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::Add(tagItem* pItem, OUT INT16 &n16Index, OUT s_item_move &ItemMove, 
						 BOOL bCheckAdd/* = TRUE*/, BOOL bChangeOwner/* = TRUE*/)
{
	//ASSERT(pItem != NULL);

	if(pItem->n16Num <= 0)
	{
		return INVALID_VALUE;
	}

	if(bCheckAdd && !m_pRestrict->CanAdd(pItem))
	{
		return E_Item_CanNotAdd;
	}

	// 初始化传出参数
	n16Index = INVALID_VALUE;
	ItemMove.p_item2	= NULL;
	ItemMove.n_num_res1	= 0;
	ItemMove.n_num_res2	= 0;
	ItemMove.b_create_item = FALSE;
	ItemMove.b_overlap = FALSE;
	ItemMove.b_change_pos = TRUE;

	INT16 n16Add = 0;
	
	if(pItem->pProtoType->n16MaxLapNum > 1)	// 可堆叠物品
	{
		package_list<tagItem*> itemList;
		GetSameBindItemList(itemList, pItem->dw_data_id, pItem->IsBind());

		tagItem *pItemInCon;
		INT16 n16CanLap;

		// 和同类物品放在一起
		itemList.reset_iterator();
		while(itemList.find_next(pItemInCon))
		{
			n16CanLap = pItemInCon->pProtoType->n16MaxLapNum - pItemInCon->n16Num;
			if(n16CanLap >= pItem->n16Num)
			{
				if(pItem->dwOwnerID != pItemInCon->dwOwnerID
					&& (pItem->IsBind() != pItemInCon->IsBind()))
				{
					continue;
				}
				
				n16Add = pItem->n16Num;
				pItemInCon->n16Num += n16Add;

				// 设置传出参数
				ItemMove.p_item2	= pItemInCon;
				ItemMove.n_num_res1	= 0;
				ItemMove.n_num_res2	= pItemInCon->n16Num;
				ItemMove.b_overlap	= TRUE;

				n16Index = pItemInCon->n16Index;

				// 设置更新数据信息位
				pItemInCon->SetUpdate(EUDBS_Update);

				break;
			}
		}
	}
	
	// 同类物品中放不下,则放入空位
	if(0 == n16Add)
	{		
		// 设置传出参数
		ItemMove.n_num_res1	= pItem->n16Num;
		n16Index = GetOneFreeSpace();

		if(INVALID_VALUE == n16Index)
		{
			print_message(_T("Container is full!<roleid:%u, eConType:%d>"), m_dwOwnerID, GetConType());
			return E_Con_NotEnoughSpace;
		}
		
		return this->Add(pItem, n16Index, bChangeOwner, FALSE);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 向容器中指定位置添加新物品,返回添加成功物品个数(指定位置必须为空)
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::Add(tagItem* pItem, INT16 n16NewIndex, BOOL bChangeOwner/* = TRUE*/, BOOL bCheckAdd/* = TRUE*/)
{
	// 新位置合法性判断
	//if(n16NewIndex < 0 || n16NewIndex >= GetCurSpaceSize())
	//{
	//	ASSERT(n16NewIndex >=0 && n16NewIndex < GetCurSpaceSize());
	//	return INVALID_VALUE;
	//}
	
	// 是否可放入容器
	if(bCheckAdd && !m_pRestrict->CanAdd(pItem))
	{
		return E_Item_CanNotAdd;
	}
	
	// 待放入位置是否合法
	if(!IsOnePlaceFree(n16NewIndex))
	{
		return E_Item_Place_NotFree;
	}

	// 判断物品个数合法性
	if(pItem->n16Num <= 0)
	{
		return INVALID_VALUE;
	}

	INT16 n16Add = Container::Add(pItem, n16NewIndex);
	if(0 == n16Add)
	{
		return E_Item_AddFailed;
	}
	
	//ASSERT(n16Add == pItem->n16Num);
	if(bChangeOwner/* && !pItem->IsBind()*/)
	{
		pItem->SetOwner(m_dwOwnerID, m_dwAccountID);
	}

	// 是否为时限物品
	if(pItem->is_time_limit())
	{
		m_TimeLimitMgr.Add2TimeLeftList(pItem->n64_serial, pItem->pProtoType->dwTimeLimit, pItem->dw1stGainTime, 0);
	}
	
	// 是否为npc绑定物品
	if (pItem->dwBindTime > 0)
	{
		m_TimeLimitMgr.Add2TimeLeftList(pItem->n64_serial, NPC_BIND_TIME, pItem->dwBindTime, 1);
	}
	//// 设置更新数据信息位
	//pItem->SetUpdate(EUDBS_Update);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除指定物品,返回错误码(该处与基类的定义返回值不同)
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::Remove(INT64 n64_serial, BOOL bChangeOwner/* = FALSE*/, BOOL bCheckRemove/* = TRUE*/)
{
	tagItem *pItem = GetItem(n64_serial);
	if(NULL == pItem)
	{
		return E_Item_NotFound;
	}

	if(bCheckRemove && !m_pRestrict->CanRemove(pItem))
	{
		return E_Item_CanNotRemove;
	}

	INT16 n16OldIndex = pItem->n16Index;
	Container::Remove(n64_serial);

	if(bChangeOwner)
	{
		pItem->SetOwner(INVALID_VALUE, INVALID_VALUE);
	}

	// 重置m_n16MinFreeIndex
	if(n16OldIndex < m_n16MinFreeIndex)
	{
		m_n16MinFreeIndex = n16OldIndex;
	}

	// 是否为时限物品
	if(pItem->is_time_limit() || pItem->dwBindTime > 0)
	{
		m_TimeLimitMgr.RemoveFromTimeLeftList(pItem->n64_serial);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 从容器中删除指定个数的指定物品
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::Remove(INT64 n64_serial, INT16 n16Num, BOOL bCheckRemove/* = TRUE*/)
{
	if(n16Num <= 0)
	{
		//ASSERT(n16Num > 0);
		return INVALID_VALUE;
	}

	tagItem *pItem = GetItem(n64_serial);
	if(NULL == pItem)
	{
		return E_Item_NotFound;
	}

	if(bCheckRemove && !m_pRestrict->CanRemove(pItem))
	{
		return E_Item_CanNotRemove;
	}

	// 不够
	if(pItem->n16Num < n16Num)
	{
		// 删除失败,发送监控信息 //??

		//ASSERT(pItem->n16Num >= n16Num);
		return INVALID_VALUE;
	}

	// 刚好
	if(pItem->n16Num == n16Num)
	{
		// 执行到该处，会有内存泄漏 -- 外层应调用别的接口
		ASSERT(0);
		return this->Remove(n64_serial, TRUE, FALSE);
	}

	// 有富余
	pItem->n16Num -= n16Num;

	// 保存
	pItem->SetUpdate(EUDBS_Update);

	return E_Success;
}

////-------------------------------------------------------------------------------------------------------
//// 从容器中删除某类物品，返回成功个数
////-------------------------------------------------------------------------------------------------------
//INT32 ItemContainer::Remove(DWORD dw_data_id, BOOL bCheckRemove/* = TRUE*/)
//{
//	INT32 nNumDel = 0;
//
//	package_list<tagItem*> itemList;
//	GetSameItemList(itemList, dw_data_id);
//
//	tagItem *pItem;
//	itemList.reset_iterator();
//	while(itemList.find_next(pItem))
//	{		
//		nNumDel += this->Remove(pItem->n64_serial, TRUE, bCheckRemove);
//	}
//
//	return nNumDel;
//}
//
////-------------------------------------------------------------------------------------------------------
//// 从容器中删除指定个数的某类物品，返回成功个数
////-------------------------------------------------------------------------------------------------------
//INT32 ItemContainer::Remove(DWORD dw_data_id, INT32 n_num, BOOL bCheckRemove/* = TRUE*/)
//{
//	if(n_num <= 0)
//	{
//		ASSERT(n_num > 0);
//		return 0;
//	}
//	
//	package_list<tagItem*> itemList;
//	INT32 nNumTotal = GetSameItemList(itemList, dw_data_id);
//
//	// 不够
//	if(nNumTotal < n_num)
//	{
//		// 删除失败,发送监控信息
//
//		ASSERT(nNumTotal >= n_num);
//		return 0;
//	}
//
//	// 刚好, 或富余
//	INT16 n16NumDel = 0;
//	INT32 nNumNeedDel = n_num;
//	tagItem *pItem = NULL;
//	itemList.reset_iterator();
//	while(itemList.find_next(pItem) && nNumNeedDel != 0)
//	{
//		if(bCheckRemove && !m_pRestrict->CanRemove(pItem))
//		{
//			ASSERT(0 == n16NumDel);	
//			continue;
//		}
//
//		if(pItem->n16Num <= nNumNeedDel)
//		{
//			n16NumDel = this->Remove(pItem->n64_serial, TRUE, FALSE);
//			ASSERT(n16NumDel == pItem->n16Num);
//			nNumNeedDel -= n16NumDel;
//		}
//		else
//		{
//			pItem->n16Num -= nNumNeedDel;
//			nNumNeedDel = 0;
//
//			// 数据库保存状态
//		}
//	}
//
//	return n_num - nNumNeedDel;
//}

//-------------------------------------------------------------------------------------------------------
// 将物品移动到指定位置
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64Serial1, INT16 n16Index2, OUT s_item_move &ItemMove)
{
	// 0a.目标位置合法性检查
	if(n16Index2 < 0 || n16Index2 >= GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1, *pItem2;

	// 0b.检查移动目标
	pItem1 = GetItem(n64Serial1);
	if(NULL == pItem1 || pItem1->n16Index == n16Index2)
	{
		//ASSERT(pItem1 != NULL);
		//ASSERT(pItem1->n16Index != n16Index2);
		return INVALID_VALUE;
	}

	// 初始化传出参数
	ItemMove.p_item2			= NULL;
	ItemMove.n_num_res1		= pItem1->n16Num;
	ItemMove.n_num_res2		= 0;
	ItemMove.b_create_item	= FALSE;
	ItemMove.b_overlap		= FALSE;
	ItemMove.b_change_pos		= TRUE;

	// 1。目标位置为空
	if(IsOnePlaceFree(n16Index2))
	{
		DWORD dw_error_code = this->Remove(n64Serial1, FALSE);
		if(dw_error_code != E_Success)
		{
			return dw_error_code;
		}

		return this->Add(pItem1, n16Index2, FALSE);
	}

	// 2。目标位置不为空
	pItem2 = GetItem(n16Index2);
	if(!VALID_POINT(pItem2))
	{
		//ASSERT(VALID_POINT(pItem2));
		return INVALID_VALUE;
	}
	
	// 得到目标位置物品信息
	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.物品类性(TypeID || 绑定物品OwnerID)不同,或某一堆物品个数达到了堆叠上限
	if(pItem1->dw_data_id != pItem2->dw_data_id 
		|| (pItem1->dwOwnerID != pItem2->dwOwnerID && (pItem1->IsBind() || pItem2->IsBind()))
		|| pItem1->n16Num >= pItem1->pProtoType->n16MaxLapNum
		|| pItem2->n16Num >= pItem2->pProtoType->n16MaxLapNum
		|| pItem1->IsBind() != pItem2->IsBind())
	{
		Swap(pItem1->n16Index, pItem2->n16Index);

		return E_Success;
	}

	// 2b.同种类物品，且两堆均没有达到堆叠上限
	INT32 n_num = pItem1->pProtoType->n16MaxLapNum - pItem2->n16Num;
	n_num = min(n_num, pItem1->n16Num);
	pItem1->n16Num -= n_num;
	pItem2->n16Num += n_num;

	// 重新设置物品信息
	ItemMove.b_overlap	= TRUE;
	ItemMove.b_change_pos	= FALSE;
	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 设置数据库保存状态
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	// 物品1全部移动到物品2里 -- 需外部检查，并回收内存
	if(0 == pItem1->n16Num)
	{
		this->Remove(pItem1->n64_serial, TRUE, FALSE);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 将指定个数物品移动到指定位置,返回移动成功的个数
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64Serial1, INT16 n16NumMove, INT16 n16Index2, OUT s_item_move &ItemMove)
{	
	if(n16NumMove <= 0)
	{
		//ASSERT(n16NumMove > 0);
		return INVALID_VALUE;
	}
	
	tagItem *pItem1 = GetItem(n64Serial1);
	if(NULL == pItem1)
	{
		//ASSERT(pItem1 != NULL);
		return E_Item_NotFound;
	}

	if(pItem1->n16Num <= n16NumMove)
	{
		// 全部移动
		return this->MoveTo(n64Serial1, n16Index2, ItemMove);
	}

	// 初始化传出参数
	ItemMove.p_item2			= NULL;
	ItemMove.n_num_res1		= pItem1->n16Num;
	ItemMove.n_num_res2		= 0;
	ItemMove.b_create_item	= FALSE;
	ItemMove.b_overlap		= FALSE;
	ItemMove.b_change_pos		= FALSE;

	// 0.目标位置合法性检查
	if(n16Index2 < 0 || n16Index2 >= GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	// 1。目标位置为空
	if(IsOnePlaceFree(n16Index2))
	{
		pItem1->n16Num -= n16NumMove;

		// 设置数据库保存状态
		pItem1->SetUpdate(EUDBS_Update);

		// 重新设置物品信息
		ItemMove.n_num_res1 = pItem1->n16Num;

		// 生成新的堆叠物品
		tagItem *pNewItem = ItemCreator::Create(*pItem1, n16NumMove);

		// 得到目标位置物品信息
		ItemMove.p_item2 = pNewItem;
		ItemMove.n_num_res2 = pNewItem->n16Num;
		ItemMove.b_create_item = TRUE;

		return this->Add(pNewItem, n16Index2, FALSE, FALSE);
	}

	// 2。目标位置不为空
	if(pItem1->n16Index == n16Index2)
	{
		//ASSERT(pItem1->n16Index != n16Index2);
		return INVALID_VALUE;
	}

	// 该物品必存在，否则，不会执行到此处
	tagItem *pItem2 = GetItem(n16Index2);

	// 得到目标位置物品信息
	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.物品类性不同,或移动后超过堆叠上限
	if(pItem1->dw_data_id != pItem2->dw_data_id
		|| (pItem1->dwOwnerID != pItem2->dwOwnerID && (pItem1->IsBind() || pItem2->IsBind()))
		|| pItem2->n16Num + n16NumMove > pItem2->pProtoType->n16MaxLapNum 
		|| pItem1->IsBind() != pItem2->IsBind())
	{
		return INVALID_VALUE;
	}

	// 2b.同种类物品，且移动后目标物品也不会达到堆叠上限
	pItem1->n16Num -= n16NumMove;
	pItem2->n16Num += n16NumMove;

	// 重新设置物品信息
	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;
	ItemMove.b_overlap	= TRUE;

	// 设置数据库保存状态
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 将物品移动到其他容器中(仓库和背包间)，不指定目标容器中具体位置
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, 
							OUT s_item_move &ItemMove, OUT INT16 &n16IndexDst)
{
	// 检查目标容器中是否有空间
	if(conDst.GetFreeSpaceSize() < 1)
	{
		return INVALID_VALUE;
	}
	
	// 从原容器中取出待移动物品
	tagItem *pItem = GetItem(n64SerialSrc);
	if(NULL == pItem)
	{
		// 网络卡时,客户端发重复的消息,可能执行到此
		//ASSERT(pItem != NULL);
		return INVALID_VALUE;
	}

	if(!m_pRestrict->CanMoveToOther(pItem, &conDst))
	{
		return E_Item_CanNotMove_Other;
	}

	this->Remove(n64SerialSrc, FALSE, FALSE);
	
	// 放入目标容器中
	return conDst.Add(pItem, n16IndexDst, ItemMove, FALSE, TRUE);
}

//-------------------------------------------------------------------------------------------------------
// 将物品移动到其他容器中(仓库和背包间)，指定目标背包中具体位置
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, INT16 n16IndexDst, OUT s_item_move &ItemMove)
{
	// 0.目标位置合法性检查
	if(n16IndexDst < 0 || n16IndexDst >= conDst.GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1, *pItem2;

	// 获得待移动物品指针
	pItem1 = GetItem(n64SerialSrc);
	if(NULL == pItem1)
	{
		// 网络卡时,客户端发重复的消息,可能执行到此		
		//ASSERT(pItem1 != NULL);
		return INVALID_VALUE;
	}

	// 初始化传出参数
	ItemMove.p_item2 = NULL;
	ItemMove.n_num_res2 = 0;
	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.b_create_item = FALSE;
	ItemMove.b_overlap = FALSE;
	ItemMove.b_change_pos = TRUE;

	if(!m_pRestrict->CanMoveToOther(pItem1, &conDst))
	{
		return E_Item_CanNotMove_Other;
	}

	// 1。目标位置为空
	if(conDst.IsOnePlaceFree(n16IndexDst))
	{		
		this->Remove(n64SerialSrc);
		return conDst.Add(pItem1, n16IndexDst, TRUE);
	}

	// 2。目标位置不为空
	pItem2 = conDst.GetItem(n16IndexDst);

	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.物品类性不同,或某一堆物品个数达到了堆叠上限
	if(pItem1->dw_data_id != pItem2->dw_data_id
		|| (pItem1->dwOwnerID != pItem2->dwOwnerID && (pItem1->IsBind() || pItem2->IsBind()))
		|| pItem1->n16Num >= pItem1->pProtoType->n16MaxLapNum
		|| pItem2->n16Num >= pItem2->pProtoType->n16MaxLapNum
		|| pItem1->IsBind() != pItem2->IsBind())
	{
		if(!conDst.GetRestrict()->CanMoveToOther(pItem2, this))
		{
			return E_Item_CanNotMove_Other;
		}
		
		// 记录原栏位号
		INT16 n16IndexSrc = pItem1->n16Index;

		// 从容器中清除物品
		Container::Remove(n16IndexSrc);
		((Container *)&conDst)->Remove(n16IndexDst);

		// 放回
		this->Add(pItem2, n16IndexSrc, TRUE);
		conDst.Add(pItem1, n16IndexDst, TRUE);

		return E_Success;
	}

	// 2b.同种类物品，且两堆均没有达到堆叠上限
	INT32 n_num = pItem2->pProtoType->n16MaxLapNum - pItem2->n16Num;
	n_num = min(n_num, pItem1->n16Num);
	pItem1->n16Num -= n_num;
	pItem2->n16Num += n_num;

	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;
	ItemMove.b_overlap	= TRUE;
	ItemMove.b_change_pos = FALSE;

	// 设置数据库保存状态
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	// 物品1全部移动到物品2里
	if(0 == pItem1->n16Num)
	{
		this->Remove(pItem1->n64_serial, TRUE);
	}

	return E_Success;
}

//****************************************** EquipContainer ***********************************************

//-------------------------------------------------------------------------------------------------------
// 构造函数
//-------------------------------------------------------------------------------------------------------
EquipContainer::EquipContainer(EItemConType eConType,  INT16 n16CurSize, INT16 n16MaxSize)
								: Container(eConType, n16CurSize, n16MaxSize), m_TimeLimitMgr(ITEM_UPDATE_TIME)
{
}

//-------------------------------------------------------------------------------------------------------
// 析构函数
//-------------------------------------------------------------------------------------------------------
EquipContainer::~EquipContainer()
{
}

//-------------------------------------------------------------------------------------------------------
// 放入容器中
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Add(tagEquip *pEquip, EEquipPos ePos)
{
	// 检查空位
	if(!IsOnePlaceFree(ePos))
	{
		return E_Item_Place_NotFree;
	}
	
	// 放入容器中
	if(Container::Add(pEquip, ePos) < 1)
	{
		return E_Equip_OnFailed;
	}

	//// 设置数据库保存状态
	//pEquip->SetUpdate(EUDBS_Update);

	// 是否为时限物品
	if(pEquip->is_time_limit())
	{
		m_TimeLimitMgr.Add2TimeLeftList(pEquip->n64_serial, pEquip->pProtoType->dwTimeLimit, pEquip->dw1stGainTime, 0);
	}

	// 是否为npc绑定物品
	if (pEquip->dwBindTime > 0)
	{
		m_TimeLimitMgr.Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 移动(仅限两个戒指位)
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::MoveTo(INT64 n64SerialSrc, EEquipPos ePosDst)
{
	// 检查目标位置是否为戒指位
	if(ePosDst != EEP_Finger1 && ePosDst != EEP_Finger2 &&
		ePosDst != EEP_Wrist1 && ePosDst != EEP_Wrist2)
	{
		return E_Equip_NotRingPos;
	}

	// 获取待移动装备
	tagEquip *pEquip = GetItem(n64SerialSrc);
	if(NULL == pEquip)
	{
		//ASSERT(pEquip != NULL);
		return INVALID_VALUE;
	}

	// 待移动装备是否为ring
	if(M_is_ring(pEquip))
	{
		if (ePosDst != EEP_Finger1 && ePosDst != EEP_Finger2)
		{
			return E_Equip_NotRing;
		}
		
	}
	else if(M_is_Wrist(pEquip))
	{
		if (ePosDst != EEP_Wrist1 && ePosDst != EEP_Wrist1)
		{
			return E_Equip_NotRing;
		}
	}
	else
	{
		return E_Equip_NotRing;
	}

	// 是否已在目标位置
	if(pEquip->n16Index == ePosDst)
	{
		return E_Equip_SamePos;
	}

	// 移动
	if(IsOnePlaceFree(ePosDst))	// 目标位置为空
	{
		Container::Remove(n64SerialSrc);
		return this->Add(pEquip, ePosDst);
	}
	else // 目标位置有装备(ring)
	{
		Container::Swap(pEquip->n16Index, ePosDst);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 穿上装备
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Equip(ItemContainer &bagSrc, INT64 n64SerialSrc, EEquipPos ePosDst)
{
	// 取得待物品指针
	tagItem *pItemNew = bagSrc.GetItem(n64SerialSrc);
	if(NULL == pItemNew)
	{
		// 网络卡时,客户端发重复的消息,可能执行到此
		//ASSERT(pItemNew != NULL);
		return E_Item_NotFound;
	}

	// 判断是否为装备
	if(!MIsEquipment(pItemNew->dw_data_id))
	{
		return E_Item_NotEquipment;
	}

	tagEquip *pEquipNew = (tagEquip *)pItemNew;

	// 得到可装备位置
	EEquipPos ePos = (pEquipNew->pEquipProto)->eEquipPos;
	
	// 检查是否可以装备到目标位置(ring要做特殊判断)
	// if(!(ePos == ePosDst || (ePos + 1 == ePosDst && MIsRing(pEquipNew))))
	if (M_is_ring(pEquipNew))
	{
		if (ePos != ePosDst && (ePos + 1 != ePosDst))
			return E_Equip_InvalidPos;
	}
	else if(M_is_Wrist(pEquipNew))
	{
		if (ePos != ePosDst && (ePos + 1 != ePosDst))
			return E_Equip_InvalidPos;
	}
	else if (ePos != ePosDst)
	{
		return E_Equip_InvalidPos;
	}
	//if(ePos != ePosDst && (ePos + 1 != ePosDst || (!M_is_ring(pEquipNew) && !M_is_Wrist(pEquipNew))))
	//{
	//	return E_Equip_InvalidPos;
	//}

	INT16 n16IndexBag = pEquipNew->n16Index;

	// 目标位不为空
	if(!IsOnePlaceFree(ePosDst))
	{
		// 从背包中取出装备
		((Container *)&bagSrc)->Remove(n16IndexBag);
		// 从装备栏中取下装备
		tagEquip *pEquipOld = Container::Remove((INT16)ePosDst);
		// 将原装备栏中的装备放入背包中
		bagSrc.Add(pEquipOld, n16IndexBag, FALSE);
	}
	else
	{
		// 从背包中取出装备
		bagSrc.Remove(n64SerialSrc);
	}

	// 将原背包中物品装备上
	return this->Add(pEquipNew, ePosDst);
}

//-------------------------------------------------------------------------------------------------------
// 脱下装备
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Unequip(INT64 n64SerialSrc, ItemContainer &bagDst)
{
	//// 检查背包中是否有空间
	//if(bagDst.GetFreeSpaceSize() < 1)
	//{
	//	return E_Bag_NotEnoughSpace;
	//}

	//// 脱下装备
	//tagEquip *pEquipSrc = Container::Remove(n64SerialSrc);
	//if(NULL == pEquipSrc)
	//{
	//	// 网络卡时,客户端发重复的消息,可能执行到此
	//	ASSERT(pEquipSrc != NULL);
	//	return E_Item_NotFound;
	//}

	//// 放入背包中
	//bagDst.Add(pEquipSrc, bagDst.GetOneFreeSpace(), FALSE);

	//return E_Success;

	return Unequip(n64SerialSrc, bagDst, bagDst.GetOneFreeSpace());
}

//-------------------------------------------------------------------------------------------------------
// 脱下装备(指定背包中位置)
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Unequip(INT64 n64SerialSrc, ItemContainer &bagDst, INT16 n16IndexDst)
{
	// 检查目标位置是否合法
	if(n16IndexDst < 0 || n16IndexDst > bagDst.GetCurSpaceSize() - 1)
	{
		return INVALID_VALUE;
	}
	
	// 检查背包中指定位置是否有物品，有则走穿上装备流程
	if(!bagDst.IsOnePlaceFree(n16IndexDst))
	{
		tagEquip *pEquip = GetItem(n64SerialSrc);
		if(NULL == pEquip)
		{
			// 网络卡时,客户端发重复的消息,可能执行到此
			//ASSERT(pEquip != NULL);
			return E_Item_NotFound;
		}
		
		return Equip(bagDst, bagDst.GetItem(n16IndexDst)->n64_serial, (EEquipPos)pEquip->n16Index);
	}

	// 脱下装备
	tagEquip *pEquipSrc = Container::Remove(n64SerialSrc);
	if(NULL == pEquipSrc)
	{
		// 网络卡时,客户端发重复的消息,可能执行到此
		//ASSERT(pEquipSrc != NULL);
		return E_Item_NotFound;
	}

	// 是否为时限物品
	if(pEquipSrc->is_time_limit() || pEquipSrc->byBind == EBS_Bind)
	{
		m_TimeLimitMgr.RemoveFromTimeLeftList(pEquipSrc->n64_serial);
	}

	// 放入背包中
	bagDst.Add(pEquipSrc, n16IndexDst, FALSE);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 主副手交换
//-------------------------------------------------------------------------------------------------------
//DWORD EquipContainer::SwapWeapon()
//{
//	tagEquip *pEquipRight	= GetItem((INT16)EEP_RightHand);
//	tagEquip *pEquipLeft	= GetItem((INT16)EEP_LeftHand);
//
//	if(VALID_POINT(pEquipRight))
//	{
//		Container::Remove((INT16)EEP_RightHand);
//	}
//
//	if(VALID_POINT(pEquipLeft))
//	{
//		Container::Remove((INT16)EEP_LeftHand);
//	}
//
//	if(VALID_POINT(pEquipRight))
//	{
//		Add(pEquipRight, EEP_LeftHand);
//	}
//
//	if(VALID_POINT(pEquipLeft))
//	{
//		Add(pEquipLeft, EEP_RightHand);
//	}
//
//	return E_Success;
//}