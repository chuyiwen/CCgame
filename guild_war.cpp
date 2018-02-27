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
*	@file		guild_war.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/


#include "StdAfx.h"
#include "guild_war.h"
#include "guild.h"
#include "guild_manager.h"
#include "role_mgr.h"
#include "role.h"
#include "map.h"
#include "map_creator.h"
#include "map_instance_guild.h"

#include "../common/ServerDefine/log_server_define.h"
#include "creature.h"
guild_war::guild_war(void)
{
	e_guild_war_state = EGWS_NULL;
	dw_declare_guild_id = INVALID_VALUE;
	dw_bedeclare_guild_id = INVALID_VALUE;
	dw_war_guild_id = INVALID_VALUE;
	dw_achievement = 0;
	dw_enemy_achievement = 0;
	dw_begin_time = 0;
	//dw_prepare_begin_time = 0;
	//dw_war_begin_time = 0;
	by_enemy_level = 0;
	m_dw_Max_num = 80;
	m_dw_member_number = 0;
	m_bWin = FALSE;
	m_bLost = FALSE;
	m_fParam = 0.8;
	m_dw_baoming_num = 0;
	m_bRealy = FALSE;
	m_bInitDoor = FALSE;
	m_byRebornTime = 10;
	memset(dw_relive_id, INVALID_VALUE, sizeof(dw_relive_id));
	memset(&st_declare_broadcast, 0, sizeof(st_declare_broadcast));
	memset(&st_prepare_broadcast, 0, sizeof(st_prepare_broadcast));
}

guild_war::~guild_war(void)
{
}

VOID guild_war::init(guild* p_guild_)
{
	if(!VALID_POINT(p_guild_))
		return;

	p_guild = p_guild_;
}

VOID guild_war::zero_relive_id()
{
	memset(dw_relive_id, INVALID_VALUE, sizeof(dw_relive_id));
}

VOID guild_war::set_relive_id(DWORD dw_creature_id_, BOOL b_add_)
{
	for(INT i = 0; i < MAX_GUILD_RELIVE_NUM; i++)
	{
		if(b_add_)
		{
			if(dw_relive_id[i] == INVALID_VALUE)
			{
				dw_relive_id[i] = dw_creature_id_;
				return ;
			}
		}
		else
		{
			if(dw_relive_id[i] == dw_creature_id_)
			{
				dw_relive_id[i] = INVALID_VALUE;
				return;
			}
		}
	}
}

VOID guild_war::reset()
{
	e_guild_war_state = EGWS_NULL;
	dw_declare_guild_id = INVALID_VALUE;
	dw_bedeclare_guild_id = INVALID_VALUE;
	dw_war_guild_id = INVALID_VALUE;
	dw_achievement = 0;
	dw_enemy_achievement = 0;
	dw_begin_time = 0;
	//dw_prepare_begin_time = 0;
	//dw_war_begin_time = 0;
	by_enemy_level = 0;
	m_bWin = FALSE;
	m_bLost = FALSE;
	m_dw_Max_num = 80;
	m_fParam = 0.8;
	m_dw_member_number = 0;
	m_dw_baoming_num = 0;
	m_bRealy = FALSE;
	m_bInitDoor = FALSE;
	m_byRebornTime = 10;
	memset(&st_declare_broadcast, 0, sizeof(st_declare_broadcast));
	memset(&st_prepare_broadcast, 0, sizeof(st_prepare_broadcast));
}

VOID guild_war::reset_avlidance_state()
{
	//if(e_guild_war_state == EGWS_Avoidance)
	//{
		e_guild_war_state = EGWS_NULL;
	//}
}

DWORD guild_war::get_relive_id(INT n_index_)
{
	if(n_index_ < 0 || n_index_ > MAX_GUILD_RELIVE_NUM)
		return INVALID_VALUE;

	return dw_relive_id[n_index_];
}

