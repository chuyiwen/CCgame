/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//帮会副本 

#include "StdAfx.h"
#include "map_instance_guild.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"
#include "guild.h"
#include "guild_manager.h"
#include "role_mgr.h"
#include "sspawnpoint_define.h"

map_instance_guild::map_instance_guild() : map_instance(), m_p_instance(NULL), m_dw_creator_id(INVALID_VALUE),
							m_dw_start_tick(INVALID_VALUE), m_dw_end_tick(INVALID_VALUE),
							m_b_no_enter(TRUE), m_dw_guild_id(INVALID_VALUE),m_dw_boss_id(INVALID_VALUE),
							m_dw_lobby_id(INVALID_VALUE),m_bInWar(false),m_bEnd(false)
{	
	m_pGuild = NULL;
	m_pGuildAck = NULL;
}

map_instance_guild::~map_instance_guild()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// 初始化
//------------------------------------------------------------------------------------------------------
BOOL map_instance_guild::init(const tag_map_info* p_info_, DWORD dw_instance_id_, Role* p_creator_, DWORD dw_misc_)
{	

	ASSERT( VALID_POINT(p_info_) );
	ASSERT( EMT_Guild == p_info_->e_type );

	if( !VALID_POINT(p_creator_) ) return FALSE;	// 必须要有创建者的

	// 读取副本静态属性
	m_p_instance = AttRes::GetInstance()->get_instance_proto(p_info_->dw_id);
	if( !VALID_POINT(m_p_instance) )	return FALSE;

	// 地图相关属性
	p_map_info = p_info_;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();

	// 副本相关属性
	dw_id = p_map_info->dw_id;
	dw_instance_id = dw_instance_id_;
	m_dw_creator_id = p_creator_->GetID();
	//m_eInstanceHardMode = (EInstanceHardMode)dwMisc;
	m_dw_start_tick = g_world.GetWorldTick();
	m_dw_end_tick = INVALID_VALUE;

	p_npc_team_mgr = new NPCTeamMgr(this);
	if(!VALID_POINT(p_npc_team_mgr))
		return FALSE;

	// 初始化寻路系统
	//if (!initSparseGraph(p_info_))
	//	return FALSE;


	// 根据mapinfo来初始化地图的怪物，可视列表等信息
	/*if( FALSE == init_logical_info() )
	{
		SAFE_DELETE(m_pNav);
		return FALSE;
	}*/
	m_bEnd = false;
	// 进入敌对帮会副本
	if(dw_misc_ == 1)
	{
		if(!VALID_POINT(p_creator_))
			return FALSE;

		if( !VALID_POINT(p_creator_) )
			return FALSE;
		DWORD dw_guild_id = p_creator_->GetGuildID();
		if( !VALID_POINT(dw_guild_id) )
			return FALSE;
		guild* p_guild = g_guild_manager.get_guild(dw_guild_id);
		if( !VALID_POINT(p_guild) )
			return FALSE;	

		if(p_guild->get_guild_war().get_guild_war_state() != EGWS_WAR &&
			p_guild->get_guild_war().get_guild_war_state() != EGWS_WAR_relay)
			return FALSE;
		
		m_bInWar = TRUE;
		
		guild* p_enemy_guild = g_guild_manager.get_guild(p_guild->get_guild_war().get_war_guild_id());
		if(!VALID_POINT(p_enemy_guild))
			return FALSE;
		
		m_pGuild = p_enemy_guild;
		m_pGuildAck = p_guild;
		// 根据mapinfo来初始化地图的怪物，可视列表等信息
		if( FALSE == init_logical_info(p_enemy_guild->get_guild_att().dwID) )
		{
			return FALSE;
		}

		p_enemy_guild->set_guild_instance_id(dw_instance_id_);
		m_dw_guild_id = p_enemy_guild->get_guild_att().dwID;
		init_guild_build(p_enemy_guild, TRUE);
		//init_guild_relive_pos(p_enemy_guild, m_p_instance);
		//init_guild_enlistee(p_enemy_guild, TRUE);
		//on_guild_relay_war(p_enemy_guild);

	}
	else
	{
		if( !VALID_POINT(p_creator_) )
			return FALSE;
		DWORD dw_guild_id = p_creator_->GetGuildID();
		if( !VALID_POINT(dw_guild_id) )
			return FALSE;
		guild* p_guild = g_guild_manager.get_guild(dw_guild_id);
		if( !VALID_POINT(p_guild) )
			return FALSE;	
		
		m_pGuild = p_guild;

		// 根据mapinfo来初始化地图的怪物，可视列表等信息
		if( FALSE == init_logical_info(p_guild->get_guild_att().dwID) )
		{
			return FALSE;
		}


		p_guild->set_guild_instance_id(dw_instance_id_);
		m_dw_guild_id = dw_guild_id;
		if(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR ||
			p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay)
		{
			m_bInWar = TRUE;
			
			guild* p_enemy_guild = g_guild_manager.get_guild(p_guild->get_guild_war().get_war_guild_id());
			if(!VALID_POINT(p_enemy_guild))
				return FALSE;

			m_pGuildAck = p_enemy_guild;

			//on_guild_relay_war(m_pGuild);

		}
		else
		{
			m_bInWar = FALSE;
		}
		
		init_guild_build(p_guild, m_bInWar);
		
		//init_guild_relive_pos(p_guild, m_p_instance);

		//init_guild_enlistee(p_guild, m_bInWar);
	}

	return TRUE;	
}

