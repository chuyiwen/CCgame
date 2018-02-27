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
 *	@file		quest
 *	@author		mwh
 *	@date		2011/10/21	initial
 *	@version	0.0.1.0
 *	@brief		任务数据
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/quest_protocol.h"

#include "role.h"
#include "quest.h"
#include "quest_mgr.h"
#include "script_mgr.h"
#include "guild.h"
#include "guild_member.h"
#include "guild_manager.h"

// 
quest::quest() : ScriptData<ESD_Quest>(),
p_protocol_(NULL), p_role_(NULL),dw_accept_time_(INVALID_VALUE), 
n_index_(INVALID_VALUE), p_script_(NULL),p_dynamic_target_(NULL),
p_dynamic_reward_(NULL),p_script_var_(NULL),dw_quest_flag_(0)
{
	memset(b_invest_, false, sizeof(b_invest_));
	memset(b_trigger_, false, sizeof(b_trigger_));
	memset(b_talk_npc_, false, sizeof(b_talk_npc_));
	memset(n16_item_count_, 0, sizeof(n16_item_count_));
	memset(n16_creature_count_, 0, sizeof(n16_creature_count_));
}

//
quest::~quest()
{
	this->destroy();
}

// 初始化任务
VOID quest::init(const tagQuestProto* p_proto, Role* p_role, INT n_index)
{
	ASSERT( VALID_POINT(p_proto) && VALID_POINT(p_role) );
	ASSERT( n_index >= 0 && n_index < QUEST_MAX_COUNT );

	p_protocol_ = p_proto;
	p_role_ = p_role;
	n_index_ = n_index;
	dw_accept_time_ = g_world.GetWorldTime();
	p_dynamic_target_ = NULL;
	p_dynamic_reward_ = NULL;
	p_script_var_ = NULL;
	b_complete_ = FALSE;
	dw_quest_flag_ = 0;

	memset(b_invest_, false, sizeof(b_invest_));
	memset(b_trigger_, false, sizeof(b_trigger_));
	memset(b_talk_npc_, false, sizeof(b_talk_npc_));
	memset(n16_item_count_, 0, sizeof(n16_item_count_));
	memset(n16_creature_count_, 0, sizeof(n16_creature_count_));

	// 初始化脚本
	p_script_ = g_ScriptMgr.GetQuestScript(p_protocol_->id);

	// 初始化任务动态目标/动态奖励/脚本变量
	if( p_proto->is_dynamic( ) )
	{
		p_dynamic_target_ = new tagQuestDynamicTarget;
		p_dynamic_reward_ = new tagQuestDynamicReward;
		p_script_var_ = new tagScriptQuestVar;

		if(	VALID_POINT(p_script_) && 
			VALID_POINT(p_dynamic_target_) && 
			VALID_POINT(p_dynamic_reward_) &&
			VALID_POINT(p_script_var_) )
		{
			p_script_->on_init(p_proto->id, p_role_);
		}
	}

	// 组装玩家身上的物品个数
	init_need_item();
}

VOID quest::init(const tagQuestSave* p_save, Role* p_role, INT n_index)
{
	ASSERT( VALID_POINT(p_save) && VALID_POINT(p_role) );
	ASSERT( n_index >= 0 && n_index < QUEST_MAX_COUNT );

	p_protocol_ = g_questMgr.get_protocol(QuestIDHelper::RestoreID(p_save->u16QuestID));
	if( !VALID_POINT(p_protocol_) ) return;

	p_role_ = p_role;
	n_index_ = n_index;
	dw_accept_time_ = p_save->dwAcceptTime;
	p_dynamic_target_ = NULL;
	p_dynamic_reward_ = NULL;
	p_script_var_ = NULL;
	b_complete_ = FALSE;
	dw_quest_flag_ = p_save->dwQuestFlag;

	get_fast_code()->memory_copy(n16_creature_count_, 
							p_save->n16MonsterNum, 
							sizeof(n16_creature_count_));

	get_fast_code()->memory_copy(m_ScriptData.dwData, 
							p_save->Data.dwData, 
							sizeof(DWORD)*ESD_Quest);

	memset(b_talk_npc_, false, sizeof(b_talk_npc_));
	memset(b_trigger_, false, sizeof(b_trigger_));
	memset(n16_item_count_, 0, sizeof(n16_item_count_));
	memset(b_invest_, false, sizeof(b_invest_));

	// 初始化脚本
	p_script_ = g_ScriptMgr.GetQuestScript(p_protocol_->id);

	// 初始化任务动态目标/动态奖励/脚本变量
	if( p_protocol_->is_dynamic( ) )
	{
		p_dynamic_target_ = new tagQuestDynamicTarget;
		p_dynamic_reward_ = new tagQuestDynamicReward;
		p_script_var_ = new tagScriptQuestVar;

		if( !VALID_POINT(p_dynamic_target_)|| 
			!VALID_POINT(p_dynamic_reward_)|| 
			!VALID_POINT(p_script_var_) )
			return;

		p_dynamic_target_->eTargetType = p_save->DynamicTarget.eTargetType;
		p_dynamic_target_->dwQuestTipsID = p_save->DynamicTarget.dwQuestTipsID;
		
		get_fast_code()->memory_copy(p_dynamic_target_->dwTargetID, 
								p_save->DynamicTarget.dwTargetID, 
								sizeof(DWORD)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(p_dynamic_target_->nTargetNum, 
								p_save->DynamicTarget.nTargetNum, 
								sizeof(INT)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(p_dynamic_reward_,
								&p_save->DynamicReward, 
								sizeof(tagQuestDynamicReward));

		get_fast_code()->memory_copy(p_script_var_,
								&p_save->ScriptQuestVar, 
								sizeof(tagScriptQuestVar));
	}

	// 组装玩家身上的物品个数
	init_need_item();
}

// 初始化任务所需物品
VOID quest::init_need_item()
{
	if( !VALID_POINT(p_protocol_) ) return;

	// 动态收集物品
	if (VALID_POINT(p_dynamic_target_) && EQTT_Collect == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			if( !VALID_POINT(p_dynamic_target_->dwTargetID[i]) ) break;

			n16_item_count_[i]  = p_role_->GetItemMgr().GetBagSameItemCount(p_dynamic_target_->dwTargetID[i]);
			n16_item_count_[i] += p_role_->GetItemMgr().GetQuestBagSameItemCount(p_dynamic_target_->dwTargetID[i]);
		}

		return;
	}

	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) ) return;

	// 静态数据收集物品
	for(INT n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;

		n16_item_count_[n]  = p_role_->GetItemMgr().GetBagSameItemCount(p_protocol_->complete_req_item[n]);
		n16_item_count_[n] += p_role_->GetItemMgr().GetQuestBagSameItemCount(p_protocol_->complete_req_item[n]);
	}
}


