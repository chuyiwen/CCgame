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
*	@file		att_res.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		资源管理器定义
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/skill_define.h"
#include "../../common/WorldDefine/buff_define.h"
#include "../../common/WorldDefine/trigger_define.h"
#include "../../common/WorldDefine/compose_define.h"
#include "../../common/WorldDefine/shop_define.h"
#include "../../common/WorldDefine/func_npc_define.h"
#include "../../common/WorldDefine/move_define.h"
#include "../../common/WorldDefine/compose_define.h"
#include "../../common/WorldDefine/suit_define.h"
#include "../../common/WorldDefine/mall_define.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../../common/WorldDefine/role_god_level.h"
#include "../../common/WorldDefine/Raid_define.h"

#include "pet_server_define.h"
#include "../../common/WorldDefine/pet_equip_define.h"
#include "../../common/WorldDefine/motion_define.h"

#include "pet_skill_server_define.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/consolidate_server_define.h"
#include "sspawnpoint_define.h"
#include "../common/ServerDefine/vip_netbar_server_define.h"
#include "../common/ServerDefine/guild_server_define.h"

#include "att_res.h"
#include "script_mgr.h"
#include "guild_upgrade.h"
#include "guild_chamber.h"

#include "world.h"

//********************************* 读取标志位常量 **********************************
const INT X_READ_FILE_1	= 1;
const INT X_READ_FILE_2	= 2;

//*********************************** 文件名常量 ************************************
const LPCTSTR FileName_NameFilter		= _T("data/local/zhCN/table/Nameban.xml");

const LPCTSTR FileName_RoleAttLevelUp	= _T("data/config/table/RoleLevelUp.xml");

const LPCTSTR FileName_Item				= _T("data/config/table/ItemInfo.xml");
const LPCTSTR FileName_Equip			= _T("data/config/table/EquipInfo.xml");
//const LPCTSTR FileName_Gem				= _T("data/config/table/gem_proto.xml");
const LPCTSTR FileName_EquipQltyEffect	= _T("data/config/table/EquipQualityAtt.xml");
const LPCTSTR FileName_Suit				= _T("data/config/table/EquipSetInfo.xml");

const LPCTSTR FileName_Shop				= _T("data/config/table/ShopInfo.xml");
const LPCTSTR FileName_ShopRare			= _T("data/config/table/ShopRation.xml");

const LPCTSTR FileName_Dak				= _T("data/config/table/SceneMoveInfo.xml");

const LPCTSTR FileName_buff             = _T("data/config/table/BuffInfo.xml");
const LPCTSTR FileName_skill            = _T("data/config/table/SkillInfo.xml");
const LPCTSTR FileName_trigger          = _T("data/config/table/TriggerInfo.xml");


const LPCTSTR FileName_creature			= _T("data/config/table/NpcInfo.xml");

const LPCTSTR FileName_Consolidate_Proto= _T("data/config/table/EnchaseItem.xml");

const LPCTSTR FileName_Produce_Proto	= _T("data/config/table/ComposeInfo.xml");
const LPCTSTR FileName_DeCompose_Proto  = _T("data/config/table/DecomposeInfo.xml");
const LPCTSTR FileName_EquipChange_Proto = _T("data/config/table/EquipChange.xml");
const LPCTSTR FileName_Instance_Item	= _T("data/config/table/instance_item.xml");
const LPCTSTR FileName_Instance_Skill	= _T("data/config/table/instance_item.xml");
const LPCTSTR FileName_Level_Mapping	= _T("data/config/table/level_mapping.xml");
const LPCTSTR FileName_Instance_Proto	= _T("data/config/table/InstanceInfo.xml");

const LPCTSTR FileName_Title_Proto		= _T("data/config/table/RoleTitleInfo.xml");
const LPCTSTR FileName_Achievement		= _T("data/config/table/RoleAchievement.xml");
const LPCTSTR FileName_Achievement_Criteria		= _T("data/config/table/RoleAchieveCriteriament.xml");

const LPCTSTR FileName_GuildAffair		= _T("data/config/table/GuildAffair.xml");
const LPCTSTR FileName_GuildSkill		= _T("data/config/table/GuildSkill.xml");
const LPCTSTR FileName_GuildCommerce	= _T("data/config/table/GuildCommerce.xml");

const LPCTSTR FileName_GuildBannerPos	= _T("data/config/table/guild_pvp_banner_pos.xml");

const LPCTSTR FileName_Pet_Proto				= _T("data/config/table/PetInfo.xml");

const LPCTSTR FileName_PetSkill_Proto			= _T("data/config/table/PetSkillInfo.xml");

const LPCTSTR FileName_PetGather_Proto			= _T("data/config/table/pet_gather_proto.xml");


const LPCTSTR FileName_Weather_Proto			= _T("data/config/table/WeathersInfo.xml");
const LPCTSTR FileName_Formula_Proto			= _T("data/config/table/FormulaInfo.xml");
const LPCTSTR FileName_EquipLevel_Proto			= _T("data/config/table/EquipLevelup.xml");
const LPCTSTR FileName_LearnSkillList_Proto		= _T("data/config/table/LearnSkill.xml");
const LPCTSTR FileName_EquipQltyPct				= _T("data/config/table/EquipCreateQuality.xml");

const LPCTSTR FileName_RoleAttChange			= _T("server_data/RoleAttChange.xml");
const LPCTSTR FileName_AttDefMinMax				= _T("server_data/RoleDefaultAtt.xml");


const LPCTSTR FileName_FashionQltyEffect		= _T("server_data/fashion_qlty_effect.xml");
const LPCTSTR FileName_FashionColorPct			= _T("server_data/fashion_color_pct.xml");

const LPCTSTR FileName_MallItem					= _T("server_data/mall_item_proto.xml");
const LPCTSTR FileName_MallPack					= _T("server_data/mall_pack_proto.xml");
const LPCTSTR FileName_MallFreeItem				= _T("server_data/mall_free_item_proto.xml");

const LPCTSTR FileName_VIPStall					= _T("server_data/vip_stall_proto.xml");

const LPCTSTR FileName_EquipLevelPct_Proto		= _T("data/config/table/EquipLevelAtt.xml");
const LPCTSTR FileName_GuildCommodity			= _T("server_data/GuildCommodityInfo.xml");
const LPCTSTR FileName_CreatureLoot_Proto		= _T("server_data/Drop.xml");
const LPCTSTR FileName_LootItemSet_Proto		= _T("server_data/DropSet.xml");
const LPCTSTR FileName_LootQuestItem_Proto		= _T("server_data/DropQuest.xml");

const LPCTSTR FildName_GuildEnlisteePos			= _T("server_data/GuildEnlisteePos.xml");
const LPCTSTR FileName_GuildUpgrade				= _T("server_data/GuildFacility.xml");
const LPCTSTR FileName_GuildCofC				= _T("server_data/GuildGoodsInfo.xml");
const LPCTSTR FileName_GuildCofC_SP				= _T("server_data/GuildGoodsSpe.xml");
const LPCTSTR FileName_GuildGradePos			= _T("server_data/GuildGradePos.xml");
const LPCTSTR FileName_Guild_Material_Receive	= _T("data/config/table/MaterialReceive.xml");
const LPCTSTR FileName_NewRoleGift_Proto		= _T("data/config/table/NewRoleGift.xml");

const LPCTSTR FileName_creature_ai				= _T("server_data/NpcAI.xml");
const LPCTSTR FileName_creature_camp			= _T("data/config/table/NpcCamp.xml");
const LPCTSTR FileName_PetLvlUpProto			= _T("server_data/PetLevelup.xml");
const LPCTSTR FileName_PetLvlUpItemProto		= _T("server_data/PetLevelupItem.xml");
const LPCTSTR FileName_PetRandomSKill			= _T("server_data/PetRandomSkill.xml");
const LPCTSTR FileName_SingleSpawnPoint_Proto	= _T("server_data/SpawnPoint.xml");
const LPCTSTR FileName_Spawn_Point				= _T("server_data/SpawnPoint.xml");
const LPCTSTR FileName_Spawn_Group				= _T("server_data/SpawnGroup.xml");
const LPCTSTR FileName_Spawn_Guild_Group		= _T("server_data/SpawnGuildGroup.xml");

const LPCTSTR FileName_Scene_Fishing = _T("data/config/table/SceneFishInfo.xml");

const LPCTSTR FileName_YanZhenMa				= _T("server_data/yanzhengma.xml");
const LPCTSTR FileName_VIP						= _T("data/config/table/VipInfo.xml");

const LPCTSTR FileName_Lottery		= _T("data/config/table/lottery.xml");
const LPCTSTR FileName_Sign				=	_T("data/config/table/SignInfo.xml");
const LPCTSTR FileName_Huenjing		= _T("data/config/table/HuenJing.xml");
const LPCTSTR FileName_GodLevel		= _T("data/config/table/GodLevel.xml");

const LPCTSTR FileName_Raid			= _T("data/config/table/Raid.xml");


//AttRes g_attRes;
AttRes* AttRes::m_pInstance = NULL;

//*********************************** 模版方法 ************************************

//-----------------------------------------------------------------------------
// 读取资源文件模版函数(如果使用szFileName2，则field中对应的字段应该为key)
//-----------------------------------------------------------------------------
template<class K, class T>
BOOL AttRes::LoadResMap(package_map<K, T*> &mapRes, LPCTSTR szFileName, LPCTSTR szFileName2/* = NULL*/)
{
	std::list<tstring>				ListField;
	std::list<tstring>::iterator	it;

	// 读入文件
	if(!m_pVar->load(g_world.get_virtual_filesys(), szFileName, "id", &ListField))
	{
		return FALSE;
	}

	for(it = ListField.begin(); it != ListField.end(); ++it)
	{
		T* pResNode = new T;

		// 初始化
		K key = *((K*)m_pFun(pResNode, it->c_str(), X_READ_FILE_1));

		// 放入map中
		mapRes.add(key, pResNode);
	}

	// 发送读入基本物品数目信息
	INT nOneMem = mapRes.size() * sizeof(T);
	print_message(_T("Read %d records from file<%s>!\n"), mapRes.size(), szFileName);
	g_world.get_log()->write_log(_T("Read %d BYTE memory from file<%s>!\n"), nOneMem, szFileName);
	m_nMemoryCount +=  nOneMem;
	g_world.get_log()->write_log(_T("Count memory is %d BYTE!\n"), m_nMemoryCount);

	// 清空容器
	m_pVar->clear();
	
	// 如果文件2不为NULL，也同时读入
	if(szFileName2 != NULL)
	{
		ListField.clear();
		
		// 注意：此处用id2，是为了避免读入Field时与文件1冲突
		m_pVar->load(g_world.get_virtual_filesys(), szFileName2, "id2", &ListField);

		for(it = ListField.begin(); it != ListField.end(); ++it)
		{
			K key = (K)_atoi64(get_tool()->unicode_to_unicode8(it->c_str()));
			T* pResNode = mapRes.find(key);
			if(!VALID_POINT(pResNode))
			{
				ASSERT(0);
				print_message(_T("There is something wrong in reading proto<%s>!\n"), szFileName2);
				continue;
			}

			// 初始化
			m_pFun(pResNode, it->c_str(), X_READ_FILE_2);
		}

		// 发送读入基本物品数目信息
		print_message(_T("Read %d records from file<%s>!\n"), ListField.size(), szFileName2);

		// 清空容器
		m_pVar->clear();
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 释放资源的模版函数
//-----------------------------------------------------------------------------
template<class K, class T> 
VOID AttRes::FreeResMap(package_map<K, T*> &mapRes)
{
	T *pResNode = NULL;

	mapRes.reset_iterator();
	while(mapRes.find_next(pResNode))
	{
		SAFE_DELETE(pResNode);
	}

	mapRes.clear();
}

//-----------------------------------------------------------------------------
// 读取资源文件模版函数
//-----------------------------------------------------------------------------
template<class T>
BOOL AttRes::LoadResArray(T *arrayRes, INT32 nIndexStart, INT32 nIndexEnd, LPCTSTR szFileName)
{
	std::list<tstring>				ListField;
	std::list<tstring>::iterator	it;

	// 读入文件
	if(!m_pVar->load(g_world.get_virtual_filesys(), szFileName,"id", &ListField))
	{
		return FALSE;
	}

	// 按顺序读取
	TCHAR szField[SHORT_STRING];
	for(INT32 i = nIndexStart; i <= nIndexEnd; ++i)
	{
		_stprintf(szField, _T("%d"), i);

		// 检查指定Field是否存在，如果不存在，说明读取的文件有问题
		for(it = ListField.begin(); it != ListField.end(); ++it)
		{
			if(szField == *it)
			{
				break;
			}
		}

		// 如果此处断住，说明读取的文件可能有问题
		// ASSERT(it != ListField.end());

		// 初始化
		m_pFun(arrayRes, szField, i);
	}

	// 清空容器
	m_pVar->clear();

	return TRUE;
}

//*****************************************************************************************

//-----------------------------------------------------------------------------
// 读取所有资源文件
//-----------------------------------------------------------------------------
BOOL AttRes::Init()
{
	m_pVar = new file_container;
	
	m_pFun = NULL;
	m_nMemoryCount = 0;
	// 从脚步管理器中获得不同系统中相关字段长度
	InitVarLen();
	
	// 加载过滤词表
	InitFilterWords(m_vectNameFilter, FileName_NameFilter);
	//InitFilterWords(m_vectChatFilter, FileName_ChatFilter);
	
	// 加载人物升级静态数据
	//m_LevelUpEffect[0].n_exp_level_up_ = 0; // 重新初始化该值
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitRoleAttLevelUp));
	LoadResMap(m_LevelUpEffect, FileName_RoleAttLevelUp);
	//LoadResArray(m_LevelUpEffect, 1, MAX_ROLE_LEVEL, FileName_RoleAttLevelUp);
	
	// 加载一级属性对二级属性转变表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitRoleAttChange));
	LoadResMap(m_RoleAttChange, FileName_RoleAttChange);
	// 属性默认值及最小最大值静态数据
	for(INT n = 0; n < ERA_End; n++)
	{
		m_AttDefMinMax[n].nDefRole		=	0;
		m_AttDefMinMax[n].nDefCreature	=	0;
		m_AttDefMinMax[n].nMax			=	INT_MAX;
		m_AttDefMinMax[n].nMin			=	INT_MIN;
	}

	// 初始化可铭纹装备部位
	for(INT n = 0; n < EPosyAtt_End; ++n)
	{
		for(INT m = 0; m < MAX_CONSOLIDATE_POS_QUANTITY; ++m)
		{
			m_PosyPos[n][m].byConsolidate = false;
			m_PosyPos[n][m].ePos = EEP_Start;
		}
	}

	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitAttDefMinMax));
	LoadResArray(m_AttDefMinMax, ERA_Physique, ERA_End - 1, FileName_AttDefMinMax);


	// 加载帮派设施升级需求信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildUpgradeProto));
	LoadResMap(m_GuildUpgradeNeedInfo, FileName_GuildUpgrade);

	// 加载帮派设施位置信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildGradePosProto));
	LoadResMap(m_GuildGradePosInfo, FileName_GuildGradePos);

	// 加载帮会练兵场士兵位置
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildEnlisteePos));
	LoadResMap(m_GuildEnlisteePos, FildName_GuildEnlisteePos);

	// 加载pvp旗帜位置
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildPvPBannerPos));
	LoadResMap(m_GuildPvPBannerPos, FileName_GuildBannerPos);

	// 加载帮派事务信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildAffairProto));
	LoadResMap(m_GuildAffairInfo, FileName_GuildAffair);

	// 加载帮派技能信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildSkillProto));
	LoadResMap(m_GuildSkillInfo, FileName_GuildSkill);

	// 加载帮派跑商信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGuildCommerceProto));
	LoadResMap(m_GuildCommerceInfo, FileName_GuildCommerce);

	// 商货信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCommodityProto));
	LoadResMap(m_GuildCommodityProto, FileName_GuildCommodity);

	// 商会信息
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCofCProto));
	LoadResMap(m_CofCProto, FileName_GuildCofC);

	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCofCSPProto));
	LoadResMap(m_CofCSPProto, FileName_GuildCofC_SP);
	
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOngGuildMaterialProto));
	LoadResMap(m_GuildMaterialReceive, FileName_Guild_Material_Receive);


	// 加载物品
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneItemProto));
	LoadResMap(m_mapItemProto, FileName_Item);

	// 加载装备
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneEquipProto));
	LoadResMap(m_mapEquipProto, FileName_Equip);

	// 加载宝石等
	//SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneGemProto));
	//LoadResMap(m_mapGemProto, FileName_Gem);

	
	// 加载套装等
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSuitProto));
	LoadResMap(m_mapSuitProto, FileName_Suit);

	// 加载装备各品级鉴定几率
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneEquipQltyPct));
	LoadResMap(m_mapEquipQltyPct, FileName_EquipQltyPct);

	// 加载品级装备属性参数
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitEquipQltyEffect));
	LoadResArray(m_EquipQltyEffect, 0, X_EQUIP_QUALITY_NUM - 1, FileName_EquipQltyEffect);
	
	//初始化装备等级属性加成
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneEquipLevelPctProto));
	LoadResMap(m_mapEquipLevelPctProto, FileName_EquipLevelPct_Proto);

	// 加载时装生成相关参数
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitFashionQltyEffect));
	LoadResArray(m_FashionGen, 0, X_EQUIP_QUALITY_NUM - 1, FileName_FashionQltyEffect);

	// 加载时装生成时颜色概率
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitFashionColorPct));
	LoadResArray(m_FashionColorPct, 0, X_COLOR_NUM - 1, FileName_FashionColorPct);

	// 加载商店
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneShopProto));
	LoadResMap(m_mapShopProto, FileName_Shop, FileName_ShopRare);

	//gx modify 2013.6.13
	// 加载商城
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallItemProto));
	LoadResMap(m_mapMallItemProto, FileName_MallItem);

	/*SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallPackProto));
	LoadResMap(m_mapMallPackProto, FileName_MallPack);*/
	//end
	/*SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallFreeItemProto));
	LoadResArray(&m_MallFreeItemProto, 1, 1, FileName_MallFreeItem);*/

	// 加载VIP摊位
	/*SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneVIPStallProto));
	LoadResArray(m_nVIPStallRent, 0, VIP_STALL_MAX_NUM-1, FileName_VIPStall);*/

	// 加载驿站
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneDakProto));
	LoadResMap(m_mapDakProto, FileName_Dak);
	
	// 初始化技能
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSkillProto));
	LoadResMap(m_mapSkillProto, FileName_skill);

    // 加载状态
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneBuffProto));
	LoadResMap(m_mapBuffProto, FileName_buff);
	
	// 加载技能学习
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneLearnSKillProto));
	LoadResMap(m_mapLearnSkillListProto, FileName_LearnSkillList_Proto);

	// 加载触发器
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneTriggerProto));
	LoadResMap(m_mapTriggerProto, FileName_trigger);

	// 计算技能、状态、触发器之间影响
    LoadModifyMap();

	// 加载生物
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCreatureProto));
	LoadResMap(m_mapCreatureProto, FileName_creature);

	// 加载生物AI
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCreatureAIProto));
	LoadResMap(m_mapCreatureAI, FileName_creature_ai);

	// 加载生物阵营关系
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneCreatureCampProto));
	LoadResMap(m_mapCreatureCamp, FileName_creature_camp);

	// 加载镶嵌,烙印,龙附静态表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneConsolidateProto));
	LoadResMap(m_mapConsolidateProto, FileName_Consolidate_Proto);

	// 加载生活技能静态表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneProduceProto));
	LoadResMap(m_mapProduceProto, FileName_Produce_Proto);

	// 点化,装备分解
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneDeComposeProto));
	LoadResMap(m_mapDeComposeProto, FileName_DeCompose_Proto);
	
	// 装备升级经验
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneEquipLevelProto));
	LoadResArray(m_EquipLevelUpEffect, 1, MAX_WEAPON_LEVEL, FileName_EquipLevel_Proto);
	
	// 装备变化表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneEquipChangeProto));
	LoadResMap(m_mapEquipChangeProto, FileName_EquipChange_Proto);

	// 称号
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneTitleProto));
	//LoadResArray(m_TitleProto, 0, MAX_TITLE_NUM - 1, FileName_Title_Proto);
	LoadResMap(m_TitleProto, FileName_Title_Proto);

	// 成就
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneAchievementProto));
	LoadResMap(m_mapAchievementProto, FileName_Achievement);
	
	// 成就条件
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneAchievementCriteriaProto));
	LoadResMap(m_mapAchievementCriteriaProto, FileName_Achievement_Criteria);

	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitVipProto));
	LoadResMap(m_mapVipProto, FileName_VIP);

	LoadAchievementReferenceList();
	LoadAchievementCriteriaList();

	// 怪物AI进行分组
	GroupCreatureAI();

	// 加载副本中不能使用的物品
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneInsItemProto));
	LoadResMap(m_mapInstanceItem, FileName_Instance_Item);

	// 加载副本中不能使用的技能
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneInsSkillProto));
	LoadResMap(m_mapInstanceSkill, FileName_Instance_Skill);

	// 副本随机刷怪点
	//SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSpawnPointProto));
	//LoadResMap(m_mapSpawnPointProto, FileName_Spawn_Point);

	// 副本随机刷怪点等级映射表
	//SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneLevelMapping));
	//LoadResMap(m_mapLevelMapping, FileName_Level_Mapping);

	// 初始化副本静态数据
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneInstanceProto));
	LoadResMap(m_mapInstanceProto, FileName_Instance_Proto);

	// 初始化非副本刷怪点静态数据
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSSpawnPointProto));
	LoadResMap(m_mapSSpawnPoint, FileName_SingleSpawnPoint_Proto);

	// 初始化刷怪点组
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSSpawnGroupProto));
	LoadResMap(m_mapSSpawnGroup, FileName_Spawn_Group);
	
	// 初始化帮会矿点数据
	//SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneSSpawnGuildGroupProto));
	//LoadResMap(m_mapGuildSSpawnPoint, FileName_Spawn_Guild_Group);

	// 初始化宠物技能原型
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetSkillProto));
	LoadResMap(m_mapPetSkillProto, FileName_PetSkill_Proto);  

	// 初始化宠物等级对应固定技能id
	InitPetSkillsVec();	

	// 初始化宠物原型
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetProto));
	LoadResMap(m_mapPetProto, FileName_Pet_Proto);  
	
	// 初始化宠物随机技能表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetRandomSkill));
	LoadResMap(m_mapPetSkillListProto, FileName_PetRandomSKill);

	// 初始化宠物原型
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetLvlUpProto));
	LoadResMap(m_mapPetLvlUpProto, FileName_PetLvlUpProto);  

	// 初始化宠物原型
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetLvlUpItemProto));
	LoadResMap(m_mapPetLvlUpItemProto, FileName_PetLvlUpItemProto);  

	// 初始化宠物采集表
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOnePetGatherProto));
	LoadResMap(m_mapPetGatherProto, FileName_PetGather_Proto);  
	
	// 加在公式参数
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneFormulaParamProto));
	LoadResArray(&m_FormulaParam, 0, 0, FileName_Formula_Proto);

	// 加载天气
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneWeatherProto));
	LoadResMap(m_mapWeather, FileName_Weather_Proto);  

	// 初始化新手礼包
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneNewRoleGiftProto));
	LoadResMap(m_mapNewRoleGiftProto, FileName_NewRoleGift_Proto);
	
	// 加载怪物掉落
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitCreatureLootProto));
	LoadResMap(m_mapCreatureLoot, FileName_CreatureLoot_Proto);

	// 加载集合掉落
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitItemSetLootProto));
	LoadResMap(m_mapLootItemSet, FileName_LootItemSet_Proto);

	// 加载任务掉落
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitQuestItemLootProto));
	LoadResMap(m_mapLootQuestItem, FileName_LootQuestItem_Proto);


	// 加载钓鱼
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitSceneFishingProto));
	LoadResMap(m_mapNpcFishProto, FileName_Scene_Fishing);
	
	// 加载验证码
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitVerificationProto));
	LoadResMap(m_mapVerificationCode, FileName_YanZhenMa);
	
	// 加载彩票机
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitLotteryProto));
	LoadResMap(m_mapLotteryProto, FileName_Lottery);
	

	//加载签到
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitSignProto));
	LoadResMap(m_mapSign, FileName_Sign);
	
	//加载魂精
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitHuenJingProto));
	LoadResMap(m_maphuenjing, FileName_Huenjing);
	
	//加载神级
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitGodLevelProto));
	LoadResMap(m_mapgodlevel, FileName_GodLevel);

	//加载坐骑
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitRaidProto));
	LoadResMap(m_mapRaid, FileName_Raid);

// 	// 初始化宠物属性默认最大最小值
// 	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitPetAttDefMinMax));
// 	LoadResArray(m_nPetAttDefMinMax, EPA_Begin, EPA_End - 1, FileName_PetDefMinMax_Proto);

	SAFE_DELETE(m_pVar);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 释放所有资源空间
//-----------------------------------------------------------------------------
VOID AttRes::destory()
{
	FreeResMap(m_LevelUpEffect);
	FreeResMap(m_RoleAttChange);
	FreeResMap(m_mapItemProto);
	FreeResMap(m_mapEquipProto);
	//FreeResMap(m_mapGemProto);
	FreeResMap(m_mapEquipQltyPct);
	FreeResMap(m_mapEquipLevelPctProto);
	FreeResMap(m_mapNewRoleGiftProto);

	FreeResMap(m_mapShopProto);
	FreeResMap(m_mapDakProto);
	FreeResMap(m_mapSuitProto);

	FreeResMap(m_mapMallItemProto);
	FreeResMap(m_mapMallPackProto);

    FreeResMap(m_mapSkillProto);
	FreeResMap(m_mapTriggerProto);
	FreeResMap(m_mapBuffProto);
	FreeResMap(m_mapSkillModify);
	FreeResMap(m_mapLearnSkillListProto);

	FreeResMap(m_mapCreatureProto);
	FreeResMap(m_mapCreatureCamp);
	FreeResMap(m_mapCreatureAI);
	FreeResMap(m_mapCreatureAIGroup);
	//FreeResMap(m_mapPosyProto);
	//FreeResMap(m_mapEngraveProto);
	FreeResMap(m_mapConsolidateProto);
	FreeResMap(m_mapProduceProto);
	//FreeResMap(m_mapQuenchProto);
	FreeResMap(m_mapDeComposeProto);
	FreeResMap(m_TitleProto);
	FreeResMap(m_mapAchievementProto);
	FreeResMap(m_mapAchievementCriteriaProto);
	FreeResMap(m_mapPetLvlUpProto);
	FreeResMap(m_mapPetLvlUpItemProto);
	FreeResMap(m_mapPetSkillListProto);
	FreeResMap(m_mapNpcFishProto);

	FreeResMap(m_mapSpawnPointProto);
	FreeResMap(m_mapInstanceProto);
	FreeResMap(m_mapInstanceItem);
	FreeResMap(m_mapInstanceSkill);
	//FreeResMap(m_mapLevelMapping);
	FreeResMap(m_mapSSpawnPoint);
	FreeResMap(m_mapSSpawnGroup);

	FreeResMap(m_mapPetProto);
	FreeResMap(m_mapPetSkillProto);
	FreeResMap(m_mapPetEquipProto);
	FreeResMap(m_mapPetGatherProto);

	FreeResMap(m_GuildUpgradeNeedInfo);
	FreeResMap(m_GuildAffairInfo);
	FreeResMap(m_GuildSkillInfo);
	FreeResMap(m_GuildGradePosInfo);
	FreeResMap(m_GuildEnlisteePos);
	FreeResMap(m_GuildPvPBannerPos);

	FreeResMap(m_mapCreatureLoot);
	FreeResMap(m_mapLootItemSet);
	FreeResMap(m_mapLootQuestItem);
	FreeResMap(m_mapVerificationCode);

	FreeResMap(m_mapSign);
	FreeResMap(m_maphuenjing);
	FreeResMap(m_mapgodlevel);
	FreeResMap(m_mapRaid);

	FreeResMap(m_mapWeather);
	FreeResMap(m_mapVipProto);
	FreeResMap(m_mapLotteryProto);//gx add 2013.6.26
	FreeResMap(m_GuildMaterialReceive);
	FreeResMap(m_mapEquipChangeProto);
	FreeResMap(m_mapGuildSSpawnPoint);
}

//-------------------------------------------------------------------------------------------
// 初始化过滤词表
//-------------------------------------------------------------------------------------------
BOOL AttRes::InitFilterWords(OUT std::vector<tstring>& vectFilterWords, LPCTSTR szFileName)
{
	vectFilterWords.clear();

	std::list<tstring>				ListField;
	std::list<tstring>::iterator	it;

	// 读入文件
	if(!m_pVar->load(g_world.get_virtual_filesys(), szFileName,"id", &ListField))
	{
		m_att_res_caution(szFileName, _T("load"), INVALID_VALUE);
		return FALSE;
	}

	// 按顺序读取
	for(it = ListField.begin(); it != ListField.end(); ++it)
	{
		vectFilterWords.push_back(m_pVar->get_string(_T("Name"), it->c_str()));
	}

	// 清空容器
	m_pVar->clear();

	return TRUE;
}

