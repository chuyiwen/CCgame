/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//״̬Ч��

#include "StdAfx.h"

#include "../../common/WorldDefine/drop_protocol.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "unit.h"
#include "buff_effect.h"
#include "role.h"
#include "creature.h"
#include "loot_mgr.h"
#include "script_mgr.h"
#include "pet_pocket.h"
#include "pet_soul.h"
#include "creature_ai.h"
#include "guild.h"
#include "guild_manager.h"
#include "buff.h"
#include "map_creator.h"
#include "pet_heti.h"
#include "SparseGraph.h"

BUFFEFFECTRUTINE BuffEffect::m_Effect[EBET_End];

VOID BuffEffect::Init()
{
	RegisterBuffEffectRutine();
}

//--------------------------------------------------------------------------------
// ��ʼ��
//--------------------------------------------------------------------------------
VOID BuffEffect::RegisterBuffEffectRutine()
{
	m_Effect[EBET_Null]					=	&BuffEffect::BuffEffectNull;
	m_Effect[EBET_Teleport]				=	&BuffEffect::BuffEffectTeleport;
	m_Effect[EBET_InterruptSpell]		=	&BuffEffect::BuffEffectInterruptSpell;
	m_Effect[EBET_Dizzy]				=	&BuffEffect::BuffEffectDizzy;
	m_Effect[EBET_Repel]				=	&BuffEffect::BuffEffectRepel;
	m_Effect[EBET_Assault]				=	&BuffEffect::BuffEffectAssault;
	m_Effect[EBET_NoSkill]				=	&BuffEffect::BuffEffectNoSkill;
	m_Effect[EBET_Spor]					=	&BuffEffect::BuffEffectSpor;
	m_Effect[EBET_Tie]					=	&BuffEffect::BuffEffectTie;
	m_Effect[EBET_Dispel]				=	&BuffEffect::BuffEffectDispel;
	m_Effect[EBET_Cancel]				=	&BuffEffect::BuffEffectCancel;
	m_Effect[EBET_Invincible]			=	&BuffEffect::BuffEffectInvincible;
	m_Effect[EBET_InstantDie]			=	&BuffEffect::BuffEffectInstantDie;
	m_Effect[EBET_Pull]					=	&BuffEffect::BuffEffectPull;
	m_Effect[EBET_ReboundDmg]			=	&BuffEffect::BuffEffectNull;
	m_Effect[EBET_AbsorbDmg]			=	&BuffEffect::BuffectAbsorbDmg;
	m_Effect[EBET_TransmitDmg]			=	&BuffEffect::BuffEffectNull;
	m_Effect[EBET_HPDrain]				=	&BuffEffect::BuffEffectNull;
	m_Effect[EBET_MPDrain]				=	&BuffEffect::BuffEffectNull;
	m_Effect[EBET_HPTransfer]			=	&BuffEffect::BuffEffectHPTransfer;
	m_Effect[EBET_MPTransfer]			=	&BuffEffect::BuffEffectMPTransfer;
	m_Effect[EBET_Revive]				=	&BuffEffect::BuffEffectRevive;
	m_Effect[EBET_InstantCD]			=	&BuffEffect::BuffEffectInstantCD;
	m_Effect[EBET_Lurk]					=	&BuffEffect::BuffEffectLurk;
	m_Effect[EBET_Fly]					=	&BuffEffect::BuffEffectFly;
	m_Effect[EBET_ChangeEnmity]			=	&BuffEffect::BuffEffectChangeEnmity;
	m_Effect[EBET_Transmit]				=	&BuffEffect::BuffEffectTransmit;
	m_Effect[EBET_Gather]				=	&BuffEffect::BuffEffectGather;
	m_Effect[EBET_DisArm]				=	&BuffEffect::BuffEffectDisArm;
	m_Effect[EBET_ExchangePos]			=	&BuffEffect::BuffEffectExchangePos;
	m_Effect[EBET_Explode]				=	&BuffEffect::BuffEffectExplode;
	m_Effect[EBET_Funk]					=	&BuffEffect::BuffEffectFunk;
	m_Effect[EBET_Pursue]				=	&BuffEffect::BuffEffectPursue;
	m_Effect[EBET_NoPrepare]			=	&BuffEffect::BuffEffectNoPrepare;
	m_Effect[EBET_OnWater]				=	&BuffEffect::BuffEffectOnWater;
	m_Effect[EBET_MoveHPDmg]			=	&BuffEffect::BuffEffectMoveHPDmg;
	m_Effect[EBET_IgnoreArmor]			=	&BuffEffect::BuffEffectIgnoreArmor;
	m_Effect[EBET_Sneer]				=	&BuffEffect::BuffEffectSneer;
	m_Effect[EBET_Mount]				=	&BuffEffect::BuffEffectMount;
	m_Effect[EBET_MountInvite]			=	&BuffEffect::BuffEffectMountInvite;
	m_Effect[EBET_Ring]					=	&BuffEffect::BuffEffectRing;
	m_Effect[EBET_HPExchange]			=	&BuffEffect::BuffectHPExchange;
	m_Effect[EBET_FilchBuff]			=	&BuffEffect::BuffectFilchBuff;
	m_Effect[EBET_RongHePet]			=	&BuffEffect::BuffectRonghePet;
	m_Effect[EBET_CancelBeheti]			=	&BuffEffect::BuffectCancelBeHeti;
	m_Effect[EBET_CancelHeti]			=	&BuffEffect::BUffectCancelHeTi;
	m_Effect[EBET_ClearPkValue]			=	&BuffEffect::BuffectClaerPkValue;
	m_Effect[EBET_CantAttackUnMount]	=	&BuffEffect::BuffectCantAttackUnMount;
	m_Effect[EBET_CallAttackPet]		=	&BuffEffect::BuffectCallAttackPet;
	m_Effect[EBET_Coller]				=	&BuffEffect::BuffEffectCollid;
}

//----------------------------------------------------------------------------------
// ����Buff����Ч��
//----------------------------------------------------------------------------------
VOID BuffEffect::CalBuffEffect(Buff* pBuff, Unit* pTarget, Unit* pSrc, EBuffEffectType eEffect, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto)
{
	if( eEffect < EBET_Null || eEffect >= EBET_End )
		return;

	BUFFEFFECTRUTINE handler = m_Effect[eEffect];
	(*handler)(pTarget, pSrc, dwEffectMisc1, dwEffectMisc2, bSet, pProto, pBuff);
}