// 生成消息结构
VOID quest::make_msg_info(OUT tagIncompleteQuestMsgInfo* p_info,OUT DWORD &dw_msg_size, OUT DWORD &dw_quest_size)
{
	ASSERT( valid() );

	dw_quest_size = sizeof(tagIncompleteQuestMsgInfo);
	p_info->u16QuestID = QuestIDHelper::GenerateID(p_protocol_->id, dw_quest_flag_.dwTakeFromGuerdon);
	p_info->dwAcceptTime = dw_accept_time_;
	p_info->dwQuestFlag = dw_quest_flag_;
	get_fast_code()->memory_copy(p_info->n16CreatureNum, n16_creature_count_, sizeof(p_info->n16CreatureNum));
	get_fast_code()->memory_copy(p_info->n16ItemNum, n16_item_count_, sizeof(p_info->n16ItemNum));

	if(  p_protocol_->is_dynamic( ) && p_script_var_&&
		p_dynamic_reward_ && p_dynamic_target_  )
	{
		p_info->DynamicTarget.eTargetType = p_dynamic_target_->eTargetType;
		p_info->DynamicTarget.dwQuestTipsID = p_dynamic_target_->dwQuestTipsID;
		get_fast_code()->memory_copy(p_info->DynamicTarget.dwTargetID, 
								p_dynamic_target_->dwTargetID, 
								sizeof(DWORD)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(p_info->DynamicTarget.nTargetNum, 
								p_dynamic_target_->nTargetNum, 
								sizeof(INT)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(&p_info->DynamicReward, 
								p_dynamic_reward_, 
								sizeof(tagQuestDynamicReward));

		get_fast_code()->memory_copy(&p_info->ScriptQuestVar, 
								p_script_var_, 
								sizeof( tagScriptQuestVar ));
	}
	else
	{
		// 修正消息大小
		INT OffSize = sizeof(tagQuestDynamicTarget)+sizeof(tagQuestDynamicReward)+sizeof(tagScriptQuestVar);
		dw_quest_size -= OffSize;  dw_msg_size -= OffSize;
	}
}

// 填充任务初始物品
VOID quest::fill_quest_init_item(OUT INT16* const n16_item_count)
{
	ASSERT( valid() );

	get_fast_code()->memory_copy(n16_item_count, n16_item_count_, sizeof(n16_item_count_));
}


// 填充任务动态目标
VOID quest::fill_quest_dynamic_target(OUT tagQuestDynamicTarget* p_target, OUT DWORD &dw_size)
{
	if( p_protocol_->is_dynamic( ) )
	{
		p_target->eTargetType = p_dynamic_target_->eTargetType;
		p_target->dwQuestTipsID = p_dynamic_target_->dwQuestTipsID;

		get_fast_code()->memory_copy(p_target->dwTargetID, 
								p_dynamic_target_->dwTargetID, 
								sizeof(DWORD)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(p_target->nTargetNum, 
								p_dynamic_target_->nTargetNum, 
								sizeof(INT)*DYNAMIC_TARGET_COUNT);
	}
	else dw_size -= sizeof(tagQuestDynamicTarget);
}


// 填充动态奖励
VOID quest::fill_quest_dynmaic_reward(OUT tagQuestDynamicReward* p_reward, OUT DWORD &dw_size)
{
	if( p_protocol_->is_dynamic( ) )
	{
		if(VALID_POINT(p_dynamic_reward_))
			get_fast_code()->memory_copy(p_reward, 
									p_dynamic_reward_, 
									sizeof(tagQuestDynamicReward));
	}
	else dw_size -= sizeof(tagQuestDynamicReward);
}

// 填充脚本设置变量
VOID quest::fill_quest_script_var(OUT tagScriptQuestVar* p_var, OUT DWORD &dw_size)
{
	if( p_protocol_->is_dynamic( ) )
	{
		if(VALID_POINT(p_script_var_))
			get_fast_code()->memory_copy(p_var, 
									p_script_var_, 
									sizeof(tagScriptQuestVar));
	}
	else
	{
		dw_size -= sizeof(tagScriptQuestVar);
	}
}

// 填充任务保存数据
VOID quest::fill_quest_save(OUT tagQuestSave* p_save)
{
	p_save->u16QuestID = QuestIDHelper::GenerateID(get_id(), get_quest_flag().dwTakeFromGuerdon);
	p_save->dwAcceptTime = get_accept_time();
	p_save->dwQuestFlag = get_quest_flag();
	//判断每日任务
	if (VALID_POINT(p_protocol_))
	{
		if (p_protocol_->period && EQuestPeriodic_DAY == p_protocol_->period_type)
		{
			p_save->u16Quest_NewFlag = 1;
		}
		else
		{
			p_save->u16Quest_NewFlag = 0;
		}
	}
	get_fast_code()->memory_copy(p_save->n16MonsterNum, 
							n16_creature_count_, 
							sizeof(p_save->n16MonsterNum));

	get_fast_code()->memory_copy(p_save->Data.dwData, 
							m_ScriptData.dwData, 
							sizeof(DWORD)*ESD_Quest);

	if(VALID_POINT(p_dynamic_target_))
	{
		p_save->DynamicTarget.eTargetType = p_dynamic_target_->eTargetType;
		p_save->DynamicTarget.dwQuestTipsID = p_dynamic_target_->dwQuestTipsID;

		get_fast_code()->memory_copy(p_save->DynamicTarget.dwTargetID, 
								p_dynamic_target_->dwTargetID, 
								sizeof(DWORD)*DYNAMIC_TARGET_COUNT);

		get_fast_code()->memory_copy(p_save->DynamicTarget.nTargetNum, 
								p_dynamic_target_->nTargetNum, 
								sizeof(INT)*DYNAMIC_TARGET_COUNT);		
	}

	if(VALID_POINT(p_dynamic_reward_))
		get_fast_code()->memory_copy(&p_save->DynamicReward, 
								p_dynamic_reward_, 
								sizeof(tagQuestDynamicReward));

	if(VALID_POINT(p_script_var_))
		get_fast_code()->memory_copy(&p_save->ScriptQuestVar, 
								p_script_var_, 
								sizeof(tagScriptQuestVar) );
}

// 接取任务时
VOID quest::on_accept(Creature* p_npc)
{
	if( VALID_POINT(p_script_) )
		p_script_->on_accept(p_protocol_->id, p_role_, p_npc);
}


// 完成任务时
VOID quest::on_complete(Creature* p_npc)
{
	if( VALID_POINT(p_script_) )
		p_script_->on_complete(p_protocol_->id, p_role_, p_npc);
}


// 删除任务时
VOID quest::on_remove()
{
	if( VALID_POINT(p_script_) )
		p_script_->on_cancel(p_protocol_->id, p_role_);
}

// 触发事件
VOID quest::on_event(EQuestEvent e_event, DWORD dw_parm1, DWORD dw_parm2, DWORD dw_parm3, DWORD dw_parm4)
{
	switch(e_event)
	{
	case EQE_Kill:
		creature_kill_event(dw_parm1, dw_parm2, dw_parm3, dw_parm4 ? dw_parm4 : 1);
		break;

	case EQE_Item:
		item_event(dw_parm1, dw_parm2, dw_parm3);
		break;

	case EQE_Talk:
		npc_talk_event(dw_parm1, dw_parm2);
		break;

	case EQE_Trigger:
		trigger_event(dw_parm1);
		break;

	case EQE_Invest:
		invest_event(dw_parm1,dw_parm2);
		break;

	case EQE_DlgDefault:
		default_dialog_event(dw_parm1);
		break;

	default:
		break;
	}
}


// 杀怪事件
VOID quest::creature_kill_event(DWORD dw_creature_type, INT n_creature_level, BOOL bAddExp, INT n_kill_num)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// 调用脚本
	if( VALID_POINT(p_script_) )
		p_script_->on_creature_kill(p_protocol_->id, p_role_, dw_creature_type, bAddExp);

	// 动态击杀目标计数
	if(VALID_POINT(p_dynamic_target_) && EQTT_Kill == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			BOOL b_find = FALSE;
			if(p_dynamic_target_->dwTargetID[i] == dw_creature_type	  ||
				p_dynamic_target_->dwTargetID[i+1] == dw_creature_type ||
				p_dynamic_target_->dwTargetID[i+2] == dw_creature_type ||
				p_dynamic_target_->dwTargetID[i+3] == dw_creature_type)
			{
				b_find = TRUE;
			}

			if( b_find && n16_creature_count_[i] < p_dynamic_target_->dwTargetID[i])
			{
				n16_creature_count_[i] += n_kill_num;
				n16_creature_count_[i] = min(n16_creature_count_[i], p_dynamic_target_->dwTargetID[i]);
				// 发送更新
				NET_SIS_quest_update_kill_creature send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nCreatureIndex = i;
				send.n16NewKillNum = n16_creature_count_[i];
				p_role_->SendMessage(&send, send.dw_size);

				break;
			}
		}
			
		return;
	}

	// 如果该任务没有杀怪，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_KILL) ) return;
	
	for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
	{
		//gx modify 2013.5.10
		//修改原因：新增任务杀怪需求：击杀若干个怪，怪物ID未指定
		//配表方法：cc_require_creature_id字段均为0，第一个value字段赋相应的值
		if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) && (p_protocol_->complete_req_creature_num[n] == 0))
			break;
		//end
		BOOL b_find = FALSE;
		if( p_protocol_->creature_level )
		{
			if( p_protocol_->complete_req_creature[n*3]	== n_creature_level ||
				p_protocol_->complete_req_creature[n*3+1]	== n_creature_level ||
				p_protocol_->complete_req_creature[n*3+2]	== n_creature_level
			)
			{
				if( n16_creature_count_[n] < p_protocol_->complete_req_creature_num[n] )
				{
					b_find = TRUE;
				}
			}
		}
		else
		{
			if( p_protocol_->complete_req_creature[n*3]	== dw_creature_type ||
				p_protocol_->complete_req_creature[n*3+1]	== dw_creature_type ||
				p_protocol_->complete_req_creature[n*3+2]	== dw_creature_type ||
				(!VALID_POINT(p_protocol_->complete_req_creature[n*3]) 
				&& (p_protocol_->complete_req_creature_num[n] > 0)//gx add 2013.5.10 修改原因同上
				))
			{
				if( n16_creature_count_[n] < p_protocol_->complete_req_creature_num[n] )
				{
					b_find = TRUE;
				}
			}
		}

		if( b_find && n16_creature_count_[n] < p_protocol_->complete_req_creature_num[n])
		{
			n16_creature_count_[n] += n_kill_num;
			n16_creature_count_[n] = min(n16_creature_count_[n], p_protocol_->complete_req_creature_num[n]);
			// 发送更新
			NET_SIS_quest_update_kill_creature send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nCreatureIndex = n;
			send.n16NewKillNum = n16_creature_count_[n];
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// 物品事件
VOID quest::item_event(DWORD dw_item_type, INT n_num, BOOL b_add)
{
	if( !VALID_POINT(p_protocol_) ) return;
	if( n_num <= 0 || b_complete_) return;

	// 动态收集物品计数
	if(VALID_POINT(p_dynamic_target_) && EQTT_Collect == p_dynamic_target_->eTargetType)
	{
		for(INT i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			BOOL b_find = FALSE;
			if(p_dynamic_target_->dwTargetID[i] == dw_item_type)
			{
				if( b_add) n16_item_count_[i] += n_num;
				else n16_item_count_[i] -= n_num;

				if(n16_item_count_[i] > p_dynamic_target_->nTargetNum[i])
						n16_item_count_[i] = p_dynamic_target_->nTargetNum[i];
				else if(n16_item_count_[i] < 0)
						n16_item_count_[i] = 0;

				b_find = TRUE;
			}

			if( b_find )
			{
				// 发送更新
				NET_SIS_quest_update_item send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nItemIndex = i;
				send.n16NewItemNum = n16_item_count_[i];
				p_role_->SendMessage(&send, send.dw_size);

				break;
			}
		}

		return;
	}

	// 如果该任务没有获得物品，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) ) return;

	// 依次找对应的物品
	for(INT n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;

		BOOL b_find = FALSE;

		if( p_protocol_->complete_req_item[n] == dw_item_type )
		{
			if( b_add )	n16_item_count_[n] += n_num;
			else n16_item_count_[n] -= n_num;

			if(n16_item_count_[n] > p_protocol_->complete_req_item_num[n])
				n16_item_count_[n] = p_protocol_->complete_req_item_num[n];
			else if(n16_item_count_[n] < 0)
				n16_item_count_[n] = 0;

			b_find = TRUE;
		}

		if( b_find )
		{
			// 发送更新
			NET_SIS_quest_update_item send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nItemIndex = n;
			send.n16NewItemNum = n16_item_count_[n];
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// NPC对话事件
VOID quest::npc_talk_event(DWORD dw_npc_id, DWORD dw_npc_type)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// 动态NPC对话统计
	if(VALID_POINT(p_dynamic_target_) && EQTT_NPCTalk == p_dynamic_target_->eTargetType)
	{
		for(INT i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			BOOL b_find = FALSE;

			if( p_dynamic_target_->dwTargetID[i] == dw_npc_type )
			{
				if( !b_talk_npc_[i] )
				{
					b_talk_npc_[i] = true;
					b_find = TRUE;
				}
			}
			
			if( b_find )
			{
				// 发送更新
				NET_SIS_quest_update_npc_talk send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nIndex = i;
				send.dw_error_code = E_Success;
				p_role_->SendMessage(&send, send.dw_size);

				// 调用NPC对话脚本
				if( VALID_POINT(p_script_) )
					p_script_->on_npc_talk(p_protocol_->id, p_role_, dw_npc_id, dw_npc_type);

				break;
			}
		}

		return;
	}

	// 如果该任务没有获得物品，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_NPC_TALK) ) return;

	// 依次找对应的物品
	for(INT n = 0; n < QUEST_NPC_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_npc[n]) )break;

		BOOL b_find = FALSE;
		if( p_protocol_->complete_req_npc[n] == dw_npc_type )
		{
			if( !b_talk_npc_[n] )
			{
				b_talk_npc_[n] = true;
				b_find = TRUE;
			}
		}
		else
		{
			if( p_protocol_->only_in_order )
			{
				if( false == b_talk_npc_[n] )
				{
					NET_SIS_quest_update_npc_talk send;
					send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
					send.nIndex = n;
					send.dw_error_code = E_CanUpdateQuest_Fail;
					p_role_->SendMessage(&send, send.dw_size);

					break;
				}
			}
		}

		if( b_find )
		{
			// 发送更新
			NET_SIS_quest_update_npc_talk send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nIndex = n;
			send.dw_error_code = E_Success;
			p_role_->SendMessage(&send, send.dw_size);

			// 调用NPC对话脚本
			if( VALID_POINT(p_script_) )
				p_script_->on_npc_talk(p_protocol_->id, p_role_, dw_npc_id, dw_npc_type);

			break;
		}
	}

	// 没有找到，也返回客户端
	NET_SIS_quest_update_npc_talk send;
	send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
	send.nIndex = INVALID_VALUE;
	send.dw_error_code = E_CanUpdateQuest_Fail;
	p_role_->SendMessage(&send, send.dw_size);
}


// 触发器事件
VOID quest::trigger_event(DWORD dw_trigger_id)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// 如果该任务没有获得物品，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_Trigger) ) return;

	// 依次找对应的物品
	for(INT n = 0; n < QUEST_TRIGGERS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_trigger[n]) ) break;

		BOOL b_find = FALSE;

		if( p_protocol_->complete_req_trigger[n] == dw_trigger_id )
		{
			b_trigger_[n] = true;
			b_find = TRUE;
		}

		if( b_find )
		{
			// 发送更新
			NET_SIS_quest_update_trigger send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nIndex = n;
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// 调查地物事件
VOID quest::invest_event(DWORD dw_npc_id, DWORD dw_npc_type)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// 动态NPC对话统计
	if(VALID_POINT(p_dynamic_target_) && EQTT_Invest == p_dynamic_target_->eTargetType)
	{
		for(INT i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			BOOL b_find = FALSE;

			if( p_dynamic_target_->dwTargetID[i] == dw_npc_type )
			{
				if( !b_invest_[i] )
				{
					b_invest_[i] = true;
					b_find = TRUE;
				}
			}

			if( b_find )
			{
				// 发送更新
				NET_SIS_quest_update_inveset send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nIndex = i;
				p_role_->SendMessage(&send, send.dw_size);

				if(VALID_POINT(p_script_))
					p_script_->on_invest(p_protocol_->id, p_role_, dw_npc_type);

				// 找到了，就不需要往下找了
				break;
			}
		}

		return;
	}

	// 如果该任务没有获得物品，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_Invest) ) return;

	// 依次找对应的物品
	for(INT n = 0; n < DYNAMIC_TARGET_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->investigate_objects[n]) && 
			!VALID_POINT(p_protocol_->investigate_objects[n+4]) )
			break;

		BOOL b_find = FALSE;

		if( p_protocol_->investigate_objects[n] == dw_npc_type || 
			p_protocol_->investigate_objects[n+4] == dw_npc_type)
		{
			b_invest_[n] = true;
			b_find = TRUE;
		}

		if( b_find )
		{
			// 发送更新
			NET_SIS_quest_update_inveset send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nIndex = n;
			p_role_->SendMessage(&send, send.dw_size);

			if(VALID_POINT(p_script_))
				p_script_->on_invest(p_protocol_->id, p_role_, dw_npc_type);

			break;
		}
	}
}


