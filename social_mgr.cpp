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


#include "StdAfx.h"
#include "social_mgr.h"
#include "player_session.h"
#include "unit.h"
#include "role.h"
#include "role_mgr.h"
#include "db_session.h"
#include "hearSay_helper.h"

#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/social_protocol.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

social_mgr g_socialMgr;


BOOL social_mgr::init()
{
	send_gifts_.clear();
	register_event();
	return TRUE;
}

// 更新
VOID social_mgr::update()
{
	EventMgr<social_mgr>::Update();
	
	// 更新礼物
	_update_gift();
}

// 更新礼物
VOID social_mgr::_update_gift()
{
	vector<tagSendGift>::iterator it = send_gifts_.begin();
	while( it != send_gifts_.end())
	{
		--it->nStoreTick;
	
		// 30 tick
		if(it->nStoreTick < 0 )
		{
			NET_SIS_send_gift_broad	 send;
			send.dwSrcRoleID = it->dwSrcRoleID;
			send.dwDestRoleID = it->dwDestRoleID;
			send.dw_data_id = it->dw_data_id;
			send.bResult = FALSE;
			g_roleMgr.send_world_msg(&send, send.dw_size);
			it = send_gifts_.erase(it);
			continue;
		}
		++it;
	}
}

// 注册管理类处理函数
VOID social_mgr::register_event()
{
	RegisterEventFunc(EVT_MakeFriend,		&social_mgr::make_friend);
	RegisterEventFunc(EVT_CancelFriend,		&social_mgr::cancel_friend);
	RegisterEventFunc(EVT_FriendGrp,		&social_mgr::update_friend_group);
	RegisterEventFunc(EVT_InsertBlkList,	&social_mgr::insert_black_list);
	RegisterEventFunc(EVT_DeleteBlkList,	&social_mgr::delete_black_list);
	RegisterEventFunc(EVT_DeleteEmList,		&social_mgr::delete_enemy_list);
	RegisterEventFunc(EVT_GetEnemyPos,		&social_mgr::get_enemy_position);
	RegisterEventFunc(EVT_SendGift,			&social_mgr::send_gift);
	RegisterEventFunc(EVT_SendGiftRpy,		&social_mgr::send_gift_reply);
	RegisterEventFunc(EVT_SynRoleLevel,		&social_mgr::syn_role_level);
	RegisterEventFunc(EVT_AddEmList,		&social_mgr::add_enemy_list);

	//gx add 双修相关 2013.6.27
	RegisterEventFunc(EVT_ComPractice,		&social_mgr::invitePlayerToCompractice);
	RegisterEventFunc(EVT_ComPracticeReply,	&social_mgr::invitePlayerToCompracticeReply);
	RegisterEventFunc(EVT_CancelPractice,	&social_mgr::playerCancelCompractice);
	//结婚相关
	RegisterEventFunc(EVT_Propose,			&social_mgr::maleplayer_propose);
	RegisterEventFunc(EVT_ProposeReply,		&social_mgr::femaleplayer_propose_reply);
	RegisterEventFunc(EVT_Divorce,		&social_mgr::player_divorce);
	RegisterEventFunc(EVT_QbjjReward,	&social_mgr::get_qbjjreward);
}

// 玩家上线
VOID social_mgr::send_login_to_friend(Role *p_role)
{
	NET_SIS_login_to_friend send;
	send.dw_role_id = p_role->GetID();
	send_to_all_friends(p_role, &send, send.dw_size);	
}

// 玩家下线
VOID social_mgr::send_logout_to_friend(Role *p_role)
{
	NET_SIS_logout_to_friend send;
	send.dw_role_id = p_role->GetID();
	send_to_all_friends(p_role, &send, send.dw_size);	
}

// 给好友发消息
VOID social_mgr::send_to_all_friends(Role *p_role, VOID *p_message, DWORD dw_size)
{
	if(!VALID_POINT(p_role))
		return;

	for(INT i = 0; i < MAX_FRIENDNUM; ++i)
	{
		DWORD dw_friend_id_ = p_role->GetFriend(i).dwFriendID;
		DWORD dwFriVal = p_role->GetFriend(i).dwFriVal;
		//gx modify 2013.5.30
		//修改后，只需要双方互为好友，就会结束好友上下线的消息
		if(VALID_VALUE(dw_friend_id_)/* && dwFriVal > 0*/)
		{
			Role* pFriend = g_roleMgr.get_role(dw_friend_id_);
			if(VALID_POINT(pFriend))
				pFriend->SendMessage(p_message, dw_size);
		}
	}
}

// 添加好友
VOID social_mgr::make_friend(DWORD dw_sender, VOID *p_message)
{
	NET_SIC_role_make_friend* p_receive = (NET_SIC_role_make_friend*)p_message;

	DWORD dw_src_role = dw_sender;
	DWORD dw_dest_role = p_receive->dwDestRoleID;
	Role* p_dest = (Role*)INVALID_VALUE;
	DWORD dw_friend_val = 0;
	BOOL  b_online = FALSE;
	INT	  level = 1;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);
	
	if(!VALID_POINT(p_src_role))
		return;

	DWORD	dw_error_code = E_Success;

	// 不能加自己
	if( dw_dest_role == dw_src_role )
	{
		dw_error_code = E_Friend_Target_Not_Sel;	
		goto Exit;
	}

	// 角色ID不存在
	s_role_info* p_role_info = g_roleMgr.get_role_info(dw_dest_role);

	if(!VALID_POINT(p_role_info))
	{
		dw_error_code = E_Friend_Target_Not_Exist;
		goto Exit;
	}
	else
	{
		b_online = p_role_info->b_online_;
		level = p_role_info->by_level;
	}

	// 无法加黑名单内的角色
	for(INT i = 0; i < MAX_BLACKLIST; ++i)
	{
		if(p_src_role->GetBlackList(i) == dw_dest_role)
		{
			dw_error_code = E_Friend_Target_Black_List;
			goto Exit;
		}
	}

	// 角色已经是好友
	tagFriend *p_friend = p_src_role->GetFriendPtr(dw_dest_role);
	if(VALID_POINT(p_friend))
	{
		dw_error_code = E_Friend_Target_Already_Exit;
		goto Exit;
	}

	if(dw_error_code == E_Success)
	{
		for (INT i = 0; i < MAX_FRIENDNUM; ++i)
		{
			if(p_src_role->GetFriend(i).dwFriendID == INVALID_VALUE)
			{
				p_src_role->SetFriend(i, dw_dest_role);
				if( VALID_POINT( p_src_role->GetScript( ) ) )
					p_src_role->GetScript( )->OnAddFriend( p_src_role, dw_dest_role );
				
				NET_DB2C_insert_friend send;
				send.dw_role_id = dw_src_role;
				send.s_friend_save_.dw_friend_id_ = dw_dest_role;
				send.s_friend_save_.n_group_id_ = 1;
				g_dbSession.Send(&send, send.dw_size);

				p_dest = g_roleMgr.get_role(dw_dest_role);
				if(VALID_POINT(p_dest))
				{
					// 双向好友？
					tagFriend *p_friend_dest = p_dest->GetFriendPtr(dw_src_role);
					if(VALID_POINT(p_friend_dest))
					{
						dw_friend_val = p_friend_dest->dwFriVal ;

						NET_DB2C_insert_friend_value sendDB;
						sendDB.dw_role_id = dw_dest_role;
						sendDB.s_friendship_save_.dw_friend_id_ = dw_src_role;
						sendDB.s_friendship_save_.n_frival_ = dw_friend_val;
						g_dbSession.Send(&sendDB, sendDB.dw_size);

						NET_DB2C_insert_friend_value sendDB1;
						sendDB1.dw_role_id = dw_src_role ;
						sendDB1.s_friendship_save_.dw_friend_id_ = dw_dest_role;
						sendDB1.s_friendship_save_.n_frival_ = dw_friend_val;
						g_dbSession.Send(&sendDB1, sendDB1.dw_size);
					}

					NET_SIS_make_friend_notice sendNotice;
					sendNotice.dw_role_id = dw_src_role;
					p_dest->SendMessage(&sendNotice, sendNotice.dw_size);


				}
				p_src_role->GetAchievementMgr().UpdateAchievementCriteria(eta_add_friend, p_src_role->GetFriendCount());

				break;
			}

			// 好友列表是否达到上限100
			if( i == (MAX_FRIENDNUM - 1))
			{
				dw_error_code = E_Friend_Max_Num;
				goto Exit;
			}
			
		}
	}

	{
		NET_SIS_role_make_friend send;
		send.dwDestRoleID = dw_dest_role;
		send.bOnline = b_online;
		send.nLevel = level;
		send.eClassType = p_role_info->e_class_type_;
		send.by_sex = p_role_info->by_sex_;
		send.dw_error_code = dw_error_code;
		p_src_role->SendMessage(&send, send.dw_size);

		if(dw_friend_val > 0)
		{// 增加好友度
			NET_SIS_update_friend_value sendD;
			sendD.dw_role_id = dw_dest_role;
			sendD.nFriVal = dw_friend_val;
			p_src_role->SendMessage(&sendD, sendD.dw_size);

			NET_SIS_update_friend_value sendS;
			sendS.dw_role_id = dw_src_role;
			sendS.nFriVal = dw_friend_val;
			p_dest->SendMessage(&sendS, sendS.dw_size);
		}
		return;
	}
	

