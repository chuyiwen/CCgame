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
*	@file		creature.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		游戏内生物AI类，一个有限状态机，包括多种AI状态
*/

#include "stdafx.h"
#include "../../common/WorldDefine/creature_define.h"
#include "NPCTeam_define.h"

#include "unit.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "ai_transition.h"
#include "ai_trigger.h"
#include "path_finder.h"
#include "NPCTeam.h"

//------------------------------------------------------------------------------
// 空闲AI状态
//------------------------------------------------------------------------------
AIStateIdle* AIStateIdle::Instance()
{
	static AIStateIdle instance;
	return &instance;
}

VOID AIStateIdle::OnEnter(AIController* pAI)
{
	pAI->ClearAllEnmity();
	pAI->SetEnterCombatTick(0);

	//pAI->SetIsPatroling(FALSE);
	pAI->SetIsReversePatrol(FALSE);
	pAI->ReSetPatrolRestTick();
	pAI->ReSetLookForTargetTick();

	Creature* pOwner = pAI->GetOwner();
	// 如果有AI静态属性，则根据这个属性给怪物增加buff
	if( VALID_POINT(pAI->GetAIProto()) )
	{
		// 首先给怪物添加buff
		for(INT n = 0; n < X_MAX_CREATURE_AI_BUFF; n++)
		{
			if( INVALID_VALUE == pAI->GetAIProto()->dwBuffTypeID[n] )
				break;

			// 找到buff属性
			const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(pAI->GetAIProto()->dwBuffTypeID[n]);
			if( !VALID_POINT(pBuffProto) ) continue;
			
			// 添加buff
			pOwner->TryAddBuff(pOwner, pBuffProto, NULL, NULL, NULL);
		}
	}

	// 删除战备状态
	if( !VALID_POINT(pOwner) ) return;
	pOwner->UnsetState(ES_Combat);
	//pOwner->RemoveBuff(88888, FALSE);
	//pOwner->SetZhanbei( false );
}

VOID AIStateIdle::OnExit(AIController* pAI)
{
	AITriggerMgr* pAITrigger = pAI->GetAITriggerMgr();
	if( VALID_POINT(pAITrigger) )
		pAITrigger->SetTriggerActive(ETEE_DetectTarget);
}

VOID AIStateIdle::Update(AIController* pAI)
{
	// 更新巡逻
	Creature* pOwner = pAI->GetOwner();
	if( !pOwner->IsInState(ES_Special) )
	{
		pAI->UpdatePatrol();
	}

	// 更新索敌
	pAI->UpdateLookForEnemy();
}


BOOL AIStateIdle::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//--------------------------------------------------------------------------------
// 追击状态
//--------------------------------------------------------------------------------
AIStatePursue* AIStatePursue::Instance()
{
	static AIStatePursue instance;
	return &instance;
}

VOID AIStatePursue::OnEnter(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	pAI->ReSetLockTargetTime();
	
	Unit* pUnit = pAI->GetOwner()->get_map()->find_unit(pAI->GetTargetUnitID());
	if( VALID_POINT(pUnit) )
	{
		pAI->StartPursue(pUnit);
	}
	else
	{
		pAI->SetPursueTargetPos(pAI->GetOwner()->GetCurPos());
	}

	if( 0 == pAI->GetEnterCombatTick() )
	{
		pAI->SetEnterCombatTick(g_world.GetWorldTick());
		pAI->SetEnterCombatPos(pAI->GetOwner()->GetCurPos());
		pOwner->SetState(ES_Combat);
		const CreatureScript *pScript = pOwner->GetScript();
		if (VALID_POINT(pScript))
		{
			pScript->OnEnterCombat(pOwner);
		}
	}
}

VOID AIStatePursue::OnExit(AIController* pAI)
{

}

VOID AIStatePursue::Update(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	// 更新锁敌倒计时
	pAI->CountDownNextAttackWaitTick();
	BOOL bTargetChange = pAI->UpdateLockTarget();

	// 如果没找到，直接返回
	if( !VALID_POINT(pAI->GetTargetUnitID()) ) return;

	// 找到真正的target
	Unit* pTarget = pAI->GetPursueTarget();
	if( !VALID_POINT(pTarget) )  return;

	// 如果目标改变了，则重新发起追踪
	if( bTargetChange )
	{
		pAI->StartPursue(pTarget);
	}
	else
	{
		pAI->UpdatePursue(pTarget);
	}

	// 可能有效率问题
	if( pAI->m_bCalHelp )
		pAI->UpdateCallHelp();
}

BOOL AIStatePursue::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//---------------------------------------------------------------------------------
// 攻击状态
//---------------------------------------------------------------------------------
AIStateAttack* AIStateAttack::Instance()
{
	static AIStateAttack instance;
	return &instance;
}

VOID AIStateAttack::OnEnter(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	if( 0 == pAI->GetEnterCombatTick() )
	{
		pAI->SetEnterCombatTick(g_world.GetWorldTick());
		pAI->SetEnterCombatPos(pAI->GetOwner()->GetCurPos());
		pOwner->SetState(ES_Combat);
		const CreatureScript *pScript = pOwner->GetScript();
		if (VALID_POINT(pScript))
		{
			pScript->OnEnterCombat(pOwner);
		}
	}

	AITriggerMgr* pAITrigger = pAI->GetAITriggerMgr();
	if( VALID_POINT(pAITrigger) )
	{
		pAITrigger->SetTriggerActive(ETEE_InterCombatTime);
	}
	pOwner->GetMoveData().StopMove();
}

VOID AIStateAttack::OnExit(AIController* pAI)
{

}

VOID AIStateAttack::Update(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	BOOL bTargetChange = pAI->UpdateLockTarget();

	CombatHandler& handler = pOwner->GetCombatHandler();

	// 如果当前技能不在释放了
 	if( !handler.IsValid() )
	{
		pAI->CountDownNextAttackWaitTick();

		if( VALID_POINT(pAI->GetTargetUnitID()) && pAI->GetNextAttackWaitTick() < 0 )
		{
			// 可以再次发动
			if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
			{
				INT nRet = pAI->AIUseSkill(pAI->GetAIUseSkillID(), pAI->GetTargetUnitID());

				if( E_Success == nRet )
					pOwner->OnAIAttack();
				/*else
					pAI->SetTargetUnitID(INVALID_VALUE);*/
			}
		}
	}

	// 可能有效率问题
	if( pAI->m_bCalHelp )
		pAI->UpdateCallHelp();
}

BOOL AIStateAttack::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//-----------------------------------------------------------------------------------
// 逃跑状态
//-----------------------------------------------------------------------------------
AIStateFlee* AIStateFlee::Instance()
{
	static AIStateFlee instance;
	return &instance;
}

VOID AIStateFlee::OnEnter(AIController* pAI)
{
	pAI->SetFleeTime(EFT_FirstEnter);
}

VOID AIStateFlee::OnExit(AIController* pAI)
{
	pAI->SetIsArrivedFleePos(FALSE);
}

VOID AIStateFlee::Update(AIController* pAI)
{
	// 递减进入逃跑状态的时间
	pAI->CountDownNextFleeTick();

	if(50 > pAI->GetFleeTick())
		return;

	if( EFT_FirstEnter == pAI->GetFleeTime() )
	{
		pAI->CalFleePos();
		pAI->StartFlee(pAI->GetFleePos());

		pAI->SetFleeTime(EFT_Enter);
	}

	// 已经到达逃跑的目标点
	if( pAI->IsArrivedFleePos() )
	{
		pAI->CalFleePos();
		pAI->StartFlee(pAI->GetFleePos());
	}
	else
	{
		pAI->UpdateFlee();
	}
}

BOOL AIStateFlee::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//-----------------------------------------------------------------------------------
// 呼救状态
//-----------------------------------------------------------------------------------
AIStateCallHelp* AIStateCallHelp::Instance()
{
	static AIStateCallHelp instance;
	return &instance;
}

VOID AIStateCallHelp::OnEnter(AIController* pAI)
{
	pAI->SetHelperID(INVALID_VALUE);
	pAI->GetHelpList().clear();
	pAI->SetFleeTime(EFT_FirstEnter);
}

VOID AIStateCallHelp::OnExit(AIController* pAI)
{
	pAI->SetHelperID(INVALID_VALUE);
	pAI->GetHelpList().clear();
	pAI->SetIsArrivedFleePos(FALSE);
	pAI->SetFleeTime(EFT_NotEnter);
}

VOID AIStateCallHelp::Update(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	// 寻找求救目标
	if( EFT_FirstEnter == pAI->GetFleeTime() )
	{
		if( pAI->CalHelpPos() )
		{
			if(pOwner->GetProto()->eHelpType == EHT_Stand)
			{
				pAI->SetIsArrivedFleePos(TRUE);
				pAI->SetFleeTime(EFT_Enter);
			}
			else
			{
				pAI->StartFlee(pAI->GetFleePos());
				pAI->SetFleeTime(EFT_Enter);
			}
		}
		else
		{
			pAI->SetFleeTime(EFT_CallHelp);
		}
	}

	// 已经到达逃跑的目标点
	if( pAI->IsArrivedFleePos() )
	{
		if( EFT_CallHelp != pAI->GetFleeTime() )
		{
			pAI->SetFleeTime(EFT_CallHelp);
			if(pOwner->GetProto()->eHelpType == EHT_Stand)
			{
				package_list<DWORD>::list_iter iter = pAI->GetHelpList().begin();
				DWORD	dwHelpID = INVALID_VALUE;
				while(pAI->GetHelpList().find_next(iter, dwHelpID))
				{
					if(dwHelpID != INVALID_VALUE)
					{
						Creature* pCreature = pOwner->get_map()->find_creature(dwHelpID);
						if( VALID_POINT(pCreature) && !pCreature->IsDead() )
						{
							if( VALID_POINT(pCreature->GetAI()) )
							{
								Unit* pTarget = pOwner->get_map()->find_role(pAI->GetTargetUnitID());
								if(VALID_POINT(pTarget))
								{
									if(VALID_POINT(pCreature->GetScript()))
									{
										pCreature->GetScript()->OnBeHelp(pOwner, pCreature);
									}
									pCreature->GetAI()->AddEnmity(pTarget, 1);
								}
							}
						}
					}
				}
			}
			else
			{
				if( VALID_POINT(pAI->GetHelperID()))
				{
					Creature* pCreature = pOwner->get_map()->find_creature(pAI->GetHelperID());
					if( VALID_POINT(pCreature) && !pCreature->IsDead() )
					{
						if( VALID_POINT(pCreature->GetAI()) )
						{
							Unit* pTarget = pOwner->get_map()->find_role(pAI->GetTargetUnitID());
							if(VALID_POINT(pCreature->GetScript()))
							{
								pCreature->GetScript()->OnBeHelp(pOwner, pCreature);
							}
							pCreature->GetAI()->AddEnmity(pTarget, 1);
						}
					}
				}
			}
		}
	}
	else
	{
		if( pAI->NeedReCalHelpPos() )
		{
			if( pAI->CalHelpPos() )
				pAI->StartFlee(pAI->GetFleePos());
			else
				pAI->SetFleeTime(EFT_CallHelp);
		}
		pAI->UpdateFlee();
	}
}

BOOL AIStateCallHelp::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}


//------------------------------------------------------------------------------------
// 返回状态
//------------------------------------------------------------------------------------
AIStateReturn* AIStateReturn::Instance()
{
	static AIStateReturn instance;
	return &instance;
}

VOID AIStateReturn::OnEnter(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	pOwner->SetState(ES_Invincible);
	pOwner->SetTagged(INVALID_VALUE);
	pAI->ClearAllEnmity();
	pOwner->RemoveAllBuff(FALSE);
	pAI->SetTargetUnitID(INVALID_VALUE);
	pAI->SetFleeTime(EFT_NotEnter);

	const CreatureScript *pScript = pOwner->GetScript();
	if (VALID_POINT(pScript))
	{
		pScript->OnLeaveCombat(pOwner);
	}

	AITriggerMgr* pAITrigger = pAI->GetAITriggerMgr();
	if( VALID_POINT(pAITrigger) )
	{
		pAITrigger->Refresh();
	}

	//// 删除战备状态
	pOwner->UnsetState(ES_Combat);

	pAI->StartReturn();
	//pOwner->RemoveBuff(88888, FALSE);
	//pOwner->SetZhanbei( false );

}

VOID AIStateReturn::OnExit(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	pOwner->SetAttValue(ERA_HP, pOwner->GetAttValue(ERA_MaxHP));
	pOwner->SetAttValue(ERA_MP, pOwner->GetAttValue(ERA_MaxMP));
	pOwner->UnsetState(ES_Invincible);
	pOwner->SetTagged(INVALID_VALUE);
	pOwner->ResetFaceTo();
	pAI->m_bCalHelp = FALSE;
}

