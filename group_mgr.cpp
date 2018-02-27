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
*	@file		group_mgr.cpp
*	@author		mmz 
*	@date		2010/10/07	initial
*	@version	0.0.1.0
*	团队管理类
*/


#include "StdAfx.h"

#include "../../common/WorldDefine/group_define.h"
#include "../../common/WorldDefine/team_protocol.h"
#include "../../common/WorldDefine/drop_protocol.h"
#include "../../common/WorldDefine/TeamRandShareProtocol.h"

#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "group_mgr.h"
#include "player_session.h"
#include "unit.h"
#include "role.h"
#include "role_mgr.h"
#include "team.h"
#include "map_creator.h"
#include "map.h"
#include "map_instance.h"
#include "map_mgr.h"
#include "hearSay_helper.h"
#include "TeamRandShareMgr.h"

//-------------------------------------------------------------------------------------------------------
// 析构函数
//-------------------------------------------------------------------------------------------------------
GroupMgr::~GroupMgr()
{
}

//-------------------------------------------------------------------------------------------------------
// 管理类初时化
//-------------------------------------------------------------------------------------------------------
BOOL GroupMgr::Init()
{
	m_dwTeamIndex = 3000000001;
	m_mapTeam.clear();
	RegisterGroupEventFunc();
	return TRUE;
}

//--------------------------------------------------------------------------------------------------------
// 注册事件
//--------------------------------------------------------------------------------------------------------
VOID GroupMgr::RegisterGroupEventFunc()
{
	RegisterEventFunc(EVT_OwnCreate,		&GroupMgr::OnOwnCreateTeam);
	RegisterEventFunc(EVT_Dismiss,			&GroupMgr::OnDismissTeam);
	RegisterEventFunc(EVT_ApplyJoin,		&GroupMgr::OnApplyJoinTeam);
	RegisterEventFunc(EVT_ApplyJoinReply,	&GroupMgr::OnApplyJoinReply);
	RegisterEventFunc(EVT_CleanApply,		&GroupMgr::OnCleanApply);
	RegisterEventFunc(EVT_MemJoinTeam,		&GroupMgr::OnMemInviteJoinTeam);
	RegisterEventFunc(EVT_MemJoinTeamReply,	&GroupMgr::OnMemInviteJoinTeamReply);
	RegisterEventFunc(EVT_ChangeTeamPlacard,&GroupMgr::OnChangeTeamPlacard);
	RegisterEventFunc(EVT_GetMapTeamInfo,	&GroupMgr::OnGetMapTeamInfo);
	RegisterEventFunc(EVT_JoinTeam,			&GroupMgr::OnInviteJoinTeam);
	RegisterEventFunc(EVT_JoinTeamRepley,	&GroupMgr::OnInviteJoinTeamReply);
	RegisterEventFunc(EVT_KickMember,		&GroupMgr::OnLeaderKickMember);
	RegisterEventFunc(EVT_LeaveTeam,		&GroupMgr::OnMemberLeaveTeam);
	RegisterEventFunc(EVT_SetPickMol,		&GroupMgr::OnSetPickMode);
	RegisterEventFunc(EVT_ChangeLeader,		&GroupMgr::OnChangeLeader);
	RegisterEventFunc(EVT_ChangeLevel,      &GroupMgr::OnChangeLevel);
	RegisterEventFunc(EVT_ChangeRein,		&GroupMgr::OnChangeLeaderRein);
	RegisterEventFunc(EVT_AddAllRoleBuff,	&GroupMgr::OnAddAllRoleBuff);
	RegisterEventFunc(EVT_CreateTeam,		&GroupMgr::OnCreateTeam);
	RegisterEventFunc(EVT_AddMember,		&GroupMgr::OnAddMember);
	RegisterEventFunc(EVT_NoticeAssign,		&GroupMgr::OnNoticeAssign);
	RegisterEventFunc(EVT_TeamMemberSice,	&GroupMgr::OnAssignSice);
	RegisterEventFunc(EVT_TeamSiceFinish,	&GroupMgr::OnSiceFinish);
	RegisterEventFunc(EVT_LeaderAssign,		&GroupMgr::OnLeaderAssign);
	RegisterEventFunc(EVT_SetAssignQuality,	&GroupMgr::OnSetAssignQuality);

	//mwh 2011-09-06
	//RegisterEventFunc(EVT_SetShareLeaderCircleQuest,	&GroupMgr::OnSetShareLeaderCircleQuest);
}

//----------------------------------------------------------------------------------------------------------
// 更新
//----------------------------------------------------------------------------------------------------------
VOID GroupMgr::Update()
{
	EventMgr<GroupMgr>::Update();

	// 同步队员之间的位置
	UpdateTeamatePos();
}

//----------------------------------------------------------------------------------------------------------
// 销毁
//----------------------------------------------------------------------------------------------------------
VOID GroupMgr::Destroy()
{
	package_map<DWORD, Team*>::map_iter it = m_mapTeam.begin();
	Team* pTeam = NULL;

	while( m_mapTeam.find_next(it, pTeam) )
	{
		SAFE_DELETE(pTeam);
	}
	m_mapTeam.clear();
}

//----------------------------------------------------------------------------------------------------------
// 同步每个队伍各个成员的位置
//----------------------------------------------------------------------------------------------------------
VOID GroupMgr::UpdateTeamatePos()
{
	DWORD dwTick = g_world.GetWorldTick();

	Team* pTeam = NULL;
	package_map<DWORD, Team*>::map_iter it = m_mapTeam.begin();

	while( m_mapTeam.find_next(it, pTeam) )
	{
		if( dwTick - pTeam->get_syn_tick() > TEAM_SYN_TICK )
		{
			pTeam->update_teamate_position();
			pTeam->set_syn_tick(dwTick); 
		}
	}
}

//--------------------------------------------------------------------------------------------------------
// 创建队伍
//--------------------------------------------------------------------------------------------------------
Team* GroupMgr::CreateTeam(Role* pInviter, Role* pReplyer)
{
	Team* pTeam = new Team(m_dwTeamIndex, pInviter, pReplyer);
	m_mapTeam.add(m_dwTeamIndex, pTeam);

	// 小队ID索引加一
	++m_dwTeamIndex;

	pTeam->on_create();

	if(VALID_POINT(pReplyer->GetScript()))
	{
		pReplyer->GetScript()->OnJoinTeam(pReplyer);
	}

	if(VALID_POINT(pInviter->GetScript()))
	{
		pInviter->GetScript()->OnJoinTeam(pInviter);
	}

	return pTeam;
}

//--------------------------------------------------------------------------------------------------------
// 创建队伍
//--------------------------------------------------------------------------------------------------------
Team* GroupMgr::CreateTeam(Role* pCreateRole)
{
	Team* pTeam = new Team(m_dwTeamIndex, pCreateRole);
	m_mapTeam.add(m_dwTeamIndex, pTeam);

	// 小队ID索引加一
	++m_dwTeamIndex;

	pTeam->on_create(TRUE);

	return pTeam;
}

//----------------------------------------------------------------------------------------------------------
// 删除队伍
//----------------------------------------------------------------------------------------------------------
VOID GroupMgr::DeleteTeam(Team* pTeam)
{
	if( !VALID_POINT(pTeam) ) return;
	
	// 删除成员队伍信息
	pTeam->delete_member_team_info();

	pTeam->on_delete();

	m_mapTeam.erase(pTeam->get_team_id());
	SAFE_DELETE(pTeam);
}

