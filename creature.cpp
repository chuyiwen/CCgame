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
*	@brief		非玩家生物类,怪物,npc,宠物,地物
*/


#include "StdAfx.h"

#include "../../common/WorldDefine/move_define.h"
#include "../../common/WorldDefine/creature_define.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "NPCTeam_define.h"

#include "unit.h"
#include "buff.h"
#include "creature.h"
#include "creature_ai.h"
#include "role.h"
#include "map.h"
#include "loot_mgr.h"
#include "team.h"
#include "group_mgr.h"
#include "script_mgr.h"
#include "path_finder.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "ai_trigger.h"
#include "guild.h"
#include "guild_manager.h"
#include "map_instance.h"
#include "mail_mgr.h"

Creature* Creature::Create( DWORD dwID, DWORD dwMapID, Map* pMap,const tagCreatureProto* pProto, Vector3& vPos, Vector3& vFaceTo, ECreatureSpawnedType eSpawnedType, DWORD dwSpawnerID, DWORD dwSpawnGroupID, BOOL bCollide, 
						   const tag_map_way_point_info_list* patrolList, DWORD dwTeamID/*=INVALID_VALUE*/, DWORD dwGuildID/*=INVALID_VALUE*/)
{
	Creature* pNew = new Creature(dwID, dwMapID, vPos, vFaceTo, eSpawnedType, dwSpawnerID, dwSpawnGroupID, bCollide, dwTeamID, dwGuildID);
	//pNew->m_bCanBeAttack = pProto->bCanBeAttack;
	pNew->m_eCreatureType = pProto->eType;
	pNew->SetMap(pMap);
	pNew->Init(pProto, patrolList);
	
	return pNew;
}

VOID Creature::Delete( Creature* &pCreature )
{
	SAFE_DELETE(pCreature);
}
//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
Creature::Creature(DWORD dwID, DWORD dwMapID, const Vector3& vPos, const Vector3& vFaceTo, 
				   ECreatureSpawnedType eSpawnedType, DWORD dwSpawnerID, DWORD dwSpawnGroupID,BOOL bCollide, DWORD dwTeamID, DWORD dwGuildID)
				   :Unit(dwID, dwMapID, vPos, vFaceTo), ScriptData(),
				   m_pProto(NULL), m_pAIController(NULL), m_eSpawnedType(eSpawnedType), m_dwSpawnerID(dwSpawnerID),m_dwSpawnGroupID(dwSpawnGroupID), m_nLiveTick(0),
				   m_vBornPos(vPos), m_vBornFace(vFaceTo), m_bCollide(bCollide), m_bTagged(FALSE), m_dwTaggedOwner(INVALID_VALUE),
				   m_nRespawnTickCountDown(0),/* m_pMeleeSkill(NULL), m_pRangedSkill(NULL),*/ m_dwTeamID(dwTeamID), m_pScript(NULL),
				   m_dwGuildID(dwGuildID), m_bDoorOpened(FALSE),m_DoorOpenTime(0),/*m_bBeiZhan(FALSE),*/m_bDelSelf(FALSE),m_bDelMap(FALSE),
				  /* m_bCanBeAttack(FALSE),*/m_eCreatureType(ECT_NULL), m_DoorObjID(0)/*,m_bUsed(false),n_guildPlantIndex(INVALID_VALUE),
				  m_dwTuDui(INVALID_VALUE),m_dwMaxTield(0),m_pPlantdata(NULL)*/

{

}

//-------------------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------------------
Creature::~Creature()
{
	Destroy();
}

//--------------------------------------------------------------------------------
// 初始化
//--------------------------------------------------------------------------------
BOOL Creature::Init(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList)
{
	m_pProto = pProto;

	// 初始化脚本
	m_pScript = g_ScriptMgr.GetCreatureScript(pProto->dw_data_id);

	// 初始化属性
	InitAtt(pProto);

	// 初始化AI
	InitAI(pProto, patrolList);

	// 计算生物初始属性
	CalInitAtt();

	// 调用脚本---在这里初始化脚本的话,由于还没给怪设置地图,所以总是失败的 LC
	/*if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnLoad(this);
	}*/

	return TRUE;
}
VOID Creature::OnScriptLoad()
{
	// 调用脚本
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnLoad(this);
	}
}

VOID Creature::ResetFaceTo()
{
	m_MoveData.SetFaceTo(m_vBornFace);

	NET_SIS_synchronize_stand send;
	send.dw_size	= sizeof(NET_SIS_synchronize_stand);
	send.dw_role_id	= GetID();
	send.curPos	= GetCurPos();
	send.faceTo	= GetFaceTo();
	get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
}
//---------------------------------------------------------------------------------
// 初始化基本属性
//---------------------------------------------------------------------------------
BOOL Creature::InitAtt(const tagCreatureProto* pProto)
{
	// 设置包裹盒
	m_Size = pProto->vSize;

	// 从静态属性中拷贝出怪物基本属性
	get_fast_code()->memory_copy(m_nBaseAtt, pProto->nBaseAtt, sizeof(m_nBaseAtt));

	m_nLevel = pProto->nLevel;
	m_nCurLevelExp = 0;

	// 根据怪物的普通攻击id来生成技能

	for (int i = 0; i < NORAML_SKILL_NUMBER; i++)
	{
		const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(pProto->dwNormalSkill[i]);
		if ( VALID_POINT(pSkillProto))
		{
			DWORD dwSkillID = Skill::GetIDFromTypeID(pSkillProto->dwID);
			DWORD nLevel = Skill::GetLevelFromTypeID(pSkillProto->dwID);
			
			if (m_mapSkill.is_exist(dwSkillID))
				continue;

			Skill* pSkill = new Skill(dwSkillID, nLevel, 0, 0, 0);
			ASSERT(VALID_POINT(pSkill));
			m_pNormalSkill.push_back(pSkill);
			m_mapSkill.add(pSkill->GetID(), pSkill);
		}
	}
	//const tagSkillProto* pMeleeSkillProto = AttRes::GetInstance()->GetSkillProto(pProto->dwNormalSkillIDMelee);
	//if( VALID_POINT(pMeleeSkillProto) )
	//{
	//	DWORD dwSkillID = Skill::GetIDFromTypeID(pMeleeSkillProto->dwID);
	//	DWORD nLevel = Skill::GetLevelFromTypeID(pMeleeSkillProto->dwID);

	//	m_pMeleeSkill = new Skill(dwSkillID, nLevel, 0, 0, 0);
	//	ASSERT(VALID_POINT(m_pMeleeSkill));
	//	m_mapSkill.add(m_pMeleeSkill->GetID(), m_pMeleeSkill);
	//}
	//else
	//{
	//	m_pMeleeSkill = NULL;
	//}

	//const tagSkillProto* pRangedSkillProto = AttRes::GetInstance()->GetSkillProto(pProto->dwNormalSkillIDRanged);
	//if( VALID_POINT(pRangedSkillProto) )
	//{
	//	DWORD dwSkillID = Skill::GetIDFromTypeID(pRangedSkillProto->dwID);
	//	DWORD nLevel = Skill::GetLevelFromTypeID(pRangedSkillProto->dwID);

	//	m_pRangedSkill = new Skill(dwSkillID, nLevel, 0, 0, 0);

	//	m_mapSkill.add(m_pRangedSkill->GetID(), m_pRangedSkill);
	//}
	//else
	//{
	//	m_pRangedSkill = NULL;
	//}

	return TRUE;
}

