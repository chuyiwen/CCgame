/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//技能对状态影响

#include "stdafx.h"

#include "../../common/WorldDefine/skill_define.h"
#include "../../common/WorldDefine/buff_define.h"

#include "att_res.h"
#include "skill_buff.h"
#include "world.h"


//------------------------------------------------------------------------------------
// 设置加成
//------------------------------------------------------------------------------------
VOID tagBuffMod::SetMod(const tagSkillProto* pProto)
{
	if( FALSE == VALID_POINT(pProto) ) return;
	if( ESTT_Buff != pProto->eTargetType ) return;

	// 持续时间
	nPersistTickMod += pProto->nBuffPersistTimeAdd / TICK_TIME;

	// 叠加次数
	nWarpTimesMod += pProto->nBuffWarpTimesAdd;

	// 攻击消除几率
	nAttackInterruptRateMod += pProto->nBuffInterruptResistAdd;

	// 效果加成
	if( EBEM_Null != pProto->eModBuffMode )
	{
		// 之前没有过特殊效果加成
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
		// 之前有特殊效果加成，则新技能的特殊效果加成的阶段必须和其一样
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
		// 新技能的特殊效果加成与之前的加成不一样，不会出现这种情况
		else
		{
			SI_LOG->write_log(_T("skill mod buff failed, skill type id is %u\r\n"), pProto->dwID);
		}
	}

	// 人物属性加成
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

	// 将该技能TypeID存入本地list，以便保存
	listModifier.push_back(pProto->dwID);

	// 设置该Mod本身有效
	bValid = TRUE;
}

//------------------------------------------------------------------------------------
// 取消加成
//------------------------------------------------------------------------------------
VOID tagBuffMod::UnSetMod(const tagSkillProto* pProto)
{
	if( FALSE == VALID_POINT(pProto) ) return;
	if( ESTT_Buff != pProto->eTargetType ) return;

	// 持续时间
	nPersistTickMod -= pProto->nBuffPersistTimeAdd / TICK_TIME;

	// 叠加次数
	nWarpTimesMod -= pProto->nBuffWarpTimesAdd;

	// 攻击消除几率
	nAttackInterruptRateMod -= pProto->nBuffInterruptResistAdd;

	// 效果加成
	if( EBEM_Null != pProto->eModBuffMode )
	{
		// 有特殊效果加成，则技能的特殊效果加成的阶段必须和其一样
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

	// 人物属性加成
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
