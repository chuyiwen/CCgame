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

#include "stdafx.h"

#include "../../common/WorldDefine/loot.h"
#include "../../common/WorldDefine/drop_protocol.h"
#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/quest_protocol.h"

#include "loot_mgr.h"
#include "unit.h"
#include "role.h"
#include "item_creator.h"
#include "creature.h"
#include "hearSay_helper.h"

//---------------------------------------------------------------------------------------------
// 怪物掉落
//---------------------------------------------------------------------------------------------
VOID drop_mgr::monster_drop(Creature* p_creature, Role* p_picker)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_picker) )
		return;
	
	if (p_creature->IsRes())
		return;

	// 进行小队任务掉落
	if(p_creature->GetProto()->nType2 != EGOT_QuestInves)
	{
		quest_drop(p_creature, p_picker, FALSE, TRUE);
	}
	
	//大于10级不掉落
	ASSERT(VALID_POINT(p_creature->get_map()));
	ASSERT(VALID_POINT(p_creature->get_map()->get_map_info()));
	if (p_creature->get_map()->get_map_info()->e_type != EMT_Instance && p_creature->get_map()->get_map_info()->e_type != EMT_Guild)
	{
		//if (p_picker->get_level()-p_creature->get_level() >=10)
		//	return;

		if (VALID_POINT(p_picker->GetSession()) && p_picker->GetSession()->GetFatigueGuarder().GetEarnRate() < 10000)
		{
			return;
		}
	}
	
	// 掉落在地面，非单一掉落， 小队掉落
	normal_drop(p_creature, p_picker, EDT_GROUND, FALSE, TRUE);
}

//----------------------------------------------------------------------------------------------
// 资源掉落
//----------------------------------------------------------------------------------------------
VOID drop_mgr::resource_drop(Creature* p_creature, Role* p_picker)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_picker) )
		return;

	// 掉落在包里，包满则掉落在地面，单一掉落，非小队掉落
	normal_drop(p_creature, p_picker, EDT_BAGFIRST, FALSE, FALSE);
}

//----------------------------------------------------------------------------------------------
// 调查地物掉落
//----------------------------------------------------------------------------------------------
VOID drop_mgr::investigate_drop(Creature* p_creature, Role* p_picker)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_picker) )
		return;

	// 非小队任务掉落
	quest_drop(p_creature, p_picker, TRUE, FALSE);

	// 强制落在背包里，否则删除，非单一掉落，非小队掉落
	normal_drop(p_creature, p_picker, EDT_GROUND, FALSE, TRUE);
	//normal_drop(p_creature, p_picker, EDT_MUSTBAG, FALSE, FALSE);
}
// 种植物掉落
VOID drop_mgr::resource_plant_drop(Creature* p_creature, Role* p_picker, int nNum)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_picker) )
		return;

	plant_drop(p_creature, p_picker, EDT_BAGFIRST, nNum);
}

//-------------------------------------------------------------------------------------------------------
// 普通掉落
//-------------------------------------------------------------------------------------------------------
VOID drop_mgr::normal_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, BOOL b_singel, BOOL b_team)
{
	if( !VALID_POINT(p_creature) ) return;
	if( !VALID_POINT(p_role) ) return;

	// 得到怪物掉落
	const tagCreatureLoot* pLootProto = AttRes::GetInstance()->GetCreatureLoot(p_creature->GetLootID());
	if( !VALID_POINT(pLootProto) ) return;

	// 掉落位置索引
	INT	nPosIndex =	0;

	// 得到物品掉落的加成率
	FLOAT fLootChanceAdd = creature_drop_prob_delta(p_creature, p_role);

	// 金钱掉落
	drop_money_to(pLootProto, p_creature, e_drop_to, p_role, fLootChanceAdd, b_team, nPosIndex);
	
	// 物品掉落
	for(INT nLootIndex = 0; nLootIndex < MAX_CREATURE_LOOT_NUM; ++nLootIndex)
	{
		const tagLoot* p_drop = &(pLootProto->Loot[nLootIndex]);
		if( !VALID_POINT(p_drop->dwItemID) ) break;

		DWORD bRtv = FALSE;

		// 如果是物品掉落
		if( ELT_Item == p_drop->eLootType )
		{
			bRtv = drop_item_to(p_drop, p_creature, e_drop_to, p_role, b_team, fLootChanceAdd, nPosIndex);
		}

		// 如果是集合掉落
		else if( ELT_Set == p_drop->eLootType )
		{
			bRtv = drop_set_to(p_drop, p_creature, e_drop_to, p_role, b_team, fLootChanceAdd, nPosIndex);
		}
		else
		{
			ASSERT(0);
			bRtv = FALSE;
		}

		// 如果是只掉落一次，则直接返回
		if( bRtv && b_singel )
			break;
	}
}

