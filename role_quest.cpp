/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma warning(disable:4390)	

/**
 *	@file		role_quest
 *	@author		mawenhong
 *	@date		2010/11/04	initial
 *	@version	0.0.1.0
 *	@brief		人物任务
 */
#include "StdAfx.h"

#include "player_session.h"
#include "db_session.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/script_protocol.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/quest_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "db_session.h"
#include "map.h"
#include "map_instance.h"
#include "world.h"
#include "map_creator.h"
#include "map_mgr.h"
#include "att_res.h"
#include "skill.h"
#include "buff.h"
#include "quest.h"
#include "quest_mgr.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "item_creator.h"
#include "role_mgr.h"
#include "title_mgr.h"
#include "master_prentice_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "TeamRandShareMgr.h"

// 从NPC处接取任务
INT Role::accept_quest_from_npc(UINT16 quest_id, DWORD dw_npc)
{
	// 任务数据
	const tagQuestProto* p_proto = g_questMgr.get_protocol(quest_id);
	if( !VALID_POINT(p_proto) )	return E_QUEST_NOT_EXIST;

	Creature* p_npc = NULL;
	if( !VALID_POINT(dw_npc) )
	{//是否需要从NPC处接取
		if( g_questMgr.is_need_accept_npc(quest_id) )
			return E_QUEST_NPC_NOT_EXIST;
	}
	else
	{// NPC是否正确
		p_npc = get_map()->find_creature(dw_npc);
		if( !VALID_POINT(p_npc) ) return E_QUEST_NPC_NOT_EXIST;
		else if( !p_npc->IsInDistance(*this, 500.0f) ) return E_QUEST_NPC_TOO_FAR;
		else if( !g_questMgr.can_accept_from_npc(p_npc, quest_id) )return E_QUEST_NOT_EXIST;
	}

	// 接取条件验证
	INT n_index = INVALID_VALUE;
	INT n_ret = accept_quest_check(quest_id, n_index, p_npc);

	// 验证通过,接取任务
	if( E_Success == n_ret )
	{
		add_quest(p_proto, n_index);
		quests_[n_index].on_accept(p_npc);
		if( p_proto->loop_rand_quest_flag ){ 
			DecCirlePerdayNumber(); SendCirleQuest();
		}
	}

	return n_ret;
}

// 从触发器处获得一个任务
INT Role::accept_quest_from_trigger(UINT16 quest_id, DWORD trigger_id)
{
	const tagQuestProto* p_proto = g_questMgr.get_protocol(quest_id);
	if( !VALID_POINT(p_proto) )	return E_QUEST_NOT_EXIST;

	//是否在触发器里面
	if( !get_map()->is_in_trigger(this, trigger_id) )
		return E_CanTakeQuest_FAILED_WRONG_TRIGGER;

	// 获得触发器任务序列号
	DWORD dw_quest_serial = get_map()->get_trigger_quest_serial(trigger_id);
	if( !VALID_POINT(dw_quest_serial) )
		return E_CanTakeQuest_FAILED_WRONG_TRIGGER;

	// 得到任务是否需要这个触发器
	INT n = 0;
	for(; n < QUEST_TRIGGERS_COUNT; ++n)
	{
		if( 0 == p_proto->accept_req_trriger[n] ) continue;
		if( dw_quest_serial == p_proto->accept_req_trriger[n] ) break;
	}

	if( n >= QUEST_TRIGGERS_COUNT ) 
		return E_CanTakeQuest_FAILED_WRONG_TRIGGER;


	// 判断接取条件
	INT n_index = INVALID_VALUE;
	INT n_ret = accept_quest_check(quest_id, n_index);

	if( E_Success == n_ret )
	{
		add_quest(p_proto, n_index);
		quests_[n].on_accept(NULL);
	}

	return n_ret;
}

