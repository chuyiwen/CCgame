/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

//地图类

#include "StdAfx.h"

#include "../../common/WorldDefine/action_protocol.h"
#include "../../common/WorldDefine/remove_role_protocol.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/drop_protocol.h"
#include "../../common/WorldDefine/script_data.h"

#include "sspawnpoint_define.h"
#include "NPCTeam_define.h"

#include "player_session.h"
#include "world_session.h"
#include "login_session.h"
#include "map_creator.h"
#include "unit.h"
#include "role.h"
#include "creature.h"
#include "pet.h"
#include "role_mgr.h"
#include "map.h"
#include "att_res.h"
#include "shop.h"
#include "team.h"
#include "group_mgr.h"
#include "move_data.h"
#include "script_mgr.h"
#include "gm_policy.h"
#include "NPCTeam_mgr.h"
#include "NPCTeam.h"
#include "guild_chamber.h"
#include "guild.h"
#include "SparseGraph.h"


//------------------------------------------------------------------------------
// 掉落物品的大小
//------------------------------------------------------------------------------
static Vector3 vGroundItemSize(50.0f, 50.0f, 50.0f);

#define UPDATE_NUM_TIME 60000
//------------------------------------------------------------------------------
// construct
//------------------------------------------------------------------------------
Map::Map() : dw_id(INVALID_VALUE), dw_instance_id(INVALID_VALUE), p_map_info(NULL),
p_visible_tile(NULL), ScriptData<ESD_Map>()
{
	dw_update_num_time = UPDATE_NUM_TIME;
	dw_update_time = 0;
	n_chang_weahter_tick_ = 0;
	dw_weather_id_ = INVALID_VALUE;
	p_npc_team_mgr = NULL;
}

//------------------------------------------------------------------------------
// destruct
//------------------------------------------------------------------------------
Map::~Map()
{
	destroy();
}

//-------------------------------------------------------------------------------
// 初始化地图
//-------------------------------------------------------------------------------
BOOL Map::init(const tag_map_info* pInfo)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_Normal == pInfo->e_type );



	p_map_info = pInfo;
	dw_id = p_map_info->dw_id;

	map_session.clear();
	map_role.clear();
	map_leave_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();

	list_scene_effect.clear();
	list_NPCLoading.clear();

	dw_weather_id_ = INVALID_VALUE;
	n_chang_weahter_tick_ = 0;


	//NavResMgr::Inst()->SetFS(NavResMgr::EFST_Map, pVfs);


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

VOID Map::init_creature_script()
{
	CREATURE_MAP::map_iter iter = map_creature.begin();
	Creature* pCreature = NULL;
	while(map_creature.find_next(iter, pCreature))
	{
		if(!VALID_POINT(pCreature))
			continue;

		pCreature->OnScriptLoad();
	}
}

//---------------------------------------------------------------------------------
// 当初始化完成时
//---------------------------------------------------------------------------------
VOID Map::on_init()
{
	if( VALID_POINT(p_script) )
		p_script->OnInit(this);
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID Map::destroy()
{
	// 删除地图里的怪物
	Creature* pCreature = NULL;

	package_map<DWORD, Creature*>::map_iter itCreature = map_creature.begin();
	while( map_creature.find_next(itCreature, pCreature) )
	{
		Creature::Delete(pCreature);
	}
	map_creature.clear();

	package_map<DWORD, Creature*>::map_iter itResCreature = map_respawn_creature.begin();
	while( map_respawn_creature.find_next(itResCreature, pCreature) )
	{
		Creature::Delete(pCreature);
	}
	map_respawn_creature.clear();

	// 门对象列表清空 不需要delete
	map_door.clear();

	// 删除地面物品集合
	map_ground_item.reset_iterator();
	tag_ground_item* pGroundItem = NULL;
	while( map_ground_item.find_next(pGroundItem) )
	{
		pGroundItem->destroy_item();
		SAFE_DELETE(pGroundItem);
	}

	// 删除商店
	Shop* pShop = NULL;
	package_map<DWORD, Shop*>::map_iter itShop = map_shop.begin();
	while( map_shop.find_next(itShop, pShop) )
	{
		Shop::Delete(pShop);
	}
	map_shop.clear();

	// 删除商会
	guild_chamber* pGuildCofC = NULL;
	CHAMBER_MAP::map_iter itCofC = map_chamber.begin();
	while (map_chamber.find_next(itCofC, pGuildCofC))
	{
		guild_chamber::delete_chamber(pGuildCofC);
	}
	map_chamber.clear();

	tagNPCLoading* pNPCLoading = NULL;
	package_list<tagNPCLoading*>::list_iter  iterNPC = list_NPCLoading.begin();
	while(list_NPCLoading.find_next(iterNPC, pNPCLoading))
	{
		if(VALID_POINT(pNPCLoading))
			SAFE_DELETE(pNPCLoading);
	}

	// 关闭九宫格和导航图
	SAFE_DELETE_ARRAY(p_visible_tile);

	// 删除怪物小队管理器
	SAFE_DELETE(p_npc_team_mgr);
	
	//SAFE_DELETE(m_pSparseGraph);
	// 清空脚本
	p_script = NULL;

	
}

//---------------------------------------------------------------------------------------
// 某两个点之间是否可以通过
//---------------------------------------------------------------------------------------
BOOL Map::if_can_direct_go(FLOAT f_srcx_, FLOAT f_src_y_, FLOAT f_dest_x_, FLOAT f_dest_z_, pathNode& n_near_pos_)
{
	// 得到两个目标之间的向量
	Vector3 vVec = Vector3(f_dest_x_-f_srcx_, 0, f_dest_z_-f_src_y_ );
	INT nTileDist = sqrt(vVec.x*vVec.x + vVec.y*vVec.y) / TILE_SCALE;
	// 对该向量进行归一化
	Vec3Normalize(vVec);
	
	if( 0 > nTileDist ) return false;

	
	Vector3 vDest = Vector3(f_srcx_, 0, f_src_y_);

	// 得到一个最大合理的终点
	n_near_pos_.x() = vDest.x;
	n_near_pos_.y() = vDest.y;
	for (int i = 1; i <= nTileDist; i++)
	{
		FLOAT fDistAbs = FLOAT(i * TILE_SCALE);	// 绝对距离
		vDest = vDest + vVec * fDistAbs;
		if (if_can_go(vDest.x, vDest.z))
		{
			n_near_pos_.x() = vDest.x;
			n_near_pos_.y() = vDest.y;
		}
		else
		{
			return false;
		}
	}

	return true;
	
	//return get_path_finder()->can_walk_between(pathNode(f_srcx_, f_src_y_), pathNode(f_dest_x_, f_dest_z_), n_near_pos_);
}

//---------------------------------------------------------------------------------
// 生成地图的逻辑属性，如可视链表，导航点，触发器，NPC怪物等等
//---------------------------------------------------------------------------------
BOOL Map::init_logical_info(DWORD dw_guild_id_)
{
	// 生成VISTILE
	n_visible_tile_array_width = (p_map_info->n_width + p_map_info->n_visit_distance - 1) / p_map_info->n_visit_distance + 2;
	n_visible_tile_array_height = (p_map_info->n_height + p_map_info->n_visit_distance - 1) / p_map_info->n_visit_distance + 2;

	p_visible_tile = new tagVisTile[n_visible_tile_array_width * n_visible_tile_array_height];
	if( !VALID_POINT(p_visible_tile) )
	{
		return FALSE;
	}

	// 生成地图脚本
	p_script = g_ScriptMgr.GetMapScript(dw_id);

	// 加载地图怪物
	if( FALSE == init_all_map_creature(dw_guild_id_) )
	{
		SAFE_DELETE_ARRAY(p_visible_tile);
		return FALSE;
	}

	this->on_init( );

	return TRUE;
}

//-----------------------------------------------------------------------------------
// 初始化所以地图中的怪物，加载时为单线程
//-----------------------------------------------------------------------------------
BOOL Map::init_all_map_creature(DWORD dw_guild_id_)
{
	// 初始化怪物ID生成器
	builder_creature_id.init(p_map_info);
	
	return init_all_fixed_creature_ex(dw_guild_id_);
	/*return init_all_fixed_creature(dw_guild_id_) && 
		init_all_spawn_point_creature(dw_guild_id_) &&
		init_all_spawn_list_creature(dw_guild_id_);*/
}

//-----------------------------------------------------------------------------------
// 初始化所有的地图内摆放的怪物
//-----------------------------------------------------------------------------------
BOOL Map::init_all_fixed_creature(DWORD dw_guild_id_)
{
	// 一个一个的创建怪物
	tagMapCreatureInfo* pCreatureInfo = NULL;
	const tagCreatureProto* pProto = NULL;
	p_map_info->map_creature_info.reset_iterator();
	while( p_map_info->map_creature_info.find_next(pCreatureInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pCreatureInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pCreatureInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// 生成出生坐标和出生朝向
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_creature_id, dw_id, this, pProto, pCreatureInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, pCreatureInfo->b_collide, pCreatureInfo->list_patrol, INVALID_VALUE);

		if(pProto->bLoading)
		{
			tagNPCLoading* st_NPCLoading = new tagNPCLoading;
			st_NPCLoading->dw_npc_id = dw_creature_id;
			st_NPCLoading->dw_Obj_id = pCreatureInfo->dw_att_id;
			list_NPCLoading.push_back(st_NPCLoading);
		}

		// 加入到地图中
		add_creature_on_load(pCreature);
		
		// 如果是巢穴，则加载巢穴怪物
		if( pCreature->IsNest() )
		{
			init_nest_creature(pCreature);
		}

		// 如果是怪物小队，则加载小队怪物
		if( pCreature->IsTeam() )
		{
			init_team_creature(pCreature);
		}
	}

	DWORD dwCreatureSize = (map_creature.size() * sizeof(Creature));
	g_world.get_log()->write_log(_T("Read %d BYTE creature memory from map<%s>!\n"), dwCreatureSize, p_map_info->sz_map_name);

	// 一个一个的创建门对象
	tag_map_door_info* pDoorInfo = NULL;
	//const tagCreatureProto* pProto = NULL;
	p_map_info->map_door_info.reset_iterator();
	while( p_map_info->map_door_info.find_next(pDoorInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pDoorInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		if ( !( pProto->eType == ECT_GameObject && pProto->nType2 == EGOT_Door ) )
		{
			print_message(_T("Detect a unknown door in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_door_id = OBJID_TO_DOORID(pDoorInfo->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id) );

		// 生成出生坐标和出生朝向
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_door_id, dw_id, this, pProto, pDoorInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);

		// 加入到地图中
		add_creature_on_load(pCreature);

		// 加入到门对象列表
		map_door.add(dw_door_id, pCreature );
	}

	return TRUE;
} 

//-----------------------------------------------------------------------------------
// 初始化所有的地图内摆放的怪物
// gx add 2013.4.16
// 暂时没有被调用，留作后续处理
//-----------------------------------------------------------------------------------
BOOL Map::init_all_fixed_creature_ex(DWORD dw_guild_id_)
{
	// 一个一个的创建怪物
	tagMapMonsterInfo* pCreatureInfo = NULL;
	const tagCreatureProto* pProto = NULL;
	p_map_info->map_monster_info.reset_iterator();
	while( p_map_info->map_monster_info.find_next(pCreatureInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pCreatureInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pCreatureInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// 生成出生坐标和出生朝向
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_creature_id, dw_id, this, pProto, pCreatureInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL, INVALID_VALUE);

		if(pProto->bLoading)
		{
			tagNPCLoading* st_NPCLoading = new tagNPCLoading;
			st_NPCLoading->dw_npc_id = dw_creature_id;
			//st_NPCLoading->dw_Obj_id = pCreatureInfo->dw_att_id;
			list_NPCLoading.push_back(st_NPCLoading);
		}

		// 加入到地图中
		add_creature_on_load(pCreature);

		// 如果是巢穴，则加载巢穴怪物
		if( pCreature->IsNest() )
		{
			init_nest_creature(pCreature);
		}

		// 如果是怪物小队，则加载小队怪物
		if( pCreature->IsTeam() )
		{
			init_team_creature(pCreature);
		}
	}

	DWORD dwCreatureSize = (map_creature.size() * sizeof(Creature));
	g_world.get_log()->write_log(_T("Read %d BYTE creature memory from map<%s>!\n"), dwCreatureSize, p_map_info->sz_map_name);

	// 一个一个的创建门对象
	tag_map_door_info* pDoorInfo = NULL;
	//const tagCreatureProto* pProto = NULL;
	p_map_info->map_door_info.reset_iterator();
	while( p_map_info->map_door_info.find_next(pDoorInfo) )
	{
		pProto = AttRes::GetInstance()->GetCreatureProto(pDoorInfo->dw_type_id);
		if( !VALID_POINT(pProto) )
		{
			print_message(_T("Detect a unknown creature in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		if ( !( pProto->eType == ECT_GameObject && pProto->nType2 == EGOT_Door ) )
		{
			print_message(_T("Detect a unknown door in map %s, dwObjID=%u\r\n"), p_map_info->sz_map_name, pDoorInfo->dw_type_id);
			continue;
		}

		// 取出一个ID
		DWORD dw_door_id = OBJID_TO_DOORID(pDoorInfo->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id) );

		// 生成出生坐标和出生朝向
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// 生成怪物
		Creature* pCreature = Creature::Create(dw_door_id, dw_id, this, pProto, pDoorInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);

		// 加入到地图中
		add_creature_on_load(pCreature);

		// 加入到门对象列表
		map_door.add(dw_door_id, pCreature );
	}

	return TRUE;
} 

//
//-----------------------------------------------------------------------------------
// 初始化普通地图刷怪点的怪物，加载时为单线程
//-----------------------------------------------------------------------------------
BOOL Map::init_all_spawn_point_creature(DWORD dw_guild_id_)
{
	// 对每一个刷怪点	
	tag_map_spawn_point_info* pMapSpawnInfo = NULL;
	p_map_info->map_spawn_point.reset_iterator();
	while(p_map_info->map_spawn_point.find_next(pMapSpawnInfo))
	{
		ASSERT_P_VALID(pMapSpawnInfo);
		if (!VALID_POINT(pMapSpawnInfo)) continue;

		// 随机选择一个怪
		const tagSSpawnPointProto* pSSpawnProto = AttRes::GetInstance()->GetSSpawnPointProto(pMapSpawnInfo->dw_spawn_point_id);
		if (!VALID_POINT(pSSpawnProto)) continue;

		INT nCandiNum	= 0;
		while (VALID_VALUE(pSSpawnProto->dwTypeIDs[nCandiNum]))
			nCandiNum++;
		if (0 == nCandiNum) continue;

		INT nIndex = get_tool()->tool_rand() % nCandiNum;
		const tagCreatureProto* pCreatureProto = AttRes::GetInstance()->GetCreatureProto(pSSpawnProto->dwTypeIDs[nIndex]);
		ASSERT_P_VALID(pCreatureProto);
		if (!VALID_POINT(pCreatureProto)) continue;

		// 获取id
		DWORD dwCreatureID = builder_creature_id.get_new_creature_id();

		// 生成出生坐标和出生朝向
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		Vector3 vFaceTo;
		vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// 创建怪
		Creature* pCreature = Creature::Create(	dwCreatureID, get_map_id(), this, pCreatureProto, pMapSpawnInfo->v_pos, vFaceTo, 
			ECST_SpawnPoint, pMapSpawnInfo->dw_spawn_point_id, INVALID_VALUE, pMapSpawnInfo->b_collide, NULL, INVALID_VALUE, dw_guild_id_);
		ASSERT_P_VALID(pCreature);
		if (!VALID_POINT(pCreature)) continue;

		// 放入地图
		add_creature_on_load(pCreature);
	}

	return TRUE;
}

// 生成所有刷怪点组怪物
BOOL Map::init_all_spawn_list_creature(DWORD dw_guild_id_)
{
	// 对每一组刷怪点	
	tag_map_spawn_point_list* pMapSpawnList = NULL;
	p_map_info->map_spawn_point_list.reset_iterator();
	while(p_map_info->map_spawn_point_list.find_next(pMapSpawnList))
	{
		ASSERT_P_VALID(pMapSpawnList);
		if (!VALID_POINT(pMapSpawnList)) continue;

		// 对于所有的怪
		const tagSSpawnPointProto* pSSpawnProto = AttRes::GetInstance()->GetSSpawnGroupProto(pMapSpawnList->dw_id);
		ASSERT_P_VALID(pSSpawnProto);
		if (!VALID_POINT(pSSpawnProto)) continue;
			
		for (int i = 0; i < MAX_CREATURE_PER_SSPAWNPOINT; i++)
		{
			const tagCreatureProto* pCreatureProto = AttRes::GetInstance()->GetCreatureProto(pSSpawnProto->dwTypeIDs[i]);
			if (!VALID_POINT(pCreatureProto)) continue;

			//DWORD dwSpawnID = 0;
			tag_map_spawn_point_info* pMapSpawnInfo = NULL;	
			RandSpwanPoint(pMapSpawnList, pMapSpawnInfo);

			//pMapSpawnList->list.rand_find(dwSpawnID, pMapSpawnInfo);
			if (!VALID_POINT(pMapSpawnInfo)) continue;

			// 获取id
			DWORD dwCreatureID = builder_creature_id.get_new_creature_id();

			// 生成出生坐标和出生朝向
			FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
			Vector3 vFaceTo;
			vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.y = 0.0f;

			// 创建怪
			Creature* pCreature = Creature::Create(	dwCreatureID, get_map_id(), this, pCreatureProto, pMapSpawnInfo->v_pos, vFaceTo, 
				ECST_SpawnList, pMapSpawnInfo->dw_att_id, pMapSpawnInfo->dw_group_id, pMapSpawnInfo->b_collide, NULL, INVALID_VALUE, dw_guild_id_);
			ASSERT_P_VALID(pCreature);
			//pMapSpawnInfo->b_hasUnit = TRUE;
			map_point_has_list[pMapSpawnInfo->dw_att_id] = TRUE;
			if (!VALID_POINT(pCreature)) continue;

			// 放入地图
			add_creature_on_load(pCreature);
		}

	}

	return TRUE;
}
//-----------------------------------------------------------------------------------
// 初始化巢穴怪物
//-----------------------------------------------------------------------------------
VOID Map::init_nest_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) && p_creature_->IsNest() );

	const tagCreatureProto* pProto = p_creature_->GetProto();
	ASSERT( VALID_POINT(pProto) && VALID_POINT(pProto->pNest) );

	const tagNestProto* pNest = pProto->pNest;
	const Vector3& vSpawnCenter = p_creature_->GetCurPos();

	Vector3 vPos;
	Vector3 vFaceTo;

	for(INT n = 0; n < pNest->nCreatureNum; n++)
	{
		DWORD dwCreatureTypeID = pNest->dwSpawnID[n];
		INT n_num = pNest->nSpawnMax[n];

		const tagCreatureProto* pSpawnedCreatureProto = AttRes::GetInstance()->GetCreatureProto(dwCreatureTypeID);

		ASSERT( VALID_POINT(pSpawnedCreatureProto) && n_num > 0 );

		for(INT m = 0; m < n_num; m++)
		{
			INT n_num = 0;
			// 找到一个可行走的随机点
			while(TRUE)
			{
				vPos.x = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + vSpawnCenter.x;
				vPos.z = FLOAT(get_tool()->tool_rand() % (pNest->nSpawnRadius * 2) - pNest->nSpawnRadius) + vSpawnCenter.z;

				if(p_creature_->NeedCollide())
				{
					vPos.y = p_creature_->GetCurPosTop().y;
					break;
				}
				else
				{
					if( if_can_go(vPos.x, vPos.z) )
					{
						vPos.y = 0;
						break;
					}
				}

				++n_num;
				if(n_num > 100)
				{
					break;
				}
			}
			if(n_num > 20)
				continue;

			// 随机一个朝向
			FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
			vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.y = 0.0f;

			// 取出一个ID
			DWORD dwID = builder_creature_id.get_new_creature_id();
			ASSERT( IS_CREATURE(dwID) );

			// 生成怪物
			Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pSpawnedCreatureProto, 
				vPos, vFaceTo, ECST_Nest, p_creature_->GetID(), INVALID_VALUE, p_creature_->NeedCollide(), NULL);

			// 加入到地图中
			add_creature_on_load(pSpawnedCreature);
		}
	}
}


//-----------------------------------------------------------------------------------
// 初始化小队怪物
//-----------------------------------------------------------------------------------
VOID Map::init_team_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) && p_creature_->IsTeam() );

	const tagCreatureProto* pProto = p_creature_->GetProto();
	ASSERT( VALID_POINT(pProto) && VALID_POINT(pProto->pNest) );

	const tagNestProto* pNest = pProto->pNest;
	const Vector3& vSpawnCenter = p_creature_->GetCurPos();

	Vector3 vPos;
	Vector3 vFaceTo = p_creature_->GetFaceTo();
	INT		nIndex = 0;
	INT		n_num = 0;
	INT		nNumIndex = 0;

	// 得到怪物小队的队形
	const tagNPCTeamOrder* pTeamOrder = p_npc_team_mgr->GetNPCTeamOrder(pNest->eOrderType);
	ASSERT(VALID_POINT(pTeamOrder));

	// 创建怪物小队
	NPCTeam* pTeam = p_npc_team_mgr->CreateTeam(p_creature_);
	if(!VALID_POINT(pTeam))		return;

	// 设置队长小队ＩＤ
	p_creature_->SetTeamID(pTeam->GetID());
	std::vector<POINT>::const_iterator it = pTeamOrder->NPCOrder.begin();
	while (it != pTeamOrder->NPCOrder.end())
	{

		POINT point = *it;
		DWORD dwCreatureTypeID = pNest->dwSpawnID[nIndex];
		n_num = pNest->nSpawnMax[nIndex];

		const tagCreatureProto* pTeamMemProto = AttRes::GetInstance()->GetCreatureProto(dwCreatureTypeID);
		if(!VALID_POINT(pTeamMemProto))
			return;

		// 取出一个ID
		DWORD dwID = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dwID) );

		// 计算怪物位置
		vPos = p_npc_team_mgr->CalTeamMemPos(p_creature_, point, vFaceTo, pNest);

		// 生成怪物
		Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pTeamMemProto , 
			vPos, vFaceTo, ECST_Team, p_creature_->GetID(),INVALID_VALUE,  p_creature_->NeedCollide(), NULL, pTeam->GetID());

		// 加入到地图中
		add_creature_on_load(pSpawnedCreature);

		// 加入到怪物小队
		pTeam->AddNPCTeamMem(pSpawnedCreature);

		++nNumIndex;
		if(nNumIndex >= n_num)
		{
			++nIndex;
			nNumIndex = 0;
		}

		++it;
	}
}