//---------------------------------------------------------------------------------
// 初始化怪物AI
//---------------------------------------------------------------------------------
BOOL Creature::InitAI(const tagCreatureProto* pProto, const tag_map_way_point_info_list*	patrolList)
{
	// 生成状态机
	if( pProto->eAICreateType != EACT_Null )
	{
		m_pAIController = new AIController(this, patrolList);
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
// 复活的时候刷新属性
//---------------------------------------------------------------------------------
VOID Creature::RefreshAtt()
{
	// 移动清空
	m_MoveData.Reset(m_vBornPos.x, m_vBornPos.y, m_vBornPos.z, m_vBornFace.x, m_vBornFace.y, m_vBornFace.z);

	// 速度清空
	m_fXZSpeed = X_DEF_XZ_SPEED;
	m_fYSpeed = X_DEF_Y_SPEED;

	// 包裹盒清空
	m_Size = m_pProto->vSize;

	// 清空之前的状态
	ClearState();

	// 某些怪物属性
	m_bTagged = FALSE;
	m_dwTaggedOwner = INVALID_VALUE;
	m_nRespawnTickCountDown = 0;
	m_nLiveTick = 0;

	// 属性调整
	ZeroMemory(m_nAttMod, sizeof(m_nAttMod));
	ZeroMemory(m_nAttModPct, sizeof(m_nAttModPct));
	ZeroMemory(m_nAtt, sizeof(m_nAtt));
	get_fast_code()->memory_copy(m_nBaseAtt, m_pProto->nBaseAtt, sizeof(m_nBaseAtt));

	// 技能清空
	package_map<DWORD, Skill*>::map_iter it = m_mapSkill.begin();
	Skill* pSkill = NULL;
	while( m_mapSkill.find_next(it, pSkill) )
	{
		// 初始技能不清空
		/*if( pSkill != m_pMeleeSkill && pSkill != m_pRangedSkill )
		{
			RemoveSkill(pSkill->GetID());
		}*/

		BOOL bDel = TRUE;
		for (int i = 0; i < m_pNormalSkill.size(); i++)
		{
			if (pSkill == m_pNormalSkill[i])
			{
				bDel = FALSE;
				break;
			}
		}
		if (bDel)
		{
			RemoveSkill(pSkill->GetID());
		}
	/*	if( pSkill != m_pNormalSkill[0] && pSkill != m_pNormalSkill[1] && pSkill != m_pNormalSkill[2])
		{
			RemoveSkill(pSkill->GetID());
		}*/
	}
}

//---------------------------------------------------------------------------------
// 复活的时候刷新AI
//---------------------------------------------------------------------------------
VOID Creature::RefreshAI()
{
	// 设置状态机
	if( VALID_POINT(m_pAIController) )
	{
		m_pAIController->Refresh();
	}
}


//---------------------------------------------------------------------------------
// 计算怪物当前基本属性
//---------------------------------------------------------------------------------
VOID Creature::CalInitAtt()
{
	for(INT n = 0; n < ERA_End; n++)
	{
		m_nAtt[n] = m_nBaseAtt[n] + m_nAttMod[n] + (INT)(m_nBaseAtt[n] * (FLOAT)m_nAttModPct[n] / 10000.0f);
		// todo：取上下限
	}

	// 将某些“当前属性”设置成最大值
	m_nAtt[ERA_HP] = m_nAtt[ERA_MaxHP];
	m_nAtt[ERA_MP] = m_nAtt[ERA_MaxMP];
	m_nAtt[ERA_Brotherhood] = m_nAtt[ERA_MaxBrotherhood];
	//m_nAtt[ERA_Vitality] = m_nAtt[ERA_MaxVitality];
	//m_nAtt[ERA_Endurance] = m_nAtt[ERA_MaxEndurance];

	// 设置某些由基本属性影响的值
	m_fXZSpeed *= FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f;
	m_fYSpeed *= FLOAT(m_nAtt[ERA_Speed_Y]) / 10000.0f;
	m_Size *= FLOAT(m_nAtt[ERA_Shape]) / 10000.0f;
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID Creature::Destroy()
{
	SAFE_DELETE(m_pAIController);
}

//---------------------------------------------------------------------------------
// 更新函数
//---------------------------------------------------------------------------------
VOID Creature::Update()
{
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	UpdateState();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateState time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	GetMoveData().Update();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature GetMoveData().Update() time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateCombat();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateCombat time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateAI();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateAI time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateSkill();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateSkill time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateBuff();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateBuff time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	update_time_issue();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature update_time_issue time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateLiveTime();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateLiveTime time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	UpdateDoor();
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 30)
	{
		g_world.get_log()->write_log(_T("Creature UpdateDoor time %d\r\n\n\n"), dw_new_time - dw_time);
	}
}

//---------------------------------------------------------------------------------
// 更新技能
//---------------------------------------------------------------------------------
VOID Creature::UpdateCombat()
{
	// 当前技能还在作用
	if( m_CombatHandler.IsValid() )
	{
		m_CombatHandler.Update();

		if( !m_CombatHandler.IsValid() )
		{
			// 本技能释放完成，设置下次攻击的等待时间
			GetAI()->SetNextAttackWaitTick(m_pProto->nAttackInterTick);
		}
	}
}

//---------------------------------------------------------------------------------
// 更新门
//---------------------------------------------------------------------------------
VOID Creature::UpdateDoor()
{
	if(!IsDoor())
		return;
	if(m_pProto->nDoorCloseTime <= 0)
		return;

}

//---------------------------------------------------------------------------------
// 更新AI
//---------------------------------------------------------------------------------
VOID Creature::UpdateAI()
{
	// 根据怪物目前处在哪一个AI状态决定如何进行更新
	if( VALID_POINT(m_pAIController) )
		m_pAIController->Update();
}

//---------------------------------------------------------------------------------
// 更新生存时间
//---------------------------------------------------------------------------------
VOID Creature::UpdateLiveTime()
{
	if(0 != m_pProto->nLiveTick)
	{
		++m_nLiveTick;
		if(m_nLiveTick >= m_pProto->nLiveTick /*|| (IsGuildManRes() && IsAotoReceive())*/)
		{
			//// 消失时处理种植数据
			//tagPlantData* pPlant = GetPlantData();
			//if (VALID_POINT(pPlant))
			//{
			//	Creature* pMount = (Creature*)get_map()->find_unit(GetTuDui());
			//	if( VALID_POINT(pMount ) )
			//	{
			//		pMount->SetUsed(false);	
			//		
			//	}
			//	package_map<DWORD, int> mapPlantItem;
			//	g_drop_mgr.get_plant_drop_list(this, pPlant->n_num, mapPlantItem);
	

			//	if (mapPlantItem.size() > 0)
			//	{
			//		tagMailBase st_mail_base;
			//		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			//		st_mail_base.dwSendRoleID = INVALID_VALUE;
			//		st_mail_base.dwRecvRoleID = pPlant->dw_master_id;

			//		TCHAR szPetMailContent[X_SHORT_NAME] = _T("111");

			//		//_stprintf(szPetMailContent, _T("%d_%s_%d"), PET_JIEJING, szName, 40);
			//		
			//		int nMailNum = (mapPlantItem.size() + Max_Item_Num -1 ) / Max_Item_Num ;
			//						
	
			//		mapPlantItem.reset_iterator();
			//		for (int i = 0; i < nMailNum; i++ )
			//		{
			//			DWORD dwItemType[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
			//			INT16 dwQlty[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
			//			INT	  n_num[Max_Item_Num] = {1, 1, 1};

			//			for (int j = 0; j < Max_Item_Num; j++)
			//			{
			//				int n = i * Max_Item_Num + j;
			//				if (n >= mapPlantItem.size())
			//					break;

			//				mapPlantItem.find_next(dwItemType[j], n_num[j]);

			//	
			//			}
		

			//			g_mailmgr.CreateMail(st_mail_base, _T("&plantItem&"), szPetMailContent, dwItemType, dwQlty, n_num, Max_Item_Num, FALSE);
			//		}

			//		
			//	}

			//	ResetPlantData();
			//}
			OnDisappear();
		}
	}
}

//---------------------------------------------------------------------------------
// 检测玩家和NPC谈话的距离
//---------------------------------------------------------------------------------
BOOL Creature::CheckNPCTalkDistance(Role* pRole)
{
	if( !VALID_POINT(pRole) ) return FALSE;
	return IsInDistance(*pRole, MAX_NPC_TALK_DISTANCE);
}

//---------------------------------------------------------------------------------
// 被攻击
//---------------------------------------------------------------------------------
VOID Creature::OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited)
{
	ASSERT( VALID_POINT(pSkill) && VALID_POINT(pSrc) );
	
	// 判断技能的敌我目标类型
	if( !pSkill->IsHostile() && !pSkill->IsFriendly() ) return;
	DWORD dwFriendEnemyFlag = pSrc->FriendEnemy(this);

	if( pSkill->IsHostile() && (ETFE_Hostile & dwFriendEnemyFlag) )
	{
		if( pSrc->IsRole() )
		{
			// 如果怪物之前无所属，则变为所属
			if( ECTT_Hit == GetTaggedType() && !IsTagged() )
			{
				SetTagged(pSrc->GetID());
			}
		}
		
		// 计算仇恨，怪物的AI触发(放到伤害处处理)
		/*if( VALID_POINT(m_pAIController) )
		{
			INT	nValue = pSkill->GetEnmity() + pSrc->GetAttValue(ERA_Enmity_Degree);
			m_pAIController->AddEnmity(pSrc, nValue);
			m_pAIController->OnEvent(pSrc, ETEE_BeAttacked);
		}*/

		if( bHited )
		{
			OnBuffTrigger(pSrc, ETEE_Hited, 0, 0);
			if( bBlock )
			{
				OnBuffTrigger(pSrc, ETEE_Block, 0, 0);
			}
			if( bCrited )
			{
				OnBuffTrigger(pSrc, ETEE_Crited, 0, 0);
			}

			// 检测当前技能的打断
			GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Skill, ESSTE_Default == pSkill->GetTypeEx());
			// 检测某些被攻击打断的buff
			OnInterruptBuffEvent(EBIF_BeAttacked);
		}
		else
		{
			OnBuffTrigger(pSrc, ETEE_Dodge, 0, 0);
		}
		OnBuffTrigger(pSrc, ETEE_BeAttacked, 0, 0);
	}
}

