/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物定义

#pragma once

struct tagPetProto
{
	DWORD	dw_data_id;		// 宠物TypeID
	DWORD	dwFoodLimit;	// 食物限制
	INT		nType3;			// 宠物类型
	INT		nRoleLvlLim;	// 人物携带等级
	INT		nMountable;		// 能否骑乘
	INT		nMountSpeed;	// 骑乘速度
	DWORD	dwMountSkillID[MOUNT_SKILL_NUM];

	Vector3	vSize;			// 碰撞尺寸
	FLOAT	fScale;			// 骑乘尺寸缩放

	BOOL	bBind;			// 是否绑定
	
	DWORD	dwSkillListID;	// 随机技能表ID

	//INT		nAptitudeMin[EIQ_End];
	//INT		nAptitudeMax[EIQ_End];

};

