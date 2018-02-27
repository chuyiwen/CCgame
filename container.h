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

#include "container_template.h"
#include "time_limit_mgr.h"
#include "../../common/WorldDefine/ItemDefine.h"

class ContainerRestrict;

struct s_item_move;
//-----------------------------------------------------------------------------
// 物品&装备容器
//-----------------------------------------------------------------------------
class ItemContainer: public Container<tagItem, INT64>
{
public:
	ItemContainer(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize, 
					DWORD dwOwnerID, DWORD dw_account_id, ContainerRestrict *pRestrict = NULL);
	~ItemContainer();

	VOID Update();

public:
	// 向容器中添加新物品,但不指定添加位置,返回错误码
	DWORD Add(tagItem* pItem, OUT INT16 &n16Index, OUT s_item_move &ItemMove, BOOL bCheckAdd = TRUE, BOOL bChangeOwner = TRUE);
	
	// 向容器中指定位置添加新物品,返回错误码(指定位置必须为空).
	DWORD Add(tagItem* pItem, INT16 n16NewIndex, BOOL bChangeOwner = TRUE, BOOL bCheckAdd = TRUE);

	// 从容器中删除指定物品,返回错误码
	DWORD Remove(INT64 n64_serial, BOOL bChangeOwner = FALSE, BOOL bCheckRemove = TRUE);
	DWORD Remove(INT64 n64_serial, INT16 n16Num, BOOL bCheckRemove = TRUE);
	
	// 将物品移动到指定位置(同一容器内)
	DWORD MoveTo(INT64 n64Serial1, INT16 n16Index2, OUT s_item_move &ItemMove);
	DWORD MoveTo(INT64 n64Serial1, INT16 n16NumMove, INT16 n16Index2, OUT s_item_move &ItemMove);

	// 将物品移动到其他容器中(仓库和背包间)
	DWORD MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, OUT s_item_move &ItemMove, OUT INT16 &n16IndexDst);
	DWORD MoveTo(INT64 n64SerialSrc, ItemContainer &conDst, INT16 n16IndexDst, OUT s_item_move &ItemMove);

	// 扩容
	VOID IncreaseSize(INT16 n16Size);

	// 按指定位置重排
	BOOL Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	BOOL ReorderEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16 &n16OutNum, const INT16 n16Num);
	
public:
	// 获得一个空位，失败返回INVALID_VALUE
	INT16 GetOneFreeSpace();

	// 获得容器中所有物品类型为dw_data_id的物品个数
	INT32 GetSameItemCount(IN DWORD dw_data_id);
	INT32 GetSameBindItemCount(IN DWORD dw_data_id, BOOL bBind = FALSE);
	// 获得容器中物品类型为dw_data_id的lis, 返回实际获得个数t -- 指定nNum时，仅找到nNum个物品即可
	INT32 GetSameItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, IN INT32 n_num = INT_MAX);
	
	INT32 GetSameBindItemList(OUT package_list<tagItem*> &list, IN DWORD dw_data_id, BOOL bBind, IN INT32 n_num = INT_MAX);

	ContainerRestrict*	GetRestrict()	const;
	DWORD				GetOwnerID()	const;
	
	TimeLimitMgr<INT64>& GetTimeLimitMgr() { return m_TimeLimitMgr; }
	// 时限管理接口
	package_list<INT64>& GetNeedDelList();
	VOID ClearNeedDelList();
	
	package_list<INT64>& GetChangeBindList();
	VOID ClearChangeBindList();
private:
	// 重置容器中空闲位置的最小下标
	VOID ResetMinFreeIndex();

protected:
	DWORD				m_dwOwnerID;			// 容器所有者
	DWORD				m_dwAccountID;			// 玩家账号ID, 百宝袋和角色仓库用(同一帐号下角色共享)

private:
	// 辅助成员
	INT16				m_n16MinFreeIndex;		// 容器中空闲位置的最小下标(== m_n16CurSize时，无空闲位置)
	ContainerRestrict*	m_pRestrict;			// 物品容器操作约束类
	TimeLimitMgr<INT64>	m_TimeLimitMgr;			// 时限物品管理器
};


//-----------------------------------------------------------------------------
// 装备容器
//-----------------------------------------------------------------------------
class EquipContainer: public Container<tagEquip, INT64>
{
public:
	EquipContainer(EItemConType eConType,  INT16 n16CurSize, INT16 n16MaxSize);
	~EquipContainer();

