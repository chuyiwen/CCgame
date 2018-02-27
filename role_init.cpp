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
*	@brief		��ʼ���������ݽṹ
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
// ��ʼ�����
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

	// ��ʼ����ҽű�
	m_pScript = g_ScriptMgr.GetRoleScript();

	// ��ҽű�����
	get_fast_code()->memory_copy(m_ScriptData.dwData, pDataSave->data_.dwData, sizeof(DWORD)*ESD_Role);

	// �ƺŹ�����
	m_pTitleMgr		= new TitleMgr;

	// �½������
	m_pPetPocket	= new PetPocket;
	
	// ��ʼ����������
	m_eClass = pDataSave->e_class_;
	get_fast_code()->memory_copy(&m_Avatar, &pDataConst->avatar, sizeof(m_Avatar));
	m_Avatar.byClass = (BYTE)pDataSave->e_class_;
	m_DisplaySet = pDataSave->display_set_;
	m_dwRebornMapID = pDataSave->dw_reborn_map_id_;
	m_CreatTime = pDataConst->create_time_;
	m_LoginTempTime = pDataSave->login_time_;
	m_LoginTime = g_world.GetWorldTime();		// ��������ʱ��
	m_LogoutTime = pDataSave->logout_time_;
	m_nOnlineTime = pDataSave->n_online_time_;
	m_nCurOnlineTime = pDataSave->n_cur_online_time_;
	m_eClassEx = pDataSave->e_class_ex_;
	m_bHasLeftMsg = pDataSave->h_has_left_msg_;
	m_LeaveGuildTime = pDataSave->leave_guild_time_;
	m_nSendMailNum = pDataSave->n_send_mail_num_;
	m_dwMasterID = pDataSave->dw_master_id_;
	//m_dwMasterPrenticeForbidTime = 0;
	m_dwMasterPrenticeForbidTime = pDataSave->dw_master_prentice_forbid_time_;//gx add 2013.12.10 ʦͽ����Ҫ����ͷ�
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
	// ��ʼ���ϴλ�ȡ�̳������Ʒʱ��
	SetLastGetMallFreeTime(pDataSave->dw_time_get_mall_free_);

	// ���������Ƿ����������������
	//m_RoleStateEx.SetState(ERSE_BagPsdExist);
	/*if(GetSession() && !GetSession()->IsHaveBagPsd())
	{
		m_RoleStateEx.UnsetState(ERSE_BagPsdExist);
		m_RoleStateEx.SetState(ERSE_BagPsdPass);
	}*/

	
	// ��ʼ���ɾ�
	m_achievementMgr.InitOpts(this);
	// PK���
	m_iPKValue = pDataSave->n_pk_value_;
	if(m_iPKValue) m_dwPKValueDecTime = g_world.GetWorldTime();
	get_fast_code()->memory_copy(m_Talent, pDataSave->talent_, sizeof(m_Talent));
	
	m_dwDestoryEquipCount = pDataSave->dw_destory_equip_count;

	// ��ʼ������Ʒ
	initRoleGift(pDataSave->n_gift_step_, pDataSave->dw_gift_leave_time_);
	//m_stNewRoleGift.init(pDataSave->n_gift_id_, pDataSave->n_gift_step_, pDataSave->dw_gift_id_, pDataSave->dw_gift_leave_time_, pDataSave->b_gift_get_);
	

