/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
*	@file		map_instance_pvp.h
*	@author		mmz
*	@date		2011/10/20	initial
*	@version	0.0.1.0
*	@brief		pvp副本
*/

#include "StdAfx.h"
#include "map_instance_pvp.h"

#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"

#include "map_instance.h"
#include "map_creator.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "guild_manager.h"


map_instance_pvp::map_instance_pvp(void) : map_instance(), m_pInstance(NULL),
m_dwStartTick(INVALID_VALUE), m_dwEndTick(INVALID_VALUE),
m_bNoEnter(TRUE)
{
}

map_instance_pvp::~map_instance_pvp(void)
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// 初始化(不必须要创建者)
//------------------------------------------------------------------------------------------------------
BOOL map_instance_pvp::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_PVP == pInfo->e_type );

	// 读取副本静态属性
	m_pInstance = AttRes::GetInstance()->get_instance_proto(pInfo->dw_id);
	if( !VALID_POINT(m_pInstance) )	return FALSE;

	// 地图相关属性
	p_map_info = pInfo;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();

	// 副本相关属性
	dw_id = p_map_info->dw_id;
	dw_instance_id = dwInstanceID;
	m_nActID = pInfo->n_act_id;
	set_instance_id(dw_instance_id, pInfo->n_act_id);
	//m_eInstanceHardMode = (EInstanceHardMode)dwMisc;
	m_dwStartTick = g_world.GetWorldTick();
	m_dwEndTick = INVALID_VALUE;

	p_npc_team_mgr = new NPCTeamMgr(this);
	if(!VALID_POINT(p_npc_team_mgr))
		return FALSE;

	// 初始化寻路系统
	//if (!initSparseGraph(pInfo))
	//	return FALSE;

	// 根据mapinfo来初始化地图的怪物，可视列表等信息
	if( FALSE == init_logical_info() )
	{
		return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------------------
// 设置副本ID
//------------------------------------------------------------------------------------------------------
VOID map_instance_pvp::set_instance_id(DWORD dwInstanceID, INT nActID)
{
	//g_guild_manager.set_pvp_instance_id(dwInstanceID, nActID);
}

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID map_instance_pvp::update()
{
	/*if( map_role.empty() )
		set_delete();*/

	Map::update();
	update_time_issue();
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID map_instance_pvp::destroy()
{
	/*s_guild_pvp_data* pGuildPvPData = g_guild_manager.get_pvp_data(m_nActID);
	if(VALID_POINT(pGuildPvPData))
	{
		pGuildPvPData->SetInstanceID(INVALID_VALUE);
	}*/
}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID map_instance_pvp::add_role(Role* pRole,  Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);

	// 重置关闭等待
	if( is_end() )
	{
		m_dwEndTick = INVALID_VALUE;
		b_end = FALSE;
	}

	// 发送进入副本消息
	NET_SIS_enter_instance send;
	send.dw_error_code = E_Success;
	send.dwTimeLimit = cal_time_limit();
	pRole->SendMessage(&send, send.dw_size);
}

//---------------------------------------------------------------------------------
// 玩家离开地图，只可能在主线程里面调用
//---------------------------------------------------------------------------------
VOID map_instance_pvp::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);

	// 是否进入等待关闭
	/*if( map_role.empty() && !is_end() && m_pInstance->dwEndTime != INVALID_VALUE )
	{
		m_dwEndTick = g_world.GetWorldTick();
		b_end = TRUE;
	}*/

	// 如果这个玩家在等待离开的列表里，则移除
	m_mapWillOutRoleID.erase(dwID);
}

//---------------------------------------------------------------------------------
// 是否能进入副本
//---------------------------------------------------------------------------------
INT	map_instance_pvp::can_enter(Role *pRole, DWORD dwParam)
{
	// 先检测通用判断
	INT nErrorCode = map_instance::can_enter(pRole);
	if( E_Success != nErrorCode ) return nErrorCode;

	// 检查人数上限
	/*if( m_pInstance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;*/

	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL map_instance_pvp::can_destroy()
{
	return map_instance::can_destroy();
}

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID map_instance_pvp::on_destroy()
{

}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID map_instance_pvp::on_delete()
{
	// 移除所有在地图内的玩家
	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(pMapMgr) ) return;

	Role* pRole = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, pRole) )
	{
		pMapMgr->RoleInstanceOut(pRole);
	}
}

//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL map_instance_pvp::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// 时限相关的更新
//---------------------------------------------------------------------------------
VOID map_instance_pvp::update_time_issue()
{
	// 如果已经处于待删除状态，就不更新了
	if( is_delete() ) return;

	// 时限副本
	if( is_time_limit() && !is_end() )
	{
		DWORD dwTick = g_world.GetWorldTick();
		if( (dwTick - m_dwStartTick) >= m_pInstance->dwTimeLimit * TICK_PER_SECOND )
		{
			m_dwEndTick = g_world.GetWorldTick();
			set_end();
		}
	}

	// 关闭倒计时
	if( is_end() )
	{
		DWORD dwTick = g_world.GetWorldTick();
		if( (dwTick - m_dwEndTick) > m_pInstance->dwEndTime * TICK_PER_SECOND )
		{
			set_delete();
		}
	}

	// 更新所有待退出的角色的时间
	if( !m_mapWillOutRoleID.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_mapWillOutRoleID.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT nTick = INVALID_VALUE;

		while( m_mapWillOutRoleID.find_next(it, dw_role_id, nTick) )
		{
			--nTick;	// 减一下倒计时
			if( nTick <= 0 )
			{
				// 时间到了，将玩家传送出去
				Role* pRole = find_role(dw_role_id);
				if( VALID_POINT(pRole) )
				{
					MapMgr* pMapMgr = g_mapCreator.get_map_manager(p_map_info->dw_id);
					if( VALID_POINT(pMapMgr) )
					{
						pMapMgr->RoleInstanceOut(pRole);
					}
				}

				m_mapWillOutRoleID.erase(dw_role_id);
			}
			else
			{
				m_mapWillOutRoleID.change_value(dw_role_id, nTick);
			}
		}
	}
}

//-----------------------------------------------------------------------------------
// 计算时限副本所剩时间
//-----------------------------------------------------------------------------------
DWORD map_instance_pvp::cal_time_limit()
{
	DWORD dwTimeLeft = INVALID_VALUE;
	if(m_pInstance->dwTimeLimit > 0 && m_pInstance->dwTimeLimit != INVALID_VALUE)
	{
		DWORD dwCurrentTick = g_world.GetWorldTick();
		DWORD dwTimePass = (dwCurrentTick - m_dwStartTick) / TICK_PER_SECOND;
		dwTimeLeft = m_pInstance->dwTimeLimit - dwTimePass;
	}

	return dwTimeLeft;
}

VOID map_instance_pvp::on_leave_instance(DWORD dw_role_id)
{
	Role* pRole = map_role.find(dw_role_id);
	if(VALID_POINT(pRole))
	{
		m_mapWillOutRoleID.add(pRole->GetID(), 0);	
	}
}

//-----------------------------------------------------------------------------------
// 活动结束时
//-----------------------------------------------------------------------------------
//VOID map_instance_pvp::on_act_end()
//{
//	package_map<DWORD, Role*>::map_iter it = map_role.begin();
//	Role* pRole = NULL;
//	while (map_role.find_next(it, pRole))
//	{
//		if( VALID_POINT(pRole) )
//		{
//			m_mapWillOutRoleID.add(pRole->GetID(), 5);		
//		}
//	}			
//}