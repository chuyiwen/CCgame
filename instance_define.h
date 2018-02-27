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

#define		RAND_CTEATUTE_NUM	3					// ���������������صĹ�������
#define		RAND_INSTANCE_NUM	3					// ��������Ѷ����͵�����

struct tagRandSpawnPointInfo						// �������ˢ�ֵ�
{
	DWORD		dwSpawnPointID;						// ˢ�ֵ�ID(��ID��
	INT			nLevel;
	DWORD		dwNormalID[RAND_CTEATUTE_NUM];		// ��ͨ�Ѷȸ�������ID		
	DWORD		dwEliteID[RAND_CTEATUTE_NUM];		// ��Ӣ�Ѷȸ�������ID
	DWORD		dwDevilID[RAND_CTEATUTE_NUM];		// ħ���Ѷȸ�������ID
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