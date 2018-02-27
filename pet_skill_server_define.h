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
#include "../../common/WorldDefine/pet_skill_define.h"

//-----------------------------------------------------------------------------
// ���＼��Ӱ������������Ŀ
//-----------------------------------------------------------------------------
const INT MAX_PET_SKILL_MOD_ATT		= 6;


const INT MAX_PET_BEIDONG_SKILL_NUM = 3;
// ���弼������
const INT MAX_PET_NORMAL_SKILL_NUM = 2;

// �Զ�ʰȡ����ID
const INT PET_PICKUP_SKILL_ID = 1300101;
//-----------------------------------------------------------------------------
// ���＼��ԭ��(�Ѹ�Ϊ�ͻ��ˣ����������ֶ���)
//-----------------------------------------------------------------------------
struct tagPetSkillProto
{
	DWORD				dw_data_id;			// ���＼��ID ����������Ͷ�Ӧ���＼��һ��
	//DWORD				dwSkillTypeID;		// ��Ӧ��ɫ����id
	EPetskillType		eType;				// ���＼������
	EPetskillType2		eType2;				// ���＼������2
	INT					nType3;				// ���＼������3

	EPetskillCastType	eCastType;			// �ͷ�����
	INT					nSkillLevel;		// ���ܵȼ�
	BYTE				byCast_condition;	// ʩ��״̬
	INT					nCooldownTick;		// �ָ�ʱ��
	INT					nWorkTimeTick;			// ����ʱ��
	INT					nWuxing_cost;		// ����������
	INT					nSpirit_cost;		// ��������
	INT					nWuxing_add;		// ����������
	INT					nBuffid;			// ״̬
	BOOL	            bLearn_condition;	// ѧϰ����
	INT					nLearn_prob;		// ѧϰ����
	INT					nLearn_Level;		// ѧϰ�ȼ�
	//INT					nLearn_step;		// ѧϰ��
	//INT					nLearn_grade;		// ѧϰ��
	INT					nLearn_PontentialCost;	// ѧϰ��

	INT					nPetLvlLim;			// ��Ӧ����ԭ���������ȼ�����

	INT					AttIndexs[MAX_PET_SKILL_MOD_ATT];	// �ӳ���������
	INT					AttMods[MAX_PET_SKILL_MOD_ATT];		// �ӳ�ֵ

	DWORD				dwTriggerID;
	DWORD				dwBufferID;

	INT					nPetAttIndex;
	INT					nPetAttMod;

	BOOL				b_cantforget;			// �ܷ�����
};

const INT	MAX_WUXING_ITEM_NUM		= 10;

struct tagPetWuXingProto
{
	DWORD				dw_data_id;
	DWORD				dwItemTypeID[MAX_WUXING_ITEM_NUM];
	INT					nProb[MAX_WUXING_ITEM_NUM];
	INT					n_num[MAX_WUXING_ITEM_NUM];
	BOOL				bNotice[MAX_WUXING_ITEM_NUM];
};

struct tagPetSkillCmdParam
{
	tagPetSkillCmdParam(PVOID pInputBuf, PVOID pOutBuf, DWORD& dw_size)
		:m_pInputBuf(pInputBuf), m_pOutBuf(pOutBuf), m_dwOutSize(dw_size){	}

	PVOID	m_pInputBuf;
	PVOID	m_pOutBuf;
	DWORD&	m_dwOutSize;
};