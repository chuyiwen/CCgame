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

#include "../../common/WorldDefine/container_define.h"
#include "../common/ServerDefine/base_server_define.h"

#include "world.h"

//****************** 容器类 该部分操作不修改物品的所有者 *********************
template<class T, class MapKey = INT64>
class Container
{
public:
	Container();
	Container(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize);
	virtual ~Container();

	VOID Init(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize);

private:
	VOID Destroy();	// 在析构函数中自动调用

private:
	Container(const Container&);
	Container& operator = (const Container&);

public:
	// 容器类型
	EItemConType GetConType()	const;

	// 得到容器中空闲空间的大小
	INT16 GetFreeSpaceSize()	const;

	// 得到容器总空间大小
	INT16 GetMaxSpaceSize()		const;

	// 得到容器当前空间大小
	INT16 GetCurSpaceSize()		const;

	// 判断指定位置是否为空
	BOOL IsOnePlaceFree(INT16 n16Index) const;

	// 根据key得到物品指针,返回NULL表示指定物品不在该容器中
	T* GetItem(MapKey key);

	// 根据index得到物品指针,返回NULL表示指定物品不在该容器中
	T* GetItem(INT16 n16Index);

	// 获取初始迭代器
	typename package_map<MapKey, INT16>::map_iter Begin()	{ return m_mapItem.begin(); }

	// 根据迭代器遍历容器中物品
	BOOL GetNextItem(typename package_map<MapKey, INT16>::map_iter& iter, T*& pItem);

	BOOL GetRandom(MapKey &key);

public:
	// 将物品存入指定位置,并返回添加成功个数(目的位置若有物品,将会被覆盖)
	INT16 Add(T* pItem, INT16 n16Index);

	// 从容器中清除物品，返回被删除物品的指针(NULL表示指定物品不存在容器中)
	T* Remove(MapKey key);	// 该接口调用方式为Container::Remove()
	T* Remove(INT16 n16Index);

	// 返回在容器中的位置,INVALID_VALUE表示不在容器中
	INT16 GetIndex(MapKey key);
	INT16 GetIndex(const T* pItem);
protected:
	// 两物品位置交换,返回物品1个数(直接交换，不做任何验证)
	INT16 Swap(INT16 n16Index1, INT16 n16Index2);



	// 重新设置容器大小
	VOID Resize(INT16 n16Size);

	// 按指定位置重排
	BOOL Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num);
	BOOL ReorderEx(IN LPVOID pData, OUT LPVOID pOutData, OUT INT16& n16OutNum, const INT16 n16Num);

private:
	T**					m_ppItem;			// 容器(该容器中存储的是物品结构的指针)
	package_map<MapKey, INT16>	m_mapItem;			// <64为ID，容器中位置>
	EItemConType		m_eContainerType;	// 容器类型
	INT16				m_n16MaxSize;		// 容器空间最大值
	INT16				m_n16CurSize;		// 容器当前空间大小
	INT16				m_n16RemainSize;	// 未使用的容器大小
};

//******************** 实现 该部分操作不修改物品的所有者 *********************

//------------------------------------------------------------------------------
// 构造函数
//------------------------------------------------------------------------------
template<class T, class MapKey>
Container<T, MapKey>::Container()
{
	m_ppItem = NULL;

	m_mapItem.clear();
	m_n16CurSize		= 0;
	m_n16RemainSize		= 0;
	m_n16MaxSize		= 0;
	m_eContainerType	= EICT_Null;
}

template<class T, class MapKey>
Container<T, MapKey>::Container(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize)
{
	Init(eConType, n16CurSize, n16MaxSize);	
}

//------------------------------------------------------------------------------
// 析构函数
//------------------------------------------------------------------------------
template<class T, class MapKey>
Container<T, MapKey>::~Container()
{
	Destroy();
}

//------------------------------------------------------------------------------
// 初始化
//------------------------------------------------------------------------------
template<class T, class MapKey>
VOID Container<T, MapKey>::Init(EItemConType eConType, INT16 n16CurSize, INT16 n16MaxSize)
{
	m_ppItem = new T*[n16CurSize];
	ZeroMemory(m_ppItem, n16CurSize * sizeof(T*));

	m_mapItem.clear();
	m_n16CurSize		= n16CurSize;
	m_n16RemainSize		= n16CurSize;
	m_n16MaxSize		= n16MaxSize;
	m_eContainerType	= eConType;
}