// 服务器可控对话框缺省事件
VOID quest::default_dialog_event(DWORD dw_option)
{
	if(!VALID_POINT(p_script_))		return;

	p_script_->on_default_dialog(p_protocol_->id, p_role_, dw_option);
}


// 检测任务物品是否已经满了
BOOL quest::is_item_full(DWORD dw_item_type)
{
	if( FALSE == valid() ) return FALSE;

	// 如果该任务没有获得物品，则直接返回
	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) ) return FALSE;

	// 检测任务物品是否已满
	for(INT n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;
		if( p_protocol_->complete_req_item[n] != dw_item_type ) continue;
		return n16_item_count_[n] >= p_protocol_->complete_req_item_num[n];
	}

	return FALSE;
}


// 完成条件检查
BOOL quest::complete_check(INT n_choice_index, Creature* p_npc, BOOL bIgnorReward, INT* pError)
{
	BOOL b_complete = FALSE;
	if( p_protocol_->only_one_condition )
	{
		b_complete = complete_check_kill_creature(TRUE)	||
			complete_check_item(TRUE) ||
			complete_check_npc_talk(TRUE) ||
			complete_check_trigger(TRUE) ||
			complete_check_money(TRUE) ||
			complete_check_level(TRUE) ||
			complete_check_map(TRUE);
	}
	else
	{
		b_complete = complete_check_kill_creature()	&&
			complete_check_item() &&
			complete_check_npc_talk() &&
			complete_check_trigger() &&
			complete_check_money() &&
			complete_check_level() &&
			complete_check_map();
	}

	b_complete = (b_complete && complete_check_extra_condition());

	if( b_complete && VALID_POINT(p_script_var_) )
		b_complete = p_script_var_->IsComplete( );
	else if(!b_complete && VALID_POINT(pError)) 
		*pError = E_CanCompleteQuest_NotCompleteAll;

	if( VALID_POINT(p_script_) && b_complete )
	{
		INT tError = 0;
		b_complete = p_script_->check_complete(p_protocol_->id, p_role_, p_npc, tError);
		if(!b_complete)
		{
			if(VALID_POINT(pError)) *pError = tError;
			return b_complete;
		}
	}

	if(bIgnorReward) return b_complete;

	return b_complete && complete_check_reward(n_choice_index);
}

