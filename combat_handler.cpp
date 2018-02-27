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
*	@file		combat_handler.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		战斗系统管理器
*/

#include "stdafx.h"

#include "../../common/WorldDefine/combat_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/ride_protocol.h"

#include "unit.h"
#include "map.h"
#include "creature.h"
#include "role.h"
#include "combat_handler.h"
#include "script_mgr.h"
#include "title_mgr.h"
#include "creature_ai.h"
#include "map_creator.h"
#include "map_mgr.h"
//-----------------------------------------------------------------------------
// 使用技能
//-----------------------------------------------------------------------------
INT CombatHandler::UseSkill(DWORD dwSkillID, DWORD dwTargetUnitID,DWORD dwSerial, Vector3 vDesPos)
{
	Skill* pSkill = m_pOwner->GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	Unit* pTargetUnit = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	
	if (m_pOwner->IsRole())
	{
		((Role*)m_pOwner)->SetCurSkillTarget(dwTargetUnitID);
	}
	

	// 检查能否对该目标发动技能
	INT nRet = CanCastSkill(pSkill, dwTargetUnitID , vDesPos);
	if( E_Success != nRet )	
		return nRet;
	
	// 火墙类技能
	if (pSkill->GetProto()->bPoint)
	{
		Vector3 vDPos = vDesPos;
		if (VALID_POINT(pTargetUnit))
		{
			vDPos = pTargetUnit->GetCurPos();
		}

		int nTitleX = vDPos.x / TILE_SCALE;
		int nTitleY = vDPos.z / TILE_SCALE;
		
		// 让点对其格子
		vDPos.x = nTitleX * TILE_SCALE;
		vDPos.z = nTitleY * TILE_SCALE;

		int nTitleIndex = nTitleX + nTitleY * m_pOwner->get_map()->get_map_info()->n_width;
		std::map<DWORD, tagPilotUnit*>::iterator it = m_listPilotUnit.find(nTitleIndex);
		if ( it != m_listPilotUnit.end())
		{
			tagPilotUnit* pPUnit = it->second;
			pPUnit->dwOverTime = pSkill->GetPrepareTime() / 3;
		}
		else
		{
			Creature *pCreature = m_pOwner->get_map()->create_creature(pSkill->GetProto()->dwTargetTriggerID, vDPos, Vector3(0, 0, 0));
			
			if (!VALID_POINT(pCreature))
				return INVALID_VALUE;

			tagPilotUnit* pPUnit = new tagPilotUnit;
			pPUnit->dwMapID = m_pOwner->GetMapID();
			pPUnit->dwInstanceID = m_pOwner->get_map()->get_instance_id();
			pPUnit->dwSkillID = pSkill->GetID();
			pPUnit->dwCretureID = pCreature->GetID();
			pPUnit->dwOverTime = pSkill->GetPrepareTime() / 3;
			pPUnit->fOPRadius = pSkill->GetOPRadius();
			m_listPilotUnit[nTitleIndex] = pPUnit;
		}

		return nRet;
	}


	//// 该机能有武器限制，则减少相应武器崭新度
	if( m_pOwner->IsRole() )
		((Role*)m_pOwner)->GetItemMgr().ProcEquipNewness();
	
	// 防具耐久度处理
	//if(m_pOwner->IsRole())
	//{
	//	((Role*)m_pOwner)->GetItemMgr().ProcArmorNewness();
	//}
	

	// 检查该技能是否能够移动施放
	
	if (pSkill->GetDmgTimes() == 0)
	{
		// 没有伤害的技能直接打断
		m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// 打断使用技能打断的buff
	}
	
	m_vPersistSkillPos	=	vDesPos;

	if( pSkill->GetProto()->eOPType == ESOPT_Persist)  //如果是持续技能
	{
		m_pOwner->GetMoveData().StopMoveForce(); //停止
		m_pOwner->GetMoveData().SetFaceTo(vDesPos - m_pOwner->GetCurPos()); //面向目标
		//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// 打断使用技能打断的buff

		m_dwSkillID			=	dwSkillID;
		m_dwTargetUnitID	=	dwTargetUnitID;
		m_dwSkillSerial		=	dwSerial;
		

		// 如果该技能需要起手，则设置起手倒计时，否则进入技能作用阶段
		m_nPersistSkillTimeCD = (INT)(pSkill->GetPilotTime());

		if( m_nPersistSkillTimeCD > 0 ) 
		{
			m_eUseSkillState = EUSS_Piloting;
			//m_bSkillPiloting	=	TRUE;
			//m_bSkillPreparing	=	FALSE;
			//m_bSkillOperating	=	FALSE;
			m_pOwner->StartSkillCoolDown(pSkill);
		}
		else
		{
			return E_UseSkill_SkillNotExist;
		}
		//else
		//{
		//	// 可以发动，设置技能冷却
		//	m_pOwner->StartSkillCoolDown(pSkill);

		//	m_bSkillPreparing	=	FALSE;
		//	m_bSkillOperating	=	TRUE;
		//	m_nSkillOperateTime	=	0;
		//	m_nSkillCurDmgIndex	=	0;

		//	// 计算目标
		//	CalSkillTargetList(vDesPos);
		//}
	}
	else
	{
		if( pSkill->IsMoveCancel() )
		{
			m_pOwner->GetMoveData().StopMoveForce();
		}

		// 如果目标存在且不是自己，则改变面向
		if( VALID_POINT(pTargetUnit) && m_pOwner->GetID() != pTargetUnit->GetID() )
		{
			m_pOwner->GetMoveData().SetFaceTo(pTargetUnit->GetCurPos() - m_pOwner->GetCurPos());
		}


		// 打断使用技能打断的buff
		//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);

		// 设置参数，准备发动
		m_dwSkillID			=	dwSkillID;
		m_dwTargetUnitID	=	dwTargetUnitID;
		m_dwSkillSerial		=	dwSerial;

		// 如果该技能需要起手，则设置起手倒计时，否则进入技能作用阶段
		m_nSkillPrepareCountDown = (INT)(pSkill->GetPrepareTime() * m_fSkillPrepareModPct);

		if(m_pOwner->IsRole())
		{
			if(pSkill->GetProto()->nType2 != ESSTE_Default)
			{
				m_dwPublicCoolDown = 0;
				if(pSkill->GetProto()->bPublicCD)
				{
					EClassType e = ((Role*) m_pOwner)->GetClass();
					m_dwPublicCoolDown = AttRes::GetInstance()->GetVariableLen().n_public_cd_time[e];
					NET_SIS_start_public_cd send;
					send.dwSkillID = m_dwSkillID;
					send.dwCoolTime = m_dwPublicCoolDown;
					((Role*)m_pOwner)->SendMessage(&send, send.dw_size);
				}
			}
		}
		
		if( m_nSkillPrepareCountDown > 0 )
		{
			m_eUseSkillState	=	EUSS_Preparing;
			const SkillScript* pScript = pSkill->GetSkillScript();
			if (VALID_POINT(pScript))
			{
				pScript->PreparingSkill(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), dwTargetUnitID);
			}

			//m_bSkillPreparing	=	TRUE;
			//m_bSkillOperating	=	FALSE;
		}
		else
		{
			// 可以发动，设置技能冷却 这里移到效果时cd
			//m_pOwner->StartSkillCoolDown(pSkill);

			m_eUseSkillState	=	EUSS_Operating;
			m_bCD				=	TRUE;
			//m_bSkillPreparing	=	FALSE;
			//m_bSkillOperating	=	TRUE;
			m_nSkillOperateTime	=	0;
			m_nSkillCurDmgIndex	=	0;
			m_bTrigger			=	FALSE;
			// 技能消耗
			CalculateCost(pSkill);
			// 计算目标	

			// 非群攻在这里计算
			if( pSkill->GetOPRadius() <= 0.0f || pSkill->GetDmgTimes() <= 0)
			{
				CalSkillTargetList(vDesPos, pSkill->GetHitNumber());
			}
			
		}
	}

	return nRet;
}

//------------------------------------------------------------------------------------------
// 结束使用技能
//------------------------------------------------------------------------------------------
VOID CombatHandler::EndUseSkill()
{
	m_dwSkillID					=	INVALID_VALUE;
	m_dwTargetUnitID			=	INVALID_VALUE;
	m_dwTargetEffectFlag		=	0;
	m_dwSkillSerial				=	0;
	
	m_eUseSkillState			=	EUSS_NULL;
	//m_bSkillPreparing			=	false;
	//m_bSkillOperating			=	false;

	m_nSkillPrepareCountDown	=	0;
	m_nSkillOperateTime			=	0;
	m_nSkillCurDmgIndex			=	0;
	m_bTrigger					=	FALSE;

	m_nPersistSkillTimeCD		=	0;
	m_nPersistSkillTime			=   0;
	m_nPersistSkillCnt			=	0;
	m_bCD						=	FALSE;
	//m_bSkillPiloting			=	false;
	//m_vPersistSkillPos			=	Vector3(0,0,0);
	memset(&m_vPersistSkillPos,0,sizeof(Vector3));

}
//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
VOID CombatHandler::Update()
{

	m_dwPublicCoolDown -= TICK_TIME;
	if(m_dwPublicCoolDown <= 0)
	{
		m_dwPublicCoolDown = 0;
	}

	m_dwCombatStateCoolDown -= TICK_TIME;
	if(m_dwCombatStateCoolDown <= 0)
	{
		m_dwCombatStateCoolDown = 0;
		if(m_pOwner->IsRole())
		{
			if(((Role*)m_pOwner)->GetEnmityCreatureSize() <= 0)
			{
				if(((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
				{
					((Role*)m_pOwner)->UnsetRoleState(ERS_Combat);
					((Role*)m_pOwner)->OnInterruptBuffEvent(EBIF_LeaveCombat);
					((Role*)m_pOwner)->SetLeaveCombatTime( );
				}
			}
		}
	}
		
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	tstring szDesc = _T("");


	DWORD dwcurrSkill = m_dwSkillID;
	// 如果正在使用技能
	if( IsUseSkill() )
	{
		switch (m_eUseSkillState)
		{
		case EUSS_Preparing:
			UpdateSkillPrepare();
			szDesc = _T("CombatHandler::UpdateSkillPrepare() time %d skill %d\r\n");
			break;
		case EUSS_Operating:
			UpdateSkillOperate();
			szDesc = _T("CombatHandler::UpdateSkillOperate() time %d skill %d\r\n");
			break;
		case EUSS_Piloting:
			UpdateSkillPiloting();
			szDesc = _T("CombatHandler::UpdateSkillPiloting() time %d skill %d\r\n");
			break;
		default:
			EndUseSkill();
			break;

		}
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(szDesc.c_str(), dw_new_time - dw_time, dwcurrSkill);
		}
		//if( IsSkillPreparing() )		UpdateSkillPrepare();
		//else if( IsSkillOperating() )	UpdateSkillOperate();
		//else if( IsSkillPiloting())		UpdateSkillPiloting(); //add by guohui
		//else							EndUseSkill();
	}
	
	//更新持续的地物技能
	m_dwChixuCoolDown -= 1;
	if (m_dwChixuCoolDown <= 0)
	{
		UpdatePrepareUnit();
		m_dwChixuCoolDown = 3 * TICK_PER_SECOND;
	}
	

	// 如果正在使用物品
	if( IsUseItem() )
	{
		if( IsItemPreparing() )			UpdateItemPrepare();
		else if( IsItemOperating() )	UpdateItemOperate();
		else							EndUseItem();
	}

	if( IsRide())
	{
		if(IsRidePreparing()) UpdateRidePrepare();
		else if(IsRideOperating())UpdateRideOperate();
		else EndRide();
	}
}

//-------------------------------------------------------------------------------
// 使用物品
//-------------------------------------------------------------------------------
INT CombatHandler::UseItem(INT64 n64ItemID, DWORD dwTargetUnitID, DWORD dwSerial, DWORD &dw_data_id, bool& bImmediate)
{
	if( INVALID_VALUE == dwTargetUnitID )
		dwTargetUnitID = m_pOwner->GetID();

	// 检查是不是玩家
	if( !m_pOwner->IsRole() ) return E_UseItem_TargetInvalid;
	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// 检查物品是否在背包里
	tagItem* pItem = pOwnerRole->GetItemMgr().GetBagItem(n64ItemID); 
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// 检查能否使用物品
	INT nRet = E_Success;
	BOOL bIgnore = FALSE;		// 是否忽略能否使用物品的通用判断

	if(VALID_POINT(pItem->pScript) && VALID_POINT(pOwnerRole->get_map()))
	{
		// 检查脚本的物品使用限制
		nRet = pItem->pScript->can_use_item(pOwnerRole->get_map(), pItem->dw_data_id, m_pOwner->GetID(), dwTargetUnitID, bIgnore, pItem->n64_serial);

		// 重新获取物品指针
		pItem = pOwnerRole->GetItemMgr().GetBagItem(n64ItemID); 
		if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;
	}

	// 检查使用物品的通用判断
	if(!bIgnore && E_Success == nRet)
		nRet = can_use_item(pItem);

	if( E_Success != nRet ) return nRet;

	// 如果该物品不能移动使用，则停下
	if( !pItem->pProtoType->bMoveable )
	{
		m_pOwner->GetMoveData().StopMoveForce();
	}

	// 打断使用物品打断的buff
	//m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);

	// 如果检查通过，则设置上相应的参数，准备发动
	m_n64ItemID			=	n64ItemID;
	m_dwItemSerial		=	dwSerial;
	dw_data_id			=	pItem->dw_data_id;
	m_dwTargetUnitIDItem = dwTargetUnitID;

	// 如果使用物品需要起手，则设置起手倒计时，否则进入作用阶段
	m_nItemPrepareCountDown = pItem->pProtoType->nPrepareTime;
	if( m_nItemPrepareCountDown > 0 )
	{
		m_bItemPreparing	=	TRUE;
		m_bItemOperating	=	FALSE;
		bImmediate			=	false;
	}
	else
	{
		m_bItemPreparing	=	FALSE;
		m_bItemOperating	=	TRUE;
		bImmediate			=	true;
	}

	return nRet;
}

//-------------------------------------------------------------------------------
// 使用坐骑
//-------------------------------------------------------------------------------
DWORD CombatHandler::UseRide()
{
	if( !m_pOwner->IsRole() ) return E_Ride_BeginRide_Failed_Not_Exist;

	Role* pOwnerRole = static_cast<Role*>(m_pOwner);
	tagEquip* pRide = pOwnerRole->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pRide)) return E_Ride_BeginRide_Failed_Not_Exist;

	DWORD tRet = CheckUseRide(pRide); 
	if(tRet != E_Success) return tRet;

	m_pOwner->GetMoveData().StopMoveForce();

	m_nRidePrepareCountDown = pOwnerRole->GetMountSpellTime( );
	m_bRide = TRUE;
	m_bRidePreparing = TRUE;
	m_bRideOperating = FALSE;


	return E_Success;
}
DWORD CombatHandler::CheckUseRide(tagEquip* pRide)
{
	DWORD dwRet = E_Success;

	if(IsUseItem() || IsUseSkill())
		return E_Ride_BeginRide_Failed_StateLimit;

	dwRet = CheckRoleStateLimit(pRide);
	if(dwRet != E_Success) 
		return E_Ride_BeginRide_Failed_StateLimit;

	dwRet = m_pOwner->IsInStateCantMove();
	if(dwRet != E_Success || 
		static_cast<Role*>(m_pOwner)->IsInRoleStateAny(ERS_Fishing | ERS_Carry))
	{
		return E_Ride_BeginRide_Failed_StateLimit;
	}

	BOOL bLimited = ((Role*)m_pOwner)->ride_limit(pRide);
	if(bLimited) return E_Ride_BeginRide_Failed_StateLimit;

	return E_Success;
}
//-----------------------------------------------------------------------------
// 更新技能起手，如果起手结束了，则切换到进行状态
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateSkillPrepare()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillPreparing() ) return;
	
	// 首先找到这个技能
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	// 减去Tick时间
	m_nSkillPrepareCountDown -= TICK_TIME;

	// 起手时间结束，切换到更新状态
	if( m_nSkillPrepareCountDown <= 0 )
	{
		// 这里移到计算机能效果后冷却
		//m_pOwner->StartSkillCoolDown(pSkill);
		m_eUseSkillState	= EUSS_Operating;
		m_bCD				= TRUE;
		//m_bSkillPreparing = FALSE;
		//m_bSkillOperating = TRUE;
		m_nSkillOperateTime = 0;
		m_nSkillCurDmgIndex = 0;
		m_bTrigger			= FALSE;
		// 技能消耗
		CalculateCost(pSkill);
		// 计算目标
		CalSkillTargetList(Vector3(0,0,0), pSkill->GetHitNumber());
	}
}

