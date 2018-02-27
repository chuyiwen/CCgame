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


#include "StdAfx.h"

#include "../../common/WorldDefine/filter.h"
#include "../../common/WorldDefine/team_protocol.h"
#include "../../common/WorldDefine/SocialDef.h"

#include "team.h"
#include "role.h"
#include "role_mgr.h"
#include "creature.h"
#include "map_creator.h"
#include "map.h"
#include "map_instance.h"
#include "pet_heti.h"
#include "TeamRandShareMgr.h"

// ctor / dtor
Team::Team(DWORD dw_team, Role* p_leader, Role* p_first_member)
: dw_team_id_(dw_team), dw_group_id_(INVALID_VALUE), dw_own_instance_(INVALID_VALUE), dw_own_map_(INVALID_VALUE)
,member_number_(2), e_pick_mode_(EPUM_Sice), f_exp_factor_(0.0f), b_need_delete_(FALSE)
,team_placard_(_T("欢迎朋友们加入本队伍。"))
{
	team_syn_tick_	=	g_world.GetWorldTick();
	
	// 初始化
	memset(dw_member_id_, 0xFF, sizeof(dw_member_id_));
	//memset(p_member_, 0xFF, sizeof(p_member_));

	// 设置队员
	dw_member_id_[0]	= p_leader->GetID();
	//p_member_[0]	= p_leader;
	add_member_data(p_leader);
	dw_member_id_[1]	= p_first_member->GetID();
	//p_member_[1]	= p_first_member;
	add_member_data(p_first_member);

	p_leader->SetTeamID(dw_team);
	p_leader->SetLeader(TRUE);
	p_leader->GetAchievementMgr().UpdateAchievementCriteria(eta_join_team, 1);
	p_first_member->SetTeamID(dw_team);
	p_first_member->SetLeader(FALSE);
	p_first_member->GetAchievementMgr().UpdateAchievementCriteria(eta_join_team, 1);

	// 加入拾取列表
	pick_list_.push_back(0);
	pick_list_.push_back(1);
	
	max_level_ = max(p_leader->get_level(), p_first_member->get_level());
	min_level = min(p_leader->get_level(), p_first_member->get_level());

	// 计算经验因子
	cal_exp_factor();

	// 默认绿色品级
	e_assign_quality_ =  EIQ_Quality1;

	b_leader_share_circle_quest_ = FALSE;
}

Team::Team(DWORD dw_team, Role* p_leader)
: dw_team_id_(dw_team), dw_group_id_(INVALID_VALUE), dw_own_instance_(INVALID_VALUE), dw_own_map_(INVALID_VALUE),
member_number_(1), e_pick_mode_(EPUM_Sice), f_exp_factor_(0.0f), b_need_delete_(FALSE),team_placard_(_T("欢迎朋友们加入本小队。"))
{
	team_syn_tick_	=	g_world.GetWorldTick();

	// 初始化
	memset(dw_member_id_, 0xFF, sizeof(dw_member_id_));
	//memset(p_member_, 0xFF, sizeof(p_member_));

	// 设置队员
	dw_member_id_[0]	= p_leader->GetID();
	//p_member_[0]	= p_leader;
	add_member_data(p_leader);

	p_leader->SetTeamID(dw_team);
	p_leader->SetLeader(TRUE);
	p_leader->GetAchievementMgr().UpdateAchievementCriteria(eta_join_team, 1);

	// 加入拾取列表
	pick_list_.push_back(0);

	max_level_ = p_leader->get_level();
	min_level = p_leader->get_level();

	// 计算经验因子
	cal_exp_factor();

	// 默认绿色品级
	e_assign_quality_ =  EIQ_Quality1;

	b_leader_share_circle_quest_ = FALSE;
}

Team::~Team()
{
	for(INT i = 0; i < MAX_TEAM_NUM; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_role) )
		{
			p_role->SetTeamID(INVALID_VALUE);
			p_role->SetLeader(FALSE);
		}			
		else
			break;
	}

	pick_list_.clear();

	clear_apply_member();

	MAP_MEMBER::map_iter iter = member_datas_.begin();
	tagTeamMemberData* pTeamMemData = NULL;
	while(member_datas_.find_next(iter, pTeamMemData))
	{
		if(VALID_POINT(pTeamMemData))
		{
			SAFE_DELETE(pTeamMemData);
		}
	}
	member_datas_.clear();
}


// 全队伍消息
VOID Team::send_team_message(LPVOID p_message, DWORD dw_size)
{
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if(!VALID_POINT(p_role) ) continue;
		p_role->SendMessage(p_message, dw_size);
	}
}

// 队友消息
VOID Team::send_teamate_message(DWORD dw_role_id, LPVOID p_message, DWORD dw_size)
{
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_role) && dw_member_id_[i] != dw_role_id )
			p_role->SendMessage(p_message, dw_size);
	}
}

// 同地图消息
INT Team::send_team_msg_in_same_map(DWORD dw_map, LPVOID p_message, DWORD dw_size)
{
	INT number = 0;
	for( INT i = 0; i < member_number_; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_role) && dw_map == p_role->GetMapID())
		{
			p_role->SendMessage(p_message, dw_size);
			++number;
		}
	}
	return number;
}


