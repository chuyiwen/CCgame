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
*	@file		guild.h
*	@author		lc
*	@date		2011/02/23	initial
*	@version	0.0.1.0
*	@brief		
*/

#ifndef GUILD
#define GUILD

#include "state_mgr.h"
#include "guild_position.h"
#include "guild_member.h"
#include "guild_warehouse.h"
#include "guild_upgrade.h"
#include "guild_affair.h"
#include "guild_skill.h"
#include "guild_commerce.h"
#include "guild_war.h"
#include "guild_delate.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "../../common/WorldDefine/guild_protocol.h"

class Role;
struct s_invite;
struct s_guild_load;
class map_instance_guild;
//-------------------------------------------------------------------------------
class guild
{
public:
	guild();
	guild(const s_guild_load *p_guild_load_);
	~guild();

	VOID	init_db_guild_member(const tagGuildMember& st_guild_member_);
	VOID	init_create(Role* p_creator_, const tstring& str_guild_name_, \
						DWORD dw_guild_id_, const tagGuildCfg &st_guild_config_);
	
	VOID	initPlanData(tagPlantData* pPlantData, int nSize);
	VOID	setPlantData(const tagPlantData& data, int nIndex);
	VOID	setPlantData(DWORD dwPlantID, DWORD dwRoleID, int nNum, int nIndex);
	VOID	resetPlantData(int nIndex);
	tagPlantData* getPlantData(int nIndex);
	VOID	update();

	BOOL	is_guild_init_ok();

public:
	DWORD	dismiss_guild(Role* p_role_);
	VOID	dismiss_guild();

	DWORD	can_invite_join(DWORD dw_inviter_id_, DWORD dw_invitee_id_, Role **pp_invitee_, BOOL b_insert_ = TRUE);
	DWORD	invite_join(DWORD dw_inviter_id_, DWORD dw_invitee_id_);
	BOOL	erase_invite_join(DWORD dw_invitee_id_);
	DWORD	appoint(DWORD dw_appointor_id_, DWORD dw_appointee_id_, EGuildMemberPos e_guild_position_);
	DWORD	kick_member(DWORD dw_role_id_, DWORD dw_kick_id_);
	DWORD	change_leader(DWORD dw_old_leader_id_, DWORD dw_new_leader_id_);
	DWORD	modify_tenet(Role* pRole, LPCTSTR str_tenet_, INT32 n_string_length_);
	DWORD	modify_symbol(DWORD dw_role_id_, LPCTSTR str_tenet_, INT32 n_string_length_);
	DWORD	modify_group_exponent(BOOL b_increase_);
	DWORD	demiss_postion(DWORD dw_role_id_, OUT INT8 &n_old_guild_pos_);
	DWORD	leave_guild(Role* p_role_);
	DWORD   modify_postion_name(DWORD dw_role_id_, TCHAR * sz_position_name_);
	DWORD   modify_postion_power(DWORD dw_role_id_, DWORD * p_position_power_);
	DWORD   get_enemy_data(Role* p_role_);
	DWORD	declare_war(Role* p_role_, DWORD dw_enemy_guild_id_);
	DWORD	declare_war_gm(Role* p_role_, DWORD dw_enemy_guild_id_);
	DWORD	declare_war_res(Role* p_role_, DWORD dw_enemy_guild_id_, BOOL b_accept_);
	DWORD	guild_qualify_war(Role* p_role, BOOL bJoin);
	DWORD	guild_qualify_war_dis(Role* p_role, DWORD dwMemberID);
	DWORD	materialReceive(Role* pRole, DWORD dwID, INT nNumber);
	DWORD	donateFund(Role* pRole, BYTE byType);
	DWORD	useMianzhan(Role* pRole, INT64 n64ItemID);
	DWORD	ModifyApplyLevel(Role* pRole, INT32 nLevel);
	DWORD	SignUpAttact(Role* pRole);

	DWORD	set_guild_dkp(Role* p_role_, INT16* p_n16_dkp_);
	DWORD	set_dkp_affirmance(Role* p_role_, INT16 n16_dkp_);

	DWORD	invite_league(Role* p_role_, DWORD dw_league_id_);
	DWORD	invite_league_res(Role* p_role_, DWORD dw_invite_guild_id_, BOOL b_agree_);
	DWORD   relieve_league(Role* p_role_, DWORD dw_guild_id_);