// 某个任务是否可以接取
INT Role::accept_quest_check(UINT16 quest_id, INT& n_index, Creature* p_npc, BOOL IDSpcial)
{
	const tagQuestProto* p_proto = g_questMgr.get_protocol(quest_id);
	if( !VALID_POINT(p_proto) )	return E_QUEST_NOT_EXIST;

	//! 如果不是从悬赏榜里接取的环随机任务需要验证
	if(!IDSpcial)// 悬赏任务将不做这些检查
	{
		if( p_proto->loop_rand_quest_flag ){
			if(!circle_quest_man_.Exist(quest_id))
				return E_QUEST_NOT_EXIST;
			if(GetCirclePerdayNumber() <= 0)
				return E_CanTakeQuest_FAILED_WRONG_TIMES;
		}

		//1. 是否已经接过该任务
		BOOL is_get = is_have_quest(quest_id);
		BOOL is_done = is_done_quest(quest_id);
		BOOL is_mutex = FALSE, is_uncomplete= FALSE;

		if(VALID_POINT(p_proto->uncomplete_quest_id))
			is_mutex = is_done_quest(p_proto->uncomplete_quest_id) || is_have_quest(p_proto->uncomplete_quest_id);

		if(VALID_POINT(p_proto->undone_quest_id)) 
			is_uncomplete = is_done_quest(p_proto->undone_quest_id);


		if(is_mutex || is_uncomplete) return INVALID_VALUE;
		if(is_get) return E_CanTakeQuest_ALREADY_TAKE;


		//2. 是否做过该任务
		if(is_done && !p_proto->period)
		{
			// 不可重复
			if(!p_proto->repeatable) return E_CanTakeQuest_ALREADY_DONE;

			// 检查次数限制的
			if(0 != p_proto->accept_times )
			{
				INT n_times = get_done_quest_times(quest_id);
				if( n_times >= p_proto->accept_times )
					return E_CanTakeQuest_FAILED_WRONG_TIMES;
			}
		}
	}

	//3. 前置任务检查
	BOOL b_pre1 = TRUE, b_pre2 = TRUE;
	if( p_proto->prev_quest_id ) b_pre1 = is_done_quest(p_proto->prev_quest_id);
	if( p_proto->prev_quest2_id ) b_pre2 = is_done_quest(p_proto->prev_quest2_id);

	if( p_proto->prev_quest_relation == 0 )
	{//前序任务必须都完成
		if( !b_pre1 || !b_pre2 ) return E_CanTakeQuest_FAILED_PREV;
	}
	//完成一个前序任务即可
	else if( !(b_pre1||b_pre2) ) return E_CanTakeQuest_FAILED_PREV;


	//4. 检测周期性任务的时间
	if(p_proto->period)
	{
#define DWORDTIME_DEC_TO_MONDAY(T,WEEK) do{\
	(T).sec = 0; (T).min = 0; (T).hour = 12; \
	if((WEEK) != EWeek_MON)(T) = DecreaseTime((T), ((WEEK) ?((WEEK) - 1):(6) ) * 3600 * 24);}while(0);

#define DWORDTIME_IS_SAME_DAY(TL, TR)\
		((TL).year == (TR).year && (TL).month == (TR).month && (TL).day == (TR).day)

		tagDWORDTime start_time; start_time = get_done_quest_accept_time(quest_id);
		DWORD start_week_day = WhichWeekday(start_time);
		tagDWORDTime current_time = GetCurrentDWORDTime();
		DWORD current_week_day = WhichWeekday(current_time);
		//每日任务次数检查
		if (EQuestPeriodic_DAY == p_proto->period_type)
		{
			//gx add 2013.8.17
			//新增每日任务次数判断
			//任务ID介于500与1000之间，偶数时试炼，奇数是魔物狩猎
			if (p_proto->id >= 500 && p_proto->id < 1000)
			{
				if (0 == p_proto->id%2)
				{
					if (E_Success != role_get_daily_quest_xlsl())
					{
						return E_CanTakeQuest_Tomorrow;
					}
				}
				else
				{
					if (E_Success != role_get_daily_quest_mowu())
					{
						return E_CanTakeQuest_Tomorrow;
					}
				}
			}
			//end
		}
		// 每天任务
		if((EQuestPeriodic_DAY == p_proto->period_type) && VALID_VALUE(start_time))
		{
			BOOL is_done = is_done_quest(quest_id);
			if(DWORDTIME_IS_SAME_DAY(start_time, current_time))
			{
				if(is_done)
				{
					// 不可重复
					if(!p_proto->repeatable) return E_CanTakeQuest_ALREADY_DONE;

					// 检查次数限制的
					if(0 != p_proto->accept_times )
					{
						INT n_times = get_done_quest_times(quest_id);
						if( n_times >= p_proto->accept_times )
							return E_CanTakeQuest_Tomorrow;
					}
				}
			}
		}

		// 每周任务
		if (EQuestPeriodic_WEEK == p_proto->period_type && VALID_VALUE(start_time))
		{
			if(DWORDTIME_IS_SAME_DAY(start_time, current_time)) 
				return E_CanTakeQuest_Week;

			if(p_proto->week != EWeek_ANY){
				if(current_week_day != (EWeek)p_proto->week){
					return E_CanTakeQuest_Week;
				}
			}else if(start_week_day != EWeek_SUN){		
				tagDWORDTime start_tmp = start_time, current_tmp = current_time;
				
				DWORDTIME_DEC_TO_MONDAY(start_tmp, start_week_day); 
				DWORDTIME_DEC_TO_MONDAY(current_tmp, current_week_day);
				if(DWORDTIME_IS_SAME_DAY(start_tmp, current_tmp)) 
					return E_CanTakeQuest_Week;
			}
		}
#undef DWORDTIME_DEC_TO_MONDAY
#undef DWORDTIME_IS_SAME_DAY
	}

	//5. 检测等级限制
	if( 0 != p_proto->accept_req_min_level )
	{
		if( get_level() < p_proto->accept_req_min_level )
			return E_CanTakeQuest_FAILED_LOW_LEVEL;
	}

	if( 0 != p_proto->accept_req_max_level )
	{
		if( get_level() > p_proto->accept_req_max_level )
			return E_CanTakeQuest_FAILED_LOW_LEVEL;
	}

	//6. 检测性别限制
	if( 0 != p_proto->sex )
	{
		BYTE bySex = GetSex();
		if( 0 == bySex && 1 == p_proto->sex )
			return E_CanTakeQuest_FAILED_WRONG_SEX;
		
		if( 1== bySex && 2 == p_proto->sex )
			return E_CanTakeQuest_FAILED_WRONG_SEX;
	}

	//7. 检测技能限制
	for(INT n = 0; n < QUEST_SKILLS_COUNT; n++)
	{
		if( !VALID_POINT(p_proto->accept_req_skill[n]) ) continue;

		Skill* p_skill = GetSkill(p_proto->accept_req_skill[n]);
		if( !VALID_POINT(p_skill) || p_skill->get_level() < p_proto->accept_req_skill_val[n] )
			return E_CanTakeQuest_FAILED_MISSING_SKILLS;
	}

	//8. 检测属性限制
	for(INT n = 0; n < QUEST_ATTS_COUNT; n++)
	{
		if( ERA_Null >= p_proto->accept_req_att[n] ||
			ERA_End <= p_proto->accept_req_att[n] )
			continue;

		if( GetAttValue(p_proto->accept_req_att[n]) < p_proto->accept_req_att_val[n] )
			return E_CanTakeQuest_FAILED_MISSING_Att;
	}

	//9. 检测物品限制
	for(INT n = 0; n < QUEST_ITEMS_COUNT; n++)
	{
		if( !VALID_POINT(p_proto->accept_req_item[n]) ) continue;

		INT nTotal = GetItemMgr().GetBagSameItemCount(p_proto->accept_req_item[n]);
		nTotal += GetItemMgr().GetQuestBagSameItemCount(p_proto->accept_req_item[n]);
		
		if( nTotal < p_proto->accept_req_item_num[n] )
			return E_CanTakeQuest_FAILED_MISSING_ITEMS;
	}

	//10. 给物品判断
	if(VALID_POINT(p_proto->src_item) && VALID_POINT(p_proto->src_item_num))
	{
		INT16 bag_use = 0, quest_use = 0, max_lap = 0;
		GetItemMgr().CalSpaceUsed(p_proto->src_item, p_proto->src_item_num, bag_use, quest_use, max_lap);
		if( GetItemMgr().GetBag().GetFreeSpaceSize()<bag_use &&
			GetItemMgr().GetQuestItemBag().GetFreeSpaceSize() < quest_use )
		{
			return E_CanTakeQuest_FAILED_BAGFULL;
		}
	}

	for(int n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if(!VALID_POINT(p_proto->accept_cant_has_item[n])) continue;

		if( GetItemMgr().GetBagSameItemCount(p_proto->accept_cant_has_item[n]))
			return E_CanTakeQuest_FAILED_CantHasItem;
	}

	//11. 脚本判断
	INT tErrorCode = E_CanTakeQuest_FAILED_SCRIPT;
	const quest_script* p_script = g_ScriptMgr.GetQuestScript(quest_id);
	if( VALID_POINT(p_script) && !p_script->check_accept(quest_id, this, p_npc, tErrorCode) )
		return tErrorCode != E_Success ? tErrorCode : E_CanTakeQuest_FAILED_SCRIPT;

	//12. 帮会
	if(p_proto->bNeedGuild)
	{
		if(!VALID_POINT(GetGuildID()))
			return E_CanTakeQuest_FAILED_GUILD;
	}

	//13. 有师傅
	if(p_proto->bNeedMaster)
	{
		if(!VALID_POINT(get_master_id())) 
			return E_CanTakeQuest_FAILED_MASTER;
	}

	//14. 有徒弟
	if(p_proto->bNeedPrentice)
	{
		if(!g_MasterPrenticeMgr.has_prentice(GetID()))
			return E_CanTakeQuest_FAILED_PRENTICE;
	}

	if(p_proto->bBangZhu)
	{
		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
		if(!VALID_POINT(pGuild) || GetID() != pGuild->get_guild_att().dwLeaderRoleID)
			return E_CanTakeQuest_FAILED_OnlyBangZhu;
	}

	if(p_proto->accept_guild_level_min || p_proto->accept_guild_level_max)
	{
		guild* pGuild = g_guild_manager.get_guild(GetGuildID());
		if(!VALID_POINT(pGuild))
			return E_CanTakeQuest_FAILED_GUILD_LEVEL;
		BYTE guild_level = pGuild->get_guild_att().byLevel;
		if(guild_level < p_proto->accept_guild_level_min || guild_level > p_proto->accept_guild_level_max)
			return E_CanTakeQuest_FAILED_GUILD_LEVEL;
	}

	//15. 取得新任务数组索引
	for(n_index = 0; n_index < QUEST_MAX_COUNT; ++n_index)
		if( !quests_[n_index].valid() ) break;

	if( n_index >= QUEST_MAX_COUNT )
		return E_CanTakeQuest_FAILED_QUEST_NUM_FULL;

	if( p_proto->accept_vip_level ){
		int vl = GET_VIP_EXTVAL(GetTotalRecharge(), VIP_LEVEL, int);
		if(vl < p_proto->accept_vip_level)
			return E_CanTakeQuest_FAILED_VIP_LEVEL;
	}

	return E_Success;
}