Exit:
	NET_SIS_role_make_friend senderror;
	senderror.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&senderror, senderror.dw_size);

	return;
}


// 删除好友
VOID social_mgr::cancel_friend(DWORD dw_sender, VOID *p_message)
{
	NET_SIC_role_cancel_friend* p_receive = (NET_SIC_role_cancel_friend*)p_message;

	DWORD dw_src_role = dw_sender;
	DWORD dw_dest_role = p_receive->dwDestRoleID;
	BYTE  by_group = 1;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);

	if(!VALID_POINT(p_src_role)) return;

	DWORD	dw_error_code = E_Success;
	// 社会关系检查
	// 夫妻，不可被删除
	// 师徒，不可被删除 
	// 情侣，不可被删除
	s_role_info *p_role_info = g_roleMgr.get_role_info(dw_dest_role);
	if(!VALID_POINT(p_role_info))
		dw_error_code = E_Friend_Target_Not_Exist;

	if(dw_error_code == E_Success)
	{
		by_group = _cancel_friend(p_src_role, dw_src_role, dw_dest_role);
	}

	NET_SIS_role_cancel_friend send;
	send.dwDestRoleID = dw_dest_role;
	send.byGroup = by_group;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}

BYTE social_mgr::_cancel_friend(Role *p_src_role, DWORD dw_src_role, DWORD dw_dest_role)
{
	BYTE  by_group = 1;
	DWORD dw_friend_val = 0;

	for (INT i = 0; i < MAX_FRIENDNUM; ++i)
	{
		if(p_src_role->GetFriend(i).dwFriendID == dw_dest_role)
		{
			by_group = p_src_role->GetFriend(i).byGroup;
			dw_friend_val = p_src_role->GetFriend(i).dwFriVal;
			p_src_role->SetFriend(i, INVALID_VALUE);

			NET_DB2C_delete_friend send;
			send.dw_role_id = dw_src_role;
			send.dw_friend_id_ = dw_dest_role;
			g_dbSession.Send(&send, send.dw_size);

			if(dw_friend_val > 0)
			{
				// 为了存储数据库方便,小ID放到dw_role_id
				NET_DB2C_delete_frival sendFV;
				sendFV.dw_role_id = (dw_src_role < dw_dest_role) ? dw_src_role : dw_dest_role;
				sendFV.dw_friend_id_ = (dw_src_role < dw_dest_role) ? dw_dest_role : dw_src_role;
				g_dbSession.Send(&sendFV, sendFV.dw_size);

				// 如果被删除的好友在线,设置其好友度为0
				Role* p_dest_role = g_roleMgr.get_role(dw_dest_role);
				if(VALID_POINT(p_dest_role))
				{
					tagFriend *p_friend = p_dest_role->GetFriendPtr(dw_src_role);
					if(VALID_POINT(p_friend))
					{
						p_friend->dwFriVal = 0;
						NET_SIS_update_friend_value send;
						send.dw_role_id = dw_src_role;
						send.nFriVal = 0;
						p_dest_role->SendMessage(&send, send.dw_size);
					}
				}				
			}

			break;
		}
	}
	return by_group;
}


// 更新好友分组
VOID social_mgr::update_friend_group(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_update_friend_group* p_receive = (NET_SIC_update_friend_group*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;
	BYTE	by_new_group = p_receive->byGroup;
	BYTE	by_old_group = 1;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);

	if(!VALID_POINT(p_src_role)) return;

	DWORD dw_error_code = E_Success;

	if(by_new_group < 1 || by_new_group > 4)
		dw_error_code = E_Friend_Group_Not_Exit;
	
	if(dw_error_code == E_Success)
	{
		for (INT i = 0; i < MAX_FRIENDNUM; ++i)
		{
			if(p_src_role->GetFriend(i).dwFriendID == dw_dest_role)
			{
				by_old_group = p_src_role->GetFriend(i).byGroup;
				p_src_role->SetFriend(i, dw_dest_role, 0, by_new_group);

				NET_DB2C_update_friend send;
				send.dw_role_id = dw_src_role;
				send.s_friend_save_.dw_friend_id_ = dw_dest_role;
				send.s_friend_save_.n_group_id_ = by_new_group;
				g_dbSession.Send(&send, send.dw_size);
					
				break;
			}

			// 不是好友
			if( i == (MAX_ENEMYNUM - 1))
				dw_error_code = E_Friend_Not_Friend;
		}
	}
	
	NET_SIS_update_friend_group send;
	send.dwDestRoleID = dw_dest_role;
	send.byOldGroup = by_old_group;
	send.byNewGroup = by_new_group;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}


// 加入玩家到黑名单
VOID social_mgr::insert_black_list(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_move_to_black_list* p_receive = (NET_SIC_move_to_black_list*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;
	BYTE	by_old_group = 0;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);
	if(!VALID_POINT(p_src_role)) return;

	if(dw_dest_role == INVALID_VALUE)
		return;

	DWORD	dw_error_code = E_Success;

	// 不能黑自己
	if( dw_dest_role == dw_src_role )
		dw_error_code = E_Black_Targer_Not_Sel;	

	s_role_info *p_role_info = g_roleMgr.get_role_info(dw_dest_role);
	if(!VALID_POINT(p_role_info))
	{
		dw_error_code = E_Friend_Target_Not_Exist;
		return;
	}

	// 社会关系检查
	// 夫妻，不可被删除
	// 师徒，不可被删除 
	// 情侣，不可被删除

	// 是否已在黑名单
	for(INT i = 0; i < MAX_BLACKLIST; ++i)
	{
		if(p_src_role->GetBlackList(i) == dw_dest_role)
			dw_error_code = E_Black_Target_Already_Exit;
	}

	if(dw_error_code == E_Success)
	{
		for(INT i = 0; i < MAX_BLACKLIST; ++i)
		{
			if(p_src_role->GetBlackList(i) == INVALID_VALUE)
			{
				p_src_role->SetBlackList(i, dw_dest_role);

				// 如果是好友则删除
				tagFriend *p_friend = p_src_role->GetFriendPtr(dw_dest_role);
				if(VALID_POINT(p_friend))
				{
					by_old_group = p_friend->byGroup;
					_cancel_friend(p_src_role, dw_src_role, dw_dest_role);
				}
				// 从仇人中删除
				DWORD dw_error_code = delete_enemy_list(p_src_role, dw_dest_role);
				if (dw_error_code == E_Success)
				{
					NET_SIS_delete_enemy_list send;
					send.dwDestRoleID = dw_dest_role;
					send.dw_error_code = dw_error_code;
					p_src_role->SendMessage(&send, send.dw_size);
				}

				NET_DB2C_insert_black send;
				send.dw_role_id = dw_sender;
				send.dw_black_id_ = dw_dest_role;
				g_dbSession.Send(&send, send.dw_size);

				break;
			}

			if(i == (MAX_BLACKLIST - 1))
				dw_error_code = E_Black_Max_Num;
		}
	}

	NET_SIS_move_to_black_list	send;
	send.dwDestRoleID = dw_dest_role;
	send.byOldGroup = by_old_group;
	send.bOnline = p_role_info->b_online_;
	send.nLevel = p_role_info->by_level;
	send.eClassType = p_role_info->e_class_type_;
	send.by_sex = p_role_info->by_sex_;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}


