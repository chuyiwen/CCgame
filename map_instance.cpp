/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ͨ������ͼ

#include "StdAfx.h"

#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"

#include "map_instance.h"
#include "map_creator.h"
#include "team.h"
#include "group_mgr.h"
#include "att_res.h"
#include "role.h"
#include "role_mgr.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "pvp_mgr.h"


map_instance_normal::map_instance_normal() : map_instance(), m_p_instance(NULL), m_dw_creator_id(INVALID_VALUE),
										 m_dw_team_id(INVALID_VALUE), m_dw_start_tick(INVALID_VALUE), m_dw_end_tick(INVALID_VALUE),
										 m_e_instance_hare_mode(EIHM_NULL), m_b_no_enter(TRUE),m_p_boss(NULL)
{
}

map_instance_normal::~map_instance_normal()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// ��ʼ��
//------------------------------------------------------------------------------------------------------
BOOL map_instance_normal::init(const tag_map_info* p_info_, DWORD dw_instance_id_, Role* p_creator_, DWORD dw_misc_)
{
	ASSERT( VALID_POINT(p_info_) );
	ASSERT( EMT_Instance == p_info_->e_type );

	if( !VALID_POINT(p_creator_) ) return FALSE;	// ����Ҫ�д����ߵ�

	// ��ȡ������̬����
	m_p_instance = AttRes::GetInstance()->get_instance_proto(p_info_->dw_id);
	if( !VALID_POINT(m_p_instance) )	return FALSE;

	// ��ͼ�������
	p_map_info = p_info_;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();
	m_list_creature_process.clear();
	b_process = FALSE;
	for(INT i = 0; i < MAX_INST_DOOR_NUM; i++)
	{
		m_door_state[i].dw_door_id = INVALID_VALUE;
		m_door_state[i].b_open = FALSE;
	}

	// �����������
	dw_id = p_map_info->dw_id;
	dw_instance_id = dw_instance_id_;
	m_dw_creator_id = p_creator_->GetID();
	m_e_instance_hare_mode = (EInstanceHardMode)dw_misc_;
	m_dw_start_tick = g_world.GetWorldTick();
	m_dw_end_tick = INVALID_VALUE;
	m_dw_team_id = p_creator_->GetTeamID();
	m_dw_create_time = GetCurrentDWORDTime();
	m_n_complete_time = INSTANCE_COMPLETE_TIME;
	dw_save_process_time = SAVE_PROCESS_TIME;

	Role* pLeaderRole = NULL;
	const Team* pTeam = g_groupMgr.GetTeamPtr(p_creator_->GetTeamID());
	if(VALID_POINT(pTeam))
	{
		pLeaderRole = pTeam->get_member(0);
		if(VALID_POINT(pLeaderRole))
		{
			init_inst_process(pLeaderRole);
		}
		else
		{
			init_inst_process(p_creator_);
		}
	}
	else
	{
		init_inst_process(p_creator_);
	}
	
	
	p_npc_team_mgr = new NPCTeamMgr(this);
	if(!VALID_POINT(p_npc_team_mgr))
		return FALSE;
	
	// ��ʼ��Ѱ·ϵͳ
	//if (!initSparseGraph(p_info_))
	//	return FALSE;


	// ����mapinfo����ʼ����ͼ�Ĺ�������б����Ϣ
	if( FALSE == init_logical_info() )
	{
		return FALSE;
	}

	// �������ɹ��ˣ�������һ������ڶ������������ID
	if( VALID_POINT(m_dw_team_id) )
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( VALID_POINT(pTeam) )
		{
			//if(pTeam->get_own_instanceid() == INVALID_VALUE && pTeam->get_own_instance_mapid() == INVALID_VALUE)
			//{
				pTeam->set_own_instance_mapid(p_map_info->dw_id);
				pTeam->set_own_instanceid(dw_instance_id);
			//}
			//else
			//{
			//	p_creator_->SetMyOwnInstanceMapID(p_map_info->dw_id);
			//	p_creator_->SetMyOwnInstanceID(dw_instance_id);
			//	p_creator_->SetMyOwnInstanceCreateTime(m_dw_create_time);
			//}
		}
	}
	else
	{
		p_creator_->SetMyOwnInstanceMapID(p_map_info->dw_id);
		p_creator_->SetMyOwnInstanceID(dw_instance_id);
		p_creator_->SetMyOwnInstanceCreateTime(m_dw_create_time);
	}

	return TRUE;		
}

