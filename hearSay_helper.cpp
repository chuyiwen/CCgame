/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		hearSay_helper
 *	@author		mwh
 *	@date		2011/05/23	initial
 *	@version	0.0.1.0
 *	@brief		传闻处理
*/

#include "StdAfx.h"
#include "hearSay_helper.h"
#include "role.h"
#include "role_mgr.h"

// 全服的消息,!!!效率警告!!!
VOID HearSayHelper::SendMessage(
							 EHearSayType eType, 
							 DWORD dwRoleID, 
							 DWORD dwParam0, 
							 DWORD dwParam1, 
							 DWORD dwParam2, 
							 DWORD dwParam3, 
							 const tagItem* pItem,
							 BOOL bChatChannel,
							 VOID* p_buffer,
							 DWORD dw_buffer_size)
{
	const static INT ITEMSIZE = sizeof(tagItem);
	const static INT EQUIPSIZE = sizeof(tagEquip);
	const static INT MSGSIZE = sizeof(NET_SIS_HearSayChannel) - sizeof(CHAR) + EQUIPSIZE;
	if(eType != EHST_CREATEGUILD && eType != EHST_GUILDFIRSTKILL)
	{
		CREATE_MSG(pSend, MSGSIZE, NET_SIS_HearSayChannel);
		if(!VALID_POINT(pSend)) return;

		NET_SIS_HearSayChannel temp;
		pSend->dw_size = temp.dw_size - sizeof(CHAR);
		pSend->dw_message_id = temp.dw_message_id;

		pSend->stData.eType = eType;
		pSend->stData.dwRoleID = dwRoleID;
		pSend->stData.dwParam0 = dwParam0;
		pSend->stData.dwParam1 = dwParam1;
		pSend->stData.dwParam2 = dwParam2;
		pSend->stData.dwParam3 = dwParam3;
		pSend->stData.bChatChannel = bChatChannel;

		if(VALID_POINT(pItem) && MIsEquipment(pItem->dw_data_id))
		{
			get_fast_code()->memory_copy(pSend->cBuffer, pItem,  EQUIPSIZE);
			pSend->dw_size += EQUIPSIZE;
		}

		g_roleMgr.send_world_msg(pSend, pSend->dw_size);
	}
	else
	{
		DWORD dw_message_size = sizeof(NET_SIS_HearSayChannel) - sizeof(CHAR) + dw_buffer_size*sizeof(TCHAR);
		CREATE_MSG(pSend, dw_message_size, NET_SIS_HearSayChannel);
		if(!VALID_POINT(pSend)) return;

		NET_SIS_HearSayChannel temp;
		pSend->dw_size = dw_message_size;
		pSend->dw_message_id = temp.dw_message_id;

		pSend->stData.eType = eType;
		pSend->stData.dwRoleID = dwRoleID;
		pSend->stData.dwParam0 = dw_buffer_size;
		pSend->stData.dwParam1 = dwParam1;
		pSend->stData.dwParam2 = dwParam2;
		pSend->stData.dwParam3 = dwParam3;
		pSend->stData.bChatChannel = bChatChannel;

		if(VALID_POINT(p_buffer))
		{
			get_fast_code()->memory_copy(pSend->cBuffer, p_buffer, dw_buffer_size);
			pSend->dw_size = dw_message_size;
			g_roleMgr.send_world_msg(pSend, pSend->dw_size);
		}
	}
}

BOOL HearSayHelper::CheckBossItem(DWORD dwBossID, const tagItem* pItem)
{
	if( !VALID_POINT(dwBossID)|| !VALID_POINT(pItem) || !VALID_POINT(pItem->pProtoType))
		return FALSE;

	EItemQuality eItemQuality = (EItemQuality)pItem->pProtoType->byQuality;
	if(MIsEquipment(pItem->dw_data_id)) eItemQuality =  (EItemQuality)((tagEquip*)pItem)->GetQuality();
	return eItemQuality >= EIQ_Quality3;
}

BOOL HearSayHelper::IsSpecialItem(const tagItem* pItem)
{
	if(!VALID_POINT(pItem) || !VALID_POINT(pItem->pProtoType))
		return FALSE;

	if(pItem->pProtoType->dw_data_id >= 5100001 && 
		pItem->pProtoType->dw_data_id <= 5100100 )
	{
		return TRUE;
	}

	return FALSE;
}


BOOL HearSayHelper::IsItemBroadcast( DWORD dwBossID, const tagItem* pItem )
{
	return HearSayHelper::IsSpecialItem(pItem) || HearSayHelper::CheckBossItem(dwBossID, pItem);
}

