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
*	@file		guild_chamber.h
*	@author		lc
*	@date		2011/02/24	initial
*	@version	0.0.1.0
*	@brief		商会
*/


#pragma once

struct tag_chamber_sell_good
{
	DWORD		dw_goods_id;
	INT32		n_cost;
	BYTE		by_free_num;
};

struct tag_chamber_buy_good
{
	DWORD		dw_goods_id;
	INT32		n_cost;
};

class guild_chamber
{
public:
	guild_chamber();
	~guild_chamber();

public:
	static guild_chamber* create_chamber(DWORD dw_npc_id_, DWORD dw_shop_id_);
	static VOID	 delete_chamber(guild_chamber *&p_chamber_) { SAFE_DELETE(p_chamber_); }

public:
	BOOL	init(DWORD dw_npc_id_, DWORD dw_shop_id_);
	VOID	update();
	VOID	destroy();

	//! 刷新商货
	VOID	refresh_sell_goods();
	VOID	refresh_buy_goods();

	BYTE	get_goods_num()	{ return map_sell_goods.size(); }
	VOID	get_goods_info(DWORD dw_role_id_, OUT tagCommerceGoodInfo* p_goods_info_, INT& by_num_);

	//！ 卖出商货
	DWORD	sell_goods(INT32 n_price_, DWORD dw_goods_id_, BYTE by_buy_num_, OUT INT32 &n_cost_price_);
	//！获得商货的收购价格
	INT32	get_goods_price(DWORD dw_goods_id_);

	VOID	remove_observer(DWORD dw_role_id_)	{ list_observer.erase(dw_role_id_); }

	DWORD	send_commerce_goods_to_role(Role* p_role_, DWORD dw_goods_id_ = INVALID_VALUE);

	DWORD	get_chamber_id()		{ return dw_shop_id; }
	BYTE	get_occupy_city()	{ return by_occupy_city; }

private:
	BOOL	is_init_ok() const { return b_init_ok; }
	VOID	refresh_goods_price(INT32& n_current_price_, INT32 n_low_price_, INT32 n_height_price_);
	VOID	send_message_to_observer(LPVOID p_message_, DWORD dw_size_);

private:
	typedef package_map<DWORD, tag_chamber_sell_good*>	MAP_SELL_GOODS;
	typedef package_map<DWORD, tag_chamber_buy_good*>	MAP_BUY_GOODS;
	typedef package_list<DWORD>					LIST_OBSERVER;

	BOOL			b_init_ok;

	DWORD			dw_npc_id;
	DWORD			dw_shop_id;
	BYTE			by_occupy_city;

	MAP_SELL_GOODS		map_sell_goods;
	MAP_BUY_GOODS		map_buy_goods;
	LIST_OBSERVER	list_observer;

	tagDWORDTime	dw_refresh_time;
};