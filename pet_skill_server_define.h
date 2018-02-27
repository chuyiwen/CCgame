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
// 宠物技能影响任务属性数目
//-----------------------------------------------------------------------------
const INT MAX_PET_SKILL_MOD_ATT		= 6;


const INT MAX_PET_BEIDONG_SKILL_NUM = 3;
// 合体技能数量
const INT MAX_PET_NORMAL_SKILL_NUM = 2;

// 自动拾取技能ID
const INT PET_PICKUP_SKILL_ID = 1300101;
//-----------------------------------------------------------------------------
// 宠物技能原型(已改为客户端，服务器各字定义)
//-----------------------------------------------------------------------------
struct tagPetSkillProto
{
	DWORD				dw_data_id;			// 宠物技能ID 被动技能需和对应人物技能一致
	//DWORD				dwSkillTypeID;		// 对应角色技能id
	EPetskillType		eType;				// 宠物技能类型
	EPetskillType2		eType2;				// 宠物技能类型2
	INT					nType3;				// 宠物技能类型3

	EPetskillCastType	eCastType;			// 释放类型
	INT					nSkillLevel;		// 技能等级
	BYTE				byCast_condition;	// 施放状态
	INT					nCooldownTick;		// 恢复时间
	INT					nWorkTimeTick;			// 工作时间
	INT					nWuxing_cost;		// 五行力增加
	INT					nSpirit_cost;		// 灵力消耗
	INT					nWuxing_add;		// 五行力消耗
	INT					nBuffid;			// 状态
	BOOL	            bLearn_condition;	// 学习条件
	INT					nLearn_prob;		// 学习几率
	INT					nLearn_Level;		// 学习等级
	//INT					nLearn_step;		// 学习阶
	//INT					nLearn_grade;		// 学习等
	INT					nLearn_PontentialCost;	// 学习等

	INT					nPetLvlLim;			// 对应宠物原型里的人物等级限制

	INT					AttIndexs[MAX_PET_SKILL_MOD_ATT];	// 加成属性类型
	INT					AttMods[MAX_PET_SKILL_MOD_ATT];		// 加成值

	DWORD				dwTriggerID;
	DWORD				dwBufferID;

	INT					nPetAttIndex;
	INT					nPetAttMod;

	BOOL				b_cantforget;			// 能否遗忘
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