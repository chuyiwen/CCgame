
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//Lua脚本函数定义
#include "StdAfx.h"
#include "script_mgr.h"

#include "map.h"
#include "map_mgr.h"
#include "map_creator.h"
#include "map_instance.h"
#include "map_instance_guild.h"
#include "map_instance_pvp.h"
#include "unit.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "skill.h"
#include "buff.h"
#include "activity_mgr.h"
#include "role_mgr.h"
#include "group_mgr.h"
#include "item_creator.h"
#include "MsgInfoMgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "pet_pocket.h"
#include "title_mgr.h"
#include "SparseGraph.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/common_server_define.h"
#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/grubbao_protocol.h"
#include "../../common/WorldDefine/TreasureChest_define.h"
#include "../../common/WorldDefine/WeatherProtocol.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_god_level.h"

#include "att_res.h"
#include "TreasureChest_mgr.h"
#include "world_session.h"
#include "NPCTeam.h"
#include "../../common/WorldDefine/activity_protocol.h"
#include "quest_mgr.h"
#include "../../common/WorldDefine/mail_define.h"
#include "mail_mgr.h"
#include "master_prentice_mgr.h"
#include "pet_soul.h"
#include "pet.h"
#include "chat_mgr.h"
#include "RankMgr.h"
#include "hearSay_helper.h"
#include "BattleGround.h"
#include "../../common/WorldDefine/chat_protocol.h"

#include "../common/ServerDefine/rank_server_define.h"

#pragma warning(disable:4244)
//--------------------------------------------------------------------------------
// 脚本函数注册
//--------------------------------------------------------------------------------
static int script_register_creature_event(lua_State* L);
static int script_register_quest_event(lua_State* L);
static int script_register_role_event(lua_State* L);
static int script_register_map_event(lua_State* L);
static int script_register_activities_event(lua_State* L);
static int script_register_world_event(lua_State* L);
static int script_register_item_event(lua_State* L);
static int script_register_skill_event(lua_State* L);
static int script_register_buff_event(lua_State* L);
static int script_register_rank_event(lua_State* L);
//---------------------------------------------------------------------------------
// 脚本锁函数
//---------------------------------------------------------------------------------
static int script_create_script_mutex(lua_State* L);
static int script_lock_script_mutex(lua_State* L);
static int script_unlock_script_mutex(lua_State* L);

//---------------------------------------------------------------------------------
// 玩家和生物共有的接口函数
//---------------------------------------------------------------------------------
static int script_change_HP(lua_State* L);
static int script_change_MP(lua_State* L);
static int script_add_buff(lua_State* L);
static int script_cancel_buff(lua_State* L); 
static int script_have_buff(lua_State* L);
static int script_get_buff_warp_times(lua_State* L);
static int script_get_position(lua_State* L);
static int script_is_dead(lua_State* L);
static int script_is_player(lua_State* L);
static int script_is_creature(lua_State* L);
static int script_stop_attack(lua_State* L);
static int script_stop_move(lua_State* L);
static int script_is_attack(lua_State* L);
static int script_friend_enemy(lua_State* L);
static int script_get_unit_distince(lua_State* L);
static int script_set_range_event(lua_State* L);
static int script_get_current_move_state(lua_State* L);
static int script_get_att_valule(lua_State* L);
static int script_mod_att_value(lua_State* L);
static int script_set_state(lua_State* L);
static int script_unset_state(lua_State* L);
//---------------------------------------------------------------------------------
// 玩家独有函数
//---------------------------------------------------------------------------------
static int script_role_add_quest(lua_State* L);
static int script_role_all_quest_event(lua_State* L);
static int script_role_quest_event(lua_State* L);
static int script_role_quest_event_effective(lua_State* L);
static int script_delete_quest(lua_State* L);
static int script_role_complete_quest(lua_State* L);
static int script_set_quest_overview_message_id(lua_State* L);
static int script_add_quest_variable(lua_State* L);
static int script_inc_quest_variable(lua_State* L);
static int script_set_quest_variable(lua_State* L);
static int script_get_quest_variable(lua_State* L);
static int script_quest_init(lua_State* L);
static int script_quest_init_reward( lua_State* L );
static int script_role_goto_new_map(lua_State* L);
static int script_get_role_mapid(lua_State* L);//gx add 2013.10.24
static int script_get_role_level(lua_State *L);
static int script_set_role_level(lua_State* L);
static int script_get_role_att_value(lua_State *L);
static int script_get_role_class(lua_State* L);
static int script_mod_role_att_value(lua_State *L);
static int script_quest_add_role_item(lua_State* L);
static int script_role_add_skill(lua_State* L);
static int script_role_remove_skill(lua_State* L);
static int script_is_role_learn_skill(lua_State* L);
static int script_get_role_name(lua_State* L);
static int script_is_role_have_team(lua_State* L);
static int script_get_role_team_member_id(lua_State* L);
static int script_set_quest_script_data(lua_State* L);
static int script_mod_quest_script_data(lua_State* L);
static int script_get_quest_script_data(lua_State* L);
static int script_is_role_have_quest(lua_State* L);
static int script_is_role_have_done_quest( lua_State* L);
static int script_add_role_item(lua_State* L);
static int script_role_equip(lua_State* L);
static int script_get_role_equip(lua_State* L);
static int script_get_role_equip_level(lua_State* L);
static int script_remove_from_role(lua_State* L);
static int script_remove_from_role_n64serial(lua_State* L);//gx add 2013.9.2
static int script_remove_from_role_bind(lua_State* L);
static int script_get_bag_free_size(lua_State* L);
static int script_get_role_item_number(lua_State* L);
static int script_get_item_number_from_kind(lua_State* L);//gx add 2013.11.11
static int script_get_role_item_n64Serial_number(lua_State* L);//gx add 2013.9.2
static int script_get_role_item_bind_number(lua_State* L);
static int script_add_role_exp(lua_State* L);
static int script_add_exploits(lua_State* L);
static int script_dec_exploits(lua_State* L);
static int script_add_role_silver(lua_State* L);
static int script_dec_role_silver(lua_State* L);
static int script_get_role_silver(lua_State* L);
static int script_add_role_bind_silver(lua_State* L);
static int script_dec_role_bind_silver(lua_State* L);
static int script_get_role_bind_silver(lua_State* L);
static int script_get_role_yuanbao(lua_State* L);
static int script_add_role_yuanbao(lua_State* L);
static int script_dec_role_yuanbao(lua_State* L);
static int script_add_role_bindyuanbao(lua_State* L);//gx add 绑定元宝
static int script_dec_role_bindyuanbao(lua_State* L);
static int script_get_role_bindyuanbao(lua_State* L);
static int script_get_all_silver(lua_State* L);
static int script_dec_bind_and_nobind_silver(lua_State* L);
static int script_get_role_sex(lua_State* L);
static int script_get_role_dress_id(lua_State* L);
static int script_set_role_script_data(lua_State* L);
static int script_mod_role_script_data(lua_State* L);
static int script_get_role_script_data(lua_State* L);
static int script_add_role_ex_volume(lua_State* L);
static int script_is_role_in_status(lua_State* L);
static int script_can_mount(lua_State* L);
static int script_can_gather(lua_State* L);
static int script_get_role_level_up_exp(lua_State* L);
static int script_is_role_online(lua_State* L);
static int script_is_role_onstall(lua_State* L);
static int script_set_role_state(lua_State* L);
static int script_unset_role_state(lua_State* L);
static int script_can_open_chest(lua_State* L);
static int script_send_chest_message(lua_State* L);
static int script_inc_treasure_sum(lua_State* L);
static int script_get_treasure_sum(lua_State* L);
static int script_get_chest_item(lua_State* L);
static int script_item_need_broadcast(lua_State* L);
static int script_stop_mount(lua_State* L);
static int script_instance_notify(lua_State* L);
static int script_sig_title_event(lua_State* L);
static int script_change_skill_exp(lua_State* L);
static int script_get_friend_count(lua_State* L);
static int script_has_master(lua_State* L);
static int script_get_master_id(lua_State* L);
static int script_is_in_master_recruit(lua_State* L);
static int script_has_prentice(lua_State* L);
static int script_get_prentice_list(lua_State* L);
static int script_is_all_equip_level_has(lua_State* L);
static int script_is_all_def_equip_level_has(lua_State* L);
static int script_get_role_camp(lua_State* L);
static int script_set_role_camp(lua_State* L);
static int script_get_role_vigour(lua_State* L);
static int script_inc_role_vigour(lua_State* L);
static int script_get_item_script_data(lua_State* L);
static int script_set_item_script_data(lua_State* L);
static int script_get_item_serial_id(lua_State* L);
static int script_change_PK_value(lua_State* L);
static int script_get_PK_value(lua_State* L);
static int script_get_called_pet(lua_State* L);
static int script_get_called_pet_type_id(lua_State* L);
static int script_delete_pet(lua_State* L);
static int script_char_guild_message(lua_State* L);
static int script_get_guild_is_join_Recruit(lua_State* L);
static int script_is_item_bind(lua_State* L);
static int script_add_pet_healthy(lua_State* L);
static int script_get_role_amends_flag(lua_State* L);
static int script_set_role_amends_flag(lua_State* L);
static int script_get_is_receive_account_reward(lua_State* L);
static int script_get_receive_account_type(lua_State* L);
static int script_get_receive_account_type_ex(lua_State* L);
static int script_add_title(lua_State* L);
static int script_has_title(lua_State* L);
static int script_add_pet_exp(lua_State* L);
static int script_add_mounts_exp(lua_State* L);
static int script_mounts_level(lua_State* L);
static int script_is_formal(lua_State* L);
static int script_get_check_safe_code(lua_State* L);
static int script_is_monster_kill(lua_State* L);
static int script_is_war(lua_State* L);

static int script_get_active_data(lua_State* L);
static int script_set_active_data(lua_State* L);
static int script_get_active_receive(lua_State* L);
static int script_set_active_receive(lua_State* L);
static int script_get_active_value(lua_State* L);
static int script_set_active_value(lua_State* L);
static int script_get_guild_active_data(lua_State* L);
static int script_set_guild_active_data(lua_State* L);
static int script_get_guild_active_receive(lua_State* L);
static int script_set_guild_active_receive(lua_State* L);
static int script_get_guild_active_value(lua_State* L);
static int script_set_guild_active_value(lua_State* L);
static int script_get_flow_unit(lua_State* L);
static int script_inc_quest_refresh_number(lua_State* L);
static int script_get_quest_refresh_number(lua_State* L);
static int script_mod_day_clear_data(lua_State* L);
static int script_get_day_clear_data(lua_State* L);
static int script_add_shihun(lua_State* L);
static int script_inst_save_creature_is_dead(lua_State* L);
static int script_get_vip_checkin_num(lua_State* L);
static int script_add_vip_checkin_num(lua_State* L);
static int script_get_vip_level(lua_State* L);
static int script_set_vip_level(lua_State* L);//设置VIP等级
static int script_update_achievement_criteria(lua_State* L);
static int script_is_in_duel(lua_State *L);
static int script_get_role_around_creature(lua_State* L);
static int script_get_role_around_role(lua_State* L);
static int script_send_hang_left_time(lua_State *L);
static int script_set_hang_left_time(lua_State *L);
static int script_add_hang_left_time(lua_State *L);
static int script_get_hang_left_time(lua_State *L);
static int script_show_consume_reward_frame(lua_State *L);
static int script_get_account(lua_State* L);
static int script_qianghua_equip(lua_State* L);
static int script_get_pet_quality(lua_State* L);
static int script_get_pet_level(lua_State* L);
static int script_get_ronghe_pet(lua_State* L);
static int script_get_xili_number(lua_State* L);
static int script_add_xili_number(lua_State* L);
static int script_get_god_level(lua_State* L);
static int script_set_god_level(lua_State* L);
static int script_set_wine_use_number(lua_State* L);
static int script_get_wine_use_number(lua_State* L);
static int script_add_reward_item(lua_State* L);
static int script_set_instance_data(lua_State* L);
static int script_get_officialsalary_number(lua_State* L);
static int script_set_score(lua_State* L);
static int script_get_score(lua_State* L);
static int script_get_role_spouseid(lua_State* L);//gx add 2013.10.25
static int script_set_femalerole_redzui(lua_State* L);//gx add 2013.10.29设置红唇标记
//---------------------------------------------------------------------------------
// 帮派相关接口
//---------------------------------------------------------------------------------
static int script_get_role_guild_id(lua_State* L);						// 得到玩家帮派ID（玩家没有帮派返回INVALID_VALUE)
static int script_get_guild_pos(lua_State* L);
static int script_get_guild_fund(lua_State* L);
static int script_get_guild_material(lua_State* L);
static int script_get_guild_peace(lua_State* L);
static int script_get_guild_reputation(lua_State* L);
static int script_get_guild_contribution(lua_State* L);
static int script_get_guild_prosperity(lua_State* L);
static int script_get_guild_script_data(lua_State* L);

static int script_is_guild_in_status(lua_State* L);
static int script_is_SBK_guild(lua_State* L);
static int script_init_guild_pvp_data(lua_State* L);

static int script_get_guild_challenge_value(lua_State* L);
static int script_mod_guild_challenge_value(lua_State* L);
static int script_get_guild_challenge_step(lua_State* L);

static int script_get_guild_leader_id(lua_State* L);
static int script_get_guild_script_data_six_list(lua_State* L);
static int script_modify_guild_fund(lua_State* L);
static int script_modify_guild_material(lua_State* L);
static int script_modify_guild_peace(lua_State* L);
static int script_modify_guild_reputation(lua_State* L);
static int script_modify_contribution(lua_State* L);
static int script_modify_prosperity(lua_State* L);
static int script_modify_script_data(lua_State* L);
static int script_set_guild_relive_id(lua_State* L);
static int script_set_guild_tripod_id(lua_State* L);
static int script_start_guild_prictice(lua_State* L);
static int script_guild_door_limit(lua_State* L);
static int script_guild_dismiss(lua_State* L);
static int script_guild_graded_destroy(lua_State* L);
static int SC_GuildWarKillRole(lua_State* L);
static int script_get_guild_level(lua_State* L);
static int script_set_guild_plant_data(lua_State* L);
static int script_can_open_statue(lua_State* L);
static int script_active_daogao(lua_State* L);
static int script_get_daogao_num(lua_State* L);
static int script_get_build_level(lua_State* L);
static int script_set_guild_win(lua_State* L);
static int script_set_guild_lose(lua_State* L);
static int script_get_jujue_war_time(lua_State* L);
//---------------------------------------------------------------------------------
// 怪物独有函数
//---------------------------------------------------------------------------------
static int script_get_around_creature(lua_State* L);
static int script_get_around_role(lua_State* L);
static int script_set_creature_script_data(lua_State* L);
static int script_mod_creature_script_data(lua_State* L);
static int script_get_creature_script_data(lua_State* L);
static int script_set_creature_update_ai_timer(lua_State* L);
static int script_creature_use_skill(lua_State* L);
static int script_monster_say(lua_State* L);
static int script_monster_play_action(lua_State* L);
static int script_creature_change_ai_state(lua_State* L);
static int script_get_enter_combat_tick(lua_State* L);
static int script_get_creature_type_id(lua_State* L);
static int script_add_enmity(lua_State* L);
static int script_clear_enmity(lua_State* L);
static int script_get_creature_cur_target_id(lua_State* L);
static int script_get_creature_att_value(lua_State* L);
static int script_monster_move(lua_State* L);
static int script_monster_move_point(lua_State* L);
static int script_clear_all_enmity(lua_State* L);
static int script_get_enmity_list(lua_State* L);
//static int SC_SetCanBeAttack(lua_State* L);
static int script_set_creature_type(lua_State* L);
static int script_set_follow_target(lua_State* L);
static int script_set_guild_id(lua_State* L);
static int script_get_guild_id(lua_State* L);
//static int script_set_guild_plant_index(lua_State* L);
//static int script_get_guild_plant_index(lua_State* L);
//static int script_set_used(lua_State* L);
//static int script_set_mound(lua_State* L);
//static int script_set_max_yield(lua_State* L);
static int script_teleport(lua_State* L);
static int script_rolekillcreature(lua_State* L);//gx add 6.17
//---------------------------------------------------------------------------------
// 固定活动接口
//---------------------------------------------------------------------------------
static int script_init_event_time(lua_State* L);
static int script_get_act_is_start(lua_State* L);
static int script_add_all_role_buff(lua_State* L);
static int script_set_activities_script_data(lua_State* L);
static int script_get_activities_script_data(lua_State* L);
static int script_save_activities_data(lua_State* L);
static int script_activities_broad(lua_State* L);
static int script_set_battle_boss(lua_State* L);
//---------------------------------------------------------------------------------
// 地图接口
//---------------------------------------------------------------------------------
static int script_set_map_script_data(lua_State* L);
static int script_get_map_script_data(lua_State* L);
static int script_map_create_creature(lua_State* L);
static int script_map_create_col_creature(lua_State* L);
static int script_map_delete_creature(lua_State* L);
static int SC_MapPlaySceneEffectByObjID(lua_State* L);
static int SC_MapPlaySceneEffectByID(lua_State* L);
static int SC_MapPlaySceneMusic(lua_State* L);
static int script_add_map_role_buff(lua_State* L);
static int script_create_instance(lua_State* L);
static int script_delete_instance(lua_State* L);
static int script_is_in_area(lua_State* L);
static int script_is_in_trigger(lua_State* L);
static int script_play_scene_effect(lua_State* L);
static int script_stop_scene_effect(lua_State* L);
static int script_open_door(lua_State* L);
static int script_close_door(lua_State* L);
static int script_is_door_open(lua_State* L);
static int script_get_param(lua_State* L);
static int script_change_weather(lua_State* L);
static int script_get_map_door_state(lua_State* L);
static int script_get_inst_mode(lua_State* L);
static int script_get_map_all_role_id(lua_State* L);
static int script_get_map_all_role_number(lua_State* L);
//---------------------------------------------------------------------------------
// 脚本消息接口
//---------------------------------------------------------------------------------
static int script_begin_message_event(lua_State* L);
static int script_add_message_event(lua_State* L);
static int script_dispatch_role_message_event(lua_State* L);
static int script_dispatch_world_message_event(lua_State* L);
static int script_dispath_map_message_event(lua_State* L); 

//---------------------------------------------------------------------------------
// 系统相关接口
//---------------------------------------------------------------------------------
static int script_time_get_time(lua_State* L);
static int script_data_time_to_dword_time(lua_State* L);
static int script_get_current_dword_time(lua_State* L);
static int script_cal_time_diff(lua_State* L);
static int script_increase_time(lua_State* L);
static int script_decrease_time(lua_State* L);
static int script_get_world_tick(lua_State* L);
static int script_is_exist_item(lua_State* L);
static int script_is_exist_creature(lua_State* L);
static int script_get_server_index(lua_State* L);
static int script_get_role_level_rank(lua_State* L);
static int script_send_award_message(lua_State* L);
static int script_send_guild_boss_message(lua_State* L);
static int script_send_hearsay_message(lua_State* L);
static int script_shihun_give_reward(lua_State* L);
static int script_get_pet_ernie_level(lua_State* L);
static int script_get_zhanli_role_level(lua_State* L);
static int script_get_open_server_day(lua_State* L);
//---------------------------------------------------------------------------------
// 脚本log记录
//---------------------------------------------------------------------------------
static int script_write_log(lua_State* L);
static int script_print(lua_State* L);                                      //打印整型到控制台
static int script_lua_console_print(lua_State* L);                     //打印字符串到控制台
//--------------------------------------------------------------------------------
// 64位数据处理
//--------------------------------------------------------------------------------
static void	push_64bit_data(lua_State* L, INT64 n64Data);
static INT64	pop_64bit_data(lua_State* L, int nHigh, int nLow);

//---------------------------------------------------------------------------------
// 玩家小队接口
//---------------------------------------------------------------------------------
static int script_get_team_leader_id(lua_State* L);
static int script_create_team(lua_State* L);
static int script_add_team_member(lua_State* L);

//---------------------------------------------------------------------------------
// 邮件接口
//---------------------------------------------------------------------------------
static int script_system_mail(lua_State* L);

//---------------------------------------------------------------------------------
// 辅助包
//---------------------------------------------------------------------------------
static const luaL_Reg auxLib[] =
{
	{"register_creature_event",			script_register_creature_event},
	{"register_quest_event",			script_register_quest_event},
	{"register_role_event",				script_register_role_event},
	{"register_map_event",				script_register_map_event},
	{"register_activities_event",		script_register_activities_event},
	{"register_world_event",			script_register_world_event},
	{"register_item_event",				script_register_item_event},
	{"register_skill_event",			script_register_skill_event},
	{"register_buff_event",				script_register_buff_event},
	{"register_rank_event",				script_register_rank_event},
	{"create_script_mutex",				script_create_script_mutex},
	{"lock_script_mutex",				script_lock_script_mutex},
	{"unlock_script_mutex",				script_unlock_script_mutex},
	{"time_get_time",					script_time_get_time},
	{"get_world_tick",					script_get_world_tick},
	{"write_log",						script_write_log},
	{"data_time_to_dword_time",			script_data_time_to_dword_time},
	{"get_current_dword_time",			script_get_current_dword_time},
	{"cal_time_diff",					script_cal_time_diff},
	{"increase_time",					script_increase_time},
	{"decrease_time",					script_decrease_time},
	{"console_print",					script_lua_console_print},
	{"print_int",						script_print},
	{"is_exist_item",					script_is_exist_item},
	{"is_exist_creature",				script_is_exist_creature},
	{"get_server_index",				script_get_server_index},
	{"get_role_level_rank",				script_get_role_level_rank},
	{"send_award_message",				script_send_award_message},
	{"send_guild_boss_message",			script_send_guild_boss_message},
	{"send_hearsay_message",			script_send_hearsay_message},
	{"shihun_give_reward",				script_shihun_give_reward},
	{"get_pet_ernie_level",				script_get_pet_ernie_level},
	{"get_zhanli_role_level",			script_get_zhanli_role_level},
	{"get_open_server_day",				script_get_open_server_day},
	{NULL, NULL}
};

//----------------------------------------------------------------------------------
// 玩家和生物共有接口包
//----------------------------------------------------------------------------------
static const luaL_Reg unitLib[] = 
{
	{"change_HP",						script_change_HP},
	{"change_MP",						script_change_MP},
	{"add_buff",						script_add_buff},
	{"cancel_buff",						script_cancel_buff},
	{"have_buff",						script_have_buff},
	{"get_buff_warp_times",				script_get_buff_warp_times},
	{"get_position",					script_get_position},
	{"is_dead",							script_is_dead},
	{"is_player",						script_is_player},
	{"is_creature",						script_is_creature},
	{"stop_move",						script_stop_move},
	{"stop_attack",						script_stop_attack},
	{"is_attack",						script_is_attack},
	{"friend_enemy",					script_friend_enemy},
	{"get_unit_distince",				script_get_unit_distince},
	{"set_range_event",					script_set_range_event},
	{"get_current_move_state",			script_get_current_move_state},
	{"get_att_valule",					script_get_att_valule},
	{"mod_att_value",					script_mod_att_value},
	{"set_state",						script_set_state},
	{"unset_state",						script_unset_state},
	{NULL, NULL},
};

//----------------------------------------------------------------------------------
// 固定活动接口
//----------------------------------------------------------------------------------
static const luaL_Reg actLib[] =
{
	{"init_event_time",					script_init_event_time},
	{"get_act_is_start",				script_get_act_is_start},
	{"add_all_role_buff",				script_add_all_role_buff},
	{"set_activities_script_data",		script_set_activities_script_data},
	{"get_activities_script_data",		script_get_activities_script_data},
	{"save_activities_data",			script_save_activities_data},
	{"activities_broad",				script_activities_broad},
	{"set_battle_boss",					script_set_battle_boss},
	{NULL, NULL},
};

//----------------------------------------------------------------------------------
// 地图事件接口
//----------------------------------------------------------------------------------
static const luaL_Reg mapLib[] = 
{
	{"set_map_script_data",				script_set_map_script_data},
	{"get_map_script_data",				script_get_map_script_data},
	{"map_create_creature",				script_map_create_creature},
	{"map_create_col_creature",			script_map_create_col_creature},
	{"map_delete_creature",				script_map_delete_creature},
	{"create_instance",					script_create_instance},
	{"delete_instance",					script_delete_instance},
	{"add_map_role_buff",				script_add_map_role_buff},
	{"is_in_area",						script_is_in_area},
	{"is_in_trigger",					script_is_in_trigger},
//	{"map_play_scene_effect_by_obj_id",	script_map_play_scene_effect_by_obj_id},
//	{"map_play_scene_effect_by_id",		script_map_play_scene_effect_by_id},
//	{"map_play_scene_music",			script_map_play_scene_music},
	{"play_scene_effect",				script_play_scene_effect},
	{"stop_scene_effect",				script_stop_scene_effect},
	{"open_door",						script_open_door},
	{"close_door",						script_close_door},
	{"is_door_open",					script_is_door_open},
	{"system_mail",						script_system_mail},
	{"get_map_param",					script_get_param},
	{"change_weather",					script_change_weather},
	{"get_map_door_state",				script_get_map_door_state},
	{"get_inst_mode",					script_get_inst_mode},
	{"get_map_all_role_id",				script_get_map_all_role_id},
	{"get_map_all_role_number",			script_get_map_all_role_number},
	{NULL, NULL},
};