//-----------------------------------------------------------------------------
// 更新起手，如果起手结束了，则切换到进行状态
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateItemPrepare()
{
	if( !IsUseItem() ) return;
	if( !IsItemPreparing() ) return;

	// 减去Tick时间
	m_nItemPrepareCountDown -= TICK_TIME;

	// 起手时间结束，切换到更新状态
	if( m_nItemPrepareCountDown <= 0 )
	{
		m_bItemPreparing = FALSE;
		m_bItemOperating = TRUE;
	}
}
//-----------------------------------------------------------------------------
// 更新起手，如果起手结束了，则切换到进行状态
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateRidePrepare()
{
	if( !IsRide() ) return;
	if( !IsRidePreparing() ) return;
	
	m_nRidePrepareCountDown -= TICK_TIME;
	if( m_nRidePrepareCountDown <= 0 )
	{
		m_bRidePreparing = FALSE;
		m_bRideOperating = TRUE;
	}
}

//add by guohui 
//更新引导型技能的操作，新的计算伤害流程
VOID CombatHandler::UpdateSkillPiloting()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillPiloting()) return;

	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	Map* pMap = m_pOwner->get_map();
	if( !VALID_POINT(pMap) ) return;

	m_nPersistSkillTimeCD -= TICK_TIME;

	INT nDmgTimes = pSkill->GetPilotNum();

	// 如果伤害次数为0，说明该技能无伤害，则直接进入到计算buff阶段
	if( nDmgTimes <= 0 )
	{
		// 计算buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listTargetID, ETEE_Use);
		// 计算消耗
		//CalculateCost(pSkill);

		// 结束
		EndUseSkill();
		return;
	}

	// 计算消耗
	//CalculateCost(pSkill);
	//m_nPersistSkillTime += TICK_TIME;  //持续技能时间加一个tick
	//static bool flag = true;

	for(; m_nPersistSkillCnt < nDmgTimes; m_nPersistSkillCnt++)  //如果打击次数不够
	{
		// 本tick完成不了如此多的伤害计算，等到下个tick
		if(	pSkill->GetPilotTime() -(m_nPersistSkillCnt+1)* (pSkill->GetPilotTime()/nDmgTimes) < m_nPersistSkillTimeCD )
			break;

		CalSkillTargetList(m_vPersistSkillPos, pSkill->GetHitNumber()); //先算伤害列表
		// 时间到了，则开始计算伤害
		package_list<DWORD>::list_iter it = m_listTargetID.begin();
		DWORD dwTargetID = INVALID_VALUE;

		while( m_listTargetID.find_next(it, dwTargetID) )
		{
			// 找到这个目标
			Unit* pTarget = pMap->find_unit(dwTargetID);

			if( !VALID_POINT(pTarget) || pTarget == m_pOwner) continue;

			// 计算伤害
			CalculateDmg(pSkill, pTarget);
		}

		// 计算主动技能Buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Hit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listDodgedTarget, ETEE_Dodged);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listBlockedTarget, ETEE_Blocked);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listCritedTarget, ETEE_Crit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Use);

		//if(m_bDropMP)
		//{
			//CalculateCost(pSkill);  //只扣掉一次蓝
		//	m_bDropMP = false;
		//}
	}

	if(m_nPersistSkillCnt >= nDmgTimes )
	{
		EndUseSkill();
		//m_bDropMP = true;
	}
}
//end
//-------------------------------------------------------------------------------
// 更新技能操作，如果到了计算伤害的时候，则计算伤害，如果伤害计算完了，则计算buff
//-------------------------------------------------------------------------------
VOID CombatHandler::UpdateSkillOperate()
{
	if( !IsUseSkill() ) return;
	if( !IsSkillOperating() ) return;

	// 首先找到这个技能
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) )
	{
		EndUseSkill();
		return;
	}

	Map* pMap = m_pOwner->get_map();
	if( !VALID_POINT(pMap) ) return;

	// 得到技能总伤害次数
	INT nDmgTimes = pSkill->GetDmgTimes();

	if (m_pOwner->IsRole())
	{
		((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(ete_use_skill, pSkill->GetTypeID()/100, 1);
	}
	

	// 防止多次调用
	if (m_bCD)
	{
		// 技能cd
		m_pOwner->StartSkillCoolDown(pSkill);
		m_bCD = FALSE;
	}


	// 如果伤害次数为0，说明该技能无伤害，则直接进入到计算buff阶段
	if( nDmgTimes <= 0 )
	{
		// 计算buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listTargetID, ETEE_Use);
		// 计算消耗
		//CalculateCost(pSkill);
		// 结束
		EndUseSkill();
		return;
	}
	
	bool bCaldmg = FALSE;
	// 伤害次数不为0，则检测当前时间到了哪次伤害
	m_nSkillOperateTime += TICK_TIME;

	DWORD dwStartTime = timeGetTime();
	for(; m_nSkillCurDmgIndex < nDmgTimes; m_nSkillCurDmgIndex++)
	{
		// 本tick完成不了如此多的伤害计算，等到下个tick
		if( pSkill->GetDmgTime(m_nSkillCurDmgIndex) > m_nSkillOperateTime )
			break;

		/** Ares 群攻在计算伤害是重新计算目标，需要测试效率 ===========> **/
		if( pSkill->GetOPRadius() > 0.0f )
		{
			//if( pSkill->IsMoveable( ) && !m_pOwner->IsInStateCantMove( ) )
			CalSkillTargetList( m_pOwner->GetCurPos( ), pSkill->GetHitNumber() );
		}
		/** <=============== */
		
		bCaldmg = TRUE;
		// 时间到了，则开始计算伤害
		package_list<DWORD>::list_iter it = m_listTargetID.begin();
		DWORD dwTargetID = INVALID_VALUE;

		while( m_listTargetID.find_next(it, dwTargetID) )
		{
			// 找到这个目标
			Unit* pTarget = pMap->find_unit(dwTargetID);

			if( !VALID_POINT(pTarget) || pTarget == m_pOwner) continue;

			// 计算伤害
			CalculateDmg(pSkill, pTarget);
		}
	}
	DWORD dwEndTime = timeGetTime();

	DWORD dwMaxPcallTime = dwEndTime - dwStartTime;
	if(dwMaxPcallTime > 0)
	{
		g_world.get_log()->write_log(_T("damage time useing %d millisecond.\r\n"), dwMaxPcallTime);
	}

	if (!m_bTrigger && bCaldmg)
	{

		m_bTrigger = TRUE;
		// 计算主动技能Buff
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Hit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listDodgedTarget, ETEE_Dodged);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listBlockedTarget, ETEE_Blocked);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listCritedTarget, ETEE_Crit);
		m_pOwner->OnActiveSkillBuffTrigger(pSkill, m_listHitedTarget, ETEE_Use);

		// 找到目标
		Unit* pTarget = pMap->find_unit(m_dwTargetUnitID);
		BOOL bhit = m_dwTargetEffectFlag & ETEF_Hited;
		BOOL bCrit = m_dwTargetEffectFlag & ETEF_Crited;
		if (!VALID_POINT(pTarget))
		{
			pTarget = pMap->find_unit(m_listHitedTarget.front());
			bhit = TRUE;
		}

		if( VALID_POINT(pTarget) )
		{	
			Creature* pPetTarget = pTarget->get_map()->find_creature(pTarget->GetFlowUnit());
			if (VALID_POINT(pPetTarget))
			{
				pPetTarget->GetAI()->AddEnmity(m_pOwner, 1);
				pPetTarget->GetAI()->SetTargetUnitID(m_pOwner->GetID());
			}

			// 让战斗宠物攻击目标
			Creature* pPet = m_pOwner->get_map()->find_creature(m_pOwner->GetFlowUnit());
			if (VALID_POINT(pPet))
			{
				pPet->GetAI()->AddEnmity(pTarget, 1);
				pPet->GetAI()->SetTargetUnitID(pTarget->GetID());
			}

			// Buff触发
			if( m_dwTargetEffectFlag & ETEF_Hited )
			{
				// 命中
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Hit);
			}
			else
			{
				// 被闪避
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Dodged);
			}

			if( m_dwTargetEffectFlag & ETEF_Block )
			{
				// 被格挡
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Blocked);
			}

			if( m_dwTargetEffectFlag & ETEF_Crited )
			{
				// 暴击
				m_pOwner->OnBuffTrigger(pTarget, ETEE_Crit);
			}
		
			// 计算能量相关逻辑
			//if (bhit)
			//	OnHit(pSkill, m_pOwner->get_level()-pTarget->get_level(), bCrit);

			// 计算被动技能和装备Buff
			if( m_pOwner->IsRole() )
			{
				// 针对第一目标进行计算
				Role* pOwnerRole = static_cast<Role*>(m_pOwner);

				if( m_dwTargetEffectFlag & ETEF_Hited )
				{
					// 命中
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Hit);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Hit);
				}
				else
				{
					// 被闪避
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Dodged);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Dodged);
				}

				if( m_dwTargetEffectFlag & ETEF_Block )
				{
					// 被格挡
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Blocked);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Blocked);
				}

				if( m_dwTargetEffectFlag & ETEF_Crited )
				{
					// 暴击
					pOwnerRole->OnPassiveSkillBuffTrigger(pTarget, ETEE_Crit);
					pOwnerRole->OnEquipmentBuffTrigger(pTarget, ETEE_Crit);
				}
			}
		}

	}

	// 检测所有伤害是否已经计算完毕
	if( m_nSkillCurDmgIndex >= nDmgTimes )
	{
		m_pOwner->OnInterruptBuffEvent(EBIF_InterCombat);					// 打断使用技能打断的buff
		// 计算消耗
		//CalculateCost(pSkill);
		// 技能结束
		EndUseSkill();
	}

}

