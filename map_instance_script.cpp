/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//脚本副本

#include "StdAfx.h"
#include "map_instance_script.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"

MapInstanceScript::MapInstanceScript() : map_instance()
{
}

MapInstanceScript::~MapInstanceScript()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// 初始化（创建者不需要）
//------------------------------------------------------------------------------------------------------
BOOL MapInstanceScript::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_ScriptCreate == pInfo->e_type );

	// 地图相关属性
	p_map_info = pInfo;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_ground_item.clear();

	// 副本相关属性
	dw_id = p_map_info->dw_id;
	dw_instance_id = dwInstanceID;

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
VOID MapInstanceScript::update()
{
	Map::update();
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID MapInstanceScript::destroy()
{

}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID MapInstanceScript::add_role(Role* pRole, Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);
}

//---------------------------------------------------------------------------------
// 玩家离开地图，只可能在主线程里面调用
//---------------------------------------------------------------------------------
VOID MapInstanceScript::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);
}
//---------------------------------------------------------------------------------
// 是否能进入副本
//---------------------------------------------------------------------------------
INT MapInstanceScript::can_enter(Role *pRole)
{
	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL MapInstanceScript::can_destroy()
{
	return map_instance::can_destroy();
}


//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL MapInstanceScript::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID MapInstanceScript::on_delete()
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

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID MapInstanceScript::on_destroy()
{
}