//-------------------------------------------------------------------------------------------
// 从脚本管理器中获取变量长度
//-------------------------------------------------------------------------------------------
VOID AttRes::InitVarLen()
{
	// 角色名字
	g_ScriptMgr.GetGlobal("len_RoleNameMax", m_VarLen.nRoleNameMax);
	g_ScriptMgr.GetGlobal("len_RoleNameMin", m_VarLen.nRoleNameMin);
	
	// 帮派相关
	g_ScriptMgr.GetGlobal("len_GuildNameMax", m_VarLen.nGuildNameMax);
	g_ScriptMgr.GetGlobal("len_GuildNameMin", m_VarLen.nGuildNameMin);
	g_ScriptMgr.GetGlobal("len_GuildTenet", m_VarLen.nGuildTenet);
	g_ScriptMgr.GetGlobal("len_GuildSymbol", m_VarLen.nGuildSymbol);

	// 摆摊
	g_ScriptMgr.GetGlobal("len_StallTitleMax", m_VarLen.nStallTitleMax);

	//PK
	g_ScriptMgr.GetGlobal("PK_Kill_Write", m_VarLen.nPKKillWrite);
	g_ScriptMgr.GetGlobal("PK_Kill_Red", m_VarLen.nPKKillRed);
	g_ScriptMgr.GetGlobal("PK_Value_Time_Decrease", m_VarLen.nPKValueDec);

	// 组队杀怪提升的义气
	g_ScriptMgr.GetGlobal("team_brother", m_VarLen.nBrother);

	// 邮费
	g_ScriptMgr.GetGlobal("MailMoney", m_VarLen.nMailMoney);
	
	// 公告最大长度
	g_ScriptMgr.GetGlobal("len_TeamPlacardMax", m_VarLen.nTeamPlacardMax);
	
	// 能量更新
	g_ScriptMgr.GetGlobal("Power_rage_loss", m_VarLen.nRageLoss);
	g_ScriptMgr.GetGlobal("Power_Point_loss", m_VarLen.nPointLoss);
	g_ScriptMgr.GetGlobal("Power_Energy_Combot", m_VarLen.nEnergyCom);
	g_ScriptMgr.GetGlobal("Power_Energy", m_VarLen.nEnergy);
	g_ScriptMgr.GetGlobal("Power_Focus_loss", m_VarLen.nFocusLoss);

	// 拍卖税
	g_ScriptMgr.GetGlobal("paimai_duty", m_VarLen.n_paimai_duty);

	g_ScriptMgr.GetGlobal("pet_return_min_time", m_VarLen.n_pet_return_min_time);
	g_ScriptMgr.GetGlobal("pet_return_max_time", m_VarLen.n_pet_return_max_time);

	g_ScriptMgr.GetGlobal("n_fishing_vigour_cost_second", m_VarLen.n_fishing_vigour_cost_second);

	//钱庄汇率
	g_ScriptMgr.GetGlobal("bank_radio", m_VarLen.n_bank_radio);

	g_ScriptMgr.GetGlobal("verifcation", m_VarLen.n_verification);
	g_ScriptMgr.GetGlobal("min_verificationtime", m_VarLen.n_min_verification_time);
	g_ScriptMgr.GetGlobal("max_verificationtime", m_VarLen.n_max_verification_time);

	g_ScriptMgr.GetGlobal("LeaveTimeReward", m_VarLen.n_LeaveTimeReward);
	
	m_VarLen.n_public_cd_time[0] = 1500;
	g_ScriptMgr.GetGlobal("public_cd_time_Warrior", m_VarLen.n_public_cd_time[EV_Warrior]);
	g_ScriptMgr.GetGlobal("public_cd_time_Mage", m_VarLen.n_public_cd_time[EV_Mage]);
	g_ScriptMgr.GetGlobal("public_cd_time_Hunter", m_VarLen.n_public_cd_time[EV_Taoist]);
	//g_ScriptMgr.GetGlobal("public_cd_time_Assassin", m_VarLen.n_public_cd_time[EV_Blader]);
	//g_ScriptMgr.GetGlobal("public_cd_time_Astrologer", m_VarLen.n_public_cd_time[EV_Astrologer]);

	g_ScriptMgr.GetGlobal("dancing_multiple_start_time", m_VarLen.dancing_multiple_start_time);
	g_ScriptMgr.GetGlobal("dancing_multiple_end_time", m_VarLen.dancing_multiple_end_time);
	g_ScriptMgr.GetGlobal("dancing_factor", m_VarLen.dancing_factor);
	g_ScriptMgr.GetGlobal("pet_ernie_level", m_VarLen.pet_ernie_level);
	g_ScriptMgr.GetGlobal("attack_stop_mount_prob", m_VarLen.attack_stop_mount_prob);
	g_ScriptMgr.GetGlobal("sign_level", m_VarLen.nSignLevel);

	m_VarLen.n_kick_fast_move = 1;

	// 简单验证设置合法性
	if(!m_VarLen.CheckValid())
	{
		ASSERT(0);
		print_message(_T("\n\n\tName length script maybe have problem!\n\n"));
	}
}

AttRes* AttRes::GetInstance()
{
	if(!VALID_POINT(m_pInstance))
	{
		m_pInstance = new AttRes;
	}

	return m_pInstance;
}

VOID AttRes::Destory()
{
	SAFE_DELETE(m_pInstance);
}

//-----------------------------------------------------------------------------
// 重新加载商城数据
//-----------------------------------------------------------------------------
BOOL AttRes::ReloadMallProto()
{
	FreeResMap(m_mapMallItemProto);
	FreeResMap(m_mapMallPackProto);
	m_MallFreeItemProto.Clear();

	m_pVar = new file_container;

	// 加载商城
	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallItemProto));
	if(!LoadResMap(m_mapMallItemProto, FileName_MallItem))
	{
		return FALSE;
	}

	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallPackProto));
	if(!LoadResMap(m_mapMallPackProto, FileName_MallPack))
	{
		return FALSE;
	}

	SetInitOneRes(fastdelegate::MakeDelegate(this, &AttRes::InitOneMallFreeItemProto));
	if(!LoadResArray(&m_MallFreeItemProto, 1, 1, FileName_MallFreeItem))
	{
		return FALSE;
	}

	SAFE_DELETE(m_pVar);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 加载人物升级静态数据
//-----------------------------------------------------------------------------
VOID* AttRes::InitRoleAttLevelUp(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex)
{
	//s_level_up_effect* pLevelUpEffect = (s_level_up_effect*)pArray;

	M_trans_pointer(pLevelUpEffect, nProtoType, s_level_up_effect);

	pLevelUpEffect->dw_level_up_id_						= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pLevelUpEffect->n_exp_level_up_						= m_pVar->get_int(_T("exp_levelup"), szField);
	pLevelUpEffect->n_role_att_[0]		= (INT16)m_pVar->get_dword(_T("wuli_min"), szField, 0);
	pLevelUpEffect->n_role_att_[1]		= (INT16)m_pVar->get_dword(_T("wuli_max"), szField, 0);
	pLevelUpEffect->n_role_att_[2]		= (INT16)m_pVar->get_dword(_T("mofa_min"), szField, 0);
	pLevelUpEffect->n_role_att_[3]		= (INT16)m_pVar->get_dword(_T("mofa_max"), szField, 0);
	pLevelUpEffect->n_role_att_[4]		= (INT16)m_pVar->get_dword(_T("daoshu_min"), szField, 0);
	pLevelUpEffect->n_role_att_[5]		= (INT16)m_pVar->get_dword(_T("daoshu_max"), szField, 0);
	pLevelUpEffect->n_role_att_[6]		= (INT16)m_pVar->get_dword(_T("wufang_min"), szField, 0);
	pLevelUpEffect->n_role_att_[7]		= (INT16)m_pVar->get_dword(_T("wufang_max"), szField, 0);
	pLevelUpEffect->n_role_att_[8]		= (INT16)m_pVar->get_dword(_T("mofang_min"), szField, 0);
	pLevelUpEffect->n_role_att_[9]		= (INT16)m_pVar->get_dword(_T("mofang_max"), szField, 0);

	//pLevelUpEffect->n16RoleAttAvail				= (INT16)m_pVar->get_dword(_T("RoleAttAvail_e"), szField, 0);
	pLevelUpEffect->n_talent_avail_				= (INT16)m_pVar->get_dword(_T("talentAvail_e"), szField, 0);
	pLevelUpEffect->n_hp_						= m_pVar->get_dword(_T("hp"), szField);
	pLevelUpEffect->n_mp_						= m_pVar->get_dword(_T("mp"), szField);
	pLevelUpEffect->n_rating				= m_pVar->get_int(_T("rating"), szField, 0);
	
	return &pLevelUpEffect->dw_level_up_id_;
}

//-------------------------------------------------------------------------------------------
// 加载人物一二级属性转换表
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitRoleAttChange(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex)
{
	M_trans_pointer(pRoleAttChange, nProtoType, s_role_att_change);

	pRoleAttChange->dw_id_							= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pRoleAttChange->n_att_change_[erac_attack]		= (INT16)m_pVar->get_dword(_T("attack"), szField, 0);
	pRoleAttChange->n_att_change_[earc_defense]		= (INT16)m_pVar->get_dword(_T("defense"), szField, 0);
	pRoleAttChange->n_att_change_[earc_maxhp]		= (INT16)m_pVar->get_dword(_T("hp"), szField, 0);
	pRoleAttChange->n_att_change_[earc_hit]			= (INT16)m_pVar->get_dword(_T("hit"), szField, 0);
	pRoleAttChange->n_att_change_[earc_dodge]		= (INT16)m_pVar->get_dword(_T("dodge"), szField, 0);
	pRoleAttChange->n_att_change_[earc_block]		= (INT16)m_pVar->get_dword(_T("block"), szField, 0);
	pRoleAttChange->n_att_change_[earc_crit]			= (INT16)m_pVar->get_dword(_T("crit"), szField, 0);
	pRoleAttChange->n_att_change_[earc_critvalue]	= (INT16)m_pVar->get_dword(_T("critValue"), szField, 0);
	pRoleAttChange->n_att_change_[earc_fancrit]		= (INT16)m_pVar->get_dword(_T("fanCrit"), szField, 0);
	pRoleAttChange->n_att_change_[earc_fancritvalue]	= (INT16)m_pVar->get_dword(_T("fanCritValue"), szField, 0);
	pRoleAttChange->n_att_change_[earc_maxmp]		= (INT16)m_pVar->get_dword(_T("mp"), szField, 0);
	pRoleAttChange->n_att_change_[earc_hp_reborn]	= (INT16)m_pVar->get_dword(_T("hpReborn"), szField, 0);
	pRoleAttChange->n_att_change_[earc_mp_reborn]	= (INT16)m_pVar->get_dword(_T("mpReborn"), szField, 0);
	
	return &pRoleAttChange->dw_id_;
}

VOID* AttRes::InitOneEquipLevelProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagEquipLevelUpEffect* pLevelUpEffect = (tagEquipLevelUpEffect*)pArray;

	pLevelUpEffect[nIndex].nExpLevelUp		= m_pVar->get_int(_T("ExpLevelUp"), szField, 1);
	pLevelUpEffect[nIndex].nExpLevelUpShipin= m_pVar->get_int(_T("ExpLevelUpShipin"), szField, 1);
	pLevelUpEffect[nIndex].n16TalentAvail	= m_pVar->get_int(_T("TalentAvail"), szField, 0);

	return NULL;
}

//-------------------------------------------------------------------------------------
// 加载属性默认值、最小及最大值
//-------------------------------------------------------------------------------------
VOID* AttRes::InitAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagAttDefMinMax* pAttDefMinMax = (tagAttDefMinMax*)pArray;

	pAttDefMinMax[nIndex].nDefRole			=	m_pVar->get_int(_T("DefaultRole"), szField, 0);
	pAttDefMinMax[nIndex].nDefCreature		=	m_pVar->get_int(_T("DefaultCreature"), szField, 0);
	pAttDefMinMax[nIndex].nMin				=	m_pVar->get_int(_T("Min"), szField, INT_MIN);
	pAttDefMinMax[nIndex].nMax				=	m_pVar->get_int(_T("Max"), szField, INT_MAX);

	return NULL;
}


//-------------------------------------------------------------------------------------
// 初始化物品的一条记录
//-------------------------------------------------------------------------------------
VOID* AttRes::InitOneItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pItem, pProtoType, tagItemProto);

	// 初始化结构信息
	pItem->dw_data_id				=	(DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pItem->eType					=	(EItemType)m_pVar->get_int(_T("type"), szField, EIT_Null);
	pItem->eTypeEx					=	(EItemTypeEx)m_pVar->get_int(_T("type_ex"), szField, EITE_Null);
	pItem->eTypeReserved			=	(EItemTypeReserved)m_pVar->get_int(_T("type_reserved"), szField, EITR_Null);

	pItem->eStuffType				=	(EStuffType)m_pVar->get_int(_T("stuff_type"), szField, EST_Null);

	pItem->eSpecFunc				=	(EItemSpecFunc)m_pVar->get_int(_T("special_function"), szField, EISF_Null);
	pItem->nSpecFuncVal1			=	m_pVar->get_int(_T("special_function_value_1"), szField, 0);
	pItem->nSpecFuncVal2			=	m_pVar->get_int(_T("special_function_value_2"), szField, 0);

	pItem->byLevel					=	(BYTE)m_pVar->get_dword(_T("level"), szField, 1);
	pItem->byQuality				=	(BYTE)m_pVar->get_dword(_T("quality"), szField, EIQ_Quality0);
	pItem->byBindType				=	(BYTE)m_pVar->get_dword(_T("bind_type"), szField, EBM_Null);
	pItem->dwQuestID				=	m_pVar->get_dword(_T("quest_id"), szField, INVALID_VALUE);
	pItem->nBasePrice				=	m_pVar->get_int(_T("base_price"), szField, 0);
	pItem->nGemNess					=	m_pVar->get_int(_T("gemness"), szField, 1);
	pItem->nMaxUseTimes				=	m_pVar->get_int(_T("use_times"), szField, 1);
	pItem->n16MaxLapNum				=	(INT16)m_pVar->get_dword(_T("stack_number"), szField, 1);
	pItem->n16MaxHoldNum			=	(INT16)m_pVar->get_dword(_T("hold_number"), szField, INVALID_VALUE);

	pItem->dwTimeLimit				=	m_pVar->get_dword(_T("time_limit"), szField, INVALID_VALUE);

	pItem->n16Enmity				=	(INT16)m_pVar->get_dword(_T("enmity_value"), szField, 0);
	pItem->bNeedBroadcast			=	((BOOL)m_pVar->get_dword(_T("need_broadcase"), szField, FALSE) ? true : false);
	pItem->bCanSell					=	((BOOL)m_pVar->get_dword(_T("can_sell"), szField, TRUE) ? true : false);

	pItem->byMinUseLevel			=	(BYTE)m_pVar->get_dword(_T("use_level_min"), szField, 1);
	pItem->byMaxUseLevel			=	(BYTE)m_pVar->get_dword(_T("use_level_max"), szField, 255);
	pItem->eSexLimit				=	(ESexLimit)m_pVar->get_int(_T("sex_limit"), szField, ESL_Woman);

	pItem->dwVocationLimit			=	m_pVar->get_dword(_T("vocation_limit"), szField, INVALID_VALUE);

	//pItem->eClanRepute				=	(EReputationType)m_pVar->get_int(_T("zone_rep_type"), szField, ERT_NULL);
	//pItem->nClanReputeVal			=	m_pVar->get_int(_T("zone_rep_value"), szField, 0);
	//pItem->eOtherClanRepute			=	(EReputationType)m_pVar->get_int(_T("other_rep_type"), szField, ERT_NULL);
	//pItem->nOtherClanReputeVal		=	m_pVar->get_int(_T("other_rep_value"), szField, 0);

	pItem->eOPType					=	(ESkillOPType)m_pVar->get_int(_T("operation_type"), szField, ESOPT_NUll);
	pItem->fOPDist					=	m_pVar->get_float(_T("operation_distance"), szField, 0);
	pItem->fOPRadius				=	m_pVar->get_float(_T("operation_raduis"), szField, 0);
	pItem->bMoveable				=	(BOOL)m_pVar->get_dword(_T("moveable"), szField, FALSE);
	pItem->nInterruptSkillOrdRate	=	m_pVar->get_int(_T("interrupt_rate"), szField, 0);
	pItem->nPrepareTime				=	m_pVar->get_int(_T("prepare_time"), szField, 0);
	pItem->dwTargetLimit			=	m_pVar->get_dword(_T("target_limit"), szField, INVALID_VALUE);
	pItem->dwSpecBuffLimitID		=	m_pVar->get_dword(_T("buff_limit_id"), szField, INVALID_VALUE);
	pItem->dwStateLimit				=	m_pVar->get_dword(_T("state_limit"), szField, INVALID_VALUE);

	pItem->bInterruptMove			=	(BOOL)m_pVar->get_dword(_T("interrupt_move"), szField, FALSE) ? true : false;
	//pItem->bFriendly				=	(BOOL)m_pVar->get_dword(_T("Friendly"), szField, FALSE) ? true : false;
	//pItem->bHostile					=	(BOOL)m_pVar->get_dword(_T("Hostile"), szField, FALSE) ? true : false;
	//pItem->bIndependent				=	(BOOL)m_pVar->get_dword(_T("Independent"), szField, FALSE) ? true : false;

	pItem->dwCooldownTime			=	m_pVar->get_dword(_T("cd_time"), szField, 0);

	pItem->dwTriggerID0				=	m_pVar->get_dword(_T("trigger_id_0"), szField, INVALID_VALUE);
	pItem->dwBuffID0				=	m_pVar->get_dword(_T("buff_id_0"), szField, INVALID_VALUE);
	
	pItem->dwTriggerID1				=	m_pVar->get_dword(_T("trigger_id_1"), szField, INVALID_VALUE);
	pItem->dwBuffID1				=	m_pVar->get_dword(_T("buff_id_1"), szField, INVALID_VALUE);

	pItem->dwTransTriggerID			=	m_pVar->get_dword(_T("trans_trigger_id"), szField, INVALID_VALUE);
	pItem->dwTransID				=	m_pVar->get_dword(_T("trans_id"), szField, INVALID_VALUE);

	pItem->bDeadLoot				=	(BOOL)m_pVar->get_dword(_T("dead_loot"), szField, TRUE);

	pItem->n_keeping				=   m_pVar->get_int(_T("keeping"), szField, 0);

	pItem->bCantDrop				=	(BOOL)m_pVar->get_dword(_T("cant_drop"), szField, FALSE);

	// 添加新属性
	return &pItem->dw_data_id;
}

//-----------------------------------------------------------------------------
// 初始化装备的一条记录
//-----------------------------------------------------------------------------
VOID* AttRes::InitOneEquipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	// 先初始化和物品静态属性相同的部分
	InitOneItemProto(pProtoType, szField, nDummy);

	// 初始化装备静态属性特有部分
	M_trans_pointer(pEquip, pProtoType, tagEquipProto);

	ASSERT(pEquip->dw_data_id >= MIN_EQUIP_ID);
	
	pEquip->eTalentType				=	(ETalentType)m_pVar->get_int(_T("talent_type"), szField, ETT_Null);

	pEquip->dwSuitID[0]				=	m_pVar->get_dword(_T("suit_id_1"), szField, INVALID_VALUE);
	pEquip->dwSuitID[1]				=	m_pVar->get_dword(_T("suit_id_2"), szField, INVALID_VALUE);
	pEquip->dwSuitID[2]				=	m_pVar->get_dword(_T("suit_id_3"), szField, INVALID_VALUE);

	pEquip->bySuitMinQlty[0]		=	(BYTE)m_pVar->get_dword(_T("suit_min_quality_1"), szField, EIQ_Quality0);
	pEquip->bySuitMinQlty[1]		=	(BYTE)m_pVar->get_dword(_T("suit_min_quality_2"), szField, EIQ_Quality0);
	pEquip->bySuitMinQlty[2]		=	(BYTE)m_pVar->get_dword(_T("suit_min_quality_3"), szField, EIQ_Quality0);

	//pEquip->n16AttALimit[0]			=	(INT16)m_pVar->get_int(_T("PhysiqueLim"), szField, 0);
	//pEquip->n16AttALimit[1]			=	(INT16)m_pVar->get_int(_T("StrengthLim"), szField, 0);
	//pEquip->n16AttALimit[2]			=	(INT16)m_pVar->get_int(_T("PneumaLim"), szField, 0);
	//pEquip->n16AttALimit[3]			=	(INT16)m_pVar->get_int(_T("InnerForceLim"), szField, 0);
	//pEquip->n16AttALimit[4]			=	(INT16)m_pVar->get_int(_T("TechniqueLim"), szField, 0);
	//pEquip->n16AttALimit[5]			=	(INT16)m_pVar->get_int(_T("anility_limit"), szField, 0);

    pEquip->eEquipPos				=	(EEquipPos)m_pVar->get_int(_T("equip_position"), szField);
   // pEquip->eWeaponPos				=	(EWeaponPos)m_pVar->get_int(_T("WeaponPos"), szField, EWP_NULL);

	pEquip->n16MinDmg				=	(INT16)m_pVar->get_dword(_T("min_demage"), szField, 0);
	pEquip->n16MaxDmg				=	(INT16)m_pVar->get_dword(_T("max_demage"), szField, 0);
	//pEquip->n16MinDmgIn				=	(INT16)m_pVar->get_dword(_T("MinDmgIn"), szField, 0);
	//pEquip->n16MaxDmgIn				=	(INT16)m_pVar->get_dword(_T("MaxDmgIn"), szField, 0);
	pEquip->n16Armor				=	(INT16)m_pVar->get_dword(_T("armor"), szField, 0);
	//pEquip->n16ArmorIn				=	(INT16)m_pVar->get_dword(_T("ArmorIn"), szField, 0);

	//pEquip->nPotVal					=	m_pVar->get_int(_T("Potval"), szField, 0);
	//pEquip->nMaxPotVal				=	m_pVar->get_int(_T("MaxPotval"), szField, 0);
	
	pEquip->n16Newness				=	(INT16)m_pVar->get_dword(_T("max_newness"), szField, 0);
	pEquip->dwVocationLimitWear		=	m_pVar->get_int(_T("vocation_limit_wear"), szField, 63);
	pEquip->dwPkValueLimit			=	m_pVar->get_int(_T("pk_value_limit"), szField, INVALID_VALUE);

	//pEquip->n16HolePrice				=   (INT16)m_pVar->get_dword(_T("HolePrice"), szField, 0);

	//pEquip->n16HolePriceDiff			=	(INT16)m_pVar->get_dword(_T("HolePriceDiff"), szField, 0);

	TCHAR szTmp[8];

	// 加工前，影响的属性(和品级无关)
	for(INT32 i=0; i<MAX_ROLEATT_BASE_EFFECT; ++i)
	{
		_stprintf(szTmp, _T("%d"), i);
		pEquip->BaseEffect[i].eRoleAtt	= (ERoleAttribute)m_pVar->get_int(_T("role_att_type_"), szTmp, szField, ERA_Null);
		if(ERA_Null == pEquip->BaseEffect[i].eRoleAtt)
		{
			break;
		}

		pEquip->BaseEffect[i].nValue	=	m_pVar->get_int(_T("role_att_value_"), szTmp, szField, 0);
	}
	
	for(INT32 i=0; i<MAX_BASE_ATT; ++i)
	{
		_stprintf(szTmp, _T("%d"), i);
		pEquip->RandEffect[i].eRoleAtt	= (ERoleAttribute)m_pVar->get_int(_T("role_rand_att_type_"), szTmp, szField, ERA_Null);
		if(ERA_Null == pEquip->RandEffect[i].eRoleAtt)
		{
			break;
		}

		pEquip->RandEffect[i].nValue	=	m_pVar->get_int(_T("role_rand_att_value_"), szTmp, szField, 0);
		pEquip->dwRandEffectPro[i]		=	m_pVar->get_int(_T("role_rand_att_pro_"), szTmp, szField, 100);
	}

	pEquip->dwTriggerID2			=	m_pVar->get_dword(_T("trigger_id_2"), szField, INVALID_VALUE);
	pEquip->dwBuffID2				=	m_pVar->get_dword(_T("buff_id_2"), szField, INVALID_VALUE);
	
	for (INT32 i = 0; i < EQUIP_SKILL_NUMBER; i++)
	{
		_stprintf(szTmp, _T("%d"), i);

		pEquip->dwSkillID[i] = m_pVar->get_int(_T("skill_"), szTmp, szField, 0);

	}
	//pEquip->bCanDye					=	((BOOL)m_pVar->get_dword(_T("CanDye"), szField, FALSE) ? true : false);
	//pEquip->dwColor					=	m_pVar->get_dword(_T("Corlor"), szField, EC_Null);

	//pEquip->bIdentifyProduct		=	((BOOL)m_pVar->get_dword(_T("IdentifyProduct"), szField, FALSE) ? true : false);
	//pEquip->bIdentifyLoot			=	((BOOL)m_pVar->get_dword(_T("IdentifyLoot"), szField, FALSE) ? true : false);

	pEquip->dwPetID					=	m_pVar->get_dword(_T("pet_data_id"), szField, INVALID_VALUE );
	
	pEquip->byLuck					=	(CHAR)m_pVar->get_dword(_T("luck"), szField, 0);
	pEquip->bySuitTriggerCount		=	(BYTE)m_pVar->get_dword(_T("suit_trigger_count"), szField, 0);
	return &pEquip->dw_data_id;
}

//-----------------------------------------------------------------------------
// 初始化宝石等的一条记录
//-----------------------------------------------------------------------------
//VOID* AttRes::InitOneGemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
//{
//	M_trans_pointer(pGemProto, pProtoType, tagGemProto);
//
//	pGemProto->dw_data_id			= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
//
//	pGemProto->n16PotValNeed	= (INT16)m_pVar->get_dword(_T("potval_consume"), szField);
//	pGemProto->n16SuccessPct	= (INT16)m_pVar->get_dword(_T("success_rate"), szField);
//
//	TCHAR szTmp[8];
//
//	for(INT32 i=0; i<MAX_ROLEATT_ENHANCE_EFFECT; ++i)
//	{
//		_stprintf(szTmp, _T("%d"), i);
//		pGemProto->BaseEffect[i].eRoleAtt	= (ERoleAttribute)m_pVar->get_int(_T("role_att"), szTmp, szField, ERA_Null);
//		if(ERA_Null == pGemProto->BaseEffect[i].eRoleAtt)
//		{
//			break;
//		}
//
//		pGemProto->BaseEffect[i].nValue	=	m_pVar->get_int(_T("att_val"), szTmp, szField, 0);
//	}
//
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("right_hand"), szField, 0) == 1 ? EEEP_Hand : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("head"), szField, 0) == 1 ? EEEP_Head : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("body"), szField, 0) == 1 ? EEEP_Body : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("leg"), szField, 0) == 1 ? EEEP_Legs : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("wrist"), szField, 0) == 1 ? EEEP_Wrist : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("feet"), szField, 0) == 1 ? EEEP_Feet : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("back"), szField, 0) == 1 ? EEEP_Back : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("neck"), szField, 0) == 1 ? EEEP_Neck : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("finger1"), szField, 0) == 1 ? EEEP_Finger : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("waist"), szField, 0) == 1 ? EEEP_Waist : 0);
//	pGemProto->nEnhanceFlg		|= (m_pVar->get_int(_T("face"), szField, 0) == 1 ? EEEP_Face : 0);
//
//	return &pGemProto->dw_data_id;
//}

//-----------------------------------------------------------------------------
// 初始化套装的一条记录
//-----------------------------------------------------------------------------
VOID* AttRes::InitOneSuitProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagSuitProto);

	pProto->dwID			= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	TCHAR szTmp[8];
	for(INT32 i=0; i<MAX_SUIT_ATT_NUM; ++i)
	{
		_stprintf(szTmp, _T("%d"), i+1);
		
		pProto->dwTriggerID[i]	= m_pVar->get_dword(_T("trigger_id_"), szTmp, szField, INVALID_VALUE);
		pProto->dwBuffID[i]		= m_pVar->get_dword(_T("buff_id_"), szTmp, szField, INVALID_VALUE);
		pProto->n8ActiveNum[i]	= m_pVar->get_int(_T("active_number_"), szTmp, szField, MAX_SUIT_EQUIP_NUM);
	}

	pProto->n8SpecEffectNum	= m_pVar->get_int(_T("special_effect_number"), szField, INVALID_VALUE);
	pProto->nEffectConutPos	= m_pVar->get_int(_T("effect_count_pos"), szField, INVALID_VALUE);
	return &pProto->dwID;
}

