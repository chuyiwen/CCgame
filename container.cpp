/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ʒ����

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
// ���캯��
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
// ��������
//-------------------------------------------------------------------------------------------------------
ItemContainer::~ItemContainer()
{
	m_n16MinFreeIndex = INVALID_VALUE;

	SAFE_DELETE(m_pRestrict);
}

//-------------------------------------------------------------------------------------------------------
// ���������������Ʒ,����ָ�����λ��,���ش�����
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

	// ��ʼ����������
	n16Index = INVALID_VALUE;
	ItemMove.p_item2	= NULL;
	ItemMove.n_num_res1	= 0;
	ItemMove.n_num_res2	= 0;
	ItemMove.b_create_item = FALSE;
	ItemMove.b_overlap = FALSE;
	ItemMove.b_change_pos = TRUE;

	INT16 n16Add = 0;
	
	if(pItem->pProtoType->n16MaxLapNum > 1)	// �ɶѵ���Ʒ
	{
		package_list<tagItem*> itemList;
		GetSameBindItemList(itemList, pItem->dw_data_id, pItem->IsBind());

		tagItem *pItemInCon;
		INT16 n16CanLap;

		// ��ͬ����Ʒ����һ��
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

				// ���ô�������
				ItemMove.p_item2	= pItemInCon;
				ItemMove.n_num_res1	= 0;
				ItemMove.n_num_res2	= pItemInCon->n16Num;
				ItemMove.b_overlap	= TRUE;

				n16Index = pItemInCon->n16Index;

				// ���ø���������Ϣλ
				pItemInCon->SetUpdate(EUDBS_Update);

				break;
			}
		}
	}
	
	// ͬ����Ʒ�зŲ���,������λ
	if(0 == n16Add)
	{		
		// ���ô�������
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
// ��������ָ��λ���������Ʒ,������ӳɹ���Ʒ����(ָ��λ�ñ���Ϊ��)
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::Add(tagItem* pItem, INT16 n16NewIndex, BOOL bChangeOwner/* = TRUE*/, BOOL bCheckAdd/* = TRUE*/)
{
	// ��λ�úϷ����ж�
	//if(n16NewIndex < 0 || n16NewIndex >= GetCurSpaceSize())
	//{
	//	ASSERT(n16NewIndex >=0 && n16NewIndex < GetCurSpaceSize());
	//	return INVALID_VALUE;
	//}
	
	// �Ƿ�ɷ�������
	if(bCheckAdd && !m_pRestrict->CanAdd(pItem))
	{
		return E_Item_CanNotAdd;
	}
	
	// ������λ���Ƿ�Ϸ�
	if(!IsOnePlaceFree(n16NewIndex))
	{
		return E_Item_Place_NotFree;
	}

	// �ж���Ʒ�����Ϸ���
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

	// �Ƿ�Ϊʱ����Ʒ
	if(pItem->is_time_limit())
	{
		m_TimeLimitMgr.Add2TimeLeftList(pItem->n64_serial, pItem->pProtoType->dwTimeLimit, pItem->dw1stGainTime, 0);
	}
	
	// �Ƿ�Ϊnpc����Ʒ
	if (pItem->dwBindTime > 0)
	{
		m_TimeLimitMgr.Add2TimeLeftList(pItem->n64_serial, NPC_BIND_TIME, pItem->dwBindTime, 1);
	}
	//// ���ø���������Ϣλ
	//pItem->SetUpdate(EUDBS_Update);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��������ɾ��ָ����Ʒ,���ش�����(�ô������Ķ��巵��ֵ��ͬ)
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

	// ����m_n16MinFreeIndex
	if(n16OldIndex < m_n16MinFreeIndex)
	{
		m_n16MinFreeIndex = n16OldIndex;
	}

	// �Ƿ�Ϊʱ����Ʒ
	if(pItem->is_time_limit() || pItem->dwBindTime > 0)
	{
		m_TimeLimitMgr.RemoveFromTimeLeftList(pItem->n64_serial);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��������ɾ��ָ��������ָ����Ʒ
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

	// ����
	if(pItem->n16Num < n16Num)
	{
		// ɾ��ʧ��,���ͼ����Ϣ //??

		//ASSERT(pItem->n16Num >= n16Num);
		return INVALID_VALUE;
	}

	// �պ�
	if(pItem->n16Num == n16Num)
	{
		// ִ�е��ô��������ڴ�й© -- ���Ӧ���ñ�Ľӿ�
		ASSERT(0);
		return this->Remove(n64_serial, TRUE, FALSE);
	}

	// �и���
	pItem->n16Num -= n16Num;

	// ����
	pItem->SetUpdate(EUDBS_Update);

	return E_Success;
}

////-------------------------------------------------------------------------------------------------------
//// ��������ɾ��ĳ����Ʒ�����سɹ�����
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
//// ��������ɾ��ָ��������ĳ����Ʒ�����سɹ�����
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
//	// ����
//	if(nNumTotal < n_num)
//	{
//		// ɾ��ʧ��,���ͼ����Ϣ
//
//		ASSERT(nNumTotal >= n_num);
//		return 0;
//	}
//
//	// �պ�, ����
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
//			// ���ݿⱣ��״̬
//		}
//	}
//
//	return n_num - nNumNeedDel;
//}

//-------------------------------------------------------------------------------------------------------
// ����Ʒ�ƶ���ָ��λ��
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64Serial1, INT16 n16Index2, OUT s_item_move &ItemMove)
{
	// 0a.Ŀ��λ�úϷ��Լ��
	if(n16Index2 < 0 || n16Index2 >= GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1, *pItem2;

	// 0b.����ƶ�Ŀ��
	pItem1 = GetItem(n64Serial1);
	if(NULL == pItem1 || pItem1->n16Index == n16Index2)
	{
		//ASSERT(pItem1 != NULL);
		//ASSERT(pItem1->n16Index != n16Index2);
		return INVALID_VALUE;
	}

	// ��ʼ����������
	ItemMove.p_item2			= NULL;
	ItemMove.n_num_res1		= pItem1->n16Num;
	ItemMove.n_num_res2		= 0;
	ItemMove.b_create_item	= FALSE;
	ItemMove.b_overlap		= FALSE;
	ItemMove.b_change_pos		= TRUE;

	// 1��Ŀ��λ��Ϊ��
	if(IsOnePlaceFree(n16Index2))
	{
		DWORD dw_error_code = this->Remove(n64Serial1, FALSE);
		if(dw_error_code != E_Success)
		{
			return dw_error_code;
		}

		return this->Add(pItem1, n16Index2, FALSE);
	}

	// 2��Ŀ��λ�ò�Ϊ��
	pItem2 = GetItem(n16Index2);
	if(!VALID_POINT(pItem2))
	{
		//ASSERT(VALID_POINT(pItem2));
		return INVALID_VALUE;
	}
	
	// �õ�Ŀ��λ����Ʒ��Ϣ
	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.��Ʒ����(TypeID || ����ƷOwnerID)��ͬ,��ĳһ����Ʒ�����ﵽ�˶ѵ�����
	if(pItem1->dw_data_id != pItem2->dw_data_id 
		|| (pItem1->dwOwnerID != pItem2->dwOwnerID && (pItem1->IsBind() || pItem2->IsBind()))
		|| pItem1->n16Num >= pItem1->pProtoType->n16MaxLapNum
		|| pItem2->n16Num >= pItem2->pProtoType->n16MaxLapNum
		|| pItem1->IsBind() != pItem2->IsBind())
	{
		Swap(pItem1->n16Index, pItem2->n16Index);

		return E_Success;
	}

	// 2b.ͬ������Ʒ�������Ѿ�û�дﵽ�ѵ�����
	INT32 n_num = pItem1->pProtoType->n16MaxLapNum - pItem2->n16Num;
	n_num = min(n_num, pItem1->n16Num);
	pItem1->n16Num -= n_num;
	pItem2->n16Num += n_num;

	// ����������Ʒ��Ϣ
	ItemMove.b_overlap	= TRUE;
	ItemMove.b_change_pos	= FALSE;
	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// �������ݿⱣ��״̬
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	// ��Ʒ1ȫ���ƶ�����Ʒ2�� -- ���ⲿ��飬�������ڴ�
	if(0 == pItem1->n16Num)
	{
		this->Remove(pItem1->n64_serial, TRUE, FALSE);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ��ָ��������Ʒ�ƶ���ָ��λ��,�����ƶ��ɹ��ĸ���
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
		// ȫ���ƶ�
		return this->MoveTo(n64Serial1, n16Index2, ItemMove);
	}

	// ��ʼ����������
	ItemMove.p_item2			= NULL;
	ItemMove.n_num_res1		= pItem1->n16Num;
	ItemMove.n_num_res2		= 0;
	ItemMove.b_create_item	= FALSE;
	ItemMove.b_overlap		= FALSE;
	ItemMove.b_change_pos		= FALSE;

	// 0.Ŀ��λ�úϷ��Լ��
	if(n16Index2 < 0 || n16Index2 >= GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	// 1��Ŀ��λ��Ϊ��
	if(IsOnePlaceFree(n16Index2))
	{
		pItem1->n16Num -= n16NumMove;

		// �������ݿⱣ��״̬
		pItem1->SetUpdate(EUDBS_Update);

		// ����������Ʒ��Ϣ
		ItemMove.n_num_res1 = pItem1->n16Num;

		// �����µĶѵ���Ʒ
		tagItem *pNewItem = ItemCreator::Create(*pItem1, n16NumMove);

		// �õ�Ŀ��λ����Ʒ��Ϣ
		ItemMove.p_item2 = pNewItem;
		ItemMove.n_num_res2 = pNewItem->n16Num;
		ItemMove.b_create_item = TRUE;

		return this->Add(pNewItem, n16Index2, FALSE, FALSE);
	}

	// 2��Ŀ��λ�ò�Ϊ��
	if(pItem1->n16Index == n16Index2)
	{
		//ASSERT(pItem1->n16Index != n16Index2);
		return INVALID_VALUE;
	}

	// ����Ʒ�ش��ڣ����򣬲���ִ�е��˴�
	tagItem *pItem2 = GetItem(n16Index2);

	// �õ�Ŀ��λ����Ʒ��Ϣ
	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.��Ʒ���Բ�ͬ,���ƶ��󳬹��ѵ�����
	if(pItem1->dw_data_id != pItem2->dw_data_id
		|| (pItem1->dwOwnerID != pItem2->dwOwnerID && (pItem1->IsBind() || pItem2->IsBind()))
		|| pItem2->n16Num + n16NumMove > pItem2->pProtoType->n16MaxLapNum 
		|| pItem1->IsBind() != pItem2->IsBind())
	{
		return INVALID_VALUE;
	}

	// 2b.ͬ������Ʒ�����ƶ���Ŀ����ƷҲ����ﵽ�ѵ�����
	pItem1->n16Num -= n16NumMove;
	pItem2->n16Num += n16NumMove;

	// ����������Ʒ��Ϣ
	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;
	ItemMove.b_overlap	= TRUE;

	// �������ݿⱣ��״̬
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ����Ʒ�ƶ�������������(�ֿ�ͱ�����)����ָ��Ŀ�������о���λ��
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, 
							OUT s_item_move &ItemMove, OUT INT16 &n16IndexDst)
{
	// ���Ŀ���������Ƿ��пռ�
	if(conDst.GetFreeSpaceSize() < 1)
	{
		return INVALID_VALUE;
	}
	
	// ��ԭ������ȡ�����ƶ���Ʒ
	tagItem *pItem = GetItem(n64SerialSrc);
	if(NULL == pItem)
	{
		// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���
		//ASSERT(pItem != NULL);
		return INVALID_VALUE;
	}

	if(!m_pRestrict->CanMoveToOther(pItem, &conDst))
	{
		return E_Item_CanNotMove_Other;
	}

	this->Remove(n64SerialSrc, FALSE, FALSE);
	
	// ����Ŀ��������
	return conDst.Add(pItem, n16IndexDst, ItemMove, FALSE, TRUE);
}

//-------------------------------------------------------------------------------------------------------
// ����Ʒ�ƶ�������������(�ֿ�ͱ�����)��ָ��Ŀ�걳���о���λ��
//-------------------------------------------------------------------------------------------------------
DWORD ItemContainer::MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, INT16 n16IndexDst, OUT s_item_move &ItemMove)
{
	// 0.Ŀ��λ�úϷ��Լ��
	if(n16IndexDst < 0 || n16IndexDst >= conDst.GetCurSpaceSize())
	{
		return INVALID_VALUE;
	}

	tagItem *pItem1, *pItem2;

	// ��ô��ƶ���Ʒָ��
	pItem1 = GetItem(n64SerialSrc);
	if(NULL == pItem1)
	{
		// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���		
		//ASSERT(pItem1 != NULL);
		return INVALID_VALUE;
	}

	// ��ʼ����������
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

	// 1��Ŀ��λ��Ϊ��
	if(conDst.IsOnePlaceFree(n16IndexDst))
	{		
		this->Remove(n64SerialSrc);
		return conDst.Add(pItem1, n16IndexDst, TRUE);
	}

	// 2��Ŀ��λ�ò�Ϊ��
	pItem2 = conDst.GetItem(n16IndexDst);

	ItemMove.p_item2 = pItem2;
	ItemMove.n_num_res2 = pItem2->n16Num;

	// 2a.��Ʒ���Բ�ͬ,��ĳһ����Ʒ�����ﵽ�˶ѵ�����
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
		
		// ��¼ԭ��λ��
		INT16 n16IndexSrc = pItem1->n16Index;

		// �������������Ʒ
		Container::Remove(n16IndexSrc);
		((Container *)&conDst)->Remove(n16IndexDst);

		// �Ż�
		this->Add(pItem2, n16IndexSrc, TRUE);
		conDst.Add(pItem1, n16IndexDst, TRUE);

		return E_Success;
	}

	// 2b.ͬ������Ʒ�������Ѿ�û�дﵽ�ѵ�����
	INT32 n_num = pItem2->pProtoType->n16MaxLapNum - pItem2->n16Num;
	n_num = min(n_num, pItem1->n16Num);
	pItem1->n16Num -= n_num;
	pItem2->n16Num += n_num;

	ItemMove.n_num_res1 = pItem1->n16Num;
	ItemMove.n_num_res2 = pItem2->n16Num;
	ItemMove.b_overlap	= TRUE;
	ItemMove.b_change_pos = FALSE;

	// �������ݿⱣ��״̬
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	// ��Ʒ1ȫ���ƶ�����Ʒ2��
	if(0 == pItem1->n16Num)
	{
		this->Remove(pItem1->n64_serial, TRUE);
	}

	return E_Success;
}

//****************************************** EquipContainer ***********************************************

//-------------------------------------------------------------------------------------------------------
// ���캯��
//-------------------------------------------------------------------------------------------------------
EquipContainer::EquipContainer(EItemConType eConType,  INT16 n16CurSize, INT16 n16MaxSize)
								: Container(eConType, n16CurSize, n16MaxSize), m_TimeLimitMgr(ITEM_UPDATE_TIME)
{
}

//-------------------------------------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------------------------------------
EquipContainer::~EquipContainer()
{
}

//-------------------------------------------------------------------------------------------------------
// ����������
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Add(tagEquip *pEquip, EEquipPos ePos)
{
	// ����λ
	if(!IsOnePlaceFree(ePos))
	{
		return E_Item_Place_NotFree;
	}
	
	// ����������
	if(Container::Add(pEquip, ePos) < 1)
	{
		return E_Equip_OnFailed;
	}

	//// �������ݿⱣ��״̬
	//pEquip->SetUpdate(EUDBS_Update);

	// �Ƿ�Ϊʱ����Ʒ
	if(pEquip->is_time_limit())
	{
		m_TimeLimitMgr.Add2TimeLeftList(pEquip->n64_serial, pEquip->pProtoType->dwTimeLimit, pEquip->dw1stGainTime, 0);
	}

	// �Ƿ�Ϊnpc����Ʒ
	if (pEquip->dwBindTime > 0)
	{
		m_TimeLimitMgr.Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �ƶ�(����������ָλ)
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::MoveTo(INT64 n64SerialSrc, EEquipPos ePosDst)
{
	// ���Ŀ��λ���Ƿ�Ϊ��ָλ
	if(ePosDst != EEP_Finger1 && ePosDst != EEP_Finger2 &&
		ePosDst != EEP_Wrist1 && ePosDst != EEP_Wrist2)
	{
		return E_Equip_NotRingPos;
	}

	// ��ȡ���ƶ�װ��
	tagEquip *pEquip = GetItem(n64SerialSrc);
	if(NULL == pEquip)
	{
		//ASSERT(pEquip != NULL);
		return INVALID_VALUE;
	}

	// ���ƶ�װ���Ƿ�Ϊring
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

	// �Ƿ�����Ŀ��λ��
	if(pEquip->n16Index == ePosDst)
	{
		return E_Equip_SamePos;
	}

	// �ƶ�
	if(IsOnePlaceFree(ePosDst))	// Ŀ��λ��Ϊ��
	{
		Container::Remove(n64SerialSrc);
		return this->Add(pEquip, ePosDst);
	}
	else // Ŀ��λ����װ��(ring)
	{
		Container::Swap(pEquip->n16Index, ePosDst);
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// ����װ��
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Equip(ItemContainer &bagSrc, INT64 n64SerialSrc, EEquipPos ePosDst)
{
	// ȡ�ô���Ʒָ��
	tagItem *pItemNew = bagSrc.GetItem(n64SerialSrc);
	if(NULL == pItemNew)
	{
		// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���
		//ASSERT(pItemNew != NULL);
		return E_Item_NotFound;
	}

	// �ж��Ƿ�Ϊװ��
	if(!MIsEquipment(pItemNew->dw_data_id))
	{
		return E_Item_NotEquipment;
	}

	tagEquip *pEquipNew = (tagEquip *)pItemNew;

	// �õ���װ��λ��
	EEquipPos ePos = (pEquipNew->pEquipProto)->eEquipPos;
	
	// ����Ƿ����װ����Ŀ��λ��(ringҪ�������ж�)
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

	// Ŀ��λ��Ϊ��
	if(!IsOnePlaceFree(ePosDst))
	{
		// �ӱ�����ȡ��װ��
		((Container *)&bagSrc)->Remove(n16IndexBag);
		// ��װ������ȡ��װ��
		tagEquip *pEquipOld = Container::Remove((INT16)ePosDst);
		// ��ԭװ�����е�װ�����뱳����
		bagSrc.Add(pEquipOld, n16IndexBag, FALSE);
	}
	else
	{
		// �ӱ�����ȡ��װ��
		bagSrc.Remove(n64SerialSrc);
	}

	// ��ԭ��������Ʒװ����
	return this->Add(pEquipNew, ePosDst);
}

//-------------------------------------------------------------------------------------------------------
// ����װ��
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Unequip(INT64 n64SerialSrc, ItemContainer &bagDst)
{
	//// ��鱳�����Ƿ��пռ�
	//if(bagDst.GetFreeSpaceSize() < 1)
	//{
	//	return E_Bag_NotEnoughSpace;
	//}

	//// ����װ��
	//tagEquip *pEquipSrc = Container::Remove(n64SerialSrc);
	//if(NULL == pEquipSrc)
	//{
	//	// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���
	//	ASSERT(pEquipSrc != NULL);
	//	return E_Item_NotFound;
	//}

	//// ���뱳����
	//bagDst.Add(pEquipSrc, bagDst.GetOneFreeSpace(), FALSE);

	//return E_Success;

	return Unequip(n64SerialSrc, bagDst, bagDst.GetOneFreeSpace());
}

//-------------------------------------------------------------------------------------------------------
// ����װ��(ָ��������λ��)
//-------------------------------------------------------------------------------------------------------
DWORD EquipContainer::Unequip(INT64 n64SerialSrc, ItemContainer &bagDst, INT16 n16IndexDst)
{
	// ���Ŀ��λ���Ƿ�Ϸ�
	if(n16IndexDst < 0 || n16IndexDst > bagDst.GetCurSpaceSize() - 1)
	{
		return INVALID_VALUE;
	}
	
	// ��鱳����ָ��λ���Ƿ�����Ʒ�������ߴ���װ������
	if(!bagDst.IsOnePlaceFree(n16IndexDst))
	{
		tagEquip *pEquip = GetItem(n64SerialSrc);
		if(NULL == pEquip)
		{
			// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���
			//ASSERT(pEquip != NULL);
			return E_Item_NotFound;
		}
		
		return Equip(bagDst, bagDst.GetItem(n16IndexDst)->n64_serial, (EEquipPos)pEquip->n16Index);
	}

	// ����װ��
	tagEquip *pEquipSrc = Container::Remove(n64SerialSrc);
	if(NULL == pEquipSrc)
	{
		// ���翨ʱ,�ͻ��˷��ظ�����Ϣ,����ִ�е���
		//ASSERT(pEquipSrc != NULL);
		return E_Item_NotFound;
	}

	// �Ƿ�Ϊʱ����Ʒ
	if(pEquipSrc->is_time_limit() || pEquipSrc->byBind == EBS_Bind)
	{
		m_TimeLimitMgr.RemoveFromTimeLeftList(pEquipSrc->n64_serial);
	}

	// ���뱳����
	bagDst.Add(pEquipSrc, n16IndexDst, FALSE);

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// �����ֽ���
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