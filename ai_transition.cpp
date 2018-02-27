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
*	@brief		AI状态转换器
*				将逃跑、求助状态转换器屏蔽。同时根据根据的子行为来确定是否有这些状态
*				增加警戒AI、宠物AI等 [ 2010-04-01:10-52 ]

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
		// 当前体力小于基础体力20％
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// 当前体力小于基础体力20％
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
// 攻击型怪物状态转换
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
// 空闲状态转换
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::IdleTransition(AIController* pAI)
{
	// 索敌
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pursue;
	}

	// 如果当前目标在警戒范围之内了, 怪物进入警戒范围内
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// 如果在攻击范围内，怪物进入攻击范围内
		return AIST_Attack;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// 追击状态转换
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::PursueTransition(AIController* pAI)
{
	// 如果现在target非法，则切换成返回状态
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// 如果当前目标还存在，则检测技能
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// 攻击状态转换
//------------------------------------------------------------------------------------------
AIStateType AggressiveTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// 如果没有目标了
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// 当前体力小于基础体力20％
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// 当前体力小于基础体力20％
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

	// 如果当前目标不在攻击范围之内了
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// 返回状态转换
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
// 呼救状态
//-----------------------------------------------------------------------------------------
AIStateType AggressiveTransition::CallForHelpTransition(AIController *pAI)
{
		// 逃跑状态持续时间结束
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// 逃跑状态
//-----------------------------------------------------------------------------------------
AIStateType AggressiveTransition::FleeTransition(AIController* pAI)
{
	// 逃跑状态持续时间结束
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
// 防御型怪物状态转换
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
// 空闲状态转换
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::IdleTransition(AIController* pAI)
{
	AIStateType _state = TransitionToFlee(pAI);
	if (_state != AIST_Idle)
	{
		return _state;
	}

	// 索敌
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
		// 如果在攻击范围内，怪物进入攻击范围内
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

	// 如果现在target非法，则切换成返回状态
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		OutputDebugString( L"guai wu tao pao\n" );
		return AIST_Return;
	}

	// 如果当前目标还存在，则检测技能
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// 攻击状态转换
//------------------------------------------------------------------------------------------
AIStateType GuardTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();

	
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// 如果没有目标了
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// 当前体力小于基础体力20％
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// 当前体力小于基础体力20％
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

	// 如果目标不在攻击范围内了，则清空它的仇恨
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// 返回状态转换
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
// 呼救状态
//-----------------------------------------------------------------------------------------
AIStateType GuardTransition::CallForHelpTransition(AIController *pAI)
{
		// 逃跑状态持续时间结束
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// 逃跑状态
//-----------------------------------------------------------------------------------------
AIStateType GuardTransition::FleeTransition(AIController* pAI)
{
	// 逃跑状态持续时间结束
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
// 炮塔状态转换
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
// 炮塔型空闲状态转换
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::IdleTransition(AIController* pAI)
{
	// 索敌
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		return AIST_Attack;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// 炮塔型攻击状态转换
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();

	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// 如果没有目标了
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// 如果当前目标不在范围内，则清空其仇恨
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		pAI->SetEnmityActive(pAI->GetTargetUnitID(), FALSE);
		return AIST_Attack;
	}

	return AIST_Attack;
}

//------------------------------------------------------------------------------------------
// 返回状态转换
//------------------------------------------------------------------------------------------
AIStateType BarbetteTransition::ReturnTransition(AIController* pAI)
{
	if( pAI->IsArrivedReturnPos() )
	{
		return AIST_Idle;
	}

	return AIST_Return;
}
// 风筝型状态转换
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
// 空闲状态转换
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::IdleTransition(AIController* pAI)
{
	// 索敌
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		return AIST_Pursue;
	}

	return AIST_Idle;
}

//------------------------------------------------------------------------------------------
// 追击状态转换
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::PursueTransition(AIController* pAI)
{
	// 如果现在target非法，则切换成返回状态
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// 如果当前目标还存在，则检测技能
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;
}

//------------------------------------------------------------------------------------------
// 攻击状态转换
//------------------------------------------------------------------------------------------
AIStateType SpaceOutTransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	Unit* pTarget = pAI->GetPursueTarget();

	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// 如果没有目标了
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// 如果当前目标不在攻击范围之内了
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	// 满足保持距离触发间隔
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
// 宠物攻击型状态转换器
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
	// 如果现在target非法，则切换成索敌状态
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_PetIdle;
	}

	// 如果当前目标还存在，则检测技能
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_PetAttack;
	}

	return AIST_Pet_Pursue;
}

