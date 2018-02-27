
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端返回角色选择时与服务器之间的消息处理

#include "StdAfx.h"
#include "player_session.h"
#include "map_creator.h"
#include "role.h"
#include "../../common/WorldDefine/return_select_role_protocol.h"
#include "world_session.h"
#include "role_mgr.h"

//--------------------------------------------------------------------------
// 客户端返回到角色选择界面
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleReturnRoleSelect(tag_net_message* pCmd)
{
	//	获取角色
	Role* pRole = GetRole();
	NET_SIS_return_role_select send;

	if(!VALID_POINT(pRole))
	{
		send.dw_error_code = INVALID_VALUE;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}
	
	if(pRole->is_leave_pricitice())
		return INVALID_VALUE;

	g_mapCreator.role_return_character(pRole->GetID());

	send.dw_error_code = E_Success;
	SendMessage(&send, send.dw_size);
	

	return E_Success;
}