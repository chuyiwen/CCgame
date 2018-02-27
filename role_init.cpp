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
*	@file		role_init.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		初始化人物数据结构
*/

#include "StdAfx.h"

#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../../common/WorldDefine/Raid_define.h"

#include "role.h"
#include "att_res.h"
#include "skill.h"
#include "quest.h"
#include "quest_mgr.h"
#include "map_creator.h"
#include "script_mgr.h"
#include "title_mgr.h"
#include "pet_pocket.h"
#include "vip_netbar.h"
#include "guild.h"
#include "guild_manager.h"
#include "pet_server_define.h"
#include "map_instance.h"
#include "hearSay_helper.h"
#include "RankMgr.h"

//-------------------------------------------------------------------------------
// 初始化玩家
//-------------------------------------------------------------------------------
VOID Role::Init(const s_role_data_load* pData)
{
	ASSERT( VALID_POINT(pData) );

	const s_role_data_const* pDataConst = &pData->role_data_const_;
	const s_role_data_save* pDataSave = &pData->role_data_save_;

	//
	mCarryTypeID = 0;
	
	//
	ZeroMemory(&ride_data_, sizeof(ride_data_));

	// 初始化玩家脚本
	m_pScript = g_ScriptMgr.GetRoleScript();

	// 玩家脚本数据
	get_fast_code()->memory_copy(m_ScriptData.dwData, pDataSave->data_.dwData, sizeof(DWORD)*ESD_Role);

	// 称号管理器
	m_pTitleMgr		= new TitleMgr;

	// 新建宠物带
	m_pPetPocket	= new PetPocket;
	
	// 初始化常规属性
	m_eClass = pDataSave->e_class_;
	get_fast_code()->memory_copy(&m_Avatar, &pDataConst->avatar, sizeof(m_Avatar));
	m_Avatar.byClass = (BYTE)pDataSave->e_class_;
	m_DisplaySet = pDataSave->display_set_;
	m_dwRebornMapID = pDataSave->dw_reborn_map_id_;
	m_CreatTime = pDataConst->create_time_;
	m_LoginTempTime = pDataSave->login_time_;
	m_LoginTime = g_world.GetWorldTime();		// 重置上线时间
	m_LogoutTime = pDataSave->logout_time_;
	m_nOnlineTime = pDataSave->n_online_time_;
	m_nCurOnlineTime = pDataSave->n_cur_online_time_;
	m_eClassEx = pDataSave->e_class_ex_;
	m_bHasLeftMsg = pDataSave->h_has_left_msg_;
	m_LeaveGuildTime = pDataSave->leave_guild_time_;
	m_nSendMailNum = pDataSave->n_send_mail_num_;
	m_dwMasterID = pDataSave->dw_master_id_;
	//m_dwMasterPrenticeForbidTime = 0;
	m_dwMasterPrenticeForbidTime = pDataSave->dw_master_prentice_forbid_time_;//gx add 2013.12.10 师徒决裂要加入惩罚
	m_n16HangNum = pDataSave->n16_hang_num_;
	m_bExp = pDataSave->b_exp_;
	m_bBrotherhood = pDataSave->b_brotherhood_;
	m_nLeaveExp = pDataSave->n_leave_exp_;
	m_nLeaveBrotherHood = pDataSave->n_leave_brotherhood_;
	m_nTotalMasterMoral = pDataSave->n_total_mater_moral_;
	m_nKillNum = pDataSave->n_kill_num_;
	e_role_camp = pDataSave->e_role_camp;
	n_paimai_limit = pDataSave->n_paimai_limit;
	n_bank_limit = pDataSave->n_bank_limit;
	n_shop_exploits_limit = pDataSave->n_shop_exploits_limit;
	dw_today_online_tick_ = pDataSave->dw_today_online_tick_;
	dw_history_vigour_cost_ = pDataSave->dw_history_vigour_cost_;
	m_PerdayGetVigourTotal = pDataSave->dw_perday_get_vigour_total;
	n16_exbag_step = pDataSave->n16_exbag_step;
	n16_exware_step = pDataSave->n16_exware_step;
	m_nYuanbaoExchangeNum = pDataSave->n_yuanbao_exchange_num;
	m_nAchievementPoint = pDataSave->n_achievement_point;
	m_dwForbidTalkStart = pDataSave->dw_forbid_talk_start_;
	m_dwForbidTalkEnd = pDataSave->dw_forbid_talk_end_;
	m_nGraduates = pDataSave->n_graduates_num;
	mCoolDownReviveCD = pDataSave->n_CoolDownReviveCD;
	mPerDayHangGetExpTimeMS = pDataSave->n_PerDayHangGetExpTimeMS;
	SetCirleRefresh(pDataSave->n_circle_quest_fresh);
	SetCirlePerdayNumber(pDataSave->n_circle_quest_perdaynum);
	SetCirleRefreshMax(pDataSave->n_circle_quest_freshMax);
	

	m_n32_active_num = pDataSave->n32_active_num;
	m_n32_guild_active_num = pDataSave->n32_guild_active_num;
	n32_justice = pDataSave->n32_rating;
	//mIsPurpureDec = pDataSave->b_PurpureDec;
	dw_shihun = pDataSave->dw_shihun;
	n16_pet_xiulian_size = pDataSave->n16_pet_xiulian_size;
		
	m_nInstanceData = pDataSave->nInstanceData;
	m_nInstanceShaodangTime = pDataSave->nInstanceShaodang;
	m_nShaodangIndex = pDataSave->nSaodangIndex;
	get_fast_code()->memory_copy(&st_1v1_scroe, &pDataSave->st_1v1_score, sizeof(st_1v1_scroe));

	get_fast_code()->memory_copy(m_szSignature_name, pDataSave->sz_signature_name, sizeof(pDataSave->sz_signature_name));
	// 初始化上次获取商城免费物品时间
	SetLastGetMallFreeTime(pDataSave->dw_time_get_mall_free_);

	// 根据行囊是否加密设置行囊限制
	//m_RoleStateEx.SetState(ERSE_BagPsdExist);
	/*if(GetSession() && !GetSession()->IsHaveBagPsd())
	{
		m_RoleStateEx.UnsetState(ERSE_BagPsdExist);
		m_RoleStateEx.SetState(ERSE_BagPsdPass);
	}*/

	
	// 初始化成就
	m_achievementMgr.InitOpts(this);
	// PK相关
	m_iPKValue = pDataSave->n_pk_value_;
	if(m_iPKValue) m_dwPKValueDecTime = g_world.GetWorldTime();
	get_fast_code()->memory_copy(m_Talent, pDataSave->talent_, sizeof(m_Talent));
	
	m_dwDestoryEquipCount = pDataSave->dw_destory_equip_count;

	// 初始新手礼品
	initRoleGift(pDataSave->n_gift_step_, pDataSave->dw_gift_leave_time_);
	//m_stNewRoleGift.init(pDataSave->n_gift_id_, pDataSave->n_gift_step_, pDataSave->dw_gift_id_, pDataSave->dw_gift_leave_time_, pDataSave->b_gift_get_);
	

// 	if(pDataSave->bSafeGuard)
// 	{
// 		SetRoleState(ERS_Safeguard, FALSE);
// 	}

	// 修正1v1竞技积分
	if(st_1v1_scroe.n_cur_score > st_1v1_scroe.n_day_max_score)
	{
		st_1v1_scroe.n_cur_score = st_1v1_scroe.n_day_max_score;
	}

	if(st_1v1_scroe.n_cur_score < 0)
	{
		st_1v1_scroe.n_cur_score = 0;
	}
	// 帮派
	m_dwGuildID = pDataSave->dw_guild_id;
	if (VALID_VALUE(m_dwGuildID))
	{
		// 检查玩家是否在帮派中
		guild* pGuild	= g_guild_manager.get_guild(m_dwGuildID);
		if (!VALID_POINT(pGuild) || !VALID_POINT(pGuild->get_member(GetID())))
		{
			SetGuildID(INVALID_VALUE);
			SetLeaveGuildTime();
		}
		else
		{
			// 跑商状态
			guild_commodity* pCommodity = pGuild->get_guild_commerce().get_commodity(m_dwID);
			if (VALID_POINT(pCommodity))
			{
				SetRoleState(ERS_Commerce, FALSE);
			}
		}
	}

	m_dwOwnInstanceID = pDataSave->dw_own_instance_id_;
	m_dwOwnInstanceMapID = pDataSave->dw_own_instance_map_id_;
	ZeroMemory(&m_dwInstanceCreateTime, sizeof(tagDWORDTime));
	m_dwInstanceCreateTime = pDataSave->dw_instance_create_time_;
	//副本判定
	Map* pInstance = g_mapCreator.get_map(m_dwOwnInstanceMapID, m_dwOwnInstanceID);
	if(!VALID_POINT(pInstance))
	{
		// 如果副本不存在，清空标记
		m_dwOwnInstanceID = INVALID_VALUE;
		m_dwOwnInstanceMapID = INVALID_VALUE;
		m_dwInstanceCreateTime = 0;
	}
	else
	{
		if(pInstance->get_map_type() == EMT_Instance)
		{
			// 如果副本存在，但是创建时间不对，清空标记
			if(((map_instance_normal*)pInstance)->get_create_time() != m_dwInstanceCreateTime)
			{
				m_dwOwnInstanceID = INVALID_VALUE;
				m_dwOwnInstanceMapID = INVALID_VALUE;
				m_dwInstanceCreateTime = 0;
			}
		}
	}

	// 人物帮助数据
	ZeroMemory(m_byRoleHelp, sizeof(m_byRoleHelp));
	get_fast_code()->memory_copy(m_byRoleHelp, pDataSave->by_role_help_, sizeof(m_byRoleHelp));

	// 人物对话数据
	ZeroMemory(m_byTalkData, sizeof(m_byTalkData));
	get_fast_code()->memory_copy(m_byTalkData, pDataSave->by_talk_data_, sizeof(m_byTalkData));

	// 快捷键数据
	ZeroMemory(&m_stKeyInfo, sizeof(m_stKeyInfo));
	get_fast_code()->memory_copy(&m_stKeyInfo, &pDataSave->st_key_info_, sizeof(m_stKeyInfo));
	
	// 清零数据
	ZeroMemory(m_byDayClear, sizeof(m_byDayClear));
	get_fast_code()->memory_copy(m_byDayClear, pDataSave->by_role_day_clear, sizeof(m_byDayClear));

	ZeroMemory(m_n32_active_data, sizeof(m_n32_active_data));
	get_fast_code()->memory_copy(m_n32_active_data, pDataSave->n32_active_data, sizeof(m_n32_active_data));
	
	ZeroMemory(m_n32_guild_active_data, sizeof(m_n32_guild_active_data));
	get_fast_code()->memory_copy(m_n32_guild_active_data, pDataSave->n32_guild_active_data, sizeof(m_n32_guild_active_data));

	ZeroMemory(m_b_active_receive, sizeof(m_b_active_receive));
	get_fast_code()->memory_copy(m_b_active_receive, pDataSave->b_active_receive, sizeof(m_b_active_receive));
	
	ZeroMemory(m_b_guild_active_receive, sizeof(m_b_guild_active_receive));
	get_fast_code()->memory_copy(m_b_guild_active_receive, pDataSave->b_guild_active_receive, sizeof(m_b_guild_active_receive));


	// 读取并设置人物数据库属性
	InitAtt(pDataSave);

	// 计算每天登入
	AccountSignLevel();


	// 初始化换随机任务
	circle_quest_man_.Init(&pDataSave->circle_quest_man_);

	// todo：读取技能列表，装备列表，buff列表，并计算各自对人物属性的影响
	/*********************************************************************************************************
	*技能列表，状态列表，称号列表，称号条件列表，物品列表，装备列表，好友列表，仇敌列表，任务列表，已完成任务列表，角色名帖
	*必须按顺序读取(该顺序和tagRoleDataSave中对应)
	*********************************************************************************************************/

	const BYTE *pDBData = &pDataSave->by_data[0];	// 列表数据指针,该指针需在读取函数中移动,即pDBData的值是变化的
	
	// 初始化技能列表
	InitSkill(pDBData, pDataSave->n_skill_num_);

	// 初始化状态列表
	InitBuff(pDBData, pDataSave->n_buff_num_);

	// 初始化称号列表
	m_pTitleMgr->InitTitles(pDBData, pDataSave->n_title_num_);

	// 初始化物品装备列表
	InitItem(pDBData, pDataSave->n_item_num_);

	// 初始化好友仇敌列表
	InitFriend(pDBData, pDataSave->n_friend_num_);

	// 初始化仇人
	InitEnemyList(pDBData, pDataSave->n_enemy_num_);
	
	// 初始化玩家当前任务列表
	init_current_quest(pDBData, pDataSave->n_quest_num_);

	// 初始化玩家完成任务列表
	init_complete_quest(pDBData, pDataSave->n_quest_done_num);

	// 初始化物品冷却时间
	InitItemCDTime(pDBData, pDataSave->n_item_cd_time_num_);

	// 初始化好友度
	InitFriendValue(pDBData, pDataSave->n_friendship_num_);

	// 初始化黑名单
	InitBlackList(pDBData, pDataSave->n_black_num_);

	// 初始化角色名贴
	m_VCard.Init(pDBData, this);

	// 初始化氏族数据
	//m_ClanData.Init(pDBData, this);

	// 初始化宠物带
	//m_pPetPocket->Init(pDBData, pDataSave->n_pets_num_, this, pDataSave->n16_pet_packet_size_);

	// 初始化地图限制
	InitMapLimit(pDBData, pDataSave->n_map_limit_num_);
	
	// 初始化完成成就
	//m_achievementMgr.InitComplate(pDBData, pDataSave->n_achievement_num_);

	// 初始化条件
	//m_achievementMgr.InitCriteria(pDBData, pDataSave->n_achievement_criteria_num_);

	// 初始化副本进度
	InitInstProcess(pDBData, pDataSave->n_inst_process_num);

	// 初始化签到
	init_sign_data(pDBData);
	
	// 魂精数据
	//init_HuenJing_data(pDBData);
	
	init_reward_data(pDBData);

	init_mounts_data(pDBData);


	// 初始化称号选项
	m_pTitleMgr->InitOpts(this, pDataSave->u16_active_title_id[0], pDataSave->u16_active_title_id[1], pDataSave->u16_active_title_id[2], pDataSave->s_remote_open_set_.bTitle);

	// 激活坐骑属性
	GetRaidMgr().AcitveAtt(TRUE);

	// 所有内容已经加载完毕，重新计算人物初始当前属性
	CalInitAtt();

	//RecalAtt(TRUE, FALSE);

	//SetRating();

	//InitHuenJingAtt();

	// 计算所有状态
	CalInitState();

	//队伍
	m_dwTeamID = pDataSave->dw_team_id_;
	m_bLeader = FALSE;
	if(VALID_VALUE(m_dwTeamID))
	{
		BOOL bHave = FALSE;
		Team* pTeam = const_cast<Team*>(g_groupMgr.GetTeamPtr(m_dwTeamID));
		if(VALID_POINT(pTeam))
		{
			INT iNum = pTeam->get_member_number();
			INT iIndex = 0;
			for(INT i = 0; i < iNum; i++)
			{
				DWORD dw_role_id = pTeam->get_member_id(i);
				if(m_dwID == dw_role_id)
				{
					bHave = TRUE;
					iIndex = i;
					break;
				}
			}
			if(bHave)
			{
				pTeam->member_online(iIndex, this);
			}
			else
			{
				m_dwTeamID = INVALID_VALUE;
			}
			m_bLeader = pTeam->is_leader( this->GetID() );
		}
		else
		{
			m_dwTeamID = INVALID_VALUE;
		}
	}

	g_VipNetBarMgr.PlayerNotify(GetSession()->GetSessionID());

	// 初始化角色开宝箱相关状态
	InitChestState();

	tagDWORDTime dwLoginTime = pDataSave->login_time_;
	if(VALID_POINT(dwLoginTime) && 
		CalcTimeDiff(g_world.GetWorldTime(), dwLoginTime) > ROLE_COMEBACK_HEARSAY_TIME_SEC){
		HearSayHelper::SendMessage(EHST_ROLEBACK30DAYS, this->GetID( ), dwLoginTime);
	}

	//设置摊位等级
	if( VALID_POINT(m_pStall) ) m_pStall->check_vip( );
	
	//判断下修炼成就
	//tagEquip* pWeapon = GetWeapon();
	//if (VALID_POINT(pWeapon))
	//{
	//	m_achievementMgr.UpdateAchievementCriteria(ete_xiulian, pWeapon->equipSpec.nLevel);
	//}
}

