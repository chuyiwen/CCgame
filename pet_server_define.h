/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ﶨ��

#pragma once

struct tagPetProto
{
	DWORD	dw_data_id;		// ����TypeID
	DWORD	dwFoodLimit;	// ʳ������
	INT		nType3;			// ��������
	INT		nRoleLvlLim;	// ����Я���ȼ�
	INT		nMountable;		// �ܷ����
	INT		nMountSpeed;	// ����ٶ�
	DWORD	dwMountSkillID[MOUNT_SKILL_NUM];

	Vector3	vSize;			// ��ײ�ߴ�
	FLOAT	fScale;			// ��˳ߴ�����

	BOOL	bBind;			// �Ƿ��
	
	DWORD	dwSkillListID;	// ������ܱ�ID

	//INT		nAptitudeMin[EIQ_End];
	//INT		nAptitudeMax[EIQ_End];

};

