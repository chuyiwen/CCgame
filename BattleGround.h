
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


#include "../../common/WorldDefine/BattleGround.h"


class map_instance_battle;

// 战场类
class BattleGround : public singleton<BattleGround>
{
public:
	BattleGround();
	~BattleGround();

	VOID	setState(EBATTLEGROUNDSTATE eState);
	EBATTLEGROUNDSTATE	getState();

	VOID	setMap(map_instance_battle* pMap);

	// 加入新玩家
	VOID	addRole(DWORD dwRoleID, INT nLevel, EBATTLENTEAM eTeam);

	// 玩家离开
	VOID	delRole(DWORD dwRoleID);


	VOID	killUnit(DWORD dwRoleID, BYTE byKillType);
	
	VOID	BeKill(DWORD dwRoleID, DWORD dwKiller);
	// 获取一个新的队伍类型
	EBATTLENTEAM getTeam();

	INT		getTeamNum(EBATTLENTEAM eTeamType);

	INT		getRoleNum();

	VOID	update(tagDWORDTime time);

	VOID	reset();
	// 到时间结束了
	VOID	end(DWORD dwActivityID);

	// 设置获胜方
	VOID	setWiner(EBATTLENTEAM eTeam, BOOL bKillBoss);

	// 设置首领id
	VOID	setTeamLeader(EBATTLENTEAM eTeam, DWORD dwID);


	VOID	sendToClient();
private:
	// 当前状态
	EBATTLEGROUNDSTATE m_eBattleState;

	//战场里的玩家
	std::map<DWORD, tagBattleRoleData>		m_setTeam[EBT_NUM];

	map_instance_battle*	m_pMapBattle;

	//每队首领id
	DWORD	m_dwLeaderID[EBT_NUM];
};