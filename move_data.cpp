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
// ��̬����
//---------------------------------------------------------------------------------------------
Timer MoveData::m_Timer;

//---------------------------------------------------------------------------------------------
// �����ƶ�������
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

	// �����������
	FLOAT fDistanceX = m_vDest.x - m_vStart.x;
	FLOAT fDistanceZ = m_vDest.z - m_vStart.z;
	FLOAT fDistanceSquare = fDistanceX * fDistanceX + fDistanceZ * fDistanceZ;

	// ������ʱ�����������
	m_fTotalTime = get_fast_code()->fast_square_root(fDistanceSquare) / m_fXZSpeed;
	m_nMoveTicks = (INT)(m_fTotalTime * 1000.0f) / TICK_TIME;
	m_nMoveTicks += ( (INT(m_fTotalTime * 1000.0f) - TICK_TIME * m_nMoveTicks) > 0 ? 1 : 0 );

	// ��ǰ�ƶ�ʱ��
	m_fCurTime = 0.0f;
}

ENavResult NavCollider_TileMove::Update(Vector3& vOutPos)
{
	// �������Ͳ����ƶ�
	if( m_nMoveTicks <= 0 )
	{
		vOutPos = m_vStart;
		return ENR_Arrive;
	}

	// ��ȥtick
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
// �������
//---------------------------------------------------------------------------------------------
BOOL MoveData::IsPosInvalid(const Vector3& vPos)
{
	// ����ǲ���ʵ��
	if( _isnan(vPos.x) || _isnan(vPos.y) || _isnan(vPos.z) )
		return TRUE;

	// ����ǲ�������
	if( 0 == _finite(vPos.x) || 0 == _finite(vPos.y) || 0 == _finite(vPos.z) )
	{
		return TRUE;
	}

	return FALSE;	
}

//---------------------------------------------------------------------------------------------
// ��鳯��
//---------------------------------------------------------------------------------------------
BOOL MoveData::IsFaceInvalid(const Vector3& vFace)
{
	// ����ǲ���ʵ��
	if( _isnan(vFace.x) || _isnan(vFace.y) || _isnan(vFace.z) )
		return TRUE;

	// ����ǲ�������
	if( 0 == _finite(vFace.x) || 0 == _finite(vFace.y) || 0 == _finite(vFace.z) )
	{
		return TRUE;
	}

	// �������һ����鿴������
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
// ��ʼ���ƶ��ṹ����Ҫ������Һ������ʼ��ʱ���ƶ�״̬
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
// �����ƶ��ṹ������Ҫ��������л���ͼ�͹�������ʱ��һ�������
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
// ��������
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
// �������ߣ�bPatrol�����Ƿ�Ѳ�ߣ�bCheck�����Ƿ���·����
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
// ����ֹͣ�ƶ�
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
// ������ֹͣ�ƶ���һ�����ڼ��ٵ������������ڲ������ֹͣ�ƶ���״̬����ֹͣ
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMove(BOOL bSend)
{
	// ����ֹͣ���ƶ�
	if( 
		EMS_HitFly	==	m_eCurMove	)
		return EMR_Conflict;

	// ��������վ����Ư��״̬
	if( EMS_Stand == m_eCurMove  )
		return EMR_Success;

	// �������ƶ�������Ӿ�ֱ��л���վ����Ư��
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

	// ͬ������Χ���
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// ֹͣ�ƶ���������һ���µķ���
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMove(const Vector3& vNewFace, BOOL bSend)
{
	// ���һ�³���
	if( IsFaceInvalid(vNewFace) )
	{
		SI_LOG->write_log(_T("face invalid when try to rotate, face=<%f, %f, %f>\r\n"),
					vNewFace.x, vNewFace.y, vNewFace.z);
		return EMR_Invalid;
	}

	// ����ֹͣ���ƶ�
	if( 
		EMS_HitFly	==  m_eCurMove)
		return EMR_Conflict;

	// ��������վ����Ư��״̬
	if( EMS_Stand == m_eCurMove  )
		return EMR_Success;

	// �������ƶ�������Ӿ�ֱ��л���վ����Ư��
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

	// ͬ������Χ���
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// ������ǿ��ֹͣ�ƶ�
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::StopMoveForce(BOOL bSend)
{
	// ��������վ����Ư��״̬
	if( EMS_Stand == m_eCurMove )
		return EMR_Success;

	// ��������ƶ�����Ӿ���䣬�������л�����ǰ״̬
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

		// ͬ������Χ���
		if( bSend )
		{
			m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
		}

		return EMR_Success;
	}

	// ���������Ծ���������
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
// ˲�Ƶ�ĳ��λ��վ��
//-----------------------------------------------------------------------------------------------
VOID MoveData::ForceTeleport(const Vector3& vPos, BOOL bSend)
{
	DropDownStandPoint(vPos, bSend);
}

//-----------------------------------------------------------------------------------------------
// �����ƶ�
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
// ������������
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateRoleWalk()
{
	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// ��������
	Vector3 vOutPos;
	float fEndTime = 0.0f;
	DWORD dwCarrierID = INVALID_VALUE;

	ENavResult eResult = GetColliderWalk()->Update(vOutPos);
	
	OnPosChange(vOutPos);

	if( ENR_ToBeContinued != eResult )
	{
		// ��ǰ�ƶ�ֹͣ
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
// ���¹�������
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateCreatureWalk()
{
	Creature* pCreature = static_cast<Creature*>(m_pOwner);

	// ��������
	Vector3 vOutPos;
	FLOAT fEndTime = 0.0f;
	DWORD dwCarrierID = INVALID_VALUE;

	ENavResult eResult = ENR_ToBeContinued;

	eResult = GetColliderWalk()->Update(vOutPos);
	
	OnPosChange(vOutPos);

	// ��鷵��ֵ
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
// ���±�����
//-----------------------------------------------------------------------------------------------
VOID MoveData::UpdateHitFly()
{

}


//-----------------------------------------------------------------------------------------------
// ���ݵ�ǰ�������£��ҵ�һ����ײλ��վ�������ܻᵽ��Ӿ
//------------------------------------------------------------------------------------------------
VOID MoveData::DropDownStandPoint(BOOL bSend)
{
	m_vPos.y = 0;


	BOOL bWaterWalk = FALSE;	// �Ƿ�ˮ������

	if( m_pOwner->IsRole() )
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		bWaterWalk = pRole->IsInRoleState(ERS_WaterWalk);
	}


	// վ��
	m_eCurMove		=	EMS_Stand;
	m_eNextPreMove	=	EMS_Stand;
	
	m_vPosStart			=		m_vPos;
	m_vDest				=		m_vPos;
	m_bWaitClientMsg	=		FALSE;

	// ͬ������Χ���
	if( bSend )
	{
		m_pOwner->get_map()->synchronize_movement_to_big_visible_tile(m_pOwner);
	}
}

//-------------------------------------------------------------------------------------
// ����һ�����������£��ҵ�һ����ײλ��վ�������ܻᵽ��Ӿ
//-------------------------------------------------------------------------------------
VOID MoveData::DropDownStandPoint(const Vector3& vPos, BOOL bSend)
{
	// ���һ������
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
// λ�øı�
//-------------------------------------------------------------------------------------
VOID MoveData::OnPosChange(const Vector3& vNewPos)
{
	// ���������⺯��
	if( IsPosInvalid(vNewPos) )
	{
		SI_LOG->write_log(_T("Invalid Pos, Current movestate: %d\r\n"), m_eCurMove);
	}

	// ��������
	Vector3 vPos = vNewPos;
	m_pOwner->get_map()->fix_position(vPos);
	//m_pOwner->get_map()->set_have_unit(m_vPos, false);
	//m_pOwner->get_map()->set_have_unit(vPos, true);

	m_vPos = vPos;	// ����Ϊ������

	INT nNewVisIndex = m_pOwner->get_map()->world_position_to_visible_index(m_vPos);
	if( m_nVisTileIndex != nNewVisIndex )
	{
		m_pOwner->get_map()->synchronize_change_visible_tile(m_pOwner, m_nVisTileIndex, nNewVisIndex);
	}

	// ���������λ����ͬ������Χ���
	if( m_pOwner->IsInStateInvisible() )
	{
		m_pOwner->get_map()->update_lurk_to_big_visible_tile_role(m_pOwner);
	}

	// �������ң������ͼ���򣬼�����Χ�������ͬ������
	if( m_pOwner->IsRole() )
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		pRole->CheckMapArea();

		pRole->get_map()->update_big_visible_tile_lurk_unit_to_role(pRole);
	}
}

//-------------------------------------------------------------------------------------
// ��ʼ�ƶ�
//-------------------------------------------------------------------------------------
VOID MoveData::OnStartMove()
{
	if( EMS_Stand == m_eCurMove )
		return;

	// ��⼼�����ִ��
	m_pOwner->GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Move, FALSE);

	// ��⼼��ʩ�Ŵ��
	m_pOwner->GetCombatHandler().InterruptOperate(CombatHandler::EIT_Move, m_eCurMove);

	// ���buff���
	m_pOwner->OnInterruptBuffEvent(EBIF_Move);

	// ��ϵ���
	if(m_pOwner->IsRole())static_cast<Role*>(m_pOwner)->StopFishing();
}

//-------------------------------------------------------------------------------------
// ��ͼ�Ƿ�Ϸ�
//-------------------------------------------------------------------------------------
BOOL MoveData::IsMapValid()
{
	// �����ǰû�е�ͼ���ߵ�ͼû�е���ͼ����ֱ�ӷ���
	Map* pMap = m_pOwner->get_map();
	if( FALSE == VALID_POINT(pMap) ) return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------------------
// ������
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


		// ��ֹ�ܴ���, �ж�һ������
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
// ����Ƿ��������
//-------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleWalk(const Vector3& vStart, const Vector3& vEnd)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������
	if( IsPosInvalid(vStart) || IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to walk, start=<%f, %f, %f>, end=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vEnd.x, vEnd.y, vEnd.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) || !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;


	// ���һ�¾���
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	// ���һ���ٶ�
	if( m_pOwner->GetXZSpeed() <= 0.0f )
	{
		SI_LOG->write_log(_T("find a role which have a xz speed less than 0, ID: %u, Speed: %f!!!!!!!!!\r\n"), 
			m_pOwner->GetID(), m_pOwner->GetXZSpeed());
		return EMR_SelfLimit;
	}

	// ��⵱ǰ�Ƿ��ڲ����ƶ���״̬
	if( m_pOwner->IsInStateCantMove() )
	{
		return EMR_SelfLimit;
	}

	// �ж�Ŀ����Ƿ��ܵ�
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
// ����Ƿ������Ծ
//----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanJump(const Vector3& vStart, const Vector3& vDir)
{

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������ͳ���
	if( IsPosInvalid(vStart) || IsFaceInvalid(vDir) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to jump, start=<%f, %f, %f>, dir=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vDir.x, vDir.y, vDir.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vStart, TRUE) )
	{
		return EMR_Invalid;
	}

	// ���һ���ٶ�
	if( m_pOwner->GetXZSpeed() <= 0.0f || m_pOwner->m_fYSpeed <= 0.0f )
	{
		print_message(_T("find a role whose xz speed or y speed less than 0, ID: %u, xzspeed: %f, yspeed: %f\r\n"), 
			m_pOwner->GetID(), m_pOwner->GetXZSpeed(), m_pOwner->m_fYSpeed);
		return EMR_SelfLimit;
	}

	// ��⵱ǰ�Ƿ��ڲ����ƶ���״̬
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
	// [2011.01.06 zhaopeng] �����״̬����Ծ
	//if (VALID_POINT(pRole) && pRole->IsInRoleState(ERS_Mount))
	//{
	//	return EMR_SelfLimit;
	//}

	return EMR_Success;
}

//----------------------------------------------------------------------------------------
// ����Ƿ���Ե���
//----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleDrop(const Vector3& vStart, const Vector3& vDir)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������ͳ���
	if( IsPosInvalid(vStart) || IsFaceInvalid(vDir) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to drop, start=<%f, %f, %f>, dir=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vDir.x, vDir.y, vDir.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------
// �Ƿ���Դ�ֱ����
//-----------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleVDrop(const Vector3& vStart)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������
	if( IsPosInvalid(vStart) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to vDrop, start=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//------------------------------------------------------------------------------------------
// �Ƿ���Ի���
//------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleSlide(const Vector3& vStart)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������
	if( IsPosInvalid(vStart) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to vdrop, start=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	return EMR_Success;
}

//---------------------------------------------------------------------------------------------
// �Ƿ������Ӿ
//---------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleSwim(const Vector3& vStart, const Vector3& vEnd)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	if( IsPosInvalid(vStart) || IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to swim, start=<%f, %f, %f>, end=<%f, %f, %f>\r\n"),
			vStart.x, vStart.y, vStart.z, vEnd.x, vEnd.y, vEnd.z);
		return EMR_Invalid;

	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vStart) || !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vStart) )
	{
		return EMR_Invalid;
	}

	// ���һ����Ӿ���ٶ�
	if( m_pOwner->m_fSwimXZSpeed <= 0.0f )
	{
		return EMR_SelfLimit;
	}

	// ��⵱ǰ�Ƿ��ڲ����ƶ���״̬
	if( m_pOwner->IsInStateCantMove() )
	{
		return EMR_SelfLimit;
	}

	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// ����Ƿ����ͣ��
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanRoleStop(const Vector3& vPos)
{
	// ���������ң�����
	if( !m_pOwner->IsRole() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������
	if( IsPosInvalid(vPos) )
	{
		SI_LOG->write_log(_T("client pos invalid when try to stop, pos=<%f, %f, %f>\r\n"),
			vPos.x, vPos.y, vPos.z);
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vPos) )
		return EMR_Invalid;

	// ���һ�¾���
	if( !IsInValidDistance(vPos) )
	{
		return EMR_Invalid;
	}
	
	return EMR_Success;
}

//-----------------------------------------------------------------------------------------------
// �����Ƿ��������
//-----------------------------------------------------------------------------------------------
MoveData::EMoveRet MoveData::IsCanCreatureWalk(const Vector3& vEnd, EMoveState eState, BOOL bCheck, Vector3* vNearPos)
{
	// ������ǹ������
	if( !m_pOwner->IsCreature() ) return EMR_Invalid;

	// ��ͼ�Ƿ�
	if( !IsMapValid() ) return EMR_Invalid;

	// ���һ������
	if( IsPosInvalid(vEnd) )
	{
		SI_LOG->write_log(_T("creature pos invalid when try to walk, dest=<%f, %f, %f>\r\n creature typeid=%d id=%d\r\n"),
			vEnd.x, vEnd.y, vEnd.z, ((Creature*)m_pOwner)->GetProto()->dw_data_id, ((Creature*)m_pOwner)->GetID());
		return EMR_Invalid;
	}

	// ��ͼ�ж�����
	if( !m_pOwner->get_map()->is_position_valid(vEnd) )
		return EMR_Invalid;

	// ���һ���ٶ�
	FLOAT fRealSpeed = GetCreatureMoveStateSpeed(eState);

	if( fRealSpeed < 0.0001f )
	{
		return EMR_SelfLimit;
	}

	// �����Ҫ���·��
	if( bCheck )
	{
		// ��⵱ǰ�Ƿ��ڲ����ƶ���״̬
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
// ���ݹ�����ƶ���ʽ�ĵ���ǰ�ٶ�
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
	//���ѡ��һ���Ƕ�
	FLOAT fAngle	= (get_tool()->tool_rand() % 360) / 360.0f * 3.14f;
	//���ѡ��һ������ķ�Χ
	FLOAT fDist		= get_tool()->tool_rand() % INT(fMaxRange - fMinRange) + fMinRange;
	//����Ŀ���
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
