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
// 初始化（创建者不需要）
//------------------------------------------------------------------------------------------------------
BOOL map_instance_battle::init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator, DWORD dwMisc)
{
	ASSERT( VALID_POINT(pInfo) );
	ASSERT( EMT_SBK == pInfo->e_type );

	// 读取副本静态属性
	m_p_instance = AttRes::GetInstance()->get_instance_proto(pInfo->dw_id);
	if( !VALID_POINT(m_p_instance) )	return FALSE;

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


	BattleGround::get_singleton().setMap(this);

	return TRUE;		
}

//---------------------------------------------------------------------------------
// 更新
//---------------------------------------------------------------------------------
VOID map_instance_battle::update()
{
	Map::update();

	update_time_issue();
}

//---------------------------------------------------------------------------------
// 销毁
//---------------------------------------------------------------------------------
VOID map_instance_battle::destroy()
{

}

//---------------------------------------------------------------------------------
// 正式加入一个玩家，这只能由管理该地图的MapMgr调用
//---------------------------------------------------------------------------------
VOID map_instance_battle::add_role(Role* pRole, Map* pSrcMap)
{
	Map::add_role(pRole, pSrcMap);

	// 重置关闭等待
	if( is_end() )
	{
		m_dw_end_tick = INVALID_VALUE;
		b_end = FALSE;
	}


	// 根据阵营设置战场势力
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
// 玩家离开地图，只可能在主线程里面调用
//---------------------------------------------------------------------------------
VOID map_instance_battle::role_leave_map(DWORD dwID, BOOL b_leave_online/* = FALSE*/)
{
	Map::role_leave_map(dwID);

	// 是否进入等待关闭
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
// 是否能进入副本
//---------------------------------------------------------------------------------
INT map_instance_battle::can_enter(Role *pRole)
{
	return E_Success;
}

//---------------------------------------------------------------------------------
// 是否可以删除
//---------------------------------------------------------------------------------
BOOL map_instance_battle::can_destroy()
{
	return false;
}


//---------------------------------------------------------------------------------
// 初始化刷怪点怪物
//---------------------------------------------------------------------------------
BOOL map_instance_battle::init_all_spawn_point_creature(DWORD dwGuildID/* = INVALID_VALUE*/)
{
	return Map::init_all_spawn_point_creature(dwGuildID);
}

//---------------------------------------------------------------------------------
// 副本结束
//---------------------------------------------------------------------------------
VOID map_instance_battle::on_delete()
{

}

//-----------------------------------------------------------------------------------
// 副本销毁
//-----------------------------------------------------------------------------------
VOID map_instance_battle::on_destroy()
{
	BattleGround::get_singleton().setMap(NULL);
}


DWORD map_instance_battle::FriendEnemy(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_)
{
	DWORD dwFlag = 0;
	// 准备中都是友好的
	if (BattleGround::get_singleton().getState() == EBS_BEGIN)
	{
		dwFlag |= ETFE_Friendly;
		b_ignore_ = TRUE;
	}
	else if(BattleGround::get_singleton().getState() == EBS_ING)
	{
		// 原目标是怪物
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
				// 宠物的敌我判断
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

		// 原目标是玩家
		if (p_src_->IsRole())
		{
			Role* pRoleSrc = (Role*)p_src_;
			if (p_target_->IsCreature())
			{
				Creature* pCreatureTar = (Creature*)p_target_;

				// 宠物的敌我判断
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
	// 如果已经处于待删除状态，就不更新了
	if( is_delete() ) return;

	// 关闭倒计时
	//if( is_end() )
	//{
	//	DWORD dw_tick = g_world.GetWorldTick();
	//	if( (dw_tick - m_dw_end_tick) > m_p_instance->dwEndTime * TICK_PER_SECOND)
	//	{
	//		set_delete();
	//	}
	//}

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

VOID map_instance_battle::on_creature_die(Creature* p_creature_, Unit* p_killer_)
{
	// 击杀类型不是首领就是卫兵
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

	// 把所有玩家移除副本
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


