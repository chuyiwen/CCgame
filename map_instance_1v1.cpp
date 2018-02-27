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
*	@date		2010/11/3	initial
*	@version	0.0.1.0
*	@brief		pvp副本
*/


#include "StdAfx.h"
#include "map_instance_1v1.h"

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
#include "pvp_mgr.h"


map_instance_1v1::map_instance_1v1(void) : map_instance(), m_pInstance(NULL),
m_dwStartTick(INVALID_VALUE), m_dwEndTick(INVALID_VALUE),
m_bNoEnter(TRUE),dw_pvp_id(INVALID_VALUE)
{
}

map_instance_1v1::~map_instance_1v1(void)
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// 初始化(不必须要创建者)
//------------------------------------------------------------------------------------------------------
BOOL map_instance_1v1::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_1v1 == pInfo->e_type );

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
	dw_pvp_id = dwMisc;
	//set_instance_id(dw_instance_id, pInfo->n_act_id);
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

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID map_instance_1v1::update()
{
	if( map_role.empty() )
	set_delete();

	Map::update();
	update_time_issue();
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID map_instance_1v1::destroy()
{
	
}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID map_instance_1v1::add_role(Role* pRole,  Map* pSrcMap)
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
VOID map_instance_1v1::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
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

	if(map_role.size() <= 1)
		set_delete();
}

//---------------------------------------------------------------------------------
// 是否能进入副本
//---------------------------------------------------------------------------------
INT	map_instance_1v1::can_enter(Role *pRole, DWORD dwParam)
{
	// 先检测通用判断
	INT nErrorCode = map_instance::can_enter(pRole);
	if( E_Success != nErrorCode ) return nErrorCode;

	// 检查人数上限
	if( m_pInstance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;

	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL map_instance_1v1::can_destroy()
{
	return map_instance::can_destroy();
}

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID map_instance_1v1::on_destroy()
{

}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID map_instance_1v1::on_delete()
{
	// 移除所有在地图内的玩家
	MapMgr* pMapMgr = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(pMapMgr) ) return;

	Role* pRole = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, pRole) )
	{
		if(VALID_POINT(pRole))
		{
			pMapMgr->RoleInstanceOut(pRole);

			tag1v1* p = NULL;
			if(pRole->get_role_pvp().b1v1)
			{
				p = g_pvp_mgr.get_1v1_info(pRole);
			}
			else
			{
				p = g_pvp_mgr.get_reservation_info(pRole);
			}
			
			if(VALID_POINT(p))
			{
				p->b_delete = TRUE;

				if(!pRole->get_role_pvp().b1v1)
				{
					pRole->GetCurMgr().IncBagSilver(p->dw_yuanbao*2, elcid_1v1);

					NET_SIS_1v1_result send;
					g_roleMgr.get_role_name(pRole->GetID(), send.sz_win_name);

					DWORD	dw_faile_id = INVALID_VALUE;
					for(INT i = 0; i < 2; i++)
					{
						if(p->dw_role_id[i] != pRole->GetID())
						{
							dw_faile_id = p->dw_role_id[i];
							g_roleMgr.get_role_name(p->dw_role_id[i], send.sz_faile_name);
						}
					}
					pRole->SendMessage(&send, send.dw_size);

					Role* pFaildRole = g_roleMgr.get_role(dw_faile_id);
					if(VALID_POINT(pFaildRole))
					{
						pFaildRole->SendMessage(&send, send.dw_size);
					}
				}
				else
				{
					INT nPumping = g_pvp_mgr.get_level_pumping_into(pRole->get_level());
					pRole->GetCurMgr().IncBagSilver(p->dw_yuanbao*2-nPumping, elcid_1v1);
					g_pvp_mgr.updata_role_score(pRole->GetID(), TRUE);

					NET_SIS_1v1_score_result send_win;
					send_win.bWin = TRUE;
					send_win.bAward = (pRole->get_1v1_score().n16_score_award > 0) ? TRUE : FALSE;
					send_win.nJoinNum = pRole->get_1v1_score().n_day_scroe_num;
					send_win.nMaxScore = pRole->get_1v1_score().n_day_max_score;
					send_win.nScore = pRole->get_1v1_score().n_cur_score;
					pRole->SendMessage(&send_win, send_win.dw_size);
					pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win, 1);
					DWORD	dw_faile_id = INVALID_VALUE;
					for(INT i = 0; i < 2; i++)
					{
						if(p->dw_role_id[i] != pRole->GetID())
						{
							dw_faile_id = p->dw_role_id[i];
						}
					}

					Role* pFaildRole = g_roleMgr.get_role(dw_faile_id);
					g_pvp_mgr.updata_role_score(dw_faile_id, FALSE);
					if(VALID_POINT(pFaildRole))
					{
						send_win.bWin = FALSE;
						send_win.bAward = (pFaildRole->get_1v1_score().n16_score_award > 0) ? TRUE : FALSE;
						send_win.nJoinNum = pFaildRole->get_1v1_score().n_day_scroe_num;
						send_win.nMaxScore = pFaildRole->get_1v1_score().n_day_max_score;
						send_win.nScore = pFaildRole->get_1v1_score().n_cur_score;
						pFaildRole->SendMessage(&send_win, send_win.dw_size);
						pFaildRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_lost, 1);
						pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win_class, pFaildRole->GetClass(), 1);
						pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_1v1_win_level, pFaildRole->get_level() - pRole->get_level(), 1);
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL map_instance_1v1::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// 时限相关的更新
//---------------------------------------------------------------------------------
VOID map_instance_1v1::update_time_issue()
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
DWORD map_instance_1v1::cal_time_limit()
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

VOID map_instance_1v1::on_leave_instance(DWORD dw_role_id)
{
	Role* pRole = map_role.find(dw_role_id);
	if(VALID_POINT(pRole))
	{
		m_mapWillOutRoleID.add(pRole->GetID(), 0);	
	}

	if(map_role.size() <= 1)
		set_delete();
}
