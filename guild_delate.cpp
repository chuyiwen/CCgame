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
*	@file		guild_delate.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		
*/

#include "StdAfx.h"
#include "guild_delate.h"
#include "guild.h"
#include "db_session.h"
#include "role.h"

#include "../common/ServerDefine/guild_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

guild_delate::guild_delate(void)
{
	b_init			= FALSE;
}

guild_delate::~guild_delate(void)
{
}

VOID guild_delate::init(guild* p_guild_, BOOL b_request_/* = FALSE*/)
{
	if(!VALID_POINT(p_guild_))
		return ;

	p_guild = p_guild_;
	b_init = FALSE;

	if(b_request_)
	{
		NET_DB2C_load_guild_delate send;
		send.dw_guild_id = p_guild->get_guild_att().dwID;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID guild_delate::update()
{
	if(p_guild->is_in_guild_state(EGDSS_Delate))
	{
		if(CalcTimeDiff(g_world.GetWorldTime(), st_guild_delate.dwDelateBeginTime) >= MAX_GUILD_DELATE_TIME)
		{
			st_guild_delate.dwDelateBeginTime = 0;
			st_guild_delate.dwDelateEneTime = g_world.GetWorldTime();

			FLOAT f_num = st_guild_delate.nAgreeNum + st_guild_delate.nOpposeNum;
			if(f_num > 0)
			{
				INT n_pro = (st_guild_delate.nAgreeNum/f_num) * 100;
				if(n_pro > 70)
				{
					st_guild_delate.bSucceed = TRUE;
				}
				else
				{
					st_guild_delate.bSucceed = FALSE;
				}
			}
			else
			{
				st_guild_delate.bSucceed = FALSE;
			}

			p_guild->unset_guild_state(EGDSS_Delate);

			if(st_guild_delate.bSucceed)
			{
				DWORD dw_error_code = p_guild->change_leader(p_guild->get_guild_att().dwLeaderRoleID, st_guild_delate.dwInitiateRoleID);
				if(dw_error_code != E_Success)
					st_guild_delate.bSucceed = FALSE;
			}

			p_guild->set_guild_state(EGDSS_DelateEnd);

			save_guild_delate();
		}
	}

	if(p_guild->is_in_guild_state(EGDSS_DelateEnd))
	{
		if(CalcTimeDiff(g_world.GetWorldTime(), st_guild_delate.dwDelateEneTime) > MAX_GUILD_DELATE_END_TIME)
		{
			st_guild_delate.bSucceed = FALSE;
			st_guild_delate.dwBeDelateRoleID = INVALID_VALUE;
			st_guild_delate.dwDelateBeginTime = 0;
			st_guild_delate.dwDelateEneTime = 0;
			st_guild_delate.dwInitiateRoleID = INVALID_VALUE;
			st_guild_delate.nAgreeNum = 0;
			st_guild_delate.nOpposeNum = 0;

			save_guild_delate();

			p_guild->unset_guild_state(EGDSS_DelateEnd);

			reset_member_ballot();
		}
	}
}

// 创建帮会弹劾数据
VOID guild_delate::create_guild_delate()
{
	st_guild_delate.Init(p_guild->get_guild_att().dwID);


	NET_DB2C_create_guild_delate send;
	send.dw_guild_id = p_guild->get_guild_att().dwID;
	g_dbSession.Send(&send, send.dw_size);

	b_init = TRUE;
}

// 获取帮会弹劾数据
VOID guild_delate::load_guild_delate(s_guild_delate_load* p_guild_delate_load_)
{
	st_guild_delate.dwGuildID = p_guild_delate_load_->dwGuildID;
	st_guild_delate.bSucceed = p_guild_delate_load_->bSucceed;
	st_guild_delate.content = p_guild_delate_load_->sz_content;
	st_guild_delate.dwBeDelateRoleID = p_guild_delate_load_->dwBeDelateRoleID;
	st_guild_delate.dwDelateBeginTime = p_guild_delate_load_->dwDelateBeginTime;
	st_guild_delate.dwDelateEneTime = p_guild_delate_load_->dwDelateEneTime;
	st_guild_delate.dwInitiateRoleID = p_guild_delate_load_->dwInitiateRoleID;
	st_guild_delate.nAgreeNum = p_guild_delate_load_->nAgreeNum;
	st_guild_delate.nOpposeNum = p_guild_delate_load_->nOpposeNum;

	b_init = TRUE;
}

// 移除帮会弹劾数据
VOID guild_delate::remove_guild_delate()
{
	if(!b_init)
		return ;

	NET_DB2C_remove_guild_delate send;
	send.dw_guild_id = p_guild->get_guild_att().dwID;
	g_dbSession.Send(&send, send.dw_size);
}

// 保存帮会弹劾数据
VOID guild_delate::save_guild_delate()
{
	NET_DB2C_update_guild_dalate send;
	send.s_guild_delate_base_.bSucceed = st_guild_delate.bSucceed;
	send.s_guild_delate_base_.dwBeDelateRoleID = st_guild_delate.dwBeDelateRoleID;
	send.s_guild_delate_base_.dwDelateBeginTime = st_guild_delate.dwDelateBeginTime;
	send.s_guild_delate_base_.dwDelateEneTime = st_guild_delate.dwDelateEneTime;
	send.s_guild_delate_base_.dwGuildID = st_guild_delate.dwGuildID;
	send.s_guild_delate_base_.dwInitiateRoleID = st_guild_delate.dwInitiateRoleID;
	send.s_guild_delate_base_.nAgreeNum = st_guild_delate.nAgreeNum;
	send.s_guild_delate_base_.nOpposeNum = st_guild_delate.nOpposeNum;

	g_dbSession.Send(&send, send.dw_size);
}

// 保存帮会弹劾内容数据
VOID guild_delate::save_guild_delate_content()
{
	INT32 n_str_num = st_guild_delate.content.size();
	INT32 n_message_size = sizeof(NET_DB2C_update_guild_delate_content) + n_str_num * sizeof(TCHAR);

	CREATE_MSG(p_send, n_message_size, NET_DB2C_update_guild_delate_content);
	p_send->dw_guild_id = p_guild->get_guild_att().dwID;
	memcpy(p_send->sz_content, st_guild_delate.content.c_str(), n_str_num  * sizeof(TCHAR));
	p_send->sz_content[n_str_num] = _T('\0');

	g_dbSession.Send(p_send, p_send->dw_size);

	MDEL_MSG(pSend);
	
}

// 获取帮会弹劾数据
VOID guild_delate::get_guild_delate_data(Role* p_role_)
{
	if(!VALID_POINT(p_role_))
		return;

	NET_SIS_get_guild_delate_data send;
	send.stGuildDelateBase.dwGuildID = p_guild->get_guild_att().dwID;
	send.stGuildDelateBase.bSucceed = st_guild_delate.bSucceed;
	send.stGuildDelateBase.dwBeDelateRoleID = st_guild_delate.dwBeDelateRoleID;
	send.stGuildDelateBase.dwDelateBeginTime = st_guild_delate.dwDelateBeginTime;
	send.stGuildDelateBase.dwDelateEneTime = st_guild_delate.dwDelateEneTime;
	send.stGuildDelateBase.dwGuildID = st_guild_delate.dwGuildID;
	send.stGuildDelateBase.dwInitiateRoleID = st_guild_delate.dwInitiateRoleID;
	send.stGuildDelateBase.nAgreeNum = st_guild_delate.nAgreeNum;
	send.stGuildDelateBase.nOpposeNum = st_guild_delate.nOpposeNum;

	p_role_->SendMessage(&send, send.dw_size);
}

// 获取帮会弹劾内容
VOID guild_delate::get_guild_delate_content(Role* p_role_)
{
	if(!VALID_POINT(p_role_))
		return;

	INT32 n_num_size = st_guild_delate.content.size();

	if(n_num_size <= 0)
		return;

	INT32 n_message_size = sizeof(NET_SIS_get_guild_delate_content) + n_num_size * sizeof(TCHAR);
	CREATE_MSG(p_send, n_message_size, NET_SIS_get_guild_delate_content);
	memcpy(p_send->szContent, st_guild_delate.content.c_str(), (n_num_size + 1) * sizeof(TCHAR));
	p_send->szContent[n_num_size] = _T('\0');
	p_role_->SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);
}

// 弹劾帮主
DWORD guild_delate::delate_leader(Role* p_role_, INT64 n64_item_id_, LPCTSTR sz_content_, INT32 n_content_num_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	if(p_role_->GetID() == p_guild->get_guild_att().dwLeaderRoleID)
		return E_Guild_Is_Leader;

	if(p_guild->is_in_guild_state(EGDSS_Delate))
		return E_Guild_State_Delate;

	DWORD dw_end_time = CalcTimeDiff(g_world.GetWorldTime(), st_guild_delate.dwDelateEneTime);
	if(dw_end_time > 0)
	{
		if(dw_end_time < MAX_GUILD_DELATE_END_TIME)
			return E_Guild_Delate_Time_Limit;
	}

	tagItem* p_item = p_role_->GetItemMgr().GetBagItem(n64_item_id_);
	if(!VALID_POINT(p_item))
		return E_Guild_Delate_Item_NoExists;

	if(p_item->pProtoType->eSpecFunc != EIST_Delate)
		return E_Guild_Delate_Item_NoExists;

	if(n_content_num_ > MAX_GUILD_DELATE_LEN)
		return E_Filter_Text_TooLong;

	st_guild_delate.content = sz_content_;

	st_guild_delate.dwBeDelateRoleID = p_guild->get_guild_att().dwLeaderRoleID;
	st_guild_delate.dwInitiateRoleID = p_role_->GetID();
	st_guild_delate.dwDelateBeginTime = g_world.GetWorldTime();
	
	save_guild_delate();

	save_guild_delate_content();

	p_guild->set_guild_state(EGDSS_Delate);

	p_role_->GetItemMgr().DelFromBag(n64_item_id_, (DWORD)elcid_guild_delate, 1);

	return E_Success;
}

// 弹劾投票
DWORD guild_delate::delate_ballot(Role* p_role_, BOOL b_agree_)
{
	if(!VALID_POINT(p_role_))
		return INVALID_VALUE;

	tagGuildMember* p_guild_member = p_guild->get_member(p_role_->GetID());
	if(!VALID_POINT(p_guild_member))
		return INVALID_VALUE;

	if(!p_guild->is_in_guild_state(EGDSS_Delate))
		return E_Guild_State_NoDelate;

	if(p_role_->GetID() == p_guild->get_guild_att().dwLeaderRoleID)
		return E_Guild_Is_Leader;

	if(p_guild_member->bBallot)
		return E_Guild_Have_Ballot;

	p_guild_member->bBallot = TRUE;
	p_guild->change_member_ballot(p_role_->GetID(), p_guild_member->bBallot);

	if(b_agree_)
	{
		st_guild_delate.nAgreeNum++;
	}
	else
	{
		st_guild_delate.nOpposeNum++;
	}
	save_guild_delate();

	return E_Success;
}

// 重置帮派成员投票权限
VOID guild_delate::reset_member_ballot()
{
	MAP_GUILD_MEMBER::map_iter iter = p_guild->get_guild_member_map().begin();
	tagGuildMember* p_guild_member = NULL;
	while(p_guild->get_guild_member_map().find_next(iter, p_guild_member))
	{
		if(!VALID_POINT(p_guild_member))
			continue;

		if(!p_guild_member->bBallot)
			continue;

		p_guild_member->bBallot = FALSE;

		p_guild->change_member_ballot(p_guild_member->dw_role_id, p_guild_member->bBallot = FALSE);
	}
	
}
