/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//技能

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
// 技能初始化
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

	// 如果该能被其它技能影响，则生成Mod结构
	if( AttRes::GetInstance()->CanBeModified(m_dwID) )
	{
		m_pMod = new tagSkillMod;

		// 将技能属性表中的buff先设置到mod里面
		SetSkillBuff(m_pProto);
	}
	// 否则就不生成
	else
	{
		m_pMod = NULL;
	}

	m_pScript = g_ScriptMgr.GetSkillScript(m_dwID);
	m_pOwner = NULL;

	return TRUE;
}

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID Skill::Update()
{

}

//---------------------------------------------------------------------------
// 保存
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
// 生成技能消息结构
//----------------------------------------------------------------------------
VOID Skill::GenerateMsgInfo(tagSkillMsgInfo* const pMsgInfo)
{
	pMsgInfo->dwID			=	GetID();
	pMsgInfo->nLevel		=	get_level();
	pMsgInfo->nProficiency	=	GetProficiency();
	pMsgInfo->nCoolDown		=	GetCoolDownCountDown();
	pMsgInfo->fOpDist		=	GetOPDist();

	// 伤害
	//for(INT n = 0; n < MAX_CHANNEL_TIMES; n++)
	//{
	//	pMsgInfo->nChannelDmg[n] = GetDmg(n);
	//}

	//// 消耗
	//for(INT n = 0; n < ESCT_End; n++)
	//{
	//	pMsgInfo->nCost[n] = GetCost((ESkillCostType)n);
	//}
}