//-----------------------------------------------------------------------------
// 更新使用物品效果
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateItemOperate()
{
	if( !IsUseItem() ) return;
	if( !IsItemOperating() ) return;
	if( !m_pOwner->IsRole() ) return;

	Role* pOwnerRole = static_cast<Role*>(m_pOwner);

	// 首先找到这个物品
	tagItem* pItem = pOwnerRole->GetItemMgr().GetBagItem(m_n64ItemID); 
	if( !VALID_POINT(pItem) )
	{
		EndUseItem();
		return;
	}

	DWORD	dw_data_id = pItem->dw_data_id;
	Map* pMap = pOwnerRole->get_map();
	if( !VALID_POINT(pMap) ) return;

	/* js zhaopeng 2010.03.28 任务系统要求能对NPC使用物品
	* 之前的使用物品写死只能对自己使用
	* 现在取当前目标使用物品 可能会有新的BUG  
	*/
	Unit* pTargetUnit = pOwnerRole->get_map()->find_unit( m_dwTargetUnitIDItem );

	// 发送命中目标给客户端
	NET_SIS_hit_target send;
	send.dw_role_id = m_dwTargetUnitIDItem;
	send.dwSrcRoleID = m_pOwner->GetID();
	send.eCause = EHTC_Item;
	send.dwMisc = pItem->dw_data_id;
	send.dwSerial = m_dwItemSerial;
	pMap->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);

	// 计算buff
	pOwnerRole->OnActiveItemBuffTrigger(pTargetUnit, pItem, ETEE_Use);
	
	// 是否还要删除
	BOOL bDelete = TRUE;
	// 计算物品的脚本使用效果
	if(VALID_POINT(pItem->pScript) && VALID_POINT(pOwnerRole->get_map()))
	{
		bDelete = pItem->pScript->UseItem(pOwnerRole->get_map(), pItem->dw_data_id, m_pOwner->GetID(), m_dwTargetUnitIDItem, pItem->n64_serial);
	}

	// 称号消息
	pOwnerRole->GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, dw_data_id, 1);

	// 加入物品公共冷却时间
	pOwnerRole->GetItemMgr().Add2CDTimeMap(dw_data_id);
	
	if (bDelete)
	{
		// 处理物品消失
		pOwnerRole->GetItemMgr().ItemUsedFromBag(m_n64ItemID, 1, (DWORD)elcid_item_use);
	}
	
	// 处理任务事件
	pOwnerRole->on_quest_event( EQE_UseItem, dw_data_id , m_dwTargetUnitIDItem );

	EndUseItem();
}
//-----------------------------------------------------------------------------
// 开始骑乘
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdateRideOperate()
{
	if(!IsRide())return;
	if(!IsRideOperating())return;

	tagEquip* pRide = ((Role*)m_pOwner)->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(VALID_POINT(pRide))
	{
		Role* pOwner = static_cast<Role*>(m_pOwner);
		DWORD dwRet = pOwner->begin_ride_op();
		if(VALID_POINT(pOwner->get_map()) && dwRet != E_Success)
		{
			NET_SIS_begin_ride send;
			send.dwError = dwRet;
			send.dwRoleID = pOwner->GetID();
			//send.nSpellTime = 0;
			send.dwTypeID = 0;
			pOwner->get_map()->send_big_visible_tile_message(pOwner,&send, send.dw_size);
		}
	}

	EndRide();
}
//-----------------------------------------------------------------------------------
// 取消技能使用
//-----------------------------------------------------------------------------------
VOID CombatHandler::CancelSkillUse(DWORD dwSkillTypeID)
{
	if( !IsValid() || !IsUseSkill() ) return;

	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) || pSkill->GetTypeID() != dwSkillTypeID )
		return;

	BOOL bCanCancel = FALSE;

	// 如果技能正在起手\引导，一定可以
	if( IsSkillPreparing() || IsSkillPiloting() )
	{
		bCanCancel = TRUE;
	}
	// 如果正在释放，则只有普通攻击才可以
	else
	{
		if( ESSTE_Default == pSkill->GetTypeEx() )
		{
			bCanCancel = TRUE;
		}
	}

	// 如果可以取消
	if( bCanCancel )
	{
		NET_SIS_skill_interrupt send;
		send.dw_role_id = m_pOwner->GetID();
		send.dwSkillID = dwSkillTypeID;

		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndUseSkill();
	}
}

//-----------------------------------------------------------------------------------
// 取消物品释放
//-----------------------------------------------------------------------------------
VOID CombatHandler::CancelItemUse(INT64 n64ItemSerial)
{
	if( !IsValid() || !IsUseItem() ) return;

	if( m_n64ItemID != n64ItemSerial ) return;

	BOOL bCanCancel = FALSE;

	// 物品只有在起手时才能取消
	if( IsItemPreparing() )
	{
		bCanCancel = TRUE;
	}

	if( bCanCancel )
	{
		NET_SIS_use_item_interrupt send;
		send.dw_role_id = m_pOwner->GetID();
		send.n64ItemID = m_n64ItemID;
		send.dw_data_id = INVALID_VALUE;

		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndUseItem();
	}
}

//----------------------------------------------------------------------------
// 取消骑乘
//----------------------------------------------------------------------------
VOID CombatHandler::InterruptRide()
{
	if( !IsValid() || !IsRide()) return;
	if(IsRidePreparing())
	{
		NET_SIS_interrupt_begin_ride send;
		send.dwRoleID = m_pOwner->GetID();
		if( VALID_POINT(m_pOwner->get_map()) )
		{
			m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
		}
		EndRide();
	}
}
//-----------------------------------------------------------------------------------
// 打断起手
//-----------------------------------------------------------------------------------
BOOL CombatHandler::InterruptPrepare(EInterruptType eType, BOOL bOrdinary, BOOL bForce)
{
	if( FALSE == IsValid() || FALSE == IsPreparing() )
		return TRUE;

	BOOL bSkill = FALSE;		// 是技能在起手还是物品在起手
	DWORD dwSkillTypeID = INVALID_VALUE;
	if( IsSkillPreparing() )	bSkill = TRUE;
	else if(IsSkillPiloting())	bSkill = TRUE;
	else						bSkill = FALSE;

	// 通过是使用物品还是使用技能来判断打断值
	BOOL bMoveInterrupt = FALSE;
	INT nInterruptSkillRate = 0;

	if( bSkill )
	{
		Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
		if( VALID_POINT(pSkill) )
		{
			const tagSkillProto* pProto = pSkill->GetProto();
			if( VALID_POINT(pProto) )
			{
				bMoveInterrupt = pProto->bInterruptMove;
				nInterruptSkillRate = (bOrdinary ? pProto->nInterruptSkillOrdRate : pProto->nInterruptSkillSpecRate);
			}
			dwSkillTypeID = pSkill->GetTypeID();
		}
	}
	else
	{
		Role* pRole = static_cast<Role*>(m_pOwner);
		tagItem* pItem = pRole->GetItemMgr().GetBagItem(m_n64ItemID);
		if( VALID_POINT(pItem) )
		{
			bMoveInterrupt = pItem->pProtoType->bInterruptMove;
			nInterruptSkillRate = pItem->pProtoType->nInterruptSkillOrdRate;
		}

		if(IsRide() || IsRidePreparing())
		{
			bMoveInterrupt = TRUE;
			nInterruptSkillRate = 10000;
		}
	}

	BOOL bCanInterrupt = FALSE;	// 是否能够打断

	if( bForce )
	{
		bCanInterrupt = TRUE;
	}
	else
	{
		// 尝试打断
		switch(eType)
		{
		case EIT_Move:
			{
				if( bMoveInterrupt )
				{
					bCanInterrupt = TRUE;
				}
			}
			break;

		case EIT_Skill:
			{
				// 普通攻击打断几率
				if( get_tool()->tool_rand() % 10000 < nInterruptSkillRate )
				{
					bCanInterrupt = TRUE;
				}
			}
			break;

		default:
			break;
		}
	}

	if( bCanInterrupt )
	{
		// 发送打断给周围玩家
		if( bSkill )
		{
			NET_SIS_skill_interrupt send;
			send.dw_role_id = m_pOwner->GetID();
			send.dwSkillID = dwSkillTypeID;

			if( VALID_POINT(m_pOwner->get_map()) )
			{
				m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
			}
			EndUseSkill();
		}
		else
		{
			if(IsRide() || IsRidePreparing())
			{
				NET_SIS_interrupt_begin_ride send;
				send.dwRoleID = m_pOwner->GetID();
				if( VALID_POINT(m_pOwner->get_map()) )
				{
					m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
				}
				EndRide();
			}
			else
			{
				NET_SIS_use_item_interrupt send;
				send.dw_role_id = m_pOwner->GetID();
				send.n64ItemID = m_n64ItemID;
				send.dw_data_id = INVALID_VALUE;

				if( VALID_POINT(m_pOwner->get_map()) )
				{
					m_pOwner->get_map()->send_big_visible_tile_message(m_pOwner, &send, send.dw_size);
				}
				EndUseItem();
			}
		}

		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// 打断释放
//-------------------------------------------------------------------------------
BOOL CombatHandler::InterruptOperate(EInterruptType eType, DWORD dwMisc, BOOL bForce/* =FALSE */)
{
	if( FALSE == IsValid() || FALSE == IsSkillOperating() )
		return TRUE;

	if( EIT_Move == eType )
	{
		EMoveState eState = (EMoveState)dwMisc;

		// 走和游泳相关的移动，则只有移动打断的普通攻击才打断
		if( EMS_Walk			== eState ||
			EMS_CreaturePatrol	== eState ||
			EMS_CreatureWalk	== eState ||
			EMS_CreatureFlee	== eState ||
			EMS_CreatureRun		== eState)
		{
			Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
			if( VALID_POINT(pSkill) && pSkill->IsMoveCancel() )
			{
				EndUseSkill();
				return TRUE;
			}
		}
		// 其它移动方式，则只要是普通攻击就打断
		else
		{
			Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
			if( VALID_POINT(pSkill) && ESSTE_Default == pSkill->GetTypeEx() )
			{
				EndUseSkill();
				return TRUE;
			}
		}
	}

	return FALSE;
}


//-------------------------------------------------------------------------------
// 是否可以使用技能
//-------------------------------------------------------------------------------
INT CombatHandler::CanCastSkill(Skill* pSkill, DWORD dwTargetUnitID ,const Vector3& vDesPos)
{
	if( !VALID_POINT(pSkill) )
		return E_SystemError;

	// 不能移动释放
	//if( !pSkill->IsMoveable() && m_pOwner->GetMoveData().GetCurMoveState() != EMS_Stand)
	//{
	//	return E_UseSkill_SelfStateLimit;
	//}
	
	//if( IsPreparing() ) return E_UseSkill_Operating;	// 当前正在起手，则不能使用任何技能

	//if(IsPiloting()) return E_UseSkill_Operating;  //add by guohui 检测是否在引导状态下，如果是则不能使用技能

	INT nRet = E_Success;

	nRet = CheckSkillAbility(pSkill);
	if( E_Success != nRet ) return nRet;

	//nRet = CheckOwnerLimitSkill();
	//if( E_Success != nRet ) return nRet;

	nRet = CheckSkillLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	nRet = CheckTargetLimit(pSkill, dwTargetUnitID ,vDesPos);
	if( E_Success != nRet ) return nRet;

	nRet = CheckCostLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	//nRet = CheckVocationLimit(pSkill);
	//if( E_Success != nRet ) return nRet;

	nRet = CheckMapLimit(pSkill);
	if( E_Success != nRet ) return nRet;

	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		nRet = pScript->CanCastSkill(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), dwTargetUnitID);
		if( E_Success != nRet ) return nRet;
	}
	
	if (m_pOwner->IsRole())
	{
		DWORD dwItemID;
		INT nItemNum;
		pSkill->GetCostItem(dwItemID, nItemNum);
		

		if (dwItemID != INVALID_VALUE && dwItemID != 0 && nItemNum > 0)
		{
			if (((Role*)m_pOwner)->GetItemMgr().GetBag().GetSameItemCount(dwItemID) >= nItemNum)
			{
				package_list<tagItem*> itemList;

				((Role*)m_pOwner)->GetItemMgr().GetBag().GetSameItemList(itemList, dwItemID, nItemNum);
				((Role*)m_pOwner)->GetItemMgr().DelBagSameItem(itemList, nItemNum, elcid_skill);
			}
			else
			{
				nRet = E_UseSKill_Not_Item;
			}
		}
		
	}
	
	if( CheckSkillConflict(pSkill) ) 
		return E_UseSkill_Operating;

	return nRet;
}