//---------------------------------------------------------------------------------
// 初始帮派建筑
//---------------------------------------------------------------------------------
VOID map_instance_guild::init_guild_build(guild* p_guild_, BOOL b_war_)
{
	GuildUpgrade& st_grade = p_guild_->get_guild_facilities();

	for(INT i = EFT_Lobby; i < EFT_End; i++)
	{
		INT n_level = st_grade.GetFacilitiesLevel(i);

		if(n_level <= 0)
			continue;

		if(st_grade.IsFacilitesDestory((EFacilitiesType)i))
			continue;

		const s_guild_grade_pos* p_guild_grade_pos = AttRes::GetInstance()->GetGuildGradePosInfo(i, n_level);
		if(!VALID_POINT(p_guild_grade_pos) || p_guild_grade_pos->dw_creature_type_id_ == INVALID_VALUE)
			continue;

		Vector3 vPos;
		Vector3 vFaceTo;

		if(!b_war_)
		{
			
			const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_guild_grade_pos->dw_npc_way_pos);
			if(!VALID_POINT(p_list))
				continue;
			
			int j = 0;
			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			p_list->list.reset_iterator();
			while (p_list->list.find_next(wayPoint) && j < MAX_GUILD_UPGRADE_NUM)
			{


				Vector3 v_face;
				v_face.y = 0;
				v_face.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
				v_face.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
				
				Creature* p_creature = NULL;

				if(i == EFT_Holiness)
				{
					p_creature = this->create_creature(p_guild_grade_pos->dw_npc_type_id, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID, 
																INVALID_VALUE, FALSE, _T("shengshou"));
				}
				else
				{
					p_creature = this->create_creature(p_guild_grade_pos->dw_npc_type_id, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID);
				}

				// 如果是商店职能NPC，则挂载对应的商店
				if (ECT_NPC == p_creature->GetProto()->eType)
				{
					switch (p_creature->GetProto()->eFunctionType)
					{
					case EFNPCT_Shop:
						add_shop(p_creature->GetTypeID(), p_creature->GetShopID());
						break;

					case EFNPCT_CofC:
						add_chamber(p_creature->GetTypeID(), p_creature->GetShopID());
						break;
					}
				}

				if(VALID_POINT(p_creature))
				{
					st_grade.SetFacilitiesID(i, p_creature->GetID(), j);
					if(i == EFT_Lobby)
					{
						set_lobby_id(p_creature->GetID());
						if(st_grade.IsLobbyUsing())
							p_creature->SetState(ES_Occupied);
					}
				}
				j++;
			}

		}
		else
		{
			const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_guild_grade_pos->dw_npc_way_pos);
			if(!VALID_POINT(p_list))
				continue;

			int j = 0;
			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			p_list->list.reset_iterator();
			while (p_list->list.find_next(wayPoint) && j < MAX_GUILD_UPGRADE_NUM)
			{

				Vector3 v_face;
				v_face.y = 0;
				v_face.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
				v_face.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
				Creature* p_creature = NULL;

				if(i == EFT_Holiness)
				{
					p_creature = this->create_creature(p_guild_grade_pos->dw_creature_type_id_, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID, 
													INVALID_VALUE, FALSE, _T("shengshou"));
				}
				else
				{
					p_creature = this->create_creature(p_guild_grade_pos->dw_creature_type_id_, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID);
				}
				
				if(VALID_POINT(p_creature))
				{
					st_grade.SetFacilitiesID(i, p_creature->GetID(), j);

					if(p_creature->IsTeam())
					{
						set_boss_id(p_creature->GetID());
						init_team_creature(p_creature, p_guild_);
					}
				}
				j++;
			}
		}
	}

	// 得到目标地图的导航点
	const tag_map_info* p_map_info = get_map_info();
	if( !VALID_POINT(p_map_info) ) return ;


	const tag_map_way_point_info_list* p_list = p_map_info->map_way_point_list.find(m_p_instance->dw_guild_flag);
	if( !VALID_POINT(p_list) ) return ;

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	p_list->list.rand_find(wayPoint);

	Vector3 v_face;
	v_face.y = 0;
	v_face.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	v_face.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	create_creature(1051010, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID);

	//if (!b_war_)
	//{
	//	INT nIndex = 0;
	//	nIndex = init_guild_plant(p_guild_, _T("P1"), 4001050, nIndex);
	//	nIndex = init_guild_plant(p_guild_, _T("P2"), 4001051, nIndex);
	//	init_guild_plant(p_guild_, _T("P3"), 4001052, nIndex);
	//	init_guild_daogao(p_guild_);
	//}
}

