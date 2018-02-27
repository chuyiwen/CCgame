/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

//��ͼ��

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
// ������Ʒ�Ĵ�С
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
// ��ʼ����ͼ
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

	// ��ʼ��Ѱ·ϵͳ
	//if (!initSparseGraph(pInfo))
	//	return FALSE;


	// ����mapinfo����ʼ����ͼ�Ĺ�������б����Ϣ
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
// ����ʼ�����ʱ
//---------------------------------------------------------------------------------
VOID Map::on_init()
{
	if( VALID_POINT(p_script) )
		p_script->OnInit(this);
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID Map::destroy()
{
	// ɾ����ͼ��Ĺ���
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

	// �Ŷ����б���� ����Ҫdelete
	map_door.clear();

	// ɾ��������Ʒ����
	map_ground_item.reset_iterator();
	tag_ground_item* pGroundItem = NULL;
	while( map_ground_item.find_next(pGroundItem) )
	{
		pGroundItem->destroy_item();
		SAFE_DELETE(pGroundItem);
	}

	// ɾ���̵�
	Shop* pShop = NULL;
	package_map<DWORD, Shop*>::map_iter itShop = map_shop.begin();
	while( map_shop.find_next(itShop, pShop) )
	{
		Shop::Delete(pShop);
	}
	map_shop.clear();

	// ɾ���̻�
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

	// �رվŹ���͵���ͼ
	SAFE_DELETE_ARRAY(p_visible_tile);

	// ɾ������С�ӹ�����
	SAFE_DELETE(p_npc_team_mgr);
	
	//SAFE_DELETE(m_pSparseGraph);
	// ��սű�
	p_script = NULL;

	
}

//---------------------------------------------------------------------------------------
// ĳ������֮���Ƿ����ͨ��
//---------------------------------------------------------------------------------------
BOOL Map::if_can_direct_go(FLOAT f_srcx_, FLOAT f_src_y_, FLOAT f_dest_x_, FLOAT f_dest_z_, pathNode& n_near_pos_)
{
	// �õ�����Ŀ��֮�������
	Vector3 vVec = Vector3(f_dest_x_-f_srcx_, 0, f_dest_z_-f_src_y_ );
	INT nTileDist = sqrt(vVec.x*vVec.x + vVec.y*vVec.y) / TILE_SCALE;
	// �Ը��������й�һ��
	Vec3Normalize(vVec);
	
	if( 0 > nTileDist ) return false;

	
	Vector3 vDest = Vector3(f_srcx_, 0, f_src_y_);

	// �õ�һ����������յ�
	n_near_pos_.x() = vDest.x;
	n_near_pos_.y() = vDest.y;
	for (int i = 1; i <= nTileDist; i++)
	{
		FLOAT fDistAbs = FLOAT(i * TILE_SCALE);	// ���Ծ���
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
// ���ɵ�ͼ���߼����ԣ���������������㣬��������NPC����ȵ�
//---------------------------------------------------------------------------------
BOOL Map::init_logical_info(DWORD dw_guild_id_)
{
	// ����VISTILE
	n_visible_tile_array_width = (p_map_info->n_width + p_map_info->n_visit_distance - 1) / p_map_info->n_visit_distance + 2;
	n_visible_tile_array_height = (p_map_info->n_height + p_map_info->n_visit_distance - 1) / p_map_info->n_visit_distance + 2;

	p_visible_tile = new tagVisTile[n_visible_tile_array_width * n_visible_tile_array_height];
	if( !VALID_POINT(p_visible_tile) )
	{
		return FALSE;
	}

	// ���ɵ�ͼ�ű�
	p_script = g_ScriptMgr.GetMapScript(dw_id);

	// ���ص�ͼ����
	if( FALSE == init_all_map_creature(dw_guild_id_) )
	{
		SAFE_DELETE_ARRAY(p_visible_tile);
		return FALSE;
	}

	this->on_init( );

	return TRUE;
}

//-----------------------------------------------------------------------------------
// ��ʼ�����Ե�ͼ�еĹ������ʱΪ���߳�
//-----------------------------------------------------------------------------------
BOOL Map::init_all_map_creature(DWORD dw_guild_id_)
{
	// ��ʼ������ID������
	builder_creature_id.init(p_map_info);
	
	return init_all_fixed_creature_ex(dw_guild_id_);
	/*return init_all_fixed_creature(dw_guild_id_) && 
		init_all_spawn_point_creature(dw_guild_id_) &&
		init_all_spawn_list_creature(dw_guild_id_);*/
}

//-----------------------------------------------------------------------------------
// ��ʼ�����еĵ�ͼ�ڰڷŵĹ���
//-----------------------------------------------------------------------------------
BOOL Map::init_all_fixed_creature(DWORD dw_guild_id_)
{
	// һ��һ���Ĵ�������
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

		// ȡ��һ��ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// ���ɳ�������ͳ�������
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// ���ɹ���
		Creature* pCreature = Creature::Create(dw_creature_id, dw_id, this, pProto, pCreatureInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, pCreatureInfo->b_collide, pCreatureInfo->list_patrol, INVALID_VALUE);

		if(pProto->bLoading)
		{
			tagNPCLoading* st_NPCLoading = new tagNPCLoading;
			st_NPCLoading->dw_npc_id = dw_creature_id;
			st_NPCLoading->dw_Obj_id = pCreatureInfo->dw_att_id;
			list_NPCLoading.push_back(st_NPCLoading);
		}

		// ���뵽��ͼ��
		add_creature_on_load(pCreature);
		
		// ����ǳ�Ѩ������س�Ѩ����
		if( pCreature->IsNest() )
		{
			init_nest_creature(pCreature);
		}

		// ����ǹ���С�ӣ������С�ӹ���
		if( pCreature->IsTeam() )
		{
			init_team_creature(pCreature);
		}
	}

	DWORD dwCreatureSize = (map_creature.size() * sizeof(Creature));
	g_world.get_log()->write_log(_T("Read %d BYTE creature memory from map<%s>!\n"), dwCreatureSize, p_map_info->sz_map_name);

	// һ��һ���Ĵ����Ŷ���
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

		// ȡ��һ��ID
		DWORD dw_door_id = OBJID_TO_DOORID(pDoorInfo->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id) );

		// ���ɳ�������ͳ�������
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// ���ɹ���
		Creature* pCreature = Creature::Create(dw_door_id, dw_id, this, pProto, pDoorInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);

		// ���뵽��ͼ��
		add_creature_on_load(pCreature);

		// ���뵽�Ŷ����б�
		map_door.add(dw_door_id, pCreature );
	}

	return TRUE;
} 

//-----------------------------------------------------------------------------------
// ��ʼ�����еĵ�ͼ�ڰڷŵĹ���
// gx add 2013.4.16
// ��ʱû�б����ã�������������
//-----------------------------------------------------------------------------------
BOOL Map::init_all_fixed_creature_ex(DWORD dw_guild_id_)
{
	// һ��һ���Ĵ�������
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

		// ȡ��һ��ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// ���ɳ�������ͳ�������
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pCreatureInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0;

		// ���ɹ���
		Creature* pCreature = Creature::Create(dw_creature_id, dw_id, this, pProto, pCreatureInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL, INVALID_VALUE);

		if(pProto->bLoading)
		{
			tagNPCLoading* st_NPCLoading = new tagNPCLoading;
			st_NPCLoading->dw_npc_id = dw_creature_id;
			//st_NPCLoading->dw_Obj_id = pCreatureInfo->dw_att_id;
			list_NPCLoading.push_back(st_NPCLoading);
		}

		// ���뵽��ͼ��
		add_creature_on_load(pCreature);

		// ����ǳ�Ѩ������س�Ѩ����
		if( pCreature->IsNest() )
		{
			init_nest_creature(pCreature);
		}

		// ����ǹ���С�ӣ������С�ӹ���
		if( pCreature->IsTeam() )
		{
			init_team_creature(pCreature);
		}
	}

	DWORD dwCreatureSize = (map_creature.size() * sizeof(Creature));
	g_world.get_log()->write_log(_T("Read %d BYTE creature memory from map<%s>!\n"), dwCreatureSize, p_map_info->sz_map_name);

	// һ��һ���Ĵ����Ŷ���
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

		// ȡ��һ��ID
		DWORD dw_door_id = OBJID_TO_DOORID(pDoorInfo->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id) );

		// ���ɳ�������ͳ�������
		Vector3 vFaceTo;
		vFaceTo.x = cosf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(pDoorInfo->f_yaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// ���ɹ���
		Creature* pCreature = Creature::Create(dw_door_id, dw_id, this, pProto, pDoorInfo->v_pos, vFaceTo, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);

		// ���뵽��ͼ��
		add_creature_on_load(pCreature);

		// ���뵽�Ŷ����б�
		map_door.add(dw_door_id, pCreature );
	}

	return TRUE;
} 

