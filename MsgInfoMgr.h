/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//脚本消息流管理

#pragma once
#include "../../common/WorldDefine/ScriptMsgInfo.h"
#include "mutex.h"

class Role;
class Team;

class MsgInfoMgr
{
public:
	MsgInfoMgr():m_dMsgInfoID(1) {}
	~MsgInfoMgr();

	BOOL	Init() { return TRUE; }

	DWORD	BeginMsgEvent();
	VOID	AddMsgEvent(DWORD dwMsgInfoID, EMsgUnitType eMsgUnitType, LPVOID pData);
	VOID	DispatchRoleMsgEvent(DWORD dwMsgInfoID, Role *pRole);
	VOID	DispatchWorldMsgEvent(DWORD dwMsgInfoID);
	VOID	DispatchMapMsgEvent(DWORD dwMsgInfoID, Map* pMap);
	VOID	DispatchTeamMapMsgEvent(DWORD dwMsgInfoID, Map *pMap, Team* pTeam);

private:
	VOID	RemoveMsgInfo(MsgInfo* pInfo)	{ m_mapMsgInfo.erase(pInfo->GetMsgID()); SAFE_DELETE(pInfo); }

private:
	package_safe_map<DWORD, MsgInfo*>	m_mapMsgInfo;
	DWORD					m_dMsgInfoID;
	Mutex					m_Lock;	
};

extern MsgInfoMgr g_MsgInfoMgr;