//---------------------------------------------------------------------------------
// 初始单个帮派建筑
//---------------------------------------------------------------------------------
VOID map_instance_guild::init_guild_build(guild* p_guild_, BYTE type_, BOOL b_war_)
{
	GuildUpgrade& st_grade = p_guild_->get_guild_facilities();

	INT n_level = st_grade.GetFacilitiesLevel(type_);

	if(n_level <= 0)
		return;

	const s_guild_grade_pos* p_guild_grade_pos = AttRes::GetInstance()->GetGuildGradePosInfo(type_, n_level);
	if(!VALID_POINT(p_guild_grade_pos) || p_guild_grade_pos->dw_creature_type_id_ == INVALID_VALUE)
		return;

	Vector3 v_pos;
	Vector3 v_faceto;

	if(!b_war_)
	{
		const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_guild_grade_pos->dw_npc_way_pos);
		if(!VALID_POINT(p_list))
			return;

		int j = 0;
		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		p_list->list.reset_iterator();
		while (p_list->list.find_next(wayPoint) && j < MAX_GUILD_UPGRADE_NUM)
		{
			Vector3 vFace;
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			
			Creature* p_creature = NULL;

			if(type_ == EFT_Holiness)
			{
				p_creature = this->create_creature(p_guild_grade_pos->dw_npc_type_id, wayPoint.v_pos, vFace, ECST_Guild, p_guild_->get_guild_att().dwID, 
															INVALID_VALUE, FALSE, _T("shengshou"));
			}
			else
			{
				p_creature = this->create_creature(p_guild_grade_pos->dw_npc_type_id, wayPoint.v_pos, vFace, ECST_Guild, p_guild_->get_guild_att().dwID);
			}

			// 如果是商店职能NPC，则挂载对应的商店
			if (ECT_NPC == p_creature->GetProto()->eType)
			{
				switch (p_creature->GetProto()->eFunctionType)
				{
				case EFNPCT_Shop:
					add_shop(p_creature->GetTypeID(), p_creature->GetShopID());
					break;

				case EFNPCT_CofC:
					add_chamber(p_creature->GetTypeID(), p_creature->GetShopID());
					break;
				}
			}

			if(VALID_POINT(p_creature))
			{
				st_grade.SetFacilitiesID(type_, p_creature->GetID(), j);
				if(type_ == EFT_Lobby)
				{
					set_lobby_id(p_creature->GetID());
					if(st_grade.IsLobbyUsing())
						p_creature->SetState(ES_Occupied);
				}
			}
			j++;
		}
	}
	else
	{
		const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_guild_grade_pos->dw_npc_way_pos);
		if(!VALID_POINT(p_list))
			return;

		int j = 0;
		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		p_list->list.reset_iterator();
		while (p_list->list.find_next(wayPoint) && j < MAX_GUILD_UPGRADE_NUM)
		{
			Vector3 vFace;
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

			Creature* p_creature = NULL;

			if(type_ == EFT_Holiness)
			{
				p_creature = this->create_creature(p_guild_grade_pos->dw_creature_type_id_, wayPoint.v_pos, vFace, ECST_Guild, p_guild_->get_guild_att().dwID, 
												INVALID_VALUE, FALSE, _T("shengshou"));
			}
			else
			{
				p_creature = this->create_creature(p_guild_grade_pos->dw_creature_type_id_, wayPoint.v_pos, vFace, ECST_Guild, p_guild_->get_guild_att().dwID);
			}
			
			if(VALID_POINT(p_creature))
			{
				st_grade.SetFacilitiesID(type_, p_creature->GetID(), j);

				set_boss_id(p_creature->GetID());

				if(p_creature->IsTeam())
				{
					init_team_creature(p_creature, p_guild_);
				}
			}
			j++;
		}
	}
}

//INT map_instance_guild::init_guild_plant(guild* p_guild, TCHAR* szPointName, DWORD dwCreatureID, INT nCurIndex)
//{
//	const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(get_tool()->crc32(szPointName));
//	if(!VALID_POINT(p_list))
//		return nCurIndex;
//	
//	int i = nCurIndex;
//	// 从目标导航点列表中任取一个导航点
//	tag_way_point_info wayPoint;
//	p_list->list.reset_iterator();
//	while (p_list->list.find_next(wayPoint))
//	{
//		Vector3 vFace;
//		vFace.y = 0;
//		vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
//		vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
//
//		Creature* p_creature = NULL;
//
//		p_creature = this->create_creature(dwCreatureID, wayPoint.v_pos, vFace, ECST_Load, p_guild->get_guild_att().dwID);
//		if (!VALID_POINT(p_creature))
//			continue;
//
//		p_creature->SetPlantDataIndex(i);
//		p_creature->OnDisappear();
//
//		tagPlantData* pPlant = p_guild->getPlantData(i);
//		if (VALID_POINT(pPlant) && pPlant->dw_npc_id != 0)
//		{
//			Creature* p_plant= NULL;
//			p_plant = this->create_creature(pPlant->dw_npc_id, wayPoint.v_pos, vFace, ECST_Dynamic, p_guild->get_guild_att().dwID);
//			if (VALID_POINT(p_plant))
//			{
//				p_plant->SetPlantDataIndex(i);
//				p_plant->SetTuDui(p_creature->GetID());
//				if (dwCreatureID == 4001050)
//				{
//					p_plant->SetMaxYield(50);
//				}
//				else if (dwCreatureID == 4001051)
//				{
//					p_plant->SetMaxYield(75);
//				}
//				else if (dwCreatureID == 4001052)
//				{
//					p_plant->SetMaxYield(100);
//				}
//				p_creature->SetUsed(TRUE);
//			}
//		}
//		i++;
//
//	}
//
//	return i;
//}

