/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ܴ�����

#pragma once

#include "att_res.h"

//------------------------------------------------------------------------------
// ���ܶԴ�����Ӱ��
//------------------------------------------------------------------------------
struct tagTriggerMod
{
	INT		nEventMisc1Mod;				// �¼�����ֵ1�ӳ�
	INT		nEventMisc2Mod;				// �¼�����ֵ2�ӳ�
	INT		nStateMisc1Mod;				// ״̬����ֵ1�ӳ�
	INT		nStateMisc2Mod;				// ״̬����ֵ2�ӳ�
	INT		nPropMod;					// �������ʼӳ�

	tagTriggerMod()
	{
		nEventMisc1Mod = 0;
		nEventMisc2Mod = 0;
		nStateMisc1Mod = 0;
		nStateMisc2Mod = 0;
		nPropMod = 0;
	}

	VOID SetMod(const tagSkillProto* pProto)
	{
		// �¼�����ֵ1�ӳ�
		nEventMisc1Mod += pProto->nTriggerEventMisc1Add;
		// �¼�����ֵ2�ӳ�
		nEventMisc2Mod += pProto->nTriggerEventMisc2Add;
		// ״̬����ֵ1�ӳ�
		nStateMisc1Mod += pProto->nTriggerStateMisc1Add;
		// ״̬����ֵ2�ӳ�
		nStateMisc2Mod += pProto->nTriggerStateMisc2Add;
		// �������ʼӳ�
		nPropMod += pProto->nTriggerPropAdd;
	}

	VOID UnSetMod(const tagSkillProto* pProto)
	{
		// �¼�����ֵ1�ӳ�
		nEventMisc1Mod -= pProto->nTriggerEventMisc1Add;
		// �¼�����ֵ2�ӳ�
		nEventMisc2Mod -= pProto->nTriggerEventMisc2Add;
		// ״̬����ֵ1�ӳ�
		nStateMisc1Mod -= pProto->nTriggerStateMisc1Add;
		// ״̬����ֵ2�ӳ�
		nStateMisc2Mod -= pProto->nTriggerStateMisc2Add;
		// �������ʼӳ�
		nPropMod -= pProto->nTriggerPropAdd;
	}
};