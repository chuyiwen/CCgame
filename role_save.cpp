/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//保存人物数据

#include "StdAfx.h"

#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"

#include "../../common/WorldDefine/currency_define.h"

#include "role.h"
#include "db_session.h"
#include "buff.h"
#include "title_mgr.h"
#include "pet_pocket.h"

//-------------------------------------------------------------------------------
// 保存玩家到DB
//-------------------------------------------------------------------------------
VOID Role::SaveToDB()
{
	if( _isnan(m_MoveData.m_vPos.x) || _isnan(m_MoveData.m_vPos.y) || _isnan(m_MoveData.m_vPos.z) )
	{
		SI_LOG->write_log(_T("Error1!!!! Role's position Invalid, Role ID is %u, movestate is %d, nextstate is %d, startpos is <%f, %f, %f> endpos is <%f, %f, %f>\r\n\r\n"), 
			m_dwID, m_MoveData.m_eCurMove, m_MoveData.m_eNextPreMove, m_MoveData.m_vPosStart.x, m_MoveData.m_vPosStart.y, m_MoveData.m_vPosStart.z, 
			m_MoveData.m_vDest.x, m_MoveData.m_vDest.y, m_MoveData.m_vDest.z);
	}

	if(m_LogoutTime < m_LoginTime)
	{
		m_nOnlineTime += CalcTimeDiff(g_world.GetWorldTime(), m_LoginTime);
	}
	else
	{
		m_nOnlineTime += CalcTimeDiff(g_world.GetWorldTime(), m_LogoutTime);
	}

	m_LogoutTime = g_world.GetWorldTime();
	m_nCurOnlineTime = CalcTimeDiff(m_LogoutTime, m_LoginTime);
	//m_nOnlineTime += m_nCurOnlineTime;

	m_SaveRole.Init();
	NET_DB2C_save_role *pSend = m_SaveRole;

	pSend->dw_role_id = m_dwID;

	pSend->RoleData.login_time_ = m_LoginTime;
	pSend->RoleData.logout_time_ = m_LogoutTime;
	pSend->RoleData.n_online_time_ = m_nOnlineTime;
	pSend->RoleData.n_cur_online_time_ = m_nCurOnlineTime;
	pSend->RoleData.dw_time_get_mall_free_ = m_LastGetMallFreeTime;
	pSend->RoleData.leave_guild_time_ = m_LeaveGuildTime;

	pSend->RoleData.display_set_ = m_DisplaySet;
	pSend->RoleData.avatar_equip_ = GetAvatarEquip();

	pSend->RoleData.dw_map_id_ = m_dwMapID;
	pSend->RoleData.dw_reborn_map_id_ = m_dwRebornMapID;
	pSend->RoleData.f_coordinate_[0] = m_MoveData.m_vPos.x;
	pSend->RoleData.f_coordinate_[1] = m_MoveData.m_vPos.y;
	pSend->RoleData.f_coordinate_[2] = m_MoveData.m_vPos.z;
	pSend->RoleData.f_face_[0] = m_MoveData.m_vFace.x;
	pSend->RoleData.f_face_[1] = m_MoveData.m_vFace.y;
	pSend->RoleData.f_face_[2] = m_MoveData.m_vFace.z;

	pSend->RoleData.e_class_ = m_eClass;
	pSend->RoleData.e_class_ex_ = m_eClassEx;
	pSend->RoleData.n_level_ = m_nLevel;
	pSend->RoleData.n_cur_exp_ = m_nCurLevelExp;
	pSend->RoleData.n_hp_ = GetAttValue(ERA_HP);
	pSend->RoleData.n_mp_ = GetAttValue(ERA_MP);
	pSend->RoleData.n_att_point_ = GetAttValue(ERA_AttPoint);
	pSend->RoleData.n_talent_point_ = GetAttValue(ERA_TalentPoint);
	pSend->RoleData.n_rage_ = GetAttValue(ERA_Love);
	//pSend->RoleData.nEndurance = GetAttValue(ERA_Endurance);
	pSend->RoleData.n_god_level_ = m_nSignLevel;

	get_fast_code()->memory_copy(pSend->RoleData.n_att_point_add_, m_nAttPointAdd, sizeof(m_nAttPointAdd));

	pSend->RoleData.n_brotherhood_ = GetAttValue(ERA_Brotherhood);
	pSend->RoleData.n_endurance_ = GetAttValue(ERA_Wuhuen);
	pSend->RoleData.n_injury_ = GetAttValue(ERA_Injury);
	pSend->RoleData.n_knowledge_ = GetAttValue(ERA_Knowledge);
	pSend->RoleData.n_morale_ = GetAttValue(ERA_Morale);
	pSend->RoleData.n_morality_ = GetAttValue(ERA_Morality);
	//pSend->RoleData.n_luck_ = GetAttValue(ERA_Luck);
	pSend->RoleData.n_credit_ = m_nCredit;
	pSend->RoleData.n_identity_ = m_nIdentity;
	pSend->RoleData.n_vip_point_ = m_nVIPPoint;
	pSend->RoleData.n_vigour_ = GetAttValue(ERA_Fortune);
	pSend->RoleData.dw_today_online_tick_ = dw_today_online_tick_;
	pSend->RoleData.dw_history_vigour_cost_ = dw_history_vigour_cost_;
	pSend->RoleData.dw_perday_get_vigour_total = m_PerdayGetVigourTotal;

	pSend->RoleData.n_send_mail_num_ = m_nSendMailNum;

	pSend->RoleData.n16_bag_size_ = GetItemMgr().GetBagCurSize();
	pSend->RoleData.n_bag_gold_ = MSilver2DBGold(GetCurMgr().GetBagSilver());
	pSend->RoleData.n_bag_silver_ = MSilver2DBSilver(GetCurMgr().GetBagSilver());
	pSend->RoleData.n_bag_copper_ = MSilver2DBCopper(GetCurMgr().GetBagSilver());
	pSend->RoleData.n_bag_bind_gold_ = MSilver2DBGold(GetCurMgr().GetBagBindSilver());
	pSend->RoleData.n_bag_bind_silver_ = MSilver2DBSilver(GetCurMgr().GetBagBindSilver());
	pSend->RoleData.n_bag_bind_copper_ = MSilver2DBCopper(GetCurMgr().GetBagBindSilver());
	pSend->RoleData.n_bag_tuanbao_ = GetCurMgr().GetBagYuanBao();
	pSend->RoleData.n_ex_volume_ = GetCurMgr().GetExchangeVolume();

	pSend->RoleData.n16WareSize = GetItemMgr().GetWareCurSize();
	pSend->RoleData.nWareGold = MSilver2DBGold(GetCurMgr().GetWareSilver());
	pSend->RoleData.nWareSilver = MSilver2DBSilver(GetCurMgr().GetWareSilver());
	pSend->RoleData.nWareCopper = MSilver2DBCopper(GetCurMgr().GetWareSilver());

	pSend->RoleData.n32_exploits = GetCurMgr().GetExploits();
	
	pSend->RoleData.n16_pet_packet_size_ = 0;//m_pPetPocket->GetSize();
	pSend->RoleData.dw_guild_id = m_dwGuildID;
	pSend->RoleData.dw_team_id_ = m_dwTeamID;	
	pSend->RoleData.b_safe_guard_ = (GetRoleState() & ERS_Safeguard);
	pSend->RoleData.n_pk_value_ = GetPKValue();
	pSend->RoleData.n_kill_num_ = m_nKillNum;
	get_fast_code()->memory_copy(pSend->RoleData.talent_, m_Talent, sizeof(pSend->RoleData.talent_));

	pSend->RoleData.dw_shihun = dw_shihun;
	pSend->RoleData.n16_pet_xiulian_size = n16_pet_xiulian_size;

	// 对远端玩家公开选项设置
	pSend->RoleData.s_remote_open_set_.bTitle	= GetTitleMgr()->Visibility();

	// 称号
	pSend->RoleData.u16_active_title_id[0]		= GetTitleMgr()->GetActiviteTitle(0);
	pSend->RoleData.u16_active_title_id[1]		= GetTitleMgr()->GetActiviteTitle(1);
	pSend->RoleData.u16_active_title_id[2]		= GetTitleMgr()->GetActiviteTitle(2);

	//pSend->RoleData.dw_master_prentice_forbid_time_ = 0;
	pSend->RoleData.dw_master_prentice_forbid_time_ = get_master_prentice_forbid_time();//gx add 2013.12.10
	pSend->RoleData.dw_master_id_ = m_dwMasterID;

	pSend->RoleData.dw_own_instance_id_ = m_dwOwnInstanceID;
	pSend->RoleData.dw_own_instance_map_id_ = m_dwOwnInstanceMapID;
	pSend->RoleData.dw_instance_create_time_ = m_dwInstanceCreateTime;

	pSend->RoleData.n_gift_step_ = m_stNewRoleGift.n_step_;
	pSend->RoleData.dw_gift_id_ = m_stNewRoleGift.dw_gift_id_;
	pSend->RoleData.dw_gift_leave_time_ = m_stNewRoleGift.dw_leave_time_;
	pSend->RoleData.b_gift_get_ = m_stNewRoleGift.b_get_;
	pSend->RoleData.n_gift_id_ = m_stNewRoleGift.n_id_;

	pSend->RoleData.n16_exbag_step = n16_exbag_step;
	pSend->RoleData.n16_exware_step = n16_exware_step;

	pSend->RoleData.n_yuanbao_exchange_num = m_nYuanbaoExchangeNum;
	pSend->RoleData.n_achievement_point = m_nAchievementPoint;
	pSend->RoleData.n_achievement_num = GetAchievementMgr().GetComplateNumber();
	pSend->RoleData.n32_active_num = m_n32_active_num;
	pSend->RoleData.n32_rating = st_EquipTeamInfo.n32_Rating;
	//pSend->RoleData.b_PurpureDec = mIsPurpureDec;
	pSend->RoleData.n_CoolDownReviveCD = mCoolDownReviveCD;
	pSend->RoleData.n_PerDayHangGetExpTimeMS = mPerDayHangGetExpTimeMS;
	pSend->RoleData.n32_guild_active_num = m_n32_guild_active_num;

	pSend->RoleData.nInstanceData = m_nInstanceData;
	pSend->RoleData.nInstanceShaodang = m_nInstanceShaodangTime;
	pSend->RoleData.nSaodangIndex = m_nShaodangIndex;

	
	pSend->RoleData.dwSpouseID = GetSpouseID();
	// 玩家脚本数据
	get_fast_code()->memory_copy(pSend->RoleData.data_.dwData, m_ScriptData.dwData, sizeof(DWORD)*ESD_Role);

	// 玩家摊位信息
	if (VALID_POINT(m_pStall))
	{
		m_pStall->save_to_db(&(pSend->RoleData));
	}
	
	// 玩家禁言状态
	pSend->RoleData.dw_forbid_talk_start_ = m_dwForbidTalkStart;
	pSend->RoleData.dw_forbid_talk_end_ = m_dwForbidTalkEnd;

	// 挂机
	pSend->RoleData.n16_hang_num_ = m_n16HangNum;
	pSend->RoleData.b_exp_ = m_bExp;
	pSend->RoleData.b_brotherhood_ = m_bBrotherhood;
	pSend->RoleData.n_leave_exp_ = m_nLeaveExp;
	pSend->RoleData.n_leave_brotherhood_ = m_nLeaveBrotherHood;
	pSend->RoleData.n_total_mater_moral_ = m_nTotalMasterMoral;

	pSend->RoleData.e_role_camp = e_role_camp;
	pSend->RoleData.n_paimai_limit = n_paimai_limit;
	pSend->RoleData.n_bank_limit = n_bank_limit;
	pSend->RoleData.n_shop_exploits_limit = n_shop_exploits_limit;
	pSend->RoleData.n_graduates_num = m_nGraduates;
	pSend->RoleData.n_circle_quest_fresh = mnCirleRefresh;
	pSend->RoleData.n_circle_quest_freshMax = mnCirleRefreshMax ;
	pSend->RoleData.n_circle_quest_perdaynum = m_CirclePerDayNum;

	get_fast_code()->memory_copy(&pSend->RoleData.st_1v1_score, &st_1v1_scroe, sizeof(st_1v1_scroe));

	// 玩家帮助数据
	get_fast_code()->memory_copy(pSend->RoleData.by_role_help_, m_byRoleHelp, sizeof(m_byRoleHelp));

	// 玩家对话数据
	get_fast_code()->memory_copy(pSend->RoleData.by_talk_data_, m_byTalkData, sizeof(m_byTalkData));

	// 快捷键数据
	get_fast_code()->memory_copy(&pSend->RoleData.st_key_info_, &m_stKeyInfo, sizeof(m_stKeyInfo));
	
	// 每天清零数据
	get_fast_code()->memory_copy(pSend->RoleData.by_role_day_clear, m_byDayClear, sizeof(m_byDayClear));

	// 玩家签名
	get_fast_code()->memory_copy(pSend->RoleData.sz_signature_name, m_szSignature_name, sizeof(m_szSignature_name));

	get_fast_code()->memory_copy(pSend->RoleData.n32_active_data, m_n32_active_data, sizeof(m_n32_active_data));
	get_fast_code()->memory_copy(pSend->RoleData.b_active_receive, m_b_active_receive, sizeof(m_b_active_receive));
	
	get_fast_code()->memory_copy(pSend->RoleData.n32_guild_active_data, m_n32_guild_active_data, sizeof(m_n32_guild_active_data));
	get_fast_code()->memory_copy(pSend->RoleData.b_guild_active_receive, m_b_guild_active_receive, sizeof(m_b_guild_active_receive));

	// 换随机任务
	circle_quest_man_.Fill(&pSend->RoleData.circle_quest_man_);

	/*********************************************************************************************************
	*技能列表，状态列表，称号列表，物品列表，装备列表，好友列表，仇敌列表，任务列表，已完成任务列表，
	*物品冷却时间表
	*必须按顺序
	*********************************************************************************************************/
	LPVOID pData = pSend->RoleData.by_data;

	// 1.技能列表
	pSend->RoleData.n_skill_num_		= 0;
	SaveSkill2DB(pData, pData, pSend->RoleData.n_skill_num_);

	// 2.状态列表
	pSend->RoleData.n_buff_num_		= 0;
	SaveBuff2DB(pData, pData, pSend->RoleData.n_buff_num_);

	// 3.称号条件列表
	pSend->RoleData.n_title_num_		= 0;
	GetTitleMgr()->SaveTitlesToDB(pData, pData, pSend->RoleData.n_title_num_);

	// 4.物品列表 5.装备列表
	pSend->RoleData.n_item_num_		= 0;
	GetItemMgr().SaveItem2DB(pData, pData, pSend->RoleData.n_item_num_);

	// 6.好友列表 -- 存: 实时,即单独发消息
	pSend->RoleData.n_friend_num_		= 0;

	// 7.仇敌列表 -- 存: 实时,即单独发消息
	pSend->RoleData.n_enemy_num_		= 0;

	// 8.任务列表
	pSend->RoleData.n_quest_num_		= 0;
	save_quest_to_db(pData, pData, pSend->RoleData.n_quest_num_);

	// 9.已完成任务列表 -- 存: 实时,即单独发消息
	pSend->RoleData.n_quest_done_num	= 0;

	// 10.物品冷却时间
	pSend->RoleData.n_item_cd_time_num_	= 0;
	GetItemMgr().SaveCDTime2DB(pData, pData, pSend->RoleData.n_item_cd_time_num_);

	// 11.氏族数据
	//m_ClanData.Save2DB(pData, pData, pSend->RoleData.b_clan_data_chg_);

	// 12.宠物
	//GetPetPocket()->SaveToDB(pData, pData, pSend->RoleData.n_pets_num_);

	// 13.进入地图限制
	pSend->RoleData.n_map_limit_num_ = 0;
	SaveMapEnterLimit(pData, pData, pSend->RoleData.n_map_limit_num_);
	
	// 14.完成成就
	//pSend->RoleData.n_achievement_num_ = 0;
	//GetAchievementMgr().SaveComplateToDB(pData, pData, pSend->RoleData.n_achievement_num_);

	// 15.成就条件
	//pSend->RoleData.n_achievement_criteria_num_ = 0;
	//GetAchievementMgr().SaveCriteriaToDB(pData, pData, pSend->RoleData.n_achievement_criteria_num_);

	// 16.副本进度
	pSend->RoleData.n_inst_process_num = 0;
	SaveInstProcess(pData, pData, pSend->RoleData.n_inst_process_num);

	//17.签到数据
	SaveSignData(pData, pData);

	//18.魂晶数据
	//SaveHuenjingData(pData, pData);
	
	//19.奖励数据
	SaveRewardData(pData, pData);

	// 重新计算消息大小
	pSend->dw_size = (DWORD)((BYTE*)pData - (BYTE*)pSend);

	g_dbSession.Send(pSend, pSend->dw_size);

	// 重置保存倒计时
	ResetNeedSave2DBCD();

	if( _isnan(m_MoveData.m_vPos.x) || _isnan(m_MoveData.m_vPos.y) || _isnan(m_MoveData.m_vPos.z) )
	{
		SI_LOG->write_log(_T("Error2!!!! Role's position Invalid, Role ID is %u, movestate is %d, nextstate is %d, startpos is <%f, %f, %f> endpos is <%f, %f, %f>\r\n\r\n"), 
			m_dwID, m_MoveData.m_eCurMove, m_MoveData.m_eNextPreMove, m_MoveData.m_vPosStart.x, m_MoveData.m_vPosStart.y, m_MoveData.m_vPosStart.z, 
			m_MoveData.m_vDest.x, m_MoveData.m_vDest.y, m_MoveData.m_vDest.z);
	}
}

