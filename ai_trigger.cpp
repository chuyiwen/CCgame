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
*	@file		ai_trigger.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		����AI������
*/

#include "StdAfx.h"
#include "ai_trigger.h"
#include "unit.h"
#include "creature.h"
#include "creature_ai.h"
#include "map.h"
#include "NPCTeam.h"


#include "../../common/WorldDefine/trigger_define.h"

AITrigger::AITrigger():m_bActive(FALSE),m_nActiveTick(1),m_dwTriggeredTimes(0),
									m_dwSkillID(INVALID_VALUE),m_pProto(NULL)
{

}

//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
BOOL AITrigger::Init(DWORD dwCreatureTypeID, DWORD dw_data_id, INT nTargetType, DWORD dwSkillID)
{
	m_pProto = AttRes::GetInstance()->GetTriggerProto(dw_data_id);
	if(!VALID_POINT(m_pProto))
	{
		print_message(_T("can not fine Trigger proto <TypeID: %u>\r\n"), dw_data_id);
		return FALSE;
	}

	m_dwSkillID = dwSkillID;
	m_eTargeType = (ECreatureTargetFriendEnemy)nTargetType;

	return TRUE;
}

//------------------------------------------------------------------------------
// ������ˢ��
//------------------------------------------------------------------------------
VOID AITrigger::Refresh()
{
	m_bActive			=		FALSE;
	m_nActiveTick		=		1;
	m_dwTriggeredTimes	=		0;
}

