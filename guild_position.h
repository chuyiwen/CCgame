

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
*	@file		guild_position.h
*	@author		lc
*	@date		2010/10/14	initial
*	@version	0.0.1.0
*	@brief		帮会职位管理
*/

#ifndef GUILD_POSITION
#define GUILD_POSITION

#include "../common/ServerDefine/guild_server_define.h"
//-------------------------------------------------------------------------------
class guild_position
{
public:
	guild_position();
	~guild_position();

	VOID	init(BYTE by_guild_level_);

	//BOOL	is_member_full() const;
	BOOL	is_position_full(EGuildMemberPos e_guild_position_);
	DWORD	add_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_);
	VOID	remove_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_);
	DWORD	change_position(DWORD dw_role_id_, EGuildMemberPos e_old_guild_position_, EGuildMemberPos e_new_guild_position_);
	INT32	get_number() const;
	VOID    init_position_name(s_guild *p_att_);
	VOID    init_position_power(s_guild *p_att_);

private:
	//BOOL	is_bangzhu_full() const;
	//BOOL	is_fubangzhu_full() const;

	//DWORD	add_bangzhu(DWORD dw_role_id_);
	//DWORD	add_fubangzhu(DWORD dw_role_id_);
	//DWORD	add_putong(DWORD dw_role_id_);

	//VOID	remove_bangzhu(DWORD dw_role_id_);
	//VOID	remove_fubangzhu(DWORD dw_role_id_);
	//VOID	remove_putong(DWORD dw_role_id_);
	
	BOOL	is_full(EGuildMemberPos pos) const;
	DWORD	add(EGuildMemberPos pos);
	VOID	remove(EGuildMemberPos pos);

private:

	//INT8		n8_current_bangzhu_num;
	//INT8		n8_current_fubangzhu_num;
	
	INT8		n8_current_num[EGMP_Num];

	INT16		n16_current_member_num;
	//INT16		n16_max_member_num;
};


inline INT32 guild_position::get_number() const
{
	return n16_current_member_num;
}

// inline BOOL guild_position::is_member_full() const
// {
// 	return n16_current_member_num >= n16_max_member_num;
// }
//
//inline BOOL guild_position::is_bangzhu_full() const
//{
//	return n8_current_bangzhu_num >= MAX_GUILDPOS_BANGZHU;
//}
//
//inline BOOL guild_position::is_fubangzhu_full() const
//{
//	return n8_current_fubangzhu_num >= MAX_GUILDPOS_FUBANGZHU;
//}

inline DWORD guild_position::change_position(DWORD dw_role_id_, EGuildMemberPos e_old_guild_position_, EGuildMemberPos e_new_guild_position_)
{
	remove_member(dw_role_id_, e_old_guild_position_);
	return add_member(dw_role_id_, e_new_guild_position_);
}
#endif