// 增加任务
BOOL Role::add_quest(const tagQuestProto* p_proto, INT n_index, BOOL IDSpcial)
{
	if( !VALID_POINT(p_proto) ) return FALSE;
	if( n_index < 0 || n_index >= QUEST_MAX_COUNT ) return FALSE;
	if( quests_[n_index].valid( ) ) return FALSE;

	// 添加任务
	current_quest_.add(QuestIDHelper::GenerateID(p_proto->id, IDSpcial), &quests_[n_index]);
	quests_[n_index].init(p_proto, this, n_index);
	quests_[n_index].set_quest_track(can_track_quest());
	if(quests_[n_index].get_quest_flag().dwQuestTrack) set_track_number_mod(1);
	quests_[n_index].get_quest_flag().dwTakeFromGuerdon = IDSpcial ? 1 : 0;

	{// 发给客户端任务数据
		NET_SIS_add_quest send;
		send.u16QuestID = QuestIDHelper::GenerateID(p_proto->id, IDSpcial);
		//gx modify 2013.5.13
		send.dwFlag = quests_[n_index].get_quest_flag().dwQuestTrack;//仅考虑任务是否被追踪
		/*quests_[n_index].fill_quest_init_item(send.n16ItemCount);
		quests_[n_index].fill_quest_dynamic_target(&send.DynamicTarget, send.dw_size);
		quests_[n_index].fill_quest_dynmaic_reward(&send.DynamicReward, send.dw_size);
		quests_[n_index].fill_quest_script_var(&send.ScriptSetData, send.dw_size);*/
		//end
		SendMessage(&send, send.dw_size);
	}

	{// 发送给数据库保存
		NET_DB2C_accept_quest send;
		send.dw_role_id = GetID();
		send.accept_quest_.u16QuestID = QuestIDHelper::GenerateID(p_proto->id, IDSpcial);
		send.accept_quest_.dwAcceptTime = quests_[n_index].get_accept_time();
		//gx modify nNewFlag更改含义为标识是否是每日任务，1代表是，0代表常规任务
		//若是每日任务
		if (p_proto->period && EQuestPeriodic_DAY == p_proto->period_type)
		{
			send.accept_quest_.nNewFlag = 1;
		}
		//若不是
		else
		{
			send.accept_quest_.nNewFlag = 0;
		}
		g_dbSession.Send(&send, send.dw_size);
	}

	//1. 接任务时需要删除某些任务物品
	if( p_proto->del_req_item )
	{
		for(INT n = 0; n < QUEST_ITEMS_COUNT; n++)
		{
			if( !VALID_POINT(p_proto->accept_req_item[n]) ) break;

			GetItemMgr().RemoveFromRole(p_proto->accept_req_item[n], 
				(INT32)p_proto->accept_req_item_num[n], 
				(DWORD)elcid_quest_accept);
		}
	}

	//2. 接任务时会给任务物品
	if( VALID_POINT(p_proto->src_item) )
	{
		GetItemMgr().Add2Role(EICM_AccQuest, p_proto->id, 
			p_proto->src_item, p_proto->src_item_num, EIQ_Null, elcid_quest_accept,  p_proto->src_item_bind);
	}

	return TRUE;
}
// WARNING !!!!! 悬赏任务专用 (!!!! 任何系统都不能调用此函数，否则会有多线程问题)
INT Role::complete_quest_ex(UINT16 quest_id, DWORD dw_npc, INT choice_index)
{
	//1.  基本判断
	quest* p_quest = current_quest_.find(quest_id);
	if( !VALID_POINT(p_quest) ) return E_QUEST_NOT_EXIST;

	const tagQuestProto* p_proto = p_quest->get_protocol();
	if( !VALID_POINT(p_proto) ) return E_QUEST_NOT_EXIST;

	BOOL bCheck = p_quest->get_quest_flag().dwQuestBeGuerdon  && 
									p_quest->get_quest_flag().dwCompleteByReciver;
	
	if(!bCheck) return INVALID_VALUE;


	//2. 任务检查阶段
	Creature* p_npc = NULL;
	if( VALID_POINT(dw_npc) )
	{
		p_npc = get_map()->find_creature(dw_npc);

		if( !VALID_POINT(p_npc) ) return E_QUEST_NPC_NOT_EXIST;
		else if( ! p_npc->IsInDistance(*this, 500.0f) )return E_QUEST_NPC_TOO_FAR;
		else if( !g_questMgr.can_complete_from_npc(p_npc, p_quest->get_id()) )return E_QUEST_NOT_EXIST;
	}
	else
	{
		if( g_questMgr.is_need_complete_npc(p_quest->get_id()) )
			return E_QUEST_NPC_NOT_EXIST;
	}

	INT tError = E_CanCompleteQuest_BAG_FULL;
	if(!p_quest->complete_check_ex(choice_index, p_npc, &tError))
		return tError;


	//3. 完成阶段

	p_quest->set_complete_flag(TRUE);

	// 调用脚本的完成时函数
	p_quest->on_complete(p_npc);

	// 给奖
	reward_quest(p_quest, choice_index);

	// 发送称号消息
	GetAchievementMgr().UpdateAchievementCriteria(ete_quest_complete, p_quest->get_id(), 1);
	
	if (p_proto->loop_rand_quest_flag)
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_loop_rand_quest, 1);
	}
	if (p_proto->team_rand_share)
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_team_rand_share_quest, 1);
	}
	// 删除任务
	remove_quest(quest_id, TRUE);

	return E_Success;
}
// 完成任务
INT Role::complete_quest(UINT16 quest_id, DWORD dw_npc, INT choice_index, UINT16& next_quest)
{
	quest* p_quest = current_quest_.find(quest_id);
	if( !VALID_POINT(p_quest) ) return E_QUEST_NOT_EXIST;

	const tagQuestProto* p_proto = p_quest->get_protocol();
	if( !VALID_POINT(p_proto) ) return E_QUEST_NOT_EXIST;


	// 检测任务是否完成
	INT n_ret = complete_quest_check(p_quest, dw_npc, choice_index);

	// 可以交任务
	if( E_Success == n_ret )
	{
		p_quest->set_complete_flag(TRUE);

		// 调用脚本的完成时函数
		Creature* p_npc = get_map()->find_creature(dw_npc);
		p_quest->on_complete(p_npc);

		// 给奖
		reward_quest(p_quest, choice_index);

		// 发送称号消息
		GetAchievementMgr().UpdateAchievementCriteria(ete_quest_complete, p_quest->get_id(), 1);
		
		if (p_proto->loop_rand_quest_flag)
		{
			GetAchievementMgr().UpdateAchievementCriteria(eta_loop_rand_quest, 1);
		}

		if (p_proto->team_rand_share)
		{
			GetAchievementMgr().UpdateAchievementCriteria(eta_team_rand_share_quest, 1);
		}

		// 删除任务
		remove_quest(quest_id, TRUE);
		//对每日任务次数累加,任务ID介于500与1000之间，偶数时试炼，奇数是魔物狩猎 gx add
		if (p_proto->id >= 500 && p_proto->id < 1000)
		{
			if (0 == p_proto->id%2)
			{
				role_set_daily_quest_xlsl();
			}
			else
			{
				role_set_daily_quest_mowu();
			}
		}
		//end

		// 后续自动接取任务
		if(p_proto->auto_add_quest) next_quest = p_proto->next_quest_id;

		auto_rand_circle_quest_after_complete_quest(quest_id);
		auto_rand_team_share_quest_after_complete_quest(quest_id);
	}

	return n_ret;
}

