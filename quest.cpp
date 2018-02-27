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
 *	@brief		��������
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

// ��ʼ������
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

	// ��ʼ���ű�
	p_script_ = g_ScriptMgr.GetQuestScript(p_protocol_->id);

	// ��ʼ������̬Ŀ��/��̬����/�ű�����
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

	// ��װ������ϵ���Ʒ����
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

	// ��ʼ���ű�
	p_script_ = g_ScriptMgr.GetQuestScript(p_protocol_->id);

	// ��ʼ������̬Ŀ��/��̬����/�ű�����
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

	// ��װ������ϵ���Ʒ����
	init_need_item();
}

// ��ʼ������������Ʒ
VOID quest::init_need_item()
{
	if( !VALID_POINT(p_protocol_) ) return;

	// ��̬�ռ���Ʒ
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

	// ��̬�����ռ���Ʒ
	for(INT n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;

		n16_item_count_[n]  = p_role_->GetItemMgr().GetBagSameItemCount(p_protocol_->complete_req_item[n]);
		n16_item_count_[n] += p_role_->GetItemMgr().GetQuestBagSameItemCount(p_protocol_->complete_req_item[n]);
	}
}


// ������Ϣ�ṹ
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
		// ������Ϣ��С
		INT OffSize = sizeof(tagQuestDynamicTarget)+sizeof(tagQuestDynamicReward)+sizeof(tagScriptQuestVar);
		dw_quest_size -= OffSize;  dw_msg_size -= OffSize;
	}
}

// ��������ʼ��Ʒ
VOID quest::fill_quest_init_item(OUT INT16* const n16_item_count)
{
	ASSERT( valid() );

	get_fast_code()->memory_copy(n16_item_count, n16_item_count_, sizeof(n16_item_count_));
}


// �������̬Ŀ��
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


// ��䶯̬����
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

// ���ű����ñ���
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

// ������񱣴�����
VOID quest::fill_quest_save(OUT tagQuestSave* p_save)
{
	p_save->u16QuestID = QuestIDHelper::GenerateID(get_id(), get_quest_flag().dwTakeFromGuerdon);
	p_save->dwAcceptTime = get_accept_time();
	p_save->dwQuestFlag = get_quest_flag();
	//�ж�ÿ������
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

// ��ȡ����ʱ
VOID quest::on_accept(Creature* p_npc)
{
	if( VALID_POINT(p_script_) )
		p_script_->on_accept(p_protocol_->id, p_role_, p_npc);
}


// �������ʱ
VOID quest::on_complete(Creature* p_npc)
{
	if( VALID_POINT(p_script_) )
		p_script_->on_complete(p_protocol_->id, p_role_, p_npc);
}


// ɾ������ʱ
VOID quest::on_remove()
{
	if( VALID_POINT(p_script_) )
		p_script_->on_cancel(p_protocol_->id, p_role_);
}

// �����¼�
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


// ɱ���¼�
VOID quest::creature_kill_event(DWORD dw_creature_type, INT n_creature_level, BOOL bAddExp, INT n_kill_num)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// ���ýű�
	if( VALID_POINT(p_script_) )
		p_script_->on_creature_kill(p_protocol_->id, p_role_, dw_creature_type, bAddExp);

	// ��̬��ɱĿ�����
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
				// ���͸���
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

	// ���������û��ɱ�֣���ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_KILL) ) return;
	
	for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
	{
		//gx modify 2013.5.10
		//�޸�ԭ����������ɱ�����󣺻�ɱ���ɸ��֣�����IDδָ��
		//�������cc_require_creature_id�ֶξ�Ϊ0����һ��value�ֶθ���Ӧ��ֵ
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
				&& (p_protocol_->complete_req_creature_num[n] > 0)//gx add 2013.5.10 �޸�ԭ��ͬ��
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
			// ���͸���
			NET_SIS_quest_update_kill_creature send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nCreatureIndex = n;
			send.n16NewKillNum = n16_creature_count_[n];
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// ��Ʒ�¼�
VOID quest::item_event(DWORD dw_item_type, INT n_num, BOOL b_add)
{
	if( !VALID_POINT(p_protocol_) ) return;
	if( n_num <= 0 || b_complete_) return;

	// ��̬�ռ���Ʒ����
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
				// ���͸���
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

	// ���������û�л����Ʒ����ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) ) return;

	// �����Ҷ�Ӧ����Ʒ
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
			// ���͸���
			NET_SIS_quest_update_item send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nItemIndex = n;
			send.n16NewItemNum = n16_item_count_[n];
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// NPC�Ի��¼�
VOID quest::npc_talk_event(DWORD dw_npc_id, DWORD dw_npc_type)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// ��̬NPC�Ի�ͳ��
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
				// ���͸���
				NET_SIS_quest_update_npc_talk send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nIndex = i;
				send.dw_error_code = E_Success;
				p_role_->SendMessage(&send, send.dw_size);

				// ����NPC�Ի��ű�
				if( VALID_POINT(p_script_) )
					p_script_->on_npc_talk(p_protocol_->id, p_role_, dw_npc_id, dw_npc_type);

				break;
			}
		}

		return;
	}

	// ���������û�л����Ʒ����ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_NPC_TALK) ) return;

	// �����Ҷ�Ӧ����Ʒ
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
			// ���͸���
			NET_SIS_quest_update_npc_talk send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nIndex = n;
			send.dw_error_code = E_Success;
			p_role_->SendMessage(&send, send.dw_size);

			// ����NPC�Ի��ű�
			if( VALID_POINT(p_script_) )
				p_script_->on_npc_talk(p_protocol_->id, p_role_, dw_npc_id, dw_npc_type);

			break;
		}
	}

	// û���ҵ���Ҳ���ؿͻ���
	NET_SIS_quest_update_npc_talk send;
	send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
	send.nIndex = INVALID_VALUE;
	send.dw_error_code = E_CanUpdateQuest_Fail;
	p_role_->SendMessage(&send, send.dw_size);
}


