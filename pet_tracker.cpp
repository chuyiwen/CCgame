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

#include "StdAfx.h"
#include "pet_tracker.h"

//#include "pet_soul.h"
#include "unit.h"
#include "role.h"
#include "creature.h"
#include "move_data.h"
#include "creature_ai.h"


//----------------------------------------------------------------------------------------------------
// 更新
//----------------------------------------------------------------------------------------------------
VOID PetTracker::Update()
{
	// 若没有目标，则跟着主人
	//if (!VALID_POINT(m_pTarget) && VALID_POINT(GetMaster()))
	//	SetTarget(GetMaster());
	//else if (!VALID_POINT(GetMaster()))
	//	return ;
	
	if (!VALID_POINT(m_pTarget))
		return;

	// 若目标或者宠物无效
	if (!VALID_POINT(m_pTarget) ||	!VALID_POINT(m_pPet) ) 
		return;

	// 更新宠物速度
	UpdatePetSpeed(m_pTarget);

	// 更新移动数据
	GetMoveData(m_pPet)->Update();

	// 根据目标状态找到目的地点
	Vector3 vTargetPos		= m_pTarget->GetMoveData().m_vPos;
	Vector3 vTargetFace		= GetMoveData(m_pTarget)->m_vFace;
	Vector3 vPetDest		= GetMoveData(m_pPet)->m_vDest;
	Vector3 vPetCur			= m_pPet->GetCurPos();


	// 是否需要移动
	if (!NeedMove(vTargetPos, m_pPet->GetCurPos()))
		return;

	// 尝试走到目标身边
	Vector3 vTmp = GetNearPos(vTargetPos, vTargetFace, (FLOAT)MAX_NEED_MOVE_LEN, (FLOAT)MIN_NEED_MOVE_LEN);
	if (TryGoto(vTmp) )
	{
		GetMoveData(m_pPet)->StartCreatureWalk(vTmp, EMS_CreatureWalk, FALSE);
	}
	else
	{
		PutDownBack(m_pTarget->GetMoveData().m_vPos, m_pTarget->GetMoveData().m_vFace);
	}
	UpdateNeedPutDown();

}

VOID PetTracker::UpdateNeedPutDown()
{
	Vector3 vTargetPos		= m_pTarget->GetMoveData().m_vPos;

	// 是否需要瞬移
	if (NeedPutdown(vTargetPos, m_pPet->GetCurPos()))
	{
		PutDownBack(m_pTarget->GetMoveData().m_vPos, m_pTarget->GetMoveData().m_vFace);

		// 不打目标了
		if (VALID_POINT(m_pPet) && VALID_POINT(m_pPet->GetAI()))
		{
			m_pPet->GetAI()->ClearAllEnmity();
			m_pPet->GetAI()->SetTargetUnitID(INVALID_VALUE);
		}

		return ;
	}
}
//----------------------------------------------------------------------------------------------------
// 根据目标找到目的点
//----------------------------------------------------------------------------------------------------
Vector3 PetTracker::FindTargetDest( Unit* pTarget )
{
	Vector3	vTargetPos;
	EMoveState eTarMove	= GetMoveData(pTarget)->m_eCurMove;
	Vector3 vTargetCurPos	= GetMoveData(m_pTarget)->m_vPos;
	Vector3	vTargetDestPos	= GetMoveData(m_pTarget)->m_vDest;
	if (EMS_Stand == eTarMove)
	{
		vTargetPos = vTargetCurPos;
	}
	else
	{
		vTargetPos = vTargetDestPos;
	}
	return vTargetPos;
}

//----------------------------------------------------------------------------------------------------
// 是否需要移动
//----------------------------------------------------------------------------------------------------
BOOL PetTracker::NeedMove(const Vector3 &vMasterPos, const Vector3 &vPet)
{
	if (GetMoveData(m_pPet)->GetCurMoveState() == EMS_CreatureWalk)
	{
		return false;
	}

	FLOAT fDist = Vec3Dist(vMasterPos, vPet);
	if (fDist >= (FLOAT)MAX_NEED_MOVE_LEN + 10)
	{
		return true;
	}
	//else if (fDist <= (FLOAT)MIN_NEED_MOVE_LEN)
	//{
	//	bNeedMove	= TRUE;
	//}
	return false;
}