//-----------------------------------------------------------------------------
// 初始化一类装备品级鉴定几率
//-----------------------------------------------------------------------------
VOID* AttRes::InitOneEquipQltyPct(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pEquipQltyPct, pProtoType, tagEquipQltyPct);

	pEquipQltyPct->dw_data_id						=	(DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));	// 这里干吗的？
	ASSERT(pEquipQltyPct->dw_data_id >= MIN_EQUIP_ID);

	pEquipQltyPct->nEquipQltyPct[EIQ_Quality0]		=	m_pVar->get_int(_T("Quality1"), szField);
	pEquipQltyPct->nEquipQltyPct[EIQ_Quality1]		=	m_pVar->get_int(_T("Quality2"), szField);
	pEquipQltyPct->nEquipQltyPct[EIQ_Quality2]	=	m_pVar->get_int(_T("Quality3"), szField);
	pEquipQltyPct->nEquipQltyPct[EIQ_Quality3]	=	m_pVar->get_int(_T("Quality4"), szField);
	pEquipQltyPct->nEquipQltyPct[EIQ_Quality4]	=	m_pVar->get_int(_T("Quality5"), szField);
	pEquipQltyPct->nEquipQltyPct[EIQ_Quality5]	=	m_pVar->get_int(_T("Quality6"), szField);

	return &pEquipQltyPct->dw_data_id;
}


//-----------------------------------------------------------------------------
// 加载品级装备属性参数
//-----------------------------------------------------------------------------
VOID* AttRes::InitEquipQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagEquipQltyEffect* pEquipQltyEffect = (tagEquipQltyEffect*)pArray;

	//pEquipQltyEffect[nIndex].fWeaponFactor			=	m_pVar->get_float(_T("weapon_factor"), szField, 1);
	//pEquipQltyEffect[nIndex].fArmorFactor			=	m_pVar->get_float(_T("armor_factor"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ExAttackMin]		=   m_pVar->get_int(_T("wuli_min"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ExAttackMax]		=	m_pVar->get_int(_T("wuli_max"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_InAttackMin]		=	m_pVar->get_int(_T("mofa_min"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_InAttackMax]		=	m_pVar->get_int(_T("mofa_max"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ArmorEx]			=	m_pVar->get_int(_T("daosu_min"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ArmorIn]			=	m_pVar->get_int(_T("daosu_max"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ExAttack]			=	m_pVar->get_int(_T("wufang_min"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_ExDef]				=	m_pVar->get_int(_T("wufang_max"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_InAttack]			=	m_pVar->get_int(_T("mofang_min"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_InDefense]		=   m_pVar->get_int(_T("mofang_max"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_HitRate]		=	m_pVar->get_int(_T("hit_rate"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_Dodge]			=	m_pVar->get_int(_T("dodge"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_Crit_Rate]		=	m_pVar->get_int(_T("cirt"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_UnCrit_Rate]	=	m_pVar->get_int(_T("fan_cirt"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_MaxHP]		=	m_pVar->get_int(_T("hp"), szField, 1);
	pEquipQltyEffect[nIndex].EquipAddAtt[EAA_MaxMP]		=	m_pVar->get_int(_T("mp"), szField, 1);
	pEquipQltyEffect[nIndex].nAttAFactor				=	m_pVar->get_int(_T("fudong"), szField, 0);
	//pEquipQltyEffect[nIndex].EquipAddAtt[EAA_Parry]			=	m_pVar->get_int(_T("parry"), szField, 1);

	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Potence]		=   m_pVar->get_int(_T("strength_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Agility]		=	m_pVar->get_int(_T("agility_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Brains]		=	m_pVar->get_int(_T("inner_force_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Stamina]		=	m_pVar->get_int(_T("physique_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_HP]			=	m_pVar->get_int(_T("hp_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_MP]			=	m_pVar->get_int(_T("mp_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_ExAttack]		=	m_pVar->get_int(_T("attack_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_ExDef]			=	m_pVar->get_int(_T("def_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Cirt]			=	m_pVar->get_int(_T("cirt_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_FanCirt]		=   m_pVar->get_int(_T("fan_cirt_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_HitRate]		=	m_pVar->get_int(_T("hit_rate_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Dodge]			=	m_pVar->get_int(_T("dodge_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_CirtNum]		=	m_pVar->get_int(_T("cirt_value_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_FanCirtNum]	=	m_pVar->get_int(_T("fan_cirt_value_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_HpRevert]		=	m_pVar->get_int(_T("hp_revert_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_MpRevert]		=	m_pVar->get_int(_T("mp_revert_p"), szField, 1);
	//pEquipQltyEffect[nIndex].EquipAddAttProduct[EAA_Parry]			=	m_pVar->get_int(_T("parry_p"), szField, 1);

	//pEquipQltyEffect[nIndex].nXiliAtt								=	m_pVar->get_int(_T("xili"), szField, 1);
	//pEquipQltyEffect[nIndex].nAttAFactor			=	m_pVar->get_int(_T("AttAFactor1"), szField, 0);
	//pEquipQltyEffect[nIndex].fAttAFactor			=	m_pVar->get_float(_T("AttAFactor2"), szField, 0);
	pEquipQltyEffect[nIndex].nAttANumEffect			=	m_pVar->get_int(_T("att_number"), szField, 0);

	//pEquipQltyEffect[nIndex].fPotFactor				=	m_pVar->get_float(_T("PotFactor"), szField, 1);

	pEquipQltyEffect[nIndex].nHoleNumPct[0]			=	m_pVar->get_int(_T("HolePct0"), szField, 0);
	pEquipQltyEffect[nIndex].nHoleNumPct[1]			=	m_pVar->get_int(_T("HolePct1"), szField, 0);
	pEquipQltyEffect[nIndex].nHoleNumPct[2]			=	m_pVar->get_int(_T("HolePct2"), szField, 0);
	pEquipQltyEffect[nIndex].nHoleNumPct[3]			=	m_pVar->get_int(_T("HolePct3"), szField, 0);
	pEquipQltyEffect[nIndex].nHoleNumPct[4]			=	m_pVar->get_int(_T("HolePct4"), szField, 0);
	pEquipQltyEffect[nIndex].nHoleNumPct[5]			=	m_pVar->get_int(_T("HolePct5"), szField, 0);

	//pEquipQltyEffect[nIndex].nSpecAttPct			=	m_pVar->get_int(_T("SpecAttPct"), szField, 0);
	
	return NULL;

}

//-----------------------------------------------------------------------------
// 加载时装生成参数
//-----------------------------------------------------------------------------
VOID* AttRes::InitFashionQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	M_trans_pointer(p, pArray, tagFashionGen);

	p[nIndex].fAppearanceFactor = m_pVar->get_float(_T("AppearanceFactor"), szField);
	p[nIndex].n16ReinPct		= (INT16)m_pVar->get_float(_T("ReinPct"), szField);
	p[nIndex].n16SavvyPct		= (INT16)m_pVar->get_float(_T("SavvyPct"), szField);
	p[nIndex].n16FortunePct		= (INT16)m_pVar->get_float(_T("FortunePct"), szField);
	p[nIndex].n8ReinVal			= (INT8)m_pVar->get_float(_T("ReinVal"), szField);
	p[nIndex].n8SavvyVal		= (INT8)m_pVar->get_float(_T("SavvyVal"), szField);
	p[nIndex].n8FortuneVal1		= (INT8)m_pVar->get_float(_T("FortuneVal1"), szField);
	p[nIndex].n8FortuneVal2		= (INT8)m_pVar->get_float(_T("FortuneVal2"), szField);

	return NULL;
}

//-----------------------------------------------------------------------------
// 加载时装生成时颜色概率
//-----------------------------------------------------------------------------
VOID* AttRes::InitFashionColorPct(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	M_trans_pointer(p, pArray, tagFashionColorPct);

	if(nIndex >= X_COLOR_NUM)
	{
		ASSERT(nIndex < X_COLOR_NUM);
		print_message(_T("\n\nCaution:\n"));
		print_message(_T("\tThere is a critical in fashion_color_pct.xml!!!!!!!!\n\n"));
		return NULL;
	}
	
	p[EIQ_Quality0].n16ColorPct[nIndex] = m_pVar->get_int(_T("WhitePct"), szField);
	p[EIQ_Quality2].n16ColorPct[nIndex] = m_pVar->get_int(_T("YellowPct"), szField);
	p[EIQ_Quality4].n16ColorPct[nIndex] = m_pVar->get_int(_T("GreenPct"), szField);
	p[EIQ_Quality1].n16ColorPct[nIndex] = m_pVar->get_int(_T("BluePct"), szField);
	p[EIQ_Quality3].n16ColorPct[nIndex] = m_pVar->get_int(_T("OrangePct"), szField);

	return NULL;
}

//-------------------------------------------------------------------------------------------
// 初始化商店
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneShopProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pShop, pProtoType, tagShopProto);

	TCHAR szTmp[64];
	
	// 读取的为稀有物品原型
	if(X_READ_FILE_2 == nDummy)
	{
		for(INT32 i=0; i<MAX_SHOP_RARE_ITEM; ++i)
		{
			_stprintf(szTmp, _T("%d"), i+101);
			pShop->RareItem[i].dw_data_id = m_pVar->get_dword(_T("scarce_id"), szTmp, szField, INVALID_VALUE);

			if(INVALID_VALUE == pShop->RareItem[i].dw_data_id)
			{
				break;
			}

			pShop->RareItem[i].byRepLevel	= (BYTE)m_pVar->get_dword(_T("reputation_level_min"), szTmp, szField, ERL_Hostile);
			pShop->RareItem[i].nSilver		= m_pVar->get_int(_T("money_number"), szTmp, szField, 0);
			pShop->RareItem[i].nCostNum		= m_pVar->get_int(_T("special_cost_number"), szTmp, szField, 0);
			pShop->RareItem[i].byShelf		= (BYTE)m_pVar->get_dword(_T("page_number"), szTmp, szField, 1) - 1;
			ASSERT(pShop->RareItem[i].byShelf - 1 < MAX_SHOP_SHELF_NUM);

			pShop->RareItem[i].byQuality	= (BYTE)m_pVar->get_dword(_T("item_quality"), szTmp, szField, EIQ_Quality0);
			/*ASSERT((CHAR)pShop->RareItem[i].by_quality > EIQ_Start && (CHAR)pShop->RareItem[i].by_quality < EIQ_End);*/

			pShop->RareItem[i].byRefreshNum	= (BYTE)m_pVar->get_dword(_T("refresh_number"), szTmp, szField, 0);
			pShop->RareItem[i].dwRefreshTime = m_pVar->get_dword(_T("refresh_time"), szTmp, szField, INVALID_VALUE);

			if(pShop->RareItem[i].nSilver < 0 || pShop->RareItem[i].nCostNum < 0)
			{
				ASSERT(0);
				print_message(_T("\n\n ShopRation.xml has error, scarce_id =%d !!!\n\n\n"), pShop->RareItem[i].dw_data_id);
			}
		}

		return &pShop->dwID;
	}

	// 普通物品原型
	pShop->dwID			= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	pShop->eCostType	= (ECurCostType)m_pVar->get_int(_T("spend_type"), szField, ECCT_Null);
	pShop->dwItemTypeID	= m_pVar->get_dword(_T("spend_item_id"), szField, INVALID_VALUE);
	pShop->bEquip		= m_pVar->get_dword(_T("all_equipment"), szField);
	pShop->bBind		= m_pVar->get_dword(_T("bind"), szField, FALSE);
	pShop->bClanTreasury= m_pVar->get_dword(_T("family_item"), szField, 0);
	pShop->bRandAtt		= m_pVar->get_dword(_T("rand_att"), szField, 1);

	for(INT32 i=0; i<MAX_SHOP_SHELF_NUM; ++i)
	{
		_stprintf(szTmp, _T("item_number%d"), i+1);
		pShop->n16Num[i] = (INT16)m_pVar->get_dword(szTmp, szField, 0);
	}

	for(INT32 i=0; i<MAX_SHOP_COMMON_ITEM; ++i)
	{
		_stprintf(szTmp, _T("%d"), i+1);
		pShop->Item[i].dw_data_id = m_pVar->get_dword(_T("item_id"), szTmp, szField, INVALID_VALUE);

		if(INVALID_VALUE == pShop->Item[i].dw_data_id)
		{
			break;
		}

		pShop->Item[i].byRepLevel	= (BYTE)m_pVar->get_dword(_T("reputation_level_min"), szTmp, szField, ERL_Hostile);
		pShop->Item[i].nSilver		= m_pVar->get_int(_T("money_number"), szTmp, szField, 0);
		pShop->Item[i].nCostNum		= m_pVar->get_int(_T("special_cost_number"), szTmp, szField, 0);
		pShop->Item[i].byShelf		= (BYTE)m_pVar->get_dword(_T("page_number"), szTmp, szField, 1) - 1;

		if(pShop->Item[i].nSilver < 0 || pShop->Item[i].nCostNum < 0
			|| pShop->Item[i].byShelf - 1 >= MAX_SHOP_SHELF_NUM)
		{
			ASSERT(0);
			ASSERT(pShop->Item[i].byShelf - 1 < MAX_SHOP_SHELF_NUM);
			m_att_res_caution(_T("shopInfo"), _T("ID"), pShop->dwID);
		}
	}

	return &pShop->dwID;
}

//-------------------------------------------------------------------------------------------
// 初始化商场商品及礼品包
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneMallItemProtoBase(OUT LPVOID pProtoType, IN LPCTSTR szField)
{
	M_trans_pointer(p, pProtoType, tagMallItemProtoBase);

	p->dwID						= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	p->nPrice					= m_pVar->get_int(_T("PurchasePrice"), szField);
	p->nSalePrice				= m_pVar->get_int(_T("PromotePrice"), szField, INVALID_VALUE);
	p->dwPresentID				= m_pVar->get_dword(_T("PresentID"), szField, INVALID_VALUE);
	p->byNum					= (BYTE)m_pVar->get_dword(_T("SaleNumber"), szField, INVALID_VALUE);
	p->byPresentNum				= (BYTE)m_pVar->get_dword(_T("PresentNumber"), szField, 0);
	p->byExAssign				= (BYTE)m_pVar->get_dword(_T("ExchangeVolumeAssign"), szField, 0);
	p->byExNum					= (BYTE)m_pVar->get_dword(_T("ExchangeVolumeNumber"), szField, INVALID_VALUE);
	p->bNew						= m_pVar->get_int(_T("NewGoods"), szField, 0);
	p->bHot						= m_pVar->get_int(_T("HotGoods"), szField, 0);
	//p->bySmallGroupHeadcount	= (BYTE)m_pVar->get_dword(_T("GroupRoleNumberMin"), szField, INVALID_VALUE);
	//p->bySmallGroupDiscount		= (BYTE)m_pVar->get_dword(_T("GroupRebateMin"), szField, INVALID_VALUE);
	//p->byMediumGroupHeadcount	= (BYTE)m_pVar->get_dword(_T("GroupRoleNumberMiddle"), szField, INVALID_VALUE);
	//p->byMediumGroupDiscount	= (BYTE)m_pVar->get_dword(_T("GroupRebateMiddle"), szField, INVALID_VALUE);
	//p->byBigGroupHeadcount		= (BYTE)m_pVar->get_dword(_T("GroupRoleNumberMax"), szField, INVALID_VALUE);
	//p->byBigGroupDiscount		= (BYTE)m_pVar->get_dword(_T("GroupRebateMax"), szField, INVALID_VALUE);
	p->dwPersistTime			= m_pVar->get_dword(_T("PersistTime"), szField, INVALID_VALUE);

	p->dwTimeSaleStart			= INVALID_VALUE;//促销开始时间
	if(m_pVar->get_dword(_T("PromoteHourStart"), szField, INVALID_VALUE) != INVALID_VALUE)
	{
		p->dwTimeSaleStart		= tagDWORDTime(0, 0, (BYTE)m_pVar->get_dword(_T("PromoteHourStart"), szField), 
			(BYTE)m_pVar->get_dword(_T("PromoteDayStart"), szField), 
			(BYTE)m_pVar->get_dword(_T("PromoteMonthStart"), szField), 
			(BYTE)m_pVar->get_dword(_T("PromoteYearStart"), szField));
	}

	p->dwTimeSaleEnd			= INVALID_VALUE;
	if(m_pVar->get_dword(_T("PromoteHour"), szField, INVALID_VALUE) != INVALID_VALUE)
	{
		p->dwTimeSaleEnd		= tagDWORDTime(0, 0, (BYTE)m_pVar->get_dword(_T("PromoteHour"), szField), 
												(BYTE)m_pVar->get_dword(_T("PromoteDay"), szField), 
												(BYTE)m_pVar->get_dword(_T("PromoteMonth"), szField), 
												(BYTE)m_pVar->get_dword(_T("PromoteYear"), szField));
	}

	p->dwSaleBegin = INVALID_VALUE;
	if(m_pVar->get_dword(_T("BeginSaleHour"), szField, INVALID_VALUE) != INVALID_VALUE)
	{
		p->dwSaleBegin		= tagDWORDTime(0, 0, (BYTE)m_pVar->get_dword(_T("BeginSaleHour"), szField), 
			(BYTE)m_pVar->get_dword(_T("BeginSaleDay"), szField), 
			(BYTE)m_pVar->get_dword(_T("BeginSaleMonth"), szField), 
			(BYTE)m_pVar->get_dword(_T("BeginSaleYear"), szField));
	}

	p->dwSaleEnd = INVALID_VALUE;
	if(m_pVar->get_dword(_T("EndSaleHour"), szField, INVALID_VALUE) != INVALID_VALUE)
	{
		p->dwSaleEnd		= tagDWORDTime(0, 0, (BYTE)m_pVar->get_dword(_T("EndSaleHour"), szField), 
			(BYTE)m_pVar->get_dword(_T("EndSaleDay"), szField), 
			(BYTE)m_pVar->get_dword(_T("EndSaleMonth"), szField), 
			(BYTE)m_pVar->get_dword(_T("EndSaleYear"), szField));
	}

	// 检查
	if(p->nPrice <= 0)
	{
		print_message(_T("There is a fatal error in mall item<id: %u>!!!!!!!!!!!!!\n\n"), p->dwID);
		ASSERT(0);
	}
	
	// 额外处理
	if(INVALID_VALUE == p->nSalePrice)
	{
		p->nSalePrice = p->nPrice;
	}

	return NULL;
}

VOID* AttRes::InitOneMallItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	InitOneMallItemProtoBase(pProtoType, szField);
	
	M_trans_pointer(p, pProtoType, tagMallItemProto);

	p->dw_data_id	= m_pVar->get_dword(_T("TypeID"), szField);
	p->n8Kind	= (INT8)m_pVar->get_int(_T("Kind"), szField);
	p->byRank	= (BYTE)m_pVar->get_dword(_T("BestSellRank"), szField, 0);
	p->byGroupPurchaseAmount = (BYTE)m_pVar->get_dword(_T("GroupBuyAmount"), szField, 1);

	return &p->dwID;
}

VOID* AttRes::InitOneMallPackProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	InitOneMallItemProtoBase(pProtoType, szField);

	M_trans_pointer(p, pProtoType, tagMallPackProto);

	TCHAR szTmp[32];
	INT i = 0;
	for(; i<MALL_PACK_ITEM_NUM; ++i)
	{
		_stprintf(szTmp, _T("%d"), i+1);
		p->dw_data_id[i]	= m_pVar->get_dword(_T("ItemID"), szTmp, szField, INVALID_VALUE);
		if(INVALID_VALUE == p->dw_data_id[i])
		{
			break;
		}

		p->byItemNum[i]		= (BYTE)m_pVar->get_dword(_T("ItemNumber"), szTmp, szField, 0);
		p->nItemPrice[i]	= m_pVar->get_int(_T("ItemPrice"), szTmp, szField, 0);
	}

	p->n8ItemKind = i;
	// 检查
	if(p->n8ItemKind <= 0)
	{
		print_message(_T("There is a fatal error in mall item<id: %u>!!!!!!!!!!!!!\n\n"), p->dwID);
		ASSERT(p->n8ItemKind > 0);
	}

	return &p->dwID;
}

//-------------------------------------------------------------------------------------------
// 初始化驿站
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneDakProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pDak, pProtoType, tagDakProto);

	pDak->dwID			= (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	TCHAR szTmp[8];
	for(INT32 i=0; i<MAX_DAK_SITE_NUM; ++i)
	{
		_stprintf(szTmp, _T("%d"), i+1);
		
		LPCTSTR szMapName = m_pVar->get_string(_T("Scene"), szTmp, szField, NULL);
		if(NULL == szMapName)
		{
			pDak->dakSite[i].dwMapID = INVALID_VALUE;
			break;
		}

		pDak->dakSite[i].dwMapID	= get_tool()->crc32(szMapName);

		LPCTSTR szWayPoint = m_pVar->get_string(_T("WayPoint"), szTmp, szField, NULL);
		if(NULL == szWayPoint)
		{
			pDak->dakSite[i].dwWayPointID = INVALID_VALUE;
			break;
		}

		pDak->dakSite[i].dwWayPointID = get_tool()->crc32(szWayPoint);
		pDak->dakSite[i].eCostType	= (ECurCostType)m_pVar->get_int(_T("CostTypeScene"), szTmp, szField, ECCT_Null);
		pDak->dakSite[i].nCostVal	= m_pVar->get_int(_T("CostValueScene"), szTmp, szField, 0);
	}

	return &pDak->dwID;
}

//-----------------------------------------------------------------------------
// 根据TypeID重新设置是否记录log
//-----------------------------------------------------------------------------
VOID AttRes::ResetItemLog(s_need_log_item dw_data_id[], INT32 n_num)
{
	ASSERT(n_num >= 0);
	ASSERT(m_mapItemProto.size() != 0);
	ASSERT(m_mapEquipProto.size() != 0);

	tagItemProto	*pItem;
	tagEquipProto	*pEquip;
	if(0 == n_num)
	{
		m_mapItemProto.reset_iterator();
		while(m_mapItemProto.find_next(pItem))
		{
			pItem->bNeedLog = TRUE;
			pItem->byLogMinQlty = EIQ_Quality0;
		}

		m_mapEquipProto.reset_iterator();
		while(m_mapEquipProto.find_next(pEquip))
		{
			pEquip->bNeedLog = TRUE;
			pItem->byLogMinQlty = EIQ_Quality0;
		}
	}

	if(n_num > 0)
	{
		for(INT32 i = 0; i < n_num; ++i)
		{
			if(MIsEquipment(dw_data_id[i].dw_data_id))
			{
				pItem = m_mapEquipProto.find(dw_data_id[i].dw_data_id);
			}
			else
			{
				pItem = m_mapItemProto.find(dw_data_id[i].dw_data_id);
			}

			if(!VALID_POINT(pItem))
			{
				ASSERT(VALID_POINT(pItem));
				print_message(_T("Can not find item prototype in m_mapItemProto!!!!!\nPlease check <typeid: %u>!!!!\n"), dw_data_id[i]);
				continue;
			}

			pItem->bNeedLog = TRUE;
			pItem->byLogMinQlty = dw_data_id[i].by_quality;
		}
	}
}


//-----------------------------------------------------------------------------
// 处理Skill中人物属性中的一条属性
//-----------------------------------------------------------------------------
VOID AddSkillRoleAtt(tagSkillProto* pSkill, ERoleAttribute eType, INT nValue)
{
	ASSERT(eType > ERA_Null && eType < ERA_End);

	if(0 == nValue)
		return;

	if(M_is_value_pct(nValue))
	{
		pSkill->mapRoleAttModPct.add(eType, M_value_pct_trans(nValue));
	}
	else
	{
		pSkill->mapRoleAttMod.add(eType, nValue);
	}
}
//-----------------------------------------------------------------------------
// 处理Buff中人物属性中的一条属性
//-----------------------------------------------------------------------------
VOID AddBuffRoleAtt(tagBuffProto* pSkill, ERoleAttribute eType, INT nValue)
{
	ASSERT(eType > ERA_Null && eType < ERA_End);

	if(0 == nValue)
		return;

	if(M_is_value_pct(nValue))
	{
		pSkill->mapRoleAttModPct.add(eType, M_value_pct_trans(nValue));
	}
	else
	{
		pSkill->mapRoleAttMod.add(eType, nValue);
	}
}

//-----------------------------------------------------------------------------
// 初始化一条Skill记录
//-----------------------------------------------------------------------------
VOID *AttRes::InitOneSkillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pSkill, pProtoType, tagSkillProto);

	// 初始化结构信息
	pSkill->dwID								=	m_pVar->get_dword(_T("id"), szField);
	pSkill->eType								=	(enum ESkillType)m_pVar->get_int(_T("Type"), szField);
	pSkill->nType2								=	m_pVar->get_int(_T("Type2"), szField);
	pSkill->nType3								=	m_pVar->get_int(_T("Type3"), szField);
	pSkill->bPoint								=	m_pVar->get_int(_T("Point"), szField, 0);

	// 目标类型
	pSkill->eTalentType							=	(enum ETalentType)m_pVar->get_int(_T("TalentType"), szField, ETT_Null);
	pSkill->eTargetType							=	(enum ESkillTargetType)m_pVar->get_int(_T("TargetType"), szField, ESTT_Combat);
	pSkill->dwTargetSkillID[0]					=	m_pVar->get_dword(_T("TargetSkillId"), szField, INVALID_VALUE);
	pSkill->dwTargetSkillID[1]					=	m_pVar->get_dword(_T("TargetSkillId2"), szField, INVALID_VALUE);
	pSkill->dwTargetSkillID[2]					=	m_pVar->get_dword(_T("TargetSkillId3"), szField, INVALID_VALUE);

	pSkill->dwTargetBuffID						=	m_pVar->get_dword(_T("TargetBuffId"), szField, INVALID_VALUE);
	pSkill->dwTargetTriggerID					=	m_pVar->get_dword(_T("TargetTriggerId"), szField, INVALID_VALUE);

	// 攻击判定
	pSkill->eDistType							=	(enum ESkillDistType)m_pVar->get_int(_T("DistinceType"), szField, ESDT_Null);
	pSkill->eUseType							=	(enum ESkillUseType)m_pVar->get_int(_T("UseType"), szField, ESUT_Null);
	pSkill->eOPType								=	(enum ESkillOPType)m_pVar->get_int(_T("OperationType"), szField);
	pSkill->fOPDist								=	m_pVar->get_float(_T("OperationDistance"),	szField, 0.0f);
	pSkill->fOPRadius							=	m_pVar->get_float(_T("OperationRaduis"), szField, 0.0f);
	pSkill->bInterruptMove						=	(BOOL)m_pVar->get_dword(_T("InterruptMove"), szField, 0);
	pSkill->nInterruptSkillOrdRate				=	m_pVar->get_int(_T("InterruptSkillOrdRate"), szField, 0);
	pSkill->nInterruptSkillSpecRate				=	m_pVar->get_int(_T("InterruptSkillSpecRate"), szField, 0);
	pSkill->nPrepareTime						=	m_pVar->get_int(_T("ReadyTime"), szField, 0);
	pSkill->nCoolDown							=	m_pVar->get_int(_T("Cooldown"), szField);
	pSkill->nMaxCoolDown						=	m_pVar->get_int(_T("MaxCoolDown"), szField, 0);
	pSkill->eDmgType							=	(enum ESkillDmgType)m_pVar->get_int(_T("DemageType"), szField);
	pSkill->nEnmity								=	m_pVar->get_int(_T("Enmity"), szField, 1);
	pSkill->fEnmityParam						=	m_pVar->get_float(_T("EnmityParam"), szField, 0.0f);
	pSkill->nHit								=	m_pVar->get_int(_T("Hit"), szField, 0);
	pSkill->nCrit								=	m_pVar->get_int(_T("Crit"), szField, 0);
	pSkill->nHitNumber							=	m_pVar->get_int(_T("HitNumber"), szField, INVALID_VALUE);

	// 管道
	pSkill->nChannelDmg[0]						=	m_pVar->get_int(_T("PipelineDemage_1"), szField, 0);
	pSkill->nChannelDmg[1]						=	m_pVar->get_int(_T("PipelineDemage_2"), szField, 0);
	pSkill->nChannelDmg[2]						=	m_pVar->get_int(_T("PipelineDemage_3"), szField, 0);
	pSkill->nChannelDmg[3]						=	m_pVar->get_int(_T("PipelineDemage_4"), szField, 0);
	pSkill->nChannelDmg[4]						=	m_pVar->get_int(_T("PipelineDemage_5"), szField, 0);
	pSkill->nChannelTime[0]						=	m_pVar->get_int(_T("PipelineTime_1"), szField, 0);
	pSkill->nChannelTime[1]						=	m_pVar->get_int(_T("PipelineTime_2"), szField, 0);
	pSkill->nChannelTime[2]						=	m_pVar->get_int(_T("PipelineTime_3"), szField, 0);
	pSkill->nChannelTime[3]						=	m_pVar->get_int(_T("PipelineTime_4"), szField, 0);
	pSkill->nChannelTime[4]						=	m_pVar->get_int(_T("PipelineTime_5"), szField, 0);

	// 状态
	pSkill->dwBuffID[0]							=	m_pVar->get_dword(_T("BuffId_1"), szField, INVALID_VALUE);
	pSkill->dwBuffID[1]							=	m_pVar->get_dword(_T("BuffId_2"), szField, INVALID_VALUE);
	pSkill->dwBuffID[2]							=	m_pVar->get_dword(_T("BuffId_3"), szField, INVALID_VALUE);
	pSkill->dwTriggerID[0]						=	m_pVar->get_dword(_T("BuffTrigger_1"), szField, INVALID_VALUE);
	pSkill->dwTriggerID[1]						=	m_pVar->get_dword(_T("BuffTrigger_2"), szField, INVALID_VALUE);
	pSkill->dwTriggerID[2]						=	m_pVar->get_dword(_T("BuffTrigger_3"), szField, INVALID_VALUE);

	// 消耗
	pSkill->dwCostItemID						=	m_pVar->get_dword(_T("CostItemId"), szField, INVALID_VALUE);
	pSkill->nCostItemNum						=	m_pVar->get_int(_T("CostItemNum"), szField, 0);
	pSkill->nSkillCost[ESCT_HP]					=	m_pVar->get_int(_T("CostHp"), szField, 0);
	pSkill->nSkillCost[ESCT_MP]					=	m_pVar->get_int(_T("CostMp"), szField, 0);
	pSkill->nSkillCost[ESCT_Love]				=	m_pVar->get_int(_T("CostLove"), szField, 0);

	// 升级
	pSkill->nLevel								=	m_pVar->get_int(_T("Level"), szField);
	pSkill->eLevelUpType						=	(enum ESkillLevelUpType)m_pVar->get_int(_T("LevelUpType"), szField, ESLUT_Fixed);
	pSkill->nLevelUpExp							=	m_pVar->get_int(_T("Exp"), szField, 0);
	pSkill->dwPreLevelSkillID					=	m_pVar->get_dword(_T("FrontSkillId"), szField, INVALID_VALUE);
	pSkill->nMaxLevel							=	m_pVar->get_int(_T("MaxLevel"), szField);
	pSkill->nMaxLearnLevel						=	m_pVar->get_int(_T("MaxLearnLevel"), szField, 10);
	pSkill->nNeedRoleLevel						=	m_pVar->get_int(_T("NeedRoleLevel"), szField, 0);
	pSkill->nNeedTalentPoint					=	m_pVar->get_int(_T("NeedTalentPoint"), szField, 0);
	//pSkill->eNeedClassType						=	(enum EClassType)m_pVar->get_int(_T("vocation_type"), szField);

	// 学习限制
	pSkill->dwLearnVocationLimit				=	m_pVar->get_dword(_T("VocationLimitLearn"), szField, INVALID_VALUE);
	pSkill->dwMoneyConsume						=	m_pVar->get_dword(_T("MoneyConsume"), szField, 0);
	
	// 使用限制
	pSkill->nUseHPPctLimit						=	m_pVar->get_int(_T("HpLimit"), szField, 0);
	pSkill->nUseMPPctLimit						=	m_pVar->get_int(_T("MpLimit"), szField, 0);
	pSkill->eSexLimit							=	(enum ESkillSexLimit)m_pVar->get_int(_T("SexLimit"), szField, 0);
	pSkill->nWeaponLimit						=	m_pVar->get_int(_T("WeaponLimit"), szField, 0);
	pSkill->ePosType							=	(enum ESkillPosType)m_pVar->get_int(_T("PositionLimit"), szField, 0);
	pSkill->bMoveable							=	(BOOL)m_pVar->get_dword(_T("Moveable"), szField, FALSE);
	pSkill->bMoveCancel							=	(BOOL)m_pVar->get_dword(_T("MoveCancel"), szField, FALSE);
	pSkill->nHitFlyPro							=	m_pVar->get_int(_T("HitFlyPro"), szField, 0);
	pSkill->bCollide							=	(BOOL)m_pVar->get_dword(_T("Collide"), szField, TRUE);	
	pSkill->dwTargetLimit						=	m_pVar->get_dword(_T("TargetLimit"), szField, 0);
	pSkill->nTargetLevelLimit					=	m_pVar->get_int(_T("TargetLevelLimit"), szField, 0);
	pSkill->dwSelfStateLimit					=	m_pVar->get_dword(_T("SelfStateLimit"), szField, INVALID_VALUE);
	pSkill->dwTargetStateLimit					=	m_pVar->get_dword(_T("TargetStateLimit"), szField, 251657982);
	pSkill->bInterCombat						=	(BOOL)m_pVar->get_dword(_T("IntoCombat"), szField, 0);
	pSkill->dwBuffLimitID						=	m_pVar->get_dword(_T("BuffLimitId"), szField, INVALID_VALUE);
	pSkill->dwTargetBuffLimitID					=	m_pVar->get_dword(_T("TargetBuffLimitId"), szField, INVALID_VALUE);
	pSkill->dwVocationLimit						=	m_pVar->get_dword(_T("VocationLimit"), szField, INVALID_VALUE);
	pSkill->bHasPet								=	m_pVar->get_dword(_T("HasPet"), szField, FALSE);
	pSkill->bPublicCD							=   m_pVar->get_dword(_T("PublicCD"), szField, TRUE);
	pSkill->bPublicCDLimit						=	m_pVar->get_dword(_T("PublicCDLimit"), szField, FALSE);

	pSkill->bFriendly							=	(BOOL)m_pVar->get_dword(_T("Friendly"), szField, FALSE);
	pSkill->bHostile							=	(BOOL)m_pVar->get_dword(_T("Enmey"), szField, FALSE);
	pSkill->bIndependent						=	(BOOL)m_pVar->get_dword(_T("Neutral"), szField, FALSE);

	// 触发器加成
	pSkill->nTriggerEventMisc1Add				=	m_pVar->get_int(_T("TriggerEventValueAdd_1"), szField, 0);
	pSkill->nTriggerEventMisc2Add				=	m_pVar->get_int(_T("TriggerEventValueAdd_2"), szField, 0);
	pSkill->nTriggerStateMisc1Add				=	m_pVar->get_int(_T("TriggerStateValueAdd_1"), szField, 0);
	pSkill->nTriggerStateMisc2Add				=	m_pVar->get_int(_T("TriggerStateValueAdd_2"), szField, 0);
	pSkill->nTriggerPropAdd						=	m_pVar->get_int(_T("TriggerProbabilityAdd"), szField, 0);

	// 状态加成
	pSkill->nBuffPersistTimeAdd					=	m_pVar->get_int(_T("BuffTimeAdd"), szField, 0);
	pSkill->nBuffWarpTimesAdd					=	m_pVar->get_int(_T("BuffWrapTimesAdd"), szField, 0);
	pSkill->nBuffInterruptResistAdd				=	m_pVar->get_int(_T("BuffInterupt"), szField, 0);
	pSkill->eModBuffMode						=	(enum EBuffEffectMode)m_pVar->get_int(_T("BuffEffectMode"), szField, EBEM_Null);
	pSkill->nBuffMisc1Add						=	m_pVar->get_int(_T("BuffValueAdd_1"), szField, 0);
	pSkill->nBuffMisc2Add						=	m_pVar->get_int(_T("BuffValueAdd_2"), szField, 0);
	pSkill->nBuffEffectAttMod[EBEA_HP]			=	m_pVar->get_int(_T("BuffHp"), szField, 0);
	pSkill->nBuffEffectAttMod[EBEA_MP]			=	m_pVar->get_int(_T("BuffMp"), szField, 0);
	pSkill->nBuffEffectAttMod[EBEA_Rage]		=	m_pVar->get_int(_T("BuffLove"), szField, 0);
	// 人物属性加成
	TCHAR szName[X_SHORT_NAME] = _T("");
	for (int i = 0; i < ERA_End; i++)
	{
		_stprintf(szName, _T("RoleAtt_%02d"), i);
		INT nAtt =	m_pVar->get_int(szName, szField, 0);
		AddSkillRoleAtt(pSkill, ERoleAttribute(i), nAtt);
	}

	// 计算总伤害次数
	pSkill->nDmgTimes = 0;
	for(INT n = 0; n < MAX_CHANNEL_TIMES; n++)
	{
		if( pSkill->nChannelDmg[n] > 0 )
			++pSkill->nDmgTimes;
		else
			break;
	}

	// 返回
	return &pSkill->dwID;
}

