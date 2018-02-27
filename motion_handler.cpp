/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"
#include "player_session.h"
#include "role.h"
#include "role_mgr.h"
#include "../../common/WorldDefine/motion_protocol.h"

//-------------------------------------------------------------------------------
// 玩家播放个性动作
//-------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStyleAction(tag_net_message* pCmd)
{
	NET_SIC_role_style_action* p_receive = (NET_SIC_role_style_action*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	// 广播
	NET_SIS_role_style_action send;
	send.dw_role_id = pRole->GetID();
	send.dwActionID = p_receive->dwActionID;

	if( VALID_POINT(pRole->get_map()) )
	{
		pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
	}
	return 0;
}


//-------------------------------------------------------------------------
// 双人动作邀请
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDuetMotionInvite( tag_net_message* pCmd )
{
	if (!VALID_POINT(GetRole()))		return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_double_motion_invite);

	DWORD dwRtv = E_Success;

	Role* pDestRole = g_roleMgr.get_role(p_receive->dwDstRoleID);
	if (!VALID_POINT(pDestRole))
	{
		dwRtv = E_Motion_RoleNotFround;
	}
	else
	{
		dwRtv = GetRole()->CanCastMotion(pDestRole, p_receive->dwActionID);
	}

	if (E_Motion_Success == dwRtv)
	{
		GetRole()->SetMotionInviteStatus(TRUE, p_receive->dwDstRoleID);
		pDestRole->SetMotionInviteStatus(TRUE, GetRole()->GetID());

		NET_SIS_double_motion_on_invite send;
		send.dwActionID = p_receive->dwActionID;
		send.dwSrcRoleID = GetRole()->GetID();

		pDestRole->SendMessage(&send, send.dw_size);
	}
	else
	{
		NET_SIS_double_motion_invite send;
		send.dwErrCode = dwRtv;
		SendMessage(&send, send.dw_size);
	}

	return E_Success;
}

//-------------------------------------------------------------------------
// 双人动作应答
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDuetMotionOnInvite( tag_net_message* pCmd )
{
	if (!VALID_POINT(GetRole()))		return INVALID_VALUE;

	M_trans_pointer(p_receive, pCmd, NET_SIC_double_motion_on_invite);

	NET_SIS_double_motion_invite send;
	send.dwErrCode = E_Motion_Success;

	Role* pSrcRole = g_roleMgr.get_role(p_receive->dwSrcRoleID);
	if (!VALID_POINT(pSrcRole))
	{
		send.dwErrCode = E_Motion_RoleNotFround;
	}
	else if (!p_receive->bAccpet)
	{
		send.dwErrCode = E_Motion_DstRoleRefuse;
	}
	else
	{
		send.dwErrCode = GetRole()->CanCastMotion(pSrcRole, p_receive->dwActionID);
	}

	if (send.dwErrCode != E_Motion_Success)
	{
		GetRole()->SetMotionInviteStatus(FALSE, p_receive->dwSrcRoleID);
		pSrcRole->SetMotionInviteStatus(FALSE, GetRole()->GetID());
	}

	if (VALID_POINT(pSrcRole))
	{
		pSrcRole->SendMessage(&send, send.dw_size);
	}	

	return E_Success;
}

//-------------------------------------------------------------------------
// 双人动作开始
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDuetMotionStart( tag_net_message* pCmd )
{
	M_trans_pointer(p_receive, pCmd, NET_SIC_double_motion_start);

	Role* pDestRole = GetOtherInMap(p_receive->dwDstRoleID);
	if (!VALID_POINT(GetRole()))		return INVALID_VALUE;
	if (!VALID_POINT(pDestRole))		return INVALID_VALUE;

	GetRole()->CastMotion(pDestRole, p_receive->dwActionID);

	GetRole()->SetMotionInviteStatus(FALSE, p_receive->dwDstRoleID);
	pDestRole->SetMotionInviteStatus(FALSE, GetRole()->GetID());

	return E_Success;
}