VOID Map::init_team_creature(Creature* p_creature_, guild* p_guild_)
{
	ASSERT( VALID_POINT(p_creature_) && p_creature_->IsTeam() );

	const tagCreatureProto* pProto = p_creature_->GetProto();
	ASSERT( VALID_POINT(pProto) && VALID_POINT(pProto->pNest) );

	const tagNestProto* pNest = pProto->pNest;
	const Vector3& vSpawnCenter = p_creature_->GetCurPos();

	Vector3 vPos;
	Vector3 vFaceTo = p_creature_->GetFaceTo();
	INT		nIndex = 0;
	INT		n_num = 0;
	INT		nNumIndex = 0;

	// 得到怪物小队的队形
	const tagNPCTeamOrder* pTeamOrder = p_npc_team_mgr->GetNPCTeamOrder(pNest->eOrderType);
	ASSERT(VALID_POINT(pTeamOrder));

	// 创建怪物小队
	NPCTeam* pTeam = p_npc_team_mgr->CreateTeam(p_creature_);
	if(!VALID_POINT(pTeam))		return;

	if(!VALID_POINT(p_guild_)) return ;

	// 设置队长小队ＩＤ
	p_creature_->SetTeamID(pTeam->GetID());

	BYTE byType = EFT_Bank;
	std::vector<POINT>::const_iterator it = pTeamOrder->NPCOrder.begin();
	while (it != pTeamOrder->NPCOrder.end())
	{

		POINT point = *it;
		DWORD dwCreatureTypeID = pNest->dwSpawnID[nIndex];
		n_num = pNest->nSpawnMax[nIndex];

		if(!p_guild_->get_upgrade().IsFacilitesDestory((EFacilitiesType)byType))
		{
			const tagCreatureProto* pTeamMemProto = AttRes::GetInstance()->GetCreatureProto(dwCreatureTypeID);
			if(!VALID_POINT(pTeamMemProto))
				return;

			// 取出一个ID
			DWORD dwID = builder_creature_id.get_new_creature_id();
			ASSERT( IS_CREATURE(dwID) );

			// 计算怪物位置
			vPos = p_npc_team_mgr->CalTeamMemPos(p_creature_, point, vFaceTo, pNest);

			// 生成怪物
			Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pTeamMemProto , 
				vPos, vFaceTo, ECST_GuildSentinel, p_creature_->GetID(), INVALID_VALUE, p_creature_->NeedCollide(), NULL, pTeam->GetID(), p_guild_->get_guild_att().dwID);

			// 加入到地图中
			add_creature_on_load(pSpawnedCreature);

			// 加入到怪物小队
			pTeam->AddNPCTeamMem(pSpawnedCreature);
		}

		byType++;
		++nNumIndex;
		if(nNumIndex >= n_num)
		{
			++nIndex;
			nNumIndex = 0;
		}

		++it;
	}
}

//-----------------------------------------------------------------------------------
// 更新，被管理该地图的MapMgr的线程函数调用
//-----------------------------------------------------------------------------------
VOID Map::update()
{
	DWORD	dw_begin_time = timeGetTime();
	update_session();
	update_all_objects();
	update_all_shops();
	//update_all_chamber();
	update_num();
	//update_weather( );
	DWORD	dw_end_time = timeGetTime();

	dw_update_time = dw_end_time - dw_begin_time;
}

VOID Map::update_weather()
{
	++n_chang_weahter_tick_;
	if(n_chang_weahter_tick_ < p_map_info->n_weather_change_tick)
		return;

	size_t n = p_map_info->vec_weahter_ids.size();
	if(n == 0)
		return;

	n_chang_weahter_tick_ = 0;
	n = get_tool()->tool_rand() % n;

	SetWeather(p_map_info->vec_weahter_ids[n]);
}

//bool Map::initSparseGraph(const tag_map_info* pInfo)
//{
//	// 文件里的是从左上角为原点，转到左下角为原点
//	m_pSparseGraph = new c_sparse_graph(pInfo->n_width * TILE_SCALE, pInfo->n_height * TILE_SCALE, pInfo->n_width, pInfo->n_height);
//	if (!VALID_POINT(m_pSparseGraph))
//		return false;
//
//	for(int j = 0; j < pInfo->n_height; j ++)
//	{
//		for(int i = 0; i < pInfo->n_width; i ++)
//		{
//			if (0 != pInfo->map_block_data[j * pInfo->n_width + i])
//			{
//				m_pSparseGraph->brush(i, pInfo->n_height - j -1 , BT_OBSTACLE);
//			}
//			else
//			{
//				m_pSparseGraph->brush(i, pInfo->n_height - j -1 , BT_NORMAL);
//			}
//		}
//	}
//
//	return true;
//}
//-------------------------------------------------------------------------------------
// 更新地图管理的所有玩家的消息
//-------------------------------------------------------------------------------------
VOID Map::update_session()
{
	PlayerSession* pSession = NULL;
	map_session.reset_iterator();

	while( map_session.find_next(pSession) )
	{
		pSession->Update();
	}
}

