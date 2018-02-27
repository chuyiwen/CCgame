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

#include "container_template.h"
#include "time_limit_mgr.h"
#include "../../common/WorldDefine/ItemDefine.h"

class ContainerRestrict;

struct s_item_move;
//-----------------------------------------------------------------------------
// ��Ʒ&װ������
//-----------------------------------------------------------------------------
class ItemContainer: public Container<tagItem, INT64>
{
public:
	ItemContainer(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize, 
					DWORD dwOwnerID, DWORD dw_account_id, ContainerRestrict *pRestrict = NULL);
	~ItemContainer();

	VOID Update();

public:
	// ���������������Ʒ,����ָ�����λ��,���ش�����
	DWORD Add(tagItem* pItem, OUT INT16 &n16Index, OUT s_item_move &ItemMove, BOOL bCheckAdd = TRUE, BOOL bChangeOwner = TRUE);
	
	// ��������ָ��λ���������Ʒ,���ش�����(ָ��λ�ñ���Ϊ��).
	DWORD Add(tagItem* pItem, INT16 n16NewIndex, BOOL bChangeOwner = TRUE, BOOL bCheckAdd = TRUE);

	// ��������ɾ��ָ����Ʒ,���ش�����
	DWORD Remove(INT64 n64_serial, BOOL bChangeOwner = FALSE, BOOL bCheckRemove = TRUE);
	DWORD Remove(INT64 n64_serial, INT16 n16Num, BOOL bCheckRemove = TRUE);
	
	// ����Ʒ�ƶ���ָ��λ��(ͬһ������)
	DWORD MoveTo(INT64 n64Serial1, INT16 n16Index2, OUT s_item_move &ItemMove);
	DWORD MoveTo(INT64 n64Serial1, INT16 n16NumMove, INT16 n16Index2, OUT s_item_move &ItemMove);

	// ����Ʒ�ƶ�������������(�ֿ�ͱ�����)
	DWORD MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, OUT s_item_move &ItemMove, OUT INT16 &n16IndexDst);
	DWORD MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, INT16 n16IndexDst, OUT s_item_move &ItemMove);

	// ����
	VOID IncreaseSize(INT16 n16Size);

	// ��ָ��λ������
	BOOL Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	BOOL ReorderEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);
	
public:
	// ���һ����λ��ʧ�ܷ���INVALID_VALUE
	INT16 GetOneFreeSpace();

	// ���������������Ʒ����Ϊdw_data_id����Ʒ����
	INT32 GetSameItemCount(IN DWORD dw_data_id);
	INT32 GetSameBindItemCount(IN DWORD dw_data_id, BOOL bBind = FALSE);
	// �����������Ʒ����Ϊdw_data_id��lis, ����ʵ�ʻ�ø���t -- ָ��nNumʱ�����ҵ�nNum����Ʒ����
	INT32 GetSameItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, IN INT32 n_num = INT_MAX);
	
	INT32 GetSameBindItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, BOOL bBind, IN INT32 n_num = INT_MAX);

	ContainerRestrict*	GetRestrict()	const;
	DWORD				GetOwnerID()	const;
	
	TimeLimitMgr<INT64>& GetTimeLimitMgr() { return m_TimeLimitMgr; }
	// ʱ�޹���ӿ�
	package_list<INT64>& GetNeedDelList();
	VOID ClearNeedDelList();
	
	package_list<INT64>& GetChangeBindList();
	VOID ClearChangeBindList();
private:
	// ���������п���λ�õ���С�±�
	VOID ResetMinFreeIndex();

protected:
	DWORD				m_dwOwnerID;			// ����������
	DWORD				m_dwAccountID;			// ����˺�ID, �ٱ����ͽ�ɫ�ֿ���(ͬһ�ʺ��½�ɫ����)

private:
	// ������Ա
	INT16				m_n16MinFreeIndex;		// �����п���λ�õ���С�±�(== m_n16CurSizeʱ���޿���λ��)
	ContainerRestrict*	m_pRestrict;			// ��Ʒ��������Լ����
	TimeLimitMgr<INT64>	m_TimeLimitMgr;			// ʱ����Ʒ������
};


//-----------------------------------------------------------------------------
// װ������
//-----------------------------------------------------------------------------
class EquipContainer: public Container<tagEquip, INT64>
{
public:
	EquipContainer(EItemConType eConType,  INT16 n16CurSize, INT16 n16MaxSize);
	~EquipContainer();

