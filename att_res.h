/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once

#include "../common/ServerDefine/role_data_server_define.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/skill_define.h"
#include "../../common/WorldDefine/buff_define.h"
#include "../../common/WorldDefine/trigger_define.h"
#include "../../common/WorldDefine/compose_define.h"
#include "../../common/WorldDefine/MapAttDefine.h"
#include "../../common/WorldDefine/mall_define.h"
#include "../../common/WorldDefine/variable_len.h"
#include "../../common/WorldDefine/guild_define.h"
#include "../../common/WorldDefine/stall_define.h"
#include "../common/ServerDefine/pet_server_define.h"
#include "../common/ServerDefine/consolidate_server_define.h"
#include "../../common/WorldDefine/title_define.h"
#include "../../common/WorldDefine/achievement_define.h"
#include "../../common/WorldDefine/Sign_define.h"
#include "instance_define.h"
#include "att_res_define.h"
#include "../../common/WorldDefine/pet_define.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "../../common/WorldDefine/FormulaParamDefine.h"
#include "famehall_part.h"
#include "../../common/WorldDefine/ride_define.h"
#include "../../common/WorldDefine/loot.h"
#include "../common/ServerDefine/fishing_server_define.h"
#include "../common/ServerDefine/WeatherDefine.h"
#include "../common/ServerDefine/TeamShareQuestServerDefine.h"
#include "../../common/WorldDefine/vip_define.h"
#include "../common/WorldDefine/gp_mall_define.h"
#include "../common/WorldDefine/LianHun_define.h"

struct	tagMotionProto;
struct	tagPetProto;
struct	tagPetSkillProto;
struct	tagPetEquipProto;
struct	tagPetGatherProto;
struct	tagPetWuXingProto;
struct	s_vnb_equip_proto;
struct	s_vnb_gift_proto;
struct	tagItemProto;
struct	tagShopProto;
struct	tagDakProto;
struct	tagSuitProto;
struct	tagMallItemProto;
struct	tagMallPackProto;
struct	tagSSpawnPointProto;
struct	tagGuildFacilities;
struct	s_guild_upgrade_need;
struct	tag_chamber_sell_good; 
struct	tag_chamber_buy_good; 
struct	tagFormulaParam; 
struct	s_new_role_gift_proto;
struct tagGodLevelProto;
struct tagRaidProto;

class	quest;

typedef std::list<AchievementCriteriaEntry const*> AchievementCriteriaEntryList;
typedef std::list<AchievementEntry const*>         AchievementEntryList;

typedef std::map<UINT32,AchievementCriteriaEntryList> AchievementCriteriaListByAchievement;
typedef std::map<UINT32,AchievementEntryList>         AchievementListByReferencedId;

//-----------------------------------------------------------------------------
class AttRes
{
public:
	typedef fastdelegate::FastDelegate3<LPVOID, LPCTSTR, INT32, VOID*> Fun_p;

public:
	AttRes(){}
	~AttRes(){ destory(); }
	BOOL Init();
	VOID destory();

public: // 上层确保互斥 -- 最好是在地图线程的上层线程执行
	// 重新加载指定属性文件
	BOOL ReloadItemProto();

	// 重新加载商城数据
	BOOL ReloadMallProto();

	static AttRes* GetInstance();
	static VOID	   Destory();

	

public:
	// 根据TypeID重新设置是否记录log
	VOID ResetItemLog(s_need_log_item dw_data_id[], INT32 n_num);

public:
	//-------------------------------------------------------------------------------------------
	// 升级
	//-------------------------------------------------------------------------------------------
	//const s_level_up_effect* GetLevelUpEffect(INT32 nLevel) const { return &m_LevelUpEffect[nLevel]; }
	const s_level_up_effect* GetLevelUpEffect(INT32 id) { return m_LevelUpEffect.find(id); }
	const tagEquipLevelUpEffect* GetEquipLevelUpEffect(INT32 nLevel) const { return &m_EquipLevelUpEffect[nLevel];}
	const s_role_att_change* GetRoleAttChange(DWORD dwClass, DWORD dwAtt1) { return m_RoleAttChange.find(dwClass*1000 + dwAtt1); }
	//-------------------------------------------------------------------------------------------
	// 默认值
	//-------------------------------------------------------------------------------------------
	INT GetAttDefRole(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nDefRole; }
	INT GetAttDefCreature(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nDefCreature; }
	INT GetAttMin(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nMin; }
	INT GetAttMax(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nMax; }

	//-------------------------------------------------------------------------------------------
	// 过滤词表
	//-------------------------------------------------------------------------------------------
	std::vector<tstring>* GetNameFilterWords() 	{ return &m_vectNameFilter; }
	//std::vector<tstring>* GetChatFilterWords() 	{ return &m_vectChatFilter; }

	//-------------------------------------------------------------------------------------------
	// 不同语言版本名称长度
	//-------------------------------------------------------------------------------------------
	tagVariableLen& GetVariableLen() { return m_VarLen; }

	//-------------------------------------------------------------------------------
	// 获取帮派设施升级需求信息
	//-------------------------------------------------------------------------------
	BOOL GetGuildUpgradeBaseInfo(BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo);
	BOOL GetGuildUpgradeItemInfo(BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo);

	
	const s_guild_grade_pos* GetGuildGradePosInfo(BYTE eType, BYTE byLevel);

	//-------------------------------------------------------------------------------
	// 获取帮派练兵场士兵
	//-------------------------------------------------------------------------------
	const s_guild_enlistee_pos* GetGuildEnlisteePos(INT16 nLevel) { return m_GuildEnlisteePos.find(nLevel); }

