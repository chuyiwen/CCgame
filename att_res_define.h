/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once

//-----------------------------------------------------------------------------
// Ʒ��Ӱ���װ��������ز���
//-----------------------------------------------------------------------------
struct tagEquipQltyEffect
{
	// ��������Ӱ��ϵ��
	//DOUBLE fWeaponFactor;
	//DOUBLE fArmorFactor;
	
	// ����������ֵ
	INT32 EquipAddAtt[ADDATT_TYPE_NUM];	
	
	// ���츽��������ֵ
	//INT32 EquipAddAttProduct[ADDATT_TYPE_NUM];	
	
	// ϴ���������ֵ
	//INT32 nXiliAtt;

	// һ������
	INT32 nAttAFactor;
	//DOUBLE fAttAFactor;
	INT32 nAttANumEffect;

	// Ǳ��ֵ
	//FLOAT fPotFactor;

	// ��Ƕ������ -- ��¼���ֵļ���
	INT32 nHoleNumPct[MAX_EQUIPHOLE_NUM + 1];

	// ��������
	//INT32 nSpecAttPct;
};

//-----------------------------------------------------------------------------
// ��������֮���Ӱ��ṹ
//-----------------------------------------------------------------------------
struct tagSkillModify
{
	package_list<DWORD>	listModify;		// ֱ��Ӱ��������ܵĴӼ���
};

//-----------------------------------------------------------------------------
// ʱװ���ɹ�����ض���
//-----------------------------------------------------------------------------
struct tagFashionGen
{
	FLOAT		fAppearanceFactor;	// ��������(AppearancePct)
	INT16		n16ReinPct;			// ͳ�����Լӳ�(ReinPct)
	INT16		n16SavvyPct;		// �������Լӳ�(SavvyPct)
	INT16		n16FortunePct;		// ��Ե���Լӳ�(FortunePct)
	INT8		n8ReinVal;			// ֵ=��Ʒ�ȼ���ReinVal[ȡ��](ReinVal)
	INT8		n8SavvyVal;			// ֵ=��Ʒ�ȼ���SavvyVal[ȡ��](SavvyVal)
	INT8		n8FortuneVal1;		// ֵ=FortuneVal1�����¸���20%��+װ��Ʒ��/FortuneVal2
	INT8		n8FortuneVal2;		// (FortuneVal1, FortuneVal2)
	INT8		n8Dummy[2];
};

struct tagFashionColorPct	// ʱװ����ʱ��ɫ����
{
	INT16	n16ColorPct[X_COLOR_NUM];	// ��ɫ����
};