	VOID Update();

public: 
	// ����װ��
	DWORD Equip(ItemContainer &bagSrc, INT64 n64SerialSrc, EEquipPos ePosDst);
	// ����װ��
	DWORD Unequip(INT64 n64SerialSrc, ItemContainer &bagDst);
	// ����װ��(ָ��������λ��)
	DWORD Unequip(INT64 n64SerialSrc, ItemContainer &bagDst, INT16 n16IndexDst);
	// �ƶ�(����������ָλ)
	DWORD MoveTo(INT64 n64SerialSrc, EEquipPos ePosDst);
	// �����ֽ���
	//DWORD SwapWeapon();
	TimeLimitMgr<INT64>& GetTimeLimitMgr() { return m_TimeLimitMgr; }
	// ʱ�޹���ӿ�
	package_list<INT64>& GetNeedDelList();
	VOID ClearNeedDelList();
	
	package_list<INT64>& GetChangeBindList();
	VOID ClearChangeBindList();

public:
	// װ��
	DWORD Add(tagEquip *pEquip, EEquipPos ePos);

private:
	// ����
	TimeLimitMgr<INT64>	m_TimeLimitMgr;			// ʱ����Ʒ������
};

//************************ ��ͨ��������ʵ�� ************************//

//----------------------------------------------------------------------
// ����
//----------------------------------------------------------------------
inline VOID ItemContainer::IncreaseSize(INT16 n16Size) 
{ 
	INT16 n16CurSize = GetCurSpaceSize();

	if(n16Size <= 0 || n16Size + n16CurSize > GetMaxSpaceSize())
	{
		ASSERT(n16Size > 0);
		ASSERT(n16Size + n16CurSize <= GetMaxSpaceSize());
		return ;
	}

	if(GetOneFreeSpace() == INVALID_VALUE)
	{
		m_n16MinFreeIndex = n16CurSize;
	}

	Container<tagItem>::Resize(n16Size + n16CurSize);
}

//----------------------------------------------------------------------
// ��ָ��λ������
//----------------------------------------------------------------------
inline BOOL ItemContainer::Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	if(Container::Reorder(pData, pOutData, n16Num))
	{
		// ��С�����±�������
		m_n16MinFreeIndex = n16Num;
		ResetMinFreeIndex();
		return TRUE;
	}

	return FALSE;
}