// 完成检查
INT Role::complete_quest_check(quest* p_quest, DWORD dw_npc, INT choice_index)
{
	if( !VALID_POINT(p_quest) ) return INVALID_VALUE;

	Creature* p_npc = NULL;
	if( VALID_POINT(dw_npc) )
	{
		p_npc = get_map()->find_creature(dw_npc);

		if( !VALID_POINT(p_npc) ) return E_QUEST_NPC_NOT_EXIST;
		else if( ! p_npc->IsInDistance(*this, 500.0f) )return E_QUEST_NPC_TOO_FAR;
		else if( !g_questMgr.can_complete_from_npc(p_npc, p_quest->get_id()) )return E_QUEST_NOT_EXIST;
	}
	//else
	//{
	//	if( g_questMgr.is_need_complete_npc(p_quest->get_id()) )
	//		return E_QUEST_NPC_NOT_EXIST;
	//}

	// 完成检查
	INT tError = E_CanCompleteQuest_BAG_FULL;
	if( !p_quest->complete_check(choice_index, p_npc, FALSE, &tError) )
		return tError;

	return E_Success;
}

// 删除任务
INT Role::delete_quest(UINT16 quest_id)
{
	quest* p_quest = get_quest(quest_id);
	if(!VALID_POINT(p_quest) ) return E_QUEST_NOT_EXIST;

	const tagQuestProto* p_proto = p_quest->get_protocol();
	if(!VALID_POINT(p_proto) || p_proto->no_delete) return E_FAILED_QUEST_FORBID_DELETE;
	
	p_quest->on_remove();
	remove_quest(quest_id, FALSE);

	return E_Success;
}