VOID drop_mgr::get_plant_drop_list(Creature* p_creature, int nNum, package_map<DWORD ,int>& mapPlantItem)
{
	if( !VALID_POINT(p_creature) ) return;

	// 得到怪物掉落
	const tagCreatureLoot* pLootProto = AttRes::GetInstance()->GetCreatureLoot(p_creature->GetLootID());
	if( !VALID_POINT(pLootProto) ) return;


	for (int i = 0; i < nNum; i++)
	{

		INT nPro = get_tool()->tool_rand() % 100000;
		FLOAT fChance = 0.0f;
		// 物品掉落
		for(INT nLootIndex = 0; nLootIndex < MAX_CREATURE_LOOT_NUM; ++nLootIndex)
		{
			const tagLoot* p_drop = &(pLootProto->Loot[nLootIndex]);
			if( !VALID_POINT(p_drop->dwItemID) ) break;

			DWORD bRtv = FALSE;


			if( !VALID_POINT(p_drop) || !VALID_POINT(p_creature))
				continue;

			if( ELT_Item != p_drop->eLootType ) continue;

			fChance += p_drop->fChance;
			// 首先判断一下掉率，看是否满足
			if( nPro > INT(fChance * 100000.0f) )
				continue;
			
			mapPlantItem.modify_value(p_drop->dwItemID, 1);

			break;
	
		}

	}
}
// 种植物掉落
VOID drop_mgr::plant_drop(Creature* p_creature, Role* p_role, EDropTo e_drop_to, int nNum)
{
	if( !VALID_POINT(p_creature) ) return;
	if( !VALID_POINT(p_role) ) return;

	// 得到怪物掉落
	const tagCreatureLoot* pLootProto = AttRes::GetInstance()->GetCreatureLoot(p_creature->GetLootID());
	if( !VALID_POINT(pLootProto) ) return;

	// 掉落位置索引
	INT	nPosIndex =	0;
	INT nPro = get_tool()->tool_rand() % 100000;
	FLOAT fChance = 0.0f;

	package_map<DWORD, int> mapPlantItem;
	get_plant_drop_list(p_creature, nNum, mapPlantItem);
	
	DWORD dwItemID = 0;
	int nNumber = 0;
	mapPlantItem.reset_iterator();
	while(mapPlantItem.find_next(dwItemID, nNumber))
	{
		tagItemProto* pProto = NULL;
		if( MIsEquipment(dwItemID) )
		{
			pProto = AttRes::GetInstance()->GetEquipProto(dwItemID);
		}
		else
		{
			pProto = AttRes::GetInstance()->GetItemProto(dwItemID);
		}
		if( !VALID_POINT(pProto) ) continue;

		// 生成物品
		tagItem* pItem = ItemCreator::CreateEx(EICM_Loot, p_creature->GetID(), pProto->dw_data_id, nNumber, EIQ_Null);
		if( !VALID_POINT(pItem) ) continue;

		// 掉落物品
		BOOL bRet = drop_item(p_creature, pItem, e_drop_to, p_role, p_role->GetID(), INVALID_VALUE, nPosIndex, EPUM_Free, EGTAS_Null, 0);

		if( bRet ) ++nPosIndex;

	}
}
//-------------------------------------------------------------------------------------------------------
// 任务掉落
//-------------------------------------------------------------------------------------------------------
VOID drop_mgr::quest_drop(Creature* p_creature, Role* p_role, BOOL bInves, BOOL b_team)
{
	if( !VALID_POINT(p_creature) ) return;
	if( !VALID_POINT(p_role) ) return;

	const Team* pTeam = g_groupMgr.GetTeamPtr(p_role->GetTeamID());

	// 得到任务掉落
	const tagLootQuestItem* pQuestLoot	= AttRes::GetInstance()->GetLootQuestItem(p_creature->GetTypeID());
	if( !VALID_POINT(pQuestLoot) ) return;

	for(INT i = 0; i < MAX_LOOT_QUEST_ITEM_NUM; ++i)
	{
		DWORD dwItemID = pQuestLoot->QuestItem[i].dwQuestItemID;

		// 如果有小队，取小队几率，否则取玩家个人几率
		FLOAT fChance = 0.0f;
		if( VALID_POINT(pTeam) )
		{
			fChance = pQuestLoot->QuestItem[i].fTeamChance;
		}
		else
		{
			fChance = pQuestLoot->QuestItem[i].fChance;
		}

		// 检查物品ID是否为零
		if( !VALID_POINT(dwItemID) ) break;

		// 计算几率
		if( get_tool()->tool_rand() % 100000 > INT(fChance * 100000.0f) )
		{
			if(bInves)
			{
				NET_SIS_quest_inves_faild send;
				p_role->SendMessage(&send, send.dw_size);
			}
			continue;
		}

		// 找到物品
		tagItemProto* pProto = NULL;
		if( MIsEquipment(dwItemID) )
		{
			pProto = AttRes::GetInstance()->GetEquipProto(dwItemID);
		}
		else
		{
			pProto = AttRes::GetInstance()->GetItemProto(dwItemID);
		}

		if( !VALID_POINT(pProto) ) continue;

		// 队伍任务拾取
		if(b_team && VALID_POINT(pTeam))
		{
			INT		nTeamNum	=	pTeam->get_member_number();
			Role*	p_picker		=	NULL;

			for(INT i = 0; i < nTeamNum; ++i)
			{
				p_picker = pTeam->get_member(i);

				if( !VALID_POINT(p_picker) || !p_creature->IsLootShareDistance(p_picker) )
					continue;

				drop_quest_item(p_picker, pProto, p_creature->GetID());
			}
		}
		// 单人任务拾取
		else
		{
			drop_quest_item(p_role, pProto, p_creature->GetID());
		}				
	}
}

