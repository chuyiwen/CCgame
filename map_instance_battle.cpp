#include "stdafx.h"

#include "map_instance_battle.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "map_mgr.h"
#include "NPCTeam.h"
#include "NPCTeam_mgr.h"
#include "BattleGround.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/MapAttDefine.h"


map_instance_battle::map_instance_battle() : map_instance(),
m_dw_end_tick(INVALID_VALUE)
{
}

map_instance_battle::~map_instance_battle()
{
	destroy();
}

//------------------------------------------------------------------------------------------------------
// ��ʼ���������߲���Ҫ��
//------------------------------------------------------------------------------------------------------
BOOL map_instance_battle::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_SBK == pInfo->e_type );

	// ��ȡ������̬����
	m_p_instance = AttRes::GetInstance()->get_instance_proto(pInfo->dw_id);
	if( !VALID_POINT(m_p_instance) )	return FALSE;

	// ��ͼ�������
	p_map_info = pInfo;
	map_session.clear();
	map_role.clear();
	map_shop.clear();
	map_chamber.clear();
	map_ground_item.clear();

	// �����������
	dw_id = p_map_info->dw_id;
	dw_instance_id = dwInstanceID;

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


	BattleGround::get_singleton().setMap(this);

	return TRUE;		
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_battle::update()
{
	Map::update();

	update_time_issue();
}

//---------------------------------------------------------------------------------
// ����
//---------------------------------------------------------------------------------
VOID map_instance_battle::destroy()
{

}

//---------------------------------------------------------------------------------
// ��ʽ����һ����ң���ֻ���ɹ���õ�ͼ��MapMgr����
//---------------------------------------------------------------------------------
VOID map_instance_battle::add_role(Role* pRole, Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);

	// ���ùرյȴ�
	if( is_end() )
	{
		m_dw_end_tick = INVALID_VALUE;
		b_end = FALSE;
	}


	// ������Ӫ����ս������
	if (pRole->get_temp_role_camp() == ECA_Battle_a)
	{
		BattleGround::get_singleton().addRole(pRole->GetID(), pRole->get_level(), EBT_A);
	}
	else if (pRole->get_temp_role_camp() == ECA_Battle_b)
	{
		BattleGround::get_singleton().addRole(pRole->GetID(), pRole->get_level(), EBT_B);
	}
	
}

//---------------------------------------------------------------------------------
// ����뿪��ͼ��ֻ���������߳��������
//---------------------------------------------------------------------------------
VOID map_instance_battle::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);

	// �Ƿ����ȴ��ر�
	if( map_role.empty() && !is_end() )
	{
		m_dw_end_tick = g_world.GetWorldTick();
		b_end = TRUE;
	}

	if (!b_leave_online)
	{
		BattleGround::get_singleton().delRole(dwID);
	}
	
}
//---------------------------------------------------------------------------------
// �Ƿ��ܽ��븱��
//---------------------------------------------------------------------------------
INT map_instance_battle::can_enter(Role *pRole)
{
	return E_Success;
}

//---------------------------------------------------------------------------------
// �Ƿ����ɾ��
//---------------------------------------------------------------------------------
BOOL map_instance_battle::can_destroy()
{
	return false;
}


//---------------------------------------------------------------------------------
// ��ʼ��ˢ�ֵ����
//---------------------------------------------------------------------------------
BOOL map_instance_battle::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// ��������
//---------------------------------------------------------------------------------
VOID map_instance_battle::on_delete()
{

}

//-----------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------
VOID map_instance_battle::on_destroy()
{
	BattleGround::get_singleton().setMap(NULL);
}


