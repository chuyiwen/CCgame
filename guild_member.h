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

#ifndef GUILD_MEMBER
#define GUILD_MEMBER

struct	tagGuildMember;
enum	EGuildMemberPos;
//-------------------------------------------------------------------------------
typedef package_map<DWORD, tagGuildMember*>		MAP_GUILD_MEMBER;

class guild_member
{
public:
	guild_member();
	~guild_member();

	VOID	init(DWORD dw_guild_id_);

	VOID	add_member(DWORD dw_invitee_id_, EGuildMemberPos e_position_, BOOL b_save_db_ = TRUE);
	VOID	load_member(const tagGuildMember& st_guild_member_);
	VOID	remove_member(DWORD dw_role_id_, BOOL b_save_db_ = TRUE);

	INT32	get_member_num();
	MAP_GUILD_MEMBER& get_member_map();
	
	VOID	send_guild_message(LPVOID p_message_, DWORD dw_size_);
	VOID	send_all_members_to_client(Role *p_role_);

	tagGuildMember* get_member(DWORD dw_role_id_);

	BOOL	is_exist(DWORD dw_role_id_);
	
	INT32	get_online_num();
public:
	
	VOID	set_guild_position(tagGuildMember *p_member_, EGuildMemberPos e_position_, BOOL b_save_db_ = TRUE);
	VOID	set_guild_ware_use_privilege(DWORD dw_role_id_, BOOL b_can_use_, BOOL b_save_db_ = TRUE);
	VOID	increase_member_contribution(DWORD dw_role_id_, INT32 n_contribution_, BOOL b_save_db_ = TRUE);
	VOID	decrease_member_contribution(DWORD dw_role_id_, INT32 n_contribution_, BOOL b_save_db_ = TRUE);
	VOID	set_member_exploit(DWORD dw_role_id_, INT32 n_exploit_, BOOL b_save_db_ = TRUE);
	VOID	set_guild_ballot(DWORD	dw_role_id_, BOOL b_ballot_,	BOOL b_save_db_ = TRUE);
	VOID	set_guild_war(DWORD dw_role_id_, BOOL bWar, BOOL b_save_db_ = TRUE);
	VOID	set_member_dkp(DWORD dw_role_id_, INT32 n_dkp_, BOOL b_save_db_ = TRUE);
	VOID	increase_member_total_fund(DWORD dw_role_id_, INT32 nValue, BOOL b_save_db_ = TRUE);
	DWORD	get_in_war_number();
	VOID	reset_in_war();
private:
	MAP_GUILD_MEMBER			map_member;

	DWORD						dw_guild_id;
};



inline VOID guild_member::init(DWORD dw_guild_id_)
{
	ASSERT(map_member.empty());

	dw_guild_id = dw_guild_id_;
}

//-------------------------------------------------------------------------------
//! 获取获取成员
//-------------------------------------------------------------------------------
inline tagGuildMember* guild_member::get_member(DWORD dw_role_id_)
{
	return map_member.find(dw_role_id_);
}

//-------------------------------------------------------------------------------
//! 获取帮会成员个数
//-------------------------------------------------------------------------------
inline INT32 guild_member::get_member_num()
{
	return map_member.size();
}

//-------------------------------------------------------------------------------
//! 帮会成员是否存在
//-------------------------------------------------------------------------------
inline BOOL guild_member::is_exist(DWORD dw_role_id_)
{
	return map_member.is_exist(dw_role_id_);
}


inline MAP_GUILD_MEMBER& guild_member::get_member_map()
{
	return map_member;
}
#endif