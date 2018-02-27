/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//NPC队伍

#pragma once
#include "creature.h"

enum ENPCTeamEvent;

class NPCTeam
{
public:
	NPCTeam(DWORD dwID, Creature* pLeader);
	~NPCTeam();

	DWORD GetID() { return m_dwID; }

	VOID AddNPCTeamMem(Creature* pCreature);
	VOID DelGuildSentinel(BYTE byType);
	VOID OnEvent(ENPCTeamEvent eType, Unit* pUnit, DWORD dwEventMisc1=0, DWORD dwEventMisc2=0, DWORD dwEventMisc3=0);
	VOID OrderTeammateMove(Creature* pLeader, Vector3& vDes);
	DWORD GetLeaderID();
	Creature* IsTeamateHPLower(DWORD dwValue);
	Creature* IsTeamateMPLower(DWORD dwValue);

private:

	// 同步怪物小队仇恨
	VOID OnSynEnmity(Unit* pUnit, DWORD dwValue);

private:
	DWORD						m_dwID;
	package_map<DWORD, Creature*>		m_mapMemPtr;
	const tagNestProto			m_pNestProto;
};

