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

#include "stdafx.h"
#include "../common/ServerDefine/log_server_define.h"
#include "channel_mgr.h"
#include "chat_mgr.h"
#include "role.h"
#include "ps_bomb.h"
#include "role_mgr.h"
#include "map.h"
#include "guild.h"
#include "guild_manager.h"

channel_mgr g_channelMgr;

void ugly_set_msg_gm_flag(tag_net_message* p, BYTE b)
{
	static NET_SIS_role_char nsrc;
	static NET_SIS_role_show_item nsrsi;
	static NET_SIS_role_show_equip nsrse;

	if(nsrc.dw_message_id == p->dw_message_id)
		static_cast<NET_SIS_role_char*>(p)->bGM = b;
	else if (nsrsi.dw_message_id == p->dw_message_id)
		static_cast<NET_SIS_role_show_item*>(p)->bGM = b;
	else if (nsrse.dw_message_id == p->dw_message_id)
		static_cast<NET_SIS_role_show_equip*>(p)->bGM = b;
}


DWORD channel_mgr::send_packet(msg_packet* p_cmd)
{
	M_trans_else_ret(p_sender, p_cmd->get_sender(), Role, E_SystemError);
	M_trans_else_ret(p_map, p_sender->get_map(), Map, E_SystemError);
	M_trans_else_ret(p_send, p_cmd->get_send(), tag_net_message, E_SystemError);
	M_trans_else_ret(p_receive, p_cmd->get_recv(), tag_net_message, E_SystemError);
	ESendChatChannel e_channel = p_cmd->get_channel();

	// 包长限制
	if ( p_cmd->get_size() > get_size_limit(p_send, e_channel) )
		return E_RoleChat_TooLength;

	switch( e_channel )
	{
		// 普通频道
	case ESCC_Common:
		{
			NET_SIC_role_char* pRolcChat = static_cast<NET_SIC_role_char*>(p_receive);
			if(!g_pSGuarder.OnMsg(pRolcChat->szMsg, 0))
				p_map->foreach_unit_in_big_visible_tile(p_sender, send_op(e_channel, p_send));

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_common, 1);

		}
		break;
		// 密语频道
	case ESCC_Secret:
	case ESCC_SiLiao:
		{
			M_trans_else_ret(p_info, g_roleMgr.get_role_info(p_cmd->get_reciver()), s_role_info, E_RoleChat_Secret_NoRoleName);
			M_trans_else_ret(p_dest, g_roleMgr.get_role(p_cmd->get_reciver()), Role, E_RoleChat_Secret_RemoteRoleLeave);
			if(e_channel == ESCC_Secret)ugly_set_msg_gm_flag(p_send, p_sender->GetGMPrivilege( ));
			p_dest->SendMessage(p_send, p_send->dw_size);

			if (p_sender != p_dest)
			{
				if(e_channel == ESCC_Secret) ugly_set_msg_gm_flag(p_send, p_dest->GetGMPrivilege( ));
				p_sender->SendMessage(p_send, p_send->dw_size);	
			}

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_secret, 1);
		}
		break;
		// 世界频道
	case ESCC_World:
		{
			// 判断时间间隔
			if (!p_sender->TalkToWorld())
				return E_RoleChat_World_Frequently;

			// 等级判断 暂不做等级限制
			/*if (p_sender->get_level() < WORLD_CHANNEL_ROLE_LVL)
				return E_RoleChat_World_RoleNo10;*/

			g_roleMgr.for_each(send_op(e_channel, p_send));

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_world, 1);

			if(VALID_POINT(p_sender->GetScript()))
			{
				p_sender->GetScript()->OnChatWorld(p_sender);
			}

		}
		break;
		// 区域频道(同一地图)
	case ESCC_Map:
		{
			// 判断时间间隔
			if (!p_sender->talk_to_map())
				return E_RoleChat_Map_Frequently;

			//gx modify 2013.6.24
			// 等级判断
			/*if (p_sender->get_level() < WORLD_CHANNEL_ROLE_LVL)
				return E_RoleChat_Map_RoleNo10;*/

			p_map->for_each_role_in_map(send_op(e_channel, p_send));

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_map, 1);
		}
		break;
		//公告频道
	case ESCC_Affiche:
		{
			//判断GM权限
			if (!VALID_POINT(p_sender->GetSession()) || !p_sender->GetSession()->IsPrivilegeEnough(1))
				return E_RoleChat_Not_GM;

			g_roleMgr.for_each(send_op(e_channel, p_send));
		}
		break;
		// 队伍频道
	case ESCC_Team:
		{
			//给队伍所有成员发送消息
			DWORD dwTeamID = p_sender->GetTeamID();
			const Team* pTeam = g_groupMgr.GetTeamPtr(dwTeamID);

			if (!VALID_POINT(pTeam))return  E_RoleChat_Team_NoJoin; 

			pTeam->for_each(send_op(e_channel, p_send));

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_team, 1);

		}
		break;
		// 昭告<!!!!!>
	case ESCC_Decree:
	case ESCC_Qianli:
		{
			if(!p_sender->IsGM())
			{
				package_list<tagItem*> list_item;
				if (ESCC_Decree == e_channel)
				{
					INT32 n_num = p_sender->GetItemMgr().GetBagSameItemList(list_item, TYPEID_IM_DECREE, 1);
				}
				else
				{
					INT32 n_num = p_sender->GetItemMgr().GetBagSameItemList(list_item, TYPEID_IM_QIANLI, 1);
				}
				if(!list_item.empty())
				{
					package_list<tagItem*>::list_iter iter = list_item.begin();
					tagItem* pItem = *iter;
					p_sender->GetItemMgr().ItemUsedFromBag(pItem->GetKey(), 1, (DWORD)elcid_item_use);
				}
				else
				{
					return E_RoleChat_Decree_OutOfYuanBao;
				}
			}

			g_roleMgr.for_each(send_op(e_channel, p_send));
			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_decree, 1);
		}
		break;
		// 帮派频道
	case ESCC_Guild:
		{
			M_trans_else_ret(pGuild, g_guild_manager.get_guild(p_sender->GetGuildID()), guild, E_RoleChat_Guild_NoJoin);
			pGuild->for_every_role_in_guild(send_op(e_channel, p_send));

			p_sender->GetAchievementMgr().UpdateAchievementCriteria(eta_use_guild, 1);
		}
		break;
	default: {ASSERT(FALSE);} break;
	}

	return E_Success;
}