// 将玩家从黑名单中删除
VOID social_mgr::delete_black_list(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_delete_black_list* p_receive = (NET_SIC_delete_black_list*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);
	if(!VALID_POINT(p_src_role)) return;
	
	DWORD dw_error_code = delete_black_list(p_src_role, dw_dest_role);

	//s_role_info *p_role_info = g_roleMgr.get_role_info(dw_dest_role);
	//if(!VALID_POINT(p_role_info)) dw_error_code = E_Friend_Target_Not_Exist;

	//if(dw_error_code == E_Success)
	//{
	//	for(INT i = 0; i < MAX_BLACKLIST; ++i)
	//	{
	//		if(p_src_role->GetBlackList(i) == dw_dest_role)
	//		{
	//			p_src_role->SetBlackList(i, INVALID_VALUE);

	//			NET_DB2C_delete_black send;
	//			send.dw_role_id = dw_src_role;
	//			send.dw_black_id_ = dw_dest_role;
	//			g_dbSession.Send(&send, send.dw_size);

	//			break;
	//		}

	//		if(i == (MAX_BLACKLIST - 1))
	//			dw_error_code = E_Black_Target_Not_Exit;
	//	}
	//}

	NET_SIS_delete_black_list	send;
	send.dwDestRoleID = dw_dest_role;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}

DWORD social_mgr::delete_black_list(Role* p_src_role, DWORD dw_dest_role)
{
	DWORD dw_error_code = E_Success;

	s_role_info *p_role_info = g_roleMgr.get_role_info(dw_dest_role);
	if(!VALID_POINT(p_role_info)) dw_error_code = E_Friend_Target_Not_Exist;

	if(dw_error_code == E_Success)
	{
		for(INT i = 0; i < MAX_BLACKLIST; ++i)
		{
			if(p_src_role->GetBlackList(i) == dw_dest_role)
			{
				p_src_role->SetBlackList(i, INVALID_VALUE);

				NET_DB2C_delete_black send;
				send.dw_role_id = p_src_role->GetID();
				send.dw_black_id_ = dw_dest_role;
				g_dbSession.Send(&send, send.dw_size);

				break;
			}

			if(i == (MAX_BLACKLIST - 1))
				dw_error_code = E_Black_Target_Not_Exit;
		}
	}

	return dw_error_code;
}

// 添加到仇人
VOID social_mgr::add_enemy_list(DWORD dw_sender, VOID* p_message)
{
	Role* pRole = g_roleMgr.get_role(dw_sender);
	if (!VALID_POINT(pRole))
		return;

	NET_SIC_add_enemy* p_receive = (NET_SIC_add_enemy*)p_message;
	s_role_info* pEnemy = g_roleMgr.get_role_info(p_receive->dwDestRoleID);
	if (!VALID_POINT(pEnemy))
		return;


	if(!pRole->IsEnemyList(p_receive->dwDestRoleID) && !pRole->IsFriend(p_receive->dwDestRoleID))
	{// 加入仇人列表	
		for(INT i = 0; i < MAX_ENEMYNUM; i++)
		{
			if(pRole->GetEnemyList(i) == INVALID_VALUE)
			{
				// 从黑名单中删除
				DWORD dw_error_code = g_socialMgr.delete_black_list(pRole, p_receive->dwDestRoleID);
				if (dw_error_code == E_Success)
				{
					NET_SIS_delete_black_list	send;
					send.dwDestRoleID = p_receive->dwDestRoleID;
					send.dw_error_code = dw_error_code;
					pRole->SendMessage(&send, send.dw_size);
				}

				pRole->SetEnemyList(i, p_receive->dwDestRoleID);

				NET_DB2C_insert_enemy send;
				send.dw_role_id = pRole->GetID();
				send.dw_enemy_id_ = p_receive->dwDestRoleID;
				g_dbSession.Send(&send, send.dw_size);

				NET_SIS_add_enemy AddSend;
				AddSend.dwDestRoleID = p_receive->dwDestRoleID;
				AddSend.bOnline = TRUE;
				AddSend.nLevel = pEnemy->by_level;
				AddSend.eClassType = pEnemy->e_class_type_;
				AddSend.by_sex = pEnemy->by_sex_;
				pRole->SendMessage(&AddSend, AddSend.dw_size);
				break;
			}
		}
	}
}
// 将玩家从仇人列表中删除
DWORD social_mgr::delete_enemy_list(Role* p_src_role, DWORD dw_dest_role)
{
	DWORD dw_error_code = E_Success;
	for(INT i = 0; i < MAX_ENEMYNUM; i++)
	{
		if(p_src_role->GetEnemyList(i) == dw_dest_role)
		{
			p_src_role->SetEnemyList(i, INVALID_VALUE);

			NET_DB2C_delete_enemy send;
			send.dw_role_id = p_src_role->GetID();
			send.dw_enemy_id_ = dw_dest_role;
			g_dbSession.Send(&send, send.dw_size);
			break;
		}

		if(i == (MAX_ENEMYNUM - 1))
			dw_error_code = E_Enemy_Target_Not_Exit;
	}

	return dw_error_code;
}
VOID social_mgr::delete_enemy_list(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_delete_enemy_list* p_receive = (NET_SIC_delete_enemy_list*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);
	if(!VALID_POINT(p_src_role)) return;

	DWORD dw_error_code = delete_enemy_list(p_src_role, dw_dest_role);
	//for(INT i = 0; i < MAX_ENEMYNUM; i++)
	//{
	//	if(p_src_role->GetEnemyList(i) == dw_dest_role)
	//	{
	//		p_src_role->SetEnemyList(i, INVALID_VALUE);

	//		NET_DB2C_delete_enemy send;
	//		send.dw_role_id = dw_src_role;
	//		send.dw_enemy_id_ = dw_dest_role;
	//		g_dbSession.Send(&send, send.dw_size);
	//		break;
	//	}

	//	if(i == (MAX_ENEMYNUM - 1))
	//		dw_error_code = E_Enemy_Target_Not_Exit;
	//}

	NET_SIS_delete_enemy_list send;
	send.dwDestRoleID = dw_dest_role;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}