	VOID Update();

public: 
	// 穿上装备
	DWORD Equip(ItemContainer &bagSrc, INT64 n64SerialSrc, EEquipPos ePosDst);
	// 脱下装备
	DWORD Unequip(INT64 n64SerialSrc, ItemContainer &bagDst);
	// 脱下装备(指定背包中位置)
	DWORD Unequip(INT64 n64SerialSrc, ItemContainer &bagDst, INT16 n16IndexDst);
	// 移动(仅限两个戒指位)
	DWORD MoveTo(INT64 n64SerialSrc, EEquipPos ePosDst);
	// 主副手交换
	//DWORD SwapWeapon();
	TimeLimitMgr<INT64>& GetTimeLimitMgr() { return m_TimeLimitMgr; }
	// 时限管理接口
	package_list<INT64>& GetNeedDelList();
	VOID ClearNeedDelList();
	
	package_list<INT64>& GetChangeBindList();
	VOID ClearChangeBindList();

public:
	// 装备
	DWORD Add(tagEquip *pEquip, EEquipPos ePos);

private:
	// 辅助
	TimeLimitMgr<INT64>	m_TimeLimitMgr;			// 时限物品管理器
};

//************************ 普通容器内联实现 ************************//

//----------------------------------------------------------------------
// 扩容
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
// 按指定位置重排
//----------------------------------------------------------------------
inline BOOL ItemContainer::Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	if(Container::Reorder(pData, pOutData, n16Num))
	{
		// 最小空闲下标需重置
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
		// 最小空闲下标需重置
		ResetMinFreeIndex();
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------
// 获得一个空位，失败返回INVALID_VALUE
//----------------------------------------------------------------------
inline INT16 ItemContainer::GetOneFreeSpace()
{
	ResetMinFreeIndex();
	return m_n16MinFreeIndex >= GetCurSpaceSize() ? INVALID_VALUE : m_n16MinFreeIndex;
}

//----------------------------------------------------------------------
// 获得容器中所有物品类型为dw_data_id的物品个数
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
// 获得容器中物品类型为dw_data_id的lis, 返回实际获得个数t -- 指定n_num时，仅找到n_num个物品即可
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
// 获取约束类指针
//----------------------------------------------------------------------
inline ContainerRestrict* ItemContainer::GetRestrict() const
{
	return m_pRestrict;
}

//----------------------------------------------------------------------
// 获取容器所以者RoleID
//----------------------------------------------------------------------
inline DWORD ItemContainer::GetOwnerID() const
{
	return m_dwOwnerID;
}

//----------------------------------------------------------------------
// 重置容器中空闲位置的最小下标
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

	// 此种写法，避免可能由于bug产生的死循环
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

	// 重要：执行到此处，说明容器中的m_n16RemainSize有问题
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
// 获取待删除列表
//----------------------------------------------------------------------
inline package_list<INT64>& ItemContainer::GetNeedDelList()
{
	return m_TimeLimitMgr.GetNeedDelList();
}

//----------------------------------------------------------------------
// 清空待删除列表
//----------------------------------------------------------------------
inline VOID ItemContainer::ClearNeedDelList()
{
	m_TimeLimitMgr.ClearNeedDelList();
}

//----------------------------------------------------------------------
// 获取待解绑列表
//----------------------------------------------------------------------
inline package_list<INT64>& ItemContainer::GetChangeBindList()
{
	return m_TimeLimitMgr.GetNeedChangeBindList();
}

//----------------------------------------------------------------------
// 清空待解绑列表
//----------------------------------------------------------------------
inline VOID ItemContainer::ClearChangeBindList()
{
	m_TimeLimitMgr.ClearNeedChangeBindList();
}

//************************ 装备栏内联实现 ************************//

//----------------------------------------------------------------------
// update
//----------------------------------------------------------------------
inline VOID EquipContainer::Update()
{
	m_TimeLimitMgr.Update();
}

//----------------------------------------------------------------------
// 获取待删除列表
//----------------------------------------------------------------------
inline package_list<INT64>& EquipContainer::GetNeedDelList()
{
	return m_TimeLimitMgr.GetNeedDelList();
}

//----------------------------------------------------------------------
// 清空待删除列表
//----------------------------------------------------------------------
inline VOID EquipContainer::ClearNeedDelList()
{
	m_TimeLimitMgr.ClearNeedDelList();
}

//----------------------------------------------------------------------
// 获取待解绑列表
//----------------------------------------------------------------------
inline package_list<INT64>& EquipContainer::GetChangeBindList()
{
	return m_TimeLimitMgr.GetNeedChangeBindList();
}

//----------------------------------------------------------------------
// 清空待解绑列表
//----------------------------------------------------------------------
inline VOID EquipContainer::ClearChangeBindList()
{
	m_TimeLimitMgr.ClearNeedChangeBindList();
}