VOID Creature::OnBuffInjury(Unit* pSrc, INT dmg)
{
	if( !VALID_POINT(pSrc) )	return;

	if( pSrc->IsRole() )
	{
		// 如果怪物之前无所属，则变为所属
		if( ECTT_Hit == GetTaggedType() && !IsTagged() )
		{
			SetTagged(pSrc->GetID());
		}
	}

	// 计算仇恨，怪物的AI触发
	if( VALID_POINT(m_pAIController) )
	{
		m_pAIController->AddEnmity(pSrc, abs(dmg));
		m_pAIController->OnEvent(pSrc, ETEE_BeAttacked);
	}
}

VOID Creature::OnAIAttack()
{
	DWORD dwSkillID = m_CombatHandler.GetSkillID();

	// 如果是普通攻击，触发怪物触发器攻击事件

	for (int i = 0; i < m_pNormalSkill.size(); i++)
	{
		if( VALID_POINT(m_pNormalSkill[i]) && dwSkillID  == m_pNormalSkill[i]->GetID() )
		{
			if(VALID_POINT(m_pAIController))
			{
				AITriggerMgr* pTrggerMgr = GetAI()->GetAITriggerMgr();
				if(VALID_POINT(pTrggerMgr))
				{
					pTrggerMgr->OnEvent(ETEE_Attack); 
					return;
				}
			}
		}
	}

	//if( VALID_POINT(m_pMeleeSkill) && dwSkillID  == m_pMeleeSkill->GetID() )
	//{
	//	if(VALID_POINT(m_pAIController))
	//	{
	//		AITriggerMgr* pTrggerMgr = GetAI()->GetAITriggerMgr();
	//		if(VALID_POINT(pTrggerMgr))
	//		{
	//			pTrggerMgr->OnEvent(ETEE_Attack); 
	//			return;
	//		}
	//	}
	//}

	//if(VALID_POINT(m_pRangedSkill) && dwSkillID == m_pRangedSkill->GetID())
	//{
	//	if(VALID_POINT(GetAI()))
	//	{
	//		AITriggerMgr* pTrggerMgr = GetAI()->GetAITriggerMgr();
	//		if(VALID_POINT(pTrggerMgr))
	//		{
	//			pTrggerMgr->OnEvent(ETEE_Attack); 
	//			return;
	//		}
	//	}
	//}

}