//-----------------------------------------------------------------------------------
// ����Buff��˲ʱ��Ч����˲ʱ��Ч��������˲ʱЧ�����������Ч���ͽ���ʱЧ��
//-----------------------------------------------------------------------------------
VOID BuffEffect::CalBuffInstantEffect(Unit* pOwner, Unit* pSrc, EBuffEffectMode eMode, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes, Unit* pTriggerTarget)
{
	if (!VALID_POINT(pSrc)) return;
	if( !VALID_POINT(pProto) ) return;
	if( EBEM_Instant != eMode && EBEM_Inter != eMode && EBEM_Finish != eMode ) return;

	// ˲ʱЧ�������ĵ��Ӵ���
	if( EBEM_Instant == eMode ) nWrapTimes = 1;

	BOOL bHaveMod = FALSE;
	if( VALID_POINT(pMod) && pMod->IsValid() && pMod->eModBuffEffectMode == eMode )
	{
		bHaveMod = TRUE;
	}

	// ���Լӳ�Ӱ��
	INT nAttMod[EBEA_End] = {0};
	const INT* pnAttMod = NULL;
	switch(eMode)
	{
	case EBEM_Instant:
		pnAttMod = pProto->nInstantAttMod;
		break;

	case EBEM_Inter:
		pnAttMod = pProto->nInterAttMod;
		break;

	case EBEM_Finish:
		pnAttMod = pProto->nFinishAttMod;
		break;

	default:
		break;
	}

	// Ч��
	EBuffEffectType eEffect = pProto->eEffect[eMode];
	DWORD dwEffectMisc1 = pProto->dwEffectMisc1[eMode];
	DWORD dwEffectMisc2 = pProto->dwEffectMisc2[eMode];

	// mod�����Ӱ��
	if( bHaveMod )
	{
		for(INT n = 0; n < EBEA_End; ++n)
		{
			nAttMod[n] = pnAttMod[n];

			if( abs(nAttMod[n]) > 100000 )	// �ٷֱ�
			{
				INT nAtt = 0;
				switch(n)
				{
				case EBEA_HP:
					nAtt = pOwner->GetAttValue(ERA_MaxHP);
					break;

				case EBEA_MP:
					nAtt = pOwner->GetAttValue(ERA_MaxMP);
					break;

				//case EBEA_Vitality:
				//	nAtt = pOwner->GetAttValue(ERA_MaxVitality);
				//	break;

				//case ERA_Endurance:
				//	nAtt = pOwner->GetAttValue(ERA_MaxEndurance);
				//	break;

				default:
					break;
				}
				
				if ( abs(pMod->nEffectAttMod[n]) > 100000 )
				{
					nAttMod[n] = nAttMod[n] - 100000;
					nAttMod[n] = (pMod->nEffectAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAttMod[n] * (FLOAT(abs(pMod->nEffectAttMod[n]) - 100000) / 10000.0f));
					nAttMod[n] = (nAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAtt * (FLOAT(abs(nAttMod[n])) / 10000.0f));
				}
				else
				{
					nAttMod[n] = (nAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAtt * (FLOAT(abs(nAttMod[n]) - 100000) / 10000.0f));
					nAttMod[n] += pMod->nEffectAttMod[n];
				}
				
			}
			else
			{
				if ( abs(pMod->nEffectAttMod[n]) > 100000 )
				{
					nAttMod[n] = (pMod->nEffectAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAttMod[n] * (FLOAT(abs(pMod->nEffectAttMod[n]) - 100000) / 10000.0f));
				}
				else
				{
					nAttMod[n] += pMod->nEffectAttMod[n];
				}
			}
		
			nAttMod[n] *= nWrapTimes;
		}
		dwEffectMisc1 += pMod->nEffectMisc1Mod;
		dwEffectMisc2 += pMod->nEffectMisc2Mod;
	}
	else
	{
		for(INT n = 0; n < EBEA_End; ++n)
		{
			nAttMod[n] = pnAttMod[n];

			if( abs(nAttMod[n]) > 100000 )	// �ٷֱ�
			{
				INT nAtt = 0;
				switch(n)
				{
				case EBEA_HP:
					nAtt = pOwner->GetAttValue(ERA_MaxHP);
					break;

				case EBEA_MP:
					nAtt = pOwner->GetAttValue(ERA_MaxMP);
					break;

				//case EBEA_Vitality:
				//	nAtt = pOwner->GetAttValue(ERA_MaxVitality);
				//	break;

				//case ERA_Endurance:
				//	nAtt = pOwner->GetAttValue(ERA_MaxEndurance);
				//	break;

				default:
					break;
				}

				nAttMod[n] = (nAttMod[n] > 0 ? 1 : -1) * INT((FLOAT)nAtt * (FLOAT(abs(nAttMod[n]) - 100000) / 10000.0f));
			}

			nAttMod[n] *= nWrapTimes;
		}
	}

	//�ڽű���õ������˺�
	DWORD	dwID = Buff::GetIDFromTypeID(pProto->dwID);
	INT		nLevel = Buff::GetLevelFromTypeID(pProto->dwID);

	const BuffScript* pScript = g_ScriptMgr.GetBuffScript(dwID);
	if (VALID_POINT(pScript) && VALID_POINT(pSrc) )
	{
		nAttMod[EBEA_HP] = pScript->CalculateDmg(pSrc->get_map(), dwID, nLevel, pSrc->GetID(), pOwner->GetID(), nAttMod[EBEA_HP]);
	}
	// ���ݹ������ͣ���������͹�����Χ��ȷ����Χ��
	std::vector<Unit*> listTargetUnit;
	pOwner->CalBuffTargetList(pSrc, pProto->eOPType, pProto->fOPDistance, pProto->fOPRadius, 
		pProto->eFriendly, pProto->dwTargetLimit, pProto->dwTargetStateLimit, listTargetUnit, pTriggerTarget);

	for (std::size_t i = 0; i < listTargetUnit.size(); ++i)
	{
		Unit* pTarget = listTargetUnit[i];

		//����ǳ�� �ȼ�������Ч�� ���λ��
		if( EBET_Assault == eEffect )
		{
			BuffEffect::CalBuffEffect(NULL, pTarget, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, TRUE, pProto);
		}
		// ��������Ӱ��
		for(INT n = 0; n < EBEA_End; ++n)
		{
			if( 0 == nAttMod[n] ) continue;

			switch(n)
			{
			case EBEA_HP:
				{
					// ����ǻָ���buff
					if (pProto->eResistType == EBRT_Regain)
					{
						ASSERT(pSrc->GetAttModValue(ERA_Regain_Addtion) > -100 && pOwner->GetAttModValue(ERA_Regain_Addtion)< 100);
						FLOAT f = (1 + (FLOAT(pSrc->GetAttModValue(ERA_Regain_Addtion))/200)) * (1 + ((FLOAT)(pOwner->GetAttModValue(ERA_Regain_Addtion))/200));
						nAttMod[EBEA_HP] = INT((FLOAT)nAttMod[EBEA_HP] * f);
					}

					if( pTarget->IsCreature() )
					{
						Creature* pCreature = static_cast<Creature*>(pTarget);
						if( !VALID_POINT(pCreature) )	break;

						pCreature->OnBuffInjury(pSrc, nAttMod[EBEA_HP]);
					}
					
					// ��Ѫ���buff
					if (pTarget->IsRole() && nAttMod[EBEA_HP] > 0)
					{
						// ������ϱ�����������Ĺ���ȫ�����ӳ��
						DWORD dwCreatureID = INVALID_VALUE;
						Creature* pCreature = (Creature*)INVALID_VALUE;
						pTarget->GetEnmityCreature().reset_iterator();
						while( pTarget->GetEnmityCreature().find_next(dwCreatureID) )
						{
							pCreature = pSrc->get_map()->find_creature(dwCreatureID);
							if( !VALID_POINT(pCreature) ) continue;

							if( VALID_POINT(pCreature->GetAI()) )
								pCreature->GetAI()->AddEnmity(pSrc, (int)(nAttMod[EBEA_HP] * 0.3f));
						}
						
					}
					pTarget->ChangeHP(nAttMod[EBEA_HP], pSrc, NULL, pProto);
				}
				break;

			case EBEA_MP:
				{
					pTarget->ChangeMP(nAttMod[EBEA_MP]);
				}
				break;

			case EBEA_Rage:
				{
					pTarget->ChangeRage(nAttMod[n]);
				}
				break;

			default:
				break;
			}
		}

		// ��������Ч��
		if( EBET_Null != eEffect && EBET_Assault != eEffect )
		{
			CalBuffEffect(NULL, pTarget, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, TRUE, pProto);
		}
	}
	//Unit* pTarget = listTargetUnit.PopFront();
	//while( VALID_POINT(pTarget) )
	//{
	//	

	//	// ��ȡһ��
	//	pTarget = listTargetUnit.PopFront();
	//}
}