//----------------------------------------------------------------------------------------------
// 初始化数据库中中的人物属性
//----------------------------------------------------------------------------------------------
VOID Role::InitAtt(const s_role_data_save* pDataSave)
{
	// 各个一级属性的投点
	get_fast_code()->memory_copy(m_nAttPointAdd, pDataSave->n_att_point_add_, sizeof(m_nAttPointAdd));

	// 所有属性附上默认值
	for(INT n = 0; n < ERA_End; n++)
	{
		m_nBaseAtt[n] = AttRes::GetInstance()->GetAttDefRole(n);
	}

	// 设置一些保存在数据库中的属性（一些不随基础值改变的属性，还缺少一个士气）
	m_nAtt[ERA_HP]			=	pDataSave->n_hp_;
	m_nAtt[ERA_MP]			=	pDataSave->n_mp_;
	m_nAtt[ERA_Love]		=	pDataSave->n_rage_;
	m_nAtt[ERA_Brotherhood]	=	pDataSave->n_brotherhood_;
	m_nAtt[ERA_Wuhuen]		=	pDataSave->n_endurance_;
	//m_nAtt[ERA_Endurance]	=	pDataSave->nEndurance;
	m_nAtt[ERA_Knowledge]	=	pDataSave->n_knowledge_;
	m_nAtt[ERA_Injury]		=	pDataSave->n_injury_;
	m_nAtt[ERA_Morale]		=	pDataSave->n_morale_;
	m_nAtt[ERA_Morality]	=	pDataSave->n_morality_;
	//m_nAtt[ERA_Luck]		=	pDataSave->n_luck_;
	m_nAtt[ERA_AttPoint]	=	pDataSave->n_att_point_;
	m_nAtt[ERA_TalentPoint]	=	pDataSave->n_talent_point_;
	m_nAtt[ERA_Fortune]		=   pDataSave->n_vigour_;

	m_nLevel		= pDataSave->n_level_;	// 等级
	m_nCurLevelExp	= pDataSave->n_cur_exp_;	// 当前升级经验

	m_eClass	= pDataSave->e_class_;		// 职业
	m_eClassEx	= pDataSave->e_class_ex_;		// 职业扩展
	m_nCredit	= pDataSave->n_credit_;		// 信用度
	m_nIdentity = pDataSave->n_identity_;		// 身份
	m_nVIPPoint = pDataSave->n_vip_point_;		// 会员积分
	m_nTreasureSum = pDataSave->n_treasure_sum_;	//开启宝箱计数
	
	m_nSignLevel	= pDataSave->n_god_level_;	// 神级
	SetSpouseID(pDataSave->dwSpouseID);//gx add 2013.7.3
	//vip相关
	SetVIPLevel(pDataSave->dwVIPLevel);//gx add 2013.8.14
	m_VIP_DeadLine = pDataSave->dwVIP_Deadline;//gx add
	//设置基础属性
	SetBaseAttByLevel(m_eClass, pDataSave->n_level_);
}