//---------------------------------------------------------------------------------
// 怪物死亡
//---------------------------------------------------------------------------------
VOID Creature::OnDead(Unit* pSrc, Skill* pSkill, const tagBuffProto* pBuff, DWORD dwSerial, DWORD dwMisc, BOOL bCrit)
{
	ASSERT(!IsPet());
	
	//if (IsPet())
	//{
	//	DWORD dwSkillID = INVALID_VALUE;
	//	if (VALID_POINT(pSkill))
	//	{
	//		dwSkillID = pSkill->GetProto()->dwID;
	//	}
	//	DWORD dwBuffID = INVALID_VALUE;
	//	if (VALID_POINT(pBuff))
	//	{
	//		dwBuffID = pBuff->dwID;
	//	}
	//	g_world.get_log()->write_log(_T("Pet Dead!! Skill:%d Buff:%d\r\n"), dwSkillID, dwBuffID);

	//	if (VALID_POINT(pSrc) && pSrc->IsRole())
	//	{
	//		g_world.get_log()->write_log(_T("Pet Dead src is role\r\n"));
	//	}
	//	
	//	if (VALID_POINT(pSrc) && pSrc->IsCreature())
	//	{
	//		DWORD dwCreaturID = ((Creature*)pSrc)->m_pProto->dw_data_id;
	//		g_world.get_log()->write_log(_T("Pet Dead src is creature :%d\r\n"), dwCreaturID);
	//	}

	//	return;
	//}

	if (pSrc->IsCreature())
	{
		Role* pMarster = ((Creature*)pSrc)->GetMarster();
		if (VALID_POINT(pMarster))
		{
			pSrc = pMarster;
		}
	}

	if (!m_pProto->bCanKill)
	{
		SetAttValue(ERA_HP, 1);
		return;
	}
	
	// 好大头的log
	//if (m_pProto->dw_data_id == 3020023)
	//{
	//	DWORD dwSkillID = INVALID_VALUE;
	//	if (VALID_POINT(pSkill))
	//	{
	//		dwSkillID = pSkill->GetProto()->dwID;
	//	}
	//	DWORD dwBuffID = INVALID_VALUE;
	//	if (VALID_POINT(pBuff))
	//	{
	//		dwBuffID = pBuff->dwID;
	//	}
	//	g_world.get_log()->write_log(_T("Creature 3020023 Dead!! Skill:%d Buff:%d\r\n"), dwSkillID, dwBuffID);
	//}

	// 如果本身已经死亡，则直接返回
	if( IsInState(ES_Dead) ) return;

	// 触发器
	OnBuffTrigger(pSrc, ETEE_Die);

	//触发脚本
	if (VALID_POINT(m_pScript))
	{
		m_pScript->OnDie(this, pSrc, m_dwTaggedOwner);
	}

	// 设置为死亡状态
	SetState(ES_Dead);

	// 停止移动
	m_MoveData.StopMoveForce();

	// 停止当前的技能
	m_CombatHandler.End();

	// 怪物死亡时AI触发

	// 移除身上所有的buff
	RemoveAllBuff();

	// 设置副本权值
	SetInstLimit(pSrc, GetProto()->nInstPoint);

	// 设置副本进度
	SetInstProcess();

	// 保存副本进度
	if(GetProto()->b_save_process)
	{
		SaveRoleProcess();
	}
	
	// 清空仇恨列表
	if( VALID_POINT(GetAI()) )
		GetAI()->ClearAllEnmity();

	ClearEnmityCreature();
	SetTargetID(INVALID_VALUE);

	// 重置跟随相关
	Role* pTarget = GetAI()->GetTracker()->GetTarget();
	if ( VALID_POINT(pTarget))
	{
		pTarget->SetFlowUnit(INVALID_VALUE);
	}

	BOOL bAddYiQi = FALSE;
	// 找到奖励授予者
	Role* pReward = FindRewardRole(pSrc);
	if( VALID_POINT(pReward) )
	{
		// 计算经验奖励
		BOOL bKill = FALSE;
		if( VALID_POINT(pSkill) )
			bKill = TRUE;

		if(pReward->GetTeamID() ==INVALID_VALUE)
		{
			std::vector<Role*>	vecRole;
			bAddYiQi = ExpReward(pReward,vecRole,bKill);

			// 有经验才加义气
			if(bAddYiQi)
			{
				float fEarnRate	= pReward->GetEarnRate() / 10000.0f;
				pReward->ChangeBrotherhood(AttRes::GetInstance()->GetVariableLen().nBrother * fEarnRate);
			}

			// 计算任务杀怪计数
			pReward->on_quest_event(EQE_Kill, GetTypeID(), get_level(), bAddYiQi);
		}
		else
		{
			const Team *pTeam = g_groupMgr.GetTeamPtr(pReward->GetTeamID());
			std::vector<Role*>	vecRole;
			INT   nShareRole = CalExpShareRole(pTeam, vecRole);

			std::vector<Role*>::iterator it = vecRole.begin();
			while (it != vecRole.end())
			{
				Role* pMember = *it;
				++it;

				if( !VALID_POINT(pMember) )
					continue;
				bool bSelf = false;
				//技能点只给杀死怪的那个，gx modify 2013.8.30
				if (pReward->GetID() == pMember->GetID())
				{
					bSelf = true;
				}
				else
				{
					bSelf = false;
				}
				BOOL bAdd = ExpReward(pMember,vecRole,bKill, pTeam, nShareRole,bSelf);
				//end
				if(bAdd)
				{
					bAddYiQi = TRUE;
				}

				// 计算任务杀怪计数
				pMember->on_quest_event(EQE_Kill, GetTypeID(), get_level(), bAdd);
			}

			// 有一个人有经验才加义气
			if(bAddYiQi)
			{
				std::vector<Role*>::iterator it = vecRole.begin();
				while (it != vecRole.end())
				{
					Role* pMember = *it;
					++it;

					if( !VALID_POINT(pMember) )
						continue;
				
					if (pMember->IsDead())
						continue;

					float fEarnRate	= pMember->GetEarnRate() / 10000.0f;
					pMember->ChangeBrotherhood(AttRes::GetInstance()->GetVariableLen().nBrother * fEarnRate);
				}
			}
		}
		
		// 计算掉落奖励
		g_drop_mgr.monster_drop(this, pReward);
	}

	// 击杀者
	if( VALID_POINT(pSrc) )
	{
		pSrc->OnKill(this);
	}

	// 地图事件
	get_map()->on_creature_die(this, pSrc);

	// 发送怪物死亡消息
	NET_SIS_role_dead send;
	send.dw_role_id = GetID();
	send.dwSrcRoleID = (VALID_POINT(pSrc) ? pSrc->GetID() : INVALID_VALUE);
	send.dwSerial = dwSerial;
	send.bCrit = bCrit;
	send.bHitFly = FALSE;
	if( VALID_POINT(pSkill) )
	{
		send.eCause = ERDC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		send.dwMisc2 = dwMisc;
		if (m_pProto->bCanHitFly)
		{
			send.bHitFly = (get_tool()->tool_rand() % 10000) < pSkill->GetProto()->nHitFlyPro ? TRUE: FALSE;
		}
	}
	else if( VALID_POINT(pBuff) )
	{
		send.eCause = ERDC_Buff;
		send.dwMisc = pBuff->dwID;
		send.dwMisc2 = dwMisc;
	}
	else
	{
		send.eCause = ERDC_Other;
		send.dwMisc = INVALID_VALUE;
		send.dwMisc2 = dwMisc;
	}

	get_map()->send_big_visible_tile_message(this, &send, send.dw_size);

	// 设置死亡复活时间
	m_nRespawnTickCountDown = GetRespawnTick();

	// 从地图中拿掉该生物
	get_map()->remove_creature(this);
}

//---------------------------------------------------------------------------------
// 设置副本进度
//---------------------------------------------------------------------------------
VOID Creature::SetInstProcess()
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return;

	if(pMap->get_map_type() != EMT_Instance)
		return;

	map_instance_normal* p_inst = (map_instance_normal*)pMap;

	if(GetSpawnPtID() != INVALID_VALUE && ECST_Load == GetSpawnType())
	{
		DWORD	dw_spawnpt_id = GetSpawnPtID();
		p_inst->get_list_creature_pro().push_back(dw_spawnpt_id);
	}
}

//---------------------------------------------------------------------------------
// 通知保存副本进度
//---------------------------------------------------------------------------------
VOID Creature::SaveRoleProcess()
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return;

	if(pMap->get_map_type() != EMT_Instance)
		return;

	map_instance_normal* p_inst = (map_instance_normal*)pMap;

	package_map<DWORD, Role*>::map_iter iter = p_inst->get_role_map().begin();
	Role* pRole = NULL;
	while(p_inst->get_role_map().find_next(iter, pRole))
	{
		if(!VALID_POINT(pRole))
			continue;

		// 添加副本进度
		p_inst->add_inst_process_to_role(pRole);
	}
}

//---------------------------------------------------------------------------------
// 设置副本权值
//---------------------------------------------------------------------------------
VOID Creature::SetInstLimit(Unit* pSrc, INT nPoint)
{
	if(!VALID_POINT(pSrc))
		return ;

	if(pSrc->IsRole())
	{
		Role* pRole = (Role*)pSrc;
		if(VALID_POINT(pRole))
		{
			if(nPoint > 0)
			{
				Map* pMap = pRole->get_map();
				if(!VALID_POINT(pMap))
					return;

				package_map<DWORD, Role*>::map_iter it = pMap->get_role_map().begin();
				Role* pMapRole = NULL;

				while(pMap->get_role_map().find_next(it, pMapRole))
				{
					if(!VALID_POINT(pMapRole))
						continue;

					s_enter_map_limit* pEnterMapLimit = pMapRole->GetMapLimitMap().find(GetMapID());
					if(VALID_POINT(pEnterMapLimit))
					{
						pEnterMapLimit->dw_enter_num_ += nPoint;
					}
				}
				/*if(pRole->GetTeamID() != INVALID_VALUE)
				{
					const Team* pTeam = g_groupMgr.GetTeamPtr(pRole->GetTeamID());
					if(VALID_POINT(pTeam))
					{
						for(INT i = 0; i < pTeam->get_member_number(); i++)
						{
							Role* pMember = pTeam->get_member(i);
							if(VALID_POINT(pMember))
							{
								if(pMember->GetMapID() == GetMapID())
								{
									s_enter_map_limit* pEnterMapLimit = pMember->GetMapLimitMap().find(GetMapID());
									if(VALID_POINT(pEnterMapLimit))
									{
										pEnterMapLimit->dw_enter_num_ += nPoint;
									}
								}
							}
						}
					}
				}
				else
				{
					s_enter_map_limit* pEnterMapLimit = pRole->GetMapLimitMap().find(GetMapID());
					if(VALID_POINT(pEnterMapLimit))
					{
						pEnterMapLimit->dw_enter_num_ += nPoint;
					}
				}*/
			}
		}
	}
}