// 	if(pDataSave->bSafeGuard)
// 	{
// 		SetRoleState(ERS_Safeguard, FALSE);
// 	}

	// ����1v1��������
	if(st_1v1_scroe.n_cur_score > st_1v1_scroe.n_day_max_score)
	{
		st_1v1_scroe.n_cur_score = st_1v1_scroe.n_day_max_score;
	}

	if(st_1v1_scroe.n_cur_score < 0)
	{
		st_1v1_scroe.n_cur_score = 0;
	}
	// ����
	m_dwGuildID = pDataSave->dw_guild_id;
	if (VALID_VALUE(m_dwGuildID))
	{
		// �������Ƿ��ڰ�����
		guild* pGuild	= g_guild_manager.get_guild(m_dwGuildID);
		if (!VALID_POINT(pGuild) || !VALID_POINT(pGuild->get_member(GetID())))
		{
			SetGuildID(INVALID_VALUE);
			SetLeaveGuildTime();
		}
		else
		{
			// ����״̬
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
	//�����ж�
	Map* pInstance = g_mapCreator.get_map(m_dwOwnInstanceMapID, m_dwOwnInstanceID);
	if(!VALID_POINT(pInstance))
	{
		// ������������ڣ���ձ��
		m_dwOwnInstanceID = INVALID_VALUE;
		m_dwOwnInstanceMapID = INVALID_VALUE;
		m_dwInstanceCreateTime = 0;
	}
	else
	{
		if(pInstance->get_map_type() == EMT_Instance)
		{
			// ����������ڣ����Ǵ���ʱ�䲻�ԣ���ձ��
			if(((map_instance_normal*)pInstance)->get_create_time() != m_dwInstanceCreateTime)
			{
				m_dwOwnInstanceID = INVALID_VALUE;
				m_dwOwnInstanceMapID = INVALID_VALUE;
				m_dwInstanceCreateTime = 0;
			}
		}
	}

	// �����������
	ZeroMemory(m_byRoleHelp, sizeof(m_byRoleHelp));
	get_fast_code()->memory_copy(m_byRoleHelp, pDataSave->by_role_help_, sizeof(m_byRoleHelp));

	// ����Ի�����
	ZeroMemory(m_byTalkData, sizeof(m_byTalkData));
	get_fast_code()->memory_copy(m_byTalkData, pDataSave->by_talk_data_, sizeof(m_byTalkData));

	// ��ݼ�����
	ZeroMemory(&m_stKeyInfo, sizeof(m_stKeyInfo));
	get_fast_code()->memory_copy(&m_stKeyInfo, &pDataSave->st_key_info_, sizeof(m_stKeyInfo));
	
	// ��������
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


	// ��ȡ�������������ݿ�����
	InitAtt(pDataSave);

	// ����ÿ�����
	AccountSignLevel();


	// ��ʼ�����������
	circle_quest_man_.Init(&pDataSave->circle_quest_man_);

	// todo����ȡ�����б�װ���б�buff�б���������Զ��������Ե�Ӱ��
	/*********************************************************************************************************
	*�����б�״̬�б��ƺ��б��ƺ������б���Ʒ�б�װ���б������б�����б������б�����������б���ɫ����
	*���밴˳���ȡ(��˳���tagRoleDataSave�ж�Ӧ)
	*********************************************************************************************************/

	const BYTE *pDBData = &pDataSave->by_data[0];	// �б�����ָ��,��ָ�����ڶ�ȡ�������ƶ�,��pDBData��ֵ�Ǳ仯��
	
	// ��ʼ�������б�
	InitSkill(pDBData, pDataSave->n_skill_num_);

	// ��ʼ��״̬�б�
	InitBuff(pDBData, pDataSave->n_buff_num_);

	// ��ʼ���ƺ��б�
	m_pTitleMgr->InitTitles(pDBData, pDataSave->n_title_num_);

	// ��ʼ����Ʒװ���б�
	InitItem(pDBData, pDataSave->n_item_num_);

	// ��ʼ�����ѳ���б�
	InitFriend(pDBData, pDataSave->n_friend_num_);

	// ��ʼ������
	InitEnemyList(pDBData, pDataSave->n_enemy_num_);
	
	// ��ʼ����ҵ�ǰ�����б�
	init_current_quest(pDBData, pDataSave->n_quest_num_);

	// ��ʼ�������������б�
	init_complete_quest(pDBData, pDataSave->n_quest_done_num);

	// ��ʼ����Ʒ��ȴʱ��
	InitItemCDTime(pDBData, pDataSave->n_item_cd_time_num_);

	// ��ʼ�����Ѷ�
	InitFriendValue(pDBData, pDataSave->n_friendship_num_);

	// ��ʼ��������
	InitBlackList(pDBData, pDataSave->n_black_num_);

	// ��ʼ����ɫ����
	m_VCard.Init(pDBData, this);

	// ��ʼ����������
	//m_ClanData.Init(pDBData, this);

	// ��ʼ�������
	//m_pPetPocket->Init(pDBData, pDataSave->n_pets_num_, this, pDataSave->n16_pet_packet_size_);

	// ��ʼ����ͼ����
	InitMapLimit(pDBData, pDataSave->n_map_limit_num_);
	
	// ��ʼ����ɳɾ�
	//m_achievementMgr.InitComplate(pDBData, pDataSave->n_achievement_num_);

	// ��ʼ������
	//m_achievementMgr.InitCriteria(pDBData, pDataSave->n_achievement_criteria_num_);

	// ��ʼ����������
	InitInstProcess(pDBData, pDataSave->n_inst_process_num);

	// ��ʼ��ǩ��
	init_sign_data(pDBData);
	
	// �꾫����
	//init_HuenJing_data(pDBData);
	
	init_reward_data(pDBData);

	init_mounts_data(pDBData);


	// ��ʼ���ƺ�ѡ��
	m_pTitleMgr->InitOpts(this, pDataSave->u16_active_title_id[0], pDataSave->u16_active_title_id[1], pDataSave->u16_active_title_id[2], pDataSave->s_remote_open_set_.bTitle);

	// ������������
	GetRaidMgr().AcitveAtt(TRUE);

	// ���������Ѿ�������ϣ����¼��������ʼ��ǰ����
	CalInitAtt();

	//RecalAtt(TRUE, FALSE);

	//SetRating();

	//InitHuenJingAtt();

	// ��������״̬
	CalInitState();

	//����
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

	// ��ʼ����ɫ���������״̬
	InitChestState();

	tagDWORDTime dwLoginTime = pDataSave->login_time_;
	if(VALID_POINT(dwLoginTime) && 
		CalcTimeDiff(g_world.GetWorldTime(), dwLoginTime) > ROLE_COMEBACK_HEARSAY_TIME_SEC){
		HearSayHelper::SendMessage(EHST_ROLEBACK30DAYS, this->GetID( ), dwLoginTime);
	}

	//����̯λ�ȼ�
	if( VALID_POINT(m_pStall) ) m_pStall->check_vip( );
	
	//�ж��������ɾ�
	//tagEquip* pWeapon = GetWeapon();
	//if (VALID_POINT(pWeapon))
	//{
	//	m_achievementMgr.UpdateAchievementCriteria(ete_xiulian, pWeapon->equipSpec.nLevel);
	//}
}

//----------------------------------------------------------------------------------------------
// ��ʼ�����ݿ����е���������
//----------------------------------------------------------------------------------------------
VOID Role::InitAtt(const s_role_data_save* pDataSave)
{
	// ����һ�����Ե�Ͷ��
	get_fast_code()->memory_copy(m_nAttPointAdd, pDataSave->n_att_point_add_, sizeof(m_nAttPointAdd));

	// �������Ը���Ĭ��ֵ
	for(INT n = 0; n < ERA_End; n++)
	{
		m_nBaseAtt[n] = AttRes::GetInstance()->GetAttDefRole(n);
	}

	// ����һЩ���������ݿ��е����ԣ�һЩ�������ֵ�ı�����ԣ���ȱ��һ��ʿ����
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

	m_nLevel		= pDataSave->n_level_;	// �ȼ�
	m_nCurLevelExp	= pDataSave->n_cur_exp_;	// ��ǰ��������

	m_eClass	= pDataSave->e_class_;		// ְҵ
	m_eClassEx	= pDataSave->e_class_ex_;		// ְҵ��չ
	m_nCredit	= pDataSave->n_credit_;		// ���ö�
	m_nIdentity = pDataSave->n_identity_;		// ���
	m_nVIPPoint = pDataSave->n_vip_point_;		// ��Ա����
	m_nTreasureSum = pDataSave->n_treasure_sum_;	//�����������
	
	m_nSignLevel	= pDataSave->n_god_level_;	// ��
	SetSpouseID(pDataSave->dwSpouseID);//gx add 2013.7.3
	//vip���
	SetVIPLevel(pDataSave->dwVIPLevel);//gx add 2013.8.14
	m_VIP_DeadLine = pDataSave->dwVIP_Deadline;//gx add
	//���û�������
	SetBaseAttByLevel(m_eClass, pDataSave->n_level_);
}

//---------------------------------------------------------------------------------
// ��ʼ������
//---------------------------------------------------------------------------------
VOID Role::InitSkill(const BYTE* &pData, INT32 n_num)
{
	//// ���ȼ�����ͨ��������
	//for(INT n = 0; n < X_NORMAL_ATTACK_SKILL_NUM; n++)
	//{
	//	DWORD dwSkillID = NORMAL_ATTACK_SKILL_ID[n];
	//	Skill* pSkill = new Skill(dwSkillID, 1, 0, 0, 0, 0);
	//	AddSkill(pSkill, FALSE);
	//}

	// ���ر���ļ���
	if( n_num <= 0 ) return;
	
	// ����ʱ��
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
		// �����ܼ��뵽�б���
		AddSkill(pSkill, FALSE);

		pSkillSave++;
	}

	// ����ָ��
	pData = (const BYTE*)((s_skill_save*)pData + n_num);
	//pData = (const BYTE*)pSkillSave;
}