VOID AIStateReturn::Update(AIController* pAI)
{
	pAI->UpdateReturn();
}

BOOL AIStateReturn::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//--------------------------------------------------------------------------------------
// 跟随状态
//--------------------------------------------------------------------------------------
AIStateFollow* AIStateFollow::Instance()
{
	static AIStateFollow instance;
	return &instance;
}

VOID AIStateFollow::OnEnter(AIController* pAI)
{

}

VOID AIStateFollow::OnExit(AIController* pAI)
{
	pAI->GetTracker()->SetTarget(NULL);
	pAI->GetTracker()->SetTargetID(INVALID_VALUE);
}

VOID AIStateFollow::Update(AIController* pAI)
{
	pAI->GetTracker()->Update();
}


BOOL AIStateFollow::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//--------------------------------------------------------------------------------------
// 拉开距离
//--------------------------------------------------------------------------------------
AIStateSpaceOut* AIStateSpaceOut::Instance()
{
	static AIStateSpaceOut instance;
	return &instance;
}

VOID AIStateSpaceOut::OnEnter(AIController* pAI)
{
	pAI->StartSpaceOut();
}

VOID AIStateSpaceOut::OnExit(AIController* pAI)
{
	pAI->SetIfArrivedPos(FALSE);
}

VOID AIStateSpaceOut::Update(AIController* pAI)
{
	pAI->UpdateSpaceOut();
}


BOOL AIStateSpaceOut::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//-----------------------------------------------------------------------------
// 宠物索敌状态
// 
//-----------------------------------------------------------------------------
AIStatePetIdle* AIStatePetIdle::Instance( )
{
	static AIStatePetIdle instance;
	return &instance;
}


VOID AIStatePetIdle::OnEnter(AIController* pAI)
{
	//pAI->ReSetLookForTargetTick();
}

VOID AIStatePetIdle::OnExit(AIController* pAI)
{

}

VOID AIStatePetIdle::Update(AIController* pAI)
{
	// 更新索敌, 因该对应新的索敌规则
	//if( !pAI->IsTargetValid(pAI->GetTargetUnitID()) )
	//	pAI->UpdateLookForEnemyPet();

	pAI->GetTracker()->Update();

}


BOOL AIStatePetIdle::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//-----------------------------------------------------------------------------
// 宠物战斗状态
// 
//-----------------------------------------------------------------------------
AIStatePetAttack* AIStatePetAttack::Instance( )
{
	static AIStatePetAttack instance;
	return &instance;
}


VOID AIStatePetAttack::OnEnter(AIController* pAI)
{

	
}


VOID AIStatePetAttack::OnExit(AIController* pAI)
{

}

VOID AIStatePetAttack::Update(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	BOOL bTargetChange = pAI->UpdateLockTarget();

	CombatHandler& handler = pOwner->GetCombatHandler();

	// 如果当前技能不在释放了
	if( !handler.IsValid() )
	{
		pAI->CountDownNextAttackWaitTick();

		if( VALID_POINT(pAI->GetTargetUnitID()) && pAI->GetNextAttackWaitTick() < 0 )
		{
			// 可以再次发动
			if( pAI->IsInAttackDist(pAI->GetTargetUnitID()) )
			{
				INT nRet = pAI->AIUseSkill(pAI->GetAIUseSkillID(), pAI->GetTargetUnitID());

				if( E_Success == nRet )
					pOwner->OnAIAttack();
			}
		}
	}

	pAI->GetTracker()->UpdateNeedPutDown();
}


BOOL AIStatePetAttack::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//-----------------------------------------------------------------------------
// 宠物的追敌状态
// 
//-----------------------------------------------------------------------------
AIStatePetPursue* AIStatePetPursue::Instance()
{
	static AIStatePetPursue instance;
	return &instance;
}

VOID AIStatePetPursue::OnEnter(AIController* pAI)
{
	Unit* pTarget = pAI->GetPursueTarget();
	if( !VALID_POINT(pTarget) )  return;

	pAI->StartPursue( pTarget, false);
}

VOID AIStatePetPursue::OnExit(AIController* pAI)
{

}

VOID AIStatePetPursue::Update(AIController* pAI)
{
	
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	// 更新锁敌倒计时
	pAI->CountDownNextAttackWaitTick();
	BOOL bTargetChange = pAI->UpdateLockTarget();

	// 如果没找到，直接返回
	if( !VALID_POINT(pAI->GetTargetUnitID()) ) return;

	// 找到真正的target
	Unit* pTarget = pAI->GetPursueTarget();
	if( !VALID_POINT(pTarget) )  return;

	// 如果目标改变了，则重新发起追踪
	if( bTargetChange )
	{
		pAI->StartPursue(pTarget, false);
	}
	else
	{
		pAI->UpdatePursue(pTarget);
	}


}

BOOL AIStatePetPursue::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}


//-----------------------------------------------------------------------------
// 怪物警戒状态
// 
//-----------------------------------------------------------------------------
AIStateAlert* AIStateAlert::Instance()
{
	static AIStateAlert the;
	return &the;
}

VOID AIStateAlert::OnEnter(AIController* pAI)
{
	// 是不时应该通知客户端，怪物进入了警戒状态
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	// 得到目标
	Unit* pTarget = pOwner->get_map()->find_unit(pAI->GetTargetUnitID() );
	if( !VALID_POINT(pTarget) ) return ;
	pOwner->GetMoveData().SetFaceTo( pTarget->GetCurPos() - pOwner->GetCurPos());
	//pOwner->TryAddBuff( pOwner, AttRes::GetInstance()->GetBuffProto(8888888), NULL, NULL, NULL);
	
	pOwner->SetState(ES_Combat);

	// 发送怪物的朝向
	if( pTarget->IsRole() )
	{

		NET_SIS_synchronize_stand pSend;
		pSend.dw_size	= sizeof(NET_SIS_synchronize_stand);
		pSend.dw_role_id	= pOwner->GetID();
		pSend.curPos	= pOwner->GetCurPos();
		pSend.faceTo	= pOwner->GetFaceTo();
		((Role*)pTarget)->SendMessage(&pSend, pSend.dw_size);
	}

	if( pOwner->IsInState(ES_Special) )
	{
		pOwner->UnsetState(ES_Special);
		NET_SIS_unset_state send;
		send.dw_role_id = pOwner->GetID();
		send.eState   = ES_Special;

		if( VALID_POINT( pOwner->get_map() ) )
			pOwner->get_map()->send_big_visible_tile_message(pOwner, &send, send.dw_size);
	}
	

}

VOID AIStateAlert::OnExit(AIController* pAI)
{
	// 是不时应该通知客户端，怪物退出了警戒状态
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;
	//pOwner->RemoveBuff(88888, FALSE);
	pOwner->UnsetState(ES_Combat);
}

VOID AIStateAlert::Update(AIController* pAI)
{

	
	// 如果没找到，直接返回
	if( !VALID_POINT(pAI->GetTargetUnitID()) ) return;

	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;
	
	// 得到目标
	Unit* pTarget = pOwner->get_map()->find_unit(pAI->GetTargetUnitID() );
	if( !VALID_POINT(pTarget) ) return ;

	// 检查自己的数据是否全
	if( !pOwner->GetProto() )
		return ;

	if( !pOwner->IsInCombatDistance(*pTarget, pOwner->GetProto()->nAlertDis, 32) )
	{
		pAI->SetTargetUnitID(INVALID_VALUE);;
	}
}

BOOL AIStateAlert::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	return TRUE;
}

//---------------------------------------------------------------------------------------
// 脚本状态
//---------------------------------------------------------------------------------------
AIStateScript* AIStateScript::Instance()
{
	static AIStateScript instance;
	return &instance;
}

VOID AIStateScript::OnEnter(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	AITriggerMgr* pAITrigger = pAI->GetAITriggerMgr();
	if( VALID_POINT(pAITrigger) )
	{
		pAITrigger->Pause();		// 暂停触发器
	}

	const CreatureScript* pScript = pOwner->GetScript();
	if( VALID_POINT(pScript) ) pScript->OnEnterCurAI(pOwner);
}

VOID AIStateScript::OnExit(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	AITriggerMgr* pAITrigger = pAI->GetAITriggerMgr();
	if( VALID_POINT(pAITrigger) )
	{
		pAITrigger->UnPause();		// 恢复触发器
	}

	const CreatureScript* pScript = pOwner->GetScript();
	if( VALID_POINT(pScript) ) pScript->OnLeaveCurAI(pOwner);
}

VOID AIStateScript::Update(AIController* pAI)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return;

	const CreatureScript* pScript = pOwner->GetScript();
	if( VALID_POINT(pScript) ) pScript->OnUpdateCurAI(pOwner);
}


BOOL AIStateScript::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	Creature* pOwner = pAI->GetOwner();
	if( !VALID_POINT(pOwner) ) return FALSE;

	const CreatureScript* pScript = pOwner->GetScript();
	if( VALID_POINT(pScript) ) return pScript->OnEventCurAI(pOwner);
	
	return FALSE;
}

//---------------------------------------------------------------------------------------
// 对话状态
//---------------------------------------------------------------------------------------
AIStateTalk* AIStateTalk::Instance()
{
	static AIStateTalk instance;
	return &instance;
}

VOID AIStateTalk::OnEnter(AIController* pAI)
{
	pAI->SetTalkTime(30 * TICK_TIME) ;
}

VOID AIStateTalk::OnExit(AIController* pAI)
{

}

VOID AIStateTalk::Update(AIController* pAI)
{
	if(pAI->GetTalkTime() > 0)
	{
		pAI->GetTalkTime() -= TICK_TIME;
		return;
	}
	pAI->ChangeState(AIST_Idle);
}

BOOL AIStateTalk::OnEvent(AIController* pAI, Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */)
{
	return FALSE;
}


//---------------------------------------------------------------------------------------
// AI控制器
//---------------------------------------------------------------------------------------
AIController::AIController(Creature* pCreature, const tag_map_way_point_info_list* patrolList) : 
m_pOwner(pCreature), m_pCurrentState(NULL), 
m_pPreState(NULL), m_pGlobalState(NULL), m_pProto(NULL), m_pAIProto(NULL), m_pAITrigger(NULL), 
m_pScript(NULL), m_pTransition(NULL), m_nScriptUpdateTimer(INVALID_VALUE), m_nScriptUpdateCountDown(0), 
m_pWPArray(NULL), m_pWayPointTime(NULL), m_nWPNum(0), m_nWPIndex(0), m_bReversePatrol(FALSE), m_bIsPatroling(FALSE), 
m_nPatrolRestTick(0), m_nLookForTargetTick(0), m_bPursueFailed(FALSE), m_bCanNotPursue(FALSE), m_vPursueTargetPos(0.0f, 0.0f, 0.0f), 
m_dwTargeUnitID(INVALID_VALUE), m_nCheckAIEventTick(0), m_dwEnterCombatTick(0), m_dwUseSkillID(INVALID_VALUE),
m_nNextAttackWaitTick(0), m_dwSerial(0), m_vFleePos(0.0f, 0.0f, 0.0f), m_vPosEnterCombat(0.0f, 0.0f, 0.0f), m_eFleeTime(EFT_NotEnter),
m_dwMaxEnmityUnitID(INVALID_VALUE), m_nLockTargetTick(0), m_mapEnmity(), m_pPathFinder(NULL), m_dwFleeTick(0), m_bArrivedFleePos(FALSE),
m_dwSpaceOutTick(CREATURE_SPACE_OUT_MIN), m_bArrivedPos(FALSE), m_dwCurrentVictim(INVALID_VALUE),m_nNoAttackTick(0)
{
	// 初始化静态属性
	m_pProto = m_pOwner->GetProto();

	// 初始化脚本
	m_pScript = pCreature->GetScript();

	// 初始化巡逻
	InitPatrol(patrolList);

	// 初始化AI属性
	InitAIProto();

	// 初始化触发器管理器
	InitTriggerMgr();

	// 初始化状态管理器
	InitAITransition();

	// 初始化寻路
	InitPathFinder();

	// 初始化为Idle状态
	m_eCurAIState = AIST_Idle;
	m_pCurrentState = AIStateIdle::Instance();

	// 初始化战斗属性
	ReSetPatrolRestTick();
	ReSetLookForTargetTick();
	ReSetLockTargetTime();

	// 初始化AI的子行为
	m_ePursueType  = m_pOwner->GetProto()->ePursueType;
	m_bCalHelp	   = FALSE;	
	m_dwCallHelpTime = 0;
	dw_help_time = m_pProto->dw_help_time;

	m_pTracker = new PetTracker(m_pOwner);

	// 好大头的log
	if (m_pProto->dw_data_id == 3020023)
	{

		g_world.get_log()->write_log(_T("Creature 3020023 aiInit"));

	}

}