//
//-----------------------------------------------------------------------------------
// ��ʼ����ͨ��ͼˢ�ֵ�Ĺ������ʱΪ���߳�
//-----------------------------------------------------------------------------------
BOOL Map::init_all_spawn_point_creature(DWORD dw_guild_id_)
{
	// ��ÿһ��ˢ�ֵ�	
	tag_map_spawn_point_info* pMapSpawnInfo = NULL;
	p_map_info->map_spawn_point.reset_iterator();
	while(p_map_info->map_spawn_point.find_next(pMapSpawnInfo))
	{
		ASSERT_P_VALID(pMapSpawnInfo);
		if (!VALID_POINT(pMapSpawnInfo)) continue;

		// ���ѡ��һ����
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

		// ��ȡid
		DWORD dwCreatureID = builder_creature_id.get_new_creature_id();

		// ���ɳ�������ͳ�������
		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		Vector3 vFaceTo;
		vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
		vFaceTo.y = 0.0f;

		// ������
		Creature* pCreature = Creature::Create(	dwCreatureID, get_map_id(), this, pCreatureProto, pMapSpawnInfo->v_pos, vFaceTo, 
			ECST_SpawnPoint, pMapSpawnInfo->dw_spawn_point_id, INVALID_VALUE, pMapSpawnInfo->b_collide, NULL, INVALID_VALUE, dw_guild_id_);
		ASSERT_P_VALID(pCreature);
		if (!VALID_POINT(pCreature)) continue;

		// �����ͼ
		add_creature_on_load(pCreature);
	}

	return TRUE;
}

// ��������ˢ�ֵ������
BOOL Map::init_all_spawn_list_creature(DWORD dw_guild_id_)
{
	// ��ÿһ��ˢ�ֵ�	
	tag_map_spawn_point_list* pMapSpawnList = NULL;
	p_map_info->map_spawn_point_list.reset_iterator();
	while(p_map_info->map_spawn_point_list.find_next(pMapSpawnList))
	{
		ASSERT_P_VALID(pMapSpawnList);
		if (!VALID_POINT(pMapSpawnList)) continue;

		// �������еĹ�
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

			// ��ȡid
			DWORD dwCreatureID = builder_creature_id.get_new_creature_id();

			// ���ɳ�������ͳ�������
			FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
			Vector3 vFaceTo;
			vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.y = 0.0f;

			// ������
			Creature* pCreature = Creature::Create(	dwCreatureID, get_map_id(), this, pCreatureProto, pMapSpawnInfo->v_pos, vFaceTo, 
				ECST_SpawnList, pMapSpawnInfo->dw_att_id, pMapSpawnInfo->dw_group_id, pMapSpawnInfo->b_collide, NULL, INVALID_VALUE, dw_guild_id_);
			ASSERT_P_VALID(pCreature);
			//pMapSpawnInfo->b_hasUnit = TRUE;
			map_point_has_list[pMapSpawnInfo->dw_att_id] = TRUE;
			if (!VALID_POINT(pCreature)) continue;

			// �����ͼ
			add_creature_on_load(pCreature);
		}

	}

	return TRUE;
}
//-----------------------------------------------------------------------------------
// ��ʼ����Ѩ����
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
			// �ҵ�һ�������ߵ������
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

			// ���һ������
			FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
			vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
			vFaceTo.y = 0.0f;

			// ȡ��һ��ID
			DWORD dwID = builder_creature_id.get_new_creature_id();
			ASSERT( IS_CREATURE(dwID) );

			// ���ɹ���
			Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pSpawnedCreatureProto, 
				vPos, vFaceTo, ECST_Nest, p_creature_->GetID(), INVALID_VALUE, p_creature_->NeedCollide(), NULL);

			// ���뵽��ͼ��
			add_creature_on_load(pSpawnedCreature);
		}
	}
}


