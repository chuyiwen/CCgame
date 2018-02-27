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
*	@file		guild_manager.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/

#ifndef GUILD_MANAGER
#define GUILD_MANAGER

#include "event_mgr.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../common/ServerDefine/guild_server_define.h"

class guild;
class Role;

struct s_guild_load;
struct s_guild_member_load;
//-------------------------------------------------------------------------------
class guild_manager
{
public:
	guild_manager();
	~guild_manager();

	BOOL	init();
	VOID	update();
	VOID	destroy();

	INT32	get_guild_num();
	guild*	get_guild(DWORD dw_guild_id_);
	VOID	get_script_data_six_list(std::map<int, DWORD>& map);

	DWORD	create_guild(Role* p_creator_, LPCTSTR sz_guild_name_, INT32 n_string_length_, INT64 n64ItemID);
	DWORD	create_guild_gm(Role* p_creator_, LPCTSTR sz_guild_name_, INT32 n_string_length_);
	DWORD	dismiss_guild(Role* p_leader_, DWORD dw_guild_id_);

	
	VOID	init_db_guild(const s_guild_load* p_guild_load_);
	VOID	init_db_guild_member(const s_guild_member_load *p_guild_member_load_, const INT32 n_num_);
	BOOL	is_guild_init_ok();

	//! 关闭服务器是保存所有帮会信息
	VOID	send_all_guild_to_db();

	//! 发送连线客户端的帮会信息
	VOID	send_role_guild_info(Role *p_role_, 
							OUT tagGuildBase &st_guild_base_, 
							OUT tagGuildMember &st_guild_member_);
	
	VOID	send_all_guild_info(Role* p_role, BOOL bSBKList);

	//! 客户端请求通用错误处理
	VOID	send_guild_failed_to_client(PlayerSession *p_session_, DWORD dw_error_code_);

	//! 删除角色时清理帮会数据
	DWORD	clear_role_remove(DWORD dw_role_id_);

	//！ 帮会配置
	const tagGuildCfg& get_guild_config() const;

	//! 帮会仓库添加事件
	VOID add_guild_ware_event(DWORD dw_sender_, EEventType e_event_type_, DWORD dw_size_, VOID* p_message_);

	//! 帮会技能升级
	VOID upgrade_guild_skill();

	//! 每天重置帮会状态，扣除消耗
	VOID daily_guild_reset();
	
	//! 没周清除宣战过的帮会
	VOID week_clear_enemy();

	// 清空帮会圣兽使用次数
	VOID clear_guild_holiness_num();

	//! 刷新帮会免战状态
	VOID daily_guild_war_state_reset();

	// 刷新帮会鼎使用次数
	VOID daily_guild_lobby_use();

	VOID reset_member_ballot();

	VOID guild_quest_reset( );

	VOID getSBKData(tagGuildSBKData& data);

public:

	s_guild_pvp_data*	get_pvp_data(INT n_act_id_) { return map_pvp_data.find(n_act_id_); }

	VOID	set_pvp_instance_id(DWORD dw_instance_id_, DWORD dw_act_id_);

	VOID	init_pvp_data(s_load_guild_pvp_data *p_load_guild_pvp_data_, INT n_num_);

	VOID	update_pvp_data(INT n_act_id_);

public:
	VOID	load_guild_recruit(DWORD* p_data, INT n_num);
	BOOL	is_have_recruit(DWORD dw_role_id, DWORD dwGuildID = INVALID_VALUE);
	DWORD	get_recruit_guild(DWORD dw_role_id);
	DWORD	join_guild_recruit(Role* pRole, DWORD dwGuildID);
	DWORD	leave_guild_recruit(Role* pRole);
	VOID	query_guild_recruit(Role* pRole);
	VOID	cal_page_num(INT n_size, INT& n_page);
	VOID	send_query_result(Role* p_role, INT n_page_num,	package_list<DWORD>& list_query_guild_recruid);
	VOID	delete_role_from_guild_recruit(DWORD dw_role_id);
	VOID	clean_guild_recruit_to_db();

public:
	VOID	load_guild_kill_boss(s_guild_kill_boss* p_kill_boss, INT n_num);
	BOOL	is_monster_kill(DWORD	dw_monster_id);
	VOID	add_monster_kill(DWORD dw_monster_id, DWORD dw_guild_id);
	
	VOID	load_guild_war_history(tagGuildWarHistory* p_war_history, INT n_num);
	VOID	load_guild_plant(DWORD dwID, tagPlantData* p_plant_data, INT n_num);

	VOID	set_SBK_gulid(DWORD dwGuildID, BOOL bSave = TRUE); 
	DWORD	get_SBK_guild() { return m_dw_SBK_guild;}

	// 是否在攻城战
	BOOL	is_begin_SBK() { return m_bSBKStart; }
	void	set_begin_SBK(BOOL b) { m_bSBKStart = b; }

	// 传送点是否开启
	BOOL	is_door_open() { return m_bOpen; }
	VOID	set_door_open(BOOL b) { m_bOpen = b; }

	void	onEndSBKWar();
	VOID	resetSBKData();
private:
	guild*	create_guild(Role* p_creator_, const tstring& str_guild_name_, DWORD dw_new_guild_id_);

	VOID	send_create_guild_to_db(const s_guild& st_guild_);
	VOID	send_dismiss_guild_to_db(DWORD dw_guild_id_);
	VOID	send_reset_signUpAttact_to_db();
private:
	typedef package_map<DWORD, guild*>		MAP_GUILD;	

	tagGuildCfg			st_guild_config;
	MAP_GUILD			map_guild;

private:
	typedef package_map<INT,	s_guild_pvp_data*> MAP_PVP;

	MAP_PVP				map_pvp_data;

	package_map<DWORD, DWORD>	list_guild_recruit;
	//package_list<DWORD, DWORD>		list_guild_recruit;

private:
	typedef package_map<DWORD, DWORD>	MAP_GUILD_KILL_BOSS;

	MAP_GUILD_KILL_BOSS		map_guild_kill_boss;

	//当前沙巴克公会
	DWORD		m_dw_SBK_guild;
	tagGuildSBKData	m_SBK_data;
	BOOL		m_bSBKStart;
	BOOL		m_bOpen;
	INT			m_SBK_tick;
};


//-------------------------------------------------------------------------------
//！ 帮会配置
//-------------------------------------------------------------------------------
inline const tagGuildCfg& guild_manager::get_guild_config() const
{
	return st_guild_config;
}

//-------------------------------------------------------------------------------
//！ 获取帮会个数
//-------------------------------------------------------------------------------
inline INT32 guild_manager::get_guild_num()
{
	return map_guild.size();
}

//-------------------------------------------------------------------------------
//! 获取帮会对象
//-------------------------------------------------------------------------------
inline guild* guild_manager::get_guild(DWORD dw_guild_id_)
{
	return map_guild.find(dw_guild_id_);
}

//-------------------------------------------------------------------------------
extern guild_manager g_guild_manager;

#endif