//-----------------------------------------------------------------------------------------
// 刷新
//-----------------------------------------------------------------------------------------
VOID AIController::Refresh()
{

	// 好大头的log
	if (m_pProto->dw_data_id == 3020023)
	{
		
		g_world.get_log()->write_log(_T("Creature 3020023 Refresh"));
		
	}

	// 重置属性
	m_eCurAIState				=	AIST_Idle;
	m_pCurrentState				=	AIStateIdle::Instance();
	m_pPreState					=	NULL;
	m_pGlobalState				=	NULL;
	m_nScriptUpdateTimer		=	INVALID_VALUE;
	m_nScriptUpdateCountDown	=	0;
	m_nWPIndex					=	0;
	m_bReversePatrol			=	FALSE;
	m_bIsPatroling				=	FALSE;
	m_nPatrolRestTick			=	0;
	m_nLookForTargetTick		=	0;
	m_vPursueTargetPos			=	Vector3(0.0f, 0.0f, 0.0f);
	m_bPursueFailed				=	FALSE;
	m_bCanNotPursue				=	FALSE;
	m_dwTargeUnitID				=	INVALID_VALUE;
	m_nCheckAIEventTick			=	0;
	m_dwEnterCombatTick			=	0;
	m_dwUseSkillID				=	INVALID_VALUE;
	m_nNextAttackWaitTick		=	0;
	m_dwSerial					=	0;
	m_vFleePos					=	Vector3(0.0f, 0.0f, 0.0f);
	m_vPosEnterCombat			=	Vector3(0.0f, 0.0f, 0.0f);
	m_dwMaxEnmityUnitID			=	INVALID_VALUE;
	m_dwCurrentVictim			=	INVALID_VALUE;
	m_nLockTargetTick			=	0;
	m_eFleeTime					=	EFT_NotEnter;
	m_bArrivedFleePos			=	FALSE;
	m_bCalHelp					=	FALSE;	


	// 重新初始化属性
	InitAIProto();

	// 初始化触发器管理器
	InitTriggerMgr();

	// 初始化状态管理器
	InitAITransition();

	// 重置战斗属性
	ReSetPatrolRestTick();
	ReSetLookForTargetTick();
	ReSetLockTargetTime();
}

//-----------------------------------------------------------------------------------------
// 初始化巡逻
//-----------------------------------------------------------------------------------------
VOID AIController::InitPatrol(const tag_map_way_point_info_list* patrolList)
{
	if( !VALID_POINT(m_pProto) ) return;

	// 导航点巡逻
	//if( (ECPT_Path == m_pProto->ePatrolType||ECPT_Patrol == m_pProto->ePatrolType) && VALID_POINT(patrolList) && patrolList->list.Size() >= 1 )
	if (VALID_POINT(patrolList) && patrolList->list.size() >= 1)
	{
		INT nWPNum = patrolList->list.size();

		m_pWPArray		= new Vector3[nWPNum];
		m_pWayPointTime = new DWORD[nWPNum];			
		m_nWPNum = nWPNum;
		m_nWPIndex = 0;

		tag_way_point_info info;
		INT nIndex = 0;
		package_list<tag_way_point_info>::list_iter it = patrolList->list.begin();

		while( patrolList->list.find_next(it, info) )
		{
			m_pWPArray[nIndex] = info.v_pos;
			m_pWayPointTime[nIndex]    = info.dw_time;
			++nIndex;
		}

		m_bReversePatrol = FALSE;
	}
	else
	{
		m_pWPArray = NULL;
		m_nWPNum = 0;
		m_nWPIndex = 0;
	}
}

//-----------------------------------------------------------------------------------------
// 初始化AI静态属性
//-----------------------------------------------------------------------------------------
VOID AIController::InitAIProto()
{
	if( !VALID_POINT(m_pProto) ) return;

	// 得到静态AI属性
	switch(m_pProto->eAICreateType)
	{
	case EACT_Indicate:
		{
			// 指定方式，则直接根据怪物AI的ID得到AI静态属性
			m_pAIProto = AttRes::GetInstance()->GetCreatureAI(m_pProto->dwAIID);
		}
		break;

	case EACT_Random:
		{
			// 随机方式，则随机的给怪物增加一个AI
			m_pAIProto = AttRes::GetInstance()->RandGetCreatureAI();
		}
		break;

	case EACT_GroupRandom:
		{
			// 组随机方式，则在一个组种随机的选择一个AI
			m_pAIProto = AttRes::GetInstance()->RandGetCreatureAIInGroup(m_pProto->dwAIID);
		}
		break;
	}

	// 如果有AI静态属性，则根据这个属性给怪物增加buff和技能
	if( VALID_POINT(m_pAIProto) )
	{
		// 首先给怪物添加buff
		for(INT n = 0; n < X_MAX_CREATURE_AI_BUFF; n++)
		{
			if( INVALID_VALUE == m_pAIProto->dwBuffTypeID[n] )
				break;

			// 找到buff属性
			const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(m_pAIProto->dwBuffTypeID[n]);
			if( !VALID_POINT(pBuffProto) ) continue;

			// 添加buff
			m_pOwner->TryAddBuff(m_pOwner, pBuffProto, NULL, NULL, NULL);
		}

		// 再给怪物添加技能
		for(INT n = 0; n < X_MAX_CREATURE_AI_SKILL; n++)
		{
			if( INVALID_VALUE == m_pAIProto->dwSkillTypeID[n] )
				break;

			// 找到技能属性和触发器属性
			const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(m_pAIProto->dwSkillTypeID[n]);

			if( !VALID_POINT(pSkillProto) ) continue;

			// 生成技能
			DWORD dwSkillID = Skill::GetIDFromTypeID(pSkillProto->dwID);
			INT nLevel = Skill::GetLevelFromTypeID(pSkillProto->dwID);
			Skill* pSkill = new Skill(dwSkillID, nLevel, 0, 0, 0);
			m_pOwner->AddSkill(pSkill);
		}
	}
}

//------------------------------------------------------------------------------------------
// 初始化触发器管理器
//-------------------------------------------------------------------------------------------
VOID AIController::InitTriggerMgr()
{
	if( VALID_POINT(m_pAITrigger) ) SAFE_DELETE(m_pAITrigger);

	if( VALID_POINT(m_pAIProto) )
	{
		m_pAITrigger = new AITriggerMgr;
		m_pAITrigger->Init(m_pOwner, m_pAIProto);
	}
}

//------------------------------------------------------------------------------------------
// 初始化
//------------------------------------------------------------------------------------------
VOID AIController::InitAITransition()
{
	m_pTransition = GetTransitionByType();
}

//------------------------------------------------------------------------------------------
// 初始化寻路
//------------------------------------------------------------------------------------------
VOID AIController::InitPathFinder()
{
	if( m_pOwner->NeedCollide() )
		m_pPathFinder = new NPCPathFinderCol();
	else
		m_pPathFinder = new NPCPathFinder();

	m_pPathFinder->Init(GetOwner());
}

//------------------------------------------------------------------------------------------
// 销毁
//------------------------------------------------------------------------------------------
VOID AIController::Destroy()
{
	tagEnmity*	pEnmity		=	NULL;
	m_mapEnmity.reset_iterator();
	while( m_mapEnmity.find_next(pEnmity) )
	{
		SAFE_DELETE(pEnmity);
	}

	SAFE_DELETE(m_pAITrigger);
	SAFE_DELETE_ARRAY(m_pWPArray);
	SAFE_DELETE(m_pPathFinder);
	SAFE_DELETE_ARRAY(m_pWayPointTime);
	SAFE_DELETE(m_pTracker);
}

//------------------------------------------------------------------------------------------
// 更新AI状态
//------------------------------------------------------------------------------------------
VOID AIController::Update()
{
	// 好大头的log
	//if (m_pProto->dw_data_id == 3020023 && m_pOwner->IsMonster())
	//{
	//	static DWORD dw_time = timeGetTime();
	//	DWORD dw_new_time = timeGetTime();
	//	if(dw_new_time - dw_time > 5 * 60 * 1000)
	//	{
	//		g_world.get_log()->write_log(_T("Creature 3020023 state:%d AIstate:%d scriptUpdateTimer: %d\r\n"), m_pOwner->GetState(), (int)m_eCurAIState, m_nScriptUpdateTimer);
	//		dw_time = timeGetTime();
	//	}
	//}
	// 如果处于不能更新AI的状态，则返回
	if( IsInStateCantUpdateAI() ) return;

	// 更新状态机
	UpdateAIController();

	// 更新切换
	UpdateTransition();

	// 更新当前状态
	UpdateCurrentState();

	// 更新触发器
	UpdateTriggerMgr();

}

//------------------------------------------------------------------------------------------
// 处理事件
//------------------------------------------------------------------------------------------
VOID AIController::OnEvent(Unit* pSrc, INT nEventType, DWORD dwEventMisc1/* =0 */, DWORD dwEventMisc2/* =0 */)
{
	// 如果当前状态不存在，则用全局状态处理
	if( !VALID_POINT(m_pCurrentState) )
	{
		if( VALID_POINT(m_pGlobalState) )
		{
			m_pGlobalState->OnEvent(this, pSrc, nEventType, dwEventMisc1, dwEventMisc2);
		}
	}
	// 弱当前状态不处理该事件，则用全局状态处理
	else
	{
		if( FALSE == m_pCurrentState->OnEvent(this, pSrc, nEventType, dwEventMisc1, dwEventMisc2) )
		{
			if( VALID_POINT(m_pGlobalState) )
			{
				m_pGlobalState->OnEvent(this, pSrc, nEventType, dwEventMisc1, dwEventMisc2);
			}
		}
	}
}

//------------------------------------------------------------------------------------------
// 根据怪物移动类型得到状态转换器
//------------------------------------------------------------------------------------------
AITransition* AIController::GetTransitionByType()
{
	const tagCreatureProto* pProto = m_pOwner->GetProto();
	if( !VALID_POINT(pProto) ) return NULL;

	switch(pProto->eAIAction)
	{
	case AIAT_Attack:			// 攻击型
		return AggressiveTransition::Instance();
		
	case AIAT_Guard:			// 防守型
		return GuardTransition::Instance();

	case AIAT_Barbette:			// 炮塔型
		return BarbetteTransition::Instance();

	case AIAT_SpaceOut:			// 风筝型
		return SpaceOutTransition::Instance();

	case AIAT_PetAttack:		// 宠物攻击型
		return PetATTransition::Instance();

	case AIAT_Alert:
		return AlertAITransition::Instance();

	case AIAT_Hucheng:
		return HuchengAITransition::Instance();

	default:
		return NULL;
	}
}

//--------------------------------------------------------------------------------------------
// 更新巡逻
//--------------------------------------------------------------------------------------------
VOID AIController::UpdatePatrol()
{
	// 如果怪物本身就是不动的，直接return
	if( ECPT_Stand == m_pProto->ePatrolType || 
		ECPT_Null == m_pProto->ePatrolType )
		return;
	
	// 正在移动
	if( m_bIsPatroling )
	{

		if( EMS_Stand == m_pOwner->GetMoveData().m_eCurMove  )
		{
			// 已经到达终点，则原地休息一段时间
			m_bIsPatroling = FALSE;

			if( VALID_POINT( m_pWayPointTime ) && m_nWPIndex < m_nWPNum )
			{
				ReSetPatrolRestTick( m_pWayPointTime[m_nWPIndex]);
				if( VALID_POINT(m_pScript) )
					m_pScript->OnArrivalPoint( m_pOwner, m_nWPIndex );
			}
			else
			{
				ReSetPatrolRestTick( );
			}
		}
		return;
	}

	// 正在休息
	if( m_nPatrolRestTick > 0 )
	{
		m_nPatrolRestTick--;
		return;
	}

	// 开始新一次的巡逻
	StartPatrol();
}