//------------------------------------------------------------------------------
// 销毁(不用显示调用)
//------------------------------------------------------------------------------
template<class T, class MapKey>
VOID Container<T, MapKey>::Destroy()
{
	// 删除物品
	for(INT32 i=0; i<m_n16CurSize; ++i)
	{
		::Destroy(m_ppItem[i]);
	}

	SAFE_DELETE_ARRAY(m_ppItem);

	m_mapItem.clear();
	m_n16CurSize	= 0;
	m_n16RemainSize = 0;
	m_n16MaxSize	= 0;
}

//------------------------------------------------------------------------------
// 将物品存入指定位置,并返回添加成功个数(目的位置若有物品,将会被覆盖)
//------------------------------------------------------------------------------
template<class T, class MapKey>
INT16 Container<T, MapKey>::Add(T* pItem, INT16 n16Index)
{
	// 此处做判断,是为了外部调用方便(负面效果: 可能会做了多次判断)
	//if(n16Index < 0 || n16Index >= m_n16CurSize)
	//{
	//	return 0;
	//}

	ASSERT(IsOnePlaceFree(n16Index));

	m_ppItem[n16Index] = pItem;
	m_mapItem.add(pItem->GetKey(), n16Index);

	// 容器中可用空间减少1
	--m_n16RemainSize;

	// 设置物品所在位置<容器类型,下标>，方便子类容器的操作(不用考虑物品中记录的位置信息变化)
	pItem->SetPos(m_eContainerType, n16Index);

	// 设置更新数据信息位
	pItem->SetUpdate(EUDBS_Update);

	return pItem->GetNum();
}

//------------------------------------------------------------------------------
// 根据64位id从容器中清除物品，返回被删除物品的指针
//------------------------------------------------------------------------------
template<class T, class MapKey>
T* Container<T, MapKey>::Remove(MapKey key)
{
	INT16 n16Index = m_mapItem.find(key);
	if(!VALID_VALUE(n16Index))
	{
		return NULL;
	}

	// 从map中清除
	m_mapItem.erase(key);

	// 数组中指针清空
	T* pRetItem = m_ppItem[n16Index];
	if(NULL == pRetItem)
	{
		ASSERT(pRetItem != NULL);
		return NULL;
	}

	m_ppItem[n16Index] = NULL;

	// 容器中可以空间增加1
	++m_n16RemainSize;

	pRetItem->SetPos(EICT_Ground, INVALID_VALUE);

	return pRetItem;
}

//------------------------------------------------------------------------------
// 根据16位index从容器中清除物品，返回被删除物品的指针
//------------------------------------------------------------------------------
template<class T, class MapKey>
T* Container<T, MapKey>::Remove(INT16 n16Index)
{
	// 此处做判断,是为了外部调用方便(负面效果: 可能会做了多次判断)
	if(n16Index < 0 || n16Index >= m_n16CurSize)
	{
		return NULL;
	}

	// 数组中指针清空
	T* pRetItem = m_ppItem[n16Index];
	if(NULL == pRetItem)
	{
		//ASSERT(pRetItem != NULL);
		return NULL;
	}

	m_ppItem[n16Index] = NULL;

	// 从map中清除
	m_mapItem.erase(pRetItem->GetKey());

	// 容器中可以空间增加1
	++m_n16RemainSize;

	pRetItem->SetPos(EICT_Ground, INVALID_VALUE);

	return pRetItem;
}