VOID guild_war::update()
{
	switch(e_guild_war_state)
	{
	case EGWS_BeDeclare:
		{
			//if(st_declare_broadcast.n_time > 0)
			//{
			//	if(CalcTimeDiff(st_declare_broadcast.dw_broadcast_time, g_world.GetWorldTime()) <= 0)
			//	{
			//		NET_SIS_guild_war_broad send;
			//		send.dwGuildID = p_guild->get_guild_att().dwID;
			//		send.dwEnemyGuildID = dw_declare_guild_id;
			//		send.eGuildWarState = EGWS_BeDeclare;
			//		p_guild->send_guild_message(&send, send.dw_size);

			//		IncreaseTime(st_declare_broadcast.dw_broadcast_time, st_declare_broadcast.n_space_time);
			//		st_declare_broadcast.n_time--;
			//	}
			//}

			if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= 20/*DECLARE_WAIT_TIME*/)
			{
			

				guild* p_enemy = g_guild_manager.get_guild(dw_declare_guild_id);
				if (!VALID_POINT(p_enemy))
					return;

				tagGuildWarHistory sWarHistory;

				sWarHistory.dw_time = GetCurrentDWORDTime();
				sWarHistory.dw_guild_id = p_guild->get_guild_att().dwID;
				sWarHistory.dw_enemy_id = p_enemy->get_guild_att().dwID;
				sWarHistory.dw_enemy_leader_id = p_enemy->get_guild_att().dwLeaderRoleID;
				sWarHistory.e_war_history_type = EWHT_JUJUE;
				p_guild->add_war_history(sWarHistory);

				sWarHistory.dw_guild_id = p_enemy->get_guild_att().dwID;
				sWarHistory.dw_enemy_id = p_guild->get_guild_att().dwID;
				sWarHistory.dw_enemy_leader_id = p_guild->get_guild_att().dwLeaderRoleID;
				sWarHistory.e_war_history_type = EWHT_DUIFANG_JUJUE;
				p_enemy->add_war_history(sWarHistory);


				// 比目标帮派等级高,惩罚下
				if (p_guild->get_guild_att().byLevel >= p_enemy->get_guild_att().byLevel)
				{
					/*INT32 n_prosperity = 1000 * p_guild->get_guild_att().byLevel;
					p_guild->decrease_prosperity(INVALID_VALUE, n_prosperity, elcid_guild_declarewar);*/

					INT64 n64Silver = 100000 * p_guild->get_guild_att().byLevel;

					Role* pRole = g_roleMgr.get_role(p_guild->get_guild_att().dwLeaderRoleID);
					if (VALID_POINT(pRole))
					{
						pRole->GetCurMgr().DecBagBindSilver(n64Silver, elcid_guild_declarewar);
					}
					
				}


				NET_SIS_refuse_declare_broad send;
				send.dwDeclareGuildID = dw_declare_guild_id;
				send.dwRefuseGuildID = p_guild->get_guild_att().dwID;
				
				_tcsncpy(send.szDeclareGuildName, p_guild->get_guild_att().str_name.c_str(), p_guild->get_guild_att().str_name.size());
				_tcsncpy(send.szRefuseGuildName, p_enemy->get_guild_att().str_name.c_str(), p_enemy->get_guild_att().str_name.size());


				g_roleMgr.send_world_msg(&send, send.dw_size);

				reset();

			}
			break;
		}
	case EGWS_Declare:
		{
			if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= 20/*DECLARE_WAIT_TIME*/)
			{
				reset();
				//p_guild->increase_guild_fund(INVALID_VALUE, g_guild_manager.get_guild_config().nGoldGuildWarNeeded, elcid_guild_declarewar);
			}
			break;
		}
	case EGWS_Prepare:
		{
			//if(st_prepare_broadcast.n_time > 0)
			//{
			//	if(CalcTimeDiff(st_prepare_broadcast.dw_broadcast_time, GetCurrentDWORDTime()) <= 0)
			//	{
			//		NET_SIS_guild_war_broad send;
			//		send.dwGuildID = p_guild->get_guild_att().dwID;
			//		send.dwEnemyGuildID = dw_war_guild_id;
			//		send.eGuildWarState = EGWS_Prepare;
			//		send.byTime = st_prepare_broadcast.n_time;
			//		p_guild->send_guild_message(&send, send.dw_size);

			//		IncreaseTime(st_prepare_broadcast.dw_broadcast_time, st_prepare_broadcast.n_space_time);
			//		st_prepare_broadcast.n_time--;
			//	}
			//}
			


			// 隔天的晚上8点开始帮会战
			if (
				(GetCurrentDWORDTime().day > dw_begin_time.day ||GetCurrentDWORDTime().month > dw_begin_time.month ||GetCurrentDWORDTime().year > dw_begin_time.year) 
				&& GetCurrentDWORDTime().hour >= 20)
			{
				guild* p_enemy = g_guild_manager.get_guild(dw_war_guild_id);
				if (!VALID_POINT(p_enemy))
					return;

				set_guild_war_state(EGWS_WAR_relay, _T(""), isDefenter());

				DWORD dw_guild_instance_id = p_guild->get_guild_instance_id();
				if( VALID_POINT(dw_guild_instance_id) )
				{
					map_instance_guild* p_guild_instance = p_guild->get_guild_map();
					if( VALID_POINT(p_guild_instance) )
					{
						//p_guild->set_guild_instance_id(INVALID_VALUE);
						p_guild_instance->on_guild_prepare_war(p_guild);
						p_guild_instance->set_in_war(true);
					}
				}

			}

			//if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= 20*60|| 
			//	(m_bRealy && p_enemy->get_guild_war().isRelay()))
			//{
			//	set_guild_war_state(EGWS_WAR_relay, _T(""), isDefenter());	
			//}
			break;
		}
	case EGWS_WAR_relay:
		{
			
			guild* p_enemy = g_guild_manager.get_guild(dw_war_guild_id);
			if (!VALID_POINT(p_enemy))
				return;
			
			// 处理开战相关
			if (!m_bInitDoor)
			{
				DWORD dw_guild_instance_id = p_guild->get_guild_instance_id();
				if( VALID_POINT(dw_guild_instance_id) )
				{
					DWORD dw_guild_map_id = get_tool()->crc32(szGuildMapName);
					Map* p_map = g_mapCreator.get_map(dw_guild_map_id, dw_guild_instance_id);

					if( VALID_POINT(p_map) )
					{
						map_instance_guild* p_guild_instance = dynamic_cast<map_instance_guild*>(p_map);
						if( VALID_POINT(p_guild_instance) )
						{
							p_guild_instance->on_guild_relay_war(p_guild);
							m_bInitDoor = TRUE;
						}
					}

				}
				
			}

			// 到时间了,或者都准备好了
			if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= WAR_PREPARE_TIME )
			{
				set_guild_war_state(EGWS_WAR);
				m_byRebornTime = 10 + m_dw_member_number / 4;
				send_reborn_time_to_guild();
				dw_begin_time = GetCurrentDWORDTime();
				
				// 处理没人的情况
				
				if (isDefenter()) // 防守方
				{
					if (p_enemy->get_guild_war().get_member_number() <= 0) //对方没人
					{
						m_bWin = TRUE;
						p_enemy->get_guild_war().m_bLost = TRUE;
					}
					else if (m_dw_member_number <= 0)
					{
						m_bLost = TRUE;
						p_enemy->get_guild_war().m_bWin = TRUE;
					}
				}
				//else // 进攻方
				//{
				//	if (m_dw_member_number <= 0) // 没人
				//	{
				//		war_outcome(FALSE);
				//	}
				//	else if(p_enemy->get_guild_war().get_member_number() <= 0) // 对方没人
				//	{
				//		war_outcome(TRUE);
				//	}
				//}
				
	

				// 处理开战相关
				if (isDefenter())
				{
					DWORD dw_guild_instance_id = p_guild->get_guild_instance_id();
					if( VALID_POINT(dw_guild_instance_id) )
					{
						DWORD dw_guild_map_id = get_tool()->crc32(szGuildMapName);
						Map* p_map = g_mapCreator.get_map(dw_guild_map_id, dw_guild_instance_id);

						if( VALID_POINT(p_map) )
						{
							map_instance_guild* p_guild_instance = dynamic_cast<map_instance_guild*>(p_map);
							if( VALID_POINT(p_guild_instance) )
							{
								p_guild_instance->on_guild_start_war(p_guild);
							}
						}

					}
				}
			}
			break;
		}
	case EGWS_WAR:
		{
			if(st_war_broadcast.n_time > 0)
			{
				if(CalcTimeDiff(st_war_broadcast.dw_broadcast_time, GetCurrentDWORDTime()) <= 0)
				{
					NET_SIS_guild_war_broad send;
					send.dwGuildID = p_guild->get_guild_att().dwID;
					send.dwEnemyGuildID = dw_war_guild_id;
					send.eGuildWarState = EGWS_WAR;
					send.byTime = st_prepare_broadcast.n_time;
					p_guild->send_guild_message(&send, send.dw_size);

					IncreaseTime(st_war_broadcast.dw_broadcast_time, st_war_broadcast.n_space_time);
					st_war_broadcast.n_time--;
				}
			}

			if(CalcTimeDiff(GetCurrentDWORDTime(), dw_begin_time) >= WAR_TIME_LIMIT)
			{
				if (isDefenter())
				{
					war_outcome(TRUE);
				}
				else
				{
					war_outcome(FALSE);
				}
			}

			if (m_bWin)
			{
				war_outcome(TRUE);
			}
			if (m_bLost)
			{
				war_outcome(FALSE);
			}

			break;
		}
	default:
		break;
	}
}