DWORD channel_mgr::get_size_limit( tag_net_message* p_cmd, BYTE channel )
{
	static const NET_SIS_role_char			__msg_chat;
	static const NET_SIS_role_show_equip	__msg_equip;
	static const NET_SIS_role_show_item		__msg_item;

	DWORD max_size = 1024;
	if (__msg_chat.dw_message_id == p_cmd->dw_message_id)
	{
		switch(ESendChatChannel(channel))
		{
		case ESCC_Decree:
		case ESCC_Qianli:
			max_size = WORLD_CHANNEL_MSG_LEN;
			break;
		case ESCC_World:
		case ESCC_Map:
			max_size = MAX_CHAT_LEN;
			break;
		case ESCC_Secret:
		case ESCC_SiLiao:
		case ESCC_Guild:
		case ESCC_Team:
		case ESCC_Common:
			max_size = MAX_CHAT_LEN;
			break;
		
		case ESCC_Combat:
		case ESCC_Affiche:
		case ESCC_GM:
		case ESCC_System:
			break;
		default:
			max_size = 0;
			break;
		}
	}
	else if (__msg_equip.dw_message_id == p_cmd->dw_message_id)
		max_size = max_size;
	else if (__msg_item.dw_message_id == p_cmd->dw_message_id)
		max_size = sizeof(tagItem);

	return max_size;
}

