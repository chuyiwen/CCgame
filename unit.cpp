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
*	@file		role.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		�������ݽṹ
*/

#include "StdAfx.h"

#include "../common/ServerDefine/base_server_define.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/buff_define.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "../../common/WorldDefine/move_define.h"

#include "world.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "map.h"
#include "map_creator.h"
#include "map_mgr.h"
#include "att_res.h"
#include "buff.h"
#include "buff_effect.h"
#include "unit.h"
#include "group_mgr.h"

//-------------------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------------------
Unit::Unit(DWORD dwID, DWORD dwMapID, const Vector3& vPos, const Vector3& vFaceTo): 
m_dwID(dwID), 
m_Size(X_DEF_ROLE_SIZE_X, X_DEF_ROLE_SIZE_Y, X_DEF_ROLE_SIZE_Z), 
m_fXZSpeed(X_DEF_XZ_SPEED), 
m_fYSpeed(X_DEF_Y_SPEED), 
m_fSwimXZSpeed(X_DEF_XZ_SPEED), 
m_fDropXZSpeed(X_DEF_DROP_XZ_SPEED), 
m_fSlideSpeed(X_DEF_SLIDE_SPEED),
m_fCarryXZSpeed(X_DEF_XZ_SPEED/2),
m_dwMapID(dwMapID), 
m_pMap(NULL), 
m_dwInstanceID(INVALID_VALUE), 
m_bNeedRecal(FALSE), 
m_StateMgr(), 
m_nHPMPRegainTickCountDown(HP_MP_REGAIN_INTER_TICK),
m_nVitalityRegainTickCountDown(VITALITY_REGAIN_INTER_TICK), 
m_nEnduranceRegainTickCountDown(ENDURANCE_REGAIN_INTER_TICK), 
m_nInjuryRegainTickCountDown(INJURY_REGAIN_INTER_TICK),
m_nRageRegainTickCountDown(RAGE_GEGAIN_INTER_TICK),
m_nPointRegainTickCountDown(POINT_GEGAIN_INTER_TICK),
m_nEnergyRegainTickCountDown(ENERGY_GEGAIN_INTER_TICK),
m_nFocusRegainTickCountDown(FOCUS_GEGAIN_INTER_TICK),
m_nProduceSkillNum(0),
m_nRingBuffID(INVALID_VALUE),
m_bDmgAbs(0),
m_bDmgReturn(FALSE),
m_bIsFeating(FALSE),
m_nFeatRestTick(0),
m_dwTargetID(INVALID_VALUE),
m_dwFlowUnit(INVALID_VALUE)
{
	ZeroMemory(m_nAtt, sizeof(m_nAtt));
	ZeroMemory(m_nBaseAtt, sizeof(m_nBaseAtt));
	ZeroMemory(m_nAttMod, sizeof(m_nAttMod));
	ZeroMemory(m_nAttModPct, sizeof(m_nAttModPct));
	ZeroMemory(m_bAttRecalFlag, sizeof(m_bAttRecalFlag));

	m_MoveData.Init(this, vPos, vFaceTo);
	m_CombatHandler.Init(this);
}

//-------------------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------------------
Unit::~Unit()
{
	// ɾ�����м���
	package_map<DWORD, Skill*>::map_iter itSkill = m_mapSkill.begin();
	Skill* pSkill = NULL;

	while( m_mapSkill.find_next(itSkill, pSkill) )
	{
		SAFE_DELETE(pSkill);
	}
	m_mapSkill.clear();

	// ɾ�����е�buff�޸���
	package_list<DWORD>* pListModifier = NULL;
	package_map<DWORD, package_list<DWORD>*>::map_iter itModifier = m_mapBuffModifier.begin();
	while( m_mapBuffModifier.find_next(itModifier, pListModifier) )
	{
		SAFE_DELETE(pListModifier);
	}
	m_mapBuffModifier.clear();

	// ɾ������trigger�޸���
	itModifier = m_mapTriggerModifier.begin();
	while( m_mapTriggerModifier.find_next(itModifier, pListModifier) )
	{
		SAFE_DELETE(pListModifier);
	}
	m_mapTriggerModifier.clear();
}

//-------------------------------------------------------------------------------
// ������ʱ����µĺ���
//-------------------------------------------------------------------------------
VOID Unit::update_time_issue()
{
	if( IsDead() ) return;

	// ��Ѫ����
	if( --m_nHPMPRegainTickCountDown <= 0 )
	{
		m_nHPMPRegainTickCountDown = HP_MP_REGAIN_INTER_TICK;

		if( GetAttValue(ERA_HPRegainRate) != 0 )
		{
			ChangeHP(GetAttValue(ERA_HPRegainRate), this);
		}
		if( GetAttValue(ERA_MPRegainRate) != 0 )
		{
			ChangeMP(GetAttValue(ERA_MPRegainRate));
		}
	}

	// �ػ���
	//if( --m_nVitalityRegainTickCountDown <= 0 )
	//{
	//	m_nVitalityRegainTickCountDown = VITALITY_REGAIN_INTER_TICK;

	//	if( GetAttValue(ERA_VitalityRegainRate) != 0 )
	//	{
	//		ChangeVitality(GetAttValue(ERA_VitalityRegainRate));
	//	}
	//}

	// �س־���
	//if( --m_nEnduranceRegainTickCountDown <= 0 )
	//{
	//	m_nEnduranceRegainTickCountDown = ENDURANCE_REGAIN_INTER_TICK;

	//	ChangeEndurance(5);
	//}

	// ������һЩ��ʱ��仯��

	// ���н�ɫ�ص���
	//if( IsRole() )
	//{
	//	Role* pRole = static_cast<Role *>(this);
	//	ASSERT(VALID_POINT(pRole));
	//	if( pRole->IsInRoleState(ERS_PrisonArea) && --m_nInPrisonMoraleRegainTickCountDown <= 0 )
	//	{
	//		m_nInPrisonMoraleRegainTickCountDown = INPRISON_MORALE_REGAIN_INTER_TICK;

	//		INT nCurMorale = pRole->GetAttValue(ERA_Morality);
	//		INT nNeed = 0;
	//		if (nCurMorale < 0)
	//		{
	//			nNeed = 0 - nCurMorale;
	//			if (nNeed > INPRISON_MORAL_INCREASE_VAL)
	//			{
	//				nNeed = INPRISON_MORAL_INCREASE_VAL;
	//			}
	//		}
	//		else
	//		{
	//			DWORD	dwPrisonMapID	= g_mapCreator.get_prison_map_id();
	//			Vector3	PutOutPoint		= g_mapCreator.get_putout_prison_point();

	//			pRole->GotoNewMap(dwPrisonMapID, PutOutPoint.x, PutOutPoint.y, PutOutPoint.z);
	//		}

	//		ModAttValue(ERA_Morality, nNeed);
	//	}		 
	//}
}
VOID	Unit::ReAccAtt()
{
	// ��ʼ����
	for(INT n = 0; n < ERA_End; n++)
	{
		if( false == GetAttRecalFlag(n) )
			continue;

		// ��������Ҫ���¼���
		m_nAtt[n] = CalAttMod(m_nBaseAtt[n], n);
	}
}
//--------------------------------------------------------------------------------
// ���������־λ���е�ǰ��������
//--------------------------------------------------------------------------------
VOID Unit::RecalAtt(BOOL bSendMsg, BOOL bHp)
{
	// ���Ƚ�ĳЩ����ǰ���ԡ���ȡ����
	INT nHP				=	m_nAtt[ERA_HP];
	INT nMaxHP			=	m_nAtt[ERA_MaxHP];
	INT nMP				=	m_nAtt[ERA_MP];
	INT nMaxMp			=	m_nAtt[ERA_MaxMP];
	INT nVitality		=	m_nAtt[ERA_Brotherhood];
	INT nWuhuen			=	m_nAtt[ERA_Wuhuen];
	//INT nEndurance		=	m_nAtt[ERA_Endurance];
	INT nKnowledge		=	m_nAtt[ERA_Knowledge];
	INT nInjury			=	m_nAtt[ERA_Injury];
	INT nMorale			=	m_nAtt[ERA_Morale];
	INT nMorality		=	m_nAtt[ERA_Morality];
	//INT nCulture		=	m_nAtt[ERA_Luck];
	INT nAttPoint		=	m_nAtt[ERA_AttPoint];
	INT nTalentPoint	=	m_nAtt[ERA_TalentPoint];
	//INT nLove			=	m_nAtt[ERA_Love];

	float fHp = nHP * 1.0f / nMaxHP; 
	float fMp = nMP * 1.0f / nMaxMp;

	//��ʼ����
	ReAccAtt();

	// ����ԭ�ȱ��������
	m_nAtt[ERA_Knowledge]	=	nKnowledge;
	m_nAtt[ERA_Injury]		=	nInjury;
	m_nAtt[ERA_Morale]		=	nMorale;
	m_nAtt[ERA_Morality]	=	nMorality;
	//m_nAtt[ERA_Luck]		=	nCulture;
	m_nAtt[ERA_AttPoint]	=	nAttPoint;
	m_nAtt[ERA_TalentPoint]	=	nTalentPoint;
	//m_nAtt[ERA_Love]		=	nLove;

	if (bHp)
	{
		m_nAtt[ERA_HP] = fHp * m_nAtt[ERA_MaxHP];
		m_nAtt[ERA_MP] = fMp * m_nAtt[ERA_MaxMP];
	}
	else
	{
		m_nAtt[ERA_HP] = min(nHP, m_nAtt[ERA_MaxHP]);
		m_nAtt[ERA_MP] = min(nMP, m_nAtt[ERA_MaxMP]);
	}
	

	// ��֮ǰ�����ĳЩ����ǰ���ԡ����Ӧ�ġ����ֵ��ȡ��Сֵ
	//m_nAtt[ERA_HP] = min(nHP, m_nAtt[ERA_MaxHP]);
	//m_nAtt[ERA_MP] = min(nMP, m_nAtt[ERA_MaxMP]);
	m_nAtt[ERA_Brotherhood] = min(nVitality, m_nAtt[ERA_MaxBrotherhood]);
	m_nAtt[ERA_Wuhuen]	= min(nWuhuen, m_nAtt[ERA_MaxBrotherhood]);
	//m_nAtt[ERA_Endurance] = min(nEndurance, m_nAtt[ERA_MaxEndurance]);

	// �Ƚ�ԭ����HP���µ�HP֮���Ƿ�һ�£������һ�£���Ѫԭ������Ҫ���¼��㣬��������Ҫ���¼��㣬�Ա㷢��Ϣ
	if( m_nAtt[ERA_HP] != nHP && !m_bAttRecalFlag[ERA_HP] ) m_bAttRecalFlag[ERA_HP] = true;
	if( m_nAtt[ERA_MP] != nMP && !m_bAttRecalFlag[ERA_MP] ) m_bAttRecalFlag[ERA_MP] = true;
	//if( m_nAtt[ERA_Brotherhood] != nVitality && !m_bAttRecalFlag[ERA_Brotherhood] ) m_bAttRecalFlag[ERA_Brotherhood] = true;
	//if( m_nAtt[ERA_Endurance] != nEndurance && !m_bAttRecalFlag[ERA_Endurance] ) m_bAttRecalFlag[ERA_Endurance] = true;

	// �����Ҫ������Ϣ
	if( bSendMsg )
	{
		SendAttChangeMsg();

		// ����ĳЩ��Ϊ���Ըı��Ӱ���ֵ
		OnAttChange(m_bAttRecalFlag);
	}


	// ��������־λ
	ClearAttRecalFlag();
}