VOID map_instance_guild::init_guild_daogao(guild* p_guild)
{
	const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(get_tool()->crc32(_T("D")));
	if(!VALID_POINT(p_list))
		return;

	int i = 0;
	tag_way_point_info wayPoint;
	p_list->list.reset_iterator();
	while (p_list->list.find_next(wayPoint))
	{
		//if (i > 7)
		//	break;

		Vector3 vFace;
		vFace.y = 0;
		vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

		Creature* p_creature = NULL;
		
		if (p_guild->IsDaogaoActive(i))
		{
			if (i > 7 )
			{
				p_creature = this->create_creature(4001059, wayPoint.v_pos, vFace, ECST_Dynamic, p_guild->get_guild_att().dwID);	
			}
			else
			{
				p_creature = this->create_creature(4001058, wayPoint.v_pos, vFace, ECST_Dynamic, p_guild->get_guild_att().dwID);	
			}
			
		}
		//else
		//{
		//	if (i < min(8, p_guild->get_upgrade().GetFacilitiesLevel(EFT_Lobby)+1))
		//	{
		//		p_creature = this->create_creature(4001260, wayPoint.v_pos, vFace, ECST_Dynamic, p_guild->get_guild_att().dwID);
		//		p_creature->SetPlantDataIndex(i);
		//	}

		//}
		

		i++;
	}

}
//---------------------------------------------------------------------------------
// 初始帮派复活点
//---------------------------------------------------------------------------------
VOID map_instance_guild::init_guild_relive_pos(guild* p_guild_, const tagInstance* p_instance_)
{
	if(!VALID_POINT(p_guild_) || !VALID_POINT(p_instance_))
		return ;

	for(INT i = 0; i < MAX_GUILD_RELIVE_NUM; i++)
	{
		const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_instance_->dwRelivePos[i]);
		if(!VALID_POINT(p_list))
			continue;

		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		p_list->list.rand_find(wayPoint);

		Vector3 v_face;
		v_face.y = 0;
		v_face.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		v_face.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		

		Creature* p_creature = this->create_creature(2000001, wayPoint.v_pos, v_face, ECST_Guild, p_guild_->get_guild_att().dwID);
		if(!VALID_POINT(p_creature))
			continue;

		p_guild_->get_guild_war().set_relive_id(p_creature->GetID(), TRUE);
	}
}

//---------------------------------------------------------------------------------
// 初始帮派士兵
//---------------------------------------------------------------------------------
VOID map_instance_guild::init_guild_enlistee(guild* p_guild_, BOOL b_war_)
{
	if(!VALID_POINT(p_guild_))
		return;

	if(!b_war_)
		return;

	if(p_guild_->get_upgrade().IsFacilitesDestory(EFT_Maidan))
		return;

	INT n_level = p_guild_->get_upgrade().GetFacilitiesLevel(EFT_Maidan);

	if(n_level <= 0)
		return ;

	const s_guild_enlistee_pos* p_guild_enlistee_pos = AttRes::GetInstance()->GetGuildEnlisteePos(n_level);
	if(!VALID_POINT(p_guild_enlistee_pos))
		return ;

	for(INT i = 0; i < MAX_GUILD_ENLISTEE_NUM; i++)
	{
		if(p_guild_enlistee_pos->dw_creature_id_[i] == INVALID_VALUE)
			continue;

		const tag_map_way_point_info_list* p_list = get_map_info()->map_way_point_list.find(p_guild_enlistee_pos->dw_way_pos[i]);
		if(!VALID_POINT(p_list))
			continue;

		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		p_list->list.rand_find(wayPoint);

		Vector3 vFace;
		vFace.y = 0;
		vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
		vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

		Creature* pCreature = this->create_creature(p_guild_enlistee_pos->dw_creature_id_[i], wayPoint.v_pos, vFace, ECST_Maidan, p_guild_->get_guild_att().dwID);
	}
}

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID map_instance_guild::update()
{
	if( map_role.empty() )
		set_end();

	Map::update();
	update_time_issue();
}