//---------------------------------------------------------------------------------
// 怪物消失
//---------------------------------------------------------------------------------
VOID Creature::OnDisappear()
{
	//if (IsPet())
	//{
	//	g_world.get_log()->write_log(_T("Pet OnDisappeard!!\r\n"));
	//	return;
	//}

	// 如果本身已经死亡，则直接返回
	if( IsInState(ES_Dead) ) return;

	// 设置为死亡状态
	SetState(ES_Dead);

	// 停止移动
	m_MoveData.StopMoveForce();

	// 停止当前的技能
	m_CombatHandler.End();

	// 怪物死亡时AI触发

	// 移除身上所有的buff
	RemoveAllBuff();

	// 清空仇恨列表
	if( VALID_POINT(GetAI()) )
		GetAI()->ClearAllEnmity();

	// 地图事件
	get_map()->on_creature_disappear(this);

	// 设置死亡复活时间
	m_nRespawnTickCountDown = GetRespawnTick();

	// 从地图中拿掉该生物
	get_map()->remove_creature(this);

	// 同步给客户端
	get_map()->synchronize_remove_unit_to_big_visible_tile(this);
}

//----------------------------------------------------------------------------------
// 生物击杀
//----------------------------------------------------------------------------------
VOID Creature::OnKill(Unit* pTarget)
{
	OnBuffTrigger(pTarget, ETEE_Kill);

	// 成就
	if (pTarget->IsRole())
	{
		((Role*)pTarget)->GetAchievementMgr().UpdateAchievementCriteria(eta_be_kill_monster, m_pProto->dw_data_id, 1);
	}

	Role* pMaster = GetMarster();
	if (VALID_POINT(pMaster))
	{
		pMaster->OnKill(pTarget);
	}
}

//----------------------------------------------------------------------------------
// 尝试复活
//----------------------------------------------------------------------------------
ECreatureReviveResult Creature::TryRevive()
{
	// 如果怪物是动态生成的，则需要删除内存了
	if( ECST_Dynamic == m_eSpawnedType || ECST_Guild == m_eSpawnedType) return ECRR_NeedDestroy;

	// 如果是帮会兵工厂生成
	if(ECST_Maidan == m_eSpawnedType)
	{
		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
		if(!VALID_POINT(pGuild))
			return ECRR_Failed;

		if(pGuild->get_upgrade().IsFacilitesDestory(EFT_Maidan))
			return ECRR_NeedDestroy;
	}

	//如果是帮会守卫生成
	if(ECST_GuildSentinel == m_eSpawnedType)
	{
		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
		if(!VALID_POINT(pGuild))
			return ECRR_Failed;

		if(pGuild->get_upgrade().IsFacilitesDestory((EFacilitiesType)(GetProto()->eGuildType - EGT_Holiness)))
			return ECRR_NeedDestroy;
	}
	
	//if (!IsUsed())
	//{
		// 复活倒计时减少一个Tick
		m_nRespawnTickCountDown--;
	//}
	
	// 如果复活时间没到，则返回失败
	if( m_nRespawnTickCountDown > 0 ) return ECRR_Failed;

	// 如果是刷新点怪，则需要产生替代品
	if ( ECST_SpawnPoint == m_eSpawnedType ) return ECRR_NeedReplace;
	
	// 刷拐点组刷新的, 则需要重新生成位置
	if ( ECST_SpawnList == m_eSpawnedType) return ECRR_NeedRepos;

	// 如果是巢穴的怪物
	if( ECST_Nest == m_eSpawnedType )
	{
		// 找到刷出的巢穴是否还存在，如果不存在了，说明巢穴还没刷新，那么自己也不刷新
		Creature* pNestCreature = get_map()->find_creature(m_dwSpawnerID);
		if( !VALID_POINT(pNestCreature) ) return ECRR_Failed;

		// 如果找到，则找到该巢穴的的巢穴属性
		const tagCreatureProto* pProto = pNestCreature->GetProto();
		if( !VALID_POINT(pProto) || !VALID_POINT(pProto->pNest) ) return ECRR_Failed;

		const tagNestProto* pNest = pProto->pNest;

		// 找到一个可行走的随机点
		Vector3 vNewBornPos;

		vNewBornPos.x = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + pNestCreature->GetCurPos().x;
		vNewBornPos.z = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + pNestCreature->GetCurPos().z;

		if( get_map()->if_can_go(vNewBornPos.x, vNewBornPos.z) )
		{
			vNewBornPos.y = 0;
		}
		else
		{
			// 没有找到一个复活点，本Tick不复活
			return ECRR_Failed;
		}

		// 随机一个朝向
		Vector3 vNewBornFaceTo;
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		vNewBornFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vNewBornFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vNewBornFaceTo.y = 0.0f;


		// 让生物的出生坐标和出生朝向等于这个新值
		m_vBornPos = vNewBornPos;
		m_vBornFace = vNewBornFaceTo;
	}

	// 检测通过，怪物可以复活了
	OnRevive();

	return ECRR_Success;
}

//---------------------------------------------------------------------------------
// 生物复活
//---------------------------------------------------------------------------------
VOID Creature::OnRevive()
{
	// 刷新属性
	RefreshAtt();
	// 刷新AI
	RefreshAI();
	// 重新计算属性
	CalInitAtt();

	if(VALID_POINT(m_pScript))
	{
		m_pScript->OnRespawn(this);
	}
}

//---------------------------------------------------------------------------------
// 查找到奖励授予者（参数中为击杀者）
//---------------------------------------------------------------------------------
Role* Creature::FindRewardRole(Unit* pSrc)
{
	Role* pReward = NULL;
	if( ECTT_Hit == m_pProto->eTaggedType )
	{
		pReward = get_map()->find_role(m_dwTaggedOwner);
	}
	else
	{
		if( VALID_POINT(pSrc) && pSrc->IsRole() )
			pReward = static_cast<Role*>(pSrc);
	}

	return pReward;
}

