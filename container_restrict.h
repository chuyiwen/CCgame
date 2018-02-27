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

//****************** ��Ʒ��������Լ���� -- ���� **********************
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


//************************ ��ɫ�ֿ����Լ���� ************************
class WareRestrict: public ContainerRestrict
{
public:
	virtual BOOL CanRemove(const tagItem *pItem)
	{
		// ���Ϊ����Ʒ,��ֻ��������ɫ������ȡ
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

//************************ ������Ʒ������Լ���� ************************
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

//************************ �ٱ�������Լ���� ************************
class BaiBaoBagRestrict: public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)	{ return FALSE; }
	virtual BOOL CanMove(const tagItem *pItem)	{ return FALSE; }
	virtual BOOL CanRemove(const tagItem *pItem)
	{
		// ���Ϊ����Ʒ,��ֻ��������ɫ������ȡ
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

//************************ ���ɲֿ����Լ���� ***********************
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
//************************ ��������ֿ����Լ���� ***********************
class XSQuestWareRestrict : public ContainerRestrict
{
public:
	virtual BOOL CanAdd(const tagItem *pItem)  { return TRUE; }
	virtual BOOL CanMove(const tagItem *pItem) { return FALSE; }
};