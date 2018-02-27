/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ܶ�״̬Ӱ��

#include "stdafx.h"

#include "../../common/WorldDefine/skill_define.h"
#include "../../common/WorldDefine/buff_define.h"

#include "att_res.h"
#include "skill_buff.h"
#include "world.h"


//------------------------------------------------------------------------------------
// ���üӳ�
//------------------------------------------------------------------------------------
VOID tagBuffMod::SetMod(const tagSkillProto* pProto)
{
	if( FALSE == VALID_POINT(pProto) ) return;
	if( ESTT_Buff != pProto->eTargetType ) return;

	// ����ʱ��
	nPersistTickMod += pProto->nBuffPersistTimeAdd / TICK_TIME;

	// ���Ӵ���
	nWarpTimesMod += pProto->nBuffWarpTimesAdd;

	// ������������
	nAttackInterruptRateMod += pProto->nBuffInterruptResistAdd;

	// Ч���ӳ�
	if( EBEM_Null != pProto->eModBuffMode )
	{
		// ֮ǰû�й�����Ч���ӳ�
		if( EBEM_Null == eModBuffEffectMode )
		{
			eModBuffEffectMode = pProto->eModBuffMode;
			nEffectMisc1Mod = pProto->nBuffMisc1Add;
			nEffectMisc2Mod = pProto->nBuffMisc2Add;

			if( EBEM_Persist != pProto->eModBuffMode )
			{
				get_fast_code()->memory_copy(nEffectAttMod, pProto->nBuffEffectAttMod, sizeof(nEffectAttMod));
			}
		}
		// ֮ǰ������Ч���ӳɣ����¼��ܵ�����Ч���ӳɵĽ׶α������һ��
		else if( eModBuffEffectMode == pProto->eModBuffMode )
		{
			nEffectMisc1Mod += pProto->nBuffMisc1Add;
			nEffectMisc2Mod += pProto->nBuffMisc2Add;

			if( EBEM_Persist != pProto->eModBuffMode )
			{
				for(INT n = 0; n < EBEA_End; ++n)
				{
					nEffectAttMod[n] += pProto->nBuffEffectAttMod[n];
				}
			}
		}
		// �¼��ܵ�����Ч���ӳ���֮ǰ�ļӳɲ�һ������������������
		else
		{
			SI_LOG->write_log(_T("skill mod buff failed, skill type id is %u\r\n"), pProto->dwID);
		}
	}

	// �������Լӳ�
	ERoleAttribute eAtt = ERA_Null;
	INT nMod = 0;

	package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
	package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
	while( mapMod.find_next(itMod, eAtt, nMod) )
	{
		mapRoleAttMod.modify_value(eAtt, nMod);
	}

	package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
	package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
	while( mapModPct.find_next(itModPct, eAtt, nMod) )
	{
		mapRoleAttModPct.modify_value(eAtt, nMod);
	}

	// ���ü���TypeID���뱾��list���Ա㱣��
	listModifier.push_back(pProto->dwID);

	// ���ø�Mod������Ч
	bValid = TRUE;
}

//------------------------------------------------------------------------------------
// ȡ���ӳ�
//------------------------------------------------------------------------------------
VOID tagBuffMod::UnSetMod(const tagSkillProto* pProto)
{
	if( FALSE == VALID_POINT(pProto) ) return;
	if( ESTT_Buff != pProto->eTargetType ) return;

	// ����ʱ��
	nPersistTickMod -= pProto->nBuffPersistTimeAdd / TICK_TIME;

	// ���Ӵ���
	nWarpTimesMod -= pProto->nBuffWarpTimesAdd;

	// ������������
	nAttackInterruptRateMod -= pProto->nBuffInterruptResistAdd;

	// Ч���ӳ�
	if( EBEM_Null != pProto->eModBuffMode )
	{
		// ������Ч���ӳɣ����ܵ�����Ч���ӳɵĽ׶α������һ��
		if( eModBuffEffectMode == pProto->eModBuffMode )
		{
			nEffectMisc1Mod -= pProto->nBuffMisc1Add;
			nEffectMisc2Mod -= pProto->nBuffMisc2Add;

			if( EBEM_Persist != pProto->eModBuffMode )
			{
				for(INT n = 0; n < EBEA_End; ++n)
				{
					nEffectAttMod[n] -= pProto->nBuffEffectAttMod[n];
				}
			}
		}
	}

	// �������Լӳ�
	ERoleAttribute eAtt = ERA_Null;
	INT nMod = 0;

	package_map<ERoleAttribute, INT>& mapMod = pProto->mapRoleAttMod;
	package_map<ERoleAttribute, INT>::map_iter itMod = mapMod.begin();
	while( mapMod.find_next(itMod, eAtt, nMod) )
	{
		mapRoleAttMod.modify_value(eAtt, -nMod);
	}

	package_map<ERoleAttribute, INT>& mapModPct = pProto->mapRoleAttModPct;
	package_map<ERoleAttribute, INT>::map_iter itModPct = mapModPct.begin();
	while( mapModPct.find_next(itModPct, eAtt, nMod) )
	{
		mapRoleAttModPct.modify_value(eAtt, -nMod);
	}

	listModifier.erase(const_cast<tagSkillProto*>(pProto)->dwID);
}