// �������¼�
VOID quest::trigger_event(DWORD dw_trigger_id)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// ���������û�л����Ʒ����ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_Trigger) ) return;

	// �����Ҷ�Ӧ����Ʒ
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
			// ���͸���
			NET_SIS_quest_update_trigger send;
			send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
			send.nIndex = n;
			p_role_->SendMessage(&send, send.dw_size);

			break;
		}
	}
}


// ��������¼�
VOID quest::invest_event(DWORD dw_npc_id, DWORD dw_npc_type)
{
	if( !VALID_POINT(p_protocol_) ) return;

	// ��̬NPC�Ի�ͳ��
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
				// ���͸���
				NET_SIS_quest_update_inveset send;
				send.u16QuestID = QuestIDHelper::GenerateID(get_id(), dw_quest_flag_.dwTakeFromGuerdon);
				send.nIndex = i;
				p_role_->SendMessage(&send, send.dw_size);

				if(VALID_POINT(p_script_))
					p_script_->on_invest(p_protocol_->id, p_role_, dw_npc_type);

				// �ҵ��ˣ��Ͳ���Ҫ��������
				break;
			}
		}

		return;
	}

	// ���������û�л����Ʒ����ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_Invest) ) return;

	// �����Ҷ�Ӧ����Ʒ
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
			// ���͸���
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


// �������ɿضԻ���ȱʡ�¼�
VOID quest::default_dialog_event(DWORD dw_option)
{
	if(!VALID_POINT(p_script_))		return;

	p_script_->on_default_dialog(p_protocol_->id, p_role_, dw_option);
}


// ���������Ʒ�Ƿ��Ѿ�����
BOOL quest::is_item_full(DWORD dw_item_type)
{
	if( FALSE == valid() ) return FALSE;

	// ���������û�л����Ʒ����ֱ�ӷ���
	if( !(p_protocol_->quest_flags & Quest_Flag_ITEM) ) return FALSE;

	// ���������Ʒ�Ƿ�����
	for(INT n = 0; n < QUEST_ITEMS_COUNT; ++n)
	{
		if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;
		if( p_protocol_->complete_req_item[n] != dw_item_type ) continue;
		return n16_item_count_[n] >= p_protocol_->complete_req_item_num[n];
	}

	return FALSE;
}


// ����������
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

// ��������ר��
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


// ɱ�ּ��
BOOL quest::complete_check_kill_creature(BOOL b_one_condition)
{
	// �ж϶�̬��ɱĿ��
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

	// ����һ���Ϳ���
	if( p_protocol_->only_one_creature )
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			if( n16_creature_count_[n] >= p_protocol_->complete_req_creature_num[n] ) return TRUE;
		}

		return FALSE;
	}
	// ȫ��
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


// ��Ʒ���
BOOL quest::complete_check_item(BOOL b_one_condition)
{
	// �ж϶�̬�ռ�����Ʒ
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

	// ����һ���Ϳ���
	if( p_protocol_->only_one_item )
	{
		for(INT n = 0; n < QUEST_ITEMS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_item[n]) ) break;
			if( n16_item_count_[n] >= p_protocol_->complete_req_item_num[n] ) return TRUE;
		}

		return FALSE;
	}
	// ȫ��
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