	const s_guild_pvp_banner_pos* GetGuildPvPBannerPos(INT nActID) { return m_GuildPvPBannerPos.find(nActID); }

	//-------------------------------------------------------------------------------
	// 获取帮派事务信息
	//-------------------------------------------------------------------------------
	const s_guild_affair_info* GetGuildAffairInfo(DWORD dwBuffID)	{ return m_GuildAffairInfo.find(dwBuffID); }

	//-------------------------------------------------------------------------------
	// 获取帮派技能信息
	//-------------------------------------------------------------------------------
	BOOL get_guild_skill_info(DWORD dwSkillID, INT nLevel, tagGuildSkill& sGuildSkill);
	BOOL LoadGuildSkillInfo(package_map<DWORD, tagGuildSkill*>& mapGuildSkill);
	const tagGuildSkill* GetGuildSkillProto(DWORD dwSkillID)		{ return m_GuildSkillInfo.find(dwSkillID); }

	//-------------------------------------------------------------------------------
	// 获取帮派跑商信息
	//-------------------------------------------------------------------------------
	const s_commerce_info*		GetGuildCommerceInfo(INT nLevel);
	const s_commodity_proto*	GetCommodityProto(DWORD dwGoodID)	{ return m_GuildCommodityProto.find(dwGoodID); }
	const s_cof_csp_proto*		GetCofCSPProto(DWORD dwCofCID)		{ return m_CofCSPProto.find(dwCofCID); }
	const tagGuildMaterialReceive* GetMaterialReceive(DWORD dwID)	{ return m_GuildMaterialReceive.find(dwID); }
	const FLOAT					GetCofCProfit(DWORD dwDstID, DWORD dwSrcID);
	BOOL LoadCofCGoodInfo(DWORD dwCofCID, package_map<DWORD, tag_chamber_sell_good*>& mapGoodSell,
		package_map<DWORD, tag_chamber_buy_good*>& mapGoodBuy);
	