//--------------------------------------------------------------------------------
// �������Ըı���Ϣ
//--------------------------------------------------------------------------------
VOID Unit::SendAttChangeMsg()
{
	// ���Ȳ鿴�Ƿ���Ҫ�㲥��Զ�������Ҫ֪����һЩ���ԣ�
	BOOL bNeedBroadcast = FALSE;			// �Ƿ���Ҫ�㲥
	bool bRemoteRoleAtt[ERRA_End] = {false};// Զ���������
	INT nRemoteRoleAtt[ERRA_End] = {0};		// Զ���������
	INT nRoleAttNum = 0;					// ����������Ըı������
	INT nRemoteRoleAttNum = 0;				// Զ��������Ըı������

	// �������ϲ���
	for(INT n = 0; n < ERA_End; ++n)
	{
		if( m_bAttRecalFlag[n] )
		{
			++nRoleAttNum;
			ERemoteRoleAtt eType = ERA2ERRA((ERoleAttribute)n);
			if( eType != ERRA_Null )
			{
				++nRemoteRoleAttNum;
				bRemoteRoleAtt[eType] = true;
				nRemoteRoleAtt[eType] = m_nAtt[n];

				bNeedBroadcast = TRUE;
			}
		}
	}

	// ���Ȳ鿴�Ƿ���Ҫ�㲥
	if( bNeedBroadcast && nRemoteRoleAttNum > 0 )
	{
		DWORD dw_size = sizeof(NET_SIS_mutiple_remote_att_change) + sizeof(tagRemoteAttValue) * (nRemoteRoleAttNum - 1);
		CREATE_MSG(pSend, dw_size, NET_SIS_mutiple_remote_att_change);

		pSend->dw_size = dw_size;
		pSend->dw_role_id = m_dwID;
		pSend->n_num = nRemoteRoleAttNum;

		INT nIndex = 0;
		for(INT n = 0; n < ERRA_End; ++n)
		{
			if( bRemoteRoleAtt[n] )
			{
				pSend->value[nIndex].eType = (ERemoteRoleAtt)n;
				pSend->value[nIndex].nValue = nRemoteRoleAtt[n];
				++nIndex;
			}
		}

		if( VALID_POINT(get_map()) )
			get_map()->send_big_visible_tile_message(this, pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	// ��������ﲢ����Ҫ���ͣ����͸��������
	if( IsRole() && nRoleAttNum > 0 )
	{
		Role* pRole = reinterpret_cast<Role*>(this);

		DWORD dw_size = sizeof(NET_SIS_mutiple_role_att_change) + sizeof(tagRoleAttValue) * (nRoleAttNum - 1);
		CREATE_MSG(pSend, dw_size, NET_SIS_mutiple_role_att_change);

		pSend->dw_size = dw_size;
		pSend->n_num = nRoleAttNum;

		INT nIndex = 0;
		for(INT n = 0; n < ERA_End; ++n)
		{
			if( GetAttRecalFlag(n) )
			{
				pSend->value[nIndex].eType = (ERoleAttribute)n;
				pSend->value[nIndex].nValue = m_nAtt[n];
				++nIndex;
				
				ERemoteRoleAtt eRemote = ERA2ERRA((ERoleAttribute)n);

				// ͬ�����Ը�С�����
				if(pRole->GetTeamID() != INVALID_VALUE && IsTeamRemoteAtt(eRemote))
				{
					//ͬ��С�ӳ�Ա
					NET_SIS_single_remote_att_change send;
					send.dw_role_id = GetID();
					send.eType = eRemote;
					send.nValue = m_nAtt[n];
					g_groupMgr.SendTeamMesOutBigVis(pRole, pRole->GetTeamID(), &send, send.dw_size);
				}
					
			}
		}

		pRole->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
}
//--------------------------------------------------------------------------------
// �������Լӳ�����ֵ����ȡ������
//--------------------------------------------------------------------------------
INT Unit::CalAttMod(INT nBase, INT nIndex)
{
	// ���nIndex�Ƿ�Ϸ�
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );

	INT nValue = nBase + m_nAttMod[nIndex]; 
	nValue += nValue * (FLOAT(m_nAttModPct[nIndex]) / 10000.0f);

	if( nValue < AttRes::GetInstance()->GetAttMin(nIndex) ) nValue = AttRes::GetInstance()->GetAttMin(nIndex);
	if( nValue > AttRes::GetInstance()->GetAttMax(nIndex) ) nValue = AttRes::GetInstance()->GetAttMax(nIndex);

	return nValue;
}

//--------------------------------------------------------------------------------
// ����ĳ�����Ա仯���ͻ��ˣ���������Ա仯��Ҫ�㲥��㲥
//--------------------------------------------------------------------------------
VOID Unit::SendAttChange(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );

	ERemoteRoleAtt eRemote = ERA2ERRA((ERoleAttribute)nIndex);

	// ����
	if( IsCreature() )
	{
		if( ERRA_Null != eRemote )
		{
			// ����Զ��ͬ������
			NET_SIS_single_remote_att_change send;
			send.dw_role_id = GetID();
			send.eType = eRemote;
			send.nValue = m_nAtt[nIndex];

			if( get_map() )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}
	}
	// ���
	else if( IsRole() )
	{
		// ���ȷ��͸��Լ�
		Role* pRole = reinterpret_cast<Role*>(this);

		NET_SIS_single_role_att_change send;
		send.eType = (ERoleAttribute)nIndex;
		send.nValue = m_nAtt[nIndex];
		pRole->SendMessage(&send, send.dw_size);

		// �����Ҫͬ����Զ�����
		if( ERRA_Null != eRemote )
		{
			// ����Զ��ͬ������
			NET_SIS_single_remote_att_change send;
			send.dw_role_id = GetID();
			send.eType = eRemote;
			send.nValue = m_nAtt[nIndex];

			if( get_map() )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
			
			// ͬ�����Ը�С�����
			if(pRole->GetTeamID() != INVALID_VALUE && IsTeamRemoteAtt(eRemote))
				g_groupMgr.SendTeamMesOutBigVis(pRole, pRole->GetTeamID(), &send, send.dw_size);
		}
	}
}

//-----------------------------------------------------------------------------------------
// �õ�����״̬��־λ
//-----------------------------------------------------------------------------------------
DWORD Unit::GetStateFlag()
{
	DWORD dwStateFlag = 0;

	dwStateFlag |=	( IsInState(ES_Dead)		?	ESF_Dead		:	ESF_NoDead );
	dwStateFlag |=	( IsInState(ES_Dizzy)		?	ESF_Dizzy		:	ESF_NoDizzy );
	dwStateFlag |=	( IsInState(ES_Spor)		?	ESF_Spor		:	ESF_NoSpor );
	dwStateFlag |=	( IsInState(ES_Tie)			?	ESF_Tie			:	ESF_NoTie );
	dwStateFlag |=	( IsInState(ES_Invincible)	?	ESF_Invincible	:	ESF_NoInvincible );
	dwStateFlag |=	( IsInState(ES_Lurk)		?	ESF_Lurk		:	ESF_NoLurk );
	dwStateFlag |=	( IsInState(ES_DisArm)		?	ESF_DisArm		:	ESF_NoDisArm );
	dwStateFlag |=	( IsInState(ES_NoSkill)		?	ESF_NoSkill		:	ESF_NoNoSkill );

	return dwStateFlag;
}


//-----------------------------------------------------------------------------
// ����ĳ�����ܵ�Ӱ��
//-----------------------------------------------------------------------------
VOID Unit::AddSkillMod(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;

	ESkillTargetType eTarget = pSkill->GetTargetType();

	switch(eTarget)
	{
		// Ӱ�켼��
	case ESTT_Skill:
		{
			const tagSkillProto* pSkillProto = pSkill->GetProto();
			if( !VALID_POINT(pSkillProto) ) return;
			
			for (int i = 0; i < MAX_TARGET_NUM; i ++)
			{
				Skill* pTargetSkill = m_mapSkill.find(pSkillProto->dwTargetSkillID[i]);
				if( !VALID_POINT(pTargetSkill) ) return;

				if( pTargetSkill->SetMod(pSkill->GetProto()) && IsRole() )
				{
					Role* pRole = reinterpret_cast<Role*>(this);

					NET_SIS_update_skill send;
					pTargetSkill->GenerateMsgInfo(&send.Skill);
					pRole->SendMessage(&send, send.dw_size);
				}
			}
		
		}
		break;

		// Ӱ��Buff
	case ESTT_Buff:
		{
			RegisterBuffModifier(pSkill->GetTargetBuffID(), pSkill);
		}
		break;

		// Ӱ��Trigger
	case ESTT_Trigger:
		{
			RegisterTriggerModifier(pSkill);
		}
		break;

	default:
		break;
	}
}

//------------------------------------------------------------------------------
// ���㵱ǰ�����б��е����м��ܶ�ĳ�����ܵ�Ӱ��
//------------------------------------------------------------------------------
VOID Unit::AddSkillBeMod(Skill* pSkill)
{
	// �鿴��ǰ�����б����Ƿ������Ӱ��ü��ܵļ���
	tagSkillModify* pModify = AttRes::GetInstance()->GetSkillModifier(pSkill->GetID());

	if( !VALID_POINT(pModify) || pModify->listModify.empty() )
		return;

	// ͨ���б���Ҽ����б�������Ӱ�켼��
	package_list<DWORD>& list = pModify->listModify;

	package_list<DWORD>::list_iter it = list.begin();

	Skill* pModSkill = NULL;
	DWORD dwModSkillID = INVALID_VALUE;
	while( list.find_next(it, dwModSkillID) )
	{
		pModSkill = m_mapSkill.find(dwModSkillID);
		if( !VALID_POINT(pModSkill) ) continue;

		// ����һ�����ܣ�����Ӱ��
		pSkill->SetMod(pModSkill->GetProto());
	}
}

//-----------------------------------------------------------------------------
// ȥ��ĳ�����ܵ�Ӱ��
//-----------------------------------------------------------------------------
VOID Unit::RemoveSkillMod(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;

	ESkillTargetType eTarget = pSkill->GetTargetType();

	switch(eTarget)
	{
		// Ӱ�켼��
	case ESTT_Skill:
		{
			const tagSkillProto* pSkillProto = pSkill->GetProto();
			if( !VALID_POINT(pSkillProto) ) return;

			for (int i = 0; i < MAX_TARGET_NUM; i ++)
			{
				Skill* pTargetSkill = m_mapSkill.find(pSkillProto->dwTargetSkillID[i]);
				if( !VALID_POINT(pTargetSkill) ) return;

				if( pTargetSkill->UnSetMod(pSkill->GetProto()) && IsRole() )
				{
					Role* pRole = reinterpret_cast<Role*>(this);

					NET_SIS_update_skill send;
					pTargetSkill->GenerateMsgInfo(&send.Skill);
					pRole->SendMessage(&send, send.dw_size);
				}
			}
		}
		break;

		// Ӱ��Buff
	case ESTT_Buff:
		{
			UnRegisterBuffModifier(pSkill->GetTargetBuffID(), pSkill);
		}
		break;

		// Ӱ��Trigger
	case ESTT_Trigger:
		{
			UnRegisterTriggerModifier(pSkill);
		}
		break;

	default:
		break;
	}
}

//----------------------------------------------------------------------------------
// ����һ������
//----------------------------------------------------------------------------------
VOID Unit::AddSkill(Skill* pSkill, BOOL bSendMsg)
{
	ASSERT( VALID_POINT(pSkill) );

	pSkill->SetOwner(this);

	AddSkillMod(pSkill);
	AddSkillBeMod(pSkill);

	
	m_mapSkill.add(pSkill->GetID(), pSkill);
	if( pSkill->GetCoolDownCountDown() > 0 )
	{
		m_listSkillCoolDown.push_back(pSkill->GetID());
	}

	pSkill->SetOwnerMod();

	// ������������ܣ����������е��������ܸ�����1
	if( ESSTE_Produce == pSkill->GetTypeEx() )
	{
		++m_nProduceSkillNum;
	}
}

//----------------------------------------------------------------------------------
// ȥ��һ������
//----------------------------------------------------------------------------------
VOID Unit::RemoveSkill(DWORD dwSkillID)
{
	Skill* pSkill = m_mapSkill.find(dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	// ������������ܣ����������е��������ܸ�����1
	if( ESSTE_Produce == pSkill->GetTypeEx() )
	{
		--m_nProduceSkillNum;
	}

	// ��������ɸü��ܲ���������buff
	RemoveAllBuffBelongSkill(pSkill->GetID(), FALSE);

	// ����Ӱ��
	pSkill->UnsetOwnerMod();
	m_mapSkill.erase(dwSkillID);
	m_listSkillCoolDown.erase(dwSkillID);

	pSkill->SetOwner(NULL);

	RemoveSkillMod(pSkill);	

	SAFE_DELETE(pSkill);
}

//----------------------------------------------------------------------------------
// �����������򽵼�
//----------------------------------------------------------------------------------
VOID Unit::ChangeSkillLevel(Skill* pSkill, ESkillLevelChange eType, INT nChange)
{
	if( !VALID_POINT(pSkill) ) return;
	if( 0 == nChange ) return;

	// ������ȥ����������Ӱ�죬��ȥ������������Ӱ��
	pSkill->UnsetOwnerMod();
	RemoveSkillMod(pSkill);

	// �ı似�ܵȼ�
	if( nChange > 0 )
		pSkill->IncLevel(eType, nChange);
	else
		pSkill->DecLevel(eType, nChange);

	// ���¼�������Ӱ��
	AddSkillMod(pSkill);
	pSkill->SetOwnerMod();
}

//----------------------------------------------------------------------------------
// �ı似��������
//----------------------------------------------------------------------------------
VOID Unit::ChangeSkillExp(Skill *pSkill, INT nValue)
{
	if( !VALID_POINT(pSkill) )	return;
	if( nValue <= 0 )	return;
	//if( ESLUT_Exp != pSkill->GetLevelUpType() ) return;
	if( pSkill->get_level() == pSkill->GetMaxLevel() ) return;
	
	// ������
	if ( IsRole() )
	{
		Role* pRole = static_cast<Role*>(this);
		FLOAT fEarnRate = pRole->GetEarnRate() / 10000.0f;
		nValue = INT(nValue * fEarnRate);
	}

	if( nValue <= 0 ) return;

	// �Ƿ�������
	BOOL bLevelChanged = FALSE;

	// �õ��ü��ܵ���������
	INT nRemainProficency = pSkill->GetLevelUpExp() - pSkill->GetProficiency();

	// ����þ���С����������
	if( nValue < nRemainProficency )
	{
		pSkill->AddProficiency(nValue);
	}
	// ����þ�����ڵ�����������
	else
	{
		// �鿴��Ҫ������
		while( nValue >= nRemainProficency )
		{
			// �Ѿ�������ߵȼ�
			if( pSkill->get_level() >= pSkill->GetMaxLevel() )
				break;

			nValue -= nRemainProficency;
			ChangeSkillLevel(pSkill, ESLC_Self);
			if( !bLevelChanged ) bLevelChanged = TRUE;

			// �õ���һ������������
			nRemainProficency = pSkill->GetLevelUpExp();
		}

		// �鿴������ۼӾ���
		if( nValue >= nRemainProficency )
		{
			pSkill->SetProficiency(nRemainProficency);
		}
		else
		{
			pSkill->SetProficiency(nValue);
		}
	}

	// �����������������Ըı�
	if( bLevelChanged && NeedRecalAtt() )
	{
		RecalAtt();
	}
}

//----------------------------------------------------------------------------------
// ������CD
//----------------------------------------------------------------------------------
VOID Unit::StartSkillCoolDown(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;
	
	DWORD dwSkillID = pSkill->GetID();

	pSkill->StartCoolDown();
	
	if (m_listSkillCoolDown.is_exist(dwSkillID)) return;

	m_listSkillCoolDown.push_back(dwSkillID);
}

//----------------------------------------------------------------------------------
// ���ĳ�����ܵ�CD
//----------------------------------------------------------------------------------
VOID Unit::ClearSkillCoolDown(DWORD dwSkillID)
{
	Skill* pSkill = GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	pSkill->ClearCoolDown();
	m_listSkillCoolDown.erase(dwSkillID);
}

//------------------------------------------------------------------------------
// ��������״̬����
//------------------------------------------------------------------------------
BOOL Unit::OnActiveSkillBuffTrigger(Skill* pSkill, package_list<DWORD>& listTarget, ETriggerEventType eEvent, 
									DWORD dwEventMisc1/*=INVALID_VALUE*/, DWORD dwEventMisc2/*=INVALID_VALUE*/)
{
	// ���ܲ����ڻ��߲�����������
	if( !VALID_POINT(pSkill) || FALSE == pSkill->IsActive() ) return FALSE;

	// ���ܽű�
	const SkillScript* pScript = pSkill->GetSkillScript();
	if (VALID_POINT(pScript))
	{
		BOOL bIgnore = FALSE;
		pScript->CastSkill(get_map(), pSkill->GetID(), pSkill->get_level(), GetID(), bIgnore);
		if (bIgnore)
		{
			return TRUE;
		}
	}

	// ��鼼������skillbuff���������Ա��е�buffid
	for(INT n = 0; n < MAX_BUFF_PER_SKILL; ++n)
	{
		DWORD dwBuffTypeID = pSkill->GetBuffTypeID(n);
		DWORD dwTriggerID = pSkill->GetTriggerID(n);
		if( !VALID_POINT(dwBuffTypeID) || !VALID_POINT(dwTriggerID) ) continue;

		const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
		const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
		if( !VALID_POINT(pBuffProto) || !VALID_POINT(pTriggerProto) ) continue;
		

		// �鿴trigger�����Ƿ���ͬ����Ҫ��Ϊ���Ż�
		if( ETEE_Null != pTriggerProto->eEventType && eEvent != pTriggerProto->eEventType )
			continue;

		// ͨ��buff����Ӷ���������
		if (listTarget.empty())
			continue;
		
		BOOL bSelfAdd = FALSE;
		DWORD dwTargetUnitID = INVALID_VALUE;
		package_list<DWORD>::list_iter it = listTarget.begin();
		while( listTarget.find_next(it, dwTargetUnitID) )
		{
			Unit* pTarget = get_map()->find_unit(dwTargetUnitID);
			if( !VALID_POINT(pTarget) ) continue;

			pTarget->TryAddBuff(this, pBuffProto, pTriggerProto, pSkill, NULL, TRUE, dwEventMisc1, dwEventMisc2);
			if (pTarget == this)
				bSelfAdd = TRUE;
		}
		
		if (bSelfAdd)
			continue;
		// ���Ը��������
		// ������������buff��ӹ���
		// ��1��������ܵ�Ŀ����������buff����Ӷ���������ֻҪ��������ͳ������
		// ��2��������ܵ�Ŀ������������buff����Ӷ���Ϊ����ʱ�ų������
		DWORD dwSkillTargetUnitID = GetCombatHandler().GetTargetUnitID();
		if( INVALID_VALUE == dwSkillTargetUnitID || GetID() == dwSkillTargetUnitID )
		{
			if( pBuffProto->dwTargetAddLimit & ETF_Self )
				TryAddBuff(this, pBuffProto, pTriggerProto, pSkill, NULL, TRUE, dwEventMisc1, dwEventMisc2);
		}
		else
		{
			if( pBuffProto->dwTargetAddLimit == ETF_Self )
				TryAddBuff(this, pBuffProto, pTriggerProto, pSkill, NULL, TRUE, dwEventMisc1, dwEventMisc2);
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------
// Buff����
//-----------------------------------------------------------------------------------
BOOL Unit::OnBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1/* =INVALID_VALUE */, DWORD dwEventMisc2/* =INVALID_VALUE */)
{
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		m_Buff[n].OnTrigger(pTarget, eEvent, dwEventMisc1, dwEventMisc2);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------
// ע��Buff�޸���
//-----------------------------------------------------------------------------------
VOID Unit::RegisterBuffModifier(DWORD dwBuffID, Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;
	if( ESTT_Buff != pSkill->GetTargetType() ) return;

	//DWORD dwBuffID = pSkill->GetTargetBuffID();
	if( FALSE == VALID_POINT(dwBuffID) ) return;

	package_list<DWORD>* pList = m_mapBuffModifier.find(dwBuffID);
	if( !VALID_POINT(pList) )
	{
		pList = new package_list<DWORD>;
		m_mapBuffModifier.add(dwBuffID, pList);
	}

	DWORD dwSkillTypeID = pSkill->GetTypeID();
	pList->push_back(dwSkillTypeID);				// ����ע����Ǽ��ܵ�����id
	
}

//-----------------------------------------------------------------------------------
// ����Buff�޸���
//-----------------------------------------------------------------------------------
VOID Unit::UnRegisterBuffModifier(DWORD dwBuffID, Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;
	if( ESTT_Buff != pSkill->GetTargetType() ) return;

	//DWORD dwBuffID = pSkill->GetTargetBuffID();
	if( FALSE == VALID_POINT(dwBuffID) ) return;

	package_list<DWORD>* pList = m_mapBuffModifier.find(dwBuffID);
	if( VALID_POINT(pList) )
	{
		DWORD dwSkillTypeID = pSkill->GetTypeID();
		pList->erase(dwSkillTypeID);
	}
}

//-----------------------------------------------------------------------------------
// ע��Trigger�޸���
//-----------------------------------------------------------------------------------
VOID Unit::RegisterTriggerModifier(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;
	if( ESTT_Trigger != pSkill->GetTargetType() ) return;

	DWORD dwTriggerID = pSkill->GetTargetTriggerID();
	if( FALSE == VALID_POINT(dwTriggerID) ) return;

	package_list<DWORD>* pList = m_mapTriggerModifier.find(dwTriggerID);
	if( !VALID_POINT(pList) )
	{
		pList = new package_list<DWORD>;
		m_mapTriggerModifier.add(dwTriggerID, pList);
	}
	DWORD dwSkillTypeID = pSkill->GetTypeID();
	pList->push_back(dwSkillTypeID);				// ����ע����Ǽ��ܵ�����id

}

//-----------------------------------------------------------------------------------
// ����Buff�޸���
//-----------------------------------------------------------------------------------
VOID Unit::UnRegisterTriggerModifier(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;
	if( ESTT_Trigger != pSkill->GetTargetType() ) return;

	DWORD dwTriggerID = pSkill->GetTargetTriggerID();
	if( FALSE == VALID_POINT(dwTriggerID) ) return;

	package_list<DWORD>* pList = m_mapTriggerModifier.find(dwTriggerID);
	if( VALID_POINT(pList) )
	{
		DWORD dwSkillTypeID = pSkill->GetTypeID();
		pList->erase(dwSkillTypeID);
	}
}

//-----------------------------------------------------------------------------------
// �������Buff
//-----------------------------------------------------------------------------------
BOOL Unit::TryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, 
					  Skill* pSkill, tagItem* pItem, BOOL bRecalAtt, DWORD dwEventMisc1, DWORD dwEventMisc2)
{

	if( !VALID_POINT(pBuffProto) ) return FALSE;

	// ͨ������ID�õ�ID�͵ȼ�
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);
	INT nBuffLevel = Buff::GetLevelFromTypeID(pBuffProto->dwID);

	// �ж�trigger
	if( VALID_POINT(pSrc) && VALID_POINT(pTriggerProto) )
	{
		if( !pSrc->TestTrigger(this, pTriggerProto, dwEventMisc1, dwEventMisc2) )
			return FALSE;
	}

	// �������ߴ��ڣ���������ߺ����������
	if( VALID_POINT(pSrc) )
	{
		// ���ȼ�������������
		DWORD dwTargetFlag = pSrc->TargetTypeFlag(this);
		if( !(pBuffProto->dwTargetAddLimit & dwTargetFlag) )
			return FALSE;

		// �����е����ж�
	}

	if (IsDead())
	{
		return false;
	}
	// �ټ��buff���״̬����
	//DWORD dwStatFlag = GetStateFlag();
	//if( (dwStatFlag & pBuffProto->dwTargetAddStateLimit) != dwStatFlag )
	//{
	//	return FALSE;
	//}

	// �жϿ�������
	//ERoleAttribute eReistType = BuffResistType2ERA(pBuffProto->eResistType);
	//if( ERA_Null != eReistType && ERA_Regain_Addtion != eReistType )
	//{
	//	// ����������
	//	// ��ʽ��������=��1-##����/200��
	//	INT nProp = 100 - INT((FLOAT)GetAttValue(eReistType) / 2.0f);
	//	if( nProp < 0 ) nProp = 0;
	//	if( (IUTIL->tool_rand() % 100) > nProp )
	//		return FALSE;
	//}

	// �жϵ����͵���
	INT nIndex = INVALID_VALUE;
	INT nRet = BuffCounteractAndWrap(pSrc, dwBuffID, nBuffLevel, pBuffProto->nLevel, pBuffProto->dwGroupFlag, pBuffProto->bBenifit, nIndex);

	if( EBCAWR_CanNotAdd == nRet ) return FALSE;
	else if( EBCAWR_Wraped == nRet ) return TRUE;

	ASSERT( EBCAWR_CanAdd == nRet );

	// ���buff
	AddBuff(pBuffProto, pSrc, (VALID_POINT(pSkill) ? pSkill->GetID() : INVALID_VALUE), pItem, nIndex, bRecalAtt);
	return TRUE;
}

//-----------------------------------------------------------------------------------
// ��GM�������Buff
//-----------------------------------------------------------------------------------
BOOL Unit::GMTryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, 
					  Skill* pSkill, tagItem* pItem, BOOL bRecalAtt, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	if( !VALID_POINT(pBuffProto) ) return FALSE;

	// ͨ������ID�õ�ID�͵ȼ�
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);
	INT nBuffLevel = Buff::GetLevelFromTypeID(pBuffProto->dwID);

	// �ж�trigger
	if( VALID_POINT(pSrc) && VALID_POINT(pTriggerProto) )
	{
		if( !pSrc->TestTrigger(this, pTriggerProto, dwEventMisc1, dwEventMisc2) )
			return FALSE;
	}

	// �������ߴ��ڣ���������ߺ����������
	//if( VALID_POINT(pSrc) )
	//{
	//	// ���ȼ�������������tbc:inves
	//	DWORD dwTargetFlag = pSrc->TargetTypeFlag(this);
	//	if( !(pBuffProto->dwTargetAddLimit & dwTargetFlag) )
	//		return FALSE;

	//	// �����е����ж�
	//}

	// �жϿ�������
	//ERoleAttribute eReistType = BuffResistType2ERA(pBuffProto->eResistType);
	//if( ERA_Null != eReistType && ERA_Regain_Addtion != eReistType )
	//{
	//	// ����������
	//	// ��ʽ��������=��1-##����/200��
	//	INT nProp = 100 - INT((FLOAT)GetAttValue(eReistType) / 2.0f);
	//	if( nProp < 0 ) nProp = 0;
	//	if( (IUTIL->tool_rand() % 100) > nProp )
	//		return FALSE;
	//}

	// �жϵ����͵���
	INT nIndex = INVALID_VALUE;
	INT nRet = BuffCounteractAndWrap(pSrc, dwBuffID, nBuffLevel, pBuffProto->nLevel, pBuffProto->dwGroupFlag, pBuffProto->bBenifit, nIndex);

	if( EBCAWR_CanNotAdd == nRet ) return FALSE;
	else if( EBCAWR_Wraped == nRet ) return TRUE;

	ASSERT( EBCAWR_CanAdd == nRet );

	// ���buff
	//for (int i= ERA_AttA_Start; i< ERA_End; i++)
	//{
	//	SetAttRecalFlag(i);
	//}
	AddBuff(pBuffProto, pSrc, (VALID_POINT(pSkill) ? pSkill->GetID() : INVALID_VALUE), pItem, nIndex, bRecalAtt);
	return TRUE;
}

//----------------------------------------------------------------------------------
// ����һ��״̬
//----------------------------------------------------------------------------------
VOID Unit::AddBuff(const tagBuffProto* pBuffProto, Unit* pSrc, DWORD dwSrcSkillID, const tagItem* pItem, INT nIndex, BOOL bRecalAtt)
{
	if( !VALID_POINT(pBuffProto) ) return;
	if( nIndex < 0 && nIndex >= MAX_BUFF_NUM ) return;
	
	// ͨ��BuffID�ҵ���Buff�Ƿ���Mod
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);

	// ���Ƿ���Ӱ���buff�ļ���
	//if (VALID_POINT(pBuffProto->dwBeModSkillID))
	//{
	//	Skill* pModSkill = GetSkill(pBuffProto->dwBeModSkillID);
	//	if (VALID_POINT(pModSkill))
	//	{
	//		RegisterBuffModifier(dwBuffID, pModSkill);
	//	}
	//}

	//// �ɾ���buff��������Ч��
	//if (pBuffProto->eType == EBT_Title)
	//{
	//	for (int i = 0; i < 5; i++)
	//	{
	//		Skill* pModSkill = GetSkill(ACHIEVEMENT_SIGNET_SKILL[i]);
	//		if (VALID_POINT(pModSkill))
	//		{
	//			RegisterBuffModifier(dwBuffID, pModSkill);
	//		}
	//	}
	//}

	package_list<DWORD>* pListModifier = NULL;
	if( VALID_POINT(pSrc) ) pListModifier = pSrc->GetBuffModifier(dwBuffID);

	// �����˲ʱbuff
	if( pBuffProto->bInstant )
	{
		// ����˲ʱЧ��
		if( !VALID_POINT(pListModifier) || pListModifier->empty() )
		{
			BuffEffect::CalBuffInstantEffect(this, pSrc, EBEM_Instant, pBuffProto, NULL);
		}
		else
		{
			// ��ʱ����һ��mod�ṹ
			tagBuffMod mod;
			package_list<DWORD>::list_iter it = pListModifier->begin();
			DWORD dwSkillTypeID = INVALID_VALUE;

			while( pListModifier->find_next(it, dwSkillTypeID) )
			{
				const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
				if( !VALID_POINT(pSkillProto) ) continue;

				mod.SetMod(pSkillProto);
			}

			BuffEffect::CalBuffInstantEffect(this, pSrc, EBEM_Instant, pBuffProto, &mod);
		}

		// ������Ϣ
		if( VALID_POINT(get_map()) )
		{
			NET_SIS_add_role_buffer send;
			send.dw_role_id = GetID();
			send.Buff.dwBuffTypeID = pBuffProto->dwID;
			send.Buff.nWarpTimes = 1;
			send.Buff.nMaxPersistTime = INVALID_VALUE;
			send.Buff.nPersistTimeLeft = INVALID_VALUE;

			get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
		}
	}
	else
	{
		m_Buff[nIndex].Init(pBuffProto, this, pSrc, dwSrcSkillID, pItem, nIndex, pListModifier);
		m_mapBuff.add(m_Buff[nIndex].GetID(), &m_Buff[nIndex]);

		// ������Ϣ
		if( get_map() )
		{
			NET_SIS_add_role_buffer send;
			send.dw_role_id = GetID();
			send.Buff.dwBuffTypeID = m_Buff[nIndex].GetTypeID();
			send.Buff.nWarpTimes = m_Buff[nIndex].GetWrapTimes();
			send.Buff.nMaxPersistTime = m_Buff[nIndex].GetPersistTime();
			send.Buff.nPersistTimeLeft = m_Buff[nIndex].GetPersistTimeLeft();

			if( VALID_POINT(get_map()) )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}
	}

	// ˫��������ң�����buff�������<Ares>
	if( VALID_POINT(pSrc) && pSrc->IsRole() && this->IsRole( ) &&  pBuffProto->bBenifit )
	{
		if( VALID_VALUE(((Role*)this)->GetTeamID()) && VALID_VALUE(((Role*)pSrc)->GetTeamID()) )
		{
			if( ((Role*)this)->GetTeamID( ) == ((Role*)pSrc)->GetTeamID( ) )
			{
				// ����Ǻ�������ʩ�ż����߽���ս��״̬
				if(((Role*)this)->HasPKValue( ) )
				{
					((Role*)pSrc)->SetRoleState(ERS_Combat);
					((Role*)pSrc)->GetCombatHandler().SetCombatStateCoolDown();
				}
			}
		}
	}

	// ���¼����������
	if( bRecalAtt && NeedRecalAtt() ) RecalAtt();
}


