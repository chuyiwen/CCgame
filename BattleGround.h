
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

// ս����
class BattleGround : public singleton<BattleGround>
{
public:
	BattleGround();
	~BattleGround();

	VOID	setState(EBATTLEGROUNDSTATE eState);
	EBATTLEGROUNDSTATE	getState();

	VOID	setMap(map_instance_battle* pMap);

	// ���������
	VOID	addRole(DWORD dwRoleID, INT nLevel, EBATTLENTEAM eTeam);

	// ����뿪
	VOID	delRole(DWORD dwRoleID);


	VOID	killUnit(DWORD dwRoleID, BYTE byKillType);
	
	VOID	BeKill(DWORD dwRoleID, DWORD dwKiller);
	// ��ȡһ���µĶ�������
	EBATTLENTEAM getTeam();

	INT		getTeamNum(EBATTLENTEAM eTeamType);

	INT		getRoleNum();

	VOID	update(tagDWORDTime time);

	VOID	reset();
	// ��ʱ�������
	VOID	end(DWORD dwActivityID);

	// ���û�ʤ��
	VOID	setWiner(EBATTLENTEAM eTeam, BOOL bKillBoss);

	// ��������id
	VOID	setTeamLeader(EBATTLENTEAM eTeam, DWORD dwID);


	VOID	sendToClient();
private:
	// ��ǰ״̬
	EBATTLEGROUNDSTATE m_eBattleState;

	//ս��������
	std::map<DWORD, tagBattleRoleData>		m_setTeam[EBT_NUM];

	map_instance_battle*	m_pMapBattle;

	//ÿ������id
	DWORD	m_dwLeaderID[EBT_NUM];
};