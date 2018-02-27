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
 *	@brief		��Ϣ������
*/

#ifndef __CHAT_MGR__
#define __CHAT_MGR__

#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/show_item_protocol.h"
#include "channel_mgr.h"



// ��Ϣ�ṹ
class msg_packet
{
public:
	// dctor
	virtual ~msg_packet(){}

	static msg_packet* create_packet(PVOID p_message, Role* p_sender);
	static VOID delete_packet(msg_packet* &p_packet);

	// �������ݰ�
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

	// ������ID
	DWORD dw_dest_role_id_;
	// Ƶ������
	ESendChatChannel chat_channel_;			

	// ������
	Role* p_sender_;
	// ��Ϣ��С
	DWORD content_size_;
	// ��Ϣ����
	BYTE* p_content_;

	// �յ�����Ϣ
	tag_net_message* p_recv_;

	// Ҫ���͵���Ϣ
	tag_net_message* p_send_;

	// ������Ϣ������
	BYTE buffer_[1024 * 10];	
};

// ��ͨ������Ϣ
class chat_packet : public msg_packet
{
	friend class msg_packet;

public:
	virtual DWORD parse();

private:
	chat_packet(tag_net_message* p_message)
		:msg_packet(p_message){}
};

// չʾװ��
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

// չʾ��Ʒ
class item_packet : public msg_packet
{
	friend class msg_packet;
public:
	virtual DWORD parse();
private:
	item_packet(tag_net_message* p_message)
		:msg_packet(p_message){}
};


// ��Ϣ������
class chat_msg_mgr
{
public:
	DWORD load_offline_msg(DWORD dw_role_id);

	DWORD deal_chat_msg(PVOID p_message, Role* p_sender);

 	DWORD deal_load_offline_msg(PVOID p_message);
private:
	// ����˽����Ϣ
	DWORD save_offline_msg(tag_net_message* p_message, DWORD msg_size, DWORD dw_role_id);
	
private:	
	channel_mgr channel_mgr_;
};


extern chat_msg_mgr g_msgMgr;

#endif //__CHAT_MGR__