//-----------------------------------------------------------------------------------
// ��ʼ��С�ӹ���
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

	// �õ�����С�ӵĶ���
	const tagNPCTeamOrder* pTeamOrder = p_npc_team_mgr->GetNPCTeamOrder(pNest->eOrderType);
	ASSERT(VALID_POINT(pTeamOrder));

	// ��������С��
	NPCTeam* pTeam = p_npc_team_mgr->CreateTeam(p_creature_);
	if(!VALID_POINT(pTeam))		return;

	// ���öӳ�С�ӣɣ�
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

		// ȡ��һ��ID
		DWORD dwID = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dwID) );

		// �������λ��
		vPos = p_npc_team_mgr->CalTeamMemPos(p_creature_, point, vFaceTo, pNest);

		// ���ɹ���
		Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pTeamMemProto , 
			vPos, vFaceTo, ECST_Team, p_creature_->GetID(),INVALID_VALUE,  p_creature_->NeedCollide(), NULL, pTeam->GetID());

		// ���뵽��ͼ��
		add_creature_on_load(pSpawnedCreature);

		// ���뵽����С��
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

	// �õ�����С�ӵĶ���
	const tagNPCTeamOrder* pTeamOrder = p_npc_team_mgr->GetNPCTeamOrder(pNest->eOrderType);
	ASSERT(VALID_POINT(pTeamOrder));

	// ��������С��
	NPCTeam* pTeam = p_npc_team_mgr->CreateTeam(p_creature_);
	if(!VALID_POINT(pTeam))		return;

	if(!VALID_POINT(p_guild_)) return ;

	// ���öӳ�С�ӣɣ�
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

			// ȡ��һ��ID
			DWORD dwID = builder_creature_id.get_new_creature_id();
			ASSERT( IS_CREATURE(dwID) );

			// �������λ��
			vPos = p_npc_team_mgr->CalTeamMemPos(p_creature_, point, vFaceTo, pNest);

			// ���ɹ���
			Creature* pSpawnedCreature = Creature::Create(dwID, dw_id, this, pTeamMemProto , 
				vPos, vFaceTo, ECST_GuildSentinel, p_creature_->GetID(), INVALID_VALUE, p_creature_->NeedCollide(), NULL, pTeam->GetID(), p_guild_->get_guild_att().dwID);

			// ���뵽��ͼ��
			add_creature_on_load(pSpawnedCreature);

			// ���뵽����С��
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
// ���£�������õ�ͼ��MapMgr���̺߳�������
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
//	// �ļ�����Ǵ����Ͻ�Ϊԭ�㣬ת�����½�Ϊԭ��
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
// ���µ�ͼ�����������ҵ���Ϣ
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
// ���µ�ͼ�����������Ϸ�����״̬
//--------------------------------------------------------------------------------------
VOID Map::update_all_objects()
{
	// �������е�ͼ������
	ROLE_MAP::map_iter itRole = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itRole, pRole) )
	{
		pRole->Update();
	}

	// �������е�ͼ��Ĺһ����
	ROLE_MAP::map_iter iter_leave_role = map_leave_role.begin();
	Role* pLeaveRole = NULL;
	while(map_leave_role.find_next(iter_leave_role, pLeaveRole))
	{
		pLeaveRole->Update();
	}

	// ���µ�ͼ������й���
	CREATURE_MAP::map_iter itCreature = map_creature.begin();
	Creature* pCreature = NULL;

	while( map_creature.find_next(itCreature, pCreature) )
	{
		if(pCreature->IsDelMap())
		{
			map_creature.erase(pCreature->GetID());
			pCreature->SetDelMap(FALSE);
			// ��ɾ������ʱ�Ƶ�����
			remove_from_visible_tile(pCreature);
			// ���뵽�����б��еȴ�����
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

	// ���µ�ͼ�������д�ˢ�µĹ���
	CREATURE_MAP::map_iter itDeadCreature = map_respawn_creature.begin();
	Creature* pDeadCreature = NULL;

	while( map_respawn_creature.find_next(itDeadCreature, pDeadCreature) )
	{
		ECreatureReviveResult eRet = pDeadCreature->TryRevive();

		if( ECRR_Success == eRet )
		{
			// �����ɹ�
			map_respawn_creature.erase(pDeadCreature->GetID());
			add_creature(pDeadCreature);
		}
		else if( ECRR_Failed == eRet )
		{
			// ����ʧ��
		}
		else if( ECRR_NeedDestroy == eRet )
		{
			// ��̬���ɵģ���Ҫɾ����
			map_respawn_creature.erase(pDeadCreature->GetID());
			Creature::Delete(pDeadCreature);
		}
		else if( ECRR_NeedReplace == eRet )
		{
			// ˢ�ֵ����ɵģ���Ҫ�滻
			spawn_creature_replace(pDeadCreature);
		}
		else if (ECRR_NeedRepos == eRet)
		{
			// ���������λ��
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
	// ���µ�����Ʒ
	package_map<INT64, tag_ground_item*>::map_iter it = map_ground_item.begin();
	tag_ground_item* pGroundItem = NULL;

	while( map_ground_item.find_next(it, pGroundItem) )
	{
		switch(pGroundItem->update())
		{
			// �Ƴ�����
		case EGIU_remove:
			{
				// �Ƴ�����
				remove_ground_item(pGroundItem);

				// ������Ʒ
				pGroundItem->destroy_item();

				// �ͷ��ڴ�
				SAFE_DELETE(pGroundItem);
			}
			break;
			// ͬ������״̬
		case EGIU_synchronize:
			{
				// ������Ʒ���ڵĿ��ӵ�ש����
				INT nVisIndex = world_position_to_visible_index(pGroundItem->v_pos);

				// �õ��Ź���
				tagVisTile* pVisTile[EUD_end] = {0};
				get_visible_tile(nVisIndex, pVisTile);

				// ͬ�������뵽�ͻ��˵���Һ�����
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

	// ����Ҫɾ���ĳ����ڴ�����
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
// ���¸õ�ͼ�������̵�
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
// ���¸õ�ͼ�������̻�
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
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------------
VOID Map::add_role(Role* p_role_, Map* pScrMap)
{
	ASSERT( VALID_POINT(p_role_) );

	send_npc_loading(p_role_);

	// ���뵽����б���
	map_role.add(p_role_->GetID(), p_role_);
	map_leave_role.erase(p_role_->GetID());

	// ������ҵ�session���뵽session�б���
	PlayerSession* pSession = p_role_->GetSession();
	if( VALID_POINT(pSession) )
	{
		map_session.add(pSession->GetSessionID(), pSession);
	}

	// ������ҵĵ�ͼ
	p_role_->SetMap(this);

	// ��������
	fix_position(p_role_->GetCurPos());

	// �����߶�
	p_role_->GetMoveData().DropDownStandPoint();

	//// �����ͼ����  // fix mwh2011-5-29 �в�ͬ��ͼʱ�����õ���������
	//p_role_->CheckMapArea();

	// ���ͽ����ͼ��Ϣ���ͻ���
	send_goto_new_map_to_player(p_role_);

	// ���͸õ�ͼ�г�����Ч
	send_scene_effect(p_role_);

	// ����������ڵĿ��ӵ�ש����
	INT nVisIndex = world_position_to_visible_index(p_role_->GetCurPos());

	// �õ��Ź���
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// ͬ�������뵽�ͻ��˵���Һ�����
	//synchronize_add_units(pRole, pVisTile);

	// �����ڷ�����λ������
	syn_big_visible_tile_visible_unit_and_ground_item_to_role(p_role_, pVisTile);

	// ����������λ
	syn_big_visible_tile_invisible_unit_to_role(p_role_, pVisTile);
	

	// ͬ���Ŷ��� 
	// �Ŷ����ڿ��ӵ�ש������  ��Ҫ����ҽ����ͼ��ʱ��ͬ��
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

	// ������䵽9����֮��
	add_to_visible_tile(p_role_, nVisIndex);

	//set_have_unit(p_role_->GetCurPos(), true);

	// �����ͼ���� // fix mwh 2011-05-26
	p_role_->CheckMapArea();

	// �����ͼ���������
	//mmz
	p_role_->OnEnterMap(pScrMap);

	//// ���ýű�
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
// ����Ԥ����NPC��Ϣ
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
// ��ͼ�ڼ���һ���������������ʱ��ӵģ����Ի�����Ҫͬ��
//-----------------------------------------------------------------------------------------
VOID Map::add_creature_on_load(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// ���뵽�����б���
	map_creature.add(p_creature_->GetID(), p_creature_);

	// ��������ĵ�ͼ 
	p_creature_->SetMap(this);
	
	//pCreature->OnScriptLoad();

	// �����������ڵĿ��ӵ�ש����
	INT nVisIndex = world_position_to_visible_index(p_creature_->GetCurPos());

	// �������䵽�Ź���֮��
	add_to_visible_tile(p_creature_, nVisIndex);

	// ������̵�ְ��NPC������ض�Ӧ���̵�
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
// ��ͼ�м���һ�������������Ϸ����ʱ��ӣ�������Ҫͬ��
//-----------------------------------------------------------------------------------------
VOID Map::add_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// ���뵽�����б���
	map_creature.add(p_creature_->GetID(), p_creature_);

	// ��������ĵ�ͼ
	p_creature_->SetMap(this);

	// �����������ڵĿ��ӵ�ש����
	INT nVisIndex = world_position_to_visible_index(p_creature_->GetCurPos());

	// �õ��Ź���
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// ͬ�������뵽�ͻ��˵���Һ�����
	synchronize_add_units(p_creature_, pVisTile);

	// �������䵽9����֮��
	add_to_visible_tile(p_creature_, nVisIndex);

	// ������̵�ְ��NPC������ض�Ӧ���̵�
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
		// ���뵽�����б���
		map_pet_delete.add(pPet->GetID(), pPet);
	}
}
//-----------------------------------------------------------------------------------------
// �ӵ�ͼ������һ��������������б�
//-----------------------------------------------------------------------------------------
VOID Map::remove_creature(Creature* p_creature_)
{
	ASSERT( VALID_POINT(p_creature_) );

	// �������б��������������(����map��ȫ����lc)
	//m_mapCreature.Erase(pCreature->GetID());

	p_creature_->SetDelMap(TRUE);

	// ˢ�ֵ����������
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
	// ����������ڵ�vistile
	//INT nVisIndex = p_creature_->GetVisTileIndex();

	// �õ��Ź���
	//tagVisTile* pVisTile[EUD_end] = {0};
	//get_visible_tile(nVisIndex, pVisTile);

	// �����Ƶ�updataʱ����
	// �ӿ��ӵ�ש�����ߣ�����ͬ���ͻ��� 
	//remove_from_visible_tile(p_creature_);

	// ���뵽�����б��еȴ�����
	//m_mapRespawnCreature.Add(pCreature->GetID(), pCreature);
}

//-----------------------------------------------------------------------------------------
// �ӵ�ͼ���Ƴ�����
//-----------------------------------------------------------------------------------------
VOID Map::remove_pet(Pet* p_pet_, BOOL bSendMsg)
{
	if (!VALID_POINT(p_pet_))
	{
		ASSERT(0);
		return;
	}

	// �������б��������������
	map_creature.erase(p_pet_->GetID());

	p_pet_->SetMap(NULL);

	// �ӿ��ӵ�ש�����ߣ�����ͬ���ͻ���
	remove_from_visible_tile(p_pet_);

	// ͬ�����ͻ���
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
	// �������б��������������
	map_creature.erase(p_creature_->GetID());

	// ����������ڵ�vistile
	INT nVisIndex = p_creature_->GetVisTileIndex();


	// ���뵽�����б��еȴ�����
	map_respawn_creature.add(p_creature_->GetID(), p_creature_);
}
//-----------------------------------------------------------------------------------------
// ���뵽��ͼ�е�ĳ��VisTile
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

	// �����⴦�� ����������б�
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
// ��ĳ�����ӵ�ש��ɾ��һ�����
//-----------------------------------------------------------------------------------------
VOID Map::remove_from_visible_tile(Unit* p_unit_, BOOL b_leave_online)
{
	ASSERT( VALID_POINT(p_unit_) );

	// �����⴦�� ����������б�	
	if ( IS_DOOR(p_unit_->GetID()) )
		return;

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT( VALID_VALUE(nVisIndex) );

	if( p_unit_->IsRole() )
	{
		p_visible_tile[nVisIndex].map_role.erase(p_unit_->GetID());
		// ������߹һ��߼�
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
		// ������߹һ��߼�
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
// ���絥λתΪ���ӵ�ש������
//-----------------------------------------------------------------------------------------
INT Map::world_position_to_visible_index(const Vector3& v_pos_)
{
	// ����Ƿ�
	if( !is_position_valid(v_pos_) ) return INVALID_VALUE;

	INT nIndexX = INT(v_pos_.x / TILE_SCALE / p_map_info->n_visit_distance) + 1;
	INT nIndexZ = INT(v_pos_.z / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	ASSERT( nIndexX > 0 && nIndexX < n_visible_tile_array_width - 1 );
	ASSERT( nIndexZ > 0 && nIndexZ < n_visible_tile_array_height - 1 );

	return nIndexZ * n_visible_tile_array_width + nIndexX;
}

//------------------------------------------------------------------------------------------
// �õ���Χ��ש
//------------------------------------------------------------------------------------------
VOID Map::get_visible_tile(INT n_visible_index_, tagVisTile* p_visible_tile_[EUD_end])
{
	// todo: ����ط���Ҫ��һ�£��жϲ��Ǻ��Ͻ�
	if( n_visible_index_ == INVALID_VALUE )
		return;

	// ��Ұ�ĵ�ש����
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
// ����ĳ��������Լ���Χȷ���м���visTile�ڸþ��η�Χ��
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

	// ȡ�����ĵ��visIndex
	INT nIndexX = INT(v_pos_.x / TILE_SCALE / p_map_info->n_visit_distance) + 1;
	INT nIndexZ = INT(v_pos_.z / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	ASSERT( nIndexX > 0 && nIndexX < n_visible_tile_array_width - 1 );
	ASSERT( nIndexZ > 0 && nIndexZ < n_visible_tile_array_height - 1 );

	// ȡ���߽������
	FLOAT fTempX = v_pos_.x - f_range_;
	INT nSrcIndexX = INT(fTempX / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	fTempX = v_pos_.x + f_range_;
	INT nDestIndexX = INT(fTempX / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	FLOAT fTempZ = v_pos_.z - f_range_;
	INT nSrcIndexZ = INT(fTempZ / TILE_SCALE / p_map_info->n_visit_distance) + 1;

	fTempZ = v_pos_.z + f_range_;
	INT nDestIndexZ = INT(fTempZ / TILE_SCALE / p_map_info->n_visit_distance) + 1;


	// �����Ƿ����
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
// �õ���Ҹı�λ�ú��뿪��Ұ�ĵ�ש
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

	// X����ƶ�
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
	// X�ұ��ƶ�
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
	// Xû���ƶ�
	else
	{

	}

	// Z�ϲ��ƶ�
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
	// Z�²��ƶ�
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
	// Zû���ƶ�
	else
	{

	}

	// ͳ�������
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_[n] = &p_visible_tile[m[n]];
	}
}

//------------------------------------------------------------------------------------------
// �õ���Ҹı�λ�ú������Ұ�ĵ�ש
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

	// X����ƶ�
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
	// X�ұ��ƶ�
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
	// Xû���ƶ�
	else
	{

	}

	// Z�ϲ��ƶ�
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
	// Z�²��ƶ�
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
	// Zû���ƶ�
	else
	{

	}

	// ͳ�������
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_[n] = &p_visible_tile[m[n]];
	}
}

//---------------------------------------------------------------------------------------------
// �õ���Ҹı�λ�ú������Ұ�ĵ�ש���뿪��Ұ�ĵ�ש
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

	// ������Ұ�ĵ�ש����
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

	// �뿪��Ұ�ĵ�ש����
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
		// ������Ұ�Ŀ��ӵ�ש
		m[EUD_center]		= n_new_visible_index_;
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_top]			= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		// �뿪��Ұ�Ŀ��ӵ�ש
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

	// X����ƶ�
	if( nNewVisIndexX < nOldVisIndexX )
	{
		// ������Ұ�Ŀ��ӵ�ש
		m[EUD_left]			= n_new_visible_index_ - 1;
		m[EUD_left_top]		= m[EUD_left] - n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_left] + n_visible_tile_array_width;

		// �뿪��Ұ�Ŀ��ӵ�ש
		r[EUD_right]			= n_old_visible_index_ + 1;
		r[EUD_right_top]		= r[EUD_right] - n_visible_tile_array_width;
		r[EUD_right_bottom]	= r[EUD_right] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == -2 )
		{
			// ������Ұ�Ŀ��ӵ�ש
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;

			// �뿪��Ұ�Ŀ��ӵ�ש
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_top]		= r[EUD_center] - n_visible_tile_array_width;
			r[EUD_bottom]	= r[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// X�ұ��ƶ�
	else if( nNewVisIndexX > nOldVisIndexX )
	{
		// ������Ұ�Ŀ��ӵ�ש
		m[EUD_right]			= n_new_visible_index_ + 1;
		m[EUD_right_top]		= m[EUD_right] - n_visible_tile_array_width;
		m[EUD_right_bottom]	= m[EUD_right] + n_visible_tile_array_width;

		// �뿪��Ұ�Ŀ��ӵ�ש
		r[EUD_left]			= n_old_visible_index_ - 1;
		r[EUD_left_top]		= r[EUD_left] - n_visible_tile_array_width;
		r[EUD_left_bottom]	= r[EUD_left] + n_visible_tile_array_width;

		if( nNewVisIndexX - nOldVisIndexX == 2 )
		{
			// ������Ұ�Ŀ��ӵ�ש
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_top]		= m[EUD_center] - n_visible_tile_array_width;
			m[EUD_bottom]	= m[EUD_bottom] + n_visible_tile_array_width;

			// �뿪��Ұ�Ŀ��ӵ�ש
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_top]		= r[EUD_center] - n_visible_tile_array_width;
			r[EUD_bottom]	= r[EUD_bottom] + n_visible_tile_array_width;
		}
	}
	// Xû���ƶ�
	else
	{

	}

	// Z�ϲ��ƶ�
	if( nNewVisIndexZ < nOldVisIndexZ )
	{
		// ������Ұ�Ŀ��ӵ�ש
		m[EUD_top]			= n_new_visible_index_ - n_visible_tile_array_width;
		m[EUD_left_top]		= m[EUD_top] - 1;
		m[EUD_right_top]		= m[EUD_top] + 1;

		// �뿪��Ұ�Ŀ��ӵ�ש
		r[EUD_bottom]		= n_old_visible_index_ + n_visible_tile_array_width;
		r[EUD_left_bottom]	= r[EUD_bottom] - 1;
		r[EUD_right_bottom]	= r[EUD_bottom] + 1;


		if( nNewVisIndexZ - nOldVisIndexZ == -2 )
		{
			// ������Ұ�Ŀ��ӵ�ש
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;

			// �뿪��Ұ�Ŀ��ӵ�ש
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_left]		= r[EUD_center] - 1;
			r[EUD_bottom]	= r[EUD_center] + 1;

		}
	}
	// Z�²��ƶ�
	else if( nNewVisIndexZ > nOldVisIndexZ )
	{
		// ������Ұ�Ŀ��ӵ�ש
		m[EUD_bottom]		= n_new_visible_index_ + n_visible_tile_array_width;
		m[EUD_left_bottom]	= m[EUD_bottom] - 1;
		m[EUD_right_bottom]	= m[EUD_bottom] + 1;

		// �뿪��Ұ�Ŀ��ӵ�ש
		r[EUD_top]			= n_old_visible_index_ - n_visible_tile_array_width;
		r[EUD_left_top]		= r[EUD_top] - 1;
		r[EUD_right_top]		= r[EUD_top] + 1;


		if( nNewVisIndexZ - nOldVisIndexZ == 2 )
		{
			// ������Ұ�Ŀ��ӵ�ש
			m[EUD_center]	= n_new_visible_index_;
			m[EUD_left]		= m[EUD_center] - 1;
			m[EUD_bottom]	= m[EUD_center] + 1;

			// �뿪��Ұ�Ŀ��ӵ�ש
			r[EUD_center]	= n_old_visible_index_;
			r[EUD_left]		= r[EUD_center] - 1;
			r[EUD_bottom]	= r[EUD_center] + 1;
		}
	}
	// Zû���ƶ�
	else
	{

	}

	// ͳ�������
	for(INT n = 0; n < EUD_end; n++)
	{
		if( m[n] != INVALID_VALUE )
			p_visible_tile_add_[n] = &p_visible_tile[m[n]];

		if( r[n] != INVALID_VALUE )
			p_visible_tile_dec_[n] = &p_visible_tile[r[n]];
	}
}

//------------------------------------------------------------------------------------------
// ���;Ź���Χ�ڵ���Ϣ
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
// ���;Ź���Χ�ڵ���Ϣ
//------------------------------------------------------------------------------------------
VOID Map::send_big_visible_tile_message(tagVisTile *p_visible_tile_[EUD_end], const LPVOID p_message_, const DWORD dw_size_)
{
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == p_visible_tile_[n] )
			continue;
		
		// �ҵ�ÿ����ש����
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
// ������Ϣ������ͼ�ڵ����
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
// ͬ�����ӵ�ש�ڵ�����λ�����
//--------------------------------------------------------------------------------------------
VOID Map::syn_big_visible_tile_invisible_unit_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end])
{
	// 1.�Ƿ���̽������
	if( !pRole->HasDectiveSkill() )
		return;
	
	BYTE			byMsg[1024] = {0};
	package_list<DWORD>	listRemove;
	
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// �ҵ�ÿ����ש�ĵ�λ
		package_map<DWORD, Unit*>& mapUnit = pVisTile[n]->map_invisible_unit;
		mapUnit.reset_iterator();
		Unit* pUnit = NULL;

		while( mapUnit.find_next(pUnit) )
		{
			if( !VALID_POINT(pUnit) )
				continue;

			// 2.�Ƿ���ȫ�ɼ�
			if( (pRole->CanSeeTargetEntirely(pUnit)) )
				continue;

			// 3.�ڿ����б���
			if( pRole->IsInVisList(pUnit->GetID()) )
			{
				// �ڿ��ӷ�Χ��
				if( pRole->IsInVisDist(pUnit->GetID()) )
					continue;

				// ���ڿ��ӷ�Χ��
				listRemove.push_back(pUnit->GetID());

				pRole->RemoveFromVisList(pUnit->GetID());

				continue;
			}

			// 4.����A�Ŀ����б���
			// 4.1����A���ӷ�Χ��
			if( !pRole->IsInVisDist(pUnit->GetID()) )
				continue;

			// 4.2 ��A���ӷ�Χ��
			DWORD dw_size = calculate_movement_message(pUnit, byMsg, 1024);
			if(dw_size == 0)
				continue;

			pRole->SendMessage(byMsg, dw_size);

			pRole->Add2VisList(pUnit->GetID());
		}
	}

	// ���list��Ϊ�գ���self����ҵĻ������͸������Ҫɾ����Զ�����
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
// �����ӵ�ש�����п��ӵ�λ���������Ʒͬ�������
//--------------------------------------------------------------------------------------------
VOID Map::syn_big_visible_tile_visible_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end])
{
	ASSERT( VALID_POINT(pRole) );

	BYTE byMsg[1024] = {0};
	DWORD dw_size = 0;	

	NET_SIS_synchronize_item send;

	// �����Ź����б�
	for(INT n = 0; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) )
			continue;

		// ͬ��������״̬��Զ�����
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

		// ͬ��������״̬�¹�
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

		//ͬ��������Ʒ���ͻ���
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
// ������λͬ�������ӵ�ש�ڵĿ��Կ����������
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

		// �ҵ�ÿ����ש����
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.�Ƿ���ȫ�ɼ�
			if( pRole->CanSeeTargetEntirely(pUnit) )
			{
				pRole->SendMessage(p_message, dw_size);
				continue;
			}

			// 2.B�Ƿ���̽������
			if( !pRole->HasDectiveSkill() )
				continue;

			// 3.��B�Ŀ����б���
			if( pRole->IsInVisList(pUnit->GetID()) )
			{
				// ����B���ӷ�Χ��
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

			// 4.����B�Ŀ����б���
			// 4.1����B���ӷ�Χ��
			if( !pRole->IsInVisDist(pUnit->GetID()) )
				continue;

			// 4.2 ��B���ӷ�Χ��
			pRole->SendMessage(p_message, dw_size);

			pRole->Add2VisList(pUnit->GetID());
		}
	}
}

