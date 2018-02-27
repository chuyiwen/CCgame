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

public: // �ϲ�ȷ������ -- ������ڵ�ͼ�̵߳��ϲ��߳�ִ��
	// ���¼���ָ�������ļ�
	BOOL ReloadItemProto();

	// ���¼����̳�����
	BOOL ReloadMallProto();

	static AttRes* GetInstance();
	static VOID	   Destory();

	

public:
	// ����TypeID���������Ƿ��¼log
	VOID ResetItemLog(s_need_log_item dw_data_id[], INT32 n_num);

public:
	//-------------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------------
	//const s_level_up_effect* GetLevelUpEffect(INT32 nLevel) const { return &m_LevelUpEffect[nLevel]; }
	const s_level_up_effect* GetLevelUpEffect(INT32 id) { return m_LevelUpEffect.find(id); }
	const tagEquipLevelUpEffect* GetEquipLevelUpEffect(INT32 nLevel) const { return &m_EquipLevelUpEffect[nLevel];}
	const s_role_att_change* GetRoleAttChange(DWORD dwClass, DWORD dwAtt1) { return m_RoleAttChange.find(dwClass*1000 + dwAtt1); }
	//-------------------------------------------------------------------------------------------
	// Ĭ��ֵ
	//-------------------------------------------------------------------------------------------
	INT GetAttDefRole(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nDefRole; }
	INT GetAttDefCreature(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nDefCreature; }
	INT GetAttMin(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nMin; }
	INT GetAttMax(INT nIndex) { ASSERT( nIndex > ERA_Null && nIndex < ERA_End ); return m_AttDefMinMax[nIndex].nMax; }

	//-------------------------------------------------------------------------------------------
	// ���˴ʱ�
	//-------------------------------------------------------------------------------------------
	std::vector<tstring>* GetNameFilterWords() 	{ return &m_vectNameFilter; }
	//std::vector<tstring>* GetChatFilterWords() 	{ return &m_vectChatFilter; }

	//-------------------------------------------------------------------------------------------
	// ��ͬ���԰汾���Ƴ���
	//-------------------------------------------------------------------------------------------
	tagVariableLen& GetVariableLen() { return m_VarLen; }

	//-------------------------------------------------------------------------------
	// ��ȡ������ʩ����������Ϣ
	//-------------------------------------------------------------------------------
	BOOL GetGuildUpgradeBaseInfo(BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo);
	BOOL GetGuildUpgradeItemInfo(BYTE eType, BYTE byLevel, OUT tagGuildFacilities& sInfo);

	
	const s_guild_grade_pos* GetGuildGradePosInfo(BYTE eType, BYTE byLevel);

	//-------------------------------------------------------------------------------
	// ��ȡ����������ʿ��
	//-------------------------------------------------------------------------------
	const s_guild_enlistee_pos* GetGuildEnlisteePos(INT16 nLevel) { return m_GuildEnlisteePos.find(nLevel); }

	const s_guild_pvp_banner_pos* GetGuildPvPBannerPos(INT nActID) { return m_GuildPvPBannerPos.find(nActID); }

	//-------------------------------------------------------------------------------
	// ��ȡ����������Ϣ
	//-------------------------------------------------------------------------------
	const s_guild_affair_info* GetGuildAffairInfo(DWORD dwBuffID)	{ return m_GuildAffairInfo.find(dwBuffID); }

	//-------------------------------------------------------------------------------
	// ��ȡ���ɼ�����Ϣ
	//-------------------------------------------------------------------------------
	BOOL get_guild_skill_info(DWORD dwSkillID, INT nLevel, tagGuildSkill& sGuildSkill);
	BOOL LoadGuildSkillInfo(package_map<DWORD, tagGuildSkill*>& mapGuildSkill);
	const tagGuildSkill* GetGuildSkillProto(DWORD dwSkillID)		{ return m_GuildSkillInfo.find(dwSkillID); }

	//-------------------------------------------------------------------------------
	// ��ȡ����������Ϣ
	//-------------------------------------------------------------------------------
	const s_commerce_info*		GetGuildCommerceInfo(INT nLevel);
	const s_commodity_proto*	GetCommodityProto(DWORD dwGoodID)	{ return m_GuildCommodityProto.find(dwGoodID); }
	const s_cof_csp_proto*		GetCofCSPProto(DWORD dwCofCID)		{ return m_CofCSPProto.find(dwCofCID); }
	const tagGuildMaterialReceive* GetMaterialReceive(DWORD dwID)	{ return m_GuildMaterialReceive.find(dwID); }
	const FLOAT					GetCofCProfit(DWORD dwDstID, DWORD dwSrcID);
	BOOL LoadCofCGoodInfo(DWORD dwCofCID, package_map<DWORD, tag_chamber_sell_good*>& mapGoodSell,
		package_map<DWORD, tag_chamber_buy_good*>& mapGoodBuy);
	
	//-------------------------------------------------------------------------------------------
	// ��Ʒ
	//-------------------------------------------------------------------------------------------
	tagItemProto* GetItemProto(DWORD dw_data_id)			{ return m_mapItemProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// װ��
	//-------------------------------------------------------------------------------------------
	tagEquipProto* GetEquipProto(DWORD dw_data_id)		{ return m_mapEquipProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// ����(��ʯ,ӡ��)
	//-------------------------------------------------------------------------------------------
	//tagGemProto* GetGemProto(DWORD dw_data_id)			{ return m_mapGemProto.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// �̵�
	//-------------------------------------------------------------------------------------------
	tagShopProto* GetShopProto(DWORD dwShopID)			{ return m_mapShopProto.find(dwShopID); }

	//-------------------------------------------------------------------------------------------
	// �̳���Ʒ, ��Ʒ���������Ʒ
	//-------------------------------------------------------------------------------------------
	const tagMallItemProto* GetMallItemProto(DWORD dwID){ return m_mapMallItemProto.find(dwID); }
	const tagMallPackProto* GetMallPackProto(DWORD dwID){ return m_mapMallPackProto.find(dwID); }
	const tagMallFreeItem*	GetMallFreeProto()			{ return &m_MallFreeItemProto; }
	
	package_map<DWORD, tagMallItemProto*>& GetMallItem()		 { return m_mapMallItemProto; }
	package_map<DWORD, tagMallPackProto*>& GetMallPack()		 { return m_mapMallPackProto; }

	INT	GetMallItemNum()								{ return m_mapMallItemProto.size(); }
	INT GetMallPackNum()								{ return m_mapMallPackProto.size(); }

	//-------------------------------------------------------------------------------------------
	// ��վ
	//-------------------------------------------------------------------------------------------
	tagDakProto* GetDakProto(DWORD dwDakID)				{ return m_mapDakProto.find(dwDakID); }
	//-------------------------------------------------------------------------------------------
	// װ��Ʒ����������
	//-------------------------------------------------------------------------------------------
	tagEquipQltyPct* GetEquipQltyPct(DWORD dw_data_id)	{ return m_mapEquipQltyPct.find(dw_data_id); }
	//-------------------------------------------------------------------------------------------
	// ��װ��̬����
	//-------------------------------------------------------------------------------------------
	const tagSuitProto* GetSuitProto(DWORD dwSuitID)	{ return m_mapSuitProto.find(dwSuitID); }
	//-------------------------------------------------------------------------------------------
	// Ʒ��װ�����Բ���
	//-------------------------------------------------------------------------------------------
	const tagEquipQltyEffect* GetEquipQltyEffect(INT32 nQlty) const { return &m_EquipQltyEffect[nQlty]; }


	//-------------------------------------------------------------------------------------------
	// ʱװƷ��������Ӱ�����
	//-------------------------------------------------------------------------------------------
	const tagFashionGen* GetFashionQltyEffect(INT32 nQlty) const { return &m_FashionGen[nQlty]; }

	//-------------------------------------------------------------------------------------------
	// ʱװ��ɫ���ɸ��ʲ���
	//-------------------------------------------------------------------------------------------
	const tagFashionColorPct* GetFashionColorPct(INT32 nQlty) const { return &m_FashionColorPct[nQlty]; }
	
	//-------------------------------------------------------------------------------------------
	// �õ�����Ҫ��������Ʒ��ָ���б�
	//-------------------------------------------------------------------------------------------
	package_list<tagItemProto*> GetItemProtoList();
	//-------------------------------------------------------------------------------------------
	// �õ��������Զ�Ӧ��װ���Ƿ��ǿ��
	//-------------------------------------------------------------------------------------------
	BOOL IsPosyPos(EPosyAtt ePosyAtt, EEquipPos eEquipPos);
	//-------------------------------------------------------------------------------------------
	// �õ��Կ����Զ�Ӧ��װ���Ƿ��ǿ��
	//-------------------------------------------------------------------------------------------
	BOOL IsEngravePos(EEngraveAtt ePosyAtt, EEquipPos eEquipPos);

	//-------------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------------
	const tagSkillProto* GetSkillProto(DWORD dw_data_id) { return m_mapSkillProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// ״̬
	//-------------------------------------------------------------------------------------------
	const tagBuffProto* GetBuffProto(DWORD dw_data_id) { return m_mapBuffProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// ������
	//-------------------------------------------------------------------------------------------
	const tagTriggerProto* GetTriggerProto(DWORD dwTriggerID) { return m_mapTriggerProto.find(dwTriggerID); }

	//-------------------------------------------------------------------------------------------
	// ĳ�������Ƿ��ܱ���������Ӱ��
	//-------------------------------------------------------------------------------------------
	BOOL CanBeModified(DWORD dwSkillID) { return m_mapSkillModify.is_exist(dwSkillID); }

	//-------------------------------------------------------------------------------------------
	// �õ���Ӱ��ĳ�����ܵļ����б�
	//-------------------------------------------------------------------------------------------
	tagSkillModify* GetSkillModifier(DWORD dwSkillID) { return m_mapSkillModify.find(dwSkillID); }

	//-------------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------------
	const tagCreatureProto* GetCreatureProto(DWORD dw_data_id) { return m_mapCreatureProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// ����AI
	//-------------------------------------------------------------------------------------------
	const tagCreatureAI* GetCreatureAI(DWORD dwAIID) { return m_mapCreatureAI.find(dwAIID); }

	//-------------------------------------------------------------------------------------------
	// ������Ӫ��ϵ
	//-------------------------------------------------------------------------------------------
	const tagCreatureCamp* GetCreatureCamp(INT16 n16Camp) { return m_mapCreatureCamp.find(n16Camp); }

	//-------------------------------------------------------------------------------------------
	// �����������ͼˢ�ֵ�
	//-------------------------------------------------------------------------------------------
	const tagSSpawnPointProto *GetSSpawnPointProto(DWORD dwSpawnPointID) { return m_mapSSpawnPoint.find(dwSpawnPointID); }
	
	const tagSSpawnPointProto* GetSSpawnGroupProto(DWORD dwSpawnGroupID) { return m_mapSSpawnGroup.find(dwSpawnGroupID); }
	
	const tagGuildSSpawnPointProto* GetSSpawnGuildProto(DWORD dwCreatureID) { return m_mapGuildSSpawnPoint.find(dwCreatureID); }
	//-------------------------------------------------------------------------------------------
	// �������ˢ�ֵ�
	//-------------------------------------------------------------------------------------------
	const tagRandSpawnPointInfo* GetSpawnPointProto(DWORD dwSpawnPoint) { return m_mapSpawnPointProto.find(dwSpawnPoint); }

	//-------------------------------------------------------------------------------------------
	// �������ˢ�ֵ�
	//-------------------------------------------------------------------------------------------
	const tagInstance*	get_instance_proto(DWORD dwMapID)	{ return m_mapInstanceProto.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// �����в���ʹ�õ���Ʒ
	//-------------------------------------------------------------------------------------------
	const tagInstanceItem* GetInstanceItem(DWORD dwMapID) { return m_mapInstanceItem.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// �����в���ʹ�õļ���
	//-------------------------------------------------------------------------------------------
	const tagInstanceSkill* GetInstanceSkill(DWORD dwMapID) { return m_mapInstanceSkill.find(dwMapID); }

	//-------------------------------------------------------------------------------------------
	// �������ˢ�ֵ�ȼ�ӳ���
	//-------------------------------------------------------------------------------------------
	//const tagLevelMapping* GetLevelMapping(INT nLevel) { return m_mapLevelMapping.find(nLevel); }

	//-------------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------------
	//const tagPosyProtoSer* GetPosyProto(DWORD dwPosyID) { return m_mapPosyProto.find(dwPosyID); }

	//-------------------------------------------------------------------------------------------
	// �Կ�
	//-------------------------------------------------------------------------------------------
	//const tagEngraveProtoSer* GetEngraveProto(DWORD dwEngraveID) { return m_mapEngraveProto.find(dwEngraveID); }

	//-------------------------------------------------------------------------------------------
	// ��Ƕ,��ӡ,����
	//-------------------------------------------------------------------------------------------
	const tagConsolidateItem* GetConsolidateProto(DWORD dw_data_id) { return m_mapConsolidateProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// �ϳ�
	//-------------------------------------------------------------------------------------------
	const s_produce_proto_ser* GetProduceProto(DWORD dwFormulaID) { return m_mapProduceProto.find(dwFormulaID); }
	
	//-------------------------------------------------------------------------------------------
	// װ���任
	//-------------------------------------------------------------------------------------------
	const tagEquipChange*	GetEquipChangeProto(DWORD dwId) { return m_mapEquipChangeProto.find(dwId); }
	//-------------------------------------------------------------------------------------------
	// �㻯,װ���ֽ�
	//-------------------------------------------------------------------------------------------
	const s_decompose_proto_ser* GetDeComposeProto(DWORD dwFormulaID) { return m_mapDeComposeProto.find(dwFormulaID); }

	//-------------------------------------------------------------------------------------------
	// ���
	//-------------------------------------------------------------------------------------------
	//const tagQuenchProtoSer *GetQuenchProto(DWORD dwFormulaID) { return m_mapQuenchProto.find(dwFormulaID); }

	//-------------------------------------------------------------------------------------------
	// �ƺ�
	//-------------------------------------------------------------------------------------------
	//const tagTitleProto *GetTitleProto(UINT16 u16TitleID) { return &m_TitleProto[u16TitleID]; }
	const tagTitleProto *GetTitleProto(DWORD u16TitleID) { return m_TitleProto.find(u16TitleID); }
	// �ɾ�
	// ��ȡ�ɾ�
	const AchievementEntry* GetAchievementProto(DWORD id) { return m_mapAchievementProto.find(id); }
	// ��ȡ����
	const AchievementCriteriaEntry*	GetAchievementCriteriaProto(DWORD id) { return m_mapAchievementCriteriaProto.find(id); }
	// ͨ�����ͻ�ȡ��������
	AchievementCriteriaEntryList const& GetAchievementCriteriaByType(e_achievement_event type)
	{
		 return m_AchievementCriteriasByType[type];
	}
	
	// ��ȡ��Ӧ�ɾ͵���������
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
	// �����䱦
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
	// ��ó�������Ĭ����С���ֵ
	//-------------------------------------------------------------------------------------------
// 	INT GetPetDef(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nDef;	}
// 	INT GetPetMin(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nMin;	}
// 	INT GetPetMax(INT nPetAtt) { ASSERT(IS_EPA(nPetAtt));	return m_nPetAttDefMinMax[nPetAtt].nMax;	}

	//-------------------------------------------------------------------------------------------
	// ��ó���ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetProto* GetPetProto(DWORD dwPetTypeID) { return m_mapPetProto.find(dwPetTypeID); }
	
	//-------------------------------------------------------------------------------------------
	//��ó��＼������б�
	//-------------------------------------------------------------------------------------------
	const s_pet_skill_list* GetPetRandomSkill(DWORD dwID) { return m_mapPetSkillListProto.find(dwID); }
	//-------------------------------------------------------------------------------------------
	// ��ó�������ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetLvlUpProto* GetPetLvlUpProto(DWORD dwVLevel) { return m_mapPetLvlUpProto.find(dwVLevel); }

	//-------------------------------------------------------------------------------------------
	// ��ó�������ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetLvlUpItemProto* GetPetLvlUpItemProto(DWORD dw_data_id) { return m_mapPetLvlUpItemProto.find(dw_data_id); }

	//-------------------------------------------------------------------------------------------
	// ��ó��＼��ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetSkillProto* GetPetSkillProto(DWORD dwPetSkillTypeID) { return m_mapPetSkillProto.find(dwPetSkillTypeID); }

	//-------------------------------------------------------------------------------------------
	// ��ó���װ��ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetEquipProto* GetPetEquipProto(DWORD dwPetEquipTypeID) { return m_mapPetEquipProto.find(dwPetEquipTypeID); }

	//-------------------------------------------------------------------------------------------
	// ��ó�����������ԭ��
	//-------------------------------------------------------------------------------------------
	const tagPetWuXingProto* GetPetWuXingProto(DWORD dwPetWuXingTypeID) { return m_mapPetWuXingProto.find(dwPetWuXingTypeID); }

	//-------------------------------------------------------------------------------------------
	// ��ó��＼������
	//-------------------------------------------------------------------------------------------
	const std::list<DWORD>& GetPetNormalSkillList(INT nPetLevel) { return m_PetLevelSkillVec[nPetLevel - 1]; }

	//-------------------------------------------------------------------------------------------
	// VIP̯λ��Ϣ
	//-------------------------------------------------------------------------------------------
	const INT32 GetVIPStallRent(INT nIndex) { return m_nVIPStallRent[nIndex]; }

	//-------------------------------------------------------------------------------------------
	// ��ö���ԭ��
	//-------------------------------------------------------------------------------------------
	const tagMotionProto* GetMotionProto(DWORD dw_data_id) { return m_mapMotionProto.find(dw_data_id); }
	
	//��ʽ����
	const tagFormulaParam* GetFormulaParam(){return &m_FormulaParam;}
	
	//���װ�����Լӳ�
	const tagEquipLevelPctProto* GetEquipLevelPct(INT16 n16Index){return m_mapEquipLevelPctProto.find(n16Index);}
	
	//��ü���ѧϰ�б�
	const tagLearnSkill*	GetLearnSkillProto(DWORD dwID) { return m_mapLearnSkillListProto.find(dwID); }

	// ����������
	const s_new_role_gift_proto* GetNewRoleGift(INT16 nID) { return m_mapNewRoleGiftProto.find(nID); }

	const tagWeatherProto* GetWeather(DWORD dwID) { return m_mapWeather.find(dwID); }
	
	// ��ȡ�������
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
	// ���ó�ʼ��ĳ����Դ��һ����¼�Ĵ�����
	//-------------------------------------------------------------------------------------------
	VOID SetInitOneRes(Fun_p pFun) { m_pFun = pFun; }

	//-------------------------------------------------------------------------------------------
	// ��ʼ����Դ��ģ�溯�� -- ����map��
	//-------------------------------------------------------------------------------------------
	template<class K, class T> 
	BOOL LoadResMap(package_map<K, T*> &mapRes, LPCTSTR szFileName, LPCTSTR szFileName2 = NULL);

	//-------------------------------------------------------------------------------------------
	// �ͷ���Դ��ģ�溯��
	//-------------------------------------------------------------------------------------------
	template<class K, class T> VOID FreeResMap(package_map<K, T*> &mapRes);

	//-------------------------------------------------------------------------------------------
	// ��ʼ����Դ��ģ�溯�� -- ����array��
	//-------------------------------------------------------------------------------------------
	template<class T>
	BOOL LoadResArray(T *arrayRes, INT32 nIndexStart, INT32 nIndexEnd, LPCTSTR szFileName);

private:
	//-------------------------------------------------------------------------------------------
	// ��ʼ�����˴ʱ�
	//-------------------------------------------------------------------------------------------
	BOOL InitFilterWords(OUT std::vector<tstring>& vectFilterWords, LPCTSTR szFileName);

	//-------------------------------------------------------------------------------------------
	// �ӽű��������л�ȡ��������
	//-------------------------------------------------------------------------------------------
	VOID InitVarLen();

private:
	//-------------------------------------------------------------------------------------------
	// ��ʼ����Ʒ��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��װ����һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ����ʯ�ȵ�һ����¼
	//-------------------------------------------------------------------------------------------
	//VOID* InitOneGemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ����װ��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneSuitProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��һ��װ��Ʒ����������
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipQltyPct(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ���̵�
	//-------------------------------------------------------------------------------------------
	VOID* InitOneShopProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ���̳���Ʒ����Ʒ��
	//-------------------------------------------------------------------------------------------
	VOID*  InitOneMallItemProtoBase(OUT LPVOID pProtoType, IN LPCTSTR szField);
	VOID* InitOneMallItemProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitOneMallPackProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ����վ
	//-------------------------------------------------------------------------------------------
	VOID* InitOneDakProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ�����ܵ�һ����¼
	//-------------------------------------------------------------------------------------------
    VOID* InitOneSkillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��buff��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneBuffProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��trigger��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneTriggerProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��ѧϰ���ܵ�һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneLearnSKillProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��Creature��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��Creature_ai��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureAIProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��Creature_Camp��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneCreatureCampProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ�����Ƶ�һ����¼
	//--------------------------------------------------------------------------------------------
	//VOID* InitOnePosyProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ���Կ̵�һ����¼
	//--------------------------------------------------------------------------------------------
	//VOID* InitOneEngraveProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ����Ƕ,��ӡ,������һ����¼
	//--------------------------------------------------------------------------------------------
	VOID* InitOneConsolidateProto(OUT LPVOID pProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������ܵ�һ����¼
	//--------------------------------------------------------------------------------------------
	VOID* InitOneProduceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOneEquipChangeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ���,ͨ�÷ֽ�
	//--------------------------------------------------------------------------------------------
	VOID* InitOneDeComposeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ���
	//--------------------------------------------------------------------------------------------
	//VOID* InitOneQuenchProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// �����в���ʹ�õ���Ʒ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInsItemProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// �����в���ʹ�õļ���
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInsSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	// �ɾ�
	VOID* InitOneAchievementProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneAchievementCriteriaProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// �������ˢ�ֵ�
	//--------------------------------------------------------------------------------------------
	VOID* InitOneSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ������̬ˢ�ֵ�ȼ�ӳ���
	//--------------------------------------------------------------------------------------------
	VOID* InitOneLevelMapping(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ��������̬����
	//--------------------------------------------------------------------------------------------
	VOID* InitOneInstanceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ���Ǹ���ˢ�ֵ�ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOneSSpawnPointProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	// ��ʼ��ˢ�ֵ���
	VOID* InitOneSSpawnGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOneSSpawnGuildGroupProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	//--------------------------------------------------------------------------------------------
	// ��ʼ����������Ĭ����С���ֵ
	//--------------------------------------------------------------------------------------------
//	VOID InitPetAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ����������ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetLvlUpProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������Ʒԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetLvlUpItemProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ�����＼��ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������װ��ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetEquipProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ���������������ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetWuXingProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������Ʒ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneVNBGiftProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������Ʒ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneVNBEquipProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������ɼ���
	//--------------------------------------------------------------------------------------------
	VOID* InitOnePetGatherProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������
	//--------------------------------------------------------------------------------------------
	VOID* InitOneWeatherProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ�����＼��ԭ��
	//--------------------------------------------------------------------------------------------
	VOID* InitPetSkillsVec();

	//--------------------------------------------------------------------------------------------
	// ��ʼ��������ʩ����������Ϣ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildUpgradeProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ��������ʩλ����Ϣ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildGradePosProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������ʿ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildEnlisteePos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������pvp����λ��
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildPvPBannerPos(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������Ϣ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildAffairProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ�����ɼ�����Ϣ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildSkillProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//--------------------------------------------------------------------------------------------
	// ��ʼ������������Ϣ
	//--------------------------------------------------------------------------------------------
	VOID* InitOneGuildCommerceProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCommodityProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCofCProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	VOID* InitOneCofCSPProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	VOID* InitOngGuildMaterialProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	//--------------------------------------------------------------------------------------------
	// ��ʼ�����Զ���
	//--------------------------------------------------------------------------------------------
	VOID* InitOneMotionProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	//--------------------------------------------------------------------------------------------
	// ��ʼ��װ���ȼ����Լӳ�
	//--------------------------------------------------------------------------------------------
	VOID* InitOneEquipLevelPctProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);

	//-------------------------------------------------------------------------------------------
	// ���������������Ӱ�쾲̬����
	//-------------------------------------------------------------------------------------------
	VOID* InitRoleAttLevelUp(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ��������һ��������ת����
	//-------------------------------------------------------------------------------------------
	VOID* InitRoleAttChange(OUT LPVOID nProtoType, IN LPCTSTR szField, INT32 nIndex);
private:
	
	
	//-------------------------------------------------------------------------------------------
	// ����װ���������Ӱ�쾲̬����
	//-------------------------------------------------------------------------------------------
	VOID* InitOneEquipLevelProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ��������������Ե�Ĭ��ֵ����Сֵ�����ֵ
	//-------------------------------------------------------------------------------------------
	VOID* InitAttDefMinMax(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ����Ʒ��װ�����Բ���
	//-------------------------------------------------------------------------------------------
	VOID* InitEquipQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ����ʱװ���ɲ���
	//-------------------------------------------------------------------------------------------
	VOID* InitFashionQltyEffect(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ����ʱװ����ʱ��ɫ����
	//-------------------------------------------------------------------------------------------
	VOID* InitFashionColorPct(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ������װ����λ��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOnePosyPosProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// �ƺŵ�һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneTitleProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// �����䱦��һ����¼
	//-------------------------------------------------------------------------------------------
	VOID* InitOneClanTreasureProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ��ʼ���̳������Ʒ(����һ��)
	//-------------------------------------------------------------------------------------------
	VOID* InitOneMallFreeItemProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ��ʼ������ĳ��ְλȨ��
	//-------------------------------------------------------------------------------------------
	VOID* InitOnePosGuildPower(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	VOID* InitOnePosGuildKick(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	VOID* InitOnePosGuildAppoint(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);

	//-------------------------------------------------------------------------------------------
	// ��ʼ��VIP̯λ
	//-------------------------------------------------------------------------------------------
	VOID* InitOneVIPStallProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	
	//-------------------------------------------------------------------------------------------
	// ��ʼ����ʽ����
	//-------------------------------------------------------------------------------------------
	VOID* InitOneFormulaParamProto(OUT LPVOID pArray, IN LPCTSTR szField, INT32 nIndex);
	
	VOID* InitOnePetRandomSkill( OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy );

	//-------------------------------------------------------------------------------------------
	// ��ʼ�������
	//-------------------------------------------------------------------------------------------
	VOID*	InitOneNewRoleGiftProto(OUT LPVOID nProtoType, LPCTSTR szField, INT nDummy);
	
	//-------------------------------------------------------------------------------------------
	// ��ʼ������
	//-------------------------------------------------------------------------------------------
	VOID* InitCreatureLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitItemSetLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	VOID* InitQuestItemLootProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	//-------------------------------------------------------------------------------------------
	// ����
	//-------------------------------------------------------------------------------------------
	VOID* InitSceneFishingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	VOID* InitVipProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// ��֤��
	VOID* InitVerificationProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// ��Ʊ��
	VOID* InitLotteryProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// ǩ��
	VOID* InitSignProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

	// �꾫
	VOID* InitHuenJingProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// ��
	VOID* InitGodLevelProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);
	
	// ����
	VOID* InitRaidProto(OUT LPVOID pProtoType, IN LPCTSTR szField, INT32 nDummy);

private:
	static AttRes* m_pInstance;

	//-------------------------------------------------------------------------------------------
	// ����Ӱ��ĳһ���ܣ�ĳһ״̬��ĳһ�����������м����б�
	//-------------------------------------------------------------------------------------------
	VOID* LoadModifyMap();

	//-------------------------------------------------------------------------------------------
	// ����AI����
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
	//����ѧϰ�б�
	package_map<DWORD, tagLearnSkill*>			m_mapLearnSkillListProto;

	package_map<DWORD, tagSkillModify*>		m_mapSkillModify;

	package_map<DWORD, tagCreatureProto*>		m_mapCreatureProto;
	package_map<DWORD, tagCreatureAI*>			m_mapCreatureAI;
	package_map<DWORD, package_list<DWORD>*>			m_mapCreatureAIGroup;
	package_map<INT16,	tagCreatureCamp*>		m_mapCreatureCamp;

	package_map<DWORD, s_level_up_effect*>		m_LevelUpEffect;
	//s_level_up_effect					m_LevelUpEffect[MAX_ROLE_LEVEL+1];		// �±�͵ȼ���Ӧ
	tagAttDefMinMax						m_AttDefMinMax[ERA_End];
	package_map<DWORD, s_role_att_change*>		m_RoleAttChange;		
	tagEquipQltyEffect					m_EquipQltyEffect[X_EQUIP_QUALITY_NUM];
	tagFashionGen						m_FashionGen[X_EQUIP_QUALITY_NUM];
	tagFashionColorPct					m_FashionColorPct[X_EQUIP_QUALITY_NUM];
	tagEquipLevelUpEffect				m_EquipLevelUpEffect[MAX_WEAPON_LEVEL+1];//װ�����������
	

	// Ĭ�ϰ��ɳ�ԱȨ�ޱ�
	tagGuildPower						m_GuildPowerDefault[X_GUILD_POS_NUM];

	// ������ʩ����������Ʒ
	package_map<DWORD, s_guild_upgrade_need*>	m_GuildUpgradeNeedInfo;

	// ������ʩλ����Ϣ
	package_map<DWORD, s_guild_grade_pos*>		m_GuildGradePosInfo;

	// ���������ʿ��λ��
	package_map<INT16,	s_guild_enlistee_pos*>	m_GuildEnlisteePos;

	package_map<INT,	s_guild_pvp_banner_pos*>  m_GuildPvPBannerPos;

	// ��������
	package_map<DWORD, s_guild_affair_info*>	m_GuildAffairInfo;

	// ���ɼ���
	package_map<DWORD, tagGuildSkill*>			m_GuildSkillInfo;

	// ��������
	package_map<DWORD, s_commerce_info*>		m_GuildCommerceInfo;
	package_map<DWORD, s_commodity_proto*>		m_GuildCommodityProto;
	package_map<INT64, s_cof_c_proto*>			m_CofCProto;
	package_map<DWORD, s_cof_csp_proto*>		m_CofCSPProto;
	
	// ���ɲ��ϻ���
	package_map<DWORD, tagGuildMaterialReceive*> m_GuildMaterialReceive;

	// ������װ����λ 
	tagConsolidatePos					m_PosyPos[EPosyAtt_End][MAX_CONSOLIDATE_POS_QUANTITY];
	//package_map<DWORD, tagPosyProtoSer*>		m_mapPosyProto;			// ���ƾ�̬��
	// ���Կ�װ����λ
	tagConsolidatePos					m_EngravePos[EEngraveAtt_End][MAX_CONSOLIDATE_POS_QUANTITY];
	//package_map<DWORD, tagEngraveProtoSer*>	m_mapEngraveProto;		// �Կ̾�̬��
	// ��Ƕ,��ӡ,������̬��
	package_map<DWORD, tagConsolidateItem*>	m_mapConsolidateProto;
	// �ϳ�
	package_map<DWORD, s_produce_proto_ser*>	m_mapProduceProto;
	// �仯
	package_map<DWORD, tagEquipChange*>		m_mapEquipChangeProto;
	// ���
	//package_map<DWORD, tagQuenchProtoSer*>		m_mapQuenchProto;
	// �㻯,װ���ֽ�
	package_map<DWORD, s_decompose_proto_ser*>	m_mapDeComposeProto;
	// �ƺ�
	//tagTitleProto						m_TitleProto[MAX_TITLE_NUM];
	package_map<DWORD, tagTitleProto*> m_TitleProto;
	// �ɾ�
	package_map<DWORD,AchievementEntry* >	m_mapAchievementProto;
	package_map<DWORD,AchievementCriteriaEntry* >	m_mapAchievementCriteriaProto;

	// �ɾ����Ͷ�Ӧ��
	AchievementCriteriaEntryList m_AchievementCriteriasByType[ete_max_event_num];

	// �ɾ�id��Ӧ�����б�
	AchievementCriteriaListByAchievement m_AchievementCriteriaListByAchievement;
	
	// �ɾ����ö�Ӧ�б�
	AchievementListByReferencedId		m_AchievementListByReferencedId;

	// �����䱦
	tagClanTreasureProto				m_ClanTreasureProto[CLAN_TREASURE_NUM];
	// �������ˢ�ֵ�
	package_map<DWORD, tagRandSpawnPointInfo*> m_mapSpawnPointProto;
	// ������̬����
	package_map<DWORD, tagInstance*>			m_mapInstanceProto;
	// �����в���ʹ�õ���Ʒ
	package_map<DWORD, tagInstanceItem*>		m_mapInstanceItem;
	// �����в���ʹ�õļ���
	package_map<DWORD, tagInstanceSkill*>		m_mapInstanceSkill;
	// �������ˢ�ֵ�ȼ�ӳ���
	//package_map<INT, tagLevelMapping*>			m_mapLevelMapping;
	// �Ǹ���ˢ�ֵ�
	package_map<INT, tagSSpawnPointProto*>		m_mapSSpawnPoint;
	//ˢ�ֵ���
	package_map<INT, tagSSpawnPointProto*>		m_mapSSpawnGroup;
	//���������
	package_map<DWORD, tagGuildSSpawnPointProto*> m_mapGuildSSpawnPoint;
	// ����ԭ��
	package_map<DWORD, tagPetProto*>			m_mapPetProto;
	
	// ���＼�������
	package_map<DWORD, s_pet_skill_list*>		m_mapPetSkillListProto;

	// ��������ԭ��
	package_map<DWORD, tagPetLvlUpProto*>		m_mapPetLvlUpProto;

	// ��������ԭ��
	package_map<DWORD, tagPetLvlUpItemProto*>	m_mapPetLvlUpItemProto;

	// ���＼��ԭ��
	package_map<DWORD, tagPetSkillProto*>		m_mapPetSkillProto;
	
	// ����ȼ���Ӧ�̶�����id
	std::list<DWORD>					m_PetLevelSkillVec[NUM_PET_VLEVEL];

	// ����װ��ԭ��
	package_map<DWORD, tagPetEquipProto*>		m_mapPetEquipProto;

	// ����ɼ���
	package_map<DWORD, tagPetGatherProto*>		m_mapPetGatherProto;

	// �������������
	package_map<DWORD, tagPetWuXingProto*>		m_mapPetWuXingProto;

	package_map<DWORD, s_vnb_gift_proto*>		m_mapVNBGiftProto;
	package_map<DWORD, s_vnb_equip_proto*>		m_mapVNBEquipProto;

	// ��������ֵ
//	tagPetDefMinMax						m_nPetAttDefMinMax[EPA_NUM];

	// VIP̯λ���(�±���ID��Ӧ)
	INT32								m_nVIPStallRent[VIP_STALL_MAX_NUM];

	package_map<DWORD, tagMotionProto*>		m_mapMotionProto;

	tagFormulaParam						m_FormulaParam;		//��ʽ�������
	
	//װ�����Լӳɱ�
	package_map<INT16, tagEquipLevelPctProto*>	m_mapEquipLevelPctProto;

	//�������
	package_map<INT16, s_new_role_gift_proto*> m_mapNewRoleGiftProto;

	package_map<DWORD, tagWeatherProto*> m_mapWeather;

	package_map<DWORD, tagNpcFishingProto*> m_mapNpcFishProto;

	package_map<DWORD, tagCreatureLoot*>		m_mapCreatureLoot;		// �������
	package_map<DWORD, tagLootItemSet*>		m_mapLootItemSet;		// ������Ʒ����
	package_map<DWORD, tagLootQuestItem*>		m_mapLootQuestItem;		// ����������Ʒ

	package_map<DWORD, vip_proto*> m_mapVipProto;

	package_map<DWORD, s_verification_code*>			m_mapVerificationCode;	// ��֤��

	package_map<DWORD, tagLotteryProto*>				m_mapLotteryProto;		// ��Ʊ������
	


	package_map<DWORD, tagSignProto*>			m_mapSign;

	package_map<DWORD, tagHunJingProto*>		m_maphuenjing;
	
	package_map<DWORD, tagGodLevelProto*>		m_mapgodlevel;

	package_map<DWORD, tagRaidProto*>			m_mapRaid;

public:
	// GM�������
	package_map<DWORD, tagEquipProto*>			GetEquipMap(){return m_mapEquipProto;}	
	
};

//extern AttRes g_attRes;
