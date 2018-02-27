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
*	@file		guild_buy.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		帮会团购
*/

#pragma once

struct tagGroupPurchase;
struct tagSimGPInfo;
struct tagMallItemSell;
struct tagItem;
class guild;

class guild_buy
{
public:
	guild_buy();
	~guild_buy();

	BOOL init(DWORD dw_guild_id_);
	VOID update(DWORD dw_time_);
	VOID destroy();

	BOOL	add(tagGroupPurchase* p_guild_buy_info_, BOOL b_notify_db_ = TRUE);
	BOOL	remove(tagGroupPurchase* p_guild_buy_info_, BOOL b_notify_db_ = TRUE);
	BOOL	is_empty()	{ return map_guild_buy_info.empty(); }

	//! 获取帮会团购数据
	DWORD	get_all_guild_buy_info(INT &n_guild_buy_info_num_, tagSimGPInfo* p_data_);
	DWORD	get_response(DWORD dw_id_, DWORD dw_role_id_, DWORD *p_data_);

	INT		get_guild_buy_info_num();
	INT		get_response_num(DWORD dw_id_, DWORD dw_role_id_);

	//！ 发起/响应帮会团购
	DWORD	lauch_guild_buy(Role *p_role_, DWORD dw_id_, BYTE by_scope_,
		BYTE by_index_, INT n_unit_price_, OUT tagGroupPurchase* &p_guild_buy_info_, OUT DWORD& dw_item_type_id_);
	DWORD	respond_guild_buy(Role *p_role_, DWORD dw_id_, DWORD dw_role_id_,
		INT n_price_, OUT tagGroupPurchase* &p_guild_buy_info_);

	//！ 创建帮会团购物品
	DWORD	create_guild_buy_items(DWORD dw_id_, OUT tagMallItemSell &st_item_sell_);

	//！ 删除帮会团购数据
	VOID	remove_guild_buy_info(tagGroupPurchase* &p_guild_buy_info_, BOOL b_success_ = TRUE);
	VOID	remove_guild_buy_info();

	//！ 退还玩家费用
	VOID	return_cost_to_role(tagGroupPurchase* p_guild_buy_info_);

	//！ 更新帮会相应列表
	VOID	update_respond_info_to_db(tagGroupPurchase* p_guild_buy_info_);

	//！ 帮会团购成功
	DWORD	guild_buy_success(tagGroupPurchase* p_guild_buy_info_);

private:
	
	VOID add_guild_buy_info_to_db(tagGroupPurchase* p_guild_buy_info_);
	VOID remove_guild_buy_info_to_db(tagGroupPurchase* p_guild_buy_info_);
	VOID remove_guild_buy_info_to_db();

private:
	
	INT64	get_key(DWORD dw_role_id_, DWORD dw_id_);
	
private:
	typedef package_map<INT64, tagGroupPurchase*> MAP_GUILD_BUY_INFO;
	MAP_GUILD_BUY_INFO				map_guild_buy_info;

	guild*					p_guild;
};