// 获取仇人位置
VOID social_mgr::get_enemy_position(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_get_enemy_position* p_receive = (NET_SIC_get_enemy_position*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;

	Role* p_src_role = g_roleMgr.get_role(dw_src_role);
	if(!VALID_POINT(p_src_role)) return;

	DWORD dw_error_code = E_Success;

	Role* p_dest_role = g_roleMgr.get_role(dw_dest_role);
	if(!VALID_POINT(p_dest_role))
	{
		dw_error_code = E_Enemy_No_OnLine;
		NET_SIS_get_enemy_position send;
		send.dw_error_code = dw_error_code;
		p_src_role->SendMessage(&send, send.dw_size);
		return;
	}

	NET_SIS_get_enemy_position send;
	send.fx = p_dest_role->GetCurPos().x;
	send.fz = p_dest_role->GetCurPos().z;
	send.dw_role_id = p_dest_role->GetID();
	send.dwMapID = p_dest_role->GetMapID();
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
}


// 送礼物
VOID social_mgr::send_gift(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_send_gift* p_receive = (NET_SIC_send_gift*)p_message;

	DWORD	dw_src_role = dw_sender;
	DWORD	dw_dest_role = p_receive->dwDestRoleID;
	//INT64	n64_item_id = p_receive->n64ItemID;
	DWORD	dw_data_id = INVALID_VALUE;
	tagItem *p_item = (tagItem*)INVALID_VALUE;

	Role	*p_src_role = g_roleMgr.get_role(dw_src_role);
	Role	*p_dest_role = g_roleMgr.get_role(dw_dest_role);

	if(!VALID_POINT(p_src_role)) return;

	DWORD	dw_error_code = E_Success;

	if (!VALID_POINT(p_dest_role))//检查是否在线
	{
		dw_error_code = E_Gift_Friend_Not_Online;
		NET_SIS_send_gift_to_friend send;
		send.dwSrcRoleID = dw_src_role;
		send.byLevel = p_receive->byLevel;
		send.byAddValue = 0;
		send.dw_error_code = dw_error_code;
		p_src_role->SendMessage(&send, send.dw_size);
		return;
	}
	//玩家性别检查
	if (p_src_role->GetSex() == p_dest_role->GetSex())
	{
		dw_error_code = E_Gift_Not_Gift;
		NET_SIS_send_gift_to_friend send;
		send.dwSrcRoleID = dw_src_role;
		send.byLevel = p_receive->byLevel;
		send.byAddValue = 0;
		send.dw_error_code = dw_error_code;
		p_src_role->SendMessage(&send, send.dw_size);
		return;
	}
	//package_list<tagItem*> list_item;
	INT32 n_FlowerNum_Total = 0;//背包中总花数
	INT32 n_FlowerNum_Bind = 0;//背包中绑定的花数
	INT16 n_SendFlower = 0;
	//dw_error_code = can_send_gift(p_src_role, p_dest_role, dw_src_role, dw_dest_role, n64_item_id, p_item);
	dw_error_code = can_send_gift(p_src_role, p_dest_role, dw_dest_role,p_receive->byLevel,n_FlowerNum_Total,n_FlowerNum_Bind,n_SendFlower);
	if(E_Success == dw_error_code)
	{
		//// 删除礼物
		//dw_data_id = p_item->dw_data_id;
		//p_src_role->GetItemMgr().DelFromBag(n64_item_id, (DWORD)elcid_send_gift, (INT16)1);

		//// 加入礼物队列
		//tagSendGift SendGift(dw_src_role, dw_dest_role, dw_data_id);
		//send_gifts_.push_back(SendGift);
		//送礼方消耗物品
		if (n_FlowerNum_Total - n_SendFlower >= 0)//此时不需要消耗元宝
		{
			if (n_FlowerNum_Bind >= n_SendFlower)//绑定的玫瑰花够数
			{
				p_src_role->GetItemMgr().RemoveFromRole((DWORD)SEND_GIFT_NEED_ITEM,n_SendFlower,(DWORD)elcid_send_gift,1);
			}
			else//绑定的玫瑰花不够
			{
				//先消耗绑定的
				if (n_FlowerNum_Bind > 0)
				{
					p_src_role->GetItemMgr().RemoveFromRole((DWORD)SEND_GIFT_NEED_ITEM,n_FlowerNum_Bind,(DWORD)elcid_send_gift,1);
				}
				p_src_role->GetItemMgr().RemoveFromRole((DWORD)SEND_GIFT_NEED_ITEM,(n_SendFlower-n_FlowerNum_Bind),(DWORD)elcid_send_gift,0);
			}
		}
		else//若还需要消耗元宝
		{
			//先消耗背包中所有的玫瑰花
			p_src_role->GetItemMgr().RemoveFromRole((DWORD)SEND_GIFT_NEED_ITEM,n_FlowerNum_Total,(DWORD)elcid_send_gift);
			if (n_SendFlower - n_FlowerNum_Total > 0)//防止异常
			{
				p_src_role->GetCurMgr().DecBaiBaoYuanBao((n_SendFlower-n_FlowerNum_Total)*10,(DWORD)elcid_send_gift);
			}
		}
		//收礼方增加魅力值
		p_dest_role->add_shihun(n_SendFlower);
		//若送99朵，则全服广播
		if (3 == p_receive->byLevel)
		{
			HearSayHelper::SendMessage(EHST_LianHuen, p_src_role->GetID(),p_dest_role->GetID());
		}

		if(VALID_POINT(p_src_role->GetScript()))
			p_src_role->GetScript()->OnSonghua(p_src_role,n_SendFlower);

		NET_SIS_send_gift_to_friend send;
		send.dwSrcRoleID = dw_src_role;
		send.byLevel = p_receive->byLevel;
		send.byAddValue = n_SendFlower;
		send.dw_error_code = dw_error_code;
		p_dest_role->SendMessage(&send, send.dw_size);
	}

	NET_SIS_send_gift_to_friend send;
	send.dwSrcRoleID = dw_src_role;
	send.byLevel = p_receive->byLevel;
	send.byAddValue = n_SendFlower;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);
	/*NET_SIS_send_gift_to_sender send;
	send.dw_error_code = dw_error_code;
	p_src_role->SendMessage(&send, send.dw_size);*/
}


// 是否可送礼物
DWORD social_mgr::can_send_gift(Role* p_src_role, Role* p_dest_role, DWORD dw_dest_role, BYTE level, INT32 &nTotalNum,INT32 &nBindNum,INT16 &n_SendFlowers)
{
	// 判断好友ID是否在线
	if(!VALID_POINT(p_dest_role))
		return E_Gift_Friend_Not_Online;

	// 角色ID不存在
	s_role_info *p_role_info = g_roleMgr.get_role_info(dw_dest_role);
	if(!VALID_POINT(p_role_info)) return E_Friend_Target_Not_Exist;
	
	// 检查送礼者是否满足赠送当前玫瑰数的条件
	nTotalNum = p_src_role->GetItemMgr().GetBagSameItemCount((DWORD)SEND_GIFT_NEED_ITEM);//获得背包中的玫瑰花总数，包括绑定的
	nBindNum = p_src_role->GetItemMgr().GetBagSameBindItemCount((DWORD)SEND_GIFT_NEED_ITEM,TRUE);
	INT32 n_Yuanbao = p_src_role->GetCurMgr().GetBaiBaoYuanBao();
	if (1 == level)
	{
		n_SendFlowers = 1;
	}
	else if (2 == level)
	{
		n_SendFlowers = 9;
	}
	else if (3 == level)
	{
		n_SendFlowers = 99;
	}
	else
	{
		return INVALID_VALUE;
	}
	if ((nTotalNum + n_Yuanbao / 10) < n_SendFlowers)
	{
		return E_Gift_Not_Exit;//玩家道具不足，无法赠送相应的玫瑰
	}
	/*p_item = p_src_role->GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(p_item)) return  E_Gift_Not_Exit;*/

	// 检测是否为礼物
	/*if(p_item->pProtoType->eSpecFunc != EISF_FriendGift)
		return E_Gift_Not_Gift;*/

	// 只能向好友赠送礼物
	/*tagFriend *p_src_friend = p_src_role->GetFriendPtr(dw_dest_role);
	tagFriend *p_dest_friend = p_dest_role->GetFriendPtr(dw_src_role);
	if(!VALID_POINT(p_src_friend) || !VALID_POINT(p_dest_friend))
		return E_Gift_Not_Both_Friend;*/

	return E_Success;
}