//----------------------------------------------------------------------------------
// 玩家事件接口
//----------------------------------------------------------------------------------
static const luaL_Reg roleLib[] = 
{
	{"role_goto_new_map",				script_role_goto_new_map},
	{"get_role_mapid",					script_get_role_mapid},
	{"get_role_level",					script_get_role_level},
	{"set_role_level",					script_set_role_level},
	{"get_role_att_value",				script_get_role_att_value},
	{"get_role_class",					script_get_role_class},
	{"mod_role_att_value",				script_mod_role_att_value},
	{"add_quest",						script_role_add_quest},
	{"role_all_quest_event",			script_role_all_quest_event},
	{"role_quest_event",				script_role_quest_event},
	{"role_quest_event_effective",		script_role_quest_event_effective},
	{"delete_quest",					script_delete_quest},
	{"complete_quest",					script_role_complete_quest},
	{"set_quest_overview",				script_set_quest_overview_message_id},
	{"add_quest_variable",				script_add_quest_variable},
	{"inc_quest_variable",				script_inc_quest_variable},
	{"set_quest_variable",				script_set_quest_variable},
	{"get_quest_variable",				script_get_quest_variable},
	{"quest_init",						script_quest_init},
	{"quest_init_reward",				script_quest_init_reward},
	{"quest_add_role_item",				script_quest_add_role_item},
	{"add_skill",						script_role_add_skill},
	{"remove_skill",					script_role_remove_skill},
	{"is_learn_skill",					script_is_role_learn_skill},
	{"get_name",						script_get_role_name},
	{"is_role_have_team",				script_is_role_have_team},
	{"get_role_team_member_id",			script_get_role_team_member_id},
	{"set_quest_script_data",			script_set_quest_script_data},
	{"get_quest_script_data",			script_get_quest_script_data},
	{"mod_quest_script_data",			script_mod_quest_script_data},
	{"is_role_have_quest",				script_is_role_have_quest},
	{"is_role_have_done_quest",			script_is_role_have_done_quest},
	{"add_item",						script_add_role_item},
	{"role_equip",						script_role_equip},
	{"get_role_equip",					script_get_role_equip},
	{"get_role_equip_level",			script_get_role_equip_level},
	{"remove_from_role",				script_remove_from_role},
	{"remove_from_role_n64serial",		script_remove_from_role_n64serial},
	{"remove_from_role_bind",			script_remove_from_role_bind},
	{"get_bag_free_size",				script_get_bag_free_size},
	{"get_item_number",					script_get_role_item_number},
	{"get_item_numbr_from_kind",		script_get_item_number_from_kind},
	{"get_item_n64serial_number",		script_get_role_item_n64Serial_number},
	{"get_bind_item_number",			script_get_role_item_bind_number},
	{"add_role_exp",					script_add_role_exp},
	{"add_role_exploits",				script_add_exploits},
	{"dec_role_exploits",				script_dec_exploits},
	{"add_role_silver",					script_add_role_silver},
	{"dec_role_silver",					script_dec_role_silver},
	{"get_role_silver",					script_get_role_silver},
	{"add_role_bind_silver",			script_add_role_bind_silver},
	{"dec_role_bind_silver",			script_dec_role_bind_silver},
	{"get_role_bind_silver",			script_get_role_bind_silver},
	{"add_role_yuanbao",				script_add_role_yuanbao},
	{"get_role_yuanbao",				script_get_role_yuanbao},
	{"dec_role_yuanbao",				script_dec_role_yuanbao},
	{"add_role_bindyuanbao",			script_add_role_bindyuanbao},
	{"dec_role_bindyuanbao",			script_dec_role_bindyuanbao},
	{"get_role_bindyuanbao",			script_get_role_bindyuanbao},
	{"get_all_silver",					script_get_all_silver},
	{"dec_bind_and_nobind_silver",		script_dec_bind_and_nobind_silver},
	{"get_role_sex",					script_get_role_sex},
	{"get_role_dress_id",				script_get_role_dress_id},
	{"set_role_script_data",			script_set_role_script_data},
	{"mod_role_script_data",			script_mod_role_script_data},
	{"get_role_script_data",			script_get_role_script_data},
	{"add_role_ex_volume",				script_add_role_ex_volume},
	{"is_role_in_status",				script_is_role_in_status},
	{"can_mount",						script_can_mount},
	{"can_gather",						script_can_gather},
	{"get_role_level_up_exp",			script_get_role_level_up_exp},
	{"is_role_online",					script_is_role_online},
	{"is_role_onstall",					script_is_role_onstall},	
	{"can_open_chest",					script_can_open_chest},		
	{"send_chest_message",				script_send_chest_message},		
	{"inc_treasure_sum",				script_inc_treasure_sum},		
	{"get_treasure_sum",				script_get_treasure_sum},		
	{"get_chest_item",					script_get_chest_item},
	{"item_need_broadcast",				script_item_need_broadcast},
	{"set_role_state",					script_set_role_state},
	{"unset_role_state",				script_unset_role_state},
	{"stop_mount",						script_stop_mount},
	{"instance_notify",					script_instance_notify},
	{"sig_title_event",					script_sig_title_event},
	{"change_skill_exp",				script_change_skill_exp},
	{"set_guild_relive_id",				script_set_guild_relive_id},
	{"set_guild_tripod_id",				script_set_guild_tripod_id},
	{"start_guild_prictice",			script_start_guild_prictice},
	{"guild_door_limit",				script_guild_door_limit},
	{"guild_dismiss",					script_guild_dismiss},
	{"guild_graded_destroy",			script_guild_graded_destroy},
	{"get_friend_count",				script_get_friend_count},
	{"has_master",						script_has_master},
	{"get_master_id",						script_get_master_id},
	{"is_in_master_recruit",			script_is_in_master_recruit},
	{"has_prentice",					script_has_prentice},
	{"get_prentice_list",					script_get_prentice_list},
	{"is_all_ack_equip_level_has",		script_is_all_equip_level_has},
	{"is_all_def_equip_level_has",		script_is_all_def_equip_level_has},
	{"get_role_camp",					script_get_role_camp},
	{"set_role_camp",					script_set_role_camp},
	{"get_role_vigour",					script_get_role_vigour},
	{"inc_role_vigour",					script_inc_role_vigour},
	{"get_item_script_data",			script_get_item_script_data},
	{"set_item_script_data",			script_set_item_script_data},
	{"get_item_serial_id",				script_get_item_serial_id},
	{"change_pk_value",					script_change_PK_value},
	{"get_pk_value",					script_get_PK_value},
	{"get_called_pet",					script_get_called_pet},
	{"get_called_pet_type_id",			script_get_called_pet_type_id},
	{"delete_pet",						script_delete_pet},
	{"get_guild_is_join_Recruit",		script_get_guild_is_join_Recruit},
	{"char_guild_message",				script_char_guild_message},
	{"is_item_bind",					script_is_item_bind},
	{"add_pet_healthy",					script_add_pet_healthy},
	{"get_role_amends_flag",			script_get_role_amends_flag},
	{"set_role_amends_flag",			script_set_role_amends_flag},
	{"get_is_receive_account_reward",	script_get_is_receive_account_reward},
	{"get_receive_account_type",		script_get_receive_account_type},
	{"get_receive_account_type_ex",		script_get_receive_account_type_ex},
	{"add_title",						script_add_title},
	{"has_title",						script_has_title},
	{"add_pet_exp",						script_add_pet_exp},
	{"add_mounts_exp",					script_add_mounts_exp},
	{"get_mounts_level",				script_mounts_level},
	{"is_pass_validate",				script_get_check_safe_code},
	{"is_monster_kill",					script_is_monster_kill},
	{"get_active_data",					script_get_active_data},
	{"set_active_data",					script_set_active_data},
	{"get_active_receive",				script_get_active_receive},
	{"set_active_receive",				script_set_active_receive},
	{"get_active_value",				script_get_active_value},
	{"set_active_value",				script_set_active_value},
	{"get_guild_active_data",			script_get_guild_active_data},
	{"set_guild_active_data",			script_set_guild_active_data},
	{"get_guild_active_receive",		script_get_guild_active_receive},
	{"set_guild_active_receive",		script_set_guild_active_receive},
	{"get_guild_active_value",			script_get_guild_active_value},
	{"set_guild_active_value",			script_set_guild_active_value},
	{"get_flow_unit",					script_get_flow_unit},
	{"inc_quest_refresh_number",		script_inc_quest_refresh_number},
	{"get_quest_refresh_number",		script_get_quest_refresh_number},
	{"mod_day_clear_data",				script_mod_day_clear_data},
	{"get_day_clear_data",				script_get_day_clear_data},
	{"add_shihun",						script_add_shihun},
	{"inst_save_creature_is_dead",		script_inst_save_creature_is_dead},
	{"get_vip_checkin_num",				script_get_vip_checkin_num},
	{"add_vip_checkin_num",				script_add_vip_checkin_num},
	{"get_vip_level",					script_get_vip_level},
	{"set_vip_level",					script_set_vip_level},
	{"update_achievement_criteria",		script_update_achievement_criteria},
	{"is_in_duel",						script_is_in_duel},
	{"get_around_role",					script_get_role_around_role},
	{"get_around_creature",				script_get_role_around_creature},
	{"send_hang_left_time",				script_send_hang_left_time},
	{"get_hang_left_time",				script_get_hang_left_time},
	{"set_hang_left_time",				script_set_hang_left_time},
	{"add_hang_left_time",				script_add_hang_left_time},
	{"show_consume_reward_frame",		script_show_consume_reward_frame},
	{"get_account",						script_get_account},
	{"qianghua_equip",					script_qianghua_equip},
	{"get_pet_att",						script_get_pet_quality},
	{"get_pet_level",					script_get_pet_level},
	{"get_rolehe_pet",					script_get_ronghe_pet},
	{"script_get_xili_number",			script_get_xili_number},
	{"script_add_xili_number",			script_add_xili_number},
	{"get_god_level",					script_get_god_level},
	{"set_god_level",					script_set_god_level},
	{"set_wine_use_number",				script_set_wine_use_number},
	{"get_wine_use_number",				script_get_wine_use_number},
	{"add_reward_item",					script_add_reward_item},
	{"set_instance_data",				script_set_instance_data},
	{"get_officialsalary_number",		script_get_officialsalary_number},
	{"set_score",						script_set_score},
	{"get_score",						script_get_score},
	{"get_role_spouseid",				script_get_role_spouseid},
	{"set_femalerole_redzui",			script_set_femalerole_redzui},
	{NULL, NULL},	
};

//--------------------------------------------------------------------------------------
// 消息事件接口
//--------------------------------------------------------------------------------------
static const luaL_Reg msgLib[] = 
{
	{"begin_message_event",				script_begin_message_event},
	{"add_message_event",				script_add_message_event},
	{"dispatch_role_message_event",		script_dispatch_role_message_event},
	{"dispatch_world_message_event",	script_dispatch_world_message_event},
	{"dispath_map_message_event",		script_dispath_map_message_event},
	{NULL, NULL},
};

//--------------------------------------------------------------------------------------
// 怪物事件接口
//--------------------------------------------------------------------------------------
static const luaL_Reg creLib[] = 
{
	{"get_around_creature",				script_get_around_creature},
	{"get_around_role",					script_get_around_role},
	{"get_creature_script_data",		script_get_creature_script_data},
	{"set_creature_script_data",		script_set_creature_script_data},
	{"mod_creature_script_data",		script_mod_creature_script_data},
	{"set_creature_update_ai_timer",	script_set_creature_update_ai_timer},
	{"creature_use_skill",				script_creature_use_skill},
	{"monster_say",						script_monster_say},
	{"monster_play_action",				script_monster_play_action},
	{"creature_change_ai_state",		script_creature_change_ai_state},
	{"get_enter_combat_tick",			script_get_enter_combat_tick},
	{"get_creature_type_id",			script_get_creature_type_id},
	{"add_enmity",						script_add_enmity},
	{"clear_enmity",					script_clear_enmity},
	{"get_creature_cur_target_id",		script_get_creature_cur_target_id},
	{"get_creature_att_value",			script_get_creature_att_value},
	{"monster_move",					script_monster_move},
	{"monster_move_point",				script_monster_move_point},
	{"clear_all_enmity",				script_clear_all_enmity},
	{"get_enmity_list",					script_get_enmity_list},
	//{"set_can_beattack",				script_set_can_beattack},
	{"set_creature_type",				script_set_creature_type},
	{"set_follow_target",				script_set_follow_target},
	{"set_guild_id",					script_set_guild_id},
	{"get_guild_id",					script_get_guild_id},
	//{"set_guild_plant_index",			script_set_guild_plant_index},
	//{"get_guild_plant_index",			script_get_guild_plant_index},
	//{"set_used",						script_set_used},
	//{"set_mound",						script_set_mound},
	//{"set_max_yield",					script_set_max_yield},
	{"teleport",						script_teleport},
	{"rolekillcreature",				script_rolekillcreature},
	{NULL,	NULL},
};

static const luaL_Reg teamLib[] = 
{
	{"create_team",						script_create_team},
	{"add_team_member",					script_add_team_member},
	{"get_team_leader_id",				script_get_team_leader_id},
	{NULL, NULL},
};

static const luaL_Reg guildLib[] = 
{
	{"get_role_guild_id",				script_get_role_guild_id},
	{"get_guild_pos",					script_get_guild_pos},
	{"get_guild_fund",					script_get_guild_fund},
	{"get_guild_material",				script_get_guild_material},
	{"get_guild_peace",					script_get_guild_peace},
	{"get_guild_reputation",			script_get_guild_reputation},
	{"get_guild_contribution",			script_get_guild_contribution},
	{"get_guild_prosperity",			script_get_guild_prosperity},
	{"get_guild_script_data",			script_get_guild_script_data},
	{"modify_contribution",				script_modify_contribution},
	{"modify_guild_fund",				script_modify_guild_fund},
	{"modify_guild_material",			script_modify_guild_material},
	{"modify_guild_peace",				script_modify_guild_peace},
	{"modify_guild_reputation",			script_modify_guild_reputation},
	{"modify_prosperity",				script_modify_prosperity},
	{"modify_script_data",				script_modify_script_data},
	{"is_guild_in_status",				script_is_guild_in_status},
	{"is_SBK_guild",					script_is_SBK_guild},
	{"init_guild_pvp_data",				script_init_guild_pvp_data},
	{"get_guild_challenge_value",		script_get_guild_challenge_value},
	{"mod_guild_challenge_value",		script_mod_guild_challenge_value},
	{"get_guild_challenge_step",		script_get_guild_challenge_step},
	{"get_guild_leader_id",				script_get_guild_leader_id},
	{"get_guild_script_data_six_list",	script_get_guild_script_data_six_list},
	{"is_formal",						script_is_formal},
	{"is_war",							script_is_war},
	{"get_guild_level",					script_get_guild_level},
	{"set_guild_plant_data",			script_set_guild_plant_data},
	{"can_open_statue",					script_can_open_statue},
	{"active_daogao",					script_active_daogao},
	{"get_daogao_num",					script_get_daogao_num},
	{"get_build_level",					script_get_build_level},
	{"set_guild_war_win",				script_set_guild_win},
	{"set_guild_war_lose",				script_set_guild_lose},
	{"get_jujue_war_time",				script_get_jujue_war_time},
	{NULL,	NULL},
};

//----------------------------------------------------------------------------------
// 注册C函数库函数
//----------------------------------------------------------------------------------
VOID LuaOpenCommLibs(lua_State*  L)
{
	luaL_openlib(L, "auxiliary",	auxLib,  0);
	luaL_openlib(L, "units",		unitLib, 0);
	luaL_openlib(L, "player",		roleLib, 0);
	luaL_openlib(L, "activities",	actLib,  0);
	luaL_openlib(L,	"scene",		mapLib,  0);
	luaL_openlib(L, "news",			msgLib,  0);
	luaL_openlib(L, "monster",		creLib,  0);
	luaL_openlib(L, "team",			teamLib, 0);
	luaL_openlib(L, "guild",		guildLib, 0);
}

//--------------------------------------------------------------------------------
// 64位数据处理
//--------------------------------------------------------------------------------
void push_64bit_data(lua_State* L, INT64 n64Data)
{
	const INT32 nMask	= 0xFFFFFFFF;

	INT32 n32High	= (INT32)((n64Data >> 32) & nMask);
	INT32 n32Low	= (INT32)(n64Data & nMask);

	lua_pushnumber(L, n32High);
	lua_pushnumber(L, n32Low);
}

INT64 pop_64bit_data(lua_State* L, int nHigh, int nLow)
{
	const INT64 n64Mask	= 0x00000000FFFFFFFF;

	INT32 n32High	= lua_tonumber(L, nHigh);
	INT32 n32Low	= lua_tonumber(L, nLow);

	INT64 n64Data	= n32High;
	n64Data			= (n64Data << 32) | (n32Low & n64Mask);

	return n64Data;
}

//-----------------------------------------------------------------------------------
// 外部的注册函数
//-----------------------------------------------------------------------------------
int script_register_creature_event(lua_State* L)
{
	DWORD dw_data_id = luaL_checknumber(L, 1);			// 生物的ID
	INT nEventType = luaL_checkint(L, 2);				// 生物的事件类型
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if( !VALID_VALUE(dw_data_id) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterCreatureEvent(dw_data_id, (EScriptCreatureEvent)nEventType, szFunction);
	return 0;
}

int script_register_quest_event(lua_State* L)
{
	UINT16 u16QuestID = (UINT16)luaL_checknumber(L, 1);	// 任务ID
	INT nEventType = luaL_checkint(L, 2);				// 事件
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if( (INT16)u16QuestID < 0 || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterQuestEvent(u16QuestID, (EScriptQuestEvent)nEventType, szFunction);

	return 0;
}

int script_register_role_event(lua_State* L)
{
	INT nEventType = luaL_checkint(L, 1);				// 事件
	const CHAR* szFunction = luaL_checkstring(L, 2);	// 脚本函数

	if( !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterRoleEvent((EScriptRoleEvent)nEventType, szFunction);

	return 0;
}

int script_register_map_event(lua_State* L)
{
	const CHAR* szMapName = luaL_checkstring(L, 1);		// 地图名字
	INT nEventType = luaL_checkint(L, 2);				// 事件
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if( !VALID_POINT(szMapName) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterMapEvent(szMapName, (EScriptMapEvent)nEventType, szFunction);

	return 0;
}

int script_register_activities_event(lua_State* L)
{
	DWORD dwActID = luaL_checkint(L, 1);				// 活动ID
	INT nEventType = luaL_checkint(L, 2);				// 事件
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if( !VALID_VALUE(dwActID) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterActEvent(dwActID, (EScriptActEvent)nEventType, szFunction);

	return 0;
}

int script_register_world_event(lua_State* L)
{
	INT		nEventType = luaL_checkint(L, 1);			// 事件
	const CHAR* szFunction = luaL_checkstring(L, 2);	// 脚本函数

	if( !VALID_VALUE(nEventType) || !VALID_POINT(szFunction) )
		return 0;

	g_ScriptMgr.RegisterWorldEvent((EScriptWorldEvent)nEventType, szFunction);

	return 0;
}

int script_register_item_event(lua_State* L)
{
	DWORD dw_data_id	=	luaL_checknumber(L, 1);			// 物品ID
	INT	nEventType	=	luaL_checkint(L, 2);			// 事件
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if(!VALID_VALUE(dw_data_id) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction))
		return 0;

	g_ScriptMgr.RegisterItemEvent(dw_data_id, (EScriptItemEvent)nEventType, szFunction);

	return 0;
}

int script_register_skill_event(lua_State* L)
{
	DWORD dwSkillID	=	luaL_checknumber(L, 1);			// 技能ID
	INT	nEventType	=	luaL_checkint(L, 2);			// 事件
	const CHAR* szFunction = luaL_checkstring(L, 3);	// 脚本函数

	if(!VALID_VALUE(dwSkillID) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction))
		return 0;

	g_ScriptMgr.RegisterSkillEvent(dwSkillID, (EScriptSkillEvent)nEventType, szFunction);

	return 0;
}

int script_register_buff_event(lua_State* L)
{
	DWORD dwBuffID = luaL_checknumber(L, 1);		//buff ID
	INT nEventType = luaL_checkint(L, 2);			//事件
	const CHAR* szFunction = luaL_checkstring(L, 3);//脚本函数

	if (!VALID_VALUE(dwBuffID) || !VALID_VALUE(nEventType) || !VALID_POINT(szFunction))
		return 0;

	g_ScriptMgr.RegisterBuffEvent(dwBuffID, (EScriptBuffEvent)nEventType, szFunction);

	return 0;
}

int script_register_rank_event(lua_State* L)
{
	INT nEventType = luaL_checkint(L, 1);				//事件
	const CHAR* szFunction = luaL_checkstring(L, 2);	//脚本函数

	if (!VALID_VALUE(nEventType) || !VALID_VALUE(szFunction))
		return 0;
	
	g_ScriptMgr.RegisterRankEvent((EScriptRankEvent)nEventType, szFunction);

	return 0;
}
//-------------------------------------------------------------------------------------
// 锁包
//-------------------------------------------------------------------------------------
int script_create_script_mutex(lua_State* L)
{
	DWORD dwMutexID = g_ScriptMgr.CreateScriptMutex();

	lua_pushnumber(L, dwMutexID);

	return 1;
}

int script_lock_script_mutex(lua_State* L)
{
	DWORD dwMutexID = lua_tonumber(L, 1);

	g_ScriptMgr.LockScriptMutex(dwMutexID);

	return 0;
}

int script_unlock_script_mutex(lua_State* L)
{
	DWORD dwMutexID = lua_tonumber(L, 1);

	g_ScriptMgr.UnLockScriptMutex(dwMutexID);

	return 0;
}

//-------------------------------------------------------------------------------------
// 获得系统时间
//-------------------------------------------------------------------------------------
int script_time_get_time(lua_State* L)
{
	DWORD dw_time = timeGetTime();

	lua_pushnumber(L, dw_time);

	return 1;
}

//-------------------------------------------------------------------------------------
// 得到服务器当前的心跳数
//-------------------------------------------------------------------------------------
int script_get_world_tick(lua_State* L)
{
	DWORD dwTick = g_world.GetWorldTick();

	lua_pushnumber(L, dwTick);

	return 1;
}

int script_get_server_index(lua_State* L)
{
	DWORD dwSessionID	=	World::p_var->get_dword(_T("section_id world"));
	DWORD dwWorldID		=	World::p_var->get_dword(_T("id world"));

	lua_pushnumber(L, dwSessionID);
	lua_pushnumber(L, dwWorldID);

	return 2;
}

int script_get_open_server_day(lua_State* L)
{
	DWORD dwDay = g_world.get_open_server_day();

	lua_pushnumber(L, dwDay);

	return 1;
}

int script_get_role_level_rank(lua_State* L)
{
	std::vector<DWORD> vecRoleID;
	RankMgr::GetInstance()->GetLevelRank(vecRoleID);
	
	lua_newtable(L);
	for( std::size_t i = 0; i < vecRoleID.size(); i++ )
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, vecRoleID[i]);
		lua_settable(L, -3);
	}

	return 1;
}

int script_send_award_message(lua_State* L)
{
	DWORD   dw_role_id		=	lua_tonumber(L, 1);
	DWORD   dw_item_id		=	lua_tonumber(L, 2);
	INT		nValue			=	lua_tointeger(L, 3);

	HearSayHelper::SendMessage(EHST_BAOXIANGYUANBAO,
		dw_role_id, dw_item_id,  nValue,   INVALID_VALUE, INVALID_VALUE);
	
	return 0;
}

int script_shihun_give_reward(lua_State* L)
{
	package_list<tagShihunRank*>::list_iter iter = RankMgr::GetInstance()->GetShiHunRank().begin();
	tagShihunRank* pShihun = NULL;

	lua_newtable(L);

	INT n_index = 0;
	while(RankMgr::GetInstance()->GetShiHunRank().find_next(iter, pShihun))
	{
		if(!VALID_POINT(pShihun))
			continue;

		lua_pushnumber(L, n_index);
		lua_newtable(L);
		lua_pushnumber(L, pShihun->dw_role_id);
		lua_pushnumber(L, pShihun->dw_shihun);
		lua_settable(L, -3);
		lua_settable(L, -3);
		n_index++;
	}
	
	return 1;
}

int script_get_pet_ernie_level(lua_State* L)
{
	INT level = AttRes::GetInstance()->GetVariableLen( ).pet_ernie_level;
	lua_pushinteger(L, level);
	return 1;
}

int script_get_zhanli_role_level(lua_State* L)
{
	DWORD dw_class = lua_tonumber(L, 1);
	DWORD dw_sex = lua_tonumber(L, 2);


	INT32 nLevel = RankMgr::GetInstance()->getZhanliLevel(dw_class, dw_sex);

	lua_pushinteger(L, nLevel);

	return 1;
		
}

int script_send_hearsay_message(lua_State* L)
{
	EHearSayType dw_msg_type=	(EHearSayType)lua_tointeger(L, 1);
	DWORD   dw_role_id		=	lua_tonumber(L, 2);
	DWORD   dw_param_1		=	lua_tonumber(L, 3);
	DWORD   dw_param_2		=	lua_tonumber(L, 4);
	DWORD   dw_param_3		=	lua_tonumber(L, 5);
	DWORD   dw_param_4		=	lua_tonumber(L, 6);

	if(dw_msg_type < EHST_START || dw_msg_type >= EHST_MAX)
		return 0;

	HearSayHelper::SendMessage(dw_msg_type,
		dw_role_id, dw_param_1, dw_param_2, dw_param_3, dw_param_4);

	return 0;
}

int script_send_guild_boss_message(lua_State* L)
{
	DWORD   guildID			=	lua_tonumber(L, 1);
	DWORD   cretypeID		=	lua_tonumber(L, 2);
	
	guild* pGuild = g_guild_manager.get_guild(guildID);
	if (!VALID_POINT(pGuild))
		return 0;

	TCHAR sz_buff[X_LONG_NAME] = _T("");
	tstring stname = pGuild->get_guild_att().str_name;
	_tcsncpy(sz_buff, stname.c_str(), stname.size());

	HearSayHelper::SendMessage(EHST_GUILDFIRSTKILL,
		INVALID_VALUE, guildID,  cretypeID, INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, (stname.length() + 1) * sizeof(TCHAR));

	return 0;
}
static int script_is_exist_item(lua_State* L)
{
	DWORD dw_item_id = lua_tonumber(L, 1);

	BOOL b_have = FALSE;

	const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dw_item_id);
	if(VALID_POINT(pItemProto))
		b_have = TRUE;

	lua_pushinteger(L, b_have);

	return 1;
}