// 悬赏任务专用
BOOL quest::complete_check_ex(INT n_choice_index, Creature* p_npc,  INT* pError)
{
	BOOL b_complete = TRUE;

	if( !p_protocol_->only_one_condition )
		b_complete = complete_check_map(TRUE) ;

	if( VALID_POINT(p_script_) && b_complete )
	{
		INT tError = 0;
		b_complete = p_script_->check_complete(p_protocol_->id, p_role_, p_npc, tError);
		if(!b_complete)
		{
			if(VALID_POINT(pError)) *pError = tError;
			return b_complete;
		}
	}

	return b_complete && complete_check_reward(n_choice_index);
}


// 杀怪检查
BOOL quest::complete_check_kill_creature(BOOL b_one_condition)
{
	// 判断动态击杀目标
	if (VALID_POINT(p_dynamic_target_) && EQTT_Kill == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			if(p_dynamic_target_->nTargetNum[i] == 0) continue;
			if(n16_creature_count_[i] < p_dynamic_target_->nTargetNum[i]) return FALSE;
		}

		return TRUE;
	}

	if( !(p_protocol_->quest_flags & Quest_Flag_KILL) )
		return (b_one_condition ? FALSE : TRUE);

	// 满足一个就可以
	if( p_protocol_->only_one_creature )
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			if( n16_creature_count_[n] >= p_protocol_->complete_req_creature_num[n] ) return TRUE;
		}

		return FALSE;
	}
	// 全部
	else
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			if( n16_creature_count_[n] < p_protocol_->complete_req_creature_num[n] ) return FALSE;
		}

		return TRUE;
	}
}


