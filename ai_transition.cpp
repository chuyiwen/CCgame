/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
/**
*	@file		ai_transition.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		AI״̬ת����
*				�����ܡ�����״̬ת�������Ρ�ͬʱ���ݸ��ݵ�����Ϊ��ȷ���Ƿ�����Щ״̬
*				���Ӿ���AI������AI�� [ 2010-04-01:10-52 ]

*/

#include "stdafx.h"
#include "ai_transition.h"
#include "creature_ai.h"
#include "creature.h"

AITransition* AggressiveTransition::Instance()
{
	static AggressiveTransition instance;
	return &instance;
}


AIStateType AITransition::TransitionToFlee(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();

	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_Flee )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				DWORD	dwFleeTick = get_tool()->tool_rand() % 50 + 100;
				pAI->SetFleeTick(dwFleeTick);
				return AIST_Flee;
			}
		}
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// �����͹���״̬ת��
//------------------------------------------------------------------------------------------
AIStateType	AggressiveTransition::Transition(AIController* pAI, AIStateType eCurState)
{
	switch(eCurState)
	{
	case AIST_Idle:
		return IdleTransition(pAI);

	case AIST_Pursue:
		return PursueTransition(pAI);

	case AIST_Attack:
		return AttackTransition(pAI);

	case AIST_Return:
		return ReturnTransition(pAI);

	case AIST_SOS:
		return CallForHelpTransition(pAI);

	case AIST_Flee:
		return FleeTransition(pAI);

	default:
		return eCurState;
	}
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::IdleTransition(AIController* pAI)
{
	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pursue;
	}

	// �����ǰĿ���ھ��䷶Χ֮����, ������뾯�䷶Χ��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// ����ڹ�����Χ�ڣ�������빥����Χ��
		return AIST_Attack;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// ׷��״̬ת��
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::PursueTransition(AIController* pAI)
{
	// �������target�Ƿ������л��ɷ���״̬
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// ���û��Ŀ����
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_Flee )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				DWORD	dwFleeTick = get_tool()->tool_rand() % 50 + 100;
				pAI->SetFleeTick(dwFleeTick);
				return AIST_Flee;
			}
		}
	}

	// �����ǰĿ�겻�ڹ�����Χ֮����
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType AggressiveTransition::CallForHelpTransition(AIController *pAI)
{
		// ����״̬����ʱ�����
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType AggressiveTransition::FleeTransition(AIController* pAI)
{
	// ����״̬����ʱ�����
	if( pAI->GetFleeTick() <= 0 )
	{
		return AIST_Pursue;
	}

	return AIST_Flee;
}

AITransition* GuardTransition::Instance()
{
	static GuardTransition instance;
	return &instance;
}

//------------------------------------------------------------------------------------------
// �����͹���״̬ת��
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::Transition(AIController* pAI, AIStateType eCurState)
{
	switch(eCurState)
	{
	case AIST_Idle:
		return IdleTransition(pAI);

	case AIST_Pursue:
		return PursueTransition(pAI);

	case AIST_Attack:
		return AttackTransition(pAI);

	case AIST_Return:
		return ReturnTransition(pAI);

	case AIST_SOS:
		return CallForHelpTransition(pAI);

	case AIST_Flee:
		return FleeTransition(pAI);

	default:
		return eCurState;
	}
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::IdleTransition(AIController* pAI)
{
	AIStateType _state = TransitionToFlee(pAI);
	if (_state != AIST_Idle)
	{
		return _state;
	}

	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pursue;
	}

	if( pAI->GetTargetUnitID() == INVALID_VALUE )
		return AIST_Idle;

	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// ����ڹ�����Χ�ڣ�������빥����Χ��
		return AIST_Pursue;
	}
	

	return AIST_Idle;

}