//-------------------------------------------------------------------------------
// 保存技能
//-------------------------------------------------------------------------------
VOID Role::SaveSkill2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT &n_num)
{
	INT i	= 0;
	Skill* pSkill = NULL;
	s_skill_save *pSkillSave = (s_skill_save*)pData;

	package_map<DWORD, Skill*>::map_iter iter = m_mapSkill.begin();
	while(m_mapSkill.find_next(iter, pSkill))
	{
		// 如果是普通攻击技能或是从装备上获得，则不保存
		if( ESSTE_Default == pSkill->GetTypeEx() || pSkill->IsFromeEquip()) continue;
		
		pSkill->InitSkillSave(&pSkillSave[i++]);
	}

	n_num = i;
	pOutPointer = &pSkillSave[n_num];
}

//-------------------------------------------------------------------------------
// 保存进入地图限制
//-------------------------------------------------------------------------------
VOID Role::SaveMapEnterLimit(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT &n_num)
{
	INT i = 0;
	s_enter_map_limit* pEnterMapLimit = (s_enter_map_limit*)pData;

	MapLimitMap::map_iter iter = m_mapMapLimit.begin();
	s_enter_map_limit* pTemp = NULL;
	while(m_mapMapLimit.find_next(iter, pTemp))
	{
		if(VALID_POINT(pTemp))
		{
			memcpy(&pEnterMapLimit[i++], pTemp, sizeof(s_enter_map_limit));
		}
	}
	n_num = i;
	pOutPointer = &pEnterMapLimit[n_num];
}