//---------------------------------------------------------------------------------
// ��ʼ����ͼ����
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
// ��ʼ����������
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
// ��ʼ��ǩ������
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
// ��ʼ��״̬
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

		// �������߼�ʱBuff
		if (pProto->bOfflineConsume)
		{
			// ��ȡ��ǰʱ��͸�����ϴ����ߵ�ʱ���(��λ����)
			DWORD dwOfflineTick = CalcTimeDiff(g_world.GetWorldTime(), m_LogoutTime) * TICK_PER_SECOND;
			
			// ����Buffʣ�����ʱ��(��ʹDWORD=>INT32Ҳû����)
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

		// ���������Buff
		INT nIndex = INVALID_VALUE;
		if( pProto->bBenifit )
		{
			nIndex = nBeBuffIndex;
			// ����Buff����������
			if( nIndex >= MAX_BENEFIT_BUFF_NUM ) continue;
			++nBeBuffIndex;
		}
		// �������к�Buff
		else
		{
			nIndex = nDeBuffIndex;
			// �к�Buff����������
			if( nIndex >= MAX_BUFF_NUM ) continue;
			++nDeBuffIndex;
		}

		m_Buff[nIndex].Init(this, pBuffSave, nIndex);
		m_mapBuff.add(m_Buff[nIndex].GetID(), &m_Buff[nIndex]);

next_buff:
		// �ۼ�ָ��
		pData += sizeof(s_buff_save) - 1 + pBuffSave->n_modifier_num_ * sizeof(DWORD);
	}