//-----------------------------------------------------------------------------
// 按指定位置重排
//-----------------------------------------------------------------------------
template<class T, class MapKey>
BOOL Container<T, MapKey>::Reorder(IN LPVOID pData, OUT LPVOID pOutData, const INT16 n16Num)
{
	// 弱检测：个数是否与现有数量相符
	if(n16Num != GetCurSpaceSize() - GetFreeSpaceSize())
	{
		return FALSE;
	}
	
	//M_trans_pointer(pSrcPos, pData, INT16);
	//M_trans_pointer(pOut, pOutData, INT16);
	INT16* pSrcPos = (INT16*)(pData);
	INT16* pOut = (INT16*)(pOutData);

	// 将容器中物品数据备份
	T** ppBak = new T*[GetCurSpaceSize()];
	memcpy(ppBak, m_ppItem, sizeof(T*) * GetCurSpaceSize());

	// 清空容器中所以物品数据
	m_mapItem.clear();
	ZeroMemory(m_ppItem, sizeof(T*) * GetCurSpaceSize());
	m_n16RemainSize = GetCurSpaceSize();

	// 根据最新的顺序将物品放入容器
	INT16 n16CurIndex = 0;
	for(INT16 i=0; i<n16Num; ++i)
	{
		// 消息中数据不正确，或指定位置没有物品
		if(pSrcPos[i] < 0 || pSrcPos[i] >= GetCurSpaceSize()
			|| !VALID_POINT(ppBak[pSrcPos[i]]))
		{
			continue;
		}
		
		// 放入容器中
		Add(ppBak[pSrcPos[i]], n16CurIndex);

		// 更新
		pOut[n16CurIndex] = pSrcPos[i];
		++n16CurIndex;
		ppBak[pSrcPos[i]] = NULL;
	}

	// 检查是否所有物品都已放到容器中
	if(n16CurIndex != n16Num)
	{
		// 执行到此，说明消息中有错误数据，需特殊处理
		for(INT16 i=0; i<GetCurSpaceSize(); ++i)
		{
			if(!VALID_POINT(ppBak[i]))
			{
				continue;
			}

			// 放入容器中
			Add(ppBak[i], n16CurIndex);

			// 更新
			pOut[n16CurIndex] = i;
			++n16CurIndex;
			ppBak[i] = NULL;
		}

		ASSERT(n16Num == n16CurIndex);
	}

	// 释放临时内存
	SAFE_DELETE_ARRAY(ppBak);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 按指定位置重排 -- 待移动物品的数据结构与上面函数不同
//-----------------------------------------------------------------------------
template<class T, class MapKey>
BOOL Container<T, MapKey>::ReorderEx(IN LPVOID pData, OUT LPVOID pOutData, 
									 OUT INT16& n16OutNum, const INT16 n16Num)
{
	n16OutNum = 0;

	// 弱检测：个数是否与现有数量相符
	if(n16Num > GetCurSpaceSize() - GetFreeSpaceSize())
	{
		return FALSE;
	}


	M_trans_pointer(pSrcPos, pData, tagItemOrder);
	M_trans_pointer(pOut, pOutData, tagItemOrder);

	// 将容器中需移动的物品数据备份
	T** ppBak = new T*[n16Num];
	ZeroMemory(ppBak, sizeof(T*) * n16Num);

	// 记录将失败的位置,最后在统一重排
	INT nFailNumber = 0;
	INT16* pFailIndex = new INT16[n16Num];
	ZeroMemory(pFailIndex, sizeof(INT16) * n16Num);

	for(INT16 i=0; i<n16Num; ++i)
	{
		// 消息中数据不正确，或指定位置没有物品
		if(pSrcPos[i].n16OldIndex < 0 || pSrcPos[i].n16OldIndex >= GetCurSpaceSize()
			|| pSrcPos[i].n16NewIndex < 0 || pSrcPos[i].n16NewIndex >= GetCurSpaceSize()
			|| !VALID_POINT(GetItem(pSrcPos[i].n16OldIndex)))
		{
			continue;
		}

		// 从容器中取出,并放入备份数组中
		ppBak[i] = Remove(pSrcPos[i].n16OldIndex);
	}

	for(INT16 i=0; i<n16Num; ++i)
	{
		// 检查是否有物品
		if(!VALID_POINT(ppBak[i]))
		{
			continue;
		}

		// 检查目标位置是否为空
		if(!IsOnePlaceFree(pSrcPos[i].n16NewIndex))
		{// mwh 2011-11-11记录下来统一重排
			pFailIndex[nFailNumber] = i;
			++nFailNumber; continue;
		}

		// 放入容器中
		Add(ppBak[i], pSrcPos[i].n16NewIndex);

		// 更新
		pOut[n16OutNum++] = pSrcPos[i];
		ppBak[i] = NULL;
	}

	// 检查是否所有物品都已放到容器中
	if(n16OutNum != n16Num && nFailNumber)
	{
		// 执行到此，是因为网络延迟造成的。mwh 2011-11-11
		--nFailNumber; // 最大循环次数小于背包大小
		for(INT16 n = 0;  n < GetCurSpaceSize( ) && nFailNumber >= 0; ++n)
		{
			if(!IsOnePlaceFree(n)) continue;

			INT16 n16CurFailIndex = pFailIndex[nFailNumber];
			if(!VALID_POINT(ppBak[n16CurFailIndex]))
			{
				//! 当前处理的位置ppBak没有物品,应该不能可能
				//! 万一有未知错误,就放弃处理当前索引的物品
				//! 如果真失败客户端将看不到失败的物品，重登陆OK
				ASSERT(FALSE);
				--nFailNumber;  continue; 
			}

			//添加到背包
			Add(ppBak[n16CurFailIndex], n);

			//修正排序
			pSrcPos[n16CurFailIndex].n16NewIndex = n;
			pOut[n16OutNum++] = pSrcPos[n16CurFailIndex];
			ppBak[n16CurFailIndex] = NULL;
			--nFailNumber;
		}
	}

	// 释放临时内存
	SAFE_DELETE_ARRAY(pFailIndex);
	SAFE_DELETE_ARRAY(ppBak);

	return TRUE;
}


//*************************** 内联函数 ****************************************

//-----------------------------------------------------------------------------
// 容器类型
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline EItemConType Container<T, MapKey>::GetConType() const
{
	return m_eContainerType;
}

//-----------------------------------------------------------------------------
// 得到容器中空闲空间的大小
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline INT16 Container<T, MapKey>::GetFreeSpaceSize() const
{
	return m_n16RemainSize;
}

//-----------------------------------------------------------------------------
// 得到容器总空间大小
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline INT16 Container<T, MapKey>::GetMaxSpaceSize() const
{
	return m_n16MaxSize;
}

//-----------------------------------------------------------------------------
// 得到容器当前空间大小
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline INT16 Container<T, MapKey>::GetCurSpaceSize() const
{
	return m_n16CurSize;
}

//-----------------------------------------------------------------------------
// 判断指定位置是否为空
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline BOOL Container<T, MapKey>::IsOnePlaceFree(INT16 n16Index) const
{
	if(n16Index < 0 || n16Index >= m_n16CurSize)
	{
		ASSERT(0);
		return FALSE;
	}
	
	return NULL == m_ppItem[n16Index];
}

//-----------------------------------------------------------------------------
// 根据key得到物品指针,返回NULL表示指定物品不在该容器中
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline T* Container<T, MapKey>::GetItem(MapKey key)
{
	INT16 n16Index = m_mapItem.find(key);
	if(!VALID_VALUE(n16Index))
	{
		return NULL;
	}

	return m_ppItem[n16Index];
}

//-----------------------------------------------------------------------------
// 根据index得到物品指针,返回NULL表示指定物品不在该容器中
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline T* Container<T, MapKey>::GetItem(INT16 n16Index) 
{
	if(n16Index>=0 && n16Index<m_n16CurSize) 
	{
		return m_ppItem[n16Index];
	}
	return NULL; 
}

//-----------------------------------------------------------------------------
// 根据迭代器遍历容器中内容
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline BOOL Container<T, MapKey>::GetNextItem(typename package_map<MapKey, INT16>::map_iter& iter, T*& pItem)
{
	INT64 n64_serial	= INVALID_VALUE;
	INT16 n16Index	= INVALID_VALUE;
	if (m_mapItem.find_next(iter, n64_serial, n16Index) && VALID_VALUE(n16Index))
	{
		pItem = m_ppItem[n16Index];
		return TRUE;
	}

	pItem = NULL;

	return FALSE;
}

//-----------------------------------------------------------------------------
// 随机抽取
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline BOOL Container<T, MapKey>::GetRandom(MapKey &key)
{
	INT16 n16Index;
	return m_mapItem.rand_find(key, n16Index);
}

//-----------------------------------------------------------------------------
// 两物品位置交换,返回物品1个数(直接交换，不做任何验证)
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline INT16 Container<T, MapKey>::Swap(INT16 n16Index1, INT16 n16Index2)
{
	// 删除
	T *pItem1 = Remove(n16Index1);
	T *pItem2 = Remove(n16Index2);

	// 添加
	Add(pItem1, n16Index2);
	Add(pItem2, n16Index1);

	// 设置数据库保存状态
	pItem1->SetUpdate(EUDBS_Update);
	pItem2->SetUpdate(EUDBS_Update);

	return pItem1->GetNum();
}

//-----------------------------------------------------------------------------
// 返回物品指针存储在容器中的位置,INVALID_VALUE表示不在容器中
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline INT16 Container<T, MapKey>::GetIndex(MapKey key)
{
	return m_mapItem.find(key);
}

template<class T, class MapKey>
inline INT16 Container<T, MapKey>::GetIndex(const T* pItem)
{
	return m_mapItem.find(pItem->GetKey());
}

//-----------------------------------------------------------------------------
// 重新设置容器大小
//-----------------------------------------------------------------------------
template<class T, class MapKey>
inline VOID Container<T, MapKey>::Resize(INT16 n16Size) 
{ 
	ASSERT(n16Size > m_n16CurSize);
	ASSERT(n16Size <= m_n16MaxSize);

	// 申请空间，并复制数据
	T **ppNew = new T*[n16Size];
	ZeroMemory(ppNew, n16Size * sizeof(T*));
	get_fast_code()->memory_copy(ppNew, m_ppItem, m_n16CurSize * sizeof(T*));

	// 重置容器属性
	SAFE_DELETE_ARRAY(m_ppItem);
	m_ppItem = ppNew;
	m_n16RemainSize += n16Size - m_n16CurSize;
	m_n16CurSize = n16Size;
}