//--------------------------------------------------------------------------------------------
// ͬ�����ӵ�ש�ڵ��ƶ�
//--------------------------------------------------------------------------------------------
VOID Map::synchronize_movement_to_big_visible_tile(Unit* p_self_)
{
	if( !VALID_POINT(p_self_) ) return;

	BYTE	byMsg[1024] = {0};
	DWORD	dw_size = calculate_movement_message(p_self_, byMsg, 1024);

	if( dw_size == 0 ) return;

	tagVisTile* pVisTile[EUD_end] = {0};

	// �����Χ�Ź���Ŀ��ӵ�ש
	get_visible_tile(p_self_->GetVisTileIndex(), pVisTile);
	
	// ����ӵ�ש���˷���ͬ������
	if( !p_self_->IsInStateInvisible() )	// A�ɼ�
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, pVisTile, byMsg, dw_size);
	}
	else	// A���ɼ�
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, pVisTile, byMsg, dw_size);
	}

	// �����ӵ�ש������λͬ������ǰ���
	if( p_self_->IsRole() )
		syn_big_visible_tile_invisible_unit_to_role(static_cast<Role*>(p_self_), pVisTile);
}

//-------------------------------------------------------------------------------------------------------------
// �ı��ͼ���ӵ�שʱͬ��
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_change_visible_tile(Unit* p_self_, INT n_old_visible_index_, INT n_new_visible_index_)
{
	// �Ȱ���Ҵӵ�ǰ�Ŀ���������ɾ��
	remove_from_visible_tile(p_self_);

	// ȡ����Ϊ�л���ש��ɾ������ӵĵ�ש
	tagVisTile* pVisTileAdd[EUD_end] = {0};
	tagVisTile* pVisTileDec[EUD_end] = {0};

	get_visible_tile_add_and_dec(n_old_visible_index_, n_new_visible_index_, pVisTileAdd, pVisTileDec);

	// �������ɾ������Һ���������Ƴ�
	synchronize_remove_units(p_self_, pVisTileDec);

	// ��ͬ�����뵽��Ұ�е���Һ�����
	synchronize_add_units(p_self_, pVisTileAdd);

	// �ٰ�����뵽�µĿ��ӵ�ש��
	add_to_visible_tile(p_self_, n_new_visible_index_);
}

