
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//移动消息处理类

#include "StdAfx.h"
#include "player_session.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/action_protocol.h"

//-----------------------------------------------------------------------------
// 行走
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleWalk(tag_net_message* pCmd)
{
 	NET_SIC_mouse_walk* p_receive = (NET_SIC_mouse_walk*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) )
	{
		return INVALID_VALUE;
	}

	MoveData::EMoveRet eRet = MoveData::EMR_Success;

	eRet = pRole->MoveAction(&MoveData::StartRoleWalk, p_receive->srcPos, p_receive->dstPos);
	//eRet = pRole->GetMoveData().StartRoleWalk(p_receive->srcPos, p_receive->dstPos);
	

	// 如果移动失败，则发送客户端失败
	if( MoveData::EMR_Success != eRet )
	{
		NET_SIS_move_failed send;
		send.curPos = pRole->GetCurPos();
		send.faceTo = pRole->GetFaceTo();

		EMoveState eMoveState = pRole->GetMoveState();

		SendMessage(&send, send.dw_size);
	}

	return 0;	
}


//------------------------------------------------------------------------------
// 停止移动
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStopWalk(tag_net_message* pCmd)
{
	NET_SIC_stop_walk* p_receive = (NET_SIC_stop_walk*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) )
	{
		return INVALID_VALUE;
	}

	MoveData::EMoveRet eRet = MoveData::EMR_Success;
	
	eRet = pRole->MoveAction(&MoveData::StopRoleMove, p_receive->curPos);
	
	// 如果移动失败，则发送客户端失败
	if( MoveData::EMR_Success != eRet )
	{
		NET_SIS_move_failed send;
		send.curPos = pRole->GetCurPos();
		send.faceTo = pRole->GetFaceTo();

		EMoveState eMoveState = pRole->GetMoveState();

		SendMessage(&send, send.dw_size);
	}

	return 0;
}