//-------------------------------------------------------------------------------------------------------
// 取得物品掉落位置
//-------------------------------------------------------------------------------------------------------
INT drop_mgr::get_drop_pos(Unit* p_unit, Vector3 &r_pos, INT &r_index)
{
	ASSERT(VALID_POINT(p_unit));

	M_trans_else_ret(pMap, p_unit->get_map(), Map, 0);

	for( ; r_index < MAX_LOOT_POS; ++r_index)
		if (pMap->can_putdown(p_unit, r_index, r_pos))
			return TRUE;

	return FALSE;
}

//-------------------------------------------------------------------------------------------------------
// 得到生物掉率加成
//-------------------------------------------------------------------------------------------------------
FLOAT drop_mgr::creature_drop_prob_delta(Creature* p_creature, Role* p_owner)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_owner) ) return 1.0f;

	if( ECTT_All == p_creature->GetTaggedType() ) return 1.0f;

	// 如果是资源
	if( p_creature->IsRes() )
	{
		return resource_drop_prob_delta(p_creature, p_owner);
	}
	else if( p_creature->IsInves() || p_creature->IsInves_two())
	{
		return investobj_drop_prob_delta(p_owner);
	}
	else
	{
		return moster_drop_prob_delta(p_owner);
	}
}

//-------------------------------------------------------------------------------------------------------
// 怪物掉率加成
//-------------------------------------------------------------------------------------------------------
FLOAT drop_mgr::moster_drop_prob_delta(Role* p_owner)
{
	if( !VALID_POINT(p_owner) ) return 1.0f;

	FLOAT	fGMLootRate		=	p_owner->get_map()->get_drop_rate();				// 全服双倍，0-5 倍数
	INT		nFatigueRate	=	p_owner->GetEarnRate();							// 防沉迷， 0-10000映射为0-1的分数 倍数
	INT		nAddOn			=	p_owner->GetAttValue(ERA_Loot_Add_Rate);		// 自身掉率加成
	FLOAT	fVNBLootAdd		=	p_owner->GetVNBLootRate();

	return fGMLootRate * (FLOAT(nFatigueRate) / 10000.0f) * (1.0f + FLOAT(nAddOn) / 10000.0f + fVNBLootAdd);
}

