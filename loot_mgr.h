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
 *	@brief		�������
*/


#pragma once

#include "singleton.h"
#include "../../common/WorldDefine/loot.h"
#include "../../common/WorldDefine/group_define.h"

// ������λ����
const INT MAX_LOOT_POS		= 50;

struct tagLoot;
struct tagCreatureLoot;
struct tagLootItemSet;
struct tagLootQuestItem;
class Creature;
class Role;
class Team;
class Unit;

// ���䷽ʽ
enum EDropTo
{
	// ���ȱ������������
	EDT_BAGFIRST = 0,
	
	// ǿ�Ʊ���
	EDT_MUSTBAG = 1,
	
	// ����
	EDT_GROUND   = 2,
};

class drop_mgr : public GameServer::Singleton<drop_mgr>
{	
public:
	drop_mgr(){};
	~drop_mgr(){};
	
public:  
	// ��ʼ��������
	BOOL init(){return TRUE;};

public:
	// �������
	VOID monster_drop(Creature* p_creature, Role* p_picker);

	// ��Դ����
	VOID resource_drop(Creature* p_creature, Role* p_picker);

	// ����������
	VOID investigate_drop(Creature* p_creature, Role* p_picker);
	
	// ��ֲ�����
	VOID resource_plant_drop(Creature* p_creature, Role* p_picker, int nNum);

	VOID get_plant_drop_list(Creature* p_creature, int nNum, package_map<DWORD ,int>& mapPlantItem);
	
	EItemQuality	get_loot_equip_quality();

	// ��ȡ��һ������λ��
	INT	get_drop_pos(Unit* p_unit, Vector3 &r_pos, INT &r_index);

private:
	// ��ͨ����
	VOID normal_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, BOOL b_singel, BOOL b_team);
	
	// ��ֲ�����
	VOID plant_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, int nNum);
	
	// �������
	VOID quest_drop(Creature* p_creature, Role* p_role, BOOL bInves, BOOL b_team);

	// �����ж�
	FLOAT creature_drop_prob_delta(Creature* p_creature, Role* p_owner);
	FLOAT moster_drop_prob_delta(Role* p_owner);
	FLOAT resource_drop_prob_delta(Creature* p_creature, Role* p_owner);
	FLOAT investobj_drop_prob_delta(Role* p_owner);

	// �õ�ĳ�����������
	VOID set_drop_owner(Creature* p_creature, Role* p_picker, BOOL b_team, 
						DWORD& dw_owner, Role* &p_owner, DWORD& dw_team_id);

	// ���ֵ��䷽ʽ
	BOOL drop_item(Creature* p_creature, tagItem* &p_item, EDropTo e_drop_to, 
					Role* p_owner, DWORD dw_owner, DWORD dw_team_id, INT& r_index,
					EPickMode pick_mode = EPUM_Free, EGTAssignState assign_state = EGTAS_Null, INT tick = 0);

	// ���ֵ��䷽��
	BOOL drop_item_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, 
					Role* p_picker, BOOL b_team, FLOAT f_prod_delta, INT &r_index);

	BOOL drop_set_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, 
		Role* p_picker, BOOL b_team, FLOAT f_prod_delta, INT &r_index);

	BOOL drop_money_to(const tagCreatureLoot* p_proto, Creature* p_creature, EDropTo e_drop_to, 
		Role* p_picker, FLOAT f_prod_delta, bool bteam, INT &r_index);

	BOOL drop_quest_item(Role* p_picker, const tagItemProto* p_proto, DWORD dw_creature);

	//���������Ҫ���ݷ��䷽ʽ�趨�������
	VOID team_drop_set( tagItem* p_item, DWORD dw_team_id, EPickMode& pick_mode, 
					EGTAssignState& assign_state, INT& tick );

	VOID SendHearChat(Role* pRole, Creature* pCreature, tagItem* pItem);
};

#define g_drop_mgr drop_mgr::getSingleton()

