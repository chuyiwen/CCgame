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
*	@file		guild_commerce.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		�������
*/


#ifndef GUILD_COMMERCE
#define GUILD_COMMERCE

class guild;
class guild_commodity;

struct s_guild_commerce_info;

class rank_compare
{
public:
	bool operator()(const tagCommerceRank* lhs, const tagCommerceRank* rhs)
	{
		return lhs->nTael > rhs->nTael;
	}
};

class guild_commerce
{
public:
	guild_commerce();
	~guild_commerce();

	BOOL	init(guild* p_guild_, BOOL b_request_ = FALSE);
	VOID	destroy();

	BOOL	is_init_ok()		{ return b_init_ok; }

	//! ������������Ϣ
	DWORD	load_commerce_info(s_guild_commerce_info* p_info_, INT n_info_num_);

	//! ����������������Ϣ
	DWORD	load_commerce_rank_info(tagCommerceRank* p_info_, INT n_info_num_);

	//! ������������
	DWORD	accept_commerce(Role* p_role_);

	//! �����������
	DWORD	complete_commerce(Role* p_role_, INT32& n_fund_);

	//! ������������
	DWORD	abandon_commerce(DWORD dw_role_id_, BOOL b_clear_rank_ = FALSE);

	//! ����̻���Ϣ
	DWORD	get_commodity_goods_info(Role* p_role_, tagCommerceGoodInfo* p_goods_info_, INT& n_goods_num_, INT32& n_tael_, INT& n_level_);

	//! ������̳�ʼ��Ϣ
	DWORD	get_commerce_init_info(DWORD dw_role_id_, INT& n_level_, tagTaelInfo& st_tael_info_);

	//! �����̻�
	DWORD	buy_goods(Role* p_role_, DWORD dw_npc_id_, DWORD dw_goods_id_, BYTE by_buy_num_);

	//! �����̻�
	DWORD	sell_goods(Role* p_role_, DWORD dw_npc_id_, DWORD dw_goods_id_, BYTE by_sell_num_);

	//! ��ȡ�������а�
	DWORD	get_commerce_rank_info(tagCommerceRank* p_rank_info_, INT& n_rank_num_, BOOL& b_commend_);

	//! ��/�� ���̽���
	DWORD	switch_commendation(DWORD dw_role_id_, BOOL b_on_);

	//! ���̽���
	VOID	extend_commendation();

public:
	guild_commodity*	get_commodity(DWORD dw_role_id)	{ return map_commodity.find(dw_role_id); }
	VOID	add_to_commerce_rank(DWORD dw_role_id_, INT32 n_tael_, INT n_times_ = INVALID_VALUE, BOOL b_save_db_ = TRUE);
	VOID	remove_commerce_rank(DWORD dw_role_id_);

private:
	typedef package_map<DWORD, guild_commodity*>		MAP_COMMODITY;
	typedef std::vector<tagCommerceRank*>		VECTOR_RANK;

	BOOL				b_init_ok;
	guild*				p_guild;

	BOOL				b_commend;

	MAP_COMMODITY		map_commodity;
	VECTOR_RANK			vector_rank;
};
#endif