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
*	@brief		�����������,����,npc,����,����
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
// ��ʼ��
//--------------------------------------------------------------------------------
BOOL Creature::Init(const tagCreatureProto* pProto, const tag_map_way_point_info_list* patrolList)
{
	m_pProto = pProto;

	// ��ʼ���ű�
	m_pScript = g_ScriptMgr.GetCreatureScript(pProto->dw_data_id);

	// ��ʼ������
	InitAtt(pProto);

	// ��ʼ��AI
	InitAI(pProto, patrolList);

	// ���������ʼ����
	CalInitAtt();

	// ���ýű�---�������ʼ���ű��Ļ�,���ڻ�û�������õ�ͼ,��������ʧ�ܵ� LC
	/*if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnLoad(this);
	}*/

	return TRUE;
}
VOID Creature::OnScriptLoad()
{
	// ���ýű�
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
// ��ʼ����������
//---------------------------------------------------------------------------------
BOOL Creature::InitAtt(const tagCreatureProto* pProto)
{
	// ���ð�����
	m_Size = pProto->vSize;

	// �Ӿ�̬�����п����������������
	get_fast_code()->memory_copy(m_nBaseAtt, pProto->nBaseAtt, sizeof(m_nBaseAtt));

	m_nLevel = pProto->nLevel;
	m_nCurLevelExp = 0;

	// ���ݹ������ͨ����id�����ɼ���

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
// ��ʼ������AI
//---------------------------------------------------------------------------------
BOOL Creature::InitAI(const tagCreatureProto* pProto, const tag_map_way_point_info_list*	patrolList)
{
	// ����״̬��
	if( pProto->eAICreateType != EACT_Null )
	{
		m_pAIController = new AIController(this, patrolList);
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
// �����ʱ��ˢ������
//---------------------------------------------------------------------------------
VOID Creature::RefreshAtt()
{
	// �ƶ����
	m_MoveData.Reset(m_vBornPos.x, m_vBornPos.y, m_vBornPos.z, m_vBornFace.x, m_vBornFace.y, m_vBornFace.z);

	// �ٶ����
	m_fXZSpeed = X_DEF_XZ_SPEED;
	m_fYSpeed = X_DEF_Y_SPEED;

	// ���������
	m_Size = m_pProto->vSize;

	// ���֮ǰ��״̬
	ClearState();

	// ĳЩ��������
	m_bTagged = FALSE;
	m_dwTaggedOwner = INVALID_VALUE;
	m_nRespawnTickCountDown = 0;
	m_nLiveTick = 0;

	// ���Ե���
	ZeroMemory(m_nAttMod, sizeof(m_nAttMod));
	ZeroMemory(m_nAttModPct, sizeof(m_nAttModPct));
	ZeroMemory(m_nAtt, sizeof(m_nAtt));
	get_fast_code()->memory_copy(m_nBaseAtt, m_pProto->nBaseAtt, sizeof(m_nBaseAtt));

	// �������
	package_map<DWORD, Skill*>::map_iter it = m_mapSkill.begin();
	Skill* pSkill = NULL;
	while( m_mapSkill.find_next(it, pSkill) )
	{
		// ��ʼ���ܲ����
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
// �����ʱ��ˢ��AI
//---------------------------------------------------------------------------------
VOID Creature::RefreshAI()
{
	// ����״̬��
	if( VALID_POINT(m_pAIController) )
	{
		m_pAIController->Refresh();
	}
}


//---------------------------------------------------------------------------------
// ������ﵱǰ��������
//---------------------------------------------------------------------------------
VOID Creature::CalInitAtt()
{
	for(INT n = 0; n < ERA_End; n++)
	{
		m_nAtt[n] = m_nBaseAtt[n] + m_nAttMod[n] + (INT)(m_nBaseAtt[n] * (FLOAT)m_nAttModPct[n] / 10000.0f);
		// todo��ȡ������
	}

	// ��ĳЩ����ǰ���ԡ����ó����ֵ
	m_nAtt[ERA_HP] = m_nAtt[ERA_MaxHP];
	m_nAtt[ERA_MP] = m_nAtt[ERA_MaxMP];
	m_nAtt[ERA_Brotherhood] = m_nAtt[ERA_MaxBrotherhood];
	//m_nAtt[ERA_Vitality] = m_nAtt[ERA_MaxVitality];
	//m_nAtt[ERA_Endurance] = m_nAtt[ERA_MaxEndurance];

	// ����ĳЩ�ɻ�������Ӱ���ֵ
	m_fXZSpeed *= FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f;
	m_fYSpeed *= FLOAT(m_nAtt[ERA_Speed_Y]) / 10000.0f;
	m_Size *= FLOAT(m_nAtt[ERA_Shape]) / 10000.0f;
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID Creature::Destroy()
{
	SAFE_DELETE(m_pAIController);
}

//---------------------------------------------------------------------------------
// ���º���
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
// ���¼���
//---------------------------------------------------------------------------------
VOID Creature::UpdateCombat()
{
	// ��ǰ���ܻ�������
	if( m_CombatHandler.IsValid() )
	{
		m_CombatHandler.Update();

		if( !m_CombatHandler.IsValid() )
		{
			// �������ͷ���ɣ������´ι����ĵȴ�ʱ��
			GetAI()->SetNextAttackWaitTick(m_pProto->nAttackInterTick);
		}
	}
}

//---------------------------------------------------------------------------------
// ������
//---------------------------------------------------------------------------------
VOID Creature::UpdateDoor()
{
	if(!IsDoor())
		return;
	if(m_pProto->nDoorCloseTime <= 0)
		return;

}

//---------------------------------------------------------------------------------
// ����AI
//---------------------------------------------------------------------------------
VOID Creature::UpdateAI()
{
	// ���ݹ���Ŀǰ������һ��AI״̬������ν��и���
	if( VALID_POINT(m_pAIController) )
		m_pAIController->Update();
}

//---------------------------------------------------------------------------------
// ��������ʱ��
//---------------------------------------------------------------------------------
VOID Creature::UpdateLiveTime()
{
	if(0 != m_pProto->nLiveTick)
	{
		++m_nLiveTick;
		if(m_nLiveTick >= m_pProto->nLiveTick /*|| (IsGuildManRes() && IsAotoReceive())*/)
		{
			//// ��ʧʱ������ֲ����
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
// �����Һ�NPC̸���ľ���
//---------------------------------------------------------------------------------
BOOL Creature::CheckNPCTalkDistance(Role* pRole)
{
	if( !VALID_POINT(pRole) ) return FALSE;
	return IsInDistance(*pRole, MAX_NPC_TALK_DISTANCE);
}

//---------------------------------------------------------------------------------
// ������
//---------------------------------------------------------------------------------
VOID Creature::OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited)
{
	ASSERT( VALID_POINT(pSkill) && VALID_POINT(pSrc) );
	
	// �жϼ��ܵĵ���Ŀ������
	if( !pSkill->IsHostile() && !pSkill->IsFriendly() ) return;
	DWORD dwFriendEnemyFlag = pSrc->FriendEnemy(this);

	if( pSkill->IsHostile() && (ETFE_Hostile & dwFriendEnemyFlag) )
	{
		if( pSrc->IsRole() )
		{
			// �������֮ǰ�����������Ϊ����
			if( ECTT_Hit == GetTaggedType() && !IsTagged() )
			{
				SetTagged(pSrc->GetID());
			}
		}
		
		// �����ޣ������AI����(�ŵ��˺�������)
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

			// ��⵱ǰ���ܵĴ��
			GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Skill, ESSTE_Default == pSkill->GetTypeEx());
			// ���ĳЩ��������ϵ�buff
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
		// �������֮ǰ�����������Ϊ����
		if( ECTT_Hit == GetTaggedType() && !IsTagged() )
		{
			SetTagged(pSrc->GetID());
		}
	}

	// �����ޣ������AI����
	if( VALID_POINT(m_pAIController) )
	{
		m_pAIController->AddEnmity(pSrc, abs(dmg));
		m_pAIController->OnEvent(pSrc, ETEE_BeAttacked);
	}
}

VOID Creature::OnAIAttack()
{
	DWORD dwSkillID = m_CombatHandler.GetSkillID();

	// �������ͨ�������������ﴥ���������¼�

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
// ��������
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
	
	// �ô�ͷ��log
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

	// ��������Ѿ���������ֱ�ӷ���
	if( IsInState(ES_Dead) ) return;

	// ������
	OnBuffTrigger(pSrc, ETEE_Die);

	//�����ű�
	if (VALID_POINT(m_pScript))
	{
		m_pScript->OnDie(this, pSrc, m_dwTaggedOwner);
	}

	// ����Ϊ����״̬
	SetState(ES_Dead);

	// ֹͣ�ƶ�
	m_MoveData.StopMoveForce();

	// ֹͣ��ǰ�ļ���
	m_CombatHandler.End();

	// ��������ʱAI����

	// �Ƴ��������е�buff
	RemoveAllBuff();

	// ���ø���Ȩֵ
	SetInstLimit(pSrc, GetProto()->nInstPoint);

	// ���ø�������
	SetInstProcess();

	// ���渱������
	if(GetProto()->b_save_process)
	{
		SaveRoleProcess();
	}
	
	// ��ճ���б�
	if( VALID_POINT(GetAI()) )
		GetAI()->ClearAllEnmity();

	ClearEnmityCreature();
	SetTargetID(INVALID_VALUE);

	// ���ø������
	Role* pTarget = GetAI()->GetTracker()->GetTarget();
	if ( VALID_POINT(pTarget))
	{
		pTarget->SetFlowUnit(INVALID_VALUE);
	}

	BOOL bAddYiQi = FALSE;
	// �ҵ�����������
	Role* pReward = FindRewardRole(pSrc);
	if( VALID_POINT(pReward) )
	{
		// ���㾭�齱��
		BOOL bKill = FALSE;
		if( VALID_POINT(pSkill) )
			bKill = TRUE;

		if(pReward->GetTeamID() ==INVALID_VALUE)
		{
			std::vector<Role*>	vecRole;
			bAddYiQi = ExpReward(pReward,vecRole,bKill);

			// �о���ż�����
			if(bAddYiQi)
			{
				float fEarnRate	= pReward->GetEarnRate() / 10000.0f;
				pReward->ChangeBrotherhood(AttRes::GetInstance()->GetVariableLen().nBrother * fEarnRate);
			}

			// ��������ɱ�ּ���
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
				//���ܵ�ֻ��ɱ���ֵ��Ǹ���gx modify 2013.8.30
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

				// ��������ɱ�ּ���
				pMember->on_quest_event(EQE_Kill, GetTypeID(), get_level(), bAdd);
			}

			// ��һ�����о���ż�����
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
		
		// ������佱��
		g_drop_mgr.monster_drop(this, pReward);
	}

	// ��ɱ��
	if( VALID_POINT(pSrc) )
	{
		pSrc->OnKill(this);
	}

	// ��ͼ�¼�
	get_map()->on_creature_die(this, pSrc);

	// ���͹���������Ϣ
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

	// ������������ʱ��
	m_nRespawnTickCountDown = GetRespawnTick();

	// �ӵ�ͼ���õ�������
	get_map()->remove_creature(this);
}

//---------------------------------------------------------------------------------
// ���ø�������
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
// ֪ͨ���渱������
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

		// ��Ӹ�������
		p_inst->add_inst_process_to_role(pRole);
	}
}

//---------------------------------------------------------------------------------
// ���ø���Ȩֵ
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
// ������ʧ
//---------------------------------------------------------------------------------
VOID Creature::OnDisappear()
{
	//if (IsPet())
	//{
	//	g_world.get_log()->write_log(_T("Pet OnDisappeard!!\r\n"));
	//	return;
	//}

	// ��������Ѿ���������ֱ�ӷ���
	if( IsInState(ES_Dead) ) return;

	// ����Ϊ����״̬
	SetState(ES_Dead);

	// ֹͣ�ƶ�
	m_MoveData.StopMoveForce();

	// ֹͣ��ǰ�ļ���
	m_CombatHandler.End();

	// ��������ʱAI����

	// �Ƴ��������е�buff
	RemoveAllBuff();

	// ��ճ���б�
	if( VALID_POINT(GetAI()) )
		GetAI()->ClearAllEnmity();

	// ��ͼ�¼�
	get_map()->on_creature_disappear(this);

	// ������������ʱ��
	m_nRespawnTickCountDown = GetRespawnTick();

	// �ӵ�ͼ���õ�������
	get_map()->remove_creature(this);

	// ͬ�����ͻ���
	get_map()->synchronize_remove_unit_to_big_visible_tile(this);
}

//----------------------------------------------------------------------------------
// �����ɱ
//----------------------------------------------------------------------------------
VOID Creature::OnKill(Unit* pTarget)
{
	OnBuffTrigger(pTarget, ETEE_Kill);

	// �ɾ�
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
// ���Ը���
//----------------------------------------------------------------------------------
ECreatureReviveResult Creature::TryRevive()
{
	// ��������Ƕ�̬���ɵģ�����Ҫɾ���ڴ���
	if( ECST_Dynamic == m_eSpawnedType || ECST_Guild == m_eSpawnedType) return ECRR_NeedDestroy;

	// ����ǰ�����������
	if(ECST_Maidan == m_eSpawnedType)
	{
		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
		if(!VALID_POINT(pGuild))
			return ECRR_Failed;

		if(pGuild->get_upgrade().IsFacilitesDestory(EFT_Maidan))
			return ECRR_NeedDestroy;
	}

	//����ǰ����������
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
		// �����ʱ����һ��Tick
		m_nRespawnTickCountDown--;
	//}
	
	// �������ʱ��û�����򷵻�ʧ��
	if( m_nRespawnTickCountDown > 0 ) return ECRR_Failed;

	// �����ˢ�µ�֣�����Ҫ�������Ʒ
	if ( ECST_SpawnPoint == m_eSpawnedType ) return ECRR_NeedReplace;
	
	// ˢ�յ���ˢ�µ�, ����Ҫ��������λ��
	if ( ECST_SpawnList == m_eSpawnedType) return ECRR_NeedRepos;

	// ����ǳ�Ѩ�Ĺ���
	if( ECST_Nest == m_eSpawnedType )
	{
		// �ҵ�ˢ���ĳ�Ѩ�Ƿ񻹴��ڣ�����������ˣ�˵����Ѩ��ûˢ�£���ô�Լ�Ҳ��ˢ��
		Creature* pNestCreature = get_map()->find_creature(m_dwSpawnerID);
		if( !VALID_POINT(pNestCreature) ) return ECRR_Failed;

		// ����ҵ������ҵ��ó�Ѩ�ĵĳ�Ѩ����
		const tagCreatureProto* pProto = pNestCreature->GetProto();
		if( !VALID_POINT(pProto) || !VALID_POINT(pProto->pNest) ) return ECRR_Failed;

		const tagNestProto* pNest = pProto->pNest;

		// �ҵ�һ�������ߵ������
		Vector3 vNewBornPos;

		vNewBornPos.x = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + pNestCreature->GetCurPos().x;
		vNewBornPos.z = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + pNestCreature->GetCurPos().z;

		if( get_map()->if_can_go(vNewBornPos.x, vNewBornPos.z) )
		{
			vNewBornPos.y = 0;
		}
		else
		{
			// û���ҵ�һ������㣬��Tick������
			return ECRR_Failed;
		}

		// ���һ������
		Vector3 vNewBornFaceTo;
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		vNewBornFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vNewBornFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vNewBornFaceTo.y = 0.0f;


		// ������ĳ�������ͳ���������������ֵ
		m_vBornPos = vNewBornPos;
		m_vBornFace = vNewBornFaceTo;
	}

	// ���ͨ����������Ը�����
	OnRevive();

	return ECRR_Success;
}

//---------------------------------------------------------------------------------
// ���︴��
//---------------------------------------------------------------------------------
VOID Creature::OnRevive()
{
	// ˢ������
	RefreshAtt();
	// ˢ��AI
	RefreshAI();
	// ���¼�������
	CalInitAtt();

	if(VALID_POINT(m_pScript))
	{
		m_pScript->OnRespawn(this);
	}
}

//---------------------------------------------------------------------------------
// ���ҵ����������ߣ�������Ϊ��ɱ�ߣ�
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
// ���齱��
//---------------------------------------------------------------------------------
//gx add team_vec���ڷ��������
//����˫����Ӵ�ָ��Ի105%�ľ���ӳ�
BOOL Creature::ExpReward(Role* pReward, vector<Role*> &team_vec,BOOL bKill, const Team *pTeam, INT nShareNum,bool bSelf)
{
	if( !VALID_POINT(pReward) ) return FALSE;
	
	if (pReward->IsDead()) return FALSE;

	INT nExp  = 0;

	// ������������
	float fEarnRate		= pReward->GetEarnRate() / 10000.0f;
	FLOAT fExpRate	= get_map()->get_exp_rate();
	FLOAT fVNBExpRate = pReward->GetVNBExpRate();

	// ��������
	FLOAT fBaseExp = (FLOAT)m_pProto->nExpGive;
	// ����ս��
	FLOAT fBaseExploit = (FLOAT)m_pProto->nExploits;
	
	INT nBaseExp = 0;
	
	// �����صľ��齱��
	if(VALID_POINT(pTeam))
	{
		// ��Ӿ���ӳ�ϵ��
		FLOAT fCoe1 = 1.0f;
		if(nShareNum > 1)
		{
			fCoe1 = ((FLOAT)nShareNum/5) + 1.0f;
		}
		// ��Ӿ������ϵ��
		FLOAT fCoe2 = 1.0f;
		if(nShareNum > 1)
		{
			fCoe2 = 1.0f / nShareNum;
		}
		nBaseExp = (INT)(fBaseExp * fCoe1 * CalLevelCoefficient(pReward->get_level(), get_level()));
		nExp = (INT)(nBaseExp * fCoe2 * fExpRate);//mwh 2011-07-29 ����������ӱ�
		nExp = (INT)(nExp * (1+(FLOAT)pReward->GetAttValue(ERA_Exp_Add_Rate)/10000));
		//nExp = (INT)(nExp * pReward->CalFriendExpPer(pTeam));
		nExp = (INT)(nExp * fEarnRate);//������Ӱ��

		//gx add 2013.10.24
		FLOAT fCoe3 = 1.0f;//������Ӿ���ӳ�ϵ��
		if (pReward->GetSpouseID() != (DWORD)INVALID_VALUE)//���������ż
		{
			INT vec_Num = team_vec.size();
			for (INT i = 0;i < nShareNum;i++)
			{
				if (i >= vec_Num)//��ֹ�쳣
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
		// mwh 2011-07-29 ����ӱ�
		nExp = (INT)(fBaseExp * CalLevelCoefficient(pReward->get_level(), get_level()) * fExpRate);
		nExp = (INT)(nExp * (1+(FLOAT)pReward->GetAttValue(ERA_Exp_Add_Rate)/10000));
		nExp = (INT)(nExp * fEarnRate);//������Ӱ��
	}
	
	//pReward->GetCurMgr().IncExploits(fBaseExploit, elcid_pickup_money);
	//���ܵ�ֻ��ɱ��������Ǹ����
	if (bSelf)
	{
		// ���ڹ���15��û�м��ܵ�
		if (pReward->get_level() <= get_level() + 15)
		{
			pReward->ModAttValue(ERA_TalentPoint, fBaseExploit);
		}
	}
	
	
	pReward->ExpChange(nExp, bKill);

	// ��֤���ж�
	if (VALID_POINT(pReward->GetSession()))
	{
		pReward->GetSession()->GetVerification().beginVerification(pReward, TRUE);
	}
	
	if(nExp > 0)
		return TRUE;

	return FALSE;
	
}

//--------------------------------------------------------------------------
// ���㾭��ȼ�ϵ��
//--------------------------------------------------------------------------
FLOAT Creature::CalLevelCoefficient(INT nRoleLevel, INT nCreatureLevel)
{
	INT nLevelDiff = abs(nRoleLevel - nCreatureLevel);

	// �����ֵȼ��������ʱ
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
// ���ʱ,����Ƿ��ھ���������Ч��Χ
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
// ���ʱ,����Ƿ��ڵ���������Ч��Χ
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
// ������ӷ������С�����(������Ҹ�����
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
// Ŀ���ʾ�����ж�
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

	// ����
	if( pTarget->IsMonster() )
	{
		// ��ͨ����
		if( pTarget->IsNormalMonster() )
		{
			if( !pTarget->IsBoss())
				dwFlag |= ETF_NormalMonster;
			else
				dwFlag |= ETF_Boss;
		}

		// ��Ѩ
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

	// ����
	else if( pTarget->IsPet() )
	{
		dwFlag |= ETF_Pet;
	}

	// ��Դ
	else if( pTarget->IsRes() )
	{
		if ( pTarget->IsNatuRes() || pTarget->IsGuildNatuRes())
			dwFlag |= ETF_NatuRes;
		else if ( pTarget->IsManRes() || pTarget->IsGuildManRes())
			dwFlag |= ETF_ManRes;
		else
			ASSERT(0);
	}

	// �ɵ������
	else if( pTarget->IsInves() || pTarget->IsInves_two())
	{
		dwFlag |= ETF_InvesGameObject;
	}

	// �źͽ����ٿ����ŵ��ĸ�������ȥ

	return dwFlag;
}

//---------------------------------------------------------------------------
// ��Ŀ��ĵ��������ж�
//---------------------------------------------------------------------------
DWORD Creature::FriendEnemy(Unit* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;
	if( !VALID_POINT(get_map()) || get_map() != pTarget->get_map() ) return 0;

	// ����Ļ����������
	if( this == pTarget )
	{
		return ETFE_Friendly | ETFE_Hostile | ETFE_Independent;
	}

	// ȡ���ڵ�ͼ�ĵ��ҹ�ϵ�ж�
	BOOL bIgnore = FALSE;
	DWORD dwMapFlag = get_map()->get_friend_enemy_flag(this, pTarget, bIgnore);

	// �������Ҫ�жϵ�������ģ����ж�����
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

	// ��������ǹ���
	if( IsMonster() )
	{
		// ����ĵ����ж�
		Role* pMarster = GetMarster();
		if (VALID_POINT(pMarster))
		{
			if(pTarget == pMarster)
				return ETFE_Friendly;
			else
				return pMarster->FriendEnemy(pTarget);
		}
		
		// ��������,��������
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
				// ��Ӫ�ж�
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
			// ��Ӫ�ж�
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

	// ���������NPC
	else if( IsNPC() )
	{
		// ������������״̬����Ϊ�з�
		if( pTarget->IsInRoleState(ERS_PK) )
		{
			dwFlag |= ETFE_Hostile;
		}
		// ����Ϊ�ѷ�
		else
		{
			dwFlag |= ETFE_Friendly;
		}
	}

	// ��������ǵ����ʲô������
	else if( IsGameObject() )
	{

	}

	// ����ǳ�����Ȳ���
	else if( IsPet() )
	{

	}

	return dwFlag;
}

DWORD Creature::FriendEnemy(Creature* pTarget)
{
	DWORD dwFlag = 0;

	// ��������ǹ���
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

	// ���������NPC
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

	// ��������ǵ���Ȳ�������
	else if( IsGameObject() )
	{

	}

	// ��������ǳ���Ȳ�������
	else if( IsPet() )
	{

	}

	return dwFlag;
}

//----------------------------------------------------------------------------------
// ������������¼�
//----------------------------------------------------------------------------------
VOID Creature::OnInvest(Role* pSrc)
{
	if(!VALID_POINT(m_pScript))	return;

	m_pScript->OnInvest(this, pSrc);
}

//----------------------------------------------------------------------------------
// �õ�����С��ָ��
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
// �õ���Χ�����ID������������
//----------------------------------------------------------------------------------
INT Creature::GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh)
{
	// �����ΧΪ0����ֱ�ӷ���
	if( 0.0f == fOPRadius )
		return 0;

	INT nCreatureNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// �õ���Χ�ڵ�vistile�б�
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Creature*	pCreature	= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// �������
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
			if(pCreature->IsDead()) continue;

			// ��������뵽�б���
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
// �õ���Χ��ҵ�ID������������
//----------------------------------------------------------------------------------
INT	Creature::GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh)
{
	// ���������ΧΪ0����ֱ�ӷ���
	if( 0.0f == fOPRadius )
		return 0;

	INT nRoleNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// �õ���Χ�ڵ�vistile�б�
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Role*		pRole		= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// �������
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			if(pRole->IsDead()) continue;

			// ����Ҽ��뵽�б���
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
// ����
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
// ���Ŷ���
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
// ��������
//----------------------------------------------------------------------------------
VOID Creature::SetTagged(DWORD dwOwner)
{
	m_dwTaggedOwner = dwOwner;
	m_bTagged = VALID_VALUE(m_dwTaggedOwner);

	// ���͸��ͻ���
	NET_SIS_change_creuture_adscription send;
	send.dwCreatureID = GetID();
	send.dwTaggedOwner = m_dwTaggedOwner;
	
	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}


//----------------------------------------------------------------------------------
// ���ڻ���NPC��·���������
//----------------------------------------------------------------------------------
VOID Creature::OnReachEndPath( )
{
	//�����ű�
	if (VALID_POINT(m_pScript))
	{
		m_pScript->OnReachEndPath(this);
	}

	// ˢ���µľŹ���
	get_map()->synchronize_remove_unit_to_big_visible_tile(this);

	// �ӵ�ͼ���õ�������
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
// ���Ըı�
//---------------------------------------------------------------------------------
VOID Creature::OnAttChange( INT nIndex )
{
	switch(nIndex)
	{
		// Ѫ��
	case ERA_HP:
		m_nAtt[ERA_HP] = min(m_nAtt[ERA_HP], m_nAtt[ERA_MaxHP]);
		break;

		// ����
	case ERA_MP:
		m_nAtt[ERA_MP] = min(m_nAtt[ERA_MP], m_nAtt[ERA_MaxMP]);
		break;

		// ����
	case ERA_Brotherhood:
		m_nAtt[ERA_Brotherhood] = min(m_nAtt[ERA_Brotherhood], m_nAtt[ERA_MaxBrotherhood]);
		break;
		
		// ���
	case ERA_Wuhuen:
		m_nAtt[ERA_Wuhuen] = min(m_nAtt[ERA_Wuhuen], m_nAtt[ERA_MaxBrotherhood]);
		break;

		// �־���
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
// �Ƿ����
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
//// �Ƿ��Զ��ջ�
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