VOID Role::remove_quest(UINT16 quest_id, BOOL b_complete, BOOL no_del_complete_item)
{
	quest* p_quest = get_quest(quest_id);
	if( !VALID_POINT(p_quest) ) return;

	DWORD start_time = p_quest->get_accept_time();
	const tagQuestProto* p_proto = p_quest->get_protocol();
	tagQuestDynamicTarget*	pQuestDynamicTarget = p_quest->get_dynamic_target();

	//1. 移除
	current_quest_.erase(quest_id);
	if(p_quest->get_quest_flag().dwQuestTrack) set_track_number_mod(-1);

	// 完成任务
	if( b_complete )
	{
		//删除需要的物品
		if( VALID_POINT(p_proto) && p_proto->delete_item && !no_del_complete_item)
		{
			for(INT i = 0; i < QUEST_ITEMS_COUNT; i++)
			{
				if( !VALID_POINT(p_proto->complete_req_item[i]) )break;
				GetItemMgr().RemoveFromRole(p_proto->complete_req_item[i], 
						(INT32)p_proto->complete_req_item_num[i], 
						(DWORD)elcid_quest_complete);
			}
		}

		// 删除任务收集物品
		if( VALID_POINT(pQuestDynamicTarget) && EQTT_Collect == pQuestDynamicTarget->eTargetType)
		{
			for(INT n = 0; n < DYNAMIC_TARGET_COUNT; n++)
			{
				if( !VALID_VALUE(pQuestDynamicTarget->dwTargetID[n]) )break;
				GetItemMgr().RemoveFromRole(pQuestDynamicTarget->dwTargetID[n], 
					(INT32)pQuestDynamicTarget->nTargetNum[n], 
					(DWORD)elcid_quest_complete);
			}
		}


		// 删除元宝
		if(p_proto->complete_need_yuanbao)
			GetCurMgr().DecBaiBaoYuanBao( p_proto->complete_need_yuanbao, elcid_quest_complete);

		// 删除对应的所有任务物品
		if(!no_del_complete_item)
			GetItemMgr().RemoveFromRole(p_proto->id, (DWORD)elcid_quest_complete);

		// 删除元气值
		if(p_proto->del_vigour && p_proto->vigour_val)
			ModAttValue(ERA_Fortune, (INT)(0-p_proto->vigour_val));

		// 删除帮贡值
		if(p_proto->del_banggong && p_proto->banggong_val)
		{
				guild* pGuild = g_guild_manager.get_guild(GetGuildID());
				if(VALID_VALUE(pGuild)) 
					pGuild->change_member_contribution(this->GetID( ), p_proto->banggong_val, FALSE, TRUE);
		}

		// 放入完成任务列表
		tagQuestDoneSave* p_done = done_quest_.find(p_proto->id);
		if( !VALID_POINT(p_done) )
		{
			p_done = new tagQuestDoneSave;
			p_done->nTimes = 0;
			p_done->u16QuestID = p_proto->id;
			p_done->dwStartTime = start_time;
			//判断是否是每日任务
			if (p_proto->period && EQuestPeriodic_DAY == p_proto->period_type)
			{
				p_done->u16QuestFalg = 1;//每日任务
			}
			else
			{
				p_done->u16QuestFalg = 0;//常规任务
			}
			done_quest_.add(quest_id, p_done);
		}
		p_done->nTimes += 1;
		p_done->dwStartTime = start_time;

		// 发给数据库保存
		NET_DB2C_complete_quest send;
		send.dw_role_id = GetID();
		send.quest_done_.u16QuestID = quest_id;
		send.quest_done_.dwStartTime = p_done->dwStartTime;
		send.quest_done_.nTimes = p_done->nTimes;
		send.quest_done_.u16QuestFalg = p_done->u16QuestFalg;//gx add
		g_dbSession.Send(&send, send.dw_size);
	}
	// 删除任务
	else
	{
		// 删除所对应的所有任务物品
		if(!no_del_complete_item)
			GetItemMgr().RemoveFromRole(quest_id, (DWORD)elcid_quest_discard);

		// 发数据库删除
		NET_DB2C_discard_quest send;
		send.dw_role_id = GetID();
		send.u16_quest_id_ = quest_id;
		g_dbSession.Send(&send, send.dw_size);
	}

	// 删除接取任务时给的物品
	if( VALID_POINT(p_proto->src_item) && p_proto->src_item_num > 0 )
	{
		GetItemMgr().RemoveFromRole(p_proto->src_item, 
			p_proto->src_item_num, 
			(DWORD)elcid_quest_complete);
	}

	//此处清除
	p_quest->destroy();
}