//-------------------------------------------------------------------------------------------------------
// 资源掉率加成
//-------------------------------------------------------------------------------------------------------
FLOAT drop_mgr::resource_drop_prob_delta(Creature* p_creature, Role* p_owner)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_owner) ) return 1.0f;

	INT		nFatigueRate	=	p_owner->GetEarnRate();							// 防沉迷， 0-10000映射为0-1的分数 倍数
	INT		nAddOn			=	p_owner->CalGatherRate(p_creature);				// 采集加成

	return (FLOAT(nFatigueRate) / 10000.0f) * (1.0f + FLOAT(nAddOn) / 10000.0f);
}

//-------------------------------------------------------------------------------------------------------
// 可调查地物掉率加成
//-------------------------------------------------------------------------------------------------------
FLOAT drop_mgr::investobj_drop_prob_delta(Role* p_owner)
{
	return 1.0f;
}

//----------------------------------------------------------------------------------------
// 得到某个掉落的所属
//----------------------------------------------------------------------------------------
VOID drop_mgr::set_drop_owner(Creature* p_creature, Role* p_picker, BOOL b_team, DWORD& dw_owner, Role* &p_owner, DWORD& dw_team_id)
{
	// 全体所属
	if( ECTT_All == p_creature->GetTaggedType() )
	{
		dw_team_id	=	INVALID_VALUE;
		dw_owner	=	INVALID_VALUE;
		p_owner	=	NULL;
	}
	// 非全体所属
	else if (ECTT_NOTALL == p_creature->GetTaggedType() )
	{
		dw_team_id	=	INVALID_VALUE;
		dw_owner	=	-2;
		p_owner	=	NULL;
	}
	else
	{
		if( b_team )
		{
			dw_team_id = p_picker->GetTeamID();

			if( VALID_POINT(dw_team_id) )
			{
				const Team* pTeam = g_groupMgr.GetTeamPtr(dw_team_id);
				if( VALID_POINT(pTeam) )
				{
					p_owner = pTeam->get_pick_role(p_creature);
				}
				else
				{
					p_owner = NULL;
				}

				if( VALID_POINT(p_owner) )
				{
					dw_owner = p_owner->GetID();
				}
				else
				{
					dw_owner = INVALID_VALUE;
				}
			}
			else
			{
				p_owner = p_picker;
				dw_owner = p_picker->GetID();
			}
		}
		else
		{
			dw_team_id	=	INVALID_VALUE;
			dw_owner	=	p_picker->GetID();
			p_owner	=	p_picker;
		}
	}
}

