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
*	@file		Auto_Kill_Hanlder
*	@author		mwh
*	@date		2011/08/01	initial
*	@version	0.0.1.0
*	@brief		自动打怪处理
*/

#include "stdafx.h"
#include "player_session.h"
#include "role.h"
#include "../../common/WorldDefine/auto_kill_protocol.h"

DWORD PlayerSession::HandleAutoKillStart(tag_net_message* p_cmd)
{
	NET_SIC_Auto_Kill_Start* p_recv = (NET_SIC_Auto_Kill_Start*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;
	//gx modify 2013.6.20
	/*if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}*/

	NET_SIS_Auto_Kill_Start send;
	send.dwErrorCode = pRole->CanAutoKill( );
	pRole->SendMessage(&send, send.dw_size);
	pRole->SetAutoKill(send.dwErrorCode == E_Success);
	return 0;
}
DWORD PlayerSession::HandleAutoKillEnd(tag_net_message* p_cmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;

	NET_SIS_Auto_Kill_End send;
	send.dwErrorCode = E_Success;
	pRole->SendMessage(&send, send.dw_size);
	pRole->SetAutoKill(FALSE);

	return 0;
}