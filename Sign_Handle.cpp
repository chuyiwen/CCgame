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
*	@file		Sign_Handle
*	@author		lc
*	@date		2012/7/31	initial
*	@version	0.0.1.0
*	@brief		签到处理
*/

#include "stdafx.h"
#include "player_session.h"
#include "role.h"
#include "../../common/WorldDefine/Sign_define.h"

DWORD PlayerSession::HandleGetSignData(tag_net_message* p_cmd)
{
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error = pRole->get_sign_data();

	if(dw_error != E_Success)
	{
		NET_SIS_get_sign_data send;
		send.dw_error = dw_error;
		pRole->SendMessage(&send, send.dw_size);
	}

	return 0;
}

DWORD PlayerSession::HandleSign(tag_net_message* p_cmd)
{
	NET_SIC_sign* p_recv = (NET_SIC_sign*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error = pRole->role_sign(p_recv->b_buqian, p_recv->dw_date);

	if(dw_error != E_Success)
	{
		NET_SIS_sign send;
		send.dw_error = dw_error;
		pRole->SendMessage(&send, send.dw_size);
	}

	return 0;
}

DWORD PlayerSession::HandleSignReward(tag_net_message* p_cmd)
{
	NET_SIC_sign_reward* p_recv = (NET_SIC_sign_reward*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error = pRole->role_sign_reward(p_recv->n_index);

	NET_SIS_sign_reward send;
	send.n_index = p_recv->n_index;
	send.dw_error = dw_error;
	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleGetSignReward(tag_net_message* p_cmd)
{
	NET_SIC_get_sign_reward* p_recv = (NET_SIC_get_sign_reward*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error = pRole->role_get_sign_reward();

	NET_SIS_get_sign_reward send;
	send.dw_error = dw_error;
	SendMessage(&send, send.dw_size);
	return 0;
}


