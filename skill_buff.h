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

#pragma once

class Unit;
struct tagBuffProto;

#include "att_res.h"

//-------------------------------------------------------------------------------
// ���ܶ�״̬��Ӱ��
//-------------------------------------------------------------------------------
struct tagBuffMod
{
	BOOL				bValid;											// �Ƿ���Ч
	INT					nPersistTickMod;								// ����ʱ��
	INT					nWarpTimesMod;									// ��������

	INT					nAttackInterruptRateMod;						// ������������
	EBuffEffectMode		eModBuffEffectMode;								// Ӱ������Ľ׶�BuffЧ��
	INT					nEffectMisc1Mod;								// Ч������1�ӳ�
	INT					nEffectMisc2Mod;								// Ч������2�ӳ�
	INT					nEffectAttMod[EBEA_End];						// ĳ���׶�Buff���������Ըı�Ӱ��

	mutable package_map<ERoleAttribute, INT>	mapRoleAttMod;					// ���ܶԸ�buff���������Ӱ��ļӳ�
	mutable package_map<ERoleAttribute, INT>	mapRoleAttModPct;				// ���ܶԸ�buff���������Ӱ��ļӳ�

	mutable package_list<DWORD>				listModifier;					// ����Ӱ���buff�ļ����б�

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
