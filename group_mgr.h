/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/


#pragma once
#include "event_mgr.h"
#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/ItemDefine.h"

class Team;

class GroupMgr : public EventMgr<GroupMgr>
{
public:
	//---------------------------------------------------------------------------------------------------
	// CONSTRUCT
	//---------------------------------------------------------------------------------------------------
	~GroupMgr();

	//---------------------------------------------------------------------------------------------------
	// 初始化、更新及销毁
	//---------------------------------------------------------------------------------------------------
	BOOL	Init();
	VOID	Update();
	VOID	Destroy();

	//---------------------------------------------------------------------------------------------------
	// 小队相关
	//---------------------------------------------------------------------------------------------------
	VOID	OnOwnCreateTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnDismissTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID    OnApplyJoinTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnApplyJoinReply(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnCleanApply(DWORD dwSenderID,	LPVOID pEventMessage);
	VOID	OnMemInviteJoinTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnMemInviteJoinTeamReply(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnChangeTeamPlacard(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnGetMapTeamInfo(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnInviteJoinTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnInviteJoinTeamReply(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnLeaderKickMember(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnMemberLeaveTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnSetPickMode(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnChangeLeader(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnChangeLevel(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnChangeLeaderRein(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnNoticeAssign( DWORD dwSenderID, LPVOID p_message);
	VOID	OnAssignSice( DWORD dwSenderID, LPVOID p_message );
	VOID	OnSiceFinish( DWORD dwSender, LPVOID p_message );
	VOID	OnLeaderAssign( DWORD dwSender, LPVOID p_message );
	VOID	OnSetAssignQuality( DWORD dwSender, LPVOID p_message );
	VOID	OnSetShareLeaderCircleQuest( DWORD dwSender, LPVOID p_message );

	VOID    SendTeamInstanceNotify(Role* pRole, DWORD dwTeamID, LPVOID p_message, DWORD dw_size);
	VOID	SendTeamMessage(DWORD dwTeamID, LPVOID p_message, DWORD dw_size);
	VOID	SendTeamMesOutBigVis(Role *pRole, DWORD dwTeamID, LPVOID p_message, DWORD dw_size);
	VOID	SendTeamateMessage(DWORD dwTeamID, DWORD dwRole, LPVOID p_message, DWORD dw_size);
	VOID	RoleOutline(Role* pRole);

	const Team* GetTeamPtr(DWORD dwTeamID)	{ return m_mapTeam.find(dwTeamID); }
	Team* GetTeamPtrEx(DWORD dwTeamID)	{ return m_mapTeam.find(dwTeamID); }
	//---------------------------------------------------------------------------------------------------
	// 给服务器所有在线玩家加一个buff
	//---------------------------------------------------------------------------------------------------
	VOID	OnAddAllRoleBuff(DWORD dwSenderID, LPVOID pEventMessage);

	//---------------------------------------------------------------------------------------------------
	// 脚本触发小队相关事件
	//---------------------------------------------------------------------------------------------------
	VOID	OnCreateTeam(DWORD dwSenderID, LPVOID pEventMessage);
	VOID	OnAddMember(DWORD dwSenderID, LPVOID pEventMessage);

private:
	static VOID	RegisterGroupEventFunc();
	VOID		UpdateTeamatePos();

	Team*		CreateTeam(Role* pInviter, Role* pReplyer);
	Team*		CreateTeam(Role* pCreateRole);
	VOID		DeleteTeam(Team* pTeam);

private:
	package_map<DWORD, Team*>			m_mapTeam;
	DWORD						m_dwTeamIndex;			// 小队ID索引
};

extern GroupMgr g_groupMgr;