//-------------------------------------------------------------------------------------------------------------
// ֪ͨ����Զ�����ɾ���ö��󣬲������Self����ҵĻ�������ЩԶ����Ҵ��Լ����ؿͻ���ɾ��
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_remove_units(Unit* p_self_, tagVisTile* p_visible_tile_dec_[EUD_end])
{
	ASSERT( VALID_POINT(p_self_) );

	BOOL bSelfInvisible = p_self_->IsInStateInvisible();

	NET_SIS_remove_remote send;
	send.dw_role_id[0] = p_self_->GetID();

	// ͬ�����Ź������������
	if( p_self_->IsInStateInvisible() )
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_dec_, &send, send.dw_size);
	}
	else
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_dec_, &send, send.dw_size);
	}

	// ���Ź�������Ϣͬ���������
	if( p_self_->IsRole() )
	{
		Role *pRole = static_cast<Role*>(p_self_);

		syn_remove_big_visible_tile_unit_and_ground_item_to_role(pRole, p_visible_tile_dec_);
	}
}

//-------------------------------------------------------------------------------------------------------------
// ��ɫ�쳣�Ź���󣬽�������Ϣͬ������ɫ
//-------------------------------------------------------------------------------------------------------------
VOID Map::syn_remove_big_visible_tile_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTileDec[EUD_end])
{
	package_list<DWORD> listRemovedUnit;
	package_list<INT64> listGroundItem;
	
	// �����Ź����б�
	for(INT n = 0;  n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTileDec[n]) )
			continue;

		// �ÿ��ӵ�שҪ��ɾ����������ÿ��ӵ�ש�е������б�
		package_map<DWORD, Role*>& mapRole = pVisTileDec[n]->map_role;
		mapRole.reset_iterator();
		Role* pRemoteRole = NULL;

		while( mapRole.find_next(pRemoteRole) )
		{
			// ��Ҫɾ������Ҽ��뵽�б���
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
			// ��Ҫɾ������Ҽ��뵽�б���
			if( !pRemoteRole->IsInStateInvisible() )
				listRemovedUnit.push_back(pRemoteRole->GetID());
			else if( pRole->IsInVisList(pRemoteRole->GetID()) )
			{
				listRemovedUnit.push_back(pRemoteRole->GetID());
				pRole->RemoveFromVisList(pRemoteRole->GetID());
			}
		}

		// �����ÿ��ӵ�ש�еķ�����б�
		package_map<DWORD, Creature*>& mapCreature = pVisTileDec[n]->map_creature;
		mapCreature.reset_iterator();
		Creature* pCreature = NULL;

		while( mapCreature.find_next(pCreature) )
		{
			// ��Ҫɾ���ķ���Ҽ��뵽�б���
			if( !pCreature->IsInStateInvisible() )
				listRemovedUnit.push_back(pCreature->GetID());
			else if( pRole->IsInVisList(pCreature->GetID()) )
			{
				listRemovedUnit.push_back(pCreature->GetID());
				pRole->RemoveFromVisList(pCreature->GetID());
			}
		}

		//�����ÿ��ӵ�ש�е�GroundItem�б�
		package_map<INT64, tag_ground_item*>& mapGroundItem = pVisTileDec[n]->map_ground_item;
		mapGroundItem.reset_iterator();
		tag_ground_item* pGroundItem = NULL;
		while (mapGroundItem.find_next(pGroundItem))
		{
			//��Ҫͬ����GroundItem�����б�
			if( pGroundItem->is_valid())
			{
				listGroundItem.push_back(pGroundItem->n64_id);
			}
		}
	}

	// ���list��Ϊ�գ���self����ҵĻ������͸������Ҫɾ����Զ�����
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

	//���list��Ϊ��,��self����ҵĻ�����һ�ν�GroundItem���͸������Ҫɾ����GroundItem
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
// ֪ͨ����Զ�����ɾ���ö��󣬲������Self����ҵĻ�������ЩԶ����Ҵ��Լ����ؿͻ���ɾ��
//-------------------------------------------------------------------------------------------------------------
VOID Map::synchronize_remove_unit_to_big_visible_tile(Unit* p_self_)
{
	INT nVisIndex = world_position_to_visible_index(p_self_->GetMoveData().m_vPos);

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	synchronize_remove_units(p_self_, pVisTile);
}

