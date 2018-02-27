/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ϸ�����¼�����

#include "StdAfx.h"
#include "world_event_mgr.h"
#include "script_mgr.h"
#include "activity_mgr.h"
#include "role_mgr.h"
#include "world.h"
#include "role.h"

BOOL CWorldEventMgr::Init()
{
	// ��ʼ���ű�
	m_pScript = g_ScriptMgr.GetWorldScript();
	if(!VALID_POINT(m_pScript))
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------------
// ����Update
//-----------------------------------------------------------------------------
VOID CWorldEventMgr::OnClock(BYTE byHour)
{
	OnAdventure();
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
VOID CWorldEventMgr::OnAdventure()
{
	// �Ƿ��й̶���ڽ�����
	if(TRUE == activity_mgr::GetInstance()->any_activity_start())
		return;

	// ���ѡȡ3��������һ������
	Role	*pRole = (Role*)INVALID_VALUE;
	std::vector<Role*>	vecRole;
	INT		n_num	= 0;
	FLOAT	fProp	= 0.0f;
	BOOL	bResult	= FALSE;

	// ��һ�γ�ȡ500�Σ������ɫ��Ե����Ӱ��
	for(INT i = 0; i < 500; ++i)
	{
		// �Ѿ��ҵ��������
		if(n_num == 3)
			break;

		pRole = g_roleMgr.get_rand_role();
		if(!VALID_POINT(pRole))
			continue;

		// С��15�����²����ȡ
		if(pRole->get_level() < 15)
			continue;

		// ��������Ҳ����ȡ
		

		// �����������ĸ���
		fProp = 1.0f / 20.0f * (1.0f + (FLOAT)pRole->GetAttValue(ERA_Fortune) / 10.0f);
		bResult = get_tool()->tool_rand() % 100 <= (INT)fProp;
		if(bResult)
		{
			vecRole.push_back(pRole);
			++n_num;
		}
	}

	// ��ȡ�����С��3��������еڶ��ֳ�ȡ
	if(n_num < 3)
	{
		for(INT n = 0; n < 500; ++n)
		{
			// �Ѿ��ҵ��������
			if(n_num == 3)
				break;

			pRole = g_roleMgr.get_rand_role();
			if(!VALID_POINT(pRole))
				continue;

			// С��15�����²����ȡ
			if(pRole->get_level() < 15)
				continue;

			// ��������Ҳ����ȡ


			vecRole.push_back(pRole);
			++n_num;
		}
	}

	// �Գ�ȡ����Ҳ�������
	std::vector<Role*>::iterator it = vecRole.begin();
	while (it != vecRole.end())
	{
		if (VALID_POINT(m_pScript))
		{
			m_pScript->OnAdventure(*it);
		}
		++it;
	}
}