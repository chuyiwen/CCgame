/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家间交易

#include "StdAfx.h"
#include "exchange.h"
#include "../../common/WorldDefine/exchange_protocol.h"

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
ExchangeMgr::ExchangeMgr()
{
	m_pExData		= NULL;
	m_dwTgtRoleID	= INVALID_VALUE;
}

ExchangeMgr::~ExchangeMgr()
{
	SAFE_DELETE(m_pExData);
}

//-----------------------------------------------------------------------------
// 创建交易空间 -- 如果已存在，则重新初始化
//-----------------------------------------------------------------------------
VOID ExchangeMgr::CreateData()
{
	if(NULL == m_pExData)
	{
		m_pExData = new tagExchangeData;
	}

	ASSERT(m_pExData != NULL);

	ZeroMemory(m_pExData, sizeof(tagExchangeData));
}

//-----------------------------------------------------------------------------
// 向交易数据结构中添加交易物品 -- 返回 -- 成功:插入位置下标；失败:INVALID_VALUE
//-----------------------------------------------------------------------------
INT32 ExchangeMgr::add_item(INT64 n64_serial, INT16 n16Num)
{
	ASSERT(m_pExData != NULL);

	INT32 nInsIndex = INVALID_VALUE;
	for(INT32 i=0; i<MAX_EXCHANGE_ITEM; ++i)
	{
		if(m_pExData->n64_serial[i] == n64_serial)
		{
			return INVALID_VALUE;
		}
		else if(0 == m_pExData->n64_serial[i] && INVALID_VALUE == nInsIndex)
		{
			nInsIndex = i;
		}
	}

	if(nInsIndex != INVALID_VALUE)
	{
		m_pExData->n64_serial[nInsIndex] = n64_serial;
		m_pExData->n16ItemNum[nInsIndex] = n16Num;

		++m_pExData->byTypeNum;
	}

	return nInsIndex;
}

//-----------------------------------------------------------------------------
// 从交易物品中取出指定物品 -- 成功返回E_Success，失败返回相应错误码
//-----------------------------------------------------------------------------
DWORD ExchangeMgr::DecItem(INT64 n64_serial)
{
	ASSERT(m_pExData != NULL);

	for(INT32 i=0; i<MAX_EXCHANGE_ITEM; ++i)
	{
		if(m_pExData->n64_serial[i] == n64_serial)
		{
			m_pExData->n64_serial[i] = 0;
			--m_pExData->byTypeNum;
			return E_Success;
		}
	}

	return E_Exchange_ItemCanNot_Find;
}