//----------------------------------------------------------------------------------
// ȥ��һ��״̬
//----------------------------------------------------------------------------------
INT Unit::RemoveBuff(DWORD dwBuffID, BOOL bRecalAtt, BOOL bSelf)
{
	Buff* pBuff = m_mapBuff.find(dwBuffID);
	if( !VALID_POINT(pBuff) ) return INVALID_VALUE;

	return RemoveBuff(pBuff, bRecalAtt, bSelf);
}

INT	 Unit::RemoveBuff(Buff* pBuff, BOOL bRecalAtt, BOOL bSelf /*= FALSE*/)
{
	ASSERT( VALID_POINT(pBuff) );
	
	// �õ�Buff��ǰ��״̬
	Buff::EBuffState eState = pBuff->GetState();

	// ����ɾ����Buff������Ҫ�ٴ���
	if( Buff::EBS_Destroying == eState )
	{
		return INVALID_VALUE;
	}

	// ��ǰ���ڳ�ʼ������µ�Buff��Ҫ��ӵ�ɾ���б�
	if( Buff::EBS_Initing	== eState ||
		Buff::EBS_Updating	== eState )
	{
		m_listDestroyBuffID.push_back(pBuff->GetID());
		return INVALID_VALUE;
	}

	// ��ǰ�ڿ���״̬��Buff����������ɾ��
	if( get_map() )
	{
		NET_SIS_remove_role_buffer send;
		send.dw_role_id = GetID();
		send.dwBuffTypeID = pBuff->GetTypeID();

		if( VALID_POINT(get_map()) )
		{
			get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
		}
	}

	DWORD dwBuffInterruptID = pBuff->GetBuffInterruptID();

	INT nIndex = pBuff->GetIndex();
	m_mapBuff.erase(pBuff->GetID());
	pBuff->Destroy(bSelf);

	// ɾ������BuffID
	if( VALID_POINT(dwBuffInterruptID) )
	{
		RemoveBuff(dwBuffInterruptID, FALSE);
	}

	// ���¼����������
	if( bRecalAtt && NeedRecalAtt() ) RecalAtt();

	return nIndex;

}
//-----------------------------------------------------------------------------------
// ȥ��������ĳ�����ܲ�����buff
//-----------------------------------------------------------------------------------
VOID Unit::RemoveAllBuffBelongSkill(DWORD dwSkillID, BOOL bRecalAtt)
{
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		// �������Լ�ʩ�ŵĲ����ɸü��ܲ���
		if( m_Buff[n].GetSrcUnitID() != GetID() || m_Buff[n].GetSrcSkillID() != dwSkillID )
			continue;

		RemoveBuff(m_Buff[n].GetID(), FALSE);
	}

	if( bRecalAtt && NeedRecalAtt() )
	{
		RecalAtt();
	}
}

