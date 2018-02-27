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
*	@file		guild_commodity.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		��������
*/

#ifndef GUILD_COMMODITY
#define GUILD_COMMODITY


#include "../common/ServerDefine/guild_server_define.h"

class Role;

class guild_commodity
{
public:
	guild_commodity();
	~guild_commodity();

	BOOL	init(DWORD dw_role_id_, INT n_level_, const tagTaelInfo* p_tael_info_ = NULL, const s_redound_info* p_redound_info_ = NULL);
	VOID	destroy();

	//! ����������Ϣ
	DWORD	load_commodity_info(s_guild_commerce_info* p_load_info_);

	//! ��ȡ�̻���Ϣ
	DWORD	get_commodity_info(tagCommerceGoodInfo* p_goods_, INT& n_goods_num_, INT32& n_tael_, INT32& n_level_);
	
	const tagTaelInfo* get_tael_info()		{ return &st_tael_info; }
	const s_redound_info* get_redound_info()	{ return &st_redound_info; }
	INT   get_level()						{ return n_level; }
	INT32 get_tael()							{ return n_current_tael; }
	INT	  get_goods_num()						{ return map_goods.size(); }
	INT   get_tael_progress()					{ return (INT)(((FLOAT)n_current_tael / (FLOAT)st_tael_info.nPurposeTael) * 100.0f); }
	INT32 get_gain();

	DWORD is_full(DWORD dw_goods_id_, BYTE by_num_);
	DWORD is_exist(DWORD dw_goods_id_, BYTE by_num_);

	BOOL	increase_tael(INT32 n_tael_);
	BOOL	decrease_tael(INT32 n_tael_);

	BOOL	add_goods(DWORD dw_goods_id_, BYTE by_num_, INT32 n_price_ = 0);
	BOOL	remove_goods(DWORD dw_goods_id_, BYTE by_num_, INT32 n_price_ = 0);

	VOID	dead_penalty();

private:
	
	VOID	save_tael_to_db();
	VOID	save_commodity_to_db();

private:
	typedef package_map<DWORD, tagCommerceGoodInfo*> MAP_GOODS;

	DWORD					dw_owner_id;			// !�̻�������
	INT						n_level;				// !������������ʱ�ĵȼ�
	tagTaelInfo				st_tael_info;			// !���̳�ʼ��Ϣ
	s_redound_info			st_redound_info;		// !���̽���

	INT32					n_current_tael;			// !��������
	MAP_GOODS				map_goods;				// !�̻���Ϣmap
};
#endif