//-----------------------------------------------------------------------------------------
// 各种掉落方式
//-----------------------------------------------------------------------------------------
BOOL drop_mgr::drop_item(Creature* p_creature, tagItem* &p_item, EDropTo e_drop_to, Role* p_owner, 
						 DWORD dw_owner, DWORD dw_team_id, INT& r_index, 
						 EPickMode pick_mode, EGTAssignState assign_state, INT tick)
{
	if( !VALID_POINT(p_creature) || !VALID_POINT(p_item) ) return FALSE;

	// 得到地图
	Map* pMap = p_creature->get_map();
	if( !VALID_POINT(pMap) ) return FALSE;

	// 根据掉落方式
	switch(e_drop_to)
	{
		// 强制掉落背包
	case EDT_MUSTBAG:
		{
			if( !VALID_POINT(p_owner) || E_Success != p_owner->GetItemMgr().Add2Bag(p_item, elcid_loot, TRUE) )
			{
				::Destroy(p_item);
				return FALSE;
			}
			if( !VALID_POINT(p_owner)) SendHearChat(p_owner, p_creature, p_item);
			return TRUE;
		}
		break;

		// 先背包
	case EDT_BAGFIRST:
		{
			if( VALID_POINT(p_owner) )
			{
				if( E_Success == p_owner->GetItemMgr().Add2Bag(p_item, elcid_loot, TRUE) )
				{
					if( !VALID_POINT(p_owner)) SendHearChat(p_owner, p_creature, p_item);
					return TRUE;
				}
				else
				{
					Vector3 vPos;
					if( !get_drop_pos(p_creature, vPos, r_index) )
					{
						::Destroy(p_item);
						return FALSE;
					}
					pMap->putdown_item(p_creature, p_item, dw_owner, dw_team_id, vPos, pick_mode, assign_state, tick);

					return TRUE;
				}
			}
			else
			{
				Vector3 vPos;
				if( !get_drop_pos(p_creature, vPos, r_index) )
				{
					::Destroy(p_item);
					return FALSE;
				}
				pMap->putdown_item(p_creature, p_item, dw_owner, dw_team_id, vPos, pick_mode, assign_state, tick);

				return TRUE;
			}
		}
		break;

		// 掉落地面
	case EDT_GROUND:
		{
			Vector3 vPos;
			if( !get_drop_pos(p_creature, vPos, r_index) )
			{
				::Destroy(p_item);
				return FALSE;
			}
			pMap->putdown_item(p_creature, p_item, dw_owner, dw_team_id, vPos, pick_mode, assign_state, tick);
			return TRUE;
		}
		break;

		// 默认
	default:
		{
			ASSERT(0);
			return FALSE;
		}
		break;
	}
}