//---------------------------------------------------------------------------------
// 初始化技能
//---------------------------------------------------------------------------------
VOID Role::InitSkill(const BYTE* &pData, INT32 n_num)
{
	//// 首先加载普通攻击技能
	//for(INT n = 0; n < X_NORMAL_ATTACK_SKILL_NUM; n++)
	//{
	//	DWORD dwSkillID = NORMAL_ATTACK_SKILL_ID[n];
	//	Skill* pSkill = new Skill(dwSkillID, 1, 0, 0, 0, 0);
	//	AddSkill(pSkill, FALSE);
	//}

	// 加载保存的技能
	if( n_num <= 0 ) return;
	
	// 离线时间
	DWORD dwLeaveTime = CalcTimeDiff(m_LoginTime, m_LogoutTime) * 1000;

	const s_skill_save* pSkillSave = (const s_skill_save*)pData;

	for(INT n = 0; n < n_num; n++)
	{
		//Skill* pSkill = new Skill(pSkillSave->dw_id, pSkillSave->n_self_level_, pSkillSave->n_learn_level_, pSkillSave->n_cool_down_, pSkillSave->nProficiency);
		Skill* pSkill = new Skill;
		
		INT nCoolDown = 0;
		if ((DWORD)pSkillSave->n_cool_down_ > dwLeaveTime)
		{
			nCoolDown = pSkillSave->n_cool_down_ - dwLeaveTime;
		}
		if ( FALSE == pSkill->Init(pSkillSave->dw_id, pSkillSave->n_self_level_, pSkillSave->n_learn_level_, nCoolDown, pSkillSave->n_proficiency_, FALSE) )
		{
			SAFE_DELETE(pSkill);
			continue;
		}
		// 将技能加入到列表中
		AddSkill(pSkill, FALSE);

		pSkillSave++;
	}

	// 重设指针
	pData = (const BYTE*)((s_skill_save*)pData + n_num);
	//pData = (const BYTE*)pSkillSave;
}