static int script_is_exist_creature(lua_State* L)
{
	DWORD dw_creature_id = lua_tonumber(L, 1);

	BOOL b_have = FALSE;

	const tagCreatureProto* pCreatrueProto = AttRes::GetInstance()->GetCreatureProto(dw_creature_id);
	if(VALID_POINT(pCreatrueProto))
		b_have = TRUE;

	lua_pushinteger(L, b_have);
	return 1;
}

//-------------------------------------------------------------------------------------
//记录脚本log
//-------------------------------------------------------------------------------------
int script_write_log(lua_State* L)
{
	SI_LOG->write_log(get_tool()->ansi_to_unicode(lua_tostring(L, 1)));
	return 0;
}

//-------------------------------------------------------------------------------------
// 转换时间为DWORD
//-------------------------------------------------------------------------------------
int script_data_time_to_dword_time(lua_State* L)
{
	BYTE	byYear		=	lua_tonumber(L, 1);
	BYTE	byMonth		=	lua_tonumber(L, 2);
	BYTE	byDay		=	lua_tonumber(L, 3);
	BYTE	byHour		=	lua_tonumber(L, 4);
	BYTE	byMin		=	lua_tonumber(L, 5);
	BYTE	bySec		=	lua_tonumber(L, 6);

	if( bySec < 0 || bySec > 59 || byMin < 0 || byMin > 59 || byHour < 0 || byHour > 23 ||
		byDay < 0 || byDay > 31 || byMonth <  0 || byMonth > 12 || byYear < 0 || byYear > 63 )
	{
		ASSERT(0);
		return 0;
	}

	DWORD	dwDWORDTime = tagDWORDTime(bySec, byMin, byHour, byDay, byMonth, byYear);

	lua_pushnumber(L, dwDWORDTime);

	return 1;
}

//-------------------------------------------------------------------------------------
// 得到当前DWORD时间
//-------------------------------------------------------------------------------------
int script_get_current_dword_time(lua_State* L)
{
	DWORD	dwCurTime = GetCurrentDWORDTime();
	lua_pushnumber(L, dwCurTime);

	return 1;
}

//-------------------------------------------------------------------------------------
// 计算两个日期的时间差，返回秒
//-------------------------------------------------------------------------------------
int script_cal_time_diff(lua_State* L)
{
	DWORD	dwDestTime		=	lua_tonumber(L, 1);
	DWORD	dwSrcTime		=	lua_tonumber(L, 2);
	
	DWORD dwSecond = CalcTimeDiff(dwDestTime, dwSrcTime);
	lua_pushnumber(L, dwSecond);

	return 1;
}

//-------------------------------------------------------------------------------------
// 让某个时间加上某个值
//-------------------------------------------------------------------------------------
int script_increase_time(lua_State* L)
{
	DWORD	dwSrcTime		=	lua_tonumber(L, 1);
	INT		nIncSecond		=	lua_tonumber(L, 2);

	DWORD	dwDestTime = IncreaseTime(dwSrcTime, nIncSecond);	
	lua_tonumber(L, dwDestTime);

	return 1;
}

//-------------------------------------------------------------------------------------
// 时间上递减指定秒数
//-------------------------------------------------------------------------------------
int script_decrease_time(lua_State* L)
{
	DWORD	dwSrcTime		=	lua_tonumber(L, 1);
	INT		nDecSecond		=	lua_tonumber(L, 2);

	DWORD	dwDestTime = DecreaseTime(dwSrcTime, nDecSecond);
	lua_tonumber(L, dwDestTime);

	return 1;
}

//-------------------------------------------------------------------------------------
// 玩家和怪物共有的接口函数
//-------------------------------------------------------------------------------------
int script_change_HP(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	INT		nHP				=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	pUnit->ChangeHP(nHP);

	return 0;
}

int script_change_MP(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	INT		nMP				=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	pUnit->ChangeMP(nMP);

	return 0;
}

//-------------------------------------------------------------------------------------
// 得到Unit所在地图MapCrc和坐标
//-------------------------------------------------------------------------------------
int script_get_position(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	INT nX = pUnit->GetCurPos().x / TILE_SCALE;
	INT nY = pUnit->GetCurPos().y;
	INT nZ = pUnit->GetCurPos().z / TILE_SCALE;

	lua_pushinteger(L, nX);
	lua_pushinteger(L, nY);
	lua_pushinteger(L, nZ);

	return 3;
}

int script_is_dead(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if(!VALID_POINT(pUnit))	return 0;

	BOOL bDead = pUnit->IsDead();
	lua_pushboolean(L, bDead);

	return 1;
}

int script_is_player(lua_State* L)
{
	DWORD	dwUnitID		=	lua_tonumber(L, 1);
	BOOL	bPlayer			=	FALSE;

	if(IS_PLAYER(dwUnitID))
		bPlayer = TRUE;

	lua_pushboolean(L, bPlayer);

	return 1;
}

int script_is_creature(lua_State* L)
{
	DWORD	dwUnitID		=	lua_tonumber(L, 1);
	BOOL	bCreature		=	FALSE;

	if(IS_CREATURE(dwUnitID) || IS_PET(dwUnitID))
		bCreature = TRUE;

	lua_pushboolean(L, bCreature);

	return 1;
}

//-------------------------------------------------------------------------------------
// 让Unit停止移动
//-------------------------------------------------------------------------------------
int script_stop_move(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	BOOL	bSendMsg		=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	pUnit->GetMoveData().StopMove(bSendMsg);

	return 0;
}

//-------------------------------------------------------------------------------------
// 让Unit停止攻击
//-------------------------------------------------------------------------------------
int script_stop_attack(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	pUnit->GetCombatHandler().End();

	return 0;
}

//-------------------------------------------------------------------------------------
// 单位是否在战斗中，只适用于creature
//-------------------------------------------------------------------------------------
int script_is_attack(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) )
	{
		return 0;
	}

	// 仇恨列表为空
	if (pCreature->GetAI()->IsEnmityListEmpty())
	{
		lua_pushboolean(L, false);	
	}
	else
	{
		lua_pushboolean(L, true);
	}

	return 1;

}

//-------------------------------------------------------------------------------------
// 敌我判断
//-------------------------------------------------------------------------------------
int script_friend_enemy(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwSrcID			=	lua_tonumber(L, 3);
	DWORD	dwDstID			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pSrc = pMap->find_unit(dwSrcID);
	if( !VALID_POINT(pSrc) ) return 0;

	Unit* pDest = pMap->find_unit(dwDstID);
	if( !VALID_POINT(pDest) ) return 0;

	DWORD dwFriendEnemy = pSrc->FriendEnemy(pDest);

	lua_pushnumber(L, dwFriendEnemy);

	return 1;
}
//-------------------------------------------------------------------------------------
// 得到两个单位的距离
//-------------------------------------------------------------------------------------
int script_get_unit_distince(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID1		=	lua_tonumber(L, 3);
	DWORD	dwUnitID2		=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit1 = pMap->find_unit(dwUnitID1);
	if( !VALID_POINT(pUnit1) ) return 0;

	Unit* pUnit2 = pMap->find_unit(dwUnitID2);
	if( !VALID_POINT(pUnit2) ) return 0;

	Vector3 v1 = pUnit1->GetCurPos();
	v1.y = 0;
	Vector3 v2 = pUnit2->GetCurPos();
	v2.y = 0; 
	float fDis = Vec3Dist(v1, v2);

	lua_pushnumber(L, fDis);

	return 1;

}

//-------------------------------------------------------------------------------------
// 玩家范围事件
//-------------------------------------------------------------------------------------
int script_set_range_event(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	FLOAT	fRadius			=	lua_tonumber(L, 4);
	DWORD	dwEventType		=	lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	tagVisTile* pVisTile[EUD_end] = {0};
	// 得到攻击范围内的vistile列表
	pUnit->get_map()->get_visible_tile(pUnit->GetCurPos(), fRadius, pVisTile);
	Role*		pRole		= NULL;
	Creature*	pCreature	= NULL;
	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 首先检测人物
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			if( pRole == pUnit ) continue;

			// 距离判断
			FLOAT fDistSQ = Vec3DistSq(pRole->GetCurPos(), pUnit->GetCurPos());
			if( fDistSQ > fRadius  ) continue;


			const RoleScript* pRoleScript = g_ScriptMgr.GetRoleScript();
			if (VALID_POINT(pRoleScript))
			{
				pRoleScript->OnRangeEvent(pRole, dwEventType);
			}

		}
		// 检测怪物
		package_map<DWORD, Creature*>& mapCreatrue = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter itC = mapCreatrue.begin();

		while( mapCreatrue.find_next(itC, pCreature) )
		{
			if( pCreature == pUnit ) continue;
			
			// 距离判断
			FLOAT fDistSQ = Vec3DistSq(pUnit->GetCurPos(), pCreature->GetCurPos());
			if( fDistSQ > fRadius  ) continue;

			const CreatureScript* pCreatureScript = g_ScriptMgr.GetCreatureScript(pCreature->GetTypeID());
			if (VALID_POINT(pCreatureScript))
			{
				pCreatureScript->OnRangeEvent(pCreature, dwEventType);
			}

		}
	}
	return 0;
}
//-------------------------------------------------------------------------------------
// 得到当前移动状态
//-------------------------------------------------------------------------------------
int script_get_current_move_state(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	EMoveState ems = pUnit->GetMoveData().GetCurMoveState();
	
	lua_pushnumber(L, ems);

	return 1;
}

// 得到单位属性
int script_get_att_valule(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	INT		nIndex			=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if(!VALID_POINT(pUnit))	return 0;

	if( nIndex < 0 || nIndex >= ERA_End ) return 0;

	lua_pushinteger(L, pUnit->GetAttValue(nIndex));
	return 1;
}

// 增加单位属性
int script_mod_att_value(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	INT		nIndex			=	lua_tointeger(L, 4);
	INT		nValue			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if(!VALID_POINT(pUnit))	return 0;

	if( nIndex < 0 || nIndex >= ERA_End ) return 0;

	pUnit->ModAttValue(nIndex, nValue);

	return 0;
	
}

// 设置状态
static int script_set_state(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	EState	nState			=	(EState)lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if(!VALID_POINT(pUnit))	return 0;

	pUnit->SetState(nState);

	return 0;
}

// 取消状态
static int script_unset_state(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	EState	nState			=	(EState)lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if(!VALID_POINT(pUnit))	return 0;

	pUnit->UnsetState(nState);

	return 0;
}
//-------------------------------------------------------------------------------------
// 添加技能
//-------------------------------------------------------------------------------------
static int script_role_add_skill(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dwSkillTypeID	=	lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	const tagSkillProto* pProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	if( !VALID_POINT(pProto) ) return 0;

	DWORD	dwSkillID	= Skill::GetIDFromTypeID(dwSkillTypeID);
	INT		nLevel		= Skill::GetLevelFromTypeID(dwSkillTypeID);
	
	if (VALID_POINT(pRole->GetSkill(dwSkillID)))
		return 0;

	Skill*  pSkill = new Skill(dwSkillID, nLevel, 0, 0, 0, 0);
	pRole->AddSkill(pSkill);

	return 0;
}

//删除技能
static int script_role_remove_skill(lua_State* L)
{	
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dwSkillTypeID	=	lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	const tagSkillProto* pProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	if( !VALID_POINT(pProto) ) return 0;

	DWORD	dwSkillID	= Skill::GetIDFromTypeID(dwSkillTypeID);

	pRole->RemoveSkill(dwSkillID);
	return 0;
}
//-------------------------------------------------------------------------------------
// 是否已经学会指定技能
//-------------------------------------------------------------------------------------
static int script_is_role_learn_skill(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	DWORD	dwSkillID	=	lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )	return 0;

	Skill* pSkill = pRole->GetSkill(dwSkillID);	
	if (!VALID_POINT(pSkill))	return 0;

	lua_pushinteger(L, pSkill->get_level());

	return 1;
}

int script_add_buff(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	DWORD	dwBuffTypeID	=	lua_tonumber(L, 4);
	DWORD	dwSrcUnitID		=	lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	Unit* pSrcUnit = pMap->find_unit(dwSrcUnitID);

	const tagBuffProto* pProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
	if( !VALID_POINT(pProto) ) return 0;

	pUnit->TryAddBuff(pSrcUnit, pProto, NULL, NULL, NULL);

	return 0;
}

int script_cancel_buff(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	DWORD	dwBuffTypeID	=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	//pUnit->CancelBuff(dwBuffTypeID);
	pUnit->RemoveBuff(Buff::GetIDFromTypeID(dwBuffTypeID), TRUE);

	return 0;
}

int script_have_buff(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	DWORD	dwBuffID		=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;

	Buff* pBuff = pUnit->GetBuffPtr(dwBuffID);
	if (VALID_POINT(pBuff))
	{
		lua_pushinteger(L, pBuff->get_level());
	}
	else
	{
		lua_pushboolean(L, 0);
	}
	
	return 1;
}

int script_get_buff_warp_times(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwUnitID		=	lua_tonumber(L, 3);
	DWORD	dwBuffTypeID	=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Unit* pUnit = pMap->find_unit(dwUnitID);
	if( !VALID_POINT(pUnit) ) return 0;
	
	int nBuffWarp = 0;
	Buff* pBuff = pUnit->GetBuffPtr(Buff::GetIDFromTypeID(dwBuffTypeID));
	if (VALID_POINT(pBuff))
	{
		nBuffWarp = pBuff->GetWrapTimes();
	}
	lua_pushinteger(L, nBuffWarp);
	return 1;
}

//-------------------------------------------------------------------------------------
// 玩家独有函数
//-------------------------------------------------------------------------------------
int script_role_add_quest(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	// 添加任务
	const tagQuestProto* pProto = g_questMgr.get_protocol(u16QuestID);
	if( !VALID_POINT(pProto) )	return 0;

	INT nIndex;
	for(nIndex = 0; nIndex < QUEST_MAX_COUNT; ++nIndex)
	{
		if( FALSE == pRole->quest_valid(nIndex) )
		{
			break;
		}
	}
	if(pRole->is_have_quest(u16QuestID))
	{
		return 0;
	}
	if( nIndex >= QUEST_MAX_COUNT )
	{
		return 0;
	}

	pRole->add_quest(pProto, nIndex);

	//pRole->accept_quest_from_npc(u16QuestID, INVALID_VALUE);

	return 0;
}

int script_role_all_quest_event(lua_State* L)
{
	DWORD dw_role_id	=	lua_tonumber(L, 1);
	DWORD dw_event_id = lua_tonumber(L, 2);
	DWORD dw_param_1 = lua_tonumber(L, 3);
	DWORD dw_param_2 = lua_tonumber(L, 4);
	DWORD dw_param_3 = lua_tonumber(L, 5);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	if(dw_event_id == EQE_Kill)
		pRole->on_quest_event((EQuestEvent)dw_event_id, dw_param_1, dw_param_2, dw_param_3);

	return 0;
}

int script_role_quest_event(lua_State* L)
{
	DWORD dw_role_id	=	lua_tonumber(L, 1);
	DWORD dw_quest_id	=	lua_tonumber(L, 2);
	DWORD dw_event_id = lua_tonumber(L, 3);
	DWORD dw_param_1 = lua_tonumber(L, 4);
	DWORD dw_param_2 = lua_tonumber(L, 5);
	DWORD dw_param_3 = lua_tonumber(L, 6);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest((UINT16)dw_quest_id);
	if(!VALID_POINT(pQuest)) return 0;

	if(dw_event_id == EQE_Kill)
		pQuest->on_event((EQuestEvent)dw_event_id, dw_param_1, dw_param_2, dw_param_3);

	return 0;
}

int script_role_quest_event_effective(lua_State* L)
{
	DWORD dw_role_id	=	lua_tonumber(L, 1);
	DWORD dw_quest_id	=	lua_tonumber(L, 2);
	DWORD dw_event_id = lua_tonumber(L, 3);
	DWORD dw_param_1 = lua_tonumber(L, 4);
	DWORD dw_param_2 = lua_tonumber(L, 5);
	DWORD dw_param_3 = lua_tonumber(L, 6);
	DWORD dw_param_4 = lua_tonumber(L, 7);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest((UINT16)dw_quest_id);
	if(!VALID_POINT(pQuest)) return 0;

	if(dw_event_id == EQE_Kill)
		pQuest->on_event((EQuestEvent)dw_event_id, dw_param_1, dw_param_2, dw_param_3, dw_param_4);

	return 0;
}


int script_delete_quest(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	// 删除任务
	pRole->remove_quest(u16QuestID, FALSE);

	// 发送返回消息给客户端
	NET_SIS_delete_quest send;
	send.u16QuestID = u16QuestID;
	send.dw_error_code = E_Success;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

int script_role_complete_quest(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);
	DWORD	dwNPCID		=	lua_tonumber(L, 3);
	UINT16	u16NextQuestID = 0;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	INT nRet = pRole->complete_quest(u16QuestID, dwNPCID, -1, u16NextQuestID);

	// 发送返回
	NET_SIS_complete_quest send;
	send.u16QuestID = u16QuestID;
	send.dw_error_code = nRet;
	pRole->SendMessage(&send, send.dw_size);

	// 检测是否有后续自动接取的任务
	if(E_Success == nRet && u16NextQuestID != 0)
		pRole->accept_quest_from_npc(u16NextQuestID, INVALID_VALUE);

	return 0;
}

int script_set_quest_overview_message_id(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);
	UINT32	u32MsgID	=	(UINT32)lua_tointeger(L, 3);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest( u16QuestID );
	if( !VALID_POINT(pQuest) ) return 0;


	tagScriptQuestVar* pScriptVar = pQuest->get_script_var( );
	if( !VALID_POINT(pScriptVar) ) return 0;

	pScriptVar->SetOverviewMsgID( u32MsgID );

	return 0;
}
int script_add_quest_variable(lua_State* L)
{
	DWORD	dw_role_id	= lua_tonumber(L, 1);
	UINT16	u16QuestID	= (UINT16)lua_tointeger(L, 2);
	UINT16  u16VarID	= (UINT16)lua_tointeger(L, 3);
	UINT32  u32TargetMsgID = (UINT32)lua_tointeger(L, 4);
	INT16	n16VarInit	 = (INT16)lua_tointeger(L, 5);
	INT16   n16Comp = (INT16)lua_tointeger(L, 6);
	INT16	n16Fail	= (INT16)lua_tointeger(L, 7);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest( u16QuestID );
	if( !VALID_POINT(pQuest) ) return 0;

	tagScriptQuestVar* pScriptVar = pQuest->get_script_var( );
	if( !VALID_POINT(pScriptVar) ) return 0;

	if(  !pScriptVar->AddVar( u16VarID, u32TargetMsgID, n16VarInit, n16Comp, n16Fail) )
		print_message(_T("参数个数>%s\n\n"), SCRIPTQUEST_VARIABLE_MAX );

	return 0;
}
int script_inc_quest_variable(lua_State* L)
{
	DWORD	dw_role_id	= lua_tonumber(L, 1);
	UINT16	u16QuestID	= (UINT16)lua_tointeger(L, 2);
	UINT16	u16VarID	= (UINT16)lua_tointeger(L, 3);
	INT16	n16Delta	= (INT16)lua_tointeger(L, 4);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest( u16QuestID );
	if( !VALID_POINT(pQuest) ) return 0;

	tagScriptQuestVar* pScriptVar = pQuest->get_script_var( );
	if( !VALID_POINT(pScriptVar) ) return 0;

	if( pScriptVar->IncVar(u16VarID, n16Delta) )
	{
		NET_SIS_quest_var_value send;
		send.u16QuestID = u16QuestID;
		send.VarID = u16VarID;
		pScriptVar->GetVar( u16VarID, send.VarValue );
		pRole->SendMessage( &send, send.dw_size );
	}

	if( pScriptVar->IsComplete( ) && !g_questMgr.is_need_complete_npc( u16QuestID ) )
	{//完成发送通知 
		UINT16	u16NextQuestID = 0;
		INT nRet = pRole->complete_quest(u16QuestID, INVALID_VALUE, -1, u16NextQuestID);

		// 发送返回
		NET_SIS_complete_quest send;
		send.u16QuestID = u16QuestID;
		send.dw_error_code = nRet;
		pRole->SendMessage(&send, send.dw_size);

		// 检测是否有后续自动接取的任务
		if(E_Success == nRet && u16NextQuestID != 0)
			pRole->accept_quest_from_npc(u16NextQuestID, INVALID_VALUE);
	}
	return 0;
}
int script_set_quest_variable(lua_State* L)
{
	DWORD	dw_role_id	= lua_tonumber(L, 1);
	UINT16	u16QuestID	= (UINT16)lua_tointeger(L, 2);
	UINT16	u16VarID	= (UINT16)lua_tointeger(L, 3);
	INT16	n16Value	= (INT16)lua_tointeger(L, 4);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest( u16QuestID );
	if( !VALID_POINT(pQuest) ) return 0;

	tagScriptQuestVar* pScriptVar = pQuest->get_script_var( );
	if( !VALID_POINT(pScriptVar) ) return 0;

	if( pScriptVar->SetVar(u16VarID, n16Value ) )
	{
		NET_SIS_quest_var_value send;
		send.u16QuestID = u16QuestID;
		send.VarID = u16VarID;
		pScriptVar->GetVar( u16VarID, send.VarValue );
		pRole->SendMessage( &send, send.dw_size );
	}

	if( pScriptVar->IsFail( ) )
	{//任务失败
		NET_SIS_quest_faild send;
		send.u16QuestID= u16QuestID;
		pRole->SendMessage(&send, send.dw_size);
		pRole->remove_quest(u16QuestID, FALSE);
	}
	else if( pScriptVar->IsComplete( ) && !g_questMgr.is_need_complete_npc( u16QuestID ) )
	{//完成发送通知 
		UINT16	u16NextQuestID = 0;
		INT nRet = pRole->complete_quest(u16QuestID, INVALID_VALUE, -1, u16NextQuestID);

		// 发送返回
		NET_SIS_complete_quest send;
		send.u16QuestID = u16QuestID;
		send.dw_error_code = nRet;
		pRole->SendMessage(&send, send.dw_size);

		// 检测是否有后续自动接取的任务
		if(E_Success == nRet && u16NextQuestID != 0)
			pRole->accept_quest_from_npc(u16NextQuestID, INVALID_VALUE);
	}
	return 0;
}
int script_get_quest_variable(lua_State* L)
{
	DWORD	dw_role_id	= lua_tonumber(L, 1);
	UINT16	u16QuestID	= (UINT16)lua_tointeger(L, 2);
	UINT16	u16VarID	= (UINT16)lua_tointeger(L, 3);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest( u16QuestID );
	if( !VALID_POINT(pQuest) ) return 0;

	tagScriptQuestVar* pScriptVar = pQuest->get_script_var( );
	if( !VALID_POINT(pScriptVar) ) return 0;

	INT16 u16Value;
	if(!pScriptVar->GetVar( u16VarID, u16Value ))
		print_message(_T("玩家%d任务%d变量ID: %d 不存在!\n\n"), dw_role_id, u16QuestID, u16VarID);

	lua_pushnumber( L, u16Value );

	return 1;
}
//-------------------------------------------------------------------------------------
// 玩家独有函数
//-------------------------------------------------------------------------------------
int script_quest_init(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);
	
	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest(u16QuestID);
	if(!VALID_POINT(pQuest)) return 0;

	tagQuestDynamicTarget* pTarget = pQuest->get_dynamic_target();
	if(!VALID_POINT(pTarget)) return 0;

	pTarget->eTargetType = (EQuestTargetType)lua_tointeger(L, 3);	// 任务动态目标类型
	pTarget->dwQuestTipsID = lua_tonumber(L, 4);

	if(EQTT_NPCTalk	== pTarget->eTargetType || EQTT_Invest == pTarget->eTargetType)
	{
		for(INT i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			pTarget->dwTargetID[i] = lua_tonumber(L ,i + 5);
		}
	}
	else
	{
		for(INT n = 0; n < DYNAMIC_TARGET_COUNT; ++n)
		{
			pTarget->dwTargetID[n] = lua_tonumber(L, (2*n)+5);
			pTarget->nTargetNum[n] = lua_tonumber(L, (2*n)+6);
		}
	}

	return 0;
}
/*
* 参数列表:
	1.角色ID
	2.任务ID
	3.是否可选
	4.奖励的物品ID
	5.奖励物品数量
	6.奖励物品品级
*/
int script_quest_init_reward( lua_State* L )
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	UINT16	u16QuestID	=	(UINT16)lua_tointeger(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest(u16QuestID);
	if(!VALID_POINT(pQuest)) return 0;

	tagQuestDynamicReward* pReward = pQuest->get_dynamic_reward();
	if(!VALID_POINT(pReward)) return 0;

	INT16 n16IsChoice = (INT16)lua_tointeger(L, 3);	
	DWORD dwItemID = (DWORD)lua_tonumber(L, 4);
	INT16 n16ItemNum = (INT16)lua_tonumber(L, 5);
	INT16 n16ItemQuality = (INT16)lua_tonumber(L, 6);
	BOOL bBind = (BOOL)lua_tonumber(L, 7);
	

	if( VALID_POINT(n16IsChoice) )
	{//可选奖励
		for(INT16 n = 0; n < QUEST_REW_ITEM; ++n )
		{
			if( !VALID_POINT(pReward->rew_choice_item[n]))
			{
				pReward->rew_choice_item[n] = dwItemID;
				pReward->rew_choice_item_num[n] = n16ItemNum;
				pReward->rew_choice_quality[n] = n16ItemQuality;
				pReward->rew_choice_bind[n] = bBind;
				break;
			}
		}
	}
	else
	{//不可选奖励
		for(INT16 n = 0; n < QUEST_REW_ITEM; ++n )
		{
			if( !VALID_POINT(pReward->rew_item[n]))
			{
				pReward->rew_item[n] = dwItemID;
				pReward->rew_item_num[n] = n16ItemNum;
				pReward->rew_item_quality[n] = n16ItemQuality;
				pReward->rew_item_bind[n] = bBind;
				break;
			}
		}
	}
	return 0;
}