//-------------------------------------------------------------------------------------------------------
// ͬ�����뵽��ש�е���ң�������
//-------------------------------------------------------------------------------------------------------
VOID Map::synchronize_add_units(Unit* p_self_, tagVisTile* p_visible_tile_add_[EUD_end])
{
	ASSERT( VALID_POINT(p_self_) );

	BOOL bSelfInvisible = p_self_->IsInStateInvisible();

	BYTE byMsg[1024] = {0};
	DWORD dw_size = calculate_movement_message(p_self_, byMsg, 1024);
	if(dw_size == 0)
		return;

	// ��Aͬ�������������
	if( !p_self_->IsInStateInvisible() )	// A�ɼ�
	{
		syn_visible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_add_, byMsg, dw_size);
	}
	else	// A���ɼ�
	{
		syn_invisible_unit_to_big_visible_tile_role(p_self_, p_visible_tile_add_, byMsg, dw_size);
	}

	// ��AΪ��ң��򽫸�������Ϣͬ������
	if( p_self_->IsRole() )
	{
		Role *pRole = static_cast<Role*>(p_self_);

		// �����ڷ�����λ������
		syn_big_visible_tile_visible_unit_and_ground_item_to_role(pRole, p_visible_tile_add_);

		// ����������λ
		syn_big_visible_tile_invisible_unit_to_role(pRole, p_visible_tile_add_);
	}
}