	DWORD	invite_sign(Role* p_invite_role_, Role* p_beinvite_role_, INT64 n64_item_id_);
	DWORD   affirmance_sign(Role* p_affirm_role_, Role* p_leader_role_);
	DWORD   refer_sign(Role* p_role_, DWORD dw_npc_id_, INT64 n64_item_id_);
	
	DWORD	handleLevelUp(Role* p_role);

	VOID	change_guild_to_formal();

	DWORD	set_enemy_guild(Role* p_role_, DWORD dw_enemy_guild_);
	DWORD	delete_enemy_guild(Role* p_role_, DWORD dw_enemy_guild_);

	VOID	setSignUpAttack(BOOL bSet, BOOL bSave = false);
	//! 通知客户端数据
	VOID	send_all_members_to_client(Role *p_role_);
	DWORD	send_special_memberex_to_client(Role *p_role_, DWORD dw_special_role_id_);
	DWORD	send_special_member_to_client(Role *p_role_, DWORD dw_special_role_id_);
	DWORD	send_guild_name_to_client(PlayerSession *p_session_);
	DWORD	send_guild_tenet_to_client(PlayerSession *p_session_);
	DWORD	send_guild_symbol_to_client(PlayerSession* p_session_);
	DWORD	send_facilities_info_to_client(Role* p_role_, EFacilitiesType e_facilities_type_);
	
	VOID	synchronize_guild_war_info();
	//! 角色删除时清理帮会数据
	DWORD	clear_role_remove(DWORD dw_role_id_);

	// 鼎炉修炼
	BOOL	IsLobbyPracitec(Role* pRole);
	FLOAT	LobbyPractice(Role* pRole);
	FLOAT	LevelToMultiple(INT byLevel);
	
	INT		get_member_full();

	BOOL	isMianzhan();

	DWORD	getGuildSBKReward(Role* pRole);
public:
	//! 获取帮会数据
	const s_guild&	get_guild_att() const;
	GuildWarehouse& get_guild_warehouse()		{ return _guild_warehouse; }
	GuildUpgrade&	get_guild_facilities()	{ return _guild_upgrade; }
	guild_affair&	get_guild_affair()		{ return _guild_affair; }
	guild_skill&		get_guild_skill()			{ return _guild_skill; }
	guild_commerce&	get_guild_commerce()		{ return _guild_commerce; }
	guild_war&		get_guild_war()			{ return _guild_war;}
	GuildUpgrade&   get_upgrade()			{ return _guild_upgrade; }
	guild_delate&    get_delate()				{ return _guild_delate; }
	
	map_instance_guild* get_guild_map();
	BOOL	is_occupy_city(BYTE by_city_index_);
	//! 帮会内广播
	VOID	send_guild_message(LPVOID p_message_, DWORD dw_size_);
	
	template<typename Type>
	VOID for_every_role_in_guild( Type oper );
public:
	// ! 设置帮会仓库使用权限
	DWORD	set_guild_ware_use_privilege(Role* p_role_, DWORD dw_role_id_, BOOL b_can_use_);

public:
	// ! 帮会升级处理
	VOID	reinit_guild_upgrade(BYTE by_new_level_ = 0);

public:
	//! 获取帮会状态
	DWORD get_guild_state() const { return _guild_state.GetState(); }

	BOOL is_in_guild_state(EGuildSpecState e_state_) const
	{
		return _guild_state.IsInState(e_state_);
	}

	BOOL is_in_guild_state_all(DWORD dw_state_) const
	{
		return _guild_state.IsInStateAll(dw_state_);
	}

	BOOL is_in_guild_state_any(DWORD dw_state_) const
	{
		return _guild_state.IsInStateAny(dw_state_);
	}

	VOID set_guild_state(EGuildSpecState e_state_, BOOL b_send_message_=TRUE)
	{
		if( TRUE == is_in_guild_state(e_state_) ) return;

		_guild_state.SetState(e_state_);
		guild_att.dwSpecState = _guild_state.GetState();
		save_guild_att_to_db();

		if( b_send_message_ )
		{
			NET_SIS_guild_set_state send;
			send.eState = e_state_;
			send_guild_message(&send, send.dw_size);
		}
	}

	VOID unset_guild_state(EGuildSpecState e_state_, BOOL b_send_message_=TRUE)
	{
		if( FALSE == is_in_guild_state(e_state_) ) return;

		_guild_state.UnsetState(e_state_);
		guild_att.dwSpecState = _guild_state.GetState();
		save_guild_att_to_db();

		if( b_send_message_ )
		{
			NET_SIS_guild_unset_state send;
			send.eState = e_state_;
			send_guild_message(&send, send.dw_size);
		}
	}