//---------------------------------------------------------------------------------------------
// 开始巡逻
//---------------------------------------------------------------------------------------------
VOID AIController::StartPatrol()
{
	switch(m_pProto->ePatrolType)
	{
		// 站立和木桩
	case ECPT_Null:
	case ECPT_Stand:
	case ECPT_Wood:
		return;
		break;

		// 范围内随机移动 
	case ECPT_Range:
		{
			Vector3 vDest = m_pOwner->GetCurPos();

			INT nRange = m_pProto->nPatrolRadius * 2;
			if( 0 >= nRange ) return;

			vDest.x = FLOAT(get_tool()->tool_rand() % nRange - m_pProto->nPatrolRadius) + m_pOwner->GetBornPos().x;
			vDest.z = FLOAT(get_tool()->tool_rand() % nRange - m_pProto->nPatrolRadius) + m_pOwner->GetBornPos().z;

			if( MoveData::EMR_Success == m_pOwner->GetMoveData().StartCreatureWalk(vDest, EMS_CreaturePatrol) )
			{
				m_bIsPatroling = TRUE;
			}
		}
		break;

		// 往返路径
	case ECPT_Path:
	case ECPT_Patrol:  
		{
			if( !VALID_POINT(m_pWPArray) || m_nWPNum <= 1 ) return;

			// 先检测一下当前导航点索引
			if( !m_bReversePatrol && (m_nWPIndex == m_nWPNum - 1) )
			{
				// 正向巡逻已经到了最终点，则折返
				m_bReversePatrol = !m_bReversePatrol;

				// 我们需要在在这里做一些关于任务相关的东西w
				//
				if( m_pProto->ePatrolType == ECPT_Patrol )
					m_pOwner->OnReachEndPath( );
			}
			else if( m_bReversePatrol && 0 == m_nWPIndex )
			{
				// 反向巡逻已经到了最终点，则折返
				m_bReversePatrol = !m_bReversePatrol;
			}

			// 取出导航点类表中下一个点
			float fDif = Vec3DistSq(m_pOwner->GetCurPos(), m_pWPArray[m_nWPIndex] );
			if( fDif <= 100.0f ) 
			{
				if( m_bReversePatrol )
				{
					--m_nWPIndex;
				}
				else
				{
					++m_nWPIndex;
				}
			}

			// 取出下一个点
			Vector3 vDest = m_pWPArray[m_nWPIndex];

			if( m_pOwner->IsTeam() )
			{
				NPCTeam* pTeam = m_pOwner->GetTeamPtr();
				if( VALID_POINT(pTeam) )
					pTeam->OrderTeammateMove(m_pOwner, vDest);

				return;
			}

			if( MoveData::EMR_Success == m_pOwner->GetMoveData().StartCreatureWalk(vDest, EMS_CreaturePatrol) )
			{
				m_bIsPatroling = TRUE;
			}
		}
		break;

	

	default:
		break;
	}
}

// 暂停巡逻
VOID AIController::PausePatrol(const Vector3& vFace, DWORD dw_time)
{
	ReSetPatrolRestTick(dw_time);
	m_bIsPatroling = FALSE;
	m_pOwner->GetMoveData().StopMove(vFace);
}

// 移动到路点
BOOL AIController::MoveWayPoint(INT nIndex, EMoveState nMoveType)
{
	if (nIndex < 0 || nIndex > m_nWPNum)
		return FALSE;

	// 取出一个点
	Vector3 vDest = m_pWPArray[nIndex];
	if( MoveData::EMR_Success != m_pOwner->GetMoveData().StartCreatureWalk(vDest, nMoveType) )
	{
		return FALSE;
	}
	
	return TRUE;
}
//------------------------------------------------------------------------------------
// 锁敌行为更新 0-休息 1-没有找到敌人 2-找到敌人
//------------------------------------------------------------------------------------
DWORD AIController::UpdateLookForEnemy()
{
	const tagCreatureProto* pProto = m_pOwner->GetProto();
	if( !VALID_POINT(pProto) ) return 0;

	// 非主动怪不索敌
	if( pProto->eAIAction == AIAT_Guard )
		return 0;

	// 如果正在休息
	if( m_dwCallHelpTime > 0 )
	{
		m_dwCallHelpTime--;
		return 0;
	}

	// 如果休息时间到了，则开始锁敌
	ReSetLookForTargetTick();

	return StartLookForEnemy();
}


//------------------------------------------------------------------------------------
// 生物的求助逻辑
//------------------------------------------------------------------------------------
VOID AIController::UpdateCallHelp( )
{

	const tagCreatureProto* pProto = m_pOwner->GetProto();
	if( !VALID_POINT(pProto) ) return;

	if(  m_pProto->ePursueType != ECPST_BeHitCallHelp ) return;

	// 如果正在休息
	if( m_dwCallHelpTime > 0 )
	{
		m_dwCallHelpTime--;
		return;
	}

	ResetCallHelpTime();
	CallHelp();
}


//------------------------------------------------------------------------------------
// 开始锁敌 0-休息 1-没有找到敌人 2-找到敌人
//------------------------------------------------------------------------------------
DWORD AIController::StartLookForEnemy()
{
	tagVisTile* pVisTile[EUD_end] = {0};

	FLOAT fMaxDist = (FLOAT)m_pOwner->GetAttValue(ERA_Inspiration);
	if( fMaxDist < (FLOAT)m_pOwner->m_pProto->nAlertDis )
		fMaxDist   = (FLOAT)m_pOwner->m_pProto->nAlertDis;

	if( fMaxDist <= 0.0f  ) return 0;

	FLOAT fMaxDistSQ = fMaxDist * fMaxDist;

	// 得到视野范围内的vistile列表
	m_pOwner->get_map()->get_visible_tile(m_pOwner->GetMoveData().m_vPos, fMaxDist, pVisTile);

	Unit* pBestRole = NULL;
	FLOAT fNearestDistSq = FLOAT_MAX;

	// 这个地方可以再优化一下，如果本vistile能找到，则一定是最近的
	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;

		package_map<DWORD, Role*>::map_iter it = mapRole.begin();
		Role* pRole = NULL;

		while( mapRole.find_next(it, pRole) )
		{
			// 死亡人物不检测
			if( pRole->IsDead() ) continue;

			// 是否无敌
			if( pRole->IsInState(ES_Invincible)) continue;

			// 是否隐身
			if( pRole->IsInState(ES_Lurk)) continue;

			// 不是敌人不检测
			if( m_pOwner->FriendEnemy(pRole) != ETFE_Hostile ) continue;

			// 检测距离
			FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pRole->GetCurPos());

			// 如果距离小于之前最近的距离
			if( fDistSQ > fNearestDistSq || fDistSQ > fMaxDistSQ )
				continue;

			// 查看是否可达
			if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pRole->GetCurPos(), EMS_CreatureWalk, TRUE) )
			{
				pBestRole = pRole;
				fNearestDistSq = fDistSQ;
			}
			else
			{
				if( IsInAttackDist(pRole->GetID()) )
				{
					pBestRole = pRole;
				}
			}
		}


		// 怪现在需要攻击NPC了 
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;

		package_map<DWORD, Creature*>::map_iter it1 = mapCreature.begin();
		Creature* pCreature = NULL;
		
		while( mapCreature.find_next( it1, pCreature ) )
		{
			if( pCreature->IsDead() ) continue;

			// 自身的话，则均满足
			if( m_pOwner == pCreature )
			{
				continue;
			}

			if( m_pOwner->FriendEnemy(pCreature) != ETFE_Hostile ) continue;
			// 检测距离
			FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pCreature->GetCurPos());

			// 如果距离小于之前最近的距离
			if( fDistSQ > fNearestDistSq || fDistSQ > fMaxDistSQ )
				continue;

			// 查看是否可达
			if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pCreature->GetCurPos(), EMS_CreatureWalk, TRUE) )
			{
				pBestRole = pCreature;
				fNearestDistSq = fDistSQ;
			}
			else
			{
				if( IsInAttackDist(pCreature->GetID()) )
				{
					pBestRole = pCreature;
				}
			}
		}

	}

	// 找到
	if( VALID_POINT(pBestRole) )
	{
		AddEnmity(pBestRole, 1);

		SetTargetUnitID(pBestRole->GetID());
		// 应该换种方式了，可能影响怪物老的逻辑
		if( m_pOwner->GetProto()->ePatrolType ==  ECPT_Patrol )
		{	
			m_dwCurrentVictim =  pBestRole->GetID() ;
		}
		return 2;
	}

	// 没有找到
	ReSetLookForTargetTick();
	return 1;

}

//-----------------------------------------------------------------------------
// 宠物索敌 0-休息 1-没有找到敌人 2-找到敌人
//-----------------------------------------------------------------------------
DWORD AIController::UpdateLookForEnemyPet()
{
	// 如果正在休息
	if( m_nLookForTargetTick > 0 )
	{
		m_nLookForTargetTick--;
		return 0;
	}

	// 如果休息时间到了，则开始锁敌
	ReSetLookForTargetTick();

	return StartLookForEnemyPet();
}


DWORD	AIController::StartLookForEnemyPet()
{
	tagVisTile* pVisTile[EUD_end] = {0};

	// FLOAT fMaxDist = (FLOAT)m_pOwner->GetAttValue(ERA_Inspiration);
	FLOAT fMaxDist = 800.0f;
	if( fMaxDist <= 0.0f  ) return 0;

	FLOAT fMaxDistSQ = fMaxDist * fMaxDist;

	// 得到视野范围内的vistile列表
	m_pOwner->get_map()->get_visible_tile(m_pOwner->GetMoveData().m_vPos, fMaxDist, pVisTile);

	Unit* pBestRole = NULL;
	FLOAT fNearestDistSq = FLOAT_MAX;

	// 这个地方可以再优化一下，如果本vistile能找到，则一定是最近的
	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		package_map<DWORD, Creature*>& mapUnit = pVisTile[n]->map_creature;

		package_map<DWORD, Creature*>::map_iter it = mapUnit.begin();
		Creature* pRole = NULL;

		while( mapUnit.find_next(it, pRole) )
		{
			// 死亡人物不检测
			if( pRole->IsDead() ) continue;

			// 是否无敌
			if( pRole->IsInState(ES_Invincible)) continue;

			// 是否隐身
			if( pRole->IsInState(ES_Lurk)) continue;

			// 不是敌人不检测
			if( m_pOwner->FriendEnemy(pRole) != ETFE_Hostile ) continue;

			// 检测距离
			FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pRole->GetCurPos());

			// 如果距离小于之前最近的距离
			if( fDistSQ > fNearestDistSq || fDistSQ > fMaxDistSQ )
				continue;

			// 查看是否可达
			if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pRole->GetCurPos(), EMS_CreatureWalk, TRUE) )
			{
				pBestRole = pRole;
				fNearestDistSq = fDistSQ;
			}
		}
	}

	// 找到
	if( VALID_POINT(pBestRole) )
	{
		SetTargetUnitID( pBestRole->GetID() );
		return 2;
	}

	// 没有找到
	ReSetLookForTargetTick();
	return 1;
}

//------------------------------------------------------------------------------
// 护城NPC的索敌逻辑
// 
//------------------------------------------------------------------------------
void AIController::StartLookForEnemyHuCheng()
{
	FLOAT fMaxDist = (FLOAT)m_pOwner->GetAttValue(ERA_Inspiration);
	FLOAT fMaxDistSQ = fMaxDist * fMaxDist;
	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到视野范围内的vistile列表
	m_pOwner->get_map()->get_visible_tile(m_pOwner->GetMoveData().m_vPos, fMaxDist, pVisTile);

	Unit* pRoleDest = NULL;
	FLOAT fNearestDistSq = FLOAT_MAX;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		package_map<DWORD, Role*>& mapRole			= pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();
		
		Role* pRole = NULL;
		while( mapRole.find_next(it, pRole) )
		{
			// 死亡人物不检测
			if( pRole->IsDead() ) continue;

			// 不是敌人不检测
			if( m_pOwner->FriendEnemy(pRole) != ETFE_Hostile ) continue;

			// 检测距离
			FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pRole->GetCurPos());

			// 如果距离小于之前最近的距离
			if( fDistSQ > fNearestDistSq || fDistSQ > fMaxDistSQ )
				continue;

			// 查看是否可达
			if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pRole->GetCurPos(), EMS_CreatureWalk, TRUE) )
			{
				pRoleDest	   = pRole;
				fNearestDistSq = fDistSQ;
			}
		}

	}

	// 找到
	if( VALID_POINT( pRoleDest ) )
	{
		SetTargetUnitID( pRoleDest->GetID() );
		return ;
	}

	// 没有找到
	ReSetLookForTargetTick();
}