//---------------------------------------------------------------------------------
// 初始化地图限制
//---------------------------------------------------------------------------------
VOID Role::InitMapLimit(const BYTE* &pData, INT32 n_num)
{
	if(n_num <= 0) return;

	const s_enter_map_limit* pEnterMapLimit = (const s_enter_map_limit*)pData;

	for(INT i = 0; i < n_num; i++)
	{
		s_enter_map_limit* pTemp = new s_enter_map_limit;
		memcpy(pTemp, pEnterMapLimit, sizeof(s_enter_map_limit));
		m_mapMapLimit.add(pTemp->dw_map_id_, pTemp);

		pEnterMapLimit++;
	}
	pData = (const BYTE*)((s_enter_map_limit*)pData + n_num);
}

//---------------------------------------------------------------------------------
// 初始化副本进度
//---------------------------------------------------------------------------------
VOID Role::InitInstProcess(const BYTE* &pData, INT32 n_num)
{
	if(n_num <= 0) return;

	const s_inst_process* p_inst = (const s_inst_process*)pData;

	for(INT i = 0; i < n_num; i++)
	{
		s_inst_process* pTemp = new s_inst_process;
		memcpy(pTemp, p_inst, sizeof(s_inst_process));
		list_inst_process.push_back(pTemp);

		p_inst++;
	}
	pData = (const BYTE*)((s_inst_process*)pData + n_num);
}