AIStateType GuardTransition::PursueTransition(AIController* pAI)
{
	AIStateType _state = TransitionToFlee(pAI);
	if (_state != AIST_Idle)
	{
		return _state;
	}

	// �������target�Ƿ������л��ɷ���״̬
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		OutputDebugString( L"guai wu tao pao\n" );
		return AIST_Return;
	}

	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();

	
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// ���û��Ŀ����
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_Flee )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				DWORD	dwFleeTick = get_tool()->tool_rand() % 50 + 100;
				pAI->SetFleeTick(dwFleeTick);
				return AIST_Flee;
			}
		}
	}

	// ���Ŀ�겻�ڹ�����Χ���ˣ���������ĳ��
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType GuardTransition::CallForHelpTransition(AIController *pAI)
{
		// ����״̬����ʱ�����
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType GuardTransition::FleeTransition(AIController* pAI)
{
	// ����״̬����ʱ�����
	if( pAI->GetFleeTick() <= 0 )
	{
		return AIST_Pursue;
	}

	return AIST_Flee;
}


AITransition* BarbetteTransition::Instance()
{
	static BarbetteTransition instance;
	return &instance;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::Transition(AIController* pAI, AIStateType eCurState)
{
	switch(eCurState)
	{
	case AIST_Idle:
		return IdleTransition(pAI);

	case AIST_Attack:
		return AttackTransition(pAI);
	
	case AIST_Return:
		return ReturnTransition(pAI);

	default:
		return eCurState;
	}
}

//------------------------------------------------------------------------------------------
// �����Ϳ���״̬ת��
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::IdleTransition(AIController* pAI)
{
	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		return AIST_Attack;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// �����͹���״̬ת��
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();

	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// ���û��Ŀ����
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// �����ǰĿ�겻�ڷ�Χ�ڣ����������
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		pAI->SetEnmityActive(pAI->GetTargetUnitID(), FALSE);
		return AIST_Attack;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}
// ������״̬ת��
//------------------------------------------------------------------------------------------
AITransition* SpaceOutTransition::Instance()
{
	static SpaceOutTransition instance;
	return &instance;
}

AIStateType	SpaceOutTransition::Transition(AIController* pAI, AIStateType eCurState)
{
	switch(eCurState)
	{
	case AIST_Idle:
		return IdleTransition(pAI);

	case AIST_Pursue:
		return PursueTransition(pAI);

	case AIST_Attack:
		return AttackTransition(pAI);

	case AIST_Return:
		return ReturnTransition(pAI);

	case AIST_SpaceOut:
		return SpaceTransition(pAI);

	default:
		return eCurState;
	}
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::IdleTransition(AIController* pAI)
{
	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		return AIST_Pursue;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// ׷��״̬ת��
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::PursueTransition(AIController* pAI)
{
	// �������target�Ƿ������л��ɷ���״̬
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// ����״̬ת��
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	Unit* pTarget = pAI->GetPursueTarget();

	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// ���û��Ŀ����
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// �����ǰĿ�겻�ڹ�����Χ֮����
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	// ���㱣�־��봥�����
	DWORD dwCombatTick = g_world.GetWorldTick() - pAI->GetEnterCombatTick();
	if(0 == dwCombatTick % pAI->GetSpaceOutTick() )
	{
		if( pOwner->IsInDistance(*pTarget, CREATURE_SPACE_OUT_CHANGE_DIST) )
		{
			if( pAI->CalSpaceOutPos() )
				return AIST_SpaceOut;
		}
	}

	return AIST_Attack;
}

AIStateType SpaceOutTransition::SpaceTransition(AIController* pAI)
{
	if( pAI->IsArrivedPos())
		return AIST_Attack;

	return AIST_SpaceOut;
}

AIStateType SpaceOutTransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}

//-----------------------------------------------------------------------------
// ���﹥����״̬ת����
// 
//-----------------------------------------------------------------------------
AITransition* PetATTransition::Instance()
{
	static PetATTransition instance;
	return &instance;
}


AIStateType	PetATTransition::Transition(AIController* pAI, AIStateType eCurState)
{
	switch(eCurState)
	{
	case AIST_PetIdle:
		return IdleTransition(pAI);

	case AIST_PetAttack:
		return AttackTransition(pAI);

	case AIST_Pet_Pursue:
		return PursueTransition(pAI);

	default:
		return AIST_PetIdle;
	}
}

AIStateType PetATTransition::PursueTransition(AIController* pAI)
{
	// �������target�Ƿ������л�������״̬
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_PetIdle;
	}

	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_PetAttack;
	}

	return AIST_Pet_Pursue;
}

AIStateType PetATTransition::IdleTransition(AIController* pAI)
{
	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pet_Pursue;
	}

	// �����ǰĿ���ھ��䷶Χ֮����, ������뾯�䷶Χ��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// ����ڹ�����Χ�ڣ�������빥����Χ��
		return AIST_PetAttack;
	}

	return AIST_PetIdle;

	// ����
	//if( pAI->IsTargetValid(pAI->GetTargetUnitID()) )
	//{
	//	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	//	{
	//		return AIST_PetAttack;
	//	}
	//	
	//	else
	//	{
	//		return AIST_Pet_Pursue;
	//	}
	//}

	//return AIST_PetIdle;
}


AIStateType PetATTransition::AttackTransition(AIController* pAI)
{

	Creature* pOwner = pAI->GetOwner();
	
	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_PetAttack;
	}

	// ���û��Ŀ����
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_PetIdle;
	}

	// �����ǰĿ�겻�ڹ�����Χ֮����
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pet_Pursue; 
	}

	return AIST_PetAttack;
}



//------------------------------------------------------------------------------
// ����AI�����״̬ת����
// 
//------------------------------------------------------------------------------
AITransition* AlertAITransition::Instance( )
{
	static AlertAITransition instance;
	return &instance;
}


AIStateType AlertAITransition::Transition( AIController* pAI, AIStateType eCurState )
{
	switch( eCurState )
	{
	case AIST_Idle:
		return IdleTransition( pAI );

	case AIST_Pursue:
		return PursueTransition( pAI );

	case AIST_Attack:
		return AttackTransition( pAI );

	case AIST_Return :
		return ReturnTransition( pAI );

	case AIST_SOS:
		return CallForHelpTransition(pAI);

	case AIST_Flee:
		return FleeTransition(pAI);

	case AIST_Alert:
		return AlertTransition( pAI );
	}

	return AIST_Idle;
}