// 物品检查
BOOL quest::complete_check_item(BOOL b_one_condition)
{
	// 判断动态收集的物品
	if (VALID_POINT(p_dynamic_target_) && EQTT_Collect == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			if(p_dynamic_target_->nTargetNum[i] == 0) continue;
			if(n16_item_count_[i] < p_dynamic_target_->nTargetNum[i]) return FALSE;
		}

		return TRUE;
	}

	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) )
		return (b_one_condition ? FALSE : TRUE);

	// 满足一个就可以
	if( p_protocol_->only_one_item )
	{
		for(INT n = 0; n < QUEST_ITEMS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;
			if( n16_item_count_[n] >= p_protocol_->complete_req_item_num[n] ) return TRUE;
		}

		return FALSE;
	}
	// 全部
	else
	{
		for(INT n = 0; n < QUEST_ITEMS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;
			if( n16_item_count_[n] < p_protocol_->complete_req_item_num[n] ) return FALSE;
		}

		return TRUE;
	}
}


// NPC谈话检查
BOOL quest::complete_check_npc_talk(BOOL b_one_condition)
{
	if (VALID_POINT(p_dynamic_target_) && EQTT_NPCTalk == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			if(p_dynamic_target_->dwTargetID[i] == 0) continue;
			if( !b_talk_npc_[i]) return FALSE;
		}

		return TRUE;
	}

	if( !(p_protocol_->quest_flags & Quest_Flag_NPC_TALK) )
		return (b_one_condition ? FALSE : TRUE);

	// 满足一个就可以
	if( p_protocol_->only_one_npc )
	{
		for(INT n = 0; n < QUEST_NPC_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_npc[n]) ) break;
			if( b_talk_npc_[n] ) return TRUE;
		}
		return FALSE;
	}
	// 全部
	else
	{
		for(INT n = 0; n < QUEST_NPC_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_npc[n]) ) break;
			if( !b_talk_npc_[n] ) return FALSE;	
		}
		return TRUE;
	}
}