//-----------------------------------------------------------------------------------
// ����Buff�ĳ�����Ч����ֻ�ܶ���������
//-----------------------------------------------------------------------------------
VOID BuffEffect::CalBuffPersistEffect(Buff* pBuff, Unit* pOwner, Unit* pSrc, const tagBuffProto* pProto, const tagBuffMod* pMod, INT nWrapTimes, BOOL bSet/* =TRUE */)
{
	if( !VALID_POINT(pProto) ) return;

	BOOL bHaveMod = FALSE;
	if( VALID_POINT(pMod) && pMod->IsValid() && EBEM_Persist == pMod->eModBuffEffectMode )
	{
		bHaveMod = TRUE;
	}

	// �ȼ������Լӳ�
	ERoleAttribute eAtt = ERA_Null;
	INT nValue = 0;
	package_map<ERoleAttribute, INT>::map_iter it;
	package_map<ERoleAttribute, INT>::map_iter itPct;

	// �ȼ��㾲̬���Ե�
	INT nAttMod[ERA_End] = {0};
	INT nAttModPct[ERA_End] = {0};
	it = pProto->mapRoleAttMod.begin();
	while( pProto->mapRoleAttMod.find_next(it, eAtt, nValue) )
	{
		pOwner->ModAttModValue(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
		nAttMod[eAtt] += nValue;
	}

	itPct = pProto->mapRoleAttModPct.begin();
	while( pProto->mapRoleAttModPct.find_next(itPct, eAtt, nValue) )
	{
		pOwner->ModAttModValuePct(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
		nAttModPct[eAtt] += nValue;
	}

	// �ڼ���mod��
	if( bHaveMod )
	{
		//it = pMod->mapRoleAttMod.begin();
		//while( pMod->mapRoleAttMod.find_next(it, eAtt, nValue) )
		//{
		//	pOwner->ModAttModValue(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
		//}

		//itPct = pMod->mapRoleAttModPct.begin();
		//while( pMod->mapRoleAttModPct.find_next(itPct, eAtt, nValue) )
		//{
		//	pOwner->ModAttModValuePct(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
		//}
		INT nAddAttMod[ERA_End] = {0};
		INT nAddAttModPct[ERA_End] = {0};

		it = pMod->mapRoleAttMod.begin();
		while( pMod->mapRoleAttMod.find_next(it, eAtt, nValue) )
		{
			pOwner->ModAttModValue(eAtt, (bSet ? nValue : -nValue) * nWrapTimes);
		}

		itPct = pMod->mapRoleAttModPct.begin();
		while( pMod->mapRoleAttModPct.find_next(itPct, eAtt, nValue) )
		{
			nAddAttMod[eAtt] = nAttMod[eAtt] * (FLOAT(nValue) / 10000.0f - 1.0f);
			nAddAttModPct[eAtt] = nAttModPct[eAtt] * (FLOAT(nValue) / 10000.0f - 1.0f);
		}

		for (INT i = 0; i < ERA_End; i++)
		{
			if (nAddAttMod[i] != 0)
			{
				INT nValue = nAddAttMod[i];
				pOwner->ModAttModValue(i, (bSet ? nValue : -nValue) * nWrapTimes);
			}
		}	

		for (INT i = 0; i < ERA_End; i++)
		{
			if (nAddAttModPct[i] != 0)
			{
				INT nValue = nAddAttModPct[i];
				pOwner->ModAttModValuePct(i, (bSet ? nValue : -nValue) * nWrapTimes);
			}
		}


	}

	EBuffEffectType eEffect = pProto->eEffect[EBEM_Persist];

	// �ټ���Ч��
	if( EBET_Null != eEffect )
	{
		DWORD dwEffectMisc1 = pProto->dwEffectMisc1[EBEM_Persist];
		DWORD dwEffectMisc2 = pProto->dwEffectMisc2[EBEM_Persist];

		if( bHaveMod )
		{
			dwEffectMisc1 += pMod->nEffectMisc1Mod;
			dwEffectMisc2 += pMod->nEffectMisc2Mod;
		}

		CalBuffEffect(pBuff, pOwner, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, bSet, pProto);
	}
}


//----------------------------------------------------------------------------------
// ����Ч������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectNull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{

}
//----------------------------------------------------------------------------------
// ����Ч����˲��
// dwEffectMisc1���ƶ��ľ��룬����0ʱΪ��ǰ�ƶ���С��0ʱΪ����ƶ�
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectTeleport(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	INT nTileDist = (INT)dwEffectMisc1;
	if( 0 == nTileDist ) return;

	BOOL bForward = (nTileDist > 0);	// ��ǰ��������ƶ�

	FLOAT fDistAbs = FLOAT(abs(nTileDist) * TILE_SCALE);	// ���Ծ���

	Vector3 vFaceTo = pTarget->GetFaceTo();	// �õ���ҳ���

	// �Ըó�����й�һ��
	Vec3Normalize(vFaceTo);

	// �õ��յ�����
	Vector3 vDest;

	if( bForward )
	{
		vDest = pTarget->GetCurPos() + vFaceTo * fDistAbs;
	}
	else
	{
		vDest = pTarget->GetCurPos() - vFaceTo * fDistAbs;
	}

	// �õ�һ����������յ�
	Vector3 vRealDest;

	pathNode nearPos;
	if( !pTarget->get_map()->if_can_direct_go(pTarget->GetCurPos().x, pTarget->GetCurPos().z, vDest.x, vDest.z, nearPos) )
	{
		vRealDest.x = nearPos.x();
		vRealDest.z = nearPos.y();
	}
	else
	{
		vRealDest = vDest;
	}
		
		
	// ��������㲻��ȣ���˲��
	if( pTarget->GetCurPos() != vRealDest )
	{
		// ���ͻ��˷�����Ϣ
		NET_SIS_special_move send;
		send.dw_role_id = pTarget->GetID();
		send.eType = ESMT_Teleport;
		send.vDestPos = vRealDest;
		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

		// ˲�ƣ�����������Ϣ
		pTarget->GetMoveData().ForceTeleport(vRealDest, FALSE);
	}
}

//----------------------------------------------------------------------------------
// ����Ч�����������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectInterruptSpell(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	// ��ϼ���
	pTarget->GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Null, FALSE, TRUE);
}
//----------------------------------------------------------------------------------
// ����Ч����ѣ��
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectDizzy(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_Dizzy) : pTarget->UnsetState(ES_Dizzy);

	if (bSet && pTarget->IsRole() && pSrc != pTarget)
	{
		((Role*)pTarget)->OnPassiveSkillBuffTrigger(pSrc, ETEE_Be_Dizzy);
	}
}

//----------------------------------------------------------------------------------
// ����Ч��������
// dwEffectMisc1�����˵ľ���
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectRepel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) ) return;

	INT nTileDist = (INT)dwEffectMisc1;
	if( 0 >= nTileDist ) return;

	// �õ�����Ŀ��֮�������
	Vector3 vVec = pTarget->GetCurPos() - pSrc->GetCurPos();
	// �Ը��������й�һ��
	Vec3Normalize(vVec);

	// �õ�һ����������յ�
	Vector3 vRealDest = pTarget->GetCurPos();
	for (int i = 1; i <= nTileDist; i++)
	{
		FLOAT fDistAbs = FLOAT(i * TILE_SCALE);	// ���Ծ���
		Vector3 vDest = pTarget->GetCurPos() + vVec * fDistAbs;
		if (pTarget->get_map()->if_can_go(vDest.x, vDest.z))
		{
			vRealDest = vDest;
		}
		else
		{
			break;
		}
	}

	// ��������㲻��ȣ���˲��
	if( pTarget->GetCurPos() != vRealDest )
	{
		// ���ͻ��˷�����Ϣ
		NET_SIS_special_move send;
		send.dw_role_id = pTarget->GetID();
		send.eType = ESMT_Repel;
		send.vDestPos = vRealDest;
		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

		// ˲�ƣ�����������Ϣ
		pTarget->GetMoveData().ForceTeleport(vRealDest, FALSE);

		// ������ǰ�ͷŵļ���
		pTarget->GetCombatHandler().End();

	}

}
//----------------------------------------------------------------------------------
// ����Ч�������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectAssault(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) || pSrc != pTarget ) return;	// ��漼�ܱ����Լ��ͷ�

	// �õ���һĿ����
	DWORD dwTargetUnitID = pTarget->GetCombatHandler().GetTargetUnitID();

	// �ڵ�ͼ�ڲ��Ҹ�Ŀ��
	Unit* pUnit = pTarget->get_map()->find_unit(dwTargetUnitID);
	if( !VALID_POINT(pUnit) ) return;

	// todo��Ҫȡ��Ŀ���ǰ�����꣬�����ȿ���Ч����ȡĿ������
	Vector3 vPosUnit	=	pUnit->GetCurPos();
	Vector3 vPosTarget	=	pTarget->GetCurPos();

	// �õ�����֮��ľ���
	FLOAT fDist = Vec3Dist(vPosUnit, vPosTarget);
	if( fDist <= 0.001f ) return;

	Vector3 vVec = vPosUnit - vPosTarget;	// �õ�����֮������

	// ���þ����ȥ��ҵ����ߵİ�����֮��
	FLOAT fPullDist = fDist - pSrc->GetSize().z - pTarget->GetSize().z;
	vVec *= ( fPullDist / fDist );

	// ��Ŀ��ĵ�ǰ������ϸ�����������ק��Ŀ���
	vPosTarget += vVec;

	// �鿴�ܷ�ͨ��
	
	pathNode temp;
	if (!pTarget->get_map()->if_can_direct_go(vPosUnit.x, vPosUnit.z, vPosTarget.x, vPosTarget.z, temp))
		return ;

	// ���Գ���ȥ��������Ϣ
	NET_SIS_special_move send;
	send.dw_role_id = pTarget->GetID();
	send.eType = ESMT_Assault;
	send.vDestPos = vPosTarget;
	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

	// ˲�ƹ�ȥ
	pTarget->GetMoveData().ForceTeleport(vPosTarget, FALSE);

}
//----------------------------------------------------------------------------------
// ����Ч������ײ
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectCollid(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) ) return;

	INT nTileDist = (INT)dwEffectMisc1;
	if( 0 >= nTileDist ) return;

	// �õ�����Ŀ��֮�������
	Vector3 vVec = pTarget->GetCurPos() - pSrc->GetCurPos();
	// �Ը��������й�һ��
	Vec3Normalize(vVec);

	// �õ�һ����������յ�
	Vector3 vRealDest = pTarget->GetCurPos();
	Vector3 vRealDestSelf = pSrc->GetCurPos();
	for (int i = 1; i <= nTileDist; i++)
	{
		FLOAT fDistAbs = FLOAT(i * TILE_SCALE);	// ���Ծ���
		Vector3 vDest = pTarget->GetCurPos() + vVec * fDistAbs;
		Vector3 vDestSelf = pSrc->GetCurPos() + vVec * fDistAbs;
		if (pTarget->get_map()->if_can_go(vDest.x, vDest.z))
		{
			vRealDest = vDest;
			vRealDestSelf = vDestSelf;
		}
		else
		{
			break;
		}
	}

	// ��������㲻��ȣ���˲��
	if( pTarget->GetCurPos() != vRealDest )
	{
		// ���ͻ��˷�����Ϣ
		NET_SIS_special_move send;
		send.dw_role_id = pTarget->GetID();
		send.eType = ESMT_Repel;
		send.vDestPos = vRealDest;
		pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

		// ˲�ƣ�����������Ϣ
		pTarget->GetMoveData().ForceTeleport(vRealDest, FALSE);
		pTarget->GetMoveData().StopMove();

		// ������ǰ�ͷŵļ���
		pTarget->GetCombatHandler().End();

		NET_SIS_special_move send1;
		send1.dw_role_id = pSrc->GetID();
		send1.eType = ESMT_Assault;
		send1.vDestPos = vRealDestSelf;
		pSrc->get_map()->send_big_visible_tile_message(pSrc, &send1, send1.dw_size);

		// ˲�ƣ�����������Ϣ
		pSrc->GetMoveData().ForceTeleport(vRealDestSelf, FALSE);

		// ������ǰ�ͷŵļ���
		pSrc->GetCombatHandler().End();
	}
}
//----------------------------------------------------------------------------------
// ����Ч�����⼼
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectNoSkill(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_NoSkill) : pTarget->UnsetState(ES_NoSkill);
}
//----------------------------------------------------------------------------------
// ����Ч������˯
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectSpor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_Spor) : pTarget->UnsetState(ES_Spor);
}
//----------------------------------------------------------------------------------
// ����Ч��������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectTie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_Tie) : pTarget->UnsetState(ES_Tie);
}
//----------------------------------------------------------------------------------
// ����Ч������ɢ
// dwEffectMisc1��
//	1��������ɫ�������һ������״̬
//	2��������ɫ�������һ���к�״̬
//	3��������ɫ�������һ��ĳ�ֿ������͵�״̬
//  4��������ɫ����ָ��ID��״̬
//	5��������ɫ����ָ��Ч�����͵�buff
// dwEffectMisc2������dwEffectMisc1Ϊ3ʱ����Ϊ��Ӧ��״̬��������
// dwEffectMisc2������dwEffectMisc1Ϊ5ʱ����Ϊ��Ӧ��Ч�����ͣ�bSet��ȥ�����һ������������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectDispel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	switch(dwEffectMisc1)
	{
	case 1:
		pTarget->DispelBuff(TRUE);
		break;

	case 2:
		pTarget->DispelBuff(FALSE);
		break;

	case 3:
		pTarget->DispelBuff((EBuffResistType)dwEffectMisc2);
		break;

	case 4:
		pTarget->DispelBuff(dwEffectMisc2);
		break;

	default:
		break;
	}

}