// 更新NPC对话任务
VOID Role::update_quest_npc_talk(UINT16 quest_id, DWORD dw_npc)
{
	quest* p_quest = get_quest(quest_id);
	if( !VALID_POINT(p_quest) ) return;

	// 验证NPC
	Creature* p_npc = get_map()->find_creature(dw_npc);
	if( !VALID_POINT(p_npc) ) return;

	// 更新任务
	p_quest->on_event(EQE_Talk, p_npc->GetID(), p_npc->GetTypeID());
}

// 更新Trigger任务
VOID Role::update_quest_trigger(UINT16 quest_id, DWORD trigger_id)
{
	quest* p_quest = get_quest(quest_id);
	if( !VALID_POINT(p_quest) ) return;

	// 验证触发器
	if( !get_map()->is_in_trigger(this, trigger_id) ) return;

	DWORD quest_serial = get_map()->get_trigger_quest_serial(trigger_id);
	if( !VALID_POINT(quest_serial) ) return;

	// 更新任务
	p_quest->on_event(EQE_Trigger, trigger_id);
}

// 触发任务事件
VOID Role::on_quest_event(EQuestEvent quest_event, DWORD param1, DWORD param2,DWORD param3)
{
	for(INT n = 0; n < QUEST_MAX_COUNT; ++n)
	{
		if( !quests_[n].valid() ) continue;
		quests_[n].on_event(quest_event, param1, param2, param3);
	}
}

// 服务器可控对话框缺省事件
VOID Role::on_default_dialog_event(EMsgUnitType unit_type, DWORD target_id, EDlgOption option)
{
	switch (unit_type)
	{
	case EMUT_DlgTarget_Quest:
		{
			quest* p_quest = get_quest((UINT16)target_id); 
			if(!VALID_POINT(p_quest)) return;
			p_quest->on_event(EQE_DlgDefault, option);
		}
		break;
	case EMUT_DlgTarget_Item: break;
	case EMUT_DlgTarget_Creature: break;
	default: break;
	}
}