inline BOOL ItemContainer::ReorderEx(IN LPVOID pData, OUT LPVOID pOutData, 
									 OUT INT16 &n16OutNum, const INT16 n16Num)
{
	if(Container::ReorderEx(pData, pOutData, n16OutNum, n16Num))
	{
		// ��С�����±�������
		ResetMinFreeIndex();
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------
// ���һ����λ��ʧ�ܷ���INVALID_VALUE
//----------------------------------------------------------------------
inline INT16 ItemContainer::GetOneFreeSpace()
{
	ResetMinFreeIndex();
	return m_n16MinFreeIndex >= GetCurSpaceSize() ? INVALID_VALUE : m_n16MinFreeIndex;
}

//----------------------------------------------------------------------
// ���������������Ʒ����Ϊdw_data_id����Ʒ����
//----------------------------------------------------------------------
__forceinline INT32 ItemContainer::GetSameItemCount(IN DWORD dw_data_id)
{
	INT32 nNumRet = 0;

	for(INT16 i=0; i<Container::GetCurSpaceSize(); ++i)
	{
		if(!Container::IsOnePlaceFree(i) && (Container::GetItem(i))->dw_data_id == dw_data_id)
		{
			nNumRet += Container::GetItem(i)->n16Num;
		}
	}

	return nNumRet;
}

__forceinline INT32 ItemContainer::GetSameBindItemCount(IN DWORD dw_data_id, BOOL bBind)
{
	INT32 nNumRet = 0;

	for(INT16 i=0; i<Container::GetCurSpaceSize(); ++i)
	{
		if(!Container::IsOnePlaceFree(i) && (Container::GetItem(i))->dw_data_id == dw_data_id && (Container::GetItem(i))->IsBind() == bBind)
		{
			nNumRet += Container::GetItem(i)->n16Num;
		}
	}

	return nNumRet;
}
//----------------------------------------------------------------------
// �����������Ʒ����Ϊdw_data_id��lis, ����ʵ�ʻ�ø���t -- ָ��n_numʱ�����ҵ�n_num����Ʒ����
//----------------------------------------------------------------------
__forceinline INT32 ItemContainer::GetSameItemList(OUT package_list<tagItem*> &list, 
												   IN DWORD dw_data_id, IN INT32 n_num/* = INT_MAX*/)
{
	INT32 nNumRet = 0;

	for(INT16 i=0; i<Container::GetCurSpaceSize(); ++i)
	{
		if(!Container::IsOnePlaceFree(i) && (Container::GetItem(i))->dw_data_id == dw_data_id)
		{
			nNumRet += Container::GetItem(i)->n16Num;
			list.push_back(Container::GetItem(i));

			if(nNumRet >= n_num)
			{
				break;
			}
		}
	}

	return nNumRet;
}

__forceinline INT32 ItemContainer::GetSameBindItemList(OUT package_list<tagItem*> &list, 
												   IN DWORD dw_data_id, BOOL bBind, IN INT32 n_num/* = INT_MAX*/)
{
	INT32 nNumRet = 0;

	for(INT16 i=0; i<Container::GetCurSpaceSize(); ++i)
	{
		if(!Container::IsOnePlaceFree(i) && (Container::GetItem(i)->dw_data_id == dw_data_id )&& (Container::GetItem(i)->IsBind() == bBind))
		{
			nNumRet += Container::GetItem(i)->n16Num;
			list.push_back(Container::GetItem(i));

			if(nNumRet >= n_num)
			{
				break;
			}
		}
	}

	return nNumRet;
}
//----------------------------------------------------------------------
// ��ȡԼ����ָ��
//----------------------------------------------------------------------
inline ContainerRestrict* ItemContainer::GetRestrict() const
{
	return m_pRestrict;
}

//----------------------------------------------------------------------
// ��ȡ����������RoleID
//----------------------------------------------------------------------
inline DWORD ItemContainer::GetOwnerID() const
{
	return m_dwOwnerID;
}

//----------------------------------------------------------------------
// ���������п���λ�õ���С�±�
//----------------------------------------------------------------------
__forceinline VOID ItemContainer::ResetMinFreeIndex()
{
	if(GetFreeSpaceSize() <= 0)
	{
		m_n16MinFreeIndex = INVALID_VALUE;
		return;
	}

	while(m_n16MinFreeIndex < GetCurSpaceSize())
	{
		if(IsOnePlaceFree(m_n16MinFreeIndex))
		{
			return;
		}

		++m_n16MinFreeIndex;
	}

	// ����д���������������bug��������ѭ��
	if(GetCurSpaceSize() == m_n16MinFreeIndex)
	{
		m_n16MinFreeIndex = 0;
	}

	while(m_n16MinFreeIndex < GetCurSpaceSize())
	{
		if(IsOnePlaceFree(m_n16MinFreeIndex))
		{
			return;
		}

		++m_n16MinFreeIndex;
	}

	// ��Ҫ��ִ�е��˴���˵�������е�m_n16RemainSize������
	ASSERT(0);
}

//----------------------------------------------------------------------
// update
//----------------------------------------------------------------------
inline VOID ItemContainer::Update()
{
	m_TimeLimitMgr.Update();
}

//----------------------------------------------------------------------
// ��ȡ��ɾ���б�
//----------------------------------------------------------------------
inline package_list<INT64>& ItemContainer::GetNeedDelList()
{
	return m_TimeLimitMgr.GetNeedDelList();
}

//----------------------------------------------------------------------
// ��մ�ɾ���б�
//----------------------------------------------------------------------
inline VOID ItemContainer::ClearNeedDelList()
{
	m_TimeLimitMgr.ClearNeedDelList();
}

//----------------------------------------------------------------------
// ��ȡ������б�
//----------------------------------------------------------------------
inline package_list<INT64>& ItemContainer::GetChangeBindList()
{
	return m_TimeLimitMgr.GetNeedChangeBindList();
}

//----------------------------------------------------------------------
// ��մ�����б�
//----------------------------------------------------------------------
inline VOID ItemContainer::ClearChangeBindList()
{
	m_TimeLimitMgr.ClearNeedChangeBindList();
}

//************************ װ��������ʵ�� ************************//

//----------------------------------------------------------------------
// update
//----------------------------------------------------------------------
inline VOID EquipContainer::Update()
{
	m_TimeLimitMgr.Update();
}

//----------------------------------------------------------------------
// ��ȡ��ɾ���б�
//----------------------------------------------------------------------
inline package_list<INT64>& EquipContainer::GetNeedDelList()
{
	return m_TimeLimitMgr.GetNeedDelList();
}

//----------------------------------------------------------------------
// ��մ�ɾ���б�
//----------------------------------------------------------------------
inline VOID EquipContainer::ClearNeedDelList()
{
	m_TimeLimitMgr.ClearNeedDelList();
}

//----------------------------------------------------------------------
// ��ȡ������б�
//----------------------------------------------------------------------
inline package_list<INT64>& EquipContainer::GetChangeBindList()
{
	return m_TimeLimitMgr.GetNeedChangeBindList();
}

//----------------------------------------------------------------------
// ��մ�����б�
//----------------------------------------------------------------------
inline VOID EquipContainer::ClearChangeBindList()
{
	m_TimeLimitMgr.ClearNeedChangeBindList();
}

