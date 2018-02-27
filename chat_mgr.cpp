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
 *	@file		chat_mgr
 *	@author		mwh
 *	@date		2011/03/21	initial
 *	@version	0.0.1.0
 *	@brief		消息管理器
*/

#include "stdafx.h"
#include "chat_mgr.h"
#include "role.h"
#include "../common/ServerDefine/leftmsg_server_define.h"
#include "role_mgr.h"


chat_msg_mgr g_msgMgr;

msg_packet* msg_packet::create_packet( PVOID p_message, Role* p_sender)
{
	if (!VALID_POINT(p_message))
	{
		ASSERT(0);
		return NULL;
	}
	static const NET_SIC_role_char			__msg_chat;
	static const NET_SIC_role_show_equip	__msg_equip;
	static const NET_SIC_role_show_item		__msg_item;

	msg_packet* p_new_packet = NULL;

	// 根据不同Msg类型生成不同对象
	tag_net_message* p_cmd = static_cast<tag_net_message*>(p_message);
	if (p_cmd->dw_message_id == __msg_chat.dw_message_id)
	{
		p_new_packet = new chat_packet(p_cmd);
	}
	else if (p_cmd->dw_message_id == __msg_equip.dw_message_id)
	{
		p_new_packet = new equip_packet(p_cmd);
	}
	else if (p_cmd->dw_message_id == __msg_item.dw_message_id)
	{
		p_new_packet = new item_packet(p_cmd);
	}

	if (VALID_POINT(p_new_packet))
	{
		p_new_packet->set_sender(p_sender);
		if (E_Success != p_new_packet->parse())
		{
			ASSERT(0);
			delete_packet(p_new_packet);
		}
	}

	return p_new_packet;
}

VOID msg_packet::delete_packet( msg_packet* &p_packet )
{
	SAFE_DELETE(p_packet);
}

msg_packet::msg_packet( tag_net_message* p_receive )
{
	p_sender_ = NULL;
	dw_dest_role_id_ = INVALID_VALUE;
	chat_channel_ = ESCC_NULL;
	p_send_ = reinterpret_cast<tag_net_message*>(buffer_);
	p_recv_ = p_receive;
}

DWORD chat_packet::parse()
{
	NET_SIS_role_char* p_send = static_cast<NET_SIS_role_char*>(p_send_);
	NET_SIC_role_char* p_receive = static_cast<NET_SIC_role_char*>(p_recv_);

	DWORD content_size = p_receive->dw_size - sizeof(NET_SIC_role_char) + sizeof(TCHAR);

	// 设置发送消息
	p_send->bGM = p_sender_->GetGMPrivilege( );
	p_send->dw_error_code = 0;
	p_send->dwSrcRoleID	= p_sender_->GetID();
	p_send->byAutoReply = p_receive->byAutoReply;
	p_send->byChannel = (BYTE)p_receive->byChannel;
	p_send->dwDestRoleID = p_receive->dwDestRoleID; 
	p_send->dw_message_id = get_tool()->crc32("NET_SIS_role_char");
	p_send->dw_size = sizeof(NET_SIS_role_char) + content_size - sizeof(TCHAR);
	get_fast_code()->memory_copy(p_send->szMsg, p_receive->szMsg, content_size);

	content_size_ = content_size;
	p_content_ = (BYTE*)p_send->szMsg;
	dw_dest_role_id_ = p_send->dwDestRoleID;
	chat_channel_ = ESendChatChannel(p_send->byChannel);

	return E_Success;
}

DWORD equip_packet::parse()
{
	NET_SIS_role_show_equip* p_send = static_cast<NET_SIS_role_show_equip*>(p_send_);
	NET_SIC_role_show_equip* p_receive = static_cast<NET_SIC_role_show_equip*>(p_recv_);
	DWORD equip_size = sizeof(tagEquip);
	DWORD msg_size = p_receive->dw_size - sizeof(NET_SIC_role_show_equip) + sizeof(TCHAR);

	// 获得装备
	tagItem* pItem = p_sender_->GetItemMgr().GetDisplayItem(p_receive->eConType, p_receive->n64_serial);
	if(!VALID_POINT(pItem)) return E_RoleShowItem_SendFailure;
	if(!MIsEquipment(pItem->dw_data_id)) return E_RoleShowItem_SendFailure;

	tagEquip* pEquip = static_cast<tagEquip*>(pItem);
	
	// 设置发送消息

	p_send->bGM = p_sender_->IsGM( );
	p_send->dw_error_code = 0;
	p_send->dwSrcRoleID	= p_sender_->GetID();
	p_send->byChannel = p_receive->byChannel;
	p_send->dwDestRoleID = p_receive->dwDestRoleID;
	p_send->dw_message_id = get_tool()->crc32("NET_SIS_role_show_equip");
	p_send->sEquipSize = (SHORT)equip_size;
	get_fast_code()->memory_copy(p_send->byBuffer, pEquip, equip_size);

	p_send->sMsgSize = (SHORT)msg_size;
	if(msg_size>0) 
		get_fast_code()->memory_copy(p_send->byBuffer+equip_size, p_receive->szMsg, msg_size);

	DWORD content_size = equip_size + msg_size;

	p_send->dw_size = sizeof(NET_SIS_role_show_equip) + content_size - sizeof(BYTE);

	chat_channel_ = ESendChatChannel(p_send->byChannel);
	dw_dest_role_id_ = p_send->dwDestRoleID;
	p_content_ = (BYTE*)p_send->byBuffer;
	content_size_ = content_size;

	return E_Success;
}

