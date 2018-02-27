/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//移动

#pragma once
#include "Timer.h"

class Unit;
class Role;
class Creature;
class Map;

//------------------------------------------------------------------------------------------------------
// 格子怪物移动控制器
//------------------------------------------------------------------------------------------------------
class NavCollider_TileMove
{
public:
	NavCollider_TileMove();
	~NavCollider_TileMove();

public:
	VOID		Init(const Vector3& vStart, const Vector3& vDest, FLOAT fXZSpeed);
	ENavResult	Update(Vector3& vOutPos);

private:
	Vector3			m_vStart;		// 起始坐标
	Vector3			m_vDest;		// 终点坐标
	FLOAT			m_fTotalTime;	// 总共需要移动多长时间
	FLOAT			m_fCurTime;		// 当前走了多长时间
	INT				m_nMoveTicks;	// 需要的总的心跳数
	FLOAT			m_fXZSpeed;		// xz方向的移动速度
};

class MoveData
{
public:
	//---------------------------------------------------------------------------------------------
	// 移动的结果
	//---------------------------------------------------------------------------------------------
	enum EMoveRet
	{
		EMR_Success		=	0,		// 可以移动
		EMR_SelfLimit	=	1,		// 自身限制，比如中了眩晕，定身等等
		EMR_NoArrive	=	2,		// 不能到达
		EMR_Conflict	=	3,		// 移动状态之间冲突
		EMR_Invalid		=	4,		// 非法调用
	};

public:
	//---------------------------------------------------------------------------------------------
	// 辅助函数
	//---------------------------------------------------------------------------------------------
	static BOOL		IsPosInvalid(const Vector3& vPos);
	static BOOL		IsFaceInvalid(const Vector3& vFace);

	//---------------------------------------------------------------------------------------------
	// CONSTRUCT
	//---------------------------------------------------------------------------------------------
	MoveData() : m_pOwner(NULL), m_eCurMove(EMS_Stand), m_eNextPreMove(EMS_Stand), 
				 m_nVisTileIndex(INVALID_VALUE), m_bWaitClientMsg(FALSE), m_bIsStopped(FALSE),
				 m_pColliderWalk(NULL){}

	~MoveData()
	{
		SAFE_DELETE(m_pColliderWalk);
	}

	//-------------------------------------------------------------------------------------
	// 得到当前移动状态
	//-------------------------------------------------------------------------------------
	EMoveState	GetCurMoveState()		{ return m_eCurMove; }
	EMoveState	GetNextMoveState()		{ return m_eNextPreMove; }
	BOOL		IsStopped()				{ return m_bIsStopped; }

	//---------------------------------------------------------------------------------------------
	// 初始化移动结构，主要用于玩家和生物初始化时的移动状态
	//---------------------------------------------------------------------------------------------
	VOID Init(Unit* pOwner, const Vector3& vPos, const Vector3& vFaceTo);

	//----------------------------------------------------------------------------------------------
	// 重置移动结构，这主要用于玩家切换地图和怪物重生时，一般很少用
	//----------------------------------------------------------------------------------------------
	VOID Reset(FLOAT fX, const FLOAT fY, const FLOAT fZ, const FLOAT fFaceX, const FLOAT fFaceY, const FLOAT fFaceZ);

	//-----------------------------------------------------------------------------------------------
	// 开始移动
	//-----------------------------------------------------------------------------------------------
	EMoveRet	StartRoleWalk(const Vector3& vStart, const Vector3& vEnd);
	EMoveRet	StartCreatureWalk(const Vector3& vDest, EMoveState eState, BOOL bCheck=TRUE);
	EMoveRet	StopRoleMove(const Vector3& vPos);
	EMoveRet	StartFearRun(const Vector3& vDest);
	//-----------------------------------------------------------------------------------------------
	// 停止移动
	//-----------------------------------------------------------------------------------------------
	EMoveRet	StopMove(BOOL bSend=TRUE);
	EMoveRet	StopMove(const Vector3& vNewFace, BOOL bSend=TRUE);
	EMoveRet	StopMoveForce(BOOL bSend=TRUE);
	
	//-----------------------------------------------------------------------------------------------
	// 瞬移
	//-----------------------------------------------------------------------------------------------
	VOID		ForceTeleport(const Vector3& vPos, BOOL bSend=TRUE);

	//-----------------------------------------------------------------------------------------------
	// 改变朝向
	//-----------------------------------------------------------------------------------------------
	VOID		SetFaceTo(const Vector3& vFace);