//-----------------------------------------------------------------------------------
// ȥ������Buff
//-----------------------------------------------------------------------------------
VOID Unit::RemoveAllBuff()
{
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		RemoveBuff(&m_Buff[n], FALSE);
	}

	if( NeedRecalAtt() )
		RecalAtt();
}

//-----------------------------------------------------------------------------------
// ȥ������������к�Buff
//-----------------------------------------------------------------------------------
VOID Unit::RemoveAllBuff(BOOL bBenefit)
{
	INT nStart = 0, nEnd = MAX_BENEFIT_BUFF_NUM;

	if( !bBenefit )
	{
		nStart = MAX_BENEFIT_BUFF_NUM;
		nEnd = MAX_BUFF_NUM;
	}

	INT nIndex = INVALID_VALUE;
	for(INT n = nStart; n < nEnd; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;
		RemoveBuff(&m_Buff[n], FALSE);
	}

	if( NeedRecalAtt() )
		RecalAtt();
}

//-----------------------------------------------------------------------------------
// ����Buff
//-----------------------------------------------------------------------------------
VOID Unit::UpdateBuff()
{
	// ��ɾ��ɾ���б����Buff
	if( !m_listDestroyBuffID.empty() )
	{
		DWORD dwBuffID = m_listDestroyBuffID.pop_front();
		while( VALID_POINT(dwBuffID) )
		{
			RemoveBuff(dwBuffID, TRUE);

			dwBuffID = m_listDestroyBuffID.pop_front();
		}
	}

	// ���ȸ������д��ڵ�Buff
	for(INT n = 0; n < MAX_BUFF_NUM; n++)
	{
		if( !m_Buff[n].IsValid() ) continue;

		// ���ø�Buff��Update
		if( m_Buff[n].Update() )
		{
			RemoveBuff(m_Buff[n].GetID(), TRUE, TRUE);
		}
	}


}

