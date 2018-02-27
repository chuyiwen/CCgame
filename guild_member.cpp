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
*	@file		guild_member.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		
*/

#include "StdAfx.h"

#include "guild_member.h"
#include "role.h"
#include "role_mgr.h"
#include "db_session.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../common/ServerDefine/guild_server_define.h"

guild_member::guild_member()
{
}

guild_member::~guild_member()
{
	tagGuildMember *p_member;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		SAFE_DELETE(p_member);
	}

	map_member.clear();
}

DWORD	guild_member::get_in_war_number()
{
	DWORD dwCount = 0;
	tagGuildMember *p_member;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		if (p_member->bWar)
		{
			dwCount++;
		}
	}
	return dwCount;
}
VOID	guild_member::reset_in_war()
{
	DWORD dwCount = 0;
	tagGuildMember *p_member;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		p_member->bWar = false;
	}
}
//-------------------------------------------------------------------------------
//! 添加帮会成员
//-------------------------------------------------------------------------------
VOID guild_member::add_member(DWORD dw_invitee_id_, EGuildMemberPos e_position_, BOOL b_save_db_/* = TRUE*/)
{
	ASSERT(!map_member.is_exist(dw_invitee_id_));
	
	tagGuildMember *p_new_member = new tagGuildMember(dw_invitee_id_, e_position_);
	if(!VALID_POINT(p_new_member))
	{
		ASSERT(0);
		return;
	}

	map_member.add(dw_invitee_id_, p_new_member);

	if(b_save_db_)
	{
		NET_DB2C_join_guild send;
		send.guild_member_info.dw_role_id		= dw_invitee_id_;
		send.guild_member_info.dw_guild_id	= dw_guild_id;
		send.guild_member_info.n8_guild_pos	= e_position_;

		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------
//! 载入帮会成员
//-------------------------------------------------------------------------------
VOID guild_member::load_member(const tagGuildMember& st_guild_member_)
{
	if (map_member.is_exist(st_guild_member_.dw_role_id))
	{
		return;
	}

	tagGuildMember *p_new_member = new tagGuildMember(st_guild_member_);
	if(!VALID_POINT(p_new_member))
	{
		ASSERT(0);
		return;
	}

	map_member.add(st_guild_member_.dw_role_id, p_new_member);
}

//-------------------------------------------------------------------------------
//! 删除帮会成员
//-------------------------------------------------------------------------------
VOID guild_member::remove_member(DWORD dw_role_id_, BOOL b_save_db_/* = TRUE*/)
{
	tagGuildMember *p_member = map_member.find(dw_role_id_);
	if(!VALID_POINT(p_member))
	{
		return;
	}

	map_member.erase(dw_role_id_);

	if(b_save_db_)
	{
		NET_DB2C_leave_guild send;
		send.dw_role_id	= dw_role_id_;

		g_dbSession.Send(&send, send.dw_size);
	}

	SAFE_DELETE(p_member);
}

//-------------------------------------------------------------------------------
//! 设置帮会成员职位
//-------------------------------------------------------------------------------
VOID guild_member::set_guild_position(tagGuildMember *p_member_, EGuildMemberPos e_position_, BOOL b_save_db_/* = TRUE*/)
{
	ASSERT(VALID_POINT(p_member_));
	ASSERT(::IsGuildPosValid(e_position_));

	p_member_->eGuildPos = e_position_;

	if(b_save_db_)
	{
		NET_DB2C_change_guild_pos send;
		send.dw_role_id	= p_member_->dw_role_id;
		send.n_new_pos	= e_position_;

		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------
//! 帮会内广播
//-------------------------------------------------------------------------------
VOID guild_member::send_guild_message(LPVOID p_message_, DWORD dw_size_)
{
	tagGuildMember* p_member = NULL;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		Role *pRole = g_roleMgr.get_role(p_member->dw_role_id);
		if(!VALID_POINT(pRole))
		{
			continue;
		}

		pRole->SendMessage(p_message_, dw_size_);
	}
}

//-------------------------------------------------------------------------------
//! 发送所有帮会成员信息
//-------------------------------------------------------------------------------
VOID guild_member::send_all_members_to_client(Role *p_role_)
{
	INT16 n16_num = map_member.size();	
	ASSERT(n16_num > 0);

	INT32 n_message_size = sizeof(NET_SIS_guild_get_all_member) - sizeof(BYTE) + n16_num * sizeof(tagGuildMemInfo);
	CREATE_MSG(p_send, n_message_size, NET_SIS_guild_get_all_member);

	p_send->n16Num = 0;
	M_trans_pointer(p_member_info, p_send->byData, tagGuildMemInfo);
	
	tagGuildMember* p_member = NULL;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		//if(p_member->dw_role_id == p_role_->GetID())
		//{
		//	continue;
		//}

		p_member_info[p_send->n16Num].dw_role_id		= p_member->dw_role_id;
		p_member_info[p_send->n16Num].n8GuildPos	= p_member->eGuildPos;
		p_member_info[p_send->n16Num].nCurContrib	= p_member->nContribution;
		p_member_info[p_send->n16Num].nDKP			= p_member->nDKP;
		p_member_info[p_send->n16Num].dwJoinTime   = p_member->dwJoinTime;
		p_member_info[p_send->n16Num].dwMapID = INVALID_VALUE;
		p_member_info[p_send->n16Num].bInWar		= p_member->bWar;
		p_member_info[p_send->n16Num].nTotalFund	= p_member->nTotalFund;

		Role* pRole = g_roleMgr.get_role(p_member->dw_role_id);
		if(VALID_POINT(pRole))
		{
			p_member_info[p_send->n16Num].dwMapID = pRole->GetMapID();
			p_member_info[p_send->n16Num].n32_Rating = pRole->GetEquipTeamInfo().n32_Rating;
		}
		
		s_role_info *p_role_info = g_roleMgr.get_role_info(p_member->dw_role_id);
		if(VALID_POINT(p_role_info))
		{
			p_member_info[p_send->n16Num].byLevel			= p_role_info->by_level;
			p_member_info[p_send->n16Num].bySex				= p_role_info->by_sex_;
			p_member_info[p_send->n16Num].byClass			= p_role_info->e_class_type_;		
			p_member_info[p_send->n16Num].dwTimeLastLogout	= p_role_info->dw_time_last_logout_;
			p_member_info[p_send->n16Num].bOnline			= p_role_info->b_online_;
		}
		
		ASSERT(VALID_POINT(p_role_info));

		++p_send->n16Num;
	}

	if(n16_num != p_send->n16Num)
	{
		ASSERT(n16_num == p_send->n16Num);
	}

	p_role_->SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}

//-------------------------------------------------------------------------------------------------------
//! 设置帮会仓库操作权限
//-------------------------------------------------------------------------------------------------------
VOID guild_member::set_guild_ware_use_privilege( DWORD dw_role_id_, BOOL b_can_use_, BOOL b_save_db_ /*= TRUE*/ )
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	if (b_save_db_ && (p_member->bUseGuildWare != b_can_use_))
	{
		p_member->bUseGuildWare = b_can_use_;

		// 通知数据库
		NET_DB2C_guild_ware_pri send;
		send.dw_role_id	= dw_role_id_;
		send.b_enable	= b_can_use_;

		g_dbSession.Send(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
//! 增加帮会贡献
//-------------------------------------------------------------------------------------------------------
VOID guild_member::increase_member_contribution( DWORD dw_role_id_, INT32 n_contribution_, BOOL b_save_db_ /*= TRUE*/ )
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	if (n_contribution_ <= 0)
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	p_member->nContribution += n_contribution_;
	if (p_member->nContribution > MAX_GUILDMEM_CURCONTRIB)
	{
		p_member->nContribution = MAX_GUILDMEM_CURCONTRIB;
	}
	p_member->nTotalContribution += n_contribution_;
	if (p_member->nTotalContribution > MAX_GUILDMEM_TOTALCONTRIB)
	{
		p_member->nTotalContribution = MAX_GUILDMEM_TOTALCONTRIB;
	}
	
	if (b_save_db_)
	{
		NET_DB2C_change_contrib send;
		send.dw_role_id			= dw_role_id_;
		send.n_contribution		= p_member->nContribution;
		send.n_total_contribution	= p_member->nTotalContribution;
		send.n_total_fund		= p_member->nTotalFund;

		g_dbSession.Send(&send, send.dw_size);
	}

	Role* p_role = g_roleMgr.get_role(dw_role_id_);
	if (VALID_POINT(p_role))
	{
		p_role->GetAchievementMgr().UpdateAchievementCriteria(eta_increase_contribution, n_contribution_);

		NET_SIS_guild_contribution send;
		send.dw_role_id			= dw_role_id_;
		send.nContribution		= p_member->nContribution;
		send.nTotalContribution	= p_member->nTotalContribution;
		p_role->SendMessage(&send, send.dw_size);
	}
}

VOID guild_member::decrease_member_contribution( DWORD dw_role_id_, INT32 n_contribution_, BOOL b_save_db_ /*= TRUE*/ )
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	if (n_contribution_ <= 0)
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	p_member->nContribution -= n_contribution_;
	if (p_member->nContribution < 0)
	{
		p_member->nContribution = 0;
	}

	if (b_save_db_)
	{
		NET_DB2C_change_contrib send;
		send.dw_role_id			= dw_role_id_;
		send.n_contribution		= p_member->nContribution;
		send.n_total_contribution	= p_member->nTotalContribution;
		send.n_total_fund		= p_member->nTotalFund;

		g_dbSession.Send(&send, send.dw_size);
	}

	Role* p_role = g_roleMgr.get_role(dw_role_id_);
	if (VALID_POINT(p_role))
	{
		p_role->GetAchievementMgr().UpdateAchievementCriteria(eta_decrease_contribution, n_contribution_);

		NET_SIS_guild_contribution send;
		send.dw_role_id			= dw_role_id_;
		send.nContribution		= p_member->nContribution;
		send.nTotalContribution	= p_member->nTotalContribution;
		p_role->SendMessage(&send, send.dw_size);
	}
}

VOID guild_member::increase_member_total_fund(DWORD dw_role_id_, INT32 nValue, BOOL b_save_db_)
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	if (nValue <= 0)
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	p_member->nTotalFund += nValue;

	if (b_save_db_)
	{
		NET_DB2C_change_contrib send;
		send.dw_role_id			= dw_role_id_;
		send.n_contribution		= p_member->nContribution;
		send.n_total_contribution	= p_member->nTotalContribution;
		send.n_total_fund		= p_member->nTotalFund;

		g_dbSession.Send(&send, send.dw_size);
	}

	//Role* p_role = g_roleMgr.get_role(dw_role_id_);
	//if (VALID_POINT(p_role))
	//{
	//	p_role->GetAchievementMgr().UpdateAchievementCriteria(eta_increase_contribution, n_contribution_);

	//	NET_SIS_guild_contribution send;
	//	send.dw_role_id			= dw_role_id_;
	//	send.nContribution		= p_member->nContribution;
	//	send.nTotalContribution	= p_member->nTotalContribution;
	//	p_role->SendMessage(&send, send.dw_size);
	//}
}

VOID guild_member::set_member_exploit(DWORD dw_role_id_, INT32 n_exploit_, BOOL b_save_db_ /* = TRUE */)
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;
	
	p_member->nExploit += n_exploit_;
		
	if (p_member->nExploit > MAX_GUILDMEM_EXPLOIT)
		p_member->nExploit = MAX_GUILDMEM_EXPLOIT;
	if (p_member->nExploit < 0)
		p_member->nExploit = 0;

	if (b_save_db_)
	{
		NET_DB2C_change_exmloit send;
		send.dw_role_id	= dw_role_id_;
		send.n_exploit	= n_exploit_;

		g_dbSession.Send(&send, send.dw_size);
	}

	Role* p_role = g_roleMgr.get_role(dw_role_id_);
	if (VALID_POINT(p_role))
	{
		NET_SIS_guild_exploit send;
		send.dw_role_id			= dw_role_id_;
		send.nExploit		= p_member->nExploit;
		p_role->SendMessage(&send, send.dw_size);
	}
}

