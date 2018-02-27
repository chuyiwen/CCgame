/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "../../common/WorldDefine/RoleDefine.h"

#pragma once

#pragma pack(push, 1)

#define		RAND_CTEATUTE_NUM	3					// 随机副本导航点挂载的怪物数量
#define		RAND_INSTANCE_NUM	3					// 随机副本难度类型的数量

struct tagRandSpawnPointInfo						// 随机副本刷怪点
{
	DWORD		dwSpawnPointID;						// 刷怪点ID(大ID）
	INT			nLevel;
	DWORD		dwNormalID[RAND_CTEATUTE_NUM];		// 普通难度副本怪物ID		
	DWORD		dwEliteID[RAND_CTEATUTE_NUM];		// 精英难度副本怪物ID
	DWORD		dwDevilID[RAND_CTEATUTE_NUM];		// 魔王难度副本怪物ID
};

struct tagInstanceItem
{
	DWORD					dwMapID;
	package_map<DWORD, DWORD>		mapInstanceItem;
};

struct tagInstanceSkill
{
	DWORD					dwMapID;
	package_map<DWORD, DWORD>		mapInstanceSkill;
};

struct tagLevelMapping
{
	INT						nLevel;
	INT						nTransmitLevel;
};

#pragma pack(pop)