	INT	get_enemy_num()
	{
		INT n_num = 0;
		for(INT i = 0; i < MAX_ENEMY_NUM; i++)
		{
			if(guild_att.dwEnemyID[i] != INVALID_VALUE)
				n_num++;
		}
		return n_num;
	}

	VOID set_enemy_num(DWORD dw_guild_id_)
	{
		for(INT i = 0; i < MAX_ENEMY_NUM; i++)
		{
			if(guild_att.dwEnemyID[i] == INVALID_VALUE)
			{
				guild_att.dwEnemyID[i] = dw_guild_id_;
				return;
			}
		}
	}

	VOID delete_enemy_num(DWORD dw_guild_id_)
	{
		for(INT i = 0; i < MAX_ENEMY_NUM; i++)
		{
			if(guild_att.dwEnemyID[i] == dw_guild_id_)
			{
				guild_att.dwEnemyID[i] = INVALID_VALUE;
				return;
			}
		}
	}

	BOOL is_enemy(DWORD dw_guild_id_)
	{
		for(INT i = 0; i < MAX_ENEMY_NUM; i++)
		{
			if(guild_att.dwEnemyID[i] != INVALID_VALUE)
			{
				if(guild_att.dwEnemyID[i] == dw_guild_id_)
					return TRUE;
			}
		}
		return FALSE;
	}

	VOID set_league_id(DWORD dw_league_id_)
	{
		guild_att.dwLeagueID = dw_league_id_;
		save_guild_att_to_db();
	}

	VOID set_unleague_time()
	{
		guild_att.dwUnLeagueBeginTime = g_world.GetWorldTime();
		save_guild_att_to_db();
	}

	VOID set_delete(BOOL b_delete_) { b_delete = b_delete_; }

	BOOL get_delete()	{ return b_delete; }

public:
	//! 帮会数值操作
	BOOL	increase_guild_fund(DWORD dw_role_id_, INT32 n_fund_, DWORD dw_commond_id_);
	BOOL	increase_guild_material(DWORD dw_role_id_, INT32 n_material_, DWORD dw_command_id_);
	BOOL	increase_guild_reputation(DWORD dw_role_id_, INT32 n_reputation_, DWORD dw_command_id_);
	BOOL	increase_guild_peace(DWORD dw_role_id_, INT16 n_peace_, DWORD dw_command_id_);
	BOOL	increase_prosperity(DWORD dw_role_id_, INT32 n_prosperity_, DWORD dw_command_id_);

	BOOL	decrease_guild_fund(DWORD dw_role_id_, INT32 n_fund_, DWORD dw_command_id_);
	BOOL	decrease_guild_material(DWORD dw_role_id_, INT32 n_material_, DWORD dw_command_id_);
	BOOL	decrease_guild_reputation(DWORD dw_role_id_, INT32 n_reputation_, DWORD dw_command_id_);
	BOOL	decrease_guild_peace(DWORD dw_role_id_, INT16 n_peace_, DWORD dw_command_id_);
	BOOL    decrease_prosperity(DWORD dw_role_id_, INT32 n_prosperity_, DWORD dw_command_id_);

	VOID	set_script_data(INT nIndex, INT n_data);

	VOID	re_calculate_affair_remain_num(BYTE by_times);

	//! 帮会数值记录LOG
	VOID log_guild_fund(DWORD dw_role_id_, INT32 n_fund_, INT32 n_total_fund_, DWORD dw_command_id_);
	VOID log_guild_material(DWORD dw_role_id_, INT32 n_material_, INT32 n_total_material_, DWORD dw_command_id_);
	VOID log_guild_reputation(DWORD dw_role_id_, INT32 n_reputation_, INT32 n_total_reputation_, DWORD dw_command_id_);

	//! 每日重置帮会状态
	VOID daily_guild_reset();

	VOID guild_quest_reset( );
	
	VOID	get_plant_data(tagPlantData* pPlant)
	{
		memcpy(pPlant, m_plantData, sizeof(m_plantData));
	}