//-----------------------------------------------------------------------------------
// �ֶ�ȡ��һ��Buff
//-----------------------------------------------------------------------------------
BOOL Unit::CancelBuff(DWORD dwBuffID)
{
	Buff* pBuff = GetBuffPtr(dwBuffID);
	if( !VALID_POINT(pBuff) ) return FALSE;

	// ���Ƿ����ֶ����
	if( !pBuff->Interrupt(EBIF_Manual) ) return FALSE;

	// ɾ��Buff
	RemoveBuff(dwBuffID, TRUE);

	return TRUE;
}

//-----------------------------------------------------------------------------------
// ���Buff�¼�
//-----------------------------------------------------------------------------------
VOID Unit::OnInterruptBuffEvent(EBuffInterruptFlag eFlag, INT nMisc/* =INVALID_VALUE */)
{
	BOOL bNeedRecal = FALSE;
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;
		if( !m_Buff[n].Interrupt(eFlag, nMisc) ) continue;
		
		RemoveBuff(m_Buff[n].GetID(), FALSE);
		bNeedRecal = TRUE;
	}

	if( bNeedRecal && NeedRecalAtt() )	RecalAtt();
}

//-----------------------------------------------------------------------------------
// ��ɢ���һ��Buff������ָ����Buff����Debuff��
//-----------------------------------------------------------------------------------
VOID Unit::DispelBuff(BOOL bBenefit)
{
	INT nStart = 0, nEnd = MAX_BENEFIT_BUFF_NUM;

	if( !bBenefit )
	{
		nStart = MAX_BENEFIT_BUFF_NUM;
		nEnd = MAX_BUFF_NUM;
	}

	INT nIndex = INVALID_VALUE;
	for(INT n = nStart; n < nEnd; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;
		nIndex = n;
	}

	if( INVALID_VALUE != nIndex )
	{
		RemoveBuff(m_Buff[nIndex].GetID(), TRUE);
	}
}

//-----------------------------------------------------------------------------------
// ��ɢ���һ����������ΪeType��Buff
//-----------------------------------------------------------------------------------
VOID Unit::DispelBuff(EBuffResistType eType)
{
	INT nIndex = INVALID_VALUE;
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		if( m_Buff[n].GetResistType() != eType ) continue;

		nIndex = n;
	}

	if( INVALID_VALUE != nIndex )
	{
		RemoveBuff(m_Buff[nIndex].GetID(), TRUE);
	}
}

//-----------------------------------------------------------------------------------
// ��ɢĳһ��ָ��ID��Buff
//-----------------------------------------------------------------------------------
VOID Unit::DispelBuff(DWORD dwBuffID)
{
	RemoveBuff(dwBuffID, TRUE);
}

//-----------------------------------------------------------------------------------
// ��ɢĳһ��ָ��Ч�����͵�buff
//-----------------------------------------------------------------------------------
VOID Unit::DispelBuff(INT nType, BOOL bLastOne)
{
	//if (!(eType == EBET_Dizzy || eType == EBET_NoSkill || eType == EBET_Spor || eType == EBET_Tie || eType == EBET_Invincible
	//	  || eType == EBET_DisArm || eType == EBET_NoPrepare || eType == EBET_IgnoreArmor || eType == EBET_Sneer || eType == EBET_Immunity))
	//	return;
	EBuffEffectType eType = EBET_Null;
	switch(nType)
	{
	case 1:
		eType = EBET_Dizzy;
		break;
	case 2:
		eType = EBET_NoSkill;
		break;
	case 3:
		eType = EBET_Spor;
		break;
	case 4:
		eType = EBET_Tie;
		break;
	case 5:
		eType = EBET_Invincible;
		break;
	case 6:
		eType = EBET_DisArm;
		break;
	case 7:
		eType = EBET_NoPrepare;
		break;
	case 8:
		eType = EBET_IgnoreArmor;
		break;
	case 9:
		eType = EBET_Sneer;
		break;
	case 10:
		eType = EBET_Immunity;
		break;
	default:
		break;
	}

	INT nIndex = INVALID_VALUE;
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		if( m_Buff[n].GetEffectType(EBEM_Persist) != eType ) continue;

		if(bLastOne)
			nIndex = n;
		else
			RemoveBuff(m_Buff[n].GetID(), TRUE);
	}

	if( INVALID_VALUE != nIndex && bLastOne)
	{
		RemoveBuff(m_Buff[nIndex].GetID(), TRUE);
	}
}

