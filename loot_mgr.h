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
 *	@file		loot_mgr
 *	@author		mwh
 *	@date		2011/01/01	initial
 *	@version	0.0.1.0
 *	@brief		掉落管理
*/


#pragma once

#include "singleton.h"
#include "../../common/WorldDefine/loot.h"
#include "../../common/WorldDefine/group_define.h"

// 最大掉落位置数
const INT MAX_LOOT_POS		= 50;

struct tagLoot;
struct tagCreatureLoot;
struct tagLootItemSet;
struct tagLootQuestItem;
class Creature;
class Role;
class Team;
class Unit;

// 掉落方式
enum EDropTo
{
	// 优先背包，否则地面
	EDT_BAGFIRST = 0,
	
	// 强制背包
	EDT_MUSTBAG = 1,
	
	// 地上
	EDT_GROUND   = 2,
};

class drop_mgr : public GameServer::Singleton<drop_mgr>
{	
public:
	drop_mgr(){};
	~drop_mgr(){};
	
public:  
	// 初始化和销毁
	BOOL init(){return TRUE;};

public:
	// 怪物掉落
	VOID monster_drop(Creature* p_creature, Role* p_picker);

	// 资源掉落
	VOID resource_drop(Creature* p_creature, Role* p_picker);

	// 调查地物掉落
	VOID investigate_drop(Creature* p_creature, Role* p_picker);
	
	// 种植物掉落
	VOID resource_plant_drop(Creature* p_creature, Role* p_picker, int nNum);

	VOID get_plant_drop_list(Creature* p_creature, int nNum, package_map<DWORD ,int>& mapPlantItem);
	
	EItemQuality	get_loot_equip_quality();

	// 获取下一个掉落位置
	INT	get_drop_pos(Unit* p_unit, Vector3 &r_pos, INT &r_index);

private:
	// 普通掉落
	VOID normal_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, BOOL b_singel, BOOL b_team);
	
	// 种植物掉落
	VOID plant_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, int nNum);
	
	// 任务掉落
	VOID quest_drop(Creature* p_creature, Role* p_role, BOOL bInves, BOOL b_team);

	// 掉率判断
	FLOAT creature_drop_prob_delta(Creature* p_creature, Role* p_owner);
	FLOAT moster_drop_prob_delta(Role* p_owner);
	FLOAT resource_drop_prob_delta(Creature* p_creature, Role* p_owner);
	FLOAT investobj_drop_prob_delta(Role* p_owner);

	// 得到某个掉落的所属
	VOID set_drop_owner(Creature* p_creature, Role* p_picker, BOOL b_team, 
						DWORD& dw_owner, Role* &p_owner, DWORD& dw_team_id);

	// 各种掉落方式
	BOOL drop_item(Creature* p_creature, tagItem* &p_item, EDropTo e_drop_to, 
					Role* p_owner, DWORD dw_owner, DWORD dw_team_id, INT& r_index,
					EPickMode pick_mode = EPUM_Free, EGTAssignState assign_state = EGTAS_Null, INT tick = 0);

	// 各种掉落方法
	BOOL drop_item_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, 
					Role* p_picker, BOOL b_team, FLOAT f_prod_delta, INT &r_index);

	BOOL drop_set_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, 
		Role* p_picker, BOOL b_team, FLOAT f_prod_delta, INT &r_index);

	BOOL drop_money_to(const tagCreatureLoot* p_proto, Creature* p_creature, EDropTo e_drop_to, 
		Role* p_picker, FLOAT f_prod_delta, bool bteam, INT &r_index);

	BOOL drop_quest_item(Role* p_picker, const tagItemProto* p_proto, DWORD dw_creature);

	//队伍掉落需要根据分配方式设定相关数据
	VOID team_drop_set( tagItem* p_item, DWORD dw_team_id, EPickMode& pick_mode, 
					EGTAssignState& assign_state, INT& tick );

	VOID SendHearChat(Role* pRole, Creature* pCreature, tagItem* pItem);
};

#define g_drop_mgr drop_mgr::getSingleton()