//---------------------------------------------------------------------------------------------------------
// 申请加入小队
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnApplyJoinTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_apply_join_team* p_receive = (NET_SIC_apply_join_team*)pEventMessage;

	DWORD	dwApplyeeID = p_receive->dwApplyeeID;

	Role* pApplyRole = g_roleMgr.get_role(dwSenderID);
	Role* pApplyeeRole = g_roleMgr.get_role(dwApplyeeID);
	if(!VALID_POINT(pApplyRole) || !VALID_POINT(pApplyeeRole))
		return;

	DWORD dwError = E_Success;
	if(pApplyeeRole->GetTeamID() == INVALID_VALUE)
	{
		dwError = E_Team_Map_NoHave_Team;
	}

	if(pApplyRole->GetTeamID() != INVALID_VALUE)
	{
		dwError = E_Team_Apply_Have_Team;
	}

	Team* pTeam = m_mapTeam.find(pApplyeeRole->GetTeamID());
	if(!VALID_POINT(pTeam))
		return;

	dwError = pTeam->add_apply_member(pApplyRole);

	
	NET_SIS_apply_join_team send;
	send.dwError = dwError;
	pApplyRole->SendMessage(&send, send.dw_size);
	
}

//---------------------------------------------------------------------------------------------------------
// 申请加入小队反馈
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnApplyJoinReply(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_apply_join_team_reply* p_receive = (NET_SIC_apply_join_team_reply*)pEventMessage;

	DWORD dwApplyID = p_receive->dwDestRoleID;

	DWORD dwError = E_Success;

	Role* pSenderRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pSenderRole))
		return;

	Team* pTeam = m_mapTeam.find(pSenderRole->GetTeamID());
	if(!VALID_POINT(pTeam))
		return;
	
	pSenderRole->SetTeamInvite(INVALID_VALUE);

	Role* pApplyRole = g_roleMgr.get_role(dwApplyID);
	if(!VALID_POINT(pApplyRole))
	{
		pTeam->delete_apply_member(dwApplyID);
		dwError = E_Team_Apply_Not_Online;
	}

	if(dwError == E_Success)
	{
		if(p_receive->bApply)
		{
			pTeam->delete_apply_member(dwApplyID);
			dwError = pTeam->add_member(pSenderRole, pApplyRole);
			if(dwError == E_Success)
			{	
				// 发送给所有玩家邀请成功
				NET_SIS_invite_reply send;
				send.bAgree = p_receive->bApply;
				send.dw_error_code = dwError;
				pTeam->export_all_member_id(send.dwTeamMemID);

				pTeam->send_team_message(&send, send.dw_size);

				// 发送队员初始消息
				pTeam->send_role_init_state_to_team(pApplyRole);

				// 发送给新加入的玩家
				pTeam->send_team_state(pApplyRole);
			}
		}
		else
		{
			pTeam->delete_apply_member(dwApplyID);
			dwError = E_Team_Leader_Not_Agree;
			
			if(VALID_POINT(pApplyRole))
			{
				NET_SIS_apply_join_team_reply send;
				send.dwApplyRoleID = dwApplyID;
				send.dwError = dwError;
				pApplyRole->SendMessage(&send, send.dw_size);	
			}
			return;
		}
	}

	NET_SIS_apply_join_team_reply send;
	send.dwApplyRoleID = dwApplyID;
	send.dwError = dwError;
	pSenderRole->SendMessage(&send, send.dw_size);
	
}
//-----------------------------------------------------------------------------------------------------
// 自己创建队伍
//-----------------------------------------------------------------------------------------------------
VOID GroupMgr::OnOwnCreateTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_own_create_team* p_receive = (NET_SIC_own_create_team*)pEventMessage;

	Role* pSenderRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pSenderRole))
		return;

	INT nRet = E_Success;
	Team* pTeam = NULL;

	// 检查自身是否有队
	if( INVALID_VALUE != pSenderRole->GetTeamID() )
	{
		nRet = E_Team_Target_Have_Team;
	}

	if(nRet == E_Success)
	{
		pTeam = CreateTeam(pSenderRole);

		pTeam->send_team_state(pSenderRole);

		NET_SIS_team_leader_set send1;
		send1.dwTeamID = pTeam->get_team_id();
		send1.bLeader = TRUE;
		send1.dwRoleID = pSenderRole->GetID();
		if(VALID_POINT(pSenderRole->get_map()))
			pSenderRole->get_map()->send_big_visible_tile_message(pSenderRole,&send1, send1.dw_size);
	}

	NET_SIS_own_create_team send;
	send.dwError = nRet;
	pSenderRole->SendMessage(&send, send.dw_size);
}

//----------------------------------------------------------------------------------------------------------
// 解散队伍
//----------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnDismissTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_dismiss_team* p_receive = (NET_SIC_dismiss_team*)pEventMessage;

	DWORD dw_error_code = E_Success;

	Role* pSenderRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pSenderRole))
		return;

	// 判断是否有队
	if(INVALID_VALUE == pSenderRole->GetTeamID())
		return;
	
	// 找到队伍
	Team* pTeam = m_mapTeam.find(pSenderRole->GetTeamID());
	if(!VALID_POINT(pTeam))
		return;
	
	//判断是否为队长
	if(!pTeam->is_leader(pSenderRole->GetID()))
	{
		dw_error_code = E_Team_Not_Leader;
	}

	if(dw_error_code == E_Success)
	{
		//向客户端发送消息
		NET_SIS_dismiss_team send;
		send.dwError = E_Success;
		pTeam->send_team_message(&send, send.dw_size);

		NET_SIS_team_leader_set send1;
		send1.dwTeamID = pTeam->get_team_id();
		send1.dwRoleID = pSenderRole->GetID();
		send1.bLeader = FALSE;
		if(VALID_POINT(pSenderRole->get_map()))
			pSenderRole->get_map()->send_big_visible_tile_message(pSenderRole,&send1, send1.dw_size);
		
		DeleteTeam(pTeam);
	}
	else
	{
		NET_SIS_dismiss_team send;
		send.dwError = dw_error_code;
		pSenderRole->SendMessage(&send, send.dw_size);	
	}
}
//---------------------------------------------------------------------------------------------------------
// 清空申请列表
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnCleanApply(DWORD dwSenderID,	LPVOID pEventMessage)
{
	NET_SIC_clean_apply* pCleanApply = (NET_SIC_clean_apply*)pEventMessage;

	Role* pSenderRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pSenderRole))
		return;

	Team* pTeam = m_mapTeam.find(pSenderRole->GetTeamID());
	if(!VALID_POINT(pTeam))
		return;

	pTeam->clear_apply_member();
}

