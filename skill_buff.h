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

#pragma once

class Unit;
struct tagBuffProto;

#include "att_res.h"

//-------------------------------------------------------------------------------
// 技能对状态的影响
//-------------------------------------------------------------------------------
struct tagBuffMod
{
	BOOL				bValid;											// 是否有效
	INT					nPersistTickMod;								// 持续时间
	INT					nWarpTimesMod;									// 叠加上限

	INT					nAttackInterruptRateMod;						// 攻击消除几率
	EBuffEffectMode		eModBuffEffectMode;								// 影响的是哪阶段Buff效果
	INT					nEffectMisc1Mod;								// 效果参数1加成
	INT					nEffectMisc2Mod;								// 效果参数2加成
	INT					nEffectAttMod[EBEA_End];						// 某个阶段Buff的人物属性改变影响

	mutable package_map<ERoleAttribute, INT>	mapRoleAttMod;					// 技能对该buff对玩家属性影响的加成
	mutable package_map<ERoleAttribute, INT>	mapRoleAttModPct;				// 技能对该buff对玩家属性影响的加成

	mutable package_list<DWORD>				listModifier;					// 所有影响该buff的技能列表

	tagBuffMod()
	{
		bValid = FALSE;
		nPersistTickMod = 0;
		nWarpTimesMod = 0;

		nAttackInterruptRateMod = 0;
		eModBuffEffectMode = EBEM_Null;
		nEffectMisc1Mod = 0;
		nEffectMisc2Mod = 0;

		ZeroMemory(nEffectAttMod, sizeof(nEffectAttMod));
	}

	VOID Clear()
	{
		bValid = FALSE;
		nPersistTickMod = 0;
		nWarpTimesMod = 0;

		nAttackInterruptRateMod = 0;
		eModBuffEffectMode = EBEM_Null;
		nEffectMisc1Mod = 0;
		nEffectMisc2Mod = 0;

		ZeroMemory(nEffectAttMod, sizeof(nEffectAttMod));

		mapRoleAttMod.clear();
		mapRoleAttModPct.clear();
		listModifier.clear();
	}

	BOOL IsValid() const { return bValid; }

	VOID SetMod(const tagSkillProto* pProto);
	VOID UnSetMod(const tagSkillProto* pProto);
};