//---------------------------------------------------------------------------------
// 初始化签到数据
//---------------------------------------------------------------------------------
VOID Role::init_sign_data(const BYTE* &pData)
{
	const tagRoleSignData* p_data = (const tagRoleSignData*)pData;

	memcpy(&st_role_sign_data, p_data, sizeof(tagRoleSignData));

	pData = pData + sizeof(tagRoleSignData);
}

VOID Role::init_HuenJing_data(const BYTE* &pData)
{
	const tagRoleHuenJingData* p_data = (const tagRoleHuenJingData*)pData;

	memcpy(&st_role_huenjing_data, p_data, sizeof(tagRoleHuenJingData));

	pData = pData + sizeof(tagRoleHuenJingData);

}

VOID Role::init_reward_data( const BYTE* &pData )
{
	const tagRewardData* p_data = (const tagRewardData*)pData;

	memcpy(&st_role_reward_data, pData, sizeof(st_role_reward_data));

	pData = pData + sizeof(st_role_reward_data);
}

VOID Role::init_mounts_data(const BYTE* &pData)
{
	const s_mounts_save* p_data = (const s_mounts_save*)pData;

	m_RaidMgr.init(this, p_data->nStep, p_data->nGrade, p_data->nExp);

	pData = pData + sizeof(s_mounts_save);
}

//-------------------------------------------------------------------------------
// 初始化状态
//-------------------------------------------------------------------------------
VOID Role::InitBuff(const BYTE* &pData, INT32 n_num)
{
	if( n_num <= 0 ) return;

	INT nBeBuffIndex = 0;
	INT nDeBuffIndex = MAX_BENEFIT_BUFF_NUM;

/*	const tagBuffSave* pEnd = (tagBuffSave*)pData + n_num;*/

	for(INT n = 0; n < n_num; n++)
	{
		s_buff_save* pBuffSave = (s_buff_save*)pData;

		const tagBuffProto* pProto = AttRes::GetInstance()->GetBuffProto(Buff::GetTypeIDFromIDAndLevel(pBuffSave->dw_buff_id_, pBuffSave->n_level_));
		if( !VALID_POINT(pProto) ) goto next_buff;

		// 计算离线计时Buff
		if (pProto->bOfflineConsume)
		{
			// 获取当前时间和该玩家上次下线的时间差(单位：秒)
			DWORD dwOfflineTick = CalcTimeDiff(g_world.GetWorldTime(), m_LogoutTime) * TICK_PER_SECOND;
			
			// 计算Buff剩余持续时间(即使DWORD=>INT32也没问题)
			if (dwOfflineTick > MAX_BUFF_PERSIST_TICK)
			{
				goto next_buff;
			}
			else if ( pProto->nPersistTick > (INT32)dwOfflineTick + pBuffSave->n_persist_tick_ )
			{
				pBuffSave->n_persist_tick_ += (INT32)dwOfflineTick;
			}
			else
			{
				goto next_buff;
			}
		}

		// 如果是有益Buff
		INT nIndex = INVALID_VALUE;
		if( pProto->bBenifit )
		{
			nIndex = nBeBuffIndex;
			// 有益Buff的数量已满
			if( nIndex >= MAX_BENEFIT_BUFF_NUM ) continue;
			++nBeBuffIndex;
		}
		// 否则是有害Buff
		else
		{
			nIndex = nDeBuffIndex;
			// 有害Buff的数量已满
			if( nIndex >= MAX_BUFF_NUM ) continue;
			++nDeBuffIndex;
		}

		m_Buff[nIndex].Init(this, pBuffSave, nIndex);
		m_mapBuff.add(m_Buff[nIndex].GetID(), &m_Buff[nIndex]);

next_buff:
		// 累加指针
		pData += sizeof(s_buff_save) - 1 + pBuffSave->n_modifier_num_ * sizeof(DWORD);
	}

/*	pData = (const BYTE*)pEnd;*/
}

