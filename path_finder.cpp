/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//路径探寻器

#include "StdAfx.h"



#include "map.h"
#include "creature.h"
#include "unit.h"
#include "path_finder.h"

NPCPathFinderCol::~NPCPathFinderCol()
{
	m_listPathPoint.clear();
}

//-------------------------------------------------------------------------------------------
// Init
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinderCol::Init(Unit* pUnit)
{
	if (!pUnit->IsCreature())	return FALSE;

	m_pOwner = dynamic_cast<Creature*>(pUnit);
	m_listPathPoint.clear();
	m_PathPointIt = m_listPathPoint.end();
	m_bMoveingPathPoint = FALSE;
	m_nSynMoveTick = 0;

	return TRUE;
}

//-------------------------------------------------------------------------------------------
// 得到下个路径点
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinderCol::GetCurPathPoint(const Vector3& startPos, Vector3& movePos)
{
	if(m_PathPointIt == m_listPathPoint.end())	return FALSE;

	pathNode node = *m_PathPointIt;

	movePos.x = node.x();
	movePos.z = node.y();

	++m_PathPointIt;
	return TRUE;
}

//-------------------------------------------------------------------------------------------
// 计算生成移动路径
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinderCol::FindingPath(const Vector3& startPos,const Vector3& targetPos)
{
	m_listPathPoint.clear();
	Map* pMap = m_pOwner->get_map();
	if(!VALID_POINT(pMap))	return FALSE;

	//pMap->get_path_finder()->go(pathNode(startPos.x, startPos.z), pathNode(targetPos.x, targetPos.z), m_listPathPoint);
	//m_PathPointIt = m_listPathPoint.begin();

	
	//if(m_PathPointIt != m_listPathPoint.end())
	//	return TRUE;
	//else
		return FALSE;
}

NPCPathFinder::~NPCPathFinder()
{
	m_listPathPoint.clear();
}

//-------------------------------------------------------------------------------------------
// Init
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinder::Init(Unit* pUnit)
{
	if (!pUnit->IsCreature())	return FALSE;

	m_pOwner = dynamic_cast<Creature*>(pUnit);
	m_listPathPoint.clear();
	m_bMoveingPathPoint = FALSE;
	m_nSynMoveTick = 0;

	return TRUE;
}

//-------------------------------------------------------------------------------------------
// 计算生成移动路径
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinder::FindingPath(const Vector3& startPos,const Vector3& targetPos)
{
	m_listPathPoint.clear();
	Map* pMap = m_pOwner->get_map();
	if(!VALID_POINT(pMap))	return FALSE;

	//pMap->get_path_finder()->go(pathNode(startPos.x, startPos.z), pathNode(targetPos.x, targetPos.z), m_listPathPoint);

	//if(m_listPathPoint.begin() != m_listPathPoint.end())
	//	return TRUE;
	//else
		return FALSE;
}

//-------------------------------------------------------------------------------------------
// 得到下个路径点
//-------------------------------------------------------------------------------------------
BOOL NPCPathFinder::GetCurPathPoint(const Vector3& startPos, Vector3& movePos)
{
	Map* pMap = m_pOwner->get_map();
	if(!VALID_POINT(pMap))	return FALSE;

	//pMap->get_path_finder()->smooth_path(startPos.x, startPos.z, m_listPathPoint);
	//std::list<pathNode>::iterator it = m_listPathPoint.begin();
	//if(it == m_listPathPoint.end())	return FALSE;

	//pathNode PathPoint = *it;
	//m_listPathPoint.pop_front();

	//movePos.x = PathPoint.x();
	//movePos.z = PathPoint.y();

	return TRUE;
}