//--------------------------------------------------------------------------------------
// 更新地图管理的所有游戏对象的状态
//--------------------------------------------------------------------------------------
VOID Map::update_all_objects()
{
	// 更新所有地图里的玩家
	ROLE_MAP::map_iter itRole = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itRole, pRole) )
	{
		pRole->Update();
	}

	// 更新所有地图里的挂机玩家
	ROLE_MAP::map_iter iter_leave_role = map_leave_role.begin();
	Role* pLeaveRole = NULL;
	while(map_leave_role.find_next(iter_leave_role, pLeaveRole))
	{
		pLeaveRole->Update();
	}

	// 更新地图里的所有怪物
	CREATURE_MAP::map_iter itCreature = map_creature.begin();
	Creature* pCreature = NULL;

	while( map_creature.find_next(itCreature, pCreature) )
	{
		if(pCreature->IsDelMap())
		{
			map_creature.erase(pCreature->GetID());
			pCreature->SetDelMap(FALSE);
			// 从删除怪物时移到这里
			remove_from_visible_tile(pCreature);
			// 放入到复活列表中等待复活
			map_respawn_creature.add(pCreature->GetID(), pCreature);
			continue;
		}
		pCreature->Update();

		//if(pCreature->IsPet())
		//{
		//	Pet* pPet = (Pet*)pCreature;
		//	if(pPet->IsDel())
		//	{
		//		Pet::Delete(pPet);
		//	}
		//}
	}

	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	// 更新地图里面所有待刷新的怪物
	CREATURE_MAP::map_iter itDeadCreature = map_respawn_creature.begin();
	Creature* pDeadCreature = NULL;

	while( map_respawn_creature.find_next(itDeadCreature, pDeadCreature) )
	{
		ECreatureReviveResult eRet = pDeadCreature->TryRevive();

		if( ECRR_Success == eRet )
		{
			// 重生成功
			map_respawn_creature.erase(pDeadCreature->GetID());
			add_creature(pDeadCreature);
		}
		else if( ECRR_Failed == eRet )
		{
			// 重生失败
		}
		else if( ECRR_NeedDestroy == eRet )
		{
			// 动态生成的，需要删除了
			map_respawn_creature.erase(pDeadCreature->GetID());
			Creature::Delete(pDeadCreature);
		}
		else if( ECRR_NeedReplace == eRet )
		{
			// 刷怪点生成的，需要替换
			spawn_creature_replace(pDeadCreature);
		}
		else if (ECRR_NeedRepos == eRet)
		{
			// 重新随机个位置
			spawn_list_creature_replace(pDeadCreature);
		}
		else
		{
			ASSERT(0);
		}
	}
	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 10)
	{
		g_world.get_log()->write_log(_T("Creature map_respawn_creature time %d\r\n"), dw_new_time - dw_time);
	}

	dw_time = timeGetTime();
	// 更新地面物品
	package_map<INT64, tag_ground_item*>::map_iter it = map_ground_item.begin();
	tag_ground_item* pGroundItem = NULL;

	while( map_ground_item.find_next(it, pGroundItem) )
	{
		switch(pGroundItem->update())
		{
			// 移除地物
		case EGIU_remove:
			{
				// 移除地物
				remove_ground_item(pGroundItem);

				// 销毁物品
				pGroundItem->destroy_item();

				// 释放内存
				SAFE_DELETE(pGroundItem);
			}
			break;
			// 同步地物状态
		case EGIU_synchronize:
			{
				// 计算物品所在的可视地砖格子
				INT nVisIndex = world_position_to_visible_index(pGroundItem->v_pos);

				// 得到九宫格
				tagVisTile* pVisTile[EUD_end] = {0};
				get_visible_tile(nVisIndex, pVisTile);

				// 同步给加入到客户端的玩家和生物
				synchronize_ground_item_state(pGroundItem, pVisTile);
			}
			break;
		case EGIU_null:
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	// 更新要删除的宠物内存数据
	CREATURE_MAP::map_iter itPet = map_pet_delete.begin();
	Creature* pPetCreature = NULL;

	while( map_pet_delete.find_next(itPet, pPetCreature) )
	{
		Pet* pPet = (Pet*)pPetCreature;
		Pet::Delete(pPet);
	}
	
	map_pet_delete.clear();

	dw_new_time = timeGetTime();
	if(dw_new_time - dw_time > 10)
	{
		g_world.get_log()->write_log(_T("map_ground_item time %d\r\n"), dw_new_time - dw_time);
	}
}

//---------------------------------------------------------------------------------------
// 更新该地图内所有商店
//---------------------------------------------------------------------------------------
VOID Map::update_all_shops()
{
	Shop *pShop = NULL;
	map_shop.reset_iterator();

	while( map_shop.find_next(pShop) )
	{
		pShop->Update();
	}
}

//---------------------------------------------------------------------------------------
// 更新该地图内所有商会
//---------------------------------------------------------------------------------------
VOID Map::update_all_chamber()
{
	guild_chamber *pCofC = NULL;
	map_chamber.reset_iterator();

	while( map_chamber.find_next(pCofC) )
	{
		pCofC->update();
	}
}

VOID Map::update_num()
{
	dw_update_num_time -= TICK_TIME;

	if(dw_update_num_time <= 0)
	{
		for(INT i = 0; i < n_visible_tile_array_width * n_visible_tile_array_height; i++)
		{
			INT n_max_num = p_visible_tile[i].map_role.size();
			n_max_num += p_visible_tile[i].map_creature.size();

			if(n_max_num > p_visible_tile[i].n_max_num)
			{
				g_world.get_log()->write_log(_T("MapName=%s VisibleIndex=%d Max_Num=%d\r\n"), get_map_info()->sz_map_name, i, n_max_num);
				package_map<DWORD, Role*>::map_iter role_iter = p_visible_tile[i].map_role.begin();
				Role* pRole = NULL;
				while(p_visible_tile[i].map_role.find_next(role_iter, pRole))
				{
					if(VALID_POINT(pRole))
					{
						g_world.get_log()->write_log(_T("MapName=%s	Role_ID=%u\r\n"), get_map_info()->sz_map_name, pRole->GetID());
					}
				}

				package_map<DWORD, Creature*>::map_iter creature_iter = p_visible_tile[i].map_creature.begin();
				Creature* pCreature = NULL;
				while(p_visible_tile[i].map_creature.find_next(creature_iter, pCreature))
				{
					if(VALID_POINT(pCreature))
					{
						g_world.get_log()->write_log(_T("MapName=%s Creature_ID=%u\r\n"), get_map_info()->sz_map_name, pCreature->GetTypeID());
					}
				}
			}
		}

		dw_update_num_time = UPDATE_NUM_TIME;
	}
}

//---------------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------------
VOID Map::add_role(Role* p_role_, Map* pScrMap)
{
	ASSERT( VALID_POINT(p_role_) );

	send_npc_loading(p_role_);

	// 加入到玩家列表中
	map_role.add(p_role_->GetID(), p_role_);
	map_leave_role.erase(p_role_->GetID());

	// 将该玩家的session加入到session列表中
	PlayerSession* pSession = p_role_->GetSession();
	if( VALID_POINT(pSession) )
	{
		map_session.add(pSession->GetSessionID(), pSession);
	}

	// 设置玩家的地图
	p_role_->SetMap(this);

	// 修正坐标
	fix_position(p_role_->GetCurPos());

	// 修正高度
	p_role_->GetMoveData().DropDownStandPoint();

	//// 计算地图区域  // fix mwh2011-5-29 切不同地图时不能拿到区域索引
	//p_role_->CheckMapArea();

	// 发送进入地图信息给客户端
	send_goto_new_map_to_player(p_role_);

	// 发送该地图中场景特效
	send_scene_effect(p_role_);

	// 计算玩家所在的可视地砖格子
	INT nVisIndex = world_position_to_visible_index(p_role_->GetCurPos());

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// 同步给加入到客户端的玩家和生物
	//synchronize_add_units(pRole, pVisTile);

	// 格子内非隐身单位及掉落
	syn_big_visible_tile_visible_unit_and_ground_item_to_role(p_role_, pVisTile);

	// 格子内隐身单位
	syn_big_visible_tile_invisible_unit_to_role(p_role_, pVisTile);
	

	// 同步门对象 
	// 门对象不在可视地砖格子内  需要在玩家进入地图的时候同步
	map_door.reset_iterator();	
	Creature* pDoor = NULL;
	while( map_door.find_next(pDoor) )
	{
		if( VALID_POINT(pDoor) )
		{
			BYTE byMsg[1024] = {0};
			DWORD dw_size = calculate_movement_message( pDoor, byMsg, 1024 );			
			p_role_->SendMessage( byMsg, dw_size );
		}
	}

	// 让玩家落到9宫格之内
	add_to_visible_tile(p_role_, nVisIndex);

	//set_have_unit(p_role_->GetCurPos(), true);

	// 计算地图区域 // fix mwh 2011-05-26
	p_role_->CheckMapArea();

	// 进入地图的相关事项
	//mmz
	p_role_->OnEnterMap(pScrMap);

	//// 调用脚本
	if( VALID_POINT(p_script) )
	{
		p_script->OnPlayerEnter(p_role_, this);
	}
	//mmz

}

DWORD Map::FriendEnemy(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_)
{
	b_ignore_ = FALSE;
	return 0;
}

//----------------------------------------------------------------------------------------
// 发送预加载NPC信息
//----------------------------------------------------------------------------------------
VOID Map::send_npc_loading(Role* p_role_)
{
	if(list_NPCLoading.size() > 0)
	{
		DWORD	dw_message_size = sizeof(NET_SIS_NPC_Loading) + (list_NPCLoading.size() - 1) * sizeof(tagNPCLoading);

		CREATE_MSG(pSend, dw_message_size, NET_SIS_NPC_Loading);

		pSend->n32_num = list_NPCLoading.size();

		package_list<tagNPCLoading*>::list_iter iter = list_NPCLoading.begin();
		tagNPCLoading* p_npc_loading = NULL;
		INT n_index = 0;
		while(list_NPCLoading.find_next(iter, p_npc_loading))
		{
			if(!VALID_POINT(p_npc_loading))
				continue;

			memcpy(&pSend->st_NPCLoading[n_index], p_npc_loading, sizeof(tagNPCLoading));
			n_index++;
		}

		p_role_->SendMessage(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}
}



//-----------------------------------------------------------------------------------------
// 地图内加入一个生物，这是在载入时添加的，所以还不需要同步
//-----------------------------------------------------------------------------------------
VOID Map::add_creature_on_load(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// 加入到生物列表中
	map_creature.add(p_creature_->GetID(), p_creature_);

	// 设置生物的地图 
	p_creature_->SetMap(this);
	
	//pCreature->OnScriptLoad();

	// 计算生物所在的可视地砖格子
	INT nVisIndex = world_position_to_visible_index(p_creature_->GetCurPos());

	// 让生物落到九宫格之内
	add_to_visible_tile(p_creature_, nVisIndex);

	// 如果是商店职能NPC，则挂载对应的商店
	if (ECT_NPC == p_creature_->GetProto()->eType)
	{
		switch (p_creature_->GetProto()->eFunctionType)
		{
		case EFNPCT_Shop:
			add_shop(p_creature_->GetTypeID(), p_creature_->GetShopID());
			break;

		case EFNPCT_CofC:
			add_chamber(p_creature_->GetTypeID(), p_creature_->GetShopID());
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------
// 地图中加入一个生物，这是在游戏运行时添加，所以需要同步
//-----------------------------------------------------------------------------------------
VOID Map::add_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// 加入到生物列表中
	map_creature.add(p_creature_->GetID(), p_creature_);

	// 设置生物的地图
	p_creature_->SetMap(this);

	// 计算生物所在的可视地砖格子
	INT nVisIndex = world_position_to_visible_index(p_creature_->GetCurPos());

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// 同步给加入到客户端的玩家和生物
	synchronize_add_units(p_creature_, pVisTile);

	// 让生物落到9宫格之内
	add_to_visible_tile(p_creature_, nVisIndex);

	// 如果是商店职能NPC，则挂载对应的商店
	if (ECT_NPC == p_creature_->GetProto()->eType)
	{
		switch (p_creature_->GetProto()->eFunctionType)
		{
		case EFNPCT_Shop:
			add_shop(p_creature_->GetTypeID(), p_creature_->GetShopID());
			break;

		case EFNPCT_CofC:
			add_chamber(p_creature_->GetTypeID(), p_creature_->GetShopID());
			break;
		}
	}
}

VOID Map::add_pet_delete_map(Creature* pPet)
{
	if( VALID_POINT(pPet) )
	{
		// 加入到生物列表中
		map_pet_delete.add(pPet->GetID(), pPet);
	}
}
//-----------------------------------------------------------------------------------------
// 从地图中拿走一个生物，放入死亡列表
//-----------------------------------------------------------------------------------------
VOID Map::remove_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// 从生物列表中拿走这个怪物(存在map安全问题lc)
	//m_mapCreature.Erase(pCreature->GetID());

	p_creature_->SetDelMap(TRUE);

	// 刷怪点怪物标记重置
	tag_map_spawn_point_list* pMapSpawnList = p_map_info->map_spawn_point_list.find(p_creature_->GetSpawnGroupID());
	if (VALID_POINT(pMapSpawnList))
	{
		tag_map_spawn_point_info* pMapSpawnInfo = pMapSpawnList->list.find(p_creature_->GetSpawnPtID());	
		if (VALID_POINT(pMapSpawnInfo))
		{
			//pMapSpawnInfo->b_hasUnit = FALSE;
			map_point_has_list[pMapSpawnInfo->dw_att_id] = FALSE;
		}
	}
	// 检查生物所在的vistile
	//INT nVisIndex = p_creature_->GetVisTileIndex();

	// 得到九宫格
	//tagVisTile* pVisTile[EUD_end] = {0};
	//get_visible_tile(nVisIndex, pVisTile);

	// 这里移到updata时调用
	// 从可视地砖中拿走，但不同步客户端 
	//remove_from_visible_tile(p_creature_);

	// 放入到复活列表中等待复活
	//m_mapRespawnCreature.Add(pCreature->GetID(), pCreature);
}

//-----------------------------------------------------------------------------------------
// 从地图中移除宠物
//-----------------------------------------------------------------------------------------
VOID Map::remove_pet(Pet* p_pet_, BOOL bSendMsg)
{
	if (!VALID_POINT(p_pet_))
	{
		ASSERT(0);
		return;
	}

	// 从生物列表中拿走这个怪物
	map_creature.erase(p_pet_->GetID());

	p_pet_->SetMap(NULL);

	// 从可视地砖中拿走，但不同步客户端
	remove_from_visible_tile(p_pet_);

	// 同步给客户端
	if (bSendMsg)
		synchronize_remove_unit_to_big_visible_tile(p_pet_);
}

VOID Map::remove_npc( Creature* p_creature_ )
{
	if( !VALID_POINT(p_creature_) )
	{
		ASSERT(0);
		return;
	}
	// 从生物列表中拿走这个怪物
	map_creature.erase(p_creature_->GetID());

	// 检查生物所在的vistile
	INT nVisIndex = p_creature_->GetVisTileIndex();


	// 放入到复活列表中等待复活
	map_respawn_creature.add(p_creature_->GetID(), p_creature_);
}
//-----------------------------------------------------------------------------------------
// 加入到地图中的某个VisTile
//-----------------------------------------------------------------------------------------
VOID Map::add_to_visible_tile(Unit* p_unit_, INT n_visible_index_)
{
	ASSERT( VALID_POINT(p_unit_) );
	ASSERT( n_visible_index_ != INVALID_VALUE );

	if(!VALID_POINT(p_unit_))
		return;

	if(INVALID_VALUE == n_visible_index_)
	{
		if(p_unit_->IsCreature())
		{
			Creature* pCreature = (Creature*)p_unit_;
			if(VALID_POINT(pCreature))
			{
				print_message(_T("CreatureID %d add map error!!! \n"), pCreature->GetProto()->dw_data_id);
			}
		}
		return;
	}

	// 门特殊处理 不放入可视列表
	if ( IS_DOOR(p_unit_->GetID()) )
		return;

	p_unit_->SetVisTileIndex(n_visible_index_);
	if( p_unit_->IsRole() )
	{
		p_visible_tile[n_visible_index_].map_role.add(p_unit_->GetID(), static_cast<Role*>(p_unit_));
		p_visible_tile[n_visible_index_].map_leave_role.erase(p_unit_->GetID());
	}
	else
		p_visible_tile[n_visible_index_].map_creature.add(p_unit_->GetID(), static_cast<Creature*>(p_unit_));

	if( p_unit_->IsInStateInvisible() )
		p_visible_tile[n_visible_index_].map_invisible_unit.add(p_unit_->GetID(), p_unit_);
}

//-----------------------------------------------------------------------------------------
// 从某个可视地砖中删除一个玩家
//-----------------------------------------------------------------------------------------
VOID Map::remove_from_visible_tile(Unit* p_unit_, BOOL b_leave_online)
{
	ASSERT( VALID_POINT(p_unit_) );

	// 门特殊处理 不放入可视列表	
	if ( IS_DOOR(p_unit_->GetID()) )
		return;

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT( VALID_VALUE(nVisIndex) );

	if( p_unit_->IsRole() )
	{
		p_visible_tile[nVisIndex].map_role.erase(p_unit_->GetID());
		// 添加离线挂机逻辑
		Role* pRole = (Role*)p_unit_;
		if(pRole->is_leave_pricitice() && pRole->IsInRoleState(ERS_Prictice) && b_leave_online)
		{
			p_visible_tile[nVisIndex].map_leave_role.add(p_unit_->GetID(), pRole);
		}
		else
		{
			p_visible_tile[nVisIndex].map_leave_role.erase(p_unit_->GetID());
		}
	}
	else
		p_visible_tile[nVisIndex].map_creature.erase(p_unit_->GetID());

	if( p_unit_->IsInStateInvisible() )
		p_visible_tile[nVisIndex].map_invisible_unit.erase(p_unit_->GetID());
	
	if(p_unit_->IsRole())
	{
		// 添加离线挂机逻辑
		Role* pRole = (Role*)p_unit_;
		if(!pRole->is_leave_pricitice())
		{
			p_unit_->SetVisTileIndex(INVALID_VALUE);
		}
	}
	else
	{
		p_unit_->SetVisTileIndex(INVALID_VALUE);
	}
	
}

//-----------------------------------------------------------------------------------------
// 世界单位转为可视地砖的索引
//-----------------------------------------------------------------------------------------
INT Map::world_position_to_visible_index(const Vector3& v_pos_)
{
	// 坐标非法
	if( !is_position_valid(v_pos_) ) return INVALID_VALUE;

	INT nIndexX = INT(v_pos_.x / TILE_SCALE / p_map_info->n_visit_distance) + 1;
	INT nIndexZ = INT(v_pos_.z / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	ASSERT( nIndexX > 0 && nIndexX < n_visible_tile_array_width - 1 );
	ASSERT( nIndexZ > 0 && nIndexZ < n_visible_tile_array_height - 1 );

	return nIndexZ * n_visible_tile_array_width + nIndexX;
}

//------------------------------------------------------------------------------------------
// 得到周围地砖
//------------------------------------------------------------------------------------------
VOID Map::get_visible_tile(INT n_visible_index_, tagVisTile* p_visible_tile_[EUD_end])
{
	// todo: 这个地方还要改一下，判断不是很严谨
	if( n_visible_index_ == INVALID_VALUE )
		return;

	// 视野的地砖索引
	INT m[EUD_end] = {0};
	m[EUD_center]		= n_visible_index_;
	m[EUD_left]			= n_visible_index_ - 1;
	m[EUD_right]			= n_visible_index_ + 1;
	m[EUD_top]			= n_visible_index_ - n_visible_tile_array_width;
	m[EUD_bottom]		= n_visible_index_ + n_visible_tile_array_width;
	m[EUD_left_top]		= m[EUD_top] - 1;
	m[EUD_right_top]		= m[EUD_top] + 1;
	m[EUD_left_bottom]	= m[EUD_bottom] - 1;
	m[EUD_right_bottom]	= m[EUD_bottom] + 1;

	for(INT n = 0 ; n < EUD_end; n++)
	{
		p_visible_tile_[n] = &p_visible_tile[m[n]];
	}
}

//------------------------------------------------------------------------------------------
// 根据某个坐标点以及范围确定有几个visTile在该矩形范围内
//------------------------------------------------------------------------------------------
VOID Map::get_visible_tile(Vector3& v_pos_, FLOAT f_range_, tagVisTile* p_visible_tile_[EUD_end])
{
	BOOL b[EUD_end] = {0};

	INT m[EUD_end] = {0};
	m[EUD_center]		= INVALID_VALUE;
	m[EUD_left]			= INVALID_VALUE;
	m[EUD_right]			= INVALID_VALUE;
	m[EUD_top]			= INVALID_VALUE;
	m[EUD_bottom]		= INVALID_VALUE;
	m[EUD_left_top]		= INVALID_VALUE;
	m[EUD_right_top]		= INVALID_VALUE;
	m[EUD_left_bottom]	= INVALID_VALUE;
	m[EUD_right_bottom]	= INVALID_VALUE;

	// 取到中心点的visIndex
	INT nIndexX = INT(v_pos_.x / TILE_SCALE / p_map_info->n_visit_distance) + 1;
	INT nIndexZ = INT(v_pos_.z / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	ASSERT( nIndexX > 0 && nIndexX < n_visible_tile_array_width - 1 );
	ASSERT( nIndexZ > 0 && nIndexZ < n_visible_tile_array_height - 1 );

	// 取到边界的索引
	FLOAT fTempX = v_pos_.x - f_range_;
	INT nSrcIndexX = INT(fTempX / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	fTempX = v_pos_.x + f_range_;
	INT nDestIndexX = INT(fTempX / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	FLOAT fTempZ = v_pos_.z - f_range_;
	INT nSrcIndexZ = INT(fTempZ / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	fTempZ = v_pos_.z + f_range_;
	INT nDestIndexZ = INT(fTempZ / TILE_SCALE / p_map_info->n_visit_distance) + 1;


	// 设置是否包含
	b[EUD_center] = TRUE;
	if( nSrcIndexX < nIndexX )
	{
		b[EUD_left] = TRUE;	
	}
	if( nDestIndexX > nIndexX )
	{
		b[EUD_right] = TRUE;
	}
	if( nSrcIndexZ < nIndexZ )
	{
		b[EUD_top] = TRUE;
	}
	if( nDestIndexZ > nIndexZ )
	{
		b[EUD_bottom] = TRUE;
	}

	b[EUD_left_top]		=	b[EUD_left] && b[EUD_top];
	b[EUD_left_bottom]	=	b[EUD_left] && b[EUD_bottom];
	b[EUD_right_top]		=	b[EUD_right] && b[EUD_top];
	b[EUD_right_bottom]	=	b[EUD_right] && b[EUD_bottom];


	m[EUD_center] = nIndexZ * n_visible_tile_array_width + nIndexX;

	if( b[EUD_left] )		m[EUD_left] = m[EUD_center] - 1;
	if( b[EUD_right] )		m[EUD_right] = m[EUD_center] + 1;
	if( b[EUD_top] )			m[EUD_top] = m[EUD_center] - n_visible_tile_array_width;
	if( b[EUD_bottom] )		m[EUD_bottom] = m[EUD_center] + n_visible_tile_array_width;
	if( b[EUD_left_top] )		m[EUD_left_top] = m[EUD_top] - 1;
	if( b[EUD_right_top] )	m[EUD_right_top] = m[EUD_top] + 1;
	if( b[EUD_left_bottom] )	m[EUD_left_bottom] = m[EUD_bottom] - 1;
	if( b[EUD_right_bottom] )	m[EUD_right_bottom] = m[EUD_bottom] + 1;

	for(INT n = 0 ; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_[n] = &p_visible_tile[m[n]];
	}

}

//------------------------------------------------------------------------------------------
// 得到玩家改变位置后离开视野的地砖
//-------------------------------------------------------------------------------------------
VOID Map::get_visible_tile_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end])
{
	if( n_old_visible_index_ == INVALID_VALUE || n_new_visible_index_ == INVALID_VALUE )
		return;

	if( n_new_visible_index_ == n_old_visible_index_ )
		return;

	INT nOldVisIndexX = n_old_visible_index_ % n_visible_tile_array_width;
	INT nOldVisIndexZ = n_old_visible_index_ / n_visible_tile_array_width;
	INT nNewVisIndexX = n_new_visible_index_ % n_visible_tile_array_width;
	INT nNewVisIndexZ = n_new_visible_index_ / n_visible_tile_array_width;

	INT m[EUD_end] = {0};
	m[EUD_center]		= INVALID_VALUE;
	m[EUD_left]			= INVALID_VALUE;
	m[EUD_right]			= INVALID_VALUE;
	m[EUD_top]			= INVALID_VALUE;
	m[EUD_bottom]		= INVALID_VALUE;
	m[EUD_left_top]		= INVALID_VALUE;
	m[EUD_right_top]		= INVALID_VALUE;
	m[EUD_left_bottom]	= INVALID_VALUE;
	m[EUD_right_bottom]	= INVALID_VALUE;

	if( abs(nOldVisIndexX - nNewVisIndexX) >= 3
		|| abs(nOldVisIndexZ - nNewVisIndexZ) >= 3 )
	{
		m[EUD_center]		= n_old_visible_index_;
		m[EUD_left]			= n_old_visible_index_ - 1;
		m[EUD_right]			= n_old_visible_index_ + 1;
		m[EUD_top]			= n_old_visible_index_ - n_visible_tile_array_width;
		m[EUD_bottom]		= n_old_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		for(INT n = 0; n < EUD_end; n++)
		{
			p_visible_tile_[n] = &p_visible_tile[m[n]];
		}
		return;
	}

	// X左边移动
	if( nNewVisIndexX < nOldVisIndexX )
	{
		m[EUD_right]			= n_old_visible_index_ + 1;
		m[EUD_right_top]		= m[EUD_right] - n_visible_tile_array_width;
		m[EUD_right_bottom]	= m[EUD_right] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == -2 )
		{
			m[EUD_center]	= n_old_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X右边移动
	else if( nNewVisIndexX > nOldVisIndexX )
	{
		m[EUD_left]			= n_old_visible_index_ - 1;
		m[EUD_left_top]		= m[EUD_left] - n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_left] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == 2 )
		{
			m[EUD_center]	= n_old_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X没有移动
	else
	{

	}

	// Z上部移动
	if( nNewVisIndexZ < nOldVisIndexZ )
	{
		m[EUD_bottom]		= n_old_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		if( nNewVisIndexZ - nOldVisIndexZ == -2 )
		{
			m[EUD_center]	= n_old_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;
		}
	}
	// Z下部移动
	else if( nNewVisIndexZ > nOldVisIndexZ )
	{
		m[EUD_top]			= n_old_visible_index_ - n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;

		if( nNewVisIndexZ - nOldVisIndexZ == 2 )
		{
			m[EUD_center]	= n_old_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;
		}
	}
	// Z没有移动
	else
	{

	}

	// 统计坐标点
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_[n] = &p_visible_tile[m[n]];
	}
}

//------------------------------------------------------------------------------------------
// 得到玩家改变位置后进入视野的地砖
//-------------------------------------------------------------------------------------------
VOID Map::get_visible_tile_add(INT n_ole_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end])
{
	if( n_ole_visible_index_ == INVALID_VALUE || n_new_visible_index_ == INVALID_VALUE )
		return;

	if( n_new_visible_index_ == n_ole_visible_index_ )
		return;

	INT nOldVisIndexX = n_ole_visible_index_ % n_visible_tile_array_width;
	INT nOldVisIndexZ = n_ole_visible_index_ / n_visible_tile_array_width;
	INT nNewVisIndexX = n_new_visible_index_ % n_visible_tile_array_width;
	INT nNewVisIndexZ = n_new_visible_index_ / n_visible_tile_array_width;

	INT m[EUD_end] = {0};
	m[EUD_center]		= INVALID_VALUE;
	m[EUD_left]			= INVALID_VALUE;
	m[EUD_right]			= INVALID_VALUE;
	m[EUD_top]			= INVALID_VALUE;
	m[EUD_bottom]		= INVALID_VALUE;
	m[EUD_left_top]		= INVALID_VALUE;
	m[EUD_right_top]		= INVALID_VALUE;
	m[EUD_left_bottom]	= INVALID_VALUE;
	m[EUD_right_bottom]	= INVALID_VALUE;

	if( abs(nOldVisIndexX - nNewVisIndexX) >= 3
		|| abs(nOldVisIndexZ - nNewVisIndexZ) >= 3 )
	{
		m[EUD_center]		= n_new_visible_index_;
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_top]			= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		for(INT n = 0; n < EUD_end; n++)
		{
			p_visible_tile_[n] = &p_visible_tile[m[n]];
		}
		return;
	}

	// X左边移动
	if( nNewVisIndexX < nOldVisIndexX )
	{
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_left_top]		= m[EUD_left] - n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_left] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == -2 )
		{
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X右边移动
	else if( nNewVisIndexX > nOldVisIndexX )
	{
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_right_top]		= m[EUD_right] - n_visible_tile_array_width;
		m[EUD_right_bottom]	= m[EUD_right] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == 2 )
		{
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X没有移动
	else
	{

	}

	// Z上部移动
	if( nNewVisIndexZ < nOldVisIndexZ )
	{
		m[EUD_top]		= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_left_top]	= m[EUD_top] - 1;
		m[EUD_right_top]	= m[EUD_top] + 1;

		if( nNewVisIndexZ - nOldVisIndexZ == -2 )
		{
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;
		}
	}
	// Z下部移动
	else if( nNewVisIndexZ > nOldVisIndexZ )
	{
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		if( nNewVisIndexZ - nOldVisIndexZ == 2 )
		{
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;
		}
	}
	// Z没有移动
	else
	{

	}

	// 统计坐标点
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_[n] = &p_visible_tile[m[n]];
	}
}

//---------------------------------------------------------------------------------------------
// 得到玩家改变位置后进入视野的地砖和离开视野的地砖
//---------------------------------------------------------------------------------------------
VOID Map::get_visible_tile_add_and_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_add_[EUD_end], tagVisTile* p_visible_tile_dec_[EUD_end])
{
	if( n_old_visible_index_ == INVALID_VALUE || n_new_visible_index_ == INVALID_VALUE )
		return;

	if( n_new_visible_index_ == n_old_visible_index_ )
		return;

	INT nOldVisIndexX = n_old_visible_index_ % n_visible_tile_array_width;
	INT nOldVisIndexZ = n_old_visible_index_ / n_visible_tile_array_width;
	INT nNewVisIndexX = n_new_visible_index_ % n_visible_tile_array_width;
	INT nNewVisIndexZ = n_new_visible_index_ / n_visible_tile_array_width;

	// 进入视野的地砖索引
	INT m[EUD_end] = {0};
	m[EUD_center]		= INVALID_VALUE;
	m[EUD_left]			= INVALID_VALUE;
	m[EUD_right]			= INVALID_VALUE;
	m[EUD_top]			= INVALID_VALUE;
	m[EUD_bottom]		= INVALID_VALUE;
	m[EUD_left_top]		= INVALID_VALUE;
	m[EUD_right_top]		= INVALID_VALUE;
	m[EUD_left_bottom]	= INVALID_VALUE;
	m[EUD_right_bottom]	= INVALID_VALUE;

	// 离开视野的地砖索引
	INT r[EUD_end] = {0};
	r[EUD_center]		= INVALID_VALUE;
	r[EUD_left]			= INVALID_VALUE;
	r[EUD_right]			= INVALID_VALUE;
	r[EUD_top]			= INVALID_VALUE;
	r[EUD_bottom]		= INVALID_VALUE;
	r[EUD_left_top]		= INVALID_VALUE;
	r[EUD_right_top]		= INVALID_VALUE;
	r[EUD_left_bottom]	= INVALID_VALUE;
	r[EUD_right_bottom]	= INVALID_VALUE;


	if( abs(nOldVisIndexX - nNewVisIndexX) >= 3
		|| abs(nOldVisIndexZ - nNewVisIndexZ) >= 3 )
	{
		// 进入视野的可视地砖
		m[EUD_center]		= n_new_visible_index_;
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_top]			= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		// 离开视野的可视地砖
		r[EUD_center]		= n_old_visible_index_;
		r[EUD_left]			= n_old_visible_index_ - 1;
		r[EUD_right]			= n_old_visible_index_ + 1;
		r[EUD_top]			= n_old_visible_index_ - n_visible_tile_array_width;
		r[EUD_bottom]		= n_old_visible_index_ + n_visible_tile_array_width;
		r[EUD_left_top]		= r[EUD_top] - 1;
		r[EUD_right_top]		= r[EUD_top] + 1;
		r[EUD_left_bottom]	= r[EUD_bottom] - 1;
		r[EUD_right_bottom]	= r[EUD_bottom] + 1;

		for(INT n = 0; n < EUD_end; n++)
		{
			p_visible_tile_add_[n] = &p_visible_tile[m[n]];
			p_visible_tile_dec_[n] = &p_visible_tile[r[n]];
		}
		return;
	}

	// X左边移动
	if( nNewVisIndexX < nOldVisIndexX )
	{
		// 进入视野的可视地砖
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_left_top]		= m[EUD_left] - n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_left] + n_visible_tile_array_width;

		// 离开视野的可视地砖
		r[EUD_right]			= n_old_visible_index_ + 1;
		r[EUD_right_top]		= r[EUD_right] - n_visible_tile_array_width;
		r[EUD_right_bottom]	= r[EUD_right] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == -2 )
		{
			// 进入视野的可视地砖
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;

			// 离开视野的可视地砖
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_top]		= r[EUD_center] - n_visible_tile_array_width;
			r[EUD_bottom]	= r[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X右边移动
	else if( nNewVisIndexX > nOldVisIndexX )
	{
		// 进入视野的可视地砖
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_right_top]		= m[EUD_right] - n_visible_tile_array_width;
		m[EUD_right_bottom]	= m[EUD_right] + n_visible_tile_array_width;

		// 离开视野的可视地砖
		r[EUD_left]			= n_old_visible_index_ - 1;
		r[EUD_left_top]		= r[EUD_left] - n_visible_tile_array_width;
		r[EUD_left_bottom]	= r[EUD_left] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == 2 )
		{
			// 进入视野的可视地砖
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;

			// 离开视野的可视地砖
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_top]		= r[EUD_center] - n_visible_tile_array_width;
			r[EUD_bottom]	= r[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X没有移动
	else
	{

	}

	// Z上部移动
	if( nNewVisIndexZ < nOldVisIndexZ )
	{
		// 进入视野的可视地砖
		m[EUD_top]			= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;

		// 离开视野的可视地砖
		r[EUD_bottom]		= n_old_visible_index_ + n_visible_tile_array_width;
		r[EUD_left_bottom]	= r[EUD_bottom] - 1;
		r[EUD_right_bottom]	= r[EUD_bottom] + 1;


		if( nNewVisIndexZ - nOldVisIndexZ == -2 )
		{
			// 进入视野的可视地砖
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;

			// 离开视野的可视地砖
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_left]		= r[EUD_center] - 1;
			r[EUD_bottom]	= r[EUD_center] + 1;

		}
	}
	// Z下部移动
	else if( nNewVisIndexZ > nOldVisIndexZ )
	{
		// 进入视野的可视地砖
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		// 离开视野的可视地砖
		r[EUD_top]			= n_old_visible_index_ - n_visible_tile_array_width;
		r[EUD_left_top]		= r[EUD_top] - 1;
		r[EUD_right_top]		= r[EUD_top] + 1;


		if( nNewVisIndexZ - nOldVisIndexZ == 2 )
		{
			// 进入视野的可视地砖
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;

			// 离开视野的可视地砖
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_left]		= r[EUD_center] - 1;
			r[EUD_bottom]	= r[EUD_center] + 1;
		}
	}
	// Z没有移动
	else
	{

	}

	// 统计坐标点
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_add_[n] = &p_visible_tile[m[n]];

		if( r[n] != INVALID_VALUE )
			p_visible_tile_dec_[n] = &p_visible_tile[r[n]];
	}
}