VOID  guild_war::war_outcome(BOOL bWin)
{
	NET_SIS_guild_war_outcome_broad send;

	//if(dw_achievement == dw_enemy_achievement)
	//{
	//	if(dw_declare_guild_id != INVALID_VALUE)
	//	{
	//		send.dwWinGuild = p_guild->get_guild_att().dwID;
	//		send.dwAbortGuild = dw_war_guild_id;
	//		send.bEqual = TRUE;
	//	}
	//	reset();
	//}

	set_guild_war_state(EGWS_NULL);
	
	tagGuildWarHistory sWarHistory;
	sWarHistory.dw_time = GetCurrentDWORDTime();
	sWarHistory.dw_guild_id = p_guild->get_guild_att().dwID;
	sWarHistory.dw_enemy_id = dw_war_guild_id;
	sWarHistory.dw_guild_id = p_guild->get_guild_att().dwID;
	guild* pEnemyGuild = g_guild_manager.get_guild(dw_war_guild_id);
	if (VALID_POINT(pEnemyGuild))
	{
		sWarHistory.dw_enemy_leader_id = pEnemyGuild->get_guild_att().dwLeaderRoleID;
	}

	if(bWin)
	{
		send.dwWinGuild = p_guild->get_guild_att().dwID;
		send.dwAbortGuild = dw_war_guild_id;
		send.bEqual = FALSE;
		_tcsncpy(send.szWinGuildName, p_guild->get_guild_att().str_name.c_str(), p_guild->get_guild_att().str_name.size());
		guild* pEnemy = g_guild_manager.get_guild(dw_war_guild_id);
		if (VALID_POINT(pEnemy))
		{
			_tcsncpy(send.szAbortGuildName, pEnemy->get_guild_att().str_name.c_str(), pEnemy->get_guild_att().str_name.size());
		}
		send.bDefenter = isDefenter();
		INT32 n_fund = GuildHelper::getGuildWarFund(by_enemy_level);
		INT32 n_prosperity = 1000 * by_enemy_level ;

		p_guild->get_upgrade().SetFacilitesReliveTime(TRUE);

		p_guild->increase_guild_fund(INVALID_VALUE, n_fund, elcid_guild_declarewar);

		p_guild->increase_prosperity(INVALID_VALUE, n_prosperity, elcid_guild_declarewar);
		p_guild->decrease_guild_peace(INVALID_VALUE, 50, elcid_guild_declarewar);


		// 处理祷告者
		INT nLest = p_guild->get_upgrade().GetFacilitiesLevel(EFT_Lobby)+1-p_guild->GetDaogaoNumber(TRUE);

		if (nLest > 0)
		{
			DWORD dwAddDaogao = min(min(pEnemy->GetDaogaoNumber(), 2), nLest);
			p_guild->ActiveDaogaoNumber(dwAddDaogao, true);
			send.dwDaogao = dwAddDaogao;
		}
		send.nFund = n_fund;

		// 处理祷告者
		if (pEnemy->GetDaogaoNumber() >= 1)
		{
			DWORD dwAddDaogao = min(pEnemy->GetDaogaoNumber(), 2);
			pEnemy->ActiveDaogaoNumber(dwAddDaogao, false);
		}


		//INT32 nSilver = (6000000 + 6000000 * by_enemy_level) * m_fParam / m_dw_Max_num;
		// 给在线的人发奖
		//MAP_GUILD_MEMBER::map_iter iter = p_guild->get_guild_member_map().begin();
		//tagGuildMember* p_guild_member = NULL;
		//while(p_guild->get_guild_member_map().find_next(iter, p_guild_member))
		//{
		//	if(!VALID_POINT(p_guild_member))
		//		continue;


		//	if (!p_guild_member->bWar)
		//		continue;
		//	

		//	Role* pRole = g_roleMgr.get_role(p_guild_member->dw_role_id);
		//	if (VALID_POINT(pRole))
		//	{
		//		//pRole->GetCurMgr().IncBagSilver(nSilver, elcid_guild_win_war);
		//		p_guild->change_member_contribution(pRole->GetID(), 100, TRUE);
		//	}
		//}

		// 把对方的鼎炉占领过来
		//if (VALID_POINT(pEnemyGuild))
		//{
		//	if (pEnemy->get_tripod_id() != INVALID_VALUE)
		//	{
		//		Map* pMap = g_mapCreator.get_map(pEnemy->get_tripod_map_id(), INVALID_VALUE);
		//		if (VALID_POINT(pMap))
		//		{
		//			Creature* pCreature = pMap->find_creature(pEnemy->get_tripod_id());
		//			if (VALID_POINT(pCreature))
		//			{
		//				if (p_guild->get_guild_att().byLevel >= (MAX_GUILD_LEVEL - 1))
		//				{
		//					pCreature->SetGuildID(p_guild->get_guild_att().dwID);
		//					p_guild->set_tripod_id(pCreature->GetID());
		//					p_guild->set_tripod_map_id(pMap->get_map_id());
		//					p_guild->send_tripod_message();

		//					if(p_guild->get_guild_facilities().IsLobbyUsing())
		//					{
		//						pCreature->SetState(ES_Occupied);
		//					}
		//					else
		//					{
		//						pCreature->UnsetState(ES_Occupied);
		//					}

		//					NET_SIS_Guild_Tripod send;
		//					_tcsncpy(send.szGuildName, p_guild->get_guild_att().str_name.c_str(), sizeof(send.szGuildName));
		//					g_roleMgr.send_world_msg(&send, send.dw_size);

		//				}
		//				else
		//				{
		//					pCreature->SetGuildID(INVALID_VALUE);
		//					pEnemy->set_lost_tripod_time(g_world.GetWorldTime());
		//					send.bTripodLost = TRUE;
		//					pCreature->UnsetState(ES_Occupied);
		//				}

		//				pEnemy->set_tripod_id(INVALID_VALUE);
		//				pEnemy->set_tripod_map_id(INVALID_VALUE);
		//				pEnemy->send_tripod_message();

		//			}
		//		}
		//	}
		//}

		if (isDefenter())
		{
			sWarHistory.e_war_history_type = EWHT_Defent_Win;
		}
		else
		{
			sWarHistory.e_war_history_type = EWHT_Offensive_Win;
		}

		reset();

		g_roleMgr.send_world_msg(&send, send.dw_size);

	}
	else
	{
		
		INT32 n_fund = GuildHelper::getGuildWarFund(p_guild->get_guild_att().byLevel);
		//INT32 n_prosperity = 200 * p_guild->get_guild_att().byLevel;

		p_guild->get_upgrade().SetFacilitesReliveTime(FALSE);

		p_guild->decrease_guild_fund(INVALID_VALUE, n_fund, elcid_guild_declarewar);

		//p_guild->decrease_prosperity(INVALID_VALUE, n_prosperity, elcid_guild_declarewar);
		p_guild->decrease_guild_peace(INVALID_VALUE, 50, elcid_guild_declarewar);


		if(p_guild->get_guild_att().nProsperity <= 0)
		{
			p_guild->get_upgrade().ChangeFacilitesLevel(EFT_Lobby, FALSE);

			p_guild->get_upgrade().SaveUpgradeInfo2DB(EFT_Lobby);

		}

		if (isDefenter())
		{
			sWarHistory.e_war_history_type = EWHT_Defent_Fail;
		}
		else
		{
			sWarHistory.e_war_history_type = EWHT_Offensive_Fail;
		}

	}
	p_guild->add_war_history(sWarHistory);
	p_guild->get_guild_member().reset_in_war();
	
	if (!isDefenter())
	{
		p_guild->decrease_guild_fund(INVALID_VALUE, GUILDWARMONERY, elcid_guild_declarewar);
	}

	DWORD dw_guild_instance_id = p_guild->get_guild_instance_id();
	if( VALID_POINT(dw_guild_instance_id) )
	{
		map_instance_guild* p_guild_instance = p_guild->get_guild_map();
		if( VALID_POINT(p_guild_instance) )
		{
			p_guild_instance->on_guild_end_war(p_guild);
		}
	}

	reset();

}