// 触发器检查
BOOL quest::complete_check_trigger(BOOL b_one_condition)
{
	if( !(p_protocol_->quest_flags & Quest_Flag_Trigger) )
		return (b_one_condition ? FALSE : TRUE);

	// 满足一个就可以
	if( p_protocol_->only_one_trigger )
	{
		for(INT n = 0; n < QUEST_TRIGGERS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_trigger[n]) ) break;
			if( b_trigger_[n] ) return TRUE;
		}
		return FALSE;
	}
	// 全部
	else
	{
		for(INT n = 0; n < QUEST_TRIGGERS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_trigger[n]) ) break;
			if( !b_trigger_[n] ) return FALSE;
		}
		return TRUE;
	}
}


// 调查地物检查
BOOL quest::complete_check_invest(BOOL b_one_condition)
{
	if (VALID_POINT(p_dynamic_target_) && EQTT_Invest == p_dynamic_target_->eTargetType)
	{
		for(int i = 0; i < DYNAMIC_TARGET_COUNT; ++i)
		{
			if( !VALID_POINT(p_dynamic_target_->dwTargetID[i]) ) continue;
			if( !b_invest_[i]) return FALSE;
		}

		return TRUE;
	}

	if( !(p_protocol_->quest_flags & Quest_Flag_Invest) )
		return (b_one_condition ? FALSE : TRUE);

	for(INT n = 0; n < DYNAMIC_TARGET_COUNT; n++)
	{
		if( !VALID_POINT(p_protocol_->investigate_objects[n]) && 
			!VALID_POINT(p_protocol_->investigate_objects[n+4]) )
			break;

		if( !b_invest_[n] ) return FALSE;	
	}
	return TRUE;
}


