
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//时限类对象管理器
#pragma once

//******************** 类定义 *****************************//
template<typename KeyType>
class TimeLimitMgr
{
public:
	TimeLimitMgr(DWORD dwUpdateTicks);
	~TimeLimitMgr();

	VOID Update();

	VOID Add2TimeLeftList(KeyType key, DWORD dwTimeLimit, tagDWORDTime dwSrcTime, DWORD dwtype);
	VOID RemoveFromTimeLeftList(KeyType key);

	package_list<KeyType>& GetNeedDelList();
	VOID ClearNeedDelList();
	
	package_list<KeyType>&	GetNeedChangeBindList();
	VOID ClearNeedChangeBindList();
private:
	// 定时更新(单位：秒)
	VOID UpdateTimeLeftList(DWORD dwTimePass);

private:
	// 时限物品统计结构
	struct tagTimeLeft
	{
		KeyType	key;		
		DWORD	dwTimeLeft;		// 剩余存在时间(单位：秒)
		DWORD	dwType;			// 类型:0,删除计时 1,绑定计时

		tagTimeLeft(KeyType	key, DWORD dwTimeLeft, DWORD dwType)
		{
			this->key			= key;
			this->dwTimeLeft	= dwTimeLeft;
			this->dwType		= dwType;
		}
	};

	// 相关list
	package_list<tagTimeLeft*>		m_LstTimeLeft;
	package_list<KeyType>			m_LstNeedDel;
	package_list<KeyType>			m_LstChangBind;

	// 辅助
	DWORD					m_dwUpdateTicks;
	INT32					m_nTickCountDown;
	tagDWORDTime			m_dwLastCalTime;
};


//******************** 类实现 *****************************//
template<typename KeyType>
TimeLimitMgr<KeyType>::TimeLimitMgr(DWORD dwUpdateTicks)
{
	m_dwUpdateTicks		= dwUpdateTicks;
	m_nTickCountDown	= dwUpdateTicks;
	m_dwLastCalTime		= g_world.GetWorldTime();
}

template<typename KeyType>
TimeLimitMgr<KeyType>::~TimeLimitMgr()
{
	if(m_LstTimeLeft.size() > 0)
	{
		tagTimeLeft *pTimeLeft = NULL;
		package_list<tagTimeLeft*>::list_iter iter = m_LstTimeLeft.begin();
		while(m_LstTimeLeft.find_next(iter, pTimeLeft))
		{
			// 删除该节点
			m_LstTimeLeft.erase(pTimeLeft);
			SAFE_DELETE(pTimeLeft);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// update
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
inline VOID TimeLimitMgr<KeyType>::Update()
{
	--m_nTickCountDown;
	
	if(0 == m_nTickCountDown)
	{
		if(m_LstTimeLeft.size() > 0)
		{
			UpdateTimeLeftList(m_dwUpdateTicks / TICK_PER_SECOND);
		}
		
		m_nTickCountDown = m_dwUpdateTicks;
	}
}

//-------------------------------------------------------------------------------------------------------
// 时限物品统计列表操作
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
VOID TimeLimitMgr<KeyType>::Add2TimeLeftList(KeyType key, DWORD dwTimeLimit, tagDWORDTime dwSrcTime, DWORD dwType)
{
	ASSERT(dwSrcTime != INVALID_VALUE);

	DWORD dwTimeLeft = CalcTimeDiff(GetCurrentDWORDTime(), dwSrcTime);
	if(dwTimeLeft >= dwTimeLimit)
	{
		// 放到待删除列表
		if(!m_LstNeedDel.is_exist(key) && dwType == 0)
		{
			m_LstNeedDel.push_back(key);
		}
		
		// 放入解除绑定列表
		if (!m_LstChangBind.is_exist(key) && dwType == 1)
		{
			m_LstChangBind.push_back(key);
		}
		return;
	}
	

	tagTimeLeft *pTimeLeft = NULL;
	package_list<tagTimeLeft*>::list_iter iter = m_LstTimeLeft.begin();
	while(m_LstTimeLeft.find_next(iter, pTimeLeft))
	{
		if(pTimeLeft->key == key)
		{
			pTimeLeft->dwTimeLeft = dwTimeLimit - dwTimeLeft;
			pTimeLeft->dwType = dwType;
			return;
		}

	}

	pTimeLeft = new tagTimeLeft(key, dwTimeLimit - dwTimeLeft, dwType);
	ASSERT(VALID_POINT(pTimeLeft));

	m_LstTimeLeft.push_back(pTimeLeft);

}

//-------------------------------------------------------------------------------------------------------
// 时限物品统计列表操作
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
VOID TimeLimitMgr<KeyType>::RemoveFromTimeLeftList(KeyType key)
{
	ASSERT(m_LstTimeLeft.size() > 0);

	BOOL bCheck = FALSE;
	tagTimeLeft *pTimeLeft = NULL;
	package_list<tagTimeLeft*>::list_iter iter = m_LstTimeLeft.begin();
	while(m_LstTimeLeft.find_next(iter, pTimeLeft))
	{
		if(pTimeLeft->key == key)
		{
			m_LstTimeLeft.erase(pTimeLeft);
			SAFE_DELETE(pTimeLeft);
			bCheck = TRUE;
			break;
		}
	}

	ASSERT(bCheck);
}

//-------------------------------------------------------------------------------------------------------
// 时限物品统计列表操作(单位：秒)
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
VOID TimeLimitMgr<KeyType>::UpdateTimeLeftList(DWORD dwTimePass)
{
	tagTimeLeft *pTimeLeft = NULL;
	package_list<tagTimeLeft*>::list_iter iter = m_LstTimeLeft.begin();
	while(m_LstTimeLeft.find_next(iter, pTimeLeft))
	{
		if(pTimeLeft->dwTimeLeft <= dwTimePass)
		{
			if(!m_LstNeedDel.is_exist(pTimeLeft->key) && pTimeLeft->dwType == 0)
			{
				m_LstNeedDel.push_back(pTimeLeft->key);
			}
			if (!m_LstChangBind.is_exist(pTimeLeft->key) && pTimeLeft->dwType == 1)
			{
				m_LstChangBind.push_back(pTimeLeft->key);
			}

			// 删除该节点
			m_LstTimeLeft.erase(pTimeLeft);
			SAFE_DELETE(pTimeLeft);

			continue;
		}

		pTimeLeft->dwTimeLeft -= dwTimePass;
	}
}

//-------------------------------------------------------------------------------------------------------
// 获取待删除列表
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
inline package_list<KeyType>& TimeLimitMgr<KeyType>::GetNeedDelList()
{
	return m_LstNeedDel;
}

//-------------------------------------------------------------------------------------------------------
// 清空待删除列表
//-------------------------------------------------------------------------------------------------------
template<typename KeyType>
inline VOID TimeLimitMgr<KeyType>::ClearNeedDelList()
{
	m_LstNeedDel.clear();
}

template<typename KeyType>
inline package_list<KeyType>& TimeLimitMgr<KeyType>::GetNeedChangeBindList()
{
	return m_LstChangBind;
}

template<typename KeyType>
inline VOID TimeLimitMgr<KeyType>::ClearNeedChangeBindList()
{
	m_LstChangBind.clear();
}