//---------------------------------------------------------------------------------
// 设置技能属性加成，返回是否技能属性本身更新过
//---------------------------------------------------------------------------------
BOOL Skill::SetMod(const tagSkillProto* pProto)
{
	if( !VALID_POINT(pProto) ) return FALSE;
	if( ESTT_Skill != pProto->eTargetType ) return FALSE;

	// 攻击距离
	m_pMod->fOPDistMod += pProto->fOPDist;

	// 攻击半径
	m_pMod->fOPRadiusMod += pProto->fOPRadius;

	// 起手时间	
	m_pMod->nPrepareTimeMod += pProto->nPrepareTime;

	// 冷却时间
	m_pMod->nCoolDownMod += pProto->nCoolDown;

	// 仇恨
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

	// 命中
	if( abs(pProto->nHit) < 100000 )
	{
		m_pMod->nHitMod += pProto->nHit;
	}
	else
	{
		m_pMod->nHitModPct += (pProto->nHit > 0 ? 
			pProto->nHit - 100000 : pProto->nHit + 100000);
	}

	// 暴击
	if( abs(pProto->nCrit) < 100000 )
	{
		m_pMod->nCritMod += pProto->nCrit;
	}
	else
	{
		m_pMod->nCritModPct += (pProto->nCrit > 0 ? 
			pProto->nCrit - 100000 : pProto->nCrit + 100000);
	}

	// 伤害
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

	// 消耗
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

	// Buff和Trigger
	SetSkillBuff(pProto);

	// 人物属性加成
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

	// 如果该被影响的技能本身就在人物身上，需要立即计算属性影响
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
// 撤销技能属性加成，返回技能属性本身是否更新过
//----------------------------------------------------------------------------------
BOOL Skill::UnSetMod(const tagSkillProto* pProto)
{
	if( !VALID_POINT(m_pMod) ) return FALSE;
	if( ESTT_Skill != pProto->eTargetType ) return FALSE;

	// 攻击距离
	m_pMod->fOPDistMod -= pProto->fOPDist;

	// 攻击半径
	m_pMod->fOPRadiusMod -= pProto->fOPRadius;

	// 起手时间	
	m_pMod->nPrepareTimeMod -= pProto->nPrepareTime;

	// 冷却时间
	m_pMod->nCoolDownMod -= pProto->nCoolDown;

	// 仇恨
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

	// 命中
	if( abs(pProto->nHit) < 100000 )
	{
		m_pMod->nHitMod -= pProto->nHit;
	}
	else
	{
		m_pMod->nHitModPct -= (pProto->nHit > 0 ? 
			pProto->nHit - 100000 : pProto->nHit + 100000);
	}

	// 暴击
	if( abs(pProto->nCrit) < 100000 )
	{
		m_pMod->nCritMod -= pProto->nCrit;
	}
	else
	{
		m_pMod->nCritModPct -= (pProto->nCrit > 0 ? 
			pProto->nCrit - 100000 : pProto->nCrit + 100000);
	}

	// 伤害
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

	// 消耗
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

	// Buff和Trigger
	UnSetSkillBuff(pProto);

	// 人物属性加成
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

	// 如果该被影响的技能本身就在人物身上，需要立即计算属性影响
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
// 给所有者加上技能属性加成
//--------------------------------------------------------------------------------------
VOID Skill::SetOwnerMod()
{
	// 不是对人物属性影响的技能不计算
	if( !VALID_POINT(m_pProto) || FALSE == IsCanModAtt() )
		return;

	// 如果所有者不存在，也不计算
	if( !VALID_POINT(m_pOwner) ) return;

	// 先计算静态属性加成
	ERoleAttribute eType = ERA_Null;
	INT nValue = 0;

	// 先计算平值加成
	package_map<ERoleAttribute, INT>::map_iter it = m_pProto->mapRoleAttMod.begin();
	while( m_pProto->mapRoleAttMod.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValue(eType, nValue);
	}

	// 再计算百分比加成
	it = m_pProto->mapRoleAttModPct.begin();
	while( m_pProto->mapRoleAttModPct.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValuePct(eType, nValue);
	}

	// 再计算技能被影响加成
	if( VALID_POINT(m_pMod) )
	{
		ERoleAttribute eType = ERA_Null;
		INT nValue = 0;

		// 先计算平值加成
		package_map<ERoleAttribute, INT>::map_iter it = m_pMod->mapRoleAttMod.begin();
		while( m_pMod->mapRoleAttMod.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValue(eType, nValue);
		}

		// 再计算百分比加成
		it = m_pMod->mapRoleAttModPct.begin();
		while( m_pMod->mapRoleAttModPct.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValuePct(eType, nValue);
		}
	}

	// Buff注册
	RegisterTriggerEvent();
}

//-------------------------------------------------------------------------------------
// 撤销所有者身上的技能属性加成
//-------------------------------------------------------------------------------------
VOID Skill::UnsetOwnerMod()
{
	// 不是对人物属性影响的技能不计算
	if( !VALID_POINT(m_pProto) || FALSE == IsCanModAtt() )
		return;

	// 如果所有者不存在，也不计算
	if( !VALID_POINT(m_pOwner) ) return;

	// 先计算静态属性加成
	ERoleAttribute eType = ERA_Null;
	INT nValue = 0;

	// 先撤销平值加成
	package_map<ERoleAttribute, INT>::map_iter it = m_pProto->mapRoleAttMod.begin();
	while( m_pProto->mapRoleAttMod.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValue(eType, -nValue);
	}

	// 再撤销百分比加成
	it = m_pProto->mapRoleAttModPct.begin();
	while( m_pProto->mapRoleAttModPct.find_next(it, eType, nValue) )
	{
		m_pOwner->ModAttModValuePct(eType, -nValue);
	}

	// 再撤销技能被影响加成
	if( VALID_POINT(m_pMod) )
	{
		ERoleAttribute eType = ERA_Null;
		INT nValue = 0;

		// 先计算平值加成
		package_map<ERoleAttribute, INT>::map_iter it = m_pMod->mapRoleAttMod.begin();
		while( m_pMod->mapRoleAttMod.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValue(eType, -nValue);
		}

		// 再计算百分比加成
		it = m_pMod->mapRoleAttModPct.begin();
		while( m_pMod->mapRoleAttModPct.find_next(it, eType, nValue) )
		{
			m_pOwner->ModAttModValuePct(eType, -nValue);
		}
	}

	// Buff反注册
	UnRegisterTriggerEvent();
}

//-----------------------------------------------------------------------------------------
// 注册技能自身的触发器关联
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
// 反注册技能自身的触发器关联
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