//---------------------------------------------------------------------------------
// 时限相关的更新
//---------------------------------------------------------------------------------
VOID map_instance_guild::update_time_issue()
{
	// 如果已经处于待删除状态，就不更新了
	if( is_delete() ) return;

	// 时限副本
	if( is_time_limit() && !is_end() )
	{
		DWORD dw_tick = g_world.GetWorldTick();
		if( (dw_tick - m_dw_start_tick) >= m_p_instance->dwTimeLimit * TICK_PER_SECOND )
		{
			m_dw_end_tick = g_world.GetWorldTick();
			set_end();
		}
	}

	// 关闭倒计时
	if( is_end() )
	{
		DWORD dw_tick = g_world.GetWorldTick();
		if( (dw_tick - m_dw_end_tick) > m_p_instance->dwEndTime * TICK_PER_SECOND || m_bInWar)
		{
			set_delete();
		}
	}

	// 更新所有待退出的角色的时间
	if( !m_map_will_out_role_id.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_map_will_out_role_id.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT n_tick = INVALID_VALUE;

		while( m_map_will_out_role_id.find_next(it, dw_role_id, n_tick) )
		{
			--n_tick;	// 减一下倒计时
			if( n_tick <= 0 )
			{
				// 时间到了，将玩家传送出去
				Role* p_role = find_role(dw_role_id);
				if( VALID_POINT(p_role) )
				{
					MapMgr* p_map_manager = g_mapCreator.get_map_manager(p_map_info->dw_id);
					if( VALID_POINT(p_map_manager) )
					{
						p_map_manager->RoleInstanceOut(p_role);
					}
				}

				m_map_will_out_role_id.erase(dw_role_id);
			}
			else
			{
				m_map_will_out_role_id.change_value(dw_role_id, n_tick);
			}
		}
	}
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID map_instance_guild::destroy()
{
	if( !VALID_POINT(m_dw_guild_id) )
		return;

	guild* p_guild = g_guild_manager.get_guild(m_dw_guild_id);
	if( VALID_POINT(p_guild) )
	{
		//p_guild->set_guild_instance_id(INVALID_VALUE);
		p_guild->get_guild_war().zero_relive_id();
		p_guild->get_upgrade().CleanFacilitiesID();

		if(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR)
		{
			guild* p_enemy_guild = g_guild_manager.get_guild(p_guild->get_guild_war().get_war_guild_id());
			if(VALID_POINT(p_enemy_guild))
			{
				p_enemy_guild->get_guild_war().zero_relive_id();
			}
		}
	}	
}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID map_instance_guild::add_role(Role* p_role_,  Map* pSrcMap)
{
	Map::add_role(p_role_, pSrcMap);

	// 重置关闭等待
	if( is_end() )
	{
		m_dw_end_tick = INVALID_VALUE;
		b_end = FALSE;
	}

	guild* p_guild = g_guild_manager.get_guild(m_dw_guild_id);
	if(!VALID_POINT(p_guild))
		return;
	
	// 帮会战时,增加人数
	if (m_bInWar)
	{
		p_role_->set_in_guild_war(true);
		if (p_role_->GetGuildID() == m_pGuild->get_guild_att().dwID)
		{
			m_pGuild->get_guild_war().add_member_number();
			p_role_->set_defender(true);
			const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(4200301);
			if (VALID_POINT(pBuffProto))
			{
				p_role_->TryAddBuff(p_role_, pBuffProto, NULL, NULL, NULL);
			}
			
			send_war_number_to_guild();
		}

		if (p_role_->GetGuildID() == m_pGuildAck->get_guild_att().dwID)
		{
			m_pGuildAck->get_guild_war().add_member_number();
			p_role_->set_defender(false);
			const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(4200201);
			if (VALID_POINT(pBuffProto))
			{
				p_role_->TryAddBuff(p_role_, pBuffProto, NULL, NULL, NULL);
			}
			send_war_number_to_guild();
		}

		p_role_->SetPKState(ERolePK_Guild);
		NET_SIS_change_pk_state send;
		send.dw_role_id = p_role_->GetID();
		send.ePKState = ERolePK_Guild;
		send.dwError = E_Success;
		p_role_->SendMessage(&send, send.dw_size);
	}
	// 发送进入副本消息
	NET_SIS_enter_instance send;
	send.dw_error_code = E_Success;
	send.dwTimeLimit = cal_time_limit();
	//send.dwGuildSymbol = p_guild->get_guild_att().dwValue;
	//memcpy(send.n_family_name, p_guild->get_guild_att().n_family_name, sizeof(send.n_family_name));
	//memcpy(send.n_name, p_guild->get_guild_att().n_name, sizeof(send.n_name));
	//memcpy(send.szText, p_guild->get_guild_att().szText, sizeof(send.szText));
	p_role_->SendMessage(&send, send.dw_size);
}

//---------------------------------------------------------------------------------
// 玩家离开地图，只可能在主线程里面调用
//---------------------------------------------------------------------------------
VOID map_instance_guild::role_leave_map(DWORD dw_id_, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dw_id_);

	// 是否进入等待关闭
	if( map_role.empty() && !is_end() && m_p_instance->dwEndTime != INVALID_VALUE )
	{
		m_dw_end_tick = g_world.GetWorldTick();
		b_end = TRUE;
	}
	
	if (m_bInWar)
	{
		Role* pRole = g_roleMgr.get_role(dw_id_);
		if (!VALID_POINT(pRole))
			return ;
		
		pRole->set_in_guild_war(false);

		if (VALID_POINT(m_pGuild) && pRole->GetGuildID() == m_pGuild->get_guild_att().dwID)
		{
			m_pGuild->get_guild_war().sub_member_number();
			
			send_war_number_to_guild();

			if (m_pGuild->get_guild_war().get_guild_war_state() == EGWS_WAR)
			{
				if (m_pGuild->get_guild_war().get_member_number() <= 0)
				{
					m_pGuild->get_guild_war().m_bLost = TRUE;
					m_pGuildAck->get_guild_war().m_bWin = TRUE;
				}
			}

		}

		if (VALID_POINT(m_pGuildAck) && pRole->GetGuildID() == m_pGuildAck->get_guild_att().dwID)
		{
			m_pGuildAck->get_guild_war().sub_member_number();
			
			send_war_number_to_guild();

			if (m_pGuildAck->get_guild_war().get_guild_war_state() == EGWS_WAR)
			{
				if (m_pGuildAck->get_guild_war().get_member_number() <= 0)
				{
					m_pGuild->get_guild_war().m_bWin = TRUE;
					m_pGuildAck->get_guild_war().m_bLost = TRUE;
				}
			}
		}

		ERolePKState ers = ERolePK_Peace;
		//if (pRole->IsYellowName())
		//{
		//	ers = ERolePK_All;
		//}
		pRole->SetPKState(ers);
		NET_SIS_change_pk_state send;
		send.dw_role_id = pRole->GetID();
		send.ePKState = ers;
		send.dwError = E_Success;
		pRole->SendMessage(&send, send.dw_size);
	}

	// 如果这个玩家在等待离开的列表里，则移除
	m_map_will_out_role_id.erase(dw_id_);

	
}
//---------------------------------------------------------------------------------
// 是否能进入副本
//---------------------------------------------------------------------------------
INT map_instance_guild::can_enter(Role *p_role_, DWORD dw_param_)
{

	// 先检测通用判断
	INT n_error_code = map_instance::can_enter(p_role_);
	if( E_Success != n_error_code ) return n_error_code;
	if( p_role_->IsRedName( ) /*|| p_role_->IsPurpureName( )*/ ) return E_Instance_RedName;

	// 检查人数上限
	if( m_p_instance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;
	
	// 帮会战结束后不能再进入
	if (m_bEnd)
		return E_Instance_Act_NoBegin;

	// 家族属性
	if( m_dw_guild_id != INVALID_VALUE )
	{
		guild* p_guild = g_guild_manager.get_guild(m_dw_guild_id);
		if( VALID_POINT(p_guild) )
		{
			if(dw_param_ == 1)
			{
				if(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay ||
					p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR)
				{
					guild* p_enemy_guild = g_guild_manager.get_guild(p_guild->get_guild_war().get_war_guild_id());
					if(!VALID_POINT(p_enemy_guild))
						return E_Instance_Not_Exit;
					
					if(p_guild->get_guild_war().get_war_guild_id() != p_enemy_guild->get_guild_att().dwID)
						return E_Instance_Not_Enemy_Guild;

					// 不在名单中不能进入,人数满了不能进入
					tagGuildMember* pMember = p_enemy_guild->get_member(p_role_->GetID());
					if (!VALID_POINT(pMember) || !pMember->bWar || 
						p_enemy_guild->get_guild_war().get_member_number() >= p_enemy_guild->get_guild_war().get_max_number())
					{
						return E_Instance_Lobby_Repair;
					}

				}

			}
			else
			{
				// 不是一个帮会的
				if( p_guild->get_guild_att().dwID != p_role_->GetGuildID() )
				{
					return E_Instance_Not_Same_Guild;
				}
				
				// 进攻方在帮会在战只能进敌对帮会
				if(p_guild->get_guild_war().get_declare_guild_id() == INVALID_VALUE &&
					(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay ||
					p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR))
				{
					return E_Instance_Act_NoBegin;
				}

				if(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR_relay ||
					p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR)
				{
					// 不在名单中不能进入,人数满了不能进入
					tagGuildMember* pMember = p_guild->get_member(p_role_->GetID());
					if (!VALID_POINT(pMember) || !pMember->bWar || 
						p_guild->get_guild_war().get_member_number() >= p_guild->get_guild_war().get_max_number())
					{
						return E_Instance_Lobby_Repair;
					}
				}


			}

			// 帮会战时不能进入
			//if(p_guild->get_guild_war().get_guild_war_state() == EGWS_WAR)
			//{
			//	return E_Instance_Act_NoBegin;
			//}
			
		}
		else
		{
			return E_Instance_Not_Exit;
		}
	}
	else
	{
		return E_Instance_Not_Exit;
	}

	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL map_instance_guild::can_destroy()
{
	//// 帮会战时,不删除
	//if (m_bInWar)
	//	return false;

	return map_instance::can_destroy();
}


//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL map_instance_guild::init_all_spawn_point_creature(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
//	// 对每一个刷怪点	
//	tag_map_spawn_point_info* pMapSpawnInfo = NULL;
//	p_map_info->map_spawn_point.reset_iterator();
//	while(p_map_info->map_spawn_point.find_next(pMapSpawnInfo))
//	{
//		ASSERT_P_VALID(pMapSpawnInfo);
//		if (!VALID_POINT(pMapSpawnInfo)) continue;
//
//		// 随机选择一个怪
//		const tagSSpawnPointProto* pSSpawnProto = AttRes::GetInstance()->GetSSpawnPointProto(pMapSpawnInfo->dw_att_id);
//		if (!VALID_POINT(pSSpawnProto)) continue;
//
//		INT nRand = get_tool()->tool_rand() % 10000;
//		DWORD dwNewTypeID = INVALID_VALUE;
//
//		INT nCandiNum	= 0;
//		INT32 nPct = 0;
//		while (VALID_VALUE(pSSpawnProto->dwTypeIDs[nCandiNum]))
//		{
//			DWORD dwCurType = pSSpawnProto->dwTypeIDs[nCandiNum];
//			nCandiNum++;
//			const tagGuildSSpawnPointProto* pSSpawnGuild = AttRes::GetInstance()->GetSSpawnGuildProto(dwCurType);
//			if (!VALID_POINT(pSSpawnGuild))
//				continue;
//
//			if (!VALID_POINT(m_pGuild))
//				continue;
//
//			nPct += pSSpawnGuild->n_pro[m_pGuild->get_guild_att().byLevel-1];
//			if (nRand < nPct)
//			{
//				dwNewTypeID = dwCurType;
//				break;
//			}
//
//		}
//
//
//		const tagCreatureProto* pCreatureProto = AttRes::GetInstance()->GetCreatureProto(dwNewTypeID);
//		ASSERT_P_VALID(pCreatureProto);
//		if (!VALID_POINT(pCreatureProto)) continue;
//
//		// 获取id
//		DWORD dwCreatureID = builder_creature_id.get_new_creature_id();
//
//		// 生成出生坐标和出生朝向
//		FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
//		Vector3 vFaceTo;
//		vFaceTo.x = cosf(fYaw * 3.1415927f / 180.0f);
//		vFaceTo.z = sinf(fYaw * 3.1415927f / 180.0f);
//		vFaceTo.y = 0.0f;
//
//		// 创建怪
//		Creature* pCreature = Creature::Create(	dwCreatureID, get_map_id(), this, pCreatureProto, pMapSpawnInfo->v_pos, vFaceTo, 
//			ECST_SpawnPoint, pMapSpawnInfo->dw_att_id, INVALID_VALUE, pMapSpawnInfo->b_collide, NULL, INVALID_VALUE, dw_guild_id_);
//		ASSERT_P_VALID(pCreature);
//		if (!VALID_POINT(pCreature)) continue;
//
//		// 放入地图
//		add_creature_on_load(pCreature);
//		pCreature->OnDisappear();
//	}
//
	return Map::init_all_spawn_point_creature(dw_guild_id_);
}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID map_instance_guild::on_delete()
{
	// 移除所有在地图内的玩家
	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = (Role*)INVALID_VALUE;
	
	guild* p_guild = g_guild_manager.get_guild(m_dw_guild_id);
	if( VALID_POINT(p_guild) )
	{
		p_guild->set_guild_instance_id(INVALID_VALUE);
	}

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		p_map_manager->RoleInstanceOut(p_role);
	}
}

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID map_instance_guild::on_destroy()
{
	
}