//------------------------------------------------------------------------------------------
// 发送九宫格范围内的消息
//------------------------------------------------------------------------------------------
VOID Map::send_big_visible_tile_message(Unit* p_self_, const LPVOID p_message_, const DWORD dw_size_)
{
	if( !VALID_POINT(p_self_) ) return;
	if( !VALID_POINT(p_message_) ) return;

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(p_self_->GetVisTileIndex(), pVisTile);

	send_big_visible_tile_message(pVisTile, p_message_, dw_size_);
}

//------------------------------------------------------------------------------------------
// 发送九宫格范围内的消息
//------------------------------------------------------------------------------------------
VOID Map::send_big_visible_tile_message(tagVisTile *p_visible_tile_[EUD_end], const LPVOID p_message_, const DWORD dw_size_)
{
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == p_visible_tile_[n] )
			continue;
		
		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = p_visible_tile_[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( VALID_POINT(pRole) )
			{
				pRole->SendMessage(p_message_, dw_size_);
			}
		}
	}
}

//------------------------------------------------------------------------------------------
// 发送消息给本地图内的玩家
//------------------------------------------------------------------------------------------
VOID Map::send_map_message(const LPVOID p_message_, const DWORD dw_size_)
{
	if(!VALID_POINT(p_message_))	return;
	Role *pRole = NULL;

	package_map<DWORD, Role*>::map_iter it = map_role.begin();
	while (map_role.find_next(it, pRole))
	{
		if( VALID_POINT(pRole) && VALID_POINT(pRole->GetSession()) )
		{
			pRole->GetSession()->SendMessage(p_message_, dw_size_);
		}
	}
}