//----------------------------------------------------------------------------------
// ����Ч����������ɫ����ָ��Ч�����͵�buff
// dwEffectMisc2��ȥ�����һ��������ȥ������  dwEffectMisc1����Ӧ��Ч������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectCancel(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT(VALID_POINT(pTarget));

	pTarget->DispelBuff(dwEffectMisc1, (BOOL)dwEffectMisc2);
}

//----------------------------------------------------------------------------------
// ����Ч�����޵�
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectInvincible(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_Invincible) : pTarget->UnsetState(ES_Invincible);
}
//----------------------------------------------------------------------------------
// ����Ч��������
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectInstantDie(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT(VALID_POINT(pTarget));

	pTarget->ChangeHP(-pTarget->GetAttValue(ERA_HP), pSrc, NULL, pProto);
}
//----------------------------------------------------------------------------------
// ����Ч������ק
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectPull(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) ) return;

	Vector3 vPosSrc		=	pSrc->GetCurPos();
	Vector3 vPosTarget	=	pTarget->GetCurPos();

	// �õ�����֮��ľ���
	FLOAT fDist = Vec3Dist(vPosSrc, vPosTarget);
	if( fDist <= 0.001f ) return;

	Vector3 vVec = vPosSrc - vPosTarget;	// �õ�����֮������

	// ���þ����ȥ��ҵ����ߵİ�����֮��
	FLOAT fPullDist = fDist - pSrc->GetSize().z - pTarget->GetSize().z;
	vVec *= ( fPullDist / fDist );

	// ��Ŀ��ĵ�ǰ������ϸ�����������ק��Ŀ���
	vPosTarget += vVec;

	// ֪ͨ�ͻ���˲��
	NET_SIS_special_move send;
	send.dw_role_id = pTarget->GetID();
	send.eType = ESMT_Teleport;
	send.vDestPos = vPosTarget;
	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

	// ˲��Ŀ��
	pTarget->GetMoveData().ForceTeleport(vPosTarget, FALSE);

}
//----------------------------------------------------------------------------------
// ����Ч����ת��
// dwEffectMisc1���������յ���
// dwEffectMisc2��ת���İٷֱ�
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectHPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( FALSE == VALID_POINT(pSrc) )
		return;

	if( pTarget->IsDead() || pSrc->IsDead() )
		return;

	INT nHP = (INT)dwEffectMisc1;
	INT nPct = (INT)dwEffectMisc2;

	nHP = min(pTarget->GetAttValue(ERA_HP), nHP);

	if( nHP <= 0 || nPct <= 0 ) return;

	// �����ȥѪ��
	pTarget->ChangeHP(-nHP, pSrc, NULL, pProto);

	// �Է�����Ѫ��
	INT nHPAdd = INT((FLOAT)nHP * ((FLOAT)nPct / 10000.0f));
	pSrc->ChangeHP(nHPAdd, pTarget, NULL, pProto);
}
//----------------------------------------------------------------------------------
// ����Ч����ת��
// dwEffectMisc1���������յ���
// dwEffectMic2��ת���İٷֱ�
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectMPTransfer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( FALSE == VALID_POINT(pSrc) )
		return;

	if( pTarget->IsDead() || pSrc->IsDead() )
		return;

	INT nMP = (INT)dwEffectMisc1;
	INT nPct = (INT)dwEffectMisc2;

	nMP = min(pTarget->GetAttValue(ERA_MP), nMP);

	if( nMP <= 0 || nPct <= 0 ) return;

	// �����ȥ����
	pTarget->ChangeMP(-nMP);

	// �Է���������
	INT nMPAdd = INT((FLOAT)nMP * ((FLOAT)nPct / 10000.0f));
	pSrc->ChangeMP(nMPAdd);
}
//----------------------------------------------------------------------------------
// ����Ч��������
// dwEffectMisc1��<100000���ָ�Ѫ�������̶�ֵ��>100000���ָ�Ѫ�������ٷֱ�
// dwEffectMisc2�����Ϊ1���򲻻����µ�����
//----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectRevive(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( FALSE == pTarget->IsRole() ) return;
	if( FALSE == pTarget->IsDead() ) return;

	Role* pRole = static_cast<Role*>(pTarget);

	INT nHPMPRegain = INT(dwEffectMisc1);

	if( nHPMPRegain <= 0 ) return;

	INT nHP = 0;
	INT nMP = 0;
	if( nHPMPRegain < 100000 )
	{
		nHP = nMP = nHPMPRegain;
	}
	else
	{
		nHP = INT((FLOAT)pRole->GetAttValue(ERA_MaxHP) * (FLOAT(nHPMPRegain  - 100000) / 10000.0f));
		nMP = INT((FLOAT)pRole->GetAttValue(ERA_MaxMP) * (FLOAT(nHPMPRegain  - 100000) / 10000.0f));
	}

	if( nHP <= 0 ) return;

	// ��ұ�����
	pRole->BeRevived(nHP, nMP, pSrc);
}
//-----------------------------------------------------------------------------------
// ����Ч����˲����ȴ
// dwEffectMisc1����Ӧ��Ӧ���������ͣ����Ϊ-1����Ϊ���м���
// dwEffectMisc2������Ĳ�ɾ���ļ���
//-----------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectInstantCD(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	pTarget->ClearAllSkillCoodDown((ETalentType)dwEffectMisc1, dwEffectMisc2);
}
//------------------------------------------------------------------------------------
// ����Ч��������
//------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectLurk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT( VALID_POINT(pTarget) );

	if( bSet )
	{
		pTarget->Lurk((INT)dwEffectMisc1);
	}
	else
	{
		pTarget->UnLurk();
	}
}
//-------------------------------------------------------------------------------------
// ����Ч��������
//-------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectFly(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{

}
//-------------------------------------------------------------------------------------
// ����Ч�����ı���
// dwEffectMisc1���ı��޵İٷֱ�
//-------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectChangeEnmity(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	pSrc->ChangeEnmityCreatureValue(dwEffectMisc1);
}
//-------------------------------------------------------------------------------------
// ����Ч��������
// dwEffectMisc1��
//	0���ص���ҵĳ����¼��
//	1��������ҵ�����ĳ��У��ݲ�ʵ�֣�
//	2������ͼ�������
//-------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectTransmit(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pTarget) || FALSE == pTarget->IsRole() )
		return;

	Role* pTargetRole = static_cast<Role*>(pTarget);

	if( 0 == dwEffectMisc1 )
	{
		pTargetRole->ReturnCity();
	}
	else if( 1 == dwEffectMisc1 )
	{
		DWORD dwGuildMapID = get_tool()->crc32(szGuildMapName);
		const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dwGuildMapID);
		if(!VALID_POINT(pInstance))
			return;

		// �õ�Ŀ���ͼ�ĵ�����
		const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwGuildMapID);
		if( !VALID_POINT(pMapInfo) ) return;

		const tag_map_way_point_info_list* pList = NULL;
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
		if( !VALID_POINT(pList) ) return;
	

		// ��Ŀ�굼�����б�����ȡһ��������
		tag_way_point_info wayPoint;
		pList->list.rand_find(wayPoint);

		Vector3 vFace;
		vFace.y = 0;
		vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

		pTargetRole->GotoNewMap(dwGuildMapID, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);
	}
	else if( 2 == dwEffectMisc1 )
	{

		// �õ�Ŀ���ͼ�ĵ�����
		const tag_map_info* pMapInfo = pTargetRole->get_map()->get_map_info();
		if( !VALID_POINT(pMapInfo) ) return;

		const tag_map_way_point_info_list* pList = NULL;
		pList = pMapInfo->map_way_point_list.find(get_tool()->crc32(_T("r1")));
		if( !VALID_POINT(pList) ) return;


		// ��Ŀ�굼�����б�����ȡһ��������
		tag_way_point_info wayPoint;
		pList->list.rand_find(wayPoint);

		Vector3 vFace;
		vFace.y = 0;
		vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

		pTargetRole->GotoNewMap(pTargetRole->get_map()->get_map_id(), wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

	}
}
//-------------------------------------------------------------------------------------
// ����Ч�����ɼ�����
//-------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectGather(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if ( 0 == dwEffectMisc1 )
	{
		GatherEffect(pTarget, pSrc, dwEffectMisc1, dwEffectMisc2, bSet);
	}
	else if ( 1 == dwEffectMisc1)
	{
		InvesEffect(pTarget, pSrc, dwEffectMisc1, dwEffectMisc2, bSet);
	}
}

