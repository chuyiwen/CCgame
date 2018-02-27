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
*	@file		guild_warehouse.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/

#ifndef GUILD_WAREHOUSE
#define GUILD_WAREHOUSE

#include "event_mgr.h"
#include "container.h"

struct	tagGuildBase;
class	guild;
//-----------------------------------------------------------------------------

class GuildWarehouse : public EventMgr<GuildWarehouse>
{
public:
	GuildWarehouse();
	~GuildWarehouse();

	VOID	init(guild* p_guild_, BOOL b_request_ = FALSE);
	VOID	update();

	BOOL	is_init_ok()	{ return b_init_ok; }

	//! 帮会升级时重新设置仓库大小
	VOID	reset_init();

	//! 初始化时载入帮会仓库物品
	VOID	load_warehouse_items(const BYTE* p_data_, INT n_item_num_);
	//! 帮会解散时删除所有物品
	VOID	remove_all_items();

	VOID	on_move_to(DWORD dw_sender_id_, VOID* p_event_message_);
	VOID	on_move_to_other(DWORD dw_sender_id_, VOID* p_event_message_);

	DWORD	get_guild_ware_info(BYTE* p_buffer_, INT& n_item_num_, DWORD& dw_last_update_time_, INT16& n_ware_size_, INT32& n_size_);
	DWORD	send_guild_ware_power_list_to_client(Role* p_role_);

	DWORD	add_item(tagItem *&p_item_, DWORD dw_command_id_, BOOL b_insert_to_db_ = TRUE);

private:
	
	DWORD move_allowable(Role* p_role_);
	DWORD move(Role* p_role_, INT64 n64_serial_, INT16 n16_num_, INT16 n16_dest_position_, DWORD dw_command_id_);
	DWORD move_to_other(Role* p_role_, EItemConType e_container_type_src_, INT64 n64_serial_, 
		EItemConType e_container_type_dest_, INT16 n16_position_dest_, DWORD dw_command_id_);

private:
	
	VOID insert_item_to_db(tagItem &st_item_);
	VOID delete_item_from_db(INT64 n64_serial_, INT32 dw_data_id_);
	VOID save_update_item_to_db();

private:
	
	VOID send_message(Role* p_role_, LPVOID p_message_, DWORD dw_size_);

private:
	
	BOOL is_item_need_log(const tagItem &st_item_) const { return st_item_.pProtoType->bNeedLog && st_item_.pProtoType->byLogMinQlty <= ItemHelper::GetQuality(&st_item_); }
	__forceinline VOID log_item(DWORD dw_role_id_, const tagItem &st_item1_, const tagItem *st_item2_, INT16 n16_num_, DWORD dw_command_id_);

private:
	static	VOID register_guild_ware_event();

private:
	
	VOID set_update_time(INT16 n_position_);				

private:
	BOOL				b_init_ok;
	guild*				p_guild;

	ItemContainer*		p_container;			

	DWORD				dw_last_save_time;		
	DWORD				dw_guild_ware_time;		
	std::vector<DWORD>	vector_update_time;		
};
#endif