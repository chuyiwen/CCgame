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
*	@brief		人物数据结构
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
	// 删除所有技能
	package_map<DWORD, Skill*>::map_iter itSkill = m_mapSkill.begin();
	Skill* pSkill = NULL;

	while( m_mapSkill.find_next(itSkill, pSkill) )
	{
		SAFE_DELETE(pSkill);
	}
	m_mapSkill.clear();

	// 删除所有的buff修改器
	package_list<DWORD>* pListModifier = NULL;
	package_map<DWORD, package_list<DWORD>*>::map_iter itModifier = m_mapBuffModifier.begin();
	while( m_mapBuffModifier.find_next(itModifier, pListModifier) )
	{
		SAFE_DELETE(pListModifier);
	}
	m_mapBuffModifier.clear();

	// 删除所有trigger修改器
	itModifier = m_mapTriggerModifier.begin();
	while( m_mapTriggerModifier.find_next(itModifier, pListModifier) )
	{
		SAFE_DELETE(pListModifier);
	}
	m_mapTriggerModifier.clear();
}

//-------------------------------------------------------------------------------
// 更新随时间更新的函数
//-------------------------------------------------------------------------------
VOID Unit::update_time_issue()
{
	if( IsDead() ) return;

	// 回血回蓝
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

	// 回活力
	//if( --m_nVitalityRegainTickCountDown <= 0 )
	//{
	//	m_nVitalityRegainTickCountDown = VITALITY_REGAIN_INTER_TICK;

	//	if( GetAttValue(ERA_VitalityRegainRate) != 0 )
	//	{
	//		ChangeVitality(GetAttValue(ERA_VitalityRegainRate));
	//	}
	//}

	// 回持久力
	//if( --m_nEnduranceRegainTickCountDown <= 0 )
	//{
	//	m_nEnduranceRegainTickCountDown = ENDURANCE_REGAIN_INTER_TICK;

	//	ChangeEndurance(5);
	//}

	// 其它的一些随时间变化的

	// 狱中角色回道德
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
	// 开始计算
	for(INT n = 0; n < ERA_End; n++)
	{
		if( false == GetAttRecalFlag(n) )
			continue;

		// 该属性需要重新计算
		m_nAtt[n] = CalAttMod(m_nBaseAtt[n], n);
	}
}
//--------------------------------------------------------------------------------
// 根据重算标志位进行当前属性重算
//--------------------------------------------------------------------------------
VOID Unit::RecalAtt(BOOL bSendMsg, BOOL bHp)
{
	// 首先将某些“当前属性”提取出来
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

	//开始计算
	ReAccAtt();

	// 附上原先保存的属性
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
	

	// 将之前保存的某些“当前属性”与对应的“最大值”取最小值
	//m_nAtt[ERA_HP] = min(nHP, m_nAtt[ERA_MaxHP]);
	//m_nAtt[ERA_MP] = min(nMP, m_nAtt[ERA_MaxMP]);
	m_nAtt[ERA_Brotherhood] = min(nVitality, m_nAtt[ERA_MaxBrotherhood]);
	m_nAtt[ERA_Wuhuen]	= min(nWuhuen, m_nAtt[ERA_MaxBrotherhood]);
	//m_nAtt[ERA_Endurance] = min(nEndurance, m_nAtt[ERA_MaxEndurance]);

	// 比较原来的HP和新的HP之间是否一致，如果不一致，且血原来不需要重新计算，则设置需要重新计算，以便发消息
	if( m_nAtt[ERA_HP] != nHP && !m_bAttRecalFlag[ERA_HP] ) m_bAttRecalFlag[ERA_HP] = true;
	if( m_nAtt[ERA_MP] != nMP && !m_bAttRecalFlag[ERA_MP] ) m_bAttRecalFlag[ERA_MP] = true;
	//if( m_nAtt[ERA_Brotherhood] != nVitality && !m_bAttRecalFlag[ERA_Brotherhood] ) m_bAttRecalFlag[ERA_Brotherhood] = true;
	//if( m_nAtt[ERA_Endurance] != nEndurance && !m_bAttRecalFlag[ERA_Endurance] ) m_bAttRecalFlag[ERA_Endurance] = true;

	// 如果需要发送消息
	if( bSendMsg )
	{
		SendAttChangeMsg();

		// 设置某些因为属性改变而影响的值
		OnAttChange(m_bAttRecalFlag);
	}


	// 清空重算标志位
	ClearAttRecalFlag();
}