//---------------------------------------------------------------------------------------------------------
// 队员邀请玩家加入小队
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnMemInviteJoinTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_member_invite_join_team* p_receive = (NET_SIC_member_invite_join_team*)pEventMessage;

	DWORD	dwSrcRoleID = dwSenderID;
	DWORD	dwDestRoleID = p_receive->dwDestRoleID;
	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);

	if( dwSrcRoleID == dwDestRoleID ) return;	// 邀请自己，外挂！
	
	Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
	if(!VALID_POINT(pTeam)) return;

	Role* pLeaderRole = pTeam->get_member(0);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);

	if(!VALID_POINT(pSrcRole) || !VALID_POINT(pDestRole) || !VALID_POINT(pLeaderRole))
		return;

	DWORD dw_error_code = E_Success;

	// 判断是否在某地图内允许邀请组队
	Map* pMap = pLeaderRole->get_map();
	if(VALID_POINT(pMap))
		dw_error_code = pMap->can_invite_join_team();

	//邀请者没有队伍
	if(INVALID_VALUE == pSrcRole->GetTeamID())
	{
		dw_error_code = E_Team_Map_NoHave_Team;
	}
	// 被邀请者有小队
	else if( INVALID_VALUE != pDestRole->GetTeamID() )
	{
		NET_SIS_Team_Msg send;
		send.byType = 0;
		s_role_info* p_role_info = g_roleMgr.get_role_info(pSrcRole->GetID( ));
		if(VALID_POINT(p_role_info))
			get_fast_code()->memory_copy(send.sz_role_name, p_role_info->sz_role_name, sizeof(send.sz_role_name));	
		pDestRole->SendMessage(&send, send.dw_size);
		dw_error_code = E_Team_Target_Have_Team;
	}

	// 查看被邀请者是不是正在被邀请
	else if( INVALID_VALUE != pDestRole->GetTeamInvite() )
	{
		dw_error_code = E_Team_Target_Busy;
	}

	// 判断队长
	else if( INVALID_VALUE != pLeaderRole->GetTeamID() )
	{	
		Team* pTeam = m_mapTeam.find(pLeaderRole->GetTeamID());
		if( !VALID_POINT(pTeam) ) return;

		// 是否为队长
		if( !pTeam->is_leader(pLeaderRole->GetID()) )
			dw_error_code = E_Team_Not_Leader;

		// 小队人数是否已满
		if( pTeam->get_member_number() >= MAX_TEAM_NUM )
			dw_error_code = E_Team_Member_Full;
	}

	// 如果判断成功，则发送邀请
	if( E_Success == dw_error_code )
	{
		NET_SIS_member_invite_join_team_to_leader send;
		send.dwDestRoleID	=	dwDestRoleID;
		send.dwTeamMemID	=	dwSenderID;
		pLeaderRole->SendMessage(&send, send.dw_size);
	}
	// 不成功，返回错误
	else
	{
		NET_SIS_member_invite_join_team send;
		send.dwError = dw_error_code;
		pSrcRole->SendMessage(&send, send.dw_size);
	}

}
//---------------------------------------------------------------------------------------------------------
// 队长回复队员邀请玩家加入小队    added by gtj
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnMemInviteJoinTeamReply(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_member_invite_join_team_replay* p_receive = (NET_SIC_member_invite_join_team_replay*)pEventMessage;

	if(p_receive->bAgree)
	{

		DWORD	dwSrcRoleID = dwSenderID;
		DWORD	dwDestRoleID = p_receive->dwDestRoleID;

		if( dwSrcRoleID == dwDestRoleID ) return;	// 邀请自己，外挂！

		Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
		Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);

		if( !VALID_POINT(pSrcRole) )
			return;

		DWORD dw_error_code = E_Success;

		// 判断是否在某地图内允许邀请组队
		Map* pMap = pSrcRole->get_map();
		if(VALID_POINT(pMap))
			dw_error_code = pMap->can_invite_join_team();

		// 被邀请人不在线
		if( !VALID_POINT(pDestRole) )
		{
			dw_error_code = E_Team_Target_Not_Online;
		}

		// 被邀请者有小队
		else if( INVALID_VALUE != pDestRole->GetTeamID() )
		{
			dw_error_code = E_Team_Target_Have_Team;
		}

		// 查看被邀请者是不是正在被邀请
		else if( INVALID_VALUE != pDestRole->GetTeamInvite() )
		{
			dw_error_code = E_Team_Target_Busy;
		}

		// 邀请者是不是有队伍
		else if( INVALID_VALUE != pSrcRole->GetTeamID() )
		{	
			Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
			if( !VALID_POINT(pTeam) ) return;

			// 是否为队长
			if( !pTeam->is_leader(pSrcRole->GetID()) )
				dw_error_code = E_Team_Not_Leader;

			// 小队人数是否已满
			if( pTeam->get_member_number() >= MAX_TEAM_NUM )
				dw_error_code = E_Team_Member_Full;
		}

		// 如果判断成功，则发送邀请
		if( E_Success == dw_error_code )
		{
			pDestRole->SetTeamInvite(pSrcRole->GetID());
			Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());

			if(VALID_POINT(pTeam))
			{
				// 发送队伍成员信息到被邀请人
				pTeam->send_team_member_info_to_invitee(pDestRole);

				NET_SIS_invite_join_team send;
				send.dwLeaderID		=	dwSrcRoleID;
				send.dwDestRoleID	=	dwDestRoleID;
				pDestRole->SendMessage(&send, send.dw_size);
			}
			
		}
		// 不成功，返回错误
		else
		{
			NET_SIS_invite_to_leader send;
			send.dw_error_code = dw_error_code;
			pSrcRole->SendMessage(&send, send.dw_size);
		}
	}
	// 队长不同意，则向队员发送消息
	else
	{
		Role* pTeamMem = g_roleMgr.get_role(p_receive->dwTeamMemID);
		if(VALID_POINT(pTeamMem))
		{
			NET_SIS_member_invite_join_team_reply send;
			send.dwDestRoleID = p_receive->dwDestRoleID;
			pTeamMem->SendMessage(&send, send.dw_size);
		}
	}
}

//---------------------------------------------------------------------------------------------------------
// 改变队伍公告    added by gtj  10-11-09
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnChangeTeamPlacard(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_change_team_placard* p_receive = (NET_SIC_change_team_placard*)pEventMessage;

	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pRole))	return;

	DWORD dwTeamID = pRole->GetTeamID();
	if(!VALID_VALUE(dwTeamID))	return;

	Team* pTeam = const_cast<Team*>(g_groupMgr.GetTeamPtr(dwTeamID));
	if(!VALID_POINT(pTeam))	return;

	// 计算大小
	INT32 nPlacardCnt = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_change_team_placard, szTeamPlacard)) / sizeof(TCHAR) - 1;

	// 进行判断处理
	DWORD dw_error_code = pTeam->change_team_placard(pRole->GetID(), p_receive->szTeamPlacard, nPlacardCnt);

	// 如果成功，则向队友发送改变消息
	if(dw_error_code == E_Success)
	{
		INT32 nMsgSz = sizeof(NET_SIS_change_team_placard) + nPlacardCnt * sizeof(TCHAR);
		CREATE_MSG(pSend, nMsgSz, NET_SIS_change_team_placard);

		pSend->dw_role_id	= pRole->GetID();
		memcpy(pSend->szTeamPlacard, p_receive->szTeamPlacard, (nPlacardCnt + 1) * sizeof(TCHAR));
		pSend->szTeamPlacard[nPlacardCnt] = _T('\0');

		pTeam->send_team_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}
	else
	{
		NET_SIS_change_team_placard_fail send;
		send.dw_error_code = dw_error_code;
		pRole->SendMessage(&send, send.dw_size);
	}


}