//-------------------------------------------------------------------------------
// 初始化玩家物品(装备)
//-------------------------------------------------------------------------------
VOID Role::InitItem(const BYTE* &pData, INT32 n_num)
{
	INT32 nItemSize		= sizeof(tagItem);
	INT32 nEquipSize	= sizeof(tagEquip);
	
	// 清除外观信息
	ZeroMemory(&m_AvatarEquipEquip, SIZE_AVATAR_EQUIP);
	ZeroMemory(&m_AvatarEquipFashion, SIZE_AVATAR_EQUIP);

	for (int i = EAE_Upper; i <= EAE_RWeapon; i++)
	{
		m_AvatarEquipEquip.AvatarEquip[i].byDisplayPos = i;
		m_AvatarEquipFashion.AvatarEquip[i].byDisplayPos = i;
	}
	
	DWORD dw_error_code;
	package_list<tagItem *> listItem;
	const tagItem	*pTmpItem	= NULL;
	tagItem			*pNewItem	= NULL;

	pTmpItem = (const tagItem *)pData;
	for(INT32 i=0; i<n_num; ++i)
	{
		if(!MIsEquipment(pTmpItem->dw_data_id))
		{
			pNewItem = new tagItem;
			get_fast_code()->memory_copy(pNewItem, pTmpItem, nItemSize);
			pNewItem->pProtoType = AttRes::GetInstance()->GetItemProto(pTmpItem->dw_data_id);

			pTmpItem = (const tagItem*)((BYTE*)pTmpItem + nItemSize);
		}
		else
		{
			pNewItem = new tagEquip;
			get_fast_code()->memory_copy(pNewItem, pTmpItem, nEquipSize);
			pNewItem->pProtoType = AttRes::GetInstance()->GetEquipProto(pTmpItem->dw_data_id);

			pTmpItem = (const tagItem*)((BYTE*)pTmpItem + nEquipSize);
			
		}

		if(!VALID_POINT(pNewItem->pProtoType))
		{
			//ASSERT(VALID_POINT(pNewItem->pProtoType));
			m_att_res_caution(_T("item/equip"), _T("typeid"), pNewItem->dw_data_id);
			print_message(_T("The item(SerialNum: %lld) hasn't found proto type!\n"), pNewItem->n64_serial);
			Destroy(pNewItem);
			continue;
		}

		if(MEquipIsRide(pNewItem->pProtoType))
			cal_ride_speed( (tagEquip*)pNewItem );

		pNewItem->eStatus = EUDBS_Null;
		pNewItem->pScript = g_ScriptMgr.GetItemScript( pNewItem->dw_data_id);
		
		dw_error_code = Put2Container(pNewItem);
		if(dw_error_code != E_Success)
		{
			if(dw_error_code != E_Item_Place_NotFree)
			{
				//ASSERT(0);
				Destroy(pNewItem);
				continue;
			}

			listItem.push_back(pNewItem);
		}
	}

	// 检查是否有因位置重复导致的不能添加到容器中
	while(listItem.size() != 0)
	{
		pNewItem = listItem.pop_front();

		// 如果是背包或仓库中的物品，放入空位
		switch(pNewItem->eConType)
		{
		case EICT_Bag:
		case EICT_Equip:
			dw_error_code = GetItemMgr().Add2Bag(pNewItem, (DWORD)elcid_bag_pos_overlap, FALSE, FALSE);
			if(E_Con_NotEnoughSpace == dw_error_code)
			{
				print_message(_T("Because container<eType: %d> is full, item<serial: %lld\n> load failed!\n"), 
					EICT_Bag, pNewItem->n64_serial);
				print_message(_T("Make some places and login again can solve this problem!\n"));
			}
			break;
		case EICT_RoleWare:
			dw_error_code = GetItemMgr().Add2RoleWare(pNewItem, (DWORD)elcid_ware_pos_overlap, FALSE, FALSE);
			if(E_Con_NotEnoughSpace == dw_error_code)
			{
				print_message(_T("Because container<eType: %d> is full, item<serial: %lld> load failed!\n"), 
					EICT_RoleWare, pNewItem->n64_serial);
				print_message(_T("Make some places and login again can solve this problem!\n"));
			}
			break;
		default:
			ASSERT(0);
			Destroy(pNewItem);
			continue;
		}
		
		ASSERT(E_Success == dw_error_code);
	}

	// 重设指针
	pData = (const BYTE*)pTmpItem;
}

//-------------------------------------------------------------------------------
// 将从数据库中读入的物品(装备),放入到相应的容器中
//-------------------------------------------------------------------------------
DWORD Role::Put2Container(tagItem *pItem)
{
	DWORD dw_error_code = GetItemMgr().Put2Container(pItem);

	if(EICT_Equip == pItem->eConType && E_Success == dw_error_code)
	{
		M_trans_pointer(p, pItem, tagEquip);
		if (p->GetEquipNewness() > 0)
		{
			ProcEquipEffectAtt(p, TRUE, p->n16Index);
		}
		ResetOneEquipDisplay(p, p->n16Index);
		m_Suit.Add(p, p->n16Index, FALSE);
	}

	return dw_error_code;
}

//-------------------------------------------------------------------------------
// 将从数据库中读入玩家物品冷却时间
//-------------------------------------------------------------------------------
VOID Role::InitItemCDTime(const BYTE* &pData, INT32 n_num)
{
	if(n_num <= 0)
	{
		return;
	}

	// 获取当前时间和该玩家上次下线的时间差(单位：秒)
	DWORD dwInterval = CalcTimeDiff(g_world.GetWorldTime(), m_LogoutTime);
	if(dwInterval > (DWORD)MAX_ITEM_DCTIME)
	{
		// 重设指针
		pData = pData + n_num * sizeof(tagCDTime);
		return;
	}

	// 转换成毫秒
	dwInterval *= 1000;
	
	ItemMgr &itemMgr = GetItemMgr();
	const tagCDTime *p = (const tagCDTime*)pData;

	for(INT32 i=0; i<n_num; ++i)
	{
		if(p->dw_time > dwInterval)
		{
			itemMgr.Add2CDTimeMap(p->dw_data_id, p->dw_time - dwInterval);
		}

		++p;
	}

	// 重设指针
	pData = (const BYTE*)((tagCDTime *)pData + n_num);
	//pData = (const BYTE*)p;
}

//-------------------------------------------------------------------------------
// 初始化好友列表
//-------------------------------------------------------------------------------
VOID Role::InitFriend(const BYTE* &pData, INT32 n_num)
{
	for(INT i = 0; i < MAX_FRIENDNUM; ++i)
	{
		m_Friend[i].dwFriendID = INVALID_VALUE;
		m_Friend[i].dwFriVal = 0;
		m_Friend[i].byGroup = 1;
	}

	// 黑名单初时化
	memset(m_dwBlackList, 0XFF, sizeof(m_dwBlackList));

	// 仇人
	memset(m_dwEnemyList, 0XFF, sizeof(m_dwEnemyList));
	if(n_num <= 0)
		return;

	const s_friend_save	*pFriend = NULL;
	DWORD dwSizeFriend = sizeof(s_friend_save);
	pFriend = (const s_friend_save*)pData;

	for(INT m = 0; m < n_num; ++m)
	{
		SetFriend(m, pFriend->dw_friend_id_, 0, pFriend->n_group_id_);
		pFriend = (const s_friend_save*)((BYTE*)pFriend + dwSizeFriend);
	}

	// 重设指针
	pData = (const BYTE*)((s_friend_save*)pData + n_num);
	//pData = (const BYTE*)pFriend;
}

