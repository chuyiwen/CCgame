/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����

#include "StdAfx.h"
#include "../../common/WorldDefine/skill_define.h"

#include "att_res.h"
#include "unit.h"
#include "role.h"
#include "skill_buff.h"
#include "skill_trigger.h"
#include "skill.h"
#include "role.h"

//------------------------------------------------------------------------------
// ���ܳ�ʼ��
//------------------------------------------------------------------------------
BOOL Skill::Init(DWORD dwID, INT nSelfLevel, INT nLearnLevel, INT nCoolDown, INT nProficiency, BOOL bFromOther)
{
	ASSERT( VALID_POINT(dwID) && nCoolDown >= 0 && nSelfLevel >= 0 && nLearnLevel >= 0 );
	ASSERT( nSelfLevel >= 0 || nLearnLevel >= 0 );

	INT nLevel = nSelfLevel + nLearnLevel;

	m_dwID = dwID;
	m_nLevel = nSelfLevel + nLearnLevel;
	m_nSelfLevel = nSelfLevel;
	m_nLearnLevel = nLearnLevel;
	m_nTempAddLevel = 0;
	m_nCoolDownCountDown = 0;//nCoolDown;
	m_nProficiency = nProficiency;
	m_bFromOther = bFromOther;

	m_pProto = AttRes::GetInstance()->GetSkillProto(CreateTypeID(m_dwID, m_nLevel));
	ASSERT( VALID_POINT(m_pProto) );
	if (!VALID_POINT(m_pProto) ) return FALSE;

	// ������ܱ���������Ӱ�죬������Mod�ṹ
	if( AttRes::GetInstance()->CanBeModified(m_dwID) )
	{
		m_pMod = new tagSkillMod;

		// ���������Ա��е�buff�����õ�mod����
		SetSkillBuff(m_pProto);
	}
	// ����Ͳ�����
	else
	{
		m_pMod = NULL;
	}

	m_pScript = g_ScriptMgr.GetSkillScript(m_dwID);
	m_pOwner = NULL;

	return TRUE;
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID Skill::Update()
{

}

//---------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------
VOID Skill::InitSkillSave(OUT s_skill_save *pSkillSave)
{
	pSkillSave->dw_id			=	m_dwID;
	pSkillSave->n_learn_level_		=	m_nLearnLevel;
	pSkillSave->n_self_level_		=	m_nSelfLevel;
	pSkillSave->n_proficiency_	=	m_nProficiency;
	pSkillSave->n_cool_down_		=	m_nCoolDownCountDown;
}

//----------------------------------------------------------------------------
// ���ɼ�����Ϣ�ṹ
//----------------------------------------------------------------------------
VOID Skill::GenerateMsgInfo(tagSkillMsgInfo* const pMsgInfo)
{
	pMsgInfo->dwID			=	GetID();
	pMsgInfo->nLevel		=	get_level();
	pMsgInfo->nProficiency	=	GetProficiency();
	pMsgInfo->nCoolDown		=	GetCoolDownCountDown();
	pMsgInfo->fOpDist		=	GetOPDist();

	// �˺�
	//for(INT n = 0; n < MAX_CHANNEL_TIMES; n++)
	//{
	//	pMsgInfo->nChannelDmg[n] = GetDmg(n);
	//}

	//// ����
	//for(INT n = 0; n < ESCT_End; n++)
	//{
	//	pMsgInfo->nCost[n] = GetCost((ESkillCostType)n);
	//}
}

//---------------------------------------------------------------------------------
// ���ü������Լӳɣ������Ƿ������Ա�����¹�
//---------------------------------------------------------------------------------
BOOL Skill::SetMod(const tagSkillProto* pProto)
{
	if( !VALID_POINT(pProto) ) return FALSE;
	if( ESTT_Skill != pProto->eTargetType ) return FALSE;

	// ��������
	m_pMod->fOPDistMod += pProto->fOPDist;

	// �����뾶
	m_pMod->fOPRadiusMod += pProto->fOPRadius;

	// ����ʱ��	
	m_pMod->nPrepareTimeMod += pProto->nPrepareTime;

	// ��ȴʱ��
	m_pMod->nCoolDownMod += pProto->nCoolDown;

	// ���
	if( abs(pProto->nEnmity) < 100000 )
	{
		m_pMod->nEnmityMod += pProto->nEnmity;
	}
	else
	{
		m_pMod->nEnmityModPct += (pProto->nEnmity > 0 ? 
			pProto->nEnmity - 100000 : pProto->nEnmity + 100000);
	}
	m_pMod->fEnmityParamMod += pProto->fEnmityParam;

	// ����
	if( abs(pProto->nHit) < 100000 )
	{
		m_pMod->nHitMod += pProto->nHit;
	}
	else
	{
		m_pMod->nHitModPct += (pProto->nHit > 0 ? 
			pProto->nHit - 100000 : pProto->nHit + 100000);
	}

	// ����
	if( abs(pProto->nCrit) < 100000 )
	{
		m_pMod->nCritMod += pProto->nCrit;
	}
	else
	{
		m_pMod->nCritModPct += (pProto->nCrit > 0 ? 
			pProto->nCrit - 100000 : pProto->nCrit + 100000);
	}

	// �˺�
	for(INT n = 0; n < MAX_CHANNEL_TIMES; n++)
	{
		if( abs(pProto->nChannelDmg[n]) < 100000 )
		{
			m_pMod->nChannelDmgMod[n] += pProto->nChannelDmg[n];
		}
		else
		{
			m_pMod->nChannelDmgModPct[n] += (pProto->nChannelDmg[n] > 0 ? 
				pProto->nChannelDmg[n] - 100000 : pProto->nChannelDmg[n] + 1000000);
		}
	}

	// ����
	for(INT n = 0; n < ESCT_End; n++)
	{
		if( abs(pProto->nSkillCost[n]) < 100000 )
		{
			m_pMod->nSkillCostMod[n] += pProto->nSkillCost[n];
		}
		else
		{
			m_pMod->nSkillCostModPct[n] += (pProto->nSkillCost[n] > 0 ? 
				pProto->nSkillCost[n] - 100000 : pProto->nSkillCost[n] + 1000000);
		}
	}

	// Buff��Trigger
	SetSkillBuff(pProto);

	// �������Լӳ�
	ERoleAttribute eAtt = ERA_Null;
	INT nMod = 0;

	package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
	package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
	while( mapMod.find_next(itMod, eAtt, nMod) )
	{
		m_pMod->mapRoleAttMod.modify_value(eAtt, nMod);
	}

	package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
	package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
	while( mapModPct.find_next(itModPct, eAtt, nMod) )
	{
		m_pMod->mapRoleAttModPct.modify_value(eAtt, nMod);
	}

	// ����ñ�Ӱ��ļ��ܱ�������������ϣ���Ҫ������������Ӱ��
	if( VALID_POINT(m_pOwner) && IsCanModAtt() )
	{
		package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
		package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
		while( mapMod.find_next(itMod, eAtt, nMod) )
		{
			m_pOwner->ModAttModValue(eAtt, nMod);
		}

		package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
		package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
		while( mapModPct.find_next(itModPct, eAtt, nMod) )
		{
			m_pOwner->ModAttModValuePct(eAtt, nMod);
		}
	}
	
	return TRUE;
}

//----------------------------------------------------------------------------------
// �����������Լӳɣ����ؼ������Ա����Ƿ���¹�
//----------------------------------------------------------------------------------
BOOL Skill::UnSetMod(const tagSkillProto* pProto)
{
	if( !VALID_POINT(m_pMod) ) return FALSE;
	if( ESTT_Skill != pProto->eTargetType ) return FALSE;

	// ��������
	m_pMod->fOPDistMod -= pProto->fOPDist;

	// �����뾶
	m_pMod->fOPRadiusMod -= pProto->fOPRadius;

	// ����ʱ��	
	m_pMod->nPrepareTimeMod -= pProto->nPrepareTime;

	// ��ȴʱ��
	m_pMod->nCoolDownMod -= pProto->nCoolDown;

	// ���
	if( abs(pProto->nEnmity) < 100000 )
	{
		m_pMod->nEnmityMod -= pProto->nEnmity;
	}
	else
	{
		m_pMod->nEnmityModPct -= (pProto->nEnmity > 0 ? 
			pProto->nEnmity - 100000 : pProto->nEnmity + 100000);
	}
	m_pMod->fEnmityParamMod -= pProto->fEnmityParam;

	// ����
	if( abs(pProto->nHit) < 100000 )
	{
		m_pMod->nHitMod -= pProto->nHit;
	}
	else
	{
		m_pMod->nHitModPct -= (pProto->nHit > 0 ? 
			pProto->nHit - 100000 : pProto->nHit + 100000);
	}

	// ����
	if( abs(pProto->nCrit) < 100000 )
	{
		m_pMod->nCritMod -= pProto->nCrit;
	}
	else
	{
		m_pMod->nCritModPct -= (pProto->nCrit > 0 ? 
			pProto->nCrit - 100000 : pProto->nCrit + 100000);
	}

	// �˺�
	for(INT n = 0; n < MAX_CHANNEL_TIMES; n++)
	{
		if( abs(pProto->nChannelDmg[n]) < 100000 )
		{
			m_pMod->nChannelDmgMod[n] -= pProto->nChannelDmg[n];
		}
		else
		{
			m_pMod->nChannelDmgModPct[n] -= (pProto->nChannelDmg[n] > 0 ? 
				pProto->nChannelDmg[n] - 100000 : pProto->nChannelDmg[n] + 1000000);
		}
	}

	// ����
	for(INT n = 0; n < ESCT_End; n++)
	{
		if( abs(pProto->nSkillCost[n]) < 100000 )
		{
			m_pMod->nSkillCostMod[n] -= pProto->nSkillCost[n];
		}
		else
		{
			m_pMod->nSkillCostModPct[n] -= (pProto->nSkillCost[n] > 0 ? 
				pProto->nSkillCost[n] - 100000 : pProto->nSkillCost[n] + 1000000);
		}
	}

	// Buff��Trigger
	UnSetSkillBuff(pProto);

	// �������Լӳ�
	ERoleAttribute eAtt = ERA_Null;
	INT nMod = 0;

	package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
	package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
	while( mapMod.find_next(itMod, eAtt, nMod) )
	{
		m_pMod->mapRoleAttMod.modify_value(eAtt, -nMod);
	}

	package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
	package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
	while( mapModPct.find_next(itModPct, eAtt, nMod) )
	{
		m_pMod->mapRoleAttModPct.modify_value(eAtt, -nMod);
	}

	// ����ñ�Ӱ��ļ��ܱ�������������ϣ���Ҫ������������Ӱ��
	if( VALID_POINT(m_pOwner) && IsCanModAtt() )
	{
		package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
		package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
		while( mapMod.find_next(itMod, eAtt, nMod) )
		{
			m_pOwner->ModAttModValue(eAtt, -nMod);
		}

		package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
		package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
		while( mapModPct.find_next(itModPct, eAtt, nMod) )
		{
			m_pOwner->ModAttModValuePct(eAtt, -nMod);
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------
// �������߼��ϼ������Լӳ�
//--------------------------------------------------------------------------------------
VOID Skill::SetOwnerMod()
{
	// ���Ƕ���������Ӱ��ļ��ܲ�����
	if( !VALID_POINT(m_pProto) || FALSE == IsCanModAtt() )
		return;

	// ��������߲����ڣ�Ҳ������
	if( !VALID_POINT(m_pOwner) ) return;

	// �ȼ��㾲̬���Լӳ�
	ERoleAttribute eType = ERA_Null;
	INT nValue = 0;

	// �ȼ���ƽֵ�ӳ�
	package_map<ERoleAttribute, INT>::map_iter it = m_pProto->mapRoleAttMod.begin();
	while( m_pProto->mapRoleAttMod.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValue(eType, nValue);
	}

	// �ټ���ٷֱȼӳ�
	it = m_pProto->mapRoleAttModPct.begin();
	while( m_pProto->mapRoleAttModPct.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValuePct(eType, nValue);
	}

	// �ټ��㼼�ܱ�Ӱ��ӳ�
	if( VALID_POINT(m_pMod) )
	{
		ERoleAttribute eType = ERA_Null;
		INT nValue = 0;

		// �ȼ���ƽֵ�ӳ�
		package_map<ERoleAttribute, INT>::map_iter it = m_pMod->mapRoleAttMod.begin();
		while( m_pMod->mapRoleAttMod.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValue(eType, nValue);
		}

		// �ټ���ٷֱȼӳ�
		it = m_pMod->mapRoleAttModPct.begin();
		while( m_pMod->mapRoleAttModPct.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValuePct(eType, nValue);
		}
	}

	// Buffע��
	RegisterTriggerEvent();
}

//-------------------------------------------------------------------------------------
// �������������ϵļ������Լӳ�
//-------------------------------------------------------------------------------------
VOID Skill::UnsetOwnerMod()
{
	// ���Ƕ���������Ӱ��ļ��ܲ�����
	if( !VALID_POINT(m_pProto) || FALSE == IsCanModAtt() )
		return;

	// ��������߲����ڣ�Ҳ������
	if( !VALID_POINT(m_pOwner) ) return;

	// �ȼ��㾲̬���Լӳ�
	ERoleAttribute eType = ERA_Null;
	INT nValue = 0;

	// �ȳ���ƽֵ�ӳ�
	package_map<ERoleAttribute, INT>::map_iter it = m_pProto->mapRoleAttMod.begin();
	while( m_pProto->mapRoleAttMod.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValue(eType, -nValue);
	}

	// �ٳ����ٷֱȼӳ�
	it = m_pProto->mapRoleAttModPct.begin();
	while( m_pProto->mapRoleAttModPct.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValuePct(eType, -nValue);
	}

	// �ٳ������ܱ�Ӱ��ӳ�
	if( VALID_POINT(m_pMod) )
	{
		ERoleAttribute eType = ERA_Null;
		INT nValue = 0;

		// �ȼ���ƽֵ�ӳ�
		package_map<ERoleAttribute, INT>::map_iter it = m_pMod->mapRoleAttMod.begin();
		while( m_pMod->mapRoleAttMod.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValue(eType, -nValue);
		}

		// �ټ���ٷֱȼӳ�
		it = m_pMod->mapRoleAttModPct.begin();
		while( m_pMod->mapRoleAttModPct.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValuePct(eType, -nValue);
		}
	}

	// Buff��ע��
	UnRegisterTriggerEvent();
}

//-----------------------------------------------------------------------------------------
// ע�Ἴ������Ĵ���������
//-----------------------------------------------------------------------------------------
VOID Skill::RegisterTriggerEvent()
{
	if( !IsCanModAtt() ) return;

	if( !VALID_POINT(m_pOwner) ) return;
	if( !m_pOwner->IsRole() ) return;

	Role* pRole = static_cast<Role*>(m_pOwner);

	for(INT n = 0; n < MAX_BUFF_PER_SKILL; ++n)
	{
		DWORD dwTriggerID = GetTriggerID(n);
		if( !VALID_POINT(dwTriggerID) ) continue;

		const tagTriggerProto* pProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
		if( !VALID_POINT(pProto) ) continue;

		pRole->RegisterTriggerSkillSet(pProto->eEventType, m_dwID);	
	}
}

//------------------------------------------------------------------------------------------
// ��ע�Ἴ������Ĵ���������
//------------------------------------------------------------------------------------------
VOID Skill::UnRegisterTriggerEvent()
{
	if( !IsCanModAtt() ) return;

	if( !VALID_POINT(m_pOwner) ) return;
	if( !m_pOwner->IsRole() ) return;

	Role* pRole = static_cast<Role*>(m_pOwner);

	for(INT n = 0; n < MAX_BUFF_PER_SKILL; ++n)
	{
		DWORD dwTriggerID = GetTriggerID(n);
		if( !VALID_POINT(dwTriggerID) ) continue;

		const tagTriggerProto* pProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
		if( !VALID_POINT(pProto) ) continue;

		pRole->UnRegisterTriggerSkillSet(pProto->eEventType, m_dwID);	
	}
}