// NPC̸�����
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

	// ����һ���Ϳ���
	if( p_protocol_->only_one_npc )
	{
		for(INT n = 0; n < QUEST_NPC_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_npc[n]) ) break;
			if( b_talk_npc_[n] ) return TRUE;
		}
		return FALSE;
	}
	// ȫ��
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


// ���������
BOOL quest::complete_check_trigger(BOOL b_one_condition)
{
	if( !(p_protocol_->quest_flags & Quest_Flag_Trigger) )
		return (b_one_condition ? FALSE : TRUE);

	// ����һ���Ϳ���
	if( p_protocol_->only_one_trigger )
	{
		for(INT n = 0; n < QUEST_TRIGGERS_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_trigger[n]) ) break;
			if( b_trigger_[n] ) return TRUE;
		}
		return FALSE;
	}
	// ȫ��
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


// ���������
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


// ��Ǯ���
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


// �ȼ����
BOOL quest::complete_check_level(BOOL b_one_condition)
{
	if( 0 == p_protocol_->complete_req_level )
		return (b_one_condition ? FALSE : TRUE);

	if( p_role_->get_level() < p_protocol_->complete_req_level )
		return FALSE;

	return TRUE;
}


// ��ͼ���
BOOL quest::complete_check_map(BOOL b_one_condition)
{
	// todo:
	return TRUE;
}


// �������
BOOL quest::complete_check_reward(INT n_choice_index)
{
	INT16 n16_bag_space = 0;
	INT16 n16_quest_bag_space = 0;
	INT16 n16_bag_space_temp = 0;
	INT16 n16_quest_bag_space_temp = 0;
	INT16 n16_max_lapp;
	EClassType e_class = p_role_->GetClass( );
	BYTE roleSex = p_role_->GetSex();//��ȡ����Ա� gx add

	// �����ռ��Ƿ���
	if( VALID_POINT(p_dynamic_reward_ ) )
	{
		for(INT n = 0; n < QUEST_REW_ITEM; ++n)
		{
			if( !VALID_POINT(p_dynamic_reward_->rew_item[n]) ) break;

			// �õ������Ʒ���ڱ�����ռ�õĸ���
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
		// �õ������Ʒ���ڱ�����ռ�õĸ���
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

	// ����п�ѡ��������鿴������Ʒ�е�������
	if(n_choice_index >= 0 && n_choice_index < QUEST_REW_ITEM )
	{
		DWORD dwItemID = 0, dwItemNum = 0;

		if(VALID_POINT(p_dynamic_reward_ ))
		{//��̬�����ѡ����
			if( p_dynamic_reward_->rew_choice_item[n_choice_index])
			{
				dwItemID =  p_dynamic_reward_->rew_choice_item[n_choice_index];
				dwItemNum = p_dynamic_reward_->rew_choice_item_num[n_choice_index];
			}
		}
		else if( VALID_POINT(p_protocol_) )
		{//���Ƕ�̬����
			INT16 n16Index = p_protocol_->GetChoiceIndex( p_role_->GetClass( ), n_choice_index );
			if( n16Index >=0 && VALID_POINT(p_protocol_->rew_choice_item[n_choice_index]) )
			{
				dwItemID = p_protocol_->rew_choice_item[n16Index ];
				dwItemNum = p_protocol_->rew_choice_item_num[n16Index ];
			}
		}

		if( VALID_POINT(dwItemNum) && VALID_POINT(dwItemID ) )
		{//�õ������Ʒ���ڱ�����ռ�õĸ���	
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

	// �õ���������е���λ�ܷ������Щ��Ʒ
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

// ��ɱ�����Զ�����
VOID quest::fill_kill_creature_auto(OUT INT16* pArray, INT ArraySize)
{
	if(VALID_POINT(pArray))
	{// ����
		ASSERT( ArraySize == QUEST_CREATURES_COUNT);
		memcpy(pArray, n16_creature_count_, 
			sizeof(INT16) * min(ArraySize,QUEST_CREATURES_COUNT));
		return;
	}

	// ����һ���Ϳ���
	if( p_protocol_->only_one_creature )
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			n16_creature_count_[n] = p_protocol_->complete_req_creature_num[n];
			break;
		}
	}
	// ȫ��
	else
	{
		for(INT n = 0; n < QUEST_CREATURES_COUNT; n++)
		{
			if( !VALID_POINT(p_protocol_->complete_req_creature[n*3]) ) break;
			n16_creature_count_[n] = p_protocol_->complete_req_creature_num[n] ;
		}
	}
}
// ����
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