/*	pData = (const BYTE*)pEnd;*/
}

//-------------------------------------------------------------------------------
// ��ʼ�������Ʒ(װ��)
//-------------------------------------------------------------------------------
VOID Role::InitItem(const BYTE* &pData, INT32 n_num)
{
	INT32 nItemSize		= sizeof(tagItem);
	INT32 nEquipSize	= sizeof(tagEquip);
	
	// ��������Ϣ
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

	// ����Ƿ�����λ���ظ����µĲ�����ӵ�������
	while(listItem.size() != 0)
	{
		pNewItem = listItem.pop_front();

		// ����Ǳ�����ֿ��е���Ʒ�������λ
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

	// ����ָ��
	pData = (const BYTE*)pTmpItem;
}

//-------------------------------------------------------------------------------
// �������ݿ��ж������Ʒ(װ��),���뵽��Ӧ��������
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
// �������ݿ��ж��������Ʒ��ȴʱ��
//-------------------------------------------------------------------------------
VOID Role::InitItemCDTime(const BYTE* &pData, INT32 n_num)
{
	if(n_num <= 0)
	{
		return;
	}

	// ��ȡ��ǰʱ��͸�����ϴ����ߵ�ʱ���(��λ����)
	DWORD dwInterval = CalcTimeDiff(g_world.GetWorldTime(), m_LogoutTime);
	if(dwInterval > (DWORD)MAX_ITEM_DCTIME)
	{
		// ����ָ��
		pData = pData + n_num * sizeof(tagCDTime);
		return;
	}

	// ת���ɺ���
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

	// ����ָ��
	pData = (const BYTE*)((tagCDTime *)pData + n_num);
	//pData = (const BYTE*)p;
}

//-------------------------------------------------------------------------------
// ��ʼ�������б�
//-------------------------------------------------------------------------------
VOID Role::InitFriend(const BYTE* &pData, INT32 n_num)
{
	for(INT i = 0; i < MAX_FRIENDNUM; ++i)
	{
		m_Friend[i].dwFriendID = INVALID_VALUE;
		m_Friend[i].dwFriVal = 0;
		m_Friend[i].byGroup = 1;
	}

	// ��������ʱ��
	memset(m_dwBlackList, 0XFF, sizeof(m_dwBlackList));

	// ����
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

	// ����ָ��
	pData = (const BYTE*)((s_friend_save*)pData + n_num);
	//pData = (const BYTE*)pFriend;
}

//-------------------------------------------------------------------------------
// ��ʼ�����Ѷ�
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

	// ����ָ��
	pData = (const BYTE*)((s_friendship_save*)pData + n_num);
	//pData = (const BYTE*)pFriendSave;
}

//-------------------------------------------------------------------------------
// ��ʼ��������
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

	// ����ָ��
	pData = (const BYTE*)((DWORD*)pData + n_num);
	//pData = (const BYTE*)pBlackList;
}