//----------------------------------------------------------------------------------------------------
// 在一个点周围找到随机点
//----------------------------------------------------------------------------------------------------
Vector3	PetTracker::GetNearPos(const Vector3 &vMasterPos, const Vector3 &vMasterFace, FLOAT fMaxRange, FLOAT fMinRange)
{
	//随机选择一个角度
	FLOAT fAngle	= (get_tool()->tool_rand() % 360) / 360.0f * 3.14f;

	FLOAT fDist = fMinRange;
	if (fMaxRange - fMinRange != 0)
	{
		//随机选择一个合理的范围
		fDist = get_tool()->tool_rand() % INT(fMaxRange - fMinRange) + fMinRange;
	}

	//计算目标点
	Vector3 vRt		= vMasterPos;
	vRt.x +=	sin(fAngle) * fDist;
	vRt.z +=	cos(fAngle) * fDist;
	return vRt;
}

//----------------------------------------------------------------------------------------------------
// 可否移动到某个点
//----------------------------------------------------------------------------------------------------
BOOL PetTracker::TryGoto(const Vector3 &vDstPos )
{
	/*GetMoveData(pPet)->StopMove();*/
	BOOL	bRtv = TRUE;
	if (!VALID_POINT(m_pPet))
	{
		bRtv = FALSE;
	}
	else if( MoveData::EMR_Success != GetMoveData(m_pPet)->IsCanCreatureWalk(vDstPos, EMS_CreatureWalk, TRUE))
	{
		bRtv = FALSE;
	}
// 	if (EMS_Stand == GetMoveData(pPet)->m_eCurMove)
// 	{
// 		bRtv = FALSE;
// 	}
	return bRtv;		
}

//----------------------------------------------------------------------------------------------------
// 把宠物放到身后
//----------------------------------------------------------------------------------------------------
BOOL PetTracker::PutDownBack( const Vector3 &vMasterPos, const Vector3 &vMasterFace )
{
	Vector3 vFaceNormal = -vMasterFace;
	Vec3Normalize(vFaceNormal);
	Vector3 vNewPos		= vMasterPos + vFaceNormal * MAX_NEED_MOVE_LEN;

	if (!VALID_POINT(m_pPet))
	{
		return FALSE;
	}
	
	GetMoveData(m_pPet)->ForceTeleport(vNewPos);
	return TRUE;
}

//----------------------------------------------------------------------------------------------------
// 取得MoveData
//----------------------------------------------------------------------------------------------------
MoveData* PetTracker::GetMoveData(Unit* pUnit)
{	
	MoveData* pMoveData = NULL;
	if (VALID_POINT(pUnit))
	{
		pMoveData = &(pUnit->GetMoveData());
	}
	return pMoveData;
}

//----------------------------------------------------------------------------------------------------
// 设置宠物的速度
//----------------------------------------------------------------------------------------------------
VOID PetTracker::UpdatePetSpeed(Role* pTarget)
{
	if(VALID_POINT(m_pPet) && VALID_POINT(pTarget) && m_pPet->GetXZSpeed() != pTarget->GetXZSpeed())
	{
		if (pTarget->IsRole() && VALID_POINT(pTarget) )
		{
			if (pTarget->IsInRoleStateAny(ERS_Mount | ERS_Mount2))
			{
				m_pPet->SetAttValue(ERA_Speed_XZ, pTarget->GetAttValue(ERA_Speed_Mount), TRUE);	
			}
			else if(pTarget->IsInRoleStateAny(ERS_Carry))
			{
				m_pPet->SetAttValue(ERA_Speed_XZ, pTarget->GetAttValue(ERA_Speed_XZ)/2, TRUE);	
			}
			else
			{
				m_pPet->SetAttValue(ERA_Speed_XZ, pTarget->GetAttValue(ERA_Speed_XZ), TRUE);	
			}
			
		}

		m_pPet->GetMoveData().StopMove();
	}
}

//----------------------------------------------------------------------------------------------------
// 构造函数Z
//----------------------------------------------------------------------------------------------------
//PetTracker::PetTracker(PetSoul* pSoul, Pet* pPet)
PetTracker::PetTracker(Creature* pPet)
{
	//ASSERT(VALID_POINT(pTarget));
	ASSERT(VALID_POINT(pPet));
	m_pPet		= pPet;
	//m_pSoul		= pSoul;

	m_pTarget	= NULL;
	m_nTargetID = INVALID_VALUE;
	m_nPutBackTicks = MAX_TRANS_TICKS;
}

//Role* PetTracker::GetMaster() const
//{
//	return m_pSoul->GetMaster();
//}

BOOL PetTracker::NeedPutdown( const Vector3 &vMasterPos, const Vector3 &vPet )
{

	if (--m_nPutBackTicks < 0)
	{	
		m_nPutBackTicks = MAX_TRANS_TICKS;
	
		FLOAT fDist = Vec3Dist(vMasterPos, vPet);
		if (fDist >= NEED_TRANS_LEN)
		{
			return TRUE;
		}
		
	}
	
	return FALSE;
}