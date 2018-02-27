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
 *	@file		team
 *	@author		mwh
 *	@date		2011/03/21	initial
 *	@version	0.0.1.0
 *	@brief		队伍
*/

#ifndef __TEAM__
#define __TEAM__

#include "../../common/WorldDefine/group_define.h"

#include <list>
#include "mutex.h"
#include "role.h"
#include "role_mgr.h"

class Creature;

class Team
{
	friend class GroupMgr;

	typedef package_map<DWORD, tagApplyRoleData*> MAP_APPLY;
	typedef package_map<DWORD,tagTeamMemberData*> MAP_MEMBER;

public:
	// ctor / dtor
	Team(DWORD dw_team, Role* p_leader, Role* p_first_member);
	Team(DWORD dw_team, Role* p_leader);
	~Team();


	// getter
	DWORD		get_team_id() const							{ return dw_team_id_; }
	DWORD		get_group_id() const						{ return dw_group_id_; }
	INT			get_member_number()	const					{ return member_number_; }
	DWORD		get_syn_tick() const						{ return team_syn_tick_; }
	INT			get_max_level() const						{ return max_level_; }
	INT			get_min_level() const						{ return min_level; }
	EPickMode	get_pick_mode() const						{ return e_pick_mode_; }
	FLOAT		get_exp_factor() const						{ return f_exp_factor_; }
	DWORD		get_own_instance_mapid() const				{ return dw_own_map_; }
	DWORD		get_own_instanceid() const					{ return dw_own_instance_; }
	tstring		get_team_placard() const					{ return team_placard_; }
	EItemQuality	get_assign_quality( ) const				{ return e_assign_quality_; }

	Role*		get_pick_role(Creature* p_creature) const;
	INT			get_average_level() const;
	Role*		get_member(INT index) const; 
	DWORD		get_member_id(INT index) const;

	VOID		export_all_member_id(DWORD dw_member[MAX_TEAM_NUM]) const;

	BOOL is_has_friend(Role* p_role);
	BOOL is_need_delete() { return b_need_delete_; }
	BOOL is_leader(DWORD dw_role_id) const	{ return dw_member_id_[0] == dw_role_id; }


	// setter
	VOID			set_max_level(INT level)		{ max_level_ = level; }
	VOID			set_min_level(INT level)		{ min_level = level; }
	VOID			set_syn_tick(DWORD tick)		{ team_syn_tick_ = tick; }
	VOID			set_group_id(DWORD group_id)	{ dw_group_id_ = group_id; }

	VOID			set_own_instanceid(DWORD dw_instance_id)const { dw_own_instance_ = dw_instance_id; }
	VOID			set_own_instance_mapid(DWORD dw_map_id) const	{ dw_own_map_ = dw_map_id; }
	VOID			set_team_placard(tstring str)		{ team_placard_ = str; }
	INT				set_assign_quality(Role* p_role, EItemQuality e_quality );
	VOID			cal_exp_factor();

	INT				add_member(Role* p_inviter, Role* p_replyer);
	INT				kick_member(Role* p_src, Role* p_dest);
	INT				kick_member(Role* p_src, DWORD dw_dest_id);
	INT				leave_team(Role* p_role, BOOL b_leave_line = FALSE);
	INT				set_pick_mode(Role* p_role, EPickMode e_pick_mode);
	INT				change_leader(Role* p_src, Role* p_dest);
	DWORD			change_team_placard(DWORD dw_role_id, LPCTSTR sz_new ,INT32 cnt);
	
	VOID			member_level_change(Role* p_role);
	VOID			leader_rein_change(Role* p_role);

	INT				send_team_msg_in_same_map(DWORD dw_map, LPVOID p_message, DWORD dw_size);

	template<typename UnitOperation>
	VOID			for_each( UnitOperation oper ) const;

	DWORD			add_apply_member(Role* p_apply_role);
	VOID			delete_apply_member(DWORD dw_apply_id);
	VOID			clear_apply_member();
	VOID			delete_member_team_info();

	VOID			add_member_data(Role* p_role);
	VOID			delete_member_data(DWORD dw_role_id);
	VOID			member_online(INT index, Role* p_role);
	BOOL			is_have_leave_role() const;

	VOID			change_member_datas(Role* p_role);