// 任务奖励
VOID Role::reward_quest(quest* p_quest, INT32 choice_index)
{
	// 防沉迷
	if (GetEarnRate() < 10000)return ;

	const tagQuestProto* p_proto = p_quest->get_protocol();
	if( !VALID_POINT(p_proto) ) return;

	EClassType e_class = GetClass( );
	BYTE roleSex = GetSex();//获取玩家性别 gx add

	//1. 动态奖励物品
	tagQuestDynamicReward* p_dynamic_reward = p_quest->get_dynamic_reward( );
	if( p_proto->target_mode == EQTM_DYNAMIC && VALID_POINT(p_dynamic_reward ) )
	{
		for(INT n = 0; n < QUEST_REW_ITEM; ++n)
		{
			if( !VALID_POINT(p_dynamic_reward->rew_item[n]) ) break;
			if( p_dynamic_reward->rew_item_num[n] <= 0 ) continue;

			GetItemMgr().Add2Role(EICM_Quest, p_quest->get_id(), 
								  p_dynamic_reward->rew_item[n], 
								 (INT16)p_dynamic_reward->rew_item_num[n], 
								 (EItemQuality)p_dynamic_reward->rew_item_quality[n], elcid_quest_rewards,
								 p_dynamic_reward->rew_item_bind[n]);
		}
	}

	//2. 任务配置按职业给奖励
	for(INT n = 0; n < QUEST_REW_ITEM * X_ClASS_TYPE_NUM; ++n)
	{
		if( !VALID_POINT(p_proto->rew_item[n]) ) break;
		if( p_proto->rew_item_num[n] <= 0 ) continue;
		if(p_proto->rew_item_classtype[n] != 0 && e_class != p_proto->rew_item_classtype[n]) continue;
		if (QUESTREWARDNOSEX != p_proto->rew_item_rolesex[n] && roleSex != p_proto->rew_item_rolesex[n]) continue;
		GetItemMgr().Add2Role(EICM_Quest, p_quest->get_id(), 
			p_proto->rew_item[n], (INT16)p_proto->rew_item_num[n], 
			(EItemQuality)p_proto->rew_item_quality[n], elcid_quest_complete,
			p_proto->rew_item_bind[n]);
	}

	//3. 可选奖励物品
	if( choice_index >= 0 && choice_index < QUEST_REW_ITEM )
	{
		BOOL b_bind = TRUE;
		EItemQuality e_quality = EIQ_Null; 
		DWORD dw_item_id = 0, dw_item_num = 0;

		if(VALID_POINT(p_dynamic_reward))
		{//动态任务可选奖励
			if( p_dynamic_reward->rew_choice_item[choice_index] )
			{
				b_bind = p_dynamic_reward->rew_choice_bind[choice_index];
				dw_item_id =  p_dynamic_reward->rew_choice_item[choice_index];
				dw_item_num = p_dynamic_reward->rew_choice_item_num[choice_index];
				e_quality = (EItemQuality)p_dynamic_reward->rew_choice_quality[choice_index];
			}
		}
		else if( VALID_POINT( p_proto ) )
		{//不是动态任务
			INT16 index = p_proto->GetChoiceIndex( GetClass( ), choice_index );
			if( index >=0 && VALID_POINT(p_proto->rew_choice_item[choice_index]) )
			{
				b_bind = p_proto->rew_choice_bind[index];
				dw_item_id = p_proto->rew_choice_item[index];
				dw_item_num = p_proto->rew_choice_item_num[index];
				e_quality = (EItemQuality)p_proto->rew_choice_quality[index];
			}
		}

		if( VALID_POINT(dw_item_num) && VALID_POINT(dw_item_id ))
		{//给可选奖励
			GetItemMgr().Add2Role(EICM_Quest, 
								  p_quest->get_id(), 
								  dw_item_id, 
								  (INT16)dw_item_num,
								  e_quality, elcid_quest_complete, b_bind);
		}
	}

	//4. 经验
	if( p_proto->rew_xp != 0 ) ExpChange(p_proto->rew_xp);

	//5. 金钱
	if( p_proto->rew_money > 0 )
	{
		if( p_proto->bind_money_flag )
			GetCurMgr( ).IncBagBindSilver(p_proto->rew_money,  (DWORD)elcid_quest_complete);
		else
			GetCurMgr().IncBagSilver(p_proto->rew_money, (DWORD)elcid_quest_complete);
	}

	if ( p_proto->rew_yuanbao )
	{
		GetCurMgr().IncBaiBaoYuanBao(p_proto->rew_yuanbao, elcid_quest_complete);
	}

	//6. 声望
	for (INT i = 0; JDG_VALID(ERT, p_proto->rew_rep[i]) && i < QUEST_REPUTATIONS_COUNT; ++i)
		GetClanData().RepModVal(MTRANS_ERT2ECLT(p_proto->rew_rep[i]), p_proto->rew_rep_val[i]);

	//7. 贡献
	for (INT i = 0; JDG_VALID(ERCT, p_proto->rew_contribution[i]) && i < QUEST_CONTRIBUTION_COUNT; ++i)
		GetCurMgr().IncClanCon(p_proto->rew_contribution_val[i], INVALID_VALUE, MTRANS_ECCT2ECLT(p_proto->rew_contribution[i]));


	//8. 属性
	for(INT n = 0; n < QUEST_ATTS_COUNT; ++n)
	{
		if( ERA_Null >= p_proto->rew_att[n] ||
			ERA_End <= p_proto->rew_att[n] )
			break;

		if( p_proto->rew_att_val[n] == 0 )continue;

		ModAttValue(p_proto->rew_att[n], p_proto->rew_att_val[n]);
	}

	//9 战功
	if(p_proto->rew_exploits > 0)
	{
		GetCurMgr().IncExploits(p_proto->rew_exploits, elcid_quest_rewards);
	}

	guild* pGuild = g_guild_manager.get_guild(GetGuildID());
	if(VALID_VALUE(pGuild)) 
	{
		if(p_proto->rew_contributions)
			pGuild->change_member_contribution(GetID(), p_proto->rew_contributions, TRUE, TRUE);

		if(p_proto->rew_prosperity)
			pGuild->increase_prosperity(GetID(), p_proto->rew_prosperity, elcid_quest_rewards);

		if(p_proto->rew_guild_fund)
		{
			pGuild->increase_guild_fund(GetID(), p_proto->rew_guild_fund, elcid_quest_rewards);
			pGuild->increase_member_total_fund(GetID(), p_proto->rew_guild_fund/10000);
		}
	}
}

// 设置追踪
INT Role::set_track_quest(UINT16 quest_id, BYTE by_track)
{
	quest* p_quest = current_quest_.find(quest_id);
	if( !VALID_POINT(p_quest) ) return E_QUEST_NOT_EXIST;

	INT nErrorCode = E_Success;
	if( by_track == 0)
	{
		if(p_quest->get_quest_flag().dwQuestTrack)
			set_track_number_mod(-1);
		p_quest->set_quest_track(FALSE);
	}
	else 
	{
		if(!p_quest->get_quest_flag().dwQuestTrack)
		{
			p_quest->set_quest_track(can_track_quest());
			if(p_quest->get_quest_flag().dwQuestTrack)
				set_track_number_mod(1);
		}

		if(!p_quest->get_quest_flag().dwQuestTrack)
			nErrorCode = E_Failed_Track_Number_Max;
	}

	return nErrorCode;
}
//--------------------------------------------------------------------
//  完成循环随机任务后继续随机
//--------------------------------------------------------------------
VOID Role::auto_rand_circle_quest_after_complete_quest(UINT16 u16QuestID)
{
	const tagQuestProto* pProto = g_questMgr.get_protocol(u16QuestID);
	if(!VALID_POINT(pProto) || !macroIsLoopQuest(pProto)) return;

	circle_quest_man_.Remove(u16QuestID);
	rand_circle_quest();
	SendCirleQuest( );
}

