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
*	@file		guild_affair.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		
*/


#ifndef GUILD_AFFAIR
#define GUILD_AFFAIR

class guild;

class guild_affair
{
public:
	guild_affair();
	~guild_affair();

	VOID	init(guild* p_guild_);
	BOOL	is_init_ok()	{ return b_init; }

	//! 发布帮会事务
	DWORD	issuance_guild_affair(Role* p_role_, DWORD dw_buffer_id_);

	//! 重置帮会事务发布次数
	VOID	reset_affair_count();

private:
	BOOL			b_init;
	guild*			p_guild;

	//! 事务发布次数
	BYTE			by_count;		
};

#endif