//-------------------------------------------------------------------------------
// 测试技能本身是否能够使用
//-------------------------------------------------------------------------------
INT CombatHandler::CheckSkillAbility(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	// 被动技能不可以使用
	if( pSkill->IsPassive() )
		return E_UseSkill_PassiveSkill;

	// 如果技能的目标类型不是战斗目标或非战斗目标，则不可以使用
	ESkillTargetType eTargetType = pSkill->GetTargetType();
	if( ESTT_Combat != eTargetType && ESTT_NoCombat != eTargetType )
		return E_UseSkill_SkillTargetInvalid;

	// 技能的冷却时间还没到，则不可以使用
	if( pSkill->GetCoolDownCountDown() > pSkill->GetProto()->nMaxCoolDown )
		return E_UseSkill_CoolDowning;

	if(pSkill->GetProto()->bPublicCDLimit)
	{
		if(m_dwPublicCoolDown > 0)
			return E_UseSkill_CoolDowning;
	}
	
	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试技能使用者是否能够使用技能
//-------------------------------------------------------------------------------
INT CombatHandler::CheckOwnerLimitSkill()
{
	// 是否处在不能使用技能的状态
	if( m_pOwner->IsInStateCantCastSkill() )
		return E_UseSkill_UseLimit;

	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试技能本身使用限制
//-------------------------------------------------------------------------------
INT CombatHandler::CheckSkillLimit(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;
	
	// 体力低于或者真气低于
	if( pProto->nUseHPPctLimit > 0 )
	{
		if( m_pOwner->GetAttValue(ERA_MaxHP) <= 0 )
			return E_UseSkill_UseLimit;

		if( (FLOAT)m_pOwner->GetAttValue(ERA_HP) / (FLOAT)m_pOwner->GetAttValue(ERA_MaxHP) * 10000.0f < pProto->nUseHPPctLimit )
			return E_UseSkill_UseLimit;

	}
	if( pProto->nUseMPPctLimit > 0 )
	{
		if( m_pOwner->GetAttValue(ERA_MaxMP) <= 0 )
			return E_UseSkill_UseLimit;

		if( (FLOAT)m_pOwner->GetAttValue(ERA_MP) / (FLOAT)m_pOwner->GetAttValue(ERA_MaxMP) * 10000.0f < pProto->nUseHPPctLimit )
			return E_UseSkill_UseLimit;

	}

	// 性别限制
	//if( pProto->eSexLimit != ESSL_Null )
	//{
	//	if( ESSL_Man == pProto->eSexLimit )
	//	{
	//		if( 1 != m_pOwner->GetSex() )
	//			return E_UseSkill_SexLimit;
	//	}
	//	else if( ESSL_Woman == pProto->eSexLimit )
	//	{
	//		if( 0 != m_pOwner->GetSex() )
	//			return E_UseSkill_SexLimit;
	//	}
	//	else
	//	{

	//	}
	//}


	//if (m_pOwner->IsRole())
	//{
	//	// 武器限制
	//	if( EITE_Null != pProto->nWeaponLimit )
	//	{
	//		// 武器耐久度不足
	//		Role* pRole = static_cast<Role*>(m_pOwner);
	//		tagEquip* pWeapon = pRole->GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	//		if( VALID_POINT(pWeapon) && pWeapon->GetEquipNewness() <= 0)
	//		{
	//			return E_UseSkill_Not_Newness;
	//		}

	//		if( !VALID_POINT(pWeapon) )
	//		{
	//			return E_UseSkill_WeaponLimit;
	//		}

	//	}

	//	if(static_cast<Role*>(m_pOwner)->IsInRoleState(ERS_Carry))
	//		return E_UseSkill_SelfStateLimit;
	//}


	// 特殊Buff限制
	//if( VALID_POINT(pProto->dwBuffLimitID) )
	//{
	//	if( !m_pOwner->IsHaveBuff(pProto->dwBuffLimitID) )
	//	{
	//		return E_UseSkill_SelfBuffLimit;
	//	}
	//}
	
	if (m_pOwner->IsDead())
	{
		return E_UseSkill_SelfStateLimit;
	}
	// 检查自身状态限制
	if (m_pOwner->IsInState(ES_Dizzy))
	{
		return E_UseSkill_SelfStateLimit;
	}
	//DWORD dwSelfStateFlag = m_pOwner->GetStateFlag();
	//if( (dwSelfStateFlag & pProto->dwSelfStateLimit) != dwSelfStateFlag)
	//{
	//	return E_UseSkill_SelfStateLimit;
	//}

	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试目标限制
//-------------------------------------------------------------------------------
INT CombatHandler::CheckTargetLimit(Skill* pSkill, DWORD dwTargetUnitID, const Vector3& vdespos)
{
	if( !VALID_POINT(pSkill) )
		return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	// 如果TargetUnitID是INVALID_VALUE，则需要特殊判断一下
	if( INVALID_VALUE == dwTargetUnitID )
	{
		if( ESOPT_Explode == pSkill->GetOPType() && 0.0f == pSkill->GetOPDist() )
		{
			return E_Success;
		}
		else if(ESOPT_Sector == pSkill->GetOPType() )  //如果是球锥型认为可以施放 add by guohui
		{
			return E_Success;
		}
		else if (ESOPT_Persist == pSkill->GetOPType()) //如果是持续型认为可释放
		{
			const Vector3& vSrc = m_pOwner->GetCurPos();
			const Vector3& vDest = vdespos;

			FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

			// 战斗距离要加上双方各自的半径，再加上一个增量，以避免网络延迟
			FLOAT fRealDist = pSkill->GetOPDist() + (m_pOwner->GetSize().z);

			if( fRealDist * fRealDist >= fDistSq )
				return E_Success;
			else
				return E_UseSkill_DistLimit;
		}
		else
		{
			return E_UseSkill_SkillTargetInvalid;
		}
	}

	Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetUnitID);
	if( !VALID_POINT(pTarget) ) return E_UseSkill_SkillTargetInvalid;

	// 位置限制，距离限制和范围判断
	if( m_pOwner != pTarget )
	{
		// 位置限制
		//if( ESPT_NUll != pProto->ePosType )
		//{
		//	if( ESPT_Front == pProto->ePosType )
		//	{
		//		if( FALSE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		//			return E_UseSkill_PosLimitFront;
		//	}
		//	else if( ESPT_Back == pProto->ePosType )
		//	{
		//		if( TRUE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		//			return E_UseSkill_PosLimitBack;
		//	}
		//}

		// 目标距离判断
		if( (FALSE == m_pOwner->IsInCombatDistance(*pTarget, pSkill->GetOPDist(), 250)) &&
			(FALSE == m_pOwner->IsInCombatDistance(*pTarget, pSkill->GetOPRadius())))
			return E_UseSkill_DistLimit;
		
		//// 射线检测
		//if( pProto->bCollide && m_pOwner->IsRayCollide(*pTarget) )
		//	return E_UseSkill_RayLimit;
		
		// 目标对象逻辑限制
		INT nRet = CheckTargetLogicLimit(pSkill, pTarget);
		if( nRet != E_Success )	return nRet;

		CheckInCombat(pTarget);
		//针对冲撞技能加判断，使用技能的玩家等级必须大于目标玩家才可使用 gx add 2013.8.30
		if ((10107 == pProto->dwID /100) || (10207 == pProto->dwID /100) || (10307 == pProto->dwID /100))
		{
			if (m_pOwner->get_level() <= pTarget->get_level())
			{
				return E_UseSkill_Not_tou_level;
			}
		}
		//end
	}
	
	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试目标逻辑限制
//-------------------------------------------------------------------------------
INT CombatHandler::CheckTargetLogicLimit(Skill* pSkill, Unit* pTarget)
{
	if( !VALID_POINT(pSkill) || !VALID_POINT(pTarget) )
		return E_UseSkill_SkillNotExist;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	// 检测目标是否不能被使用技能
	if( pTarget->IsInStateCantBeSkill() )
	{
		return E_UseSkill_TargetLimit;
	}

	// 首先检测与目标的类型标志
	DWORD dwTargetFlag = m_pOwner->TargetTypeFlag(pTarget);
	if( !(dwTargetFlag & pProto->dwTargetLimit) )
		return E_UseSkill_TargetLimit;
	
	// 目标等级限制
	//if (pProto->nTargetLevelLimit > 0 && pProto->nTargetLevelLimit < pTarget->get_level())
	//	return E_UseSkill_TargetLimit;

	// 再检测目标的状态限制
	DWORD dwTargetStatFlag = pTarget->GetStateFlag();
	if( (dwTargetStatFlag & pProto->dwTargetStateLimit) != dwTargetStatFlag )
	{
		return E_UseSkill_TargetStateLimit;
	}

	//if(pTarget->IsRole())
	//{
	//	if(static_cast<Role*>(pTarget)->IsInRoleState(ERS_Carry))
	//		return E_UseSkill_TargetStateLimit;
	//}
	// 检测目标Buff限制
	if( VALID_POINT(pProto->dwTargetBuffLimitID) )
	{
		if( !pTarget->IsHaveBuff(pProto->dwTargetBuffLimitID) )
		{
			return E_UseSkill_TargetBuffLimit;
		}
	}	
	

	//zhjl changed 比武地图不分阵营不分敌友，不用检测敌我判断
	if (m_pOwner->get_map() && m_pOwner->get_map()->get_map_type()==EMT_PVP_BIWU)
	{
		return E_Success;
	}
	// 再检测敌我判断
	if( m_pOwner != pTarget )
	{
		DWORD dwFriendEnemyFlag = m_pOwner->FriendEnemy(pTarget);

		DWORD dwFriendEnemyLimit = 0;

		if( pProto->bFriendly )		dwFriendEnemyLimit |= ETFE_Friendly;
		if( pProto->bHostile )		dwFriendEnemyLimit |= ETFE_Hostile;
		if( pProto->bIndependent )	dwFriendEnemyLimit |= ETFE_Independent;

		if( !(dwFriendEnemyLimit & dwFriendEnemyFlag) )
		{
			return E_UseSkill_TargetLimit;
		}
		
		/* 这里移到CheckInCombat函数处理 lc

		//if((dwFriendEnemyFlag & ETFE_Hostile) || (dwFriendEnemyFlag & ETFE_Independent))
		//{
		//	if(m_pOwner->IsRole())
		//	{
		//		if(pTarget->IsRole())
		//		{
		//			m_dwCombatStateCoolDown = 6000;
		//		if(!((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
		//		{
		//			((Role*)m_pOwner)->SetRoleState(ERS_Combat);
		//		}
		//	}
		//	}

		//	if(pTarget->IsRole())
		//	{
		//		if(!((Role*)pTarget)->IsInRoleState(ERS_Combat))
		//		{
		//			((Role*)pTarget)->SetRoleState(ERS_Combat);
		//			pTarget->OnInterruptBuffEvent(EBIF_InterCombat);
		//		}
		//		if(m_pOwner->IsRole())
		//		{
		//			((Role*)pTarget)->GetCombatHandler().SetCombatStateCoolDown();
		//		}
		//	}
		}*/
	}

	// 判断成功
	return E_Success;
}
//----------------------------------------------------------------------------------------
//战斗状态判定
//----------------------------------------------------------------------------------------
VOID	CombatHandler::CheckInCombat(Unit* pTarget)
{
	if(!VALID_POINT(pTarget) )
		return;

	// 再检测敌我判断
	if( m_pOwner != pTarget )
	{
		DWORD dwFriendEnemyFlag = m_pOwner->FriendEnemy(pTarget);

		if((dwFriendEnemyFlag & ETFE_Hostile) || (dwFriendEnemyFlag & ETFE_Independent))
		{
			if(m_pOwner->IsRole())
			{
				if(pTarget->IsRole())
				{
					m_dwCombatStateCoolDown = 6000;
					if(!((Role*)m_pOwner)->IsInRoleState(ERS_Combat))
					{
						((Role*)m_pOwner)->SetRoleState(ERS_Combat);
						((Role*)m_pOwner)->StopMount();
					}
				}
			}

			if(pTarget->IsRole())
			{
				if(!((Role*)pTarget)->IsInRoleState(ERS_Combat))
				{
					((Role*)pTarget)->SetRoleState(ERS_Combat);
					((Role*)pTarget)->StopMount();
					pTarget->OnInterruptBuffEvent(EBIF_InterCombat);
				}
				if(m_pOwner->IsRole())
				{
					((Role*)pTarget)->GetCombatHandler().SetCombatStateCoolDown();
				}
			}
		}
	}

}
//----------------------------------------------------------------------------------------
// 检测地图中技能限制
//----------------------------------------------------------------------------------------
INT CombatHandler::CheckMapLimit(Skill* pSkill)
{
	// 判断地图限制
	if(VALID_POINT(m_pOwner->get_map()))
	{
		//add by zhjl 2013-12-25:添加比武副本未开启时的限制
		MapMgr* p_dest_map_manager = g_mapCreator.get_map_manager(m_pOwner->get_map()->get_map_id());
		if( VALID_POINT(p_dest_map_manager) )
		{
			if ( !p_dest_map_manager->CanUseSkill() )
				return E_UseSkill_Biwu_Not_Start;
		}

		BOOL bUesAble = m_pOwner->get_map()->can_use_skill(pSkill->GetID());
		if( !bUesAble )	return E_UseSkill_MapLimit;
	}

	return E_Success;
}

//----------------------------------------------------------------------------------------
// 测试技能使用冲突，返回TRUE为冲突，FALSE为非冲突
//----------------------------------------------------------------------------------------
BOOL CombatHandler::CheckSkillConflict(Skill* pSkill)
{
	ASSERT( VALID_POINT(pSkill) );

	if( !IsValid() ) return FALSE;		// 当前没有使用任何技能和任何物品

	//if( IsPreparing() ) return TRUE;	// 当前正在起手，则不能使用任何技能

	//if(IsPiloting()) return TRUE;  //add by guohui 检测是否在引导状态下，如果是则不能使用技能

	if( IsUseSkill() )
	{
		// 当前正在使用技能，则查看该技能是否是普通攻击
		Skill* pCurSkill = m_pOwner->GetSkill(m_dwSkillID);
		if( VALID_POINT(pSkill) && ESSTE_Default != pCurSkill->GetTypeEx() )
		{
			return TRUE;
		}
		else
		{
			EndUseSkill();
			return FALSE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// 测试技能消耗
//-------------------------------------------------------------------------------
INT CombatHandler::CheckCostLimit(Skill* pSkill)
{
	// 检测体力消耗
	INT nHPCost = pSkill->GetCost(ESCT_HP);
	if( nHPCost > 0 && m_pOwner->GetAttValue(ERA_HP) < nHPCost )
		return E_UseSkill_CostLimit;


	// 检测真气消耗
	INT nMPCost = pSkill->GetCost(ESCT_MP);
	if( nMPCost > 0 && m_pOwner->GetAttValue(ERA_MP) < nMPCost )
		return E_UseSkill_CostLimit;


	// 检测爱心消耗
	INT nRageCost = pSkill->GetCost(ESCT_Love);
	if( nRageCost > 0 && m_pOwner->GetAttValue(ERA_Love) < nRageCost )
		return E_UseSkill_CostLimit;


	// 检测持久消耗
	//INT nEnduranceCost = pSkill->GetCost(ESCT_Endurance);
	//if( nEnduranceCost > 0 && m_pOwner->GetAttValue(ERA_Endurance) < nEnduranceCost )
	//	return E_UseSkill_CostLimit;


	// 检测活力消耗
	//INT nValicityCost = pSkill->GetCost(ESCT_Valicity);
	//if( nValicityCost > 0 && m_pOwner->GetAttValue(ERA_Vitality) < nValicityCost )
	//	return E_UseSkill_CostLimit;

	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试职业限制
//-------------------------------------------------------------------------------
INT CombatHandler::CheckVocationLimit(Skill* pSkill)
{
	//ASSERT(VALID_POINT(pSkill));
	if (!VALID_POINT(pSkill)) return E_UseSkill_SkillNotExist;

	if (!m_pOwner->IsRole()) return E_Success;

	const tagSkillProto* pProto = pSkill->GetProto();
	if( !VALID_POINT(pProto) ) return E_UseSkill_SkillNotExist;

	//INT nClass = (INT)((Role*)m_pOwner)->GetClass();
	//INT nClassEx = (INT)((Role*)m_pOwner)->GetClassEx();
	INT nClass = (INT)(static_cast<Role*> (m_pOwner)->GetClass());
	INT nClassEx = (INT)(static_cast<Role*> (m_pOwner)->GetClassEx());

	if (((nClassEx << 9) + nClass) & pProto->dwVocationLimit)
		return E_Success;
	else
		return E_UseSkill_VocationLimit;
}

//-------------------------------------------------------------------------------
// 计算攻击目标，放入到list中
//-------------------------------------------------------------------------------
VOID CombatHandler::CalSkillTargetList(Vector3 vDesPos, DWORD dwMaxNumber)
{
	m_listTargetID.clear();
	m_listHitedTarget.clear();
	m_listDodgedTarget.clear();
	m_listBlockedTarget.clear();
	m_listCritedTarget.clear();
	m_dwTargetEffectFlag = 0;
	
	DWORD dwCurrNumber = 0;
	// 根据该技能的攻击距离和攻击范围来判断
	Skill* pSkill = m_pOwner->GetSkill(m_dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	// 得到目标对象
	Unit* pTarget = NULL;
	if( INVALID_VALUE == m_dwTargetUnitID)	// 如果没有选目标，则目标就是自己
	{
		pTarget = m_pOwner;
	}
	else									// 如果选了目标，则找到目标
	{
		pTarget = m_pOwner->get_map()->find_unit(m_dwTargetUnitID);
	}
	if( !VALID_POINT(pTarget) ) return;

	// 根据作用类型，作用距离和作用半径来使用技能
	ESkillOPType eOPType = pSkill->GetOPType();
	FLOAT fOPDist = pSkill->GetOPDist();
	FLOAT fOPRadius = pSkill->GetOPRadius();
	
	// 检测目标
	if( E_Success == CheckTargetLogicLimit(pSkill, pTarget ) )
	{
		m_listTargetID.push_back(pTarget->GetID());
		m_dwTargetEffectFlag = CalculateSkillEffect(pSkill, pTarget);
	}
	
	// 先将目标加进去
	//if( m_pOwner != pTarget )
	//{
	//	m_listTargetID.push_back(pTarget->GetID());
	//	m_dwTargetEffectFlag = CalculateSkillEffect(pSkill, pTarget);
	//}

	// 爆炸效果
	if( ESOPT_Explode == eOPType )
	{
		// 如果攻击范围为0，则直接返回
		if( 0.0f == fOPRadius )
			return;
		
		m_listTargetID.clear();

		// 中心点
		Unit* pCent = pTarget;
		// 攻击距离为0则以自己为中心
		if (fOPDist == 0)
		{
			pCent = m_pOwner;
		}
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		
		Vector3 vecCent;
		if (pSkill->GetProto()->bPoint)
		{
			vecCent = vDesPos;
		}
		else
		{
			vecCent = pCent->GetCurPos();
		}

		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到攻击范围内的vistile列表
		m_pOwner->get_map()->get_visible_tile(vecCent, fOPRadius, pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// 首先检测人物
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// 和目标一样，不做处理
				//if( pRole == pTarget || pRole == m_pOwner ) continue;

				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vecCent, pRole->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;


				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole ) )
					continue;
				
				CheckInCombat(pRole);
				// 射线检测

				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pRole->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// 再检测生物
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// 和目标一样，不做处理
				//if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vecCent, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;

				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);

				// 射线检测

				// 判断通过，则将生物加入到列表中
				m_listTargetID.push_back(pCreature->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}

	// 矩形
	else if( ESOPT_Rect == eOPType )
	{
		// 如果攻击范围或者攻击距离为0，则直接返回
		if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;

		// 如果攻击范围不为0，则以目标为球心检测
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		FLOAT fOPDistSQ = fOPDist * fOPDist;

		// 如果攻击范围和攻击距离均不为0，则以自身为基准检测
		FLOAT fTargetX = pTarget->GetCurPos().x;
		FLOAT fTargetZ = pTarget->GetCurPos().z;

		FLOAT fSrcX = m_pOwner->GetCurPos().x;
		FLOAT fSrcZ = m_pOwner->GetCurPos().z;

		// 自身到第一目标点的向量
		FLOAT fX2 = fTargetX - fSrcX;
		FLOAT fZ2 = fTargetZ - fSrcZ;

		// 如果目标就是自身，那么直接取自身的朝向向量
		if( m_pOwner == pTarget )
		{
			fX2 = m_pOwner->GetFaceTo().x;
			fZ2 = m_pOwner->GetFaceTo().z;
		}

		if( abs(fX2) < 0.001f && abs(fZ2) < 0.001f )
			return;

		// 自身到第一目标点的距离的平方
		FLOAT fDistSQ2 = fX2*fX2 + fZ2*fZ2;

		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到vistile列表
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// 首先检测人物
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// 和目标一样，不做处理
				if( pRole == pTarget || pRole == m_pOwner ) continue;

				// 自身到当前点的向量
				FLOAT fX1 = pRole->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pRole->GetCurPos().z - fSrcZ;

				// 先检查方位 cos(a) > 0 
				if( fX1*fX2	+ fZ1*fZ2 < 0.0f )
					continue;

				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;

				// 检查投影距离
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// 检查点到直线距离
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);
				// 射线检测

				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pRole->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// 再检测生物  这个逻辑2010.3.25被修正过by guohui
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// 和目标一样，不做处理
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// 自身到当前点的向量
				FLOAT fX1 = pCreature->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pCreature->GetCurPos().z - fSrcZ;

				// 先检查方位 cos(a) > 0 
				if( fX1*fX2	+ fZ1*fZ2 < 0.0f )
					continue;

				// 检查投影距离
				FLOAT fDist1 = fX1*fX2  + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// 检查点到直线距离
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// 射线检测

				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pCreature->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}

	else if (ESOPT_Sector == eOPType)  //球锥 add by guohui
	{
		// 如果攻击范围或者攻击距离为0，则直接返回
		if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;

		// 如果攻击范围不为0，则以目标为球心检测
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;
		FLOAT fOPDistSQ = fOPDist * fOPDist;

		// 如果攻击范围和攻击距离均不为0，则以自身为基准检测
		//FLOAT fTargetX = pTarget->GetCurPos().x;
		//FLOAT fTargetY = pTarget->GetCurPos().y;
		//FLOAT fTargetZ = pTarget->GetCurPos().z;
		FLOAT fSrcX = m_pOwner->GetCurPos().x;
		FLOAT fSrcZ = m_pOwner->GetCurPos().z;

		// 自身到第一目标点的向量
		//FLOAT fX2 = fTargetX - fSrcX;
		//FLOAT fY2 = fTargetY - fSrcY;
		//FLOAT fZ2 = fTargetZ - fSrcZ;

		FLOAT fX2 = m_pOwner->GetFaceTo().x;
		FLOAT fZ2 = m_pOwner->GetFaceTo().z;

		if( abs(fX2) < 0.001f && abs(fZ2) < 0.001f )
			return;

		// 自身到第一目标点的距离的平方
		FLOAT fDistSQ2 = fX2*fX2 + fZ2*fZ2;

		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到vistile列表
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// 首先检测人物
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// 和目标一样，不做处理
				if( pRole == pTarget || pRole == m_pOwner ) continue;

				// 自身到当前点的向量
				FLOAT fX1 = pRole->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pRole->GetCurPos().z - fSrcZ;

				// 先检查方位 cos(a) > 0 
				if(( (fX1*fX2 + fZ1*fZ2)/(sqrt(fX1*fX1 + fZ1*fZ1) * sqrt(fX2*fX2 + fZ2*fZ2))) < 0.5f )
					continue;

				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;

				// 检查投影距离
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// 检查点到直线距离
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);

				// 射线检测

				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pRole->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// 再检测生物  这个逻辑2010.3.25被修正过by guohui
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// 和目标一样，不做处理
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// 自身到当前点的向量
				FLOAT fX1 = pCreature->GetCurPos().x - fSrcX;
				FLOAT fZ1 = pCreature->GetCurPos().z - fSrcZ;

				// 先检查方位 cos(a) > 0 
				if(( (fX1*fX2 + fZ1*fZ2)/(sqrt(fX1*fX1 + fZ1*fZ1) * sqrt(fX2*fX2 + fZ2*fZ2))) < 0.5f )
					continue;


				// 检查投影距离
				FLOAT fDist1 = fX1*fX2 + fZ1*fZ2;
				FLOAT fDistSQ1 = fDist1 * fDist1;
				FLOAT fProjDistSQ = fDistSQ1 / fDistSQ2;
				if( fProjDistSQ > fOPDistSQ )
					continue;

				// 检查点到直线距离
				if( fX1*fX1 + fZ1*fZ1 - fProjDistSQ > fOPRadiusSQ)
					continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// 射线检测

				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pCreature->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}
	}
	else if (ESOPT_Persist == eOPType)
	{
		// 如果攻击范围为0，则直接返回
		if( 0.0f == fOPRadius )
			return;

		// 如果攻击范围不为0，则以目标为球心检测
		FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到攻击范围内的vistile列表
		pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// 首先检测人物
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// 和目标一样，不做处理
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;

				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vDesPos, pRole->GetCurPos()); //用传进来的点做运算
				if( fDistSQ > fOPRadiusSQ  ) continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
					continue;
				
				CheckInCombat(pRole);

				// 射线检测
			
				// 判断通过，则将玩家加入到列表中
				m_listTargetID.push_back(pRole->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pRole);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}

			// 再检测生物
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// 和目标一样，不做处理
				if( pCreature == pTarget || pCreature == m_pOwner ) continue;


				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vDesPos, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;
				
				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;
				
				CheckInCombat(pCreature);
				// 射线检测

				// 判断通过，则将生物加入到列表中
				m_listTargetID.push_back(pCreature->GetID());

				// 计算技能作用结果
				CalculateSkillEffect(pSkill, pCreature);

				dwCurrNumber++;
				if (dwCurrNumber >= dwMaxNumber)
					return;
			}
		}		
	}
	//{
	//	// 如果攻击范围或者攻击距离为0，则直接返回
	//	//fOPRadius这个值在球锥技能中代表和人物朝向的夹角(角度标示)
	//	if( 0.0f == fOPRadius || 0.0f == fOPDist )	return;
	//	FLOAT angle = fOPRadius * 3.1415927f / 180; //算出一个夹角，当成扇形区的夹角的1/2

	//	Vector3 vecFaceTo = m_pOwner->GetFaceTo(); //拿到玩家朝向的向量
	//	if(abs(vecFaceTo.x) < 0.001f &&abs(vecFaceTo.z) < 0.001f) return ; 

	//	D3DXVec3Normalize(&vecFaceTo,&vecFaceTo);

	//	tagVisTile* pVisTile[EUD_end] = {0};
	//	pTarget->get_map()->get_visible_tile(m_pOwner->GetVisTileIndex(), pVisTile);//拿到vistile列表
	//	Role*		pRole		= NULL;
	//	Creature*	pCreature	= NULL;


	//	for(INT n = EUD_center; n < EUD_end; n++)
	//	{
	//		if( !VALID_POINT(pVisTile[n]) ) continue;

	//		// 首先检测人物
	//		package_map<DWORD, Role*>& mapRole = pVisTile[n]->mapRole;
	//		package_map<DWORD, Role*>::map_iter it = mapRole.Begin();

	//		while( mapRole.find_next(it, pRole) )
	//		{
	//			//目标是玩家，不做处理
	//			if( pRole == m_pOwner ) continue;

	//			// 目标对象限制判断
	//			if( E_Success != CheckTargetLogicLimit(pSkill, pRole) )
	//				continue;

	//			// 自身到当前检测物体的向量
	//			Vector3 VecToDes = pRole->GetCurPos() - m_pOwner->GetCurPos();
	//			// 先检查和玩家朝向方位是否一致 cos(a) > 0 是否在玩家前面
	//			if(VecToDes.x * vecFaceTo.x +VecToDes.z * vecFaceTo.z < 0.0f )
	//				continue;

	//			Vector3 vecToDesNomrliazed;
	//			D3DXVec3Normalize(&vecToDesNomrliazed,&VecToDes);
	//			FLOAT angleD2F = acos(D3DXVec3Dot(&vecToDesNomrliazed,&vecFaceTo));//算出两个向量的夹角
	//			if(angleD2F  < 0) angleD2F = -angleD2F;

	//			if(angleD2F > angle)	continue;//如果大于给定的角度就无法攻击到

	//			FLOAT fDis = D3DXVec3Length(&VecToDes); //距离检测
	//			if(fDis > fOPDist )		continue;

	//			// 判断通过，则将玩家加入到列表中
	//			m_listTargetID.PushBack(pRole->GetID());

	//			// 计算技能作用结果
	//			CalculateSkillEffect(pSkill, pRole);
	//		}

	//		// 再检测生物
	//		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->mapCreature;
	//		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.Begin();

	//		while( mapCreature.find_next(it2, pCreature) )
	//		{
	//			//目标是玩家，不做处理
	//			if( pCreature == m_pOwner ) continue;

	//			// 目标对象限制判断
	//			if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
	//				continue;

	//			// 自身到当前检测物体的向量
	//			Vector3 VecToDes = pCreature->GetCurPos() - m_pOwner->GetCurPos();
	//			// 先检查和玩家朝向方位是否一致 cos(a) > 0 是否在玩家前面
	//			if(VecToDes.x * vecFaceTo.x +VecToDes.z * vecFaceTo.z < 0.0f )
	//				continue;

	//			Vector3 vecToDesNomrliazed;
	//			D3DXVec3Normalize(&vecToDesNomrliazed,&VecToDes);
	//			FLOAT angleD2F = acos(D3DXVec3Dot(&vecToDesNomrliazed,&vecFaceTo));//算出两个向量的夹角
	//			if(angleD2F  < 0) angleD2F = -angleD2F;
	//			if(angleD2F > angle)	continue;//如果大于给定的角度就无法攻击到

	//			FLOAT fDis = D3DXVec3Length(&VecToDes); //距离检测，球面类型
	//			if(fDis < fOPDist )		continue;

	//			// 判断通过，则将玩家加入到列表中
	//			m_listTargetID.PushBack(pCreature->GetID());

	//			// 计算技能作用结果
	//			CalculateSkillEffect(pSkill, pCreature);
	//		}
	//	}
	//	}
}

