
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 聊天相关
#include "StdAfx.h"
#include "player_session.h"
#include "role.h"
#include "role_mgr.h"
#include "map.h"
#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/show_item_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/GMServerMessage.h"
#include "group_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "ps_bomb.h"
#include "chat_mgr.h"
#include "GMSession.h"
//------------------------------------------------------------------------------------
// 聊天
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChat(tag_net_message *pCmd)
{
	Role* pRole = GetRole();

	if(!VALID_POINT(pRole)) return 0;

	if ( pRole->SpeakOff())
	{
		NET_SIS_role_char send;
		send.dw_error_code = E_RoleChat_ForbidChat;
		SendMessage(&send, send.dw_size);

		return 0;
	}
	
	NET_SIC_role_char* p_receive = (NET_SIC_role_char*)pCmd;
	//gx modify 2013.6.24
	/*if (p_receive->byChannel == ESCC_Decree)
	{
		if(!pRole->get_check_safe_code())
		{
			if(GetBagPsd() != p_receive->dw_safe_code)
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
		}
	}*/
	DWORD dwRtv = g_msgMgr.deal_chat_msg(p_receive, GetRole());
	if (E_Success != dwRtv)
	{
		NET_SIS_role_char send;
		send.dw_error_code = dwRtv;
		SendMessage(&send, send.dw_size);
	}
	else
	{// 发送成功后我们在向GM服务器转发
		s_role_info* info = g_roleMgr.get_role_info(pRole->GetID() );
		if(VALID_POINT(info))
		{
			DWORD content_size = p_receive->dw_size - sizeof(NET_SIC_role_char) + sizeof(TCHAR);
			DWORD msg_size = content_size + sizeof(NET_S2GMS_ChatTransmit);

			CREATE_MSG(p_send, msg_size, NET_S2GMS_ChatTransmit);

			p_send->dwRoleID = pRole->GetID( );
			p_send->byChannel = p_receive->byChannel;
			p_send->dwAccountID = info->dw_account_id;
			p_send->szMsg[content_size/sizeof(TCHAR)] = 0;
			_tcscpy(p_send->szRoleName, info->sz_role_name);
			get_fast_code()->memory_copy(p_send->szMsg, p_receive->szMsg, content_size);
			g_rtSession.SendMessage(p_send, p_send->dw_size);

			MDEL_MSG(p_send);	
		}
	}


	return 0 ;

}

//------------------------------------------------------------------------
// 装备展示
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleShowEquip(tag_net_message* pCmd)
{
	NET_SIC_role_show_equip *p_receive = (NET_SIC_role_show_equip*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return 0;
	if ( pRole->SpeakOff())
	{
		NET_SIS_role_char send;
		send.dw_error_code = E_RoleChat_ForbidChat;
		SendMessage(&send, send.dw_size);
		return 0;
	}

	DWORD dwRtv = g_msgMgr.deal_chat_msg(p_receive, GetRole());
	if (E_Success != dwRtv)
	{
		NET_SIS_role_show_equip send;
		send.dw_error_code = dwRtv;
		SendMessage((LPVOID)&send, send.dw_size);
	}

	return E_Success;
}

//------------------------------------------------------------------------
// 物品展示
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleShowItem(tag_net_message* pCmd)
{
	NET_SIC_role_show_item *p_receive = (NET_SIC_role_show_item*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return 0;
	if ( pRole->SpeakOff())
	{
		NET_SIS_role_char send;
		send.dw_error_code = E_RoleChat_ForbidChat;
		SendMessage(&send, send.dw_size);
		return 0;
	}

	DWORD dwRtv = g_msgMgr.deal_chat_msg(p_receive, GetRole());
	if (E_Success != dwRtv)
	{
		NET_SIS_role_show_item send;
		send.dw_error_code = dwRtv;
		SendMessage(&send, send.dw_size);
	}
	return E_Success;

}

//------------------------------------------------------------------------
// 获取留言
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLoadLeftMsg(tag_net_message* pCmd)
{
	if (!VALID_POINT(GetRole()))
	{
		return INVALID_VALUE;
	}

	DWORD dwRtv = g_msgMgr.load_offline_msg(GetRole()->GetID());
	
	return E_Success;
}