	//-------------------------------------------------------------------------------------------
	// 物品
	//-------------------------------------------------------------------------------------------
	tagItemProto* GetItemProto(DWORD dw_data_id)			{ return m_mapItemProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// 装备
	//-------------------------------------------------------------------------------------------
	tagEquipProto* GetEquipProto(DWORD dw_data_id)		{ return m_mapEquipProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// 精华(宝石,印记)
	//-------------------------------------------------------------------------------------------
	//tagGemProto* GetGemProto(DWORD dw_data_id)			{ return m_mapGemProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// 商店
	//-------------------------------------------------------------------------------------------
	tagShopProto* GetShopProto(DWORD dwShopID)			{ return m_mapShopProto.find(dwShopID); }

	//-------------------------------------------------------------------------------------------
	// 商场商品, 礼品包及免费物品
	//-------------------------------------------------------------------------------------------
	const tagMallItemProto* GetMallItemProto(DWORD dwID){ return m_mapMallItemProto.find(dwID); }
	const tagMallPackProto* GetMallPackProto(DWORD dwID){ return m_mapMallPackProto.find(dwID); }
	const tagMallFreeItem*	GetMallFreeProto()			{ return &m_MallFreeItemProto; }
	
	package_map<DWORD, tagMallItemProto*>& GetMallItem()		 { return m_mapMallItemProto; }
	package_map<DWORD, tagMallPackProto*>& GetMallPack()		 { return m_mapMallPackProto; }

	INT	GetMallItemNum()								{ return m_mapMallItemProto.size(); }
	INT GetMallPackNum()								{ return m_mapMallPackProto.size(); }

	//-------------------------------------------------------------------------------------------
	// 驿站
	//-------------------------------------------------------------------------------------------
	tagDakProto* GetDakProto(DWORD dwDakID)				{ return m_mapDakProto.find(dwDakID); }
	//-------------------------------------------------------------------------------------------
	// 装备品级鉴定几率
	//-------------------------------------------------------------------------------------------
	tagEquipQltyPct* GetEquipQltyPct(DWORD dw_data_id)	{ return m_mapEquipQltyPct.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// 套装静态属性
	//-------------------------------------------------------------------------------------------
	const tagSuitProto* GetSuitProto(DWORD dwSuitID)	{ return m_mapSuitProto.find(dwSuitID); }
	//-------------------------------------------------------------------------------------------
	// 品级装备属性参数
	//-------------------------------------------------------------------------------------------
	const tagEquipQltyEffect* GetEquipQltyEffect(INT32 nQlty) const { return &m_EquipQltyEffect[nQlty]; }


	//-------------------------------------------------------------------------------------------
	// 时装品级对生成影响参数
	//-------------------------------------------------------------------------------------------
	const tagFashionGen* GetFashionQltyEffect(INT32 nQlty) const { return &m_FashionGen[nQlty]; }

	//-------------------------------------------------------------------------------------------
	// 时装颜色生成概率参数
	//-------------------------------------------------------------------------------------------
	const tagFashionColorPct* GetFashionColorPct(INT32 nQlty) const { return &m_FashionColorPct[nQlty]; }
	
	//-------------------------------------------------------------------------------------------
	// 得到满足要求所有物品的指针列表
	//-------------------------------------------------------------------------------------------
	package_list<tagItemProto*> GetItemProtoList();
	//-------------------------------------------------------------------------------------------
	// 得到铭纹属性对应的装备是否可强化
	//-------------------------------------------------------------------------------------------
	BOOL IsPosyPos(EPosyAtt ePosyAtt, EEquipPos eEquipPos);
	//-------------------------------------------------------------------------------------------
	// 得到镌刻属性对应的装备是否可强化
	//-------------------------------------------------------------------------------------------
	BOOL IsEngravePos(EEngraveAtt ePosyAtt, EEquipPos eEquipPos);

	//-------------------------------------------------------------------------------------------
	// 技能
	//-------------------------------------------------------------------------------------------
	const tagSkillProto* GetSkillProto(DWORD dw_data_id) { return m_mapSkillProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// 状态
	//-------------------------------------------------------------------------------------------
	const tagBuffProto* GetBuffProto(DWORD dw_data_id) { return m_mapBuffProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// 触发器
	//-------------------------------------------------------------------------------------------
	const tagTriggerProto* GetTriggerProto(DWORD dwTriggerID) { return m_mapTriggerProto.find(dwTriggerID); }

	//-------------------------------------------------------------------------------------------
	// 某个技能是否能被其它技能影响
	//-------------------------------------------------------------------------------------------
	BOOL CanBeModified(DWORD dwSkillID) { return m_mapSkillModify.is_exist(dwSkillID); }

	//-------------------------------------------------------------------------------------------
	// 得到能影响某个技能的技能列表
	//-------------------------------------------------------------------------------------------
	tagSkillModify* GetSkillModifier(DWORD dwSkillID) { return m_mapSkillModify.find(dwSkillID); }

	//-------------------------------------------------------------------------------------------
	// 怪物
	//-------------------------------------------------------------------------------------------
	const tagCreatureProto* GetCreatureProto(DWORD dw_data_id) { return m_mapCreatureProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// 怪物AI
	//-------------------------------------------------------------------------------------------
	const tagCreatureAI* GetCreatureAI(DWORD dwAIID) { return m_mapCreatureAI.find(dwAIID); }

	//-------------------------------------------------------------------------------------------
	// 怪物阵营关系
	//-------------------------------------------------------------------------------------------
	const tagCreatureCamp* GetCreatureCamp(INT16 n16Camp) { return m_mapCreatureCamp.find(n16Camp); }

	//-------------------------------------------------------------------------------------------
	// 非随机副本地图刷怪点
	//-------------------------------------------------------------------------------------------
	const tagSSpawnPointProto *GetSSpawnPointProto(DWORD dwSpawnPointID) { return m_mapSSpawnPoint.find(dwSpawnPointID); }
	
	const tagSSpawnPointProto* GetSSpawnGroupProto(DWORD dwSpawnGroupID) { return m_mapSSpawnGroup.find(dwSpawnGroupID); }
	
	const tagGuildSSpawnPointProto* GetSSpawnGuildProto(DWORD dwCreatureID) { return m_mapGuildSSpawnPoint.find(dwCreatureID); }
	//-------------------------------------------------------------------------------------------
	// 副本随机刷怪点
	//-------------------------------------------------------------------------------------------
	const tagRandSpawnPointInfo* GetSpawnPointProto(DWORD dwSpawnPoint) { return m_mapSpawnPointProto.find(dwSpawnPoint); }

	//-------------------------------------------------------------------------------------------
	// 副本随机刷怪点
	//-------------------------------------------------------------------------------------------
	const tagInstance*	get_instance_proto(DWORD dwMapID)	{ return m_mapInstanceProto.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// 副本中不能使用的物品
	//-------------------------------------------------------------------------------------------
	const tagInstanceItem* GetInstanceItem(DWORD dwMapID) { return m_mapInstanceItem.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// 副本中不能使用的技能
	//-------------------------------------------------------------------------------------------
	const tagInstanceSkill* GetInstanceSkill(DWORD dwMapID) { return m_mapInstanceSkill.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// 副本随机刷怪点等级映射表
	//-------------------------------------------------------------------------------------------
	//const tagLevelMapping* GetLevelMapping(INT nLevel) { return m_mapLevelMapping.find(nLevel); }

	//-------------------------------------------------------------------------------------------
	// 铭纹
	//-------------------------------------------------------------------------------------------
	//const tagPosyProtoSer* GetPosyProto(DWORD dwPosyID) { return m_mapPosyProto.find(dwPosyID); }

	//-------------------------------------------------------------------------------------------
	// 镌刻
	//-------------------------------------------------------------------------------------------
	//const tagEngraveProtoSer* GetEngraveProto(DWORD dwEngraveID) { return m_mapEngraveProto.find(dwEngraveID); }

	//-------------------------------------------------------------------------------------------
	// 镶嵌,烙印,龙附
	//-------------------------------------------------------------------------------------------
	const tagConsolidateItem* GetConsolidateProto(DWORD dw_data_id) { return m_mapConsolidateProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// 合成
	//-------------------------------------------------------------------------------------------
	const s_produce_proto_ser* GetProduceProto(DWORD dwFormulaID) { return m_mapProduceProto.find(dwFormulaID); }
	
	//-------------------------------------------------------------------------------------------
	// 装备变换
	//-------------------------------------------------------------------------------------------
	const tagEquipChange*	GetEquipChangeProto(DWORD dwId) { return m_mapEquipChangeProto.find(dwId); }
	//-------------------------------------------------------------------------------------------
	// 点化,装备分解
	//-------------------------------------------------------------------------------------------
	const s_decompose_proto_ser* GetDeComposeProto(DWORD dwFormulaID) { return m_mapDeComposeProto.find(dwFormulaID); }

	//-------------------------------------------------------------------------------------------
	// 淬火
	//-------------------------------------------------------------------------------------------
	//const tagQuenchProtoSer *GetQuenchProto(DWORD dwFormulaID) { return m_mapQuenchProto.find(dwFormulaID); }

	//-------------------------------------------------------------------------------------------
	// 称号
	//-------------------------------------------------------------------------------------------
	//const tagTitleProto *GetTitleProto(UINT16 u16TitleID) { return &m_TitleProto[u16TitleID]; }
	const tagTitleProto *GetTitleProto(DWORD u16TitleID) { return m_TitleProto.find(u16TitleID); }
	// 成就
	// 获取成就
	const AchievementEntry* GetAchievementProto(DWORD id) { return m_mapAchievementProto.find(id); }
	// 获取条件
	const AchievementCriteriaEntry*	GetAchievementCriteriaProto(DWORD id) { return m_mapAchievementCriteriaProto.find(id); }
	// 通过类型获取所有条件
	AchievementCriteriaEntryList const& GetAchievementCriteriaByType(e_achievement_event type)
	{
		 return m_AchievementCriteriasByType[type];
	}
	
	// 获取对应成就的所有条件
	AchievementCriteriaEntryList const* GetAchievementCriteriaByAchievement(UINT32 id)
	{
		AchievementCriteriaListByAchievement::const_iterator itr = m_AchievementCriteriaListByAchievement.find(id);
		return itr != m_AchievementCriteriaListByAchievement.end() ? &itr->second : NULL;
	}
	
	
	AchievementEntryList const* GetAchievementByReferencedId(UINT32 id) const
	{
		AchievementListByReferencedId::const_iterator itr = m_AchievementListByReferencedId.find(id);
		return itr != m_AchievementListByReferencedId.end() ? &itr->second : NULL;
	}

	//-------------------------------------------------------------------------------------------
	// 氏族珍宝
	//-------------------------------------------------------------------------------------------
	const tagClanTreasureProto *GetClanTreasureProto(UINT16 u16TreasureID) { return &m_ClanTreasureProto[u16TreasureID]; }

	VOID GetRandVNBEquipProto(std::list<s_vnb_equip_proto*>& listEquips)	
	{	
		m_mapVNBEquipProto.copy_value_to_list(listEquips);
	}

	const s_vnb_gift_proto*	GetRandVNBGiftProto()	
	{	
		s_vnb_gift_proto* pProto = NULL;
		DWORD dwId = INVALID_VALUE;
		if (!m_mapVNBGiftProto.rand_find(dwId, pProto) || !VALID_POINT(pProto))
		{
			return NULL;
		}
		else
		{
			return pProto;
		}
	}

	const tagCreatureAI* RandGetCreatureAI()
	{
		DWORD dwAIID = INVALID_VALUE;
		tagCreatureAI* pAI = NULL;

		m_mapCreatureAI.rand_find(dwAIID, pAI);

		return pAI;
	}
	const tagCreatureAI* RandGetCreatureAIInGroup(DWORD dwGroupID)
	{
		package_list<DWORD>* pList = m_mapCreatureAIGroup.find(dwGroupID);
		if( VALID_POINT(pList) && pList->size() > 0 )
		{
			DWORD dwAIID = INVALID_VALUE;
			pList->rand_find(dwAIID);

			return m_mapCreatureAI.find(dwAIID);
		}
		else
		{
			return NULL;
		}
	}

	//-------------------------------------------------------------------------------------------
	// 获得宠物属性默认最小最大值
	//-------------------------------------------------------------------------------------------
// 	INT GetPetDef(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nDef;	}
// 	INT GetPetMin(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nMin;	}
// 	INT GetPetMax(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nMax;	}

	//-------------------------------------------------------------------------------------------
	// 获得宠物原型
	//-------------------------------------------------------------------------------------------
	const tagPetProto* GetPetProto(DWORD dwPetTypeID) { return m_mapPetProto.find(dwPetTypeID); }
	
	//-------------------------------------------------------------------------------------------
	//获得宠物技能随机列表
	//-------------------------------------------------------------------------------------------
	const s_pet_skill_list* GetPetRandomSkill(DWORD dwID) { return m_mapPetSkillListProto.find(dwID); }
	//-------------------------------------------------------------------------------------------
	// 获得宠物升级原型
	//-------------------------------------------------------------------------------------------
	const tagPetLvlUpProto* GetPetLvlUpProto(DWORD dwVLevel) { return m_mapPetLvlUpProto.find(dwVLevel); }

	//-------------------------------------------------------------------------------------------
	// 获得宠物升级原型
	//-------------------------------------------------------------------------------------------
	const tagPetLvlUpItemProto* GetPetLvlUpItemProto(DWORD dw_data_id) { return m_mapPetLvlUpItemProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// 获得宠物技能原型
	//-------------------------------------------------------------------------------------------
	const tagPetSkillProto* GetPetSkillProto(DWORD dwPetSkillTypeID) { return m_mapPetSkillProto.find(dwPetSkillTypeID); }

	//-------------------------------------------------------------------------------------------
	// 获得宠物装备原型
	//-------------------------------------------------------------------------------------------
	const tagPetEquipProto* GetPetEquipProto(DWORD dwPetEquipTypeID) { return m_mapPetEquipProto.find(dwPetEquipTypeID); }

	//-------------------------------------------------------------------------------------------
	// 获得宠物五行凝结原型
	//-------------------------------------------------------------------------------------------
	const tagPetWuXingProto* GetPetWuXingProto(DWORD dwPetWuXingTypeID) { return m_mapPetWuXingProto.find(dwPetWuXingTypeID); }

	//-------------------------------------------------------------------------------------------
	// 获得宠物技能数组
	//-------------------------------------------------------------------------------------------
	const std::list<DWORD>& GetPetNormalSkillList(INT nPetLevel) { return m_PetLevelSkillVec[nPetLevel - 1]; }

	//-------------------------------------------------------------------------------------------
	// VIP摊位信息
	//-------------------------------------------------------------------------------------------
	const INT32 GetVIPStallRent(INT nIndex) { return m_nVIPStallRent[nIndex]; }

	//-------------------------------------------------------------------------------------------
	// 获得动作原型
	//-------------------------------------------------------------------------------------------
	const tagMotionProto* GetMotionProto(DWORD dw_data_id) { return m_mapMotionProto.find(dw_data_id); }
	
	//公式参数
	const tagFormulaParam* GetFormulaParam(){return &m_FormulaParam;}
	
	//获得装备属性加成
	const tagEquipLevelPctProto* GetEquipLevelPct(INT16 n16Index){return m_mapEquipLevelPctProto.find(n16Index);}
	
	//获得技能学习列表
	const tagLearnSkill*	GetLearnSkillProto(DWORD dwID) { return m_mapLearnSkillListProto.find(dwID); }

	// 获得新手礼包
	const s_new_role_gift_proto* GetNewRoleGift(INT16 nID) { return m_mapNewRoleGiftProto.find(nID); }

	const tagWeatherProto* GetWeather(DWORD dwID) { return m_mapWeather.find(dwID); }
	
	// 获取掉落相关
	const tagCreatureLoot*	GetCreatureLoot(DWORD dwCreatureID)		{ return m_mapCreatureLoot.find(dwCreatureID); }
	const tagLootItemSet*	GetLootItemSet(DWORD dwSetID)			{ return m_mapLootItemSet.find(dwSetID); }
	const tagLootQuestItem*	GetLootQuestItem(DWORD dwCreatureID)	{ return m_mapLootQuestItem.find(dwCreatureID); }

	const tagNpcFishingProto* GetNpcFishingProto(DWORD dwMapID) { return m_mapNpcFishProto.find(dwMapID); }
	
	const s_verification_code*	GetVerificationCodeCrc(DWORD dwKey) { return m_mapVerificationCode.find(dwKey); }
	//gx modify 2013.8.14
	const vip_proto* GetVipProto(INT nVipLevel){
		static vip_proto __default;

		vip_proto* proto;
		const vip_proto* ret = &__default;

		package_map<DWORD, vip_proto*>::map_iter iter = m_mapVipProto.begin( );
		while(m_mapVipProto.find_next(iter, proto))
		{
			if (nVipLevel == proto->vip_level)
			{
				ret = proto;
				break;
			}
			/*if(nYuanBao < proto->need_yuanbao)
				break;
			ret = proto;*/
		}

		return ret;
	}
	
	BOOL GetRandVerification(DWORD& dwKey, tstring& dwCrc) 
	{
		s_verification_code* pCode;
		if (m_mapVerificationCode.rand_find(dwKey, pCode))
		{
			dwCrc = pCode->dw_code;
			return TRUE;
		}
		return FALSE;
	}

	void LoadAchievementCriteriaList();
	void LoadAchievementReferenceList();

	package_map<DWORD, tagLotteryProto*> GetLotteryMap() { return m_mapLotteryProto; }
	const tagLotteryProto* GetLotteryProto(DWORD dwKey) { return m_mapLotteryProto.find(dwKey); }
	

	const tagSignProto*		GetSignProto(DWORD dw_id)  { return m_mapSign.find(dw_id); }

	const tagHunJingProto*	GetHunJingProto(DWORD dw_id) { return m_maphuenjing.find(dw_id); }

	const tagGodLevelProto*	GetGodLevelProto(DWORD dw_id) { return m_mapgodlevel.find(dw_id); }
	
	const tagRaidProto*		GetRaidProto(DWORD dw_id) { return m_mapRaid.find(dw_id) ;}
private:
	//-------------------------------------------------------------------------------------------
	// 设置初始化某个资源的一条记录的处理函数
	//-------------------------------------------------------------------------------------------
	VOID SetInitOneRes(Fun_p pFun) { m_pFun = pFun; }

	//-------------------------------------------------------------------------------------------
	// 初始化资源的模版函数 -- 存入map中
	//-------------------------------------------------------------------------------------------
	template<class K, class T> 
	BOOL LoadResMap(package_map<K, T*> &mapRes, LPCTSTR szFileName, LPCTSTR szFileName2 = NULL);

	//-------------------------------------------------------------------------------------------
	// 释放资源的模版函数
	//-------------------------------------------------------------------------------------------
	template<class K, class T> VOID FreeResMap(package_map<K, T*> &mapRes);

	//-------------------------------------------------------------------------------------------
	// 初始化资源的模版函数 -- 存入array中
	//-------------------------------------------------------------------------------------------
	template<class T>
	BOOL LoadResArray(T *arrayRes, INT32 nIndexStart, INT32 nIndexEnd, LPCTSTR szFileName);

private:
	//-------------------------------------------------------------------------------------------
	// 初始化过滤词表
	//-------------------------------------------------------------------------------------------
	BOOL InitFilterWords(OUT std::vector<tstring>& vectFilterWords, LPCTSTR szFileName);

	//-------------------------------------------------------------------------------------------
	// 从脚本管理器中获取变量长度
	//-------------------------------------------------------------------------------------------
	VOID InitVarLen();

private:
	//-------------------------------------------------------------------------------------------
	// 初始化物品的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化装备的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化宝石等的一条记录
	//-------------------------------------------------------------------------------------------
	//VOID* InitOneGemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化套装的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneSuitProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化一类装备品级鉴定几率
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipQltyPct(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化商店
	//-------------------------------------------------------------------------------------------
	VOID* InitOneShopProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化商场商品及礼品包
	//-------------------------------------------------------------------------------------------
	VOID*  InitOneMallItemProtoBase(OUT LPVOID pProtoType, IN LPCTSTR szField);
	VOID* InitOneMallItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitOneMallPackProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化驿站
	//-------------------------------------------------------------------------------------------
	VOID* InitOneDakProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化技能的一条记录
	//-------------------------------------------------------------------------------------------
    VOID* InitOneSkillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化buff的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneBuffProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化trigger的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneTriggerProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化学习技能的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneLearnSKillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化Creature的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化Creature_ai的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureAIProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy);

	//-------------------------------------------------------------------------------------------
	// 初始化Creature_Camp的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureCampProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化铭纹的一条记录
	//--------------------------------------------------------------------------------------------
	//VOID* InitOnePosyProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化镌刻的一条记录
	//--------------------------------------------------------------------------------------------
	//VOID* InitOneEngraveProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化镶嵌,烙印,龙附的一条记录
	//--------------------------------------------------------------------------------------------
	VOID* InitOneConsolidateProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化生活技能的一条记录
	//--------------------------------------------------------------------------------------------
	VOID* InitOneProduceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOneEquipChangeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 点火,通用分解
	//--------------------------------------------------------------------------------------------
	VOID* InitOneDeComposeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 淬火
	//--------------------------------------------------------------------------------------------
	//VOID* InitOneQuenchProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 副本中不能使用的物品
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInsItemProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 副本中不能使用的技能
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInsSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	// 成就
	VOID* InitOneAchievementProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneAchievementCriteriaProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 副本随机刷怪点
	//--------------------------------------------------------------------------------------------
	VOID* InitOneSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 副本动态刷怪点等级映射表
	//--------------------------------------------------------------------------------------------
	VOID* InitOneLevelMapping(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化副本静态数据
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInstanceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化非副本刷怪点原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOneSSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	// 初始化刷怪点组
	VOID* InitOneSSpawnGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOneSSpawnGuildGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	//--------------------------------------------------------------------------------------------
	// 初始化宠物属性默认最小最大值
	//--------------------------------------------------------------------------------------------
//	VOID InitPetAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物升级原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetLvlUpProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物升级物品原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetLvlUpItemProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物技能原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物装备原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetEquipProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物五行凝结表原型
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetWuXingProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化金牌网吧礼品
	//--------------------------------------------------------------------------------------------
	VOID* InitOneVNBGiftProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化金牌网吧礼品
	//--------------------------------------------------------------------------------------------
	VOID* InitOneVNBEquipProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物采集表
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetGatherProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化天气
	//--------------------------------------------------------------------------------------------
	VOID* InitOneWeatherProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化宠物技能原型
	//--------------------------------------------------------------------------------------------
	VOID* InitPetSkillsVec();

	//--------------------------------------------------------------------------------------------
	// 初始化帮派设施升级需求信息
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildUpgradeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派设施位置信息
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildGradePosProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派练兵场士兵
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildEnlisteePos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派pvp旗帜位置
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildPvPBannerPos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派事务信息
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildAffairProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派技能信息
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// 初始化帮派跑商信息
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildCommerceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCommodityProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCofCProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCofCSPProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOngGuildMaterialProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	//--------------------------------------------------------------------------------------------
	// 初始化个性动作
	//--------------------------------------------------------------------------------------------
	VOID* InitOneMotionProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	//--------------------------------------------------------------------------------------------
	// 初始化装备等级属性加成
	//--------------------------------------------------------------------------------------------
	VOID* InitOneEquipLevelPctProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//-------------------------------------------------------------------------------------------
	// 加载人物升级相关影响静态数据
	//-------------------------------------------------------------------------------------------
	VOID* InitRoleAttLevelUp(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 加载人物一二级属性转换表
	//-------------------------------------------------------------------------------------------
	VOID* InitRoleAttChange(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex);
private:
	
	
	//-------------------------------------------------------------------------------------------
	// 加载装备升级相关影响静态数据
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipLevelProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 加载人物怪物属性的默认值，最小值和最大值
	//-------------------------------------------------------------------------------------------
	VOID* InitAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 加载品级装备属性参数
	//-------------------------------------------------------------------------------------------
	VOID* InitEquipQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 加载时装生成参数
	//-------------------------------------------------------------------------------------------
	VOID* InitFashionQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 加载时装生成时颜色概率
	//-------------------------------------------------------------------------------------------
	VOID* InitFashionColorPct(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 可铭纹装备部位的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOnePosyPosProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 称号的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneTitleProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 氏族珍宝的一条记录
	//-------------------------------------------------------------------------------------------
	VOID* InitOneClanTreasureProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 初始化商城免费物品(仅有一个)
	//-------------------------------------------------------------------------------------------
	VOID* InitOneMallFreeItemProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 初始化帮派某个职位权限
	//-------------------------------------------------------------------------------------------
	VOID* InitOnePosGuildPower(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	VOID* InitOnePosGuildKick(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	VOID* InitOnePosGuildAppoint(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// 初始化VIP摊位
	//-------------------------------------------------------------------------------------------
	VOID* InitOneVIPStallProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	
	//-------------------------------------------------------------------------------------------
	// 初始化公式参数
	//-------------------------------------------------------------------------------------------
	VOID* InitOneFormulaParamProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	
	VOID* InitOnePetRandomSkill( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy );

	//-------------------------------------------------------------------------------------------
	// 初始新手礼包
	//-------------------------------------------------------------------------------------------
	VOID*	InitOneNewRoleGiftProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	//-------------------------------------------------------------------------------------------
	// 初始化掉落
	//-------------------------------------------------------------------------------------------
	VOID* InitCreatureLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitItemSetLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitQuestItemLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// 钓鱼
	//-------------------------------------------------------------------------------------------
	VOID* InitSceneFishingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	VOID* InitVipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// 验证码
	VOID* InitVerificationProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// 彩票机
	VOID* InitLotteryProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// 签到
	VOID* InitSignProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// 魂精
	VOID* InitHuenJingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// 神级
	VOID* InitGodLevelProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// 坐骑
	VOID* InitRaidProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

private:
	static AttRes* m_pInstance;

	//-------------------------------------------------------------------------------------------
	// 加载影响某一技能，某一状态，某一触发器的所有技能列表
	//-------------------------------------------------------------------------------------------
	VOID* LoadModifyMap();

	//-------------------------------------------------------------------------------------------
	// 怪物AI分组
	//-------------------------------------------------------------------------------------------
	VOID* GroupCreatureAI();

	
	file_container*			m_pVar;
	Fun_p					m_pFun;
	INT						m_nMemoryCount;
	std::vector<tstring>				m_vectNameFilter;
	//std::vector<tstring>				m_vectChatFilter;

	tagVariableLen						m_VarLen;
	
	package_map<DWORD, tagItemProto*>			m_mapItemProto;
	package_map<DWORD, tagEquipProto*>			m_mapEquipProto;

	//package_map<DWORD, tagGemProto*>			m_mapGemProto;
	package_map<DWORD, tagEquipQltyPct*>		m_mapEquipQltyPct;


	package_map<DWORD, tagShopProto*>			m_mapShopProto;
	package_map<DWORD, tagDakProto*>			m_mapDakProto;

	package_map<DWORD, tagMallItemProto*>		m_mapMallItemProto;
	package_map<DWORD, tagMallPackProto*>		m_mapMallPackProto;
	tagMallFreeItem						m_MallFreeItemProto;

	package_map<DWORD, tagSuitProto*>			m_mapSuitProto;

    package_map<DWORD, tagSkillProto*>			m_mapSkillProto;
	package_map<DWORD, tagBuffProto*>			m_mapBuffProto;
	package_map<DWORD, tagTriggerProto*>		m_mapTriggerProto;
	//技能学习列表
	package_map<DWORD, tagLearnSkill*>			m_mapLearnSkillListProto;

	package_map<DWORD, tagSkillModify*>		m_mapSkillModify;

	package_map<DWORD, tagCreatureProto*>		m_mapCreatureProto;
	package_map<DWORD, tagCreatureAI*>			m_mapCreatureAI;
	package_map<DWORD, package_list<DWORD>*>			m_mapCreatureAIGroup;
	package_map<INT16,	tagCreatureCamp*>		m_mapCreatureCamp;

	package_map<DWORD, s_level_up_effect*>		m_LevelUpEffect;
	//s_level_up_effect					m_LevelUpEffect[MAX_ROLE_LEVEL+1];		// 下标和等级对应
	tagAttDefMinMax						m_AttDefMinMax[ERA_End];
	package_map<DWORD, s_role_att_change*>		m_RoleAttChange;		
	tagEquipQltyEffect					m_EquipQltyEffect[X_EQUIP_QUALITY_NUM];
	tagFashionGen						m_FashionGen[X_EQUIP_QUALITY_NUM];
	tagFashionColorPct					m_FashionColorPct[X_EQUIP_QUALITY_NUM];
	tagEquipLevelUpEffect				m_EquipLevelUpEffect[MAX_WEAPON_LEVEL+1];//装备升级经验表
	

	// 默认帮派成员权限表
	tagGuildPower						m_GuildPowerDefault[X_GUILD_POS_NUM];

	// 帮派设施升级所需物品
	package_map<DWORD, s_guild_upgrade_need*>	m_GuildUpgradeNeedInfo;

	// 帮派设施位置信息
	package_map<DWORD, s_guild_grade_pos*>		m_GuildGradePosInfo;

	// 帮会练兵场士兵位置
	package_map<INT16,	s_guild_enlistee_pos*>	m_GuildEnlisteePos;

	package_map<INT,	s_guild_pvp_banner_pos*>  m_GuildPvPBannerPos;

	// 帮派事务
	package_map<DWORD, s_guild_affair_info*>	m_GuildAffairInfo;

	// 帮派技能
	package_map<DWORD, tagGuildSkill*>			m_GuildSkillInfo;

	// 帮派跑商
	package_map<DWORD, s_commerce_info*>		m_GuildCommerceInfo;
	package_map<DWORD, s_commodity_proto*>		m_GuildCommodityProto;
	package_map<INT64, s_cof_c_proto*>			m_CofCProto;
	package_map<DWORD, s_cof_csp_proto*>		m_CofCSPProto;
	
	// 帮派材料回收
	package_map<DWORD, tagGuildMaterialReceive*> m_GuildMaterialReceive;

	// 可铭纹装备部位 
	tagConsolidatePos					m_PosyPos[EPosyAtt_End][MAX_CONSOLIDATE_POS_QUANTITY];
	//package_map<DWORD, tagPosyProtoSer*>		m_mapPosyProto;			// 铭纹静态表
	// 可镌刻装备部位
	tagConsolidatePos					m_EngravePos[EEngraveAtt_End][MAX_CONSOLIDATE_POS_QUANTITY];
	//package_map<DWORD, tagEngraveProtoSer*>	m_mapEngraveProto;		// 镌刻静态表
	// 镶嵌,烙印,龙附静态表
	package_map<DWORD, tagConsolidateItem*>	m_mapConsolidateProto;
	// 合成
	package_map<DWORD, s_produce_proto_ser*>	m_mapProduceProto;
	// 变化
	package_map<DWORD, tagEquipChange*>		m_mapEquipChangeProto;
	// 淬火
	//package_map<DWORD, tagQuenchProtoSer*>		m_mapQuenchProto;
	// 点化,装备分解
	package_map<DWORD, s_decompose_proto_ser*>	m_mapDeComposeProto;
	// 称号
	//tagTitleProto						m_TitleProto[MAX_TITLE_NUM];
	package_map<DWORD, tagTitleProto*> m_TitleProto;
	// 成就
	package_map<DWORD,AchievementEntry* >	m_mapAchievementProto;
	package_map<DWORD,AchievementCriteriaEntry* >	m_mapAchievementCriteriaProto;

	// 成就类型对应表
	AchievementCriteriaEntryList m_AchievementCriteriasByType[ete_max_event_num];

	// 成就id对应条件列表
	AchievementCriteriaListByAchievement m_AchievementCriteriaListByAchievement;
	
	// 成就引用对应列表
	AchievementListByReferencedId		m_AchievementListByReferencedId;

	// 氏族珍宝
	tagClanTreasureProto				m_ClanTreasureProto[CLAN_TREASURE_NUM];
	// 副本随机刷怪点
	package_map<DWORD, tagRandSpawnPointInfo*> m_mapSpawnPointProto;
	// 副本静态属性
	package_map<DWORD, tagInstance*>			m_mapInstanceProto;
	// 副本中不能使用的物品
	package_map<DWORD, tagInstanceItem*>		m_mapInstanceItem;
	// 副本中不能使用的技能
	package_map<DWORD, tagInstanceSkill*>		m_mapInstanceSkill;
	// 副本随机刷怪点等级映射表
	//package_map<INT, tagLevelMapping*>			m_mapLevelMapping;
	// 非副本刷怪点
	package_map<INT, tagSSpawnPointProto*>		m_mapSSpawnPoint;
	//刷怪点组
	package_map<INT, tagSSpawnPointProto*>		m_mapSSpawnGroup;
	//帮会矿点数据
	package_map<DWORD, tagGuildSSpawnPointProto*> m_mapGuildSSpawnPoint;
	// 宠物原型
	package_map<DWORD, tagPetProto*>			m_mapPetProto;
	
	// 宠物技能随机表
	package_map<DWORD, s_pet_skill_list*>		m_mapPetSkillListProto;

	// 宠物升级原型
	package_map<DWORD, tagPetLvlUpProto*>		m_mapPetLvlUpProto;

	// 宠物升级原型
	package_map<DWORD, tagPetLvlUpItemProto*>	m_mapPetLvlUpItemProto;

	// 宠物技能原型
	package_map<DWORD, tagPetSkillProto*>		m_mapPetSkillProto;
	
	// 宠物等级对应固定技能id
	std::list<DWORD>					m_PetLevelSkillVec[NUM_PET_VLEVEL];

	// 宠物装备原型
	package_map<DWORD, tagPetEquipProto*>		m_mapPetEquipProto;

	// 宠物采集表
	package_map<DWORD, tagPetGatherProto*>		m_mapPetGatherProto;

	// 宠物五行凝结表
	package_map<DWORD, tagPetWuXingProto*>		m_mapPetWuXingProto;

	package_map<DWORD, s_vnb_gift_proto*>		m_mapVNBGiftProto;
	package_map<DWORD, s_vnb_equip_proto*>		m_mapVNBEquipProto;

	// 宠物属性值
//	tagPetDefMinMax						m_nPetAttDefMinMax[EPA_NUM];

	// VIP摊位租金(下标与ID对应)
	INT32								m_nVIPStallRent[VIP_STALL_MAX_NUM];

	package_map<DWORD, tagMotionProto*>		m_mapMotionProto;

	tagFormulaParam						m_FormulaParam;		//公式用算参数
	
	//装备属性加成表
	package_map<INT16, tagEquipLevelPctProto*>	m_mapEquipLevelPctProto;

	//新手礼包
	package_map<INT16, s_new_role_gift_proto*> m_mapNewRoleGiftProto;

	package_map<DWORD, tagWeatherProto*> m_mapWeather;

	package_map<DWORD, tagNpcFishingProto*> m_mapNpcFishProto;

	package_map<DWORD, tagCreatureLoot*>		m_mapCreatureLoot;		// 怪物掉落
	package_map<DWORD, tagLootItemSet*>		m_mapLootItemSet;		// 掉落物品集合
	package_map<DWORD, tagLootQuestItem*>		m_mapLootQuestItem;		// 掉落任务物品

	package_map<DWORD, vip_proto*> m_mapVipProto;

	package_map<DWORD, s_verification_code*>			m_mapVerificationCode;	// 验证码

	package_map<DWORD, tagLotteryProto*>				m_mapLotteryProto;		// 彩票机类型
	


	package_map<DWORD, tagSignProto*>			m_mapSign;

	package_map<DWORD, tagHunJingProto*>		m_maphuenjing;
	
	package_map<DWORD, tagGodLevelProto*>		m_mapgodlevel;

	package_map<DWORD, tagRaidProto*>			m_mapRaid;

public:
	// GM命令调用
	package_map<DWORD, tagEquipProto*>			GetEquipMap(){return m_mapEquipProto;}	
	
};

//extern AttRes g_attRes;
