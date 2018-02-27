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

#ifndef __QUEST_H__

#define __QUEST_H__

#include "../../common/WorldDefine/script_data.h"

struct tagQuestProto;
class Role;
class quest_script;

class quest:public ScriptData<ESD_Quest>
{
public:
	quest();
	~quest();
public:
	// ��ʼ������
	VOID init(const tagQuestProto* p_proto, Role* p_role, INT n_index);
	VOID init(const tagQuestSave* p_save, Role* p_role, INT n_index);
private:
	// ��ʼ������������Ʒ
	VOID init_need_item();

public:
	// ������Ϣ�ṹ
	VOID make_msg_info(OUT tagIncompleteQuestMsgInfo* p_info, OUT DWORD &dw_msg_size, OUT DWORD &dw_quest_size);

	// ��������ʼ��Ʒ
	VOID fill_quest_init_item(OUT INT16* const n16_item_count);

	// �������̬Ŀ��
	VOID fill_quest_dynamic_target(OUT tagQuestDynamicTarget* p_target, OUT DWORD &dw_size);

	// ��䶯̬����
	VOID fill_quest_dynmaic_reward(OUT tagQuestDynamicReward* p_reward, OUT DWORD &dw_size);

	// ���ű����ñ���
	VOID fill_quest_script_var(OUT tagScriptQuestVar* p_var, OUT  DWORD& dw_size);

	// ������񱣴�����
	VOID fill_quest_save(OUT tagQuestSave* p_save);

	// ��ɱ�����Զ�����
	VOID fill_kill_creature_auto(OUT INT16* pArray = NULL, INT ArraySize = 0);
public:
	// ��ȡ����ʱ
	VOID on_accept(Creature* p_npc);

	// �������ʱ
	VOID on_complete(Creature* p_npc);

	// ɾ������ʱ
	VOID on_remove();

	// �����¼�
	VOID on_event(EQuestEvent e_event, DWORD dw_parm1=0, DWORD dw_parm2=0, DWORD dw_parm3=0, DWORD dw_parm4 = 0);

	// ɱ���¼�
	VOID creature_kill_event(DWORD dw_creature_type, INT n_creature_level, BOOL bAddExp, INT n_kill_num = 1);

	// ��Ʒ�¼�
	VOID item_event(DWORD dw_item_type, INT n_num, BOOL b_add);

	// NPC�Ի��¼�
	VOID npc_talk_event(DWORD dw_npc_id, DWORD dw_npc_type);

	// �������¼�
	VOID trigger_event(DWORD dw_trigger_id);

	// ��������¼�
	VOID invest_event(DWORD dw_npc_id, DWORD dw_npc_type);

	// �������ɿضԻ���ȱʡ�¼�
	VOID default_dialog_event(DWORD dw_option);

public:
	// ���������Ʒ�Ƿ��Ѿ�����
	BOOL is_item_full(DWORD dw_item_type);

	// ����������
	BOOL complete_check(INT n_choice_index, Creature* p_npc=NULL, BOOL bIgnorReward = FALSE, INT* pError = NULL);

	// ��������ר��
	BOOL complete_check_ex(INT n_choice_index, Creature* p_npc = NULL,  INT* pError = NULL );
private:
	// ɱ�ּ��
	BOOL complete_check_kill_creature(BOOL b_one_condition=FALSE);

	// ��Ʒ���
	BOOL complete_check_item(BOOL b_one_condition=FALSE);

	// NPC̸�����
	BOOL complete_check_npc_talk(BOOL b_one_condition=FALSE);

	// ���������
	BOOL complete_check_trigger(BOOL b_one_condition=FALSE);

	// ���������
	BOOL complete_check_invest(BOOL b_one_condition=FALSE);

	// ��Ǯ���
	BOOL complete_check_money(BOOL b_one_condition=FALSE);

	// �ȼ����
	BOOL complete_check_level(BOOL b_one_condition=FALSE);

	// ��ͼ���
	BOOL complete_check_map(BOOL b_one_condition=FALSE);

	// �������
	BOOL complete_check_reward(INT n_choice_index);

	// ��������������
	BOOL complete_check_extra_condition( );

public:
	// ����
	VOID destroy();

public:
	// ���������Ƿ����
	BOOL valid() { return VALID_POINT(p_protocol_); }

	// �������������±�
	INT	get_index() { return n_index_; }

	// ����ID
	UINT16 get_id() { return VALID_POINT(p_protocol_) ? p_protocol_->id : 0; }

	// ��ȡʱ��
	DWORD get_accept_time()	{ return dw_accept_time_; }

	// ����̬����
	const tagQuestProto* get_protocol() { return p_protocol_; }

	// ����̬����
	tagScriptQuestVar* get_script_var()	{ return p_script_var_; }

	// ����̬Ŀ��
	tagQuestDynamicTarget* get_dynamic_target() { return p_dynamic_target_; }

	// ����̬����
	tagQuestDynamicReward* get_dynamic_reward() { return p_dynamic_reward_; }

	tagDWORDQuestFlag& get_quest_flag() { return dw_quest_flag_; }

public:
	// ��������̬����
	VOID set_protocol(const tagQuestProto* p_proto) { p_protocol_ = p_proto; }

	// �����������
	VOID set_role(Role* p_role) { p_role_ = p_role; }

	// ����������ɱ�־λ
	VOID set_complete_flag(BOOL b_complete) { b_complete_ = b_complete; }

	VOID set_quest_track(BOOL b){
		dw_quest_flag_.dwQuestTrack = b ? 1 : 0;
	}
private:

	// �������������±�
	INT	n_index_;			

	// �����ȡʱ��
	DWORD dw_accept_time_;		

	// 
	tagDWORDQuestFlag dw_quest_flag_;

	// ɱ����ͳ��
	INT16 n16_creature_count_[QUEST_CREATURES_COUNT];	

	// �ռ���Ʒͳ��
	INT16 n16_item_count_[QUEST_ITEMS_COUNT];			

	// �Ƿ��Ѿ����
	BOOL b_complete_;

	// ̸��NPC��¼
	bool b_talk_npc_[QUEST_NPC_COUNT];

	// ��������¼
	bool b_trigger_[QUEST_TRIGGERS_COUNT];

	// ��������¼
	bool b_invest_[DYNAMIC_TARGET_COUNT];

	// ��ɫ
	Role* p_role_;

	// �ű����ñ���
	tagScriptQuestVar* p_script_var_;	

	// ����̬Ŀ��
	tagQuestDynamicTarget* p_dynamic_target_;	

	// ����̬����
	tagQuestDynamicReward* p_dynamic_reward_;							

	// ����ű�
	const quest_script* p_script_;	

	// ��̬����
	const tagQuestProto* p_protocol_;									
};

#endif //__QUEST_H__