//-------------------------------------------------------------------------------
// 保存副本进度
//-------------------------------------------------------------------------------
VOID Role::SaveInstProcess(OUT LPVOID pData, OUT LPVOID &p_out, OUT INT &n_num)
{
	INT i = 0;
	s_inst_process* pInst = (s_inst_process*)pData;

	LIST_INST_PRO::list_iter iter = list_inst_process.begin();
	s_inst_process* pTemp = NULL;
	while(list_inst_process.find_next(iter, pTemp))
	{
		if(VALID_POINT(pTemp))
		{
			memcpy(&pInst[i++], pTemp, sizeof(s_inst_process));
		}
	}
	n_num = i;
	p_out = &pInst[n_num];
}

//-------------------------------------------------------------------------------
// 保存签到数据
//-------------------------------------------------------------------------------
VOID Role::SaveSignData(OUT LPVOID pData, OUT LPVOID &p_out)
{
	M_trans_pointer(pRepData, pData, tagRoleSignData);

	memcpy(pRepData, &st_role_sign_data, sizeof(tagRoleSignData));
	
	p_out = reinterpret_cast<BYTE*>(pData) + sizeof(tagRoleSignData);
}

//-------------------------------------------------------------------------------
// 保存魂晶数据
//-------------------------------------------------------------------------------
VOID Role::SaveHuenjingData(OUT LPVOID pData, OUT LPVOID &p_out)
{
	M_trans_pointer(pRepData, pData, tagRoleHuenJingData);

	memcpy(pRepData, &st_role_huenjing_data, sizeof(tagRoleHuenJingData));

	p_out = reinterpret_cast<BYTE*>(pData) + sizeof(tagRoleHuenJingData);
}