//-----------------------------------------------------------------------------
// 初始化一条Buff记录
//-----------------------------------------------------------------------------
VOID *AttRes::InitOneBuffProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pBuff, pProtoType, tagBuffProto);

	// 初始化结构信息
	pBuff->dwID								=	m_pVar->get_dword(_T("id"), szField);
	pBuff->eType							=	(enum EBuffType)m_pVar->get_int(_T("Type"), szField);
	pBuff->nType2							=	m_pVar->get_int(_T("Type2"), szField);
	pBuff->nType3							=	m_pVar->get_int(_T("Type3"), szField);
	pBuff->eResistType						=	(enum EBuffResistType)m_pVar->get_int(_T("ResistType"), szField);
	pBuff->bBenifit							=	(BOOL)m_pVar->get_dword(_T("Useful"), szField);
	pBuff->eFriendly						=	(enum EBuffFriendEnemy)m_pVar->get_dword(_T("Friendly"), szField);
	pBuff->bInstant							=	(BOOL)m_pVar->get_dword(_T("IsMoment"), szField);
	pBuff->dwGroupFlag						=	m_pVar->get_dword(_T("GroupFlag"), szField, INVALID_VALUE);
	pBuff->dwTargetAddLimit					=	m_pVar->get_dword(_T("TargetLimit"), szField);
	pBuff->dwTargetLimit					=	m_pVar->get_dword(_T("TargetType"), szField);// 作用对象限制
	pBuff->dwTargetAddStateLimit			=	m_pVar->get_dword(_T("TargetAddStateLimit"), szField, INVALID_VALUE);
	pBuff->dwTargetStateLimit				=	m_pVar->get_dword(_T("TargetStateLimit"), szField, INVALID_VALUE);
	pBuff->nLevel							=	m_pVar->get_int(_T("Level"), szField);
	pBuff->eOPType							=	(enum EBuffOPType)m_pVar->get_int(_T("OperationType"), szField);
	pBuff->fOPDistance						=	m_pVar->get_float(_T("OperationDistance"), szField);
	pBuff->fOPRadius						=	m_pVar->get_float(_T("OperationRaduis"), szField);
	pBuff->nPersistTick						=	m_pVar->get_int(_T("ContinuedTime"), szField);
	pBuff->nInterOPTick						=	m_pVar->get_int(_T("IntervalOperationTime"), szField);
	pBuff->nWarpTimes						=	m_pVar->get_int(_T("WrapTimes"), szField);
	pBuff->bOfflineConsume					=	(BOOL)m_pVar->get_dword(_T("OfflineConsume"), szField, FALSE);
	pBuff->dwBeModSkillID					=	m_pVar->get_dword(_T("BeModSkill"), szField, INVALID_VALUE);
	// 打断
	BOOL bInterruptMove						=	(BOOL)m_pVar->get_dword(_T("IsMoveInterrupt"), szField, FALSE);
	BOOL bInterruptInterCombat				=	(BOOL)m_pVar->get_dword(_T("IsIntocombatInterrupt"), szField, FALSE);
	BOOL bInterruptManual					=	(BOOL)m_pVar->get_dword(_T("IsManualInterrupt"), szField, FALSE);
	BOOL bInterruptDead						=	(BOOL)m_pVar->get_dword(_T("IsDeadInterrupt"), szField, TRUE);
	BOOL bInterruptChangeMap				=	(BOOL)m_pVar->get_dword(_T("IsChangemapInterrupt"), szField, FALSE);
	BOOL bInterruptBuffFull					=	(BOOL)m_pVar->get_dword(_T("IsBufffullInterrupt"), szField, FALSE);
	BOOL bInterruptOffLine					=	(BOOL)m_pVar->get_dword(_T("IsOfflineInterrupt"), szField, TRUE);
	BOOL bInterruptLeaveCombat				=	(BOOL)m_pVar->get_dword(_T("IsLeaveCombatInterrupt"), szField, FALSE);

	pBuff->dwInterruptFlag = 0;
	pBuff->dwInterruptFlag	|=	(bInterruptMove			?	EBIF_Move			: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptInterCombat	?	EBIF_InterCombat	: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptManual		?	EBIF_Manual			: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptDead			?	EBIF_Die			: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptChangeMap	?	EBIF_ChangeMap		: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptBuffFull		?	EBIF_BuffFull		: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptOffLine		?	EBIF_OffLine		: 0);
	pBuff->dwInterruptFlag	|=	(bInterruptLeaveCombat  ?	EBIF_LeaveCombat	: 0);	

	pBuff->nAttackInterruptRate				=	m_pVar->get_int(_T("AttackInterruptPro"), szField, INVALID_VALUE);
	pBuff->nHPInterruptLimit				=	m_pVar->get_int(_T("HpInterruptLimit"), szField, INVALID_VALUE);
	pBuff->nMPInterruptLimit				=	m_pVar->get_int(_T("MpInterruptLimit"), szField, INVALID_VALUE);
	pBuff->nRageInterruptLimit				=	m_pVar->get_int(_T("RageInterruptLimit"), szField, INVALID_VALUE);

	pBuff->dwBuffInterruptID				=	m_pVar->get_dword(_T("BuffInterruptId"), szField, INVALID_VALUE);


	pBuff->dwInterruptFlag	|=	((pBuff->nAttackInterruptRate > 0)		?	EBIF_BeAttacked		: 0);
	pBuff->dwInterruptFlag	|=	((pBuff->nHPInterruptLimit > 0)			?	EBIF_HPLower		: 0);
	pBuff->dwInterruptFlag	|=	((pBuff->nMPInterruptLimit > 0)			?	EBIF_MPLower		: 0);
	pBuff->dwInterruptFlag	|=	((pBuff->nRageInterruptLimit > 0)		?	EBIF_RageLower		: 0);
	//pBuff->dwInterruptFlag	|=	((pBuff->nVitalityInterruptLimit > 0)	?	EBIF_VitalityLower	: 0);
	//pBuff->dwInterruptFlag	|=	((pBuff->nEnduranceInterruptLimit > 0)	?	EBIF_EnduranceLower	: 0);

	// 效果
	pBuff->dwOPTrigger						=	m_pVar->get_dword(_T("TriggerId"), szField, INVALID_VALUE);

	pBuff->eEffect[EBEM_Persist]			=	(enum EBuffEffectType)m_pVar->get_int(_T("EffectType"), szField, EBET_Null);
	pBuff->eEffect[EBEM_Instant]			=	(enum EBuffEffectType)m_pVar->get_int(_T("InstantEffectType"), szField, EBET_Null);
	pBuff->eEffect[EBEM_Inter]				=	(enum EBuffEffectType)m_pVar->get_int(_T("InterEffectType"), szField, EBET_Null);
	pBuff->eEffect[EBEM_Finish]				=	(enum EBuffEffectType)m_pVar->get_int(_T("EndEffectType"), szField, EBET_Null);

	pBuff->dwEffectMisc1[EBEM_Persist]		=	m_pVar->get_dword(_T("ContinuedParam_1"), szField, 0);
	pBuff->dwEffectMisc2[EBEM_Persist]		=	m_pVar->get_dword(_T("ContinuedParam_2"), szField, 0);
	pBuff->dwEffectMisc1[EBEM_Instant]		=	m_pVar->get_dword(_T("InstantParam_1"), szField, 0);
	pBuff->dwEffectMisc2[EBEM_Instant]		=	m_pVar->get_dword(_T("InstantParam_2"), szField, 0);
	pBuff->dwEffectMisc1[EBEM_Inter]		=	m_pVar->get_dword(_T("Interval_param_1"), szField, 0);
	pBuff->dwEffectMisc2[EBEM_Inter]		=	m_pVar->get_dword(_T("Interval_param_2"), szField, 0);
	pBuff->dwEffectMisc1[EBEM_Finish]		=	m_pVar->get_dword(_T("FinishParam_1"), szField, 0);
	pBuff->dwEffectMisc2[EBEM_Finish]		=	m_pVar->get_dword(_T("FinishParam_2"), szField, 0);

	// 当前属性改变
	pBuff->nInstantAttMod[EBEA_HP]			=	m_pVar->get_int(_T("InstantHp"), szField, 0);
	pBuff->nInstantAttMod[EBEA_MP]			=	m_pVar->get_int(_T("InstantMp"), szField, 0);
	pBuff->nInstantAttMod[EBEA_Rage]		=	m_pVar->get_int(_T("InstantLove"), szField, 0);

	pBuff->nInterAttMod[EBEA_HP]			=	m_pVar->get_int(_T("IntervalHp"), szField, 0);
	pBuff->nInterAttMod[EBEA_MP]			=	m_pVar->get_int(_T("IntervalMp"), szField, 0);
	pBuff->nInterAttMod[EBEA_Rage]			=	m_pVar->get_int(_T("IntervalLove"), szField, 0);

	pBuff->nFinishAttMod[EBEA_HP]			=	m_pVar->get_int(_T("FinishHp"), szField, 0);
	pBuff->nFinishAttMod[EBEA_MP]			=	m_pVar->get_int(_T("FinishMp"), szField, 0);
	pBuff->nFinishAttMod[EBEA_Rage]			=	m_pVar->get_int(_T("FinishLove"), szField, 0);

	// 属性加成
	TCHAR szName[X_SHORT_NAME] = _T("");
	for (int i = 0; i < ERA_End; i++)
	{
		_stprintf(szName, _T("RoleAtt_%02d"), i);
		INT nAtt =	m_pVar->get_int(szName, szField, 0);
		AddBuffRoleAtt(pBuff, ERoleAttribute(i), nAtt);
	}

	// 修正两个时间属性
	if( VALID_POINT(pBuff->nPersistTick) )
		pBuff->nPersistTick /= TICK_TIME;

	if( VALID_POINT(pBuff->nInterOPTick) )
		pBuff->nInterOPTick /= TICK_TIME;

	return &pBuff->dwID;

}

//-----------------------------------------------------------------------------
// 初始化一条trigger记录
//-----------------------------------------------------------------------------
VOID *AttRes::InitOneTriggerProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pTrigger, pProtoType, tagTriggerProto);

	// 初始化结构信息
	pTrigger->dwID				=	m_pVar->get_dword(_T("id"), szField);
	pTrigger->eEventType		=	(enum ETriggerEventType)m_pVar->get_int(_T("EventType"), szField);
	pTrigger->dwEventMisc1		=	m_pVar->get_dword(_T("EventParam1"), szField, 0);
	pTrigger->dwEventMisc2		=	m_pVar->get_dword(_T("EventParam2"), szField, 0);
	pTrigger->nEventProp		=	m_pVar->get_int(_T("probability"), szField);
	pTrigger->eStateType		=	(enum ETriggerStateType)m_pVar->get_int(_T("StateType"), szField);
	pTrigger->dwStateMisc1		=	m_pVar->get_dword(_T("StateParam1"), szField, 0);
	pTrigger->dwStateMisc2		=	m_pVar->get_dword(_T("StateParam2"), szField, 0);

	return &pTrigger->dwID;
}

//-------------------------------------------------------------------------------------------
// 初始化学习技能的一条记录
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneLearnSKillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pLearnSkill, pProtoType, tagLearnSkill);
	pLearnSkill->dwID			=	(DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	INT32 n32Num				=	m_pVar->get_int(_T("num"), szField);
	DWORD dwSkillID = 0;
	TCHAR szTmp[X_SHORT_NAME] = _T("");
	for (int i = 0; i < n32Num; i++)
	{
		_stprintf(szTmp, _T("%s%d"), _T("learnID"), i+1);
		dwSkillID = m_pVar->get_dword(szTmp, szField, 0);
		pLearnSkill->setLearnSkill.insert(dwSkillID);
	}
	return &pLearnSkill->dwID;
}