int script_role_goto_new_map(lua_State* L)		// 去一个新地图
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	DWORD	dwDestMapID		=	lua_tonumber(L, 4);
	FLOAT	fx				=	lua_tonumber(L, 5);
	FLOAT	fy				=	lua_tonumber(L, 6);
	FLOAT	fz				=	lua_tonumber(L, 7);

	fx = fx * TILE_SCALE;
	fy = fy * TILE_SCALE;
	fz = fz * TILE_SCALE;

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	pRole->GotoNewMap(dwDestMapID, fx, fy, fz);

	return 0;
}
//-------------------------------------------------------------------------------------
// 根据roleid得到玩家地图id gx add 2013.10.24
//-------------------------------------------------------------------------------------
int script_get_role_mapid(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )
	{
		return 0;
	}
	lua_pushnumber(L, pRole->GetMapID());
	return 1;
}

//-------------------------------------------------------------------------------------
// 得到玩家等级
//-------------------------------------------------------------------------------------
int script_get_role_level(lua_State *L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	lua_pushinteger(L, pRole->get_level());
	return 1;
}

int script_set_role_level(lua_State *L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	INT		nLevel			=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if ( nLevel > 0 && nLevel <= ROLE_LEVEL_LIMIT)
		pRole->LevelChange(nLevel, FALSE, TRUE);

	return 0;
}


//-------------------------------------------------------------------------------------
// 读取玩家当前属性
//-------------------------------------------------------------------------------------
int script_get_role_att_value(lua_State *L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	INT		nIndex			=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nIndex < 0 || nIndex >= ERA_End ) return 0;

	lua_pushinteger(L, pRole->GetAttValue(nIndex));
	return 1;
}
//-------------------------------------------------------------------------------------
// 读取玩家职业
//-------------------------------------------------------------------------------------
int script_get_role_class(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;
	
	lua_pushinteger(L, pRole->GetClass());

	return 1;
}
//-------------------------------------------------------------------------------------
// 修改玩家当前属性
//-------------------------------------------------------------------------------------
int script_mod_role_att_value(lua_State *L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	INT		nIndex			=	lua_tointeger(L, 4);
	INT		nValue			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nIndex < 0 || nIndex >= ERA_End ) return 0;

	pRole->ModAttValue(nIndex, nValue);

	return 0;
};

//-------------------------------------------------------------------------------------
// 玩家获得任务物品装备
//-------------------------------------------------------------------------------------
int script_quest_add_role_item(lua_State* L)
{
	DWORD			dwMapID			=	lua_tonumber(L, 1);
	DWORD			dwInstanceID	=	lua_tonumber(L, 2);
	DWORD			dw_role_id		=	lua_tonumber(L, 3);
	UINT16			u16QuestID		=   (UINT16)lua_tointeger(L, 4);		// 任务ID
	DWORD			dw_data_id		=	lua_tonumber(L, 5);					// 物品TypeID
	INT				n_num			=	lua_tointeger(L, 6);				// 物品数量
	EItemQuality	eQuality		=	(EItemQuality)lua_tointeger(L, 7);	// 物品品质
	e_log_cmd_id	eLogCmdID		=	(e_log_cmd_id)lua_tointeger(L, 8);		// 记录log类型
	BOOL  bBind						=	lua_toboolean(L, 9);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;
	
	pRole->GetItemMgr().Add2Role(EICM_Quest, u16QuestID, dw_data_id, (INT16)n_num, eQuality, eLogCmdID, bBind);

	return 0;
}

//-------------------------------------------------------------------------------------
// 玩家获得物品装备
//-------------------------------------------------------------------------------------
static int script_add_role_item(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	DWORD dw_data_id		=	lua_tonumber(L, 4);
	INT16 n16Num		=	lua_tointeger(L, 5);
	INT32 nQlty			=	lua_tointeger(L, 6);
	INT   nCreateMode	=	lua_tointeger(L, 7);
	INT	  nLogCmdID		=	lua_tointeger(L, 8);
	BOOL  bBind			=	lua_toboolean(L, 9);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if(n16Num <= 0)
	{
		n16Num = 1;
	}

	tagItem *pNewItem = ItemCreator::Create((EItemCreateMode)nCreateMode, dw_role_id, dw_data_id, n16Num);
	if(!VALID_POINT(pNewItem))
	{
		return 0;
	}

	if(MIsEquipment(pNewItem->dw_data_id))
	{
		if(nQlty == INVALID_VALUE)
		{
			// 不鉴定	
		}
		else
		{
			ItemCreator::IdentifyEquip((tagEquip*)pNewItem, (EItemQuality)nQlty);
		}
	}

	if (bBind)
	{
		pNewItem->byBind = EBS_Bind;
		pNewItem->bCreateBind = (bBind == TRUE);
	}

	pRole->GetItemMgr().Add2Bag(pNewItem, (e_log_cmd_id)nLogCmdID, TRUE);

	//lua_pushnumber(L, pNewItem->n64_serial);
	push_64bit_data(L, pNewItem->n64_serial);
	lua_pushinteger(L, pNewItem->n16Index);

	if(!MIsEquipment(dw_data_id))
	{
		return 3;
	}

	lua_pushinteger(L, pNewItem->pEquipProto->eEquipPos);

	return 4;
}

//-------------------------------------------------------------------------------------
// 玩家穿上装备
//-------------------------------------------------------------------------------------
int script_role_equip(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	//INT64 n64_serial		=	lua_tonumber(L, 4);
	INT64 n64_serial		=	pop_64bit_data(L, 4, 5);
	INT16 n16EquipPos	=	lua_tonumber(L, 6);
	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	pRole->Equip(n64_serial, (EEquipPos)n16EquipPos);

	return 0;
}


// 获取玩家某个位置的装备
int script_get_role_equip(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	INT16 n16EquipPos	=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	tagEquip *pEquip = pRole->GetItemMgr().GetEquipBarEquip(n16EquipPos);
	
	if (VALID_POINT(pEquip))
	{
		lua_pushinteger(L, pEquip->pEquipProto->dw_data_id);

		return 1;
	}

	return 0;
}

// 获取装备等级
int script_get_role_equip_level(lua_State* L)
{
	//DWORD dwMapID		=	lua_tonumber(L, 1);
	//DWORD dwInstanceID	=	lua_tonumber(L, 2);
	//DWORD dw_role_id		=	lua_tonumber(L, 3);
	//INT16 n16EquipPos	=	lua_tonumber(L, 4);

	//Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	//if(!VALID_POINT(pMap))	return 0;

	//Role* pRole = pMap->find_role(dw_role_id);
	//if(!VALID_POINT(pRole))	return 0;

	//tagEquip *pEquip = pRole->GetItemMgr().GetEquipBarEquip(n16EquipPos);

	//if (VALID_POINT(pEquip))
	//{
	//	lua_pushinteger(L, pEquip->equipSpec.nLevel);

	//	return 1;
	//}

	return 0;
}
//-------------------------------------------------------------------------------------
// 从背包或任务栏删除物品
//-------------------------------------------------------------------------------------
int script_remove_from_role(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	DWORD dw_data_id		=	lua_tonumber(L, 4);
	INT16 n16Num		=	lua_tointeger(L, 5);
	INT	  nLogCmdID		=	lua_tointeger(L, 6);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT nRet = pRole->GetItemMgr().RemoveFromRole(dw_data_id, n16Num, nLogCmdID);

	lua_pushinteger(L, nRet);

	return 1;
}
//-------------------------------------------------------------------------------------
// 从背包删除物品，通过serverID gx add
//-------------------------------------------------------------------------------------
int script_remove_from_role_n64serial(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	INT64	n64_serial		=	pop_64bit_data(L, 4, 5);
	INT16 n16Num		=	lua_tointeger(L, 6);
	INT	  nLogCmdID		=	lua_tointeger(L, 7);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT nRet = pRole->GetItemMgr().DelFromBag(n64_serial,nLogCmdID,n16Num);
	return 0;
}
int script_remove_from_role_bind(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	DWORD dw_data_id		=	lua_tonumber(L, 4);
	INT16 n16Num		=	lua_tointeger(L, 5);
	INT	  nLogCmdID		=	lua_tointeger(L, 6);
	BOOL	bBind		=	lua_toboolean(L, 7);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT nRet = pRole->GetItemMgr().RemoveFromRole(dw_data_id, n16Num, nLogCmdID, bBind);

	lua_pushinteger(L, nRet);

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家背包空闲空间数量
//-------------------------------------------------------------------------------------
int script_get_bag_free_size(lua_State* L)
{
	DWORD dw_role_id		=	lua_tonumber(L, 1);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	ItemMgr& itemMgr = pRole->GetItemMgr();	

	INT nFreeSize = itemMgr.GetBagFreeSize();

	lua_pushinteger(L, nFreeSize);

	return 1;
}
//-------------------------------------------------------------------------------------
// 获得玩家背包和仓库中物品的数量，考虑绑定与否
//-------------------------------------------------------------------------------------
int script_get_item_number_from_kind(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dw_data_id		=	lua_tonumber(L, 2);
	INT16	n_bag_type		=	lua_tonumber(L, 3);//0:全部；1：背包；2：仓库
	//BOOL	n_bind			=	lua_tonumber(L, 4);//1:绑定；0：不绑定
	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) 
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	INT nNum_Bag = 0;//背包中该物品的总数
	INT nNum_Storage = 0;//仓库中该物品的总数
	INT nTotal = 0;
	ItemMgr& itemMgr = pRole->GetItemMgr();

	if (2 == n_bag_type)
	{
		nNum_Storage = itemMgr.GetRoleWare().GetSameItemCount(dw_data_id);
	}
	else if (1 == n_bag_type)
	{
		nNum_Bag = itemMgr.GetBagSameItemCount(dw_data_id);
	}
	else
	{
		nNum_Storage = itemMgr.GetRoleWare().GetSameItemCount(dw_data_id);
		nNum_Bag = itemMgr.GetBagSameItemCount(dw_data_id);
	}

	nTotal = nNum_Bag+nNum_Storage;

	lua_pushinteger(L, nTotal);

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家背包和任务背包中物品的数量
//-------------------------------------------------------------------------------------
int script_get_role_item_number(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dw_data_id		=	lua_tonumber(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) 
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	ItemMgr& itemMgr = pRole->GetItemMgr();

	INT nTotal = itemMgr.GetBagSameItemCount(dw_data_id) + itemMgr.GetQuestBagSameItemCount(dw_data_id);

	lua_pushinteger(L, nTotal);

	return 1;
}
//-------------------------------------------------------------------------------------
// 获得玩家背包中物品(serverID)的数量 gx add 2013.9.2
//-------------------------------------------------------------------------------------
int script_get_role_item_n64Serial_number(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	INT64	n64_serial		=	pop_64bit_data(L, 2, 3);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) 
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	tagItem *p_Item = pRole->GetItemMgr().GetBagItem(n64_serial);;
	if (!VALID_POINT(p_Item))
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, p_Item->GetNum());

	return 1;
}
// 获取背包物品数量,关心是否绑定的
int script_get_role_item_bind_number(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dw_data_id		=	lua_tonumber(L, 2);
	BOOL	bBind			=	lua_toboolean(L, 3);
	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) 
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	ItemMgr& itemMgr = pRole->GetItemMgr();

	INT nTotal = itemMgr.GetBagSameBindItemCount(dw_data_id, bBind);

	lua_pushinteger(L, nTotal);

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家名字
//-------------------------------------------------------------------------------------
int script_get_role_name(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);

	TCHAR szName[X_SHORT_NAME];
	g_roleMgr.get_role_name(dw_role_id, szName);

	lua_pushstring(L, get_tool()->unicode_to_ansi(szName));

	return 1;
}

//-------------------------------------------------------------------------------------
// 玩家是否有小队
//-------------------------------------------------------------------------------------
int script_is_role_have_team(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	lua_pushnumber(L, pRole->GetTeamID());

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家小队成员ID
//-------------------------------------------------------------------------------------
int script_get_role_team_member_id(lua_State* L)
{
	DWORD	dwTeamID		=	lua_tonumber(L, 1);
	
	const Team *pTeam = g_groupMgr.GetTeamPtr(dwTeamID);
	if(!VALID_POINT(pTeam))	return 0;

	DWORD dwMemID[MAX_TEAM_NUM];
	memset(dwMemID, 0xFF, sizeof(dwMemID));

	pTeam->export_all_member_id(dwMemID);

	for (INT i = 0; i < MAX_TEAM_NUM; ++i)
	{
		lua_pushnumber(L, dwMemID[i]);
	}

	return MAX_TEAM_NUM;
}

//-------------------------------------------------------------------------------------
// 设置任务脚本通用数据
//-------------------------------------------------------------------------------------
int script_set_quest_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	UINT16	u16QuestID		=   (UINT16)lua_tonumber(L, 2);		// 任务ID
	INT		nParamNum		=	lua_tointeger(L, 3);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest(u16QuestID);
	if(!VALID_POINT(pQuest)) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+4);
		dwValue		=	lua_tonumber(L, i*2+5);

		if( nIndex < 0 || nIndex >= ESD_Quest ) return 0;

		pQuest->SetScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 修改任务脚本通用数据
//-------------------------------------------------------------------------------------
int script_mod_quest_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	UINT16	u16QuestID		=   (UINT16)lua_tonumber(L, 2);		// 任务ID
	INT		nParamNum		=	lua_tointeger(L, 3);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest(u16QuestID);
	if(!VALID_POINT(pQuest)) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+4);
		dwValue		=	lua_tonumber(L, i*2+5);

		if( nIndex < 0 || nIndex >= ESD_Quest ) return 0;

		pQuest->ModScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 设置任务脚本通用数据
//-------------------------------------------------------------------------------------
int script_get_quest_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	UINT16	u16QuestID		=   (UINT16)lua_tonumber(L, 2);		// 任务ID
	INT		nParamNum		=	lua_tointeger(L, 3);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	quest* pQuest = pRole->get_quest(u16QuestID);
	if(!VALID_POINT(pQuest)) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex = lua_tointeger(L, i+4);		
		
		if( nIndex < 0 || nIndex >= ESD_Quest ) return 0;

		dwValue = pQuest->GetScriptData(nIndex);
		lua_pushnumber(L, dwValue);
	}

	return nParamNum;
}

//-------------------------------------------------------------------------------------
// 玩家是否接取了该任务
//-------------------------------------------------------------------------------------
int script_is_role_have_quest(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	UINT16	u16QuestID		=   (UINT16)lua_tonumber(L, 4);		// 任务ID

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	BOOL bHaveQuest = pRole->is_have_quest(u16QuestID);
	lua_pushboolean(L, bHaveQuest);

	return 1;
}
//--------------------------------------------------------------------
// 是否完成过该任务
//--------------------------------------------------------------------
int script_is_role_have_done_quest(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	UINT16	u16QuestID		=   (UINT16)lua_tonumber(L, 4);		// 任务ID

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	BOOL bHaveQuest = pRole->is_done_quest(u16QuestID);
	lua_pushboolean(L, bHaveQuest);

	return 1;
}
//-------------------------------------------------------------------------------------
// 增加玩家经验
//-------------------------------------------------------------------------------------
int script_add_role_exp(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nSpecial		=   lua_tointeger(L,5);//gx add 
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	pRole->ExpChange(nValue,FALSE,FALSE,nSpecial);

	return 0;
}

// 增加战功
int script_add_exploits(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().IncExploits(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}

// 减少战功
int script_dec_exploits(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().DecExploits(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}

//-------------------------------------------------------------------------------------
// 增加玩家金钱
//-------------------------------------------------------------------------------------
int script_add_role_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().IncBagSilver(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}

//-------------------------------------------------------------------------------------
// 减少玩家金钱
//-------------------------------------------------------------------------------------
int script_dec_role_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT64	n64Value		=	lua_tonumber(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( n64Value <= 0 ) return 0;

	pRole->GetCurMgr().DecBagSilver(n64Value, (e_log_cmd_id)nCmdID);

	return 0;
}


//-------------------------------------------------------------------------------------
// 获得玩家金钱
//-------------------------------------------------------------------------------------
int script_get_role_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT64 n64Silver = pRole->GetCurMgr().GetBagSilver();
	lua_pushnumber(L, n64Silver);
	//lua_pushinteger(L, nSilver);
	return 1;
}

static int script_add_role_bind_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().IncBagBindSilver(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}

int script_dec_role_bind_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT64	n64Value		=	lua_tonumber(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( n64Value <= 0 ) return 0;

	pRole->GetCurMgr().DecBagBindSilver(n64Value, (e_log_cmd_id)nCmdID);

	return 0;
}

int script_get_role_bind_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT64 n64Silver = pRole->GetCurMgr().GetBagBindSilver();
	lua_pushnumber(L, n64Silver);
	//lua_pushinteger(L, nSilver);
	return 1;
}

int script_add_role_yuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().IncBaiBaoYuanBao(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}

int script_get_role_yuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT32 n_yuanbao = pRole->GetCurMgr().GetBaiBaoYuanBao();

	lua_pushinteger(L, n_yuanbao);

	return 1;
}

int script_dec_role_yuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT32	n32_yuanbao		=   lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	pRole->GetCurMgr().DecBaiBaoYuanBao(n32_yuanbao, (e_log_cmd_id)nCmdID);

	return 0;
}
int script_add_role_bindyuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().IncBagYuanBao(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}
int script_dec_role_bindyuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( nValue <= 0 ) return 0;

	pRole->GetCurMgr().DecBagYuanBao(nValue,(e_log_cmd_id)nCmdID);
	return 0;
}
int script_get_role_bindyuanbao(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT32 n_yuanbao = pRole->GetCurMgr().GetBagYuanBao();

	lua_pushinteger(L, n_yuanbao);

	return 1;
}
int script_get_all_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	INT64 n64Silver = pRole->GetCurMgr().GetBagAllSilver();
	lua_pushnumber(L, n64Silver);

	return 1;
}

int script_dec_bind_and_nobind_silver(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);
	INT64	n64Value		=	lua_tonumber(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if( n64Value <= 0 ) return 0;

	pRole->GetCurMgr().DecBagSilverEx(n64Value, (e_log_cmd_id)nCmdID);

	return 0;
}


//-------------------------------------------------------------------------------------
// 得到玩家性别
//-------------------------------------------------------------------------------------
int script_get_role_sex(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )
	{
		return 0;
	}

	lua_pushinteger(L, pRole->GetSex());
	return 1;
}

//-------------------------------------------------------------------------------------
// 得到玩家服装编号
//-------------------------------------------------------------------------------------
int script_get_role_dress_id(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )
	{
		return 0;
	}

	lua_pushinteger(L, pRole->GetAvatar()->wDressMdlID);
	return 1;
}

//-------------------------------------------------------------------------------------
// 得到玩家帮派ID
//-------------------------------------------------------------------------------------
int script_get_role_guild_id(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )
	{
		return 0;
	}
	if (pRole->GetGuildID() != INVALID_VALUE)
	{
		lua_pushnumber(L, pRole->GetGuildID());
	}
	else
	{
		lua_pushnumber(L,INVALID_VALUE);
	}
	return 1;
}

//-------------------------------------------------------------------------------------
// 判断玩家是否在摆摊
//-------------------------------------------------------------------------------------
int script_is_role_onstall(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	int nOnStall = FALSE;

	// 判断是否已经处于摆摊或摆摊设置状态
	if(pRole->IsInRoleStateAny(ERS_Stall | ERS_StallSet))
		nOnStall = TRUE;
	else
		nOnStall = FALSE;

	lua_pushboolean(L, nOnStall);

	return 1;
}

//-------------------------------------------------------------------------------------
// 设置玩家状态
//-------------------------------------------------------------------------------------
int script_set_role_state(lua_State* L)
{
	DWORD		dw_role_id	=	lua_tonumber(L, 1);
	INT			nState		=	lua_tointeger(L, 2);
	BOOL		bSendMsg	=	lua_toboolean(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )	return 0;

	if( nState <= 0 || nState >= ERS_End)	return 0;

	pRole->SetRoleState((ERoleState)nState, bSendMsg);
	return 0;
}

int script_unset_role_state(lua_State* L)
{
	DWORD		dw_role_id	=	lua_tonumber(L, 1);
	INT			nState		=	lua_tointeger(L, 2);
	BOOL		bSendMsg	=	lua_toboolean(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )	return 0;

	if( nState <= 0 || nState >= ERS_End)	return 0;

	pRole->UnsetRoleState((ERoleState)nState, bSendMsg);
	return 0;
}

//-------------------------------------------------------------------------------------
// 能否开启宝箱
//-------------------------------------------------------------------------------------
int script_can_open_chest(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	DWORD dwChestID		=	lua_tonumber(L, 4);
	DWORD dwKeyID		=	lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwChestID);
	if (!VALID_POINT(pItemProto))
	{
		return 0;
	}
	lua_pushboolean(L, pItemProto->nSpecFuncVal2 == dwKeyID);

	return 1;
}

//-------------------------------------------------------------------------------------
// 向客户端发送消息
//-------------------------------------------------------------------------------------
int script_send_chest_message(lua_State* L)
{
	DWORD dwMapID			=	lua_tonumber(L, 1);
	DWORD dwInstanceID		=	lua_tonumber(L, 2);
	DWORD dw_role_id			=	lua_tonumber(L, 3);
	DWORD dwChestID			=	lua_tonumber(L, 4);
	const string strMsgName =	lua_tostring(L, 5);
	BOOL bOpened			=	lua_toboolean(L, 6);
	BOOL bDestroy			=	lua_toboolean(L, 7);
	DWORD dwItemID			=	lua_tonumber(L, 8);
	INT nItemNum			=	lua_tointeger(L, 9);
	DWORD dw_error_code		=	lua_tonumber(L, 10);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	// 发送消息给客户端
	g_TreasureChestMgr.SendMsg2Client(pRole, dwChestID, strMsgName, bOpened, bDestroy, dwItemID, nItemNum, dw_error_code);

	return 0;
}

//-------------------------------------------------------------------------------------
// 使世界宝箱开启计数和角色宝箱开启计数加1
//-------------------------------------------------------------------------------------
int script_inc_treasure_sum(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	g_worldSession.IncTreasureSum();
	pRole->IncTreasureSum();
	
	return 0;
}

