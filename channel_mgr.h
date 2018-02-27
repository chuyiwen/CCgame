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
 *	@file		channel_mgr
 *	@author		mwh
 *	@date		2011/03/21	initial
 *	@version	0.0.1.0
 *	@brief		聊天频道管理
*/

#ifndef __CHANNEL_MGR__
#define __CHANNEL_MGR__


#include "unit.h"
#include "role.h"
#include "../../common/WorldDefine/chat_define.h"

class msg_packet;

struct send_op
{
	send_op(ESendChatChannel e_channel, tag_net_message* p_send)
		:p_send_(p_send), e_channel_(e_channel){ASSERT(VALID_POINT(p_send));}

	VOID operator()(Unit* pUnit)
	{
		if (!VALID_POINT(p_send_)) return ;

		Role* p_role = dynamic_cast<Role*>(pUnit);
		if (VALID_POINT(p_role)) p_role->SendMessage(p_send_, p_send_->dw_size);
	}

private:
	tag_net_message* p_send_;
	ESendChatChannel e_channel_;
};

class channel_mgr
{
public:
	DWORD	get_size_limit(tag_net_message* p_cmd, BYTE channel);
	DWORD	send_packet(msg_packet* p_packet);
};

#endif //__CHANNEL_MGR__