//-------------------------------------------------------------------------------------
// ����Ч������е
//-------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectDisArm(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_DisArm) : pTarget->UnsetState(ES_DisArm);
}

//---------------------------------------------------------------------------------------
// ����λ��
//---------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectExchangePos(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) ) return;

	// �ֱ�õ����Եĵ�ǰ����
	Vector3 vPosSrc = pSrc->GetCurPos();
	Vector3 vPosTarget = pTarget->GetCurPos();

	// ���͸�Դ��
	NET_SIS_special_move send1;
	send1.dw_role_id = pSrc->GetID();
	send1.eType = ESMT_Teleport;
	send1.vDestPos = vPosTarget;
	pSrc->get_map()->send_big_visible_tile_message(pSrc, &send1, send1.dw_size);

	// ���͸�Ŀ�귽
	NET_SIS_special_move send2;
	send2.dw_role_id = pTarget->GetID();
	send2.eType = ESMT_Teleport;
	send2.vDestPos = vPosSrc;
	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send2, send2.dw_size);

	// ˲��Դ���
	pSrc->GetMoveData().ForceTeleport(vPosTarget, FALSE);

	// ˲��Ŀ�����
	pTarget->GetMoveData().ForceTeleport(vPosSrc, FALSE);
}

//----------------------------------------------------------------------------------------
// ����
// dwEffectMisc1�� Ҫɾ����BuffID
// dwEffectMisc2�� Ҫ��ӵ�BuffTypeID
//----------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectExplode(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	DWORD dwSrcBuffID		=	dwEffectMisc1;		// Ҫȥ����BuffID
	DWORD dwDestBuffTypeID	=	dwEffectMisc2;		// Ҫ��ӵ�Buff��TypeID

	// ���Ҫ����ԴBuff�������Ŀ������û�У��򷵻�
	if( VALID_POINT(dwSrcBuffID) )
	{
		if( !pTarget->IsHaveBuff(dwSrcBuffID) )
			return;
	}

	// ��ɾ��
	if( VALID_POINT(dwSrcBuffID) )
	{
		pTarget->RemoveBuff(dwSrcBuffID, TRUE);
	}

	// �����
	if( VALID_POINT(dwEffectMisc2) )
	{
		const tagBuffProto* pProto = AttRes::GetInstance()->GetBuffProto(dwDestBuffTypeID);
		if( VALID_POINT(pProto) )
		{
			pTarget->TryAddBuff(pSrc, pProto, NULL, NULL, NULL);
		}
	}
}

//----------------------------------------------------------------------------------------
// �־�
//----------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectFunk(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	bSet ? pTarget->SetState(ES_Feat) : pTarget->UnsetState(ES_Feat);
}

//-----------------------------------------------------------------------------------------
// ׷��
// dwEffectMisc1: 0������� 1��Ŀ��ǰ�� 2��Ŀ���
//-----------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectPursue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) || pTarget != pSrc ) return;		// ׷��Ҳһ�����ͷ���������ӵ�buff

	// ȡ��target�ĵ�һĿ��
	DWORD dwTargetUnitID = pTarget->GetCombatHandler().GetTargetUnitID();
	Unit* pUnit = pTarget->get_map()->find_unit(dwTargetUnitID);

	if( !VALID_POINT(pUnit) ) return;

	// ȡ�����Ŀ���λ��
	Vector3 vUnitPos = pUnit->GetCurPos();

	// ����dwEffectMisc1��ȡĿ���
	Vector3 vDest;

	if( 0 == dwEffectMisc1 )	// ֱ�������
	{
		Vector3 vTargetPos	=	pTarget->GetCurPos();

		// ��������֮��ľ���
		FLOAT fDist = Vec3Dist(vUnitPos, vTargetPos);
		if( fDist <= 0.001f ) return;

		Vector3 vVec = vUnitPos - vTargetPos;	// �õ�����֮������

		// ���þ����ȥ��ҵ����ߵİ�����֮��
		FLOAT fPursueDist = fDist - pSrc->GetSize().z - pTarget->GetSize().z;
		vVec *= ( fPursueDist / fDist );

		// ��Ŀ��ĵ�ǰ������ϸ�����������ק��Ŀ���
		vDest = pTarget->GetCurPos() + vVec;
	}
	else if( 1 == dwEffectMisc1 )	// Ŀ����ǰ
	{
		Vector3 vFace = pUnit->GetFaceTo();
		Vec3Normalize(vFace);		// ��һ��

		vFace *= pUnit->GetSize().z + pTarget->GetSize().z;

		vDest = vUnitPos + vFace;
	}
	else if( 2 == dwEffectMisc1 )
	{
		Vector3 vFace = pUnit->GetFaceTo();
		Vec3Normalize(vFace);		// ��һ��

		vFace *= -(pUnit->GetSize().z + pTarget->GetSize().z);

		vDest = vUnitPos + vFace;
	}
	else
	{
		return;
	}

	// ���͸��ͻ���˲����Ϣ
	NET_SIS_special_move send;
	send.dw_role_id = pTarget->GetID();
	send.eType = ESMT_Teleport;
	send.vDestPos = vDest;
	pTarget->get_map()->send_big_visible_tile_message(pTarget, &send, send.dw_size);

	// ˲��һ��
	pTarget->GetMoveData().ForceTeleport(vDest, FALSE);

	// �ı�һ�³���
	pTarget->GetMoveData().SetFaceTo(pUnit->GetCurPos() - pTarget->GetCurPos());

}

