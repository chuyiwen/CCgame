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

#include "StdAfx.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "creature.h"

#include "NPCTeam_define.h"
#include "creature_order.h"

package_map<ENPCTeamOrder, tagNPCTeamOrder*>	NPCTeamMgr::m_mapNPCOrder;

NPCTeamMgr::NPCTeamMgr(Map* pMap)
{
	m_mapNPCTeam.clear();
	m_pMap = pMap;
	m_dwTeamIndex = 0;
}

NPCTeamMgr::~NPCTeamMgr()
{
	m_pMap = NULL;

	package_map<DWORD, NPCTeam*>::map_iter it = m_mapNPCTeam.begin();
	NPCTeam* pTeam = NULL;

	while( m_mapNPCTeam.find_next(it, pTeam) )
	{
		SAFE_DELETE(pTeam);
	}

	m_mapNPCTeam.clear();
}

VOID NPCTeamMgr::DestoryNPCOrder()
{
	tagNPCTeamOrder* pNPCTeamOrder = NULL;
	m_mapNPCOrder.reset_iterator();
	while(m_mapNPCOrder.find_next(pNPCTeamOrder))
	{
		SAFE_DELETE(pNPCTeamOrder);
	}
	m_mapNPCOrder.clear();
}

//----------------------------------------------------------------------------
// 加载小队队形
//----------------------------------------------------------------------------
BOOL NPCTeamMgr::LoadNPCOrder()
{
	FILE *pFile = fopen("server_data\\NPCOrder.ini", "r");
	if (pFile == NULL)
	{
		return FALSE;
	}

	for(INT i = 1; i < NTO_END; i++)
	{
		CHAR szField[TEAM_WIDTH];
		ZeroMemory(szField, TEAM_WIDTH);
		sprintf(szField, "[%d]", i);

		CHAR cTmp[TEAM_WIDTH+2];
		ZeroMemory(cTmp, TEAM_WIDTH+2);
		fgets(cTmp, TEAM_WIDTH+2, pFile);
		if(strncmp(cTmp, szField, strlen(szField)) != 0)
		{
			fclose(pFile);
			break;
		}

		CHAR szTeamOrder[TEAM_WIDTH][TEAM_HEIGH+2];

		INT nError = 0;
		for(INT n = 0; n < TEAM_HEIGH; n++)
		{
			fgets(szTeamOrder[n], TEAM_WIDTH+2, pFile);
			nError = ferror(pFile);
		}

		tagNPCTeamOrder* pTeamOrder = new tagNPCTeamOrder;
		if(!VALID_POINT(pTeamOrder))
			break;

		for(INT k = 0; k < TEAM_WIDTH; k++)
		{
			for(INT j = 0; j < TEAM_HEIGH; j++)
			{
				if('1' == szTeamOrder[k][j])
				{
					POINT Point;
					Point.x = j-2;
					Point.y = k-2;
					pTeamOrder->NPCOrder.push_back(Point);
				}
			}
		}

		m_mapNPCOrder.add((ENPCTeamOrder)i, pTeamOrder);
	}

	fclose(pFile);

	return TRUE;
}

//----------------------------------------------------------------------------
// 计算小队成员的位置
//----------------------------------------------------------------------------
Vector3 NPCTeamMgr::CalTeamMemPos(Creature *pLeader, POINT point, Vector3 vFace, const tagNestProto* pNestProto)
{
	Vector3 vPos;
	CVector2D vector((FLOAT)point.x, (FLOAT)point.y);
	CVector2D vface(vFace.x, -vFace.z);
	Vector3 vLeaderPos = pLeader->GetCurPos();

	// 按照队长的朝向旋转队员的位置
	CMatrix2D Tran;
	Tran = CMatrix2D::CreateRotateMatrix(vface);

	CMatrix2D  MATRIX90(0.0f, 1.0f, -1.0f, 0.0f);

	// 缩放队员间的间距
	CMatrix2D Scale(pNestProto->fSpace, 0.0f, 0.0f, pNestProto->fSpace);

	vector = Tran*vector;
	vector = MATRIX90*vector*Scale;
	vPos.x = vector.fx + vLeaderPos.x;
	vPos.z = vector.fy + vLeaderPos.z;

	if(pLeader->NeedCollide())
	{
		vPos.y = pLeader->GetCurPosTop().y;
	}
	else
	{
		vPos.y = 0;
	}

	return vPos;
}

//----------------------------------------------------------------------------
// 创建怪物小队
//----------------------------------------------------------------------------
NPCTeam* NPCTeamMgr::CreateTeam(Creature* pLeader)
{
	NPCTeam* pTeam = new NPCTeam(m_dwTeamIndex, pLeader);
	if(!VALID_POINT(pTeam))		return NULL;

	m_mapNPCTeam.add(m_dwTeamIndex, pTeam);

	// 小队ID索引加一
	++m_dwTeamIndex;

	return pTeam;
}