//---------------------------------------------------------------------------------
// 经验奖励
//---------------------------------------------------------------------------------
//gx add team_vec用于夫妻组队中
//夫妻双方组队打怪各自活动105%的经验加成
BOOL Creature::ExpReward(Role* pReward, vector<Role*> &team_vec,BOOL bKill, const Team *pTeam, INT nShareNum,bool bSelf)
{
	if( !VALID_POINT(pReward) ) return FALSE;
	
	if (pReward->IsDead()) return FALSE;

	INT nExp  = 0;

	// 防沉迷收益率
	float fEarnRate		= pReward->GetEarnRate() / 10000.0f;
	FLOAT fExpRate	= get_map()->get_exp_rate();
	FLOAT fVNBExpRate = pReward->GetVNBExpRate();

	// 基础经验
	FLOAT fBaseExp = (FLOAT)m_pProto->nExpGive;
	// 基础战功
	FLOAT fBaseExploit = (FLOAT)m_pProto->nExploits;
	
	INT nBaseExp = 0;
	
	// 组队相关的经验奖励
	if(VALID_POINT(pTeam))
	{
		// 组队经验加成系数
		FLOAT fCoe1 = 1.0f;
		if(nShareNum > 1)
		{
			fCoe1 = ((FLOAT)nShareNum/5) + 1.0f;
		}
		// 组队经验分配系数
		FLOAT fCoe2 = 1.0f;
		if(nShareNum > 1)
		{
			fCoe2 = 1.0f / nShareNum;
		}
		nBaseExp = (INT)(fBaseExp * fCoe1 * CalLevelCoefficient(pReward->get_level(), get_level()));
		nExp = (INT)(nBaseExp * fCoe2 * fExpRate);//mwh 2011-07-29 服务器经验加倍
		nExp = (INT)(nExp * (1+(FLOAT)pReward->GetAttValue(ERA_Exp_Add_Rate)/10000));
		//nExp = (INT)(nExp * pReward->CalFriendExpPer(pTeam));
		nExp = (INT)(nExp * fEarnRate);//防沉迷影响

		//gx add 2013.10.24
		FLOAT fCoe3 = 1.0f;//夫妻组队经验加成系数
		if (pReward->GetSpouseID() != (DWORD)INVALID_VALUE)//该玩家有配偶
		{
			INT vec_Num = team_vec.size();
			for (INT i = 0;i < nShareNum;i++)
			{
				if (i >= vec_Num)//防止异常
					break;
				Role* pRoleTemp = team_vec[i];
				if (!pRoleTemp)
					continue;
				if (pReward->GetSpouseID() == pRoleTemp->GetID())
				{
					fCoe3 = 1.05f;
					break;
				}
			}
		}
		nExp = (INT)(nExp * fCoe3);
		//end

		NET_SIS_role_exp_share_num send;
		send.nShareNum = nShareNum;
		pReward->SendMessage(&send, send.dw_size);
	}
	else
	{
		// mwh 2011-07-29 经验加倍
		nExp = (INT)(fBaseExp * CalLevelCoefficient(pReward->get_level(), get_level()) * fExpRate);
		nExp = (INT)(nExp * (1+(FLOAT)pReward->GetAttValue(ERA_Exp_Add_Rate)/10000));
		nExp = (INT)(nExp * fEarnRate);//防沉迷影响
	}
	
	//pReward->GetCurMgr().IncExploits(fBaseExploit, elcid_pickup_money);
	//技能点只给杀死怪物的那个玩家
	if (bSelf)
	{
		// 大于怪物15级没有技能点
		if (pReward->get_level() <= get_level() + 15)
		{
			pReward->ModAttValue(ERA_TalentPoint, fBaseExploit);
		}
	}
	
	
	pReward->ExpChange(nExp, bKill);

	// 验证码判断
	if (VALID_POINT(pReward->GetSession()))
	{
		pReward->GetSession()->GetVerification().beginVerification(pReward, TRUE);
	}
	
	if(nExp > 0)
		return TRUE;

	return FALSE;
	
}

//--------------------------------------------------------------------------
// 计算经验等级系数
//--------------------------------------------------------------------------
FLOAT Creature::CalLevelCoefficient(INT nRoleLevel, INT nCreatureLevel)
{
	INT nLevelDiff = abs(nRoleLevel - nCreatureLevel);

	// 玩家与怪等级差异过大时
	if(nLevelDiff >= 30)
	{
		return 0.0f;
	}

	if(nRoleLevel > nCreatureLevel)
	{
		return 1.0f - 2 * (nRoleLevel - nCreatureLevel) / 100.0f;
	}

	return 1.0f;
}