//------------------------------------------------------------------------------------------
// ������ʱ��
//------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectNoPrepare(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT(VALID_POINT(pTarget));

	// ��ȡ����ʱ��Ӱ��ٷֱ�(+����������ʱ�䣻-����������ʱ��)
	INT nSkillPrepareModPct = (INT)dwEffectMisc1;

	if( bSet )
	{
		pTarget->GetCombatHandler().ModSkillPrepareModPct(nSkillPrepareModPct);
	}
	else
	{
		pTarget->GetCombatHandler().ModSkillPrepareModPct(-nSkillPrepareModPct);
	}
}

//-------------------------------------------------------------------------------------------
// ˮ������
//-------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectOnWater(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !pTarget->IsRole() ) return;

	Role* pRole = static_cast<Role*>(pTarget);

	if( bSet )
	{
		if( !pRole->IsInRoleState(ERS_WaterWalk) )
		{
			pRole->SetRoleState(ERS_WaterWalk);
		}
	}
	else
	{
		if( pRole->IsInRoleState(ERS_WaterWalk) )
		{
			pRole->UnsetRoleState(ERS_WaterWalk);
		}
	}
}

//--------------------------------------------------------------------------------------------
// ��·��Ѫ
//--------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectMoveHPDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT(VALID_POINT(pTarget));

	INT nVal = (INT)dwEffectMisc2;		// ��ֵΪ�ӣ���ֵΪ��

	switch(dwEffectMisc1)
	{
	case 1:		// ��Ѫ
		pTarget->ChangeHP(nVal, pSrc, NULL, pProto);
		break;
	case 2:		// ����
		pTarget->ChangeMP(nVal);
		break;
	}
}

//---------------------------------------------------------------------------------------------
// ���ӻ���
//---------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectIgnoreArmor(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	ASSERT(VALID_POINT(pTarget));

	// ��ȡ����Ӱ��ٷֱ�(ֻ��Ϊ��ֵ)
	INT nArmorDecPct = (INT)dwEffectMisc1;

	if( bSet )
	{
		pTarget->GetCombatHandler().ModTargetArmorLeftPct(-nArmorDecPct);
	}
	else
	{
		pTarget->GetCombatHandler().ModTargetArmorLeftPct(nArmorDecPct);
	}
}

//----------------------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectSneer(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if(!VALID_POINT(pTarget) || !VALID_POINT(pSrc) )	return;

	if( !pTarget->IsCreature() ) return;
	Creature* pCreature	= static_cast<Creature *>(pTarget);
	if( !VALID_POINT(pCreature->GetAI()) ) return; 

	if( bSet )
	{
		DWORD dwMaxEnmityUnitID = pCreature->GetAI()->GetCurrentVictim();
		if( dwMaxEnmityUnitID == pSrc->GetID() )
			return;

		//INT nEnmityMod = pCreature->GetAI()->GetEnmityValue(dwMaxEnmityUnitID) - pCreature->GetAI()->GetEnmityValue(pSrc->GetID())+1;
		INT nEnmityMod = pCreature->GetAI()->GetEnmityValue(dwMaxEnmityUnitID) * 10;
		// ���ӳ�޼ӳ�ֵ
		pCreature->GetAI()->AddEnmityMod(pSrc, nEnmityMod);
		pCreature->GetAI()->SetCurrentVictim(pSrc->GetID());
	}
	else
	{
		pCreature->GetAI()->ClearEnmityModValue(pSrc->GetID());
		/*DWORD dwMaxEnmityID = pCreature->GetAI()->GetCurrentVictim();
		pCreature->GetAI()->SetCurrentVictim(dwMaxEnmityID);*/
	}
}

//----------------------------------------------------------------------------------------------
// �ɼ�Ч��
//----------------------------------------------------------------------------------------------
VOID BuffEffect::GatherEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !pTarget->IsCreature() ) return;
	if( !pSrc->IsRole() ) return;

	// �����Դ�ͽ�ɫ
	Creature* pCreature	= static_cast<Creature *>(pTarget);	
	Role* pRole	= static_cast<Role *>(pSrc);	
	if ( !VALID_POINT(pCreature) || !VALID_POINT(pRole) )
		return ;

	if (pCreature->IsDead())
		return;

	if (VALID_POINT(pRole->GetSession()) && pRole->GetSession()->GetFatigueGuarder().GetEarnRate() < 10000)
	{
		return;
	}

	// �ж���Դ����ü���
	Skill* pGatherSkill = NULL;
	//if(pCreature->IsNatuRes() ||  pCreature->IsManRes())
	//	pGatherSkill = pRole->GetSkill(GATHER_SKILL_MINING);
	if ( pCreature->IsNatuRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_MINING);
	else if ( pCreature->IsManRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_HARVEST);
	else if ( pCreature->IsGuildNatuRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_GMINING);
	else if ( pCreature->IsGuildManRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_GHARVEST);

	if ( !VALID_POINT(pGatherSkill) )
		return ;

 	// ����ɫ��������				
 	ItemMgr& itemMgr = pRole->GetItemMgr();	
 	if (itemMgr.GetBagFreeSize() <= 0)
 	{
 		NET_SIS_role_pickup_item send;
 		send.dw_error_code	= E_Loot_BAG_NOT_ENOUGH_SPACE;
 		send.dw_role_id		= pRole->GetID();
 		pRole->SendMessage(&send, send.dw_size);
 		return ;
 	}
 
 	// ����Դ���빻�� 
 	//if (!pRole->IsInCombatDistance(*pCreature, pGatherSkill->GetOPDist()))
 	//	return ;	
 
 	// ��Դ�ѱ�ռ��
 	//if( pCreature->IsTagged() )
 	//	return;
	
	// ��֤���ж�
	if (VALID_POINT(pRole->GetSession()))
	{
		pRole->GetSession()->GetVerification().beginVerification(pRole, FALSE);
	}

	// �Բ����ʵ�����Դ 	
	//if (pCreature->GetPlantDataIndex() == INVALID_VALUE)
	//{
	//	if ( pCreature->IsGuildNatuRes() )
	//	{
	//		const tagCreatureProto* pProto = pCreature->GetProto();
	//		if (VALID_POINT(pProto) && pProto->dwChutouID != 0)
	//		{
	//			if (pRole->GetItemMgr().GetBag().GetSameItemCount(pProto->dwChutouID) >= 1)
	//			{
	//				//if (pRole->GetItemMgr().IsItemCDTime(pProto->dwChutouID))
	//				//{
	//				//	return;
	//				//}

	//				package_list<tagItem*> itemList;

	//				pRole->GetItemMgr().GetBag().GetSameItemList(itemList, pProto->dwChutouID, pProto->dwChutouID);
	//				pRole->GetItemMgr().ItemUsedSameItem(itemList, 1, elcid_skill);
	//				//pRole->GetItemMgr().Add2CDTimeMap(pProto->dwChutouID);
	//			}
	//			
	//		}
	//	}
	//	g_drop_mgr.resource_drop(pCreature, pRole);

	//	// �������������ӣ������������ܣ�
	//	const tagCreatureProto* pCreatureProto = AttRes::GetInstance()->GetCreatureProto(pCreature->GetTypeID());

	//	// ��������� = ��Դ���ṩ�������ȡ�(1+�������/100)
	//	//INT nExpAdd =  INT((FLOAT)pCreatureProto->nExpGive * (1.0f + (FLOAT)pRole->GetAttValue(ERA_Savvy) / 100.0f));
	//	pRole->ChangeSkillExp(pGatherSkill, 1);

	//	// ��Դ��Ѫ
	//	pCreature->ChangeHP(-1, pSrc);
	//}	
	//else
	//{
	//	if (!pCreature->IsMatrue())
	//	{
	//		return;
	//	}
	//	guild* pGuild = g_guild_manager.get_guild(pCreature->GetGuildID());
	//	if (VALID_POINT(pGuild))
	//	{
	//		tagPlantData* pPlantData = pGuild->getPlantData(pCreature->GetPlantDataIndex());
	//		if (VALID_POINT(pPlantData))
	//		{
	//			if (pPlantData->dw_master_id == pRole->GetID())
	//			{
	//				g_drop_mgr.resource_plant_drop(pCreature, pRole, pPlantData->n_num);

	//				Map* pMap = pCreature->get_map();
	//				if( !VALID_POINT(pMap) ) return;

	//				Creature* pMount = (Creature*)pMap->find_unit(pCreature->GetTuDui());
	//				if( VALID_POINT(pMount ) )
	//				{
	//					pMount->SetUsed(false);	
	//					pGuild->resetPlantData(pCreature->GetPlantDataIndex());
	//					pCreature->ChangeHP(-1, pSrc);
	//				}
	//					
	//			}
	//			else
	//			{
	//				if (pRole->GetDayClearData(ERDCT_GuildPlantTou) < 10 && pPlantData->n_num == pCreature->GetMaxYield())
	//				{
	//					INT nNum = pPlantData->n_num/5;
	//					g_drop_mgr.resource_plant_drop(pCreature, pRole, nNum);
	//					pPlantData->n_num -= nNum;
	//					pRole->ModRoleDayClearDate(ERDCT_GuildPlantTou);
	//					NET_SIS_monster_plant_yield_change send;
	//					send.dw_id = pCreature->GetID();
	//					send.dw_yield = pPlantData->n_num;

	//					if( VALID_POINT(pCreature->get_map()) )
	//					{
	//						pCreature->get_map()->send_big_visible_tile_message(pCreature, &send, send.dw_size);
	//					}
	//					
	//					pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_guild_plant_tou, 1);	

	//					if(VALID_POINT(pRole->GetScript()))
	//						pRole->GetScript()->OnTouPlant(pRole);

	//					s_role_info* pRoleInfo = g_roleMgr.get_role_info(pRole->GetID());
	//					s_role_info* pRoleInfoTarget = g_roleMgr.get_role_info(pPlantData->dw_master_id);
	//					guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	//					if (VALID_POINT(pRoleInfo) && VALID_POINT(pRoleInfoTarget) && pGuild)
	//					{
	//						NET_SIS_Guild_msg send;
	//						//_tcsncpy(send.szName, pRoleInfo->sz_role_name, X_SHORT_NAME);
	//						//_tcsncpy(send.szTargetName, pRoleInfoTarget->sz_role_name, X_SHORT_NAME);
	//						send.byType = 0;
	//						send.nChangeValue = pCreature->GetProto()->dw_data_id;
	//						pGuild->send_guild_message(&send, send.dw_size);
	//					}

	//				}
	//				
	//			}
	//		}
	//	}
	//}

}