AIStateType PetATTransition::IdleTransition(AIController* pAI)
{
	// 索敌
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pet_Pursue;
	}

	// 如果当前目标在警戒范围之内了, 怪物进入警戒范围内
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// 如果在攻击范围内，怪物进入攻击范围内
		return AIST_PetAttack;
	}

	return AIST_PetIdle;

	// 索敌
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
	
	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_PetAttack;
	}

	// 如果没有目标了
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_PetIdle;
	}

	// 如果当前目标不在攻击范围之内了
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pet_Pursue; 
	}

	return AIST_PetAttack;
}



//------------------------------------------------------------------------------
// 警戒AI怪物的状态转换器
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
	
	// 如果没有目标了, 继续索敌
	if( !VALID_VALUE(pAI->GetTargetUnitID()) ) 
	{
		return AIST_Idle;
	}

	// 如果当前目标在警戒范围之内了, 怪物进入警戒范围内
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


	// 如果当前目标还存在，则检测技能
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Attack;
	}

	return AIST_Pursue;

}


// 警戒怪在攻击状态下
AIStateType AlertAITransition::AttackTransition(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	float iHP  = pOwner->GetProto()->fChangleActHP;
	int   iAct = (int)(pOwner->GetProto()->fChangeAct * 100 );
	// 如果没有目标了, 继续索敌
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	pAI->dw_help_time -= TICK_TIME;

	if(pAI->dw_help_time <= 0)
	{
		pAI->dw_help_time = pOwner->GetProto()->dw_help_time;
		// 当前体力小于基础体力20％
		if( pAI->m_ePursueType == ECPST_CallHelp )
		{
			int iRadom = get_tool()->tool_rand() % 100;
			if(EFT_NotEnter == pAI->GetFleeTime() && (pOwner->GetBaseAttValue(ERA_MaxHP) * iHP) > pOwner->GetAttValue(ERA_HP) && iRadom <= iAct )
			{
				return AIST_SOS;
			}
		}

		// 当前体力小于基础体力20％
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

	// 如果在攻击范围之外，就追击敌人
	if( !pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		return AIST_Pursue;
	}

	// 继续攻击
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
	
	// 如果没有目标了, 继续索敌
	if( !VALID_VALUE(pAI->GetTargetUnitID()) )
	{
		return AIST_Idle;
	}

	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// 如果在攻击范围内，怪物进入攻击范围内
		return AIST_Attack;
	}

	// 如果当前目标在警戒范围之内了, 怪物进入警戒范围内
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
// 呼救状态
//-----------------------------------------------------------------------------------------
AIStateType AlertAITransition::CallForHelpTransition(AIController *pAI)
{
		// 逃跑状态持续时间结束
	if( EFT_CallHelp == pAI->GetFleeTime() )
	{
		return AIST_Pursue;
	}

	return AIST_SOS;
}

//-----------------------------------------------------------------------------------------
// 逃跑状态
//-----------------------------------------------------------------------------------------
AIStateType AlertAITransition::FleeTransition(AIController* pAI)
{
	// 逃跑状态持续时间结束
	if( pAI->GetFleeTick() <= 0 )
	{
		return AIST_Pursue;
	}

	return AIST_Flee;
}



//----------------------------------------------------------------------------------------
// 护城NPC状态管理器
//----------------------------------------------------------------------------------------
AITransition* HuchengAITransition::Instance( )
{
	static HuchengAITransition the;
	return &the;
}


// 状态切换器
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
	// 索敌
	if( pAI->GetCurrentVictim() != INVALID_VALUE )
	{
		pAI->SetTargetUnitID(pAI->GetCurrentVictim());
		//pAI->SetCurrentVictim( -1 );
		return AIST_Pursue;
	}

	// 如果当前目标在警戒范围之内了, 怪物进入警戒范围内
	if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	{
		// 如果在攻击范围内，怪物进入攻击范围内
		return AIST_Attack;
	}

	return AIST_Idle;

	//// 如果没有目标了, 继续索敌
	//if( !VALID_VALUE(pAI->GetTargetUnitID()) ) 
	//{
	//	return AIST_Idle;
	//}

	//
	//if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
	//{
	//	// 如果在攻击范围内，怪物进入攻击范围内
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
	// 如果现在target非法，则切换成返回状态
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}

	// 如果当前目标还存在，则检测技能
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
	//	// 如果在攻击范围内，怪物进入攻击范围内
	//	return AIST_Attack;
	//}

	//return AIST_Pursue;
}


AIStateType	HuchengAITransition::AttackTransition(AIController* pAI)
{

	Creature* pOwner = pAI->GetOwner();

	// 如果技能还没有完成
	if( pOwner->GetCombatHandler().IsValid() )
	{
		return AIST_Attack;
	}

	// 如果没有目标了
	if( !VALID_POINT(pAI->GetTargetUnitID()) )
	{
		return AIST_Return;
	}


	// 如果当前目标不在攻击范围之内了
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