//-------------------------------------------------------------------------------
// 初始化好友度
//-------------------------------------------------------------------------------
VOID Role::InitFriendValue(const BYTE* &pData, INT32 n_num)
{
	const s_friendship_save *pFriendSave = NULL;
	tagFriend *pFriend = NULL;
	DWORD dwSizeFriend = sizeof(s_friendship_save);
	pFriendSave = (const s_friendship_save*)pData;

	for(INT i = 0; i < n_num; ++i)
	{
		pFriend = m_mapFriend.find(pFriendSave->dw_friend_id_);

		if(VALID_POINT(pFriend))
			pFriend->dwFriVal = pFriendSave->n_frival_;

		pFriendSave = (const s_friendship_save*)((BYTE*)pFriendSave + dwSizeFriend);
	}

	// 重设指针
	pData = (const BYTE*)((s_friendship_save*)pData + n_num);
	//pData = (const BYTE*)pFriendSave;
}

//-------------------------------------------------------------------------------
// 初始化黑名单
//-------------------------------------------------------------------------------
VOID Role::InitBlackList(const BYTE* &pData, INT32 n_num)
{
	const DWORD *pBlackList = NULL;
	DWORD dw_size = sizeof(DWORD);
	pBlackList = (const DWORD*)pData;

	for(INT i = 0; i < n_num; ++i)
	{
		SetBlackList(i, *pBlackList);
		pBlackList = (const DWORD*)((BYTE*)pBlackList + dw_size);
	}

	// 重设指针
	pData = (const BYTE*)((DWORD*)pData + n_num);
	//pData = (const BYTE*)pBlackList;
}

//-------------------------------------------------------------------------------
// 初始化仇人
//-------------------------------------------------------------------------------
VOID Role::InitEnemyList(const BYTE* &pData, INT32 n_num)
{
	const DWORD *pEnemyList = NULL;
	DWORD dw_size = sizeof(DWORD);
	pEnemyList = (const DWORD*)pData;

	for(INT i = 0; i < n_num; i++)
	{
		SetEnemyList(i, *pEnemyList);
		pEnemyList = (const DWORD*)((BYTE*)pEnemyList + dw_size);
	}

	pData = (const BYTE*)((DWORD*)pData + n_num);
}

// 初始化当前任务
VOID Role::init_current_quest(const BYTE* &p_data, INT32 n_num)
{
	const tagQuestSave* p_save = (const tagQuestSave*)p_data;
	const tagQuestSave* p_end = p_save + n_num;
	const tagQuestProto* p_proto = NULL;
	
	INT n_index = 0;
	for ( ; p_save < p_end; ++p_save)
	{
		p_proto = g_questMgr.get_protocol(QuestIDHelper::RestoreID(p_save->u16QuestID));
		if( !VALID_POINT(p_proto) ) 
		{
			m_att_res_caution(_T("QuestFile"), _T("questID"), p_save->u16QuestID);
			continue;
		}
		quests_[n_index].init(p_save, this, n_index);
		current_quest_.add(p_save->u16QuestID, &quests_[n_index]);
		if(quests_[n_index].get_quest_flag().dwQuestTrack) set_track_number_mod(1);
		++n_index;
	}
	p_data = (const BYTE*)((tagQuestSave*)p_data + n_num);
}

// 初始化所有已完成任务
VOID Role::init_complete_quest(const BYTE* &p_data, INT32 n_num)
{
	const tagQuestDoneSave* p_save = (const tagQuestDoneSave*)p_data;

	for(INT32 i = 0; i < n_num; ++i)
	{
		tagQuestDoneSave* p_done = new tagQuestDoneSave;
		get_fast_code()->memory_copy(p_done, p_save, sizeof(tagQuestDoneSave));
		done_quest_.add(p_done->u16QuestID, p_done);
		
		++p_save;
	}
	p_data = (const BYTE*)((tagQuestDoneSave*)p_data + n_num);
}
//-------------------------------------------------------------------------------
// 计算人物初始当前属性
//-------------------------------------------------------------------------------
VOID Role::CalInitAtt()
{
	// 先保存一些不随基础值改变的属性
	INT nHP				=	m_nAtt[ERA_HP];
	INT nMP				=	m_nAtt[ERA_MP];
	INT nVitality		=	m_nAtt[ERA_Brotherhood];
	INT nWuhuen			=	m_nAtt[ERA_Wuhuen];
	INT nLove			=	m_nAtt[ERA_Love];
	INT nKnowledge		=	m_nAtt[ERA_Knowledge];
	INT nInjury			=	m_nAtt[ERA_Injury];
	INT nMorale			=	m_nAtt[ERA_Morale];
	INT nMorality		=	m_nAtt[ERA_Morality];
	//INT nCulture		=	m_nAtt[ERA_Luck];
	INT nAttPoint		=	m_nAtt[ERA_AttPoint];
	INT nTalentPoint	=	m_nAtt[ERA_TalentPoint];
	INT nVigour			=   m_nAtt[ERA_Fortune];

	for(INT n = 0; n < ERA_End; n++)
	{
		m_nAtt[n] = CalAttMod(m_nBaseAtt[n], n);
	}
	
	// 通过一级属性的当前值计算二级属性
	//for(INT n = 0; n < earc_num; ++n)
	//{
	//	AccountAtt(ERAC2ERA(e_role_att_to_change(n)));
	//}

	// 计算二级属性加成
	//for(INT n = ERA_AttB_Start; n < ERA_AttB_End; n++)
	//{
	//	m_nAtt[n] = CalAttMod(m_nAtt[n], n);
	//}

	// 计算某些“当前属性”与最大值之间的最小值
	if( nHP < 0 )
	{
		m_nAtt[ERA_HP] = m_nAtt[ERA_MaxHP];
	}
	else
	{
		m_nAtt[ERA_HP] = min(nHP, m_nAtt[ERA_MaxHP]);
	}

	if( nMP < 0 )
	{
		m_nAtt[ERA_MP] = m_nAtt[ERA_MaxMP];
	}
	else
	{
		m_nAtt[ERA_MP] = min(nMP, m_nAtt[ERA_MaxMP]);
	}

	if( nVitality < 0 )
	{
		m_nAtt[ERA_Brotherhood] = 0;
	}
	else
	{
		m_nAtt[ERA_Brotherhood] = min(nVitality, m_nAtt[ERA_MaxBrotherhood]);
	}

	if( nWuhuen < 0 )
	{
		m_nAtt[ERA_Wuhuen] = 0;
	}
	else
	{
		m_nAtt[ERA_Wuhuen] = min(nWuhuen, m_nAtt[ERA_MaxBrotherhood]);
	}

	//if( nEndurance < 0 )
	//{
	//	m_nAtt[ERA_Endurance] = m_nAtt[ERA_MaxEndurance];
	//}
	//else
	//{
	//	m_nAtt[ERA_Endurance] = min(nEndurance, m_nAtt[ERA_MaxEndurance]);
	//}

	// 设置上不随基础属性改变的当前属性
	m_nAtt[ERA_Knowledge]	=	nKnowledge;
	m_nAtt[ERA_Injury]		=	nInjury;
	m_nAtt[ERA_Morale]		=	nMorale;
	m_nAtt[ERA_Morality]	=	nMorality;
	//m_nAtt[ERA_Luck]		=	nCulture;
	m_nAtt[ERA_AttPoint]	=	nAttPoint;
	m_nAtt[ERA_TalentPoint]	=	nTalentPoint;
	m_nAtt[ERA_Fortune]		=	nVigour;
	m_nAtt[ERA_Love]		=	nLove;

	// 设置某些随基本属性变化的属性
	m_fMountXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_Mount]) / 10000.0f);
	//m_fMountXZSpeed *= FLOAT(m_nAtt[ERA_Speed_Mount]) / 10000.0f;
	m_fXZSpeed *= FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f;
	m_fYSpeed *= FLOAT(m_nAtt[ERA_Speed_Y]) / 10000.0f;
	m_fSwimXZSpeed *= FLOAT(m_nAtt[ERA_Speed_Swim]) / 10000.0f;
	m_Size *= FLOAT(m_nAtt[ERA_Shape]) / 10000.0f;
	// todo: pk状态

	// 清空重算字段
	ClearAttRecalFlag();
}