//-----------------------------------------------------------------------------
// 查找追击的目标
//-----------------------------------------------------------------------------
Unit* AIController::GetPursueTarget()
{
	if( VALID_POINT(m_dwTargeUnitID) )
	{
		if( IS_PLAYER(m_dwTargeUnitID) )
		{
			return m_pOwner->get_map()->find_role(m_dwTargeUnitID);
		}
		else if( IS_CREATURE(m_dwTargeUnitID) )
		{
			return m_pOwner->get_map()->find_creature(m_dwTargeUnitID);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

//------------------------------------------------------------------------------
// 找到与目标临近的点，如果找不到，返回FALSE
//------------------------------------------------------------------------------
BOOL AIController::GetPosNearPursueTarget(Unit* pTarget, Vector3& vPos)
{
	if( !VALID_POINT(pTarget) )
		return FALSE;

	// 计算目标位置
	const Vector3& vTargetPos = pTarget->GetCurPos();

	// 计算二维向量
	FLOAT fXDist = vTargetPos.x - m_pOwner->GetCurPos().x;
	FLOAT fZDist = vTargetPos.z - m_pOwner->GetCurPos().z;

	FLOAT fXDistABS = abs(fXDist);
	FLOAT fZDistABS = abs(fZDist);

	if( fXDistABS < 0.0001f && fZDistABS < 0.0001f )
	{
		vPos = vTargetPos;
		return TRUE;
	}

	// 近似取得x方向和z方向的距离分量
	FLOAT fX = (fXDistABS / (fXDistABS + fZDistABS)) * (CREATURE_PURSUE_NEAR_DIST + pTarget->GetSize().z + m_pOwner->GetSize().z);
	FLOAT fZ = (fZDistABS / (fXDistABS + fZDistABS)) * (CREATURE_PURSUE_NEAR_DIST + pTarget->GetSize().z + m_pOwner->GetSize().z);

	if(fX > fXDistABS && fZ > fZDistABS)
	{
		vPos = vTargetPos;
		return TRUE;
	}

	// 取得目标点
	vPos.x = ( (fXDist > 0) ? vTargetPos.x - fX : vTargetPos.x + fX );
	vPos.z = ( (fZDist > 0) ? vTargetPos.z - fZ : vTargetPos.z + fZ );

	// 取得y坐标
	if( !m_pOwner->NeedCollide() )
	{
		vPos.y = 0;
	}
	else
	{
		vPos.y = vTargetPos.y;
	}

	return TRUE;	
}

//------------------------------------------------------------------------------
// 检查目标是不是在攻击范围之内
//------------------------------------------------------------------------------
BOOL AIController::IsInAttackDist(DWORD dwTargetUnitID)
{
	m_dwUseSkillID = INVALID_VALUE;

	// 如果这个目标不合法，则返回错误
	if( !IsTargetValid(dwTargetUnitID) ) return FALSE;

	// 得到目标
	Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	if( !VALID_POINT(pTarget) ) return FALSE;

	
	if (m_pOwner->GetNormalNumber() > 0)
	{
		int index = get_tool()->tool_rand()%m_pOwner->GetNormalNumber();
		Skill* pNormalSkill = m_pOwner->GetNormalSKill(index);
		if( VALID_POINT(pNormalSkill) )
		{
			if( m_pOwner->IsInCombatDistance(*pTarget, pNormalSkill->GetOPDist(), 32) )
			{
				m_dwUseSkillID = pNormalSkill->GetID();
				return TRUE;
			}
		}
	}
	

	// 检测普通攻击和远程攻击
	//Skill* pMeleeSkill = m_pOwner->GetMeleeSkill();
	//Skill* pRangedSkill = m_pOwner->GetRangedSkill();
	//

	//// 先检测近程攻击
	//if( VALID_POINT(pMeleeSkill) )
	//{
	//	if( m_pOwner->IsInCombatDistance(*pTarget, pMeleeSkill->GetOPDist()) )
	//	{
	//		m_dwUseSkillID = pMeleeSkill->GetID();
	//		return TRUE;
	//	}
	//}

	//// 再检测远程攻击
	//if( VALID_POINT(pRangedSkill) )
	//{
	//	if( m_pOwner->IsInCombatDistance(*pTarget, pRangedSkill->GetOPDist()) )
	//	{
	//		m_dwUseSkillID = pRangedSkill->GetID();
	//		return TRUE;
	//	}
	//}

	return FALSE;
}

//------------------------------------------------------------------------------
// 检查目标是否在警戒范围
//------------------------------------------------------------------------------
BOOL AIController::IsInAlertDist( DWORD dwTargetID )
{

	if( IsInspiration(dwTargetID) )
		return false;
	// 得到目标
	Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetID);
	if( !VALID_POINT(pTarget) ) return FALSE;

	// 检查自己的数据是否全
	if( !m_pOwner->m_pProto )
		return FALSE;

	if( m_pOwner->IsInCombatDistance(*pTarget, (FLOAT)m_pOwner->m_pProto->nAlertDis ) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL AIController::IsInspiration( DWORD dwTargetUnitID )
{
	Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	if( !VALID_POINT(pTarget) ) return FALSE;

	if( !m_pOwner->m_pProto )
		return FALSE;

	if( m_pOwner->IsInCombatDistance(*pTarget, m_pOwner->GetAttValue(ERA_Inspiration) ) )
	{
		return TRUE;
	}

	return FALSE;
}
//------------------------------------------------------------------------------
// 追击目标
//------------------------------------------------------------------------------
VOID AIController::StartPursue(Unit* pTarget, BOOL bfanwei )
{
	if( !VALID_POINT(pTarget) ) return;

	// 停止寻路和其他一些状态
	m_bPursueFailed		=	FALSE;
	m_bCanNotPursue		=	FALSE;
	m_bPathFinding		=	FALSE;
	m_nCheckPursueTick	=	get_tool()->tool_rand() % 6 + 5;
	
	// 超过追击范围
	if (bfanwei)
	{
		FLOAT fDistSq = Vec3DistSq(m_pOwner->GetBornPos(), pTarget->GetCurPos());
		if (fDistSq > (m_pProto->nPursueRadius * m_pProto->nPursueRadius))
		{
			m_bPursueFailed = TRUE;
			ClearAllEnmity();
			return;
		}
	}

	// 先检测直线
	Vector3 vNearPos;
	Vector3 vTempPos = m_pOwner->GetMoveData().GetNearPos(pTarget->GetCurPos(), pTarget->GetFaceTo(), 48, 24);
	MoveData::EMoveRet eRet = m_pOwner->GetMoveData().IsCanCreatureWalk(vTempPos, EMS_CreatureWalk, TRUE, &vNearPos);

	// 可达
	if( MoveData::EMR_Success == eRet )
	{
		m_pOwner->GetMoveData().StartCreatureWalk(vTempPos, EMS_CreatureWalk, FALSE);
	}
	// 处在不能移动的状态
	else if( MoveData::EMR_SelfLimit == eRet )
	{
		m_bCanNotPursue = TRUE;
	}
	// 不可达
	else if( MoveData::EMR_NoArrive == eRet )
	{
		m_pOwner->GetMoveData().StartCreatureWalk(vTempPos, EMS_CreatureWalk, FALSE);

		// 如果最近的距离是可以攻击的距离，则直接过去
		//FLOAT fMaxDist = 0.0f;
		//if (m_pOwner->GetNormalNumber() > 0)
		//{
		//	for (int i = 0; i < NORAML_SKILL_NUMBER; i++)
		//	{
		//		Skill* pSkill	=	m_pOwner->GetNormalSKill(i);
		//		if( VALID_POINT(pSkill) )
		//		{
		//			if( pSkill->GetOPDist() > fMaxDist )
		//			{
		//				fMaxDist = pSkill->GetOPDist();
		//				break;
		//			}
		//		}
		//	}
		//}
		//
		//
		//// 得到路径点的最近点和目标的距离
		//FLOAT fDist = Vec3Dist(vNearPos, vTempPos);
		//Vector3 vNearPosTop = vNearPos;
		//vNearPosTop.y += m_pOwner->GetSize().y;

		//if( fDist < fMaxDist )
		//{
		//	m_pOwner->GetMoveData().StartCreatureWalk(vNearPos, EMS_CreatureWalk, FALSE);
		//}
		//else
		//{
		//	// 寻路
		//	Vector3 vPos;
		//	BOOL bRet = m_pPathFinder->FindingPath(m_pOwner->GetCurPos(), vTempPos);
		//	if( bRet && m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vPos) )
		//	{
		//		m_bPathFinding = TRUE;
		//		m_pOwner->GetMoveData().StartCreatureWalk(vPos, EMS_CreatureWalk, FALSE);
		//	}
		//	else
		//	{
		//		m_bPursueFailed = TRUE;
		//		//不能卡怪就清仇恨.奇怪的需求
		//		if (m_pOwner->m_pProto->bCantka)
		//		{
		//			ClearAllEnmity();
		//		}
		//		
		//	}
		//}
	}
	// 其他原因
	else
	{
		m_bPursueFailed = TRUE;
		ClearAllEnmity();
	}

	if( !m_bPursueFailed )
	{
		SetPursueTargetPos(pTarget->GetCurPos());
	}
}

//---------------------------------------------------------------------------------
// 更新追踪
//---------------------------------------------------------------------------------
VOID AIController::UpdatePursue(Unit* pTarget)
{
	// 重新追击倒计时
	--m_nCheckPursueTick;
	++m_nNoAnmityTick;
	++m_nNoAttackTick;
	// 如果追击失败，则取仇恨列表里面的其他目标
	if( m_bPursueFailed )
	{
		SetEnmityActive(pTarget->GetID(), FALSE);
		SetTargetUnitID(GetCurrentVictim());

		if( VALID_POINT(GetTargetUnitID()) )
		{
			pTarget = m_pOwner->get_map()->find_unit(GetTargetUnitID());
			if( VALID_POINT(pTarget) )
			{
				StartPursue(pTarget);
			}
		}
		return;
	}

	// 如果长时间没有增加仇恨，则追击失败
	if( m_nNoAnmityTick > m_pProto->nPursueTime * TICK_PER_SECOND/*CREATURE_RETURN_TICK*/ )
	{
		m_bPursueFailed = TRUE;
		return;
	}
	
	if( m_pProto->nNoAttackTime != 0 && m_nNoAttackTick > m_pProto->nNoAttackTime * TICK_PER_SECOND/*CREATURE_RETURN_TICK*/ )
	{
		m_bPursueFailed = TRUE;
		return;
	}

	// 检查追击状态
	if( m_bCanNotPursue )
	{
		if( !m_pOwner->IsInStateCantMove() )
		{
			StartPursue(pTarget);
		}
		return;
	}
	else
	{
		if( m_pOwner->IsInStateCantMove() )
		{
			m_bCanNotPursue = TRUE;
			return;
		}
	}


	//如果不能进水
	//FLOAT fY = 0.0f;
	//if (!m_pProto->bCanInWater && m_pOwner->get_map()->IfWillOnWater(pTarget->GetCurPos(), fY))
	//{
	//	m_bPursueFailed = TRUE;
	//	return;
	//}

	// 检查是否已经停下来了
	if( EMS_Stand == m_pOwner->GetMoveData().GetCurMoveState() )
	{
		if( m_pOwner->GetMoveData().IsStopped() )
		{
			// 是移动中断
			StartPursue(pTarget);
		}
		else
		{
			// 不是强制停下，说明走完了
			if( m_bPathFinding )
			{
				// 寻路中
				Vector3 vNewPos;
				if( m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vNewPos) )
				{
					m_pOwner->GetMoveData().StartCreatureWalk(vNewPos, EMS_CreatureWalk, FALSE);
					return;
				}
				else
				{
					StartPursue(pTarget);
					return;
				}
			}
			else
			{
				// 非寻路中
				StartPursue(pTarget);
				return;
			}
		}
	}

	// 检查重新检测倒计时是不是已经到了
	if( m_nCheckPursueTick < 0 )
	{
		m_nCheckPursueTick = get_tool()->tool_rand() % 6 + 5;
		if( abs(pTarget->GetCurPos().x - GetPursueTargetPos().x) > 50.0f || 
			abs(pTarget->GetCurPos().z - GetPursueTargetPos().z) > 50.0f )
		{
			StartPursue(pTarget);
		}
	}
}


//------------------------------------------------------------------------------
// 宠物追击目标, 暂时先用怪物的吧
//------------------------------------------------------------------------------
VOID AIController::StartPursuePet(Unit* pTarget)
{
	m_nCheckPursueTick	=	get_tool()->tool_rand() % 6 + 5;
}


//---------------------------------------------------------------------------------
// 宠物更新追踪
//---------------------------------------------------------------------------------
VOID AIController::UpdatePursuePet(Unit* pTarget)
{

	--m_nCheckPursueTick;

	// 如果在攻击范围内
	/*if( IsInAttackDist( GetTargetUnitID()) )
	{
		return;
	}*/

	// 检查重新检测倒计时是不是已经到了
	if( m_nCheckPursueTick < 0 )
	{
		m_nCheckPursueTick = get_tool()->tool_rand() % 6 + 5;
		if( abs(pTarget->GetCurPos().x - GetPursueTargetPos().x) > 50.0f || 
			abs(pTarget->GetCurPos().z - GetPursueTargetPos().z) > 50.0f )
		{
			StartPursue(pTarget);
		}
	}

}

//---------------------------------------------------------------------------------
// 使用AI技能
//---------------------------------------------------------------------------------
INT AIController::AIUseSkill(DWORD dwSkillID, DWORD dwTargetUnitID)
{
 	INT nRet = m_pOwner->GetCombatHandler().UseSkill(dwSkillID, dwTargetUnitID, m_dwSerial);

	if( E_Success == nRet )
	{
		Skill* pSkill = m_pOwner->GetSkill(dwSkillID);
		ASSERT( VALID_POINT(pSkill) );

		m_pOwner->GetMoveData().StopMove();
		m_dwUseSkillID = dwSkillID;

		NET_SIS_skill send;
		send.dwSrcRoleID = m_pOwner->GetID();
		send.dwTarRoleID = dwTargetUnitID;
		send.dwSkillID = pSkill->GetTypeID();
		send.nSpellTime = m_pOwner->GetCombatHandler().GetSkillPrepareCountDown();
		send.dwSerial = m_dwSerial;
		send.dw_error_code = E_Success;

		if( VALID_POINT(m_pOwner->get_map()) )
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);

		// 累加序列号
		++m_dwSerial;

		m_nNoAttackTick = 0;
	}
	else
	{
	}

	return nRet;
}

//----------------------------------------------------------------------------------
// 增加仇恨
//----------------------------------------------------------------------------------
VOID AIController::AddEnmity(Unit *pUnit, INT nValue, BOOL bSyncTeam)
{
	if( !VALID_POINT(pUnit) || pUnit->IsDead() || pUnit == m_pOwner)
		return;

	if (m_pOwner->FriendEnemy(pUnit) == ETFE_Friendly )
		return;
	
	if (!VALID_POINT(m_pTransition))
		return;

	tagEnmity* pEnmity = m_mapEnmity.find(pUnit->GetID());
	if( VALID_POINT(pEnmity) )
	{
		pEnmity->nEnmity += nValue;
		if( !pEnmity->bActive ) pEnmity->bActive = TRUE;
	}
	else
	{
		pEnmity = new tagEnmity;

		pUnit->AddEnmityCreature(m_pOwner);
		pEnmity->dwID = pUnit->GetID();
		pEnmity->nEnmity += nValue;
		m_mapEnmity.add(pUnit->GetID(), pEnmity);
	}

	// 计算最大仇恨
	CalMaxEnmity();

	// 重置仇恨无改变时间
	m_nNoAnmityTick = 0;

	// 如果是减少仇恨，重置锁敌倒计时
	if( nValue < 0 )
		m_nLockTargetTick = 0;

	if( bSyncTeam )
	{
		// 同步小队仇恨
		NPCTeam* pTeam = m_pOwner->GetTeamPtr();
		if(VALID_POINT(pTeam))
			pTeam->OnEvent(NTE_SynEnmity, pUnit, 1);
	}
}

//----------------------------------------------------------------------------------
// 增加仇恨值加成
//----------------------------------------------------------------------------------
VOID AIController::AddEnmityMod(Unit *pUnit, INT nValue, BOOL bSyncTeam)
{
	if( !VALID_POINT(pUnit) || pUnit->IsDead())
		return;

	tagEnmity* pEnmity = m_mapEnmity.find(pUnit->GetID());
	if( VALID_POINT(pEnmity) )
	{
		pEnmity->nEnmityMod += nValue;
	}
	else
	{
		pEnmity = new tagEnmity;

		pUnit->AddEnmityCreature(m_pOwner);
		pEnmity->dwID = pUnit->GetID();
		pEnmity->nEnmity += 1;
		pEnmity->nEnmityMod += nValue;
		m_mapEnmity.add(pUnit->GetID(), pEnmity);
	}

	// 计算最大仇恨
	CalMaxEnmity();

	// 重置仇恨无改变时间
	m_nNoAnmityTick = 0;

	if( bSyncTeam )
	{
		// 同步小队仇恨
		NPCTeam* pTeam = m_pOwner->GetTeamPtr();
		if(VALID_POINT(pTeam))
			pTeam->OnEvent(NTE_SynEnmity, pUnit, 1);
	}
}

//----------------------------------------------------------------------------------
// 清空怪物仇恨
//----------------------------------------------------------------------------------
VOID AIController::ClearAllEnmity()
{
	tagEnmity*	pEnmity		=	(tagEnmity*)INVALID_VALUE;
	DWORD		dwID		=	INVALID_VALUE;

	m_mapEnmity.reset_iterator();
	while( m_mapEnmity.find_next(dwID, pEnmity) )
	{
		if( VALID_POINT(pEnmity) )
		{
			Unit *pUnit = m_pOwner->get_map()->find_unit(pEnmity->dwID);
			if( VALID_POINT(pUnit) )
				pUnit->DelEnmityCreature(m_pOwner);
		}
		SAFE_DELETE(pEnmity);
	}

	m_mapEnmity.clear();

	m_dwMaxEnmityUnitID = INVALID_VALUE;
	m_dwCurrentVictim = INVALID_VALUE;
}

//----------------------------------------------------------------------------------
// 删除一个玩家的仇恨
//----------------------------------------------------------------------------------
VOID AIController::ClearEnmity(DWORD dw_role_id)
{
	tagEnmity *pEnmity = m_mapEnmity.find(dw_role_id);
	if( VALID_POINT(pEnmity) )
	{
		m_mapEnmity.erase(dw_role_id);
		SAFE_DELETE(pEnmity);
	}

	CalMaxEnmity();
}
VOID AIController::GetEnmityList(std::vector<tagEnmity*>& vecEnmityList)
{
	tagEnmity* pEnmity = NULL;
	package_map<DWORD, tagEnmity*>::map_iter it = m_mapEnmity.begin();
	while (m_mapEnmity.find_next(it, pEnmity))
	{
		if( VALID_POINT(pEnmity) )
		{
			vecEnmityList.push_back(pEnmity);
		}
	}
}

BOOL AIController::IsEnmityListEmpty()
{
	return m_mapEnmity.empty();
}
//----------------------------------------------------------------------------------
// 设置一个玩家的仇恨激活
//----------------------------------------------------------------------------------
VOID AIController::SetEnmityActive(DWORD dw_role_id, BOOL bActive)
{
	tagEnmity *pEnmity = m_mapEnmity.find(dw_role_id);
	if( VALID_POINT(pEnmity) )
	{
		pEnmity->bActive = bActive;
	}

	CalMaxEnmity();
}

//----------------------------------------------------------------------------------
// 计算仇恨最大的目标
//----------------------------------------------------------------------------------
VOID AIController::CalMaxEnmity()
{

	INT nMaxEnmity = 0;
	//INT nSecEnmity = 0;
	m_dwMaxEnmityUnitID		=	INVALID_VALUE;
	DWORD dwSecEnmityUnitID = INVALID_VALUE;

	tagEnmity* pEnmity		=	(tagEnmity*)INVALID_VALUE;

	package_map<DWORD, tagEnmity*>::map_iter it = m_mapEnmity.begin();
	while( m_mapEnmity.find_next(it, pEnmity) )
	{
		if( !VALID_POINT(pEnmity) || !pEnmity->bActive )
			continue;

		if( !IsTargetValid(pEnmity->dwID) )
		{
			pEnmity->bActive = FALSE;
			continue;
		}

		if( pEnmity->nEnmity + pEnmity->nEnmityMod > nMaxEnmity )
		{
			//nSecEnmity = nMaxEnmity;
			nMaxEnmity = pEnmity->nEnmity + pEnmity->nEnmityMod;
			dwSecEnmityUnitID = m_dwMaxEnmityUnitID;
			m_dwMaxEnmityUnitID = pEnmity->dwID;
		}
	}

	// 没有目标，则最大仇恨目标为当前目标
	if (m_dwCurrentVictim == INVALID_VALUE || !IsTargetValid(m_dwCurrentVictim) || m_dwMaxEnmityUnitID== INVALID_VALUE)
	{
		m_dwCurrentVictim = m_dwMaxEnmityUnitID;
		return;
	}
	// 当前目标为最大仇恨目标，则返回
	if (m_dwCurrentVictim == m_dwMaxEnmityUnitID)
	{
		return;
	}

	tagEnmity* pCurTargetEnmity = m_mapEnmity.find(m_dwCurrentVictim);
	if (!VALID_POINT(pCurTargetEnmity))
		return;
	
	INT nCurTargetEnmity = pCurTargetEnmity->nEnmity + pCurTargetEnmity->nEnmityMod;

	if (nMaxEnmity > 1.3 * nCurTargetEnmity)
	{
		m_dwCurrentVictim = m_dwMaxEnmityUnitID;
	}

}

//----------------------------------------------------------------------------------
// 最大的目标是否合法
//----------------------------------------------------------------------------------
BOOL AIController::IsTargetValid(DWORD dwID)
{
	Unit *pUnit = m_pOwner->get_map()->find_unit(dwID);
	if( !VALID_POINT(pUnit) )
		return FALSE;

	// 是否死亡
	if( pUnit->IsDead() )
		return FALSE;

	// 是否隐身
	if( pUnit->IsInState(ES_Lurk) ) 
		return FALSE;

	// 是否无敌
	if( pUnit->IsInState(ES_Invincible) && !IsEnmityListAllInvincible() )
		return FALSE;

	Map* pMap = pUnit->get_map();
	if( pMap != m_pOwner->get_map() )
		return FALSE;

	return TRUE;
}

//----------------------------------------------------------------------------------
// 更新锁敌倒计时
//----------------------------------------------------------------------------------
BOOL AIController::UpdateLockTarget()
{
	// 是否切换了目标
	BOOL bTargetChange = FALSE;

	// 找到目标
	while( VALID_POINT(GetTargetUnitID()) )
	{
		if( !IsTargetValid(GetTargetUnitID()) || GetTargetUnitID() != GetCurrentVictim())
		{
			SetEnmityActive(GetTargetUnitID(), FALSE);
			SetTargetUnitID(GetCurrentVictim());		
			ReSetLockTargetTime();
			bTargetChange = TRUE;
			continue;
		}
		else
		{
			break;
		}
	}

	// 如果没有切换目标
	if( !bTargetChange )
	{
		// 已经锁定目标
		if( m_nLockTargetTick > 0 )
		{
			m_nLockTargetTick--;
		}
		else
		{
			// 如果锁定倒计时结束，则重新开始计算
			ReSetLockTargetTime();

			if( GetTargetUnitID() != GetCurrentVictim() )
			{
				// 设置最大仇恨的目标位当前目标
				SetTargetUnitID(GetCurrentVictim());
				bTargetChange = TRUE;
			}
		}
	}

	return bTargetChange;
}

//----------------------------------------------------------------------------------
// 根据目标类型获得目标ID
//----------------------------------------------------------------------------------
DWORD AIController::GetTargetIDByType(ECreatureTargetFriendEnemy eTargetType)
{
	DWORD dwTargetID = INVALID_VALUE;
	switch(eTargetType)
	{
	case ECFE_NULL:
		return dwTargetID;

	case ECFE_Self:
		return m_pOwner->GetID();

	case ECFE_Leader:
		{
			NPCTeam* pTeam = m_pOwner->GetTeamPtr();
			if( VALID_POINT(pTeam) )
			{
				dwTargetID = pTeam->GetLeaderID();
			}
			return dwTargetID;
		}

	case ECFE_RandPlayer:
		{
			dwTargetID = RandRoleInEnmityList();
			return dwTargetID;
		}
	case ECFE_MinEnimty:
		{
			dwTargetID = GetMinEnmityInEnmityList();
			return dwTargetID;
		}
	case ECFE_CurPlayer:
		{
			return m_dwTargeUnitID;
		}
	case ECFE_CurPlayerNotFirst:
		{
			dwTargetID = RandRoleInEnemityListNotFirst();
			return dwTargetID;
		}
	default:
		return dwTargetID;
	}
}


//----------------------------------------------------------------------------------
// 怪物仇恨列表中随机抽取一个目标
//----------------------------------------------------------------------------------
DWORD AIController::RandRoleInEnmityList()
{
	tagEnmity*	pEnmity	=	NULL;
	DWORD		dwID	=	INVALID_VALUE;

	if( m_mapEnmity.empty() ) return INVALID_VALUE;

	m_mapEnmity.rand_find(dwID, pEnmity);
	return dwID;
}
//----------------------------------------------------------------------------------
// 怪物仇恨列表中随机抽取一个目标 不包括最大仇恨
//----------------------------------------------------------------------------------
DWORD AIController::RandRoleInEnemityListNotFirst()
{
	tagEnmity*	pEnmity	=	NULL;
	DWORD		dwID	=	INVALID_VALUE;

	if( m_mapEnmity.empty() ) return INVALID_VALUE;
	
	std::vector<DWORD> vecID;
	m_mapEnmity.reset_iterator();
	while( m_mapEnmity.find_next(dwID, pEnmity))
	{
		if (dwID != m_dwCurrentVictim)
		{
			vecID.push_back(dwID);
		}
	}

	INT n_size = vecID.size();
	if( n_size <= 0 )
		return INVALID_VALUE;

	INT n_rand = rand() % n_size;

	return vecID[n_rand];
}
//----------------------------------------------------------------------------------
// 怪物仇恨列表中仇恨最小目标
//----------------------------------------------------------------------------------
DWORD AIController::GetMinEnmityInEnmityList()
{
	tagEnmity*	pEnmity		=	NULL;
	DWORD		dwID		=	INVALID_VALUE;
	DWORD		dwTargetID	=	INVALID_VALUE;
	INT			nValue		=	INT_MAX;

	package_map<DWORD, tagEnmity*>::map_iter it = m_mapEnmity.begin();

	while( m_mapEnmity.find_next(it, dwID, pEnmity) )
	{
		if( pEnmity->nEnmity + pEnmity->nEnmityMod < nValue )
		{
			nValue = pEnmity->nEnmity + pEnmity->nEnmityMod;
			dwTargetID = dwID;
		}
	}

	return dwTargetID;
}

//----------------------------------------------------------------------------------
// 得到最大仇恨值
//----------------------------------------------------------------------------------
INT AIController::GetEnmityValue(DWORD dwID)
{
	tagEnmity* pEnmity = m_mapEnmity.find(dwID);
	if( VALID_POINT(pEnmity) )	return pEnmity->nEnmity + pEnmity->nEnmityMod;

	return 0;
}

//----------------------------------------------------------------------------------
// 得到基础仇恨值
//----------------------------------------------------------------------------------
INT AIController::GetBaseEnmityValue(DWORD dwID)
{
	tagEnmity* pEnmity = m_mapEnmity.find(dwID);
	if( VALID_POINT(pEnmity) ) return pEnmity->nEnmity;

	return 0;
}

//----------------------------------------------------------------------------------
// 设置仇恨加值
//----------------------------------------------------------------------------------
VOID AIController::ClearEnmityModValue(DWORD dwID)
{
	tagEnmity* pEnmity = m_mapEnmity.find(dwID);
	if( VALID_POINT(pEnmity) ) 
	{
		pEnmity->nEnmityMod = 0;
		CalMaxEnmity();
	}
}

//----------------------------------------------------------------------------------
// 计算得到一个怪物逃跑的目标点
//----------------------------------------------------------------------------------
VOID AIController::CalFleePos()
{
	// 第一次选择逃跑目标点
	if(EFT_FirstEnter == m_eFleeTime)
	{
		INT	  nIndex = 0;
		while(TRUE)
		{
			nIndex += 1;

			// 随机得到一个逃跑的距离
			m_vFleeDir.fx = (FLOAT)(get_tool()->tool_rand() % X_RANGE_FLEE_RADIUS) + X_MIN_FLEE_RADIUS;
			m_vFleeDir.fy = 0.0f;

			// 随机得到逃跑的方向向量
			FLOAT fAng = (FLOAT)(get_tool()->tool_rand() % 360);

			// 计算逃跑目标点
			m_vFleeDir.Vec2DRotateAroundOrigin(fAng*3.1415927f / 180.0f);

			m_vFleePos.x = m_vFleeDir.fx + m_pOwner->GetCurPos().x;
			m_vFleePos.z = m_vFleeDir.fy + m_pOwner->GetCurPos().z;

			if(m_pOwner->NeedCollide())
			{
				m_vFleePos.y = m_pOwner->GetCurPosTop().y;
				break;
			}
			else
			{
				if( m_pOwner->get_map()->if_can_go(m_vFleePos.x, m_vFleePos.z) )
				{
					m_vFleePos.y = 0;
					break;
				}
			}

			if (nIndex > 10)	break;
		}
	}
	else
	{
		INT	  nIndex = 0;
		while(TRUE)
		{
			nIndex += 1;

			// 随机得到逃跑的方向向量
			FLOAT fAng = (FLOAT)(get_tool()->tool_rand() % 160) - 80.0f;

			// 计算逃跑目标点
			m_vFleeDir.Vec2DRotateAroundOrigin(fAng*3.1415927f / 180.0f);

			m_vFleePos.x = m_vFleeDir.fx + m_pOwner->GetCurPos().x;
			m_vFleePos.z = m_vFleeDir.fy + m_pOwner->GetCurPos().z;

			if(m_pOwner->NeedCollide())
			{
				m_vFleePos.y = m_pOwner->GetCurPosTop().y;
				break;
			}
			else
			{
				if( m_pOwner->get_map()->if_can_go(m_vFleePos.x, m_vFleePos.z) )
				{
					m_vFleePos.y = 0;
					break;
				}
			}

			if (nIndex > 10)	break;
		}
	}
}

//------------------------------------------------------------------------------
// 逃跑
//------------------------------------------------------------------------------
VOID AIController::StartFlee(Vector3 vFleePos)
{
	m_bArrivedFleePos	=	FALSE;
	m_bPursueFailed		=	FALSE;
	m_bCanNotPursue		=	FALSE;
	m_bPathFinding		=	FALSE;

	// 先检测直线
	Vector3 vNearPos;
	MoveData::EMoveRet eRet = m_pOwner->GetMoveData().IsCanCreatureWalk(vFleePos, EMS_CreatureFlee, TRUE, &vNearPos);

	// 可达
	if( MoveData::EMR_Success == eRet )
	{
		m_pOwner->GetMoveData().StartCreatureWalk(vFleePos, EMS_CreatureFlee, FALSE);
	}
	// 处在不能移动的状态
	else if( MoveData::EMR_SelfLimit == eRet )
	{
		m_bCanNotPursue = TRUE;
	}
	// 不可达
	else if( MoveData::EMR_NoArrive == eRet )
	{
		// 寻路
		Vector3 vPos;
		BOOL bRet = m_pPathFinder->FindingPath(m_pOwner->GetCurPos(), vFleePos);
		if( bRet && m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vPos) )
		{
			m_bPathFinding = TRUE;
			m_pOwner->GetMoveData().StartCreatureWalk(vPos, EMS_CreatureFlee, FALSE);
		}
		else
		{
			m_bPursueFailed = TRUE;
		}
	}
	// 其他原因
	else
	{
		m_bPursueFailed = TRUE;
	}

	if( !m_bPursueFailed )
	{
		SetFleePos(vFleePos);
	}
}

//---------------------------------------------------------------------------------
// 更新逃跑
//---------------------------------------------------------------------------------
VOID AIController::UpdateFlee()
{
	// 没有找到逃跑的目标点,则重新计算
	if( m_bPursueFailed )
	{
		CalFleePos();
		StartFlee(m_vFleePos);
		return;
	}

	// 检查逃跑状态
	if( m_bCanNotPursue )
	{
		if( !m_pOwner->IsInStateCantMove() )
		{
			StartFlee(m_vFleePos);
		}
		return;
	}
	else
	{
		if( m_pOwner->IsInStateCantMove() )
		{
			m_bCanNotPursue = TRUE;
			return;
		}
	}

	// 检查是否已经停下来了
	if( EMS_Stand == m_pOwner->GetMoveData().GetCurMoveState() )
	{
		if( m_pOwner->GetMoveData().IsStopped() )
		{
			// 是移动中断
			StartFlee(m_vFleePos);
		}
		else
		{
			// 不是强制停下，说明走完了
			if( m_bPathFinding )
			{
				// 寻路中
				Vector3 vNewPos;
				if( m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vNewPos) )
				{
					m_pOwner->GetMoveData().StartCreatureWalk(vNewPos, EMS_CreatureFlee, FALSE);
					return;
				}
				else
				{
					m_bArrivedFleePos = TRUE;
					return;
				}
			}
			else
			{
				// 非寻路中
				m_bArrivedFleePos = TRUE;
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------------
// 计算得到一个怪物求救的目标点
//----------------------------------------------------------------------------------
BOOL AIController::CalHelpPos()
{
	// 第一次选择逃跑目标点
	if(EFT_FirstEnter == m_eFleeTime)
	{
		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到视野范围内的vistile列表
		FLOAT fViewDist = (FLOAT)m_pOwner->GetProto()->dwHelpRange;
		FLOAT fMaxView  = fViewDist * fViewDist;
		m_pOwner->get_map()->get_visible_tile(m_pOwner->GetMoveData().m_vPos, fViewDist, pVisTile);

		Creature* pCreature = NULL;
		FLOAT fNearestDistSq = FLOAT_MAX;

		// 添加呼救buff，用于播放呼救动作
		if(m_pOwner->GetProto()->eHelpType == EHT_Stand)
		{
			const tagBuffProto* pBuff = AttRes::GetInstance()->GetBuffProto(2010101);
			if(VALID_POINT(pBuff))
			{
				m_pOwner->TryAddBuff(m_pOwner, pBuff, NULL, NULL, NULL);
			}
		}

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;

			package_map<DWORD, Creature*>::map_iter it = mapCreature.begin();
			Creature* pCreature = NULL;

			while( mapCreature.find_next(it, pCreature) )
			{
				// 死亡怪物不检测
				if( pCreature->IsDead() ) continue;

				// 自己不检测
				if( pCreature->GetID() == m_pOwner->GetID() )	continue;

				// 如果是NPC不检测
				if(pCreature->IsNPC()) continue;

				const tagCreatureProto* pProto = pCreature->GetProto();
				if( !VALID_POINT(pProto) || ECPST_CallHelp != m_ePursueType ) continue;

				// 敌方不检测
				if( m_pOwner->FriendEnemy(pCreature) != ETFE_Friendly ) continue;

				// 求救目标的AI为空闲状态
				if( !VALID_POINT(pCreature->GetAI()) ) continue;
				if( AIST_Idle != pCreature->GetAI()->GetCurrentStateType() ) continue;

				// 检测距离
				FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pCreature->GetCurPos());

				if(m_pOwner->GetProto()->eHelpType == EHT_Stand)
				{
					if(fDistSQ > X_MAX_SOS_RADIUS_SQ)
						continue;
				
					if(m_listHelpID.size() >= m_pOwner->GetProto()->nHelpNum)
						continue;

					m_listHelpID.push_back(pCreature->GetID());
				}
				else
				{
					// 如果距离小于之前最近的距离
					if( fDistSQ > fNearestDistSq || fDistSQ > X_MAX_SOS_RADIUS_SQ )
						continue;

					// 查看是否可达
					if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pCreature->GetCurPos(), EMS_CreatureWalk, TRUE) )
					{
						GetPosNearPursueTarget(pCreature, m_vFleePos);
						fNearestDistSq = fDistSQ;
						m_dwHelperID = pCreature->GetID();
					}
				}
				
			}
		}

		if( m_dwHelperID != INVALID_VALUE || !m_listHelpID.empty())
			return TRUE;
	}
	else
	{
		Creature* pCreature = m_pOwner->get_map()->find_creature(m_dwHelperID);
		if( !VALID_POINT(pCreature) )
			return FALSE;

		// 查看是否可达
		if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pCreature->GetCurPos(), EMS_CreatureWalk, TRUE) )
		{
			GetPosNearPursueTarget(pCreature, m_vFleePos);
			return TRUE;
		}
	}

	return FALSE;
}