//--------------------------------------------------------------------------------------------
// 同步可视地砖内的隐身单位给玩家
//--------------------------------------------------------------------------------------------
VOID Map::syn_big_visible_tile_invisible_unit_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end])
{
	// 1.是否有探隐能力
	if( !pRole->HasDectiveSkill() )
		return;
	
	BYTE			byMsg[1024] = {0};
	package_list<DWORD>	listRemove;
	
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的单位
		package_map<DWORD, Unit*>& mapUnit = pVisTile[n]->map_invisible_unit;
		mapUnit.reset_iterator();
		Unit* pUnit = NULL;

		while( mapUnit.find_next(pUnit) )
		{
			if( !VALID_POINT(pUnit) )
				continue;

			// 2.是否完全可见
			if( (pRole->CanSeeTargetEntirely(pUnit)) )
				continue;

			// 3.在可视列表中
			if( pRole->IsInVisList(pUnit->GetID()) )
			{
				// 在可视范围内
				if( pRole->IsInVisDist(pUnit->GetID()) )
					continue;

				// 不在可视范围内
				listRemove.push_back(pUnit->GetID());

				pRole->RemoveFromVisList(pUnit->GetID());

				continue;
			}

			// 4.不在A的可视列表中
			// 4.1不在A可视范围内
			if( !pRole->IsInVisDist(pUnit->GetID()) )
				continue;

			// 4.2 在A可视范围内
			DWORD dw_size = calculate_movement_message(pUnit, byMsg, 1024);
			if(dw_size == 0)
				continue;

			pRole->SendMessage(byMsg, dw_size);

			pRole->Add2VisList(pUnit->GetID());
		}
	}

	// 如果list不为空，且self是玩家的话，则发送给该玩家要删除的远程玩家
	INT nListSize = listRemove.size();
	if( nListSize <= 0 )
		return;

	DWORD dw_size = sizeof(NET_SIS_remove_remote) + (nListSize-1)*sizeof(DWORD);
	CREATE_MSG(pSend, dw_size, NET_SIS_remove_remote);

	for(INT n = 0; n < nListSize; n++)
	{
		pSend->dw_role_id[n] = listRemove.pop_front();
	}

	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(p_message);
}

//--------------------------------------------------------------------------------------------
// 将可视地砖内所有可视单位及掉落的物品同步给玩家
//--------------------------------------------------------------------------------------------
VOID Map::syn_big_visible_tile_visible_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end])
{
	ASSERT( VALID_POINT(pRole) );

	BYTE byMsg[1024] = {0};
	DWORD dw_size = 0;	

	NET_SIS_synchronize_item send;

	// 遍历九宫格列表
	for(INT n = 0; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) )
			continue;

		// 同步非隐身状态下远端玩家
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRemoteRole = NULL;

		while( mapRole.find_next(pRemoteRole) )
		{
			if( pRemoteRole->IsInStateInvisible() )
				continue;

			dw_size = calculate_movement_message(pRemoteRole, byMsg, 1024);
			if( dw_size == 0 )
				continue;
			pRole->SendMessage(byMsg, dw_size);
		}

		package_map<DWORD, Role*>& map_leave = pVisTile[n]->map_leave_role;
		map_leave.reset_iterator();
		pRemoteRole = NULL;

		while( map_leave.find_next(pRemoteRole) )
		{
			if( pRemoteRole->IsInStateInvisible() )
				continue;

			dw_size = calculate_movement_message(pRemoteRole, byMsg, 1024);
			if( dw_size == 0 )
				continue;
			pRole->SendMessage(byMsg, dw_size);
		}

		// 同步非隐身状态下怪
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		mapCreature.reset_iterator();
		Creature* pCreature = NULL;

		while( mapCreature.find_next(pCreature) )
		{
			if( pCreature->IsInStateInvisible() )
				continue;

			dw_size = calculate_movement_message(pCreature, byMsg, 1024);
			if( dw_size == 0 )
				continue;
			pRole->SendMessage(byMsg, dw_size);
		}

		//同步地面物品到客户端
		package_map<INT64, tag_ground_item*>& mapGroundItem = pVisTile[n]->map_ground_item;
		mapGroundItem.reset_iterator();
		tag_ground_item* pGroundItem = NULL;

		while( mapGroundItem.find_next(pGroundItem) )
		{
			send.n64_serial	= pGroundItem->n64_id;
			send.dw_data_id	= pGroundItem->dw_type_id;
			send.n_num		= pGroundItem->n_num;

			send.dwPutDownUnitID = pGroundItem->dw_put_down_unit_id;
			send.dwOwnerID = pGroundItem->dw_owner_id;
			send.dwTeamID  = pGroundItem->dw_team_id;
			send.dwGroupID = pGroundItem->dw_group_id;
			send.vPos = pGroundItem->v_pos;
			send.bKilled = FALSE;
			if(pGroundItem->dw_type_id != TYPE_ID_MONEY && VALID_POINT(pGroundItem->p_item) )
			{
				send.byQuality = pGroundItem->p_item->GetQuality();
				if (MIsEquipment(pGroundItem->p_item->dw_data_id))
					send.byQuality = ((tagEquip*)pGroundItem->p_item)->GetQuality();
			}
			pRole->SendMessage(&send, send.dw_size);
		}
	}
}

//--------------------------------------------------------------------------------------------
// 将隐身单位同步给可视地砖内的可以看到他的玩家
//--------------------------------------------------------------------------------------------
VOID Map::syn_invisible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
										const LPVOID p_message, const DWORD dw_size)
{
	ASSERT(pUnit->IsInStateInvisible());
	
	NET_SIS_remove_remote sendRemove;
	sendRemove.dw_role_id[0] = pUnit->GetID();

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.是否完全可见
			if( pRole->CanSeeTargetEntirely(pUnit) )
			{
				pRole->SendMessage(p_message, dw_size);
				continue;
			}

			// 2.B是否有探隐能力
			if( !pRole->HasDectiveSkill() )
				continue;

			// 3.在B的可视列表中
			if( pRole->IsInVisList(pUnit->GetID()) )
			{
				// 不在B可视范围内
				if( !pRole->IsInVisDist(pUnit->GetID()) )
				{
					pRole->SendMessage(&sendRemove, sendRemove.dw_size);
					pRole->RemoveFromVisList(pUnit->GetID());
				}
				else
				{
					pRole->SendMessage(p_message, dw_size);
				}

				continue;
			}

			// 4.不在B的可视列表中
			// 4.1不在B可视范围内
			if( !pRole->IsInVisDist(pUnit->GetID()) )
				continue;

			// 4.2 在B可视范围内
			pRole->SendMessage(p_message, dw_size);

			pRole->Add2VisList(pUnit->GetID());
		}
	}
}

//--------------------------------------------------------------------------------------------
// 同步可视地砖内的移动
//--------------------------------------------------------------------------------------------
VOID Map::synchronize_movement_to_big_visible_tile(Unit* p_self_)
{
	if( !VALID_POINT(p_self_) ) return;

	BYTE	byMsg[1024] = {0};
	DWORD	dw_size = calculate_movement_message(p_self_, byMsg, 1024);

	if( dw_size == 0 ) return;

	tagVisTile* pVisTile[EUD_end] = {0};

	// 获得周围九宫格的可视地砖
	get_visible_tile(p_self_->GetVisTileIndex(), pVisTile);
	
	// 向可视地砖的人发送同步命令
	if( !p_self_->IsInStateInvisible() )	// A可见
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, pVisTile, byMsg, dw_size);
	}
	else	// A不可见
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, pVisTile, byMsg, dw_size);
	}

	// 将可视地砖内隐身单位同步给当前玩家
	if( p_self_->IsRole() )
		syn_big_visible_tile_invisible_unit_to_role(static_cast<Role*>(p_self_), pVisTile);
}

//-------------------------------------------------------------------------------------------------------------
// 改变地图可视地砖时同步
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_change_visible_tile(Unit* p_self_, INT n_old_visible_index_, INT n_new_visible_index_)
{
	// 先把玩家从当前的可视链表中删除
	remove_from_visible_tile(p_self_);

	// 取出因为切换地砖而删除和添加的地砖
	tagVisTile* pVisTileAdd[EUD_end] = {0};
	tagVisTile* pVisTileDec[EUD_end] = {0};

	get_visible_tile_add_and_dec(n_old_visible_index_, n_new_visible_index_, pVisTileAdd, pVisTileDec);

	// 首先针对删除的玩家和生物进行移除
	synchronize_remove_units(p_self_, pVisTileDec);

	// 再同步加入到视野中的玩家和生物
	synchronize_add_units(p_self_, pVisTileAdd);

	// 再把其加入到新的可视地砖中
	add_to_visible_tile(p_self_, n_new_visible_index_);
}

//-------------------------------------------------------------------------------------------------------------
// 通知各个远程玩家删除该对象，并且如果Self是玩家的话，将这些远程玩家从自己本地客户端删除
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_remove_units(Unit* p_self_, tagVisTile* p_visible_tile_dec_[EUD_end])
{
	ASSERT( VALID_POINT(p_self_) );

	BOOL bSelfInvisible = p_self_->IsInStateInvisible();

	NET_SIS_remove_remote send;
	send.dw_role_id[0] = p_self_->GetID();

	// 同步给九宫格内其他玩家
	if( p_self_->IsInStateInvisible() )
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_dec_, &send, send.dw_size);
	}
	else
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_dec_, &send, send.dw_size);
	}

	// 将九宫格内信息同步给该玩家
	if( p_self_->IsRole() )
	{
		Role *pRole = static_cast<Role*>(p_self_);

		syn_remove_big_visible_tile_unit_and_ground_item_to_role(pRole, p_visible_tile_dec_);
	}
}

//-------------------------------------------------------------------------------------------------------------
// 角色异常九宫格后，将其内信息同步给角色
//-------------------------------------------------------------------------------------------------------------
VOID Map::syn_remove_big_visible_tile_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTileDec[EUD_end])
{
	package_list<DWORD> listRemovedUnit;
	package_list<INT64> listGroundItem;
	
	// 遍历九宫格列表
	for(INT n = 0;  n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTileDec[n]) )
			continue;

		// 该可视地砖要被删除，则遍历该可视地砖中的人物列表
		package_map<DWORD, Role*>& mapRole = pVisTileDec[n]->map_role;
		mapRole.reset_iterator();
		Role* pRemoteRole = NULL;

		while( mapRole.find_next(pRemoteRole) )
		{
			// 将要删除的玩家加入到列表中
			if( !pRemoteRole->IsInStateInvisible() )
				listRemovedUnit.push_back(pRemoteRole->GetID());
			else if( pRole->IsInVisList(pRemoteRole->GetID()) )
			{
				listRemovedUnit.push_back(pRemoteRole->GetID());
				pRole->RemoveFromVisList(pRemoteRole->GetID());
			}
		}

		package_map<DWORD, Role*>& map_leave_role = pVisTileDec[n]->map_leave_role;
		map_leave_role.reset_iterator();
		pRemoteRole = NULL;
		while(map_leave_role.find_next(pRemoteRole))
		{
			// 将要删除的玩家加入到列表中
			if( !pRemoteRole->IsInStateInvisible() )
				listRemovedUnit.push_back(pRemoteRole->GetID());
			else if( pRole->IsInVisList(pRemoteRole->GetID()) )
			{
				listRemovedUnit.push_back(pRemoteRole->GetID());
				pRole->RemoveFromVisList(pRemoteRole->GetID());
			}
		}

		// 遍历该可视地砖中的非玩家列表
		package_map<DWORD, Creature*>& mapCreature = pVisTileDec[n]->map_creature;
		mapCreature.reset_iterator();
		Creature* pCreature = NULL;

		while( mapCreature.find_next(pCreature) )
		{
			// 将要删除的非玩家加入到列表中
			if( !pCreature->IsInStateInvisible() )
				listRemovedUnit.push_back(pCreature->GetID());
			else if( pRole->IsInVisList(pCreature->GetID()) )
			{
				listRemovedUnit.push_back(pCreature->GetID());
				pRole->RemoveFromVisList(pCreature->GetID());
			}
		}

		//遍历该可视地砖中的GroundItem列表
		package_map<INT64, tag_ground_item*>& mapGroundItem = pVisTileDec[n]->map_ground_item;
		mapGroundItem.reset_iterator();
		tag_ground_item* pGroundItem = NULL;
		while (mapGroundItem.find_next(pGroundItem))
		{
			//将要同步的GroundItem加入列表
			if( pGroundItem->is_valid())
			{
				listGroundItem.push_back(pGroundItem->n64_id);
			}
		}
	}

	// 如果list不为空，且self是玩家的话，则发送给该玩家要删除的远程玩家
	INT nListSize = listRemovedUnit.size();

	if( nListSize > 0 )
	{
		DWORD dw_size = sizeof(NET_SIS_remove_remote) + (nListSize-1)*sizeof(DWORD);
		CREATE_MSG(pSend, dw_size, NET_SIS_remove_remote);

		for(INT n = 0; n < nListSize; n++)
		{
			pSend->dw_role_id[n] = listRemovedUnit.pop_front();
		}

		if(VALID_POINT(pRole->GetSession()))
			pRole->GetSession()->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	//如果list不为空,且self是玩家的话，则一次将GroundItem发送给该玩家要删除的GroundItem
	INT nSize = listGroundItem.size();
	if (nSize > 0)
	{
		DWORD dw_size = sizeof(NET_SIS_role_ground_item_disappear) + (nSize - 1)*sizeof(INT64);
		CREATE_MSG(pSend, dw_size, NET_SIS_role_ground_item_disappear);

		for (INT n = 0; n < nSize; n++)
		{
			pSend->n64_serial[n] = listGroundItem.pop_front();
		}

		if(VALID_POINT(pRole->GetSession()))
			pRole->GetSession()->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
}

//-------------------------------------------------------------------------------------------------------------
// 通知各个远程玩家删除该对象，并且如果Self是玩家的话，将这些远程玩家从自己本地客户端删除
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_remove_unit_to_big_visible_tile(Unit* p_self_)
{
	INT nVisIndex = world_position_to_visible_index(p_self_->GetMoveData().m_vPos);

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	synchronize_remove_units(p_self_, pVisTile);
}

//-------------------------------------------------------------------------------------------------------
// 同步加入到地砖中的玩家（动作）
//-------------------------------------------------------------------------------------------------------
VOID Map::synchronize_add_units(Unit* p_self_, tagVisTile* p_visible_tile_add_[EUD_end])
{
	ASSERT( VALID_POINT(p_self_) );

	BOOL bSelfInvisible = p_self_->IsInStateInvisible();

	BYTE byMsg[1024] = {0};
	DWORD dw_size = calculate_movement_message(p_self_, byMsg, 1024);
	if(dw_size == 0)
		return;

	// 将A同步给格子内玩家
	if( !p_self_->IsInStateInvisible() )	// A可见
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_add_, byMsg, dw_size);
	}
	else	// A不可见
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_add_, byMsg, dw_size);
	}

	// 若A为玩家，则将格子内信息同步给他
	if( p_self_->IsRole() )
	{
		Role *pRole = static_cast<Role*>(p_self_);

		// 格子内非隐身单位及掉落
		syn_big_visible_tile_visible_unit_and_ground_item_to_role(pRole, p_visible_tile_add_);

		// 格子内隐身单位
		syn_big_visible_tile_invisible_unit_to_role(pRole, p_visible_tile_add_);
	}
}

