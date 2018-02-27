/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//NPC队伍管理

#pragma once

class NPCTeam;
class CVector2D;
struct tagNPCTeamOrder;

class NPCTeamMgr
{
public:
	NPCTeamMgr(Map* pMap);
	~NPCTeamMgr();

	// 获得小队队形结构
	const tagNPCTeamOrder*	GetNPCTeamOrder(ENPCTeamOrder eTeamOrder)	{ return m_mapNPCOrder.find(eTeamOrder); }
	// 获得小队指针
	NPCTeam*	GetNPCTeam(DWORD dwID) { return m_mapNPCTeam.find(dwID); }

	// 加载小队队形
	static BOOL	LoadNPCOrder();
	// 刷怪时计算小队队员位置
	Vector3		CalTeamMemPos(Creature *pLeader, POINT point, Vector3 vFace, const tagNestProto* pNestProto);
	// 创建怪物小队
	NPCTeam*	CreateTeam(Creature* pLeader);

	static VOID DestoryNPCOrder();
private:
	
private:
	package_map<DWORD, NPCTeam*>								m_mapNPCTeam;
	static package_map<ENPCTeamOrder, tagNPCTeamOrder*>		m_mapNPCOrder;
	Map*												m_pMap;
	DWORD												m_dwTeamIndex;		// 小队ID索引
};