//----------------------------------------------------------------------------------------
// 创建技能被动影响表
//----------------------------------------------------------------------------------------
VOID* AttRes::LoadModifyMap()
{
	m_mapSkillProto.reset_iterator();
	DWORD dwSkillTypeID = INVALID_VALUE;
	tagSkillProto* pProto = (tagSkillProto*)INVALID_VALUE;

	while( m_mapSkillProto.find_next(dwSkillTypeID, pProto) )
	{
		DWORD dwSkillID = dwSkillTypeID / 100;

		// 如果该技能的作用对象不是技能，则直接返回
		if( ESTT_Skill != pProto->eTargetType ) continue;
		
		for (int i = 0; i < MAX_TARGET_NUM; i++)
		{
			// 如果该技能影响的技能不存在，则直接返回
			if( !VALID_POINT(pProto->dwTargetSkillID[i]) ) continue;

			// 检查被影响的技能是否已经存在，如果没有就创建
			tagSkillModify* pSkillModify = m_mapSkillModify.find(pProto->dwTargetSkillID[i]);

			if( !VALID_POINT(pSkillModify) )
			{
				pSkillModify = new tagSkillModify;
				m_mapSkillModify.add(pProto->dwTargetSkillID[i], pSkillModify);
			}
			if( !pSkillModify->listModify.is_exist(dwSkillID) )
				pSkillModify->listModify.push_back(dwSkillID);
		}
		
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------
// 初始化一条Creature记录（必须在加载完默认之后进行加载）
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneCreatureProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pCreature, pProtoType, tagCreatureProto);
	
	// 初始化结构信息
	pCreature->dw_data_id							=	(DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pCreature->eType								=	(ECreatureType)m_pVar->get_int(_T("Type"), szField);
	pCreature->nType2								=	m_pVar->get_int(_T("Type2"), szField);
	pCreature->nType3								=	m_pVar->get_int(_T("Type3"), szField);
	pCreature->eGuildType							=   (EGuildType)m_pVar->get_int(_T("GuildType"), szField, EGT_NULL);
	pCreature->eCamp								=	(ECamp)m_pVar->get_int(_T("Camp"), szField, ECA_Null);
	pCreature->bySex								=	(BYTE)m_pVar->get_dword(_T("Sex"), szField);
	pCreature->eLite								=	(ERank)m_pVar->get_int(_T("Rank"), szField, ER_Null);
	pCreature->nLevel								=	m_pVar->get_int(_T("Level"), szField);
	pCreature->vSize.x								=	m_pVar->get_float(_T("BoxSizeX"), szField, X_DEF_CREATURE_SIZE_X);
	pCreature->vSize.y								=	m_pVar->get_float(_T("BoxSizeY"), szField, X_DEF_CREATURE_SIZE_Y);
	pCreature->vSize.z								=	m_pVar->get_float(_T("BoxSizeZ"), szField, X_DEF_CREATURE_SIZE_Z);
	pCreature->nDoorCloseTime						=   m_pVar->get_int(_T("DoorCloseTime"), szField, 0);
	pCreature->ePatrolType							=	(ECreaturePatrolType)m_pVar->get_int(_T("GuardType"), szField);
	pCreature->nPatrolRadius						=	m_pVar->get_int(_T("GuardRadius"), szField);
	pCreature->ePursueType							=	(ECreaturePursueType)m_pVar->get_int(_T("PursueType"), szField);
	//pCreature->dwNormalSkillIDMelee					=	m_pVar->get_dword(_T("NormalSkillID1"), szField, INVALID_VALUE);
	//pCreature->dwNormalSkillIDRanged				=	m_pVar->get_dword(_T("NormalSkillID2"), szField, INVALID_VALUE);
	pCreature->dwCarryID							=	m_pVar->get_dword(_T("CarryID"), szField, 0);
	pCreature->dwNormalSkill[0]						=	m_pVar->get_dword(_T("NormalSkillID1"), szField, INVALID_VALUE);
	pCreature->dwNormalSkill[1]						=	m_pVar->get_dword(_T("NormalSkillID2"), szField, INVALID_VALUE);
	pCreature->dwNormalSkill[2]						=	m_pVar->get_dword(_T("NormalSkillID3"), szField, INVALID_VALUE);
	pCreature->nAttackInterTick						=	m_pVar->get_int(_T("AttackInterval"), szField) / TICK_TIME;
	pCreature->eRespawnType							=	(ECreatureRespawnType)m_pVar->get_int(_T("ReliveMode"), szField, 0);
	pCreature->nRespawnTick							=	m_pVar->get_int(_T("ReliveTime"), szField) / (INT)TICK_TIME;
	pCreature->nRespawnTickAdd						=	m_pVar->get_int(_T("ReliveTimeAdd"), szField, 0) / (INT)TICK_TIME;
	pCreature->eTaggedType							=	(ECreatureTaggedType)m_pVar->get_int(_T("TaggedType"), szField);
	pCreature->nLiveTick							=	m_pVar->get_int(_T("LiveTime"), szField, 0)/ TICK_TIME;
	pCreature->nExpGive								=	m_pVar->get_int(_T("Experience"), szField);
	pCreature->nExploits							=	m_pVar->get_int(_T("Exploit"), szField, 0);
	pCreature->eRepType1							=	(EReputationType)m_pVar->get_int(_T("ReputationType1"), szField, ERT_NULL);
	pCreature->nRepNum1								=	m_pVar->get_int(_T("ReputationNumber1"), szField, 0);
	pCreature->eRepType2							=	(EReputationType)m_pVar->get_int(_T("ReputationType2"), szField, ERT_NULL);
	pCreature->nRepNum2								=	m_pVar->get_int(_T("ReputationNumber2"), szField, 0);
	pCreature->dwLootID								=	m_pVar->get_dword(_T("DropID"), szField, INVALID_VALUE);
	pCreature->bCanAttack							=	(BOOL)m_pVar->get_dword(_T("CanAttack"), szField);
	pCreature->bCanBeAttack							=	(BOOL)m_pVar->get_dword(_T("CanBeAttack"), szField);
	pCreature->bVisble								=	(BOOL)m_pVar->get_dword(_T("Seeable"), szField);
	pCreature->bCanHitFly							=	(BOOL)m_pVar->get_dword(_T("CantHitFly"), szField, TRUE);
	pCreature->eFunctionType						=	(EFunctionNPCType)m_pVar->get_int(_T("FunctionType"), szField, EFNPCT_Null);
	pCreature->eAICreateType						=	(EAICreateType)m_pVar->get_int(_T("AICreateType"), szField, EACT_Null);
	pCreature->dwAIID								=	m_pVar->get_dword(_T("AIID"), szField, INVALID_VALUE);
	pCreature->eAIAction							=	(EAIACTION)m_pVar->get_dword(_T("AIAction"), szField, 0);
	pCreature->u16QuestID							=	(UINT16)m_pVar->get_int(_T("QuestID"), szField, 0);
	pCreature->bCanInWater							=	(BOOL)m_pVar->get_dword(_T("CanWater"), szField, FALSE);
	pCreature->nPursueTime							=	m_pVar->get_int(_T("PursueTime"), szField, 10);
	pCreature->nNoAttackTime						=	m_pVar->get_int(_T("NoAttackTime"), szField, 0);
	pCreature->nPursueRadius						=	m_pVar->get_int(_T("PursueRadius"), szField, 5000);
	pCreature->nInstPoint							=   m_pVar->get_int(_T("InstancePoint"), szField, 0);
	pCreature->bCanKill								=	(BOOL)m_pVar->get_dword(_T("CanBeKill"), szField, 1);
	pCreature->bLoading								=	(BOOL)m_pVar->get_dword(_T("Loading"), szField, 0);
	pCreature->bCantka								=	(BOOL)m_pVar->get_dword(_T("Cantka"), szField, 0);
	pCreature->b_save_process						=	(BOOL)m_pVar->get_dword(_T("save_process"), szField, 0);
	pCreature->bVipPriceOff							=	(BOOL)m_pVar->get_dword(_T("VipPriceOff"), szField, 0);
	// 怪物属性
	pCreature->nBaseAtt[ERA_Physique]				=	m_pVar->get_int(_T("Physique"), szField, GetAttDefCreature(ERA_Physique));
	pCreature->nBaseAtt[ERA_Strength]				=	m_pVar->get_int(_T("Strength"), szField, GetAttDefCreature(ERA_Strength));
	pCreature->nBaseAtt[ERA_InnerForce]				=	m_pVar->get_int(_T("Innerforce"), szField, GetAttDefCreature(ERA_InnerForce));
	pCreature->nBaseAtt[ERA_Pneuma]					=	m_pVar->get_int(_T("Pneuma"), szField, GetAttDefCreature(ERA_Pneuma));
	pCreature->nBaseAtt[ERA_Agility]				=	m_pVar->get_int(_T("Technique"), szField, GetAttDefCreature(ERA_Agility));
	
	pCreature->nBaseAtt[ERA_MaxHP]					=	m_pVar->get_int(_T("MaxHP"), szField, GetAttDefCreature(ERA_MaxHP));
	pCreature->nBaseAtt[ERA_MaxMP]					=	m_pVar->get_int(_T("MaxMP"), szField, GetAttDefCreature(ERA_MaxMP));
	pCreature->nBaseAtt[ERA_ExAttack]				=	m_pVar->get_int(_T("wufangMin"), szField, GetAttDefCreature(ERA_ExAttack));
	pCreature->nBaseAtt[ERA_ExDefense]				=	m_pVar->get_int(_T("wufangMax"), szField, GetAttDefCreature(ERA_ExDefense));
	pCreature->nBaseAtt[ERA_InAttack]				=	m_pVar->get_int(_T("mofangMin"), szField, GetAttDefCreature(ERA_InAttack));
	pCreature->nBaseAtt[ERA_InDefense]				=	m_pVar->get_int(_T("mofangMax"), szField, GetAttDefCreature(ERA_InDefense));
	pCreature->nBaseAtt[ERA_HitRate]				=	m_pVar->get_int(_T("HitRate"), szField, GetAttDefCreature(ERA_HitRate));
	pCreature->nBaseAtt[ERA_Dodge]					=	m_pVar->get_int(_T("Dodge"), szField, GetAttDefCreature(ERA_Dodge));
	pCreature->nBaseAtt[ERA_UnCrit_Rate]			=	m_pVar->get_int(_T("UnCritRate"), szField, GetAttDefCreature(ERA_UnCrit_Rate));

	pCreature->nBaseAtt[ERA_HPRegainRate]			=	m_pVar->get_int(_T("RegainHPRate"), szField, GetAttDefCreature(ERA_HPRegainRate));
	pCreature->nBaseAtt[ERA_MPRegainRate]			=	m_pVar->get_int(_T("RegainMPRate"), szField, GetAttDefCreature(ERA_MPRegainRate));
	pCreature->nBaseAtt[ERA_Speed_XZ]				=	m_pVar->get_int(_T("SpeedXZ"), szField, GetAttDefCreature(ERA_Speed_XZ));
	pCreature->nBaseAtt[ERA_Speed_Y]				=	m_pVar->get_int(_T("SpeedY"), szField, GetAttDefCreature(ERA_Speed_Y));
	pCreature->nBaseAtt[ERA_ExAttackMin]			=	m_pVar->get_int(_T("wugongMin"), szField, GetAttDefCreature(ERA_ExAttackMin));
	pCreature->nBaseAtt[ERA_ExAttackMax]			=	m_pVar->get_int(_T("wugongMax"), szField, GetAttDefCreature(ERA_ExAttackMax));
	pCreature->nBaseAtt[ERA_InAttackMin]			=	m_pVar->get_int(_T("mogongMin"), szField, GetAttDefCreature(ERA_InAttackMin));
	pCreature->nBaseAtt[ERA_InAttackMax]			=	m_pVar->get_int(_T("mogongMax"), szField, GetAttDefCreature(ERA_InAttackMax));
	pCreature->nBaseAtt[ERA_ArmorEx]				=	m_pVar->get_int(_T("daogongMin"), szField, GetAttDefCreature(ERA_ArmorEx));
	pCreature->nBaseAtt[ERA_ArmorIn]				=	m_pVar->get_int(_T("daogongMax"), szField, GetAttDefCreature(ERA_ArmorIn));

	pCreature->nBaseAtt[ERA_Derate_Ordinary]		=	m_pVar->get_int(_T("DerateOrdinary"), szField, GetAttDefCreature(ERA_Derate_Ordinary));
	pCreature->nBaseAtt[ERA_Derate_Soil]			=	m_pVar->get_int(_T("DerateSoil"), szField, GetAttDefCreature(ERA_Derate_Soil));
	pCreature->nBaseAtt[ERA_Derate_Gold]			=	m_pVar->get_int(_T("DerateGold"), szField, GetAttDefCreature(ERA_Derate_Gold));
	pCreature->nBaseAtt[ERA_Derate_Wood]			=	m_pVar->get_int(_T("DerateWood"), szField, GetAttDefCreature(ERA_Derate_Wood));
	pCreature->nBaseAtt[ERA_Derate_Fire]			=	m_pVar->get_int(_T("DerateFire"), szField, GetAttDefCreature(ERA_Derate_Fire));
	pCreature->nBaseAtt[ERA_Derate_Water]			=	m_pVar->get_int(_T("DerateWater"), szField, GetAttDefCreature(ERA_Derate_Water));
	pCreature->nBaseAtt[ERA_Derate_Injury]			=	m_pVar->get_int(_T("DerateInjury"), szField, GetAttDefCreature(ERA_Derate_Injury));
	pCreature->nBaseAtt[ERA_Derate_Stunt]			=	m_pVar->get_int(_T("DerateStunt"), szField, GetAttDefCreature(ERA_Derate_Stunt));
	pCreature->nBaseAtt[ERA_Derate_ExAttack]		=	m_pVar->get_int(_T("DerateExAttack"), szField, GetAttDefCreature(ERA_Derate_ExAttack));
	pCreature->nBaseAtt[ERA_Derate_InAttack]		=	m_pVar->get_int(_T("DerateInAttack"), szField, GetAttDefCreature(ERA_Derate_InAttack));
	pCreature->nBaseAtt[ERA_Derate_ALL]				=	m_pVar->get_int(_T("DerateAll"), szField, GetAttDefCreature(ERA_Derate_ALL));
	
	pCreature->nBaseAtt[ERA_ExDamage]				=	m_pVar->get_int(_T("ExDamage"), szField, GetAttDefCreature(ERA_ExDamage));
	pCreature->nBaseAtt[ERA_ExDamage_Absorb]		=	m_pVar->get_int(_T("ExDamageAbsorb"), szField, GetAttDefCreature(ERA_ExDamage_Absorb));
	pCreature->nBaseAtt[ERA_Resist_Bleeding]		=	m_pVar->get_int(_T("ResistBleeding"), szField, GetAttDefCreature(ERA_Resist_Bleeding));
	pCreature->nBaseAtt[ERA_Resist_Weak]			=	m_pVar->get_int(_T("ResistWeak"), szField, GetAttDefCreature(ERA_Resist_Weak));
	pCreature->nBaseAtt[ERA_Resist_Choas]			=	m_pVar->get_int(_T("ResistChoas"), szField, GetAttDefCreature(ERA_Resist_Choas));
	pCreature->nBaseAtt[ERA_Resist_Special]			=	m_pVar->get_int(_T("ResistSpecial"), szField, GetAttDefCreature(ERA_Resist_Special));
	pCreature->nBaseAtt[ERA_Regain_Addtion]			=	m_pVar->get_int(_T("RegainAddtion"), szField, GetAttDefCreature(ERA_Regain_Addtion));
	pCreature->nBaseAtt[ERA_Attack_MissRate]		=	m_pVar->get_int(_T("AttackMissRate"), szField, GetAttDefCreature(ERA_Attack_MissRate));
	pCreature->nBaseAtt[ERA_CloseAttack_DodgeRate]	=	m_pVar->get_int(_T("CloseAttackDodgeRate"), szField, GetAttDefCreature(ERA_CloseAttack_DodgeRate));
	pCreature->nBaseAtt[ERA_RemoteAttack_DodgeRate] =	m_pVar->get_int(_T("RemoteAttackDodgeRate"), szField, GetAttDefCreature(ERA_RemoteAttack_DodgeRate));
	pCreature->nBaseAtt[ERA_Crit_Rate]				=	m_pVar->get_int(_T("CritRate"), szField, GetAttDefCreature(ERA_Crit_Rate));
	pCreature->nBaseAtt[ERA_Crit_Amount]			=	m_pVar->get_int(_T("CritAmount"), szField, GetAttDefCreature(ERA_Crit_Amount));
	pCreature->nBaseAtt[ERA_ShenAttack]				=	m_pVar->get_int(_T("BlockRate"), szField, GetAttDefCreature(ERA_ShenAttack));
	
	pCreature->nBaseAtt[ERA_Inspiration]			=	m_pVar->get_int(_T("Inspiration"), szField, GetAttDefCreature(ERA_Inspiration));
	pCreature->nBaseAtt[ERA_Luck]					=	m_pVar->get_int(_T("Lurk"), szField, GetAttDefCreature(ERA_Luck));
	pCreature->nBaseAtt[ERA_Savvy]					=	m_pVar->get_int(_T("Savvy"), szField, GetAttDefCreature(ERA_Savvy));
	pCreature->nBaseAtt[ERA_Morale]					=	m_pVar->get_int(_T("Morale"), szField, GetAttDefCreature(ERA_Morale));
	pCreature->nBaseAtt[ERA_Injury]					=	m_pVar->get_int(_T("Injury"), szField, GetAttDefCreature(ERA_Injury));

	pCreature->nBaseAtt[ERA_Enmity_Degree]			=	m_pVar->get_int(_T("Enmity"), szField, GetAttDefCreature(ERA_Enmity_Degree));
	pCreature->nBaseAtt[ERA_Shape]					=	m_pVar->get_int(_T("Shape"), szField, GetAttDefCreature(ERA_Shape));
	pCreature->nBaseAtt[ERA_Exp_Add_Rate]			=	m_pVar->get_int(_T("ExperienceAddRate"), szField, GetAttDefCreature(ERA_Exp_Add_Rate));
	pCreature->nBaseAtt[ERA_Money_Add_Rate]			=	m_pVar->get_int(_T("MoneyAddRate"), szField, GetAttDefCreature(ERA_Money_Add_Rate));
	pCreature->nBaseAtt[ERA_Loot_Add_Rate]			=	m_pVar->get_int(_T("DropAddRate"), szField, GetAttDefCreature(ERA_Loot_Add_Rate));


	// 社会属性全部清零
	pCreature->nBaseAtt[ERA_Fortune]				=	0;
	pCreature->nBaseAtt[ERA_Appearance]				=	0;
	pCreature->nBaseAtt[ERA_Rein]					=	0;
	pCreature->nBaseAtt[ERA_Knowledge]				=	0;
	pCreature->nBaseAtt[ERA_Morality]				=	0;
	pCreature->nBaseAtt[ERA_Culture]				=	0;

	// 设置职能ID
	pCreature->uFunctionID.dwCommonID				=	m_pVar->get_dword(_T("FunctionID"), szField, INVALID_VALUE);
	pCreature->nAlertDis							=   m_pVar->get_dword(_T("AlertDistance"), szField, 0 );
	pCreature->fChangeAct							=   (FLOAT)m_pVar->get_float(_T("FleeSOSProbability"), szField, 0.0f);
	pCreature->fChangleActHP					    =   (FLOAT)m_pVar->get_float(_T("FleeSOSHP"), szField, 0.0f);
	pCreature->dwHelpRange							=   m_pVar->get_dword(_T("SOSRange"), szField, INVALID_VALUE);
	pCreature->nHelpNum								=   m_pVar->get_int(_T("SOSNumber"), szField, 1);
	pCreature->eHelpType							=   (EHelpType)m_pVar->get_int(_T("SOSType"), szField, EHT_Stand);
	pCreature->dw_help_time							=   m_pVar->get_dword(_T("SOSTime"), szField, 1000);
	
	pCreature->dwMatureTime							=	m_pVar->get_dword(_T("MatureTime"), szField, 0);
	pCreature->dwChutouID							=	m_pVar->get_dword(_T("Chutou"), szField, 0);

	// 巢穴相关
	if( ECT_Monster == pCreature->eType && EMTT_Nest == pCreature->nType2)
	{
		pCreature->pNest = new tagNestProto;
		pCreature->pNest->nSpawnRadius = m_pVar->get_int(_T("SpawnRaduis"), szField);

		DWORD dwCreatureID = INVALID_VALUE;
		TCHAR szTemp[LONG_STRING];
		INT n = 0;
		for(; n < MAX_SPAWN_CREATURE_NUM; n++)
		{
			_stprintf(szTemp, _T("SpawnID%d"), n+1);
			dwCreatureID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);

			if( INVALID_VALUE != dwCreatureID )
			{
				pCreature->pNest->dwSpawnID[n] = dwCreatureID;
				_stprintf(szTemp, _T("SpawnMaxNumber%d"), n+1);
				pCreature->pNest->nSpawnMax[n] = m_pVar->get_int(szTemp, szField);
			}
			else
			{
				break;
			}
		}
		pCreature->pNest->nCreatureNum = n;
		
	}

	// 怪物小队相关
	if( ECT_Monster == pCreature->eType && EMTT_Team == pCreature->nType2)
	{
		pCreature->pNest = new tagNestProto;
		pCreature->pNest->eOrderType = (ENPCTeamOrder)m_pVar->get_int(_T("TeamOrderType"), szField);
		pCreature->pNest->fSpace = m_pVar->get_float(_T("TeamDistance"), szField);

		DWORD dwCreatureID = INVALID_VALUE;
		TCHAR szTemp[LONG_STRING];
		INT n = 0;
		for(; n < MAX_SPAWN_CREATURE_NUM; n++)
		{
			_stprintf(szTemp, _T("SpawnID%d"), n+1);
			dwCreatureID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);

			if( INVALID_VALUE != dwCreatureID )
			{
				pCreature->pNest->dwSpawnID[n] = dwCreatureID;
				_stprintf(szTemp, _T("SpawnMaxNumber%d"), n+1);
				pCreature->pNest->nSpawnMax[n] = m_pVar->get_int(szTemp, szField);
			}
			else
			{
				break;
			}
		}
		pCreature->pNest->nCreatureNum = n;

	}
	
	return &pCreature->dw_data_id;
}

//--------------------------------------------------------------------------------------------
// 初始化一条CreatureAI记录
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneCreatureAIProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pAI, pProtoType, tagCreatureAI);

	pAI->dwID				=	(DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pAI->dwGroupID			=	m_pVar->get_dword(_T("GroupID"), szField, 0);

	pAI->dwBuffTypeID[0]	=	m_pVar->get_dword(_T("Buff1"), szField, INVALID_VALUE);
	pAI->dwBuffTypeID[1]	=	m_pVar->get_dword(_T("Buff2"), szField, INVALID_VALUE);
	pAI->dwBuffTypeID[2]	=	m_pVar->get_dword(_T("Buff3"), szField, INVALID_VALUE);

	pAI->dwTriggerID[0]		=	m_pVar->get_dword(_T("Event1"), szField, INVALID_VALUE);
	pAI->dwTriggerID[1]		=	m_pVar->get_dword(_T("Event2"), szField, INVALID_VALUE);
	pAI->dwTriggerID[2]		=	m_pVar->get_dword(_T("Event3"), szField, INVALID_VALUE);
	pAI->dwTriggerID[3]		=	m_pVar->get_dword(_T("Event4"), szField, INVALID_VALUE);
	pAI->dwTriggerID[4]		=	m_pVar->get_dword(_T("Event5"), szField, INVALID_VALUE);

	pAI->dwSkillTypeID[0]	=	m_pVar->get_dword(_T("Skill1"), szField, INVALID_VALUE);
	pAI->dwSkillTypeID[1]	=	m_pVar->get_dword(_T("Skill2"), szField, INVALID_VALUE);
	pAI->dwSkillTypeID[2]	=	m_pVar->get_dword(_T("Skill3"), szField, INVALID_VALUE);
	pAI->dwSkillTypeID[3]	=	m_pVar->get_dword(_T("Skill4"), szField, INVALID_VALUE);
	pAI->dwSkillTypeID[4]	=	m_pVar->get_dword(_T("Skill5"), szField, INVALID_VALUE);

	pAI->nTargetType[0]		=	m_pVar->get_int(_T("TargetType1"), szField, 0);
	pAI->nTargetType[1]		=	m_pVar->get_int(_T("TargetType2"), szField, 0);
	pAI->nTargetType[2]		=	m_pVar->get_int(_T("TargetType3"), szField, 0);
	pAI->nTargetType[3]		=	m_pVar->get_int(_T("TargetType4"), szField, 0);
	pAI->nTargetType[4]		=	m_pVar->get_int(_T("TargetType5"), szField, 0);
	
	return &pAI->dwID;
}