// ��ʼ����������
VOID map_instance_normal::init_inst_process(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	if(pRole->get_list_inst_process().size() <= 0)
		return;

	package_list<s_inst_process*>::list_iter iter = pRole->get_list_inst_process().begin();
	s_inst_process* p = NULL;
	while(pRole->get_list_inst_process().find_next(iter, p))
	{
		if(!VALID_POINT(p))
			continue;

		if(p->dw_map_id != get_map_id() || p->n_mode != m_e_instance_hare_mode)
			continue;

		memcpy(m_door_state, p->st_door_state, sizeof(m_door_state));

		for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
		{
			if(p->dw_creature_id[i] == INVALID_VALUE)
				continue;
			m_list_creature_process.push_back(p->dw_creature_id[i]);
		}

		b_process = TRUE;
	}
}

// ���渱������
VOID map_instance_normal::add_inst_process_to_role(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	BOOL bFine = FALSE;
	package_list<s_inst_process*>::list_iter iter = pRole->get_list_inst_process().begin();
	s_inst_process* p_inst = NULL;
	while(pRole->get_list_inst_process().find_next(iter, p_inst))
	{
		if(!VALID_POINT(p_inst))
			continue;

		if(p_inst->dw_map_id != get_map_id() || p_inst->n_mode != m_e_instance_hare_mode)
			continue;

		memcpy(p_inst->st_door_state, m_door_state, sizeof(m_door_state));

		memset(p_inst->dw_creature_id, INVALID_VALUE, sizeof(p_inst->dw_creature_id));

		LIST_CREATURE_PRO::list_iter iter_pro = m_list_creature_process.begin();
		DWORD	dw_creature_id = INVALID_VALUE;
		while(m_list_creature_process.find_next(iter_pro, dw_creature_id))
		{
			for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
			{
				if(p_inst->dw_creature_id[i] == INVALID_VALUE && dw_creature_id != INVALID_VALUE)
				{
					p_inst->dw_creature_id[i] = dw_creature_id;
					break;
				}

			}
		}
		bFine = TRUE;
	}

	if(!bFine)
	{
		s_inst_process* p = new s_inst_process;
		if(!VALID_POINT(p))
			return;
		p->dw_map_id = get_map_id();
		p->n_mode = m_e_instance_hare_mode;
		p->n_type = m_p_instance->eInstanceEnterLimit;

		memcpy(p->st_door_state, m_door_state, sizeof(m_door_state));

		memset(p->dw_creature_id, INVALID_VALUE, sizeof(p->dw_creature_id));

		LIST_CREATURE_PRO::list_iter iter_pro = m_list_creature_process.begin();
		DWORD	dw_creature_id = INVALID_VALUE;
		while(m_list_creature_process.find_next(iter_pro, dw_creature_id))
		{
			for(INT i = 0; i < MAX_INST_CREATURE_NUM; i++)
			{
				if(p->dw_creature_id[i] == INVALID_VALUE && dw_creature_id != INVALID_VALUE)
				{
					p->dw_creature_id[i] = dw_creature_id;
					break;
				}

			}
		}

		pRole->get_list_inst_process().push_back(p);
	}
}