//------------------------------------------------------------------------------
// ��ⴥ��������ʱ��
//------------------------------------------------------------------------------
BOOL AITrigger::TestTimeIntervalTrigger()
{
	// �Ƿ�����������
	if(0 != m_pProto->dwEventMisc1)
	{
		if(0 != (m_nActiveTick % m_pProto->dwEventMisc1))
			return FALSE;
	}

	// �Ƿ񳬹�������������
	if(0 != m_pProto->dwEventMisc2)
	{
		if(m_dwTriggeredTimes >= m_pProto->dwEventMisc2)
			return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// ��������״̬�������
//------------------------------------------------------------------------------
BOOL AITrigger::TestStateTrigger(Creature* pOwner, DWORD& dwTargetID)
{
	Unit* pTarget = pOwner->get_map()->find_unit(dwTargetID);

	if(VALID_POINT(pTarget) && pTarget->IsDead())
		return FALSE;

	// ���ͬ�ӳ�Ա��������һ��ֵ
	if(ETST_TeammateHPLower == m_pProto->eStateType)
	{
		NPCTeam* pTeam = pOwner->GetTeamPtr();
		if(!VALID_POINT(pTeam))		return FALSE;

		pTarget = pTeam->IsTeamateHPLower(m_pProto->dwStateMisc1);
		if(!VALID_POINT(pTarget))	return FALSE;
		
		dwTargetID = pTarget->GetID();
		return TRUE;
	}

	// ���ͬ�ӳ�Ա��������һ��ֵ
	if(ETST_TeammateMPLower == m_pProto->eStateType)
	{
		NPCTeam* pTeam = pOwner->GetTeamPtr();
		if(!VALID_POINT(pTeam))		return FALSE;

		pTarget = pTeam->IsTeamateMPLower(m_pProto->dwStateMisc1);
		if(!VALID_POINT(pTarget))	return FALSE;

		dwTargetID = pTarget->GetID();
		return TRUE;
	}

	// ���Ŀ������Ƿ���ĳһ��Χ֮��
	if(ETST_Distance == m_pProto->eStateType)
	{
		// С��������
		if(VALID_POINT(pTarget) && pOwner->IsInDistance(*pTarget, m_pProto->dwStateMisc2*TILE_SCALE) )
		{
			// ������С����
			if(!pOwner->IsInDistance(*pTarget, m_pProto->dwStateMisc1*TILE_SCALE) )
				return TRUE;
		}

		return FALSE;
	}

	if(!pOwner->TestStateTrigger(pTarget, m_pProto, NULL))	
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
// ��������ʱ���������
//------------------------------------------------------------------------------
BOOL AITrigger::TestEventTrigger(Creature* pOwner)
{
	if(!pOwner->TestEventTrigger(NULL, m_pProto, NULL, INVALID_VALUE, INVALID_VALUE))	
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
// ������Update
//------------------------------------------------------------------------------
VOID AITrigger::Update(Creature* pOwner)
{
	if(!m_bActive)	return;

	// �ۼƼ���ʱ��
	++m_nActiveTick;

	// ��ⴥ��������ʱ��
	if( !TestTimeIntervalTrigger() ) return;

	// �õ�Ŀ��ID
	DWORD dwTargetID = pOwner->GetAI()->GetTargetIDByType(m_eTargeType);

	// ��ⴥ��
	if( TestEventTrigger(pOwner) && TestStateTrigger(pOwner, dwTargetID) )
	{
		// �����¼�
		OnTrigger(pOwner, dwTargetID);
	}
}

//------------------------------------------------------------------------------
// �������
//------------------------------------------------------------------------------
VOID AITrigger::SetTriggerActive(INT eEventType)
{
	if( eEventType != m_pProto->eEventType ) return;

	// �������
	SetActive(TRUE);
}

//------------------------------------------------------------------------------
// ����������
//------------------------------------------------------------------------------
VOID AITrigger::OnEvent(INT eEventType, Creature* pOwner)
{
	if( eEventType != m_pProto->eEventType )	return;

	// ���Դ��������¼������Ƿ�����
	if( !TestEventTrigger(pOwner) )	return;

	// �������
	SetActive(TRUE);

	// �õ�Ŀ��ID
	DWORD dwTargetID = pOwner->GetAI()->GetTargetIDByType(m_eTargeType);

	// ��ⴥ������״̬�����Ƿ�����
	if( TestStateTrigger(pOwner, dwTargetID) )
	{
		// �����¼�
		OnTrigger(pOwner, dwTargetID);
	}
}

//------------------------------------------------------------------------------
// �������¼�����
//------------------------------------------------------------------------------
VOID AITrigger::OnTrigger(Creature* pOwner, DWORD dwTargetID)
{
	if( E_Success == pOwner->GetAI()->AIUseSkill(m_dwSkillID, dwTargetID) )
	{
		// �ۼƴ�������
		++m_dwTriggeredTimes;
		m_nActiveTick = 1;
	}
	//else
	//{
	//	// ��һtick��������
	//	--m_nActiveTick;
	//}
}


//------------------------------------------------------------------------------
// ����������������
//------------------------------------------------------------------------------
AITriggerMgr::~AITriggerMgr()
{
	std::list<AITrigger*>::iterator it = m_listTrigger.begin();
	while( it != m_listTrigger.end() )
	{
		delete *it;
		++it;
	}

	m_listTrigger.clear();
}

//------------------------------------------------------------------------------
// ������������Init
//------------------------------------------------------------------------------
BOOL AITriggerMgr::Init(Creature* pOwner, const tagCreatureAI* m_pAIProto)
{
	m_pOwner = pOwner;
	m_bPaused = FALSE;

	for(INT i = 0; i < X_MAX_CREATURE_AI_SKILL; ++i)
	{
		if( INVALID_VALUE == m_pAIProto->dwTriggerID[i] )
			break;

		AITrigger* pAITrigger = new AITrigger;
	
		DWORD dwSkillID = Skill::GetIDFromTypeID(m_pAIProto->dwSkillTypeID[i]);
		if( !pAITrigger->Init(pOwner->GetTypeID(), m_pAIProto->dwTriggerID[i], m_pAIProto->nTargetType[i], dwSkillID) )
		{
			SAFE_DELETE(pAITrigger);
		}
		else
		{
			m_listTrigger.push_back(pAITrigger);
		}
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// �������
//------------------------------------------------------------------------------
VOID AITriggerMgr::SetTriggerActive(INT eEventType)
{
	std::list<AITrigger*>::iterator it = m_listTrigger.begin();
	while (it != m_listTrigger.end())
	{
		(*it)->SetTriggerActive(eEventType);
		++it;
	}
}

//------------------------------------------------------------------------------
// ������������Update
//------------------------------------------------------------------------------
VOID AITriggerMgr::Update()
{
	std::list<AITrigger*>::iterator it = m_listTrigger.begin();
	while (it != m_listTrigger.end())
	{
		(*it)->Update(m_pOwner);
		++it;
	}
}

//------------------------------------------------------------------------------
// ������������ˢ��
//------------------------------------------------------------------------------
VOID AITriggerMgr::Refresh()
{
	std::list<AITrigger*>::iterator it = m_listTrigger.begin();
	while (it != m_listTrigger.end())
	{
		(*it)->Refresh();
		++it;
	}
}


//------------------------------------------------------------------------------------------
// ����������
//------------------------------------------------------------------------------------------
VOID AITriggerMgr::OnEvent(INT eEventType)
{
	if( IsPaused() ) return;

	std::list<AITrigger*>::iterator it = m_listTrigger.begin();
	while (it != m_listTrigger.end())
	{
		(*it)->OnEvent(eEventType, m_pOwner);
		++it;
	}
}