	//-----------------------------------------------------------------------------------------------
	// 更新移动
	//-----------------------------------------------------------------------------------------------
	VOID		Update();
	VOID		UpdateRoleWalk();
	VOID		UpdateCreatureWalk();
	VOID		UpdateHitFly();

	//-------------------------------------------------------------------------------------
	// 取站立坐标点
	//-------------------------------------------------------------------------------------
	VOID		DropDownStandPoint(BOOL bSend=TRUE);
	VOID		DropDownStandPoint(const Vector3& vPos, BOOL bSend=TRUE);

	Vector3		GetNearPos(const Vector3 &vTargetPos, const Vector3 &vTargetFace, FLOAT fMaxRange, FLOAT fMinRange);

    VOID        OnPosDrift(const Vector3& vDir, const Vector3& vFrom, const Vector3& vTo);
    BOOL        CheckSpeedCheat();
    VOID        GetCheatAccumulate(FLOAT& val);
	VOID		ResetSpeedCheat() { m_fCheatAccumulate = 0.0f; }
private:
	//-------------------------------------------------------------------------------------
	// 各种检测
	//-------------------------------------------------------------------------------------
	EMoveRet	IsCanRoleWalk(const Vector3& vStart, const Vector3& vEnd);
	EMoveRet	IsCanJump(const Vector3& vStart, const Vector3& vDir);
	EMoveRet	IsCanRoleDrop(const Vector3& vStart, const Vector3& vDir);
	EMoveRet	IsCanRoleVDrop(const Vector3& vStart);
	EMoveRet	IsCanRoleSlide(const Vector3& vStart);
	EMoveRet	IsCanRoleSwim(const Vector3& vStart, const Vector3& vEnd);
	EMoveRet	IsCanRoleStop(const Vector3& vPos);
	EMoveRet	IsCanHitFly();

public:
	EMoveRet	IsCanCreatureWalk(const Vector3& vEnd, EMoveState eState, BOOL bCheck, Vector3* vNearPos=NULL);

private:

	BOOL		IsMapValid();
	BOOL		IsInValidDistance(const Vector3& vStart, BOOL b_jump_ = FALSE);
	FLOAT		GetCreatureMoveStateSpeed(EMoveState eState);

private:

	//-------------------------------------------------------------------------------------
	// 位置改变
	//-------------------------------------------------------------------------------------
	VOID		OnPosChange(const Vector3& vNewPos);

	//-------------------------------------------------------------------------------------
	// 开始移动
	//-------------------------------------------------------------------------------------
	VOID		OnStartMove();


	//-------------------------------------------------------------------------------------
	// 得到行走控制器
	//-------------------------------------------------------------------------------------
	NavCollider_TileMove*	GetColliderWalk()		{ if( !VALID_POINT(m_pColliderWalk) )		m_pColliderWalk = new NavCollider_TileMove;			return m_pColliderWalk; }

public:
	static Timer			m_Timer;							// 移动计时器

	Unit*					m_pOwner;							// 控制者
	EMoveState				m_eCurMove;							// 当前移动状态
	EMoveState				m_eNextPreMove;						// 在当前状态进行“完毕之后”，预计要切换到的状态
	Vector3					m_vPos;								// 当前位置
	Vector3					m_vPosStart;						// 当前状态开始时的初始位置
	Vector3					m_vDir;								// 当前移动方向
	Vector3					m_vDest;							// 目标点
	Vector3					m_vFace;							// 朝向

    FLOAT                   m_fCheatAccumulate;

	INT						m_nVisTileIndex;					// 当前所处的可视地砖索引
	BOOL					m_bWaitClientMsg;					// 如果当前状态完毕且下一个状态不是站立，该字段用于设置是否在等待客户端消息

	FLOAT					m_fStartTime;						// 移动开始的时间

	BOOL					m_bIsStopped;						// 被外界停下

	NavCollider_TileMove*	m_pColliderWalk;					// 行走控制器
};

//-------------------------------------------------------------------------------------------
// 改变面向
//-------------------------------------------------------------------------------------------
inline VOID MoveData::SetFaceTo(const Vector3& vFace)
{
	if( IsFaceInvalid(vFace) ) return;
	if( abs(vFace.x) < 0.001f && abs(vFace.z) < 0.001f ) return;

	m_vFace = vFace;
	m_vFace.y = 0;
}