////-----------------------------------------------------------------------------------
//// ����Buff��˲ʱ��Ч����˲ʱ��Ч��������˲ʱЧ�����������Ч���ͽ���ʱЧ��
////-----------------------------------------------------------------------------------
//VOID Unit::CalBuffInstantEffect(Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes, Unit* pTriggerTarget)
//{
//	if( !VALID_POINT(pProto) ) return;
//	if( EBEM_Instant != eMode && EBEM_Inter != eMode && EBEM_Finish != eMode ) return;
//
//	// ˲ʱЧ�������ĵ��Ӵ���
//	if( EBEM_Instant == eMode ) nWrapTimes = 1;
//
//	BOOL bHaveMod = FALSE;
//	if( VALID_POINT(pMod) && pMod->IsValid() && pMod->eModBuffEffectMode == eMode )
//	{
//		bHaveMod = TRUE;
//	}
//
//	// ���Լӳ�Ӱ��
//	INT nAttMod[EBEA_End] = {0};
//	const INT* pnAttMod = NULL;
//	switch(eMode)
//	{
//	case EBEM_Instant:
//		pnAttMod = pProto->nInstantAttMod;
//		break;
//
//	case EBEM_Inter:
//		pnAttMod = pProto->nInterAttMod;
//		break;
//
//	case EBEM_Finish:
//		pnAttMod = pProto->nFinishAttMod;
//		break;
//
//	default:
//		break;
//	}
//
//	// Ч��
//	EBuffEffectType eEffect = pProto->eEffect[eMode];
//	DWORD dwEffectMisc1 = pProto->dwEffectMisc1[eMode];
//	DWORD dwEffectMisc2 = pProto->dwEffectMisc2[eMode];
//
//	// mod�����Ӱ��
//	if( bHaveMod )
//	{
//		for(INT n = 0; n < EBEA_End; ++n)
//		{
//			nAttMod[n] = pnAttMod[n] + pMod->nEffectAttMod[n];
//
//			if( abs(nAttMod[n]) > 100000 )	// �ٷֱ�
//			{
//				INT nAtt = 0;
//				switch(n)
//				{
//				case EBEA_HP:
//					nAtt = GetAttValue(ERA_MaxHP);
//					break;
//
//				case EBEA_MP:
//					nAtt = GetAttValue(ERA_MaxMP);
//					break;
//
//				case EBEA_Vitality:
//					nAtt = GetAttValue(ERA_MaxVitality);
//					break;
//
//				case ERA_Endurance:
//					nAtt = GetAttValue(ERA_MaxEndurance);
//					break;
//
//				default:
//					break;
//				}
//
//				nAttMod[n] = (nAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAtt * (FLOAT(abs(nAttMod[n]) - 100000) / 10000.0f));
//			}
//
//			nAttMod[n] *= nWrapTimes;
//		}
//		dwEffectMisc1 += pMod->nEffectMisc1Mod;
//		dwEffectMisc2 += pMod->nEffectMisc2Mod;
//	}
//	else
//	{
//		for(INT n = 0; n < EBEA_End; ++n)
//		{
//			nAttMod[n] = pnAttMod[n];
//
//			if( abs(nAttMod[n]) > 100000 )	// �ٷֱ�
//			{
//				INT nAtt = 0;
//				switch(n)
//				{
//				case EBEA_HP:
//					nAtt = GetAttValue(ERA_MaxHP);
//					break;
//
//				case EBEA_MP:
//					nAtt = GetAttValue(ERA_MaxMP);
//					break;
//
//				case EBEA_Vitality:
//					nAtt = GetAttValue(ERA_MaxVitality);
//					break;
//
//				case ERA_Endurance:
//					nAtt = GetAttValue(ERA_MaxEndurance);
//					break;
//
//				default:
//					break;
//				}
//
//				nAttMod[n] = (nAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAtt * (FLOAT(abs(nAttMod[n]) - 100000) / 10000.0f));
//			}
//
//			nAttMod[n] *= nWrapTimes;
//		}
//	}
//
//	// ���ݹ������ͣ���������͹�����Χ��ȷ����Χ��
//	package_list<Unit*> listTargetUnit;
//	CalBuffTargetList(pSrc, pProto->eOPType, pProto->fOPDistance, pProto->fOPRadius, 
//						pProto->eFriendly, pProto->dwTargetLimit, pProto->dwTargetStateLimit, listTargetUnit, pTriggerTarget);
//
//	Unit* pTarget = listTargetUnit.PopFront();
//	while( VALID_POINT(pTarget) )
//	{
//		// ��������Ӱ��
//		for(INT n = 0; n < EBEA_End; ++n)
//		{
//			if( 0 == nAttMod[n] ) continue;
//
//			switch(n)
//			{
//			case EBEA_HP:
//				{
//					// ����ǻָ���buff
//					if (pProto->eResistType == EBRT_Regain)
//					{
//						ASSERT(pSrc->m_nAttMod[ERA_Regain_Addtion] > -100 && pSrc->m_nAttMod[ERA_Regain_Addtion] < 100);
//						FLOAT f = (1 + (FLOAT(pSrc->m_nAttMod[ERA_Regain_Addtion])/200)) * (1 + ((FLOAT)(this->m_nAttMod[ERA_Regain_Addtion])/200));
//						nAttMod[n] = (FLOAT)nAttMod[n] * f;
//					}
//
//					if( pTarget->IsCreature() )
//					{
//						Creature* pCreature = static_cast<Creature*>(pTarget);
//						if( !VALID_POINT(pCreature) )	break;
//
//						pCreature->OnBuffInjury(pSrc);
//					}
//
//					pTarget->ChangeHP(nAttMod[n], pSrc, NULL, pProto);
//				}
//				break;
//
//			case EBEA_MP:
//				{
//					pTarget->ChangeMP(nAttMod[n]);
//				}
//				break;
//
//			case EBEA_Rage:
//				{
//					pTarget->ChangeRage(nAttMod[n]);
//				}
//				break;
//
//			case EBEA_Vitality:
//				{
//					pTarget->ChangeVitality(nAttMod[n]);
//				}
//				break;
//
//			case EBEA_Endurance:
//				{
//					pTarget->ChangeEndurance(nAttMod[n]);
//				}
//				break;
//
//			case EBEA_Morale:
//				{
//					pTarget->ModAttValue(ERA_Morale, nAttMod[n]);
//				}
//				break;
//
//			case EBEA_Injury:
//				{
//					pTarget->ModAttValue(ERA_Injury, nAttMod[n]);
//				}
//				break;
//
//			default:
//				break;
//			}
//		}
//
//		// ��������Ч��
//		if( EBET_Null != eEffect )
//		{
//			BuffEffect::CalBuffEffect(pTarget, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, TRUE, pProto);
//		}
//
//		// ��ȡһ��
//		pTarget = listTargetUnit.PopFront();
//	}
//}
//
////-----------------------------------------------------------------------------------
//// ����Buff�ĳ�����Ч����ֻ�ܶ���������
////-----------------------------------------------------------------------------------
//VOID Unit::CalBuffPersistEffect(Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes, BOOL bSet/* =TRUE */)
//{
//	if( !VALID_POINT(pProto) ) return;
//
//	BOOL bHaveMod = FALSE;
//	if( VALID_POINT(pMod) && pMod->IsValid() && EBEM_Persist == pMod->eModBuffEffectMode )
//	{
//		bHaveMod = TRUE;
//	}
//
//	EBuffEffectType eEffect = pProto->eEffect[EBEM_Persist];
//	DWORD dwEffectMisc1 = pProto->dwEffectMisc1[EBEM_Persist];
//	DWORD dwEffectMisc2 = pProto->dwEffectMisc2[EBEM_Persist];
//
//	if( bHaveMod )
//	{
//		dwEffectMisc1 += pMod->nEffectMisc1Mod;
//		dwEffectMisc2 += pMod->nEffectMisc2Mod;
//	}
//
//	// �ȼ������Լӳ�
//	ERoleAttribute eAtt = ERA_Null;
//	INT nValue = 0;
//	package_map<ERoleAttribute, INT>::map_iter it;
//	package_map<ERoleAttribute, INT>::map_iter itPct;
//
//	// �ȼ��㾲̬���Ե�
//	it = pProto->mapRoleAttMod.Begin();
//	while( pProto->mapRoleAttMod.find_next(it, eAtt, nValue) )
//	{
//		ModAttModValue(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
//	}
//
//	itPct = pProto->mapRoleAttModPct.Begin();
//	while( pProto->mapRoleAttModPct.find_next(itPct, eAtt, nValue) )
//	{
//		ModAttModValuePct(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
//	}
//
//	// �ڼ���mod��
//	if( bHaveMod )
//	{
//		it = pMod->mapRoleAttMod.Begin();
//		while( pMod->mapRoleAttMod.find_next(it, eAtt, nValue) )
//		{
//			ModAttModValue(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
//		}
//
//		itPct = pMod->mapRoleAttModPct.Begin();
//		while( pMod->mapRoleAttModPct.find_next(itPct, eAtt, nValue) )
//		{
//			ModAttModValuePct(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
//		}
//	}
//
//	// �ټ���Ч��
//	if( EBET_Null != eEffect )
//	{
//		BuffEffect::CalBuffEffect(this, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, bSet, pProto);
//	}
//}
//
//-----------------------------------------------------------------------------------
// ����Buff�ĳ�����Ч��
//-----------------------------------------------------------------------------------
VOID Unit::WrapBuffPersistEffect(Buff* pBuff, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod)
{
	BuffEffect::CalBuffPersistEffect(pBuff, this, pSrc, pProto, pMod, 1, TRUE);
}
//-----------------------------------------------------------------------------------
//����⻷buff
//-----------------------------------------------------------------------------------
VOID Unit::ClearTargetRingBuff()
{
	for (std::size_t i = 0; i < m_ListRingBuffTarget.size();)
	{
		Unit* pUnit = get_map()->find_unit(m_ListRingBuffTarget[i]);
		if (VALID_POINT(pUnit))
		{
			const tagBuffProto* pBuff = AttRes::GetInstance()->GetBuffProto(m_nRingBuffID);
			if (VALID_POINT(pBuff))
			{
				pUnit->RemoveBuff(Buff::GetIDFromTypeID(pBuff->dwEffectMisc1[EBEM_Persist]), TRUE);
			}
		}

		m_ListRingBuffTarget.erase(m_ListRingBuffTarget.begin() + i);	
	}
}

