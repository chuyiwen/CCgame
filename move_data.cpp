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

#include "stdafx.h"

#include "map.h"
#include "unit.h"
#include "role.h"
#include "creature.h"
#include "move_data.h"
#include "pet_pocket.h"
#include "SparseGraph.h"

#include "..\..\common\WorldDefine\ItemDefine.h"

//---------------------------------------------------------------------------------------------
// 静态变量
//---------------------------------------------------------------------------------------------
Timer MoveData::m_Timer;

//---------------------------------------------------------------------------------------------
// 格子移动控制器
//---------------------------------------------------------------------------------------------
NavCollider_TileMove::NavCollider_TileMove() : m_vStart(), m_vDest(), m_fTotalTime(0.0f), m_fCurTime(0.0f), m_nMoveTicks(0), m_fXZSpeed(0.0f)
{

}

NavCollider_TileMove::~NavCollider_TileMove() {}

VOID NavCollider_TileMove::Init(const Vector3& vStart, const Vector3& vDest, FLOAT fXZSpeed)
{
	ASSERT( fXZSpeed > 0.0f );

	m_vStart	=	vStart;
	m_vDest		=	vDest;
	m_fXZSpeed	=	fXZSpeed;

	// 计算两点距离
	FLOAT fDistanceX = m_vDest.x - m_vStart.x;
	FLOAT fDistanceZ = m_vDest.z - m_vStart.z;
	FLOAT fDistanceSquare = fDistanceX * fDistanceX + fDistanceZ * fDistanceZ;

	// 计算总时间和总心跳数
	m_fTotalTime = get_fast_code()->fast_square_root(fDistanceSquare) / m_fXZSpeed;
	m_nMoveTicks = (INT)(m_fTotalTime * 1000.0f) / TICK_TIME;
	m_nMoveTicks += ( (INT(m_fTotalTime * 1000.0f) - TICK_TIME * m_nMoveTicks) > 0 ? 1 : 0 );

	// 当前移动时间
	m_fCurTime = 0.0f;
}

ENavResult NavCollider_TileMove::Update(Vector3& vOutPos)
{
	// 如果本身就不能移动
	if( m_nMoveTicks <= 0 )
	{
		vOutPos = m_vStart;
		return ENR_Arrive;
	}

	// 减去tick
	if( --m_nMoveTicks > 0 )
	{
		m_fCurTime += (FLOAT)TICK_TIME / 1000.0f;
	}
	else
	{
		m_fCurTime = m_fTotalTime;
	}

	vOutPos.x = m_vStart.x + (m_vDest.x - m_vStart.x) * (m_fCurTime / m_fTotalTime);
	vOutPos.z = m_vStart.z + (m_vDest.z - m_vStart.z) * (m_fCurTime / m_fTotalTime);
	vOutPos.y = 0;

	if( m_nMoveTicks > 0 )
	{
		return ENR_ToBeContinued;
	}
	else
	{
		return ENR_Arrive;
	}
}

//---------------------------------------------------------------------------------------------
// 检测坐标
//---------------------------------------------------------------------------------------------
BOOL MoveData::IsPosInvalid(const Vector3& vPos)
{
	// 检测是不是实数
	if( _isnan(vPos.x) || _isnan(vPos.y) || _isnan(vPos.z) )
		return TRUE;

	// 检测是不是无穷
	if( 0 == _finite(vPos.x) || 0 == _finite(vPos.y) || 0 == _finite(vPos.z) )
	{
		return TRUE;
	}

	return FALSE;	
}

