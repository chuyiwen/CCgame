/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//游戏世界事件触发

#include "StdAfx.h"
#include "world_event_mgr.h"
#include "script_mgr.h"
#include "activity_mgr.h"
#include "role_mgr.h"
#include "world.h"
#include "role.h"

BOOL CWorldEventMgr::Init()
{
	// 初始化脚本
	m_pScript = g_ScriptMgr.GetWorldScript();
	if(!VALID_POINT(m_pScript))
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------------
// 整点Update
//-----------------------------------------------------------------------------
VOID CWorldEventMgr::OnClock(BYTE byHour)
{
	OnAdventure();
}

//-----------------------------------------------------------------------------
// 产生奇遇
//-----------------------------------------------------------------------------
VOID CWorldEventMgr::OnAdventure()
{
	// 是否有固定活动在进行中
	if(TRUE == activity_mgr::GetInstance()->any_activity_start())
		return;

	// 随机选取3名在线玩家获得奇遇
	Role	*pRole = (Role*)INVALID_VALUE;
	std::vector<Role*>	vecRole;
	INT		n_num	= 0;
	FLOAT	fProp	= 0.0f;
	BOOL	bResult	= FALSE;

	// 第一次抽取500次，计算角色福缘属性影响
	for(INT i = 0; i < 500; ++i)
	{
		// 已经找到三个玩家
		if(n_num == 3)
			break;

		pRole = g_roleMgr.get_rand_role();
		if(!VALID_POINT(pRole))
			continue;

		// 小于15级以下不予抽取
		if(pRole->get_level() < 15)
			continue;

		// 副本内玩家不予抽取
		

		// 计算获得奇遇的概率
		fProp = 1.0f / 20.0f * (1.0f + (FLOAT)pRole->GetAttValue(ERA_Fortune) / 10.0f);
		bResult = get_tool()->tool_rand() % 100 <= (INT)fProp;
		if(bResult)
		{
			vecRole.push_back(pRole);
			++n_num;
		}
	}

	// 抽取的玩家小于3个，则进行第二轮抽取
	if(n_num < 3)
	{
		for(INT n = 0; n < 500; ++n)
		{
			// 已经找到三个玩家
			if(n_num == 3)
				break;

			pRole = g_roleMgr.get_rand_role();
			if(!VALID_POINT(pRole))
				continue;

			// 小于15级以下不予抽取
			if(pRole->get_level() < 15)
				continue;

			// 副本内玩家不予抽取


			vecRole.push_back(pRole);
			++n_num;
		}
	}

	// 对抽取的玩家产生奇遇
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