//--------------------------------------------------------------------------------------------------------
// 取得所在地图内所有小队    added by gtj  10-11-09
//--------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnGetMapTeamInfo(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_get_map_team_info* p_receive = (NET_SIC_get_map_team_info*)pEventMessage;
	
	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if(!VALID_POINT(pRole)) return;

	package_map<DWORD, Team*>	m_mapLocalTeam;

	Team* pTeam = NULL;
	package_map<DWORD, Team*>::map_iter it = m_mapTeam.begin();

	// 寻找所在地图内的小队
	while(m_mapTeam.find_next(it, pTeam))
	{
		if(VALID_POINT(pTeam) && VALID_POINT(pTeam->get_member(0)) && pTeam->get_member(0)->GetMapID() == pRole->GetMapID())
		{
			m_mapLocalTeam.add(pTeam->get_team_id(), pTeam);	
		}
	}


	INT16 n16Num = m_mapLocalTeam.size();
	// 没有队伍，则返回
	if(n16Num == 0)	return;

	// 申请内存
	INT32 nMsgSz = sizeof(NET_SIS_send_map_team_info) - sizeof(BYTE) + n16Num * sizeof(tagMapTeamInfo);
	CREATE_MSG(pSend, nMsgSz, NET_SIS_send_map_team_info);

	pSend->nTeamNum = 0;
	M_trans_pointer(pTeamInfo, pSend->byData, tagMapTeamInfo);

	Team* pLocalTeam = NULL;
	package_map<DWORD, Team*>::map_iter iter = m_mapLocalTeam.begin();
	while(m_mapLocalTeam.find_next(iter, pLocalTeam))
	{
		pTeamInfo[pSend->nTeamNum].LeaderID			= pLocalTeam->get_member(0)->GetID();
		pTeamInfo[pSend->nTeamNum].nLevel			= pLocalTeam->get_member(0)->get_level();
		pTeamInfo[pSend->nTeamNum].eClassEx			= pLocalTeam->get_member(0)->GetClass();
		pTeamInfo[pSend->nTeamNum].nTeamMemberNum	= pLocalTeam->get_member_number();
		//_tcscpy_s(pTeamInfo[pSend->nTeamNum].szTeamPlacard, pLocalTeam->get_team_placard().size() + 1, pLocalTeam->get_team_placard().c_str());
		//mwh 2010-7-27
		_tcsncpy(pTeamInfo[pSend->nTeamNum].szTeamPlacard, pLocalTeam->get_team_placard().c_str(), MAX_TEAM_PLACARD_LEN);

		++pSend->nTeamNum;
	}
	// 发送队伍信息
	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
	m_mapLocalTeam.clear();
}