//-------------------------------------------------------------------------------
// 计算技能效果
//-------------------------------------------------------------------------------
DWORD CombatHandler::CalculateSkillEffect(Skill* pSkill, Unit* pTarget)
{
	DWORD dwTargetEffectFlag = 0;

	DWORD dwTargetID = pTarget->GetID();

	INT nDmgTimes = pSkill->GetDmgTimes();

	// 无伤害技能，必命中
	if( nDmgTimes <= 0 )
	{
		m_listHitedTarget.push_back(dwTargetID);
		dwTargetEffectFlag |= ETEF_Hited;

		NET_SIS_hit_target send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = EHTC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}

		pTarget->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);
		return dwTargetEffectFlag;
	}

	// 计算命中
	BOOL bHit = CalculateHit(pSkill, pTarget);
	if( FALSE == bHit )
	{
		// 未命中
		m_listDodgedTarget.push_back(dwTargetID);

		if (m_pOwner->IsRole())
		{
			((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_miss,1);
		}
		if (pTarget->IsRole())
		{
			((Role*)pTarget)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_dodge,1);
		}
	}
	else
	{
		if(m_pOwner->IsRole())
		{
			//((Role*)m_pOwner)->KaiGuang(pTarget);
		}
		// 命中
		m_listHitedTarget.push_back(dwTargetID);
		dwTargetEffectFlag |= ETEF_Hited;
		
		//lc 命中消息改到伤害计算后发送
		//NET_SIS_hit_target send;
		//send.dw_role_id = pTarget->GetID();
		//send.dwSrcRoleID = m_pOwner->GetID();
		//send.eCause = EHTC_Skill;
		//send.dwMisc = pSkill->GetTypeID();
		//send.dwSerial = m_dwSkillSerial;

		//if( VALID_POINT(pTarget->get_map()) )
		//{
		//	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		//}

		// 计算格挡
		//BOOL bBlocked = CalculateBlock(pSkill, pTarget);
		//if( TRUE == bBlocked )
		//{
		//	// 被格挡
		//	m_listBlockedTarget.push_back(dwTargetID);
		//	dwTargetEffectFlag |= ETEF_Block;
		//}
		//else
		//{
			// 计算暴击
			BOOL bCrit = CalculateCritRate(pSkill, pTarget);
			if( TRUE == bCrit )
			{
				m_listCritedTarget.push_back(dwTargetID);
				dwTargetEffectFlag |= ETEF_Crited;

				if (m_pOwner->IsRole())
				{
					((Role*)m_pOwner)->GetAchievementMgr().UpdateAchievementCriteria(eta_combat_crit, 1);
				}
			}
		//}
	}

	// 被攻击方的被攻击触发
	pTarget->OnBeAttacked(m_pOwner, pSkill,
		dwTargetEffectFlag & ETEF_Hited, dwTargetEffectFlag & ETEF_Block, dwTargetEffectFlag & ETEF_Crited);

	return dwTargetEffectFlag;
}


