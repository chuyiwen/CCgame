/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�ƶ�

#pragma once
#include "Timer.h"

class Unit;
class Role;
class Creature;
class Map;

//------------------------------------------------------------------------------------------------------
// ���ӹ����ƶ�������
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
	Vector3			m_vStart;		// ��ʼ����
	Vector3			m_vDest;		// �յ�����
	FLOAT			m_fTotalTime;	// �ܹ���Ҫ�ƶ��೤ʱ��
	FLOAT			m_fCurTime;		// ��ǰ���˶೤ʱ��
	INT				m_nMoveTicks;	// ��Ҫ���ܵ�������
	FLOAT			m_fXZSpeed;		// xz������ƶ��ٶ�
};

class MoveData
{
public:
	//---------------------------------------------------------------------------------------------
	// �ƶ��Ľ��
	//---------------------------------------------------------------------------------------------
	enum EMoveRet
	{
		EMR_Success		=	0,		// �����ƶ�
		EMR_SelfLimit	=	1,		// �������ƣ���������ѣ�Σ�����ȵ�
		EMR_NoArrive	=	2,		// ���ܵ���
		EMR_Conflict	=	3,		// �ƶ�״̬֮���ͻ
		EMR_Invalid		=	4,		// �Ƿ�����
	};

public:
	//---------------------------------------------------------------------------------------------
	// ��������
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
	// �õ���ǰ�ƶ�״̬
	//-------------------------------------------------------------------------------------
	EMoveState	GetCurMoveState()		{ return m_eCurMove; }
	EMoveState	GetNextMoveState()		{ return m_eNextPreMove; }
	BOOL		IsStopped()				{ return m_bIsStopped; }

	//---------------------------------------------------------------------------------------------
	// ��ʼ���ƶ��ṹ����Ҫ������Һ������ʼ��ʱ���ƶ�״̬
	//---------------------------------------------------------------------------------------------
	VOID Init(Unit* pOwner, const Vector3& vPos, const Vector3& vFaceTo);

	//----------------------------------------------------------------------------------------------
	// �����ƶ��ṹ������Ҫ��������л���ͼ�͹�������ʱ��һ�������
	//----------------------------------------------------------------------------------------------
	VOID Reset(FLOAT fX, const FLOAT fY, const FLOAT fZ, const FLOAT fFaceX, const FLOAT fFaceY, const FLOAT fFaceZ);

	//-----------------------------------------------------------------------------------------------
	// ��ʼ�ƶ�
	//-----------------------------------------------------------------------------------------------
	EMoveRet	StartRoleWalk(const Vector3& vStart, const Vector3& vEnd);
	EMoveRet	StartCreatureWalk(const Vector3& vDest, EMoveState eState, BOOL bCheck=TRUE);
	EMoveRet	StopRoleMove(const Vector3& vPos);
	EMoveRet	StartFearRun(const Vector3& vDest);
	//-----------------------------------------------------------------------------------------------
	// ֹͣ�ƶ�
	//-----------------------------------------------------------------------------------------------
	EMoveRet	StopMove(BOOL bSend=TRUE);
	EMoveRet	StopMove(const Vector3& vNewFace, BOOL bSend=TRUE);
	EMoveRet	StopMoveForce(BOOL bSend=TRUE);
	
	//-----------------------------------------------------------------------------------------------
	// ˲��
	//-----------------------------------------------------------------------------------------------
	VOID		ForceTeleport(const Vector3& vPos, BOOL bSend=TRUE);

	//-----------------------------------------------------------------------------------------------
	// �ı䳯��
	//-----------------------------------------------------------------------------------------------
	VOID		SetFaceTo(const Vector3& vFace);

	//-----------------------------------------------------------------------------------------------
	// �����ƶ�
	//-----------------------------------------------------------------------------------------------
	VOID		Update();
	VOID		UpdateRoleWalk();
	VOID		UpdateCreatureWalk();
	VOID		UpdateHitFly();

	//-------------------------------------------------------------------------------------
	// ȡվ�������
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
	// ���ּ��
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
	// λ�øı�
	//-------------------------------------------------------------------------------------
	VOID		OnPosChange(const Vector3& vNewPos);

	//-------------------------------------------------------------------------------------
	// ��ʼ�ƶ�
	//-------------------------------------------------------------------------------------
	VOID		OnStartMove();


	//-------------------------------------------------------------------------------------
	// �õ����߿�����
	//-------------------------------------------------------------------------------------
	NavCollider_TileMove*	GetColliderWalk()		{ if( !VALID_POINT(m_pColliderWalk) )		m_pColliderWalk = new NavCollider_TileMove;			return m_pColliderWalk; }

public:
	static Timer			m_Timer;							// �ƶ���ʱ��

	Unit*					m_pOwner;							// ������
	EMoveState				m_eCurMove;							// ��ǰ�ƶ�״̬
	EMoveState				m_eNextPreMove;						// �ڵ�ǰ״̬���С����֮�󡱣�Ԥ��Ҫ�л�����״̬
	Vector3					m_vPos;								// ��ǰλ��
	Vector3					m_vPosStart;						// ��ǰ״̬��ʼʱ�ĳ�ʼλ��
	Vector3					m_vDir;								// ��ǰ�ƶ�����
	Vector3					m_vDest;							// Ŀ���
	Vector3					m_vFace;							// ����

    FLOAT                   m_fCheatAccumulate;

	INT						m_nVisTileIndex;					// ��ǰ�����Ŀ��ӵ�ש����
	BOOL					m_bWaitClientMsg;					// �����ǰ״̬�������һ��״̬����վ�������ֶ����������Ƿ��ڵȴ��ͻ�����Ϣ

	FLOAT					m_fStartTime;						// �ƶ���ʼ��ʱ��

	BOOL					m_bIsStopped;						// �����ͣ��

	NavCollider_TileMove*	m_pColliderWalk;					// ���߿�����
};

//-------------------------------------------------------------------------------------------
// �ı�����
//-------------------------------------------------------------------------------------------
inline VOID MoveData::SetFaceTo(const Vector3& vFace)
{
	if( IsFaceInvalid(vFace) ) return;
	if( abs(vFace.x) < 0.001f && abs(vFace.z) < 0.001f ) return;

	m_vFace = vFace;
	m_vFace.y = 0;
}