//----------------------------------------------------------------------------------------------
// �������
//----------------------------------------------------------------------------------------------
VOID BuffEffect::InvesEffect(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet,  const tagBuffProto* pProto, Buff* pBuff)
{
	if( !pTarget->IsCreature() ) return;
	if( !pSrc->IsRole() ) return;

	// ��õ���ͽ�ɫ
	Creature* pCreature	= static_cast<Creature *>(pTarget);	
	Role* pRole	= static_cast<Role *>(pSrc);	
	if ( !VALID_POINT(pCreature) || !VALID_POINT(pRole) )
		return ;

	if (pCreature->IsDead())
		return;

	// �жϵ��ﲢ��ü���
	Skill* pInvesSkill = NULL;
	if ( pCreature->IsInves() )
		pInvesSkill = pRole->GetSkill(INVES_SKILL);

	if (pCreature->IsInves_two())
		pInvesSkill = pRole->GetSkill(INVES_SKILL_OTHER);

	if ( !VALID_POINT(pInvesSkill) )
		return;

	if(pCreature->IsCarry())
	{
		pCreature->OnInvest(pRole);
		pRole->OnInvestCarryNPC(pCreature->GetProto()->dwCarryID);
		return;
	}

	// ����Դ���빻�� 
	//if (!pRole->IsInCombatDistance(*pCreature, pInvesSkill->GetOPDist()))
	//	return ;	
	 
	// �����ѱ�ռ��
	//if( pCreature->IsTagged() )
	//	return;

	// �Բ����ʵ�����Ʒ	
	g_drop_mgr.investigate_drop(pCreature, pRole);

	// ���񴥷�
	pRole->on_quest_event(EQE_Invest, pTarget->GetID(), pCreature->GetTypeID());

	//== js zhaopeng 2010.04.02
	// ���� ɾ����������ָ����Ʒ
	//if ( pCreature->GetProto()->dwInvesDeleteItem )
	//{
	//	pRole->GetItemMgr().RemoveFromRole( pCreature->GetProto()->dwInvesCheckItem, -1, 0 );
	//}	
	//==

	// ����ű�����
	pCreature->OnInvest(pRole);

	if(pCreature->GetProto()->nType2 != EGOT_Relive)
	{
		// ���ｵѪ
		pCreature->ChangeHP(-1, pSrc);
	}
}

//-----------------------------------------------------------------------------------------------------------------
// ���
//-----------------------------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectMount( Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet/*=TRUE*/, const tagBuffProto* pProto/*=NULL*/ , Buff* pBuff)
{
}

//-----------------------------------------------------------------------------------------------------------------
// �������
//-----------------------------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectMountInvite( Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet/*=TRUE*/, const tagBuffProto* pProto/*=NULL*/ , Buff* pBuff)
{

}

//-----------------------------------------------------------------------------------------------------------------
// �⻷
// dwEffectMisc1: �⻷��ӦbuffID
//-----------------------------------------------------------------------------------------------------------------
VOID BuffEffect::BuffEffectRing(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	//if (pTarget != pSrc)
	//	return;
	if (!VALID_POINT(pTarget))
		return;

	pTarget->SetRingBuffID(pProto->dwID);

	//�ȼ�����Ӱ���Ŀ�� 
	const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(dwEffectMisc1);
	if (NULL == pBuffProto)
		return;

	if (!VALID_POINT(pTarget->get_map()))
		return;

	std::vector<DWORD>& listRingTarget = pTarget->GetRingBuffTargetList();
	//ȡ������Ŀ���Ч��
	if (!bSet)
	{
		pTarget->ClearTargetRingBuff();
		/*for (std::size_t i = 0; i < listRingTarget.size();)
		{
		Unit* pUnit = pTarget->get_map()->find_unit(listRingTarget[i]);
		if (VALID_POINT(pUnit))
		{
		pUnit->RemoveBuff(Buff::GetIDFromTypeID(pBuffProto->dwID), TRUE);
		}

		listRingTarget.erase(listRingTarget.begin() + i);	
		}*/
		return;
	}


	std::vector<Unit*> listTargetUnit;
	pTarget->CalBuffTargetList(pTarget, pProto->eOPType, pProto->fOPDistance, pProto->fOPRadius, 
		pProto->eFriendly, pBuffProto->dwTargetLimit, pProto->dwTargetStateLimit, listTargetUnit, NULL);


	std::vector<Unit*>::iterator iterRing;

	//�Ѳ��ڷ�Χ�ڵĵ�λ��buffɾ��
	for (std::size_t i = 0; i < listRingTarget.size();)
	{
		Unit* pCurUnit = pTarget->get_map()->find_unit(listRingTarget[i]);

		iterRing = std::find(listTargetUnit.begin(), listTargetUnit.end(), pCurUnit);
		if (iterRing == listTargetUnit.end())
		{
			listRingTarget.erase(listRingTarget.begin() + i);
			if (VALID_POINT(pCurUnit))
			{
				pCurUnit->RemoveBuff(Buff::GetIDFromTypeID(pBuffProto->dwID), TRUE);
			}
		}
		else
		{
			++i;
		}
	}

	//���ڷ�Χ�ڵĵ�λ����buff
	std::vector<DWORD>::iterator iterRingID;
	for (std::size_t i = 0; i < listTargetUnit.size(); ++i)
	{
		iterRingID = std::find(listRingTarget.begin(), listRingTarget.end(), listTargetUnit[i]->GetID());
		if (iterRingID == listRingTarget.end())
		{
			if (listTargetUnit[i] == pTarget)
				continue;

			// ͨ������ID�õ�ID�͵ȼ�
			DWORD dwBuffID = Buff::GetIDFromTypeID(pBuffProto->dwID);
			INT nBuffLevel = Buff::GetLevelFromTypeID(pBuffProto->dwID);

			BOOL bSuss = listTargetUnit[i]->TryAddBuff(pTarget, pBuffProto, NULL, NULL, NULL);
			if (bSuss)
			{
				listRingTarget.push_back(listTargetUnit[i]->GetID());
			}
			//Buff* pBuff = pTarget->GetBuffPtr(Buff::GetIDFromTypeID(pProto->dwID));
			//if (!VALID_POINT(pBuff))
			//	continue;
			// �жϵ����͵���

			//INT nIndex = INVALID_VALUE;
			//INT nRet = listTargetUnit[i]->BuffCounteractAndWrap(pTarget, dwBuffID, nBuffLevel, pBuffProto->nLevel, pBuffProto->dwGroupFlag, pBuffProto->bBenifit, nIndex);

			//if( EBCAWR_CanAdd == nRet )
			//{
			//	listTargetUnit[i]->AddBuff(pBuffProto, pTarget, INVALID_VALUE, NULL, nIndex, TRUE);
			//}

		}
	}
}