//-----------------------------------------------------------------------------------
// 计算时限副本所剩时间
//-----------------------------------------------------------------------------------
DWORD map_instance_guild::cal_time_limit()
{
	DWORD dw_time_left = INVALID_VALUE;
	if(m_p_instance->dwTimeLimit > 0 && m_p_instance->dwTimeLimit != INVALID_VALUE)
	{
		DWORD dw_current_tick = g_world.GetWorldTick();
		DWORD dw_time_pass = (dw_current_tick - m_dw_start_tick) / TICK_PER_SECOND;
		dw_time_left = m_p_instance->dwTimeLimit - dw_time_pass;
	}

	return dw_time_left;
}

//------------------------------------------------------------------------------------------------------
// 当家族解散
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_guild_dismiss(const guild* p_guild_)
{
	//将副本里所有角色传送出副本
	if( VALID_POINT(p_guild_) )
	{
		if( p_guild_->get_guild_att().dwID == this->get_guild_id() )
		{
			// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时			
			package_map<DWORD, Role*>::map_iter it = map_role.begin();
			Role* p_role = NULL;
			while (map_role.find_next(it, p_role))
			{
				if( VALID_POINT(p_role) )
				{
					m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);		
				}
			}				
			
		}
	}
}

//------------------------------------------------------------------------------------------------------
// 当帮会升级
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_guild_level_up(const guild* p_guild_)
{
	if(VALID_POINT(p_guild_))
	{
		if(p_guild_->get_guild_att().dwID == this->get_guild_id())
		{
			// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时			
			package_map<DWORD, Role*>::map_iter it = map_role.begin();
			Role* p_role = NULL;
			while (map_role.find_next(it, p_role))
			{
				if( VALID_POINT(p_role) )
				{
					m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);
					NET_SIS_inform_leave_guild_map send;
					send.byType = 1;
					p_role->SendMessage(&send, send.dw_size);
				}
			}				
		}
	}
}

