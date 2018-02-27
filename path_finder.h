/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//·��̽Ѱ��

#pragma once
#include "SparseGraph.h"

class Unit;
class Creature;


//-----------------------------------------------------------------------------
// ·��Ѱ����
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
	BOOL									m_bMoveingPathPoint;	// �Ƿ�����ʹ��Ѱ·�б��е�·����
	INT										m_nSynMoveTick;			// ͬ���ͻ����ƶ��ļ��
};

//-----------------------------------------------------------------------------
// ��ײ��·��Ѱ����
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

	std::list<pathNode>						m_listPathPoint;		// ����׷��Ѱ·ʱ��·����
	std::list<pathNode>::iterator			m_PathPointIt;			// ��ǰѰ��·�������һ����
};

//-----------------------------------------------------------------------------
// ����ײ��·��Ѱ����
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

	std::list<pathNode>						m_listPathPoint;		// ����׷��Ѱ·ʱ��·����
};