//-------------------------------------------------------------------------------------
// 获得世界宝箱开启计数和角色宝箱开启计数
//-------------------------------------------------------------------------------------
int script_get_treasure_sum(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	lua_pushinteger(L, pRole->GetTreasureSum());
	lua_pushinteger(L, g_worldSession.GetTreasureSum());

	return 2;
}

//-------------------------------------------------------------------------------------
// 得到宝箱内物品
//-------------------------------------------------------------------------------------
int script_get_chest_item(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dw_role_id		=	lua_tonumber(L, 3);
	DWORD dwChestID		=	lua_tonumber(L, 4);
	ERateType eRate		=	(ERateType)lua_tointeger(L, 5);
	FLOAT fRand			=	(FLOAT)lua_tonumber(L, 6);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	tagChestItem* pItem = g_TreasureChestMgr.GetRandomItem(dwChestID, eRate, fRand);
	if (!VALID_POINT(pItem))	return 0;

	DWORD dwItemTypeID = pItem->dw_data_id;	// 开出物品typeid
	INT n_num = pItem->n_num;					// 开出物品数量
	lua_pushnumber(L, dwItemTypeID);
	lua_pushinteger(L, n_num);

	return 2;
}

//-------------------------------------------------------------------------------------
// 物品是否需要广播
//-------------------------------------------------------------------------------------
int script_item_need_broadcast(lua_State* L)
{
	DWORD dw_data_id	=	lua_tonumber(L, 1);

	// 找到物品
	tagItemProto* pProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	if (!VALID_POINT(pProto))	return 0;

	lua_pushnumber(L, pProto->bNeedBroadcast);
	
	return 1;
}

//-------------------------------------------------------------------------------------
// 得到宝箱内物品
//-------------------------------------------------------------------------------------
int script_stop_mount(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	pRole->StopMount();

	return 0;
}

//-------------------------------------------------------------------------------------
// 通知队友进入副本
//-------------------------------------------------------------------------------------
int script_instance_notify(lua_State* L)
{
	DWORD	dw_role_id	=	lua_tonumber(L, 1);
	BOOL	bNotify		=	lua_toboolean(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(dw_role_id))
	{
		return 0;
	}

	if(bNotify == FALSE)	return 0;

	// 找到玩家小队
	const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return 0;

	// 检查队伍是否创建了副本
	Map* pMap = g_mapCreator.get_map(pTeam->get_own_instance_mapid(), pTeam->get_own_instanceid());
	if( !VALID_POINT(pMap) || EMT_Instance != pMap->get_map_type() ) return 0;

	map_instance_normal* pInstance = static_cast<map_instance_normal*>(pMap);
	if( !VALID_POINT(pInstance) ) return 0;

	const tagInstance* pInstanceProto = pInstance->get_instance_proto();
	if( !VALID_POINT(pInstanceProto) ) return 0;	

	// 副本是否允许通知队友
	if( !pInstanceProto->bNoticeTeamate ) return 0;

	// 通知队友
	NET_SIS_instance_agree	 send;
	send.dwInviterID	=	pRole->GetID();
	send.dwMapID		=	pInstance->get_map_id();
	g_groupMgr.SendTeamInstanceNotify(pRole, pRole->GetTeamID(), &send, send.dw_size);

	return 0;
}

//-------------------------------------------------------------------------------------
// 通知队友进入副本
//-------------------------------------------------------------------------------------
int script_sig_title_event(lua_State* L)
{
	DWORD			dw_role_id	= lua_tonumber(L, 1);
	DWORD			titleEvent	= lua_tonumber(L, 2);
	DWORD			dwArg1		= lua_tonumber(L, 3);
	DWORD			dwArg2		= lua_tonumber(L, 4);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(dw_role_id))
	{
		return 0;
	}

	//BOOL bHas = pRole->GetAchievementMgr().UpdateAchievementCriteria((e_achievement_event)titleEvent, dwArg1, dwArg2);

	lua_pushboolean(L, FALSE);

	return 1;
}

//-------------------------------------------------------------------------------------
// 改变技能熟练度
//-------------------------------------------------------------------------------------
int script_change_skill_exp(lua_State* L)
{
	DWORD			dw_role_id	=	lua_tonumber(L, 1);
	DWORD			dwSkillID	=	lua_tonumber(L, 2);
	DWORD			dwSkillExp	=	lua_tonumber(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(dw_role_id))
	{
		return 0;
	}

	Skill* pSkill = pRole->GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		return 0;
	}

	pRole->ChangeSkillExp(pSkill, dwSkillExp);

	return 0;
}

static int script_get_friend_count(lua_State* L)
{
	DWORD dw_role_id	=	lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	
	if (!VALID_POINT(pRole)) return 0;
	INT nCount = pRole->GetFriendCount( );
	
	lua_pushinteger(L, nCount);
	return 1;
}
static int script_has_master(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	
	if (!VALID_POINT(pRole)) return 0;
	BOOL b = VALID_POINT( pRole->get_master_id( ) );

	lua_pushboolean(L,b);
	return 1;
}

static int script_get_master_id(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);

	if(!VALID_POINT(pRole)) return 0;
	if(!VALID_POINT(pRole->get_master_id( ))) return 0;
	lua_pushnumber(L, pRole->get_master_id( ));
	return 1;
}
static int script_is_in_master_recruit(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	if( !VALID_POINT(dw_role_id) ) return 0;

	BOOL b = g_MasterPrenticeMgr.find_role_in_recruit( dw_role_id );

	lua_pushboolean(L,b);
	return 1;
}
static int script_has_prentice(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	if( !VALID_POINT(dw_role_id) ) return 0;

	BOOL b = g_MasterPrenticeMgr.has_prentice( dw_role_id );

	lua_pushboolean(L,b);
	return 1;
}


static int script_get_prentice_list(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	if( !VALID_POINT(dw_role_id) ) return 0;

	DWORD	dw_prenices[5];
	int n = g_MasterPrenticeMgr.fill_prentice_list(dw_role_id, dw_prenices);

	lua_createtable(L, n, 0);
	for(int i = 0; i < n; ++i)
	{
		lua_pushinteger(L, i + 1);
		lua_pushinteger(L, dw_prenices[i]);
		lua_settable(L, -3);
	}

	return 1;
}

//-------------------------------------------------------------------------------------
// 设置帮会的复活点ID
//-------------------------------------------------------------------------------------
int script_set_guild_relive_id(lua_State* L)
{
	DWORD	dwMapID		= lua_tonumber(L, 1);
	DWORD	dwInstanceID = lua_tonumber(L,2);
	DWORD   dwCreatureID = lua_tonumber(L,3);
	DWORD   dwCreatureTypeID = lua_tonumber(L,4);
	DWORD   dw_role_id = lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if(!VALID_POINT(pRole)) return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_WAR)
		return 0;

	pCreature->SetGuildID(pGuild->get_guild_att().dwID);
	pGuild->get_guild_war().set_relive_id(pCreature->GetID(), TRUE);

	guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
	if(!VALID_POINT(pEnemyGuild))
		return 0;

	pEnemyGuild->get_guild_war().set_relive_id(pCreature->GetID(), FALSE);

	return 1;

}

//-------------------------------------------------------------------------------------
// 开始帮会修炼
//-------------------------------------------------------------------------------------
int script_start_guild_prictice(lua_State* L)
{
	DWORD	dwMapID		= lua_tonumber(L, 1);
	DWORD	dwInstanceID = lua_tonumber(L,2);
	DWORD   dw_role_id = lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	DWORD dwError = pGuild->get_upgrade().StartGuildPractice(pRole);

	NET_SIS_Start_Guild_Practice send;
	send.dw_error = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;

}

//-------------------------------------------------------------------------------------
// 设置帮会的鼎ID
//-------------------------------------------------------------------------------------
int script_set_guild_tripod_id(lua_State* L)
{
	DWORD	dwMapID		= lua_tonumber(L, 1);
	DWORD	dwInstanceID = lua_tonumber(L,2);
	DWORD   dwCreatureID = lua_tonumber(L,3);
	DWORD   dwCreatureTypeID = lua_tonumber(L,4);
	DWORD   dw_role_id = lua_tonumber(L, 5);
	DWORD	dw_value = 0;

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if(!VALID_POINT(pRole)) return 0;

	if(pCreature->GetProto()->nType2 != EMTT_Tripod)
		return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	if(pGuild->get_tripod_id() != INVALID_VALUE)
	{
		dw_value = 4;		// 已经占领鼎
		lua_pushinteger(L, dw_value);
		return 1;
	}

	INT n_Level = pGuild->get_upgrade().GetFacilitiesLevel(EFT_Lobby);
	if(n_Level < 7)
	{
		dw_value = 3;		// 等级不足
		lua_pushinteger(L, dw_value);
		return 1;
	}

	if(pCreature->GetGuildID() != INVALID_VALUE)
	{
		dw_value = 1;		// 已被占领
		lua_pushinteger(L, dw_value);
		return 1;
	}
	
	if (CalcTimeDiff(g_world.GetWorldTime(), pGuild->get_lost_tripod_time()) < LOST_TRIPOD_TIME)
	{
		dw_value = 5;		// 时间限制
		lua_pushinteger(L, dw_value);
		return 1;
	}
	pCreature->SetGuildID(pGuild->get_guild_att().dwID);
	pGuild->set_tripod_id(pCreature->GetID());
	pGuild->set_tripod_map_id(pMap->get_map_id());

	if(pGuild->get_guild_facilities().IsLobbyUsing())
	{
		pCreature->SetState(ES_Occupied);
	}

	dw_value = 2;
	pGuild->send_tripod_message();

	NET_SIS_Guild_Tripod send;
	_tcsncpy(send.szGuildName, pGuild->get_guild_att().str_name.c_str(), sizeof(send.szGuildName));
	g_roleMgr.send_world_msg(&send, send.dw_size);

	lua_pushinteger(L, dw_value);

	return 1;
}

//-------------------------------------------------------------------------------------
// 判定帮会门是否开启
//-------------------------------------------------------------------------------------
int script_guild_door_limit(lua_State* L)
{
	DWORD	dwMapID		= lua_tonumber(L, 1);
	DWORD	dwInstanceID = lua_tonumber(L,2);
	DWORD   dw_role_id = lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	if(pMap->get_map_type() != EMT_Guild)
		return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
	if(!VALID_POINT(pEnemyGuild))
		return 0;

	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_WAR)
		return 0;

	if(((map_instance_guild*)pMap)->get_guild_id() == pGuild->get_guild_att().dwID)
		return 0;
	
	if(((map_instance_guild*)pMap)->get_guild_id() != pEnemyGuild->get_guild_war().get_war_guild_id())
		return 0;

	return 1;
}

//-------------------------------------------------------------------------------------
// 解散帮会
//-------------------------------------------------------------------------------------
int script_guild_dismiss(lua_State* L)
{
	DWORD	dwMapID		= lua_tonumber(L, 1);
	DWORD	dwInstanceID = lua_tonumber(L,2);
	DWORD   dw_role_id = lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	if(!pGuild->get_guild_att().bFormal)
	{
		pGuild->dismiss_guild();
	}
	
	return 1;

}

//-------------------------------------------------------------------------------------
// 帮会设施被摧毁
//-------------------------------------------------------------------------------------
int script_guild_graded_destroy(lua_State* L)
{
	DWORD dwMapID = lua_tonumber(L, 1);
	DWORD dwInstanceID = lua_tonumber(L, 2);
	DWORD dwCreatureID = lua_tonumber(L, 3);
	DWORD dwCreatureTypeID = lua_tonumber(L, 4);
	DWORD dwKillID = lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if(!VALID_POINT(pCreature))
		return 0;

	guild* pGuild = g_guild_manager.get_guild(pCreature->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	pGuild->get_upgrade().FacilitesDestory((EFacilitiesType)pCreature->GetProto()->eGuildType);

	guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
	if(VALID_POINT(pEnemyGuild))
	{
		pEnemyGuild->get_guild_war().set_achievement(60);
		pGuild->get_guild_war().set_enemy_achievement(60);
	}

	if(pCreature->GetProto()->eGuildType == EGT_Bank)
	{
		INT32 nFund = 2000 * pGuild->get_guild_att().byLevel;
		pGuild->decrease_guild_fund(INVALID_VALUE, nFund, elcid_guild_declarewar);
		pEnemyGuild->increase_guild_fund(INVALID_VALUE, nFund, elcid_guild_declarewar);
	}

	if(pCreature->GetProto()->eGuildType == EGT_Lobby)
	{
		pGuild->get_guild_war().m_bLost = TRUE;
		pEnemyGuild->get_guild_war().m_bWin = TRUE;
	}

	if(pMap->get_map_info()->e_type == EMT_Guild)
	{
		if((EFacilitiesType)pCreature->GetProto()->eGuildType < EGT_Bank && (EFacilitiesType)pCreature->GetProto()->eGuildType > EGT_Factory)
			return 0;

		map_instance_guild* pGuildMap = (map_instance_guild*)pMap;
		if(!VALID_POINT(pGuildMap))
			return 0;

		Creature* pBoss = pGuildMap->find_creature(pGuildMap->get_boss_id());
		if(!VALID_POINT(pBoss))
			return 0;

		NPCTeam* pNPCTeam = pBoss->GetTeamPtr();
		if(!VALID_POINT(pNPCTeam))
			return 0;

		pNPCTeam->DelGuildSentinel((BYTE)pCreature->GetProto()->eGuildType);
	}

	return 1;
}

//-------------------------------------------------------------------------------------
// 设置玩家脚本通用数据
//-------------------------------------------------------------------------------------
int script_set_role_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	INT		nParamNum		=	lua_tointeger(L, 2);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+3);
		dwValue		=	lua_tonumber(L, i*2+1+3);

		if( nIndex < 0 || nIndex >= ESD_Role ) return 0;

		pRole->SetScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 修改玩家脚本通用数据
//-------------------------------------------------------------------------------------
int script_mod_role_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	INT		nIndex			=	lua_tointeger(L, 2);
	DWORD	dwValue			=	lua_tonumber(L, 3);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	if( nIndex < 0 || nIndex >= ESD_Role ) return 0;

	pRole->ModScriptData(nIndex, dwValue);
	
	if (nIndex == 65 || nIndex == 66)
	{
		pRole->SendConsumeReward();
	}
	return 0;
}

//-------------------------------------------------------------------------------------
// 获得玩家脚本通用数据
//-------------------------------------------------------------------------------------
int script_get_role_script_data(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	INT		nIndex			=	lua_tointeger(L, 2);
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	if( nIndex < 0 || nIndex >= ESD_Role ) return 0;

	dwValue = pRole->GetScriptData(nIndex);
	lua_pushnumber(L, dwValue);

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家脚本通用数据
//-------------------------------------------------------------------------------------
int script_can_mount(lua_State* L)
{
// 	DWORD	dw_role_id		=	lua_tonumber(L, 1);
// 
// 	// 找到玩家
// 	Role* pRole = g_roleMgr.get_role(dw_role_id);
// 	if( !VALID_POINT(pRole) ) return 0;
// 	
// 	INT nRt = pRole->GetPetPocket()->CanRidePet();
// 
// 	lua_pushinteger(L, nRt);
// 
// 	return 1;
	return 0;
}

//-------------------------------------------------------------------------------------
// 增加玩家赠点数量
//-------------------------------------------------------------------------------------
int script_add_role_ex_volume(lua_State* L)
{
	DWORD   dwMapID			=	(INT64)lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	(INT64)lua_tonumber(L, 2);
	DWORD   dw_role_id		=	(INT64)lua_tonumber(L, 3);
	INT		nValue			=	lua_tointeger(L, 4);
	INT		nCmdID			=	lua_tointeger(L, 5);



	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	pRole->GetCurMgr().IncExchangeVolume(nValue, (e_log_cmd_id)nCmdID);

	return 0;
}


//-------------------------------------------------------------------------------------
// 判断人物状态
//-------------------------------------------------------------------------------------
int script_is_role_in_status(lua_State* L)
{
	DWORD dw_role_id	= lua_tonumber(L, 1);
	DWORD dwStatus	= lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(pRole))
	{
		return 0;
	}
	
	lua_pushboolean(L, pRole->IsInRoleStateAll(dwStatus));

	return 1;
}

//-------------------------------------------------------------------------------------
// 创建脚本通用消息
//-------------------------------------------------------------------------------------
int script_begin_message_event(lua_State* L)
{
	DWORD	dwMsgInfoID		=	g_MsgInfoMgr.BeginMsgEvent();
	lua_pushnumber(L, dwMsgInfoID);

	return 1;
}

//-------------------------------------------------------------------------------------
// 向消息中添加相关事件数据
//-------------------------------------------------------------------------------------
int script_add_message_event(lua_State* L)
{
	DWORD			dwMsgInfoID		=	lua_tonumber(L, 1);
	EMsgUnitType	eMsgUnitType	=	(EMsgUnitType)lua_tointeger(L, 2);
	LPVOID			pData			=	NULL;

	if(EMUT_RoleName == eMsgUnitType)
	{
		LPCTSTR pData = get_tool()->ansi_to_unicode(lua_tostring(L, 3));
		g_MsgInfoMgr.AddMsgEvent(dwMsgInfoID, eMsgUnitType, (LPVOID)pData);
	}
	else
	{
		DWORD dwData = lua_tonumber(L, 3);
		g_MsgInfoMgr.AddMsgEvent(dwMsgInfoID, eMsgUnitType, &dwData);
	}

	return 0;
}


//-------------------------------------------------------------------------------------
// 发送脚本通用消息给玩家
//-------------------------------------------------------------------------------------
int script_dispatch_role_message_event(lua_State* L)
{	
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dwMsgInfoID		=	lua_tonumber(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	g_MsgInfoMgr.DispatchRoleMsgEvent(dwMsgInfoID, pRole);
	return 0;
}

//-------------------------------------------------------------------------------------
// 给服务器所有地图内的玩家发送脚本通用消息
//-------------------------------------------------------------------------------------
int script_dispatch_world_message_event(lua_State* L)
{
	DWORD	dwMsgInfoID		=	lua_tonumber(L, 1);

	g_MsgInfoMgr.DispatchWorldMsgEvent(dwMsgInfoID);

	return 0;
}

int script_dispath_map_message_event(lua_State* L)
{
	DWORD	dwMsgInfoID		=	lua_tonumber(L, 1);
	DWORD	dwMapID			=	lua_tonumber(L, 2);
	DWORD	dwInstanceID	=	lua_tonumber(L, 3);
	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	g_MsgInfoMgr.DispatchMapMsgEvent(dwMsgInfoID, pMap);

	return 0;
}


//-------------------------------------------------------------------------------------
// 固定活动接口
//-------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------
// 初始化活动时间事件的触发时间
//-------------------------------------------------------------------------------------
int script_init_event_time(lua_State* L)
{
	const CHAR* szTableName		=   lua_tostring(L, 1);			// 存放时间点table的名字
	INT			n_num			=   lua_tointeger(L, 2);		// 时间点的个数
	DWORD		dwID			=   lua_tonumber(L, 3);			// 活动ID

	if(n_num <= 0)	return 0;

	activity_fix *pActivity = activity_mgr::GetInstance()->get_activity(dwID);
	if(!VALID_POINT(pActivity))	return 0;

	for(INT i = 1; i <= n_num; ++i)
	{
		lua_getglobal(L, szTableName);
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		DWORD	dw_time = lua_tointeger(L, -1);

		pActivity->add_event_time(dw_time);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 活动是否开始
//-------------------------------------------------------------------------------------
int script_get_act_is_start(lua_State* L)
{
	BOOL		bStart		=	FALSE;
	DWORD		dwID		=	lua_tonumber(L, 1);			//活动ID

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(dwID);
	if( !VALID_POINT(pActivity) )	return 0;

	if( pActivity->is_start() ) bStart = TRUE;

	lua_pushboolean(L, bStart);
	return 1;
}

//-------------------------------------------------------------------------------------
// 活动是否开始
//-------------------------------------------------------------------------------------
int script_add_all_role_buff(lua_State* L)
{
	DWORD	dwBuffTypeID		=	lua_tonumber(L, 1);
	tagAllRoleBuff 	AllRoleBuff;
	AllRoleBuff.dwBuffTypeID = dwBuffTypeID;
	g_groupMgr.AddEvent(INVALID_VALUE, EVT_AddAllRoleBuff, sizeof(tagAllRoleBuff), &AllRoleBuff);

	return 0;
}

//-------------------------------------------------------------------------------------
// 设置固定活动脚本通用数据
//-------------------------------------------------------------------------------------
int script_set_activities_script_data(lua_State* L)
{
	DWORD	dwActID			=	lua_tonumber(L, 1);
	INT		nParamNum		=	lua_tointeger(L, 2);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到活动
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(dwActID);
	if( !VALID_POINT(pActivity) ) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+3);
		dwValue		=	lua_tonumber(L, i*2+1+3);

		pActivity->SetScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 获得固定活动脚本通用数据
//-------------------------------------------------------------------------------------
int script_get_activities_script_data(lua_State* L)
{
	DWORD	dwActID			=	lua_tonumber(L, 1);
	INT		nParamNum		=	lua_tointeger(L, 2);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到活动
	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(dwActID);
	if( !VALID_POINT(pActivity) ) return 0;

	if( nParamNum <= 0 ) return 0;

	std::vector<INT> vec;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex = lua_tointeger(L, i+3);
		vec.push_back(nIndex);
	}

	std::vector<INT>::iterator it = vec.begin();
	while (it != vec.end())
	{
		dwValue = pActivity->GetScriptData(*it);
		lua_pushnumber(L, dwValue);
		++it;
	}

	return nParamNum;
}

//-------------------------------------------------------------------------------------
// 保存固定活动脚本通用数据
//-------------------------------------------------------------------------------------
int script_save_activities_data(lua_State* L)
{
	DWORD	dwActID			=	lua_tonumber(L, 1);

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(dwActID);
	if( !VALID_POINT(pActivity) )	return 0;
		
	pActivity->save_to_db();

	return 0;
}


struct __SendOpLevel
{
public:
	__SendOpLevel(int minl, int maxl, PVOID p_message, DWORD dw_size)
		:minlevel_(minl), maxLevel_(maxl),msg_(p_message), size_(dw_size){}

public:
	VOID operator()(Unit* p_unit)
	{
		if (VALID_POINT(p_unit) && p_unit->IsRole())
		{
			Role* pRole = dynamic_cast<Role*>(p_unit);
			if (VALID_POINT(pRole)){
				if((minlevel_ != 255 && pRole->get_level() < minlevel_) ||
					(maxLevel_ != 255 && pRole->get_level() > maxLevel_))
					return;

				pRole->SendMessage(msg_, size_);			
			}
		}
	}
public:
	INT minlevel_;
	INT maxLevel_;
	PVOID msg_;
	DWORD size_;
};

static int script_activities_broad(lua_State* L)
{
	DWORD	dwActID		=	lua_tonumber(L, 1);
	INT		nState		=	lua_tointeger(L, 2);

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(dwActID);
	if( !VALID_POINT(pActivity) )	return 0;

	const s_act_info* pActInfo = pActivity->get_info();
	if(!VALID_POINT(pActInfo))
		return 0;

	NET_SIS_acitvity_state	send;
	send.nState = nState;
	send.dwActivityID = dwActID;
	if( nState == 0 || nState == 1 )	// 即将开始或开始
	{
		switch(pActInfo->e_act_mode)
		{
			case EAM_PERSIST:
				send.byHour = pActInfo->act_time.star_time.hour;
				send.byMinute = pActInfo->act_time.star_time.min;
				break;
			case EAM_WEEK:
				send.byHour = pActInfo->by_start_h;
				send.byMinute = pActInfo->by_start_m;
				break;

		}
	}
	else								// 即将结束或结束
	{
		switch(pActInfo->e_act_mode)
		{
		case EAM_PERSIST:
			send.byHour = pActInfo->act_time.end_time.hour;
			send.byMinute = pActInfo->act_time.end_time.min;
			break;
		case EAM_WEEK:
			send.byHour = pActInfo->by_end_h;
			send.byMinute = pActInfo->by_end_m;
			break;

		}
	}

	__SendOpLevel op(pActInfo->min_level, 
			pActInfo->max_level, &send, send.dw_size);

	g_roleMgr.send_world_msg<__SendOpLevel>(op);

	return 0;
}

int script_set_battle_boss(lua_State* L)
{
	DWORD	dwType				=	lua_tonumber(L, 1);
	DWORD	dwCreateId			=	lua_tonumber(L, 2);

	BattleGround::get_singleton().setTeamLeader((EBATTLENTEAM)dwType, dwCreateId);


	return 0;
}
//-------------------------------------------------------------------------------------
// 创建玩家小队
//-------------------------------------------------------------------------------------
int script_create_team(lua_State* L)
{
	DWORD	dwSrcRoleID			=	lua_tonumber(L, 1);
	DWORD	dwDesRoleID			=	lua_tonumber(L, 2);

	tagCreateTeam	CreateTeam;
	CreateTeam.dwSrcRoleID = dwSrcRoleID;
	CreateTeam.dwDesRoleID = dwDesRoleID;
	g_groupMgr.AddEvent(INVALID_VALUE, EVT_CreateTeam, sizeof(tagCreateTeam), &CreateTeam);

	return 0;
}

//-------------------------------------------------------------------------------------
// 增加一个玩家到玩家小队
//-------------------------------------------------------------------------------------
int script_add_team_member(lua_State* L)
{
	DWORD	dwTeamID		=	lua_tonumber(L, 1);
	DWORD	dwDesRoleID		=	lua_tonumber(L, 2);

	tagAddTeamMember	AddTeamMem;
	AddTeamMem.dwTeamID = dwTeamID;
	AddTeamMem.dwDesRoleID = dwDesRoleID;

	g_groupMgr.AddEvent(INVALID_VALUE, EVT_AddMember, sizeof(tagAddTeamMember), &AddTeamMem);

	return 0;
}

//-------------------------------------------------------------------------------------
// 得到小队队长的ID
//-------------------------------------------------------------------------------------
int script_get_team_leader_id(lua_State* L)
{
	DWORD	dwTeamID		=	lua_tonumber(L, 1);

	const Team*	pTeam		=	g_groupMgr.GetTeamPtr(dwTeamID);
	if ( !VALID_POINT(pTeam) )
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	lua_pushnumber(L, pTeam->get_member_id(0));

	return 1;
}


//-------------------------------------------------------------------------------------
// 设置地图脚本通用数据
//-------------------------------------------------------------------------------------
int script_set_map_script_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	INT		dwInstanceID	=	lua_tointeger(L, 2);
	INT		nParamNum		=	lua_tointeger(L, 3);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到地图
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+4);
		dwValue		=	lua_tonumber(L, i*2+1+4);

		pMap->SetScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 获得地图脚本通用数据
//-------------------------------------------------------------------------------------
int script_get_map_script_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	INT		nParamNum		=	lua_tointeger(L, 3);
	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到地图
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	if( nParamNum <= 0 ) return 0;

	std::vector<INT> vec;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex = lua_tointeger(L, i+4);
		vec.push_back(nIndex);
	}

	std::vector<INT>::iterator it = vec.begin();
	while (it != vec.end())
	{
		dwValue = pMap->GetScriptData(*it);
		lua_pushnumber(L, dwValue);
		++it;
	}

	return nParamNum;
}