// 收到礼物后回复
VOID social_mgr::send_gift_reply(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_send_gift_reply* p_receive = (NET_SIC_send_gift_reply*)p_message;

	DWORD dw_src_role = p_receive->dwSrcRoleID;
	DWORD dw_dest_role = dw_sender;
	DWORD dw_data_id = p_receive->dw_data_id;
	BOOL  b_result = p_receive->bResult;
	Role *p_src_role = g_roleMgr.get_role(dw_src_role);
	Role *p_dest_role = g_roleMgr.get_role(dw_dest_role);
	tagFriend *p_src_friend = (tagFriend*)INVALID_VALUE;
	tagFriend *p_dest_friend = (tagFriend*) INVALID_VALUE;

	if(b_result)
	{// 玩家接受礼物

		if(VALID_POINT(p_src_role)) 
			p_src_friend = p_src_role->GetFriendPtr(dw_dest_role);

		if(VALID_POINT(p_dest_role))
			p_dest_friend = p_dest_role->GetFriendPtr(dw_src_role);

		// 查找
		BOOL	b_send = FALSE;				
		vector<tagSendGift>::iterator it = send_gifts_.begin();
		while(it != send_gifts_.end())
		{
			if(it->dwSrcRoleID == dw_src_role && it->dwDestRoleID == dw_dest_role && it->dw_data_id == dw_data_id)
			{
				b_send = TRUE;
				it = send_gifts_.erase(it);
				continue;
			}
			++it;
		}

		if(!b_send)
			return;

		// 双向好友,增加好友度
		tagItemProto *pItemProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
		if(!VALID_POINT(dw_data_id)) return;

		BOOL b_insert = TRUE;
		DWORD dw_friend_val = 0;
		if(VALID_POINT(p_src_friend))
		{
			if(p_src_friend->dwFriVal > 0)
				b_insert = FALSE;

			dw_friend_val = p_src_friend->dwFriVal += pItemProto->nSpecFuncVal1;

			p_src_friend->dwFriVal = (p_src_friend->dwFriVal > MAX_FRIENDVAL) ? MAX_FRIENDVAL : p_src_friend->dwFriVal;
			dw_friend_val = (dw_friend_val > MAX_FRIENDVAL) ? MAX_FRIENDVAL : dw_friend_val;


			NET_SIS_update_friend_value send;
			send.dw_role_id = dw_dest_role;
			send.nFriVal = dw_friend_val;
			p_src_role->SendMessage(&send, send.dw_size);
		}

		if(VALID_POINT(p_dest_friend))
		{
			if(p_dest_friend->dwFriVal > 0)
				b_insert = FALSE;

			dw_friend_val = p_dest_friend->dwFriVal += pItemProto->nSpecFuncVal1;

			p_src_friend->dwFriVal = (p_dest_friend->dwFriVal > MAX_FRIENDVAL) ? MAX_FRIENDVAL : p_dest_friend->dwFriVal;
			dw_friend_val = (dw_friend_val > MAX_FRIENDVAL) ? MAX_FRIENDVAL : dw_friend_val;

			NET_SIS_update_friend_value send;
			send.dw_role_id = dw_src_role;
			send.nFriVal = dw_friend_val;
			p_dest_role->SendMessage(&send, send.dw_size);
		}

		if(b_insert)
		{
			NET_DB2C_insert_friend_value sendDB;
			sendDB.dw_role_id = (dw_src_role < dw_dest_role) ? dw_src_role : dw_dest_role;
			sendDB.s_friendship_save_.dw_friend_id_ = (dw_src_role > dw_dest_role) ? dw_src_role : dw_dest_role;
			sendDB.s_friendship_save_.n_frival_ = dw_friend_val;
			g_dbSession.Send(&sendDB, sendDB.dw_size);
		}
		else{
			NET_DB2C_update_friend_value sendDB;
			sendDB.dw_role_id = (dw_src_role < dw_dest_role) ? dw_src_role : dw_dest_role;
			sendDB.s_friendship_save_.dw_friend_id_ = (dw_src_role > dw_dest_role) ? dw_src_role : dw_dest_role;
			sendDB.s_friendship_save_.n_frival_ = dw_friend_val;
			g_dbSession.Send(&sendDB, sendDB.dw_size);
		}
	}

	NET_SIS_send_gift_broad	 send;
	send.dwSrcRoleID = dw_src_role;
	send.dwDestRoleID = dw_dest_role;
	send.dw_data_id = dw_data_id;
	send.bResult = b_result;
	g_roleMgr.send_world_msg(&send, send.dw_size);
}


// 同步玩家角色
VOID social_mgr::syn_role_level(DWORD dw_sender, VOID* p_message)
{
	NET_SIS_change_role_level* p_receive = (NET_SIS_change_role_level*)p_message;

	s_role_info	*p_role_info = g_roleMgr.get_role_info(dw_sender);
	if(VALID_POINT(p_role_info)) p_role_info->by_level = p_receive->nLevel;
}

// 获取好友度等级
INT social_mgr::getFriendValueLevel(INT nValue)
{
	if (nValue < 100)
		return 1;
	if (nValue < 500)
		return 2;
	if (nValue < 1300)
		return 3;
	if (nValue < 3200)
		return 4;
	return 5;
}

VOID social_mgr::invitePlayerToCompractice(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_invite_practice* p_receive = (NET_SIC_invite_practice*)p_message;

	DWORD	dwSrcRoleID = dw_sender;
	DWORD	dwDestRoleID = p_receive->dwDestRoleID;

	if( dwSrcRoleID == dwDestRoleID ) return;	// 邀请自己，外挂！

	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);

	if( !VALID_POINT(pSrcRole) )
		return;
	DWORD dw_error_code = E_Success;
	// 被邀请人不在线
	if( !VALID_POINT(pDestRole) )
	{
		dw_error_code = E_Compractice_No_OnLine;
		NET_SIS_invite_practice_to_both send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dw_error_code = dw_error_code;
		pSrcRole->SendMessage(&send, send.dw_size);
		return;
	}
	//首先判断是被邀请方否重复邀请
	if (pDestRole->GetComPracticePartner() != INVALID_VALUE)//被邀请方已别的玩家邀请
	{
		dw_error_code = E_Comparctice_Role_Invited;
		goto Exit;
	}
	//双方等级检查
	if (pSrcRole->get_level() < 20 || pDestRole->get_level() < 20)
	{
		dw_error_code = E_Compractice_Level;
		goto Exit;
	}
	//双方性别检查
	if (pSrcRole->GetSex() == pDestRole->GetSex())
	{
		dw_error_code = E_Compractice_Sex;
		goto Exit;
	}
	//双方距离检查
	FLOAT fDist = Vec3DistSq(pSrcRole->GetCurPos(), pDestRole->GetCurPos());
	if (fDist > COMPRACTICE_MAX_DISTANCE*COMPRACTICE_MAX_DISTANCE)
	{
		dw_error_code = E_Compractice_Long;
		goto Exit;
	}
	//邀请方处于双修状态
	if (pSrcRole->IsInRoleState(ERS_ComPractice))
	{
		dw_error_code = E_Compractice_Local_InPractice;
		goto Exit;
	}
	//被邀请方处于双修状态
	if (pDestRole->IsInRoleState(ERS_ComPractice))
	{
		dw_error_code = E_Compractice_Remote_InPractice;
		goto Exit;
	}
	//双方有处于摆摊状态的
	if (pSrcRole->IsInRoleState(ERS_Stall) || pDestRole->IsInRoleState(ERS_Stall))
	{
		dw_error_code = E_Compractice_InStall;
		goto Exit;
	}
	//判断双方今日双修次数
	if (pSrcRole->GetDayClearData(ERDCT_ComPractice) >= (MAX_COMPRACTICENUM+GET_VIP_EXTVAL(pSrcRole->GetVIPLevel(),COMPRACTICE_ADD,int)))
	{
		dw_error_code = E_Compractice_Local_Practice_Full;
		goto Exit;
	}
	if (pDestRole->GetDayClearData(ERDCT_ComPractice) >= (MAX_COMPRACTICENUM+GET_VIP_EXTVAL(pDestRole->GetVIPLevel(),COMPRACTICE_ADD,int)))
	{
		dw_error_code = E_Compractice_Remote_Practice_Full;
		goto Exit;
	}
	//参与双修的双方区域检查
	if (!pSrcRole->IsInRoleState(ERS_SafeArea) || !pDestRole->IsInRoleState(ERS_SafeArea))
	{
		dw_error_code = E_Compractice_NotSafearea;
		goto Exit;
	}
	
	//判断成功给被邀请方发送邀请
	if (E_Success == dw_error_code)
	{
		NET_SIS_invite_practice_to_both send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dw_error_code = dw_error_code;
		pDestRole->SendMessage(&send, send.dw_size);
		//向邀请方也发消息，给邀请方提示 gx add
		pSrcRole->SendMessage(&send,send.dw_size);
		//邀请成功后，要给标志，防止再有别人邀请 gx add 2013.9.2
		//pSrcRole->SetComPracticePartner(dwDestRoleID);
		pDestRole->SetComPracticePartner(dwSrcRoleID);
		return;
	}