//--------------------------------------------------------------------------
// 组队时,玩家是否在经验分配的有效范围
//--------------------------------------------------------------------------
BOOL Creature::IsExpShareDistance(Role* pReward)
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = pReward->GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	if(get_map() == pReward->get_map() &&fDistSq <= X_DEF_PICKUP_DIST_SQ)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
// 组队时,玩家是否在掉落分配的有效范围
//--------------------------------------------------------------------------
BOOL Creature::IsLootShareDistance(Role* pReward)
{
	const Vector3& vSrc = GetCurPos();
	const Vector3& vDest = pReward->GetCurPos();

	FLOAT fDistSq = Vec3DistSq(vSrc, vDest);

	if(GetMapID() == pReward->GetMapID() && fDistSq <= X_DEF_PICKUP_DIST_SQ)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
// 计算组队分享经验的小队玩家(返回玩家个数）
//--------------------------------------------------------------------------
INT Creature::CalExpShareRole(const Team *pTeam, std::vector<Role*>  &vecRole)
{
	INT		nShareNum = 0;
	Role	*pReward = (Role*)INVALID_VALUE;
	if(VALID_POINT(pTeam))
	{
		INT nTeamNum = pTeam->get_member_number();

		for(INT i = 0; i < nTeamNum; ++i)
		{
			pReward = pTeam->get_member(i);

			if(!VALID_POINT(pReward) || !IsExpShareDistance(pReward))
				continue;
			
			++nShareNum;
			vecRole.push_back(pReward);
		}
	}

	return nShareNum;
}

//--------------------------------------------------------------------------
// 目标标示类型判断
//--------------------------------------------------------------------------
DWORD Creature::TargetTypeFlag(Unit* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;

	if( pTarget->IsRole() )
		return TargetTypeFlag(static_cast<Role*>(pTarget));
	else if( pTarget->IsCreature() )
		return TargetTypeFlag(static_cast<Creature*>(pTarget));
	else
		return 0;
}

DWORD Creature::TargetTypeFlag(Role* pTarget)
{
	ASSERT( VALID_POINT(pTarget) );

	return ETF_Player;
}
DWORD Creature::TargetTypeFlag(Creature* pTarget)
{
	if( !VALID_POINT(pTarget) )
		return 0;

	if( this == pTarget )
		return ETF_Self;

	DWORD dwFlag = 0;

	// 怪物
	if( pTarget->IsMonster() )
	{
		// 普通怪物
		if( pTarget->IsNormalMonster() )
		{
			if( !pTarget->IsBoss())
				dwFlag |= ETF_NormalMonster;
			else
				dwFlag |= ETF_Boss;
		}

		// 巢穴
		else if( pTarget->IsNest())
		{
			dwFlag |= ETF_Nest;
		}
	}

	// NPC
	else if( pTarget->IsNPC() )
	{
		dwFlag |= ETF_NPC;
	}

	// 宠物
	else if( pTarget->IsPet() )
	{
		dwFlag |= ETF_Pet;
	}

	// 资源
	else if( pTarget->IsRes() )
	{
		if ( pTarget->IsNatuRes() || pTarget->IsGuildNatuRes())
			dwFlag |= ETF_NatuRes;
		else if ( pTarget->IsManRes() || pTarget->IsGuildManRes())
			dwFlag |= ETF_ManRes;
		else
			ASSERT(0);
	}

	// 可调查地物
	else if( pTarget->IsInves() || pTarget->IsInves_two())
	{
		dwFlag |= ETF_InvesGameObject;
	}

	// 门和建筑再看看放到哪个子类里去

	return dwFlag;
}

//---------------------------------------------------------------------------
// 与目标的敌我中立判断
//---------------------------------------------------------------------------
DWORD Creature::FriendEnemy(Unit* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;
	if( !VALID_POINT(get_map()) || get_map() != pTarget->get_map() ) return 0;

	// 自身的话，则均满足
	if( this == pTarget )
	{
		return ETFE_Friendly | ETFE_Hostile | ETFE_Independent;
	}

	// 取所在地图的敌我关系判定
	BOOL bIgnore = FALSE;
	DWORD dwMapFlag = get_map()->get_friend_enemy_flag(this, pTarget, bIgnore);

	// 如果还需要判断单体自身的，则判断自身
	if( !bIgnore )
	{
		DWORD dwSelfFlag = 0;
		if( pTarget->IsRole() )
			dwSelfFlag = FriendEnemy(static_cast<Role*>(pTarget));
		else if( pTarget->IsCreature() )
			dwSelfFlag = FriendEnemy(static_cast<Creature*>(pTarget));
		else
			dwSelfFlag = 0;

		return (dwMapFlag | dwSelfFlag);
	}
	else
	{
		return dwMapFlag;
	}
}

DWORD Creature::FriendEnemy(Role* pTarget)
{
	DWORD dwFlag = 0;

	// 如果自身是怪物
	if( IsMonster() )
	{
		// 宠物的敌我判断
		Role* pMarster = GetMarster();
		if (VALID_POINT(pMarster))
		{
			if(pTarget == pMarster)
				return ETFE_Friendly;
			else
				return pMarster->FriendEnemy(pTarget);
		}
		
		// 卫兵类型,打红名玩家
		if (GetProto()->eAIAction == AIAT_Hucheng)
		{
			if (pTarget->IsRedName())
			{
				dwFlag |= ETFE_Hostile;
			}
			else
			{
				dwFlag |= ETFE_Friendly;
			}
			return dwFlag;
		}

		if(m_dwGuildID != INVALID_VALUE)
		{
			if(m_dwGuildID == pTarget->GetGuildID())
			{
				dwFlag |= ETFE_Friendly;
			}
			else
			{
				// 阵营判定
				if(pTarget->get_temp_role_camp() != ECA_Null)
				{
					const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(GetProto()->eCamp);
					if(!VALID_POINT(pCreatureCamp))
						return ETFE_Hostile;

					if (ECA_Null == pTarget->get_temp_role_camp())
						return ETFE_Hostile;

					switch(pCreatureCamp->eCampConnection[pTarget->get_temp_role_camp()])
					{
					case ECAC_Null:
					case ECAC_Enemy:
						dwFlag |= ETFE_Hostile;
						break;
					case ECAC_Friend:
						dwFlag |= ETFE_Friendly;
						break;
					case ECAC_Neutralism:
						dwFlag = ETFE_Independent;
						break;
					}
				}
				else
				{
					const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(GetProto()->eCamp);
					if(!VALID_POINT(pCreatureCamp))
						return ETFE_Hostile;

					if (ECA_Null == pTarget->get_temp_role_camp())
						return ETFE_Hostile;

					switch(pCreatureCamp->eCampConnection[pTarget->get_role_camp()])
					{
					case ECAC_Null:
					case ECAC_Enemy:
						dwFlag |= ETFE_Hostile;
						break;
					case ECAC_Friend:
						dwFlag |= ETFE_Friendly;
						break;
					case ECAC_Neutralism:
						dwFlag = ETFE_Independent;
						break;
					}
				}
			}
		}
		else
		{
			// 阵营判定
			if(pTarget->get_temp_role_camp() != ECA_Null)
			{	
				const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(GetProto()->eCamp);
				if(!VALID_POINT(pCreatureCamp))
					return ETFE_Hostile;

				if (ECA_Null == pTarget->get_temp_role_camp())
					return ETFE_Hostile;

				switch(pCreatureCamp->eCampConnection[pTarget->get_temp_role_camp()])
				{
				case ECAC_Null:
				case ECAC_Enemy:
					dwFlag |= ETFE_Hostile;
					break;
				case ECAC_Friend:
					dwFlag |= ETFE_Friendly;
					break;
				case ECAC_Neutralism:
					dwFlag = ETFE_Independent;
					break;
				}
			}
			else
			{				
				const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(GetProto()->eCamp);
				if(!VALID_POINT(pCreatureCamp))
					return ETFE_Hostile;

				if (ECA_Null == pTarget->get_temp_role_camp())
					return ETFE_Hostile;

				switch(pCreatureCamp->eCampConnection[pTarget->get_role_camp()])
				{
				case ECAC_Null:
				case ECAC_Enemy:
					dwFlag |= ETFE_Hostile;
					break;
				case ECAC_Friend:
					dwFlag |= ETFE_Friendly;
					break;
				case ECAC_Neutralism:
					dwFlag = ETFE_Independent;
					break;
				}
			}
		}
		
	}

	// 如果自身是NPC
	else if( IsNPC() )
	{
		// 如果玩家是行凶状态，则为敌方
		if( pTarget->IsInRoleState(ERS_PK) )
		{
			dwFlag |= ETFE_Hostile;
		}
		// 否则为友方
		else
		{
			dwFlag |= ETFE_Friendly;
		}
	}

	// 如果自身是地物，则什么都不做
	else if( IsGameObject() )
	{

	}

	// 如果是宠物，则先不做
	else if( IsPet() )
	{

	}

	return dwFlag;
}

DWORD Creature::FriendEnemy(Creature* pTarget)
{
	DWORD dwFlag = 0;

	// 如果自身是怪物
	if( IsMonster() )
	{
		//if( pTarget->IsMonster() )
		//{
			const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(GetProto()->eCamp);
			if(!VALID_POINT(pCreatureCamp))
				return ETFE_Hostile;
			
			if (ECA_Null == pTarget->GetProto()->eCamp)
				return ETFE_Hostile;

			switch(pCreatureCamp->eCampConnection[pTarget->GetProto()->eCamp])
			{
			case ECAC_Null:
			case ECAC_Enemy:
				dwFlag |= ETFE_Hostile;
				break;
			case ECAC_Friend:
				dwFlag |= ETFE_Friendly;
				break;
			case ECAC_Neutralism:
				dwFlag = ETFE_Independent;
				break;
			}
		//}
		//else if( pTarget->IsNPC() )
		//{
		//	dwFlag |= ETFE_Friendly;
		//}
	}

	// 如果自身是NPC
	else if( IsNPC() )
	{
		if( pTarget->IsMonster() )
		{
			dwFlag |= ETFE_Hostile;
		}
		else if( pTarget->IsNPC() )
		{
			dwFlag |= ETFE_Friendly;
		}
	}

	// 如果自身是地物，先不做处理
	else if( IsGameObject() )
	{

	}

	// 如果自身是宠物，先不做处理
	else if( IsPet() )
	{

	}

	return dwFlag;
}

//----------------------------------------------------------------------------------
// 触发调查地物事件
//----------------------------------------------------------------------------------
VOID Creature::OnInvest(Role* pSrc)
{
	if(!VALID_POINT(m_pScript))	return;

	m_pScript->OnInvest(this, pSrc);
}

//----------------------------------------------------------------------------------
// 得到怪物小队指针
//----------------------------------------------------------------------------------
NPCTeam* Creature::GetTeamPtr()
{
	NPCTeam* pTeam = (NPCTeam*)INVALID_VALUE;
	if(INVALID_VALUE == m_dwTeamID)
		return pTeam;

	Map* pMap = get_map();
	if(VALID_POINT(pMap))
	{
		NPCTeamMgr* pTeamMgr = pMap->get_team_manager();
		if(VALID_POINT(pTeamMgr))
		{
			pTeam = pTeamMgr->GetNPCTeam(m_dwTeamID);
		}
	}
	return pTeam;
}

//----------------------------------------------------------------------------------
// 得到周围怪物的ID，并返回数量
//----------------------------------------------------------------------------------
INT Creature::GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh)
{
	// 如果范围为0，则直接返回
	if( 0.0f == fOPRadius )
		return 0;

	INT nCreatureNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到范围内的vistile列表
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Creature*	pCreature	= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 检测生物
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
			if(pCreature->IsDead()) continue;

			// 将怪物加入到列表中
			if(abs(pCreature->GetCurPos().y - GetCurPos().y) <= fHigh)
			{
				vecCreature.push_back(pCreature->GetID());
				nCreatureNum++;
			}
		}
	}

	return nCreatureNum;
}