//---------------------------------------------------------------------------------
// ��������boss
//---------------------------------------------------------------------------------
VOID map_instance_normal::set_boss(Creature* p_boss_)
{
	if(!VALID_POINT(p_boss_))
		return;

	if(m_e_instance_hare_mode == EIHM_Normal)
	{
		if(m_p_instance->eCompleteNor == ECC_NorKillBoss)
		{
			if(p_boss_->GetProto()->dw_data_id == m_p_instance->dwCompleteNorVal1)
			{
				m_p_boss = p_boss_;
			}
		}
	}
	else if(m_e_instance_hare_mode == EIHM_Elite)
	{
		if(m_p_instance->eCompleteEli == ECC_NorKillBoss)
		{
			if(p_boss_->GetProto()->dw_data_id == m_p_instance->dwCompleteEliVal1)
			{
				m_p_boss = p_boss_;
			}
		}
	}
	else if(m_e_instance_hare_mode == EIHM_Devil)
	{
		if(m_p_instance->eCompleteDev == ECC_DevilKillBoss)
		{
			if(p_boss_->GetProto()->dw_data_id == m_p_instance->dwCompleteEliVal1)
			{
				m_p_boss = p_boss_;
			}
		}
	}
	
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_normal::update()
{
	Map::update();
	update_time_issue();
}

//---------------------------------------------------------------------------------
// ʱ����صĸ���
//---------------------------------------------------------------------------------
VOID map_instance_normal::update_time_issue()
{
	// ����Ѿ����ڴ�ɾ��״̬���Ͳ�������
	if( is_delete() ) return;

	// ���㸱�������Ƿ����
	complete_estimate();

	// ʱ�޸���
	if( is_time_limit() && !is_end() )
	{
		DWORD dw_tick = g_world.GetWorldTick();
		if( (dw_tick - m_dw_start_tick) >= m_p_instance->dwTimeLimit * TICK_PER_SECOND )
		{
			m_dw_end_tick = g_world.GetWorldTick();
			set_end();
		}
	}

	// �رյ���ʱ
	if( is_end() )
	{
		DWORD dw_tick = g_world.GetWorldTick();
		if (INVALID_VALUE == m_p_instance->dwEndTime)
		{
			set_delete();
		}
		else
		{
			if( (dw_tick - m_dw_end_tick) > m_p_instance->dwEndTime * TICK_PER_SECOND )
			{
				set_delete();
			}
		}
		
	}

	// ������ɹرյ���ʱ
	if(is_complete())
	{
		m_n_complete_time -= TICK_TIME;
		if(m_n_complete_time <= 0)
			set_delete();
	}

	// �������д��˳��Ľ�ɫ��ʱ��
	if( !m_map_will_out_role_id.empty() )
	{
		package_map<DWORD, INT>::map_iter it = m_map_will_out_role_id.begin();
		DWORD dw_role_id = INVALID_VALUE;
		INT n_tick = INVALID_VALUE;

		while( m_map_will_out_role_id.find_next(it, dw_role_id, n_tick) )
		{
			--n_tick;	// ��һ�µ���ʱ
			if( n_tick <= 0 )
			{
				// ʱ�䵽�ˣ�����Ҵ��ͳ�ȥ
				Role* p_role = find_role(dw_role_id);
				if( VALID_POINT(p_role) )
				{
					MapMgr* pMapMgr = g_mapCreator.get_map_manager(p_map_info->dw_id);
					if( VALID_POINT(pMapMgr) )
					{
						pMapMgr->RoleInstanceOut(p_role);
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

	dw_save_process_time--;
	if(dw_save_process_time <= 0)
	{
		ROLE_MAP::map_iter iter = map_role.begin();
		Role* pRole = NULL;
		while(map_role.find_next(iter, pRole))
		{
			if(!VALID_POINT(pRole))
				continue;

			// ��Ӹ�������
			add_inst_process_to_role(pRole);
		}
		dw_save_process_time = SAVE_PROCESS_TIME;
	}
}

//---------------------------------------------------------------------------------
// ����ж�
//---------------------------------------------------------------------------------
VOID map_instance_normal::complete_estimate ()
{
	if(is_complete())
		return;
	if(!VALID_POINT(m_p_boss))
		return;
	if(m_p_boss->IsDead())
	{
		set_complete();

		// �Ƴ������ڵ�ͼ�ڵ����
		MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
		if( !VALID_POINT(p_map_manager) ) return;

		Role* p_role = (Role*)INVALID_VALUE;

		ROLE_MAP::map_iter it = map_role.begin();
		while( map_role.find_next(it, p_role) )
		{
			if(!VALID_POINT(p_role))
				continue;
			m_map_will_out_role_id.add(p_role->GetID(), ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN);
			NET_SIS_instance_complete send;
			p_role->SendMessage(&send, send.dw_size);
		}
	}
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_normal::destroy()
{
	m_list_creature_process.clear();
}

//---------------------------------------------------------------------------------
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------
VOID map_instance_normal::add_role(Role* p_role_, Map* pSrcMap)
{
	Map::add_role(p_role_, pSrcMap);

	// ���ùرյȴ�
	if( is_end() )
	{
		m_dw_end_tick = INVALID_VALUE;
		b_end = FALSE;
		m_b_no_enter = TRUE;
	}

	// ����ǵ�һ�����븱�������
	if( m_b_no_enter )
	{
		m_b_no_enter = FALSE;

		// ����ж��飬����֪ͨ����ȷ��
		if( INVALID_VALUE != p_role_->GetTeamID() && m_p_instance->bNoticeTeamate )
		{
			NET_SIS_instance_nofity send;
			p_role_->SendMessage(&send, send.dw_size);
		}
	}

	g_pvp_mgr.role_offline(p_role_);
	
	// ���ͽ��븱����Ϣ
	NET_SIS_enter_instance send;
	send.dw_error_code = E_Success;
	send.dwTimeLimit = cal_time_limit();
	p_role_->SendMessage(&send, send.dw_size);

	s_enter_map_limit* pEnterMapLimit = p_role_->GetMapLimitMap().find(p_map_info->dw_id);
	if(VALID_POINT(pEnterMapLimit)/* && m_p_instance->eInstanceEnterLimit != EEL_Week*/)
	{
		NET_SIS_instance_limit_info send;
		send.dw_enter_num = pEnterMapLimit->dw_enter_num_;
		send.dw_max_enter_num = m_p_instance->dwEnterNumLimit;
		p_role_->SendMessage(&send, send.dw_size);
	}
	
}

//---------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//---------------------------------------------------------------------------------
VOID map_instance_normal::role_leave_map(DWORD dw_id_, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dw_id_);

	// �Ƿ����ȴ��ر�
	if( map_role.empty() && !is_end() && m_p_instance->dwEndTime != INVALID_VALUE )
	{
		m_dw_end_tick = g_world.GetWorldTick();
		b_end = TRUE;
	}

	Role* pRole = g_roleMgr.get_role(dw_id_);
	if(VALID_POINT(pRole))
	{
		// ��Ӹ�������
		add_inst_process_to_role(pRole);
	}

	// ����������ڵȴ��뿪���б�����Ƴ�
	m_map_will_out_role_id.erase(dw_id_);
}
//---------------------------------------------------------------------------------
// �Ƿ��ܽ��븱��
//---------------------------------------------------------------------------------
INT map_instance_normal::can_enter(Role *p_role_, DWORD dw_param_)
{
	// �ȼ��ͨ���ж�
	INT n_error_code = map_instance::can_enter(p_role_);
	if( E_Success != n_error_code ) return n_error_code;

	// �����������
	if( m_p_instance->nNumUpLimit <= get_role_num() )
		return E_Instance_Role_Full;

	
	//if(dw_param_ == 0)
	//{
	//	// �ȼ�����
	//	if( m_p_instance->nLevelDownLimit > p_role_->get_level() )
	//		return E_Instance_Level_Down_Limit;

	//	if( m_p_instance->nLevelUpLimit < p_role_->get_level() )
	//		return E_Instance_Level_Up_Limit;
	//}
	//else
	//{
	//	// �ȼ�����
	//	if( m_p_instance->nLevelEliteDownLimit > p_role_->get_level() )
	//		return E_Instance_Level_Down_Limit;

	//	if( m_p_instance->nLevelEliteUpLimit < p_role_->get_level() )
	//		return E_Instance_Level_Up_Limit;
	//}

	// ������
	if( m_dw_team_id != INVALID_VALUE )
	{
		if( p_role_->GetTeamID() != m_dw_team_id )
			return E_Instance_Not_Same_Team;
	}
	// ������
	else
	{
		if( p_role_->GetID() != m_dw_creator_id )
			return E_Instance_Not_Same_Team;
	}

	// ����ͬ���Ѷȸ���
	if(dw_param_ != m_e_instance_hare_mode)
	{
		return E_Instance_diff_error;
	}

	// �ж�Ȩֵ���� gx modify 2013.8.15
	/*s_enter_map_limit* pEnterMapLimit = p_role_->GetMapLimitMap().find(m_p_instance->dwMapID);
	if(VALID_POINT(pEnterMapLimit))
	{
		if(pEnterMapLimit->dw_enter_num_ >= m_p_instance->dwEnterNumLimit)
			return E_Instance_EnterNum_Limit;
		else
		{
			pEnterMapLimit->dw_enter_num_++;
		}
	}*/

	return E_Success;
}

//---------------------------------------------------------------------------------
// �Ƿ����ɾ��
//---------------------------------------------------------------------------------
BOOL map_instance_normal::can_destroy()
{
	return map_instance::can_destroy();
}

//---------------------------------------------------------------------------------
// ��ʼ����ͼ�ڷŹ���
//---------------------------------------------------------------------------------
BOOL map_instance_normal::init_all_fixed_creature(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
	// һ��һ���Ĵ�������
	tagMapCreatureInfo* p_creature_info = NULL;
	const tagCreatureProto* p_proto = NULL;
	p_map_info->map_creature_info.reset_iterator();
	while( p_map_info->map_creature_info.find_next(p_creature_info) )
	{
		p_proto = AttRes::GetInstance()->GetCreatureProto(p_creature_info->dw_type_id);
		if( !VALID_POINT(p_proto) )
		{
			print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), p_creature_info->dw_type_id);
			continue;
		}

		// ȡ��һ��ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// ���ɳ�������ͳ�������
		Vector3 v_faceto;
		v_faceto.x = cosf(p_creature_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.z = sinf(p_creature_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// ���ɹ���
		Creature* p_creature = Creature::Create(dw_creature_id, dw_id, this, p_proto, p_creature_info->v_pos, v_faceto, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, p_creature_info->b_collide, p_creature_info->list_patrol, INVALID_VALUE, dw_guild_id_);

		// ����boss
		set_boss(p_creature);

		// ���뵽��ͼ��
		add_creature_on_load(p_creature);

		// ����ǳ�Ѩ������س�Ѩ����
		if( p_creature->IsNest() )
		{
			init_nest_creature(p_creature);
		}

		// ����ǹ���С�ӣ������С�ӹ���
		if( p_creature->IsTeam() )
		{
			init_team_creature(p_creature);
		}
	}

	// һ��һ���Ĵ����Ŷ���
	tag_map_door_info* p_door_info = NULL;
	//const tagCreatureProto* pProto = NULL;
	p_map_info->map_door_info.reset_iterator();
	while( p_map_info->map_door_info.find_next(p_door_info) )
	{
		p_proto = AttRes::GetInstance()->GetCreatureProto(p_door_info->dw_type_id);
		if( !VALID_POINT(p_proto) )
		{
			print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), p_door_info->dw_type_id);
			continue;
		}

		if ( !( p_proto->eType == ECT_GameObject && p_proto->nType2 == EGOT_Door ) )
		{
			print_message(_T("Detect a unknown door in map, dwObjID=%u\r\n"), p_door_info->dw_type_id);
			continue;
		}

		// ȡ��һ��ID
		DWORD dw_door_id = OBJID_TO_DOORID(p_door_info->dw_att_id);//m_CreatureIDGen.GetNewCreatureID();
		ASSERT( IS_DOOR(dw_door_id ));

		// ���ɳ�������ͳ�������
		Vector3 v_faceto;
		v_faceto.x = cosf(p_door_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.z = sinf(p_door_info->f_yaw * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// ���ɹ���
		Creature* p_creature = Creature::Create(dw_door_id, dw_id, this, p_proto, p_door_info->v_pos, v_faceto, 
			ECST_Load, INVALID_VALUE, INVALID_VALUE, FALSE, NULL);
	
		// ���뵽��ͼ��
		add_creature_on_load(p_creature);

		// ���뵽�Ŷ����б�
		map_door.add( dw_door_id, p_creature );
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
// ��ʼ��ˢ�ֵ����
//---------------------------------------------------------------------------------
BOOL map_instance_normal::init_all_spawn_point_creature(DWORD dw_guild_id_/* = INVALID_VALUE*/)
{
	/*if( EICM_Appoint == m_pInstance->eInstanceCreateMode )
		return TRUE;*/

	if( FALSE == recal_hard_mode() ) return FALSE;

	DWORD		dw_data_id = INVALID_VALUE;
	INT			n_base_level = 0;
	INT			n_level = 0;
	Vector3		vec3;

	// ���������������ȼ�
	/*if( FALSE == get_creature_base_level(nBaseLevel) )
		return FALSE;*/

	tag_map_spawn_point_info* p_map_spawn_info = NULL;
	const tagRandSpawnPointInfo* p_spawn_proto = NULL;
	const tagCreatureProto* p_proto = NULL;

	p_map_info->map_spawn_point.reset_iterator();
	while( p_map_info->map_spawn_point.find_next(p_map_spawn_info) )
	{
		//DWORD dwSpawnPoint = transmit_big_id(nBaseLevel, pMapSpawnInfo);

		// �ж�ˢ�ֵ��Ƿ��н���
		if(m_list_creature_process.is_exist(p_map_spawn_info->dw_att_id))
			continue;

		p_spawn_proto = AttRes::GetInstance()->GetSpawnPointProto(p_map_spawn_info->dw_spawn_point_id);
		if( !VALID_POINT(p_spawn_proto) )
		{
			print_message(_T("Detect a unknown Spawn Point in map, dwObjID=%u\r\n"), p_map_spawn_info->dw_spawn_point_id);
			continue;
		}

		dw_data_id = cal_creature_type_id(p_spawn_proto);
		if( !VALID_POINT(dw_data_id) )
		{
			//print_message(_T("Detect a unknown Instance Creature TypeID\n"));
			continue;
		}

		p_proto = AttRes::GetInstance()->GetCreatureProto(dw_data_id);
		if( !VALID_POINT(p_proto) )
		{
			//print_message(_T("Detect a unknown creature in map, dwObjID=%u\r\n"), pSpawnProto->dwSpawnPointID);
			continue;
		}

		// ȡ��һ��ID
		DWORD dw_creature_id = builder_creature_id.get_new_creature_id();
		ASSERT( IS_CREATURE(dw_creature_id) );

		// ���һ������
		Vector3 v_faceto;
		//FLOAT fYaw = FLOAT(get_tool()->tool_rand() % 360);
		v_faceto.x = cosf((270-p_map_spawn_info->f_angle) * 3.1415927f / 180.0f);
		v_faceto.z = sinf((270-p_map_spawn_info->f_angle) * 3.1415927f / 180.0f);
		v_faceto.y = 0.0f;

		// ���ɹ���
		Creature* p_creature = Creature::Create(dw_creature_id, dw_id, this, p_proto, p_map_spawn_info->v_pos, v_faceto, ECST_Load, p_map_spawn_info->dw_att_id, INVALID_VALUE, p_map_spawn_info->b_collide, p_map_spawn_info->list_patrol);

		// ��������boss
		set_boss(p_creature);
		// ���뵽��ͼ��
		add_creature_on_load(p_creature);

		// ����ǳ�Ѩ������س�Ѩ����
		if( p_creature->IsNest() )
		{
			init_nest_creature(p_creature);
		}

		// ����ǹ���С�ӣ������С�ӹ���
		if( p_creature->IsTeam() )
		{
			init_team_creature(p_creature);
		}
	}

	return TRUE;


}

//---------------------------------------------------------------------------------
// ���¼��㸱���Ѷ�
//---------------------------------------------------------------------------------
BOOL map_instance_normal::recal_hard_mode()
{
	// ����ѡ�����Ѷ�
	if( !m_p_instance->bSelectHard )
	{
		m_e_instance_hare_mode = EIHM_Normal;
		return TRUE;
	}
	 // ��ѡ
	else
	{
		switch(m_e_instance_hare_mode)
		{
			// ��ͨ
		case EIHM_Normal:
			{
				if( !m_p_instance->bSelectNormal ) return FALSE;
				else return TRUE;
			}
			break;

			// ��Ӣ
		case EIHM_Elite:
			{
				if( !m_p_instance->bSelectElite ) return FALSE;
				else return TRUE;
			}
			break;

			// ħ��
		case EIHM_Devil:
			{
				if( !m_p_instance->bSelectDevil ) return FALSE;
				else return TRUE;
			}
			break;

		default:
			return FALSE;
			break;
		}

	}
}

//---------------------------------------------------------------------------------
// ���ݸ������ɹ��򣬵õ��������Ļ����ȼ�
//---------------------------------------------------------------------------------
BOOL map_instance_normal::get_creature_base_level(INT& n_base_level_)
{
	if( m_dw_team_id != INVALID_VALUE && m_p_instance->nNumUpLimit > 1 )
	{
		const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( !VALID_POINT(p_team) ) return FALSE;

		switch( m_p_instance->eInstanceCreateMode )
		{
			// ƽ���ȼ�
		case EICM_AvgLevel:
			{
				n_base_level_ = p_team->get_average_level();
				return TRUE;
			}
			break;

			// �ӳ��ȼ�
		case EICM_LeaderLevel:
			{
				Role* p_leader = p_team->get_member(0);
				if( !VALID_POINT(p_leader) )
					return FALSE;
				else n_base_level_ = p_leader->get_level();
				return TRUE;
			}
			break;

			// ������ߵȼ�
		case EICM_MaxLevel:
			{
				n_base_level_ = p_team->get_max_level();
				return TRUE;
			}
			break;

			// ������͵ȼ�
		case EICM_MinLevel:
			{
				n_base_level_ = p_team->get_min_level();
				return TRUE;
			}
			break;

		default:
			return FALSE;
			break;
		}
	}
	else
	{
		Role* p_role = g_roleMgr.get_role(m_dw_creator_id);
		if( !VALID_POINT(p_role) )
			return FALSE;

		n_base_level_ = p_role->get_level();
		return TRUE;
	}
}


//---------------------------------------------------------------------------------
// ���ݹ�������ȼ��õ���������TypeID
//---------------------------------------------------------------------------------
DWORD map_instance_normal::cal_creature_type_id(const tagRandSpawnPointInfo* p_rand_spawn_point_)
{	
	INT n_index = get_tool()->tool_rand() % RAND_CTEATUTE_NUM;
	
	switch(m_e_instance_hare_mode)
	{
	case EIHM_Normal:
		return p_rand_spawn_point_->dwNormalID[n_index];
	case EIHM_Elite:
		return p_rand_spawn_point_->dwEliteID[n_index];
	case EIHM_Devil:
		return p_rand_spawn_point_->dwDevilID[n_index];
	default:
		return INVALID_VALUE;
	}
}




//---------------------------------------------------------------------------------
// ���ݸ��������ȼ���ˢ�ֵ�ĵȼ���������ˢ�ֵ�СID������ˢ�ֵ��ID
//---------------------------------------------------------------------------------
//DWORD map_instance_normal::transmit_big_id(INT n_base_level_, tag_map_spawn_point_info* p_map_spawn_info_)
//{
//	if(!VALID_POINT(p_map_spawn_info_))
//		return INVALID_VALUE;
//
//	const tagLevelMapping *p_level_mapping = AttRes::GetInstance()->GetLevelMapping(n_base_level_ + p_map_spawn_info_->n_level_increase);
//	return p_map_spawn_info_->dw_spawn_point_id + (DWORD)p_level_mapping->nTransmitLevel;
//}

//---------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------
VOID map_instance_normal::on_delete()
{
	// �Ƴ������ڵ�ͼ�ڵ����
	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		if(!VALID_POINT(p_role))
			continue;
		p_role->SetMyOwnInstanceID(INVALID_VALUE);
		p_role->SetMyOwnInstanceMapID(INVALID_VALUE);
		tagDWORDTime st_Time;
		ZeroMemory(&st_Time, sizeof(tagDWORDTime));
		p_role->SetMyOwnInstanceCreateTime(st_Time);
		p_map_manager->RoleInstanceOut(p_role);
	}

	const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
	if(VALID_POINT(p_team))
	{
		if(p_team->get_team_id() == m_dw_team_id)
		{
			p_team->set_own_instanceid(INVALID_VALUE);
			p_team->set_own_instance_mapid(INVALID_VALUE);
		}
	}
}

//-----------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------
VOID map_instance_normal::on_destroy()
{
	// ���һ�¶���ĸ���ID
	if( VALID_POINT(m_dw_team_id) )
	{
		const Team* p_team = g_groupMgr.GetTeamPtr(m_dw_team_id);
		if( VALID_POINT(p_team) )
		{
			p_team->set_own_instance_mapid(INVALID_VALUE);
			p_team->set_own_instanceid(INVALID_VALUE);
		}
	}
	// ���һ����ҵĸ���ID
	else
	{
		/*Role* pCreator = g_roleMgr.get_role(m_dwCreatorID);
		if( VALID_POINT(pCreator) )
		{
			pCreator->SetMyOwnInstanceMapID(INVALID_VALUE);
			pCreator->SetMyOwnInstanceID(INVALID_VALUE);
		}*/
	}

}

//-----------------------------------------------------------------------------------
// ����ʱ�޸�����ʣʱ��
//-----------------------------------------------------------------------------------
DWORD map_instance_normal::cal_time_limit()
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

//---------------------------------------------------------------------------------------------------
// �����鴴��
//---------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_team_create(const Team* p_team_)
{
	if( !VALID_POINT(p_team_) ) return;

	if(m_dw_team_id == INVALID_VALUE)
	{
		m_dw_team_id = p_team_->get_team_id();
	}
}

//----------------------------------------------------------------------------------------------------
// ������ɾ��
//----------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_team_delete(const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;
	if( p_team_->get_member_number() != 1 ) return;
	
	//if( pTeam->GetMemID(0) != m_dwCreatorID ) return;

	m_dw_team_id = INVALID_VALUE;

	// �ҵ�������Ψһһ����ҵ����ָ�룬����Ϊ���˸���
	/*Role* pRole = g_roleMgr.get_role(m_dwCreatorID);
	if( VALID_POINT(pRole) )
	{
		pRole->SetMyOwnInstanceID(dw_instance_id);
		pRole->SetMyOwnInstanceMapID(m_dwID);
	}*/

	// �Ƴ������ڵ�ͼ�ڵ����
	MapMgr* p_map_manager = g_mapCreator.get_map_manager(dw_id);
	if( !VALID_POINT(p_map_manager) ) return;

	Role* p_role = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		if(!VALID_POINT(p_role))
			continue;
		m_map_will_out_role_id.add(p_role->GetID(), ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN);
	}
}

//-----------------------------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_role_leave_team(DWORD dw_role_id_, const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;

	// ���������Ǹ����Ĵ����ߣ��򽫴������ƽ�����ǰ����Ķӳ�
	/*if( dw_role_id == m_dwCreatorID )
	{
		ASSERT( pTeam->get_member_number() > 0 );
		m_dwCreatorID = pTeam->GetMemID(0);
	}*/

	// ��������Ŀǰ�ڸ������������ҵ��뿪��������ʱ
	Role* p_role = find_role(dw_role_id_);
	if( VALID_POINT(p_role) )
	{
		m_map_will_out_role_id.add(dw_role_id_, ROLE_LEAVE_INSTANCE_TICK_COUNT_DOWN);
	}
}

//------------------------------------------------------------------------------------------------------
// ���������
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::on_role_enter_team(DWORD dw_role_id_, const Team* p_team_)
{
	if( m_dw_team_id != p_team_->get_team_id() ) return;

	// ��������Ŀǰ�ڸ�����������ҵ��뿪��������ʱ
	Role* pRole = find_role(dw_role_id_);
	if( VALID_POINT(pRole) )
	{
		m_map_will_out_role_id.erase(dw_role_id_);
	}
}

//------------------------------------------------------------------------------------------------------
// ���渱���ſ���״̬
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::set_door_state(DWORD	dw_door_id, BOOL b_open)
{
	for(INT i = 0; i < MAX_INST_DOOR_NUM; i++)
	{
		if(m_door_state[i].dw_door_id != dw_door_id)
			continue;
		m_door_state[i].b_open = b_open;
	}
}

//------------------------------------------------------------------------------------------------------
// ��ȡ�����ſ���״̬
//------------------------------------------------------------------------------------------------------
VOID map_instance_normal::get_door_state(INT nIndex, DWORD& dw_door_id, BOOL& b_open)
{
	if(nIndex < 0 || nIndex >= MAX_INST_DOOR_NUM)
		return;

	dw_door_id = m_door_state[nIndex].dw_door_id;
	b_open = m_door_state[nIndex].b_open;
}