Exit:
	//失败，仅给邀请方发送
	NET_SIS_invite_practice_to_both send2;
	send2.dwSrcRoleID = dwSrcRoleID;
	send2.dw_error_code = dw_error_code;
	pSrcRole->SendMessage(&send2, send2.dw_size);
}
void social_mgr::invitePlayerToCompracticeReply(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_invite_practice_reply* p_receive = (NET_SIC_invite_practice_reply*)p_message;
	DWORD	dwSrcRoleID = p_receive->dwSrcRoleID;
	DWORD	dwDestRoleID = dw_sender;
	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);
	if( !VALID_POINT(pSrcRole) )
		return;
	if (!VALID_POINT(pDestRole))
		return;
	DWORD dw_error_code = E_Success;
	BOOL  b_result = p_receive->bAgree;
	//若被邀请方不同意
	if (!b_result)
	{
		dw_error_code = E_Compractice_Disagree;
		//只给邀请方发消息
		NET_SIS_invite_practice_to_src send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dwDesRoleID = dwDestRoleID;
		send.dw_error_code = dw_error_code;
		//若邀请方不处于双修中，才向其发该消息
		if (!pSrcRole->IsInRoleState(ERS_ComPractice))
		{
			pSrcRole->SendMessage(&send, send.dw_size);
		}
		//去标志
		//pSrcRole->SetComPracticePartner(INVALID_VALUE);
		pDestRole->SetComPracticePartner(INVALID_VALUE);
	}
	else
	{
		//玩家同意后需要再次判断双方所处的状态
		//双方等级检查
		if (pSrcRole->get_level() < 20 || pDestRole->get_level() < 20)
		{
			dw_error_code = E_Compractice_Level;
			goto Exit;
		}
		//双方性别检查
		if (pSrcRole->GetSex() == pDestRole->GetSex())
		{
			dw_error_code = E_Compractice_Sex;
			goto Exit;
		}
		//双方距离检查
		FLOAT fDist = Vec3DistSq(pSrcRole->GetCurPos(), pDestRole->GetCurPos());
		if (fDist > COMPRACTICE_MAX_DISTANCE*COMPRACTICE_MAX_DISTANCE)
		{
			dw_error_code = E_Compractice_Long;
			goto Exit;
		}
		//邀请方处于双修状态
		if (pSrcRole->IsInRoleState(ERS_ComPractice))
		{
			dw_error_code = E_Compractice_Remote_InPractice;
			goto Exit;
		}
		//被邀请方处于双修状态
		if (pDestRole->IsInRoleState(ERS_ComPractice))
		{
			dw_error_code = E_Compractice_Local_InPractice;
			goto Exit;
		}
		//双方有处于摆摊状态的
		if (pSrcRole->IsInRoleState(ERS_Stall) || pDestRole->IsInRoleState(ERS_Stall))
		{
			dw_error_code = E_Compractice_InStall;
			goto Exit;
		}
		//判断双方今日双修次数
		if (pSrcRole->GetDayClearData(ERDCT_ComPractice) >= (MAX_COMPRACTICENUM+GET_VIP_EXTVAL(pSrcRole->GetVIPLevel(),COMPRACTICE_ADD,int)))
		{
			dw_error_code = E_Compractice_Remote_Practice_Full;
			goto Exit;
		}
		if (pDestRole->GetDayClearData(ERDCT_ComPractice) >= (MAX_COMPRACTICENUM+GET_VIP_EXTVAL(pDestRole->GetVIPLevel(),COMPRACTICE_ADD,int)))
		{
			dw_error_code = E_Compractice_Local_Practice_Full;
			goto Exit;
		}
		//参与双修的双方区域检查
		if (!pSrcRole->IsInRoleState(ERS_SafeArea) || !pDestRole->IsInRoleState(ERS_SafeArea))
		{
			dw_error_code = E_Compractice_NotSafearea;
			goto Exit;
		}
		//若判断成功，双方可以进入双修状态
		if (E_Success == dw_error_code)
		{
			//状态设置相关
			pSrcRole->ModRoleDayClearDate(ERDCT_ComPractice,1,FALSE);
			pDestRole->ModRoleDayClearDate(ERDCT_ComPractice,1,FALSE);

			pSrcRole->SetComPracticeLevel(pSrcRole->get_level());
			pDestRole->SetComPracticeLevel(pDestRole->get_level());

			pSrcRole->SetComPracticePartner(dwDestRoleID);
			pDestRole->SetComPracticePartner(dwSrcRoleID);

			pSrcRole->SetRoleState(ERS_ComPractice,FALSE);
			pDestRole->SetRoleState(ERS_ComPractice,FALSE);

			if(VALID_POINT(pSrcRole->GetScript()))
				pSrcRole->GetScript()->OnShuangxiu(pSrcRole);

			if(VALID_POINT(pDestRole->GetScript()))
				pDestRole->GetScript()->OnShuangxiu(pDestRole);

			//双修状态下，需下马
			pSrcRole->StopMount();
			pDestRole->StopMount();

			NET_SIS_invite_practice_to_src send;
			send.dwSrcRoleID = dwSrcRoleID;
			send.dwDesRoleID = dwDestRoleID;
			send.dw_error_code = dw_error_code;
			pSrcRole->SendMessage(&send, send.dw_size);
			pDestRole->SendMessage(&send,send.dw_size);
			//发视野消息
			NET_SIS_practice_to_all send2;
			send2.dwSrcRoleID = dwSrcRoleID;
			send2.dwDesRoleID = dwDestRoleID;
			if(VALID_POINT(pSrcRole->get_map()))
			{
				pSrcRole->get_map()->send_big_visible_tile_message(pSrcRole, &send2, send2.dw_size);
			}
			return ;
		}
Exit:
		//判断不成功仅给被邀请方发消息
		NET_SIS_invite_practice_to_src send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dwDesRoleID = dwDestRoleID;
		send.dw_error_code = dw_error_code;
		pDestRole->SendMessage(&send, send.dw_size);
		//去标志
		//pSrcRole->SetComPracticePartner(INVALID_VALUE);
		pDestRole->SetComPracticePartner(INVALID_VALUE);
	}
}
//玩家主动取消双修
VOID social_mgr::playerCancelCompractice(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_cancel_practice* p_receive = (NET_SIC_cancel_practice*)p_message;
	DWORD dwLocRoleID = dw_sender;
	DWORD dwPartnerID = p_receive->dwPartnerID;
	if (dwLocRoleID == dwPartnerID)
		return;
	Role* pLocRole = g_roleMgr.get_role(dwLocRoleID);
	Role* pPartner = g_roleMgr.get_role(dwPartnerID);
	if(!VALID_POINT(pLocRole))
		return;
	if (!VALID_POINT(pPartner))
		return;
	//若玩家不在双修状态中，则非法
	if (!pLocRole->IsInRoleState(ERS_ComPractice))
		return;
	if (!pPartner->IsInRoleState(ERS_ComPractice))
		return;
	DWORD dw_error_code = E_Success;
	pLocRole->UnsetRoleState(ERS_ComPractice);
	pPartner->UnsetRoleState(ERS_ComPractice);
	//去标志
	pLocRole->SetComPracticePartner(INVALID_VALUE);
	pPartner->SetComPracticePartner(INVALID_VALUE);
	return;
}
//结婚相关
VOID social_mgr::maleplayer_propose(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_male_propose* p_receive = (NET_SIC_male_propose*)p_message;

	DWORD	dwSrcRoleID = dw_sender;
	DWORD	dwDestRoleID = p_receive->dwDestRoleID;

	if( dwSrcRoleID == dwDestRoleID ) return;	// 邀请自己，外挂！

	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);

	if( !VALID_POINT(pSrcRole) )
		return;
	DWORD dw_error_code = E_Success;

	//检查男方玩家组队情况
	DWORD dwSrcTeamID = pSrcRole->GetTeamID();
	if (INVALID_VALUE == dwSrcTeamID)
	{
		return;
	}
	const Team* pSrcTeam = g_groupMgr.GetTeamPtr(dwSrcTeamID);
	if (!VALID_POINT(pSrcTeam))
		return;
	if (pSrcTeam->get_member_number() != 2)
	{
		dw_error_code = E_Propose_No_Team;
		goto Exit;
	}
	Role* pFirstRole = pSrcTeam->get_member(0);
	if (!VALID_POINT(pFirstRole))
		return;
	Role* pSecondRole = pSrcTeam->get_member(1);
	if (!VALID_POINT(pSecondRole))
		return;
	//若男方是队长
	if (dwSrcRoleID == pFirstRole->GetID())
	{
		if (p_receive->dwDestRoleID != pSecondRole->GetID())
		{
			return;//数据不同步，异常，直接退出
		}
	}
	else if (dwSrcRoleID == pSecondRole->GetID())
	{
		if (p_receive->dwDestRoleID != pFirstRole->GetID())
		{
			return;//数据不同步，异常，直接退出
		}
	}
	else
	{
		return;
	}
	// 被邀请人不在线
	if( !VALID_POINT(pDestRole) )
	{
		dw_error_code = E_Compractice_No_OnLine;
		goto Exit;
	}
	//男女双方性别判断
	if ((pSrcRole->GetSex() != 1) || (pDestRole->GetSex() != 0))
		return;
	// 男方是否有配偶
	if (VALID_POINT(pSrcRole->GetSpouseID()))
	{
		dw_error_code = E_Propose_Male_Married;
		goto Exit;
	}
	// 女方是否有配偶
	if (VALID_POINT(pDestRole->GetSpouseID()))
	{
		dw_error_code = E_Propose_Female_Married;
		goto Exit;
	}
	//男女双方等级检查
	if ((pSrcRole->get_level() < MARRY_MIN_LEVEL) || (pDestRole->get_level() < MARRY_MIN_LEVEL))
	{
		dw_error_code = E_Propose_Marray_Level;
		goto Exit;
	}
	//双方道具检查
	{
		//男方双方婚戒检查
		package_list<tagItem*> list_item;
		INT32 n_num_male = pSrcRole->GetItemMgr().GetBagSameItemList(list_item, (DWORD)MARRY_NEED_ITEM, 1);
		INT32 n_num_female = pDestRole->GetItemMgr().GetBagSameItemList(list_item, (DWORD)MARRY_NEED_ITEM, 1);
		if (n_num_female < 1 || n_num_male < 1)
		{
			dw_error_code = E_Propose_Marry_Lack_Item;
			goto Exit;
		}
		//地图检查 临时定义为新手村为结婚地图
		if ((pSrcRole->GetMapID() != get_tool()->crc32(_T("m19")))||(pDestRole->GetMapID() != get_tool()->crc32(_T("m19"))))
		{
			dw_error_code = E_Propose_No_Map;
			goto Exit;
		}
	}
	
	//若判断无误后，向女性玩家发消息
	if (E_Success == dw_error_code)
	{
		NET_SIS_propose_to_both send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dw_error_code = dw_error_code;
		pDestRole->SendMessage(&send,send.dw_size);
	}
	