//--------------------------------------------------------------------------------
// 计算命中
//--------------------------------------------------------------------------------
BOOL CombatHandler::CalculateHit(Skill* pSkill, Unit* pTarget)
{
	
	double fHit = 0.0f;
	double fShanbi = 0.0f;

	////命中
	int nCrit = m_pOwner->GetAttValue(ERA_HitRate);
	if (nCrit < 3500)
	{
		fHit = nCrit * 2;
	}
	else
	{
		fHit = 3500 * 2 + (nCrit - 3500) * 0.01;
	}

	int nFanCrit = pTarget->GetAttValue(ERA_Dodge);
	if (nFanCrit < 3500)
	{
		fShanbi = nFanCrit * 2;
	}
	else
	{
		fShanbi = 3500 * 2 + (nFanCrit - 3500) * 0.01;
	}

	////命中=暴击*(1-反暴击)
	fHit = 10000 + fHit - fShanbi;

	
	// 范围：0——100%
	if( fHit < 0.0f ) fHit = 0.0f;
	if( fHit > 10000.0f ) fHit = 10000.0f;

	// 随机看是否能命中
	return get_tool()->probability(INT(fHit/100.0f));
}

//----------------------------------------------------------------------------
// 计算格挡
//----------------------------------------------------------------------------
BOOL CombatHandler::CalculateBlock(Skill* pSkill, Unit* pTarget)
{
	// 只有攻击放处在防御方前面时，防御方才可以格挡
	if( FALSE == m_pOwner->IsInFrontOfTarget(*pTarget) )
		return FALSE;

	//招架率=招架/(招架+等级*20+180)
	FLOAT fBlock = 0.0f;
	
	INT nBlock = pTarget->GetAttValue(ERA_ShenAttack);

	fBlock = nBlock * 1.0f / (nBlock + pTarget->get_level()*20 + 180);

	//// 外功攻击
	//if( pSkill->IsExAttackSkill() )
	//{
	//	// 远程攻击
	//	if( pSkill->IsMelee() )
	//	{
	//		// 基础格挡率=（防御方当前外功防御-（攻击方外功攻击+攻击方内功攻击）/4）/30000
	//		// 格挡率=[1+（防御方当前防御技巧-攻击方当前攻击技巧）/6000] ×基础格挡率+防御方格档几率加乘
	//		FLOAT fBaseBlock = (FLOAT(pTarget->GetAttValue(ERA_ExDefense)) - FLOAT(m_pOwner->GetAttValue(ERA_ExAttack) + m_pOwner->GetAttValue(ERA_InAttack)) / 4.0f) / 30000.0f;
	//		fBlock = (1.0f + FLOAT(pTarget->GetAttValue(ERA_DefenseTec) - m_pOwner->GetAttValue(ERA_AttackTec)) / 6000.0f) * fBaseBlock + (FLOAT)pTarget->GetAttValue(ERA_Block_Rate) / 10000.0f;
	//	}

	//	// 远程攻击
	//	else if( pSkill->IsRanged() )
	//	{
	//		// 格挡率=0
	//		fBlock = 0.0f;
	//	}		
	//}

	//// 内功攻击
	//else if( pSkill->IsInAttackSkill() )
	//{
	//	// 基础格挡率=（防御方当前内功防御-（攻击方外功攻击+攻击方内功攻击）/4）/30000
	//	// 格挡率=[1+（防御方当前防御技巧-攻击方当前攻击技巧）/6600] ×基础格挡率+防御方格档几率加乘
	//	FLOAT fBaseBlock = (FLOAT(pTarget->GetAttValue(ERA_InDefense)) - FLOAT(m_pOwner->GetAttValue(ERA_ExAttack) + m_pOwner->GetAttValue(ERA_InAttack)) / 4.0f) / 30000.0f;
	//	fBlock = (1.0f + FLOAT(pTarget->GetAttValue(ERA_DefenseTec) - m_pOwner->GetAttValue(ERA_AttackTec)) / 6600.0f) * fBaseBlock + (FLOAT)pTarget->GetAttValue(ERA_Block_Rate) / 10000.0f;
	//}

	//// 绝技攻击
	//// 	else if( pSkill->IsStuntSkill() )
	//// 	{
	//// 		fBlock = 0.0f;
	//// 	}

	//// else
	//else
	//{

	//}

	// 范围：0——100%
	if( fBlock < 0.0f ) fBlock = 0.0f;
	if( fBlock > 1.0f ) fBlock = 1.0f;

	// 随机看是否能命中
	return get_tool()->probability(INT(fBlock*100.0f));
}

//-----------------------------------------------------------------------------
// 计算致命率
//-----------------------------------------------------------------------------
BOOL CombatHandler::CalculateCritRate(Skill* pSkill, Unit* pTarget)
{
	double fCrit = 0.0f;
	double fFanCirt = 0.0f;
	//double fFanCrit = 0.0f;
	////暴击率=暴击/(暴击+等级20+180)
	int nCrit = m_pOwner->GetAttValue(ERA_Crit_Rate);
	if (nCrit < 3500)
	{
		fCrit = nCrit * 2;
	}
	else
	{
		fCrit = 3500 * 2 + (nCrit - 3500) * 0.01;
	}
	
	int nFanCrit = pTarget->GetAttValue(ERA_UnCrit_Rate);
	if (nFanCrit < 3500)
	{
		fFanCirt = nFanCrit * 2;
	}
	else
	{
		fFanCirt = 3500 * 2 + (nFanCrit - 3500) * 0.01;
	}

	////暴击率=暴击*(1-反暴击)
	fCrit = fCrit - fFanCirt;

	// 范围：0——100%
	if( fCrit < 0.0f ) fCrit = 0.0f;
	if( fCrit > 10000.0f ) fCrit = 10000.0f;

	// 随机看是否能命中
	return get_tool()->probability(INT(fCrit/100));
}

//-----------------------------------------------------------------------------
// 计算致命量
//-----------------------------------------------------------------------------
FLOAT CombatHandler::CalculateCritAmount(Skill* pSkill, Unit* pTarget)
{
	//暴伤率=暴伤/(暴伤+等级*10+90)
	double fCritAmount = 1.5f;
	//double nCritAmount = m_pOwner->GetAttValue(ERA_Crit_Amount);
	////fCritAmount = nCritAmount*1.0f / ( nCritAmount + m_pOwner->get_level()*10 + 90);
	////点数/(点数+140*等级^(等级/100)+400+150^(MAX(1,(等级/42))))
	//fCritAmount = nCritAmount/(nCritAmount+140.0*pow(m_pOwner->get_level(), m_pOwner->get_level()/100.0)+400+pow(150.0, max(1, m_pOwner->get_level()/42.0)));

	////反伤率=反伤/(反伤+等级*10+90)
	//double fFanCritAmount = 0.0f;
	//double nFanCritAmount = pTarget->GetAttValue(ERA_UnCrit_Amount);
	////fFanCritAmount = nFanCritAmount * 1.0f/ (nFanCritAmount + pTarget->get_level()*10 + 90);
	////点数*(MAX(41,等级)/65)/(点数+等级*150^(等级/70))
	//fFanCritAmount =  nFanCritAmount*(max(41.0, m_pOwner->get_level())/65.0)/(nFanCritAmount+m_pOwner->get_level()*pow(150.0, m_pOwner->get_level()/70.0));


	////暴击伤害=2+暴伤-反暴伤
	//fCritAmount = 2+fCritAmount-fFanCritAmount;

	// 范围：0——100%
	//if( fCritAmount < 1.25f ) fCritAmount = 1.25f;
	//if( fCritAmount > 2.25f ) fCritAmount = 2.25f;

	return (FLOAT)fCritAmount;
}

//-----------------------------------------------------------------------------
// 计算技能伤害
//-----------------------------------------------------------------------------
VOID CombatHandler::CalculateDmg(Skill* pSkill, Unit* pTarget)
{
	// 目标已经死亡，直接返回
	if( pTarget->IsDead() ) return;

	DWORD dwTargetID = pTarget->GetID();

	// 暴击参数
	bool bCrit = false;
	bool bBlock = false;
	FLOAT fCrit = 1.0f;

	// 首先判断该目标是否被闪避了
	if( m_listDodgedTarget.is_exist(dwTargetID) )
	{
		// 发送未命中消息
		NET_SIS_role_hp_change send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = ERHPCC_SkillDamage;
		send.bMiss = true;
		send.nHPChange = 0;
		send.dwMisc = pSkill->GetTypeID();
		send.dwMisc2 = m_nSkillCurDmgIndex;
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}
		return;
	}
	else
	{
		NET_SIS_hit_target send;
		send.dw_role_id = pTarget->GetID();
		send.dwSrcRoleID = m_pOwner->GetID();
		send.eCause = EHTC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		send.dwSerial = m_dwSkillSerial;

		if( VALID_POINT(pTarget->get_map()) )
		{
			pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		}

	}
	// 再判断是否被招架了
	//if( m_listBlockedTarget.is_exist(dwTargetID) )
	//{
	//	bBlock = true;
		// 发送招架消息
		//NET_SIS_role_hp_change send;
		//send.dw_role_id = pTarget->GetID();
		//send.dwSrcRoleID = m_pOwner->GetID();
		//send.eCause = ERHPCC_SkillDamage;
		//send.bBlocked = true;
		//send.dwMisc = pSkill->GetTypeID();
		//send.dwMisc2 = m_nSkillCurDmgIndex;
		//send.dwSerial = m_dwSkillSerial;

		//if( VALID_POINT(pTarget->get_map()) )
		//{
		//	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
		//}
		//return;
	//}

	// 再判断是否暴击
	if( m_listCritedTarget.is_exist(dwTargetID) )
	{
		// 计算暴击参数
		bCrit = true;
		fCrit = CalculateCritAmount(pSkill, pTarget);
	}

	FLOAT fBaseDmg		=	CalBaseDmg(pSkill, pTarget);			// 基础伤害
	//FLOAT fAttDefCoef	=	CalAttackDefenceCoef(pSkill, pTarget);	// 护甲减免
	FLOAT fDerateCoef	=	CalDerateCoef(pSkill, pTarget);			// 减免影响
	//FLOAT fLevelCoef	=	CalLevelCoef(pSkill, pTarget);			// 等级影响

	// 最终伤害
	FLOAT fDmg = fBaseDmg * fDerateCoef/** fAttDefCoef * fMoraleCoef  * fInjuryCoef* fLevelCoef*/ ;
	fDmg = fDmg + (FLOAT)m_pOwner->GetAttValue(ERA_ExDamage) - (FLOAT)pTarget->GetAttValue(ERA_ExDamage_Absorb);
	


	// 技能脚本
	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		fDmg = pScript->CalculateDmg(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID(), bBlock, bCrit, fDmg);
	}

	// 计算暴击参数
	INT nDmg = INT(fDmg * fCrit);
	
	if( nDmg < 1 ) nDmg = 1;
	
	// 减血
	pTarget->ChangeHP(-nDmg, m_pOwner, pSkill, NULL, bCrit, bBlock, m_dwSkillSerial, m_nSkillCurDmgIndex);
	//OnDmg(pTarget, nDmg, pTarget->get_level() - m_pOwner->get_level(), bCrit);

	// 计算怪物仇恨
	if(pTarget->IsCreature())
	{
		if(!((Creature*)pTarget)->IsDead())
		{
			AIController* pAI = ((Creature*)pTarget)->GetAI();
			if( VALID_POINT(pAI) )
			{
				INT	nValue = pSkill->GetEnmity() + m_pOwner->GetAttValue(ERA_Enmity_Degree);
				// 加上伤害仇恨度
				nValue += (nDmg /** pSkill->GetEnmityParam()*/);
				pAI->AddEnmity(m_pOwner, nValue);
				pAI->OnEvent(m_pOwner, ETEE_BeAttacked);
			}
		}
	}
	

	// 击杀目标脚本
	if (pTarget->IsDead())
	{
		if (VALID_POINT(pScript))
		{
			pScript->KillUnit(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID());
		}
	}
}