	// 激活祷告者
	VOID SetDaogao(INT32 nIndex, BOOL bActive);
	//激活或取消特定个数祷告者
	VOID ActiveDaogaoNumber(DWORD dwNum, BOOL bActive);
	// 某个祷告者是否激活
	BOOL IsDaogaoActive(INT32 nIndex);
	// 获取祷告者数量
	DWORD GetDaogaoNumber(BOOL bQiang = FALSE);
public:
	//! 帮会成员相关操作 
	tagGuildMember*	get_member(DWORD dw_role_id_);
	INT32			get_guild_member_num();
	MAP_GUILD_MEMBER&	get_guild_member_map();
	guild_member& get_guild_member(){ return _member_manager; }
	VOID			change_member_contribution(DWORD dw_role_id_, INT32 n_contribution_, BOOL b_increase_, BOOL b_save_db_ = TRUE);
	VOID			change_member_exploit(DWORD dw_role_id_, INT32 n_exploit_, BOOL b_save_db_ = TRUE);
	VOID			change_member_ballot(DWORD dw_role_id_, BOOL b_ballot_, BOOL b_save_db_ = TRUE);
	VOID			change_member_war(DWORD dw_role_id_, BOOL b_war, BOOL b_save_db_ = TRUE);
	VOID			change_member_dkp(DWORD dw_role_id_, INT32 n_dkp_, BOOL b_save_db_ = TRUE);
	VOID			increase_member_total_fund(DWORD dw_role_id_, INT32 nValue, BOOL b_save_db_ = TRUE);	
public:
	
	VOID	change_facility_level(EFacilitiesType e_facilities_type_, BYTE by_value_);

	DWORD	get_guild_instance_id()const	{ return dw_guild_instance_id; }
	VOID	set_guild_instance_id(DWORD dw_guild_instance_id_)		{ dw_guild_instance_id = dw_guild_instance_id_; }
	VOID	set_guild_open_server(BOOL b);
private:
	
	DWORD	add_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_, BOOL b_save_db_ = TRUE);
	DWORD	load_member(const tagGuildMember& st_guild_member_);
	VOID	remove_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_);
	DWORD	change_guild_position(tagGuildMember *p_member_, EGuildMemberPos e_new_guild_position_);
	VOID	set_guild_bangzhu(DWORD dw_new_leader_id_);
	
	//! 保存帮会属性数据到数据库 
	VOID	save_guild_att_to_db();
	VOID	save_guild_plant_to_db();
public:
	
	const tagGuildPower&	get_guild_power(EGuildMemberPos e_guild_position_) const;
	VOID	set_tripod_id(DWORD dw_tripod)	{ dw_guild_tripod_id = dw_tripod; }
	DWORD	get_tripod_id() { return dw_guild_tripod_id; }
	VOID	set_tripod_map_id(DWORD dw_map_id) { dw_tripod_map_id = dw_map_id; }
	DWORD   get_tripod_map_id() { return dw_tripod_map_id; }
	VOID	set_lost_tripod_time(tagDWORDTime time) { dw_lost_tripod_time = time; }
	tagDWORDTime get_lost_tripod_time() { return dw_lost_tripod_time; }
	VOID	send_tripod_message();
	
	VOID	add_war_history(const tagGuildWarHistory& gwh, BOOL bSaveDB = TRUE);

	const std::vector<tagGuildWarHistory>& get_war_history() { return vecWarHistory; }

	VOID	clearEnemyGuild() { listEnemyGuild.clear(); }

	VOID	setRank(int nRank) { m_nRank = nRank; }
	int		getRank() { return m_nRank; }