//-------------------------------------------------------------------------------------
// 地图动态刷出非碰撞怪物
//-------------------------------------------------------------------------------------
int script_map_create_creature(lua_State* L)
{
 	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_data_id		=	lua_tonumber(L, 3);
	FLOAT	fx				=	lua_tonumber(L, 4);
	FLOAT	fy				=	lua_tonumber(L, 5);
	FLOAT	fz				=	lua_tonumber(L, 6);
	DWORD	dwYaw			=	lua_tonumber(L, 7);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	Vector3 vPos;
	vPos.x = fx * TILE_SCALE + TILE_SCALE / 2;
	vPos.y = fy;
	vPos.z = fz * TILE_SCALE + TILE_SCALE / 2;
	
	if (!pMap->if_can_go(vPos.x, vPos.z))
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	// 随机怪物朝向
	Vector3 vFaceTo;
	FLOAT fYaw = FLOAT(dwYaw % 360);
	vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
	vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
	vFaceTo.y = 0.0f;

	Creature *pCreature = pMap->create_creature(dw_data_id, vPos, vFaceTo);

	if (VALID_POINT(pCreature))
	{
		lua_pushnumber(L, pCreature->GetID());
		return 1;
	}

	lua_pushnumber(L, INVALID_VALUE);
	return 1;

}

//-------------------------------------------------------------------------------------
// 动态刷出碰撞怪物
//-------------------------------------------------------------------------------------
int script_map_create_col_creature(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_data_id		=	lua_tonumber(L, 3);
	FLOAT	fx				=	lua_tonumber(L, 4);
	FLOAT	fy				=	lua_tonumber(L, 5);
	FLOAT	fz				=	lua_tonumber(L, 6);
	BOOL	bCollide		=	lua_toboolean(L, 7);
	tstring pPatrolListName(get_tool()->ansi_to_unicode(lua_tostring(L, 8)));

	//CHAR*	pPatrolListName	=   const_cast<CHAR*>(lua_tostring(L, 8));


	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	Vector3 vPos;
	vPos.x = fx * TILE_SCALE;
	vPos.y = fy;
	vPos.z = fz * TILE_SCALE;
	
	if (!pMap->if_can_go(vPos.x, vPos.z))
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	// 随机怪物朝向
	Vector3 vFaceTo;
	FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
	vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
	vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
	vFaceTo.y = 0.0f;

	Creature *pCreature = pMap->create_creature(dw_data_id, vPos, vFaceTo, INVALID_VALUE, bCollide, pPatrolListName.c_str());

	if (VALID_POINT(pCreature))
	{
		lua_pushnumber(L, pCreature->GetID());
		return 1;
	}

	lua_pushnumber(L, INVALID_VALUE);
	return 1;
}

//-------------------------------------------------------------------------------------
// 删除怪物
//-------------------------------------------------------------------------------------
int script_map_delete_creature(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwID			=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		return 0;
	}

	//存在map不安全情况，不能作此操作lc
	//pMap->delete_creature(dwID);
	
	Creature* pCreature = pMap->find_creature(dwID);
	if (VALID_POINT(pCreature))
	{
		if( !pCreature->IsInState(ES_Dead) )
		{
			pCreature->OnDisappear();
			lua_pushboolean(L, TRUE);
		}
		else
		{
			lua_pushboolean(L, FALSE);
		}
	}
	else
	{
		lua_pushboolean(L, FALSE);
	}
	
	return 1;
}

//-------------------------------------------------------------------------------------
// 创建副本
//-------------------------------------------------------------------------------------
int script_create_instance(lua_State* L)
{
	DWORD	dwMapID 		=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);

	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dwMapID);
	if( !VALID_POINT(pMapMgr))		return 0;

	BOOL	bRet = pMapMgr->CreateScriptInstance(dwInstanceID);

	lua_pushboolean(L, bRet);

	return 1;	
}

//-------------------------------------------------------------------------------------
// 删除副本
//-------------------------------------------------------------------------------------
int script_delete_instance(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);

	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dwMapID);
	if( !VALID_POINT(pMapMgr))		return 0;

	map_instance* pInstance = pMapMgr->GetInstance(dwInstanceID);
	if( !VALID_POINT(pInstance))			return 0;

	pInstance->set_delete();
	
	return 0;
}

//-------------------------------------------------------------------------------------
// 播放场景特效
//-------------------------------------------------------------------------------------
int script_play_scene_effect(lua_State* L)
{
	DWORD dwMapID			= lua_tonumber(L, 1);
	DWORD dwInstanceID		= lua_tonumber(L, 2);
	ESceneEffectType eType	= (ESceneEffectType)lua_tointeger(L, 3);
	DWORD dwObjID			= lua_tonumber(L, 4);
	DWORD dwEffectID		= lua_tonumber(L, 5);
	FLOAT fPosX				= lua_tonumber(L, 6);
	FLOAT fPosY				= lua_tonumber(L, 7);
	FLOAT fPosZ				= lua_tonumber(L, 8);

	Vector3 vPos(fPosX, fPosY, fPosZ);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	pMap->play_scene_effect(eType, dwObjID, vPos, dwEffectID);

	return 0;
}

//-------------------------------------------------------------------------------------
// 停止场景特效
//-------------------------------------------------------------------------------------
int script_stop_scene_effect(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	DWORD dwObjID		= lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	pMap->stop_scene_effect(dwObjID);

	return 0;
}

int script_open_door(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	DWORD dwObjID		= lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	Creature *pCreature = pMap->find_creature( OBJID_TO_DOORID(dwObjID));
	
	if(pMap->get_map_type() == EMT_Instance)
	{
		map_instance_normal* p_map = (map_instance_normal*)pMap;
		p_map->set_door_state(dwObjID, TRUE);

	}

	return 0;
}

int script_close_door(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	DWORD dwObjID		= lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	Creature *pCreature = pMap->find_creature( OBJID_TO_DOORID(dwObjID) );

	if(pMap->get_map_type() == EMT_Instance)
	{
		map_instance_normal* p_map = (map_instance_normal*)pMap;

		p_map->set_door_state(dwObjID, FALSE);
	}

	return 0;
}

int script_is_door_open(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	DWORD dwObjID		= lua_tonumber(L, 3);

	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	Creature *pCreature = pMap->find_creature( OBJID_TO_DOORID(dwObjID) );
	BOOL bIsOpen = FALSE;
	if ( VALID_POINT( pCreature ) )
	{
		bIsOpen = true;
	}
	
	lua_pushboolean(L, bIsOpen );

	return 1;
}

// 获取帮会id
int script_get_param(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}
	
	lua_pushnumber(L, pMap->get_param());

	return 1;

}

// 获得副本难度
int script_get_inst_mode(lua_State* L)
{
	DWORD dw_map_id = lua_tonumber(L, 1);
	DWORD dw_instance_id = lua_tonumber(L, 2);

	Map* pMap = g_mapCreator.get_map(dw_map_id, dw_instance_id);
	if(!VALID_POINT(pMap))
		return 0;

	if(pMap->get_map_type() != EMT_Instance)
		return 0;

	lua_pushinteger(L, ((map_instance_normal*)pMap)->get_instance_hard());

	return 1;
}

// 获取地图上角色id
int script_get_map_all_role_id(lua_State* L)
{
	DWORD dw_map_id = lua_tonumber(L, 1);
	DWORD dw_instance_id = lua_tonumber(L, 2);

	Map* pMap = g_mapCreator.get_map(dw_map_id, dw_instance_id);
	if(!VALID_POINT(pMap))
	{
		return 0;
	}

	//if(pMap->get_map_type() != EMT_Team)
	//	return 0;

	if(pMap->get_role_num() <= 0)
		return 0;

	INT n_num = pMap->get_role_num();
	lua_createtable(L,n_num,0);

	package_map<DWORD, Role*>::map_iter iter = pMap->get_role_map().begin();
	Role* pRole = NULL;
	INT Index = 0;
	while(pMap->get_role_map().find_next(iter, pRole))
	{
		if(!VALID_POINT(pRole))
			continue;

		lua_pushnumber(L, Index);
		lua_pushnumber(L, pRole->GetID());
		lua_settable(L,-3);  
		++Index;
	}

	return 1;
}
//获取地图上的所有角色数量
int script_get_map_all_role_number(lua_State* L)
{
	DWORD dw_map_id = lua_tonumber(L, 1);
	DWORD dw_instance_id = lua_tonumber(L, 2);

	Map* pMap = g_mapCreator.get_map(dw_map_id, dw_instance_id);
	if(!VALID_POINT(pMap))
	{
		return 0;
	}

	lua_pushinteger(L, pMap->get_role_num());
	
	return 1;
}

int script_get_map_door_state(lua_State* L)
{
	DWORD dwMapID = lua_tonumber(L, 1);
	DWORD dwInstanceID = lua_tonumber(L, 2);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))
	{
		return 0;
	}

	if(pMap->get_map_type() != EMT_Instance)
	{
		return 0;
	}

	lua_newtable(L);
	int nAttCount = 0;

	for(int i = 0; i < MAX_INST_DOOR_NUM; i++)
	{
		DWORD dw_door_id = INVALID_VALUE;
		BOOL b_open = FALSE;
		((map_instance_normal*)pMap)->get_door_state(i, dw_door_id, b_open);
		if(dw_door_id != INVALID_VALUE)
		{
			lua_pushnumber(L, dw_door_id);
			lua_pushnumber(L, b_open);
			lua_settable(L, -3);
			nAttCount++;
		}
	}

	return nAttCount > 0 ? 1:0;

}

static int script_change_weather(lua_State* L)
{
	DWORD dwMapID		= lua_tonumber(L, 1);
	DWORD dwInstanceID	= lua_tonumber(L, 2);
	DWORD weather = lua_tonumber(L, 3);


	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap))
	{
		return 0;
	}

	const tagWeatherProto* pWeather = AttRes::GetInstance()->GetWeather(weather);
	if(!VALID_POINT(pWeather)){
		return 0;
	}

	pMap->SetWeather(weather);

	return 0;
}

//-------------------------------------------------------------------------------------
// 给地图所有的玩家增加一个buff
//-------------------------------------------------------------------------------------
int script_add_map_role_buff(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwBuffTypeID	=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		return 0;
	}

	const tagBuffProto* pProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
	if( !VALID_POINT(pProto) ) return 0;

	Map::ROLE_MAP::map_iter itRole = pMap->get_role_map().begin();
	Role* pRole = NULL;
	while (pMap->get_role_map().find_next(itRole, pRole))
	{
		pRole->TryAddBuff(pRole, pProto, NULL, NULL, NULL);
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------
// 玩家是否在某一脚本类型的区域
//-------------------------------------------------------------------------------------
int script_is_in_area(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	DWORD	dwObjID			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		lua_pushboolean(L, false);
		return 1;
	}

	const tag_map_info* pMapInfo = pMap->get_map_info();
	if( !VALID_POINT(pMapInfo) )
	{
		lua_pushboolean(L, false);
		return 1;
	}

	Role* pRole = pMap->find_role(dw_role_id);
	if( !VALID_POINT(pRole) )	
	{
		lua_pushboolean(L, false);
		return 1;
	}

	// 得到玩家的包裹盒和格子坐标
	//AABBox roleBox = pRole->GetAABBox();
	//INT nTileX = INT(pRole->GetCurPos().x / (FLOAT)TILE_SCALE);
	//INT nTileZ = INT(pRole->GetCurPos().z / (FLOAT)TILE_SCALE);

	tag_map_area_info* pArea = NULL;

	// 检测脚本区
	pArea = pMap->is_in_area(pMapInfo->map_script_area, pRole->GetCurPos().x, pRole->GetCurPos().z);
	if( !VALID_POINT(pArea) )
	{
		lua_pushboolean(L, false);
		return 1;
	}

	if (pArea->dw_att_id == dwObjID)
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}

	return 1;
}

// 是否在某个触发器
int script_is_in_trigger(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dw_role_id		=	lua_tonumber(L, 3);
	DWORD	dwObjID			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		lua_pushboolean(L, false);
		return 1;
	}

	const tag_map_info* pMapInfo = pMap->get_map_info();
	if( !VALID_POINT(pMapInfo) )
	{
		lua_pushboolean(L, false);
		return 1;
	}

	Role* pRole = pMap->find_role(dw_role_id);
	if( !VALID_POINT(pRole) )	
	{
		lua_pushboolean(L, false);
		return 1;
	}

	BOOL bIn = pMap->is_in_trigger(pRole, dwObjID);
	if (bIn)
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}

	return 1;
}
//-------------------------------------------------------------------------------------
// 获得怪物周围的怪物
//-------------------------------------------------------------------------------------
int script_get_around_creature(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	FLOAT	fOPRadius		=   lua_tonumber(L, 4);
	FLOAT	fHigh			=	lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		return 0;
	}

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) )
	{
		return 0;
	}

	std::vector<DWORD> vecCreature;
	INT nCreatureNum = 0;
	INT Index = 1;
	nCreatureNum = pCreature->GetAroundCreature(vecCreature, fOPRadius, fHigh);

	lua_createtable(L,nCreatureNum,0);

	std::vector<DWORD>::iterator it = vecCreature.begin();
	while(it != vecCreature.end())
	{
		lua_pushnumber(L, Index);
		lua_pushnumber(L, *it);
		lua_settable(L,-3);  
		++it;
		++Index;
	}

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得怪物周围的玩家
//-------------------------------------------------------------------------------------
int script_get_around_role(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	FLOAT	fOPRadius		=   lua_tonumber(L, 4);
	FLOAT	fHigh			=	lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		return 0;
	}

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) )
	{
		return 0;
	}

	std::vector<DWORD> vecRole;
	INT nRoleNum = 0;
	INT Index = 1;
	nRoleNum = pCreature->GetAroundRole(vecRole, fOPRadius, fHigh);

	lua_createtable(L,nRoleNum,0);

	std::vector<DWORD>::iterator it = vecRole.begin();
	while(it != vecRole.end())
	{
		lua_pushnumber(L, Index);
		lua_pushnumber(L, *it);
		lua_settable(L,-3);  
		++it;
		++Index;
	}

	return 1;
}

//-------------------------------------------------------------------------------------------
// 获得怪物的脚本数据
//-------------------------------------------------------------------------------------------
int script_get_creature_script_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	INT		nParamNum		=	lua_tointeger(L, 4);

	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) ) return 0;

	// 得到参数个数
	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex = lua_tointeger(L, i+5);
		if( nIndex < 0 || nIndex >= ESD_Creature ) return 0;

		dwValue = pCreature->GetScriptData(nIndex);
		lua_pushnumber(L, dwValue);
	}

	return nParamNum;
}

//-------------------------------------------------------------------------------------
// 设置怪物脚本通用数据
//-------------------------------------------------------------------------------------
int script_set_creature_script_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	INT		nParamNum		=	lua_tointeger(L, 4);

	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) ) return 0;

	// 参数个数
	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+5);
		dwValue		=	lua_tonumber(L, i*2+6);

		if( nIndex < 0 || nIndex >= ESD_Creature ) return 0;

		pCreature->SetScriptData(nIndex, dwValue);
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// 改变怪物脚本通用数据
//-------------------------------------------------------------------------------------
int script_mod_creature_script_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	INT		nParamNum		=	lua_tointeger(L, 4);

	INT		nIndex			=	0;
	DWORD	dwValue			=	INVALID_VALUE;

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) ) return 0;

	// 参数个数
	if( nParamNum <= 0 ) return 0;

	for(INT i = 0; i < nParamNum; ++i)
	{
		nIndex		=	lua_tointeger(L, i*2+5);
		dwValue		=	lua_tonumber(L, i*2+6);

		if( nIndex < 0 || nIndex >= ESD_Creature ) return 0;

		pCreature->ModScriptData(nIndex, dwValue);
	}

	return 0;
}

//------------------------------------------------------------------------------------------
// 设置怪物更新脚本AI的时间间隔，以tick为单位
//------------------------------------------------------------------------------------------
int script_set_creature_update_ai_timer(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	INT		nTimer			=	lua_tointeger(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	if( nTimer <= 0 && nTimer != INVALID_VALUE ) return 0;
	
	// 好大头的log
	if (dwCreatureID == 3020023)
	{
		g_world.get_log()->write_log(_T("Creature 3020023 SetScriptUpdateTimer:%d"), nTimer);
	}

	pCreature->GetAI()->SetScriptUpdateTimer(nTimer);

	return 0;
}

//------------------------------------------------------------------------------------------
// 怪物使用技能
//------------------------------------------------------------------------------------------
int script_creature_use_skill(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwSkillID		=	lua_tonumber(L, 4);
	INT		nTargetType		=	lua_tointeger(L, 5);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	// 得到目标ID
	DWORD dwTargetID = pCreature->GetAI()->GetTargetIDByType((ECreatureTargetFriendEnemy)nTargetType);

	// 使用技能
	pCreature->GetAI()->AIUseSkill(dwSkillID, dwTargetID);

	return 0;
}

//---------------------------------------------------------------------------------------------
// 怪物喊话
//----------------------------------------------------------------------------------------------
int script_monster_say(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwSayID			=	lua_tonumber(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) ) return 0;

	// 喊话
	pCreature->Say(dwSayID);

	return 0;
}

//----------------------------------------------------------------------------------------------
// 怪物播放动作
//----------------------------------------------------------------------------------------------
int script_monster_play_action(lua_State* L)
{
	DWORD		dwMapID			=	lua_tonumber(L, 1);
	DWORD		dwInstanceID	=	lua_tonumber(L, 2);
	DWORD		dwCreatureID	=	lua_tonumber(L, 3);
	const CHAR*	szFourCC		=	lua_tostring(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) ) return 0;

	// 检查是否合法，fourCC只能是四个大小的字符
	if( !VALID_POINT(szFourCC) || strlen(szFourCC) > 4 ) return 0;

	// 转换成ID
	DWORD dwActionID = 0;

	pCreature->PlayerAction(dwActionID);

	return 0;
}

//--------------------------------------------------------------------------------------------------
// 生物切换AI状态
//--------------------------------------------------------------------------------------------------
int script_creature_change_ai_state(lua_State* L)
{
	DWORD		dwMapID			=	lua_tonumber(L, 1);
	DWORD		dwInstanceID	=	lua_tonumber(L, 2);
	DWORD		dwCreatureID	=	lua_tonumber(L, 3);
	INT			nState			=	lua_tointeger(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	// 切换状态
	pCreature->GetAI()->ChangeState((AIStateType)nState);

	return 0;
}

//--------------------------------------------------------------------------------------------------
// 生物进入战斗后的时间
//--------------------------------------------------------------------------------------------------
int script_get_enter_combat_tick(lua_State* L)
{
	DWORD		dwMapID			=	lua_tonumber(L, 1);
	DWORD		dwInstanceID	=	lua_tonumber(L, 2);
	DWORD		dwCreatureID	=	lua_tonumber(L, 3);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	// 得到进入战斗时间
	DWORD dwTick = pCreature->GetAI()->GetEnterCombatTick();

	lua_pushnumber(L, dwTick);

	return 1;
}

//--------------------------------------------------------------------------------------------------
// 根据怪物ID得到TypeID
//--------------------------------------------------------------------------------------------------
int script_get_creature_type_id(lua_State* L)
{
	DWORD		dwMapID			=	lua_tonumber(L, 1);
	DWORD		dwInstanceID	=	lua_tonumber(L, 2);
	DWORD		dwCreatureID	=	lua_tonumber(L, 3);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	DWORD dw_data_id = pCreature->GetTypeID();

	lua_pushnumber(L, dw_data_id);

	return 1;
}

//-------------------------------------------------------------------------------------
// 增加怪物仇恨
//-------------------------------------------------------------------------------------
int script_add_enmity(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwTargetID		=	lua_tonumber(L, 4);
	DWORD	dwValue			=	lua_tonumber(L, 5);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	Unit* pTarget = pMap->find_unit(dwTargetID);
	if( !VALID_POINT(pTarget) ) return 0;

	pCreature->GetAI()->AddEnmity(pTarget, dwValue);

	return 0;
}
//清空一个玩家怪物仇恨
int script_clear_enmity(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dwCreatureID	=	lua_tonumber(L, 3);
	DWORD dwTargetID	=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap)) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if (!VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI())) return 0;

	pCreature->GetAI()->ClearEnmity(dwTargetID);

	return 0;
}
// 获取仇恨列表
int script_get_enmity_list(lua_State* L)
{
	DWORD dwMapID		=	lua_tonumber(L, 1);
	DWORD dwInstanceID	=	lua_tonumber(L, 2);
	DWORD dwCreatureID	=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if (!VALID_POINT(pMap)) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if (!VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI())) return 0;
	
	std::vector<tagEnmity*> vecEnmity;
	pCreature->GetAI()->GetEnmityList(vecEnmity);
	
	lua_newtable(L);
	for( std::size_t i = 0; i < vecEnmity.size(); i++ )
	{
		lua_pushnumber(L, vecEnmity[i]->dwID);
		lua_pushnumber(L, vecEnmity[i]->nEnmity + vecEnmity[i]->nEnmityMod);
		lua_settable(L, -3);
	}
	return 1;
}
//-------------------------------------------------------------------------------------
// 获取怪物当前的攻击目标
//-------------------------------------------------------------------------------------
int script_get_creature_cur_target_id(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) )
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
	{
		lua_pushnumber(L, INVALID_VALUE);
		return 1;
	}

	DWORD	dwTargetID = pCreature->GetAI()->GetTargetUnitID();

	lua_pushnumber(L, dwTargetID);

	return 1;
}
//-------------------------------------------------------------------------------------
// 获取怪物属性
//-------------------------------------------------------------------------------------
int script_get_creature_att_value(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCretureID		=	lua_tonumber(L, 3);
	INT		nIndex			=	lua_tointeger(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Creature* pCreature = pMap->find_creature(dwCretureID);
	if(!VALID_POINT(pCreature))	return 0;

	if( nIndex < 0 || nIndex >= ERA_End ) return 0;

	lua_pushinteger(L, pCreature->GetAttValue(nIndex));
	return 1;
}
//-------------------------------------------------------------------------------------
// 怪物移动
//-------------------------------------------------------------------------------------
int script_monster_move(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCretureID		=	lua_tonumber(L, 3);
	
	INT		nX				=	lua_tointeger(L, 4);
	INT		nY				=	lua_tointeger(L, 5);
	INT		nZ				=	lua_tointeger(L, 6);
	
	INT		nMoveType		=	lua_tointeger(L, 7);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCretureID);
	if( !VALID_POINT(pCreature) ) return 0;
	
	if (MoveData::EMR_Success == pCreature->GetMoveData().StartCreatureWalk(Vector3(nX, nY, nZ), (EMoveState)nMoveType))
	{
		lua_pushinteger(L, 1);
	}
	else
	{
		lua_pushinteger(L, 0);
	}

	return 1;
}