VOID Role::auto_rand_team_share_quest_after_complete_quest(UINT16 u16QuestID)
{
	const tagQuestProto* pProto = g_questMgr.get_protocol(u16QuestID);
	if(!VALID_POINT(pProto) || !macroIsTeamShareRand(pProto)) return;
	//sTeamShareMgr.AddEvent(GetID(), EVT_TeamShareCirlcleNext, 0, NULL);
}

VOID Role::auto_rand_circle_quest_to_index(UINT16 u16Old)
{
	const tagQuestProto* pProto = g_questMgr.get_protocol(u16Old);
	if(!VALID_POINT(pProto) || !macroIsLoopQuest(pProto)) return;

	INT idx = circle_quest_man_.RemoveEX(u16Old);
	if(idx == -1) return;

	INT questid = rand_circle_quest_one( );
	if(questid == 0) questid = u16Old;
	circle_quest_man_.AddEX((UINT16)questid, idx);

	SendCirleQuest();
}
VOID Role::rand_circle_quest( )
{
	this->do_rand_circle_quest_from_person( );
}

//------------------------------------------------------------------------------------------------------------------
// 以下是新需求做的//mwh 2011-09-06 
//------------------------------------------------------------------------------------------------------------------
VOID Role::do_rand_circle_quest_from_person()
{
	// 根据等级取得循环随机任务列表
	UINT16  u16RandNumMax = 0;
	package_map<UINT16,UINT16>* pCircleQuestMap = NULL;
	pCircleQuestMap = g_questMgr.get_circle_quest(this->get_level( ), u16RandNumMax);
	if(!VALID_POINT(pCircleQuestMap)) return;
	if(circle_quest_man_.GetNumber() < u16RandNumMax)
	{
		for(INT n = 0; 
			n < pCircleQuestMap->size() && circle_quest_man_.GetNumber() < u16RandNumMax; 
			++n)
		{
			UINT16 u16Key = 0, u16QuestID = 0;
			if(pCircleQuestMap->rand_find(u16Key, u16QuestID))
			{
				if(!circle_quest_man_.Exist(u16QuestID))
					circle_quest_man_.Add(u16QuestID);
			}
		}
	}
}

INT Role::rand_circle_quest_one( )
{
	UINT16  u16RandNumMax = 0;
	package_map<UINT16,UINT16>* pCircleQuestMap = NULL;
	pCircleQuestMap = g_questMgr.get_circle_quest(this->get_level( ), u16RandNumMax);
	if(!VALID_POINT(pCircleQuestMap)) return 0;

	for(INT n = 0; 
		n < pCircleQuestMap->size();
		++n)
	{
		UINT16 u16Key = 0, u16QuestID = 0;
		if(pCircleQuestMap->rand_find(u16Key, u16QuestID))
		{
			if(!circle_quest_man_.Exist(u16QuestID))
				return u16QuestID;
		}
	}

	return 0;
}

BOOL Role::is_leader_set_share() const
{
	const Team* pTeam = g_groupMgr.GetTeamPtr(this->GetTeamID());
	if(!VALID_POINT(pTeam)) return FALSE;
	return pTeam->get_leader_share_circle_quest( );
}


VOID Role::SendCirleQuest( )
{
	INT vipDelta = GET_VIP_EXTVAL(GetTotalRecharge(), QUEST_REFRESH, INT);
	vipDelta -= GetDayClearData(ERDCT_QuestRefresh);
	NET_SIS_CircleQuestList send;
	send.byRefreshLeft = GetCircleRefrshNumber() + vipDelta;
	send.byCircleNumberLeft = GetCirclePerdayNumber();
	send.byCircleNumberBuyLeft = GetCircleRefrshNumberMax( );
	send.byNumber = this->get_circle_quest_man().Fill(send.u16QuestID, NULL, CIRCLEQUESTMAXNUMBER);
	SendMessage(&send, send.dw_size);
}
DWORD Role::BuyRefreshCircleQuest( )
{
 	if(GetCircleRefrshNumber( ) >= CIRCLEQUESTFRESHNUMBER)
 		return EBuyFreshNumFull;

	int powof = GetDayClearData(ERDCT_BUYREFRESHCIRCLE);
	int yuanbao  = get_buy_refresh_yuanbao(powof);

	if(GetCurMgr().GetBaiBaoYuanBao() < yuanbao)
		return EBuyFreshNumOutOfMoney;

// 	if(GetCircleRefrshNumberMax() <= 0)
// 		return EBuyFreshNumOutOfNum;

	GetCurMgr().DecBaiBaoYuanBao(yuanbao, elcid_quest_accept);

//	DecCirleRefreshMax( );
	IncCirleRefresh(1);
	ModRoleDayClearDate(ERDCT_BUYREFRESHCIRCLE);

	SendCirleQuest( );

	return E_Success;
}

VOID Role::BuyCircleQuestPerdayNumber( )
{
	if(GetCirclePerdayNumber() >=  CIRCLEQUESTPERDAYNUMBER)
		return ;
	//gx modify 2013.7.2
	//int powof = GetDayClearData(ERDCT_BUYCIRCLE) + 1;
	//powof /= 5;
	//int yuanbao  = 1 << powof;

	//if(GetCurMgr().GetBaiBaoYuanBao()  < yuanbao)
	//	return;

	//GetCurMgr().DecBaiBaoYuanBao(yuanbao, elcid_buy_circle);
	//IncCirlePerdayNumber(1);

	//ModRoleDayClearDate(ERDCT_BUYCIRCLE);

	//
	////this->ModAttValue(ERA_Fortune, 0-BUYCQUESTNUMBERCOSVIGOUR);
	//
	//SendCirleQuest( );
}