//---------------------------------------------------------------------------------------------
// 怪物阵营关系
//---------------------------------------------------------------------------------------------
VOID* AttRes::InitOneCreatureCampProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pCreatureCamp, pProtoType, tagCreatureCamp);

	pCreatureCamp->eCamp = (ECamp)m_pVar->get_int(_T("id"), szField, ECA_Normal);

	pCreatureCamp->eCampConnection[0] = (ECampConnection)m_pVar->get_int(_T("wagang"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[1] = (ECampConnection)m_pVar->get_int(_T("tang"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[2] = (ECampConnection)m_pVar->get_int(_T("sui"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[3] = (ECampConnection)m_pVar->get_int(_T("normal"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[4] =	(ECampConnection)m_pVar->get_int(_T("common"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[5] =	(ECampConnection)m_pVar->get_int(_T("battle_a"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[6] =	(ECampConnection)m_pVar->get_int(_T("battle_b"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[7] = (ECampConnection)m_pVar->get_int(_T("xuemo"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[8] = (ECampConnection)m_pVar->get_int(_T("tianshi"), szField, ECAC_Null);
	pCreatureCamp->eCampConnection[9] = (ECampConnection)m_pVar->get_int(_T("role"), szField, ECAC_Null);

	return &pCreatureCamp->eCamp;
}

//---------------------------------------------------------------------------------------------
// 怪物AI进行分组
//---------------------------------------------------------------------------------------------
VOID* AttRes::GroupCreatureAI()
{
	tagCreatureAI* pAI = NULL;

	m_mapCreatureAI.reset_iterator();
	while( m_mapCreatureAI.find_next(pAI) )
	{
		if(0==pAI->dwGroupID)
			continue;

		// 根据AI的groupid确定其是否已经分组
		package_list<DWORD>* pList = m_mapCreatureAIGroup.find(pAI->dwGroupID);

		if( VALID_POINT(pList) )
		{
			pList->push_back(pAI->dwID);
		}
		else
		{
			pList = new package_list<DWORD>;
			pList->push_back(pAI->dwID);
			m_mapCreatureAIGroup.add(pAI->dwGroupID, pList);
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------
// 可铭纹装备部位的一条记录
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOnePosyPosProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagConsolidatePos *pConsolidatePos = (tagConsolidatePos*)pArray + nIndex * MAX_CONSOLIDATE_POS_QUANTITY;

	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("right_hand"), szField, 0);
	pConsolidatePos->ePos = EEP_RightHand;
	pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("head"), szField, 0);
	pConsolidatePos->ePos = EEP_Head;
	pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("body"), szField, 0);
	pConsolidatePos->ePos = EEP_Body;
	pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("feet"), szField, 0);
	pConsolidatePos->ePos = EEP_Feet;
	pConsolidatePos++;
	//pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("back"), szField, 0);
	//pConsolidatePos->ePos = EEP_Back;
	//pConsolidatePos++;
	//pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("face"), szField, 0);
	//pConsolidatePos->ePos = EEP_Face;
	//pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("neck"), szField, 0);
	pConsolidatePos->ePos = EEP_Neck;
	pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("finger1"), szField, 0);
	pConsolidatePos->ePos = EEP_Finger1;
	pConsolidatePos++;
	pConsolidatePos->byConsolidate = (BYTE)m_pVar->get_dword(_T("waist"), szField, 0);
	pConsolidatePos->ePos = EEP_Waist;
	
	return NULL;

}

//-------------------------------------------------------------------------------------------
// 称号的一条记录
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneTitleProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	//tagTitleProto *pTitleProto = static_cast<tagTitleProto *>( pArray ) + nIndex;
	M_trans_pointer(pTitleProto, pArray, tagTitleProto);

	pTitleProto->m_dwID = m_pVar->get_dword(_T("id"), szField, 0);

	//UINT16 u16Index = ()nIndex;
	//UINT16 u16ID	= pTitleProto->m_u16ID;
	//ASSERT(u16ID == u16Index);
	//if(u16ID != u16Index)
	//{
	//	m_att_res_caution(FileName_Title_Proto, _T("TitleProto"), pTitleProto->m_u16ID);
	//}

	pTitleProto->m_bSpecial	= m_pVar->get_int(_T("special"), szField, 1);
	pTitleProto->m_dwBuffID = m_pVar->get_dword(_T("BuffId"), szField, INVALID_VALUE);
	pTitleProto->m_byLevel = m_pVar->get_dword(_T("level"), szField, 0);
	pTitleProto->m_dwType = m_pVar->get_dword(_T("type"), szField, 0);
	pTitleProto->m_dwColor = m_pVar->get_dword(_T("color"), szField, 0);
	pTitleProto->m_dwAchievementPoint = m_pVar->get_dword(_T("achievementPoint"), szField, 0);
	pTitleProto->m_byNeedRoleLevel = m_pVar->get_dword(_T("needLevel"), szField, 0);
	pTitleProto->m_nNeedYuanbaoLevel = m_pVar->get_dword(_T("needYuanBaoLevel"), szField, 0);
	pTitleProto->m_nNeedAchievementLevel = m_pVar->get_dword(_T("needAchievementLevel"), szField, 0);
	pTitleProto->m_dwTimeLimit =	m_pVar->get_dword(_T("timeLimit"), szField, 0);

	//pTitleProto->m_CondType = (e_condition_type)m_pVar->get_dword(_T("CondId"), szField, 0);
	//pTitleProto->m_dwPara1 = (DWORD)m_pVar->get_dword(_T("Para1"), szField, INVALID_VALUE);
	//pTitleProto->m_dwPara2 = (DWORD)m_pVar->get_dword(_T("Para2"), szField, INVALID_VALUE);

	for(INT i = 0; i < MAX_TITLE_SPECNUM; i++)
	{
		TCHAR szTmp[X_SHORT_NAME];


		_stprintf(szTmp, _T("role_att_type%d"), i+1);
		pTitleProto->m_roleAtt[i].eRoleAtt = (ERoleAttribute)m_pVar->get_int(szTmp, szField, INVALID_VALUE);

		_stprintf(szTmp, _T("role_att_value%d"), i+1);
		pTitleProto->m_roleAtt[i].nValue = m_pVar->get_int(szTmp, szField, 0);

	}
	
	//E_title_event eteEvent = (E_title_event)m_pVar->get_dword(_T("EventId1"), szField, INVALID_VALUE);
	//if ( !VALID_VALUE(pTitleProto->m_Events[0] = eteEvent) ) return NULL;
	//eteEvent = (E_title_event)m_pVar->get_dword(_T("EventId2"), szField, INVALID_VALUE);
	//if ( !VALID_VALUE(pTitleProto->m_Events[1] = eteEvent) ) return NULL;
	//eteEvent = (E_title_event)m_pVar->get_dword(_T("EventId3"), szField, INVALID_VALUE);
	//if ( !VALID_VALUE(pTitleProto->m_Events[2] = eteEvent) ) return NULL;
	//eteEvent = (E_title_event)m_pVar->get_dword(_T("EventId4"), szField, INVALID_VALUE);
	//if ( !VALID_VALUE(pTitleProto->m_Events[3] = eteEvent) ) return NULL;
	//eteEvent = (E_title_event)m_pVar->get_dword(_T("EventId5"), szField, INVALID_VALUE);
	//if ( !VALID_VALUE(pTitleProto->m_Events[4] = eteEvent) ) return NULL;
	
	return  &pTitleProto->m_dwID;
}

//-------------------------------------------------------------------------------------------
// 氏族珍宝的一条记录
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneClanTreasureProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagClanTreasureProto* pTreasureProto = static_cast<tagClanTreasureProto*>(pArray) + nIndex;

	UINT16 dwID							= (UINT16)		m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pTreasureProto->dw_data_id			= (UINT32)		m_pVar->get_dword(_T("itemid"), szField, INVALID_VALUE);
	pTreasureProto->dwNpcID				= (DWORD)		m_pVar->get_dword(_T("npcid"), szField, INVALID_VALUE);
	LPCTSTR	tszMapName					= m_pVar->get_string(_T("mapid"), szField, _T(""));
	pTreasureProto->dwMapID				= get_tool()->crc32(tszMapName);
	pTreasureProto->eClanType			= (ECLanType)	m_pVar->get_dword(_T("clantype"), szField, INVALID_VALUE);
	pTreasureProto->nActClanConNeed		= (INT32)		m_pVar->get_dword(_T("act_clancon_need"), szField, INVALID_VALUE);

// 	if (!VALID_VALUE(pTreasureProto->dw_data_id) ||
// 		!VALID_VALUE(pTreasureProto->dwNpcID)	||
// 		!VALID_VALUE(pTreasureProto->dwMapID)	||
// 		!VALID_VALUE(pTreasureProto->eClanType)||
// 		!VALID_VALUE(pTreasureProto->nActClanConNeed) ||
// 		dwID != nIndex)
// 	{
// 		m_att_res_caution(FileName_ClanTreasure, _T("treasureProtoID"), dwID);
// 	}.

	return NULL;
}


//-------------------------------------------------------------------------------------------
// 初始化商城免费物品(仅有一个)
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneMallFreeItemProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagMallFreeItem *p = (tagMallFreeItem*)pArray + nIndex - 1;
	p->dw_data_id		= m_pVar->get_dword(_T("FreeItemID"), szField, INVALID_VALUE);
	p->byNum		= (BYTE)m_pVar->get_dword(_T("FreeItemNumber"), szField, 0);
	p->nUnitPrice	= m_pVar->get_dword(_T("FreeItemPrice"), szField, 0);

	return NULL;	
}

//-------------------------------------------------------------------------------------------
// 得到强化属性对应的装备是否可强化
//-------------------------------------------------------------------------------------------
BOOL AttRes::IsPosyPos(EPosyAtt ePosyAtt, EEquipPos eEquipPos)
{
	for( INT n = 0; n < MAX_CONSOLIDATE_POS_QUANTITY; ++n)
	{
		if(m_PosyPos[ePosyAtt][n].ePos == eEquipPos)
			return (BOOL)m_PosyPos[ePosyAtt][n].byConsolidate;
	}
	
	return FALSE;
}

//--------------------------------------------------------------------------------------------
// 初始化铭纹的一条记录
//--------------------------------------------------------------------------------------------
//VOID* AttRes::InitOnePosyProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy)
//{
//	MTRANS_POINTER(pPosyProto, pProtoType, tagPosyProtoSer);
//
//	pPosyProto->dwID = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
//
//	_tcscpy( pPosyProto->szName, m_pVar->GetString(_T("name"), szField, _T("")) );
//	pPosyProto->ePosyAtt = (EPosyAtt)m_pVar->GetDword(_T("att"), szField, INVALID_VALUE);
//	pPosyProto->byPosyTimes = (BYTE)m_pVar->GetDword(_T("times"), szField, INVALID_VALUE);
//	pPosyProto->nSuccessRate = m_pVar->GetDword(_T("success_rate"), szField, INVALID_VALUE);
//	pPosyProto->nPotValConsume = m_pVar->GetDword(_T("potval_consume"), szField, 0);
//	pPosyProto->dw_money_consume = m_pVar->GetDword(_T("money_consume"), szField, 0);
//	pPosyProto->fcoefficientA = (FLOAT)m_pVar->GetFloat(_T("coefficientA"), szField, INVALID_VALUE);
//	pPosyProto->fcoefficientB = (FLOAT)m_pVar->GetFloat(_T("coefficientB"), szField, INVALID_VALUE);
//	pPosyProto->fcoefficientC = (FLOAT)m_pVar->GetFloat(_T("coefficientC"), szField, INVALID_VALUE);
//
//	for( INT n = 1; n <= MAX_CONSOLIDATE_STUFF_QUANTITY; ++n )
//	{
//		TCHAR szTemp[LONG_STRING];
//		wsprintf(szTemp, _T("stuff%d_id"), n);
//		pPosyProto->ConsolidateStuff[n - 1].dwStuffID = m_pVar->GetDword(szTemp, szField, INVALID_VALUE);
//		wsprintf(szTemp, _T("stuff%d_type"), n);
//		pPosyProto->ConsolidateStuff[n - 1].eStuffType = (EStuffType)m_pVar->GetDword(szTemp, szField, EST_Null);
//		wsprintf(szTemp, _T("stuff%d_num"), n);
//		pPosyProto->ConsolidateStuff[n - 1].dwStuffNum = m_pVar->GetDword(szTemp, szField, INVALID_VALUE);
//
//		if(pPosyProto->ConsolidateStuff[n - 1].dwStuffNum != INVALID_VALUE)
//			pPosyProto->nTotleStuff += pPosyProto->ConsolidateStuff[n - 1].dwStuffNum;
//	}
//
//	return &pPosyProto->dwID;
//}

//}


//-------------------------------------------------------------------------------------------
// 初始化镌刻的一条记录
//-------------------------------------------------------------------------------------------
//VOID* AttRes::InitOneEngraveProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy)
//{
//	MTRANS_POINTER(pEngraveProto, pProtoType, tagEngraveProtoSer);
//
//	pEngraveProto->dwID = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
//
//	_tcscpy( pEngraveProto->szName, m_pVar->GetString(_T("name"), szField, _T("")) );
//	pEngraveProto->eEngraveAtt = (EEngraveAtt)m_pVar->GetDword(_T("att"), szField, INVALID_VALUE);
//	pEngraveProto->byEngraveTimes = (BYTE)m_pVar->GetDword(_T("times"), szField, INVALID_VALUE);
//	pEngraveProto->nSuccessRate = m_pVar->GetDword(_T("success_rate"), szField, INVALID_VALUE);
//	pEngraveProto->nPotValConsume = m_pVar->GetDword(_T("potval_consume"), szField, 0);
//	pEngraveProto->dw_money_consume = m_pVar->GetDword(_T("money_consume"), szField, 0);
//	pEngraveProto->fcoefficientA = (FLOAT)m_pVar->GetFloat(_T("coefficientA"), szField, INVALID_VALUE);
//	pEngraveProto->fcoefficientB = (FLOAT)m_pVar->GetFloat(_T("coefficientB"), szField, INVALID_VALUE);
//	pEngraveProto->fcoefficientC = (FLOAT)m_pVar->GetFloat(_T("coefficientC"), szField, INVALID_VALUE);
//
//	for( INT n = 1; n <= MAX_CONSOLIDATE_STUFF_QUANTITY; ++n )
//	{
//		TCHAR szTemp[LONG_STRING];
//		wsprintf(szTemp, _T("stuff%d_id"), n);
//		pEngraveProto->ConsolidateStuff[n - 1].dwStuffID = m_pVar->GetDword(szTemp, szField, INVALID_VALUE);
//		wsprintf(szTemp, _T("stuff%d_type"), n);
//		pEngraveProto->ConsolidateStuff[n - 1].eStuffType = (EStuffType)m_pVar->GetDword(szTemp, szField, EST_Null);
//		wsprintf(szTemp, _T("stuff%d_num"), n);
//		pEngraveProto->ConsolidateStuff[n - 1].dwStuffNum = m_pVar->GetDword(szTemp, szField, INVALID_VALUE);
//
//		if(pEngraveProto->ConsolidateStuff[n - 1].dwStuffNum != INVALID_VALUE)
//			pEngraveProto->nTotleStuff += pEngraveProto->ConsolidateStuff[n - 1].dwStuffNum;
//	}
//
//	return &pEngraveProto->dwID;
//}

//-------------------------------------------------------------------------------------------
// 得到强化属性对应的装备是否可镌刻
//-------------------------------------------------------------------------------------------
BOOL AttRes::IsEngravePos(EEngraveAtt ePosyAtt, EEquipPos eEquipPos)
{
	for( INT n = 0; n < MAX_CONSOLIDATE_POS_QUANTITY; ++n)
	{
		if(m_EngravePos[ePosyAtt][n].ePos == eEquipPos)
			return (BOOL)m_EngravePos[ePosyAtt][n].byConsolidate;
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------------
// 初始化镶嵌
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneConsolidateProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pConsolidateProto, pProtoType, tagConsolidateItem);

	pConsolidateProto->dwID = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	
	//pConsolidateProto->dwPotValConsume = m_pVar->get_dword(_T("potval_consume"), szField, INVALID_VALUE);
	//pConsolidateProto->nSuccessRate = m_pVar->get_dword(_T("success_rate"), szField, INVALID_VALUE);
	
	pConsolidateProto->nMoney = m_pVar->get_int(_T("Money"), szField, 0);
	pConsolidateProto->nDamoMonery = m_pVar->get_int(_T("DamoMoney"), szField, 0);
	pConsolidateProto->dwLeftItem = m_pVar->get_dword(_T("LeftItem"), szField, 0);
	pConsolidateProto->dwDamoItem = m_pVar->get_dword(_T("DamoItem"), szField, 0);

	for( INT n = 0; n < MAX_CONSOLIDATE_ROLEATT; ++n )
	{
		TCHAR szTemp[LONG_STRING];
		wsprintf(szTemp, _T("RoleAtt%d"), n);
		pConsolidateProto->tagRoleAtt[n].eRoleAtt = (ERoleAttribute)m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		wsprintf(szTemp, _T("AttValue%d"), n);
		pConsolidateProto->tagRoleAtt[n].nAttVal = m_pVar->get_int(szTemp, szField, 0);
	}
	
	pConsolidateProto->ConsolidatePos[0].byConsolidate = (BYTE)m_pVar->get_dword(_T("Weapon"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[0].ePos = EEP_RightHand;
	pConsolidateProto->ConsolidatePos[1].byConsolidate = (BYTE)m_pVar->get_dword(_T("Head"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[1].ePos = EEP_Head;
	pConsolidateProto->ConsolidatePos[2].byConsolidate = (BYTE)m_pVar->get_dword(_T("Body"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[2].ePos = EEP_Body;
	pConsolidateProto->ConsolidatePos[3].byConsolidate = (BYTE)m_pVar->get_dword(_T("Feet"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[3].ePos = EEP_Feet;
	pConsolidateProto->ConsolidatePos[4].byConsolidate = (BYTE)m_pVar->get_dword(_T("Back"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[4].ePos = EEP_Feet;//EEP_Back;
	pConsolidateProto->ConsolidatePos[5].byConsolidate = (BYTE)m_pVar->get_dword(_T("Face"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[5].ePos = EEP_Feet;//EEP_Face;
	pConsolidateProto->ConsolidatePos[6].byConsolidate = (BYTE)m_pVar->get_dword(_T("Neck"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[6].ePos = EEP_Neck;
	pConsolidateProto->ConsolidatePos[7].byConsolidate = (BYTE)m_pVar->get_dword(_T("Finger1"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[7].ePos = EEP_Finger1;
	pConsolidateProto->ConsolidatePos[8].byConsolidate = (BYTE)m_pVar->get_dword(_T("Waist"), szField, INVALID_VALUE);
	pConsolidateProto->ConsolidatePos[8].ePos = EEP_Waist;

	return &pConsolidateProto->dwID;
}

//--------------------------------------------------------------------------------------------
// 初始化生产技能的一条记录
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneProduceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pProduceProto, nProtoType, s_produce_proto_ser);

	pProduceProto->dw_id = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	pProduceProto->e_pro_type = (EProduceType)m_pVar->get_dword(_T("ProduceType"), szField, INVALID_VALUE);
	pProduceProto->e_com_type = (EComposeType)m_pVar->get_dword(_T("ComposeType"), szField, INVALID_VALUE);
	pProduceProto->e_form_from = (EFormulaFrom)m_pVar->get_dword(_T("FormulaFrom"), szField, INVALID_VALUE);
	pProduceProto->n_form_levle= m_pVar->get_dword(_T("FormulaLevel"), szField, INVALID_VALUE);
	pProduceProto->dw_pro_item_data_id = m_pVar->get_dword(_T("ComposeItem"), szField, INVALID_VALUE);
	pProduceProto->dw_pro_quan_tity = m_pVar->get_dword(_T("ComposeNumber"), szField, INVALID_VALUE);
	pProduceProto->n_success_rate = m_pVar->get_dword(_T("SuccessRate"), szField, INVALID_VALUE);
	pProduceProto->dw_master_incre = m_pVar->get_dword(_T("ExpIncre"), szField, 0);
	pProduceProto->dw_money_consume = m_pVar->get_dword(_T("MoneyConsume"), szField, 0);
	pProduceProto->bBind  = m_pVar->get_int(_T("Bind"), szField, 1);

	for( INT n = 1; n <= MAX_PRODUCE_STUFF_QUANTITY; ++n )
	{
		TCHAR szTemp[LONG_STRING];
		wsprintf(szTemp, _T("StuffId%d"), n);
		pProduceProto->produce_stuff[n - 1].dwStuffID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		//wsprintf(szTemp, _T("stuff%d_type"), n);
		//pProduceProto->produce_stuff[n - 1].eStuffType = (EStuffType)m_pVar->get_dword(szTemp, szField, EST_Null);
		wsprintf(szTemp, _T("StuffNumber%d"), n);
		pProduceProto->produce_stuff[n - 1].dwStuffNum = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
	}
	
	for (INT i = 0; i < EIQ_End; i++)
	{
		TCHAR szTemp[LONG_STRING];
		_stprintf_s(szTemp, _countof(szTemp), _T("QualityNumber%d"), i);
		pProduceProto->nQualiyNum[i] = m_pVar->get_int(szTemp, szField, 0);
		_stprintf_s(szTemp, _countof(szTemp), _T("QualityPro%d"), i);
		pProduceProto->nQualiyPro[i] = m_pVar->get_int(szTemp, szField, 0);
	}

	return &pProduceProto->dw_id;
}

VOID* AttRes::InitOneEquipChangeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pProduceProto, nProtoType, tagEquipChange);

	pProduceProto->dwID = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pProduceProto->dwTargetID = m_pVar->get_dword(_T("target_equip"), szField, INVALID_VALUE);

	for (int i = 0; i < 3; i++)
	{
		TCHAR szTemp[LONG_STRING];
		_stprintf_s(szTemp, _countof(szTemp), _T("stuffID%d"), i+1);
		pProduceProto->sSutff[i].dwStuffID = m_pVar->get_int(szTemp, szField, 0);
		_stprintf_s(szTemp, _countof(szTemp), _T("stuffNum%d"), i+1);
		pProduceProto->sSutff[i].dwStuffNum = m_pVar->get_int(szTemp, szField, 0);
	}
	
	return &pProduceProto->dwID;
}
//--------------------------------------------------------------------------------------------
// 点化,通用分解
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneDeComposeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pDeComposeProto, nProtoType, s_decompose_proto_ser);

	pDeComposeProto->dw_id = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));

	pDeComposeProto->e_com_type = (EComposeType)m_pVar->get_dword(_T("ComposeType"), szField, INVALID_VALUE);
	pDeComposeProto->by_level = (BYTE)m_pVar->get_dword(_T("Level"), szField, INVALID_VALUE);
	pDeComposeProto->by_quality = (BYTE)m_pVar->get_dword(_T("Quality"), szField, INVALID_VALUE);
	pDeComposeProto->e_type = (EItemType)m_pVar->get_dword(_T("Type"), szField, INVALID_VALUE);
	pDeComposeProto->e_type_ex = (EItemTypeEx)m_pVar->get_dword(_T("Type_ex"), szField, INVALID_VALUE);
	pDeComposeProto->e_pos = (EEquipPos)m_pVar->get_dword(_T("EquipPos"), szField, INVALID_VALUE);
	pDeComposeProto->n_form_level = m_pVar->get_dword(_T("FormulaLevel"), szField, 0);
	pDeComposeProto->e_form_from = (EFormulaFrom)m_pVar->get_dword(_T("FormulaFrom"), szField, INVALID_VALUE);
	pDeComposeProto->dw_master_incre = m_pVar->get_dword(_T("MasterIncre"), szField, 0);
	pDeComposeProto->dw_money_consume = m_pVar->get_dword(_T("MoneyConsume"), szField, 0);
	
	pDeComposeProto->n_out_stuff_num = m_pVar->get_int(_T("OutStuffNumber"), szField, 0);

	for( INT n = 1; n <= pDeComposeProto->n_out_stuff_num; ++n )
	{
		tagOutputStuff outStuff;
		TCHAR szTemp[LONG_STRING];
		wsprintf(szTemp, _T("ItemId%d"), n);
		outStuff.dwStuffTypeID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		wsprintf(szTemp, _T("SucMinValue%d"), n);
		outStuff.nSucMinVal = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		wsprintf(szTemp, _T("SucMaxValue%d"), n);
		outStuff.nSucMaxVal = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		wsprintf(szTemp, _T("ItemRate%d"), n);
		outStuff.nRate = m_pVar->get_dword(szTemp, szField, 0);

		pDeComposeProto->output_stuff.push_back(outStuff);
	}

	pDeComposeProto->out_per_stuff.dwStuffTypeID = m_pVar->get_dword(_T("PreItemId"), szField, INVALID_VALUE);
	pDeComposeProto->out_per_stuff.nSucMinVal = m_pVar->get_dword(_T("PreSucMinValue"), szField, INVALID_VALUE);
	pDeComposeProto->out_per_stuff.nSucMaxVal = m_pVar->get_dword(_T("PreSucMaxValue"), szField, INVALID_VALUE);

	return &pDeComposeProto->dw_id;
}

//--------------------------------------------------------------------------------------------
// 淬火
//--------------------------------------------------------------------------------------------
//VOID* AttRes::InitOneQuenchProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
//{
//	MTRANS_POINTER(pQuenchProto, nProtoType, tagQuenchProtoSer);
//
//	pQuenchProto->dw_id = (DWORD)_atoi64(get_tool()->unicode_to_unicode8(szField));
//	pQuenchProto->srcQuenchAtt.eWuXing = (EWuXing)m_pVar->GetDword(_T("src_att"), szField, INVALID_VALUE);
//	pQuenchProto->srcQuenchAtt.nWuXingValue = m_pVar->GetDword(_T("src_att_value"), szField, 0);
//	pQuenchProto->dstQuenchAtt.eWuXing = (EWuXing)m_pVar->GetDword(_T("dst_att"), szField, INVALID_VALUE);
//	pQuenchProto->dstQuenchAtt.nWuXingValue = m_pVar->GetDword(_T("dst_att_value"), szField, 0);
//	pQuenchProto->dwPotValConsume = m_pVar->GetDword(_T("potval_consume"), szField, 0);
//	pQuenchProto->dw_money_consume = m_pVar->GetDword(_T("money_consume"), szField, 0);
//	pQuenchProto->nSuccessRate = m_pVar->GetDword(_T("success_rate"), szField, INVALID_VALUE);
//
//	for( INT n = 1; n <= MAX_CONSOLIDATE_STUFF_QUANTITY; ++n )
//	{
//		TCHAR szTemp[LONG_STRING];
//		wsprintf(szTemp, _T("stuff%d_id"), n);
//		pQuenchProto->ConsolidateStuff[n - 1].dwStuffID = m_pVar->GetDword(szTemp, szField, INVALID_VALUE);
//		wsprintf(szTemp, _T("stuff%d_type"), n);
//		pQuenchProto->ConsolidateStuff[n - 1].eStuffType = (EStuffType)m_pVar->GetDword(szTemp, szField, EST_Null);
//		wsprintf(szTemp, _T("stuff%d_num"), n);
//		pQuenchProto->ConsolidateStuff[n - 1].dwStuffNum = m_pVar->GetDword(szTemp, szField, 0);
//	}
//
//	return &pQuenchProto->dw_id;
//}

//--------------------------------------------------------------------------------------------
// 副本中不能使用的物品
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneInsItemProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pInsItem, nProtoType, tagInstanceItem);

	LPCTSTR szMapName = (LPCTSTR)m_pVar->get_string(_T("id"), szField);
	pInsItem->dwMapID = get_tool()->crc32(szMapName);

	INT nItemNum = m_pVar->get_dword(_T("ItemNum"), szField, 0);

	for( INT n = 1; n <= nItemNum; ++n)
	{
		TCHAR szTemp[LONG_STRING];
		wsprintf(szTemp, _T("ItemID%d"), n);
		DWORD	dwItemID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		pInsItem->mapInstanceItem.add(dwItemID, dwItemID);
	}

	return &pInsItem->dwMapID;
}

//--------------------------------------------------------------------------------------------
// 副本中不能使用的技能
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneInsSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pInsSkill, nProtoType, tagInstanceSkill);

	LPCTSTR szMapName = (LPCTSTR)m_pVar->get_string(_T("id"), szField);
	pInsSkill->dwMapID = get_tool()->crc32(szMapName);
	INT nItemNum = m_pVar->get_dword(_T("SkillNum"), szField, 0);

	for( INT n = 1; n <= nItemNum; ++n)
	{
		TCHAR szTemp[LONG_STRING];
		wsprintf(szTemp, _T("SkillID%d"), n);
		DWORD	dwSkillID = m_pVar->get_dword(szTemp, szField, INVALID_VALUE);
		pInsSkill->mapInstanceSkill.add(dwSkillID, dwSkillID);
	}

	return &pInsSkill->dwMapID;
}

// 成就
VOID* AttRes::InitOneAchievementProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pProto, nProtoType, AchievementEntry);

	pProto->m_ID = m_pVar->get_dword(_T("id"), szField, 0);
	pProto->m_refAchievement	= m_pVar->get_dword(_T("achievement"), szField, 0);
	pProto->m_point = m_pVar->get_dword(_T("point"), szField, 0);
	pProto->m_count = m_pVar->get_dword(_T("criteriaCount"), szField, 0);
	pProto->m_signet = (e_achievement_signet)m_pVar->get_int(_T("signet"), szField, 0);
	pProto->m_nTitleID = m_pVar->get_int(_T("title"), szField, 0);
	return &pProto->m_ID;
}

VOID* AttRes::InitOneAchievementCriteriaProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pProto, nProtoType, AchievementCriteriaEntry);

	pProto->m_ID = m_pVar->get_dword(_T("id"), szField, 0);
	pProto->m_referredAchievement = m_pVar->get_dword(_T("achievementID"), szField, 0);
	pProto->m_CondType = (e_condition_type)m_pVar->get_dword(_T("condtype"), szField, 0);
	pProto->m_Events = (e_achievement_event)m_pVar->get_dword(_T("event"), szField, 0);
	pProto->m_dwPara1 = (DWORD)m_pVar->get_dword(_T("Para1"), szField, INVALID_VALUE);
	pProto->m_dwPara2 = (DWORD)m_pVar->get_dword(_T("Para2"), szField, INVALID_VALUE);
	
	pProto->m_dwTimeLimit = (DWORD)m_pVar->get_dword(_T("Time_limit"), szField, 0);
	return &pProto->m_ID;
}
//--------------------------------------------------------------------------------------------
// 副本随机刷怪点
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pSpawnPoint, nProtoType, tagRandSpawnPointInfo);
	
	DWORD		dwSpawnPointID = m_pVar->get_dword(_T("SpawnPoint"), szField, INVALID_VALUE);
	INT			nLevel = m_pVar->get_dword(_T("Level"), szField, 0);

	pSpawnPoint->dwSpawnPointID = dwSpawnPointID + (DWORD)nLevel;
	pSpawnPoint->nLevel = nLevel;
	pSpawnPoint->dwNormalID[0] = m_pVar->get_dword(_T("NormalID1"), szField, INVALID_VALUE);
	pSpawnPoint->dwNormalID[1] = m_pVar->get_dword(_T("NormalID2"), szField, INVALID_VALUE);
	pSpawnPoint->dwNormalID[2] = m_pVar->get_dword(_T("NormalID3"), szField, INVALID_VALUE);
	pSpawnPoint->dwEliteID[0] = m_pVar->get_dword(_T("EliteID1"), szField, INVALID_VALUE);
	pSpawnPoint->dwEliteID[1] = m_pVar->get_dword(_T("EliteID2"), szField, INVALID_VALUE);
	pSpawnPoint->dwEliteID[2] = m_pVar->get_dword(_T("EliteID3"), szField, INVALID_VALUE);
	pSpawnPoint->dwDevilID[0] = m_pVar->get_dword(_T("DevilIDl"), szField, INVALID_VALUE);
	pSpawnPoint->dwDevilID[1] = m_pVar->get_dword(_T("DevilID2"), szField, INVALID_VALUE);
	pSpawnPoint->dwDevilID[2] = m_pVar->get_dword(_T("DevilID3"), szField, INVALID_VALUE);
	
	return &pSpawnPoint->dwSpawnPointID;
}

//--------------------------------------------------------------------------------------------
// 副本动态刷怪点等级映射表
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneLevelMapping(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pLevelMapping, nProtoType, tagLevelMapping);

	pLevelMapping->nLevel = (INT32)_atoi64(get_tool()->unicode_to_unicode8(szField));
	pLevelMapping->nTransmitLevel = m_pVar->get_dword(_T("Level"), szField, 0);

	return &pLevelMapping->nLevel;	
}

//--------------------------------------------------------------------------------------------
// 初始化副本静态数据
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneInstanceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pInstance, nProtoType, tagInstance);

	LPCTSTR szMapName = (LPCTSTR)m_pVar->get_string(_T("id"), szField);
	pInstance->dwMapID = get_tool()->crc32(szMapName);
	pInstance->dwEndTime = m_pVar->get_dword(_T("stop_time"), szField, INVALID_VALUE);
	pInstance->dwTimeLimit = m_pVar->get_dword(_T("time_limit"), szField, INVALID_VALUE);
	pInstance->eInstanceMapType = (EInstanceMapType)m_pVar->get_dword(_T("instance_type"), szField, 0);
	pInstance->eInstanceCreateMode = (EInstanceCreateMode)m_pVar->get_dword(_T("instance_crate_mode"), szField, 0);
	pInstance->bAskEnter = ((BOOL)m_pVar->get_dword(_T("enter_prompt"), szField) ? true : false);
	pInstance->bSelectHard = ((BOOL)m_pVar->get_dword(_T("can_select_hard"), szField) ? true : false);
	pInstance->bSelectNormal = ((BOOL)m_pVar->get_dword(_T("can_select_normal"), szField) ? true : false);
	pInstance->bSelectElite = ((BOOL)m_pVar->get_dword(_T("can_select_elite"), szField) ? true : false);
	pInstance->bSelectDevil = ((BOOL)m_pVar->get_dword(_T("can_select_devil"), szField) ? true : false);
	pInstance->bNoticeTeamate = ((BOOL)m_pVar->get_dword(_T("notice_teamate"), szField) ? true : false);
	pInstance->nNumDownLimit = m_pVar->get_dword(_T("number_min_limit"), szField, 0);
	pInstance->nNumUpLimit = m_pVar->get_dword(_T("number_max_limit"), szField, INVALID_VALUE);
	pInstance->nLevelDownLimit = m_pVar->get_dword(_T("level_min_limit"), szField, 0);
	pInstance->nLevelUpLimit = m_pVar->get_dword(_T("level_max_limit"), szField, 0);
	pInstance->nLevelEliteDownLimit = m_pVar->get_dword(_T("level_min_limit_elite"), szField, 0);
	pInstance->nLevelEliteUpLimit = m_pVar->get_dword(_T("level_max_limit_elite"), szField, 0);
	pInstance->nLevelDevilDownLimit = m_pVar->get_dword(_T("level_min_limit_devil"), szField, 0);
	pInstance->nLevelDevilUpLimit = m_pVar->get_dword(_T("level_max_limit_devil"), szField, 0);
	pInstance->eExportMode = (EExportMode)m_pVar->get_dword(_T("export_mode"), szField, 0);
	pInstance->byConsolidateLevelMin = m_pVar->get_dword(_T("ride_consolidate_level_min"), szField, 0);

	pInstance->eInstanceEnterLimit = (EEnterLimit)m_pVar->get_dword(_T("enter_limit"), szField, EEL_NULL);
	pInstance->dwEnterNumLimit		= m_pVar->get_dword(_T("enter_times_limit"), szField, 0);
	pInstance->bClearNumLimit		= m_pVar->get_dword(_T("clear_enter_limit"), szField, 0);
	pInstance->nIndex				= m_pVar->get_int(_T("single_index"), szField, -1);

	pInstance->dwItemID		= m_pVar->get_dword(_T("enter_item"), szField, INVALID_VALUE);
	pInstance->dwLgnoreItemID = m_pVar->get_dword(_T("lgnore_item"), szField, INVALID_VALUE);

	pInstance->nProcessYB = m_pVar->get_int(_T("reset_process_yb"), szField, 0);
	pInstance->nResetLimitYB = m_pVar->get_int(_T("reset_limit_yb"), szField, 0);

	LPCTSTR	szExportMapName = (LPCTSTR)m_pVar->get_string(_T("export_map"), szField);
	pInstance->dwExportMapID = get_tool()->crc32(szExportMapName);

	LPCTSTR	cooldownposname = (LPCTSTR)m_pVar->get_string(_T("cooldown_revive"), szField, NULL);
	pInstance->coolDownReviveID = cooldownposname ? get_tool()->crc32(cooldownposname) : (INVALID_VALUE);

	LPCTSTR szTemp = (LPCTSTR)m_pVar->get_string(_T("export_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pInstance->dwExportWayPoint = INVALID_VALUE;
	}
	else
	{
		pInstance->dwExportWayPoint = get_tool()->crc32(szTemp);
	}

	szTemp = (LPCTSTR)m_pVar->get_string(_T("enter_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pInstance->dwEnterWayPoint = INVALID_VALUE;
	}
	else
	{
		pInstance->dwEnterWayPoint = get_tool()->crc32(szTemp);
	}

	szTemp = (LPCTSTR)m_pVar->get_string(_T("enemy_enter_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pInstance->dwEnemyEnterPoint = INVALID_VALUE;
	}
	else
	{
		pInstance->dwEnemyEnterPoint = get_tool()->crc32(szTemp);
	}

	szTemp = (LPCTSTR)m_pVar->get_string(_T("flag_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pInstance->dw_guild_flag = INVALID_VALUE;
	}
	else
	{
		pInstance->dw_guild_flag = get_tool()->crc32(szTemp);
	}
	/*pInstance->vExportPos.x= m_pVar->get_float(_T("x"), szField, 0);
	pInstance->vExportPos.y = m_pVar->get_float(_T("y"), szField, 0);
	pInstance->vExportPos.z = m_pVar->get_float(_T("z"), szField, 0);
	pInstance->vEnterPos.x = m_pVar->get_float(_T("EnterX"), szField, 0);
	pInstance->vEnterPos.y = m_pVar->get_float(_T("EnterY"), szField, 0);
	pInstance->vEnterPos.z = m_pVar->get_float(_T("EnterZ"), szField, 0);
	pInstance->vEnemyEnterPos.x = m_pVar->get_float(_T("EnemyEnterX"), szField, 0);
	pInstance->vEnemyEnterPos.y = m_pVar->get_float(_T("EnemyEnterY"), szField, 0);
	pInstance->vEnemyEnterPos.z = m_pVar->get_float(_T("EnemyEnterZ"), szField, 0);*/
	pInstance->dwTargetLimit = m_pVar->get_dword(_T("target_limit"), szField, 0);
	pInstance->bCombat = ((BOOL)m_pVar->get_dword(_T("can_combat"), szField) ? true : false);
	pInstance->bPK = ((BOOL)m_pVar->get_dword(_T("can_pk"), szField) ? true : false);
	pInstance->bLoseSafeguard = false; //((BOOL)m_pVar->get_dword(_T("IsloseSafeGuard"), szField) ? true : false);
	pInstance->bPKPenalty = ((BOOL)m_pVar->get_dword(_T("pk_penalty"), szField) ? true : false);
	pInstance->bDeadPenalty = ((BOOL)m_pVar->get_dword(_T("dead_penalty"), szField) ? true : false);
	pInstance->bMount = ((BOOL)m_pVar->get_dword(_T("can_mount"), szField) ? true : false);
	pInstance->bTransmit = ((BOOL)m_pVar->get_dword(_T("can_tranmit"), szField) ? true : false);
	pInstance->eRebornMode = (ERebornMode)m_pVar->get_dword(_T("reborn_mode"), szField, 0);
	pInstance->eCompleteNor = (ECompleteConditionNor)m_pVar->get_dword(_T("complete_condition_normal"), szField, 0);
	pInstance->dwCompleteNorVal1 = m_pVar->get_dword(_T("complete_normal_value1"), szField, INVALID_VALUE);
	pInstance->dwCompleteNorVal2 = m_pVar->get_dword(_T("complete_normal_value2"), szField, INVALID_VALUE);
	pInstance->eCompleteEli = (ECompleteConditionEli)m_pVar->get_dword(_T("complete_condition_elite"), szField, 0);
	pInstance->dwCompleteEliVal1 = m_pVar->get_dword(_T("complete_elite_value1"), szField, INVALID_VALUE);
	pInstance->dwCompleteEliVal2 = m_pVar->get_dword(_T("complete_elite_value2"), szField, INVALID_VALUE);
	pInstance->eCompleteDev = (ECompleteConditionDev)m_pVar->get_dword(_T("complete_condition_devil"), szField, 0);
	pInstance->dwCompleteDevVal1 = m_pVar->get_dword(_T("complete_devil_value1"), szField, INVALID_VALUE);
	pInstance->dwCompleteDevVal2 = m_pVar->get_dword(_T("complete_devil_value2"), szField, INVALID_VALUE);
	pInstance->eCompleteEvent = (ECompleteEvent)m_pVar->get_dword(_T("complete_event"), szField, 0);
	pInstance->nActID = m_pVar->get_int(_T("activity_id"), szField, 0);

	for(INT i = 0; i < MAX_GUILD_RELIVE_NUM; i++)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("relive_position%d"), i + 1);
		szTemp = (LPCTSTR)m_pVar->get_string(pTchTmp, szField, NULL);
		if(szTemp == NULL)
		{
			pInstance->dwRelivePos[i] = INVALID_VALUE;
		}
		else
		{
			pInstance->dwRelivePos[i] = get_tool()->crc32(szTemp);
		}
		
	}

	return &pInstance->dwMapID;
}

//--------------------------------------------------------------------------------------------
// 初始化非副本刷怪点原型
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneSSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pSSpawnPoint, nProtoType, tagSSpawnPointProto);

	pSSpawnPoint->dwSpawnPointID = (DWORD)m_pVar->get_dword(_T("id"), szField);

	for (INT nCreatureIndex = 0; nCreatureIndex < MAX_CREATURE_PER_SSPAWNPOINT; ++nCreatureIndex)
	{
		TCHAR pTchTmp[LONG_STRING];
		wsprintf(pTchTmp, _T("creature_id_%d"), nCreatureIndex + 1);
		pSSpawnPoint->dwTypeIDs[nCreatureIndex] = (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);
	}

	return &pSSpawnPoint->dwSpawnPointID;
}

VOID* AttRes::InitOneSSpawnGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pSSpawnPoint, nProtoType, tagSSpawnPointProto);

	pSSpawnPoint->dwSpawnPointID = (DWORD)m_pVar->get_dword(_T("id"), szField);

	for (INT nCreatureIndex = 0; nCreatureIndex < MAX_CREATURE_PER_SSPAWNPOINT; ++nCreatureIndex)
	{
		TCHAR pTchTmp[LONG_STRING];
		wsprintf(pTchTmp, _T("creature_id_%d"), nCreatureIndex + 1);
		pSSpawnPoint->dwTypeIDs[nCreatureIndex] = (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);
	}

	return &pSSpawnPoint->dwSpawnPointID;
}

VOID* AttRes::InitOneSSpawnGuildGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pSSpawnPoint, nProtoType, tagGuildSSpawnPointProto);

	pSSpawnPoint->dw_creature_id = (DWORD)m_pVar->get_dword(_T("id"), szField);

	for (INT nGuildLevel = 0; nGuildLevel < MAX_GUILD_LEVEL; ++nGuildLevel)
	{
		TCHAR pTchTmp[LONG_STRING];
		wsprintf(pTchTmp, _T("levle_pro_%d"), nGuildLevel + 1);
		pSSpawnPoint->n_pro[nGuildLevel] = (DWORD)m_pVar->get_dword(pTchTmp, szField, 0);
	}

	return &pSSpawnPoint->dw_creature_id;
}
//--------------------------------------------------------------------------------------------
// 初始化宠物技能原型
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOnePetSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pPetSkillProto, nProtoType, tagPetSkillProto);

	pPetSkillProto->dw_data_id		= (DWORD)			m_pVar->get_dword(_T("id"), szField);
	//pPetSkillProto->dwSkillTypeID	= m_pVar->get_dword(_T("skill_id"), szField, INVALID_VALUE);
	pPetSkillProto->eType			= (EPetskillType)	m_pVar->get_int(_T("Type1"), szField, EPT2_Null);
	pPetSkillProto->eType2			= (EPetskillType2)	m_pVar->get_int(_T("Type2"), szField, 2);
	pPetSkillProto->nType3			= m_pVar->get_int(_T("Type3"), szField, 0);
	pPetSkillProto->eCastType		= (EPetskillCastType)	m_pVar->get_int(_T("CastType"), szField, 0);
	pPetSkillProto->nSkillLevel		= m_pVar->get_int(_T("Level"), szField, 0);
	pPetSkillProto->byCast_condition	= (BYTE)	m_pVar->get_int(_T("CastCondition"), szField, INVALID_VALUE);
	pPetSkillProto->nCooldownTick	= (INT)(m_pVar->get_int(_T("CooldownTime"), szField, 0) / TICK_TIME);
	pPetSkillProto->nWorkTimeTick	= (INT)(m_pVar->get_int(_T("WorkTime"), szField, 0) / TICK_TIME);
	pPetSkillProto->nWuxing_cost	= m_pVar->get_int(_T("WuxingCost"), szField, 0);
	pPetSkillProto->nSpirit_cost	= m_pVar->get_int(_T("SpiritCost"), szField, 0);
	pPetSkillProto->nWuxing_add		= m_pVar->get_int(_T("WuxingAdd"), szField, 0);
	pPetSkillProto->nBuffid			= m_pVar->get_int(_T("BuffId"), szField, INVALID_VALUE);
	pPetSkillProto->bLearn_condition= (BOOL)m_pVar->get_int(_T("LearnCondition"), szField, 0);
	pPetSkillProto->b_cantforget		= (BOOL)m_pVar->get_int(_T("cantForget"), szField, 0);
	pPetSkillProto->nLearn_prob		= m_pVar->get_int(_T("LearnProb"), szField, 10000);
	//pPetSkillProto->nLearn_step		= m_pVar->get_int(_T("learn_step"), szField, 0);
	//pPetSkillProto->nLearn_grade	= m_pVar->get_int(_T("learn_grade"), szField, 0);
	pPetSkillProto->nLearn_Level	= m_pVar->get_int(_T("LearnLevel"), szField, 1);
	pPetSkillProto->nLearn_PontentialCost	= m_pVar->get_int(_T("PotentialCost"), szField, 0);

	pPetSkillProto->nPetLvlLim		= m_pVar->get_int(_T("LearnRoleLevel"), szField, INVALID_VALUE);

	pPetSkillProto->AttIndexs[0]	= (INT)m_pVar->get_int(_T("RoleAtt1"), szField, INVALID_VALUE);
	pPetSkillProto->AttIndexs[1]	= (INT)m_pVar->get_int(_T("RoleAtt2"), szField, INVALID_VALUE);
	pPetSkillProto->AttIndexs[2]	= (INT)m_pVar->get_int(_T("RoleAtt3"), szField, INVALID_VALUE);
	pPetSkillProto->AttIndexs[3]	= (INT)m_pVar->get_int(_T("RoleAtt4"), szField, INVALID_VALUE);
	pPetSkillProto->AttIndexs[4]	= (INT)m_pVar->get_int(_T("RoleAtt5"), szField, INVALID_VALUE);
	pPetSkillProto->AttIndexs[5]	= (INT)m_pVar->get_int(_T("RoleAtt6"), szField, INVALID_VALUE);

	pPetSkillProto->AttMods[0]		= (INT)m_pVar->get_int(_T("Mod1"), szField, 0);
	pPetSkillProto->AttMods[1]		= (INT)m_pVar->get_int(_T("Mod2"), szField, 0);
	pPetSkillProto->AttMods[2]		= (INT)m_pVar->get_int(_T("Mod3"), szField, 0);
	pPetSkillProto->AttMods[3]		= (INT)m_pVar->get_int(_T("Mod4"), szField, 0);
	pPetSkillProto->AttMods[4]		= (INT)m_pVar->get_int(_T("Mod5"), szField, 0);
	pPetSkillProto->AttMods[5]		= (INT)m_pVar->get_int(_T("Mod6"), szField, 0);

	pPetSkillProto->dwTriggerID		= m_pVar->get_dword(_T("TriggerID"), szField, INVALID_VALUE);
	pPetSkillProto->dwBufferID		= m_pVar->get_dword(_T("BufferID"), szField, INVALID_VALUE);

	pPetSkillProto->nPetAttIndex	= (INT)m_pVar->get_int(_T("PetAtt"), szField, INVALID_VALUE);
	pPetSkillProto->nPetAttMod		= (INT)m_pVar->get_int(_T("PetMod"), szField, 0);

	return &pPetSkillProto->dw_data_id;

}