// 怪物移动(路点版本)
int script_monster_move_point(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCretureID		=	lua_tonumber(L, 3);

	INT		nwayPoint		=	lua_tonumber(L, 4);
	INT		nMoveType		=	lua_tointeger(L, 5);


	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCretureID);
	if( !VALID_POINT(pCreature) ) return 0;
	
	AIController* pAI = pCreature->GetAI();
	if (!VALID_POINT(pAI)) return 0;

	BOOL bSuss = pAI->MoveWayPoint(nwayPoint, (EMoveState)nMoveType);

	lua_pushboolean(L, bSuss);

	return 1;
}
// 清空仇恨
int script_clear_all_enmity(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	pCreature->GetAI()->ClearAllEnmity();

	return 0;
}

//-------------------------------------------------------------------------------------
// 设置怪物是否能被攻击
//-------------------------------------------------------------------------------------
//int SC_SetCanBeAttack(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//	BOOL	bCanBeAttack	=	lua_tonumber(L, 4);
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;
//
//	pCreature->SetCanBeAttack(bCanBeAttack);
//
//	return 0;
//}
//-------------------------------------------------------------------------------------
// 设置怪物类型
//-------------------------------------------------------------------------------------
int script_set_creature_type(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	ECreatureType	dwType	=	(ECreatureType)lua_tointeger(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;

	pCreature->SetCreatureType(dwType);

	return 0;
}
//设置跟随目标
int script_set_follow_target(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwRole			=	lua_tonumber(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) return 0;
	
	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;
	
	// 有跟随目标了
	Role* pTarget = pCreature->GetAI()->GetTracker()->GetTarget();
	if (VALID_POINT(pTarget))
	{
		pTarget->SetFlowUnit(INVALID_VALUE);
	}
	
	// 主人有被更随目标了
	DWORD dwFlowID = pRole->GetFlowUnit();
	Creature* pFlow = pMap->find_creature(dwFlowID);
	if (VALID_POINT(pFlow))
	{
		pFlow->GetAI()->GetTracker()->SetTarget(NULL);
		pCreature->GetAI()->GetTracker()->SetTargetID(INVALID_VALUE);
		pFlow->OnDisappear();
	}
	pCreature->GetAI()->GetTracker()->SetTarget(pRole);
	pCreature->GetAI()->GetTracker()->SetTargetID(pRole->GetID());
	pRole->SetFlowUnit(dwCreatureID);
	return 0;
}

// 设置怪物所属帮会
int script_set_guild_id(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwGuildID		=	lua_tonumber(L, 4);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
		return 0;
	
	pCreature->SetGuildID(dwGuildID);

	return 0;

}


// 获取怪物所属帮会
int script_get_guild_id(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);


	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
		return 0;

	lua_pushnumber(L, pCreature->GetGuildID());

	return 1;

}
//int script_set_guild_plant_index(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//	int		nIndex			=	lua_tonumber(L, 4);
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
//		return 0;
//
//	pCreature->SetPlantDataIndex(nIndex);
//
//	return 0;
//}
//int script_set_mound(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//	DWORD 	dwMoundID		=	lua_tonumber(L, 4);
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
//		return 0;
//
//	pCreature->SetTuDui(dwMoundID);
//
//	return 0;
//}
//
//int script_set_max_yield(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//	DWORD 	dwYield			=	lua_tonumber(L, 4);
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
//		return 0;
//
//	pCreature->SetMaxYield(dwYield);
//
//	return 0;
//}

// 怪物瞬移
int script_teleport(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	INT		nX				=	lua_tointeger(L, 4);
	INT		nY				=	lua_tointeger(L, 5);
	INT		nZ				=	lua_tointeger(L, 6);

	Vector3 vDest(nX * TILE_SCALE, nY * TILE_SCALE, nZ * TILE_SCALE);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
		return 0;

	// 得到一个最大合理的终点
	Vector3 vRealDest;

	pathNode nearPos;
	if( !pCreature->get_map()->if_can_direct_go(pCreature->GetCurPos().x, pCreature->GetCurPos().z, vDest.x, vDest.z, nearPos) )
	{
		vRealDest.x = nearPos.x();
		vRealDest.z = nearPos.y();
	}
	else
	{
		vRealDest = vDest;
	}
	

	// 如果两个点不相等，则瞬移
	if( pCreature->GetCurPos() != vRealDest )
	{
		// 给客户端发送消息
		//NET_SIS_special_move send;
		//send.dw_role_id = pCreature->GetID();
		//send.eType = ESMT_Teleport;
		//send.vDestPos = vRealDest;
		//pCreature->get_map()->send_big_visible_tile_message(pCreature, &send, send.dw_size);

		// 瞬移，但不发送消息
		pCreature->GetMoveData().ForceTeleport(vRealDest, TRUE);
	}
}
//gx add 6.17
int script_rolekillcreature(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
	DWORD	dwRoleID		=	lua_tonumber(L, 4);
	
	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Creature* pCreature = pMap->find_creature(dwCreatureID);
	if(!VALID_POINT(pCreature)) 
		return 0;
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;
	pCreature->OnDead(pRole);
	return 0;
}

//int script_get_guild_plant_index(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
//		return 0;
//
//	lua_pushnumber(L, pCreature->GetPlantDataIndex());
//
//	return 1;
//}
//
//int script_set_used(lua_State* L)
//{
//	DWORD	dwMapID			=	lua_tonumber(L, 1);
//	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
//	DWORD	dwCreatureID	=	lua_tonumber(L, 3);
//	BOOL	bUsed			=	lua_toboolean(L, 4);
//
//	// 找到地图和怪物
//	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
//	if( !VALID_POINT(pMap) ) return 0;
//
//	Creature* pCreature = pMap->find_creature(dwCreatureID);
//	if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()) ) 
//		return 0;
//
//	pCreature->SetUsed(bUsed);
//
//	return 0;
//}

//-------------------------------------------------------------------------------------
// 获得玩家升级所需经验
//-------------------------------------------------------------------------------------
int script_get_role_level_up_exp(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	return 0;

	// 获得当前升级所需经验
	INT32 nID = pRole->GetClass()*1000 + pRole->get_level();
	const s_level_up_effect* pEffect = AttRes::GetInstance()->GetLevelUpEffect(nID);
	ASSERT(VALID_POINT(pEffect));
	lua_pushinteger(L, pEffect->n_exp_level_up_);
	return 1;
}

//-------------------------------------------------------------------------------------
// 玩家是否在线
//-------------------------------------------------------------------------------------
int script_is_role_online(lua_State* L)
{
	DWORD   dwMapID			=	lua_tonumber(L, 1);
	DWORD   dwInstanceID	=	lua_tonumber(L, 2);
	DWORD   dw_role_id		=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if(!VALID_POINT(pMap))	return 0;

	Role* pRole = pMap->find_role(dw_role_id);
	if(!VALID_POINT(pRole))	
	{
		lua_pushinteger(L, 0);
	}
	else
	{
		lua_pushinteger(L, 1);
	}

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮派成员职位
//---------------------------------------------------------------------------------
static int script_get_guild_pos(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);
	DWORD dw_role_id	= lua_tonumber(L, 2);

	guild* pGuild = NULL;
	if (!VALID_VALUE(dwGuildID))
	{
		Role* pRole = g_roleMgr.get_role(dw_role_id);
		if (!VALID_POINT(pRole))
		{
			// 不在线
			return 0;
		}
		dwGuildID = pRole->GetGuildID();
		if (!VALID_VALUE(dwGuildID))
		{
			// 不在帮派
			return 0;
		}
	}
	pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		// 帮派不存在
		return 0;
	}

	tagGuildMember* pMember = pGuild->get_member(dw_role_id);
	if (!VALID_POINT(pMember))
	{
		// 不在帮派中
		return 0;
	}

	lua_pushinteger(L, pMember->eGuildPos);

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮派资金
//---------------------------------------------------------------------------------
static int script_get_guild_fund(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().nFund);

	return 1;
}

// 获取帮会等级
static int script_get_guild_level(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().byLevel);

	return 1;
}
// 设置帮会种植数据
int script_set_guild_plant_data(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	DWORD dwPlantID = lua_tonumber(L, 2);
	DWORD dwRoleID = lua_tonumber(L, 3);
	INT	nNum = lua_tonumber(L, 4);
	INT nIndex = lua_tonumber(L, 5);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}
	
	pGuild->setPlantData(dwPlantID, dwRoleID, nNum, nIndex);
	return 0;
}

int script_can_open_statue(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole))
		return 0;
	
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return 0;
	
	tagGuildMember* pGuildMember = pGuild->get_member(pRole->GetID());
	if(!VALID_POINT(pGuildMember))
		return 0;

	lua_pushboolean(L, pGuild->get_guild_power(pGuildMember->eGuildPos).bLeague);

	return 1;
}

int script_active_daogao(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwIndex = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole))
		return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return 0;

	tagGuildMember* pGuildMember = pGuild->get_member(pRole->GetID());
	if(!VALID_POINT(pGuildMember))
		return 0;

	pGuild->SetDaogao(dwIndex, TRUE);

	return 0;
}

int script_get_daogao_num(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
		return 0;

	lua_pushnumber(L, pGuild->GetDaogaoNumber());

	return 1;
}

int script_get_build_level(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	DWORD dwType = lua_tonumber(L ,2);


	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
		return 0;

	lua_pushnumber(L, pGuild->get_upgrade().GetFacilitiesLevel(dwType));

	return 1;
}

int script_set_guild_win(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
		return 0;

	pGuild->get_guild_war().m_bWin = TRUE;

	return 0;

}

int script_set_guild_lose(lua_State* L)
{

	DWORD dwGuildID = lua_tonumber(L, 1);
	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
		return 0;

	pGuild->get_guild_war().m_bLost = TRUE;

	return 0;
}

int script_get_jujue_war_time(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
		return 0;
	
	char szTemp[X_SHORT_NAME] = "";
	DwordTime2DataTime(szTemp, X_SHORT_NAME, pGuild->get_guild_att().dwJujueTime);	


	lua_pushstring(L, szTemp);
	return 1;
}
//---------------------------------------------------------------------------------
// 获取帮派资材
//---------------------------------------------------------------------------------
static int script_get_guild_material(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().nMaterial);

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮派安定度
//---------------------------------------------------------------------------------
static int script_get_guild_peace(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().n16Peace);

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮派声望
//---------------------------------------------------------------------------------
static int script_get_guild_reputation(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().nReputation);

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮会成员帮贡
//---------------------------------------------------------------------------------
static int script_get_guild_contribution(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);
	DWORD dw_role_id	= lua_tonumber(L, 2);

	guild* pGuild = NULL;
	if (!VALID_VALUE(dwGuildID))
	{
		Role* pRole = g_roleMgr.get_role(dw_role_id);
		if (!VALID_POINT(pRole))
		{
			// 不在线
			return 0;
		}
		dwGuildID = pRole->GetGuildID();
		if (!VALID_VALUE(dwGuildID))
		{
			// 不在帮派
			return 0;
		}
	}
	pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		// 帮派不存在
		return 0;
	}

	tagGuildMember* pMember = pGuild->get_member(dw_role_id);
	if (!VALID_POINT(pMember))
	{
		// 不在帮派中
		return 0;
	}

	lua_pushinteger(L, pMember->nContribution);

	return 1;
}

//---------------------------------------------------------------------------------
//获取帮会繁荣度
//---------------------------------------------------------------------------------
static int script_get_guild_prosperity(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().nProsperity);

	return 1;
}

//---------------------------------------------------------------------------------
//获取帮会脚本数据
//---------------------------------------------------------------------------------
static int script_get_guild_script_data(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	INT	  nIndex	= lua_tointeger(L, 2);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if(nIndex < 0 || nIndex >=64)
		return 0;

	lua_pushinteger(L, pGuild->get_guild_att().n32ScriptData[nIndex]);

	return 1;
}

//---------------------------------------------------------------------------------
// 检查帮派状态
//---------------------------------------------------------------------------------
static int script_is_guild_in_status(lua_State* L)
{
	DWORD dwGuildID	= lua_tonumber(L, 1);
	DWORD dwStatus	= lua_tonumber(L, 2);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushboolean(L, pGuild->is_in_guild_state_all(dwStatus));

	return 1;
}
//---------------------------------------------------------------------------------
// 判断玩家行会是否是SBK行会
//---------------------------------------------------------------------------------
static int script_is_SBK_guild(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) )
	{
		lua_pushboolean(L,false);
		return 1;
	}
	if (pRole->GetGuildID() != INVALID_VALUE)
	{
		if (g_guild_manager.get_SBK_guild() == pRole->GetGuildID())
		{
			lua_pushboolean(L,true);
			return 1;
		}
		else
		{
			lua_pushboolean(L,false);
			return 1;
		}
	}
	else
	{
		lua_pushboolean(L,false);
		return 1;
	}
}

//---------------------------------------------------------------------------------
// 初始化帮会pvp数据
//---------------------------------------------------------------------------------
static int script_init_guild_pvp_data(lua_State* L)
{
	INT nActID = lua_tointeger(L, 1);

	activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(nActID);
	if(!VALID_POINT(pActivity))
		return 0;

	s_guild_pvp_data* pGuildPvPData = g_guild_manager.get_pvp_data(nActID);
	if(!VALID_POINT(pGuildPvPData))
		return 0;

	pGuildPvPData->Reset();
	g_guild_manager.update_pvp_data(nActID);

	// 清空pvp副本
	DWORD dwPvPInstanceID = pGuildPvPData->dw_instance_id;
	if( VALID_POINT(dwPvPInstanceID) )
	{
		DWORD dwMapID = pActivity->get_info()->dw_map_id;
		Map* pMap = g_mapCreator.get_map(dwMapID, dwPvPInstanceID);

		if( VALID_POINT(pMap) )
		{
			map_instance_pvp* pPvPInstance = dynamic_cast<map_instance_pvp*>(pMap);
			if( VALID_POINT(pPvPInstance) )
			{
				//pPvPInstance->on_act_end();
			}
		}
	}

	return 1;
}

//---------------------------------------------------------------------------------
// 帮派圣兽使用数量
//---------------------------------------------------------------------------------
int script_get_guild_challenge_value(lua_State* L)
{
	DWORD	dw_guild_id = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	tagGuildFacilitiesInfo st_Info;
	pGuild->get_upgrade().GetGuildFacilitiesInfo(&st_Info, EFT_Holiness);

	lua_pushinteger(L, st_Info.byUseNum);

	return 1;
}

//---------------------------------------------------------------------------------
// 修改帮派圣兽使用数量
//---------------------------------------------------------------------------------
int script_mod_guild_challenge_value(lua_State* L)
{
	DWORD		dw_guild_id = lua_tonumber(L, 1);
	BYTE		by_use_num = (BYTE)lua_tointeger(L, 2);

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	pGuild->get_upgrade().ChangeFacilitiesUseNum(EFT_Holiness, by_use_num);

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派圣兽阶数
//---------------------------------------------------------------------------------
int script_get_guild_challenge_step(lua_State* L)
{
	DWORD	dw_guild_id = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	tagGuildFacilitiesInfo st_Info;
	pGuild->get_upgrade().GetGuildFacilitiesInfo(&st_Info, EFT_Holiness);

	lua_pushinteger(L, st_Info.byStep);

	return 1;
}

//---------------------------------------------------------------------------------
// 获取帮主id
//---------------------------------------------------------------------------------
int script_get_guild_leader_id(lua_State* L)
{
	DWORD	dw_guild_id = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushnumber(L, pGuild->get_guild_att().dwLeaderRoleID);

	return 1;

}

// 获取帮会脚本数据索引为6的前十号帮会id
int script_get_guild_script_data_six_list(lua_State* L)
{
	std::map<int, DWORD> mapScriptData;
	g_guild_manager.get_script_data_six_list(mapScriptData);

	lua_newtable(L);
	std::map<int, DWORD>::reverse_iterator iter = mapScriptData.rbegin();
	for( int i = 0; iter != mapScriptData.rend() && i < 10; iter++, i++)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, iter->second);
		lua_settable(L, -3);
	}

	return 1;

}
//---------------------------------------------------------------------------------
// 帮派资金变更
//---------------------------------------------------------------------------------
static int script_modify_guild_fund(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT32 nFund			= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	if (nFund == 0)
	{
		return 0;
	}

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if (nFund > 0)
	{
		pGuild->increase_guild_fund(dw_role_id, nFund, dwLogCmdID);
		pGuild->increase_member_total_fund(dw_role_id, nFund/10000);
	}
	else
	{
		pGuild->decrease_guild_fund(dw_role_id, -nFund, dwLogCmdID);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派资材变更
//---------------------------------------------------------------------------------
static int script_modify_guild_material(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT32 nMaterial		= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	if (nMaterial == 0)
	{
		return 0;
	}

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if (nMaterial > 0)
	{
		pGuild->increase_guild_material(dw_role_id, nMaterial, dwLogCmdID);
	}
	else
	{
		pGuild->decrease_guild_material(dw_role_id, -nMaterial, dwLogCmdID);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派安定度变更
//---------------------------------------------------------------------------------
static int script_modify_guild_peace(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT16 n16Peace		= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	if (n16Peace == 0)
	{
		return 0;
	}

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if (n16Peace > 0)
	{
		pGuild->increase_guild_peace(dw_role_id, n16Peace, dwLogCmdID);
	}
	else
	{
		pGuild->decrease_guild_peace(dw_role_id, -n16Peace, dwLogCmdID);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派成员威望变更
//---------------------------------------------------------------------------------
static int script_modify_guild_reputation(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT32 nReputation	= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	if (nReputation == 0)
	{
		return 0;
	}

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if (nReputation > 0)
	{
		pGuild->increase_guild_reputation(dw_role_id, nReputation, dwLogCmdID);
	}
	else
	{
		pGuild->decrease_guild_reputation(dw_role_id, -nReputation, dwLogCmdID);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派成员贡献变更
//---------------------------------------------------------------------------------
static int script_modify_contribution(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT32 nContribute	= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	guild* pGuild = NULL;
	if (!VALID_VALUE(dwGuildID))
	{
		Role* pRole = g_roleMgr.get_role(dw_role_id);
		if (!VALID_POINT(pRole))
		{
			// 不在线
			return 0;
		}
		dwGuildID = pRole->GetGuildID();
		if (!VALID_VALUE(dwGuildID))
		{
			// 不在帮派
			return 0;
		}
	}
	pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		// 帮派不存在
		return 0;
	}

	if (nContribute > 0)
	{
		pGuild->change_member_contribution(dw_role_id, nContribute, TRUE);
	}
	else
	{
		pGuild->change_member_contribution(dw_role_id, nContribute, FALSE);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派繁荣度变更
//---------------------------------------------------------------------------------
int script_modify_prosperity(lua_State* L)
{
	DWORD dwGuildID		= lua_tonumber(L, 1);
	DWORD dw_role_id		= lua_tonumber(L, 2);
	INT32 n16Prosperity		= lua_tonumber(L, 3);
	DWORD dwLogCmdID	= lua_tointeger(L, 4);

	if (n16Prosperity == 0)
	{
		return 0;
	}

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if (n16Prosperity > 0)
	{
		pGuild->increase_prosperity(dw_role_id, n16Prosperity, dwLogCmdID);
	}
	else
	{
		pGuild->decrease_prosperity(dw_role_id, n16Prosperity, dwLogCmdID);
	}

	return 0;
}

//---------------------------------------------------------------------------------
// 帮派脚本数据变更
//---------------------------------------------------------------------------------
int script_modify_script_data(lua_State* L)
{
	DWORD dwGuildID = lua_tonumber(L, 1);
	INT	  nIndex	= lua_tointeger(L, 2);
	INT   n_data	= lua_tointeger(L, 3);

	guild* pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	if(nIndex < 0 || nIndex >=64)
		return 0;

	pGuild->set_script_data(nIndex, n_data);

	return 0;
}

int script_can_gather( lua_State* L )
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	DWORD	dwCreatureID	=	lua_tonumber(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) || !VALID_POINT(pRole->get_map())) return 0;

	Creature* pRes = pRole->get_map()->find_creature(dwCreatureID);
	if (!VALID_POINT(pRes)) return 0;

	INT nRt = pRole->CanGather(pRes);

	lua_pushinteger(L, nRt);

	return 1;
}

//-----------------------------------------------------------------------------
//	打印整型信息
//-----------------------------------------------------------------------------
int script_print(lua_State *L)
{
	INT info = static_cast<INT>(lua_tointeger(L, -1));
	print_message(_T("%d\r\n"), info);

	return 0;
}

//-----------------------------------------------------------------------------
// LuaConsolePrint
// Print a console string
// the stack.
//-----------------------------------------------------------------------------
int script_lua_console_print(lua_State* l)
{
	LPCSTR szText = (char*)(lua_tostring(l, -1));	// UTF-8
	std::string str;
	if( !VALID_POINT(szText) )
	{
		str = luaL_typename(l, -1);
		str.append("[?]");
		szText = str.c_str();
	}

	get_window()->print(get_tool()->unicode8_to_unicode(szText));
	return(0);
}

int script_system_mail(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	tstring Name(get_tool()->ansi_to_unicode(lua_tostring(L, 2)));
	tstring Content(get_tool()->ansi_to_unicode(lua_tostring(L, 3)));
	DWORD	dwTypeID1		=   lua_tonumber(L, 4);
	INT16	n16Qlty1		=   lua_tointeger(L, 5);
	DWORD	dwTypeID2		=   lua_tonumber(L, 6);
	INT16	n16Qlty2		=   lua_tointeger(L, 7);
	DWORD	dwTypeID3		=   lua_tonumber(L,	8);
	INT16	n16Qlty3		=   lua_tointeger(L, 9);
	INT		nItemNum		=   lua_tointeger(L, 10);
	DWORD	dwSolve			=	lua_tonumber(L, 11);						
	DWORD	dwGiveMoney		=   lua_tonumber(L, 12);
	BYTE	by_type			=   lua_tointeger(L, 13);
	INT		n_num1			=   lua_tointeger(L, 14);
	INT		n_num2			=	lua_tointeger(L, 15);
	INT		n_num3			=	lua_tointeger(L, 16);
	INT     n_yuanbao_type	=	lua_tointeger(L, 17);


	tagMailBase st_MailBase;
	ZeroMemory(&st_MailBase, sizeof(st_MailBase));
	st_MailBase.dwSendRoleID = INVALID_VALUE;
	st_MailBase.dwRecvRoleID = dw_role_id;
	st_MailBase.dwSolve = dwSolve;
	st_MailBase.dwGiveMoney = dwGiveMoney;
	st_MailBase.byType = by_type;
	st_MailBase.n_yuanbao_type = n_yuanbao_type;
	DWORD dwItemType[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
	dwItemType[0] = dwTypeID1;
	dwItemType[1] = dwTypeID2;
	dwItemType[2] = dwTypeID3;

	INT16 dwQlty[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
	dwQlty[0] = n16Qlty1;
	dwQlty[1] = n16Qlty2;
	dwQlty[2] = n16Qlty3;

	INT	  n_num[Max_Item_Num] = {1, 1, 1};
	n_num[0] = n_num1;
	n_num[1] = n_num2;
	n_num[2] = n_num3;
	g_mailmgr.CreateMail(st_MailBase, Name.c_str(), Content.c_str(), dwItemType, dwQlty, n_num, nItemNum);

	return 0;
}

int script_is_all_equip_level_has(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber( L, 1 );
	DWORD dwLevel = lua_tonumber(L, 2);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if( !VALID_POINT(pRole) ) return 0;

	BOOL bRes = pRole->IsAllEquipLevelHas(dwLevel, TRUE);

	lua_pushboolean(L, bRes);

	return 1;
}

int script_is_all_def_equip_level_has(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwLevel = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if ( !VALID_POINT(pRole)) return 0;

	BOOL bRes = pRole->IsAllEquipLevelHas(dwLevel, FALSE);

	lua_pushboolean(L, bRes);
	return 1;
}
int script_get_role_camp(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole)) return 0;

	ECamp	e_role_camp = pRole->get_role_camp();
	ECamp	e_temp_role_camp = pRole->get_temp_role_camp();

	lua_pushinteger(L, e_role_camp);
	lua_pushinteger(L, e_temp_role_camp);

	return 2;
}

int script_set_role_camp(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	ECamp e_role_camp = (ECamp)lua_tointeger(L, 2);
	ECamp e_temp_role_camp = (ECamp)lua_tointeger(L, 3);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole)) return 0;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap)) return 0;

	pRole->set_role_camp(e_role_camp);
	pRole->set_temp_role_camp(e_temp_role_camp);

	NET_SIS_change_role_camp send;
	send.e_role_camp = e_role_camp;
	send.e_temp_role_camp = e_temp_role_camp;
	pMap->send_big_visible_tile_message(pRole, &send, send.dw_size);

	return 0;
}
int script_get_role_vigour(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole)) return 0;

	INT iVigour = pRole->GetAttValue(ERA_Fortune);
	lua_pushinteger(L, iVigour);
	return 1;
}
int script_inc_role_vigour(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT   iDelta = (INT)lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole)) return 0;
	
	pRole->ModAttValue(ERA_Fortune, iDelta);
	return 0;
}