VOID CombatHandler::CalculateDmgNoSpe(Skill* pSkill, Unit* pTarget)
{
	// 目标已经死亡，直接返回
	if( pTarget->IsDead() ) return;

	DWORD dwTargetID = pTarget->GetID();

	// 暴击参数
	bool bCrit = false;
	bool bBlock = false;
	FLOAT fCrit = 1.0f;

	//// 首先判断该目标是否被闪避了
	//if( m_listDodgedTarget.is_exist(dwTargetID) )
	//{
	//	// 发送未命中消息
	//	NET_SIS_role_hp_change send;
	//	send.dw_role_id = pTarget->GetID();
	//	send.dwSrcRoleID = m_pOwner->GetID();
	//	send.eCause = ERHPCC_SkillDamage;
	//	send.bMiss = true;
	//	send.nHPChange = 0;
	//	send.dwMisc = pSkill->GetTypeID();
	//	send.dwMisc2 = m_nSkillCurDmgIndex;
	//	send.dwSerial = m_dwSkillSerial;

	//	if( VALID_POINT(pTarget->get_map()) )
	//	{
	//		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
	//	}
	//	return;
	//}
	//else
	//{
	//	NET_SIS_hit_target send;
	//	send.dw_role_id = pTarget->GetID();
	//	send.dwSrcRoleID = m_pOwner->GetID();
	//	send.eCause = EHTC_Skill;
	//	send.dwMisc = pSkill->GetTypeID();
	//	send.dwSerial = m_dwSkillSerial;

	//	if( VALID_POINT(pTarget->get_map()) )
	//	{
	//		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);
	//	}

	//}

	//// 再判断是否暴击
	//if( m_listCritedTarget.is_exist(dwTargetID) )
	//{
	//	// 计算暴击参数
	//	bCrit = true;
	//	fCrit = CalculateCritAmount(pSkill, pTarget);
	//}

	FLOAT fBaseDmg		=	CalBaseDmg(pSkill, pTarget);			// 基础伤害
	//FLOAT fAttDefCoef	=	CalAttackDefenceCoef(pSkill, pTarget);	// 护甲减免
	FLOAT fDerateCoef	=	CalDerateCoef(pSkill, pTarget);			// 减免影响
	//FLOAT fLevelCoef	=	CalLevelCoef(pSkill, pTarget);			// 等级影响

	// 最终伤害
	FLOAT fDmg = fBaseDmg * fDerateCoef/** fAttDefCoef * fMoraleCoef  * fInjuryCoef* fLevelCoef*/ ;
	fDmg = fDmg + (FLOAT)m_pOwner->GetAttValue(ERA_ExDamage) - (FLOAT)pTarget->GetAttValue(ERA_ExDamage_Absorb);



	// 技能脚本
	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		fDmg = pScript->CalculateDmg(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID(), bBlock, bCrit, fDmg);
	}

	// 计算暴击参数
	INT nDmg = INT(fDmg * fCrit);

	if( nDmg < 1 ) nDmg = 1;

	// 减血
	pTarget->ChangeHP(-nDmg, m_pOwner, pSkill, NULL, bCrit, bBlock, m_dwSkillSerial, m_nSkillCurDmgIndex);
	//OnDmg(pTarget, nDmg, pTarget->get_level() - m_pOwner->get_level(), bCrit);

	// 计算怪物仇恨
	if(pTarget->IsCreature())
	{
		if(!((Creature*)pTarget)->IsDead())
		{
			AIController* pAI = ((Creature*)pTarget)->GetAI();
			if( VALID_POINT(pAI) )
			{
				INT	nValue = pSkill->GetEnmity() + m_pOwner->GetAttValue(ERA_Enmity_Degree);
				// 加上伤害仇恨度
				nValue += (nDmg * pSkill->GetEnmityParam());
				pAI->AddEnmity(m_pOwner, nValue);
				pAI->OnEvent(m_pOwner, ETEE_BeAttacked);
			}
		}
	}

	// 击杀目标脚本
	if (pTarget->IsDead())
	{
		if (VALID_POINT(pScript))
		{
			pScript->KillUnit(m_pOwner->get_map(), pSkill->GetID(), pSkill->get_level(), m_pOwner->GetID(), pTarget->GetID());
		}
	}
}

VOID CombatHandler::ClearPilotList()
{
	std::map<DWORD, tagPilotUnit*>::iterator it = m_listPilotUnit.begin();
	while(it != m_listPilotUnit.end())
	{
		tagPilotUnit* pUnit = it->second;


		Map* pMap = g_mapCreator.get_map(pUnit->dwMapID, pUnit->dwInstanceID);
		Creature* pCreature = NULL;
		if( VALID_POINT(pMap) )
		{
			pCreature = pMap->find_creature(pUnit->dwCretureID);
		}
		
		if (VALID_POINT(pCreature))
		{
			pCreature->OnDisappear();
		}

		SAFE_DELETE(it->second);
		it->second = NULL;
		it++;
	}

	m_listPilotUnit.clear();
}
//------------------------------------------------------------------------------
// 计算基础伤害
//------------------------------------------------------------------------------
inline FLOAT CombatHandler::CalBaseDmg(Skill* pSkill, Unit* pTarget)
{
	FLOAT	fBaseDmg	=	1.0f;

	INT		nSkillDmg = 0;
	INT		nSkillDmgMod = 0;
	INT		nSKillDmgModPct = 0;
	
	nSkillDmg =	pSkill->GetDmg(m_nSkillCurDmgIndex);
	nSkillDmgMod = pSkill->GetDmgMod(m_nSkillCurDmgIndex);
	nSKillDmgModPct = pSkill->GetDmgModPct(m_nSkillCurDmgIndex);
	
	ESkillTypeEx eSkillTypeEx = pSkill->GetTypeEx();
	INT nMinAttack = 0;
	INT nMaxAttack = 0;
	INT nMinDef = 0;
	INT nMaxDef = 0;
	switch (eSkillTypeEx)
	{
	case ESSTE_Sword:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ExAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ExAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_ExAttack);
			nMaxDef = pTarget->GetAttValue(ERA_ExDefense);
		}
		break;
	case ESSTE_Blade:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_InAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_InAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_InAttack);
			nMaxDef = pTarget->GetAttValue(ERA_InDefense);
		}
		break;
	case ESSTE_Wand:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ArmorEx);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ArmorIn);
			nMinDef = pTarget->GetAttValue(ERA_InAttack);
			nMaxDef = pTarget->GetAttValue(ERA_InDefense);
		}
		break;
	default:
		{
			nMinAttack = m_pOwner->GetAttValue(ERA_ExAttackMin);
			nMaxAttack = m_pOwner->GetAttValue(ERA_ExAttackMax);
			nMinDef = pTarget->GetAttValue(ERA_ExAttack);
			nMaxDef = pTarget->GetAttValue(ERA_ExDefense);
		}
		break;
	}

	INT	nExAttack = 0;
	if (m_pOwner->GetAttValue(ERA_Luck) >= 9)
	{
		nExAttack = nMaxAttack;
	}
	else if (m_pOwner->GetAttValue(ERA_Luck) <= -9)
	{
		nExAttack = nMinAttack;
	}
	else
	{
		nExAttack = get_tool()->rand_in_range(nMinAttack, nMaxAttack);
	}

	INT nExDef = get_tool()->rand_in_range(nMinDef, nMaxDef);

	FLOAT fExAttack = nExAttack - nExDef;
	if( nSkillDmg > 100000 )
	{
		// 取的是伤害的倍数
		fBaseDmg = fExAttack * (FLOAT(nSkillDmg - 100000) / 10000.0f);
	}
	else
	{
		// 取的是技能伤害
		fBaseDmg =  fExAttack + (FLOAT)nSkillDmg;
	}
	
	fBaseDmg = fBaseDmg + fBaseDmg*nSKillDmgModPct/10000.0f + nSkillDmgMod;


	return get_tool()->fluctuate(fBaseDmg, 5, 5);
}

//-----------------------------------------------------------------------------
// 计算攻防影响
//-----------------------------------------------------------------------------
inline FLOAT CombatHandler::CalAttackDefenceCoef(Skill* pSkill, Unit* pTarget)
{
	//防御削减=护甲/（护甲+等级*50+60）
	INT nDef = pTarget->GetAttValue(ERA_ExDefense) /*+ pTarget->GetAttValue(ERA_ArmorEx)*/;


	FLOAT fCoef = nDef * 1.0f / (nDef + pTarget->get_level() * 50+ 60);

	return (1 - fCoef);
}


//------------------------------------------------------------------------
// 计算减免影响
//------------------------------------------------------------------------
inline FLOAT CombatHandler::CalDerateCoef(Skill* pSkill, Unit* pTarget)
{
	FLOAT fDerateCoef = 1.0f;
	

	//伤害减免 = 目标攻击减免+所有伤害减免+对应类型伤害减免
	fDerateCoef =  FLOAT( pTarget->GetAttValue(ERA_Derate_ALL) );
	
	fDerateCoef = fDerateCoef / 10000 ;

	// 计算最终值
	fDerateCoef = 1.0f - fDerateCoef;
	if( fDerateCoef < 0.1f ) fDerateCoef = 0.1f;
	if( fDerateCoef > 1.0f ) fDerateCoef = 1.0f;

	return fDerateCoef;
}


//-------------------------------------------------------------------------
// 计算等级影响
//-------------------------------------------------------------------------
inline FLOAT CombatHandler::CalLevelCoef(Skill* pSkill, Unit* pTarget)
{
	// 外功攻击和内功攻击
	// 1-（防御方等级-攻击方等级）/75     取值（0.2~1.8）
	INT nLevelSub = pTarget->get_level() - m_pOwner->get_level();
	//if( pSkill->IsExAttackSkill() || pSkill->IsInAttackSkill() )
	if( nLevelSub > 0)
	{
		FLOAT fDmgCof = 0.0f;
		if (nLevelSub > 20)
		{
			fDmgCof = (FLOAT)(31-(fDmgCof-50)*(fDmgCof-50)/200)/100.0f;		
		}
		else if (nLevelSub > 10)
		{
			fDmgCof = (FLOAT)(nLevelSub*nLevelSub/17+nLevelSub/2-6.8)/100.0f;
		}
		else
		{
			fDmgCof = (FLOAT)(nLevelSub * nLevelSub /25 + 1)/100.0f;
		}
		FLOAT fCoef = 1.0f - fDmgCof;
		if( fCoef < 0.7f ) fCoef = 0.7f;
		if( fCoef > 1.0f ) fCoef = 1.0f;

		return fCoef;
	}
	else
	{
		return 1.0f;
	}
}

//----------------------------------------------------------------------------
// 计算技能消耗
//----------------------------------------------------------------------------
VOID CombatHandler::CalculateCost(Skill* pSkill)
{
	// 体力消耗
	INT nHPCost = pSkill->GetCost(ESCT_HP);
	if( nHPCost > 0  )
	{
		m_pOwner->ChangeHP(-nHPCost, m_pOwner);
	}

	// 真气消耗
	INT nMPCost = pSkill->GetCost(ESCT_MP);
	if( nMPCost > 0  )
	{
		m_pOwner->ChangeMP(-nMPCost);
	}

	// 怒气消耗
	//INT nRageCost = pSkill->GetCost(ESCT_Rage);
	//if( nRageCost > 0 )
	//{
	//	m_pOwner->ChangeRage(-nRageCost);
	//}


	// 持久消耗
	//INT nEnduranceCost = pSkill->GetCost(ESCT_Endurance);
	//if( nEnduranceCost > 0 )
	//{
	//	m_pOwner->ChangeEndurance(-nEnduranceCost);
	//}

	// 活力消耗
	//INT nValicityCost = pSkill->GetCost(ESCT_Valicity);
	//if( nValicityCost > 0  )
	//{
	//	m_pOwner->ChangeVitality(-nValicityCost);
	//}
}

