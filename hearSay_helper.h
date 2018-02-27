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
#include "../../common/WorldDefine/hearchat_protocol.h"

struct tagItem;
class HearSayHelper
{
public:
	static VOID SendMessage(
		EHearSayType eType, 
		DWORD dwRoleID, 
		DWORD dwParam0 = INVALID_VALUE, 
		DWORD dwParam1 = INVALID_VALUE, 
		DWORD dwParam2 = INVALID_VALUE, 
		DWORD dwParam3 = INVALID_VALUE,   
		const tagItem* pItem = NULL,  
		BOOL bChatChannel = FALSE,
		VOID* p_buffer = NULL,
		DWORD dw_buffer_size = 0);

	static BOOL CheckBossItem(DWORD dwBossID, const tagItem* pItem);
	static BOOL IsSpecialItem(const tagItem* pItem);
	static BOOL IsItemBroadcast( DWORD dwBossID, const tagItem* pItem );
};