//------------------------------------------------------------------------------------------------------
// 当玩家离开家族
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_role_leave_guild(DWORD dw_role_id_, const guild* p_guild_)
{
	if(m_dw_guild_id != p_guild_->get_guild_att().dwID) return;

	Role* p_role = find_role(dw_role_id_);
	if(VALID_POINT(p_role))
	{
		m_map_will_out_role_id.add(dw_role_id_, 5 * TICK_PER_SECOND);
	}
}

//------------------------------------------------------------------------------------------------------
// 当玩家加入家族
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_role_enter_guild(DWORD dw_role_id_, const guild* p_guild_)
{
	if(m_dw_guild_id != p_guild_->get_guild_att().dwID) return;

	Role* p_role = find_role(dw_role_id_);
	if(VALID_POINT(p_role))
	{
		m_map_will_out_role_id.erase(dw_role_id_);
	}
}

//------------------------------------------------------------------------------------------------------
// 进入战斗准备
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_guild_prepare_war(const guild* p_guild_)
{
	//if(VALID_POINT(p_guild_))
	//{
	//	if(p_guild_->get_guild_att().dwID == this->get_guild_id())
	//	{
	//		// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时			
	//		package_map<DWORD, Role*>::map_iter it = map_role.begin();
	//		Role* p_role = NULL;
	//		while (map_role.find_next(it, p_role))
	//		{
	//			if( VALID_POINT(p_role) )
	//			{
	//				m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);
	//				NET_SIS_inform_leave_guild_map send;
	//				send.byType = 2;
	//				p_role->SendMessage(&send, send.dw_size);
	//			}
	//		}				
	//	}
	//}
	set_delete();
}