//----------------------------------------------------------------------------------
// 得到周围玩家的ID，并返回数量
//----------------------------------------------------------------------------------
INT	Creature::GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh)
{
	// 如果攻击范围为0，则直接返回
	if( 0.0f == fOPRadius )
		return 0;

	INT nRoleNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到范围内的vistile列表
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Role*		pRole		= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 检测人物
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			if(pRole->IsDead()) continue;

			// 将玩家加入到列表中
			if(abs(pRole->GetCurPos().y - GetCurPos().y) <= fHigh)
			{
				vecRole.push_back(pRole->GetID());
				nRoleNum++;
			}
		}
	}

	return nRoleNum;
}

//-------------------------------------------------------------------------------
// 喊话
//-------------------------------------------------------------------------------
VOID Creature::Say(DWORD dwSayID)
{
	NET_SIS_monster_say send;
	send.dw_role_id = GetID();
	send.dwSayID = dwSayID;

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}

//--------------------------------------------------------------------------------
// 播放动作
//--------------------------------------------------------------------------------
VOID Creature::PlayerAction(DWORD dwActionID)
{
	NET_SIS_monster_play_action send;
	send.dw_role_id = GetID();
	send.dwActionFourCC = dwActionID;

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}

//----------------------------------------------------------------------------------
// 设置所属
//----------------------------------------------------------------------------------
VOID Creature::SetTagged(DWORD dwOwner)
{
	m_dwTaggedOwner = dwOwner;
	m_bTagged = VALID_VALUE(m_dwTaggedOwner);

	// 发送给客户端
	NET_SIS_change_creuture_adscription send;
	send.dwCreatureID = GetID();
	send.dwTaggedOwner = m_dwTaggedOwner;
	
	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}


//----------------------------------------------------------------------------------
// 关于护送NPC的路经触发相关
//----------------------------------------------------------------------------------
VOID Creature::OnReachEndPath( )
{
	//触发脚本
	if (VALID_POINT(m_pScript))
	{
		m_pScript->OnReachEndPath(this);
	}

	// 刷新新的九宫格
	get_map()->synchronize_remove_unit_to_big_visible_tile(this);

	// 从地图中拿掉该生物
	m_MoveData.Reset( m_vBornPos.x, m_vBornPos.y, m_vBornPos.z, m_vBornFace.x, m_vBornFace.y, m_vBornFace.z );
	get_map()->remove_npc(this);
	m_bDelSelf = true;
}

VOID Creature::OnAttChange( bool bRecalFlag[ERA_End] )
{
	if( bRecalFlag[ERA_Speed_XZ] )
	{
		m_fXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f);
		m_MoveData.StopMove();
	}

	if( bRecalFlag[ERA_Shape] )
	{
		m_Size = m_pProto->vSize * (FLOAT(m_nAtt[ERA_Shape]) / 10000.0f);
	}
}

//---------------------------------------------------------------------------------
// 属性改变
//---------------------------------------------------------------------------------
VOID Creature::OnAttChange( INT nIndex )
{
	switch(nIndex)
	{
		// 血量
	case ERA_HP:
		m_nAtt[ERA_HP] = min(m_nAtt[ERA_HP], m_nAtt[ERA_MaxHP]);
		break;

		// 真气
	case ERA_MP:
		m_nAtt[ERA_MP] = min(m_nAtt[ERA_MP], m_nAtt[ERA_MaxMP]);
		break;

		// 义气
	case ERA_Brotherhood:
		m_nAtt[ERA_Brotherhood] = min(m_nAtt[ERA_Brotherhood], m_nAtt[ERA_MaxBrotherhood]);
		break;
		
		// 武魂
	case ERA_Wuhuen:
		m_nAtt[ERA_Wuhuen] = min(m_nAtt[ERA_Wuhuen], m_nAtt[ERA_MaxBrotherhood]);
		break;

		// 持久力
		//case ERA_Endurance:
		//	m_nAtt[ERA_Endurance] = min(m_nAtt[ERA_Endurance], m_nAtt[ERA_MaxEndurance]);
		//	break;

	default:
		break;
	}
}
//VOID Creature::SetCanBeAttack(BOOL b)
//{ 
//	if (m_bCanBeAttack == b) return;
//
//	m_bCanBeAttack = b; 
//	tagSIS_MonsterCanBeAttackChange send;
//	send.dw_role_id = GetID();
//	send.bCanBeAttack = m_bCanBeAttack;
//
//	if( VALID_POINT(get_map()) )
//	{
//		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
//	}
//}

VOID Creature::SetCreatureType(ECreatureType et)
{
	if ( m_eCreatureType == et)
		return;

	m_eCreatureType = et;

	NET_SIS_monster_type_change send;
	send.dw_role_id = GetID();
	send.eCreType = et;

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}

//DWORD Creature::GetCurYield()
//{
//	tagPlantData* pPlantData = GetPlantData();
//	if (VALID_POINT(pPlantData))
//	{
//		return pPlantData->n_num;
//	}
//	
//	return 0;
//}
//
//VOID Creature::GetPlantRoleName(TCHAR* szName)
//{
//
//	tagPlantData* pPlantData = GetPlantData();
//	if (VALID_POINT(pPlantData))
//	{
//		s_role_info* pRoleInfo = g_roleMgr.get_role_info(pPlantData->dw_master_id);
//		if (VALID_POINT(pRoleInfo))
//		{
//			_tcsncpy(szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
//		}
//	}
//	
//}
//
//DWORD Creature::GetPlantTime()
//{
//
//	tagPlantData* pPlantData = GetPlantData();
//	if (VALID_POINT(pPlantData))
//	{
//		return pPlantData->dw_plant_time;
//	}
//	
//	return 0;
//}
//
//tagPlantData* Creature::GetPlantData()
//{
//	if (VALID_POINT(m_pPlantdata))
//	{
//		return m_pPlantdata;
//	}
//	else
//	{
//		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
//		if (VALID_POINT(pGuild))
//		{
//			tagPlantData* pPlantData = pGuild->getPlantData(GetPlantDataIndex());
//			if (VALID_POINT(pPlantData))
//			{
//				m_pPlantdata = pPlantData;
//				return m_pPlantdata;
//			}
//		}
//	}
//	return NULL;
//
//}
//
//VOID Creature::ResetPlantData()
//{
//	guild* pGuild = g_guild_manager.get_guild(GetGuildID());
//	if (VALID_POINT(pGuild))
//	{
//		pGuild->resetPlantData(GetPlantDataIndex());
//	}
//}
// 是否成熟
//BOOL Creature::IsMatrue()
//{
//	tagPlantData* pPlantData = GetPlantData();
//	if (VALID_POINT(pPlantData))
//	{
//		if(CalcTimeDiff(g_world.GetWorldTime(), pPlantData->dw_plant_time) > m_pProto->dwMatureTime)
//			return true;
//	}
//	return false;
//}
//// 是否自动收获
//BOOL Creature::IsAotoReceive()
//{
//	tagPlantData* pPlantData = GetPlantData();
//	if (VALID_POINT(pPlantData))
//	{
//		if(CalcTimeDiff(g_world.GetWorldTime(), pPlantData->dw_plant_time) > m_pProto->nLiveTick * TICK_TIME/1000)
//			return true;
//	}
//	return false;
//}

Role* Creature::GetMarster()
{
	if (VALID_POINT(GetAI()) && VALID_POINT(GetAI()->GetTracker()))
	{
		return GetAI()->GetTracker()->GetTarget();
	}
	return NULL;
}

DWORD Creature::GetMarsterID()
{
	if (VALID_POINT(GetAI()) && VALID_POINT(GetAI()->GetTracker()))
	{
		return GetAI()->GetTracker()->GetTargetID();
	}
	return INVALID_VALUE;
}