VOID guild_member::set_guild_ballot(DWORD dw_role_id_, BOOL b_ballot_,	BOOL b_save_db_ /*= TRUE*/)
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	p_member->bBallot = b_ballot_;

	if(b_save_db_)
	{
		NET_DB2C_change_ballot send;
		send.dw_role_id = dw_role_id_;
		send.b_ballot = b_ballot_;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID guild_member::set_guild_war(DWORD dw_role_id_, BOOL bWar, BOOL b_save_db_)
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	tagGuildMember* p_member = map_member.find(dw_role_id_);
	if (!VALID_POINT(p_member))
		return;

	if (p_member->bWar == bWar)
		return;

	p_member->bWar = bWar;
	
	//if(b_save_db_)
	//{
	//	NET_DB2C_change_can_war send;
	//	send.dw_role_id = dw_role_id_;
	//	send.b_war = bWar;
	//	g_dbSession.Send(&send, send.dw_size);
	//}

}

VOID guild_member::set_member_dkp(DWORD dw_role_id_, INT32 n_dkp_, BOOL b_save_db_/* = TRUE*/)
{
	if (!VALID_VALUE(dw_role_id_))
		return;

	tagGuildMember* pMember = map_member.find(dw_role_id_);
	if (!VALID_POINT(pMember))
		return;
	
	pMember->nDKP += n_dkp_;

	if(pMember->nDKP > MAX_MEMBER_DKP)
	{
		pMember->nDKP = MAX_MEMBER_DKP;
	}

	if(pMember->nDKP < 0)
		pMember->nDKP = 0;

	if(b_save_db_)
	{
		NET_DB2C_change_dkp send;
		send.dw_role_id = dw_role_id_;
		send.n_dkp = pMember->nDKP;
		g_dbSession.Send(&send, send.dw_size);
	}
}

INT32	guild_member::get_online_num()
{
	DWORD dwCount = 0;
	tagGuildMember *p_member;
	MAP_GUILD_MEMBER::map_iter iter = map_member.begin();
	while(map_member.find_next(iter, p_member))
	{
		Role* pRole = g_roleMgr.get_role(p_member->dw_role_id);
		if (VALID_POINT(pRole))
		{
			dwCount++;
		}
	}
	return dwCount;
}