//-------------------------------------------------------------------------------------------------------
// 金钱掉落
//-------------------------------------------------------------------------------------------------------
BOOL drop_mgr::drop_money_to( const tagCreatureLoot* p_proto, Creature* p_creature, EDropTo e_drop_to, 
							 Role* p_picker, FLOAT f_prod_delta,  bool bteam, INT &r_index )
{
	if( !VALID_POINT(p_proto) ) return FALSE;
	if( !VALID_POINT(p_creature) ) return FALSE;

	// 得到怪物所在地图
	Map* pMap = p_creature->get_map();
	if( !VALID_POINT(pMap) ) return FALSE;

	// 得到实际掉落的钱数
	INT nMoney = INT(get_tool()->rand_in_range(p_proto->nMinMoney, p_proto->nMaxMoney) * f_prod_delta);
	if( nMoney <= 0 ) return FALSE;

	INT nPro = get_tool()->tool_rand() % 10000;
	if (nPro > p_proto->fMoneyChance * 10000)
		return FALSE;

	INT nNum = get_tool()->rand_in_range(p_proto->nMoneyNumMin, p_proto->nMoneyNumMax);
	if (nNum <= 0)
		return FALSE;

	DWORD dw_team_id	= INVALID_VALUE;
	Role* p_owner = NULL;
	DWORD dw_owner = INVALID_VALUE;

	set_drop_owner(p_creature, p_picker, bteam, dw_owner, p_owner, dw_team_id);
	if (VALID_POINT(p_owner))
	{
		p_owner->GetCurMgr().IncBagSilver(nMoney, elcid_pickup_money);
		return TRUE;
	}
	if( bteam )
	{
		dw_team_id = p_picker->GetTeamID();

		if( VALID_POINT(dw_team_id) )
		{
			const Team* pTeam = g_groupMgr.GetTeamPtr(dw_team_id);
			if( VALID_POINT(pTeam) )
			{
				INT nRoleMoney = nMoney/pTeam->get_member_number();
				for (int i = 0; i < pTeam->get_member_number(); i++)
				{
					Role* pRole = pTeam->get_member(i);
					if (VALID_POINT(pRole))
					{
						pRole->GetCurMgr().IncBagSilver(nRoleMoney, elcid_pickup_money);
					}
				}

				return TRUE;
			}
			
		}
		
	}
	
	for (int i = 0; i < nNum; i++)
	{
		// 得到掉落位置
		Vector3 vPos(0.0f, 0.0f, 0.0f);
		BOOL bHasPositon = get_drop_pos(p_creature, vPos, r_index);
		if( !bHasPositon ) return FALSE;

		pMap->putdown_money(p_creature, nMoney, dw_team_id, dw_owner, vPos);
		++r_index;
	}
	


	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 集合掉落
//-------------------------------------------------------------------------------------------------------
BOOL drop_mgr::drop_set_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, Role* p_picker,
						   BOOL b_team, FLOAT f_prod_delta, INT &r_index)
{
	if( !VALID_POINT(p_drop) ) return FALSE;
	if( !VALID_POINT(p_creature) ) return FALSE;
	if( !VALID_POINT(p_picker) ) return FALSE;

	if( ELT_Set != p_drop->eLootType ) return FALSE;

	// 得到怪物地图
	Map* pMap = p_creature->get_map();
	if( !VALID_POINT(pMap) ) return FALSE;

	// 如果已经掉满了，则不掉落了
	if( r_index >= MAX_LOOT_POS ) return FALSE;

	// 得到掉落集合
	const tagLootItemSet* pItemSet = AttRes::GetInstance()->GetLootItemSet(p_drop->dwItemID);
	if( !VALID_POINT(pItemSet) ) return FALSE;

	// 判断一下掉率
	FLOAT fRealChance = p_drop->fChance * f_prod_delta;
	if( get_tool()->tool_rand() % 100000 > INT(fRealChance * 100000.0f) )
		return FALSE;

	// 掉落几件
	INT nDropNum = get_tool()->rand_in_range(p_drop->nMin, p_drop->nMax);

	// 集合中有效物品
	std::vector<INT> listValid;
	for(INT i = 0; i < MAX_ITEM_SET_NUM; ++i)
	{
		if( 0 == pItemSet->ItemSet[i].dwItemID || pItemSet->ItemSet[i].nItemNum <= 0)
			continue;

		listValid.push_back(i);
	}
	if( listValid.size() <= 0 )
		return FALSE;

	// 对于每一项
	for(INT i = 0; i < nDropNum && r_index < MAX_LOOT_POS; ++i)
	{
		//INT nSetIndex = INVALID_VALUE;	// 获得索引
		//listValid.rand_find(nSetIndex);
		
		INT nSetIndex = 0;
		INT32 nSetIndexSumPct = 0;
		INT32 nRandPct = get_tool()->tool_rand() % 10000;
		for(INT32 i=listValid.size()-1; i>0; --i)
		{
			nSetIndexSumPct += (pItemSet->ItemSet[listValid[i]].fChance * 10000);
			if(nRandPct < nSetIndexSumPct)
			{
				nSetIndex = i;
				break;
			}
		}

		// 得到物品属性
		tagItemProto* pProto = NULL;
		if( MIsEquipment(pItemSet->ItemSet[nSetIndex].dwItemID) )
		{
			pProto = AttRes::GetInstance()->GetEquipProto(pItemSet->ItemSet[nSetIndex].dwItemID);
		}
		else
		{
			pProto = AttRes::GetInstance()->GetItemProto(pItemSet->ItemSet[nSetIndex].dwItemID);
		}
		if( !VALID_POINT(pProto) ) continue;
		
		// 得到品质
		//EItemQuality nQuality = EIQ_Quality0;
		//INT32 nEquipQltySumPct = 0;
		//nRandPct = get_tool()->tool_rand() % 10000;
		//for(INT32 i=EIQ_End - 1; i>EIQ_Start; --i)
		//{
		//	nEquipQltySumPct += pItemSet->ItemSet->nEquipQltyPct[i];
		//	if(nRandPct < nEquipQltySumPct)
		//	{
		//		nQuality = (EItemQuality)i;
		//		break;
		//	}
		//}
	
		EItemQuality nQuality = get_loot_equip_quality();
		// 生成物品
		tagItem* pItem = ItemCreator::CreateEx(EICM_Loot, p_creature->GetID(), pProto->dw_data_id, pItemSet->ItemSet[nSetIndex].nItemNum, nQuality);
		if( !VALID_POINT(pItem) ) continue;

		// 找到本次掉落的所属
		DWORD dw_team_id	= INVALID_VALUE;
		Role* p_owner = NULL;
		DWORD dw_owner = INVALID_VALUE;
		EPickMode	ePickMode = EPUM_Free;
		EGTAssignState eAssignState = EGTAS_Null;
		INT	nAssignTickDown	=	0;

		set_drop_owner(p_creature, p_picker, b_team, dw_owner, p_owner, dw_team_id);
		if( b_team ) team_drop_set( pItem, dw_team_id, ePickMode, eAssignState, nAssignTickDown );

		// 掉落物品
		if( drop_item(p_creature, pItem, e_drop_to, p_owner, dw_owner, dw_team_id, r_index, ePickMode, eAssignState, nAssignTickDown ) )
		{
			++r_index;
		}
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------------
// 物品掉落
//-------------------------------------------------------------------------------------------------------
BOOL drop_mgr::drop_item_to(const tagLoot* p_drop, Creature* p_creature, EDropTo e_drop_to, 
							Role* p_picker, BOOL b_team, FLOAT f_prod_delta, INT &r_index)
{
	if( !VALID_POINT(p_drop) || !VALID_POINT(p_creature) || !VALID_POINT(p_picker) )
		return FALSE;

	if( ELT_Item != p_drop->eLootType ) return FALSE;

	// 得到怪物地图
	Map* pMap = p_creature->get_map();
	if( !VALID_POINT(pMap) ) return FALSE;

	// 如果已经掉满了，则不掉落了
	if( r_index >= MAX_LOOT_POS ) return FALSE;

	// 首先判断一下掉率，看是否满足
	FLOAT fRealChance = p_drop->fChance * f_prod_delta;
	if( get_tool()->tool_rand() % 100000 > INT(fRealChance * 100000.0f) )
		return FALSE;

	// 得到物品属性
	tagItemProto* pProto = NULL;
	if( MIsEquipment(p_drop->dwItemID) )
	{
		pProto = AttRes::GetInstance()->GetEquipProto(p_drop->dwItemID);
	}
	else
	{
		pProto = AttRes::GetInstance()->GetItemProto(p_drop->dwItemID);
	}
	if( !VALID_POINT(pProto) ) return FALSE;
	
	// 得到物品数目
	INT16 n16RandNum = get_tool()->rand_in_range(p_drop->nMin, p_drop->nMax);
	if( n16RandNum <= 0 ) return FALSE;
	
	// 得到品质
	//EItemQuality nQuality = EIQ_Quality0;
	//INT32 nEquipQltySumPct = 0;
	//INT32 nRandPct = get_tool()->tool_rand() % 10000;
	//for(INT32 i=EIQ_End - 1; i>EIQ_Start; --i)
	//{
	//	nEquipQltySumPct += p_drop->nEquipQltyPct[i];
	//	if(nRandPct < nEquipQltySumPct)
	//	{
	//		nQuality = (EItemQuality)i;
	//		break;
	//	}
	//}
	EItemQuality nQuality = get_loot_equip_quality();

	// 生成物品
	tagItem* pItem = ItemCreator::CreateEx(EICM_Loot, p_creature->GetID(), pProto->dw_data_id, n16RandNum, nQuality);
	if( !VALID_POINT(pItem) ) return FALSE;

	// 找到本次掉落的所属
	DWORD dw_team_id	= INVALID_VALUE;
	Role* p_owner = NULL;
	DWORD dw_owner = INVALID_VALUE;
	EPickMode	ePickMode = EPUM_Free;
	EGTAssignState eAssignState = EGTAS_Null;
	INT	nAssignTickDown	=	0;

	set_drop_owner(p_creature, p_picker, b_team, dw_owner, p_owner, dw_team_id);
	if( b_team ) team_drop_set( pItem, dw_team_id, ePickMode, eAssignState, nAssignTickDown );

	// 掉落物品
	BOOL bRet = drop_item(p_creature, pItem, e_drop_to, p_owner, dw_owner, dw_team_id, r_index, ePickMode, eAssignState, nAssignTickDown);

	if( bRet ) ++r_index;

	return bRet;
}

//-------------------------------------------------------------------------------------------------------
// 任务物品掉落
//-------------------------------------------------------------------------------------------------------
BOOL drop_mgr::drop_quest_item(Role* p_picker, const tagItemProto* p_proto, DWORD dw_creature)
{
	if( !VALID_POINT(p_picker) ) return FALSE;
	if( !VALID_POINT(p_proto) ) return FALSE;

	// 找到该物品对应的任务
	UINT16	u16QuestID = (UINT16)p_proto->dwQuestID;

	// 玩家当前任务是否有该任务
	quest* pQuest = p_picker->get_quest(u16QuestID);
	if( !VALID_POINT(pQuest) ) 
		pQuest = p_picker->get_quest(QuestIDHelper::GenerateID(u16QuestID, TRUE));
	if( !VALID_POINT(pQuest) ) return FALSE;

	// 检测该任务的任务物品是否已经满了
	if( pQuest->is_item_full(p_proto->dw_data_id) ) return FALSE;

	// 生成任务物品
	tagItem* pItem = ItemCreator::Create(EICM_Loot, dw_creature, p_proto->dw_data_id, 1);
	if( !VALID_POINT(pItem) ) return FALSE;

	// 在任务背包里面加入
	if( p_picker->GetItemMgr().Add2QuestBag(pItem, elcid_loot) )
	{
		::Destroy(pItem);
		return FALSE;
	}

	return TRUE;
}
//队伍掉落需要根据分配方式设定相关数据
VOID drop_mgr::team_drop_set( tagItem* p_item, DWORD dw_team_id, 
							 EPickMode& pick_mode, EGTAssignState& assign_state, INT& tick )
{
	assign_state = EGTAS_Null; tick = 0;

	const Team* pTeam = g_groupMgr.GetTeamPtr( dw_team_id );
	pick_mode = VALID_POINT( pTeam ) ? pTeam->get_pick_mode( ) : EPUM_Free;

	if( !VALID_POINT(pTeam) || !VALID_POINT(p_item->pProtoType) ) return;
	if( !MIsEquipment(p_item->dw_data_id) )
	{
		if(p_item->pProtoType->byQuality < pTeam->get_assign_quality( ) )
			return ;
	}
	else if( ((tagEquip*)p_item)->GetQuality() < pTeam->get_assign_quality( ) ) return;

	switch( pick_mode )
	{
	case EPUM_Leader:
		assign_state = EGTAS_WaitForUmpirage;
		tick = 65 * TICK_PER_SECOND;
		break;
	case EPUM_Sice:
		assign_state = EGTAS_WaitForUmpirage;
		tick = 65 * TICK_PER_SECOND;
		break;
	};
}

VOID drop_mgr::SendHearChat(Role* pRole, Creature* pCreature, tagItem* pItem)
{
	if(!VALID_POINT(pRole) ||
		!VALID_POINT(pItem) ||
		!VALID_POINT(pCreature) || 
		!VALID_POINT(pItem->pProtoType) ||
		!VALID_POINT(pCreature->GetProto()) )
		return;

	DWORD dwBossID =  pCreature->GetProto()->is_boss() ?
		pCreature->GetProto()->dw_data_id : 0;
	
	if( HearSayHelper::IsItemBroadcast( dwBossID, pItem ))
		return ;

	HearSayHelper::SendMessage(EHST_KILLBOSSGETITEM,
		pRole->GetID(), pItem->dw_data_id, pCreature->GetProto()->dw_data_id, INVALID_VALUE, INVALID_VALUE, pItem);
}

EItemQuality drop_mgr::get_loot_equip_quality()
{
	INT nParam[] = { 4989, 0, 4500, 500, 10, 1, 0};
	EItemQuality nQuality = EIQ_Quality0;
	INT32 nEquipQltySumPct = 0;
	INT32 nRandPct = get_tool()->tool_rand() % 10000;
	for(INT32 i=EIQ_End - 1; i>EIQ_Start; --i)
	{
		nEquipQltySumPct += nParam[i];
		if(nRandPct < nEquipQltySumPct)
		{
			nQuality = (EItemQuality)i;
			break;
		}
	}
	return nQuality;
}