int script_get_item_script_data(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT64 n64_Item_Serial = pop_64bit_data(L, 2, 3);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(n64_Item_Serial);
	if(!VALID_POINT(pItem))
		return 0;

	lua_pushnumber(L, pItem->dw_script_data[0]);
	lua_pushnumber(L, pItem->dw_script_data[1]);

	return 2;
}

int script_set_item_script_data(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT64 n64_Item_Serial = pop_64bit_data(L, 2, 3);
	DWORD dw_script_data1 = lua_tonumber(L, 4);
	DWORD dw_script_data2 = lua_tonumber(L, 5);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(n64_Item_Serial);
	if(!VALID_POINT(pItem))
		return 0;

	pItem->dw_script_data[0] = dw_script_data1;
	pItem->dw_script_data[1] = dw_script_data2;
	pItem->SetUpdate(EUDBS_Update);

	return 0;
}

// 取得typeid的物品列表
int script_get_item_serial_id(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwItemTypeID = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	package_list<tagItem*> list;
	pRole->GetItemMgr().GetBagSameItemList(list, dwItemTypeID);


	lua_newtable(L);
	tagItem* pItem = NULL;
	list.reset_iterator();
	while(list.find_next(pItem))
	{
		push_64bit_data(L, pItem->GetKey());
		lua_settable(L, -3);	
	}
	return 1;
}
// 更改PK值
int script_change_PK_value(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT	  nPKDec = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	pRole->SetPKValueMod(nPKDec);
	return 0;
}
// 取得PK值
int script_get_PK_value(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	lua_pushnumber(L, pRole->GetPKValue());
	return 1;
}

// 取得当前召唤的宠物
int script_get_called_pet(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pPetSoul = pPetPock->GetCalledPetSoul();
	if (!VALID_POINT(pPetSoul))
		return 0;

	Pet* pPet = pPetSoul->GetBody();
	if (!VALID_POINT(pPet))
		return 0;

	lua_pushnumber(L, pPet->GetID());
	return 1;
}

int script_get_called_pet_type_id(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pPetSoul = pPetPock->GetCalledPetSoul();
	if (!VALID_POINT(pPetSoul))
		return 0;

	lua_pushnumber(L, pPetSoul->GetProtoID());

	return 1;
}

int script_delete_pet(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwPetID = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pSoul = pPetPock->GetAway(dwPetID, TRUE);
	if (VALID_POINT(pSoul))
	{
		PetSoul::DeleteSoul(pSoul, TRUE);

		lua_pushboolean(L, true);
		return 1;
	}

	return 0;
}

// 增加宠物经验
int script_add_pet_exp(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	DWORD dwPetID	=		lua_tonumber(L, 2);
	DWORD dwExp		=		lua_tonumber(L, 3);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pPetSoul = pPetPock->GetPetSoul(dwPetID);
	if (!VALID_POINT(pPetSoul))
		return 0;

	pPetSoul->GetPetAtt().ExpChange(dwExp, TRUE);

	return 0;
}

int script_add_mounts_exp(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	DWORD dwExp		=		lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	
	pRole->GetRaidMgr().ExpChange(dwExp, TRUE);

	return 0;
}

int script_mounts_level(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	
	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;


	INT nLevel  = pRole->GetRaidMgr().getLevel();

	lua_pushnumber(L, nLevel);

	return 1;

}
// 判断超级密码是否验证过
int script_get_check_safe_code(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;
	
	BOOL bOK = pRole->get_check_safe_code();

	lua_pushboolean(L, bOK);

	return 1;
}

// 判断是否击杀帮会boss
int script_is_monster_kill(lua_State* L)
{
	DWORD	dw_monster_id = lua_tonumber(L, 1);
	DWORD	dw_guild_id = lua_tonumber(L, 2);

	if(dw_monster_id == INVALID_VALUE)
		return 0;

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if(!VALID_POINT(pGuild))
		return 0;

	BOOL bKill = g_guild_manager.is_monster_kill(dw_monster_id);

	if(!bKill)
	{
		g_guild_manager.add_monster_kill(dw_monster_id, dw_guild_id);
	}

	lua_pushboolean(L, bKill);

	return 1;
}

// 获取角色活跃度数据
int script_get_active_data(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT nIndex = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(nIndex < 0 || nIndex > MAX_ACTIVE_DATA)
		return 0;

	DWORD dw_active = pRole->m_n32_active_data[nIndex];

	lua_pushnumber(L, dw_active);

	return 1;
}

int script_get_guild_active_data(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT nIndex = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(nIndex < 0 || nIndex > MAX_ACTIVE_DATA)
		return 0;

	DWORD dw_active = pRole->m_n32_guild_active_data[nIndex];

	lua_pushnumber(L, dw_active);

	return 1;
}

// 设置角色活跃度数据
int script_set_active_data(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT		nIndex = lua_tointeger(L, 2);
	DWORD	dw_value = lua_tonumber(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(nIndex < 0 || nIndex > MAX_ACTIVE_DATA)
		return 0;

	pRole->m_n32_active_data[nIndex] = dw_value;

	pRole->SendActiveInfo();

	return 0;
}

int script_set_guild_active_data(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT		nIndex = lua_tointeger(L, 2);
	DWORD	dw_value = lua_tonumber(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(nIndex < 0 || nIndex > MAX_ACTIVE_DATA)
		return 0;

	pRole->m_n32_guild_active_data[nIndex] = dw_value;

	pRole->SendGuildActiveInfo();

	return 0;
}


// 获取角色活跃度领奖标志
int script_get_active_receive(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT	  nIndex = lua_tointeger(L, 2);

	if(nIndex < 0 || nIndex > MAX_ACTIVE_RECEIVE)
		return 0;

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	BOOL b = pRole->m_b_active_receive[nIndex];

	lua_pushboolean(L, b);

	return 1;
}

int script_get_guild_active_receive(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT	  nIndex = lua_tointeger(L, 2);

	if(nIndex < 0 || nIndex > MAX_GUILD_ACTIVE_RECEIVE)
		return 0;

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	BOOL b = pRole->m_b_guild_active_receive[nIndex];

	lua_pushboolean(L, b);

	return 1;
}


// 设置角色活跃度领取标志
int script_set_active_receive(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT	  nIndex = lua_tointeger(L, 2);
	BOOL  b		= lua_toboolean(L, 3);

	if(nIndex < 0 || nIndex > MAX_ACTIVE_RECEIVE)
		return 0;

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	pRole->m_b_active_receive[nIndex] = b;

	pRole->SendActiveInfo();

	return 0;
}

int script_set_guild_active_receive(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT	  nIndex = lua_tointeger(L, 2);
	BOOL  b		= lua_toboolean(L, 3);

	if(nIndex < 0 || nIndex > MAX_GUILD_ACTIVE_RECEIVE)
		return 0;

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	pRole->m_b_guild_active_receive[nIndex] = b;

	pRole->SendGuildActiveInfo();

	return 0;
}

// 获取角色活跃度
int script_get_active_value(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	INT dw_value = pRole->m_n32_active_num;

	lua_pushinteger(L, dw_value);

	return 1;
}

int script_get_guild_active_value(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	INT dw_value = pRole->m_n32_guild_active_num;

	lua_pushinteger(L, dw_value);

	return 1;
}


// 设置角色活跃度
int script_set_active_value(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT32 n32_value = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	//if(n32_value < 0 || n32_value > 100)
	//	return 0;

	pRole->m_n32_active_num = n32_value;

	pRole->SendActiveInfo();

	return 0;
}

int script_set_guild_active_value(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT32 n32_value = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	//if(n32_value < 0 || n32_value > 100)
	//	return 0;

	pRole->m_n32_guild_active_num = n32_value;

	pRole->SendGuildActiveInfo();

	return 0;
}

int script_inc_quest_refresh_number(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);
	INT		delta			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;

	pRole->IncCirleRefresh(delta);

	return 0;
}

int script_get_quest_refresh_number(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;

	lua_pushinteger(L, pRole->GetCircleRefrshNumber( )) ;

	return 1;
}
int script_mod_day_clear_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);
	DWORD	dwIndex			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;

	pRole->ModRoleDayClearDate(dwIndex);

	return 0;
}
int script_get_day_clear_data(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);
	DWORD	dwIndex			=	lua_tonumber(L, 4);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;

	lua_pushnumber(L, pRole->GetDayClearData(dwIndex));

	return 1;
}

// 获取跟随的单位
int script_get_flow_unit(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;
	
	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;
	
	DWORD dwFlowID = pRole->GetFlowUnit();
	Creature* pFlow = pMap->find_creature(dwFlowID);
	if (VALID_POINT(pFlow))
	{
		lua_pushinteger(L, dwFlowID);
		return 1;
	}


	return 0;
}
int script_is_war(lua_State* L)
{
	DWORD	dw_guild_id = lua_tonumber(L, 1);
	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if(!VALID_POINT(pGuild))
		return 0;





	BOOL bwar = false;
	if (/*pGuild->get_guild_war().get_guild_war_state() == EGWS_Prepare ||*/
		pGuild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay ||
		pGuild->get_guild_war().get_guild_war_state() == EGWS_WAR)
	{
		bwar = true;
	}
	
	lua_pushboolean(L, bwar);

	return 1;
}
int script_char_guild_message(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);
	tstring Content(get_tool()->ansi_to_unicode(lua_tostring(L, 2)));

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(pRole->GetGuildID() == INVALID_VALUE)
		return 0;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return 0;

	DWORD dw_size = sizeof(NET_SIS_role_char) + Content.size()*sizeof(TCHAR);

	CREATE_MSG(pSend, dw_size, NET_SIS_role_char);

	pSend->dw_error_code = E_Success;
	pSend->dwSrcRoleID = INVALID_VALUE;
	pSend->dwDestRoleID = INVALID_VALUE;
	pSend->byAutoReply = 0;
	pSend->byChannel = 3;
	_tcsncpy(pSend->szMsg, Content.c_str(), Content.size());
	pSend->szMsg[Content.size()] = _T('\0');
	
	pGuild->send_guild_message(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return 0;
}

int script_get_guild_is_join_Recruit(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	BOOL b_have = 0;
	if(g_guild_manager.is_have_recruit(dw_role_id))
	{
		b_have = 1;
	}

	lua_pushinteger(L, b_have);

	return 1;
}


int script_is_item_bind(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT64 n64_Item_Serial = pop_64bit_data(L, 2, 3);


	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(n64_Item_Serial);
	if(!VALID_POINT(pItem))
		return 0;

	lua_pushboolean(L, pItem->IsBind());

	return 1;
}
int script_add_pet_healthy(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT nHP = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	PetSoul* pPetSoul = pRole->GetPetPocket()->GetCalledPetSoul();

	if (!VALID_POINT(pPetSoul))
		return 0;

	pPetSoul->GetPetAtt().ModAttVal(epa_spirit, nHP);

	return 0;
}

int script_get_role_amends_flag(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	s_role_info* p_info = g_roleMgr.get_role_info(dwRoleID);
	if(!VALID_POINT(p_info)) return 0;
	
	lua_pushnumber(L, p_info->dw_amends_flag);
	return 1;
}

int script_set_role_amends_flag(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwFlag = lua_tonumber(L, 2);
	s_role_info* p_info = g_roleMgr.get_role_info(dwRoleID);
	if(VALID_POINT(p_info))  p_info->dw_amends_flag = dwFlag;
	return 0;
}

int script_get_is_receive_account_reward(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	PlayerSession* pSession = pRole->GetSession();
	if(!VALID_POINT(pSession))
		return 0;

	lua_pushinteger(L, pSession->GetReceive());

	return 1;

}

int script_get_receive_account_type(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	PlayerSession* pSession = pRole->GetSession();
	if(!VALID_POINT(pSession))
		return 0;

	lua_pushinteger(L, pSession->GetReceiveType());

	return 1;
}

int script_get_receive_account_type_ex(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	PlayerSession* pSession = pRole->GetSession();
	if(!VALID_POINT(pSession))
		return 0;

	lua_pushnumber(L, pSession->GetReceiveTypeEx());

	return 1;
}

int script_add_title(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwTitle = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	pRole->SetTitle(dwTitle);

	return 1;
}

int script_has_title(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	DWORD dwTitle = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole))
		return 0;

	lua_pushboolean(L, pRole->HasTitle(dwTitle));

	return 1;
}

int script_is_formal(lua_State* L)
{
	DWORD	dw_guild_id = lua_tonumber(L, 1);

	guild* pGuild = g_guild_manager.get_guild(dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return 0;
	}

	lua_pushboolean(L, pGuild->get_guild_att().bFormal);

	return 1;
}



// 添加噬魂
int script_add_shihun(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD dw_shihun = lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);

	if (!VALID_POINT(pRole))
		return 0;

	pRole->add_shihun(dw_shihun);

	return 0;
}

// 
static int script_get_vip_checkin_num(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);

	if (!VALID_POINT(pRole)){
		lua_pushinteger(L, 0);
	} else {
		INT ndelta = GET_VIP_EXTVAL(pRole->GetTotalRecharge(), GUILD_CHECKIN, INT);
		INT nCur = pRole->GetDayClearData(ERDCT_Tog_mounts);
		if(nCur>=ndelta) lua_pushinteger(L,0);
		else lua_pushinteger(L, ndelta - nCur);
	}

	return 1;
}

static int script_add_vip_checkin_num(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);

	if (VALID_POINT(pRole))
		 pRole->ModRoleDayClearDate(ERDCT_Tog_mounts);

	return 0;
}
static int script_set_vip_level(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);
	DWORD   dwlevel			=	lua_tonumber(L, 4);
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;
	
	if (dwlevel < 0)
		return 0;
	//全服公告脚本去写
	pRole->SetVIPLevel(dwlevel);
	//设置VIP截止日期
	pRole->SetVIPDeatLine(dwlevel);
	return 0;
}
static int script_get_vip_level(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);

	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = pMap->find_role(dwRole);
	if ( !VALID_POINT(pRole) ) return 0;

	lua_pushinteger(L, pRole->GetVIPLevel());
	return 1;
}

int script_update_achievement_criteria(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD dwType = lua_tonumber(L, 2);
	DWORD dwParam1 = lua_tonumber(L, 3);
	DWORD dwParam2 = lua_tonumber(L, 4);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(pRole))
		return 0;

	pRole->GetAchievementMgr().UpdateAchievementCriteria((e_achievement_event)dwType, dwParam1, dwParam2);
	return 1;

}

int script_is_in_duel(lua_State *L){
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if (!VALID_POINT(pRole)){
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, pRole->IsDuel());
	}

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家周围的怪物
//-------------------------------------------------------------------------------------
int script_get_role_around_creature(lua_State* L)
{
	DWORD	dwRoleID		=	lua_tonumber(L, 1);
	FLOAT	fOPRadius		=   lua_tonumber(L, 2);
	FLOAT	fHigh			=	lua_tonumber(L, 3);

	Role* pRole= g_roleMgr.get_role(dwRoleID);

	if( !VALID_POINT(pRole) )
	{
		return 0;
	}

	std::vector<DWORD> vecCreature;
	INT nCreatureNum = 0;
	INT Index = 1;
	nCreatureNum = pRole->GetAroundCreature(vecCreature, fOPRadius, fHigh);

	lua_createtable(L,nCreatureNum,0);

	std::vector<DWORD>::iterator it = vecCreature.begin();
	while(it != vecCreature.end())
	{
		lua_pushnumber(L, Index);
		lua_pushnumber(L, *it);
		lua_settable(L,-3);  
		++it;
		++Index;
	}

	return 1;
}

//-------------------------------------------------------------------------------------
// 获得玩家周围的玩家
//-------------------------------------------------------------------------------------
int script_get_role_around_role(lua_State* L)
{
	DWORD	dwRoleID			=	lua_tonumber(L, 1);
	FLOAT	fOPRadius		=   lua_tonumber(L, 2);
	FLOAT	fHigh			=	lua_tonumber(L, 3);


	Role* pRole= g_roleMgr.get_role(dwRoleID);

	if( !VALID_POINT(pRole) )
	{
		return 0;
	}

	std::vector<DWORD> vecRole;
	INT nRoleNum = 0;
	INT Index = 1;
	nRoleNum = pRole->GetAroundRole(vecRole, fOPRadius, fHigh);

	lua_createtable(L,nRoleNum,0);

	std::vector<DWORD>::iterator it = vecRole.begin();
	while(it != vecRole.end())
	{
		lua_pushnumber(L, Index);
		lua_pushnumber(L, *it);
		lua_settable(L,-3);  
		++it;
		++Index;
	}

	return 1;
}

// 判断进度中是否有此怪物
int script_inst_save_creature_is_dead(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD dw_map_id = lua_tonumber(L, 2);
	DWORD dw_instance_id = lua_tonumber(L, 3);
	EInstanceHardMode e_mode = (EInstanceHardMode)lua_tointeger(L, 4);
	DWORD dw_spawnpt_id = lua_tonumber(L, 5);

	Map* pMap = g_mapCreator.get_map(dw_map_id, dw_instance_id);
	if(!VALID_POINT(pMap))
		return 0;

	if(pMap->get_map_type() != EMT_Instance)
		return 0;

	map_instance_normal* pMapInst = (map_instance_normal*)pMap;

	LIST_CREATURE_PRO::list_iter iter = pMapInst->get_list_creature_pro().begin();
	DWORD dw_creature_id = INVALID_VALUE;
	while(pMapInst->get_list_creature_pro().find_next(iter, dw_creature_id))
	{
		if(dw_creature_id == dw_spawnpt_id)
		{
			lua_pushboolean(L, TRUE);
			return 1;
		}
	}

	lua_pushboolean(L, FALSE);
	return 1;
}

int script_send_hang_left_time(lua_State *L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if ( VALID_POINT(pRole)) pRole->SendDacingLeftTime( );
	return 0;
}


int script_set_hang_left_time(lua_State *L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD seconds	 = lua_tonumber(L, 2);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(VALID_POINT(pRole)){
		pRole->SetPerDayHangGetExpTime( seconds * 1000 );
	}
	return 0;
}

int script_add_hang_left_time(lua_State *L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD seconds	 = lua_tonumber(L, 2);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(VALID_POINT(pRole)){
		pRole->AddPerDayHangGetExpTime( seconds * 1000 );
	}
	return 0;
}

int script_get_hang_left_time(lua_State *L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	lua_pushinteger(L, (VALID_POINT(pRole)) ? (pRole->GetPerDayHangGetExpTime( )/1000) : 0 );
	return 1;
}

int script_show_consume_reward_frame(lua_State *L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	DWORD dwParam0 = lua_tonumber(L, 2);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( VALID_POINT(pRole) )
	{
		NET_SIS_show_consume_reward_frame send;
		send.reserved = dwParam0;
		pRole->SendMessage(&send, send.dw_size);
	}
	return 0;
}

int script_get_account(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;
	
	PlayerSession* pSession = pRole->GetSession();
	if ( !VALID_POINT(pRole)) return 0;
		
	lua_pushlstring(L, pSession->GetAccount(), X_SHORT_NAME);

	return 1;
}

int script_qianghua_equip(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT nIndex	= lua_tointeger(L, 2);
	INT nLevel = lua_tointeger(L, 3);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;
	
	pRole->GMConsolidateEquip(nIndex, nLevel);

	return 0;
}

// 获取当前宠物属性
int script_get_pet_quality(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	DWORD dwPetID	=		lua_tonumber(L, 2);
	DWORD dwIndex	=		lua_tonumber(L, 3);

	if (dwIndex < epa_begin || dwIndex > epa_happy_value)
		return 0;

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pPetSoul = pPetPock->GetPetSoul(dwPetID);
	if (!VALID_POINT(pPetSoul))
		return 0;

	INT nQuality = pPetSoul->GetPetAtt().GetAttVal(dwIndex);

	lua_pushnumber(L, nQuality);

	return 1;
}

int script_get_pet_level(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	DWORD dwPetID	=		lua_tonumber(L, 2);


	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	PetPocket* pPetPock = pRole->GetPetPocket();
	if (!VALID_POINT(pPetPock))
		return 0;

	PetSoul* pPetSoul = pPetPock->GetPetSoul(dwPetID);
	if (!VALID_POINT(pPetSoul))
		return 0;

	INT nLevel = pPetSoul->GetPetAtt().GetVLevel();

	lua_pushnumber(L, nLevel);

	return 1;
}


int script_get_ronghe_pet(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);

	Role* pRole = g_roleMgr.get_role(dwRoleID);

	if (!VALID_POINT(pRole))
		return 0;

	DWORD dwPetID = INVALID_VALUE;
	Role* pTargetMoster = pRole->GetTargetPet(dwPetID);
	if (VALID_POINT(pTargetMoster))
	{
		lua_pushnumber(L, pTargetMoster->GetID());
		lua_pushnumber(L, dwPetID);
		return 2;
	}

	return 0;

}

int script_get_xili_number(lua_State* L)
{
	DWORD dwRoleID	=		lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;

	INT xiliNumber = EQUIP_XILI_FREE - pRole->GetDayClearData(ERDCT_WANMEI_REBORN) +
		pRole->GetDayClearData(ERDCT_Xili_LIMIT_SOMETHING);

	lua_pushnumber(L, xiliNumber);

	return 1;
}

int script_add_xili_number(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);
	INT nXiliDelta	= lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;

	pRole->ModRoleDayClearDate(ERDCT_Xili_LIMIT_SOMETHING, (BYTE)nXiliDelta);

	return 0;
}

int script_get_god_level(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);


	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;

	lua_pushnumber(L, pRole->getGodLevel());

	return 1;
}

int script_set_god_level(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);
	INT nLevel	= lua_tonumber(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;

	//pRole->setGodLevel(nLevel);

	NET_SIS_god_level_up send;
	send.nLevel = pRole->getGodLevel();
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
int script_get_wine_use_number(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;

	INT useNumber = pRole->GetDayClearData(ERDCT_ItemWine_UseNumber);

	lua_pushnumber(L, useNumber);

	return 1;
}

int script_add_reward_item(lua_State* L)
{
	DWORD	dwMapID			=	lua_tonumber(L, 1);
	DWORD	dwInstanceID	=	lua_tonumber(L, 2);
	DWORD	dwRole			=	lua_tonumber(L, 3);
	DWORD	dwItemID		=	lua_tonumber(L, 4);
	DWORD	dwNumber		=	lua_tonumber(L, 5);
	int		nType			=	lua_tointeger(L, 6);

	// 找到地图和怪物
	Map* pMap = g_mapCreator.get_map(dwMapID, dwInstanceID);
	if( !VALID_POINT(pMap) ) return 0;

	Role* pRole = g_roleMgr.get_role(dwRole);
	if (!VALID_POINT(pRole)) return 0;
	
	if (pRole->addRewardItem(dwItemID, dwNumber, (E_REWARDFROM)nType))
	{
		lua_pushboolean(L, TRUE);
		return 1;
	}
	
	return 0;

}


int script_set_instance_data(lua_State* L)
{
	DWORD dw_role_id = lua_tonumber(L, 1);
	INT nIndex = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return 0;

	if(nIndex < 0 || nIndex > MAX_ACTIVE_DATA)
		return 0;

	pRole->setInstancePass(nIndex);
	

	return 1;
}


int script_set_wine_use_number(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;
	pRole->ModRoleDayClearDate(ERDCT_ItemWine_UseNumber,1,FALSE);
	return 0;
}
int script_get_officialsalary_number(lua_State* L)
{
	DWORD dwRoleID	= lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole)) return 0;
	INT useNumber = pRole->GetDayClearData(ERDCT_Officialsalary_GetTimes);
	if (0 == useNumber)
	{
		pRole->ModRoleDayClearDate(ERDCT_Officialsalary_GetTimes,1,FALSE);
	}

	lua_pushnumber(L, useNumber);

	return 1;
}

int script_set_score(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	INT nScore = lua_tointeger(L, 2);

	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole))
		return 0;

	if (VALID_POINT(pRole->GetSession()))
	{
		
		pRole->GetSession()->SetExchange(nScore);

		NET_DB2C_exchange_volume_update send;
		send.dw_account_id = pRole->GetSession()->GetSessionID();
		send.n_volume = nScore;
		g_dbSession.Send(&send, send.dw_size);

	}
	
	return 0;
}

int script_get_score(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole))
		return 0;

	if (VALID_POINT(pRole->GetSession()))
	{
		lua_pushnumber(L, pRole->GetSession()->GetScore());
	}

	return 1;
}
//获取玩家的配偶ID
int script_get_role_spouseid(lua_State* L)
{
	DWORD dwRoleID = lua_tonumber(L, 1);
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if (!VALID_POINT(pRole))
		return 0;

	if (VALID_POINT(pRole->GetSpouseID()))
	{
		lua_pushnumber(L, pRole->GetSpouseID());
	}
	else
	{
		lua_pushnumber(L,0);
	}

	return 1;
}
int script_set_femalerole_redzui(lua_State* L)
{
	DWORD	dw_role_id		=	lua_tonumber(L, 1);

	// 找到玩家
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if( !VALID_POINT(pRole) ) return 0;

	pRole->update_scriptdata2client(10, 1);
	//还需要向客户端发消息

	return 0;
}
#pragma warning(pop)
