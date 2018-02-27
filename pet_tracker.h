/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物移动控制器

#pragma once


class Unit;
//class Role;
class Creature;
class MoveData;

//----------------------------------------------------------------------------------------------------
// 宠物跟踪器
//----------------------------------------------------------------------------------------------------
class PetTracker
{
	//----------------------------------------------------------------------------------------------------
	// 一些常量
	//----------------------------------------------------------------------------------------------------
	static const INT		MAX_NEED_MOVE_LEN	= TILE_SCALE * 2;
	static const INT		MIN_NEED_MOVE_LEN	= TILE_SCALE * 2;
	static const INT		NEED_TRANS_LEN		= TILE_SCALE * 10;
	static const INT		MAX_TRY_TIME		= 100;
	static const INT		MAX_TRANS_TICKS		= TICK_PER_SECOND * 3;

public:
	//----------------------------------------------------------------------------------------------------
	// 构造与析构
	//----------------------------------------------------------------------------------------------------
	//PetTracker(PetSoul* pSoul, Pet* pPet);
	PetTracker(Creature* pPet);
	~PetTracker(){}

	//----------------------------------------------------------------------------------------------------
	// 一些get
	//----------------------------------------------------------------------------------------------------
	//Role*		GetMaster()	const;
	Creature*	GetPet() const			{	return m_pPet;		}
	Role*		GetTarget() const		{	return m_pTarget;	}

	//----------------------------------------------------------------------------------------------------
	// 一些set
	//----------------------------------------------------------------------------------------------------
	VOID		SetTarget(Role* pTar)	{	m_pTarget = pTar;	UpdatePetSpeed(m_pTarget);}
	VOID		SetTargetID(DWORD dwID) { m_nTargetID = dwID; }
	Role*		GetTarget() { return m_pTarget; }
	DWORD		GetTargetID() { return m_nTargetID; }
	//----------------------------------------------------------------------------------------------------
	// 更新（控制宠物跟随）
	//----------------------------------------------------------------------------------------------------
	VOID		Update();
	
	VOID		UpdateNeedPutDown();
private:
	//----------------------------------------------------------------------------------------------------
	// 一些用到的方法
	//----------------------------------------------------------------------------------------------------
	Vector3		FindTargetDest( Unit* pTarget );
	BOOL		NeedMove(const Vector3 &vMasterPos, const Vector3 &vPet);
	BOOL		NeedPutdown(const Vector3 &vMasterPos, const Vector3 &vPet);
	Vector3		GetNearPos(const Vector3 &vMasterPos, const Vector3 &vMasterFace, FLOAT fMaxRange, FLOAT fMinRange);
	BOOL		TryGoto(const Vector3 &vDstPos);
	BOOL		PutDownBack(const Vector3 &vMasterPos, const Vector3 &vMasterFace);
	VOID		UpdatePetSpeed(Role* pTarget);
	MoveData*	GetMoveData(Unit* pUnit);

private:
	Role*		m_pTarget;		// 目标
	Creature*	m_pPet;			// 宠物实体
	DWORD		m_nTargetID; 
	//PetSoul*	m_pSoul;		
	INT			m_nPutBackTicks;// 瞬移计时
};