//----------------------------------------------------------------------------
// 计算两个玩家是否在同一个vistile里
//----------------------------------------------------------------------------
BOOL Map::in_same_big_visible_tile(Unit* p_self_, Unit* p_remote_)
{
	if( !VALID_POINT(p_self_) || !VALID_POINT(p_remote_) )
	{
		return FALSE;
	}

	INT nSelfVisIndex = p_self_->GetVisTileIndex();
	INT nRemoteVisIndex = p_remote_->GetVisTileIndex();

	if( INVALID_VALUE == nSelfVisIndex || INVALID_VALUE == nRemoteVisIndex )
	{
		return FALSE;
	}

	INT nMinus = abs(nSelfVisIndex - nRemoteVisIndex);

	if( ( nMinus >= n_visible_tile_array_width - 1 && nMinus <= n_visible_tile_array_width + 1 )
		|| 0 == nMinus
		|| 1 == nMinus
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//----------------------------------------------------------------------------
// 通过人物的当前动作计算发送给客户端的同步消息
//----------------------------------------------------------------------------
DWORD Map::calculate_movement_message(Unit* pSelf, LPBYTE p_message, DWORD dw_size)
{
	ASSERT( VALID_POINT(pSelf) && VALID_POINT(p_message) );

	// 根据人物当前移动状态发送同步命令
	EMoveState eCurMove = pSelf->GetMoveState();
	DWORD dwRealSize = 0;

	switch (eCurMove)
	{
		// 站立
	case EMS_Stand:
		{
			NET_SIS_synchronize_stand* pSend = (NET_SIS_synchronize_stand*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_stand");
			pSend->dw_size = sizeof(NET_SIS_synchronize_stand);
			pSend->dw_role_id = pSelf->GetID();
			pSend->curPos = pSelf->GetCurPos();
			pSend->faceTo = pSelf->GetFaceTo();

			dwRealSize = pSend->dw_size;
		}
		break;

		// 行走
	case EMS_Walk:
		{
			ASSERT( pSelf->IsRole() );

			NET_SIS_synchronize_walk* pSend = (NET_SIS_synchronize_walk*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_walk");
			pSend->dw_size = sizeof(NET_SIS_synchronize_walk);
			pSend->dw_role_id = pSelf->GetID();
			pSend->srcPos = pSelf->GetStartPos();
			pSend->dstPos = pSelf->GetDestPos();
			pSend->curTime = pSelf->GetMovePassTime();
			pSend->fXZSpeed = pSelf->GetXZSpeed();
			pSend->bCollide = true;

			dwRealSize = pSend->dw_size;
		}
		break;

		// 击飞
	case EMS_HitFly:
		{
			NET_SIS_hit_fly* pSend = (NET_SIS_hit_fly*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_hit_fly");
			pSend->dw_size = sizeof(NET_SIS_hit_fly);
			pSend->dw_role_id = pSelf->GetID();
			pSend->curPos = pSelf->GetCurPos();

			dwRealSize = pSend->dw_size;
		}


		// 怪物巡逻
	case EMS_CreaturePatrol:
		{
			ASSERT( pSelf->IsCreature() );

			NET_SIS_synchronize_walk* pSend = (NET_SIS_synchronize_walk*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_walk");
			pSend->dw_size = sizeof(NET_SIS_synchronize_walk);
			pSend->dw_role_id = pSelf->GetID();
			pSend->srcPos = pSelf->GetStartPos();
			pSend->dstPos = pSelf->GetDestPos();
			pSend->curTime = pSelf->GetMovePassTime();
			pSend->fXZSpeed = pSelf->GetXZSpeed() * 0.4f;	// 巡逻速度
			pSend->bCollide = static_cast<Creature*>(pSelf)->NeedCollide();

			dwRealSize = pSend->dw_size;
		}
		break;

		// 怪物行走
	case EMS_CreatureWalk:
		{
			ASSERT( pSelf->IsCreature() );

			NET_SIS_synchronize_walk* pSend = (NET_SIS_synchronize_walk*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_walk");
			pSend->dw_size = sizeof(NET_SIS_synchronize_walk);
			pSend->dw_role_id = pSelf->GetID();
			pSend->srcPos = pSelf->GetStartPos();
			pSend->dstPos = pSelf->GetDestPos();
			pSend->curTime = pSelf->GetMovePassTime();
			pSend->fXZSpeed = pSelf->GetXZSpeed();
			pSend->bCollide = static_cast<Creature*>(pSelf)->NeedCollide();

			dwRealSize = pSend->dw_size;
		}
		break;

		// 怪物逃跑
	case EMS_CreatureFlee:
		{
			ASSERT( pSelf->IsCreature() );

			NET_SIS_synchronize_walk* pSend = (NET_SIS_synchronize_walk*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_walk");
			pSend->dw_size = sizeof(NET_SIS_synchronize_walk);
			pSend->dw_role_id = pSelf->GetID();
			pSend->srcPos = pSelf->GetStartPos();
			pSend->dstPos = pSelf->GetDestPos();
			pSend->curTime = pSelf->GetMovePassTime();
			pSend->fXZSpeed = pSelf->GetXZSpeed() * 0.7;
			pSend->bCollide = static_cast<Creature*>(pSelf)->NeedCollide();

			dwRealSize = pSend->dw_size;
		}
		// 怪物跑步
	case EMS_CreatureRun:
		{
			ASSERT( pSelf->IsCreature() );

			NET_SIS_synchronize_walk* pSend = (NET_SIS_synchronize_walk*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_synchronize_walk");
			pSend->dw_size = sizeof(NET_SIS_synchronize_walk);
			pSend->dw_role_id = pSelf->GetID();
			pSend->srcPos = pSelf->GetStartPos();
			pSend->dstPos = pSelf->GetDestPos();
			pSend->curTime = pSelf->GetMovePassTime();
			pSend->fXZSpeed = pSelf->GetXZSpeed() * 1.5;
			pSend->bCollide = static_cast<Creature*>(pSelf)->NeedCollide();

			dwRealSize = pSend->dw_size;
		}
	default:
		break;
	}

	if( dwRealSize > dw_size )
		dwRealSize = 0;

	return dwRealSize;
}

//------------------------------------------------------------------------------
// 检测玩家所在位置对应的区域
//------------------------------------------------------------------------------
INT64 Map::check_area(Role* p_role_)
{
	if( !VALID_POINT(p_role_) ) return 0;
	if( !VALID_POINT(p_map_info) ) return 0;

	INT64 n64Ret = 0;

	// 得到玩家的包裹盒和格子坐标
	//AABBox roleBox = p_role_->GetAABBox();
	//INT nTileX = INT(p_role_->GetCurPos().x / (FLOAT)TILE_SCALE);
	//INT nTileZ = INT(p_role_->GetCurPos().z / (FLOAT)TILE_SCALE);
	
	float x = p_role_->GetCurPos().x;
	float z = p_role_->GetCurPos().z;

	tag_map_area_info* pArea = NULL;

	// 首先检测安全区
	pArea = is_in_area(p_map_info->map_safe_area, x, z);
	if( VALID_POINT(pArea) )
	{
		n64Ret |= ERS_SafeArea;

		//const tagEquip* pRaid = p_role_->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
		//if (!VALID_POINT(pRaid))
		//{
		//	// 下马
		//	p_role_->StopMount();
		//}
		
// 		//进入安全区自动设回和平模式
// 		if(p_role_->GetPKState() != ERolePK_Peace)
// 		{
// 			p_role_->SetPKState(ERolePK_Peace);
// 			NET_SIS_change_pk_state send;
// 			send.dw_role_id = p_role_->GetID();
// 			send.ePKState = p_role_->GetPKState();
// 			send.dwError = E_Success;
// 			send_big_visible_tile_message(p_role_, &send, send.dw_size);
// 		}
	}

	// 再检测PVP区
	pArea = is_in_area(p_map_info->map_pvp_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_PVPArea;

	// 再检测摆摊区
	pArea = is_in_area(p_map_info->map_stall_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_StallArea;

	// 再检牢狱区
	pArea = is_in_area(p_map_info->map_prison_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_PrisonArea;

	// 检测脚本区
	pArea = is_in_area(p_map_info->map_script_area, x, z);
	if( VALID_POINT(pArea) ) on_enter_area(p_role_, pArea);

	// 检测普通区
	pArea = is_in_area(p_map_info->map_common_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_CommonArea;

	// 检测绝对安全区
	pArea = is_in_area(p_map_info->map_real_safe_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_RealSafeArea;

	// 检测帮会战区
	pArea = is_in_area(p_map_info->map_guild_battle, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_GuildBattle;

	// 检测无惩罚区
	pArea = is_in_area(p_map_info->map_no_punish_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_NoPunishArea;

	// 检测挂机区
	pArea = is_in_area(p_map_info->map_hang_area, x, z);
	if(VALID_POINT(pArea))	n64Ret |= ERS_HangArea;

	pArea = is_in_area(p_map_info->map_KongFu_area, x, z);
	if(VALID_POINT(pArea))	n64Ret |= ERS_KongFuArea;


	pArea = is_in_area(p_map_info->map_Comprehend_area, x, z);
	if(VALID_POINT(pArea))	n64Ret |= ERS_ComprehendArea;

	pArea = is_in_area(p_map_info->map_Dancing_area, x, z);
	if(VALID_POINT(pArea))	
	{
		n64Ret |= ERS_DancingArea;
		const tagEquip* pRaid = p_role_->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
		if (!VALID_POINT(pRaid))
		{
			// 下马
			p_role_->StopMount();
		}
	}

	return n64Ret;
}

//------------------------------------------------------------------------------
// 检测玩家是不是在某个触发器范围内
//------------------------------------------------------------------------------
BOOL Map::is_in_trigger(Role* p_role_, DWORD dw_map_trigger_id_)
{
	if( !VALID_POINT(p_role_) ) return FALSE;
	if( !VALID_POINT(p_map_info) ) return FALSE;

	tag_map_trigger_info* pTrigger = p_map_info->map_trigger.find(dw_map_trigger_id_);
	if( !VALID_POINT(pTrigger) ) return FALSE;

	//Vector3 box_center;
	////box_center.x = pTrigger->boxPos.x + pTrigger->boxSize.x * 0.5;
	//box_center.z = pTrigger->boxPos.z + pTrigger->boxSize.z * 0.5;
	//box_center.y = 0;
	
	Vector3 vecLeftTop;
	vecLeftTop.x = max (pTrigger->boxPos.x - 300, 0);
	vecLeftTop.z = pTrigger->boxPos.z + 300;

	if (p_role_->GetCurPos().x > vecLeftTop.x && 
		p_role_->GetCurPos().x < (vecLeftTop.x + pTrigger->boxSize.x + 600) &&
		p_role_->GetCurPos().z < vecLeftTop.z &&
		p_role_->GetCurPos().z > max(vecLeftTop.z - pTrigger->boxSize.x - 600, 0))
	{
		return true;
	}
	//// 得到玩家的包裹盒和格子坐标
	//AABBox roleBox = p_role_->GetAABBox();
	//INT nTileX = INT(p_role_->GetCurPos().x / (FLOAT)TILE_SCALE);
	//INT nTileZ = INT(p_role_->GetCurPos().z / (FLOAT)TILE_SCALE);

	//Vector3 box_center = pTrigger->box.GetCenter();

	//FLOAT fDist = Vec3DistSq(p_role_->GetCurPos(), box_center);

	//if(get_map_type() == EMT_Normal)
	//{
	//	if(fDist >= 240 * 240) return FALSE;
	//}
	//else
	//{
	//	if( !pTrigger->box.Intersects(roleBox) ) return FALSE;
	//	if( !PtInRegion(pTrigger->h_polygon, nTileX, nTileZ) ) return FALSE;
	//}

	return false;
}

//------------------------------------------------------------------------------
// 得到某个trigger对应的任务序列号
//------------------------------------------------------------------------------
DWORD Map::get_trigger_quest_serial(DWORD dw_map_trigger_id_)
{
	if( !VALID_POINT(p_map_info) ) return INVALID_VALUE;

	tag_map_trigger_info* pTrigger = p_map_info->map_trigger.find(dw_map_trigger_id_);
	if( !VALID_POINT(pTrigger) ) return INVALID_VALUE;

	if( pTrigger->e_type != EMT_Script ) return INVALID_VALUE;

	return pTrigger->dw_quest_id;
}

//------------------------------------------------------------------------------
// 检测某个包裹盒和格子坐标是否在某个区域列表中的一个区域里
//------------------------------------------------------------------------------
tag_map_area_info* Map::is_in_area(tag_map_info::MAP_AREA& map_area_, float x, float z)
{
	tag_map_info::MAP_AREA::map_iter it = map_area_.begin();
	tag_map_area_info* pArea = NULL;

	while( map_area_.find_next(it, pArea) )
	{
		//// 跟包裹盒都不相交
		//if( !pArea->box.Intersects(role_box_) ) continue;

		//// 包裹盒相交，则检测格子坐标在不在区域里面
		//if( PtInRegion(pArea->h_polygon, n_tile_x_, n_tile_z_) )
		//	return pArea;
		
		if (x > pArea->boxPos.x && x < pArea->boxPos.x + pArea->boxSize.x &&
			z > pArea->boxPos.z && z < pArea->boxPos.z + pArea->boxSize.z)
		{
			return pArea;
		}
	}

	return NULL;
}

//------------------------------------------------------------------------------
// 将玩家添加到登出列表中，由上层来控制玩家的实际登出和保存
//------------------------------------------------------------------------------
VOID Map::role_logout(DWORD dw_id_)
{
	Role* pRole = map_role.find(dw_id_);
	if( VALID_POINT(pRole) )
	{
		pRole->removeFlowCreature();
		// 离开地图
		role_leave_map(pRole->GetID(), TRUE);
	}
}

//------------------------------------------------------------------------------
// 将玩家添加到登出列表中，由上层来控制玩家的实际登出和保存
//------------------------------------------------------------------------------
VOID Map::role_leave_logout(DWORD	dw_id_)
{
	Role* pRole = map_leave_role.find(dw_id_);
	if(VALID_POINT(pRole))
	{
		pRole->set_leave_prictice(FALSE);
		role_leave_map_practice(dw_id_);
	}
}

VOID Map::role_leave_map_practice(DWORD dw_id_)
{
	Role* pRole = map_leave_role.find(dw_id_);
	if( VALID_POINT(pRole) )
	{
		//处理离开地图前的相关事项
		pRole->OnLeaveMap();

		// 调用脚本
		if( VALID_POINT(p_script) )
		{
			p_script->OnPlayerLeave(pRole, this);
		}

		// 同步周围玩家删除自己
		synchronize_role_leave_map(pRole);

		// 将玩家从自己的列表中删除
		map_role.erase(dw_id_);

		// 添加离线修炼逻辑
		if(pRole->is_leave_pricitice())
		{
			map_leave_role.add(dw_id_, pRole);
		}
		else
		{
			map_leave_role.erase(dw_id_);
			pRole->SetMap(NULL);
		}

		// 如果玩家session有效，则现在就从列表中删除
		if( pRole->GetSession() )
		{
			map_session.erase(pRole->GetSession()->GetSessionID());
		}
	}
}

//-------------------------------------------------------------------------------
// 玩家离开地图，只可能在主线程里面调用
//-------------------------------------------------------------------------------
VOID Map::role_leave_map(DWORD dw_id_, BOOL b_leave_online)
{
	Role* pRole = map_role.find(dw_id_);
	if( VALID_POINT(pRole) )
	{
		//处理离开地图前的相关事项
		pRole->OnLeaveMap();
		
		
		// 调用脚本
		if( VALID_POINT(p_script) )
		{
			p_script->OnPlayerLeave(pRole, this);
		}

		// 同步周围玩家删除自己
		synchronize_role_leave_map(pRole, b_leave_online);
		
		//set_have_unit(pRole->GetCurPos(), false);

		// 将玩家从自己的列表中删除
		map_role.erase(dw_id_);

		// 添加离线修炼逻辑
		if(pRole->is_leave_pricitice() && pRole->IsInRoleState(ERS_Prictice) && b_leave_online)
		{
			map_leave_role.add(dw_id_, pRole);
		}
		else
		{
			map_leave_role.erase(dw_id_);
			pRole->SetMap(NULL);
		}
		
		// 如果玩家session有效，则现在就从列表中删除
		if( pRole->GetSession() )
		{
			map_session.erase(pRole->GetSession()->GetSessionID());
		}
	}
}

//-------------------------------------------------------------------------------
// 在地图中根据某个ID查找玩家或者生物（慎用，一般要在该地图线程之内使用）
//-------------------------------------------------------------------------------
Unit* Map::find_unit(DWORD dw_id_)
{
	Unit* pUnit = NULL;
	if( IS_PLAYER(dw_id_) )
		pUnit = map_role.find(dw_id_);
	else if( IS_CREATURE(dw_id_) )
	{
		pUnit = map_creature.find(dw_id_);
		if (!VALID_POINT(pUnit))
		{
			pUnit = map_respawn_creature.find(dw_id_);
		}
	}
	else if( IS_PET(dw_id_) )
		pUnit = map_creature.find(dw_id_);
	

	return pUnit;
}

//-------------------------------------------------------------------------------
// 创建并初始化NPC上所挂商店
//-------------------------------------------------------------------------------
BOOL Map::add_shop(DWORD dw_npc_id_, DWORD dw_shop_id_)
{
	if(map_shop.is_exist(dw_npc_id_))
	{
		return TRUE;
	}

	Shop* pShop = Shop::Create(dw_npc_id_, dw_shop_id_);
	if(!VALID_POINT(pShop))
	{
		ASSERT(VALID_POINT(pShop));
		print_message(_T("create shop faild npc<%u> shopid<%u>!\n"), dw_npc_id_, dw_shop_id_);
		return FALSE;
	}

	map_shop.add(dw_npc_id_, pShop);

	return TRUE;
}

//-------------------------------------------------------------------------------
// 创建并初始化NPC上所挂商会
//-------------------------------------------------------------------------------
BOOL Map::add_chamber(DWORD dw_npc_id_, DWORD dw_chamber_id_)
{
	if(map_chamber.is_exist(dw_npc_id_))
	{
		return TRUE;
	}

	guild_chamber* pCofC = guild_chamber::create_chamber(dw_npc_id_, dw_chamber_id_);
	if(!VALID_POINT(pCofC))
	{
		ASSERT(VALID_POINT(pCofC));
		print_message(_T("create chamber faild npc<%u> shopid<%u>!\n"), dw_npc_id_, dw_chamber_id_);
		return FALSE;
	}

	map_chamber.add(dw_npc_id_, pCofC);

	return TRUE;
}

//-------------------------------------------------------------------------------
// 玩家离开地图同步
//-------------------------------------------------------------------------------
VOID Map::synchronize_role_leave_map(Role* p_self_, BOOL b_leave_online)
{
	if( !VALID_POINT(p_self_) ) return;

	INT nVisTileIndex = p_self_->GetVisTileIndex();

	// 首先从VisTile中移除
	remove_from_visible_tile(p_self_, b_leave_online);

	// 添加离线修炼逻辑
	if(!p_self_->is_leave_pricitice())
	{
		// 得到九宫格的有玩家
		tagVisTile* pVisTile[EUD_end] = {0};
		get_visible_tile(nVisTileIndex, pVisTile);

		// 发送移除同步命令
		synchronize_remove_units(p_self_, pVisTile);
	}
	
}

//----------------------------------------------------------------------------------
// 发送进入新地图给客户端
//----------------------------------------------------------------------------------
VOID Map::send_goto_new_map_to_player(Role* p_self_)
{
	if( !VALID_POINT(p_self_) || !VALID_POINT(p_self_->GetSession()) )
		return;

	NET_SIS_goto_new_map send;
	send.dwMapID = p_self_->GetMapID();
	//send.nX = (int)p_self_->GetCurPos().x;
	//send.nY = (int)p_self_->GetCurPos().z;
	//send.nDirX = (int)p_self_->GetFaceTo().x;
	//send.nDirY = (int)p_self_->GetFaceTo().z;
	send.pos = p_self_->GetCurPos();
	send.faceTo = p_self_->GetFaceTo();

	p_self_->SendMessage(&send, send.dw_size);
}

//----------------------------------------------------------------------------------
// 怪物掉落物品是否到地上 --- 地面物品系统 
//----------------------------------------------------------------------------------
BOOL Map::can_putdown(Unit* p_unit, INT n_index_, Vector3& v_position_)
{
	if( !VALID_POINT(p_unit) )
		return FALSE;

	// 根据怪物算物品坐标
	const Vector3& vCreaturePos = p_unit->GetCurPos();

	// 得到格子序号
	INT nIndexX = INT(vCreaturePos.x / TILE_SCALE);
	INT nIndexZ = INT(vCreaturePos.z / TILE_SCALE);

	// 找出一个可行走区域
	if( FALSE == get_can_go_position_from_index(n_index_, nIndexX, nIndexZ, (n_index_ / 8 + 1), v_position_) )
	{
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------------
// 怪物掉落物品到地上 --- 地面物品系统 
//----------------------------------------------------------------------------------
VOID Map::putdown_item(Creature* p_creature_, tagItem* p_loot_item_, DWORD dw_owner_id_, DWORD dw_team_id_, 
					   Vector3& v_position_, EPickMode e_pick_mode_, EGTAssignState e_assign_sate_, INT n_tick_down_)
{
	if( !VALID_POINT(p_creature_) || !VALID_POINT(p_loot_item_) )
		return;

	DWORD dw_boss_id = 0;
	if(VALID_POINT(p_creature_->GetProto()) && p_creature_->GetProto()->is_boss())
		dw_boss_id = p_creature_->GetProto()->dw_data_id;


	// 取怪物头顶高度
	v_position_.y = p_creature_->GetCurPosTop().y;

	// 生成掉落物品
	tag_ground_item* pGroundItem = new tag_ground_item(get_ground_item_id(), p_loot_item_->dw_data_id, 
														p_loot_item_->n16Num, p_loot_item_, v_position_,
														dw_owner_id_, dw_team_id_, 0, p_creature_->GetID(),
														e_pick_mode_, e_assign_sate_, n_tick_down_, dw_boss_id);
	ASSERT(pGroundItem->is_valid());

	// 在地面上添加物品
	add_ground_item(pGroundItem);
}

//----------------------------------------------------------------------------------
// 怪物掉落金钱到地上 --- 地面物品系统 
//----------------------------------------------------------------------------------
VOID Map::putdown_money(Creature* p_creature_, INT n_money_, DWORD dwTeamID, DWORD dwOwnerID, Vector3 v_positon_)
{
	if( !VALID_POINT(p_creature_)/* || !VALID_POINT(p_role_)*/ )
		return;

	//DWORD dwTeamID = p_role_->GetTeamID();
	//DWORD dwOwnerID = VALID_VALUE(dwTeamID) ? INVALID_VALUE : p_role_->GetID();

	// 取怪物头顶高度
	v_positon_.y = p_creature_->GetCurPosTop().y;

	// 生成地面物品
	tag_ground_item* pGroundItem = new tag_ground_item(get_ground_item_id(), TYPE_ID_MONEY, n_money_, NULL,
													v_positon_, dwOwnerID, dwTeamID, 0, p_creature_->GetID());
	ASSERT(pGroundItem->is_valid());

	// 往地面上增加物品
	add_ground_item(pGroundItem);
}

BOOL Map::get_can_go_position_from_index(INT32 n_index_, INT32 n_index_x_, INT32 n_index_z_, INT32 n_, Vector3& v_position_)
{
	INT index = n_index_ % 8;

	for (int i = 0; i < n_ * 8; i++)
	{

	}
	switch (index)
	{
	case 0:
		n_index_x_ -= n_;
		n_index_z_ -= n_;
		break;
	case 1:
		n_index_z_ -= n_;
		break;
	case 2:
		n_index_x_ += n_;
		n_index_z_ -= n_;
		break;
	case 3:
		n_index_x_ += n_;
		break;
	case 4:
		n_index_x_ += n_;
		n_index_z_ += n_;
		break;
	case 5:
		n_index_z_ += n_;
		break;
	case 6:
		n_index_x_ -= n_;
		n_index_z_ += n_;
		break;
	case 7:
		n_index_x_ -= n_;
		break;
	}

	v_position_.x = (FLOAT)(n_index_x_ * TILE_SCALE) + TILE_SCALE/2;
	v_position_.z = (FLOAT)(n_index_z_ * TILE_SCALE) + TILE_SCALE/2;

	if( n_index_x_ < 0 || n_index_x_ > p_map_info->n_width - 1 ||
		n_index_z_ < 0 || n_index_z_ > p_map_info->n_height - 1 )
	{
		return FALSE;
	}

	if (!if_can_go(v_position_.x, v_position_.z))
	{
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 往地面上增加物品 --- 地面物品系统
//-----------------------------------------------------------------------------
VOID Map::add_ground_item(tag_ground_item *p_ground_item_)
{
	if( !VALID_POINT(p_ground_item_) ) return;
	if( !p_ground_item_->is_valid() ) return;


	p_ground_item_->v_pos.y = 0;

	// 加入到m_mapGroundItem中
	map_ground_item.add(p_ground_item_->n64_id, p_ground_item_);

	// 计算物品所在的可视地砖格子
	INT nVisIndex = world_position_to_visible_index(p_ground_item_->v_pos);

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// 同步给加入到客户端的玩家和生物
	synchronize_add_ground_item(p_ground_item_, pVisTile);

	// 让生物落到9宫格之内
	add_ground_item_to_visible_tile(p_ground_item_, nVisIndex);

}
//-----------------------------------------------------------------------------
// 从地面上移除物品 --- 地面物品系统
//-----------------------------------------------------------------------------
VOID Map::remove_ground_item(tag_ground_item* p_ground_item_)
{
	if(!VALID_POINT(p_ground_item_))
		return;

	if (!p_ground_item_->is_valid())
		return;

	//从m_mapGroundItem 中移除
	map_ground_item.erase(p_ground_item_->n64_id);
	//计算物品所在的可视地砖格子
	INT nVisIndex = world_position_to_visible_index(p_ground_item_->v_pos);

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// 同步给加入到客户端的玩家和生物
	synchronize_remove_ground_item(p_ground_item_, pVisTile);

	// 让生物落到9宫格之内
	remove_ground_item_from_visible_tile(p_ground_item_, nVisIndex);

}

//-----------------------------------------------------------------------------
// 同步加入到地砖中的物品 --- 地面物品系统
//-----------------------------------------------------------------------------
VOID Map::synchronize_add_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end])
{
	NET_SIS_synchronize_item send;

	send.n64_serial	= p_ground_item_->n64_id;
	send.dw_data_id	= p_ground_item_->dw_type_id;
	send.n_num		= p_ground_item_->n_num;

	send.dwPutDownUnitID = p_ground_item_->dw_put_down_unit_id;
	send.dwOwnerID = p_ground_item_->dw_owner_id;
	send.dwTeamID = p_ground_item_->dw_team_id;
	send.dwGroupID = p_ground_item_->dw_group_id;
	send.vPos = p_ground_item_->v_pos;
	send.bKilled = TRUE;
	if(p_ground_item_->dw_type_id != TYPE_ID_MONEY && VALID_POINT(p_ground_item_->p_item) )
	{
		send.byQuality = p_ground_item_->p_item->GetQuality();
		if (MIsEquipment(p_ground_item_->p_item->dw_data_id))
			send.byQuality = ((tagEquip*)p_ground_item_->p_item)->GetQuality();
	}
	// 向可视地砖的人发送同步命令
	send_big_visible_tile_message(p_visible_tile_add_, &send, send.dw_size);
}

//-----------------------------------------------------------------------------
// 同步加入到地砖中的物品 --- 地面物品系统
//-----------------------------------------------------------------------------
VOID Map::synchronize_ground_item_state(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end])
{
	NET_SIS_synchronize_item send;

	send.n64_serial	= p_ground_item_->n64_id;
	send.dw_data_id	= p_ground_item_->dw_type_id;
	send.n_num		= p_ground_item_->n_num;

	send.dwPutDownUnitID = p_ground_item_->dw_put_down_unit_id;
	send.dwOwnerID = p_ground_item_->dw_owner_id;
	send.dwTeamID = p_ground_item_->dw_team_id;
	send.dwGroupID = p_ground_item_->dw_group_id;
	send.vPos = p_ground_item_->v_pos;
	send.bKilled = FALSE;
	if(p_ground_item_->dw_type_id != TYPE_ID_MONEY && VALID_POINT(p_ground_item_->p_item) )
	{
		send.byQuality = p_ground_item_->p_item->GetQuality();
		if (MIsEquipment(p_ground_item_->p_item->dw_data_id))
			send.byQuality = ((tagEquip*)p_ground_item_->p_item)->GetQuality();
	}

	// 向可视地砖的人发送同步命令
	send_big_visible_tile_message(p_visible_tile_add_, &send, send.dw_size);
}


//-----------------------------------------------------------------------------
// 同步移除地砖中的物品 --- 地面物品系统
//-----------------------------------------------------------------------------
VOID Map::synchronize_remove_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end])
{
	if(!p_ground_item_->is_valid())
		return;

	//发送物品消失消息
	NET_SIS_role_ground_item_disappear disappear;
	disappear.n64_serial[0] = p_ground_item_->n64_id;

	// 向可视地砖的人发送同步命令
	send_big_visible_tile_message(p_visible_tile_add_, &disappear, disappear.dw_size);
}

//-----------------------------------------------------------------------------
// 发送地图场景特效信息	---	玩家进入场景调用
//-----------------------------------------------------------------------------
VOID Map::send_scene_effect(Role* p_self_)
{
	if (!VALID_POINT(p_self_))	return;

	// 向该客户端发送所有已经激活的场景特效
	h_mutex.Acquire();
	DWORD dwObjID = INVALID_VALUE;
	package_list<DWORD>::list_iter iter = list_scene_effect.begin();
	while (list_scene_effect.find_next(iter, dwObjID))
	{
		if (!VALID_VALUE(dwObjID))
			continue;

		NET_SIS_play_scene_effect send;
		send.eType		= ESET_ByObjID;
		send.dwObjID	= dwObjID;
		p_self_->SendMessage(&send, send.dw_size);
	}
	h_mutex.Release();
}

//-----------------------------------------------------------------------------
// 开启场景特效	--- 脚本调用
//-----------------------------------------------------------------------------
VOID Map::play_scene_effect(ESceneEffectType e_type_, DWORD dw_obj_id_, Vector3 v_pos_, DWORD dw_effect_id_)
{
	// 检查场景特效属性
	if (!VALID_POINT(p_map_info))	return;
	if (!VALID_POINT(p_map_info->map_trigger_effect.find(dw_obj_id_)))
		return;

	// 设置特效激活
	h_mutex.Acquire();
	if (!list_scene_effect.is_exist(dw_obj_id_))
		list_scene_effect.push_back(dw_obj_id_);
	h_mutex.Release();

	// 通知地图中玩家开启场景特效(允许客户端激活多次)
	NET_SIS_play_scene_effect send;
	send.eType		= e_type_;
	send.dwObjID	= dw_obj_id_;
	send.vPos		= v_pos_;
	send.dwEffectID	= dw_effect_id_;
	send_map_message(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// 关闭场景特效	--- 脚本调用
//-----------------------------------------------------------------------------
VOID Map::stop_scene_effect(DWORD dw_obj_id_)
{
	// 检查是否已经激活
	h_mutex.Acquire();
	if (!list_scene_effect.is_exist(dw_obj_id_))
	{
		h_mutex.Release();
		return;
	}
	else
	{
		list_scene_effect.erase(dw_obj_id_);
	}
	h_mutex.Release();

	// 通知地图中所有玩家
	NET_SIS_stop_scene_effect send;
	send.dwObjID	= dw_obj_id_;
	send_map_message(&send, send.dw_size);
}

//-----------------------------------------------------------------------------------------
// 加入物品到地图中的某个VisTile --- 地面物品系统
//-----------------------------------------------------------------------------------------
VOID Map::add_ground_item_to_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_)
{
	p_visible_tile[n_visible_index_].map_ground_item.add(p_ground_item_->n64_id, p_ground_item_);
}

//-----------------------------------------------------------------------------------------
// 移除地图中某个VisTile的物品 --- 地面物品系统
//-----------------------------------------------------------------------------------------
VOID Map::remove_ground_item_from_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_)
{
	p_visible_tile[n_visible_index_].map_ground_item.erase(p_ground_item_->n64_id);
}

//-----------------------------------------------------------------------------------------
// 刷新点刷怪
//-----------------------------------------------------------------------------------------
VOID Map::spawn_creature_replace( Creature* p_dead_creature_ )
{
	ASSERT_P_VALID(p_dead_creature_);

	// 需要用到的数据
	DWORD	dwReuseID		= p_dead_creature_->GetID();
	DWORD	dwSSpawnPtID	= p_dead_creature_->GetSpawnPtID();
	DWORD	dwSpawnGroupID	= p_dead_creature_->GetSpawnGroupID();
	Vector3	vPos			= p_dead_creature_->GetBornPos();
	Vector3 vFase			= p_dead_creature_->GetBornFace();
	BOOL	bCollide		= p_dead_creature_->NeedCollide();

	// 找个新的
	const tagSSpawnPointProto* pSSpawnProto = AttRes::GetInstance()->GetSSpawnPointProto(dwSSpawnPtID);
	ASSERT_P_VALID( pSSpawnProto );
	if (!VALID_POINT(pSSpawnProto))
		return ;

	INT nCandiNum	= 0;
	while (VALID_VALUE(pSSpawnProto->dwTypeIDs[nCandiNum]))
		nCandiNum++;
	INT nIndex = get_tool()->tool_rand() % nCandiNum;
	DWORD dwNewTypeID = pSSpawnProto->dwTypeIDs[nIndex];
	ASSERT( VALID_VALUE(dwNewTypeID) );
	if (!VALID_POINT(dwNewTypeID))
		return ;
	const tagCreatureProto* pCreatureProto = AttRes::GetInstance()->GetCreatureProto(dwNewTypeID);


	// 删除旧的
	map_respawn_creature.erase(p_dead_creature_->GetID());
	Creature::Delete(p_dead_creature_);

	// 生成新的
	p_dead_creature_ = Creature::Create(dwReuseID, get_map_id(), this, pCreatureProto, vPos, vFase, ECST_SpawnPoint, dwSSpawnPtID, dwSpawnGroupID, bCollide, NULL);

	// 添加
	add_creature(p_dead_creature_);

	p_dead_creature_->OnScriptLoad();
}

//-----------------------------------------------------------------------------------------
// 刷新点组刷新
//-----------------------------------------------------------------------------------------
VOID Map::spawn_list_creature_replace(Creature* p_dead_creature_)
{
	tag_map_spawn_point_list* pMapSpawnList = p_map_info->map_spawn_point_list.find(p_dead_creature_->GetSpawnGroupID());
	if (!VALID_POINT(pMapSpawnList)) return;

	//DWORD dwSpawnID = 0;
	tag_map_spawn_point_info* pMapSpawnInfo = NULL;	
	//pMapSpawnList->list.rand_find(dwSpawnID, pMapSpawnInfo);
	RandSpwanPoint(pMapSpawnList, pMapSpawnInfo);\
	ASSERT(VALID_POINT(pMapSpawnInfo));
	if (!VALID_POINT(pMapSpawnInfo)) return;
	
	Vector3 vecPos = pMapSpawnInfo->v_pos;
	int nRand = 5 * TILE_SCALE;

	vecPos.x = get_tool()->rand_in_range(pMapSpawnInfo->v_pos.x - nRand, pMapSpawnInfo->v_pos.x + nRand);
	vecPos.z = get_tool()->rand_in_range(pMapSpawnInfo->v_pos.z - nRand, pMapSpawnInfo->v_pos.z + nRand);
	fix_position(vecPos);

	p_dead_creature_->SetBronPos(vecPos);
	p_dead_creature_->SetSpawnPtID(pMapSpawnInfo->dw_att_id);

	p_dead_creature_->OnRevive();
	// 重生成功
	map_respawn_creature.erase(p_dead_creature_->GetID());
	add_creature(p_dead_creature_);
	//pMapSpawnInfo->b_hasUnit = TRUE;
	map_point_has_list[pMapSpawnInfo->dw_att_id] = TRUE;

}
//-----------------------------------------------------------------------------------------
// 动态创建怪物
//-----------------------------------------------------------------------------------------
Creature* Map::create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, DWORD dw_spawner_id_, BOOL b_collide_, LPCTSTR p_patrol_list_name_)
{
	Creature *pCreature = (Creature*)INVALID_VALUE;

	const tagCreatureProto* pProto = AttRes::GetInstance()->GetCreatureProto(dw_data_id_);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Detect a unknown creature in map %s, dw_data_id=%u\r\n"), p_map_info->sz_map_name, dw_data_id_);
		return pCreature;
	}

	if (MoveData::IsPosInvalid(v_pos_) || MoveData::IsFaceInvalid(v_face_to_))
	{
		return pCreature;
	}

	if( !is_position_valid(v_pos_) )
	{
		print_message(_T("creature is out of map, dw_data_id=%u\r\n"), dw_data_id_);
		return pCreature;
	}

	tag_map_way_point_info_list* patrolList = NULL;
	if(VALID_POINT(p_patrol_list_name_))
		patrolList = p_map_info->map_way_point_list.find(get_tool()->crc32(p_patrol_list_name_));

	DWORD dwID = builder_creature_id.get_new_creature_id();

	pCreature = Creature::Create(dwID, dw_id, this, pProto, v_pos_, v_face_to_, ECST_Dynamic, dw_spawner_id_, INVALID_VALUE, b_collide_, patrolList);

	if(VALID_POINT(pCreature))
	{
		add_creature(pCreature);
		pCreature->OnScriptLoad();
	}
	return pCreature;
}

//-----------------------------------------------------------------------------------------
// 动态创建怪物
//-----------------------------------------------------------------------------------------
Creature* Map::create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, ECreatureSpawnedType e_spawned_type_, 
							   DWORD dw_guild_id_, DWORD dw_spawner_id_, BOOL b_collide_, 
							   TCHAR* p_patrol_list_name_)
{
	Creature *pCreature = (Creature*)INVALID_VALUE;

	const tagCreatureProto* pProto = AttRes::GetInstance()->GetCreatureProto(dw_data_id_);
	if( !VALID_POINT(pProto) )
	{
		print_message(_T("Detect a unknown creature in map %s, dw_data_id=%u\r\n"), p_map_info->sz_map_name, dw_data_id_);
		return pCreature;
	}

	if (MoveData::IsPosInvalid(v_pos_) || MoveData::IsFaceInvalid(v_face_to_))
	{
		return pCreature;
	}

	if( !is_position_valid(v_pos_) )
	{
		print_message(_T("creature is out of map, dw_data_id=%u\r\n"), dw_data_id_);
		return pCreature;
	}

	tag_map_way_point_info_list* patrolList = NULL;
	if(VALID_POINT(p_patrol_list_name_))
		patrolList = p_map_info->map_way_point_list.find(get_tool()->crc32(p_patrol_list_name_));

	DWORD dwID = builder_creature_id.get_new_creature_id();

	pCreature = Creature::Create(dwID, dw_id, this, pProto, v_pos_, v_face_to_, e_spawned_type_, dw_spawner_id_, INVALID_VALUE, b_collide_, patrolList, INVALID_VALUE, dw_guild_id_);

	if(VALID_POINT(pCreature))
	{
		add_creature(pCreature);
		pCreature->OnScriptLoad();
	}
	return pCreature;
}

//-----------------------------------------------------------------------------------------
// 删除怪物(存在map安全问题，不要用此函数lc)
//-----------------------------------------------------------------------------------------
VOID Map::delete_creature(DWORD dw_id_)
{
	Creature* pCreature = find_creature(dw_id_);
	if (!VALID_POINT(pCreature))	return;

	map_creature.erase(dw_id_);

	// 从可视地砖中拿走，但不同步客户端
	remove_from_visible_tile(pCreature);

	// 同步给客户端
	synchronize_remove_unit_to_big_visible_tile(pCreature);

	Creature::Delete(pCreature);
}

//------------------------------------------------------------------------------------------
// 潜行
//------------------------------------------------------------------------------------------
VOID Map::lurk(Unit *p_unit_)
{
	ASSERT(VALID_POINT(p_unit_));

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	if(nVisIndex == INVALID_VALUE)
		return;

	// 加入格子隐身单位列表
	p_visible_tile[nVisIndex].map_invisible_unit.add(p_unit_->GetID(), p_unit_);

	// 同步给可视列表内玩家
	NET_SIS_remove_remote sendRemove;
	sendRemove.dw_role_id[0] = p_unit_->GetID();

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);
	
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.是否完全可见
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			ASSERT(!pRole->IsInVisList(p_unit_->GetID()));

			// 2.B是否有探隐能力，且是否在可视范围内
			if( !pRole->HasDectiveSkill() || !pRole->IsInVisDist(p_unit_->GetID()) )
			{
				pRole->SendMessage(&sendRemove, sendRemove.dw_size);
				continue;
			}

			// 3 在B可视范围内
			pRole->Add2VisList(p_unit_->GetID());
		}
	}
}

//------------------------------------------------------------------------------------------
// 潜行状态消失 -- 显形
//------------------------------------------------------------------------------------------
VOID Map::unlurk(Unit *p_unit_)
{
	ASSERT(VALID_POINT(p_unit_));

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	// 从格子隐身单位列表中去除
	p_visible_tile[nVisIndex].map_invisible_unit.erase(p_unit_->GetID());

	// 同步给可视列表内玩家
	BYTE	byMsg[1024] = {0};
	DWORD	dw_size = calculate_movement_message(p_unit_, byMsg, 1024);

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.是否完全可见
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			// 2.B是否有探隐能力，且是否在可视范围内
			if( !pRole->HasDectiveSkill() || !pRole->IsInVisDist(p_unit_->GetID()) )
			{
				pRole->SendMessage(byMsg, dw_size);
				continue;
			}

			// 3 在B可视范围内
			pRole->RemoveFromVisList(p_unit_->GetID());
		}
	}
}

//------------------------------------------------------------------------------------------
// 潜行单位移动过程中同步给可视范围内玩家
//------------------------------------------------------------------------------------------
VOID Map::update_lurk_to_big_visible_tile_role(Unit *p_unit_)
{
	ASSERT(p_unit_->IsInStateInvisible());

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	NET_SIS_remove_remote sendRemove;
	sendRemove.dw_role_id[0] = p_unit_->GetID();

	BYTE	byMsg[1024] = {0};
	DWORD	dw_size = calculate_movement_message(p_unit_, byMsg, 1024);

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// 找到每个地砖的人
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.是否完全可见
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			// 2.B是否有探隐能力
			if( !pRole->HasDectiveSkill() )
				continue;

			// 3.在B的可视列表中
			if( pRole->IsInVisList(p_unit_->GetID()) )
			{
				// 不在B可视范围内
				if( !pRole->IsInVisDist(p_unit_->GetID()) )
				{
					pRole->SendMessage(&sendRemove, sendRemove.dw_size);
					pRole->RemoveFromVisList(p_unit_->GetID());
				}

				continue;
			}

			// 4.不在B的可视列表中
			// 4.1不在B可视范围内
			if( !pRole->IsInVisDist(p_unit_->GetID()) )
				continue;

			// 4.2 在B可视范围内
			pRole->SendMessage(byMsg, dw_size);
			pRole->Add2VisList(p_unit_->GetID());
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
// 将周围潜行单位同步给该玩家
//---------------------------------------------------------------------------------------------------------------------
VOID Map::update_big_visible_tile_lurk_unit_to_role(Role* p_role_)
{
	INT nVisIndex = p_role_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	syn_big_visible_tile_invisible_unit_to_role(p_role_, pVisTile);
}

FLOAT Map::get_exp_rate()
{
	return g_GMPolicy.GetExpRate();
}

FLOAT Map::get_drop_rate()
{
	return g_GMPolicy.GetLootRate(get_map_type() == EMT_Normal);
}

FLOAT Map::get_practice_rate()
{
	return g_GMPolicy.GetPracticeRate( );
}

Pet* Map::find_pet( DWORD dw_id_ ) /* ?般在地图线程之内使用 */
{
	Creature* pCreature = map_creature.find(dw_id_); 
	if (VALID_POINT(pCreature))
	{
		return dynamic_cast<Pet*>(pCreature);
	}
	else
	{
		return NULL;
	}
}


BOOL Map::is_point_has_uint(DWORD id)
{
	if (map_point_has_list.find(id) == map_point_has_list.end())
		return false;
	return map_point_has_list[id]; 
}


BOOL Map::RandSpwanPoint(tag_map_spawn_point_list* spawnList, tag_map_spawn_point_info*& pMapSpawnInfo)
{
	if (!VALID_POINT(spawnList))
		return false;
	package_map<DWORD,	tag_map_spawn_point_info*> list = spawnList->list;
	DWORD dwSpawnID = 0;
	tag_map_spawn_point_info* pInfo = NULL;
	package_map<DWORD,	tag_map_spawn_point_info*>::map_iter it = list.begin();
	while(list.find_next(it, dwSpawnID, pInfo))
	{
		if (is_point_has_uint(pInfo->dw_att_id))
		{
			list.erase(dwSpawnID);
		}
	}
	if (list.size() <= 0)
		return false;

	return list.rand_find(dwSpawnID, pMapSpawnInfo);
}

Role* Map::stop_leave_prictice(DWORD	dw_id_)
{
	Role* pRole = map_leave_role.find(dw_id_);

	if(VALID_POINT(pRole))
	{
		pRole->set_leave_prictice(FALSE);
		map_leave_role.erase(dw_id_);

		return pRole;
	}

	return NULL;
}

VOID Map::remove_leave_pricitce_visible_tile(Unit* p_unit_)
{
	ASSERT( VALID_POINT(p_unit_) );

	// 门特殊处理 不放入可视列表	
	if ( IS_DOOR(p_unit_->GetID()) )
		return;

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT( VALID_VALUE(nVisIndex) );

	if( p_unit_->IsRole() )
	{
		p_visible_tile[nVisIndex].map_leave_role.erase(p_unit_->GetID());
	}
}

// 判断目标点是否有人
BOOL Map::IsHaveUnit(const Vector3 piont)
{
	tagVisTile* pVisTile[EUD_end] = {0};
	
	INT nVisIndex = world_position_to_visible_index(piont);

	get_visible_tile(nVisIndex, pVisTile);
	
	Role*		pRole		= NULL;
	Creature*	pCreature	= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 首先检测人物
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			// 自身到当前点的向量
			FLOAT fX1 = pRole->GetCurPos().x;
			FLOAT fZ1 = pRole->GetCurPos().z;

			if (fX1/TILE_SCALE == piont.x/TILE_SCALE && fZ1/TILE_SCALE == piont.z/TILE_SCALE)
				return false;

		}

		// 再检测生物  
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
		
			// 自身到当前点的向量
			FLOAT fX1 = pCreature->GetCurPos().x;
			FLOAT fZ1 = pCreature->GetCurPos().z;
			
			if (fX1/TILE_SCALE == piont.x/TILE_SCALE && fZ1/TILE_SCALE == piont.z/TILE_SCALE)
				return false;

		}
	}
	

	return true;
}


//BOOL Map::is_have_unit(const Vector3& point)
//{
//	int nTitleIndex = point.x + point.z * p_map_info->n_height;
//	return map_point_has_unit[nTitleIndex];
//}

//VOID Map::set_have_unit(const Vector3& point, bool bset)
//{
//	int nTitleIndex = point.x + point.z * p_map_info->n_height;
	//map_point_has_unit[nTitleIndex] = bset;
//}

VOID Map::remove_flow_creature( Creature* p_creature )
{
	if (!VALID_POINT(p_creature))
	{
		ASSERT(0);
		return;
	}

	// 从生物列表中拿走这个怪物
	map_creature.erase(p_creature->GetID());

	p_creature->SetMap(NULL);

	// 从可视地砖中拿走，但不同步客户端
	remove_from_visible_tile(p_creature);


	synchronize_remove_unit_to_big_visible_tile(p_creature);
}

DWORD Map::getAllRoleSameGuildID()
{
	DWORD dwOneGuildID = INVALID_VALUE;
	DWORD dwTwoGuildID = INVALID_VALUE;
	ROLE_MAP::map_iter iter = map_role.begin();
	Role* pRole = NULL;
	while(map_role.find_next(iter, pRole))
	{
		if(!VALID_POINT(pRole))
			continue;

		if (pRole->IsDead())
			continue;

		if (dwOneGuildID == INVALID_VALUE)
		{
			dwOneGuildID = pRole->GetGuildID();
			dwTwoGuildID = pRole->GetGuildID();
		}
		else
		{
			dwTwoGuildID = pRole->GetGuildID();
		}

		if (dwOneGuildID == INVALID_VALUE || 
			dwTwoGuildID == INVALID_VALUE ||
			dwOneGuildID != dwTwoGuildID)
		{
			return INVALID_VALUE;
		}
	}

	return dwOneGuildID;
}
