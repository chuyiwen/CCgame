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

#pragma once
#include "SparseGraph.h"

class Unit;
class Creature;


//-----------------------------------------------------------------------------
// 路径寻找器
//-----------------------------------------------------------------------------
class PathFinder
{
public:
	PathFinder() {}
	virtual ~PathFinder() {}

public:
	virtual BOOL	Init(Unit* pUnit) = 0;
	virtual BOOL	FindingPath(const Vector3& startPos,const Vector3& targetPos) = 0;
	virtual BOOL	GetCurPathPoint(const Vector3& startPos, Vector3& movePos) = 0;

protected:
	BOOL									m_bMoveingPathPoint;	// 是否正在使用寻路列表中的路径点
	INT										m_nSynMoveTick;			// 同步客户端移动的间隔
};

//-----------------------------------------------------------------------------
// 碰撞怪路径寻找器
//-----------------------------------------------------------------------------
class NPCPathFinderCol : public PathFinder
{
public:
	NPCPathFinderCol() {}
	~NPCPathFinderCol();

public:
	BOOL	Init(Unit* pUnit);
	BOOL	FindingPath(const Vector3& startPos,const Vector3& targetPos);
	BOOL    GetCurPathPoint(const Vector3& startPos, Vector3& movePos);

private:
	Creature*								m_pOwner;

	std::list<pathNode>						m_listPathPoint;		// 怪物追击寻路时的路径点
	std::list<pathNode>::iterator			m_PathPointIt;			// 当前寻到路径点的哪一个点
};

//-----------------------------------------------------------------------------
// 非碰撞怪路径寻找器
//-----------------------------------------------------------------------------
class NPCPathFinder : public PathFinder
{
public:
	NPCPathFinder() {}
	~NPCPathFinder();

public:
	BOOL	Init(Unit* pUnit);
	BOOL	FindingPath(const Vector3& startPos,const Vector3& targetPos);
	BOOL	GetCurPathPoint(const Vector3& startPos, Vector3& movePos);

private:
	Creature*								m_pOwner;

	std::list<pathNode>						m_listPathPoint;		// 怪物追击寻路时的路径点
};