// 视野外队伍消息(不给自己发)
VOID Team::send_team_msg_out_big_view(Role* p_role, LPVOID p_message, DWORD dw_size)
{
	if( !VALID_POINT(p_role) ) return;

	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_temp_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_temp_role) && p_role != p_temp_role 
			&& (p_role->GetMapID() != p_temp_role->GetMapID() || !p_role->get_map()->in_same_big_visible_tile(p_role, p_temp_role)) )
			p_temp_role->SendMessage(p_message, dw_size);
	}
}
// 视野内队伍消息(不给dwExcept发)
INT Team::send_team_msg_in_big_view(Role* p_role, LPVOID p_message, DWORD dw_size, DWORD dw_except)
{
	if( !VALID_POINT(p_role) ) return 0;

	INT n_Number = 0;
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_temp_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_temp_role) && 
			p_role->GetID() != dw_except && 
			p_role->GetMapID() == p_temp_role->GetMapID() &&
			p_role->get_map()->in_same_big_visible_tile(p_role, p_temp_role) )
		{
			p_temp_role->SendMessage(p_message, dw_size);
			++n_Number;
		}
	}
	return n_Number;
}

// 通知队友进副本
VOID Team::send_team_instance_notice(Role* p_role, LPVOID p_message, DWORD dw_size)
{
	if( !VALID_POINT(p_role) ) return;

	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_temp_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( VALID_POINT(p_temp_role) && p_role != p_temp_role 
		&& (p_role->GetMapID() != p_temp_role->GetMapID() || !p_role->get_map()->in_same_big_visible_tile(p_role, p_temp_role)) 
		&& !p_temp_role->IsInRoleState(ERS_PrisonArea))
			p_temp_role->SendMessage(p_message, dw_size);
	}
}


// 导出所有队员ID
VOID Team::export_all_member_id(DWORD dw_member[MAX_TEAM_NUM]) const
{
	get_fast_code()->memory_copy(dw_member, dw_member_id_, sizeof(DWORD)*MAX_TEAM_NUM);
}

// 平均等级
INT Team::get_average_level() const
{
	INT nTotalLevel = 0;
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_temp_role = g_roleMgr.get_role(dw_member_id_[i]);
		if( !VALID_POINT(p_temp_role) )
			break;
		nTotalLevel += p_temp_role->get_level();
	}

	return nTotalLevel / member_number_;
}

// 增加队友
INT Team::add_member(Role* p_inviter, Role* p_replyer)
{
	INT n_ret = can_add_member(p_inviter, p_replyer);

	if( E_Success != n_ret ) return n_ret;

	p_replyer->SetTeamID(p_inviter->GetTeamID());
	p_replyer->SetLeader(FALSE);
	p_replyer->GetAchievementMgr().UpdateAchievementCriteria(eta_join_team, 1);

	if(VALID_POINT(p_replyer->GetScript()))
	{
		p_replyer->GetScript()->OnJoinTeam(p_replyer);
	}

	// 加入成员
	dw_member_id_[member_number_] = p_replyer->GetID();
	//p_member_[member_number_] = p_replyer;
	add_member_data(p_replyer);

	max_level_ = max(max_level_, p_replyer->get_level());
	min_level = min(min_level, p_replyer->get_level());

	cal_exp_factor();

	++member_number_;


	pick_list_.push_back(member_number_ - 1);

	on_add_member(p_replyer);

	this->resort_online_offline_member();

	return E_Success;
}