// 无限制的呼救
VOID  AIController::CallHelp( )
{

	// pTarget = 
	if( !VALID_POINT( m_dwTargeUnitID ) ) return ;

	Unit* pRole =m_pOwner->get_map()->find_unit(GetTargetUnitID());
	if( !VALID_POINT( pRole ) ) return;


	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到视野范围内的vistile列表
	FLOAT fViewDist = (FLOAT)m_pOwner->GetProto()->dwHelpRange;
	FLOAT fMaxView  = fViewDist * fViewDist;
	m_pOwner->get_map()->get_visible_tile(m_pOwner->GetMoveData().m_vPos, fViewDist, pVisTile);

	Creature* pCreature = NULL;
	FLOAT fNearestDistSq = FLOAT_MAX;

	for( INT n = EUD_center; n < EUD_end; n++ )
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it = mapCreature.begin();
		Creature* pCreature = NULL;

		while( mapCreature.find_next(it, pCreature) )
		{
			// 死亡怪物不检测
			if( pCreature->IsDead() ) continue;

			// 自己不检测
			if( pCreature->GetID() == m_pOwner->GetID() )	continue;

			const tagCreatureProto* pProto = pCreature->GetProto();
			if( !VALID_POINT(pProto)  ) continue;

			if( pProto->dw_data_id != m_pOwner->GetProto()->dw_data_id ) continue;

			// 敌方不检测
			if( m_pOwner->FriendEnemy(pCreature) != ETFE_Friendly ) continue;

			// 求救目标的AI为空闲状态
			if( !VALID_POINT(pCreature->GetAI()) ) continue;
			if( AIST_Idle != pCreature->GetAI()->GetCurrentStateType() ) continue;

			// 检测距离
			FLOAT fDistSQ = Vec3DistSq(m_pOwner->GetCurPos(), pCreature->GetCurPos());

			// 如果距离小于之前最近的距离
			if( fDistSQ > fNearestDistSq || fDistSQ > X_MAX_SOS_RADIUS_SQ )
				continue;

			// 查看是否可达
			if( MoveData::EMR_Success == m_pOwner->GetMoveData().IsCanCreatureWalk(pCreature->GetCurPos(), EMS_CreatureWalk, TRUE) )
			{
				pCreature->GetAI()->AddEnmity( pRole,1 );
			}
		}
	}

}
//----------------------------------------------------------------------------------
// 是否需要重新计算呼救目标点
//----------------------------------------------------------------------------------
BOOL AIController::NeedReCalHelpPos()
{
	Creature* pCreature = m_pOwner->get_map()->find_creature(m_dwHelperID);
	if( VALID_POINT(pCreature) )
	{
		FLOAT fDistanceX = m_vFleePos.x - pCreature->GetCurPos().x;
		FLOAT fDistanceZ = m_vFleePos.z - pCreature->GetCurPos().z;
		FLOAT fDistanceSquare = fDistanceX * fDistanceX + fDistanceZ * fDistanceZ;
		FLOAT fMaxHelpDist = CREATURE_PURSUE_NEAR_DIST + pCreature->GetSize().z + m_pOwner->GetSize().z;

		if(fDistanceSquare > fMaxHelpDist*fMaxHelpDist)
			return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
// 返回
//------------------------------------------------------------------------------
VOID AIController::StartReturn()
{
	m_bPathFinding		=	FALSE;
	m_bArrivedReturnPos	=	FALSE;
	m_nNoAttackTick		=	0;

	// 先检测直线
	MoveData::EMoveRet eRet = MoveData::EMR_Success;

	if( m_pOwner->NeedCollide() )
		eRet = m_pOwner->GetMoveData().IsCanCreatureWalk(GetEnterCombatPos(), EMS_CreatureWalk, TRUE);
	else
		eRet = m_pOwner->GetMoveData().IsCanCreatureWalk(GetEnterCombatPos(), EMS_CreatureWalk, FALSE);

	// 可达
	if( MoveData::EMR_Success == eRet )
	{
		m_pOwner->GetMoveData().StartCreatureWalk(GetEnterCombatPos(), EMS_CreatureWalk, FALSE);
	}
	// 不可达
	else if( MoveData::EMR_NoArrive == eRet || MoveData::EMR_SelfLimit == eRet)
	{
		m_pOwner->GetMoveData().StartCreatureWalk(GetEnterCombatPos(), EMS_CreatureWalk, FALSE);

		// 寻路
		//Vector3 vPos;
		//BOOL bRet = m_pPathFinder->FindingPath(m_pOwner->GetCurPos(), GetEnterCombatPos());
		//if( bRet && m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vPos) )
		//{
		//	m_bPathFinding = TRUE;
		//	m_pOwner->GetMoveData().StartCreatureWalk(vPos, EMS_CreatureWalk, FALSE);
		//}
		//else
		//{
		//	m_bArrivedReturnPos = TRUE;
		//	m_pOwner->GetMoveData().ForceTeleport(m_pOwner->GetBornPos(),TRUE);
		//}
	}
	// 其他原因
	else
	{
		m_bArrivedReturnPos = TRUE;
		m_pOwner->GetMoveData().ForceTeleport(m_pOwner->GetBornPos(),TRUE);
	}
}

VOID AIController::UpdateReturn()
{
	// 检查是否已经停下来了
	if( EMS_Stand == m_pOwner->GetMoveData().GetCurMoveState() )
	{
			// 不是强制停下，说明走完了
		if( m_bPathFinding )
		{
			// 寻路中
			Vector3 vNewPos;
			if( m_pPathFinder->GetCurPathPoint(m_pOwner->GetCurPos(), vNewPos) )
			{
				m_pOwner->GetMoveData().StartCreatureWalk(vNewPos, EMS_CreatureWalk, FALSE);
				return;
			}
			else
			{
				m_bArrivedReturnPos = TRUE;
				return;
			}
		}
		else
		{
			// 非寻路中
			m_bArrivedReturnPos = TRUE;
			return;

		}
	}
}

//------------------------------------------------------------------------------
// 计算拉开距离时的目标点
//------------------------------------------------------------------------------
BOOL AIController::CalSpaceOutPos()
{
	// 找到当前目标
	Unit* pTarget = GetPursueTarget();
	if( !VALID_POINT(pTarget) )  return FALSE;

	// 随机得到方向向量
	FLOAT fAng = (FLOAT)(get_tool()->tool_rand() % 360);

	// 计算目标点
	CVector2D vDir(CREATURE_SPACE_OUT_DIST);
	vDir.Vec2DRotateAroundOrigin(fAng*3.1415927f / 180.0f);

	m_vFleePos.x = vDir.fx + pTarget->GetCurPos().x;
	m_vFleePos.z = vDir.fy + pTarget->GetCurPos().z;

	if(m_pOwner->NeedCollide())
	{
		m_vFleePos.y = m_pOwner->GetCurPosTop().y;
		return TRUE;
	}
	else
	{
		if( m_pOwner->get_map()->if_can_go(m_vFleePos.x, m_vFleePos.z) )
		{
			m_vFleePos.y = 0;
			return TRUE;
		}
	}

	return FALSE;
}

//------------------------------------------------------------------------------
// 与玩家拉开距离
//------------------------------------------------------------------------------
VOID AIController::StartSpaceOut()
{
	m_bArrivedReturnPos	=	FALSE;

	// 下一次时的触发间隔
	m_dwSpaceOutTick	=	get_tool()->rand_in_range(CREATURE_SPACE_OUT_MIN, CREATURE_SPACE_OUT_MAX);

	// 检测直线
	MoveData::EMoveRet eRet = m_pOwner->GetMoveData().IsCanCreatureWalk(m_vFleePos, EMS_CreatureWalk, TRUE);

	// 可达
	if( MoveData::EMR_Success == eRet )
	{
		m_pOwner->GetMoveData().StartCreatureWalk(m_vFleePos, EMS_CreatureWalk, FALSE);
	}
	// 不可达
	else if( MoveData::EMR_NoArrive == eRet || MoveData::EMR_SelfLimit == eRet)
	{
		m_bArrivedPos = TRUE;
	}
	// 其他原因
	else
	{
		m_bArrivedPos = TRUE;
	}

}

VOID AIController::UpdateSpaceOut()
{
	// 检查是否已经停下来了
	if( EMS_Stand == m_pOwner->GetMoveData().GetCurMoveState() )
	{
		if( m_pOwner->GetMoveData().IsStopped() )
		{
			// 是移动中断
			StartSpaceOut();
		}
		else
		{
			m_bArrivedPos = TRUE;
		}
	}
}

//----------------------------------------------------------------------------------
// 检测仇恨列表中的玩家是否都处于无敌状态
//----------------------------------------------------------------------------------
BOOL AIController::IsEnmityListAllInvincible()
{
	tagEnmity*	pEnmity		=	(tagEnmity*)INVALID_VALUE;
	Unit*		pTarget		=	NULL;	

	m_mapEnmity.reset_iterator();
	while( m_mapEnmity.find_next(pEnmity) )
	{
		if( !VALID_POINT(pEnmity) )
			continue;

		pTarget = m_pOwner->get_map()->find_unit(pEnmity->dwID);
		if( !VALID_POINT(pTarget) )
			continue;

		// 是否死亡
		if( pTarget->IsDead() )
			continue;

		// 是否隐身
		if( pTarget->IsInState(ES_Lurk) ) 
			continue;

		if( !pTarget->IsInState(ES_Invincible) )
			return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------
// 更新触发器
//------------------------------------------------------------------------------------------
VOID AIController::UpdateTriggerMgr()
{
	// 更新触发器
	if( VALID_POINT(m_pAITrigger) && !m_pAITrigger->IsPaused() )
	{
		m_pAITrigger->Update();
	}
}

//------------------------------------------------------------------------------------------
// 处在不能更新AI的状态
//------------------------------------------------------------------------------------------
BOOL AIController::IsInStateCantUpdateAI()
{
	return m_pOwner->IsInState(ES_Dead) || m_pOwner->IsInState(ES_Dizzy) || m_pOwner->IsInState(ES_Feat); // || 击飞
}


//------------------------------------------------------------------------------------------
// 更新当前状态
//------------------------------------------------------------------------------------------
VOID AIController::UpdateCurrentState()
{
	// 更新当前AI状态
	if( VALID_POINT(m_pCurrentState) )
		m_pCurrentState->Update(this);

	// 更新全局AI状态
	if( VALID_POINT(m_pGlobalState) )
		m_pGlobalState->Update(this);
}

//------------------------------------------------------------------------------------------
// 更新状态切换
//------------------------------------------------------------------------------------------
VOID AIController::UpdateTransition()
{
	if( !VALID_POINT(m_pTransition) ) return;

	// 检测状态是否转换
	AIStateType eNewState = m_pTransition->Transition(this, m_eCurAIState);

	if( m_eCurAIState != eNewState )
	{
		ChangeState(eNewState);
	}
}

//------------------------------------------------------------------------------------------
// 更新状态机
//------------------------------------------------------------------------------------------
VOID AIController::UpdateAIController()
{
	// 如果无脚本，不更新
	if( !VALID_POINT(m_pScript) ) return;

	// 检测现在是否已经暂停
	if( INVALID_VALUE == m_nScriptUpdateTimer ) return;

	// 倒计时
	if( --m_nScriptUpdateCountDown > 0 ) return;

	// 调用脚本
	m_nScriptUpdateCountDown = m_nScriptUpdateTimer;
	m_pScript->OnUpdateAI(m_pOwner);
}

//------------------------------------------------------------------------------------------
// 根据状态类型得到状态对象
//------------------------------------------------------------------------------------------
AIState* AIController::GetStateByType( AIStateType eStateType )
{
	switch (eStateType)
	{
	case AIST_Idle:
		return AIStateIdle::Instance();

	case AIST_Pursue:
		return AIStatePursue::Instance();

	case AIST_Attack:
		return AIStateAttack::Instance();

	case AIST_Return:
		return AIStateReturn::Instance();

	case AIST_Flee:
		return AIStateFlee::Instance();

	case AIST_SOS:
		return AIStateCallHelp::Instance();

	case AIST_SpaceOut:
		return AIStateSpaceOut::Instance();

		//---------------------------------------------------------------------------------------
		// 宠物AI
	case AIST_PetIdle:
		return AIStatePetIdle::Instance();

	case AIST_PetAttack:
		return AIStatePetAttack::Instance();

	case AIST_Pet_Pursue:
		return AIStatePetPursue::Instance();
		//--------------------------------------------------------------------------------------

	case AIST_Alert:
		return AIStateAlert::Instance();

	case AIST_Talk:
		return AIStateTalk::Instance();

	case AIST_Follow:
		return AIStateFollow::Instance();

	default:
		return AIStateScript::Instance();
	}
}

//---------------------------------------------------------------------------
// 改变当前AI状态
//----------------------------------------------------------------------------
VOID AIController::ChangeState( AIStateType eState )
{
	if( VALID_POINT(m_pCurrentState) )
		m_pCurrentState->OnExit(this);

	m_eCurAIState = eState;
	m_pCurrentState = GetStateByType(eState);

	if( VALID_POINT(m_pCurrentState) )
		m_pCurrentState->OnEnter(this);
}