// 金钱检查
BOOL quest::complete_check_money(BOOL b_one_condition)
{
	BOOL bRet;
	if( 0 == p_protocol_->complete_req_money )
		bRet =  (b_one_condition ? FALSE : TRUE);
	else if( p_role_->GetCurMgr().GetBagSilver() < p_protocol_->complete_req_money )
		bRet = FALSE;

	if( 0 == p_protocol_->complete_need_yuanbao)
		bRet =  (b_one_condition ? FALSE : TRUE);
	else if(p_role_->GetCurMgr().GetBaiBaoYuanBao() < p_protocol_->complete_need_yuanbao)
		bRet = FALSE;
	

	return bRet;
}


// 等级检查
BOOL quest::complete_check_level(BOOL b_one_condition)
{
	if( 0 == p_protocol_->complete_req_level )
		return (b_one_condition ? FALSE : TRUE);

	if( p_role_->get_level() < p_protocol_->complete_req_level )
		return FALSE;

	return TRUE;
}


// 地图检查
BOOL quest::complete_check_map(BOOL b_one_condition)
{
	// todo:
	return TRUE;
}


// 奖励检查
BOOL quest::complete_check_reward(INT n_choice_index)
{
	INT16 n16_bag_space = 0;
	INT16 n16_quest_bag_space = 0;
	INT16 n16_bag_space_temp = 0;
	INT16 n16_quest_bag_space_temp = 0;
	INT16 n16_max_lapp;
	EClassType e_class = p_role_->GetClass( );
	BYTE roleSex = p_role_->GetSex();//获取玩家性别 gx add

	// 包裹空间是否够用
	if( VALID_POINT(p_dynamic_reward_ ) )
	{
		for(INT n = 0; n < QUEST_REW_ITEM; ++n)
		{
			if( !VALID_POINT(p_dynamic_reward_->rew_item[n]) ) break;

			// 得到这个物品会在背包里占用的个数
			if(!ItemMgr::CalSpaceUsed(p_dynamic_reward_->rew_item[n], 
									  p_dynamic_reward_->rew_item_num[n], 
									  n16_bag_space_temp, n16_quest_bag_space_temp, n16_max_lapp))
			{
				return FALSE;
			}

			n16_bag_space		+= n16_bag_space_temp;
			n16_quest_bag_space	+= n16_quest_bag_space_temp;
		}
	}

	for(INT n = 0; n < QUEST_REW_ITEM * X_ClASS_TYPE_NUM; ++n)
	{
		if( !VALID_POINT(p_protocol_->rew_item[n]) ) break;
		if( p_protocol_->rew_item_classtype[n] != 0 && e_class != p_protocol_->rew_item_classtype[n]) continue;
		if(QUESTREWARDNOSEX != p_protocol_->rew_item_rolesex[n] && roleSex != p_protocol_->rew_item_rolesex[n]) continue;
		// 得到这个物品会在背包里占用的个数
		if(!ItemMgr::CalSpaceUsed(p_protocol_->rew_item[n], 
								  p_protocol_->rew_item_num[n], 
								  n16_bag_space_temp, 
								  n16_quest_bag_space_temp, 
								  n16_max_lapp)
		  )
		{
			return FALSE;
		}

		n16_bag_space		+= n16_bag_space_temp;
		n16_quest_bag_space	+= n16_quest_bag_space_temp;
	}

	// 如果有可选奖励，则查看奖励物品中的最大个数
	if(n_choice_index >= 0 && n_choice_index < QUEST_REW_ITEM )
	{
		DWORD dwItemID = 0, dwItemNum = 0;

		if(VALID_POINT(p_dynamic_reward_ ))
		{//动态任务可选奖励
			if( p_dynamic_reward_->rew_choice_item[n_choice_index])
			{
				dwItemID =  p_dynamic_reward_->rew_choice_item[n_choice_index];
				dwItemNum = p_dynamic_reward_->rew_choice_item_num[n_choice_index];
			}
		}
		else if( VALID_POINT(p_protocol_) )
		{//不是动态任务
			INT16 n16Index = p_protocol_->GetChoiceIndex( p_role_->GetClass( ), n_choice_index );
			if( n16Index >=0 && VALID_POINT(p_protocol_->rew_choice_item[n_choice_index]) )
			{
				dwItemID = p_protocol_->rew_choice_item[n16Index ];
				dwItemNum = p_protocol_->rew_choice_item_num[n16Index ];
			}
		}

		if( VALID_POINT(dwItemNum) && VALID_POINT(dwItemID ) )
		{//得到这个物品会在背包里占用的个数	
			if(!ItemMgr::CalSpaceUsed(dwItemID, dwItemNum, 
									  n16_bag_space_temp, 
									  n16_quest_bag_space_temp, 
									  n16_max_lapp ) 
			  )
				return FALSE;

			n16_bag_space		+= n16_bag_space_temp;
			n16_quest_bag_space	+= n16_quest_bag_space_temp;
		}
	}

	// 得到背包里空闲的栏位能否加上这些物品
	if( p_role_->GetItemMgr().GetBagFreeSize() < n16_bag_space || 
		p_role_->GetItemMgr().GetQuestItemBagFreeSize() < n16_quest_bag_space
	  )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL quest::complete_check_extra_condition()
{
	if(!VALID_POINT(p_role_)) return FALSE;

	if(p_protocol_->del_vigour && VALID_POINT(p_protocol_->vigour_val))
	{
		if(p_role_->GetAttValue(ERA_Fortune)< p_protocol_->vigour_val)
			return FALSE;
	}

	if(p_protocol_->del_banggong && VALID_POINT(p_protocol_->banggong_val))
	{
		guild* pGuild = g_guild_manager.get_guild(p_role_->GetGuildID());
		if(!VALID_POINT(pGuild)) return FALSE;

		tagGuildMember* pMember = pGuild->get_member(p_role_->GetID());
		if(!VALID_POINT(pMember)) return FALSE;

		if(pMember->nContribution < p_protocol_->banggong_val)
			return FALSE;
	}

	return TRUE;
}

// 将杀怪数自动填满
VOID quest::fill_kill_creature_auto(OUT INT16* pArray, INT ArraySize)
{
	if(VALID_POINT(pArray))
	{// 返回
		ASSERT( ArraySize == QUEST_CREATURES_COUNT);
		memcpy(pArray, n16_creature_count_, 
			sizeof(INT16) * min(ArraySize,QUEST_CREATURES_COUNT));
		return;
	}

	// 满足一个就可以
	if( p_protocol_->only_one_creature )
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			n16_creature_count_[n] = p_protocol_->complete_req_creature_num[n];
			break;
		}
	}
	// 全部
	else
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			n16_creature_count_[n] = p_protocol_->complete_req_creature_num[n] ;
		}
	}
}
// 销毁
VOID quest::destroy()
{
	p_role_ = NULL;
	p_protocol_ = NULL;
	dw_accept_time_ = INVALID_VALUE;
	n_index_ = INVALID_VALUE;

	b_complete_ = FALSE;
	p_script_ = NULL;

	SAFE_DELETE(p_dynamic_reward_);
	SAFE_DELETE(p_dynamic_target_);
	SAFE_DELETE(p_script_var_);
}
