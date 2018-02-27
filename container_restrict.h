/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#pragma once

#include "StdAfx.h"
#include "container.h"
#include "../../common/WorldDefine/ItemDefine.h"

//****************** 物品容器操作约束类 -- 基类 **********************
class ContainerRestrict
{
public:
	ContainerRestrict() {};
	virtual ~ContainerRestrict() {};

public:
	VOID init(ItemContainer *pCon) { ASSERT(pCon != NULL); m_pItemContainer = pCon; };

public:
	virtual BOOL CanAdd(const tagItem *pItem)		
	{ 
		if(MIsQuestItem(pItem->pProtoType))
		{
			return FALSE;
		}
		return TRUE; 
	}
	virtual BOOL CanMove(const tagItem *pItem)		{ return TRUE; }
	virtual BOOL CanRemove(const tagItem *pItem)	{ return TRUE; }
	BOOL CanMoveToOther(const tagItem *pItem, ItemContainer *pConDst) 
	{ 
		return (this->CanRemove(pItem) && pConDst->GetRestrict()->CanAdd(pItem)); 
	}

protected:
	ItemContainer*		m_pItemContainer;
};


//************************ 角色仓库操作约束类 ************************
class WareRestrict: public ContainerRestrict
{
public:
	virtual BOOL CanRemove(const tagItem *pItem)
	{
		// 如果为绑定物品,则只有所属角色可以领取
		if(pItem->IsBind())
		{
			if(pItem->dwOwnerID != m_pItemContainer->GetOwnerID())
			{
				return FALSE;
			}
		}

		return TRUE;
	}
};

//************************ 任务物品栏操作约束类 ************************
class QusetItemBarRestrict: public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)
	{
		if(!MIsQuestItem(pItem->pProtoType))
		{
			return FALSE;
		}

		return TRUE;
	}

	virtual BOOL CanMove(const tagItem *pItem)		{ return TRUE; }
	virtual BOOL CanRemove(const tagItem *pItem)	{ return TRUE; }
};

//************************ 百宝袋操作约束类 ************************
class BaiBaoBagRestrict: public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)	{ return FALSE; }
	virtual BOOL CanMove(const tagItem *pItem)	{ return FALSE; }
	virtual BOOL CanRemove(const tagItem *pItem)
	{
		// 如果为绑定物品,则只有所属角色可以领取
		if(pItem->IsBind())
		{
			if(pItem->dwOwnerID != m_pItemContainer->GetOwnerID())
			{
				return FALSE;
			}
		}

		return TRUE;
	}
};

//************************ 帮派仓库操作约束类 ***********************
class GuildWareRestrict : public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)
	{
		if (pItem->IsBind() || pItem->is_time_limit() || pItem->IsLock())
		{
			return FALSE;
		}
		
		return TRUE;
	}
};
//************************ 悬赏任务仓库操作约束类 ***********************
class XSQuestWareRestrict : public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)  { return TRUE; }
	virtual BOOL CanMove(const tagItem *pItem) { return FALSE; }
};