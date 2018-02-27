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
 *	@file		fish_handler
 *	@author		mwh
 *	@date		2011/06/28	initial
 *	@version	0.0.1.0
 *	@brief		钓鱼处理
*/

#include "StdAfx.h"

#include "player_session.h"
#include "role.h"
#include "role_mgr.h"
#include "../../common/WorldDefine/fishing_protocol.h"

DWORD PlayerSession::HandleStartFishing(tag_net_message* pCmd)
{
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;

	NET_SIC_Start_Fishing* pProtocol = (NET_SIC_Start_Fishing*)pCmd;

	NET_SIS_Start_Fishing send;
	send.dwError = pRole->StartFishing(pProtocol->dwNpcID,
																pProtocol->dwSkill,
																pProtocol->n64FishingRod,
																pProtocol->n64Bait,
																pProtocol->bAutoFish);
	pRole->SendMessage(&send, send.dw_size);
	return E_Success;
}

DWORD PlayerSession::HandleStopFishing(tag_net_message* pCmd)
{
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;

	NET_SIC_Stop_Fishing* pProtocol = (NET_SIC_Stop_Fishing*)pCmd;
	
	NET_SIS_Stop_Fishing send;
	send.dwError = pRole->StopFishing();
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}