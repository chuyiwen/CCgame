

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
*	@file		map_instance_guild.h
*	@author		mmz
*	@date		2010/10/12	initial
*	@version	0.0.1.0
*	@brief		帮会副本
*/


#pragma once

const INT ROLE_LEAVE_GUILD_INSTANCE_TICK_COUNT_DOWN	= 60 * TICK_PER_SECOND;		// 角色在副本内离开家族时，传送出副本的倒计时，60秒

#include "map.h"

class map_instance;
class guild;
class Team;

//------------------------------------------------------------------------------
// 家族副本类
//------------------------------------------------------------------------------
class map_instance_guild : public map_instance
{
public:
	map_instance_guild();
	virtual ~map_instance_guild();

	virtual BOOL		init(const tag_map_info* p_info_, DWORD dw_instance_id_, Role* p_creator_=NULL, DWORD dw_misc_=INVALID_VALUE);
	virtual VOID		update();
	virtual VOID		destroy();

	virtual VOID		add_role(Role* p_role_,  Map* pSrcMap = NULL);
	virtual	VOID		role_leave_map(DWORD dw_id_, BOOL b_leave_online = FALSE);
	virtual INT			can_enter(Role *p_role_, DWORD dw_param_ = 0);
	virtual BOOL		can_destroy();
	virtual VOID		on_destroy();
	virtual BOOL		changePkValue()	{ return FALSE; }
	virtual DWORD		get_param() { return m_dw_guild_id;}
	virtual VOID		on_creature_die(Creature* p_creature_, Unit* p_killer_);
	virtual	VOID		on_role_die(Role* p_role_, Unit* p_killer_);
	virtual VOID		on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
		INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_);
	virtual VOID 		spawn_creature_replace(Creature* p_dead_creature_);
	VOID	set_guild_id(DWORD dw_guild_id_)	{ m_dw_guild_id = dw_guild_id_; }
	DWORD	get_guild_id()const { return m_dw_guild_id; }
	

	DWORD				get_creator_id()			{ return m_dw_creator_id; }	
	DWORD				cal_time_limit();
	const tagInstance*	get_instance_proto()		{ return m_p_instance; }

	VOID				on_guild_dismiss(const guild* p_guild_);

	VOID				on_guild_level_up(const guild* p_guild_);

	VOID				on_guild_prepare_war(const guild* p_guild_);
	
	VOID				on_guild_relay_war(const guild* p_guild_);
	
	VOID				on_guild_start_war(const guild* p_guild_);

	VOID				on_guild_end_war(const guild* p_guild_);

	VOID				on_guild_decline_level(const guild* p_guild);

	VOID				on_role_leave_guild(DWORD dw_role_id_, const guild* p_guild_);
	VOID				on_role_enter_guild(DWORD dw_role_id_, const guild* p_guild_);

	VOID				init_guild_build(guild* p_guild_, BYTE type_, BOOL b_war_ = FALSE);
	//INT					init_guild_plant(guild* p_guild, TCHAR* szPointName, DWORD dwCreatureID, INT nCurIndex);
	VOID				init_guild_daogao(guild* p_guild);

	DWORD				get_boss_id() { return m_dw_boss_id; }

	VOID				set_boss_id(DWORD dw_boss_id_)	{ m_dw_boss_id = dw_boss_id_; }

	VOID				set_lobby_id(DWORD dw_lobby_id) { m_dw_lobby_id = dw_lobby_id; }

	DWORD				get_lobby_id()	{ return m_dw_lobby_id; }
	
	VOID				set_in_war(BOOL bIn) { m_bInWar = bIn; }

	VOID				set_guild(guild* pGuild, guild* pAckGuild) { m_pGuild = pGuild; m_pGuildAck = pAckGuild; }
	
	VOID				send_war_number_to_guild();

	bool				isEndWar() { return m_bEnd; }
protected:
	virtual VOID		on_delete();
	virtual BOOL		init_all_spawn_point_creature(DWORD dw_guild_id_ = INVALID_VALUE);

private:
	VOID				update_time_issue();
	BOOL				is_time_limit()		{ return m_p_instance->dwTimeLimit > 0 && m_p_instance->dwTargetLimit != INVALID_VALUE; }

	VOID				init_guild_build(guild* p_guild_, BOOL b_war_ = FALSE);

	VOID				init_guild_relive_pos(guild* p_guild_, const tagInstance* p_instance_);

	VOID				init_guild_enlistee(guild* p_guild_, BOOL b_war_ = FALSE);
	

private:
	DWORD					m_dw_guild_id;					// 创建副本家族ID

	BOOL					m_b_no_enter;						// 副本是否还没人进入过
	DWORD					m_dw_creator_id;					// 副本创建者ID
	DWORD					m_dw_start_tick;					// 开始时间
	DWORD					m_dw_end_tick;					// 副本开始重置倒计时	

	DWORD					m_dw_boss_id;						// 帮会BossID
	DWORD					m_dw_lobby_id;					// 帮会鼎id

	package_map<DWORD, INT>		m_map_will_out_role_id;				// 不再属于这个副本等待传输出去的玩家列表

	const tagInstance*		m_p_instance;					// 副本静态属性

	BOOL					m_bInWar;						// 是否在帮会战状态

	guild*					m_pGuild;						// 本帮会(防守方)
	guild*					m_pGuildAck;					// 进攻方帮会

	BOOL					m_bEnd;							// 帮会战结束
};