//--------------------------------------------------------------------------------
// 发送属性改变消息
//--------------------------------------------------------------------------------
VOID Unit::SendAttChangeMsg()
{
	// 首先查看是否需要广播（远程玩家需要知道的一些属性）
	BOOL bNeedBroadcast = FALSE;			// 是否需要广播
	bool bRemoteRoleAtt[ERRA_End] = {false};// 远程玩家属性
	INT nRemoteRoleAtt[ERRA_End] = {0};		// 远程玩家属性
	INT nRoleAttNum = 0;					// 本地玩家属性改变的数量
	INT nRemoteRoleAttNum = 0;				// 远程玩家属性改变的数量

	// 计算以上参数
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

	// 首先查看是否需要广播
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

	// 如果是人物并且需要发送，则发送给本地玩家
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

				// 同步属性给小队玩家
				if(pRole->GetTeamID() != INVALID_VALUE && IsTeamRemoteAtt(eRemote))
				{
					//同步小队成员
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
// 计算属性加成最终值，并取上下限
//--------------------------------------------------------------------------------
INT Unit::CalAttMod(INT nBase, INT nIndex)
{
	// 检查nIndex是否合法
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );

	INT nValue = nBase + m_nAttMod[nIndex]; 
	nValue += nValue * (FLOAT(m_nAttModPct[nIndex]) / 10000.0f);

	if( nValue < AttRes::GetInstance()->GetAttMin(nIndex) ) nValue = AttRes::GetInstance()->GetAttMin(nIndex);
	if( nValue > AttRes::GetInstance()->GetAttMax(nIndex) ) nValue = AttRes::GetInstance()->GetAttMax(nIndex);

	return nValue;
}

//--------------------------------------------------------------------------------
// 发送某个属性变化给客户端，如果该属性变化需要广播则广播
//--------------------------------------------------------------------------------
VOID Unit::SendAttChange(INT nIndex)
{
	ASSERT( nIndex > ERA_Null && nIndex < ERA_End );

	ERemoteRoleAtt eRemote = ERA2ERRA((ERoleAttribute)nIndex);

	// 生物
	if( IsCreature() )
	{
		if( ERRA_Null != eRemote )
		{
			// 发送远程同步命令
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
	// 玩家
	else if( IsRole() )
	{
		// 首先发送给自己
		Role* pRole = reinterpret_cast<Role*>(this);

		NET_SIS_single_role_att_change send;
		send.eType = (ERoleAttribute)nIndex;
		send.nValue = m_nAtt[nIndex];
		pRole->SendMessage(&send, send.dw_size);

		// 如果需要同步给远程玩家
		if( ERRA_Null != eRemote )
		{
			// 发送远程同步命令
			NET_SIS_single_remote_att_change send;
			send.dw_role_id = GetID();
			send.eType = eRemote;
			send.nValue = m_nAtt[nIndex];

			if( get_map() )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}
			
			// 同步属性给小队玩家
			if(pRole->GetTeamID() != INVALID_VALUE && IsTeamRemoteAtt(eRemote))
				g_groupMgr.SendTeamMesOutBigVis(pRole, pRole->GetTeamID(), &send, send.dw_size);
		}
	}
}

//-----------------------------------------------------------------------------------------
// 得到自身状态标志位
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
// 计算某个技能的影响
//-----------------------------------------------------------------------------
VOID Unit::AddSkillMod(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;

	ESkillTargetType eTarget = pSkill->GetTargetType();

	switch(eTarget)
	{
		// 影响技能
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

		// 影响Buff
	case ESTT_Buff:
		{
			RegisterBuffModifier(pSkill->GetTargetBuffID(), pSkill);
		}
		break;

		// 影响Trigger
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
// 计算当前技能列表中的所有技能对某个技能的影响
//------------------------------------------------------------------------------
VOID Unit::AddSkillBeMod(Skill* pSkill)
{
	// 查看当前技能列表中是否存在能影响该技能的技能
	tagSkillModify* pModify = AttRes::GetInstance()->GetSkillModifier(pSkill->GetID());

	if( !VALID_POINT(pModify) || pModify->listModify.empty() )
		return;

	// 通过列表查找技能列表中有无影响技能
	package_list<DWORD>& list = pModify->listModify;

	package_list<DWORD>::list_iter it = list.begin();

	Skill* pModSkill = NULL;
	DWORD dwModSkillID = INVALID_VALUE;
	while( list.find_next(it, dwModSkillID) )
	{
		pModSkill = m_mapSkill.find(dwModSkillID);
		if( !VALID_POINT(pModSkill) ) continue;

		// 发现一个技能，计算影响
		pSkill->SetMod(pModSkill->GetProto());
	}
}

//-----------------------------------------------------------------------------
// 去掉某个技能的影响
//-----------------------------------------------------------------------------
VOID Unit::RemoveSkillMod(Skill* pSkill)
{
	if( !VALID_POINT(pSkill) ) return;

	ESkillTargetType eTarget = pSkill->GetTargetType();

	switch(eTarget)
	{
		// 影响技能
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

		// 影响Buff
	case ESTT_Buff:
		{
			UnRegisterBuffModifier(pSkill->GetTargetBuffID(), pSkill);
		}
		break;

		// 影响Trigger
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
// 增加一个技能
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

	// 如果是生产技能，则自身所有的生产技能个数加1
	if( ESSTE_Produce == pSkill->GetTypeEx() )
	{
		++m_nProduceSkillNum;
	}
}

//----------------------------------------------------------------------------------
// 去掉一个技能
//----------------------------------------------------------------------------------
VOID Unit::RemoveSkill(DWORD dwSkillID)
{
	Skill* pSkill = m_mapSkill.find(dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	// 如果是生产技能，则自身所有的生产技能个数减1
	if( ESSTE_Produce == pSkill->GetTypeEx() )
	{
		--m_nProduceSkillNum;
	}

	// 清掉自身由该技能产生的所有buff
	RemoveAllBuffBelongSkill(pSkill->GetID(), FALSE);

	// 撤销影响
	pSkill->UnsetOwnerMod();
	m_mapSkill.erase(dwSkillID);
	m_listSkillCoolDown.erase(dwSkillID);

	pSkill->SetOwner(NULL);

	RemoveSkillMod(pSkill);	

	SAFE_DELETE(pSkill);
}

//----------------------------------------------------------------------------------
// 当技能升级或降级
//----------------------------------------------------------------------------------
VOID Unit::ChangeSkillLevel(Skill* pSkill, ESkillLevelChange eType, INT nChange)
{
	if( !VALID_POINT(pSkill) ) return;
	if( 0 == nChange ) return;

	// 首先先去掉人物属性影响，并去掉对其它技能影响
	pSkill->UnsetOwnerMod();
	RemoveSkillMod(pSkill);

	// 改变技能等级
	if( nChange > 0 )
		pSkill->IncLevel(eType, nChange);
	else
		pSkill->DecLevel(eType, nChange);

	// 重新计算属性影响
	AddSkillMod(pSkill);
	pSkill->SetOwnerMod();
}

//----------------------------------------------------------------------------------
// 改变技能熟练度
//----------------------------------------------------------------------------------
VOID Unit::ChangeSkillExp(Skill *pSkill, INT nValue)
{
	if( !VALID_POINT(pSkill) )	return;
	if( nValue <= 0 )	return;
	//if( ESLUT_Exp != pSkill->GetLevelUpType() ) return;
	if( pSkill->get_level() == pSkill->GetMaxLevel() ) return;
	
	// 防沉迷
	if ( IsRole() )
	{
		Role* pRole = static_cast<Role*>(this);
		FLOAT fEarnRate = pRole->GetEarnRate() / 10000.0f;
		nValue = INT(nValue * fEarnRate);
	}

	if( nValue <= 0 ) return;

	// 是否升级了
	BOOL bLevelChanged = FALSE;

	// 得到该技能的升级经验
	INT nRemainProficency = pSkill->GetLevelUpExp() - pSkill->GetProficiency();

	// 如果该经验小于升级经验
	if( nValue < nRemainProficency )
	{
		pSkill->AddProficiency(nValue);
	}
	// 如果该经验大于等于升级经验
	else
	{
		// 查看需要升几级
		while( nValue >= nRemainProficency )
		{
			// 已经到了最高等级
			if( pSkill->get_level() >= pSkill->GetMaxLevel() )
				break;

			nValue -= nRemainProficency;
			ChangeSkillLevel(pSkill, ESLC_Self);
			if( !bLevelChanged ) bLevelChanged = TRUE;

			// 得到下一级的升级经验
			nRemainProficency = pSkill->GetLevelUpExp();
		}

		// 查看如何来累加经验
		if( nValue >= nRemainProficency )
		{
			pSkill->SetProficiency(nRemainProficency);
		}
		else
		{
			pSkill->SetProficiency(nValue);
		}
	}

	// 如果升级并造成了属性改变
	if( bLevelChanged && NeedRecalAtt() )
	{
		RecalAtt();
	}
}

//----------------------------------------------------------------------------------
// 技能走CD
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
// 清空某个技能的CD
//----------------------------------------------------------------------------------
VOID Unit::ClearSkillCoolDown(DWORD dwSkillID)
{
	Skill* pSkill = GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return;

	pSkill->ClearCoolDown();
	m_listSkillCoolDown.erase(dwSkillID);
}

//------------------------------------------------------------------------------
// 主动技能状态触发
//------------------------------------------------------------------------------
BOOL Unit::OnActiveSkillBuffTrigger(Skill* pSkill, package_list<DWORD>& listTarget, ETriggerEventType eEvent, 
									DWORD dwEventMisc1/*=INVALID_VALUE*/, DWORD dwEventMisc2/*=INVALID_VALUE*/)
{
	// 技能不存在或者不是主动技能
	if( !VALID_POINT(pSkill) || FALSE == pSkill->IsActive() ) return FALSE;

	// 技能脚本
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

	// 检查技能是用skillbuff还是用属性表中的buffid
	for(INT n = 0; n < MAX_BUFF_PER_SKILL; ++n)
	{
		DWORD dwBuffTypeID = pSkill->GetBuffTypeID(n);
		DWORD dwTriggerID = pSkill->GetTriggerID(n);
		if( !VALID_POINT(dwBuffTypeID) || !VALID_POINT(dwTriggerID) ) continue;

		const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
		const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
		if( !VALID_POINT(pBuffProto) || !VALID_POINT(pTriggerProto) ) continue;
		

		// 查看trigger类型是否相同，主要是为了优化
		if( ETEE_Null != pTriggerProto->eEventType && eEvent != pTriggerProto->eEventType )
			continue;

		// 通过buff的添加对象进行添加
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
		// 尝试给自身添加
		// 主动技能自身buff添加规则：
		// （1）如果技能的目标是自身，则buff的添加对象类型里只要包含自身就尝试添加
		// （2）如果技能的目标非自身，则仅当buff的添加对象为自身时才尝试添加
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
// Buff触发
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
// 注册Buff修改器
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
	pList->push_back(dwSkillTypeID);				// 里面注册的是技能的属性id
	
}

//-----------------------------------------------------------------------------------
// 撤销Buff修改器
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
// 注册Trigger修改器
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
	pList->push_back(dwSkillTypeID);				// 里面注册的是技能的属性id

}

//-----------------------------------------------------------------------------------
// 撤销Buff修改器
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
// 尝试添加Buff
//-----------------------------------------------------------------------------------
BOOL Unit::TryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, 
					  Skill* pSkill, tagItem* pItem, BOOL bRecalAtt, DWORD dwEventMisc1, DWORD dwEventMisc2)
{

	if( !VALID_POINT(pBuffProto) ) return FALSE;

	// 通过属性ID得到ID和等级
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);
	INT nBuffLevel = Buff::GetLevelFromTypeID(pBuffProto->dwID);

	// 判断trigger
	if( VALID_POINT(pSrc) && VALID_POINT(pTriggerProto) )
	{
		if( !pSrc->TestTrigger(this, pTriggerProto, dwEventMisc1, dwEventMisc2) )
			return FALSE;
	}

	// 如果添加者存在，则检查添加者和自身的限制
	if( VALID_POINT(pSrc) )
	{
		// 首先检查对象类型限制
		DWORD dwTargetFlag = pSrc->TargetTypeFlag(this);
		if( !(pBuffProto->dwTargetAddLimit & dwTargetFlag) )
			return FALSE;

		// 不进行敌我判断
	}

	if (IsDead())
	{
		return false;
	}
	// 再检测buff添加状态限制
	//DWORD dwStatFlag = GetStateFlag();
	//if( (dwStatFlag & pBuffProto->dwTargetAddStateLimit) != dwStatFlag )
	//{
	//	return FALSE;
	//}

	// 判断抗性类型
	//ERoleAttribute eReistType = BuffResistType2ERA(pBuffProto->eResistType);
	//if( ERA_Null != eReistType && ERA_Regain_Addtion != eReistType )
	//{
	//	// 计算命中率
	//	// 公式：命中率=（1-##抗性/200）
	//	INT nProp = 100 - INT((FLOAT)GetAttValue(eReistType) / 2.0f);
	//	if( nProp < 0 ) nProp = 0;
	//	if( (IUTIL->tool_rand() % 100) > nProp )
	//		return FALSE;
	//}

	// 判断抵消和叠加
	INT nIndex = INVALID_VALUE;
	INT nRet = BuffCounteractAndWrap(pSrc, dwBuffID, nBuffLevel, pBuffProto->nLevel, pBuffProto->dwGroupFlag, pBuffProto->bBenifit, nIndex);

	if( EBCAWR_CanNotAdd == nRet ) return FALSE;
	else if( EBCAWR_Wraped == nRet ) return TRUE;

	ASSERT( EBCAWR_CanAdd == nRet );

	// 添加buff
	AddBuff(pBuffProto, pSrc, (VALID_POINT(pSkill) ? pSkill->GetID() : INVALID_VALUE), pItem, nIndex, bRecalAtt);
	return TRUE;
}

//-----------------------------------------------------------------------------------
// 用GM命令添加Buff
//-----------------------------------------------------------------------------------
BOOL Unit::GMTryAddBuff(Unit* pSrc, const tagBuffProto* pBuffProto, const tagTriggerProto* pTriggerProto, 
					  Skill* pSkill, tagItem* pItem, BOOL bRecalAtt, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	if( !VALID_POINT(pBuffProto) ) return FALSE;

	// 通过属性ID得到ID和等级
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);
	INT nBuffLevel = Buff::GetLevelFromTypeID(pBuffProto->dwID);

	// 判断trigger
	if( VALID_POINT(pSrc) && VALID_POINT(pTriggerProto) )
	{
		if( !pSrc->TestTrigger(this, pTriggerProto, dwEventMisc1, dwEventMisc2) )
			return FALSE;
	}

	// 如果添加者存在，则检查添加者和自身的限制
	//if( VALID_POINT(pSrc) )
	//{
	//	// 首先检查对象类型限制tbc:inves
	//	DWORD dwTargetFlag = pSrc->TargetTypeFlag(this);
	//	if( !(pBuffProto->dwTargetAddLimit & dwTargetFlag) )
	//		return FALSE;

	//	// 不进行敌我判断
	//}

	// 判断抗性类型
	//ERoleAttribute eReistType = BuffResistType2ERA(pBuffProto->eResistType);
	//if( ERA_Null != eReistType && ERA_Regain_Addtion != eReistType )
	//{
	//	// 计算命中率
	//	// 公式：命中率=（1-##抗性/200）
	//	INT nProp = 100 - INT((FLOAT)GetAttValue(eReistType) / 2.0f);
	//	if( nProp < 0 ) nProp = 0;
	//	if( (IUTIL->tool_rand() % 100) > nProp )
	//		return FALSE;
	//}

	// 判断抵消和叠加
	INT nIndex = INVALID_VALUE;
	INT nRet = BuffCounteractAndWrap(pSrc, dwBuffID, nBuffLevel, pBuffProto->nLevel, pBuffProto->dwGroupFlag, pBuffProto->bBenifit, nIndex);

	if( EBCAWR_CanNotAdd == nRet ) return FALSE;
	else if( EBCAWR_Wraped == nRet ) return TRUE;

	ASSERT( EBCAWR_CanAdd == nRet );

	// 添加buff
	//for (int i= ERA_AttA_Start; i< ERA_End; i++)
	//{
	//	SetAttRecalFlag(i);
	//}
	AddBuff(pBuffProto, pSrc, (VALID_POINT(pSkill) ? pSkill->GetID() : INVALID_VALUE), pItem, nIndex, bRecalAtt);
	return TRUE;
}

//----------------------------------------------------------------------------------
// 增加一个状态
//----------------------------------------------------------------------------------
VOID Unit::AddBuff(const tagBuffProto* pBuffProto, Unit* pSrc, DWORD dwSrcSkillID, const tagItem* pItem, INT nIndex, BOOL bRecalAtt)
{
	if( !VALID_POINT(pBuffProto) ) return;
	if( nIndex < 0 && nIndex >= MAX_BUFF_NUM ) return;
	
	// 通过BuffID找到该Buff是否有Mod
	DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);

	// 看是否有影响该buff的技能
	//if (VALID_POINT(pBuffProto->dwBeModSkillID))
	//{
	//	Skill* pModSkill = GetSkill(pBuffProto->dwBeModSkillID);
	//	if (VALID_POINT(pModSkill))
	//	{
	//		RegisterBuffModifier(dwBuffID, pModSkill);
	//	}
	//}

	//// 成就类buff加上纹章效果
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

	// 如果是瞬时buff
	if( pBuffProto->bInstant )
	{
		// 计算瞬时效果
		if( !VALID_POINT(pListModifier) || pListModifier->empty() )
		{
			BuffEffect::CalBuffInstantEffect(this, pSrc, EBEM_Instant, pBuffProto, NULL);
		}
		else
		{
			// 临时生成一个mod结构
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

		// 发送消息
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

		// 发送消息
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

	// 双方都是玩家，并且buff是有益的<Ares>
	if( VALID_POINT(pSrc) && pSrc->IsRole() && this->IsRole( ) &&  pBuffProto->bBenifit )
	{
		if( VALID_VALUE(((Role*)this)->GetTeamID()) && VALID_VALUE(((Role*)pSrc)->GetTeamID()) )
		{
			if( ((Role*)this)->GetTeamID( ) == ((Role*)pSrc)->GetTeamID( ) )
			{
				// 如果是红名，则施放技能者进入战斗状态
				if(((Role*)this)->HasPKValue( ) )
				{
					((Role*)pSrc)->SetRoleState(ERS_Combat);
					((Role*)pSrc)->GetCombatHandler().SetCombatStateCoolDown();
				}
			}
		}
	}

	// 重新计算基本属性
	if( bRecalAtt && NeedRecalAtt() ) RecalAtt();
}


//----------------------------------------------------------------------------------
// 去掉一个状态
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
	
	// 得到Buff当前的状态
	Buff::EBuffState eState = pBuff->GetState();

	// 正在删除的Buff，不需要再处理
	if( Buff::EBS_Destroying == eState )
	{
		return INVALID_VALUE;
	}

	// 当前正在初始化或更新的Buff，要添加到删除列表
	if( Buff::EBS_Initing	== eState ||
		Buff::EBS_Updating	== eState )
	{
		m_listDestroyBuffID.push_back(pBuff->GetID());
		return INVALID_VALUE;
	}

	// 当前在空闲状态的Buff，可以立即删除
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

	// 删除伴随BuffID
	if( VALID_POINT(dwBuffInterruptID) )
	{
		RemoveBuff(dwBuffInterruptID, FALSE);
	}

	// 重新计算基本属性
	if( bRecalAtt && NeedRecalAtt() ) RecalAtt();

	return nIndex;

}
//-----------------------------------------------------------------------------------
// 去掉所有由某个技能产生的buff
//-----------------------------------------------------------------------------------
VOID Unit::RemoveAllBuffBelongSkill(DWORD dwSkillID, BOOL bRecalAtt)
{
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		// 必须是自己施放的并且由该技能产生
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
// 去掉所有Buff
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
// 去掉所有有益或有害Buff
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
// 更新Buff
//-----------------------------------------------------------------------------------
VOID Unit::UpdateBuff()
{
	// 再删除删除列表里的Buff
	if( !m_listDestroyBuffID.empty() )
	{
		DWORD dwBuffID = m_listDestroyBuffID.pop_front();
		while( VALID_POINT(dwBuffID) )
		{
			RemoveBuff(dwBuffID, TRUE);

			dwBuffID = m_listDestroyBuffID.pop_front();
		}
	}

	// 首先更新所有存在的Buff
	for(INT n = 0; n < MAX_BUFF_NUM; n++)
	{
		if( !m_Buff[n].IsValid() ) continue;

		// 调用该Buff的Update
		if( m_Buff[n].Update() )
		{
			RemoveBuff(m_Buff[n].GetID(), TRUE, TRUE);
		}
	}


}

//-----------------------------------------------------------------------------------
// 手动取消一个Buff
//-----------------------------------------------------------------------------------
BOOL Unit::CancelBuff(DWORD dwBuffID)
{
	Buff* pBuff = GetBuffPtr(dwBuffID);
	if( !VALID_POINT(pBuff) ) return FALSE;

	// 看是否能手动打断
	if( !pBuff->Interrupt(EBIF_Manual) ) return FALSE;

	// 删除Buff
	RemoveBuff(dwBuffID, TRUE);

	return TRUE;
}

//-----------------------------------------------------------------------------------
// 打断Buff事件
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
// 驱散最后一个Buff（参数指定是Buff还是Debuff）
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
// 驱散最后一个抗性类型为eType的Buff
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
// 驱散某一个指定ID的Buff
//-----------------------------------------------------------------------------------
VOID Unit::DispelBuff(DWORD dwBuffID)
{
	RemoveBuff(dwBuffID, TRUE);
}

//-----------------------------------------------------------------------------------
// 驱散某一个指定效果类型的buff
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
//// 计算Buff的瞬时类效果，瞬时类效果包括：瞬时效果，间隔作用效果和结束时效果
////-----------------------------------------------------------------------------------
//VOID Unit::CalBuffInstantEffect(Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes, Unit* pTriggerTarget)
//{
//	if( !VALID_POINT(pProto) ) return;
//	if( EBEM_Instant != eMode && EBEM_Inter != eMode && EBEM_Finish != eMode ) return;
//
//	// 瞬时效果不关心叠加次数
//	if( EBEM_Instant == eMode ) nWrapTimes = 1;
//
//	BOOL bHaveMod = FALSE;
//	if( VALID_POINT(pMod) && pMod->IsValid() && pMod->eModBuffEffectMode == eMode )
//	{
//		bHaveMod = TRUE;
//	}
//
//	// 属性加成影响
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
//	// 效果
//	EBuffEffectType eEffect = pProto->eEffect[eMode];
//	DWORD dwEffectMisc1 = pProto->dwEffectMisc1[eMode];
//	DWORD dwEffectMisc2 = pProto->dwEffectMisc2[eMode];
//
//	// mod对其的影响
//	if( bHaveMod )
//	{
//		for(INT n = 0; n < EBEA_End; ++n)
//		{
//			nAttMod[n] = pnAttMod[n] + pMod->nEffectAttMod[n];
//
//			if( abs(nAttMod[n]) > 100000 )	// 百分比
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
//			if( abs(nAttMod[n]) > 100000 )	// 百分比
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
//	// 根据攻击类型，攻击距离和攻击范围来确定周围人
//	package_list<Unit*> listTargetUnit;
//	CalBuffTargetList(pSrc, pProto->eOPType, pProto->fOPDistance, pProto->fOPRadius, 
//						pProto->eFriendly, pProto->dwTargetLimit, pProto->dwTargetStateLimit, listTargetUnit, pTriggerTarget);
//
//	Unit* pTarget = listTargetUnit.PopFront();
//	while( VALID_POINT(pTarget) )
//	{
//		// 计算属性影响
//		for(INT n = 0; n < EBEA_End; ++n)
//		{
//			if( 0 == nAttMod[n] ) continue;
//
//			switch(n)
//			{
//			case EBEA_HP:
//				{
//					// 如果是恢复类buff
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
//		// 计算特殊效果
//		if( EBET_Null != eEffect )
//		{
//			BuffEffect::CalBuffEffect(pTarget, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, TRUE, pProto);
//		}
//
//		// 再取一个
//		pTarget = listTargetUnit.PopFront();
//	}
//}
//
////-----------------------------------------------------------------------------------
//// 计算Buff的持续性效果，只能对自身作用
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
//	// 先计算属性加成
//	ERoleAttribute eAtt = ERA_Null;
//	INT nValue = 0;
//	package_map<ERoleAttribute, INT>::map_iter it;
//	package_map<ERoleAttribute, INT>::map_iter itPct;
//
//	// 先计算静态属性的
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
//	// 在计算mod的
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
//	// 再计算效果
//	if( EBET_Null != eEffect )
//	{
//		BuffEffect::CalBuffEffect(this, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, bSet, pProto);
//	}
//}
//
//-----------------------------------------------------------------------------------
// 叠加Buff的持续性效果
//-----------------------------------------------------------------------------------
VOID Unit::WrapBuffPersistEffect(Buff* pBuff, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod)
{
	BuffEffect::CalBuffPersistEffect(pBuff, this, pSrc, pProto, pMod, 1, TRUE);
}
//-----------------------------------------------------------------------------------
//清除光环buff
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
// 计算Buff的作用对象
//-----------------------------------------------------------------------------------
VOID Unit::CalBuffTargetList(Unit* pSrc, EBuffOPType eOPType, FLOAT fOPDist, FLOAT fOPRadius, 
							 EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit, std::vector<Unit*>& listTarget, Unit* pTarget)
{
	// 如果添加者已经不存在了，直接返回
	if( !VALID_POINT(pSrc) ) return;

	// 首先检查一下自身
	if( pSrc->IsBuffTargetValid(this, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
	{
		listTarget.push_back(this);
	}

	// 再检查一下目标
	if( VALID_POINT(pTarget) && pSrc->IsBuffTargetValid(pTarget, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
	{
		listTarget.push_back(pTarget);
	}

	// 预先检查一下攻击范围
	if( 0.0f == fOPRadius ) return;

	// 如果攻击范围不为0，则以目标为球心检测
	FLOAT fOPRadiusSQ = fOPRadius * fOPRadius;

	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到攻击范围内的vistile列表
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
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
			if( pRole == this ) continue;

			if( FALSE == pSrc->IsBuffTargetValid(pRole, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
				continue;

			// 距离判断
			FLOAT fDistSQ = Vec3DistSq(GetCurPos(), pRole->GetCurPos());
			if( fDistSQ > fOPRadiusSQ  ) continue;

			// 射线检测

			// 判断通过，则将玩家加入到列表中
			listTarget.push_back(pRole);
		}

		// 再检测生物
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
			if( pCreature == this ) continue;

			if( FALSE == pSrc->IsBuffTargetValid(pCreature, eFriendEnemy, dwTargetLimit, dwTargetStateLimit) )
				continue;

			// 距离判断
			FLOAT fDistSQ = Vec3DistSq(GetCurPos(), pCreature->GetCurPos());
			if( fDistSQ > fOPRadiusSQ  ) continue;

			// 射线检测

			// 判断通过，则将生物加入到列表中
			listTarget.push_back(pCreature);
		}
	}
}

//-----------------------------------------------------------------------------------
// 取得指定技能施放的buff
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
		// 空的位置
		if( !m_Buff[n].IsValid() )
		{
			continue;
		}
		
		// 技能ID相同
		if (m_Buff[n].GetSrcSkillID() == dwSkillID)
		{
			return &m_Buff[n];
		}
	}

	return NULL;
}
//-----------------------------------------------------------------------------------
// 得到第一个有益buff
//-----------------------------------------------------------------------------------
Buff* Unit::GetFiretBenifitBuff()
{
	INT nStart = 0;
	INT nEnd = MAX_BENEFIT_BUFF_NUM;

	for(INT n = nStart; n < nEnd; n++)
	{
		// 空的位置
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
// 检查某个目标是不是Buff的合法目标
//-----------------------------------------------------------------------------------
BOOL Unit::IsBuffTargetValid(Unit* pTarget, EBuffFriendEnemy eFriendEnemy, DWORD dwTargetLimit, DWORD dwTargetStateLimit)
{
	if( !VALID_POINT(pTarget) ) return FALSE;

	// 类型判断
	DWORD dwTargetFlag = TargetTypeFlag(pTarget);
	if( !(dwTargetFlag & dwTargetLimit) )
		return FALSE;

	// 状态判断
	DWORD dwTargetStateFlag = pTarget->GetStateFlag();
	if( (dwTargetStateFlag & dwTargetStateLimit) != dwTargetStateFlag )
		return FALSE;

	// 敌我判断
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

	// 判断通过
	return TRUE;
}

//-----------------------------------------------------------------------------------
// 计算Buff的抵消和叠加
//-----------------------------------------------------------------------------------
INT Unit::BuffCounteractAndWrap(Unit* pSrc, DWORD dwBuffID, INT nBuffLevel, INT nGroupLevel, DWORD dwGroupFlag, BOOL bBenefit, INT& nIndex)
{
	// 首先检测ID
	Buff* pBuff = m_mapBuff.find(dwBuffID);

	// 如果存在一个同样的buff
	if( VALID_POINT(pBuff) && Buff::EBS_Idle == pBuff->GetState() )
	{
		if( pBuff->get_level() > nBuffLevel )
		{
			return EBCAWR_CanNotAdd;
		}
		else if( pBuff->get_level() == nBuffLevel )
		{
			// 检测看看能不能叠加
			
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
				// 顶掉，返回可以添加
				nIndex = RemoveBuff(pBuff->GetID(), TRUE);
				ASSERT( INVALID_VALUE != nIndex );
				return EBCAWR_CanAdd;
			}
		}
		else
		{
			// 顶掉该buff，返回可以添加
			nIndex = RemoveBuff(pBuff->GetID(), TRUE);
			ASSERT( nIndex != INVALID_VALUE );
			return EBCAWR_CanAdd;
			
		}
	}
	// 如果不存在一个同样的buff
	else
	{
		INT nStart = 0;
		INT nEnd = MAX_BENEFIT_BUFF_NUM;

		if( !bBenefit )
		{
			nStart = MAX_BENEFIT_BUFF_NUM;
			nEnd = MAX_BUFF_NUM;
		}

		INT nBlankIndex = INVALID_VALUE;		// 空位
		INT nFullIndex = INVALID_VALUE;		// 如果Buff满时，则代表可以被顶掉的索引
		for(INT n = nStart; n < nEnd; n++)
		{
			// 空的位置
			if( !m_Buff[n].IsValid() )
			{
				if( INVALID_VALUE == nBlankIndex )
				{
					nBlankIndex = n;
					if( INVALID_VALUE == dwGroupFlag ) break;
				}
				continue;
			}
			// group_flag相同
			else if( (dwGroupFlag != INVALID_VALUE) && 
					 (m_Buff[n].GetGroupFlag() == dwGroupFlag) )
			{
				// 如果当前不出于Idle状态，则不能顶掉
				if( Buff::EBS_Idle != m_Buff[n].GetState() )
				{
					return EBCAWR_CanNotAdd;
				}

				// 比较等级（这个是组等级）
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
			// buff满时顶掉
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
// 测试某个普通触发器是否满足条件
//------------------------------------------------------------------------------------------------------
BOOL Unit::TestTrigger(Unit* pTarget, const tagTriggerProto* pProto, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	if( !VALID_POINT(pProto) ) return TRUE;

	// 检查一下该触发器是否有被动技能影响
	package_list<DWORD>* pListModifier = m_mapTriggerModifier.find(pProto->dwID);

	if( !VALID_POINT(pListModifier) || pListModifier->empty() )
	{
		// 没有影响，直接计算
		return TestEventTrigger(pTarget, pProto, NULL, dwEventMisc1, dwEventMisc2) &&
			   TestStateTrigger(pTarget, pProto, NULL);
	}
	else
	{
		// 计算触发器影响
		tagTriggerMod mod;

		package_list<DWORD>::list_iter it = pListModifier->begin();
		DWORD dwSkillTypeID = INVALID_VALUE;

		while( pListModifier->find_next(it, dwSkillTypeID) )
		{
			const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
			if( !VALID_POINT(pSkillProto) ) continue;

			mod.SetMod(pSkillProto);
		}

		// 触发器影响计算完毕，开始计算触发器结果
		return TestEventTrigger(pTarget, pProto, &mod, dwEventMisc1, dwEventMisc2) &&
			   TestStateTrigger(pTarget, pProto, &mod);
	}
}

//----------------------------------------------------------------------------------------------------------
// 测试某个触发器的事件条件是否满足
//----------------------------------------------------------------------------------------------------------
BOOL Unit::TestEventTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod, DWORD dwEventMisc1, DWORD dwEventMisc2)
{
	// 没有事件要求，则直接返回成功
	if( ETEE_Null == pProto->eEventType ) return TRUE;

	// 不需要判断触发率，则直接返回成功
	if (m_bTriggerOff == TRUE)
		return TRUE;

	// 接下来测试几率
	INT nProp = pProto->nEventProp + ( VALID_POINT(pMod) ? pMod->nPropMod : 0 );
	if( get_tool()->tool_rand() % 10000 > nProp ) return FALSE;

	// 接下来根据某些特殊的事件类型来进行判断，目前没有需要特殊判断的

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------
// 测试某个触发器的状态条件是否满足
//------------------------------------------------------------------------------------------------------------
BOOL Unit::TestStateTrigger(Unit* pTarget, const tagTriggerProto* pProto, const tagTriggerMod* pMod)
{
	if( ETEE_Null == pProto->eStateType ) return TRUE;

	DWORD dwStateMisc1 = pProto->dwStateMisc1 + ( VALID_POINT(pMod) ? pMod->nStateMisc1Mod : 0 );
	DWORD dwStateMisc2 = pProto->dwStateMisc2 + ( VALID_POINT(pMod) ? pMod->nStateMisc2Mod : 0 );

	// 查看具体的trigger类型做出判断
	switch(pProto->eStateType)
	{
		// 体力高于
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
		// 目标体力高于
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
		// 体力低于
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

		// 目标体力低于
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
		// 真气高于
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

		// 真气低于
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

		// 怒气高于
	//case ETST_RageHigher:
	//	{
	//		INT nRageLimit = (INT)dwStateMisc1;
	//		return GetAttValue(ERA_Rage) > nRageLimit;
	//	}
	//	break;

	//	// 怒气低于
	//case ETST_RageLower:
	//	{
	//		INT nRageLimit = (INT)dwStateMisc1;
	//		return GetAttValue(ERA_Rage) < nRageLimit;
	//	}
	//	break;

	//	// 持久力高于
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

	//	// 持久力低于
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

		// 自身拥有某buff
	case ETST_SelfHaveBuff:
		{
			DWORD dwBuffID = dwStateMisc1;
			return IsHaveBuff(dwBuffID);
		}
		break;

		// 目标拥有某buff
	case ETST_TargetHaveBuff:
		{
			DWORD dwBuffID = dwStateMisc2;
			if( !VALID_POINT(pTarget) ) return FALSE;

			return pTarget->IsHaveBuff(dwBuffID);
		}
		break;
		// 自身在某状态
	case ETST_SelfHaveState:
		{
			DWORD dwState = dwStateMisc1;
			IsInState((EState)dwState);
		}
		break;
		// 目标有某状态
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
// 添加怪物到该玩家仇恨的怪物列表
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
// 删除怪物从该玩家仇恨的怪物列表
//----------------------------------------------------------------------------------
VOID Unit::DelEnmityCreature(Unit *pUnit)
{
	if(FALSE == pUnit->IsCreature())
		return;

	m_mapEnmityCreature.erase(pUnit->GetID());
}

//----------------------------------------------------------------------------------
// 清空玩家仇恨的怪物列表
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
// 按照百分比改变仇恨
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

// 处理伤害吸收效果
INT Unit::OnAbsorDmg(INT nDmgValue)
{
	
	Buff* pBuff = NULL;
	m_mapBuff.reset_iterator();
	INT nBuffAbs = 0; //buff伤害吸收
	INT nSubDmg = 0; //吸收后的伤害
	INT nDmgAbs = 0; //剩余的伤害吸收量
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
// 改变血量
//-----------------------------------------------------------------------------------------
VOID Unit::ChangeHP( INT nValue, Unit* pSrc/* =NULL */, Skill* pSkill/* =NULL */, const tagBuffProto* pBuff/* =NULL */, bool bCrit/* =false */, bool bBlock/*=false*/, DWORD dwSerial/* =INVALID_VALUE */, DWORD dwMisc/* =INVALID_VALUE */, INT nOtherDmg/* = 0 */)
{
	INT nDmg = nValue;

	if( 0 == nDmg ) return;

	// 怪物伤害脚本
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


	//如果是少血的话
	if (nDmg < 0)
	{
		//如果有伤害补血效果
		if (m_bDmgReturn)
		{
			nDmg = -nDmg;
		}
		//伤害吸收
		else if (ISAbsorbDmg())
		{
			nDmg = OnAbsorDmg(nDmg);
			//INT nSubDmg = (INT)m_nDmgAbs + nDmg;
			//nDmg = nSubDmg > 0 ? 0 : nSubDmg;
			//m_nDmgAbs = nSubDmg < 0 ? 0 : nSubDmg;
		}
	}

	ModAttValue(ERA_HP, nDmg);

	// 如果是战斗系统造成的血量改变，则发送战斗屏显消息
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

	// 检测减血事件
	if( nDmg < 0 )
	{
		OnInterruptBuffEvent(EBIF_HPLower);		// 检测Buff事件

		if( GetAttValue(ERA_HP) <= 0 )
		{
			OnDead(pSrc, pSkill, pBuff, dwSerial, dwMisc, bCrit);	// 死亡
		}
	}
}
//更新状态相关
VOID Unit::UpdateState()
{
	if (IsInState(ES_Feat))
	{
		// 正在移动
		if( m_bIsFeating )
		{

			if( EMS_Stand == GetMoveData().m_eCurMove  )
			{
				// 已经到达终点，则原地休息一段时间
				m_bIsFeating = FALSE;

				m_nFeatRestTick = 2 * TICK_PER_SECOND;
				//ReSetPatrolRestTick( );
				
			}
			return;
		}

		// 正在休息
		if( m_nFeatRestTick > 0 )
		{
			m_nFeatRestTick--;
			return;
		}

		// 开始逃跑巡逻
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