//------------------------------------------------------------------------------------------------------
// 战斗结束
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_guild_end_war(const guild* p_guild_)
{
	//if(VALID_POINT(p_guild_))
	//{
	//	if(p_guild_->get_guild_att().dwID == this->get_guild_id())
	//	{
	//		// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时			
	//		package_map<DWORD, Role*>::map_iter it = map_role.begin();
	//		Role* p_role = NULL;
	//		while (map_role.find_next(it, p_role))
	//		{
	//			if( VALID_POINT(p_role) )
	//			{
	//				m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);
	//				NET_SIS_inform_leave_guild_map send;
	//				send.byType = 3;
	//				p_role->SendMessage(&send, send.dw_size);
	//			}
	//		}				
	//	}
	//}
	//set_in_war(false);
	//m_pGuild = NULL;
	//m_pGuildAck = NULL;
	m_bEnd = TRUE;
	p_script->GuildWarEnd(this);
	set_delete();
}

//------------------------------------------------------------------------------------------------------
// 帮会降级
//------------------------------------------------------------------------------------------------------
VOID map_instance_guild::on_guild_decline_level(const guild* p_guild_)
{
	if(VALID_POINT(p_guild_))
	{
		if(p_guild_->get_guild_att().dwID == this->get_guild_id())
		{
			// 如果该玩家目前在副本里，则设置玩家的离开副本倒计时			
			package_map<DWORD, Role*>::map_iter it = map_role.begin();
			Role* p_role = NULL;
			while (map_role.find_next(it, p_role))
			{
				if( VALID_POINT(p_role) )
				{
					m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);
					NET_SIS_inform_leave_guild_map send;
					send.byType = 4;
					p_role->SendMessage(&send, send.dw_size);
				}
			}				
		}
	}
}

//战斗准备开始
VOID map_instance_guild::on_guild_relay_war(const guild* p_guild_)
{
	p_script->GuildWarRelay(this);
}

//战斗开始
VOID map_instance_guild::on_guild_start_war(const guild* p_guild_)
{
	DWORD dwDefentNum = m_pGuild->get_guild_war().get_member_number();
	DWORD dwAttackNum = m_pGuildAck->get_guild_war().get_member_number();
	p_script->GuildWarStart(this, dwDefentNum, dwAttackNum, m_pGuild->get_guild_att().dwID, m_pGuildAck->get_guild_att().dwID);
}

VOID map_instance_guild::on_creature_die(Creature* p_creature_, Unit* p_killer_)
{
	if (m_bInWar)
	{
		////鼎炉死亡 
		//if (p_creature_->GetProto()->dw_data_id == 4001060)
		//{
		//	m_pGuild->get_guild_war().m_bLost = TRUE;
		//	m_pGuildAck->get_guild_war().m_bWin = TRUE;
		//}
		////带头大哥死亡
		//if (p_creature_->GetProto()->dw_data_id == 4001059)
		//{
		//	m_pGuild->get_guild_war().m_bWin = TRUE;
		//	m_pGuildAck->get_guild_war().m_bLost = TRUE;
		//}
	}	
	Map::on_creature_die(p_creature_, p_killer_);
}

VOID map_instance_guild::on_role_die(Role* p_role_, Unit* p_killer_)
{
	//if (p_role_->is_in_guild_war())
	//{
	//	m_map_will_out_role_id.add(p_role_->GetID(), 30 * TICK_PER_SECOND);

	//	NET_SIS_inform_leave_guild_map send;
	//	send.byType = 5;
	//	p_role_->SendMessage(&send, send.dw_size);
	//}

	Map::on_role_die(p_role_, p_killer_);
}

VOID map_instance_guild::on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
							  INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_)
{
	//m_map_will_out_role_id.erase(p_role_->GetID());
	
	Map::on_revive(p_role_, e_type_, n_revive_hp_, n_revive_mp_, fx_, fy_, fz_, dw_reborn_map_id_);
}

VOID map_instance_guild::send_war_number_to_guild()
{
	if (!VALID_POINT(m_pGuildAck) || !VALID_POINT(m_pGuild))
		return;
	
	// 同步人数
	NET_SIS_Guil_war_member_number_update send;
	send.dw_number = m_pGuild->get_guild_war().get_member_number();
	send.dw_enemy_number = m_pGuildAck->get_guild_war().get_member_number();
	m_pGuild->send_guild_message(&send, send.dw_size);

	NET_SIS_Guil_war_member_number_update send2;
	send2.dw_number = m_pGuildAck->get_guild_war().get_member_number();
	send2.dw_enemy_number = m_pGuild->get_guild_war().get_member_number();
	m_pGuildAck->send_guild_message(&send2, send2.dw_size);
}

//-----------------------------------------------------------------------------------------
// 刷新点刷怪
//-----------------------------------------------------------------------------------------
VOID map_instance_guild::spawn_creature_replace( Creature* p_dead_creature_ )
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
	
	INT nRand = get_tool()->tool_rand() % 10000;
	DWORD dwNewTypeID = INVALID_VALUE;

	INT nCandiNum	= 0;
	INT32 nPct = 0;
	while (VALID_VALUE(pSSpawnProto->dwTypeIDs[nCandiNum]))
	{
		DWORD dwCurType = pSSpawnProto->dwTypeIDs[nCandiNum];
		nCandiNum++;
		const tagGuildSSpawnPointProto* pSSpawnGuild = AttRes::GetInstance()->GetSSpawnGuildProto(dwCurType);
		if (!VALID_POINT(pSSpawnGuild))
			continue;
		
		if (!VALID_POINT(m_pGuild))
			continue;
		
		nPct += pSSpawnGuild->n_pro[m_pGuild->get_guild_att().byLevel-1];
		if (nRand < nPct)
		{
			dwNewTypeID = dwCurType;
			break;
		}

	}


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