Exit:
	NET_SIS_propose_to_both send;
	send.dwSrcRoleID = dwSrcRoleID;
	send.dw_error_code = dw_error_code;
	pSrcRole->SendMessage(&send, send.dw_size);
	return;
}
VOID social_mgr::femaleplayer_propose_reply(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_propose_reply* p_receive = (NET_SIC_propose_reply*)p_message;
	DWORD	dwSrcRoleID = p_receive->dwSrcRoleID;
	DWORD	dwDestRoleID = dw_sender;
	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);
	if( !VALID_POINT(pSrcRole) )
		return;
	if (!VALID_POINT(pDestRole))
		return;
	DWORD dw_error_code = E_Success;
	BOOL  b_result = p_receive->bAgree;
	//若被女方不同意
	if (!b_result)
	{
		dw_error_code = E_Propose_Female_Disagree;
		//只给邀请方发消息
		NET_SIS_propose_reply_to_both send;
		send.dwSrcRoleID = dwSrcRoleID;
		send.dwDesRoleID = dwDestRoleID;
		send.dw_error_code = dw_error_code;
		pSrcRole->SendMessage(&send, send.dw_size);
	}
	else
	{
		//玩家同意后需要再次判断双方所处的状态
		//检查男方玩家组队情况
		DWORD dwSrcTeamID = pSrcRole->GetTeamID();
		if (INVALID_VALUE == dwSrcTeamID)
		{
			return;
		}
		const Team* pSrcTeam = g_groupMgr.GetTeamPtr(dwSrcTeamID);
		if (!VALID_POINT(pSrcTeam))
			return;
		if (pSrcTeam->get_member_number() != 2)
		{
			return;
		}
		Role* pFirstRole = pSrcTeam->get_member(0);
		if (!VALID_POINT(pFirstRole))
			return;
		Role* pSecondRole = pSrcTeam->get_member(1);
		if (!VALID_POINT(pSecondRole))
			return;
		//若男方是队长
		if (dwSrcRoleID == pFirstRole->GetID())
		{
			if (dwDestRoleID != pSecondRole->GetID())
			{
				return;//数据不同步，异常，直接退出
			}
		}
		else if (dwSrcRoleID == pSecondRole->GetID())
		{
			if (dwDestRoleID != pFirstRole->GetID())
			{
				return;//数据不同步，异常，直接退出
			}
		}
		else
		{
			return;
		}
		//男女双方性别判断
		if ((pSrcRole->GetSex() != 1) || (pDestRole->GetSex() != 0))
			return;
		// 男方是否有配偶
		if (VALID_POINT(pSrcRole->GetSpouseID()))
		{
			dw_error_code = E_Propose_Male_Married;
		}
		// 女方是否有配偶
		if (VALID_POINT(pDestRole->GetSpouseID()))
		{
			dw_error_code = E_Propose_Female_Married;
		}
		//男女双方等级检查
		if ((pSrcRole->get_level() < MARRY_MIN_LEVEL) || (pDestRole->get_level() < MARRY_MIN_LEVEL))
		{
			dw_error_code = E_Propose_Marray_Level;
		}
		//男方双方婚戒检查
		package_list<tagItem*> list_item_male;
		package_list<tagItem*> list_item_female;
		INT32 n_num_male = pSrcRole->GetItemMgr().GetBagSameItemList(list_item_male, (DWORD)MARRY_NEED_ITEM, 1);
		INT32 n_num_female = pDestRole->GetItemMgr().GetBagSameItemList(list_item_female, (DWORD)MARRY_NEED_ITEM, 1);
		if (n_num_female < 1 || n_num_male < 1)
		{
			dw_error_code = E_Propose_Marry_Lack_Item;
		}
		//地图检查
		if ((pSrcRole->GetMapID() != get_tool()->crc32(_T("m19")))||(pDestRole->GetMapID() != get_tool()->crc32(_T("m19"))))
		{
			dw_error_code = E_Propose_No_Map;
		}
		//判断不成功仅给女方发消息
		if (E_Success != dw_error_code)
		{
			NET_SIS_propose_reply_to_both send;
			send.dwSrcRoleID = dwSrcRoleID;
			send.dwDesRoleID = dwDestRoleID;
			send.dw_error_code = dw_error_code;
			pDestRole->SendMessage(&send, send.dw_size);
		}
		//若判断成功，双方可以结婚
		else
		{
			//男女双方扣除婚戒
			tagItem* pRing = NULL;
			pRing = *(list_item_male.begin());
			pSrcRole->GetItemMgr().ItemUsedFromBag(pRing->n64_serial, 1, elcid_group_purchase_faild);
			pRing = *(list_item_female.begin());
			pDestRole->GetItemMgr().ItemUsedFromBag(pRing->n64_serial, 1, elcid_group_purchase_faild);

			pSrcRole->SetSpouseID(dwDestRoleID);
			pDestRole->SetSpouseID(dwSrcRoleID);

			//给夫妻双方发放夫妻礼包 gx add 2013.10.24
			pSrcRole->addRewardItem(MARRY_TRANS_ITEM,1,RF_SHANCI);
			pDestRole->addRewardItem(MARRY_TRANS_ITEM,1,RF_SHANCI);
			//end

			NET_SIS_propose_reply_to_both send;
			send.dwSrcRoleID = dwSrcRoleID;
			send.dwDesRoleID = dwDestRoleID;
			send.dw_error_code = dw_error_code;
			pSrcRole->SendMessage(&send, send.dw_size);
			pDestRole->SendMessage(&send,send.dw_size);

			//结婚后，发全服公告
			HearSayHelper::SendMessage(EHST_MARRAIAGE,dwSrcRoleID,dwDestRoleID);
			
		}
	}
}
void social_mgr::player_divorce(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_role_divorce* p_receive = (NET_SIC_role_divorce*)p_message;

	DWORD	dwSrcRoleID = dw_sender;
	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	if( !VALID_POINT(pSrcRole) )
		return;
	DWORD spousID = pSrcRole->GetSpouseID();
	if (!VALID_POINT(spousID))//无配偶直接退出
		return;
	DWORD dw_error_code = E_Success;
	//检查玩家背包
	if (pSrcRole->GetCurMgr().GetBagSilver()< DIVORCE_NEED_GOLD)
	{
		dw_error_code = E_Divorce_Lack_Yuanbao;
	}
	//地图检查
	if (pSrcRole->GetMapID() != get_tool()->crc32(_T("m19")))
	{
		return;//异常
	}
	NET_SIS_role_divorce_to_both send;
	send.dw_SrcRole_ID = dwSrcRoleID;
	send.dw_error_code = dw_error_code;
	if (E_Success == dw_error_code)
	{
		//提出离婚的玩家扣除金币
		pSrcRole->GetCurMgr().DecBagSilver(DIVORCE_NEED_GOLD,elcid_group_purchase_faild);

		pSrcRole->SetSpouseID(INVALID_VALUE);
		//获取配偶的信息
		Role* pDesRole = g_roleMgr.get_role(spousID);
		//若配偶在线
		if (VALID_POINT(pDesRole))
		{
			pDesRole->SetSpouseID(INVALID_VALUE);
			pDesRole->SendMessage(&send, send.dw_size);
		}
		else
		{
			NET_C2DB_clean_role_spouse_id send2;
			send2.dw_role_id = spousID;
			g_dbSession.Send(&send2, send2.dw_size);
		}
		//离婚后，不需要删除结婚礼包赠送的物品
	}
	pSrcRole->SendMessage(&send, send.dw_size);
	return;
}
void social_mgr::get_qbjjreward(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_get_qbjj_reward* p_receive = (NET_SIC_get_qbjj_reward*)p_message;

	DWORD	dwSrcRoleID = dw_sender;
	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	if( !VALID_POINT(pSrcRole) )
		return;
	DWORD spousID = pSrcRole->GetSpouseID();
	if (!VALID_POINT(spousID))//无配偶直接退出
		return;
	//男女双方必须在同一张地图中
	Map *pMap = pSrcRole->get_map();
	if(!VALID_POINT(pMap))
		return;
	Role* pSpouseRole = pMap->find_role(spousID);
	if (!VALID_POINT(pSpouseRole))
		return;
	DWORD dw_error_code = E_QBJJ_RED_NO_REWARD;//默认没有要领取的奖励

	if (0 == pSrcRole->GetSex())//若是女方
	{
		if (1 != pSrcRole->GetScriptData(REDZUI_FLAG_INDEX))//必须是红唇女生
		{
			dw_error_code = E_QBJJ_RED_NO;//非红唇认证
			goto Exit;
		}
		if (pSrcRole->get_level() < QBJJ_REWARD_LEVEL_MIN)
		{
			dw_error_code = E_QBJJ_LEVEL_Min_Woman;//女方等级不足
			goto Exit;
		}
		INT leveltemp = QBJJ_REWARD_LEVEL_MIN;
		if (0 == pSrcRole->GetScriptData(QBJJ_MARRAY_INDEX_WOMAN))//若女神还未领取过奖励
		{
			//do nothing
		}
		else//若女神已经领取过奖励
		{
			leveltemp = pSrcRole->GetScriptData(QBJJ_MARRAY_INDEX_WOMAN);
			leveltemp += 5;//下一个可领奖励的等级
		}
		INT levelTop = pSrcRole->get_level();//当前玩家可以领取最高的等级
		while(leveltemp <= levelTop)
		{
			/*if (leveltemp < QBJJ_REWARD_LEVEL_MIN)
				continue;*/
			DWORD rewardid = getqbjjreward_bylevel(leveltemp);
			if ((INVALID_VALUE) == rewardid)
				continue;
			pSrcRole->addRewardItem(rewardid,1,RF_SHANCI);
			pSrcRole->SetScriptData(QBJJ_MARRAY_INDEX_WOMAN,leveltemp);
			leveltemp += 5;//5级一档
			dw_error_code = E_Success;
		}
	}
	else//男方
	{
		if (1 != pSpouseRole->GetScriptData(REDZUI_FLAG_INDEX))//必须是红唇女生
		{
			dw_error_code = E_QBJJ_RED_NO;//非红唇认证
			goto Exit;
		}
		if (pSpouseRole->get_level() < QBJJ_REWARD_LEVEL_MIN)
		{
			dw_error_code = E_QBJJ_LEVEL_Min_Woman;//女方等级不足
			goto Exit;
		}
		if (pSrcRole->get_level() < QBJJ_REWARD_LEVEL_MIN)
		{
			dw_error_code = E_QBJJ_LEVEL_MIN_Man;//男方等级不足
			goto Exit;
		}
		INT level_woman = pSpouseRole->GetScriptData(QBJJ_MARRAY_INDEX_WOMAN);//女方已领取奖励的最高等级
		INT	level_man = pSpouseRole->GetScriptData(QBJJ_MARRAY_INDEX_MAN);//挂在女方身上的男人已领取奖励的最高等级
		if (0 != level_woman)//若女神领取过奖励
		{
			if (0 == level_man)//男人还没领取过奖励
			{
				level_man = QBJJ_REWARD_LEVEL_MIN-5;
			}
			INT leveltemp = level_man;
			leveltemp += 5;//得到下一等级的领奖等级

			INT levelTop = min(level_woman,pSrcRole->get_level());//当前玩家可以领取最高的等级
			while(leveltemp <= levelTop)//以女神的领奖等级为标准
			{
				/*if (leveltemp < QBJJ_REWARD_LEVEL_MIN)
					continue;*/
				DWORD rewardid = getqbjjreward_bylevel(leveltemp,FALSE);
				if ((INVALID_VALUE) == rewardid)
					continue;
				pSrcRole->addRewardItem(rewardid,1,RF_SHANCI);
				pSpouseRole->SetScriptData(QBJJ_MARRAY_INDEX_MAN,leveltemp);//设置挂在女神身上的男人领奖等级
				leveltemp += 5;//5级一档
				dw_error_code = E_Success;
			}
		}
	}
Exit:
	NET_SIS_get_qbjj_reward send;
	send.dw_error_code = dw_error_code;
	pSrcRole->SendMessage(&send, send.dw_size);
	return;
}
//获取每个领奖等级对应的礼包id
DWORD social_mgr::getqbjjreward_bylevel(INT level,BOOL bRedZui /* = TRUE */)
{
	//对应红唇女神的奖励
	if (bRedZui)
	{
		if (45 == level)
		{
			return 1400096;
		}
		else if (50 == level)
		{
			return 1400097;
		}
		else if (55 == level)
		{
			return 1400098;
		}
		else if (60 == level)
		{
			return 1400099;
		}
		else if (65 == level)
		{
			return 1400100;
		}
		else if (70 == level)
		{
			return 1400101;
		}
		else if (75 == level)
		{
			return 1400102;
		}
		else if (80 == level)
		{
			return 1400103;
		}
		else
		{
			return (DWORD)INVALID_VALUE;
		}
	}
	else//对应男方的奖励
	{
		if (45 == level)
		{
			return 1400104;
		}
		else if (50 == level)
		{
			return 1400105;
		}
		else if (55 == level)
		{
			return 1400106;
		}
		else if (60 == level)
		{
			return 1400107;
		}
		else if (65 == level)
		{
			return 1400108;
		}
		else if (70 == level)
		{
			return 1400109;
		}
		else if (75 == level)
		{
			return 1400110;
		}
		else if (80 == level)
		{
			return 1400111;
		}
		else
		{
			return (DWORD)INVALID_VALUE;
		}
	}
}

VOID social_mgr::send_login_to_spouse( Role *p_role )
{
	if (!VALID_POINT(p_role))
		return;
	DWORD spouseid = p_role->GetSpouseID();
	if (!VALID_POINT(spouseid))
		return;
	Role* pSpouseRole =	g_roleMgr.get_role(spouseid);
	if(!VALID_POINT(pSpouseRole))
		return;
	NET_SIS_login_to_spouse send;
	pSpouseRole->SendMessage(&send, send.dw_size);
}

VOID social_mgr::send_loginout_to_spouse( Role *p_role )
{
	if (!VALID_POINT(p_role))
		return;
	DWORD spouseid = p_role->GetSpouseID();
	if (!VALID_POINT(spouseid))
		return;
	Role* pSpouseRole =	g_roleMgr.get_role(spouseid);
	if(!VALID_POINT(pSpouseRole))
		return;
	NET_SIS_logout_to_spouse send;
	pSpouseRole->SendMessage(&send, send.dw_size);
}