//--------------------------------------------------------------------------------------------
// 初始化宠物技能原型
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOnePetEquipProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetEquipProto, nProtoType, tagPetEquipProto);

	pPetEquipProto->dw_data_id	= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	pPetEquipProto->nType			= (INT)m_pVar->get_int(_T("type"), szField, INVALID_VALUE);
	pPetEquipProto->nStep			= (INT)m_pVar->get_int(_T("step"), szField, INVALID_VALUE);
	pPetEquipProto->nGrade			= (INT)m_pVar->get_int(_T("grade"), szField, INVALID_VALUE);

	pPetEquipProto->bTypeUnique		= (BOOL)m_pVar->get_dword(_T("unique_same_type"), szField, FALSE);
	pPetEquipProto->bUnique			= (BOOL)m_pVar->get_dword(_T("unique"), szField, FALSE);

	pPetEquipProto->nPetAtt[0]		= (INT)m_pVar->get_int(_T("att1"), szField, INVALID_VALUE);
	pPetEquipProto->nPetAtt[1]		= (INT)m_pVar->get_int(_T("att2"), szField, INVALID_VALUE);
	pPetEquipProto->nPetAtt[2]		= (INT)m_pVar->get_int(_T("att3"), szField, INVALID_VALUE);

	pPetEquipProto->nPetAttMod[0]	= (INT)m_pVar->get_int(_T("mod1"), szField, 0);
	pPetEquipProto->nPetAttMod[1]	= (INT)m_pVar->get_int(_T("mod2"), szField, 0);
	pPetEquipProto->nPetAttMod[2]	= (INT)m_pVar->get_int(_T("mod3"), szField, 0);

	return &pPetEquipProto->dw_data_id;

}
//--------------------------------------------------------------------------------------------
// 初始化宠物随机技能
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOnePetRandomSkill( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pProto, nProtoType, s_pet_skill_list);

	pProto->dw_id		= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	INT nNormalNum = m_pVar->get_int(_T("NormalSkillNumber"), szField, 0);
	INT nBuffNum = m_pVar->get_int(_T("BuffSkillNumber"), szField, 0);
	INT nSpecNum = m_pVar->get_int(_T("SpecialSkillNumber"), szField, 0);
	INT nActionNum = m_pVar->get_int(_T("ActionSkillNumber"), szField, 0);

	TCHAR szTemp1[X_SHORT_NAME] = _T("");
	for (int i = 0; i < nNormalNum; i++)
	{
		_stprintf(szTemp1, _T("%s%d"), _T("NormalSkillID"), i+1);
		DWORD id = m_pVar->get_dword(szTemp1, szField, 0);
		pProto->vec_normal.push_back(id);
	}
	
	for (int i = 0; i < nBuffNum; i++)
	{
		_stprintf(szTemp1, _T("%s%d"), _T("BuffSkillID"), i+1);
		DWORD id = m_pVar->get_dword(szTemp1, szField, 0);
		pProto->vec_buff.push_back(id);
	}

	for (int i = 0; i < nSpecNum; i++)
	{
		_stprintf(szTemp1, _T("%s%d"), _T("SpecialSkillID"), i+1);
		DWORD id = m_pVar->get_dword(szTemp1, szField, 0);
		pProto->vec_specialty.push_back(id);
	}
	for (int i = 0; i < nActionNum; i++)
	{
		_stprintf(szTemp1, _T("%s%d"), _T("ActionSkillID"), i+1);
		DWORD id = m_pVar->get_dword(szTemp1, szField, 0);
		pProto->vec_Action.push_back(id);
	}

	//for (int i = 0;i < PET_RANDOM_SKILL_NUM; i++)
	//{
	//	_stprintf(szTemp1, _T("%s%d"), _T("skill_id_"), i+1);
	//	pProto->dwSkill[i] = m_pVar->get_dword(szTemp1, szField, INVALID_VALUE);
	//}

	//
	//for (int i = 0; i < PET_RANDOM_Specialty; i++)
	//{
	//	_stprintf(szTemp2, _T("%s%d"), _T("techang_"), i+1);
	//	pProto->dwSpecialty[i] = m_pVar->get_dword(szTemp2, szField, INVALID_VALUE);
	//}
	
	return &pProto->dw_id;
}

//--------------------------------------------------------------------------------------------
// 初始化宠物技能原型
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOnePetProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetProto, nProtoType, tagPetProto);

	pPetProto->dw_data_id		= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	if (!VALID_VALUE(pPetProto->dw_data_id))
	{
		ASSERT(0);
		m_att_res_caution(FileName_Pet_Proto, _T("invalide pet protoid"), pPetProto->dw_data_id);
	}

	pPetProto->nType3		= (INT)m_pVar->get_int(_T("Type3"), szField, 0);
	pPetProto->nRoleLvlLim	= (INT)m_pVar->get_int(_T("RoleLevel"), szField, 0);
	pPetProto->nMountable	= (INT)m_pVar->get_int(_T("Mountable"), szField, 0);
	pPetProto->nMountSpeed	= (INT)m_pVar->get_int(_T("MountSpeed"), szField, 0);
	pPetProto->dwFoodLimit	= m_pVar->get_dword(_T("FoodLimit"), szField, 7);

	pPetProto->fScale		= (FLOAT)m_pVar->get_float(_T("Scale"), szField, 10000) / 10000.0f;

	pPetProto->vSize.x	= m_pVar->get_float(_T("BoxX"), szField, X_DEF_ROLE_SIZE_X);
	pPetProto->vSize.y	= m_pVar->get_float(_T("BoxY"), szField, X_DEF_ROLE_SIZE_Y);
	pPetProto->vSize.z	= m_pVar->get_float(_T("BoxZ"), szField, X_DEF_ROLE_SIZE_Z);

	pPetProto->bBind	= m_pVar->get_dword(_T("Bind"), szField, FALSE);
	
	pPetProto->dwSkillListID = m_pVar->get_dword(_T("SkillList"), szField, INVALID_VALUE);

	for(INT32 i = 0; i < MOUNT_SKILL_NUM; i++)
	{
		TCHAR szTmp[64] = {0};
		_stprintf(szTmp, _T("MountSkillID%d"), i);
		pPetProto->dwMountSkillID[i]	= m_pVar->get_dword(szTmp, szField, 0);
		if(pPetProto->dwMountSkillID[i] == 0) break;
	}

	
	//for (INT nIndex = EIQ_Quality0; nIndex < EIQ_End; ++nIndex)
	//{
	//	TCHAR pTchTmp[LONG_STRING];

	//	wsprintf(pTchTmp, _T("aptitude%d_min"), nIndex + 1);
	//	pPetProto->nAptitudeMin[nIndex]		= (DWORD)m_pVar->get_dword(pTchTmp, szField, 0);

	//	wsprintf(pTchTmp, _T("aptitude%d_max"), nIndex + 1);
	//	pPetProto->nAptitudeMax[nIndex]		= (DWORD)m_pVar->get_dword(pTchTmp, szField, 0);
	//}


	return &pPetProto->dw_data_id;

}

//-------------------------------------------------------------------------------------------
// 学习等级比较函数对象
//-------------------------------------------------------------------------------------------
class PSidComp
{
public:
	PSidComp(AttRes* pRes):m_pRes(pRes){}
	bool operator()(const DWORD& left, const DWORD& right)
	{
		return m_pRes->GetPetSkillProto(left)->nLearn_prob < m_pRes->GetPetSkillProto(right)->nLearn_prob;
	}
private:
	AttRes*	m_pRes;
};

//-------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitPetSkillsVec()
{
	package_map<DWORD, tagPetSkillProto*>::map_iter itr =  m_mapPetSkillProto.begin();
	
	tagPetSkillProto* pProto = NULL;

	while(m_mapPetSkillProto.find_next(itr, pProto))
	{
		if (!VALID_POINT(pProto)) break;
		INT nVLvl = 1;
		nVLvl = pProto->nLearn_Level;
		//TransStepGrade2VLevel(pProto->nLearn_step, pProto->nLearn_grade, nVLvl);
		if (nVLvl > NUM_PET_VLEVEL)
		{
			ASSERT("PetSkillLevel ERROR!!");
			continue;
		}
		m_PetLevelSkillVec[nVLvl - 1].push_back(pProto->dw_data_id);
	}

	for (INT i = 0; i < NUM_PET_VLEVEL; ++i)
	{
		m_PetLevelSkillVec[i].sort(PSidComp(AttRes::GetInstance()));
	//	std::sort(m_PetLevelSkillVec[i].begin(), m_PetLevelSkillVec[i].end(), );
	}
	

	return NULL;

}

//-------------------------------------------------------------------------------------
// 加载宠物属性默认值、最小及最大值
//-------------------------------------------------------------------------------------
// VOID AttRes::InitPetAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
// {
// 	tagPetDefMinMax* pAttDefMinMax = (tagPetDefMinMax*)pArray;
// 
// 	pAttDefMinMax[nIndex].nDef		=	m_pVar->get_int(_T("def"), szField, 0);
// 	pAttDefMinMax[nIndex].nMin		=	m_pVar->get_int(_T("min"), szField, INT_MIN);
// 	pAttDefMinMax[nIndex].nMax		=	m_pVar->get_int(_T("max"), szField, INT_MAX);
// }


VOID* AttRes::InitOnePetLvlUpProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetLvlUp, nProtoType, tagPetLvlUpProto);

	pPetLvlUp->dwVLevel	= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	pPetLvlUp->nStep		= (INT)m_pVar->get_dword(_T("Step"), szField, 0);
	pPetLvlUp->nGrade		= (INT)m_pVar->get_dword(_T("Grade"), szField, 0);

	pPetLvlUp->nExpLvlUpNeed			= (INT)m_pVar->get_dword(_T("Experience"), szField, 0);
	pPetLvlUp->nMoneyRatePourExpNeed	= (INT)m_pVar->get_dword(_T("NeedMoney"), szField, 0);

	INT nVLevel = 0;
	TransStepGrade2VLevel(pPetLvlUp->nStep, pPetLvlUp->nGrade, nVLevel);

	ASSERT(nVLevel == pPetLvlUp->dwVLevel);

	return &pPetLvlUp->dwVLevel;
}

VOID* AttRes::InitOnePetGatherProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetGather, nProtoType, tagPetGatherProto);

	pPetGather->dw_data_id	= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	for (INT nIndex = 0; nIndex < NUM_ITEMS_PET_GATHER; ++nIndex)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("item_id%d"), nIndex + 1);
		pPetGather->dwItemTypeID[nIndex]	= (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);

		wsprintf(pTchTmp, _T("min%d"), nIndex + 1);
		pPetGather->nMin[nIndex]		= (DWORD)m_pVar->get_int(pTchTmp, szField, 0);

		wsprintf(pTchTmp, _T("max%d"), nIndex + 1);
		pPetGather->nMax[nIndex]		= (DWORD)m_pVar->get_int(pTchTmp, szField, 0);
	}

	for (INT nIndex = 0; nIndex < NUM_RARE_ITEMS_PET_GATHER; ++nIndex)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("rare_item_id%d"), nIndex + 1);
		pPetGather->dwRareItemID[nIndex]	= (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);

		wsprintf(pTchTmp, _T("prob%d"), nIndex + 1);
		pPetGather->nProb[nIndex]		= (DWORD)m_pVar->get_int(pTchTmp, szField, 0);
	}


	return &pPetGather->dw_data_id;
}

VOID* AttRes::InitOneWeatherProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pWeatherProto, nProtoType, tagWeatherProto);
	
	pWeatherProto->dwID = (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	return &pWeatherProto->dwID;
}

//-------------------------------------------------------------------------------------------
// 初始化VIP摊位租金
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneVIPStallProto( OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex )
{
	INT32* pRent = (INT32*)pArray;

	pRent[nIndex] = m_pVar->get_int(_T("RentNumber"), szField, 0);

	return NULL;
}

//-------------------------------------------------------------------------------------------
// 初始化公式参数
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneFormulaParamProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex)
{
	tagFormulaParam* p = (tagFormulaParam*)pArray;
	p->n16ConsolidateBaseSilver = (INT16)m_pVar->get_dword(_T("ConsolidateBaseSilver"), szField, 0);
	p->n16ChiselBaseSilver = (INT16)m_pVar->get_dword(_T("ChiselBaseSilver"), szField, 0);
	p->n16InlayBaseSilver = (INT16)m_pVar->get_dword(_T("InlayBaseSilver"), szField, 0);
	p->n16UnbesetBaseSilver = (INT16)m_pVar->get_dword(_T("UnbesetBaseSilver"), szField, 0);
	
	return NULL;
}

//-------------------------------------------------------------------------------------------
// 初始化新手礼包
//-------------------------------------------------------------------------------------------
VOID* AttRes::InitOneNewRoleGiftProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(p, nProtoType, s_new_role_gift_proto);

	p->n_id_ = (INT16)m_pVar->get_int(_T("id"), szField);

	for(INT i = 0; i < MAX_GIFT_NUM; i++)
	{
		TCHAR pTchTmp[SHORT_STRING];

		wsprintf(pTchTmp, _T("gift%d"), i + 1);
		p->dw_gift_id_[i] = (DWORD)m_pVar->get_dword(pTchTmp, szField);

		wsprintf(pTchTmp, _T("time%d"), i + 1);
		p->dw_time_[i] = (DWORD)m_pVar->get_dword(pTchTmp, szField, 0);

		wsprintf(pTchTmp, _T("number%d"), i + 1);
		p->dw_gift_num[i] = (DWORD)m_pVar->get_dword(pTchTmp, szField, 0);
	}

	return &p->n_id_;
}

VOID* AttRes::InitOnePetWuXingProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetWuXing, nProtoType, tagPetWuXingProto);

	pPetWuXing->dw_data_id	= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);

	for (INT nIndex = 0; nIndex < MAX_WUXING_ITEM_NUM; ++nIndex)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("item%d"), nIndex + 1);
		pPetWuXing->dwItemTypeID[nIndex]	= (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);

		wsprintf(pTchTmp, _T("num%d"), nIndex + 1);
		pPetWuXing->n_num[nIndex]		= m_pVar->get_int(pTchTmp, szField, 0);

		wsprintf(pTchTmp, _T("prob%d"), nIndex + 1);
		pPetWuXing->nProb[nIndex]		= m_pVar->get_int(pTchTmp, szField, 0);

		wsprintf(pTchTmp, _T("notice%d"), nIndex + 1);
		pPetWuXing->bNotice[nIndex]		= (BOOL)m_pVar->get_dword(pTchTmp, szField, 0);
	}

	return &pPetWuXing->dw_data_id;
}

VOID* AttRes::InitOneVNBGiftProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pVNBGiftProto, nProtoType, s_vnb_gift_proto);

	pVNBGiftProto->dw_id			= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pVNBGiftProto->dw_item_type_id	= (DWORD)m_pVar->get_dword(_T("typeid"), szField, INVALID_VALUE);
	pVNBGiftProto->n_num			= (INT)m_pVar->get_dword(_T("num"), szField, 0);

	if (!VALID_VALUE(pVNBGiftProto->dw_id))
	{
		//m_att_res_caution(_T("vip_netbar_gift_proto.xml"), _T("invalid id"), INVALID_VALUE);		// 屏蔽不用表格    added by gtj  11-01-25
	}
	
	return &pVNBGiftProto->dw_id;
}

VOID* AttRes::InitOneVNBEquipProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pVNBEquipProto, nProtoType, s_vnb_equip_proto);

	pVNBEquipProto->dw_id			= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pVNBEquipProto->dw_equip_type_id	= (DWORD)m_pVar->get_dword(_T("typeid"), szField, INVALID_VALUE);
	pVNBEquipProto->n_quality		= (INT)m_pVar->get_dword(_T("quality"), szField, 0);

	if (!VALID_VALUE(pVNBEquipProto->dw_id))
	{
		//m_att_res_caution(_T("vip_netbar_equip_proto.xml"), _T("invalid id"), INVALID_VALUE);		// 屏蔽不用表格    added by gtj  11-01-25
	}

	return &pVNBEquipProto->dw_id;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派设施位置信息
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildGradePosProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	BYTE byType		= (BYTE)m_pVar->get_dword(_T("Type"), szField, INVALID_VALUE);
	BYTE byLevel	= (BYTE)m_pVar->get_dword(_T("Level"), szField, 0);

	M_trans_pointer(pGradePos, nProtoType, s_guild_grade_pos);

	pGradePos->dw_key_ = byType - 1;
	pGradePos->dw_key_ = (pGradePos->dw_key_ << 16) | byLevel;
	pGradePos->dw_creature_type_id_ = m_pVar->get_dword(_T("CreatureTypeID"), szField, INVALID_VALUE);

	LPCTSTR szTemp = (LPCTSTR)m_pVar->get_string(_T("creature_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pGradePos->dw_creature_way_pos = INVALID_VALUE;
	}
	else
	{
		pGradePos->dw_creature_way_pos = get_tool()->crc32(szTemp);
	}

	pGradePos->dw_npc_type_id = m_pVar->get_dword(_T("NpcTypeID"), szField, INVALID_VALUE);

	szTemp = (LPCTSTR)m_pVar->get_string(_T("npc_way_point"), szField, NULL);
	if(szTemp == NULL)
	{
		pGradePos->dw_npc_way_pos = INVALID_VALUE;
	}
	else
	{
		pGradePos->dw_npc_way_pos = get_tool()->crc32(szTemp);
	}
	

	return &pGradePos->dw_key_;

}

//--------------------------------------------------------------------------------------------
// 初始化帮派练兵场士兵
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildEnlisteePos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pEnlisteePos, nProtoType, s_guild_enlistee_pos);

	pEnlisteePos->n_level_ = m_pVar->get_int(_T("id"), szField);

	for(INT16 i = 0; i < MAX_GUILD_ENLISTEE_NUM; i++)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("NpcID%d"), i + 1);
		pEnlisteePos->dw_creature_id_[i] = m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);

		wsprintf(pTchTmp, _T("way_pos%d"), i+1);
		LPCTSTR szTemp = (LPCTSTR)m_pVar->get_string(pTchTmp, szField, NULL);
		if(szTemp == NULL)
		{
			pEnlisteePos->dw_way_pos[i] = INVALID_VALUE;
		}
		else
		{
			pEnlisteePos->dw_way_pos[i] = get_tool()->crc32(szTemp);
		}
	}

	return &pEnlisteePos->n_level_;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派pvp旗帜位置
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildPvPBannerPos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pBannerPos, nProtoType, s_guild_pvp_banner_pos);

	pBannerPos->n_act_id_ = m_pVar->get_int(_T("id"), szField);

	for(INT i = 0; i < MAX_BANNER_NUM; i++)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("creatureid%d"), i + 1);
		pBannerPos->dw_creature_id_[i] = m_pVar->get_dword(pTchTmp, szField);

		wsprintf(pTchTmp, _T("posx%d"), i+1);
		pBannerPos->v_pos_x_[i] = m_pVar->get_float(pTchTmp, szField, 0.0f);

		wsprintf(pTchTmp, _T("posz%d"), i+1);
		pBannerPos->v_pos_z_[i] = m_pVar->get_float(pTchTmp, szField, 0.0f);
	}

	return &pBannerPos->n_act_id_;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派设施升级需求信息
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildUpgradeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	BYTE byType		= (BYTE)m_pVar->get_dword(_T("Type"), szField, INVALID_VALUE);
	BYTE byLevel	= (BYTE)m_pVar->get_dword(_T("Level"), szField, 0);

	M_trans_pointer(pUpgradeInfo, nProtoType, s_guild_upgrade_need);
	pUpgradeInfo->dw_key			= byType - 1;
	pUpgradeInfo->dw_key			= (pUpgradeInfo->dw_key << 16) | byLevel;

	pUpgradeInfo->n16_full_fill	= (INT16)m_pVar->get_dword(_T("FullFill"), szField, INVALID_VALUE);
	pUpgradeInfo->n16_step		= (INT16)m_pVar->get_dword(_T("Step"), szField, INVALID_VALUE);
	pUpgradeInfo->n_fund			= m_pVar->get_int(_T("GuildFund"), szField, INVALID_VALUE);
	pUpgradeInfo->n_material		= m_pVar->get_int(_T("Material"), szField, INVALID_VALUE);
	pUpgradeInfo->n_base_exploit	= m_pVar->get_int(_T("GuildExploit"), szField, INVALID_VALUE);
	pUpgradeInfo->n_up_level_time_limit = m_pVar->get_int(_T("UpLevelTimeLimit"), szField, 0);
	pUpgradeInfo->n_dec_prosperity = m_pVar->get_int(_T("DecreaseProsperity"), szField, 0);
	pUpgradeInfo->by_step		= m_pVar->get_int(_T("step"), szField, 0);

	for (int n=1; n<=20; n++)
	{
		tstringstream tss_type;
		tss_type << _T("ItemType") << n;
		tstringstream tss_num;
		tss_num << _T("ItemNumber") << n;

		s_item_need_info sItemInfo;
		sItemInfo.dw_item_type_id	= m_pVar->get_dword(tss_type.str().c_str(), szField, INVALID_VALUE);
		sItemInfo.n_item_need_num	= m_pVar->get_int(tss_num.str().c_str(), szField, INVALID_VALUE);
		if (sItemInfo.is_valid())
		{
			pUpgradeInfo->list_item_info.push_back(sItemInfo);
		}
	}

	return &pUpgradeInfo->dw_key;
}

//-------------------------------------------------------------------------------
// 获取帮派设施升级需求信息
//-------------------------------------------------------------------------------
BOOL AttRes::GetGuildUpgradeItemInfo( BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo )
{
	DWORD dwKey = eType;
	dwKey = (dwKey << 16) | (byLevel + 1);

	s_guild_upgrade_need* pUpgradeProto = m_GuildUpgradeNeedInfo.find(dwKey);
	if (!VALID_POINT(pUpgradeProto))
	{
		return FALSE;
	}

	sInfo.eType			= (EFacilitiesType)eType;
	sInfo.nFulfill		= pUpgradeProto->n16_full_fill;
	sInfo.nStep			= pUpgradeProto->n16_step;
	sInfo.nBaseExploit	= pUpgradeProto->n_base_exploit;
	sInfo.nNeedFund		= pUpgradeProto->n_fund;
	sInfo.nMaterial		= pUpgradeProto->n_material;
	sInfo.nUpLevelLimit = pUpgradeProto->n_up_level_time_limit;
	sInfo.nDecProsperity = pUpgradeProto->n_dec_prosperity;
	sInfo.byStep		= pUpgradeProto->by_step;
	sInfo.nDayDecProsperity	= 0;
	

	// 随机获得4种物品
	package_list<s_item_need_info> listTemp = pUpgradeProto->list_item_info;

	for (int n=0; n<MAX_UPGRADE_NEED_ITEM_TYPE; n++)
	{
		s_item_need_info sItemInfo;
		if (!listTemp.rand_find(sItemInfo, TRUE))
		{
			return FALSE;
		}

		sInfo.dwItemID[n]	= sItemInfo.dw_item_type_id;
		sInfo.nNeedNum[n]	= sItemInfo.n_item_need_num;
	}

	return TRUE;
}

BOOL AttRes::GetGuildUpgradeBaseInfo( BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo )
{
	DWORD dwKey = eType;
	dwKey = (dwKey << 16) | (byLevel + 1);

	s_guild_upgrade_need* pUpgradeProto = m_GuildUpgradeNeedInfo.find(dwKey);
	if (!VALID_POINT(pUpgradeProto))
	{
		return FALSE;
	}

	sInfo.eType			= (EFacilitiesType)eType;
	sInfo.nFulfill		= pUpgradeProto->n16_full_fill;
	sInfo.nStep			= pUpgradeProto->n16_step;
	sInfo.nBaseExploit	= pUpgradeProto->n_base_exploit;
	sInfo.nNeedFund		= pUpgradeProto->n_fund;
	sInfo.nMaterial		= pUpgradeProto->n_material;
	sInfo.nUpLevelLimit = pUpgradeProto->n_up_level_time_limit;
	sInfo.nDecProsperity = pUpgradeProto->n_dec_prosperity;
	sInfo.nDayDecProsperity = 0;
	/*if(eType == EFT_Lobby)
	{
		sInfo.nDayDecProsperity = byLevel * 30;
	}
	else
	{
		sInfo.nDayDecProsperity = 0;
	}*/

	return TRUE;
}

