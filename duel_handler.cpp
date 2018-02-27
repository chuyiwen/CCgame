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
 *	@file		duel_handler
 *	@author		mwh
 *	@date		2011/05/17	initial
 *	@version	0.0.1.0
 *	@brief		ÇĞ´èÏµÍ³
*/

#include "StdAfx.h"
#include "../../common/WorldDefine/duel_protocol.h"
#include "player_session.h"
#include "role.h"

DWORD PlayerSession::HandleDuelAskFor(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	NET_SIC_AskForDuel* pProtocol = (NET_SIC_AskForDuel*)pCmd;
	DWORD tRet = pRole->AskForDuel(pProtocol->dwTarget);

	NET_SIS_AskForDuel send;
	send.dwError = tRet;
	send.dwTarget = pProtocol->dwTarget;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleDuelResponse(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	NET_SIC_AskForDuelResponse* pProtocol = (NET_SIC_AskForDuelResponse*)pCmd;
	pRole->DuelResponse(pProtocol->dwTarget, pProtocol->byAck);
	return 0;
}