// 保存奖励数据
VOID Role::SaveRewardData( OUT LPVOID pData, OUT LPVOID &p_out )
{
	M_trans_pointer(pRepData, pData, tagRewardData);

	memcpy(pRepData, &st_role_reward_data, sizeof(st_role_reward_data));

	p_out = reinterpret_cast<BYTE*>(pData) + sizeof(st_role_reward_data);
}



//-------------------------------------------------------------------------------
// 保存Buff
//-------------------------------------------------------------------------------
VOID Role::SaveBuff2DB(OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT &n_num)
{
	INT		nSize = 0;
	BYTE		*pCurPointer = (BYTE*)pData;

	INT nBuffNum = 0;
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;

		// 下线消失的buff
		if( m_Buff[n].Interrupt(EBIF_OffLine) ) continue;

		m_Buff[n].InitBuffSave((s_buff_save*)pCurPointer, nSize);
		pCurPointer += nSize;
		++nBuffNum;
	}

	n_num = nBuffNum;

	pOutPointer = pCurPointer;
}

//---------------------------------------------------------------------------------
// 保存任务
//---------------------------------------------------------------------------------
VOID Role::save_quest_to_db(OUT LPVOID p_data, OUT LPVOID &p_out, OUT INT &n_num)
{
	tagQuestSave* p_save = (tagQuestSave*)p_data;

	n_num = 0;

	for(INT n = 0; n < QUEST_MAX_COUNT; ++n)
	{
		if( !quests_[n].valid() ) continue;
		quests_[n].fill_quest_save(&p_save[n_num++]);
	}

	p_out = &p_save[n_num];
}
VOID Role::UpdateRoleRewardData()
{
	INT nMaxSize = sizeof(NET_DB2C_update_role_reward_data)-sizeof(BYTE)+sizeof(st_role_reward_data);
	CREATE_MSG(pSend, nMaxSize,NET_DB2C_update_role_reward_data);
	pSend->dw_role_id = GetID();
	memcpy(pSend->by_data, &st_role_reward_data, sizeof(st_role_reward_data));
	g_dbSession.Send(pSend, pSend->dw_size);
	MDEL_MSG(pSend);
	return;
}