DWORD item_packet::parse()
{
	NET_SIS_role_show_item* p_send = static_cast<NET_SIS_role_show_item*>(p_send_);
	NET_SIC_role_show_item* p_receive = static_cast<NET_SIC_role_show_item*>(p_recv_);

	tagItem* pItem = p_sender_->GetItemMgr().GetDisplayItem(p_receive->eConType, p_receive->n64_serial);
	if(!VALID_POINT(pItem)) return E_RoleShowItem_SendFailure;
	if(MIsEquipment(pItem->dw_data_id)) return E_RoleShowItem_SendFailure;

	DWORD msg_size = p_receive->dw_size - sizeof(NET_SIC_role_show_item) + sizeof(TCHAR);

	p_send->bGM = p_sender_->IsGM( );
	p_send->dw_error_code = 0;
	p_send->dwSrcRoleID = p_sender_->GetID(); 
	p_send->byChannel = p_receive->byChannel;
	p_send->dw_data_id = pItem->dw_data_id;
	p_send->dwDestRoleID	= p_receive->dwDestRoleID;
	p_send->dw_message_id = get_tool()->crc32("NET_SIS_role_show_item");
	if(msg_size>0) 
		get_fast_code()->memory_copy(p_send->szMsg, p_receive->szMsg, msg_size);
	p_send->dw_size = sizeof(NET_SIS_role_show_item) + msg_size - sizeof(TCHAR);

	p_content_ = (BYTE*)p_send->szMsg;
	content_size_ = msg_size;
	dw_dest_role_id_ = p_send->dwDestRoleID;
	chat_channel_ = ESendChatChannel(p_send->byChannel);

	return E_Success;
}

DWORD chat_msg_mgr::deal_chat_msg( PVOID p_message, Role* p_sender)
{
	if (!VALID_POINT(p_sender)) return INVALID_VALUE;

	DWORD dw_ret = E_Success;

	//1. 创建包
	msg_packet* p_packet = msg_packet::create_packet(p_message, p_sender);
	if (!VALID_POINT(p_packet)) return INVALID_VALUE;

	//2. 发送消息
	dw_ret = channel_mgr_.send_packet(p_packet);

	//3. 私聊不在线则存数据库
	if(E_RoleChat_Secret_RemoteRoleLeave == dw_ret)
	{
		tag_net_message* p_offline_msg = p_packet->get_recv();
		if (VALID_POINT(p_offline_msg))
		{
			static_cast<NET_SIC_role_char*>(p_offline_msg)->dwDestRoleID = p_sender->GetID();
			save_offline_msg(p_offline_msg, p_offline_msg->dw_size, p_packet->get_reciver());
		}
	}

	//4. 删除包
	msg_packet::delete_packet(p_packet);

	return dw_ret;
}

DWORD chat_msg_mgr::save_offline_msg( tag_net_message* p_message, DWORD msg_size, DWORD dw_role_id )
{
	BYTE by_buffer[1024] = {0};

	NET_DB2C_insert_left_message* p_offline_msg = reinterpret_cast<NET_DB2C_insert_left_message*>(by_buffer);
	p_offline_msg->data.dw_role_id = dw_role_id;
	p_offline_msg->data.dw_date_time = GetCurrentDWORDTime();
	p_offline_msg->dw_message_id = get_tool()->crc32("NET_DB2C_insert_left_message");
	get_fast_code()->memory_copy(p_offline_msg->data.by_data, p_message, msg_size);
	p_offline_msg->dw_size = sizeof(NET_DB2C_insert_left_message) - sizeof(BYTE) + msg_size;
	g_dbSession.Send(p_offline_msg, p_offline_msg->dw_size);

	return E_Success;
}

DWORD chat_msg_mgr::load_offline_msg( DWORD dw_role_id )
{
	NET_DB2C_load_left_message send;
	send.dw_role_id = dw_role_id;
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}

DWORD chat_msg_mgr::deal_load_offline_msg( PVOID p_message )
{
	NET_DB2S_load_left_message* pLoadLeftMsg = reinterpret_cast<NET_DB2S_load_left_message*>(p_message);

	Role* p_role = g_roleMgr.get_role(pLoadLeftMsg->dw_role_id);
	if (!VALID_POINT(p_role)) return INVALID_VALUE;

	BYTE* p_begin = pLoadLeftMsg->by_data ;
	BYTE* p_end = pLoadLeftMsg->by_data + pLoadLeftMsg->dw_size - sizeof(NET_DB2S_load_left_message) + sizeof(BYTE);

	while(p_begin < p_end)
	{
		NET_SIC_role_char* p_recv_chat = (NET_SIC_role_char*)p_begin;
		DWORD dw_src_roleid = p_recv_chat->dwDestRoleID;
		p_recv_chat->dwDestRoleID = pLoadLeftMsg->dw_role_id;

		msg_packet* p_msg_cmd = msg_packet::create_packet(p_begin, p_role);
		if (VALID_POINT(p_msg_cmd))
		{
			p_begin += p_msg_cmd->get_recv()->dw_size;

			NET_SIS_role_char* pSendCmd = (NET_SIS_role_char*)p_msg_cmd->get_send();
			pSendCmd->dwSrcRoleID = dw_src_roleid;

			channel_mgr_.send_packet(p_msg_cmd);
			msg_packet::delete_packet(p_msg_cmd);
		}
		else
		{
			break;
		}
	}

	p_role->ResetHasLeftMsg();

	NET_DB2C_clear_left_message send;
	send.dw_role_id = p_role->GetID();
	g_dbSession.Send(&send, send.dw_size);

	return E_Success;
}