// 能否加入
INT Team::can_add_member(Role* p_inviter, Role* p_replyer)
{
	// 邀请者不是队长
	if( !is_leader(p_inviter->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 被邀请者已经有队
	if( INVALID_VALUE != p_replyer->GetTeamID() )
	{
		return E_Team_Target_Have_Team;
	}

	// 人数已满
	if( member_number_ >= MAX_TEAM_NUM )
	{
		return E_Team_Member_Full;
	}

	return E_Success;
}

// 踢掉队友
INT Team::kick_member(Role* p_src, Role* p_dest)
{
	if( !VALID_POINT(p_src) || !VALID_POINT(p_dest) ) return INVALID_VALUE;

	INT index = INVALID_VALUE;

	INT ret = can_kick_member(p_src, p_dest, index);

	if( E_Success != ret ) return ret;
	
	Map* pMap = p_dest->get_map();
	if ( VALID_POINT(pMap) )
	{
		ret = pMap->can_kick_member();
		if( E_Success != ret ) return ret;
	}
	
	// 可以踢掉队友，则踢掉
	delete_member(index, FALSE);

	return E_Success;
}

// 踢掉队友
INT	Team::kick_member(Role* p_src, DWORD dw_dest_id)
{
	if(!VALID_POINT(p_src)) return INVALID_VALUE;

	INT index = INVALID_VALUE;

	INT ret = can_kick_member(p_src, dw_dest_id, index);

	if(E_Success != ret) return ret;

	// 可以踢掉队友，则踢掉
	delete_member(index, FALSE);

	return E_Success;

}

// 是否可以踢人，nIndex返回索引
INT Team::can_kick_member(Role* p_src, Role* p_dest, INT& index)
{
	// 不是队长
	if( !is_leader(p_src->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 不是自己
	if( p_src->GetID() == p_dest->GetID() )
	{
		return E_Team_Target_Not_Exit;
	}

	// 被踢者在队伍里，不能踢掉队长
	index = is_role_in_team(p_dest->GetID());
	if( INVALID_VALUE == index || 0 == index )	
	{
		return E_Team_Role_Not_In_Team;
	}

	return E_Success;
}

// 是否可以踢人，nIndex返回索引
INT	Team::can_kick_member(Role* p_src, DWORD dw_dest, INT& index)
{
	// 不是队长
	if( !is_leader(p_src->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 不是自己
	if( p_src->GetID() == dw_dest )
	{
		return E_Team_Target_Not_Exit;
	}

	// 被踢者在队伍里，不能踢掉队长
	index = is_role_in_team(dw_dest);
	if( INVALID_VALUE == index || 0 == index )
	{
		return E_Team_Role_Not_In_Team;
	}

	return E_Success;
}

// 离队
INT Team::leave_team(Role* p_role, BOOL b_leave_line)
{
	INT index = INVALID_VALUE;

	INT ret = can_leave_team(p_role, index);

	if( E_Success != ret ) return ret;

	// 地图是否允许玩家离队
	Map* p_map = p_role->get_map();
	if ( VALID_POINT(p_map) )
	{
		ret = p_map->can_leave_team();
		if( E_Success != ret ) return ret;
	}

	// 可以删除
	delete_member(index, b_leave_line);

	return E_Success;
}

// 是否可以离队
INT Team::can_leave_team(Role* p_role, INT& index)
{
	// 检查玩家是不是在队伍里
	index = is_role_in_team(p_role->GetID());
	if( INVALID_VALUE == index )
	{
		return E_Team_Role_Not_In_Team;
	}

	return E_Success;
}

// 删除某个索引队员
VOID Team::delete_member(const INT index, BOOL b_leave_line)
{
	if( index < 0 || index >= member_number_ ) return;

    Role* p_member = g_roleMgr.get_role(dw_member_id_[index]);

	// 如果成员不再线
	if( !VALID_POINT(p_member) )
	{
		delete_member_data(dw_member_id_[index]);
		dw_member_id_[index] = INVALID_VALUE;

		// 重置该小队成员的结构
		for(INT n = index; n < member_number_ - 1; ++n)
		{
			DWORD dw_id = dw_member_id_[n];
			dw_member_id_[n] = dw_member_id_[n+1];
			dw_member_id_[n+1] = dw_id;
			//p_member_[n] = p_member_[n+1];
		}
		--member_number_;

		// 没人，设置队伍删除
		if( member_number_ <= 1 )
		{
			b_need_delete_ = TRUE;
		}

		return;
	}

	// 设置该成员的小队ID
	if(!b_leave_line)
	{
		p_member->SetTeamID(INVALID_VALUE);
		p_member->SetLeader(FALSE);
	}
	
	DWORD dwOldLeaderID = (index == 0) ? dw_member_id_[0] : INVALID_VALUE;
	// 重置该小队成员的结构
	for(INT n = index; n < member_number_ - 1; ++n)
	{
		DWORD dw_id = dw_member_id_[n];
		dw_member_id_[n] = dw_member_id_[n+1];
		dw_member_id_[n+1] = dw_id;
		//p_member_[n] = p_member_[n+1];
	}

	if(!b_leave_line)
	{
		delete_member_data(dw_member_id_[member_number_ - 1]);
		dw_member_id_[member_number_ - 1] = INVALID_VALUE;
		//p_member_[member_number_ - 1] = (Role*)INVALID_VALUE;
		--member_number_;

		// 重置拾取顺序
		pick_list_.remove(index);
		list<INT>::iterator it = pick_list_.begin();
		while( it != pick_list_.end() )
		{
			if(*it > index)
				(*it)--;
			++it;
		}

		// 重算最高(低)等级
		recalc_team_level();

		// 重算队伍经验参数
		cal_exp_factor();

		// 离开队伍事件
		on_delete_member(p_member, b_leave_line);

		// 没人，设置队伍删除
		if( member_number_ <= 1 )
		{
			b_need_delete_ = TRUE;
		}
	}

	if(b_leave_line)
	{
		//p_member_[member_number_ - 1] = (Role*)INVALID_VALUE;

		BOOL bLine = FALSE;
		for(INT n = 0; n < member_number_ - 1; ++n)
		{
			Role* p_role = g_roleMgr.get_role(dw_member_id_[n]);
			if(VALID_POINT(p_role))
			{
				bLine = TRUE;
				break;
			}
		}
		if(!bLine)
		{
			b_need_delete_ = TRUE;
			return;
		}

		// 重置拾取顺序
		pick_list_.remove(index);
		list<INT>::iterator it = pick_list_.begin();
		while( it != pick_list_.end() )
		{
			if(*it > index)
				(*it)--;
			++it;
		}

		// 重算最高(低)等级
		recalc_team_level();

		// 重算队伍经验参数
		cal_exp_factor();

		// 离开队伍事件
		on_delete_member(p_member, b_leave_line);

	}

	Role* pLeader = g_roleMgr.get_role(dw_member_id_[0]);
	if(VALID_POINT(pLeader))
	{
		pLeader->SetLeader(TRUE);

		NET_SIS_team_leader_set send;
		send.dwTeamID = b_need_delete_ ? INVALID_VALUE:dw_team_id_;
		send.bLeader = b_need_delete_ ? FALSE: TRUE;
		send.dwRoleID = pLeader->GetID();
		if(VALID_POINT(pLeader->get_map()))
			pLeader->get_map()->send_big_visible_tile_message(pLeader,&send, send.dw_size);
	}	

	if(VALID_POINT(p_member))
	{
		NET_SIS_team_leader_set send;
		send.dwTeamID = INVALID_VALUE;
		send.bLeader = FALSE;
		send.dwRoleID = p_member->GetID();
		if(VALID_POINT(p_member->get_map()))
			p_member->get_map()->send_big_visible_tile_message(p_member, &send, send.dw_size);
	}
	
}

// 设置小队的拾取模式
INT Team::set_pick_mode(Role* p_role, EPickMode e_pick_mode)
{
	// 不是队长不能设置
	if( !is_leader(p_role->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 判断拾取模式是否非法 // Ares:队长分配模式暂停
	if( e_pick_mode < EPUM_Free || e_pick_mode >  EPUM_Leader)
	{
		return E_Team_Pick_Model_Not_Exit;
	}

	// 判断，成功，设置拾取模式
	e_pick_mode_ = e_pick_mode;

	return E_Success;
}

// 设置分配等级
INT Team::set_assign_quality(Role* p_role, EItemQuality e_quality )
{
	// 不是队长不能设置
	if( !is_leader(p_role->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 判断拾取模式是否非法
	if( e_quality < EIQ_Quality0 || e_quality >  EIQ_Quality4)
	{
		return E_Team_Quality_Not_Exist;
	}

	// 设置分配等级
	e_assign_quality_ = e_quality;

	return E_Success;
}

// 改变队长
INT Team::change_leader(Role* p_src, Role* p_dest)
{
	// 不是队长，不能改变队长
	if( !is_leader(p_src->GetID()) )
	{
		return E_Team_Not_Leader;
	}

	// 查看是不是自己
	if( p_src->GetID() == p_dest->GetID() )
	{
		return E_Team_Target_Not_Exit;
	}

	// 查看玩家是不是在队伍中
	INT index = is_role_in_team(p_dest->GetID());
	if( INVALID_VALUE == index || 0 == index )
	{
		return E_Team_Role_Not_In_Team;
	}

	// 判断在某地图是否允许移交队长
	Map* p_map = p_src->get_map();
	if ( VALID_POINT(p_map) )
	{
		INT ret = E_Success;
		ret = p_map->can_change_leader();
		if( E_Success != ret ) return ret;
	}

	p_src->SetLeader(FALSE);
	p_dest->SetLeader(TRUE);
	// 判断成功，交换两个人的位置
	swap(dw_member_id_[0], dw_member_id_[index]);
	//swap(p_member_[0], p_member_[index]);

	// 交换拾取顺序
	list<INT>::iterator it = pick_list_.begin();
	while (it != pick_list_.end())
	{
		if(*it == index)
		{
			*it = 0;
		}
		else if(*it == 0)	
		{
			*it = index;
		}

		++it;
	}

	// 重算整个队伍的经验参数
	cal_exp_factor();

	return E_Success;
}

// 申请入队
DWORD Team::add_apply_member(Role* p_apply_role)
{

	Role* p_role = g_roleMgr.get_role(dw_member_id_[0]);
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	if (INVALID_VALUE != p_role->GetTeamInvite())
		return E_Team_Target_Busy;

	tagApplyRoleData* p_apply_data = team_apply_.find(p_apply_role->GetID());
	if(VALID_POINT(p_apply_data))
		return E_Team_Have_Apply;
	
	if( get_member_number() >= MAX_TEAM_NUM )
		return E_Team_Member_Full;

	p_apply_data = new tagApplyRoleData;
	p_apply_data->dw_role_id = p_apply_role->GetID();
	p_apply_data->bySex = p_apply_role->GetSex();
	p_apply_data->nLevel = p_apply_role->get_level();
	p_apply_data->eClassEx = p_apply_role->GetClass();
	for(INT i = 0; i < TEAM_DISPLAY_NUM; ++i)
	{
		p_apply_data->dwEquipTypeID[i] = p_apply_role->GetAvatarEquip().AvatarEquip[i].dw_data_id;
	}
	memcpy(&p_apply_data->AvatarAtt,p_apply_role->GetAvatar(), sizeof(tagAvatarAtt));
	memcpy(&p_apply_data->stEquipTeamInfo, &p_apply_role->GetEquipTeamInfo(), sizeof(tagEquipTeamInfo));
	team_apply_.add(p_apply_role->GetID(),p_apply_data);

	/*for(INT n = 0; n < member_number_; ++n)
	{*/
		
	NET_SIS_apply_data send;
	memcpy(&send.st_ApplyRoleData, p_apply_data, sizeof(tagApplyRoleData));
	memcpy(&send.st_EquipTeamInfo, &p_apply_role->GetEquipTeamInfo(), sizeof(tagEquipTeamInfo));

	p_role->SendMessage(&send, send.dw_size);
	/*}*/
	
	p_role->SetTeamInvite(p_apply_role->GetID());
	return E_Success;
}

// 删除申请玩家
VOID Team::delete_apply_member(DWORD dw_apply_id)
{
	tagApplyRoleData* p_apply_data = team_apply_.find(dw_apply_id);
	if(VALID_POINT(p_apply_data)) SAFE_DELETE(p_apply_data);
	team_apply_.erase(dw_apply_id);
}

// 清空申请列表
VOID Team::clear_apply_member()
{
	MAP_APPLY::map_iter iter = team_apply_.begin();
	tagApplyRoleData* p_apply_data = NULL;
	while(team_apply_.find_next(iter, p_apply_data))
	{
		if(VALID_POINT(p_apply_data))
		{
			SAFE_DELETE(p_apply_data);
		}
	}
	team_apply_.clear();
}

// 添加队员数据
VOID Team::add_member_data(Role* p_role)
{
	if(!VALID_POINT(p_role))
		return;

	tagTeamMemberData* p_member_data = member_datas_.find(p_role->GetID());
	if(VALID_POINT(p_member_data))
		return;

	p_member_data = new tagTeamMemberData;
	p_member_data->dw_role_id	=	p_role->GetID();
	p_member_data->dwMapID	=	p_role->GetMapID();
	p_member_data->eClassEx	=	p_role->GetClass();
	p_member_data->bySex		=	p_role->GetSex();
	p_member_data->nLevel	=	p_role->get_level();
	p_member_data->nMaxHP	=	p_role->GetAttValue(ERA_MaxHP);
	p_member_data->nHP		=	p_role->GetAttValue(ERA_HP);
	p_member_data->nMaxMP	=	p_role->GetAttValue(ERA_MaxMP);
	p_member_data->nMP		=	p_role->GetAttValue(ERA_MP);
	p_member_data->fX		=	p_role->GetCurPos().x;
	p_member_data->fZ		=	p_role->GetCurPos().z;
	for(INT i = 0; i < TEAM_DISPLAY_NUM; ++i)
	{
		p_member_data->dwEquipTypeID[i] = p_role->GetAvatarEquip().AvatarEquip[i].dw_data_id;
	}
	memcpy(&p_member_data->AvatarAtt, p_role->GetAvatar(), sizeof(tagAvatarAtt));
	memcpy(&p_member_data->st_EquipTeamInfo, &p_role->GetEquipTeamInfo(), sizeof(tagEquipTeamInfo));

	member_datas_.add(p_member_data->dw_role_id, p_member_data);
}


// 删除队员数据
VOID Team::delete_member_data(DWORD dw_role_id)
{
	tagTeamMemberData* p_member_data = member_datas_.find(dw_role_id);
	if(VALID_POINT(p_member_data))
	{
		SAFE_DELETE(p_member_data);
	}
	member_datas_.erase(dw_role_id);
}

// 队员上线
VOID Team::member_online(INT index, Role* p_role)
{
	//p_member_[index] = p_role;

	// 重置拾取顺序
	pick_list_.push_back(index);
	
	// 重新计算小队最高,最低等级
	recalc_team_level();

	// 重算队伍经验参数
	cal_exp_factor();

	NET_SIS_member_online send;
	send.dw_role_id = p_role->GetID();
	send_team_message(&send, send.dw_size);

	// 发送队员初始消息
	send_role_init_state_to_team(p_role);

	// 发送给新加入的玩家
	send_team_state(p_role);

	// 加入队伍事件
	on_add_member(p_role);
}

// 队员等级改变
VOID Team::member_level_change(Role* p_role)
{
	INT index = is_role_in_team(p_role->GetID());

	if( INVALID_VALUE == index ) return;

	max_level_ = max(max_level_, p_role->get_level());
	min_level = min(min_level, p_role->get_level());
}

// 队长统御值改变
VOID Team::leader_rein_change(Role* p_role)
{
	if( !is_leader(p_role->GetID()) ) return;

	// 重算统御值
	cal_exp_factor();
}

// 向新队员同步队友信息, 向老队员同步新队员信息
VOID Team::send_role_init_state_to_team(Role* p_new_member)
{
	if( !VALID_POINT(p_new_member) ) return;

	for(INT n = 0; n < member_number_; ++n)
	{
		if(dw_member_id_[n] == INVALID_VALUE) continue;

		// 如果是自己就跳过
		if(dw_member_id_[n] == p_new_member->GetID() ) continue;

		// 将新队员的消息发送给老队员
		NET_SIS_role_state_to_team sendToOld;

		sendToOld.dw_role_id	=	p_new_member->GetID();
		sendToOld.dwMapID	=	p_new_member->GetMapID();
		sendToOld.eClassEx	=	p_new_member->GetClass();
		sendToOld.bySex		=	p_new_member->GetSex();
		sendToOld.nLevel	=	p_new_member->get_level();
		sendToOld.nMaxHP	=	p_new_member->GetAttValue(ERA_MaxHP);
		sendToOld.nHP		=	p_new_member->GetAttValue(ERA_HP);
		sendToOld.nMaxMP	=	p_new_member->GetAttValue(ERA_MaxMP);
		sendToOld.nMP		=	p_new_member->GetAttValue(ERA_MP);
		sendToOld.fX		=	p_new_member->GetCurPos().x;
		sendToOld.fZ		=	p_new_member->GetCurPos().z;
		sendToOld.b_leader = FALSE;
		sendToOld.b_online	=	TRUE;
		for(INT i = 0; i < TEAM_DISPLAY_NUM; ++i)
		{
			sendToOld.dwEquipTypeID[i] = p_new_member->GetAvatarEquip().AvatarEquip[i].dw_data_id;
		}
		memcpy(&sendToOld.AvatarAtt, p_new_member->GetAvatar(), sizeof(tagAvatarAtt));
		memcpy(&sendToOld.st_EquipTeamInfo, &p_new_member->GetEquipTeamInfo(), sizeof(tagEquipTeamInfo));

		Role* p_role = g_roleMgr.get_role(dw_member_id_[n]);
		if( VALID_POINT(p_role) )
		{
			p_role->SendMessage(&sendToOld,sendToOld.dw_size);
		}
	


		tagTeamMemberData* pMemData = member_datas_.find(dw_member_id_[n]);
		if(!VALID_POINT(pMemData)) continue;

		s_role_info* p_role_info = g_roleMgr.get_role_info(dw_member_id_[n]);
		if(!VALID_POINT(p_role_info)) continue;
		// 将老队员的消息发送给新队员
		NET_SIS_role_state_to_team sendToNew;

		sendToNew.dw_role_id	=	pMemData->dw_role_id;
		sendToNew.dwMapID	=	pMemData->dwMapID;
		sendToNew.eClassEx	=	pMemData->eClassEx;
		sendToNew.bySex		=	pMemData->bySex;
		sendToNew.nLevel	=	pMemData->nLevel;
		sendToNew.nMaxHP	=	pMemData->nMaxHP;
		sendToNew.nHP		=	pMemData->nHP;
		sendToNew.nMaxMP	=	pMemData->nMaxMP;
		sendToNew.nMP		=	pMemData->nMP;
		sendToNew.fX		=	pMemData->fX;
		sendToNew.fZ		=	pMemData->fZ;
		sendToNew.b_leader  =	(n == 0);
		sendToNew.b_online	=	p_role_info->b_online_;
		for(INT i = 0; i < TEAM_DISPLAY_NUM; ++i)
		{
			sendToNew.dwEquipTypeID[i] = pMemData->dwEquipTypeID[i];
		}
		memcpy(&sendToNew.AvatarAtt,&pMemData->AvatarAtt, sizeof(tagAvatarAtt));
		memcpy(&sendToNew.st_EquipTeamInfo, &pMemData->st_EquipTeamInfo, sizeof(tagEquipTeamInfo));

		p_new_member->SendMessage(&sendToNew, sendToNew.dw_size);

	}
}

// 发送队伍信息给玩家
VOID Team::send_team_state(Role* p_new_member)
{
	if( !VALID_POINT(p_new_member) ) return;

	NET_SIS_get_team_id		send; 
	send.dwTeamID = get_team_id();
	send.ePickMode = get_pick_mode();
	_tcsncpy(send.szTeamPlacard, get_team_placard().c_str(), MAX_TEAM_PLACARD_LEN);

	p_new_member->SendMessage(&send, send.dw_size);
}

// 同步队友位置
VOID Team::update_teamate_position()
{
	for(INT i = 0; i < member_number_; ++i)
	{
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if(VALID_POINT(p_role))
		{
			NET_SIS_role_position_to_team		send;
			send.dw_role_id = p_role->GetID();
			send.dwMapID = p_role->GetMapID();
			send.fX = p_role->GetCurPos().x;
			send.fZ = p_role->GetCurPos().z;
			send_team_msg_out_big_view(p_role, &send, send.dw_size);
		}	
	}
}

// 重新计算队伍最高(低)等级
VOID Team::recalc_team_level()
{
	Role* p_role = g_roleMgr.get_role(dw_member_id_[0]);
	if(VALID_POINT(p_role))
	{
		max_level_ = p_role->get_level();
		min_level = p_role->get_level();
	}

	for(INT i = 1; i < member_number_; ++i)
	{
		p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if(!VALID_POINT(p_role))
			continue;

		if(p_role->get_level() > max_level_)
			max_level_ = p_role->get_level();

		if(p_role->get_level() < min_level)
			min_level = p_role->get_level();
	}
}

// 设置队伍杀怪经验因数
VOID Team::cal_exp_factor()
{
	Role* p_role = g_roleMgr.get_role(dw_member_id_[0]);
	if(!VALID_POINT(p_role))
		return;

	FLOAT f_leader_rein = (FLOAT)p_role->GetAttValue(ERA_Rein);

	// _f = ((150-（1-队长统御力/999）*（maxlevel-minlevel）)/150)^4
	FLOAT _f = pow((150.0f - (1.0f - f_leader_rein / 999.0f) * ((FLOAT)max_level_ - (FLOAT)min_level)) / 150.0f, 4.0f);

	f_exp_factor_ = _f;
}

// 获得掉落拾取玩家
Role* Team::get_pick_role(Creature* p_creature) const
{
	if(!VALID_POINT(p_creature)) return NULL;

	switch( e_pick_mode_ )
	{
// 	case EPUM_Order:
// 	{
// 			lock_.Acquire();
// 			list<INT>::iterator it = pick_list_.begin();
// 			Role* p_role = NULL;
// 			while( it != pick_list_.end() )
// 			{
// 				p_role = g_roleMgr.get_role(dw_member_id_[*it]);
// 				if(VALID_POINT(p_role) && p_creature->IsLootShareDistance(p_role))
// 				{
// 					INT index = *it;
// 					pick_list_.remove(index);
// 					pick_list_.push_back(index);
// 					lock_.Release();
// 					return g_roleMgr.get_role(dw_member_id_[index]);
// 				}
// 
// 				++it;
// 			}
// 			lock_.Release();
// 
// 			// 如果没有，就是全体
// 			return NULL;
// 	}
	case EPUM_Free:
	case EPUM_Leader: // 等待队长分配
	case EPUM_Sice:	  // 掷骰子
		return NULL;
	// 执行到default 表示有分配方式未处理
	default:{ ASSERT(FALSE); return NULL; }
	};
}


// 队伍创建时
VOID Team::on_create(BOOL b_own)
{
	
	// 将队长的副本ID设置为队伍的副本ID，同时清空队长的副本ID
	Role* p_role = g_roleMgr.get_role(dw_member_id_[0]);
	if(!VALID_POINT(p_role))
		return;

	if( VALID_VALUE(p_role->GetMyOwnInstanceID()) && VALID_VALUE(p_role->GetMyOwnInstanceMapID()) )
	{
		Map* p_map = g_mapCreator.get_map(p_role->GetMyOwnInstanceMapID(), p_role->GetMyOwnInstanceID());
		if(VALID_POINT(p_map))
		{
			set_own_instance_mapid(p_role->GetMyOwnInstanceMapID());
			set_own_instanceid(p_role->GetMyOwnInstanceID());
		}
	}

	// 如果副本ID不为空，则设置原来单人副本为队伍副本
	if( VALID_VALUE(get_own_instanceid()) && VALID_VALUE(get_own_instance_mapid()) )
	{
		Map* p_map = g_mapCreator.get_map(get_own_instance_mapid(), get_own_instanceid());

		// 确定是普通副本
		if( VALID_POINT(p_map) && EMT_Instance == p_map->get_map_type() )
		{
			map_instance_normal* p_instance = static_cast<map_instance_normal*>(p_map);
			if( VALID_POINT(p_instance) ) p_instance->on_team_create(this);
		}
	}
}

// 队伍删除时
VOID Team::on_delete()
{
	// 如果该队伍有副本，则设置副本删除
	if( VALID_VALUE(get_own_instanceid()) && VALID_VALUE(get_own_instance_mapid()) )
	{
		Map* pMap = g_mapCreator.get_map(get_own_instance_mapid(), get_own_instanceid());

		// 确定是普通副本
		if( VALID_POINT(pMap) && EMT_Instance == pMap->get_map_type() )
		{
			map_instance_normal* p_instance = static_cast<map_instance_normal*>(pMap);
			if( VALID_POINT(p_instance) ) p_instance->on_team_delete(this);
		}
	}

	//sTeamShareMgr.DelOne(get_team_id());
}

// 当加入一个玩家
VOID Team::on_add_member(Role* p_role)
{
	if( !VALID_POINT(p_role) ) return;

	//sTeamShareMgr.MemberJoin(get_team_id(), p_role->GetID());

	// 检查队伍现在是否创建了副本
	if( VALID_VALUE(get_own_instanceid()) && VALID_VALUE(get_own_instance_mapid()) )
	{
		Map* p_map = g_mapCreator.get_map(get_own_instance_mapid(), get_own_instanceid());

		// 确定是普通副本
		if( VALID_POINT(p_map) && EMT_Instance == p_map->get_map_type() )
		{
			map_instance_normal* p_instance = static_cast<map_instance_normal*>(p_map);
			if( VALID_POINT(p_instance) ) p_instance->on_role_enter_team(p_role->GetID(), this);
		}
	}
	
	NET_SIS_team_leader_set send;
	send.dwTeamID = get_team_id();
	send.bLeader = FALSE;
	send.dwRoleID = p_role->GetID();
	if(VALID_POINT(p_role->get_map()))
		p_role->get_map()->send_big_visible_tile_message(p_role, &send, send.dw_size);

	// 检测队伍buff
	if ( member_number_ < 5 )
		return;
	
	BOOL bHasClass[5];
	memset(&bHasClass, 0, sizeof(bHasClass));
	for (int i = 0; i < member_number_; i++)
	{
		Role* pRole = get_member(i);
		if (p_role->GetID() == get_member_id(i))
		{
			pRole = p_role;
		}

		if (!VALID_POINT(pRole))
		{
			continue;
		}
		INT nClass = (INT)pRole->GetClass();
		if (nClass >= 1 && nClass <= 5)
		{
			bHasClass[nClass-1] = TRUE;
		}
		
	}

	for (int i = 0; i < 5; i++)
	{
		if (!bHasClass[i])
			return;
	}

	for (int i = 0; i < member_number_; i++)
	{
		Role* pRole = get_member(i);
		if (p_role->GetID() == get_member_id(i))
		{
			pRole = p_role;
		}

		if (!VALID_POINT(pRole))
			return;
		pRole->TryAddBuff(pRole, AttRes::GetInstance()->GetBuffProto(TEAM_BUFF_ID), NULL, NULL, NULL);
	}

}

// 当删除一个玩家
VOID Team::on_delete_member(Role* p_role, BOOL b_leave_line)
{
	if( !VALID_POINT(p_role) ) return;

	//if(b_leave_line == FALSE)
	//	sTeamShareMgr.MemberExit(get_team_id(), p_role->GetID());

	// 检查队伍现在是否创建了副本
	if( VALID_VALUE(get_own_instanceid()) && VALID_VALUE(get_own_instance_mapid()) )
	{
		Map* p_map = g_mapCreator.get_map(get_own_instance_mapid(), get_own_instanceid());

		// 确定是普通副本
		if( VALID_POINT(p_map) && EMT_Instance == p_map->get_map_type() )
		{
			map_instance_normal* p_instance = static_cast<map_instance_normal*>(p_map);
			if( VALID_POINT(p_instance) ) p_instance->on_role_leave_team(p_role->GetID(), this);
		}
	}

	// 不是合自己的宠物,删除武宠合体效果
	DWORD dwPetID;
	if (p_role->GetTargetPet(dwPetID) != p_role)
	{
		p_role->RemoveBuff(HETI_BUFF, TRUE);
		PetHeti::cancelBeHeTiPet(p_role);
		//PetHeti::cancelHeti(p_role);
	}
	

	if ( member_number_>= 4)
	{
		p_role->RemoveBuff(Buff::GetIDFromTypeID(TEAM_BUFF_ID), TRUE);

		// 删掉队伍buff
		for (int i = 0; i < member_number_; i++)
		{
			Role* pRole = get_member(i);
			if (!VALID_POINT(pRole))
				return;

			pRole->RemoveBuff(Buff::GetIDFromTypeID(TEAM_BUFF_ID), TRUE);
		}
	}
}

// 向被邀请人同步队员信息
VOID Team::send_team_member_info_to_invitee(Role* p_invitee)
{
	if(!VALID_POINT(p_invitee))	return;

	for(INT n = 0; n < member_number_; ++n)
	{
		tagTeamMemberData* p_member_data = member_datas_.find(dw_member_id_[n]);
		if(!VALID_POINT(p_member_data)) continue;

		NET_SIS_role_state_to_invitee send;

		send.dw_role_id	=	p_member_data->dw_role_id;
		send.dwMapID	=	p_member_data->dwMapID;
		send.eClassEx	=	p_member_data->eClassEx;
		send.bySex		=	p_member_data->bySex;
		send.nLevel		=	p_member_data->nLevel;
		send.nMaxHP		=	p_member_data->nMaxHP;
		send.nHP		=	p_member_data->nHP;
		send.nMaxMP		=	p_member_data->nMaxMP;
		send.nMP		=	p_member_data->nMP;
		send.fX			=	p_member_data->fX;
		send.fZ			=	p_member_data->fZ;
		for(INT i = 0; i < TEAM_DISPLAY_NUM; ++i)
		{
			send.dwEquipTypeID[i] = p_member_data->dwEquipTypeID[i];
		}
		memcpy(&send.AvatarAtt,&p_member_data->AvatarAtt, sizeof(tagAvatarAtt));
		memcpy(&send.st_EquipTeamInfo, &p_member_data->st_EquipTeamInfo, sizeof(tagEquipTeamInfo));

		p_invitee->SendMessage(&send, send.dw_size);
	}
}


// 删除成员队伍信息
VOID Team::delete_member_team_info()
{
	for(INT i = 0; i < member_number_; ++i)
	{
		// 删除成员数据信息
		delete_member_data(dw_member_id_[i]);
		
		Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
		if(VALID_POINT(p_role))
		{
			p_role->SetTeamID(INVALID_VALUE);
			p_role->SetLeader(FALSE);
		}
		
		dw_member_id_[i] = INVALID_VALUE;
		//p_member_[i] = (Role*)INVALID_VALUE;
	}
	member_datas_.clear();
}


// 修改队伍公告    added by gtj  10-11-09
DWORD Team::change_team_placard(DWORD dw_role_id, LPCTSTR sz_new, INT32 cnt)
{

	if(get_member_id(0) != dw_role_id)
		return E_Team_Not_Leader;

	// 没做修改，不发送客户端
	tstring _temp(sz_new, cnt);
	if(get_team_placard() == _temp)
		return E_Team_Placard_Same_Before;

	// 判断长度
	if(AttRes::GetInstance()->GetVariableLen().nTeamPlacardMax < cnt)
		return E_Team_Placard_Too_Loog;

	// 判断所输入字符是否合法
	DWORD dw_error_code = Filter::IsNameInNameFilterFile(sz_new,AttRes::GetInstance()->GetNameFilterWords());
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 判断通过，设置公告
	set_team_placard(_temp);
	return E_Success;	
}

// 队伍中是否有好友
BOOL Team::is_has_friend(Role* p_role)
{
	if(!VALID_POINT(p_role))
		return FALSE;

	for(INT i = 0; i < member_number_; i++)
	{
		if(dw_member_id_[i] != INVALID_VALUE)
		{
			tagFriend* pFrient = p_role->GetFriendPtr(dw_member_id_[i]);
			if(VALID_POINT(pFrient))
				return TRUE;
		}
	}

	return FALSE;
}

// 判断是否有离线队员
BOOL Team::is_have_leave_role() const
{
	for(INT i = 0; i < member_number_; i++)
	{
		if(dw_member_id_[i] != INVALID_VALUE)
		{
			Role* p_role = g_roleMgr.get_role(dw_member_id_[i]);
			if(!VALID_POINT(p_role))
				return TRUE;
		}
	}
	return FALSE;
}

// 修改队员数据
VOID Team::change_member_datas(Role* p_role)
{
	if(!VALID_POINT(p_role))
		return;

	tagTeamMemberData* pMemberData = member_datas_.find(p_role->GetID());
	if(!VALID_POINT(pMemberData))
		return;

	pMemberData->nLevel	=	p_role->get_level();
	pMemberData->nMaxHP	=	p_role->GetAttValue(ERA_MaxHP);
	pMemberData->nHP	=	p_role->GetAttValue(ERA_HP);
	pMemberData->nMaxMP	=	p_role->GetAttValue(ERA_MaxMP);
	pMemberData->nMP	=	p_role->GetAttValue(ERA_MP);
	pMemberData->fX		=	p_role->GetCurPos().x;
	pMemberData->fZ		=	p_role->GetCurPos().z;
	memcpy(&pMemberData->st_EquipTeamInfo, &p_role->GetEquipTeamInfo(), sizeof(tagEquipTeamInfo));
}

VOID Team::resort_online_offline_member()
{
	for(INT n = 1; n < member_number_  - 1; ++n)
	{
		Role* pRole = g_roleMgr.get_role(dw_member_id_[n]);
		if(!VALID_POINT(pRole))
		{ 
			swap(dw_member_id_[n], dw_member_id_[member_number_ - 1]);
			break;
		}
	}
}