//-----------------------------------------------------------------------------------------------------------------
// HP���ٷֱȽ���
// 
//-----------------------------------------------------------------------------------------------------------------
VOID BuffEffect::BuffectHPExchange(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if( !VALID_POINT(pSrc) || !VALID_POINT(pTarget))
		return;

	if( pTarget->IsDead() || pSrc->IsDead() )
		return;

	FLOAT fTarHpPct = pTarget->GetAttValue(ERA_HP)*1.0f/pTarget->GetAttValue(ERA_MaxHP);
	FLOAT fSrcHpPct = pSrc->GetAttValue(ERA_HP)*1.0f/pSrc->GetAttValue(ERA_MaxHP);

	pTarget->SetAttValue(ERA_HP, (INT)(fSrcHpPct*pTarget->GetAttValue(ERA_MaxHP)));
	pSrc->SetAttValue(ERA_HP, (INT)(fTarHpPct*pSrc->GetAttValue(ERA_MaxHP)));

}

//-----------------------------------------------------------------------------------------------------------------
// �����˺�
// dwEffectMisc1: 0 �˺����� 1 �˺���Ѫ
// dwEffectMisc2:�˺�������
//-----------------------------------------------------------------------------------------------------------------
VOID BuffEffect::BuffectAbsorbDmg(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!VALID_POINT(pBuff))
		return;

	if (!VALID_POINT(pTarget))
		return;

	switch(dwEffectMisc1)
	{
	case 0:
		{
			DWORD dmg = bSet ? dwEffectMisc2 : 0;
			pTarget->SetAbsorbDmg(TRUE);
			pBuff->SetAbsorbDmg(dmg);
		}
		break;
	case 1:
		pTarget->SetDmgReturn(bSet);
		break;
	default:
		break;
	}
}
//buff��ȡ
VOID BuffEffect::BuffectFilchBuff(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	Buff* pTargetBuff = pTarget->GetFiretBenifitBuff();
	if (!VALID_POINT(pTargetBuff))
		return;

	pSrc->TryAddBuff(pSrc, pTargetBuff->GetProto(), NULL, NULL, NULL);

	pTarget->RemoveBuff(pTargetBuff->GetID(), TRUE);
}
//������
// dwEffectMisc1:���ӳ��������
// dwEffectMisc2:���ӳ��������
VOID BuffEffect::BuffectRonghePet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!VALID_POINT(pSrc) || !VALID_POINT(pTarget) || !pTarget->IsRole())
		return;

	Role* pRole = (Role*)pTarget;
	
	Unit* pTargetUnit = pRole->get_map()->find_unit(pRole->GetCurSkillTarget());

	if (bSet)
	{
		Role* pPetMonster = NULL;
		//�ж�Ŀ���Ƿ����
		PetSoul* pPetSoul = NULL;
		if (VALID_POINT(pTargetUnit) && pTargetUnit->IsRole() )
		{
			pPetMonster = ((Role*)pTargetUnit);

		}
		else
		{
			pPetMonster = pRole;	
		}	


		pPetSoul = pPetMonster->GetPetPocket()->GetCalledPetSoul();

		if (!VALID_POINT(pPetSoul))
			return;
		
		// ��ȡ�����Ǽ�
		INT nAddSkillLevel = 0;
		//tagEquip* pWeapon = pRole->GetWeapon();
		//if (VALID_POINT(pWeapon))
		//{
		//	if (pWeapon->equipSpec.byConsolidateLevel >= 4 && pWeapon->equipSpec.byConsolidateLevel < 7)
		//	{
		//		nAddSkillLevel = 1;
		//	}
		//	else if(pWeapon->equipSpec.byConsolidateLevel >= 7 && pWeapon->equipSpec.byConsolidateLevel < 10)
		//	{
		//		nAddSkillLevel = 2;
		//	}
		//	else if(pWeapon->equipSpec.byConsolidateLevel>=10)
		//	{
		//		nAddSkillLevel = 3;
		//	}
		//}

		INT nAddHP = 500 + 50 *pPetSoul->GetPetAtt().GetAttVal(epa_quality); 
		//pPetSoul->GetPetAtt().ModAttVal(epa_spirit, nAddHP);
		pPetSoul->GetPetAtt().ModAttVal(epa_happy_value, 200);

		pPetSoul->SetHeti(TRUE, pRole, nAddSkillLevel);
		pRole->SetTargetPet( pPetMonster, pPetSoul->GetID());
		if(VALID_POINT(pRole->GetScript()))
			pRole->GetScript()->OnRonghePet(pRole, pPetSoul->GetPetAtt().GetAttVal(epa_quality), TRUE);
	}
	else
	{
		PetHeti::cancelHeti(pRole);
		if(VALID_POINT(pRole->GetScript()))
			pRole->GetScript()->OnRonghePet(pRole, INVALID_VALUE, FALSE);
	}
}

// ȡ��������״̬
VOID BuffEffect::BuffectCancelBeHeti(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!pSrc->IsRole())
		return;

	Role* pRole = (Role*)pSrc;
	
	PetHeti::cancelBeHeTiPet(pRole);
}


// ȡ������
VOID BuffEffect::BUffectCancelHeTi(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
//	if (!pSrc->IsRole())
//		return;
//
//	pSrc->RemoveBuff(HETI_BUFF, TRUE);
}

VOID BuffEffect::BuffectClaerPkValue(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!VALID_POINT(pSrc) || !VALID_POINT(pTarget))
		return;

	if (pTarget->IsRole() && pSrc->IsRole())
	{
		Role* pRole = (Role*)pSrc;
		pRole->SetPKValueMod(-(pRole->GetPKValue()));
	}
}

VOID BuffEffect::BuffectCantAttackUnMount(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!VALID_POINT(pSrc) || !VALID_POINT(pTarget))
		return;

	if (pTarget->IsRole() && pSrc->IsRole())
	{
		Role* pRole = (Role*)pTarget;
		pRole->SetAttackStopMountProtect(bSet);
	}
}

//�ٻ�ս������
// dwEffectMisc1:����id
// dwEffectMisc2:�����ٷֱ�
VOID BuffEffect::BuffectCallAttackPet(Unit* pTarget, Unit* pSrc, DWORD dwEffectMisc1, DWORD dwEffectMisc2, BOOL bSet, const tagBuffProto* pProto, Buff* pBuff)
{
	if (!VALID_POINT(pSrc))
		return;

	if (pSrc->IsRole())
	{
		Role* pRole = (Role*)pSrc;
		if (pRole->GetFlowUnit() == INVALID_VALUE)
		{
			Creature *pCreature = pRole->get_map()->create_creature(dwEffectMisc1, pRole->GetCurPos(), pRole->GetFaceTo());
			if (VALID_POINT(pCreature))
			{
				pCreature->GetAI()->GetTracker()->SetTarget(pRole);
				pCreature->GetAI()->GetTracker()->SetTargetID(pRole->GetID());
				pRole->SetFlowUnit(pCreature->GetID());	
				INT nDaoshuMin = pSrc->GetAttValue(ERA_ArmorEx) * dwEffectMisc2 / 10000;
				INT nDaoshuMax = pSrc->GetAttValue(ERA_ArmorIn) * dwEffectMisc2 / 10000;
				
				pCreature->SetAttValue(ERA_ExAttackMin, nDaoshuMin);
				pCreature->SetAttValue(ERA_InAttackMin, nDaoshuMin);
				pCreature->SetAttValue(ERA_ArmorEx, nDaoshuMin);
				pCreature->SetAttValue(ERA_ExAttackMax, nDaoshuMax);
				pCreature->SetAttValue(ERA_InAttackMax, nDaoshuMax);
				pCreature->SetAttValue(ERA_ArmorIn, nDaoshuMax);

			}
		}
	}
}