//----------------------------------------------------------------------------
// ������������Ƿ���ͬһ��vistile��
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
// ͨ������ĵ�ǰ�������㷢�͸��ͻ��˵�ͬ����Ϣ
//----------------------------------------------------------------------------
DWORD Map::calculate_movement_message(Unit* pSelf, LPBYTE p_message, DWORD dw_size)
{
	ASSERT( VALID_POINT(pSelf) && VALID_POINT(p_message) );

	// �������ﵱǰ�ƶ�״̬����ͬ������
	EMoveState eCurMove = pSelf->GetMoveState();
	DWORD dwRealSize = 0;

	switch (eCurMove)
	{
		// վ��
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

		// ����
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

		// ����
	case EMS_HitFly:
		{
			NET_SIS_hit_fly* pSend = (NET_SIS_hit_fly*)p_message;
			pSend->dw_message_id = get_tool()->crc32("NET_SIS_hit_fly");
			pSend->dw_size = sizeof(NET_SIS_hit_fly);
			pSend->dw_role_id = pSelf->GetID();
			pSend->curPos = pSelf->GetCurPos();

			dwRealSize = pSend->dw_size;
		}


		// ����Ѳ��
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
			pSend->fXZSpeed = pSelf->GetXZSpeed() * 0.4f;	// Ѳ���ٶ�
			pSend->bCollide = static_cast<Creature*>(pSelf)->NeedCollide();

			dwRealSize = pSend->dw_size;
		}
		break;

		// ��������
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

		// ��������
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
		// �����ܲ�
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
// ����������λ�ö�Ӧ������
//------------------------------------------------------------------------------
INT64 Map::check_area(Role* p_role_)
{
	if( !VALID_POINT(p_role_) ) return 0;
	if( !VALID_POINT(p_map_info) ) return 0;

	INT64 n64Ret = 0;

	// �õ���ҵİ����к͸�������
	//AABBox roleBox = p_role_->GetAABBox();
	//INT nTileX = INT(p_role_->GetCurPos().x / (FLOAT)TILE_SCALE);
	//INT nTileZ = INT(p_role_->GetCurPos().z / (FLOAT)TILE_SCALE);
	
	float x = p_role_->GetCurPos().x;
	float z = p_role_->GetCurPos().z;

	tag_map_area_info* pArea = NULL;

	// ���ȼ�ⰲȫ��
	pArea = is_in_area(p_map_info->map_safe_area, x, z);
	if( VALID_POINT(pArea) )
	{
		n64Ret |= ERS_SafeArea;

		//const tagEquip* pRaid = p_role_->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
		//if (!VALID_POINT(pRaid))
		//{
		//	// ����
		//	p_role_->StopMount();
		//}
		
// 		//���밲ȫ���Զ���غ�ƽģʽ
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

	// �ټ��PVP��
	pArea = is_in_area(p_map_info->map_pvp_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_PVPArea;

	// �ټ���̯��
	pArea = is_in_area(p_map_info->map_stall_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_StallArea;

	// �ټ�������
	pArea = is_in_area(p_map_info->map_prison_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_PrisonArea;

	// ���ű���
	pArea = is_in_area(p_map_info->map_script_area, x, z);
	if( VALID_POINT(pArea) ) on_enter_area(p_role_, pArea);

	// �����ͨ��
	pArea = is_in_area(p_map_info->map_common_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_CommonArea;

	// �����԰�ȫ��
	pArea = is_in_area(p_map_info->map_real_safe_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_RealSafeArea;

	// �����ս��
	pArea = is_in_area(p_map_info->map_guild_battle, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_GuildBattle;

	// ����޳ͷ���
	pArea = is_in_area(p_map_info->map_no_punish_area, x, z);
	if( VALID_POINT(pArea) ) n64Ret |= ERS_NoPunishArea;

	// ���һ���
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
			// ����
			p_role_->StopMount();
		}
	}

	return n64Ret;
}

//------------------------------------------------------------------------------
// �������ǲ�����ĳ����������Χ��
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
	//// �õ���ҵİ����к͸�������
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
// �õ�ĳ��trigger��Ӧ���������к�
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
// ���ĳ�������к͸��������Ƿ���ĳ�������б��е�һ��������
//------------------------------------------------------------------------------
tag_map_area_info* Map::is_in_area(tag_map_info::MAP_AREA& map_area_, float x, float z)
{
	tag_map_info::MAP_AREA::map_iter it = map_area_.begin();
	tag_map_area_info* pArea = NULL;

	while( map_area_.find_next(it, pArea) )
	{
		//// �������ж����ཻ
		//if( !pArea->box.Intersects(role_box_) ) continue;

		//// �������ཻ��������������ڲ�����������
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
// �������ӵ��ǳ��б��У����ϲ���������ҵ�ʵ�ʵǳ��ͱ���
//------------------------------------------------------------------------------
VOID Map::role_logout(DWORD dw_id_)
{
	Role* pRole = map_role.find(dw_id_);
	if( VALID_POINT(pRole) )
	{
		pRole->removeFlowCreature();
		// �뿪��ͼ
		role_leave_map(pRole->GetID(), TRUE);
	}
}

//------------------------------------------------------------------------------
// �������ӵ��ǳ��б��У����ϲ���������ҵ�ʵ�ʵǳ��ͱ���
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
		//�����뿪��ͼǰ���������
		pRole->OnLeaveMap();

		// ���ýű�
		if( VALID_POINT(p_script) )
		{
			p_script->OnPlayerLeave(pRole, this);
		}

		// ͬ����Χ���ɾ���Լ�
		synchronize_role_leave_map(pRole);

		// ����Ҵ��Լ����б���ɾ��
		map_role.erase(dw_id_);

		// ������������߼�
		if(pRole->is_leave_pricitice())
		{
			map_leave_role.add(dw_id_, pRole);
		}
		else
		{
			map_leave_role.erase(dw_id_);
			pRole->SetMap(NULL);
		}

		// ������session��Ч�������ھʹ��б���ɾ��
		if( pRole->GetSession() )
		{
			map_session.erase(pRole->GetSession()->GetSessionID());
		}
	}
}

//-------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//-------------------------------------------------------------------------------
VOID Map::role_leave_map(DWORD dw_id_, BOOL b_leave_online)
{
	Role* pRole = map_role.find(dw_id_);
	if( VALID_POINT(pRole) )
	{
		//�����뿪��ͼǰ���������
		pRole->OnLeaveMap();
		
		
		// ���ýű�
		if( VALID_POINT(p_script) )
		{
			p_script->OnPlayerLeave(pRole, this);
		}

		// ͬ����Χ���ɾ���Լ�
		synchronize_role_leave_map(pRole, b_leave_online);
		
		//set_have_unit(pRole->GetCurPos(), false);

		// ����Ҵ��Լ����б���ɾ��
		map_role.erase(dw_id_);

		// ������������߼�
		if(pRole->is_leave_pricitice() && pRole->IsInRoleState(ERS_Prictice) && b_leave_online)
		{
			map_leave_role.add(dw_id_, pRole);
		}
		else
		{
			map_leave_role.erase(dw_id_);
			pRole->SetMap(NULL);
		}
		
		// ������session��Ч�������ھʹ��б���ɾ��
		if( pRole->GetSession() )
		{
			map_session.erase(pRole->GetSession()->GetSessionID());
		}
	}
}

//-------------------------------------------------------------------------------
// �ڵ�ͼ�и���ĳ��ID������һ���������ã�һ��Ҫ�ڸõ�ͼ�߳�֮��ʹ�ã�
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
// ��������ʼ��NPC�������̵�
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
// ��������ʼ��NPC�������̻�
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
// ����뿪��ͼͬ��
//-------------------------------------------------------------------------------
VOID Map::synchronize_role_leave_map(Role* p_self_, BOOL b_leave_online)
{
	if( !VALID_POINT(p_self_) ) return;

	INT nVisTileIndex = p_self_->GetVisTileIndex();

	// ���ȴ�VisTile���Ƴ�
	remove_from_visible_tile(p_self_, b_leave_online);

	// ������������߼�
	if(!p_self_->is_leave_pricitice())
	{
		// �õ��Ź���������
		tagVisTile* pVisTile[EUD_end] = {0};
		get_visible_tile(nVisTileIndex, pVisTile);

		// �����Ƴ�ͬ������
		synchronize_remove_units(p_self_, pVisTile);
	}
	
}

//----------------------------------------------------------------------------------
// ���ͽ����µ�ͼ���ͻ���
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
// ���������Ʒ�Ƿ񵽵��� --- ������Ʒϵͳ 
//----------------------------------------------------------------------------------
BOOL Map::can_putdown(Unit* p_unit, INT n_index_, Vector3& v_position_)
{
	if( !VALID_POINT(p_unit) )
		return FALSE;

	// ���ݹ�������Ʒ����
	const Vector3& vCreaturePos = p_unit->GetCurPos();

	// �õ��������
	INT nIndexX = INT(vCreaturePos.x / TILE_SCALE);
	INT nIndexZ = INT(vCreaturePos.z / TILE_SCALE);

	// �ҳ�һ������������
	if( FALSE == get_can_go_position_from_index(n_index_, nIndexX, nIndexZ, (n_index_ / 8 + 1), v_position_) )
	{
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------------
// ���������Ʒ������ --- ������Ʒϵͳ 
//----------------------------------------------------------------------------------
VOID Map::putdown_item(Creature* p_creature_, tagItem* p_loot_item_, DWORD dw_owner_id_, DWORD dw_team_id_, 
					   Vector3& v_position_, EPickMode e_pick_mode_, EGTAssignState e_assign_sate_, INT n_tick_down_)
{
	if( !VALID_POINT(p_creature_) || !VALID_POINT(p_loot_item_) )
		return;

	DWORD dw_boss_id = 0;
	if(VALID_POINT(p_creature_->GetProto()) && p_creature_->GetProto()->is_boss())
		dw_boss_id = p_creature_->GetProto()->dw_data_id;


	// ȡ����ͷ���߶�
	v_position_.y = p_creature_->GetCurPosTop().y;

	// ���ɵ�����Ʒ
	tag_ground_item* pGroundItem = new tag_ground_item(get_ground_item_id(), p_loot_item_->dw_data_id, 
														p_loot_item_->n16Num, p_loot_item_, v_position_,
														dw_owner_id_, dw_team_id_, 0, p_creature_->GetID(),
														e_pick_mode_, e_assign_sate_, n_tick_down_, dw_boss_id);
	ASSERT(pGroundItem->is_valid());

	// �ڵ����������Ʒ
	add_ground_item(pGroundItem);
}

//----------------------------------------------------------------------------------
// ��������Ǯ������ --- ������Ʒϵͳ 
//----------------------------------------------------------------------------------
VOID Map::putdown_money(Creature* p_creature_, INT n_money_, DWORD dwTeamID, DWORD dwOwnerID, Vector3 v_positon_)
{
	if( !VALID_POINT(p_creature_)/* || !VALID_POINT(p_role_)*/ )
		return;

	//DWORD dwTeamID = p_role_->GetTeamID();
	//DWORD dwOwnerID = VALID_VALUE(dwTeamID) ? INVALID_VALUE : p_role_->GetID();

	// ȡ����ͷ���߶�
	v_positon_.y = p_creature_->GetCurPosTop().y;

	// ���ɵ�����Ʒ
	tag_ground_item* pGroundItem = new tag_ground_item(get_ground_item_id(), TYPE_ID_MONEY, n_money_, NULL,
													v_positon_, dwOwnerID, dwTeamID, 0, p_creature_->GetID());
	ASSERT(pGroundItem->is_valid());

	// ��������������Ʒ
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
// ��������������Ʒ --- ������Ʒϵͳ
//-----------------------------------------------------------------------------
VOID Map::add_ground_item(tag_ground_item *p_ground_item_)
{
	if( !VALID_POINT(p_ground_item_) ) return;
	if( !p_ground_item_->is_valid() ) return;


	p_ground_item_->v_pos.y = 0;

	// ���뵽m_mapGroundItem��
	map_ground_item.add(p_ground_item_->n64_id, p_ground_item_);

	// ������Ʒ���ڵĿ��ӵ�ש����
	INT nVisIndex = world_position_to_visible_index(p_ground_item_->v_pos);

	// �õ��Ź���
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// ͬ�������뵽�ͻ��˵���Һ�����
	synchronize_add_ground_item(p_ground_item_, pVisTile);

	// �������䵽9����֮��
	add_ground_item_to_visible_tile(p_ground_item_, nVisIndex);

}
//-----------------------------------------------------------------------------
// �ӵ������Ƴ���Ʒ --- ������Ʒϵͳ
//-----------------------------------------------------------------------------
VOID Map::remove_ground_item(tag_ground_item* p_ground_item_)
{
	if(!VALID_POINT(p_ground_item_))
		return;

	if (!p_ground_item_->is_valid())
		return;

	//��m_mapGroundItem ���Ƴ�
	map_ground_item.erase(p_ground_item_->n64_id);
	//������Ʒ���ڵĿ��ӵ�ש����
	INT nVisIndex = world_position_to_visible_index(p_ground_item_->v_pos);

	// �õ��Ź���
	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	// ͬ�������뵽�ͻ��˵���Һ�����
	synchronize_remove_ground_item(p_ground_item_, pVisTile);

	// �������䵽9����֮��
	remove_ground_item_from_visible_tile(p_ground_item_, nVisIndex);

}

//-----------------------------------------------------------------------------
// ͬ�����뵽��ש�е���Ʒ --- ������Ʒϵͳ
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
	// ����ӵ�ש���˷���ͬ������
	send_big_visible_tile_message(p_visible_tile_add_, &send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ͬ�����뵽��ש�е���Ʒ --- ������Ʒϵͳ
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

	// ����ӵ�ש���˷���ͬ������
	send_big_visible_tile_message(p_visible_tile_add_, &send, send.dw_size);
}


//-----------------------------------------------------------------------------
// ͬ���Ƴ���ש�е���Ʒ --- ������Ʒϵͳ
//-----------------------------------------------------------------------------
VOID Map::synchronize_remove_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end])
{
	if(!p_ground_item_->is_valid())
		return;

	//������Ʒ��ʧ��Ϣ
	NET_SIS_role_ground_item_disappear disappear;
	disappear.n64_serial[0] = p_ground_item_->n64_id;

	// ����ӵ�ש���˷���ͬ������
	send_big_visible_tile_message(p_visible_tile_add_, &disappear, disappear.dw_size);
}

//-----------------------------------------------------------------------------
// ���͵�ͼ������Ч��Ϣ	---	��ҽ��볡������
//-----------------------------------------------------------------------------
VOID Map::send_scene_effect(Role* p_self_)
{
	if (!VALID_POINT(p_self_))	return;

	// ��ÿͻ��˷��������Ѿ�����ĳ�����Ч
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
// ����������Ч	--- �ű�����
//-----------------------------------------------------------------------------
VOID Map::play_scene_effect(ESceneEffectType e_type_, DWORD dw_obj_id_, Vector3 v_pos_, DWORD dw_effect_id_)
{
	// ��鳡����Ч����
	if (!VALID_POINT(p_map_info))	return;
	if (!VALID_POINT(p_map_info->map_trigger_effect.find(dw_obj_id_)))
		return;

	// ������Ч����
	h_mutex.Acquire();
	if (!list_scene_effect.is_exist(dw_obj_id_))
		list_scene_effect.push_back(dw_obj_id_);
	h_mutex.Release();

	// ֪ͨ��ͼ����ҿ���������Ч(����ͻ��˼�����)
	NET_SIS_play_scene_effect send;
	send.eType		= e_type_;
	send.dwObjID	= dw_obj_id_;
	send.vPos		= v_pos_;
	send.dwEffectID	= dw_effect_id_;
	send_map_message(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// �رճ�����Ч	--- �ű�����
//-----------------------------------------------------------------------------
VOID Map::stop_scene_effect(DWORD dw_obj_id_)
{
	// ����Ƿ��Ѿ�����
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

	// ֪ͨ��ͼ���������
	NET_SIS_stop_scene_effect send;
	send.dwObjID	= dw_obj_id_;
	send_map_message(&send, send.dw_size);
}

//-----------------------------------------------------------------------------------------
// ������Ʒ����ͼ�е�ĳ��VisTile --- ������Ʒϵͳ
//-----------------------------------------------------------------------------------------
VOID Map::add_ground_item_to_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_)
{
	p_visible_tile[n_visible_index_].map_ground_item.add(p_ground_item_->n64_id, p_ground_item_);
}

//-----------------------------------------------------------------------------------------
// �Ƴ���ͼ��ĳ��VisTile����Ʒ --- ������Ʒϵͳ
//-----------------------------------------------------------------------------------------
VOID Map::remove_ground_item_from_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_)
{
	p_visible_tile[n_visible_index_].map_ground_item.erase(p_ground_item_->n64_id);
}

//-----------------------------------------------------------------------------------------
// ˢ�µ�ˢ��
//-----------------------------------------------------------------------------------------
VOID Map::spawn_creature_replace( Creature* p_dead_creature_ )
{
	ASSERT_P_VALID(p_dead_creature_);

	// ��Ҫ�õ�������
	DWORD	dwReuseID		= p_dead_creature_->GetID();
	DWORD	dwSSpawnPtID	= p_dead_creature_->GetSpawnPtID();
	DWORD	dwSpawnGroupID	= p_dead_creature_->GetSpawnGroupID();
	Vector3	vPos			= p_dead_creature_->GetBornPos();
	Vector3 vFase			= p_dead_creature_->GetBornFace();
	BOOL	bCollide		= p_dead_creature_->NeedCollide();

	// �Ҹ��µ�
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


	// ɾ���ɵ�
	map_respawn_creature.erase(p_dead_creature_->GetID());
	Creature::Delete(p_dead_creature_);

	// �����µ�
	p_dead_creature_ = Creature::Create(dwReuseID, get_map_id(), this, pCreatureProto, vPos, vFase, ECST_SpawnPoint, dwSSpawnPtID, dwSpawnGroupID, bCollide, NULL);

	// ���
	add_creature(p_dead_creature_);

	p_dead_creature_->OnScriptLoad();
}

//-----------------------------------------------------------------------------------------
// ˢ�µ���ˢ��
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
	// �����ɹ�
	map_respawn_creature.erase(p_dead_creature_->GetID());
	add_creature(p_dead_creature_);
	//pMapSpawnInfo->b_hasUnit = TRUE;
	map_point_has_list[pMapSpawnInfo->dw_att_id] = TRUE;

}
//-----------------------------------------------------------------------------------------
// ��̬��������
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
// ��̬��������
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
// ɾ������(����map��ȫ���⣬��Ҫ�ô˺���lc)
//-----------------------------------------------------------------------------------------
VOID Map::delete_creature(DWORD dw_id_)
{
	Creature* pCreature = find_creature(dw_id_);
	if (!VALID_POINT(pCreature))	return;

	map_creature.erase(dw_id_);

	// �ӿ��ӵ�ש�����ߣ�����ͬ���ͻ���
	remove_from_visible_tile(pCreature);

	// ͬ�����ͻ���
	synchronize_remove_unit_to_big_visible_tile(pCreature);

	Creature::Delete(pCreature);
}

//------------------------------------------------------------------------------------------
// Ǳ��
//------------------------------------------------------------------------------------------
VOID Map::lurk(Unit *p_unit_)
{
	ASSERT(VALID_POINT(p_unit_));

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	if(nVisIndex == INVALID_VALUE)
		return;

	// �����������λ�б�
	p_visible_tile[nVisIndex].map_invisible_unit.add(p_unit_->GetID(), p_unit_);

	// ͬ���������б������
	NET_SIS_remove_remote sendRemove;
	sendRemove.dw_role_id[0] = p_unit_->GetID();

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);
	
	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// �ҵ�ÿ����ש����
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.�Ƿ���ȫ�ɼ�
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			ASSERT(!pRole->IsInVisList(p_unit_->GetID()));

			// 2.B�Ƿ���̽�����������Ƿ��ڿ��ӷ�Χ��
			if( !pRole->HasDectiveSkill() || !pRole->IsInVisDist(p_unit_->GetID()) )
			{
				pRole->SendMessage(&sendRemove, sendRemove.dw_size);
				continue;
			}

			// 3 ��B���ӷ�Χ��
			pRole->Add2VisList(p_unit_->GetID());
		}
	}
}

//------------------------------------------------------------------------------------------
// Ǳ��״̬��ʧ -- ����
//------------------------------------------------------------------------------------------
VOID Map::unlurk(Unit *p_unit_)
{
	ASSERT(VALID_POINT(p_unit_));

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT(VALID_VALUE(nVisIndex));

	// �Ӹ�������λ�б���ȥ��
	p_visible_tile[nVisIndex].map_invisible_unit.erase(p_unit_->GetID());

	// ͬ���������б������
	BYTE	byMsg[1024] = {0};
	DWORD	dw_size = calculate_movement_message(p_unit_, byMsg, 1024);

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(nVisIndex, pVisTile);

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// �ҵ�ÿ����ש����
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.�Ƿ���ȫ�ɼ�
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			// 2.B�Ƿ���̽�����������Ƿ��ڿ��ӷ�Χ��
			if( !pRole->HasDectiveSkill() || !pRole->IsInVisDist(p_unit_->GetID()) )
			{
				pRole->SendMessage(byMsg, dw_size);
				continue;
			}

			// 3 ��B���ӷ�Χ��
			pRole->RemoveFromVisList(p_unit_->GetID());
		}
	}
}

//------------------------------------------------------------------------------------------
// Ǳ�е�λ�ƶ�������ͬ�������ӷ�Χ�����
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

		// �ҵ�ÿ����ש����
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( !VALID_POINT(pRole) )
				continue;

			// 1.�Ƿ���ȫ�ɼ�
			if( pRole->CanSeeTargetEntirely(p_unit_) )
			{
				continue;
			}

			// 2.B�Ƿ���̽������
			if( !pRole->HasDectiveSkill() )
				continue;

			// 3.��B�Ŀ����б���
			if( pRole->IsInVisList(p_unit_->GetID()) )
			{
				// ����B���ӷ�Χ��
				if( !pRole->IsInVisDist(p_unit_->GetID()) )
				{
					pRole->SendMessage(&sendRemove, sendRemove.dw_size);
					pRole->RemoveFromVisList(p_unit_->GetID());
				}

				continue;
			}

			// 4.����B�Ŀ����б���
			// 4.1����B���ӷ�Χ��
			if( !pRole->IsInVisDist(p_unit_->GetID()) )
				continue;

			// 4.2 ��B���ӷ�Χ��
			pRole->SendMessage(byMsg, dw_size);
			pRole->Add2VisList(p_unit_->GetID());
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
// ����ΧǱ�е�λͬ���������
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

Pet* Map::find_pet( DWORD dw_id_ ) /* ?���ڵ�ͼ�߳�֮��ʹ�� */
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

	// �����⴦�� ����������б�	
	if ( IS_DOOR(p_unit_->GetID()) )
		return;

	INT nVisIndex = p_unit_->GetVisTileIndex();
	ASSERT( VALID_VALUE(nVisIndex) );

	if( p_unit_->IsRole() )
	{
		p_visible_tile[nVisIndex].map_leave_role.erase(p_unit_->GetID());
	}
}

// �ж�Ŀ����Ƿ�����
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

		// ���ȼ������
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			// ������ǰ�������
			FLOAT fX1 = pRole->GetCurPos().x;
			FLOAT fZ1 = pRole->GetCurPos().z;

			if (fX1/TILE_SCALE == piont.x/TILE_SCALE && fZ1/TILE_SCALE == piont.z/TILE_SCALE)
				return false;

		}

		// �ټ������  
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
		
			// ������ǰ�������
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

	// �������б��������������
	map_creature.erase(p_creature->GetID());

	p_creature->SetMap(NULL);

	// �ӿ��ӵ�ש�����ߣ�����ͬ���ͻ���
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
