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
	// 初始化任务
	VOID init(const tagQuestProto* p_proto, Role* p_role, INT n_index);
	VOID init(const tagQuestSave* p_save, Role* p_role, INT n_index);
private:
	// 初始化任务所需物品
	VOID init_need_item();

public:
	// 生成消息结构
	VOID make_msg_info(OUT tagIncompleteQuestMsgInfo* p_info, OUT DWORD &dw_msg_size, OUT DWORD &dw_quest_size);

	// 填充任务初始物品
	VOID fill_quest_init_item(OUT INT16* const n16_item_count);

	// 填充任务动态目标
	VOID fill_quest_dynamic_target(OUT tagQuestDynamicTarget* p_target, OUT DWORD &dw_size);

	// 填充动态奖励
	VOID fill_quest_dynmaic_reward(OUT tagQuestDynamicReward* p_reward, OUT DWORD &dw_size);

	// 填充脚本设置变量
	VOID fill_quest_script_var(OUT tagScriptQuestVar* p_var, OUT  DWORD& dw_size);

	// 填充任务保存数据
	VOID fill_quest_save(OUT tagQuestSave* p_save);

	// 将杀怪数自动填满
	VOID fill_kill_creature_auto(OUT INT16* pArray = NULL, INT ArraySize = 0);
public:
	// 接取任务时
	VOID on_accept(Creature* p_npc);

	// 完成任务时
	VOID on_complete(Creature* p_npc);

	// 删除任务时
	VOID on_remove();

	// 触发事件
	VOID on_event(EQuestEvent e_event, DWORD dw_parm1=0, DWORD dw_parm2=0, DWORD dw_parm3=0, DWORD dw_parm4 = 0);

	// 杀怪事件
	VOID creature_kill_event(DWORD dw_creature_type, INT n_creature_level, BOOL bAddExp, INT n_kill_num = 1);

	// 物品事件
	VOID item_event(DWORD dw_item_type, INT n_num, BOOL b_add);

	// NPC对话事件
	VOID npc_talk_event(DWORD dw_npc_id, DWORD dw_npc_type);

	// 触发器事件
	VOID trigger_event(DWORD dw_trigger_id);

	// 调查地物事件
	VOID invest_event(DWORD dw_npc_id, DWORD dw_npc_type);

	// 服务器可控对话框缺省事件
	VOID default_dialog_event(DWORD dw_option);

public:
	// 检测任务物品是否已经满了
	BOOL is_item_full(DWORD dw_item_type);

	// 完成条件检查
	BOOL complete_check(INT n_choice_index, Creature* p_npc=NULL, BOOL bIgnorReward = FALSE, INT* pError = NULL);

	// 悬赏任务专用
	BOOL complete_check_ex(INT n_choice_index, Creature* p_npc = NULL,  INT* pError = NULL );
private:
	// 杀怪检查
	BOOL complete_check_kill_creature(BOOL b_one_condition=FALSE);

	// 物品检查
	BOOL complete_check_item(BOOL b_one_condition=FALSE);

	// NPC谈话检查
	BOOL complete_check_npc_talk(BOOL b_one_condition=FALSE);

	// 触发器检查
	BOOL complete_check_trigger(BOOL b_one_condition=FALSE);

	// 调查地物检查
	BOOL complete_check_invest(BOOL b_one_condition=FALSE);

	// 金钱检查
	BOOL complete_check_money(BOOL b_one_condition=FALSE);

	// 等级检查
	BOOL complete_check_level(BOOL b_one_condition=FALSE);

	// 地图检查
	BOOL complete_check_map(BOOL b_one_condition=FALSE);

	// 奖励检查
	BOOL complete_check_reward(INT n_choice_index);

	// 检查其他完成条件
	BOOL complete_check_extra_condition( );

public:
	// 销毁
	VOID destroy();

public:
	// 任务数据是否存在
	BOOL valid() { return VALID_POINT(p_protocol_); }

	// 任务在数组中下标
	INT	get_index() { return n_index_; }

	// 任务ID
	UINT16 get_id() { return VALID_POINT(p_protocol_) ? p_protocol_->id : 0; }

	// 接取时间
	DWORD get_accept_time()	{ return dw_accept_time_; }

	// 任务静态数据
	const tagQuestProto* get_protocol() { return p_protocol_; }

	// 任务动态变量
	tagScriptQuestVar* get_script_var()	{ return p_script_var_; }

	// 任务动态目标
	tagQuestDynamicTarget* get_dynamic_target() { return p_dynamic_target_; }

	// 任务动态奖励
	tagQuestDynamicReward* get_dynamic_reward() { return p_dynamic_reward_; }

	tagDWORDQuestFlag& get_quest_flag() { return dw_quest_flag_; }

public:
	// 设置任务静态数据
	VOID set_protocol(const tagQuestProto* p_proto) { p_protocol_ = p_proto; }

	// 设置任务归属
	VOID set_role(Role* p_role) { p_role_ = p_role; }

	// 设置任务完成标志位
	VOID set_complete_flag(BOOL b_complete) { b_complete_ = b_complete; }

	VOID set_quest_track(BOOL b){
		dw_quest_flag_.dwQuestTrack = b ? 1 : 0;
	}
private:

	// 任务在数组中下标
	INT	n_index_;			

	// 任务接取时间
	DWORD dw_accept_time_;		

	// 
	tagDWORDQuestFlag dw_quest_flag_;

	// 杀怪数统计
	INT16 n16_creature_count_[QUEST_CREATURES_COUNT];	

	// 收集物品统计
	INT16 n16_item_count_[QUEST_ITEMS_COUNT];			

	// 是否已经完成
	BOOL b_complete_;

	// 谈话NPC记录
	bool b_talk_npc_[QUEST_NPC_COUNT];

	// 触发器记录
	bool b_trigger_[QUEST_TRIGGERS_COUNT];

	// 调查地物记录
	bool b_invest_[DYNAMIC_TARGET_COUNT];

	// 角色
	Role* p_role_;

	// 脚本设置变量
	tagScriptQuestVar* p_script_var_;	

	// 任务动态目标
	tagQuestDynamicTarget* p_dynamic_target_;	

	// 任务动态奖励
	tagQuestDynamicReward* p_dynamic_reward_;							

	// 任务脚本
	const quest_script* p_script_;	

	// 静态数据
	const tagQuestProto* p_protocol_;									
};

#endif //__QUEST_H__