	//mwh 2011-09-06 
	VOID			leader_set_share_circle_quest(BOOL b);
	BOOL			get_leader_share_circle_quest( ) const;
private:
	VOID			resort_online_offline_member();
private:
	// 消息发送
	VOID			send_team_instance_notice(Role* p_role, LPVOID p_message, DWORD dw_size);
	VOID			send_team_message(LPVOID p_message, DWORD dw_size);
	VOID			send_teamate_message(DWORD dw_role_id, LPVOID p_message, DWORD dw_size);
	VOID            send_team_msg_out_big_view(Role* p_role, LPVOID p_message, DWORD dw_size);
	INT             send_team_msg_in_big_view(Role* p_role, LPVOID p_message, DWORD dw_size, DWORD dw_except = 0);
	VOID			send_role_init_state_to_team(Role* p_new_member);
	VOID			send_team_state(Role* p_new_member);
	VOID			send_team_member_info_to_invitee(Role* p_invitee);
	VOID			update_teamate_position();

private:
	INT				is_role_in_team(DWORD dw_member) const;
	VOID			delete_member(const INT index, BOOL b_leave_line);
	VOID			recalc_team_level();

	INT				can_add_member(Role* p_inviter, Role* p_replyer);
	INT				can_kick_member(Role* p_src, Role* p_dest, INT& index);
	INT				can_kick_member(Role* p_src, DWORD dw_dest, INT& index);
	INT				can_leave_team(Role* p_role, INT& index);

private:
	// 事件
	VOID			on_create(BOOL b_own = FALSE);
	VOID			on_delete();
	VOID			on_add_member(Role* p_role);
	VOID			on_delete_member(Role* p_role, BOOL b_leave_line);

private:
	// 队伍ID
	DWORD dw_team_id_;
	// 团队ID
	DWORD dw_group_id_;
	// 队伍人数
	INT member_number_;
	// 队员ID列表
	DWORD dw_member_id_[MAX_TEAM_NUM];
	// 队员指针
	//Role* p_member_[MAX_TEAM_NUM];

	// 成员位置同步(per 25 ticks)
	DWORD team_syn_tick_;

	// 队员最大等级
	INT max_level_;
	// 队员最小等级
	INT min_level;

	// 队伍副本地图ID
	mutable DWORD dw_own_map_;
	// 队伍副本ID
	mutable DWORD dw_own_instance_;


	// 拾取模式
	EPickMode e_pick_mode_;	
	// 杀怪经验因数
	FLOAT f_exp_factor_;	
	// 顺序拾取列表
	mutable list<INT> pick_list_;					
	mutable Mutex lock_;					

	// 是否需要删除
	BOOL b_need_delete_;				

	// 队伍公告
	tstring	team_placard_;				

	// 入队申请列表
	MAP_APPLY team_apply_;	
	// 队员数据
	MAP_MEMBER member_datas_;				

	// 分配品质
	EItemQuality e_assign_quality_;	

	//mwh 2011-09-06 
	BOOL b_leader_share_circle_quest_;
};

//-------------------------------------------------------------------------------------------
// 得到队员指针
//-------------------------------------------------------------------------------------------
inline Role* Team::get_member(INT index) const 
{ 
	ASSERT(index < MAX_TEAM_NUM && index >= 0);	

	Role* p_role = g_roleMgr.get_role(dw_member_id_[index]);
	
	return p_role;
}

//--------------------------------------------------------------------------------------------
// 得到队员ID
//--------------------------------------------------------------------------------------------
inline DWORD Team::get_member_id(INT index) const
{
	ASSERT(index < MAX_TEAM_NUM && index >= 0);	
	return dw_member_id_[index];	
}

//---------------------------------------------------------------------------------------------
// 玩家是否在该队伍中，如果在，返回在队伍中的索引，否则返回INVALID_VALUE
//---------------------------------------------------------------------------------------------
inline INT Team::is_role_in_team(DWORD dw_member) const
{
	for(INT i = 0; i < member_number_; ++i)
	{
		if( dw_member_id_[i] == dw_member )
			return i;
	}
	return INVALID_VALUE;
}

//---------------------------------------------------------------------------------------------
// 对于队伍中每个各成员执行UnitOperation
//---------------------------------------------------------------------------------------------
template<typename UnitOperation>
VOID Team::for_each( UnitOperation oper ) const
{
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( !VALID_POINT(p_role) ) break;
		oper(p_role);
	}
}	

//---------------------------------------------------------------------------------------------
// 设置/获取共享环随机//mwh 2011-09-06 
//---------------------------------------------------------------------------------------------
inline VOID Team::leader_set_share_circle_quest(BOOL b)
{
	b_leader_share_circle_quest_ = b;
}

inline BOOL Team::get_leader_share_circle_quest( ) const
{
	return b_leader_share_circle_quest_;
}

#endif //__TEAM__