private:
	typedef State<DWORD, EGuildSpecState>	guild_state;

	static const tagGuildPower		null_guild_power;
	//-------------------------------------------------------------------------------
	//! 帮派基础属性
	//-------------------------------------------------------------------------------
	s_guild				guild_att;

	//-------------------------------------------------------------------------------
	//! 帮会请帖
	//-------------------------------------------------------------------------------
	typedef package_map<DWORD, s_invite*>	MAP_INVITE;	
	MAP_INVITE				map_invite;

	// 帮会战绩
	typedef std::vector<tagGuildWarHistory>	WAR_HISTORY_LIST;
	WAR_HISTORY_LIST		vecWarHistory;
	
	// 种植数据
	tagPlantData				m_plantData[MAX_PLANT_NUMBER];

	//-------------------------------------------------------------------------------
	//! 帮会成员
	//-------------------------------------------------------------------------------
	guild_member				_member_manager;

	//-------------------------------------------------------------------------------
	//! 帮会职位
	//-------------------------------------------------------------------------------
	guild_position				_guild_position;

	//-------------------------------------------------------------------------------
	//! 帮会仓库
	//-------------------------------------------------------------------------------
	GuildWarehouse			_guild_warehouse;

	//-------------------------------------------------------------------------------
	//! 帮会状态
	//-------------------------------------------------------------------------------
	guild_state				_guild_state;

	//-------------------------------------------------------------------------------
	//! 帮会设施
	//-------------------------------------------------------------------------------
	GuildUpgrade			_guild_upgrade;

	//-------------------------------------------------------------------------------
	//! 帮会事务
	//-------------------------------------------------------------------------------
	guild_affair				_guild_affair;

	//-------------------------------------------------------------------------------
	//! 帮会技能
	//-------------------------------------------------------------------------------
	guild_skill				_guild_skill;

	//-------------------------------------------------------------------------------
	//! 帮会跑商
	//-------------------------------------------------------------------------------
	guild_commerce			_guild_commerce;

	//-------------------------------------------------------------------------------
	//! 帮会副本id
	//-------------------------------------------------------------------------------
	DWORD					dw_guild_instance_id;	

	//-------------------------------------------------------------------------------
	//! 帮会战争
	//-------------------------------------------------------------------------------
	guild_war				_guild_war;


	//-------------------------------------------------------------------------------
	//! 帮会弹劾
	//-------------------------------------------------------------------------------
	guild_delate				_guild_delate;

	//! 帮会解散标记
	BOOL					b_delete;	

	// 帮会占领鼎ID
	DWORD					dw_guild_tripod_id;
	DWORD					dw_tripod_map_id;
	
	tagDWORDTime			dw_lost_tripod_time;
	// 已经宣战的帮会
	std::set<DWORD>			listEnemyGuild;

	int						m_nRank; //排行榜排名
};

/***************************************************************************************************/

//-------------------------------------------------------------------------------
//! 获取帮会属性
//-------------------------------------------------------------------------------
inline const s_guild& guild::get_guild_att() const
{
	return guild_att;
}

//-------------------------------------------------------------------------------
//! 帮会广播
//-------------------------------------------------------------------------------
inline VOID guild::send_guild_message(LPVOID p_message_, DWORD dw_size_)
{
	_member_manager.send_guild_message(p_message_, dw_size_);
}

//-------------------------------------------------------------------------------
//！ 添加帮会成员
//-------------------------------------------------------------------------------
inline DWORD guild::add_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_, BOOL b_save_db_)
{
	if(_member_manager.is_exist(dw_role_id_))
	{
		return E_Success;
	}
	
	DWORD dw_error_code = _guild_position.add_member(dw_role_id_, e_guild_position_);
	if(E_Success == dw_error_code)
	{
		_member_manager.add_member(dw_role_id_, e_guild_position_, b_save_db_);
		guild_att.n16MemberNum = get_guild_member_num();
	}
	
	ASSERT(E_Success == dw_error_code);
	return dw_error_code;
}