VOID guild_war::remove_guild_war()
{
	DWORD dw_guild_id = INVALID_VALUE;
	if(e_guild_war_state == EGWS_Declare)
	{
		dw_guild_id = dw_bedeclare_guild_id;
	}

	if(e_guild_war_state == EGWS_BeDeclare)
	{
		dw_guild_id = dw_declare_guild_id;
	}

	if(e_guild_war_state == EGWS_Prepare || e_guild_war_state == EGWS_WAR)
	{
		dw_guild_id = dw_war_guild_id;
	}

	if(dw_guild_id != INVALID_VALUE)
	{
		guild* p_enemy_guild = g_guild_manager.get_guild(dw_guild_id);
		if(VALID_POINT(p_enemy_guild))
		{
			p_enemy_guild->get_guild_war().reset();
		}
	}
}

VOID guild_war::set_guild_war_state(EGuildWarState e_guild_war_state_, const tstring& name, BOOL bDefenter, BOOL bMianzhan) 
{ 
	e_guild_war_state = e_guild_war_state_; 
	dw_begin_time = g_world.GetWorldTime();


	NET_SIS_Guil_war_state_change send;
	send.eState = e_guild_war_state_;
	send.bDefenter = bDefenter;
	send.bUseMianzhan = bMianzhan;
	_tcsncpy(send.szEnemyGuildName, name.c_str(), name.size());
	p_guild->send_guild_message(&send, send.dw_size);
}

VOID guild_war::send_reborn_time_to_guild()
{
	NET_SIS_War_Reborn_time send;
	send.byTime = m_byRebornTime;
	p_guild->send_guild_message(&send, send.dw_size);
}

DWORD guild_war::get_max_number()
{
	return 23 + p_guild->get_upgrade().GetFacilitiesLevel(EFT_Bank) * 2;
}