//-----------------------------------------------------------------------------------
// ����Buff�����ö���
//-----------------------------------------------------------------------------------
VOID Unit::CalBuffTargetList(Unit* pSrc, EBuffOPType eOPType, FLOAT fOPDist, FLOAT fOPRadius, 
							 EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit, std::vector<Unit*>& listTarget, Unit* pTarget)
{
	// ���������Ѿ��������ˣ�ֱ�ӷ���
	if( !VALID_POINT(pSrc) ) return;

	// ���ȼ��һ������
	if( pSrc->IsBuffTargetValid(this, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
	{
		listTarget.push_back(this);
	}

	// �ټ��һ��Ŀ��
	if( VALID_POINT(pTarget) && pSrc->IsBuffTargetValid(pTarget, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
	{
		listTarget.push_back(pTarget);
	}

	// Ԥ�ȼ��һ�¹�����Χ
	if( 0.0f == fOPRadius ) return;

	// ���������Χ��Ϊ0������Ŀ��Ϊ���ļ��
	FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

	tagVisTile* pVisTile[EUD_end] = {0};

	// �õ�������Χ�ڵ�vistile�б�
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Role*		pRole		= NULL;
	Creature*	pCreature	= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// ���ȼ������
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			if( pRole == this ) continue;

			if( FALSE == pSrc->IsBuffTargetValid(pRole, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
				continue;

			// �����ж�
			FLOAT fDistSQ = Vec3DistSq(GetCurPos(), pRole->GetCurPos());
			if( fDistSQ > fOPRadiusSQ  ) continue;

			// ���߼��

			// �ж�ͨ��������Ҽ��뵽�б���
			listTarget.push_back(pRole);
		}

		// �ټ������
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
			if( pCreature == this ) continue;

			if( FALSE == pSrc->IsBuffTargetValid(pCreature, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
				continue;

			// �����ж�
			FLOAT fDistSQ = Vec3DistSq(GetCurPos(), pCreature->GetCurPos());
			if( fDistSQ > fOPRadiusSQ  ) continue;

			// ���߼��

			// �ж�ͨ������������뵽�б���
			listTarget.push_back(pCreature);
		}
	}
}

//-----------------------------------------------------------------------------------
// ȡ��ָ������ʩ�ŵ�buff
//-----------------------------------------------------------------------------------
Buff* Unit::GetRelativeSkillBuff(DWORD dwSkillID, BOOL bBenefit /*= TRUE*/)
{
	INT nStart = 0;
	INT nEnd = MAX_BENEFIT_BUFF_NUM;

	if( !bBenefit )
	{
		nStart = MAX_BENEFIT_BUFF_NUM;
		nEnd = MAX_BUFF_NUM;
	}

	for(INT n = nStart; n < nEnd; n++)
	{
		// �յ�λ��
		if( !m_Buff[n].IsValid() )
		{
			continue;
		}
		
		// ����ID��ͬ
		if (m_Buff[n].GetSrcSkillID() == dwSkillID)
		{
			return &m_Buff[n];
		}
	}

	return NULL;
}
//-----------------------------------------------------------------------------------
// �õ���һ������buff
//-----------------------------------------------------------------------------------
Buff* Unit::GetFiretBenifitBuff()
{
	INT nStart = 0;
	INT nEnd = MAX_BENEFIT_BUFF_NUM;

	for(INT n = nStart; n < nEnd; n++)
	{
		// �յ�λ��
		if( !m_Buff[n].IsValid() )
		{
			continue;
		}

		if (m_Buff[n].IsBenifit())
		{
			return &m_Buff[n];
		}
	}

	return NULL;
}
//-----------------------------------------------------------------------------------
// ���ĳ��Ŀ���ǲ���Buff�ĺϷ�Ŀ��
//-----------------------------------------------------------------------------------
BOOL Unit::IsBuffTargetValid(Unit* pTarget, EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit)
{
	if( !VALID_POINT(pTarget) ) return FALSE;

	// �����ж�
	DWORD dwTargetFlag = TargetTypeFlag(pTarget);
	if( !(dwTargetFlag & dwTargetLimit) )
		return FALSE;

	// ״̬�ж�
	DWORD dwTargetStateFlag = pTarget->GetStateFlag();
	if( (dwTargetStateFlag & dwTargetStateLimit) != dwTargetStateFlag )
		return FALSE;

	// �����ж�
	DWORD dwFriendEnemy = FriendEnemy(pTarget);
	if( EBFE_Friendly == eFriendEnemy )
	{
		if( !(dwFriendEnemy & ETFE_Friendly) )
			return FALSE;
	}
	else if( EBFE_Hostile == eFriendEnemy )
	{
		if( !(dwFriendEnemy & ETFE_Hostile) )
			return FALSE;
	}
	else if( EBFE_Independent == eFriendEnemy )
	{
		if( !(dwFriendEnemy & ETFE_Independent) )
			return FALSE;
	}
	else if( EBFE_All == eFriendEnemy )
	{
		if( !(dwFriendEnemy & (ETFE_Friendly | ETFE_Hostile | ETFE_Independent)) )
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	// �ж�ͨ��
	return TRUE;
}

//-----------------------------------------------------------------------------------
// ����Buff�ĵ����͵���
//-----------------------------------------------------------------------------------
INT Unit::BuffCounteractAndWrap(Unit* pSrc, DWORD dwBuffID, INT nBuffLevel, INT nGroupLevel, DWORD dwGroupFlag, BOOL bBenefit, INT& nIndex)
{
	// ���ȼ��ID
	Buff* pBuff = m_mapBuff.find(dwBuffID);

	// �������һ��ͬ����buff
	if( VALID_POINT(pBuff) && Buff::EBS_Idle == pBuff->GetState() )
	{
		if( pBuff->get_level() > nBuffLevel )
		{
			return EBCAWR_CanNotAdd;
		}
		else if( pBuff->get_level() == nBuffLevel )
		{
			// ��⿴���ܲ��ܵ���
			
			if( VALID_POINT(pSrc) /*&& pBuff->GetSrcUnitID() == pSrc->GetID()*/ )
			{
				pBuff->Wrap();
				if( NeedRecalAtt() ) RecalAtt();

				if( VALID_POINT(get_map()) )
				{
					NET_SIS_update_role_buffer send;
					send.dw_role_id = GetID();
					send.Buff.dwBuffTypeID = pBuff->GetTypeID();
					send.Buff.nWarpTimes = pBuff->GetWrapTimes();
					send.Buff.nMaxPersistTime = pBuff->GetPersistTime();
					send.Buff.nPersistTimeLeft = pBuff->GetPersistTimeLeft();

					get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
				}

				return EBCAWR_Wraped;
			}
			else
			{
				// ���������ؿ������
				nIndex = RemoveBuff(pBuff->GetID(), TRUE);
				ASSERT( INVALID_VALUE != nIndex );
				return EBCAWR_CanAdd;
			}
		}
		else
		{
			// ������buff�����ؿ������
			nIndex = RemoveBuff(pBuff->GetID(), TRUE);
			ASSERT( nIndex != INVALID_VALUE );
			return EBCAWR_CanAdd;
			
		}
	}
	// ���������һ��ͬ����buff
	else
	{
		INT nStart = 0;
		INT nEnd = MAX_BENEFIT_BUFF_NUM;

		if( !bBenefit )
		{
			nStart = MAX_BENEFIT_BUFF_NUM;
			nEnd = MAX_BUFF_NUM;
		}

		INT nBlankIndex = INVALID_VALUE;		// ��λ
		INT nFullIndex = INVALID_VALUE;		// ���Buff��ʱ���������Ա�����������
		for(INT n = nStart; n < nEnd; n++)
		{
			// �յ�λ��
			if( !m_Buff[n].IsValid() )
			{
				if( INVALID_VALUE == nBlankIndex )
				{
					nBlankIndex = n;
					if( INVALID_VALUE == dwGroupFlag ) break;
				}
				continue;
			}
			// group_flag��ͬ
			else if( (dwGroupFlag != INVALID_VALUE) && 
					 (m_Buff[n].GetGroupFlag() == dwGroupFlag) )
			{
				// �����ǰ������Idle״̬�����ܶ���
				if( Buff::EBS_Idle != m_Buff[n].GetState() )
				{
					return EBCAWR_CanNotAdd;
				}

				// �Ƚϵȼ����������ȼ���
				if( m_Buff[n].GetGroupLevel() > nGroupLevel )
				{
					return EBCAWR_CanNotAdd;
				}
				else
				{
					nIndex = RemoveBuff(m_Buff[n].GetID(), TRUE);
					return EBCAWR_CanAdd;
				}
			}
			// buff��ʱ����
			else if( INVALID_VALUE == nFullIndex && 
					 Buff::EBS_Idle == m_Buff[n].GetState() &&
					 m_Buff[n].Interrupt(EBIF_BuffFull) )
			{
				nFullIndex = n;
			}
		}

		if( INVALID_VALUE != nBlankIndex )
		{
			nIndex = nBlankIndex;
			return EBCAWR_CanAdd;
		}
		else if( INVALID_VALUE != nFullIndex )
		{
			RemoveBuff(m_Buff[nFullIndex].GetID(), TRUE);
			nIndex = nFullIndex;
			return EBCAWR_CanAdd;
		}
		else
		{
			return EBCAWR_CanNotAdd;
		}
	}
}

//------------------------------------------------------------------------------------------------------
// ����ĳ����ͨ�������Ƿ���������
//------------------------------------------------------------------------------------------------------
BOOL Unit::TestTrigger(Unit* pTarget, const tagTriggerProto* pProto, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	if( !VALID_POINT(pProto) ) return TRUE;

	// ���һ�¸ô������Ƿ��б�������Ӱ��
	package_list<DWORD>* pListModifier = m_mapTriggerModifier.find(pProto->dwID);

	if( !VALID_POINT(pListModifier) || pListModifier->empty() )
	{
		// û��Ӱ�죬ֱ�Ӽ���
		return TestEventTrigger(pTarget, pProto, NULL, dwEventMisc1, dwEventMisc2) &&
			   TestStateTrigger(pTarget, pProto, NULL);
	}
	else
	{
		// ���㴥����Ӱ��
		tagTriggerMod mod;

		package_list<DWORD>::list_iter it = pListModifier->begin();
		DWORD dwSkillTypeID = INVALID_VALUE;

		while( pListModifier->find_next(it, dwSkillTypeID) )
		{
			const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
			if( !VALID_POINT(pSkillProto) ) continue;

			mod.SetMod(pSkillProto);
		}

		// ������Ӱ�������ϣ���ʼ���㴥�������
		return TestEventTrigger(pTarget, pProto, &mod, dwEventMisc1, dwEventMisc2) &&
			   TestStateTrigger(pTarget, pProto, &mod);
	}
}

//----------------------------------------------------------------------------------------------------------
// ����ĳ�����������¼������Ƿ�����
//----------------------------------------------------------------------------------------------------------
BOOL Unit::TestEventTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	// û���¼�Ҫ����ֱ�ӷ��سɹ�
	if( ETEE_Null == pProto->eEventType ) return TRUE;

	// ����Ҫ�жϴ����ʣ���ֱ�ӷ��سɹ�
	if (m_bTriggerOff == TRUE)
		return TRUE;

	// ���������Լ���
	INT nProp = pProto->nEventProp + ( VALID_POINT(pMod) ? pMod->nPropMod : 0 );
	if( get_tool()->tool_rand() % 10000 > nProp ) return FALSE;

	// ����������ĳЩ������¼������������жϣ�Ŀǰû����Ҫ�����жϵ�

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------
// ����ĳ����������״̬�����Ƿ�����
//------------------------------------------------------------------------------------------------------------
BOOL Unit::TestStateTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod)
{
	if( ETEE_Null == pProto->eStateType ) return TRUE;

	DWORD dwStateMisc1 = pProto->dwStateMisc1 + ( VALID_POINT(pMod) ? pMod->nStateMisc1Mod : 0 );
	DWORD dwStateMisc2 = pProto->dwStateMisc2 + ( VALID_POINT(pMod) ? pMod->nStateMisc2Mod : 0 );

	// �鿴�����trigger���������ж�
	switch(pProto->eStateType)
	{
		// ��������
	case ETST_HPHigher:
		{
			INT nHPLimit = (INT)dwStateMisc1;

			if( nHPLimit < 100000 )
			{
				return GetAttValue(ERA_HP) > nHPLimit;
			}
			else
			{
				if( GetAttValue(ERA_MaxHP) > 0 )
					return GetAttValue(ERA_HP) * 10000 / GetAttValue(ERA_MaxHP) >= (nHPLimit - 100000);
				else
					return FALSE;
			}
		}
		break;
		// Ŀ����������
	case ETST_TargetHPHigher:
		{
			if( !VALID_POINT(pTarget) ) return FALSE;

			INT nHPLimit = (INT)dwStateMisc1;

			if( nHPLimit < 100000 )
			{
				return pTarget->GetAttValue(ERA_HP) > nHPLimit;
			}
			else
			{
				if( pTarget->GetAttValue(ERA_MaxHP) > 0 )
					return pTarget->GetAttValue(ERA_HP) * 10000 / pTarget->GetAttValue(ERA_MaxHP) >= (nHPLimit - 100000);
				else
					return FALSE;
			}
		}
		break;
		// ��������
	case ETST_HPLower:
		{
			INT nHPLimit = (INT)dwStateMisc1;

			if( nHPLimit < 100000 )
			{
				return GetAttValue(ERA_HP) < nHPLimit;
			}
			else
			{
				if( GetAttValue(ERA_MaxHP) > 0 )
				{
					FLOAT f = GetAttValue(ERA_HP) * 1.0f / GetAttValue(ERA_MaxHP);

					return f * 10000 < (nHPLimit - 100000);
				}
				else
					return FALSE;
			}
		}
		break;

		// Ŀ����������
	case ETST_TargetHPLower:
		{
			if( !VALID_POINT(pTarget) ) return FALSE;

			INT nHPLimit = (INT)dwStateMisc1;

			if( nHPLimit < 100000 )
			{
				return pTarget->GetAttValue(ERA_HP) < nHPLimit;
			}
			else
			{
				if( pTarget->GetAttValue(ERA_MaxHP) > 0 )
				{
					FLOAT f = pTarget->GetAttValue(ERA_HP) * 1.0f / pTarget->GetAttValue(ERA_MaxHP);

					return f * 10000 < (nHPLimit - 100000);
				}
				else
					return FALSE;
			}
		}
		break;
		// ��������
	case ETST_MPHigher:
		{
			INT nMPLimit = (INT)dwStateMisc1;

			if( nMPLimit < 100000 )
			{
				return GetAttValue(ERA_MP) > nMPLimit;
			}
			else
			{
				if( GetAttValue(ERA_MaxMP) > 0 )
					return GetAttValue(ERA_MP) * 10000 / GetAttValue(ERA_MaxMP) >= (nMPLimit - 100000);
				else
					return FALSE;
			}
		}
		break;

		// ��������
	case ETST_MPLower:
		{
			INT nMPLimit = (INT)dwStateMisc1;

			if( nMPLimit < 100000 )
			{
				return GetAttValue(ERA_MP) < nMPLimit;
			}
			else
			{
				if( GetAttValue(ERA_MaxMP) > 0 )
					return GetAttValue(ERA_MP) * 10000 / GetAttValue(ERA_MaxMP) < (nMPLimit - 100000);
				else
					return FALSE;
			}
		}
		break;

		// ŭ������
	//case ETST_RageHigher:
	//	{
	//		INT nRageLimit = (INT)dwStateMisc1;
	//		return GetAttValue(ERA_Rage) > nRageLimit;
	//	}
	//	break;

	//	// ŭ������
	//case ETST_RageLower:
	//	{
	//		INT nRageLimit = (INT)dwStateMisc1;
	//		return GetAttValue(ERA_Rage) < nRageLimit;
	//	}
	//	break;

	//	// �־�������
	//case ETST_EnduranceHigher:
	//	{
	//		INT nEnduranceLimit = (INT)dwStateMisc1;

	//		if( nEnduranceLimit < 100000 )
	//		{
	//			return GetAttValue(ERA_Endurance) > nEnduranceLimit;
	//		}
	//		else
	//		{
	//			if( GetAttValue(ERA_MaxEndurance) > 0 )
	//				return GetAttValue(ERA_Endurance) * 10000 / GetAttValue(ERA_MaxEndurance) >= (nEnduranceLimit - 100000);
	//			else
	//				return FALSE;
	//		}
	//	}
	//	break;

	//	// �־�������
	//case ETST_EnduranceLower:
	//	{
	//		INT nEnduranceLimit = (INT)dwStateMisc1;

	//		if( nEnduranceLimit < 100000 )
	//		{
	//			return GetAttValue(ERA_Endurance) < nEnduranceLimit;
	//		}
	//		else
	//		{
	//			if( GetAttValue(ERA_MaxEndurance) > 0 )
	//				return GetAttValue(ERA_Endurance) * 10000 / GetAttValue(ERA_MaxEndurance) < (nEnduranceLimit - 100000);
	//			else
	//				return FALSE;
	//		}
	//	}
	//	break;

		// ����ӵ��ĳbuff
	case ETST_SelfHaveBuff:
		{
			DWORD dwBuffID = dwStateMisc1;
			return IsHaveBuff(dwBuffID);
		}
		break;

		// Ŀ��ӵ��ĳbuff
	case ETST_TargetHaveBuff:
		{
			DWORD dwBuffID = dwStateMisc2;
			if( !VALID_POINT(pTarget) ) return FALSE;

			return pTarget->IsHaveBuff(dwBuffID);
		}
		break;
		// ������ĳ״̬
	case ETST_SelfHaveState:
		{
			DWORD dwState = dwStateMisc1;
			IsInState((EState)dwState);
		}
		break;
		// Ŀ����ĳ״̬
	case ETST_TargetHaveState:
		{
			DWORD dwState = dwStateMisc2;
			if( !VALID_POINT(pTarget) ) return FALSE;

			return pTarget->IsInState((EState)dwState);
		}
		break;
	default:
		break;
	}

	return FALSE;
}

//----------------------------------------------------------------------------------
// ��ӹ��ﵽ����ҳ�޵Ĺ����б�
//----------------------------------------------------------------------------------
VOID Unit::AddEnmityCreature(Unit *pUnit)
{
	if(FALSE == pUnit->IsCreature())
		return;

	m_mapEnmityCreature.add(pUnit->GetID(), pUnit->GetID());

	if(this->IsRole())
	{
		if(!((Role*)this)->IsInRoleState(ERS_Combat))
		{
			((Role*)this)->SetRoleState(ERS_Combat);
			((Role*)this)->StopMount();
		}
	}
}

//----------------------------------------------------------------------------------
// ɾ������Ӹ���ҳ�޵Ĺ����б�
//----------------------------------------------------------------------------------
VOID Unit::DelEnmityCreature(Unit *pUnit)
{
	if(FALSE == pUnit->IsCreature())
		return;

	m_mapEnmityCreature.erase(pUnit->GetID());
}

//----------------------------------------------------------------------------------
// �����ҳ�޵Ĺ����б�
//----------------------------------------------------------------------------------
VOID Unit::ClearEnmityCreature()
{
	DWORD	dwCreatureID = INVALID_VALUE;

	Creature* pCreature = (Creature*)INVALID_VALUE;
	m_mapEnmityCreature.reset_iterator();
	while (m_mapEnmityCreature.find_next(dwCreatureID))
	{
		pCreature = get_map()->find_creature(dwCreatureID);
		if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()))
			continue;

		pCreature->GetAI()->ClearEnmity(GetID());
	}

	m_mapEnmityCreature.clear();
}

//----------------------------------------------------------------------------------
// ���հٷֱȸı���
//----------------------------------------------------------------------------------
VOID Unit::ChangeEnmityCreatureValue(DWORD dwRate)
{
	DWORD	dwCreatureID	=	INVALID_VALUE;
	INT		nEnmityMod		=	0;
	INT		nEnmityTotal	=	0;

	Creature* pCreature = (Creature*)INVALID_VALUE;
	m_mapEnmityCreature.reset_iterator();
	while (m_mapEnmityCreature.find_next(dwCreatureID))
	{
		pCreature = get_map()->find_creature(dwCreatureID);
		if( !VALID_POINT(pCreature) || !VALID_POINT(pCreature->GetAI()))
			continue;

		nEnmityTotal = pCreature->GetAI()->GetBaseEnmityValue(GetID());
		nEnmityMod = (FLOAT)nEnmityTotal * (10000.0 - (FLOAT)dwRate) / 10000.0;
		pCreature->GetAI()->AddEnmity(this, -nEnmityMod, FALSE);
	}
}

FLOAT Unit::GetXZSpeed()
{
	if (this->IsRole())
	{
		Role* pRole = static_cast<Role*>(this);
		if (pRole->IsInRoleStateAny(ERS_Mount | ERS_Mount2))
		{
			return m_fMountXZSpeed;
		}
		else if(pRole->IsInRoleState(ERS_Carry))
		{
			return m_fCarryXZSpeed;
		}
		else
		{
			return m_fXZSpeed;
		}
	}
	else
	{
		return m_fXZSpeed;
	}
}

// �����˺�����Ч��
INT Unit::OnAbsorDmg(INT nDmgValue)
{
	
	Buff* pBuff = NULL;
	m_mapBuff.reset_iterator();
	INT nBuffAbs = 0; //buff�˺�����
	INT nSubDmg = 0; //���պ���˺�
	INT nDmgAbs = 0; //ʣ����˺�������
	while (m_mapBuff.find_next(pBuff))
	{
		if (!VALID_POINT(pBuff))
			continue;

		if (nDmgValue >= 0)
			break;

		nBuffAbs = (INT)pBuff->GetAbsorbDmg();
		if (nBuffAbs <= 0)
			continue;

		nSubDmg = nBuffAbs + nDmgValue;
		if (nSubDmg > 0 )
		{
			nDmgValue = 0;
			nDmgAbs = nSubDmg;
		}
		else
		{
			nDmgValue = nSubDmg;
			nDmgAbs = 0;
		}

		pBuff->SetAbsorbDmg(nDmgAbs);
		if (nDmgAbs == 0)
		{
			RemoveBuff(pBuff->GetID(), FALSE);
		}
	}
	if (nDmgAbs == 0)
	{
		SetAbsorbDmg(FALSE);
	}
	//INT nSubDmg = (INT)m_nDmgAbs + nDmgValue;
	//nDmgValue = nSubDmg > 0 ? 0 : nSubDmg;
	//m_nDmgAbs = nSubDmg < 0 ? 0 : nSubDmg;
	return nDmgValue;
}
//-----------------------------------------------------------------------------------------
// �ı�Ѫ��
//-----------------------------------------------------------------------------------------
VOID Unit::ChangeHP( INT nValue, Unit* pSrc/* =NULL */, Skill* pSkill/* =NULL */, const tagBuffProto* pBuff/* =NULL */, bool bCrit/* =false */, bool bBlock/*=false*/, DWORD dwSerial/* =INVALID_VALUE */, DWORD dwMisc/* =INVALID_VALUE */, INT nOtherDmg/* = 0 */)
{
	INT nDmg = nValue;

	if( 0 == nDmg ) return;

	// �����˺��ű�
	if (IsCreature())
	{
		Creature* pCreature = ((Creature*)this);
		const CreatureScript* pScript = pCreature->GetScript();
		if (VALID_POINT(pScript))
		{
			DWORD dwSkillID = INVALID_VALUE;
			DWORD dwSkillLevel = INVALID_VALUE;
			if (VALID_POINT(pSkill))
			{
				dwSkillID = pSkill->GetID();
				dwSkillLevel = pSkill->get_level();
			}
			nDmg = pScript->OnBeAttack(pCreature, dwSkillID, dwSkillLevel, bBlock, bCrit, nDmg);
		} 
	}


	//�������Ѫ�Ļ�
	if (nDmg < 0)
	{
		//������˺���ѪЧ��
		if (m_bDmgReturn)
		{
			nDmg = -nDmg;
		}
		//�˺�����
		else if (ISAbsorbDmg())
		{
			nDmg = OnAbsorDmg(nDmg);
			//INT nSubDmg = (INT)m_nDmgAbs + nDmg;
			//nDmg = nSubDmg > 0 ? 0 : nSubDmg;
			//m_nDmgAbs = nSubDmg < 0 ? 0 : nSubDmg;
		}
	}

	ModAttValue(ERA_HP, nDmg);

	// �����ս��ϵͳ��ɵ�Ѫ���ı䣬����ս��������Ϣ
	if( VALID_POINT(pSkill) || VALID_POINT(pBuff) )
	{
		NET_SIS_role_hp_change send;
		send.dw_role_id = GetID();
		send.bBlocked = bBlock;
		send.bMiss = false;
		send.bCrit = bCrit;
		send.nHPChange = nDmg;
		send.nHPChangeOther = nOtherDmg;
		send.dwMisc2 = dwMisc;
		send.dwSerial = dwSerial;

		if( VALID_POINT(pSrc) )
		{
			send.dwSrcRoleID = pSrc->GetID();
		}
		else
		{
			send.dwSrcRoleID = INVALID_VALUE;
		}

		if( VALID_POINT(pSkill) )
		{
			send.eCause = ERHPCC_SkillDamage;
			send.dwMisc = pSkill->GetTypeID();
			send.dwMisc2 = dwMisc;
			send.dwSerial = dwSerial;
		}
		else
		{
			send.eCause = ERHPCC_BuffDamage;
			send.dwMisc = pBuff->dwID;
		}

		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}

	// ����Ѫ�¼�
	if( nDmg < 0 )
	{
		OnInterruptBuffEvent(EBIF_HPLower);		// ���Buff�¼�

		if( GetAttValue(ERA_HP) <= 0 )
		{
			OnDead(pSrc, pSkill, pBuff, dwSerial, dwMisc, bCrit);	// ����
		}
	}
}
//����״̬���
VOID Unit::UpdateState()
{
	if (IsInState(ES_Feat))
	{
		// �����ƶ�
		if( m_bIsFeating )
		{

			if( EMS_Stand == GetMoveData().m_eCurMove  )
			{
				// �Ѿ������յ㣬��ԭ����Ϣһ��ʱ��
				m_bIsFeating = FALSE;

				m_nFeatRestTick = 2 * TICK_PER_SECOND;
				//ReSetPatrolRestTick( );
				
			}
			return;
		}

		// ������Ϣ
		if( m_nFeatRestTick > 0 )
		{
			m_nFeatRestTick--;
			return;
		}

		// ��ʼ����Ѳ��
		StartFeatRun();
	}
}

VOID Unit::StartFeatRun()
{	
	Vector3 vDest = GetCurPos();

	INT nRange = 500 * 2;
	if( 0 >= nRange ) return;

	vDest.x = FLOAT(get_tool()->tool_rand() % nRange - 500) + GetCurPos().x;
	vDest.z = FLOAT(get_tool()->tool_rand() % nRange - 500) + GetCurPos().z;

	if( MoveData::EMR_Success == GetMoveData().StartFearRun(vDest) )
	{
		m_bIsFeating = TRUE;
	}
}

VOID Unit::SetTargetID(DWORD dwID) 
{
	if (dwID == m_dwTargetID)
		return;

	m_dwTargetID = dwID; 

	NET_SIS_target_change send;
	send.dw_unit_id = GetID();
	send.dw_target_id = m_dwTargetID;

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}