//---------------------------------------------------------------------------------------------------------
// 邀请玩家加入小队
//---------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnInviteJoinTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_invite_join_team* p_receive = (NET_SIC_invite_join_team*)pEventMessage;

	DWORD	dwSrcRoleID = dwSenderID;
	DWORD	dwDestRoleID = p_receive->dwDestRoleID;

	if( dwSrcRoleID == dwDestRoleID ) return;	// 邀请自己，外挂！

	Role* pSrcRole	=	g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole	=	g_roleMgr.get_role(dwDestRoleID);

	if( !VALID_POINT(pSrcRole) )
		return;

	DWORD dw_error_code = E_Success;

	// 判断是否在某地图内允许邀请组队
	Map* pMap = pSrcRole->get_map();
	if(VALID_POINT(pMap))
		dw_error_code = pMap->can_invite_join_team();

	// 被邀请人不在线
	if( !VALID_POINT(pDestRole) )
	{
		dw_error_code = E_Team_Target_Not_Online;
	}

	// 被邀请者有小队
	else if( INVALID_VALUE != pDestRole->GetTeamID() )
	{
		NET_SIS_Team_Msg send;
		send.byType = 0;
		s_role_info* p_role_info = g_roleMgr.get_role_info(pSrcRole->GetID( ));
		if(VALID_POINT(p_role_info))
			get_fast_code()->memory_copy(send.sz_role_name, p_role_info->sz_role_name, sizeof(send.sz_role_name));
		pDestRole->SendMessage(&send, send.dw_size);
		dw_error_code = E_Team_Target_Have_Team;
	}

	// 查看被邀请者是不是正在被邀请
	else if( INVALID_VALUE != pDestRole->GetTeamInvite() )
	{
		dw_error_code = E_Team_Target_Busy;
	}

	// 邀请者是不是有队伍
	else if( INVALID_VALUE != pSrcRole->GetTeamID() )
	{	
		Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
		if( !VALID_POINT(pTeam) ) return;

		// 是否为队长
		if( !pTeam->is_leader(pSrcRole->GetID()) )
			dw_error_code = E_Team_Not_Leader;

		// 小队人数是否已满
		if( pTeam->get_member_number() >= MAX_TEAM_NUM )
			dw_error_code = E_Team_Member_Full;
	}

	// 被邀请者在切磋
	else if(pDestRole->IsDuel())
	{
		dw_error_code = E_Team_Target_InDuel;
	}

	// 如果判断成功，则发送邀请
	if( E_Success == dw_error_code )
	{
		if(pSrcRole->GetTeamID() == INVALID_VALUE)
		{
			pDestRole->SetTeamInvite(pSrcRole->GetID());

			NET_SIS_invite_join_team send;
			send.dwLeaderID		=	dwSrcRoleID;
			send.dwDestRoleID	=	dwDestRoleID;
			pDestRole->SendMessage(&send, send.dw_size);
		}
		else
		{
			Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
			pDestRole->SetTeamInvite(pSrcRole->GetID());

			// 发送队伍成员信息到被邀请人
			pTeam->send_team_member_info_to_invitee(pDestRole);

			NET_SIS_invite_join_team send;
			send.dwLeaderID		=	dwSrcRoleID;
			send.dwDestRoleID	=	dwDestRoleID;
			pDestRole->SendMessage(&send, send.dw_size);

		}
	}
	// 不成功，返回错误
	else
	{
		NET_SIS_invite_to_leader send;
		send.dw_error_code = dw_error_code;
		pSrcRole->SendMessage(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 玩家是否同意加入小队
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnInviteJoinTeamReply(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_invite_reply* p_receive = (NET_SIC_invite_reply*)pEventMessage;

	DWORD dwReplyRoleID = dwSenderID;

	Role* pReplyRole = g_roleMgr.get_role(dwReplyRoleID);
	if( !VALID_POINT(pReplyRole) ) return;

	DWORD dwInviteRoleID = pReplyRole->GetTeamInvite();
	if( INVALID_VALUE == dwInviteRoleID ) return;

	pReplyRole->SetTeamInvite(INVALID_VALUE);	// 重置被邀请者的邀请人ID

	Role* pSrcRole = g_roleMgr.get_role(dwInviteRoleID);
	if( !VALID_POINT(pSrcRole) ) return;

	// 判断是否在某地图内允许邀请组队
	Map* pMap = pSrcRole->get_map();
	if(VALID_POINT(pMap))
		if(pMap->can_invite_join_team())
			return;

	// 开始做判断
	INT nRet = E_Success;
	BOOL bNewGroup = FALSE;
	Team* pTeam = NULL;

	// 玩家同意加入
	if( p_receive->bAgree )
	{
		// 检查自身是否有队
		if( INVALID_VALUE != pReplyRole->GetTeamID() )
		{
			nRet = E_Team_Target_Have_Team;
		}
		// 无队
		else
		{
			// 检查邀请者是否有队
			if( INVALID_VALUE != pSrcRole->GetTeamID() )
			{
				pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
				if( !VALID_POINT(pTeam) ) return;

				nRet = pTeam->add_member(pSrcRole, pReplyRole);
			}
			// 邀请者无队
			else
			{
				bNewGroup = TRUE;
				pTeam = CreateTeam(pSrcRole, pReplyRole);

				NET_SIS_team_leader_set send1;
				send1.dwTeamID = pTeam->get_team_id();
				send1.bLeader = TRUE;
				send1.dwRoleID = pSrcRole->GetID();
				if(VALID_POINT(pSrcRole->get_map()))
					pSrcRole->get_map()->send_big_visible_tile_message(pSrcRole,&send1, send1.dw_size);
				
				NET_SIS_team_leader_set send2;
				send2.dwTeamID = pTeam->get_team_id();
				send2.bLeader = FALSE;
				send2.dwRoleID = pReplyRole->GetID();
				if(VALID_POINT(pReplyRole->get_map()))
					pReplyRole->get_map()->send_big_visible_tile_message(pReplyRole,&send2, send2.dw_size);


			}
		}
	}

	// 如果对方同意，并且判断成功，则发送全体队伍消息
	if( E_Success == nRet && p_receive->bAgree && VALID_POINT(pTeam) )
	{
		// 发送给所有玩家邀请成功
		NET_SIS_invite_reply send;
		send.bAgree = p_receive->bAgree;
		send.dw_error_code = nRet;
		pTeam->export_all_member_id(send.dwTeamMemID);
		pTeam->send_team_message(&send, send.dw_size);

		// 发送队员初始消息
		pTeam->send_role_init_state_to_team(pReplyRole);

		// 发送给新加入的玩家
		pTeam->send_team_state(pReplyRole);

		// 如果新创建队伍，则设置队长的队伍信息
		if( bNewGroup ) pTeam->send_team_state(pSrcRole);
	}
	// 如果不同意，或者判断失败，则只发送给双方
	else
	{
		NET_SIS_invite_reply send;
		send.dwTeamMemID[0] = dwInviteRoleID;
		send.dwTeamMemID[1] = dwReplyRoleID;
		send.bAgree = p_receive->bAgree;
		send.dw_error_code = nRet;

		pSrcRole->SendMessage(&send, send.dw_size);
		pReplyRole->SendMessage(&send, send.dw_size);
	}
}


//-------------------------------------------------------------------------------------------------------
// 踢玩家出小队
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnLeaderKickMember(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_kick_member* p_receive = (NET_SIC_kick_member*)pEventMessage;

	DWORD	dwSrcRoleID = p_receive->dwSrcRoleID;
	DWORD	dwDestRoleID = p_receive->dwDestRoleID;

	if( dwSrcRoleID == dwDestRoleID ) return;	// 不能踢出自己

	Role* pSrcRole = g_roleMgr.get_role(dwSrcRoleID);
	Role* pDestRole = g_roleMgr.get_role(dwDestRoleID);

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pSrcRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	//如果被踢的人在线
	if( VALID_POINT(pDestRole) && VALID_POINT(pSrcRole) )
	{
		INT nRet = pTeam->kick_member(pSrcRole, pDestRole);

		if( E_Success == nRet )
		{
			// 如果踢人成功，发送所有玩家消息
			NET_SIS_kick_member send;
			send.dwDestRoleID = dwDestRoleID;
			send.dw_error_code = nRet;
			pTeam->send_team_message(&send, send.dw_size);
			pDestRole->SendMessage(&send, send.dw_size);

			// 如果队伍需要删除，则删除队伍
			if( pTeam->is_need_delete() )
			{
				DeleteTeam(pTeam);
			}
		}
		else
		{
			// 如果不成功，返回踢人者
			NET_SIS_kick_member send;
			send.dwDestRoleID = dwDestRoleID;
			send.dw_error_code = nRet;
			pSrcRole->SendMessage(&send, send.dw_size);
		}
	}

	//如果被踢的人不在线
	if(!VALID_POINT(pDestRole) && VALID_POINT(pSrcRole))
	{
		INT nRet = pTeam->kick_member(pSrcRole, dwDestRoleID);

		if( E_Success == nRet )
		{
			// 如果踢人成功，发送所有玩家消息
			NET_SIS_kick_member send;
			send.dwDestRoleID = dwDestRoleID;
			send.dw_error_code = nRet;
			pTeam->send_team_message(&send, send.dw_size);

			// 如果队伍需要删除，则删除队伍
			if( pTeam->is_need_delete() )
			{
				DeleteTeam(pTeam);
			}
		}
		else
		{
			// 如果不成功，返回踢人者
			NET_SIS_kick_member send;
			send.dwDestRoleID = dwDestRoleID;
			send.dw_error_code = nRet;
			pSrcRole->SendMessage(&send, send.dw_size);
		}
	}
		
}

//-------------------------------------------------------------------------------------------------------
// 玩家离开小队
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnMemberLeaveTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	Role* pMember = g_roleMgr.get_role(dwSenderID);
	if( !VALID_POINT(pMember) ) return;

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pMember->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	INT nRet =E_Success; 

	nRet = pTeam->leave_team(pMember);

	if( E_Success == nRet )
	{
		// 成功，发送所有队伍成员消息
		NET_SIS_leave_team send;
		send.bLeaveLine = FALSE;
		send.dw_role_id = pMember->GetID();
		send.dwLeaderID = pTeam->get_member_id(0);
		send.dw_error_code = nRet;
		pTeam->send_team_message(&send, send.dw_size);
		pMember->SendMessage(&send, send.dw_size);

		// 如果队伍要删除，则删除队伍
		if( pTeam->is_need_delete() )
		{
			DeleteTeam(pTeam);
		}
	}
	else
	{
		// 如果不成功，返回离开者
		NET_SIS_leave_team send;
		send.bLeaveLine = FALSE;
		send.dw_role_id = pMember->GetID();
		send.dwLeaderID = pTeam->get_member_id(0);
		send.dw_error_code = nRet;
		pMember->SendMessage(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 设置小队拾取模式
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnSetPickMode(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_set_pick_mode* p_receive = (NET_SIC_set_pick_mode*)pEventMessage;

	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if( !VALID_POINT(pRole) ) return;

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	INT nRet = pTeam->set_pick_mode(pRole, p_receive->ePickMode);

	if( E_Success == nRet )
	{
		// 设置成功，发送所有队员消息
		NET_SIS_set_pick_mode send;
		send.ePickMode = p_receive->ePickMode;
		send.dw_error_code = nRet;
		pTeam->send_team_message(&send, send.dw_size);
	}
	else
	{
		// 设置不成功，发送设置者消息
		NET_SIS_set_pick_mode send;
		send.ePickMode = p_receive->ePickMode;
		send.dw_error_code = nRet;
		pRole->SendMessage(&send, send.dw_size);

	}
}

//-------------------------------------------------------------------------------------------------------
// 改变小队队长
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnChangeLeader(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIC_change_leader* p_receive = (NET_SIC_change_leader*)pEventMessage;

	Role* pSrc = g_roleMgr.get_role(dwSenderID);
	Role* pDest = g_roleMgr.get_role(p_receive->dwNewLeader);

	if( !VALID_POINT(pSrc) || !VALID_POINT(pDest) ) return;

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pSrc->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	INT nRet = E_Success;
	
	nRet = pTeam->change_leader(pSrc, pDest);

	if( E_Success == nRet )
	{
		// 成功，发送队伍消息
		NET_SIS_change_leader send;
		send.dwLeaderID = pSrc->GetID();
		send.dwNewLeaderID = pDest->GetID();
		send.dw_error_code = nRet;
		pTeam->send_team_message(&send, send.dw_size);

		NET_SIS_team_leader_set send1;
		send1.dwTeamID = pTeam->get_team_id();
		send1.bLeader = FALSE;
		send1.dwRoleID = pSrc->GetID();
		if(VALID_POINT(pSrc->get_map()))
			pSrc->get_map()->send_big_visible_tile_message(pSrc,&send1, send1.dw_size);

		send1.bLeader = TRUE;
		send1.dwRoleID = pDest->GetID();
		if(VALID_POINT(pDest->get_map()))
			pDest->get_map()->send_big_visible_tile_message(pDest,&send1, send1.dw_size);
	}
	else
	{
		// 不成功，返回交换者
		NET_SIS_change_leader send;
		send.dwLeaderID = pSrc->GetID();
		send.dwNewLeaderID = pDest->GetID();
		send.dw_error_code = nRet;
		pSrc->SendMessage(&send, send.dw_size);
	}
}

//-------------------------------------------------------------------------------------------------------
// 发送消息给整个小队
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::SendTeamMessage(DWORD dwTeamID, LPVOID p_message, DWORD dw_size)
{
	Team* pTeam = m_mapTeam.find(dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	pTeam->send_team_message(p_message, dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 发送消息给队友
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::SendTeamateMessage(DWORD dwTeamID, DWORD dwRole, LPVOID p_message, DWORD dw_size)
{
	Team* pTeam = m_mapTeam.find(dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	pTeam->send_teamate_message(dwRole, p_message, dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 给可视链表外的小队成员发消息
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::SendTeamMesOutBigVis(Role* pRole, DWORD dwTeamID, LPVOID p_message, DWORD dw_size)
{
	if( !VALID_POINT(pRole) ) return;

	Team* pTeam = m_mapTeam.find(dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	pTeam->send_team_msg_out_big_view(pRole, p_message, dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 通知队友进入副本
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::SendTeamInstanceNotify(Role* pRole, DWORD dwTeamID, LPVOID p_message, DWORD dw_size)
{
	if( !VALID_POINT(pRole) ) return;

	Team* pTeam = m_mapTeam.find(dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	pTeam->send_team_instance_notice(pRole, p_message, dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 队员下线
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::RoleOutline(Role* pRole)
{
	if( !VALID_POINT(pRole) ) return;

	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	INT nRet = pTeam->leave_team(pRole, TRUE);

	if( E_Success == nRet )
	{
		// 成功，发送所有队伍成员消息
		NET_SIS_leave_team send;
		send.bLeaveLine = TRUE;
		send.dw_role_id = pRole->GetID();
		send.dwLeaderID = pTeam->get_member_id(0);
		send.dw_error_code = nRet;
		pTeam->send_team_message(&send, send.dw_size);

		// 如果队伍要删除，则删除队伍
		if( pTeam->is_need_delete() )
		{
			DeleteTeam(pTeam);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// 小队成员等级改变
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnChangeLevel(DWORD dwSenderID, LPVOID pEventMessage)
{
	NET_SIS_change_role_level* p_receive = (NET_SIS_change_role_level*)pEventMessage;
	
	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if( !VALID_POINT(pRole) ) return;

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	// 同步队伍等级
	pTeam->member_level_change(pRole);

	// 同步给小队玩家
	pTeam->send_team_msg_out_big_view(pRole, p_receive, p_receive->dw_size);	

	// 修改队员数据
	pTeam->change_member_datas(pRole);
}

//-------------------------------------------------------------------------------------------------------
// 小队队长统御力改变
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnChangeLeaderRein(DWORD dwSenderID, LPVOID pEventMessage)
{
	tagLeaderRein* pLeaderRein = (tagLeaderRein*)pEventMessage;

	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if( !VALID_POINT(pRole) ) return;

	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	if( pTeam->is_leader(pRole->GetID()) ) return;

	pTeam->cal_exp_factor();
}
//--------------------------------------------------------------------
// 通知物品分配或者掷骰子
//--------------------------------------------------------------------
VOID GroupMgr::OnNoticeAssign(DWORD dwSenderID, LPVOID p_message)
{
#define FILLVIEWINFO(info, p_equip)\
	(info).dw_data_id = (p_equip)->dw_data_id;\
	(info).byBind = (p_equip)->byBind;\
	(info).byConsolidateLevel = (p_equip)->equipSpec.byConsolidateLevel;\
	(info).nUseTimes = (p_equip)->nUseTimes;\
	(info).byHoldNum = (p_equip)->equipSpec.byHoleNum;\
	memcpy((info).EquipAttitionalAtt, (p_equip)->equipSpec.EquipAttitionalAtt, sizeof( (p_equip)->equipSpec.EquipAttitionalAtt));\
	memcpy((info).dwHoleGemID, (p_equip)->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);\

	tagAssignNotice* p = (tagAssignNotice*)p_message;
	
	Role* pRole = g_roleMgr.get_role(dwSenderID);
	if( !VALID_POINT(pRole) ) return;

	if( p->ePickMode == EPUM_Leader)
	{
		Team* pTeam = m_mapTeam.find(p->dwTeamID);
		if( !VALID_POINT(pTeam) ) return;

		Role* pLeader = pTeam->get_member(0);
		if( !VALID_POINT(pLeader)) return;

		Map* pMap = pLeader->get_map( );
		if( !VALID_POINT(pMap) ) return;

		tag_ground_item* pGroundItem = pMap->get_ground_item(p->n64GroundID );
		if( !VALID_POINT(pGroundItem) || !pGroundItem->is_notice( ) ) return;
		
		pGroundItem->finist_umpirage( );
		
		NET_SIS_team_leader_assign send;
		send.n64GroundID = p->n64GroundID;
		send.dw_data_id = pGroundItem->dw_type_id;
		if(VALID_POINT(pGroundItem->p_item->pProtoType))
			send.byQuality = pGroundItem->p_item->pProtoType->byQuality;

		if(VALID_POINT(pGroundItem->p_item) &&
			pGroundItem->dw_type_id != TYPE_ID_MONEY && 
			MIsEquipment(pGroundItem->p_item->dw_data_id) )
		{// 必须判断p_item ,掉落金钱p_item 为空
			send.byQuality = ((tagEquip*)pGroundItem->p_item)->GetQuality( );
			FILLVIEWINFO(send.stEquipInfo, ((tagEquip*)pGroundItem->p_item));
		}else send.dw_size -= sizeof(send.stEquipInfo);
		pLeader->SendMessage(&send,send.dw_size);
		return;
	}
	
	if( p->ePickMode == EPUM_Sice)
	{
		Team* pTeam = m_mapTeam.find(p->dwTeamID);
		if( !VALID_POINT(pTeam) ) return;

		Map* pMap = pRole->get_map( );
		if( !VALID_POINT(pMap) ) return;

		tag_ground_item* pGroundItem = pMap->get_ground_item(p->n64GroundID );
		if( !VALID_POINT(pGroundItem) || !pGroundItem->is_notice( ) ) return;

		pGroundItem->finist_umpirage( );

		NET_SIS_team_assign_sicefor send;
		send.dw_data_id = pGroundItem->dw_type_id;
		send.n64GroundID = p->n64GroundID;
		if(VALID_POINT(pGroundItem->p_item->pProtoType))
			send.byQuality = pGroundItem->p_item->pProtoType->byQuality;

		if(VALID_POINT(pGroundItem->p_item) &&
			pGroundItem->dw_type_id != TYPE_ID_MONEY && 
			MIsEquipment(pGroundItem->p_item->dw_data_id) )
		{// 必须判断p_item ,掉落金钱p_item 为空
			send.byQuality = ((tagEquip*)pGroundItem->p_item)->GetQuality( );
			FILLVIEWINFO(send.stEquipInfo, ((tagEquip*)pGroundItem->p_item));
		}else send.dw_size -= sizeof(send.stEquipInfo);
		INT nSiceNumber = pTeam->send_team_msg_in_big_view( pRole, &send, send.dw_size );
		if( nSiceNumber ) pGroundItem->get_sice_data( ).SetSiceTotal( nSiceNumber );
		return;
	}
}
//--------------------------------------------------------------------
// 掷骰子
//--------------------------------------------------------------------
VOID GroupMgr::OnAssignSice( DWORD dwSenderID, LPVOID p_message )
{
	NET_SIC_team_assign_sice* p = (NET_SIC_team_assign_sice*)p_message;

	//if(  p->bySiceType == EPSM_Null ) return;

	Role* pRole = g_roleMgr.get_role( dwSenderID );
	if( !VALID_POINT(pRole) ) return;

	Map* pMap = pRole->get_map( );
	if( !VALID_POINT(pMap) ) return;

	tag_ground_item* pGroundItem = pMap->get_ground_item( p->n64GroundID );
	if( !VALID_POINT(pGroundItem) || !VALID_POINT(pGroundItem->p_item) || 
		!VALID_POINT(pGroundItem->p_item->pProtoType) || pGroundItem->e_pick_mode != EPUM_Sice)
	{
		NET_SIS_team_assign_error send;
		send.dwError = E_Team_Sice_GroundItemError;
		pRole->SendMessage(&send,send.dw_size);
		return;
	}

	BOOL bIsClassNotFit = FALSE;
	BYTE byScore =  p->bySiceType == EPSM_Null ? 0 : get_tool()->rand_in_range(1,100);
	if( p->bySiceType == EPSM_Require && 
		MIsEquipment(pGroundItem->p_item->pProtoType->dw_data_id))
	{
		EClassType eClass = pRole->GetClass( );
		DWORD dwLimitWear = ((tagEquipProto*)(pGroundItem->p_item->pEquipProto))->dwVocationLimitWear;
		if( !(dwLimitWear & (1<<eClass)) )
		{
			NET_SIS_team_assign_error send;
			send.dwError = E_Team_Sice_Cant_RequireSice;
			pRole->SendMessage(&send,send.dw_size);
			bIsClassNotFit = TRUE;
			byScore = 0;
		}

		pGroundItem->get_sice_data( ).AddRequireSice( dwSenderID, byScore );
	}		
	else
		pGroundItem->get_sice_data( ).AddGreedSice( dwSenderID, byScore );

	if( pGroundItem->get_sice_data( ).IsSiceFinishi( ) )
	{
		tagAssignFinish send;
		send.dwTeamID = pRole->GetTeamID( );
		send.n64GroundID = p->n64GroundID;
		OnSiceFinish( pRole->GetID( ), &send);
	}
}
//--------------------------------------------------------------------
// 投票结束，发送物品
//--------------------------------------------------------------------
VOID GroupMgr::OnSiceFinish( DWORD dwSender, LPVOID p_message )
{
	tagAssignFinish* p = (tagAssignFinish*)p_message;

	Team* pTeam = m_mapTeam.find(p->dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	Role* pRole = g_roleMgr.get_role(dwSender);;
	if( !VALID_POINT(pRole) ) pRole = pTeam->get_member(0);
	if( !VALID_POINT(pRole) ) return;

	Map* pMap = pRole->get_map( );
	if( !VALID_POINT(pMap) ) return;

	tag_ground_item* pGroundItem = pMap->get_ground_item(p->n64GroundID );
	if( !VALID_POINT(pGroundItem) ) return;


	{//发送所有人投票
		tagSiceData& SiceData = pGroundItem->get_sice_data( );
		NET_SIS_team_assign_sice_result send;
		send.n64GroundID = pGroundItem->n64_id;
		send.byNumber = SiceData.byNumber;
		for( UINT n = 0; n < SiceData.byNumber; ++n)
		{
			send.stResult[n].dw_role_id = SiceData.dw_role_id[n];
			if( SiceData.byRequireSice[n] )
			{
				send.stResult[n].byScore = SiceData.byRequireSice[n];
				send.stResult[n].byType = EPSM_Require;
			}
			else
			{
				send.stResult[n].byScore = SiceData.byGreedSice[n];
				send.stResult[n].byType = EPSM_Greed;
			}
		}
		send.dw_size +=(send.byNumber - MAX_TEAM_NUM) * sizeof(tagSiceResult);
		pTeam->send_team_msg_in_same_map( pMap->get_map_id( ), &send, send.dw_size );
	}

	BYTE byMaxRequire  = 0;
	DWORD dwRewardID = pGroundItem->get_sice_data( ).GetRequireMax( byMaxRequire );
	if( !VALID_POINT(dwRewardID) )dwRewardID = pGroundItem->get_sice_data( ).GetGreedMax( );

	if( VALID_POINT(dwRewardID) )
	{
		Role* pReWarder =  g_roleMgr.get_role(dwRewardID);
		if( VALID_POINT( pReWarder) && VALID_POINT(pReWarder->get_map( ) ) )
		{
			{//通知某人通过贪婪(/需求)掷骰获得该物品
				NET_SIS_team_member_obtain_ground_item send;
				send.dw_role_id = pReWarder->GetID( );
				send.n64GroundID = p->n64GroundID;
				send.byScore = pGroundItem->get_score( pReWarder->GetID( ), send.byType );
				pTeam->send_team_msg_in_same_map( pMap->get_map_id( ), &send, send.dw_size );
			}


			DWORD dwRet = pReWarder->GetItemMgr().Add2Bag(pGroundItem->p_item, elcid_pickup_item, TRUE);
			if( E_Success == dwRet )
			{
				if( HearSayHelper::IsItemBroadcast(pGroundItem->get_boss_id(), pGroundItem->p_item))
				{
					HearSayHelper::SendMessage(EHST_KILLBOSSGETITEM, dwRewardID, pGroundItem->p_item->dw_data_id,
						pGroundItem->get_boss_id(), INVALID_VALUE, INVALID_VALUE, pGroundItem->p_item);
				}

				//从Map中删除掉落物品
				//将掉落物品从地面删除
				pMap->remove_ground_item(pGroundItem);

				//将指针恢复为NULL
				pGroundItem->p_item=NULL;
				SAFE_DELETE(pGroundItem);
			}else{//不能直接加入背包，掉落地上并提示
				pGroundItem->dw_owner_id = dwRewardID;
				
				NET_SIS_team_assign_error send;
				send.dwError = E_Team_Assign_Your_Baggage_Not_Enough;
				pReWarder->SendMessage(&send,send.dw_size);
			}

			pReWarder->GetAchievementMgr().UpdateAchievementCriteria(eta_win_sice, 1);
			if (byMaxRequire == 100)
			{
				pReWarder->GetAchievementMgr().UpdateAchievementCriteria(eta_win_sice_yibai, 1);
			}
		}
	}else{//将变成自由拾取	
		pGroundItem->dw_team_id = INVALID_VALUE;
		pGroundItem->dw_owner_id = INVALID_VALUE;
	}
}
//--------------------------------------------------------------------
// 队长分配
//--------------------------------------------------------------------
VOID GroupMgr::OnLeaderAssign( DWORD dwSender, LPVOID p_message )
{
	NET_SIC_team_lead_assign* p = (NET_SIC_team_lead_assign*)p_message;

	Role* pRole = g_roleMgr.get_role( dwSender );
	if( !VALID_POINT(pRole) ) return;

	Map* pMap = pRole->get_map( );
	if( !VALID_POINT(pMap) ) return;

	tag_ground_item* pGroundItem = pMap->get_ground_item( p->n64GroundID );
	if( !VALID_POINT(pGroundItem) ) return;

	Role* pReWarder =  g_roleMgr.get_role(p->dwMember);
	if( VALID_POINT( pReWarder) && VALID_POINT(pReWarder->get_map( )) )
	{

		DWORD dwRet = pReWarder->GetItemMgr().Add2Bag(pGroundItem->p_item, elcid_pickup_item, TRUE);
		if( E_Success == dwRet )
		{
			if( HearSayHelper::IsItemBroadcast(pGroundItem->get_boss_id(), pGroundItem->p_item))
			{
				HearSayHelper::SendMessage(EHST_KILLBOSSGETITEM, p->dwMember, pGroundItem->p_item->dw_data_id,
					pGroundItem->get_boss_id(), INVALID_VALUE, INVALID_VALUE, pGroundItem->p_item);
			}

			//从Map中删除掉落物品
			//将掉落物品从地面删除
			pMap->remove_ground_item(pGroundItem);

			//将指针恢复为NULL
			pGroundItem->p_item=NULL;
			SAFE_DELETE(pGroundItem);
		}else{//不能直接加入背包，掉落地上并提示
			pGroundItem->dw_owner_id = p->dwMember;

			NET_SIS_team_assign_error send;
			send.dwError = E_Team_Assign_Your_Baggage_Not_Enough;
			pReWarder->SendMessage(&send,send.dw_size);
		}
	}else{//将变成自由拾取	
		pGroundItem->dw_team_id = INVALID_VALUE;
		pGroundItem->dw_owner_id = INVALID_VALUE;
	}
}
//--------------------------------------------------------------------
// 设置物品分配等级
//--------------------------------------------------------------------
VOID GroupMgr::OnSetAssignQuality( DWORD dwSender, LPVOID p_message )
{
	NET_SIC_set_assign_quality* p_receive = (NET_SIC_set_assign_quality*)p_message;

	Role* pRole = g_roleMgr.get_role(dwSender);
	if( !VALID_POINT(pRole) ) return;

	// 找到队伍
	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if( !VALID_POINT(pTeam) ) return;

	INT nRet = pTeam->set_assign_quality(pRole, (EItemQuality)p_receive->byQuality);

	if( E_Success == nRet )
	{
		// 设置成功，发送所有队员消息
		NET_SIS_set_assign_quality send;
		send.byQuality = p_receive->byQuality;
		send.dwError = nRet;
		pTeam->send_team_message(&send, send.dw_size);
	}
	else
	{
		// 设置不成功，发送设置者消息
		NET_SIS_set_assign_quality send;
		send.byQuality = p_receive->byQuality;
		send.dwError = nRet;
		pRole->SendMessage(&send, send.dw_size);
	}
}
//-------------------------------------------------------------------------------------------------------
// 给服务器所有在线玩家加一个buff
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnAddAllRoleBuff(DWORD dwSenderID, LPVOID pEventMessage)
{
	tagAllRoleBuff* pAllRoleBuff = (tagAllRoleBuff*)pEventMessage;

	g_roleMgr.add_buff_to_all(pAllRoleBuff->dwBuffTypeID);
}

//-------------------------------------------------------------------------------------------------------
// 创建一个小队
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnCreateTeam(DWORD dwSenderID, LPVOID pEventMessage)
{
	tagCreateTeam* pCreateTeam = (tagCreateTeam*)pEventMessage;
	DWORD	dwSrcRoleID = pCreateTeam->dwSrcRoleID;
	DWORD	dwDesRoleID = pCreateTeam->dwDesRoleID;

	Role* pSrcRole = g_roleMgr.get_role(dwSrcRoleID);
	if( !VALID_POINT(pSrcRole) ) return;

	Role* pDesRole = g_roleMgr.get_role(dwDesRoleID);
	if( !VALID_POINT(pDesRole) ) return;

	// 检查是否有队
	if( INVALID_VALUE != pSrcRole->GetTeamID() || INVALID_VALUE != pDesRole->GetTeamID() )
	{
		return;
	}

	Team* pTeam = CreateTeam(pSrcRole, pDesRole);

	// 如果对方同意，并且判断成功，则发送全体队伍消息
	if( VALID_POINT(pTeam) )
	{
		// 发送给所有玩家邀请成功
		NET_SIS_invite_reply send;
		send.bAgree = TRUE;
		send.dw_error_code = E_Success;
		pTeam->export_all_member_id(send.dwTeamMemID);

		pTeam->send_team_message(&send, send.dw_size);

		// 发送队员初始消息
		pTeam->send_role_init_state_to_team(pDesRole);

		// 发送给新加入的玩家
		pTeam->send_team_state(pDesRole);

		// 如果新创建队伍，则设置队长的队伍信息
		pTeam->send_team_state(pSrcRole);

		NET_SIS_team_leader_set send1;
		send1.dwTeamID = pTeam->get_team_id();
		send1.bLeader = TRUE;
		send1.dwRoleID = pSrcRole->GetID();
		if(VALID_POINT(pSrcRole->get_map()))
			pSrcRole->get_map()->send_big_visible_tile_message(pSrcRole,&send1, send1.dw_size);
		
		NET_SIS_team_leader_set send2;
		send2.dwTeamID = pTeam->get_team_id();
		send2.bLeader = FALSE;
		send2.dwRoleID = pDesRole->GetID();
		if(VALID_POINT(pDesRole->get_map()))
			pDesRole->get_map()->send_big_visible_tile_message(pDesRole,&send2, send2.dw_size);

	}
}

//-------------------------------------------------------------------------------------------------------
// 增加一个小队队员
//-------------------------------------------------------------------------------------------------------
VOID GroupMgr::OnAddMember(DWORD dwSenderID, LPVOID pEventMessage)
{
	tagAddTeamMember* pCreateTeam = (tagAddTeamMember*)pEventMessage;
	DWORD	dwTeamID = pCreateTeam->dwTeamID;
	DWORD	dwDesRoleID = pCreateTeam->dwDesRoleID;

	Role* pDesRole = g_roleMgr.get_role(dwDesRoleID);
	if( !VALID_POINT(pDesRole) ) return;

	Team* pTeam = m_mapTeam.find(dwTeamID);
	if( !VALID_POINT(pTeam) ) return;

	Role* pSrcRole =  pTeam->get_member(0);
	if( !VALID_POINT(pSrcRole) ) return;

	pTeam->add_member(pSrcRole, pDesRole);

	// 发送给所有玩家邀请成功
	NET_SIS_invite_reply send;
	send.bAgree = TRUE;
	send.dw_error_code = E_Success;
	pTeam->export_all_member_id(send.dwTeamMemID);

	pTeam->send_team_message(&send, send.dw_size);

	// 发送队员初始消息
	pTeam->send_role_init_state_to_team(pDesRole);

	// 发送给新加入的玩家
	pTeam->send_team_state(pDesRole);
}
//mwh 2011-09-06
VOID GroupMgr::OnSetShareLeaderCircleQuest( DWORD dwSender, LPVOID p_message)
{
	Role* pRole = g_roleMgr.get_role(dwSender);
	if( !VALID_POINT(pRole) ) return;

	Team* pTeam = m_mapTeam.find(pRole->GetTeamID());
	if(!VALID_POINT(pTeam) ) return;
	if(!pTeam->is_leader(dwSender)) return;

	NET_SIC_team_leader_set_share_circle_quest* p = (NET_SIC_team_leader_set_share_circle_quest*) p_message;
	NET_SIS_team_set_share_leader_circle_quest send;
	if(!sTeamShareMgr.CanActive(pTeam->get_team_id()))
	{
		send.dwError = ETSQ_MemberQuest_InProcess;
	}else{
		pTeam->leader_set_share_circle_quest(p->bFlag);
		if(pTeam->get_leader_share_circle_quest())
			sTeamShareMgr.AddNew(pTeam->get_team_id());
		send.dwError = E_Success;
	}
	send.bFlag = pTeam->get_leader_share_circle_quest();

	pRole->SendMessage(&send, send.dw_size);

	if(send.dwError == E_Success)
	{
		sTeamShareMgr.MemberNext(INVALID_VALUE, dwSender, TRUE);
		pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_SetShareLeaderCircleQuest, 1);
	}
}
GroupMgr g_groupMgr;