//---------------------------------------------------------------------------------------------
// 检查朝向
//---------------------------------------------------------------------------------------------
BOOL MoveData::IsFaceInvalid(const Vector3& vFace)
{
	// 检测是不是实数
	if( _isnan(vFace.x) || _isnan(vFace.y) || _isnan(vFace.z) )
		return TRUE;

	// 检测是不是无穷
	if( 0 == _finite(vFace.x) || 0 == _finite(vFace.y) || 0 == _finite(vFace.z) )
	{
		return TRUE;
	}

	// 将坐标归一化后查看有问题
	Vector3 vNewFace = vFace;
	Vec3Normalize(vNewFace);

	if( _isnan(vNewFace.x) || _isnan(vNewFace.y) || _isnan(vNewFace.z) )
		return TRUE;

	if( 0 == _finite(vNewFace.x) || 0 == _finite(vNewFace.y) || 0 == _finite(vNewFace.z) )
	{
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------------------------
// 初始化移动结构，主要用于玩家和生物初始化时的移动状态
//---------------------------------------------------------------------------------------------
VOID MoveData::Init(Unit* pOwner, const Vector3& vPos, const Vector3& vFaceTo)
{
	m_pOwner			=		pOwner;
	m_eCurMove			=		EMS_Stand;
	m_eNextPreMove		=		EMS_Stand;
	m_vPos				=		vPos;
	m_vFace				=		vFaceTo;
	//m_vPos.x			=		fX;
	//m_vPos.y			=		fY;
	//m_vPos.z			=		fZ;
	//m_vFace.x			=		fFaceX;
	//m_vFace.y			=		fFaceY;
	//m_vFace.z			=		fFaceZ;
	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_vDir				=		m_vFace;
	m_bWaitClientMsg	=		FALSE;
	m_nVisTileIndex		=		INVALID_VALUE;
	m_bIsStopped		=		FALSE;
    m_fCheatAccumulate = 0;
}

//----------------------------------------------------------------------------------------------
// 重置移动结构，这主要用于玩家切换地图和怪物重生时，一般很少用
//----------------------------------------------------------------------------------------------
VOID MoveData::Reset(const FLOAT fX, const FLOAT fY, const FLOAT fZ, const FLOAT fFaceX, const FLOAT fFaceY, const FLOAT fFaceZ)
{
	m_eCurMove			=		EMS_Stand;
	m_eNextPreMove		=		EMS_Stand;
	m_vPos.x			=		fX;
	m_vPos.y			=		fY;
	m_vPos.z			=		fZ;
	m_vFace.x			=		fFaceX;
	m_vFace.y			=		fFaceY;
	m_vFace.z			=		fFaceZ;
	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_vDir				=		m_vFace;
	m_bWaitClientMsg	=		FALSE;
	m_nVisTileIndex		=		INVALID_VALUE;
	m_bIsStopped		=		FALSE;
    m_fCheatAccumulate = 0;
}

//-----------------------------------------------------------------------------------------------
// 人物行走
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StartRoleWalk(const Vector3& vStart, const Vector3& vEnd)
{
	EMoveRet eRet = IsCanRoleWalk(vStart, vEnd);

	if( EMR_Success == eRet )
	{
		GetColliderWalk()->Init(vStart, vEnd, m_pOwner->GetXZSpeed());

		m_eCurMove			=	EMS_Walk;
		m_eNextPreMove		=	EMS_Stand;
		m_vPosStart			=	vStart;
		m_vDest				=	vEnd;
		m_vDir				=	vEnd - vStart;
		m_bWaitClientMsg	=	FALSE;
		m_fStartTime		=	m_Timer.GetElapse();
		m_bIsStopped		=	FALSE;

		SetFaceTo(m_vDir);
		OnStartMove();
       // OnPosDrift(m_vDir, m_vPos, vStart);
		OnPosChange(vStart);
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return eRet;
}


//-----------------------------------------------------------------------------------------------
// 怪物行走（bPatrol代表是否巡逻，bCheck代表是否检测路径）
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StartCreatureWalk(const Vector3& vEnd, EMoveState eState, BOOL bCheck)
{
	EMoveRet eRet = IsCanCreatureWalk(vEnd, eState, bCheck);

	if( EMR_Success == eRet )
	{
		FLOAT fRealSpeed = GetCreatureMoveStateSpeed(eState);

		Creature* pCreature = static_cast<Creature*>(m_pOwner);

		
		GetColliderWalk()->Init(m_vPos, vEnd, fRealSpeed);
		
		m_eCurMove			=	eState;
		m_eNextPreMove		=	EMS_Stand;
		m_vPosStart			=	m_vPos;
		m_vDest				=	vEnd;
		m_vDir				=	vEnd - m_vPos;
		m_bWaitClientMsg	=	FALSE;
		m_fStartTime		=	m_Timer.GetElapse();
		m_bIsStopped		=	FALSE;

		SetFaceTo(m_vDir);
		OnStartMove();
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return eRet;
}


//-----------------------------------------------------------------------------------------------
// 人物停止移动
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopRoleMove(const Vector3& vPos)
{
	EMoveRet eRet = IsCanRoleStop(vPos);

	if( EMR_Success == eRet )
	{
		m_eCurMove			=		EMS_Stand;
		m_eNextPreMove		=		EMS_Stand;
		m_vPosStart			=		vPos;
		m_vDest				=		vPos;
		m_bWaitClientMsg	=		FALSE;
		m_bIsStopped		=		FALSE;

       // OnPosDrift(m_vDir, m_vPos, vPos);
		OnPosChange(vPos);
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return eRet;
}


//-----------------------------------------------------------------------------------------------
// 服务器停止移动，一般用于减速等情况，如果处在不能随便停止移动的状态，则不停止
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMove(BOOL bSend)
{
	// 不能停止的移动
	if( 
		EMS_HitFly	==	m_eCurMove	)
		return EMR_Conflict;

	// 本来就是站立或漂浮状态
	if( EMS_Stand == m_eCurMove  )
		return EMR_Success;

	// 根据是移动还是游泳分别切换到站立或漂浮
	if( EMS_Walk			==	m_eCurMove ||
		EMS_CreaturePatrol	==	m_eCurMove ||
		EMS_CreatureWalk	==	m_eCurMove ||
		EMS_CreatureFlee	==	m_eCurMove ||
		EMS_CreatureRun		==	m_eCurMove)
	{
		m_eCurMove			=		EMS_Stand;
		m_eCurMove			=		EMS_Stand;
	}

	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_bWaitClientMsg	=		FALSE;
	m_bIsStopped		=		TRUE;

	// 同步给周围玩家
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// 停止移动，并朝向一个新的方向
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMove(const Vector3& vNewFace, BOOL bSend)
{
	// 检测一下朝向
	if( IsFaceInvalid(vNewFace) )
	{
		SI_LOG->write_log(_T("face invalid when try to rotate, face=<%f, %f, %f>\r\n"),
					vNewFace.x, vNewFace.y, vNewFace.z);
		return EMR_Invalid;
	}

	// 不能停止的移动
	if( 
		EMS_HitFly	==  m_eCurMove)
		return EMR_Conflict;

	// 本来就是站立或漂浮状态
	if( EMS_Stand == m_eCurMove  )
		return EMR_Success;

	// 根据是移动还是游泳分别切换到站立或漂浮
	if( EMS_Walk			==	m_eCurMove ||
		EMS_CreaturePatrol	==	m_eCurMove ||
		EMS_CreatureWalk	==	m_eCurMove ||
		EMS_CreatureFlee	==	m_eCurMove ||
		EMS_CreatureRun		==	m_eCurMove)
	{
		m_eCurMove			=		EMS_Stand;
		m_eCurMove			=		EMS_Stand;
	}

	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_bWaitClientMsg	=		FALSE;
	m_bIsStopped		=		TRUE;

	SetFaceTo(vNewFace);

	// 同步给周围玩家
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// 服务器强制停止移动
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMoveForce(BOOL bSend)
{
	// 本来就是站立或漂浮状态
	if( EMS_Stand == m_eCurMove )
		return EMR_Success;

	// 如果正在移动或游泳或滑落，则立即切换到当前状态
	if( EMS_Walk			==	m_eCurMove ||
		EMS_CreaturePatrol	==	m_eCurMove ||
		EMS_CreatureWalk	==	m_eCurMove ||
		EMS_CreatureFlee	==	m_eCurMove ||
		EMS_CreatureRun		==	m_eCurMove 
		)
	{
		
		m_eCurMove			=		EMS_Stand;
		
		m_vPosStart			=		m_vPos;
		m_vDest				=		m_vPos;
		m_bWaitClientMsg	=		FALSE;
		m_bIsStopped		=		TRUE;

		// 同步给周围玩家
		if( bSend )
		{
			m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
		}

		return EMR_Success;
	}

	// 如果正在跳跃，掉落或滑落
	if(
		EMS_HitFly		==	m_eCurMove)
	{
		DropDownStandPoint(bSend);
		m_bIsStopped = TRUE;
		return EMR_Success;
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// 瞬移到某个位置站立
//-----------------------------------------------------------------------------------------------
VOID MoveData::ForceTeleport(const Vector3& vPos, BOOL bSend)
{
	DropDownStandPoint(vPos, bSend);
}

//-----------------------------------------------------------------------------------------------
// 更新移动
//-----------------------------------------------------------------------------------------------
VOID MoveData::Update()
{
	if (m_pOwner->IsRole())
	{
		Role* pRole = (Role*)m_pOwner;

		if( EMS_Stand == m_eCurMove )
		{
			float fXZSpeed = pRole->GetXZSpeed();
			//const tagEquip* pEquip = pRole->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
			//if (VALID_POINT(pEquip))
			//{
			//	INT n_speed_delta = (INT)( pRole->GetBaseAttValue(ERA_Speed_XZ) /** ( pEquip->equipSpec.dwSpeed / 10000.0f )*/ );

			//	fXZSpeed = n_speed_delta + pRole->GetBaseAttValue(ERA_Speed_Mount);

			//	fXZSpeed = X_DEF_XZ_SPEED * (fXZSpeed / 10000.0f);
			//}
			
			if ( fXZSpeed <= 0 )
				fXZSpeed = 0.001;

			if ( m_fCheatAccumulate > 0.0f )
			{
				m_fCheatAccumulate -= fXZSpeed * m_Timer.GetDelta();
				if ( m_fCheatAccumulate < 0.0f )
					m_fCheatAccumulate = 0.0f;
			}
			return;
		}
	}	

	if( m_bWaitClientMsg )
		return;
	
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();
	
	//tstring szDesc = _T("");
	switch( m_eCurMove )
	{
	case EMS_Walk:
		UpdateRoleWalk();
		//szDesc = _T("MoveData::UpdateRoleWalk() time %d\r\n");
		break;

	case EMS_CreaturePatrol:
	case EMS_CreatureWalk:
	case EMS_CreatureFlee:
	case EMS_CreatureRun:
		UpdateCreatureWalk();
		//szDesc = _T("MoveData::UpdateCreatureWalk() time %d\r\n");
		break;

	default:
		break;
	}

	//dw_new_time = timeGetTime();
	//if(dw_new_time - dw_time > 30)
	//{
	//	g_world.get_log()->write_log(szDesc.c_str(), dw_new_time - dw_time);
	//}
}

//-----------------------------------------------------------------------------------------------
// 更新人物行走
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateRoleWalk()
{
	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// 更新坐标
	Vector3 vOutPos;
	float fEndTime = 0.0f;
	DWORD dwCarrierID = INVALID_VALUE;

	ENavResult eResult = GetColliderWalk()->Update(vOutPos);
	
	OnPosChange(vOutPos);

	if( ENR_ToBeContinued != eResult )
	{
		// 当前移动停止
		if( ENR_Arrive == eResult || ENR_Blocking == eResult )
		{
			m_eCurMove			=	EMS_Stand;
			m_eNextPreMove		=	EMS_Stand;
		}
		else
		{
			m_eCurMove			=	EMS_Stand;
			m_eNextPreMove		=	EMS_Stand;
		}

	}
}





//-----------------------------------------------------------------------------------------------
// 更新怪物行走
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateCreatureWalk()
{
	Creature* pCreature = static_cast<Creature*>(m_pOwner);

	// 更新坐标
	Vector3 vOutPos;
	FLOAT fEndTime = 0.0f;
	DWORD dwCarrierID = INVALID_VALUE;

	ENavResult eResult = ENR_ToBeContinued;

	eResult = GetColliderWalk()->Update(vOutPos);
	
	OnPosChange(vOutPos);

	// 检查返回值
	if( ENR_ToBeContinued != eResult )
	{
		if( ENR_Arrive == eResult || ENR_Blocking == eResult )
		{
			m_eCurMove		=	EMS_Stand;
			m_eNextPreMove	=	EMS_Stand;
		}
		else
		{
			m_eCurMove		=	EMS_Stand;
			m_eNextPreMove	=	EMS_Stand;
		}
	}
}

//-----------------------------------------------------------------------------------------------
// 更新被击飞
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateHitFly()
{

}


//-----------------------------------------------------------------------------------------------
// 根据当前坐标落下，找到一个碰撞位置站立，可能会到游泳
//------------------------------------------------------------------------------------------------
VOID MoveData::DropDownStandPoint(BOOL bSend)
{
	m_vPos.y = 0;


	BOOL bWaterWalk = FALSE;	// 是否水上行走

	if( m_pOwner->IsRole() )
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		bWaterWalk = pRole->IsInRoleState(ERS_WaterWalk);
	}


	// 站立
	m_eCurMove		=	EMS_Stand;
	m_eNextPreMove	=	EMS_Stand;
	
	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_bWaitClientMsg	=		FALSE;

	// 同步给周围玩家
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}
}

//-------------------------------------------------------------------------------------
// 根据一个新坐标落下，找到一个碰撞位置站立，可能会到游泳
//-------------------------------------------------------------------------------------
VOID MoveData::DropDownStandPoint(const Vector3& vPos, BOOL bSend)
{
	// 检测一下坐标
	if( IsPosInvalid(vPos) )
	{
		SI_LOG->write_log(_T("pos invalid when try to drop stand point, pos=<%f, %f, %f>\r\n"),
			vPos.x, vPos.y, vPos.z);
		return;
	}

	StopMoveForce(FALSE);
	OnPosChange(vPos);

	DropDownStandPoint(bSend);
}

//-------------------------------------------------------------------------------------
// 位置改变
//-------------------------------------------------------------------------------------
VOID MoveData::OnPosChange(const Vector3& vNewPos)
{
	// 加入坐标检测函数
	if( IsPosInvalid(vNewPos) )
	{
		SI_LOG->write_log(_T("Invalid Pos, Current movestate: %d\r\n"), m_eCurMove);
	}

	// 修正坐标
	Vector3 vPos = vNewPos;
	m_pOwner->get_map()->fix_position(vPos);
	//m_pOwner->get_map()->set_have_unit(m_vPos, false);
	//m_pOwner->get_map()->set_have_unit(vPos, true);

	m_vPos = vPos;	// 设置为新坐标

	INT nNewVisIndex = m_pOwner->get_map()->world_position_to_visible_index(m_vPos);
	if( m_nVisTileIndex != nNewVisIndex )
	{
		m_pOwner->get_map()->synchronize_change_visible_tile(m_pOwner, m_nVisTileIndex, nNewVisIndex);
	}

	// 如果是隐身单位，需同步给周围玩家
	if( m_pOwner->IsInStateInvisible() )
	{
		m_pOwner->get_map()->update_lurk_to_big_visible_tile_role(m_pOwner);
	}

	// 如果是玩家，则检测地图区域，及将周围隐身玩家同步给他
	if( m_pOwner->IsRole() )
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		pRole->CheckMapArea();

		pRole->get_map()->update_big_visible_tile_lurk_unit_to_role(pRole);
	}
}

//-------------------------------------------------------------------------------------
// 开始移动
//-------------------------------------------------------------------------------------
VOID MoveData::OnStartMove()
{
	if( EMS_Stand == m_eCurMove )
		return;

	// 检测技能起手打断
	m_pOwner->GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Move, FALSE);

	// 检测技能施放打断
	m_pOwner->GetCombatHandler().InterruptOperate(CombatHandler::EIT_Move, m_eCurMove);

	// 检测buff打断
	m_pOwner->OnInterruptBuffEvent(EBIF_Move);

	// 打断钓鱼
	if(m_pOwner->IsRole())static_cast<Role*>(m_pOwner)->StopFishing();
}

//-------------------------------------------------------------------------------------
// 地图是否合法
//-------------------------------------------------------------------------------------
BOOL MoveData::IsMapValid()
{
	// 如果当前没有地图或者地图没有导航图，则直接返回
	Map* pMap = m_pOwner->get_map();
	if( FALSE == VALID_POINT(pMap) ) return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------------------
// 检测距离
//-------------------------------------------------------------------------------------
BOOL MoveData::IsInValidDistance(const Vector3& vStart, BOOL b_jump_/* = FALSE*/)
{
	FLOAT fDistanceSQ = pt_distance(pathNode(m_vPos.x, m_vPos.z), pathNode(vStart.x, m_vPos.z));
	fDistanceSQ = fDistanceSQ * fDistanceSQ;

	if(m_pOwner->IsRole())
	{
		Role* pRole = (Role*)m_pOwner;
		if(pRole->IsGM())
			return TRUE;

		if(((Role*)m_pOwner)->IsInRoleStateAny(ERS_Mount | ERS_Mount2))
		{
			FLOAT fChectSQ = 0.0f;
			if(b_jump_)
			{
				fChectSQ = g_world.GetStrictJumpCheatDistanceSQ();
			}
			else
			{
				fChectSQ = g_world.GetStrictCheatDistanceSQ();
			}

			if(fDistanceSQ > fChectSQ)
			{
				pRole->inc_ce_delay_num();
				return TRUE;
			}
		}
		else
		{
			FLOAT fChectSQ = 0.0f;
			if(b_jump_)
			{
				fChectSQ = g_world.GetLaxJumpCheatDistanceSQ();
			}
			else
			{
				fChectSQ = g_world.GetLaxCheatDistanceSQ();
			}

			if( fDistanceSQ > fChectSQ )
			{
				pRole->inc_ce_delay_num();
				return TRUE;
			}
		}


		// 防止能穿门, 判断一下射线
		//Map* pMap = m_pOwner->get_map();
		//if( VALID_POINT(pMap) && VALID_POINT(pMap->get_nav_map()) /*&& fDistanceSQ > (100.0 * 100.0)*/)
		//{
		//	Vector3 pos1 = m_pOwner->GetCurPosTop();
		//	Vector3 pos2 = vStart;
		//	pos2.y = pos1.y;
		//	if( pMap->is_ray_collide(pos1, pos2) )
		//	{
		//		return FALSE;
		//	}
		//}
	}
	else
	{
		if( fDistanceSQ > g_world.GetLaxCheatDistanceSQ() )
		{
			return FALSE;
		}
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------
// 玩家是否可以行走
//-------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleWalk(const Vector3& vStart, const Vector3& vEnd)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检测一下坐标
	if( IsPosInvalid(vStart) || IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to walk, start=<%f, %f, %f>, end=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vEnd.x, vEnd.y, vEnd.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) || !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;


	// 检测一下距离
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	// 检测一下速度
	if( m_pOwner->GetXZSpeed() <= 0.0f )
	{
		SI_LOG->write_log(_T("find a role which have a xz speed less than 0, ID: %u, Speed: %f!!!!!!!!!\r\n"), 
			m_pOwner->GetID(), m_pOwner->GetXZSpeed());
		return EMR_SelfLimit;
	}

	// 检测当前是否处在不能移动的状态
	if( m_pOwner->IsInStateCantMove() )
	{
		return EMR_SelfLimit;
	}

	// 判断目标点是否能到
	if (!m_pOwner->get_map()->if_can_go(vEnd.x, vEnd.z))
	{
		return EMR_NoArrive;
	}
	
	//if (m_pOwner->get_map()->is_have_unit(vEnd))
	//{
	//	return EMR_NoArrive;
	//}

	return EMR_Success;
}

//----------------------------------------------------------------------------------------
// 玩家是否可以跳跃
//----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanJump(const Vector3& vStart, const Vector3& vDir)
{

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检测一下坐标和朝向
	if( IsPosInvalid(vStart) || IsFaceInvalid(vDir) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to jump, start=<%f, %f, %f>, dir=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vDir.x, vDir.y, vDir.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vStart, TRUE) )
	{
		return EMR_Invalid;
	}

	// 检测一下速度
	if( m_pOwner->GetXZSpeed() <= 0.0f || m_pOwner->m_fYSpeed <= 0.0f )
	{
		print_message(_T("find a role whose xz speed or y speed less than 0, ID: %u, xzspeed: %f, yspeed: %f\r\n"), 
			m_pOwner->GetID(), m_pOwner->GetXZSpeed(), m_pOwner->m_fYSpeed);
		return EMR_SelfLimit;
	}

	// 检测当前是否处在不能移动的状态
	if( m_pOwner->IsInStateCantMove() )
	{
		return EMR_SelfLimit;
	}

	if(m_pOwner->IsRole())
	{
		if(static_cast<Role*>(m_pOwner)->IsInRoleState(ERS_Carry))
			return EMR_SelfLimit;
	}
	//Role* pRole = dynamic_cast<Role*>(m_pOwner);
	// [2011.01.06 zhaopeng] 让骑乘状态能跳跃
	//if (VALID_POINT(pRole) && pRole->IsInRoleState(ERS_Mount))
	//{
	//	return EMR_SelfLimit;
	//}

	return EMR_Success;
}

//----------------------------------------------------------------------------------------
// 玩家是否可以掉落
//----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleDrop(const Vector3& vStart, const Vector3& vDir)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检测一下坐标和朝向
	if( IsPosInvalid(vStart) || IsFaceInvalid(vDir) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to drop, start=<%f, %f, %f>, dir=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vDir.x, vDir.y, vDir.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------
// 是否可以垂直掉落
//-----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleVDrop(const Vector3& vStart)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检测一下坐标
	if( IsPosInvalid(vStart) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to vDrop, start=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//------------------------------------------------------------------------------------------
// 是否可以滑落
//------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleSlide(const Vector3& vStart)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检测一下坐标
	if( IsPosInvalid(vStart) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to vdrop, start=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//---------------------------------------------------------------------------------------------
// 是否可以游泳
//---------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleSwim(const Vector3& vStart, const Vector3& vEnd)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	if( IsPosInvalid(vStart) || IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to swim, start=<%f, %f, %f>, end=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vEnd.x, vEnd.y, vEnd.z);
		return EMR_Invalid;

	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vStart) || !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	// 检测一下游泳的速度
	if( m_pOwner->m_fSwimXZSpeed <= 0.0f )
	{
		return EMR_SelfLimit;
	}

	// 检测当前是否处在不能移动的状态
	if( m_pOwner->IsInStateCantMove() )
	{
		return EMR_SelfLimit;
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// 玩家是否可以停下
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleStop(const Vector3& vPos)
{
	// 如果不是玩家，返回
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检查一下坐标
	if( IsPosInvalid(vPos) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to stop, pos=<%f, %f, %f>\r\n"),
			vPos.x, vPos.y, vPos.z);
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vPos) )
		return EMR_Invalid;

	// 检测一下距离
	if( !IsInValidDistance(vPos) )
	{
		return EMR_Invalid;
	}
	
	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// 怪物是否可以行走
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanCreatureWalk(const Vector3& vEnd, EMoveState eState, BOOL bCheck, Vector3* vNearPos)
{
	// 如果不是怪物，返回
	if( !m_pOwner->IsCreature() ) return EMR_Invalid;

	// 地图非法
	if( !IsMapValid() ) return EMR_Invalid;

	// 检查一下坐标
	if( IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("creature pos invalid when try to walk, dest=<%f, %f, %f>\r\n creature typeid=%d id=%d\r\n"),
			vEnd.x, vEnd.y, vEnd.z, ((Creature*)m_pOwner)->GetProto()->dw_data_id, ((Creature*)m_pOwner)->GetID());
		return EMR_Invalid;
	}

	// 地图判断坐标
	if( !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;

	// 检测一下速度
	FLOAT fRealSpeed = GetCreatureMoveStateSpeed(eState);

	if( fRealSpeed < 0.0001f )
	{
		return EMR_SelfLimit;
	}

	// 如果需要检测路径
	if( bCheck )
	{
		// 检测当前是否处在不能移动的状态
		if( m_pOwner->IsInStateCantMove())
		{
			return EMR_SelfLimit;
		}

		Creature* pCreature = static_cast<Creature*>(m_pOwner);

		pathNode nearPos;
		if( !pCreature->get_map()->if_can_direct_go(m_vPos.x, m_vPos.z, vEnd.x, vEnd.z, nearPos) )
		{
			if( VALID_POINT(vNearPos) )
			{
				vNearPos->x = nearPos.x();
				vNearPos->z = nearPos.y();
			}
			return EMR_NoArrive;
		}
		else
		{

		}
		
	}

	return EMR_Success;
}

MoveData::EMoveRet MoveData::IsCanHitFly()
{
	return EMR_Success;
}
//-------------------------------------------------------------------------------------------
// 根据怪物的移动方式的到当前速度
//-------------------------------------------------------------------------------------------
FLOAT MoveData::GetCreatureMoveStateSpeed(EMoveState eState)
{
	switch (eState)
	{
	case EMS_CreatureWalk:
		return m_pOwner->GetXZSpeed();

	case EMS_CreaturePatrol:
		return m_pOwner->GetXZSpeed() * 0.4;

	case EMS_CreatureFlee:
		return m_pOwner->GetXZSpeed() * 0.7;
	
	case EMS_CreatureRun:
		return m_pOwner->GetXZSpeed() * 1.5;

	default:
		return m_pOwner->GetXZSpeed();
	}
}

MoveData::EMoveRet MoveData::StartFearRun( const Vector3& vDest )
{	
	EMoveRet eRet;
	if (m_pOwner->IsRole())
	{	
		eRet = IsCanRoleWalk(m_vPos, vDest);

		if( EMR_Success == eRet )
		{
			GetColliderWalk()->Init(m_vPos, vDest, m_pOwner->GetXZSpeed());

			m_eCurMove			=	EMS_Walk;
			m_eNextPreMove		=	EMS_Stand;
			m_vPosStart			=	m_vPos;
			m_vDest				=	vDest;
			m_vDir				=	vDest - m_vPos;
			m_bWaitClientMsg	=	FALSE;
			m_fStartTime		=	m_Timer.GetElapse();
			m_bIsStopped		=	FALSE;

			SetFaceTo(m_vDir);
			OnStartMove();
			OnPosChange(vDest);
			m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
		}


	}
	else
	{

		eRet = IsCanCreatureWalk(vDest, EMS_Fear, TRUE);

		if( EMR_Success == eRet )
		{
			GetColliderWalk()->Init(m_vPos, vDest, m_pOwner->GetXZSpeed());

			m_eCurMove			=	EMS_CreatureWalk;
			m_eNextPreMove		=	EMS_Stand;
			m_vPosStart			=	m_vPos;
			m_vDest				=	vDest;
			m_vDir				=	vDest - m_vPos;
			m_bWaitClientMsg	=	FALSE;
			m_fStartTime		=	m_Timer.GetElapse();
			m_bIsStopped		=	FALSE;

			SetFaceTo(m_vDir);
			OnStartMove();
			OnPosChange(vDest);
			m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
		}

	}


	return eRet;
}

Vector3	MoveData::GetNearPos(const Vector3 &vTargetPos, const Vector3 &vTargetFace, FLOAT fMaxRange, FLOAT fMinRange)
{
	//随机选择一个角度
	FLOAT fAngle	= (get_tool()->tool_rand() % 360) / 360.0f * 3.14f;
	//随机选择一个合理的范围
	FLOAT fDist		= get_tool()->tool_rand() % INT(fMaxRange - fMinRange) + fMinRange;
	//计算目标点
	Vector3 vRt		= vTargetPos;
	vRt.x +=	sin(fAngle) * fDist;
	vRt.z +=	cos(fAngle) * fDist;
	return vRt;
}

VOID MoveData::OnPosDrift(const Vector3& vDir, const Vector3& vFrom, const Vector3& vTo)
{
    Vector3 vTemp1 = vDir;
    Vector3 vTemp2 = vTo - vFrom;
    vTemp1.y = vTemp2.y = 0;

    Vec3Normalize( vTemp1 );

    float dot_val = D3DXVec3Dot(&vTemp1, &vTemp2);

    m_fCheatAccumulate += dot_val;
    if ( m_fCheatAccumulate < 0.0f )
        m_fCheatAccumulate = 0.0f;
}

BOOL MoveData::CheckSpeedCheat()
{
    return m_fCheatAccumulate > 50 * TILE_SCALE;
}

VOID MoveData::GetCheatAccumulate(FLOAT& val)
{
    val = m_fCheatAccumulate;
}
