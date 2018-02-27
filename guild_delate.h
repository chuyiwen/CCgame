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


#pragma once

#include "../../common/WorldDefine/guild_define.h"
#include "../common/ServerDefine/guild_server_define.h"

class guild;
class Role;

class guild_delate 
{
public:
	guild_delate(void);
	~guild_delate(void);

public:

	VOID init(guild* p_guild_, BOOL b_request_ = FALSE);

	BOOL	is_init_ok()	{ return b_init; }

	VOID	update();

	VOID create_guild_delate();

	VOID load_guild_delate(s_guild_delate_load* p_guild_delate_load_);

	VOID remove_guild_delate();

	VOID save_guild_delate();

	VOID save_guild_delate_content();

	VOID get_guild_delate_data(Role* p_role_);

	VOID get_guild_delate_content(Role* p_role_);

	DWORD delate_leader(Role* p_role_, INT64 n64_item_id_, LPCTSTR sz_content_, INT32 n_content_num_);

	DWORD delate_ballot(Role* p_role_, BOOL b_agree_);

	const tagGuildDelate& get_guild_delate() { return st_guild_delate; }

	VOID reset_member_ballot();

private:

	guild*			p_guild;

	BOOL			b_init;

	tagGuildDelate		st_guild_delate;
};
