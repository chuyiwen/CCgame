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

#ifndef __CHAT_MGR__
#define __CHAT_MGR__

#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/show_item_protocol.h"
#include "channel_mgr.h"



// 消息结构
class msg_packet
{
public:
	// dctor
	virtual ~msg_packet(){}

	static msg_packet* create_packet(PVOID p_message, Role* p_sender);
	static VOID delete_packet(msg_packet* &p_packet);

	// 解析数据包
	virtual DWORD		parse()=0;
	
	// Getter
	tag_net_message*	get_send()		const	{ return p_send_; }
	tag_net_message*	get_recv()		const	{ return p_recv_; }
	DWORD				get_reciver()	const	{ return dw_dest_role_id_; }
	ESendChatChannel	get_channel()	const	{ return chat_channel_; }
	DWORD				get_size()		const	{ return content_size_; }
	BYTE*				get_content()	const	{ return p_content_; }
	Role*				get_sender()	const	{ return p_sender_; }
	VOID				set_sender(Role* p_sender)	{ p_sender_ = p_sender; }

protected:
	msg_packet(tag_net_message* p_receive);

	// 接收者ID
	DWORD dw_dest_role_id_;
	// 频道类型
	ESendChatChannel chat_channel_;			

	// 发送者
	Role* p_sender_;
	// 消息大小
	DWORD content_size_;
	// 消息内容
	BYTE* p_content_;

	// 收到的消息
	tag_net_message* p_recv_;

	// 要发送的消息
	tag_net_message* p_send_;

	// 发送消息缓冲区
	BYTE buffer_[1024 * 10];	
};

// 普通聊天消息
class chat_packet : public msg_packet
{
	friend class msg_packet;

public:
	virtual DWORD parse();

private:
	chat_packet(tag_net_message* p_message)
		:msg_packet(p_message){}
};

// 展示装备
class equip_packet : public msg_packet
{	
	friend class msg_packet;
public:
	virtual DWORD parse();

private:
	equip_packet(tag_net_message* p_message)
		:msg_packet(p_message){}

	DWORD	m_byContainerType;
};

// 展示物品
class item_packet : public msg_packet
{
	friend class msg_packet;
public:
	virtual DWORD parse();
private:
	item_packet(tag_net_message* p_message)
		:msg_packet(p_message){}
};


// 消息管理器
class chat_msg_mgr
{
public:
	DWORD load_offline_msg(DWORD dw_role_id);

	DWORD deal_chat_msg(PVOID p_message, Role* p_sender);

 	DWORD deal_load_offline_msg(PVOID p_message);
private:
	// 保存私聊消息
	DWORD save_offline_msg(tag_net_message* p_message, DWORD msg_size, DWORD dw_role_id);
	
private:	
	channel_mgr channel_mgr_;
};


extern chat_msg_mgr g_msgMgr;

#endif //__CHAT_MGR__