//-------------------------------------------------------------------------------
// ��ʼ������
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

// ��ʼ����ǰ����
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

// ��ʼ���������������
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
// ���������ʼ��ǰ����
//-------------------------------------------------------------------------------
VOID Role::CalInitAtt()
{
	// �ȱ���һЩ�������ֵ�ı������
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
	
	// ͨ��һ�����Եĵ�ǰֵ�����������
	//for(INT n = 0; n < earc_num; ++n)
	//{
	//	AccountAtt(ERAC2ERA(e_role_att_to_change(n)));
	//}

	// ����������Լӳ�
	//for(INT n = ERA_AttB_Start; n < ERA_AttB_End; n++)
	//{
	//	m_nAtt[n] = CalAttMod(m_nAtt[n], n);
	//}

	// ����ĳЩ����ǰ���ԡ������ֵ֮�����Сֵ
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

	// �����ϲ���������Ըı�ĵ�ǰ����
	m_nAtt[ERA_Knowledge]	=	nKnowledge;
	m_nAtt[ERA_Injury]		=	nInjury;
	m_nAtt[ERA_Morale]		=	nMorale;
	m_nAtt[ERA_Morality]	=	nMorality;
	//m_nAtt[ERA_Luck]		=	nCulture;
	m_nAtt[ERA_AttPoint]	=	nAttPoint;
	m_nAtt[ERA_TalentPoint]	=	nTalentPoint;
	m_nAtt[ERA_Fortune]		=	nVigour;
	m_nAtt[ERA_Love]		=	nLove;

	// ����ĳЩ��������Ա仯������
	m_fMountXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_Mount]) / 10000.0f);
	//m_fMountXZSpeed *= FLOAT(m_nAtt[ERA_Speed_Mount]) / 10000.0f;
	m_fXZSpeed *= FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f;
	m_fYSpeed *= FLOAT(m_nAtt[ERA_Speed_Y]) / 10000.0f;
	m_fSwimXZSpeed *= FLOAT(m_nAtt[ERA_Speed_Swim]) / 10000.0f;
	m_Size *= FLOAT(m_nAtt[ERA_Shape]) / 10000.0f;
	// todo: pk״̬

	// ��������ֶ�
	ClearAttRecalFlag();
}

//-----------------------------------------------------------------------
// �����ʼ״̬
//-----------------------------------------------------------------------
VOID Role::CalInitState()
{
	// ����PKģʽ������������Ϊȫ��ģʽ
	//if(GetPKValue() > 0)
	//{
	//	m_ePKState = ERolePK_All;
	//}

	// ���԰�ȫ����ǿ�ƺ�ƽģʽ��
	if(IsInRoleState(ERS_RealSafeArea))
	{
		m_ePKState = ERolePK_Peace;
	}

	// ���ս����ǿ�ư��ģʽ
	if(IsInRoleState(ERS_GuildBattle))
	{
		m_ePKState = ERolePK_Guild;
	}

	// PVP����ǿ��ȫ��ģʽ
	if(IsInRoleState(ERS_PVPArea))
	{
		m_ePKState = ERolePK_All;
	}

	// ��������״̬
	if( GetAttValue(ERA_HP) <= 0 )
	{
		SetState(ES_Dead, FALSE);
	}
}
//------------------------------------------------------------------------
// ����
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

	// �������ߵĽű�����
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnRoleOnline(this);
	}
}

//------------------------------------------------------------------------
// ��һ������
//------------------------------------------------------------------------
VOID Role::VirginOnline()
{
	// ��������ʱ��
	m_nCurOnlineTime = 0;
	m_nOnlineTime = 0;

	// ���ó�����ͼ�͸����ͼ
	m_dwRebornMapID = m_dwMapID = g_mapCreator.get_born_map_id();

	// �õ�һ������ĳ�����
	INT nIndex = 0;
	Vector3 vBornPos = g_mapCreator.rand_get_one_born_position(nIndex);

	//��ó����㳯��
	FLOAT fYaw = g_mapCreator.get_born_yaw(nIndex);

	// ������
	m_MoveData.Reset(vBornPos.x, vBornPos.y, vBornPos.z, cosf((270-fYaw) * 3.1415927f / 180.0f), 0.0f, sinf((270-fYaw) * 3.1415927f / 180.0f));

	// ���ó������޵Ľű�����
	if( VALID_POINT(m_pScript) )
	{
		m_pScript->OnRoleFirstOnline(this);
	}
}

//------------------------------------------------------------------------
// ��ʼ�������������
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