const s_guild_grade_pos* AttRes::GetGuildGradePosInfo(BYTE eType, BYTE byLevel)
{
	DWORD dwKey = eType;
	dwKey = (dwKey << 16) | (byLevel);

	s_guild_grade_pos* pGuildGradePos = m_GuildGradePosInfo.find(dwKey);

	return pGuildGradePos;

}


//--------------------------------------------------------------------------------------------
// 初始化帮派事务信息
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildAffairProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pAffairInfo, nProtoType, s_guild_affair_info);

	pAffairInfo->dw_buff_id		= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pAffairInfo->n_fund			= m_pVar->get_int(_T("GuildFund"), szField, 0);
	pAffairInfo->n_material		= m_pVar->get_int(_T("GuildMaterial"), szField, 0);
	pAffairInfo->by_guild_level	= (BYTE)m_pVar->get_dword(_T("GuildLevel"), szField, 1);
	pAffairInfo->by_hold_city		= (BYTE)m_pVar->get_dword(_T("GuildHoldCity"), szField, 0);

	return &pAffairInfo->dw_buff_id;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派技能信息
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildSkillProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pSkillInfo, nProtoType, tagGuildSkill);

	pSkillInfo->dwSkillID			= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pSkillInfo->n16Fulfill			= (INT16)m_pVar->get_int(_T("FullFill"), szField, INVALID_VALUE);
	pSkillInfo->nLearnContribution	= m_pVar->get_int(_T("LearnRequireContribution"), szField, 0);
	pSkillInfo->nLearnFund			= m_pVar->get_int(_T("LearnRequireFund"), szField, 0);
	pSkillInfo->nLearnMaterial		= m_pVar->get_int(_T("LearnRequireMaterial"), szField, 0);
	pSkillInfo->nLearnSilver		= m_pVar->get_int(_T("LearnRequireSilver"), szField, 0);
	pSkillInfo->nResearchFund		= m_pVar->get_int(_T("ResearchRequireFund"), szField, 0); 
	pSkillInfo->nResearchMaterial	= m_pVar->get_int(_T("ResearchRequireMaterial"), szField, 0);

	return &pSkillInfo->dwSkillID;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派技能信息(创建帮派时)
//--------------------------------------------------------------------------------------------
BOOL AttRes::LoadGuildSkillInfo( package_map<DWORD, tagGuildSkill*>& mapGuildSkill )
{
	mapGuildSkill.clear();

	tagGuildSkill* pGuildSkill = NULL;
	package_map<DWORD, tagGuildSkill*>::map_iter iter = m_GuildSkillInfo.begin();
	while (m_GuildSkillInfo.find_next(iter, pGuildSkill))
	{
		if (!VALID_POINT(pGuildSkill))
		{
			continue;
		}

		DWORD	dwSkillID	= pGuildSkill->dwSkillID / 100;
		INT		nLevel		= pGuildSkill->dwSkillID % 100;
		if (nLevel == 1)
		{
			// 写入初始属性
			tagGuildSkill* pSkillInfo	= new tagGuildSkill(*pGuildSkill);
			pSkillInfo->dwSkillID		= dwSkillID;
			pSkillInfo->nLevel			= 1;
			mapGuildSkill.add(dwSkillID, pSkillInfo);
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
// 读取帮派技能静态属性
//--------------------------------------------------------------------------------------------
BOOL AttRes::get_guild_skill_info( DWORD dwSkillID, INT nLevel, tagGuildSkill& sGuildSkill )
{
	DWORD dwGuildSkillID = dwSkillID * 100 + nLevel;

	sGuildSkill.dwSkillID				= dwSkillID;
	sGuildSkill.nLevel					= nLevel;

	tagGuildSkill* pSkillInfo = m_GuildSkillInfo.find(dwGuildSkillID);
	if (!VALID_POINT(pSkillInfo))
	{
		return FALSE;
	}

	sGuildSkill.n16Fulfill				= pSkillInfo->n16Fulfill;
	sGuildSkill.nLearnContribution		= pSkillInfo->nLearnContribution;
	sGuildSkill.nLearnFund				= pSkillInfo->nLearnFund;
	sGuildSkill.nLearnMaterial			= pSkillInfo->nLearnMaterial;
	sGuildSkill.nLearnSilver			= pSkillInfo->nLearnSilver;
	sGuildSkill.nResearchFund			= pSkillInfo->nResearchFund;
	sGuildSkill.nResearchMaterial		= pSkillInfo->nResearchMaterial;

	return TRUE;
}

//--------------------------------------------------------------------------------------------
// 初始化帮派跑商信息
//--------------------------------------------------------------------------------------------
VOID* AttRes::InitOneGuildCommerceProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pCommerceInfo, nProtoType, s_commerce_info);

	pCommerceInfo->dw_id							= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pCommerceInfo->tagTaelInfo.nDeposit			= m_pVar->get_int(_T("Foregift"), szField, 0);
	pCommerceInfo->tagTaelInfo.nBeginningTael		= m_pVar->get_int(_T("InitialTael"), szField, 0);
	pCommerceInfo->tagTaelInfo.nPurposeTael		= m_pVar->get_int(_T("FinalTael"), szField, 0);
	pCommerceInfo->tagTaelInfo.nMaxTael			= m_pVar->get_int(_T("MaxTael"), szField, 0);
	pCommerceInfo->s_redound_info_.n_exp			= m_pVar->get_int(_T("Experience"), szField, 0);
	pCommerceInfo->s_redound_info_.n_contribution	= m_pVar->get_int(_T("GuildContribution"), szField, 0);
	pCommerceInfo->s_redound_info_.n_exploit		= m_pVar->get_int(_T("GuildExploit"), szField, 0);

	return &pCommerceInfo->dw_id;
}

//-------------------------------------------------------------------------------
// 获取帮派跑商信息
//-------------------------------------------------------------------------------
const s_commerce_info* AttRes::GetGuildCommerceInfo( INT nLevel )
{
	DWORD dwID				= INVALID_VALUE;
	s_commerce_info* pInfo	= NULL;

	package_map<DWORD, s_commerce_info*>::map_iter iter = m_GuildCommerceInfo.begin();
	while (m_GuildCommerceInfo.find_next(iter, dwID, pInfo))
	{
		if (!VALID_VALUE(dwID) || !VALID_POINT(pInfo))
		{
			continue;
		}

		// 计算等级范围
		if ((nLevel >= (INT)(dwID / 1000)) && (nLevel <= (INT)(dwID % 1000)))
		{
			return pInfo;
		}
	}

	// 没找到
	return NULL;
}

//-------------------------------------------------------------------------------
// 初始化商货信息
//-------------------------------------------------------------------------------
VOID* AttRes::InitOneCommodityProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pProto, nProtoType, s_commodity_proto);

	pProto->dw_good_id			= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pProto->by_holder_id			= (BYTE)m_pVar->get_dword(_T("HolderID"), szField, 0);
	pProto->n_low_price			= m_pVar->get_int(_T("LowPrice"), szField, 0);
	pProto->n_high_price			= m_pVar->get_int(_T("HighPrice"), szField, 0);
	pProto->by_refresh_num		= (BYTE)m_pVar->get_dword(_T("FreshenNumber"), szField, 1);

	return &pProto->dw_good_id;
}

//-------------------------------------------------------------------------------
// 初始化商会信息
//-------------------------------------------------------------------------------
VOID* AttRes::InitOneCofCProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pProto, nProtoType, s_cof_c_proto);

	pProto->n64_key		= m_pVar->get_dword(_T("PurchaseCofC"), szField, INVALID_VALUE);
	DWORD dwID			= m_pVar->get_dword(_T("InitialCofC"), szField, INVALID_VALUE);
	pProto->n64_key		= (pProto->n64_key << 32) | dwID;
	pProto->f_profit		= m_pVar->get_float(_T("Benefit"), szField, 0);

	return &pProto->n64_key;
}

//-------------------------------------------------------------------------------
// 初始化特产商信息
//-------------------------------------------------------------------------------
VOID* AttRes::InitOneCofCSPProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pProto, nProtoType, s_cof_csp_proto);

	pProto->dw_cof_cid	= m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pProto->by_holder_id	= (BYTE)m_pVar->get_dword(_T("CityName"), szField, 0);

	return &pProto->dw_cof_cid;
}

VOID* AttRes::InitOngGuildMaterialProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pProto, nProtoType, tagGuildMaterialReceive);

	pProto->dw_item_id = m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pProto->n_contribution = m_pVar->get_int(_T("contribution"), szField, 0);
	pProto->n_material = m_pVar->get_int(_T("material"), szField, 0);
	pProto->byLevel = m_pVar->get_dword(_T("level"), szField, 1);

	return &pProto->dw_item_id;
}

//-------------------------------------------------------------------------------
// 读取卖出商货收益率
//-------------------------------------------------------------------------------
const FLOAT AttRes::GetCofCProfit( DWORD dwDstID, DWORD dwSrcID )
{
	INT64 n64Key = dwDstID;
	n64Key = (n64Key << 32) | dwSrcID;

	s_cof_c_proto* pProto = m_CofCProto.find(n64Key);
	if (!VALID_POINT(pProto))
	{
		return INVALID_VALUE;
	}

	return pProto->f_profit;
}

//-------------------------------------------------------------------------------
// 载入商会商货信息
//-------------------------------------------------------------------------------
BOOL AttRes::LoadCofCGoodInfo( DWORD dwCofCID, package_map<DWORD, tag_chamber_sell_good*>& mapGoodSell,
							  package_map<DWORD, tag_chamber_buy_good*>& mapGoodBuy )
{
	if (!VALID_VALUE(dwCofCID))
	{
		return FALSE;
	}
	mapGoodSell.clear();
	mapGoodBuy.clear();

	DWORD dwGoodID				= INVALID_VALUE;
	s_commodity_proto* pProto	= NULL;
	package_map<DWORD, s_commodity_proto*>::map_iter iter = m_GuildCommodityProto.begin();
	while (m_GuildCommodityProto.find_next(iter, dwGoodID, pProto))
	{
		if (!VALID_VALUE(dwGoodID) || !VALID_POINT(pProto))
		{
			continue;
		}

		// 售卖商货信息
		if (pProto->by_holder_id == dwCofCID)
		{
			tag_chamber_sell_good* pSellGood	= new tag_chamber_sell_good;
			pSellGood->dw_goods_id			= dwGoodID;
			pSellGood->by_free_num		= pProto->by_refresh_num;
			pSellGood->n_cost			= get_tool()->rand_in_range(pProto->n_low_price, pProto->n_high_price);
			mapGoodSell.add(dwGoodID, pSellGood);
		}
		
		// 收购商货信息
		tag_chamber_buy_good* pBuyGood	= new tag_chamber_buy_good;
		pBuyGood->dw_goods_id			= dwGoodID;
		pBuyGood->n_cost				= get_tool()->rand_in_range(pProto->n_low_price, pProto->n_high_price);
		mapGoodBuy.add(dwGoodID, pBuyGood);
	}

	return TRUE;
}

VOID* AttRes::InitOnePetLvlUpItemProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pPetLvlUp, nProtoType, tagPetLvlUpItemProto);

	pPetLvlUp->dw_data_id	= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);


	for (INT nIndex = 0; nIndex < MAX_PET_STEP_UPGRADE_SKILL_NUM; ++nIndex)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("skill%d"), nIndex + 1);
		pPetLvlUp->dwSkillIDs[nIndex]	= (DWORD)m_pVar->get_dword(pTchTmp, szField, INVALID_VALUE);
	}

	return &pPetLvlUp->dw_data_id;
}

VOID* AttRes::InitOneMotionProto( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy )
{
	M_trans_pointer(pMotionProto, nProtoType, tagMotionProto);

	pMotionProto->dw_data_id		= (DWORD)m_pVar->get_dword(_T("id"), szField, INVALID_VALUE);
	pMotionProto->eMotionType	= (EMotionType1)m_pVar->get_dword(_T("type"), szField, INVALID_VALUE);
	pMotionProto->dwFriendVal	= (DWORD)m_pVar->get_dword(_T("friendval"), szField, 0);

	return &pMotionProto->dw_data_id;
}

VOID* AttRes::InitOneEquipLevelPctProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy)
{
	M_trans_pointer(pEquipLevelPctProto, nProtoType, tagEquipLevelPctProto);

	pEquipLevelPctProto->n16Index = (INT16)m_pVar->get_dword(_T("id"), szField, 0);

	pEquipLevelPctProto->fLevelPct = (FLOAT)m_pVar->get_float(_T("Param"), szField, 1.0f);
	pEquipLevelPctProto->fLevelPct2 = (FLOAT)m_pVar->get_float(_T("Param2"), szField, 1.0f);
	pEquipLevelPctProto->nDerateMin = m_pVar->get_int(_T("DerateMin"), szField, 0);
	pEquipLevelPctProto->nDerateMax = m_pVar->get_int(_T("DerateMax"), szField, 0);
	pEquipLevelPctProto->nMaxAttValue = m_pVar->get_int(_T("MaxAttValue"), szField, 99999);

	pEquipLevelPctProto->fPosPct[EEP_RightHand] = m_pVar->get_float(_T("Weapon"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Head]		= m_pVar->get_float(_T("Head"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Body]		= m_pVar->get_float(_T("Body"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Feet]		= m_pVar->get_float(_T("Feet"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Body1]		= m_pVar->get_float(_T("Body1"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Wrist1]	= m_pVar->get_float(_T("Wrist1"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Wrist2]	= m_pVar->get_float(_T("Wrist2"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Waist]		= m_pVar->get_float(_T("Waist"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Neck]		= m_pVar->get_float(_T("Neck"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Finger1]	= m_pVar->get_float(_T("Finger1"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Finger2]	= m_pVar->get_float(_T("finger2"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Shipin1]	= m_pVar->get_float(_T("Shipin1"), szField, 1.0f);
	pEquipLevelPctProto->fPosPct[EEP_Fashion]	= m_pVar->get_float(_T("Fashion"), szField, 1.0f);

	return &pEquipLevelPctProto->n16Index;
}


VOID* AttRes::InitCreatureLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pCreatureLoot, pProtoType, tagCreatureLoot);

	pCreatureLoot->dwLootID		=	_tcstol(szField, NULL, 10);
	pCreatureLoot->eOpType		=	(ELootOpType)m_pVar->get_dword(_T("DropOpType"), szField, 0);
	pCreatureLoot->eLootMode	=	(ELootMode)m_pVar->get_dword(_T("DropMode"), szField, 0);
	pCreatureLoot->nMinMoney	=	m_pVar->get_dword(_T("DropMinMoney"), szField, 0);
	pCreatureLoot->nMaxMoney	=	m_pVar->get_dword(_T("DropMaxMoney"), szField, 0);
	pCreatureLoot->nMoneyNumMin	=	m_pVar->get_dword(_T("DropMoneyNumMin"), szField, 1);
	pCreatureLoot->nMoneyNumMax	=	m_pVar->get_dword(_T("DropMoneyNumMax"), szField, 1);
	pCreatureLoot->fMoneyChance =	m_pVar->get_float(_T("DropMoneyPro"), szField, 1.0f);
	// 掉落表物品
	for(INT i = 0; i < MAX_CREATURE_LOOT_NUM; ++i)
	{
		tstringstream ss;
		ss << _T("Item") << i+1 << _T("ID");
		pCreatureLoot->Loot[i].dwItemID = m_pVar->get_dword(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Type");
		pCreatureLoot->Loot[i].eLootType = (ELootType)m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("MaxNumber");
		pCreatureLoot->Loot[i].nMax = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("MinNumber");
		pCreatureLoot->Loot[i].nMin = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Probability");
		pCreatureLoot->Loot[i].fChance = m_pVar->get_float(ss.str().c_str(), szField, 0.0f);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality1");
		pCreatureLoot->Loot[i].nEquipQltyPct[EIQ_Quality0] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality2");
		pCreatureLoot->Loot[i].nEquipQltyPct[EIQ_Quality1] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality3");
		pCreatureLoot->Loot[i].nEquipQltyPct[EIQ_Quality2] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality4");
		pCreatureLoot->Loot[i].nEquipQltyPct[EIQ_Quality3] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality5");
		pCreatureLoot->Loot[i].nEquipQltyPct[EIQ_Quality4] = m_pVar->get_int(ss.str().c_str(), szField, 0);

	}
	
	return &pCreatureLoot->dwLootID;

}
VOID* AttRes::InitItemSetLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pLootItemSet, pProtoType, tagLootItemSet);

	pLootItemSet->dwSetID = _tcstol(szField, NULL, 10);

	// 加载物品
	for(INT i = 0; i < MAX_ITEM_SET_NUM; ++i)
	{
		tstringstream ss;
		ss << _T("Item") << i+1 << _T("ID");
		pLootItemSet->ItemSet[i].dwItemID = m_pVar->get_dword(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Number");
		pLootItemSet->ItemSet[i].nItemNum = m_pVar->get_dword(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Pro");
		pLootItemSet->ItemSet[i].fChance = m_pVar->get_float(ss.str().c_str(), szField, 0.0f);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality1");
		pLootItemSet->ItemSet[i].nEquipQltyPct[EIQ_Quality0] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality2");
		pLootItemSet->ItemSet[i].nEquipQltyPct[EIQ_Quality1] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality3");
		pLootItemSet->ItemSet[i].nEquipQltyPct[EIQ_Quality2] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality4");
		pLootItemSet->ItemSet[i].nEquipQltyPct[EIQ_Quality3] = m_pVar->get_int(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Quality5");
		pLootItemSet->ItemSet[i].nEquipQltyPct[EIQ_Quality4] = m_pVar->get_int(ss.str().c_str(), szField, 0);

	}
	
	return &pLootItemSet->dwSetID;

}
VOID* AttRes::InitQuestItemLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pLootQuestItem, pProtoType, tagLootQuestItem);

	pLootQuestItem->dwCreatureID = _tcstol(szField, NULL, 10);

	for(INT i = 0; i < MAX_LOOT_QUEST_ITEM_NUM; ++i)
	{
		tstringstream ss;
		ss << _T("Item") << i+1 << _T("ID");
		pLootQuestItem->QuestItem[i].dwQuestItemID  = m_pVar->get_dword(ss.str().c_str(), szField, 0);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("Probability");
		pLootQuestItem->QuestItem[i].fChance = m_pVar->get_float(ss.str().c_str(), szField, 0.0f);
		ss.str(_T(""));
		ss << _T("Item") << i+1 << _T("TeamProbability");
		pLootQuestItem->QuestItem[i].fTeamChance = m_pVar->get_float(ss.str().c_str(), szField, 0);
	}

	return &pLootQuestItem->dwCreatureID;

}

VOID* AttRes::InitSceneFishingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagNpcFishingProto);

	INT nOutNumber = m_pVar->get_int(_T("Number"), szField, 0);
	INT nSpOutNumber = m_pVar->get_int(_T("SPNumber"), szField, 0);

	pProto->dwID = _tcstol(szField, NULL, 10);
	pProto->nSkillLevel = m_pVar->get_int(_T("SkillLevel"), szField, 1);
	pProto->nMinLevel = m_pVar->get_int(_T("LevelMin"), szField, 0);
	pProto->dwNullGetProb = m_pVar->get_dword(_T("NullProb"), szField, 0);

	// 普通鱼
	for(INT i = 0; i < nOutNumber; ++i)
	{
		tstringstream ss; tagFishItem item;
		ss <<  _T("Item") << i + 1 << _T("ID");
		item.dwItemID  = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		ss.str(_T(""));  
		ss << _T("Item") << i + 1 << _T("Prob");
		item.dwItemProb = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		ss.str(_T(""));  
		ss << _T("Item") << i + 1 << _T("SkillProficiency");
		item.dwSkillProficiency = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		if(VALID_POINT(item.dwItemID)) pProto->outs.push_back(item);
	}

	// 特殊鱼
	for(INT i = 0; i < nSpOutNumber; ++i)
	{
		tstringstream ss; tagFishItem item;
		ss <<  _T("SPItem") << i + 1 << _T("ID");
		item.dwItemID  = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		ss.str(_T(""));  
		ss << _T("SPItem") << i + 1 << _T("Prob");
		item.dwItemProb = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		ss.str(_T(""));  
		ss << _T("SPItem") << i + 1 << _T("SkillProficiency");
		item.dwSkillProficiency = m_pVar->get_dword(ss.str().c_str(), szField, 0);

		if(VALID_POINT(item.dwItemID)) pProto->outs_sp.push_back(item);
	}

	return &pProto->dwID;
}

void AttRes::LoadAchievementCriteriaList()
{
	AchievementCriteriaEntry* pAchProto = NULL;
	package_map<DWORD,AchievementCriteriaEntry* >::map_iter iter = m_mapAchievementCriteriaProto.begin();
	while(m_mapAchievementCriteriaProto.find_next(iter, pAchProto))
	{
		m_AchievementCriteriasByType[pAchProto->m_Events].push_back(pAchProto);
		m_AchievementCriteriaListByAchievement[pAchProto->m_referredAchievement].push_back(pAchProto);

	}
}	

void AttRes::LoadAchievementReferenceList()
{
	
	AchievementEntry* achievement = NULL;
	package_map<DWORD,AchievementEntry* >::map_iter iter = m_mapAchievementProto.begin();
	while(m_mapAchievementProto.find_next(iter, achievement))
	{
		m_AchievementListByReferencedId[achievement->m_refAchievement].push_back(achievement);

	}

}

VOID* AttRes::InitVipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, vip_proto);

	pProto->vip_level = _tcstol(szField, NULL, 10);
	pProto->compractice_add = m_pVar->get_int(_T("shuangxiu_add"), szField);
	pProto->xiuluoshilian_add = m_pVar->get_int(_T("xiuluo_add"), szField);
	pProto->mowushoulie_add = m_pVar->get_int(_T("mowu_add"), szField);
	pProto->hanghuishaoxiang_add = m_pVar->get_int(_T("hanghui_add"), szField);
	pProto->wine_add = m_pVar->get_int(_T("wine_add"), szField);
	pProto->tianming_add = m_pVar->get_int(_T("wenyun_add"), szField);
	pProto->yanhuotumo_add = m_pVar->get_int(_T("yanhuo_add"), szField);
	pProto->zuojipeiyang_add = m_pVar->get_int(_T("free_ride_up_add"), szField,0);
	pProto->xinyoulixi_add = m_pVar->get_int(_T("xylx_add"), szField,0);
	pProto->biwu_cold = m_pVar->get_float(_T("biwu"), szField,0);
	pProto->vip_time = m_pVar->get_int(_T("time"), szField);
	return &pProto->vip_level;
}

VOID* AttRes::InitVerificationProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, s_verification_code);	

	pProto->dw_id =  _tcstol(szField, NULL, 10);
	pProto->dw_code = m_pVar->get_string(_T("name"), szField);
	//LPCTSTR szName = m_pVar->get_string(_T("name"), szField);
	//pProto->dw_code = g_world.LowerCrc32(szName);

	return &pProto->dw_id;
}

VOID* AttRes::InitLotteryProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagLotteryProto);

	pProto->dwID = _tcstol(szField, NULL, 10);

	pProto->dwItemID = m_pVar->get_dword(_T("item_id"), szField);
	pProto->dwNumber = m_pVar->get_dword(_T("item_num"), szField);
	pProto->byType = m_pVar->get_dword(_T("type"), szField, 0);
	pProto->b_prize = m_pVar->get_dword(_T("first_prize"), szField, 0);
	pProto->b_bind = m_pVar->get_dword(_T("bind"), szField, 1);
	pProto->b_ItemNum = m_pVar->get_dword(_T("num"), szField, 1);
	//return &pProto->dwItemID;
	return &pProto->dwID;//gx modify 2013.6.26 修改后以ID作为键值，可以运行typeid全局重复
}

// 签到
VOID* AttRes::InitSignProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagSignProto);

	pProto->dw_id = _tcstol(szField, NULL, 10);
	pProto->n_keep_time = m_pVar->get_int(_T("keeptime"), szField);
	pProto->n_reward_num = m_pVar->get_int(_T("reward_num"), szField);

	for (INT nIndex = 0; nIndex < pProto->n_reward_num; ++nIndex)
	{
		TCHAR pTchTmp[LONG_STRING];

		wsprintf(pTchTmp, _T("condition_%d"), nIndex + 1);
		pProto->st_reward_data[nIndex].n_condition	= (INT)m_pVar->get_int(pTchTmp, szField);

		wsprintf(pTchTmp, _T("reward_%d"), nIndex + 1);
		pProto->st_reward_data[nIndex].dw_reward_id = (DWORD)m_pVar->get_int(pTchTmp, szField);

		wsprintf(pTchTmp, _T("bind_%d"), nIndex + 1);
		pProto->st_reward_data[nIndex].bBind = m_pVar->get_int(pTchTmp, szField, true);
	}

	return &pProto->dw_id;
}

// 魂精
VOID* AttRes::InitHuenJingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagHunJingProto);

	pProto->dwID = _tcstol(szField, NULL, 10);
	pProto->eAtt = (ERoleAttribute)m_pVar->get_int(_T("RoleAtt"), szField, INVALID_VALUE);	
	pProto->byQuality = m_pVar->get_int(_T("Quality"), szField, 0);
	pProto->nBaseAtt = m_pVar->get_int(_T("BaseAttValue"), szField, 0);
	pProto->nGrowAtt = m_pVar->get_int(_T("GrowAttValue"), szField, 0);

	return &pProto->dwID;

}

// 神级
VOID* AttRes::InitGodLevelProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagGodLevelProto);

	pProto->nID = _tcstol(szField, NULL, 10);
	pProto->nCondition[EGLC_ROLE_LEVEL] = m_pVar->get_int(_T("role_level"), szField, 0);	
	pProto->nCondition[EGLC_WEAPON_LEVEL] = m_pVar->get_int(_T("weapon_leve"), szField, 0);
	pProto->nCondition[EGLC_HUIZHANG_LEVEL] = m_pVar->get_int(_T("huiZhang_level"), szField, 0);
	pProto->nCondition[EGLC_YAPPEI_LEVEL] = m_pVar->get_int(_T("yaoPei_level"), szField, 0);
	pProto->nCondition[EGLC_PET_LEVEL] = m_pVar->get_int(_T("pet_level"), szField, 0);
	pProto->nCondition[EGLC_HUENLIAN_TIME] = m_pVar->get_int(_T("huenLian_time"), szField, 0);
	pProto->nCondition[EGLC_HUENJIN_LEVEL] = m_pVar->get_int(_T("huenJing_level"), szField, 0);
	pProto->dwSkill[EV_Warrior-1][0] = m_pVar->get_dword(_T("Warrior"), szField, 0);
	pProto->dwSkill[EV_Warrior-1][1] = m_pVar->get_dword(_T("Warrior2"), szField, 0);
	pProto->dwSkill[EV_Mage-1][0] = m_pVar->get_dword(_T("Mage"), szField, 0);
	pProto->dwSkill[EV_Mage-1][1] = m_pVar->get_dword(_T("Mage2"), szField, 0);
	pProto->dwSkill[EV_Taoist-1][0] = m_pVar->get_dword(_T("Hunter"), szField, 0);
	pProto->dwSkill[EV_Taoist-1][1] = m_pVar->get_dword(_T("Hunter2"), szField, 0);
//	pProto->dwSkill[EV_Blader-1][0] = m_pVar->get_dword(_T("Blader"), szField, 0);
//	pProto->dwSkill[EV_Blader-1][1] = m_pVar->get_dword(_T("Blader2"), szField, 0);
//	pProto->dwSkill[EV_Astrologer-1][0] = m_pVar->get_dword(_T("Astrologer"), szField, 0);
//	pProto->dwSkill[EV_Astrologer-1][1] = m_pVar->get_dword(_T("Astrologer2"), szField, 0);
	return &pProto->nID;
}

VOID* AttRes::InitRaidProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy)
{
	M_trans_pointer(pProto, pProtoType, tagRaidProto);

	pProto->dwTypeID = _tcstol(szField, NULL, 10);

	pProto->dwNeedExp = m_pVar->get_dword(_T("exp"), szField, 0);
	pProto->dwModleID = m_pVar->get_dword(_T("modle"), szField, 0);
	pProto->nRating = m_pVar->get_dword(_T("rating"), szField, 0);

	for (INT nIndex = 0; nIndex < MAX_GODSTONE_NUM; ++nIndex)
	{
		TCHAR szTemp[X_SHORT_NAME];

		wsprintf(szTemp, _T("att_type_%d"), nIndex + 1);
		pProto->sRoleAtt[nIndex].eRoleAtt	= (ERoleAttribute)m_pVar->get_int(szTemp, szField, INVALID_VALUE);

		wsprintf(szTemp, _T("att_value_%d"), nIndex + 1);
		pProto->sRoleAtt[nIndex].nValue = m_pVar->get_int(szTemp, szField, 0);

	}

	return &pProto->dwTypeID;
}