DWORD map_instance_battle::FriendEnemy(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_)
{
	DWORD dwFlag = 0;
	// ׼���ж����Ѻõ�
	if (BattleGround::get_singleton().getState() == EBS_BEGIN)
	{
		dwFlag |= ETFE_Friendly;
		b_ignore_ = TRUE;
	}
	else if(BattleGround::get_singleton().getState() == EBS_ING)
	{
		// ԭĿ���ǹ���
		if (p_src_->IsCreature())
		{
			Creature* pCreatureSrc = (Creature*)p_src_;
			if (p_target_->IsCreature())
			{
				Creature* pCreatureTar = (Creature*)p_target_;

				if (pCreatureTar->GetProto()->eCamp == pCreatureSrc->GetProto()->eCamp)
				{
					dwFlag |= ETFE_Friendly;
				}
				else
				{
					dwFlag |= ETFE_Hostile;
				}
			}
			if (p_target_->IsRole())
			{
				Role* pRoleTar = (Role*)p_target_;
				// ����ĵ����ж�
				Role* pMarster = pCreatureSrc->GetMarster();
				if (VALID_POINT(pMarster))
				{
					if(pRoleTar == pMarster)
						return ETFE_Friendly;
				}

				if (pRoleTar->get_temp_role_camp() == pCreatureSrc->GetProto()->eCamp)
				{
					dwFlag |= ETFE_Friendly;
				}
				else
				{	
					dwFlag |= ETFE_Hostile;	
				}
			}
		}

		// ԭĿ�������
		if (p_src_->IsRole())
		{
			Role* pRoleSrc = (Role*)p_src_;
			if (p_target_->IsCreature())
			{
				Creature* pCreatureTar = (Creature*)p_target_;

				// ����ĵ����ж�
				Role* pMarster = pCreatureTar->GetMarster();
				if (VALID_POINT(pMarster))
				{
					if(pRoleSrc == pMarster)
						return ETFE_Friendly;
				}

				if (pCreatureTar->GetProto()->eCamp == pRoleSrc->get_temp_role_camp())
				{
					dwFlag |= ETFE_Friendly;
				}
				else
				{
					dwFlag |= ETFE_Hostile;
				}
			}
			if (p_target_->IsRole())
			{
				Role* pRoleTar = (Role*)p_target_;
				if (pRoleTar->get_temp_role_camp() == pRoleSrc->get_temp_role_camp())
				{
					dwFlag |= ETFE_Friendly;
				}
				else
				{	
					dwFlag |= ETFE_Hostile;	
				}
			}	
		}
	}
	else
	{
		b_ignore_ = FALSE;
		
	}
	return dwFlag;
}


VOID map_instance_battle::update_time_issue()
{
	// ����Ѿ����ڴ�ɾ��״̬���Ͳ�������
	if( is_delete() ) return;

	// �رյ���ʱ
	//if( is_end() )
	//{
	//	DWORD dw_tick = g_world.GetWorldTick();
	//	if( (dw_tick - m_dw_end_tick) > m_p_instance->dwEndTime * TICK_PER_SECOND)
	//	{
	//		set_delete();
	//	}
	//}

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

VOID map_instance_battle::on_creature_die(Creature* p_creature_, Unit* p_killer_)
{
	// ��ɱ���Ͳ��������������
	if (VALID_POINT(p_killer_))
	{
		BYTE byType = 1;
		if (p_creature_->GetProto()->dw_data_id == BATTLE_LEADER_A ||
			p_creature_->GetProto()->dw_data_id == BATTLE_LEADER_B)
		{
			byType = 2;
		}
		BattleGround::get_singleton().killUnit(p_killer_->GetID(), byType);
	}

	if (BattleGround::get_singleton().getState() == EBS_ING)
	{
		if (p_creature_->GetProto()->dw_data_id == BATTLE_LEADER_A)
		{
			BattleGround::get_singleton().setWiner(EBT_A, TRUE);

			BattleGround::get_singleton().setState(EBS_END);
		}
		else if(p_creature_->GetProto()->dw_data_id == BATTLE_LEADER_B)
		{
			BattleGround::get_singleton().setWiner(EBT_B, TRUE);

			BattleGround::get_singleton().setState(EBS_END);
		}
	}


	Map::on_creature_die(p_creature_, p_killer_);
}

VOID map_instance_battle::remove_all_player()
{
	//m_dw_end_tick = g_world.GetWorldTick();

	// ����������Ƴ�����
	Role* p_role = (Role*)INVALID_VALUE;

	ROLE_MAP::map_iter it = map_role.begin();
	while( map_role.find_next(it, p_role) )
	{
		if(!VALID_POINT(p_role))
			continue;
		m_map_will_out_role_id.add(p_role->GetID(), 5 * TICK_PER_SECOND);
	}

}

VOID map_instance_battle::on_role_die(Role* p_role_, Unit* p_killer_)
{
	if (VALID_POINT(p_killer_))
	{
		BattleGround::get_singleton().killUnit(p_killer_->GetID(), 0);
		BattleGround::get_singleton().BeKill(p_role_->GetID(), p_killer_->GetID());
	}
	Map::on_role_die(p_role_, p_killer_);
}