AIStateType AlertAITransition::IdleTransition(AIController* pAI)
{
	
	// ���û��Ŀ����, ��������
	if( !VALID_VALUE(pAI->GetTargetUnitID()) ) 
	{
		return AIST_Idle;
	}

	// �����ǰĿ���ھ��䷶Χ֮����, ������뾯�䷶Χ��
	if( pAI->IsInAlertDist( pAI->GetTargetUnitID() ) )
	{
		return AIST_Alert; 
	}

	if( VALID_VALUE(pAI->GetTargetUnitID()) && pAI->IsInspiration( pAI->GetTargetUnitID() ) )
	{
		return AIST_Pursue;
	}

	return AIST_Idle;
}

AIStateType AlertAITransition::PursueTransition(AIController* pAI)
{
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
		return AIST_Return;


	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;

}


// ������ڹ���״̬��
AIStateType AlertAITransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// ���û��Ŀ����, ��������
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// ��ǰ����С�ڻ�������20��
		if( pAI->m_ePursueType == ECPST_Flee )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				DWORD	dwFleeTick = get_tool()->tool_rand() % 50 + 100;
				pAI->SetFleeTick(dwFleeTick);
				return AIST_Flee;
			}
		}
	}

	// ����ڹ�����Χ֮�⣬��׷������
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	// ��������
	return AIST_Attack;
}

AIStateType AlertAITransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}

AIStateType AlertAITransition::AlertTransition(AIController* pAI)
{
	
	// ���û��Ŀ����, ��������
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_Idle;
	}

	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// ����ڹ�����Χ�ڣ�������빥����Χ��
		return AIST_Attack;
	}

	// �����ǰĿ���ھ��䷶Χ֮����, ������뾯�䷶Χ��
	if( pAI->IsInAlertDist( pAI->GetTargetUnitID() )&& !pAI->IsInAttackDist( pAI->GetTargetUnitID() ) )
	{
		return AIST_Alert; 
	}
	else
	{
		return AIST_Idle;
	}

	return AIST_Alert; 
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType AlertAITransition::CallForHelpTransition(AIController *pAI)
{
		// ����״̬����ʱ�����
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------------
AIStateType AlertAITransition::FleeTransition(AIController* pAI)
{
	// ����״̬����ʱ�����
	if( pAI->GetFleeTick() <= 0 )
	{
		return AIST_Pursue;
	}

	return AIST_Flee;
}



//----------------------------------------------------------------------------------------
// ����NPC״̬������
//----------------------------------------------------------------------------------------
AITransition* HuchengAITransition::Instance( )
{
	static HuchengAITransition the;
	return &the;
}


// ״̬�л���
AIStateType HuchengAITransition::Transition( AIController* pAI, AIStateType eCurState )
{

	switch( eCurState )
	{

	case AIST_Idle:
		return IdleTransition( pAI );

	case AIST_Pursue:
		return PursueTransition( pAI );

	case AIST_Attack:
		return AttackTransition( pAI );

	case AIST_Return :
		return ReturnTransition( pAI );

	case AIST_SOS:
		return CallForHelpTransition(pAI);

	}

	return AIST_Idle;
}


AIStateType	HuchengAITransition::IdleTransition(AIController* pAI)
{
	// ����
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pursue;
	}

	// �����ǰĿ���ھ��䷶Χ֮����, ������뾯�䷶Χ��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// ����ڹ�����Χ�ڣ�������빥����Χ��
		return AIST_Attack;
	}

	return AIST_Idle;

	//// ���û��Ŀ����, ��������
	//if( !VALID_VALUE(pAI->GetTargetUnitID()) ) 
	//{
	//	return AIST_Idle;
	//}

	//
	//if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	//{
	//	// ����ڹ�����Χ�ڣ�������빥����Χ��
	//	return AIST_Attack;
	//}
	//else
	//{
	//	return AIST_Pursue;
	//}

	//return AIST_Idle;
}

AIStateType	HuchengAITransition::PursueTransition(AIController* pAI)
{
	// �������target�Ƿ������л��ɷ���״̬
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// �����ǰĿ�껹���ڣ����⼼��
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;

	//if( !VALID_VALUE(pAI->GetTargetUnitID()) ) 
	//{
	//	return AIST_Idle;
	//}


	//if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	//{
	//	// ����ڹ�����Χ�ڣ�������빥����Χ��
	//	return AIST_Attack;
	//}

	//return AIST_Pursue;
}


AIStateType	HuchengAITransition::AttackTransition(AIController* pAI)
{

	Creature* pOwner = pAI->GetOwner();

	// ������ܻ�û�����
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// ���û��Ŀ����
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}


	// �����ǰĿ�겻�ڹ�����Χ֮����
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	return AIST_Attack;
}


AIStateType	HuchengAITransition::ReturnTransition(AIController* pAI)
{

	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}


AIStateType		HuchengAITransition::CallForHelpTransition(AIController* pAI)
{

	return AIST_SOS;
}