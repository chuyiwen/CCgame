/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物交易

#include "stdafx.h"
#include "pet_exchange.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "../../common/WorldDefine/pet_exchange_protocot.h"

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
PetExchangeMgr::PetExchangeMgr()
{
	m_pExData		= NULL;
	m_dwTgtRoleID	= INVALID_VALUE;
}

PetExchangeMgr::~PetExchangeMgr()
{
	SAFE_DELETE(m_pExData);
}

//-----------------------------------------------------------------------------
// 创建交易空间 -- 如果已存在，则重新初始化
//-----------------------------------------------------------------------------
VOID PetExchangeMgr::CreateData()
{
	if(NULL == m_pExData)
	{
		m_pExData = new tagPetExchangeData;
	}

	ASSERT(m_pExData != NULL);

	ZeroMemory(m_pExData, sizeof(tagPetExchangeData));
}

//-----------------------------------------------------------------------------
// 向交易数据结构中添加交易物品 -- 返回 -- 成功:插入位置下标；失败:INVALID_VALUE
//-----------------------------------------------------------------------------
INT32 PetExchangeMgr::AddPet(DWORD dwPetID)
{
	ASSERT(m_pExData != NULL);

	INT32 nInsIndex = INVALID_VALUE;
	for(INT32 i=0; i<MAX_EXCHANGE_ITEM; ++i)
	{
		if(m_pExData->dwPetIDs[i] == dwPetID)
		{
			print_message(_T("Add the same pet<id: %lld> to exchange!!!!\r\n"), dwPetID);
			return INVALID_VALUE;
		}
		else if(0 == m_pExData->dwPetIDs[i] && INVALID_VALUE == nInsIndex)
		{
			nInsIndex = i;
		}
	}

	if(nInsIndex != INVALID_VALUE)
	{
		m_pExData->dwPetIDs[nInsIndex] = dwPetID;

		++m_pExData->byPetNum;
	}

	return E_Pets_Success;
}

//-----------------------------------------------------------------------------
// 从交易物品中取出指定物品 -- 成功返回E_Success，失败返回相应错误码
//-----------------------------------------------------------------------------
DWORD PetExchangeMgr::DecPet(DWORD dwPetID)
{
	ASSERT(m_pExData != NULL);

	for(INT32 i=0; i<MAX_EXCHANGE_ITEM; ++i)
	{
		if(m_pExData->dwPetIDs[i] == dwPetID)
		{
			m_pExData->dwPetIDs[i] = 0;
			--m_pExData->byPetNum;
			return E_Success;
		}
	}

	return INVALID_VALUE;
}