//-----------------------------------------------------------------------
// 计算初始状态
//-----------------------------------------------------------------------
VOID Role::CalInitState()
{
	// 计算PK模式，红名，则设为全体模式
	//if(GetPKValue() > 0)
	//{
	//	m_ePKState = ERolePK_All;
	//}

	// 绝对安全区，强制和平模式，
	if(IsInRoleState(ERS_RealSafeArea))
	{
		m_ePKState = ERolePK_Peace;
	}

	// 帮会战区，强制帮会模式
	if(IsInRoleState(ERS_GuildBattle))
	{
		m_ePKState = ERolePK_Guild;
	}

	// PVP区，强制全体模式
	if(IsInRoleState(ERS_PVPArea))
	{
		m_ePKState = ERolePK_All;
	}

	// 计算死亡状态
	if( GetAttValue(ERA_HP) <= 0 )
	{
		SetState(ES_Dead, FALSE);
	}
}
//------------------------------------------------------------------------
// 上线
//------------------------------------------------------------------------
VOID Role::Online(BOOL bFirst/* =FALSE */)
{
	if( bFirst )
	{
		VirginOnline();
		CreateNewRoleGift();
	}
	
	//DWORD dwPos = g_RankMgr.GetRankPos(ERT_LEVELRANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 0, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_KILLRANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 1, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_JUSTICERANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 2, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_1V1RANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 3, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_SHIHUNRANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 4, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_ACHPOINTRANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 5, dwPos);

	//dwPos = g_RankMgr.GetRankPos(ERT_ACHNUMBRRANK, GetID());
	//GetAchievementMgr().UpdateAchievementCriteria(ete_rank_pos, 6, dwPos);

	// 调用上线的脚本函数
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnRoleOnline(this);
	}
}

//------------------------------------------------------------------------
// 第一次上线
//------------------------------------------------------------------------
VOID Role::VirginOnline()
{
	// 设置在线时间
	m_nCurOnlineTime = 0;
	m_nOnlineTime = 0;

	// 设置出生地图和复活地图
	m_dwRebornMapID = m_dwMapID = g_mapCreator.get_born_map_id();

	// 得到一个随机的出生点
	INT nIndex = 0;
	Vector3 vBornPos = g_mapCreator.rand_get_one_born_position(nIndex);

	//获得出生点朝向
	FLOAT fYaw = g_mapCreator.get_born_yaw(nIndex);

	// 出生点
	m_MoveData.Reset(vBornPos.x, vBornPos.y, vBornPos.z, cosf((270-fYaw) * 3.1415927f / 180.0f), 0.0f, sinf((270-fYaw) * 3.1415927f / 180.0f));

	// 调用初次上限的脚本函数
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnRoleFirstOnline(this);
	}
}

//------------------------------------------------------------------------
// 初始化宝箱相关数据
//------------------------------------------------------------------------
VOID Role::InitChestState()
{
	m_TreasureState.nChestSerial = 0;
	m_TreasureState.nKeySerial = 0;
	m_TreasureState.dwChestTypeID = 0;
	m_TreasureState.dwKeyTypeID = 0;
	m_TreasureState.ChestItem.dw_data_id = 0;
	m_TreasureState.ChestItem.n_num = 0;
	m_TreasureState.ChestItem.nTableID = 0;
	m_TreasureState.ChestItem.fBeginRate = 0.0;
	m_TreasureState.ChestItem.fNormalRate = 0.0;
	m_TreasureState.ChestItem.fRoleRate = 0.0;
	m_TreasureState.ChestItem.fServerRate = 0.0;
	
}
