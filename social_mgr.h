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
 *	@file		social_mgr
 *	@author		mwh
 *	@date		2011/03/21	initial
 *	@version	0.0.1.0
 *	@brief		社会关系管理
*/

#ifndef __SOCIAL_MGR__
#define __SOCIAL_MGR__

#include "event_mgr.h"
#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/ItemDefine.h"

class social_mgr : public EventMgr<social_mgr>
{
public:
	~social_mgr() {}
public:
	BOOL init();
	VOID update();
	
	// 好友相关
	VOID make_friend(DWORD dw_sender, VOID* p_message);
	VOID cancel_friend(DWORD dw_sender, VOID* p_message);
	VOID update_friend_group(DWORD dw_sender, VOID* p_message);
	VOID insert_black_list(DWORD dw_sender, VOID* p_message);
	VOID delete_black_list(DWORD dw_sender, VOID* p_message);
	DWORD delete_black_list(Role* p_src_role, DWORD dw_dest_role);
	VOID delete_enemy_list(DWORD dw_sender, VOID* p_message);
	DWORD delete_enemy_list(Role* p_src_role, DWORD dw_dest_role);
	VOID add_enemy_list(DWORD dw_sender, VOID* p_message);
	VOID get_enemy_position(DWORD dw_sender, VOID* p_message);
	VOID send_gift(DWORD dw_sender, VOID* p_message);
	VOID send_gift_reply(DWORD dw_sender, VOID* p_message);
	
	//gx add 2013.6.27
	VOID invitePlayerToCompractice(DWORD dw_sender, VOID* p_message);
	VOID invitePlayerToCompracticeReply(DWORD dw_sender, VOID* p_message);
	VOID playerCancelCompractice(DWORD dw_sender, VOID* p_message);
	//end
	//结婚相关 gx add 2013.7.3
	VOID maleplayer_propose(DWORD dw_sender, VOID* p_message);
	VOID femaleplayer_propose_reply(DWORD dw_sender, VOID* p_message);
	VOID player_divorce(DWORD dw_sender, VOID* p_message);
	VOID get_qbjjreward(DWORD dw_sender, VOID* p_message);//gx add 2013.10.25
	DWORD getqbjjreward_bylevel(INT level,BOOL bRedZui = TRUE);
	//end
	// 同步玩家角色
	VOID syn_role_level(DWORD dw_sender, VOID* p_message);
	static INT getFriendValueLevel(INT nValue);	
public:
	VOID send_logout_to_friend(Role *p_role);
	VOID send_login_to_friend(Role *p_role);
	VOID send_login_to_spouse(Role *p_role);//伴侣上线发给另一方 gx add 2013.10.29
	VOID send_loginout_to_spouse(Role *p_role);//伴侣下线发给另一方
private:
	static VOID register_event();
private:
	BYTE _cancel_friend(Role *p_src_role, DWORD dw_src_role, DWORD dw_dest_role);
	VOID _update_gift();								
	VOID send_to_all_friends(Role *p_role, VOID *p_message, DWORD dw_size);
	//gx modify 2013.6.27
	DWORD can_send_gift(Role* p_src_role, Role* p_dest_role, DWORD dw_dest_role, BYTE level, INT32 &nTotalNum,INT32 &nBindNum,INT16 &n_SendFlowers);

private:
	vector<tagSendGift> send_gifts_;			// 发送的礼物
};

extern social_mgr g_socialMgr;

#endif //__SOCIAL_MGR__