//----------------------------------------------------------------------------
// 物品使用判断
//----------------------------------------------------------------------------
INT	CombatHandler::can_use_item(tagItem *pItem)
{
	if( !VALID_POINT(pItem)  )
		return E_SystemError;

	if( CheckItemConflict(pItem) ) return E_UseItem_Operating;

	INT nRet = E_Success;

	nRet = CheckItemAbility(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckOwnerLimitItem();
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleProtoLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleStateLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckRoleVocationLimit(pItem);
	if( E_Success != nRet ) return nRet;

	nRet = CheckMapLimit(pItem);
	if( E_Success != nRet ) return nRet;

	return nRet;
}


//----------------------------------------------------------------------------
// 检测物品本身
//----------------------------------------------------------------------------
INT	CombatHandler::CheckItemAbility(tagItem *pItem)
{
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// 物品是否是可使用物品
	if(MIsEquipment(pItem->dw_data_id) || pItem->pProtoType->dwBuffID0 == INVALID_VALUE)
		return E_UseItem_ItemCanNotUse;

	// 物品的冷却时间还没到，则不可以使用
	if(((Role*)m_pOwner)->GetItemMgr().IsItemCDTime(pItem->dw_data_id))
		return E_UseItem_CoolDowning;

	return E_Success;
}

//----------------------------------------------------------------------------
// 检测使用者本身
//----------------------------------------------------------------------------
INT CombatHandler::CheckOwnerLimitItem()
{
	// 是否处在不能使用技能的状态
	if( m_pOwner->IsInStateCantCastSkill() )
		return E_UseItem_UseLimit;

	if (m_pOwner->IsInState(ES_Dizzy))
		return E_UseItem_UseLimit;

	if( IsRide() ) 
		return E_UseItem_UseLimit;

	return E_Success;
}

//----------------------------------------------------------------------------
// 检测人物属性限制
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleProtoLimit(tagItem *pItem)
{
	if( !VALID_POINT(pItem) ) return E_UseItem_ItemNotExist;

	// 性别限制
	if( pItem->pProtoType->eSexLimit != ESL_Null )
	{
		if( ESL_Man == pItem->pProtoType->eSexLimit )
		{
			if( 1 != m_pOwner->GetSex() )
				return E_UseItem_SexLimit;
		}
		else if( ESL_Woman == pItem->pProtoType->eSexLimit )
		{
			if( 0 != m_pOwner->GetSex() )
				return E_UseItem_SexLimit;
		}
		else
		{

		}
	}

	// 等级限制
	if(pItem->pProtoType->byMinUseLevel > m_pOwner->get_level() 
		|| pItem->pProtoType->byMaxUseLevel < m_pOwner->get_level())
		return E_UseItem_LevelLimit;

	// 职业限制

	return E_Success;
}


//----------------------------------------------------------------------------
// 检测人物状态限制
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleStateLimit(tagItem *pItem)
{
	// 特殊状态限制（死亡 ，眩晕）
	DWORD dwSelfStateFlag = m_pOwner->GetStateFlag();

	if( (dwSelfStateFlag & pItem->pProtoType->dwStateLimit) != dwSelfStateFlag )
	{
		return E_UseItem_SelfStateLimit;
	}

	// 玩家在活动中的限制

	// 副本限制
	return E_Success;
}

//----------------------------------------------------------------------------
// 检测人物职业限制
//----------------------------------------------------------------------------
INT CombatHandler::CheckRoleVocationLimit(tagItem *pItem)
{
	if(!VALID_POINT(pItem)) return E_UseItem_ItemNotExist;

	if(!m_pOwner->IsRole()) return E_Success;

	//INT nClass = (INT)((Role*)m_pOwner)->GetClass();
	//INT nClassEx = (INT)((Role*)m_pOwner)->GetClassEx();
	INT nClass = (INT)(static_cast<Role*> (m_pOwner)->GetClass());
	INT nClassEx = (INT)(static_cast<Role*> (m_pOwner)->GetClassEx());

	/*if(pItem->pProtoType->dwVocationLimit != 5)
	{
	if(pItem->pProtoType->dwVocationLimit != nClass)
	return E_UseItem_VocationLimit;
	}*/

	/*if (((nClassEx << 9) + nClass) & pItem->pProtoType->dwVocationLimit)
	return E_Success;
	else
	return E_UseItem_VocationLimit;*/

	return E_Success;
}

//----------------------------------------------------------------------------
// 检测地图限制
//----------------------------------------------------------------------------
INT CombatHandler::CheckMapLimit(tagItem* pItem)
{
	// 判断地图限制
	if(VALID_POINT(m_pOwner->get_map()))
	{
		BOOL bUesAble = m_pOwner->get_map()->can_use_item(pItem->dw_data_id, pItem->n64_serial);
		if( !bUesAble )	return E_UseItem_MapLimit;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------
// 测试物品使用冲突，如果冲突则为TRUE，不冲突为FALSE
//-------------------------------------------------------------------------------
BOOL CombatHandler::CheckItemConflict(tagItem* pItem)
{
	if( IsUseItem() ) return TRUE;	// 当前正在使用物品，则不能使用

	if( IsUseSkill() )
	{
		// 如果物品是起手物品，则不可以使用
		if( pItem->pProtoType->nPrepareTime > 0 ) return TRUE;
		else return FALSE;
	}

	return FALSE;
}

//-------------------------------------------------------------------------------
// 获取buff影响后的目标护甲值
//-------------------------------------------------------------------------------
FLOAT CombatHandler::GetTargetArmor(Unit* target, DWORD dwArmorType)
{
	//FLOAT fArmor = (FLOAT)target->GetAttValue(ERA_ArmorIn) * m_fTargetArmorLeftPct;
	//if ( target->IsRole() )
	//{
	//	Role* pRole = (Role*)target;
	//	DWORD dwClass = (DWORD)pRole->GetClass();
	//	if ((dwArmorType == 0 && (dwClass == EV_HuanMo || dwClass == EV_YinWu)) || 
	//		(dwArmorType == 1 && (dwClass == EV_JinGang || dwClass == EV_ShaXing || dwClass == EV_FengYu)))
	//	{
	//		fArmor = fArmor * 0.5f;
	//	}
	//}
	//return fArmor; 
	return 0;
}


VOID CombatHandler::CalSkillTargetList(package_list<DWORD>& targetList, const tagPilotUnit* pPilotUnit, Vector3 vPosDes)
{
	Skill* pSkill = m_pOwner->GetSkill(pPilotUnit->dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	
	FLOAT fOPRadius = pPilotUnit->fOPRadius;
	FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

		// 如果攻击范围为0，则直接返回
		if( 0.0f == fOPRadius )
			return;

		targetList.clear();

		tagVisTile* pVisTile[EUD_end] = {0};

		// 得到攻击范围内的vistile列表
		m_pOwner->get_map()->get_visible_tile(vPosDes, fOPRadius, pVisTile);
		Role*		pRole		= NULL;
		Creature*	pCreature	= NULL;

		for(INT n = EUD_center; n < EUD_end; n++)
		{
			if( !VALID_POINT(pVisTile[n]) ) continue;

			// 首先检测人物
			package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
			package_map<DWORD, Role*>::map_iter it = mapRole.begin();

			while( mapRole.find_next(it, pRole) )
			{
				// 和目标一样，不做处理
				//if( pRole == pTarget || pRole == m_pOwner ) continue;

				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vPosDes, pRole->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;


				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pRole ) )
					continue;

				CheckInCombat(pRole);
				// 射线检测

				// 判断通过，则将玩家加入到列表中
				targetList.push_back(pRole->GetID());

				// 计算技能作用结果
				//CalculateSkillEffect(pSkill, pRole);
				((Unit*)pRole)->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);

			}

			// 再检测生物
			package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
			package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

			while( mapCreature.find_next(it2, pCreature) )
			{
				// 和目标一样，不做处理
				if( pCreature == m_pOwner ) continue;

				// 技能距离判断
				FLOAT fDistSQ = Vec3DistSq(vPosDes, pCreature->GetCurPos());
				if( fDistSQ > fOPRadiusSQ  ) continue;

				// 目标对象限制判断
				if( E_Success != CheckTargetLogicLimit(pSkill, pCreature) )
					continue;

				CheckInCombat(pCreature);

				// 判断通过，则将生物加入到列表中
				targetList.push_back(pCreature->GetID());

				// 计算技能作用结果
				//CalculateSkillEffect(pSkill, pCreature);
				((Unit*)pCreature)->OnBeAttacked(m_pOwner, pSkill, TRUE, FALSE, FALSE);

			}
		}
	


}
//-----------------------------------------------------------------------------
// 更新持续地物型技能伤害
//-----------------------------------------------------------------------------
VOID CombatHandler::UpdatePrepareUnit()
{
	std::map<DWORD, tagPilotUnit*>::iterator iter = m_listPilotUnit.begin();
	while(iter != m_listPilotUnit.end())
	{
		tagPilotUnit* pUnit = iter->second;

		Map* pMap = g_mapCreator.get_map(pUnit->dwMapID, pUnit->dwInstanceID);
		Creature* pCreature = NULL;
		if( VALID_POINT(pMap) )
		{
			pCreature = pMap->find_creature(pUnit->dwCretureID);
		}

		// 时间到了，删除地物和数据
		if (pUnit->dwOverTime <= 0 || !VALID_POINT(pCreature))
		{

			if (VALID_POINT(pCreature))
			{
				pCreature->OnDisappear();
			}

			m_listPilotUnit.erase(iter++);
			SAFE_DELETE(pUnit);
		}
		else
		{
			package_list<DWORD>	listTargetID;				// 技能目标列表
			CalSkillTargetList(listTargetID, pUnit, pCreature->GetCurPos());
			
			package_list<DWORD>::list_iter it = listTargetID.begin();
			DWORD dwTargetID = INVALID_VALUE;
			
			Skill* pSkill = m_pOwner->GetSkill(pUnit->dwSkillID);
			if( !VALID_POINT(pSkill) ) return;


			while( listTargetID.find_next(it, dwTargetID) )
			{
				// 找到这个目标
				Unit* pTarget = m_pOwner->get_map()->find_unit(dwTargetID);

				if( !VALID_POINT(pTarget) || pTarget == m_pOwner || pCreature == pTarget) continue;

				// 计算伤害
				CalculateDmgNoSpe(pSkill, pTarget);
			}

			pUnit->dwOverTime--;
			iter++;
		}
		
	}
	
}
//-----------------------------------------------------------------------------
// 计算能量效果
//-----------------------------------------------------------------------------
//VOID CombatHandler::OnHit(Skill* pSkill, INT32 nlevelSub, BOOL bCrit)
//{
//	//不是玩家不计算
//	if (!m_pOwner->IsRole()) return;
//
//	Role* pRole = (Role*)m_pOwner;
//	switch (pRole->GetClass())
//	{
//	case EV_Warrior://猛将
//		{
//			//不是增长能量类型技能不计算
//			if (pSkill->GetCost(ESCT_MP) == -1) 
//			{
//				FLOAT fAddPower = 10;
//
//				/*if (nlevelSub < -5) 
//					fAddPower = 8.0f;
//				else if (nlevelSub > 5) 
//					fAddPower = 4.0f;
//				else 
//					fAddPower = nlevelSub+8.0f;*/
//				//if (bCrit)
//				//	fAddPower *= 1.5f;
//				pRole->ChangeMP((INT)fAddPower);
//			}
//		
//		}
//		break;
//	case EV_Blader://少保
//		{
//			//增长能量类型技能
//			if (pSkill->GetCost(ESCT_MP) == -1) 
//			{
//				pRole->ChangeMP(1);
//			}
//			//消耗所有能量
//			else if (pSkill->GetCost(ESCT_MP) == -2)
//			{
//				pRole->ChangeMP(-m_pOwner->GetAttValue(ERA_MP));
//			}
//		}
//		
//		break;
//	case EV_Taoist://游侠
//		{
//			switch (GetChargeType())
//			{
//			case 0:
//				{
//					//蓄力技能
//					if (pSkill->GetCost(ESCT_MP) == -2) 
//					{
//						pRole->ChangeMP(bCrit ? 3:2);
//					}
//	
//				}
//				break;
//			case 1:
//				pRole->SetAttValue(ERA_MP, 5);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			case 2:
//				pRole->SetAttValue(ERA_MP, 9);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			case 3:
//				pRole->SetAttValue(ERA_MP, 11);
//				if (pRole->GetAttValue(ERA_MP) >= pRole->GetAttValue(ERA_MaxMP))
//				{
//					pRole->SetAttValue(ERA_MP, 0);
//				}
//				break;
//			default:
//				break;
//			}
//
//		}
//		break;
//	default:
//		break;
//
//	}
//}



//VOID CombatHandler::OnDmg(Unit* pRole, INT dmg, INT32 nlevelSub, BOOL bCrit)
//{
//	if (dmg <= 0) return;
//	//不是玩家不计算
//	if (!pRole->IsRole()) return;
//		
//	switch (((Role*)pRole)->GetClass())
//	{
//	case EV_Warrior:
//		{
//			//FLOAT fAddPower = 0;
//			//FLOAT fparam = dmg/pRole->GetAttValue(ERA_MaxHP)*1.0f;
//			////判断伤害是否超过20%生命
//			//if (fparam > 0.2f)
//			//{
//			//	//怒气增加相同比例
//			//	fAddPower = fparam * pRole->GetAttValue(ERA_MaxMP);
//			//}
//			//else
//			//{
//			//	if (nlevelSub < -5) 
//			//		fAddPower = 6.0f;
//			//	else if (nlevelSub > 5) 
//			//		fAddPower = 2.0f;
//			//	else 
//			//		fAddPower = nlevelSub+4.0f;
//			//	if (bCrit)
//			//		fAddPower *= 1.5f;
//			//}
//			
//			FLOAT fAddPower = 60;
//			
//			if (bCrit)
//				fAddPower *= 2.0f;
//
//			pRole->ChangeMP((INT)fAddPower);
//		}
//		break;
//	case EV_Taoist:
//		{
//			INT nPower = pRole->GetAttValue(ERA_MP);
//			if (nPower == 0)
//				return;
//			
//			// 被打退的几率
//			INT nPro = pRole->GetAttValue(ERA_RemoteAttack_DodgeRate);
//			if (nPro > get_tool()->tool_rand() % 10000)
//				return;
//
//			INT subPower = nPower - (bCrit?3:2);
//			if (nPower > 8)
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 9 ? subPower:9);
//			}
//			else if (nPower > 4)
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 5 ? subPower:5);
//			}
//			else
//			{
//				pRole->SetAttValue(ERA_MP, subPower > 1 ? subPower:1);	
//			}
//		}
//		break;
//	default:
//		break;
//
//	}	
//}

//-----------------------------------------------------------------------------
// 游侠的能量变化逻辑相关函数
//-----------------------------------------------------------------------------
//得到蓄力类型
//DWORD CombatHandler::GetChargeType()
//{
//	INT nPower = m_pOwner->GetAttValue(ERA_MP);
//	
//	INT nChargeValue = 0;
//	if ( nPower>2 && nPower<5 )
//		nChargeValue = 1;
//
//	else if ( nPower>6 && nPower<9 )
//		nChargeValue = 2;
//
//	else if ( nPower == 10 )
//		nChargeValue = 3;
//
//	return nChargeValue;
//}