inline DWORD guild::load_member(const tagGuildMember& st_guild_member_)
{
	if(_member_manager.is_exist(st_guild_member_.dw_role_id))
	{
		return E_Success;
	}

	DWORD dw_error_code = _guild_position.add_member(st_guild_member_.dw_role_id, st_guild_member_.eGuildPos);
	if(E_Success == dw_error_code)
	{
		_member_manager.load_member(st_guild_member_);
		guild_att.n16MemberNum = get_guild_member_num();
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------
//! 删除帮会成员
//-------------------------------------------------------------------------------
inline VOID guild::remove_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_)
{
	_guild_position.remove_member(dw_role_id_, e_guild_position_);
	
	_guild_commerce.abandon_commerce(dw_role_id_, TRUE);

	_member_manager.remove_member(dw_role_id_);
	guild_att.n16MemberNum = get_guild_member_num();
}

//-------------------------------------------------------------------------------
//! 帮会成员职位变化
//-------------------------------------------------------------------------------
inline DWORD guild::change_guild_position(tagGuildMember *p_member_, EGuildMemberPos e_new_guild_position_)
{
	DWORD dw_error_code = _guild_position.change_position(p_member_->dw_role_id, p_member_->eGuildPos, e_new_guild_position_);
	if(E_Success == dw_error_code)
	{
		//！ 帮会职位变化导致其他属性变化
		if (!get_guild_power(e_new_guild_position_).bCommerce)
		{
			_guild_commerce.abandon_commerce(p_member_->dw_role_id);
		}
		
		_member_manager.set_guild_position(p_member_, e_new_guild_position_, TRUE);
		_member_manager.set_guild_ballot(p_member_->dw_role_id, TRUE, TRUE);
	}
	
	ASSERT(E_Success == dw_error_code);

	return dw_error_code;
}

//-------------------------------------------------------------------------------
//! 获取帮会成员属性
//-------------------------------------------------------------------------------
inline tagGuildMember* guild::get_member(DWORD dw_role_id_)
{
	return _member_manager.get_member(dw_role_id_);
}

//-------------------------------------------------------------------------------
// 获取成员属性
//-------------------------------------------------------------------------------
inline INT32 guild::get_guild_member_num()
{
	ASSERT(_member_manager.get_member_num() == _guild_position.get_number());
	return _guild_position.get_number();
}

//-------------------------------------------------------------------------------
// 获取成员属性表
//-------------------------------------------------------------------------------
inline MAP_GUILD_MEMBER& guild::get_guild_member_map()
{
	return _member_manager.get_member_map();
}

//-------------------------------------------------------------------------------
//! 发送所有成员基本信息
//-------------------------------------------------------------------------------
inline VOID guild::send_all_members_to_client(Role *p_role_)
{
	ASSERT(VALID_POINT(p_role_));

	ASSERT(_member_manager.get_member_num() == _guild_position.get_number());

	if(_guild_position.get_number() == 0)
	{
		return;
	}

	_member_manager.send_all_members_to_client(p_role_);
}

//-------------------------------------------------------------------------------
//! 设置帮主
//-------------------------------------------------------------------------------
inline VOID guild::set_guild_bangzhu(DWORD dw_new_leader_id_)
{
	guild_att.dwLeaderRoleID = dw_new_leader_id_;

	save_guild_att_to_db();
}

inline VOID guild::set_guild_open_server(BOOL b)
{
	guild_att.b_hasOpenServerReceive = b;
}

//-------------------------------------------------------------------------------
//! 帮会成员贡献变化
//-------------------------------------------------------------------------------
inline VOID guild::change_member_contribution( DWORD dw_role_id_, INT32 n_contribution_, BOOL b_increase_, BOOL b_save_db_ )
{
	if (b_increase_)
	{
		_member_manager.increase_member_contribution(dw_role_id_, n_contribution_, b_save_db_);
	}
	else
	{
		_member_manager.decrease_member_contribution(dw_role_id_, n_contribution_, b_save_db_);
	}
}

inline VOID guild::increase_member_total_fund(DWORD dw_role_id_, INT32 nValue, BOOL b_save_db_)
{
	_member_manager.increase_member_total_fund(dw_role_id_, nValue, b_save_db_);
}

inline VOID guild::change_member_exploit(DWORD dw_role_id_, INT32 n_exploit_, BOOL b_save_db_)
{
	_member_manager.set_member_exploit(dw_role_id_, n_exploit_, b_save_db_);
}

inline VOID	guild::change_member_ballot(DWORD dw_role_id_, BOOL b_ballot_, BOOL b_save_db_)
{
	_member_manager.set_guild_ballot(dw_role_id_, b_ballot_, b_save_db_);
}

inline VOID	guild::change_member_war(DWORD dw_role_id_, BOOL b_war, BOOL b_save_db_)
{
	_member_manager.set_guild_war(dw_role_id_, b_war, b_save_db_);
}

inline VOID	guild::change_member_dkp(DWORD dw_role_id_, INT32 n_dkp_, BOOL b_save_db_)
{
	_member_manager.set_member_dkp(dw_role_id_, n_dkp_, b_save_db_);
}

inline VOID guild::change_facility_level(EFacilitiesType e_facilities_type_, BYTE by_value_)
{
	_guild_upgrade.ChangeFacilitiesLevel(e_facilities_type_, by_value_);
}

template<typename Type>
VOID guild::for_every_role_in_guild( Type oper )
{
	MAP_GUILD_MEMBER& map = get_guild_member_map();

	tagGuildMember* pMember = NULL;
	MAP_GUILD_MEMBER::map_iter iter = map.begin();
	while(map.find_next(iter, pMember))
	{
		Role *pRole = g_roleMgr.get_role(pMember->dw_role_id);
		if(VALID_POINT(pRole))
		{
			oper(pRole);
		}
	}
}

#endif