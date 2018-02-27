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
*	@file		role.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		人物数据结构
*/
#include "StdAfx.h"

#include "player_session.h"
#include "db_session.h"

#include "../../common/WorldDefine/chat_protocol.h"
#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/talent_define.h"
#include "../../common/WorldDefine/pk_define.h"
#include "../../common/WorldDefine/talent_protocol.h"
#include "../../common/WorldDefine/pk_protocol.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "../../common/WorldDefine/stall_protocol.h"
#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/social_protocol.h"
#include "../../common/WorldDefine/prison_protocol.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/exchange_define.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/script_protocol.h"
#include "../../common/WorldDefine/drop_protocol.h"
#include "../../common/WorldDefine/motion_protocol.h"
#include "../../common/WorldDefine/pet_sns_protocol.h"
#include "../../common/WorldDefine/achievement_protocol.h"
#include "../../common/WorldDefine/formula_define.h"
#include "../../common/WorldDefine/role_god_level.h"
#include "../../Common/WorldDefine/battle_ground_protocol.h"

#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/quest_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/login_world.h"
#include "../../common/WorldDefine/motion_define.h"
#include "../../common/WorldDefine/master_prentice_protocol.h"
#include "../../common/WorldDefine/function_npc_protocol.h"
#include "../../common/WorldDefine/auto_kill_protocol.h"
#include "../../common/WorldDefine/select_role_protocol.h"
#include "../../common/WorldDefine/role_info_protocol.h"
#include "../../common/WorldDefine/ride_protocol.h"
#include "login_session.h"
#include "pet_server_define.h"
#include "db_session.h"
#include "map.h"
#include "map_instance.h"
#include "world.h"
#include "map_creator.h"
#include "map_mgr.h"
#include "att_res.h"
#include "skill.h"
#include "buff.h"
#include "quest.h"
#include "quest_mgr.h"
#include "role.h"
#include "creature.h"
#include "creature_ai.h"
#include "item_creator.h"
#include "role_mgr.h"
#include "stall.h"
#include "group_mgr.h"
#include "social_mgr.h"
#include "script_mgr.h"
#include "title_mgr.h"
#include "pet_pocket.h"
#include "guild_manager.h"
#include "pet_soul.h"
#include "guild_commodity.h"
#include "guild.h"
#include "map_instance_guild.h"
//#include "container_restrict.h"
#include "master_prentice_mgr.h"
#include "paimai_manager.h"
#include "paimai.h"
#include "bank_manager.h"
#include "hearSay_helper.h"
#include "pet_sns_mgr.h"
#include "pvp_mgr.h"
#include "att_res.h"
#include "gm_policy.h"
#include "SignManager.h"
#include "loot_mgr.h"
#include "BattleGround.h"

Role* Role::Create( DWORD dw_role_id, const s_role_data_load* pData, PlayerSession* pSession )
{
	return new Role(dw_role_id, pData, pSession);
}

VOID Role::Delete( Role* &pRole )
{
	SAFE_DELETE(pRole);
}

Role::SaveRole::SaveRole()
{
	BYTE *byData = new BYTE[X_ROLE_BUFF_SIZE];
	ZeroMemory(byData, X_ROLE_BUFF_SIZE);
	M_msg_init(byData, NET_DB2C_save_role);
	m_pSaveRole = (NET_DB2C_save_role*)(byData) ;
	m_pSaveRole->dw_size = X_ROLE_BUFF_SIZE;
}

Role::SaveRole::~SaveRole()
{
	delete [](BYTE*)m_pSaveRole;
}

VOID Role::SaveRole::Init()
{
	m_pSaveRole->dw_role_id = INVALID_VALUE;
	ZeroMemory(&m_pSaveRole->RoleData, X_ROLE_BUFF_SIZE - FIELD_OFFSET(NET_DB2C_save_role, RoleData));
}

Role::SaveRole	Role::m_SaveRole;
Mutex			Role::m_SaveRoleLock;

//-------------------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------------------
Role::Role(DWORD dw_role_id, const s_role_data_load* pData, PlayerSession* pSession)
		: Unit(dw_role_id, pData->role_data_save_.dw_map_id_, 
			Vector3(pData->role_data_save_.f_coordinate_[0], pData->role_data_save_.f_coordinate_[1], pData->role_data_save_.f_coordinate_[2]),
			Vector3(pData->role_data_save_.f_face_[0], pData->role_data_save_.f_face_[1], pData->role_data_save_.f_face_[2])),
		  m_ePKState(ERolePK_Peace), m_pStall(new stall(this, &(pData->role_data_save_))),
		  m_ItemMgr(this, pSession->GetSessionID(), dw_role_id, pData->role_data_save_.n16_bag_size_, pData->role_data_save_.n16WareSize),
		  m_CurMgr(this, pData->role_data_save_.n_bag_gold_, pData->role_data_save_.n_bag_silver_, pData->role_data_save_.n_bag_copper_,
		  pData->role_data_save_.n_bag_bind_gold_, pData->role_data_save_.n_bag_bind_silver_, pData->role_data_save_.n_bag_bind_copper_,
		  pData->role_data_save_.n_bag_tuanbao_,pData->role_data_save_.nWareGold, pData->role_data_save_.nWareSilver, 
		  pData->role_data_save_.nWareCopper, pSession->GetBaiBaoYB(), pSession->GetScore(), pData->role_data_save_.n32_exploits),
		  m_pSession(pSession), m_nNeedSave2DBCountDown(MIN_ROLE_SAVE2DB_INTERVAL),
		  m_Suit(this),m_dwTeamID(INVALID_VALUE),m_dwGroupID(INVALID_VALUE),m_dwTeamInvite(INVALID_VALUE), m_dwOwnInstanceID(INVALID_VALUE), m_dwOwnInstanceMapID(INVALID_VALUE),
		  m_dwExportMapID(INVALID_VALUE), m_pScript(NULL), m_pPetPocket(NULL),	m_nMotionCounter(INVALID_VALUE), m_dwPartnerID(INVALID_VALUE),
		  ScriptData<ESD_Role>(),m_nWorldTalkCounter(INVALID_VALUE),map_talk_counter_(INVALID_VALUE),m_bObjectCoolOff(FALSE), m_iPKValue(0),
		  m_dwSignRoleID(INVALID_VALUE),m_nAddFriendValueCounter(ADD_FRIENDVALUE_COUNT),m_PracticeMgr(this),m_n16HangNum(0),m_bExp(FALSE),m_bBrotherhood(FALSE),
		  m_nLeaveExp(0),m_nLeaveBrotherHood(0),m_nTotalMasterMoral(0),m_nKillNum(0),e_temp_role_camp(ECA_Null),dw_today_online_tick_(0),dw_history_vigour_cost_(0),current_track_number_(0),
		  n16_exbag_step(1),n16_exware_step(1), m_pTargetPetMonster(NULL),m_dwTargetPetID(INVALID_VALUE),m_dwCurSkillTarget(INVALID_VALUE),m_nYuanbaoExchangeNum(0),m_nAchievementPoint(0),
		  m_dwHangTime(HANG_TIME), mIsAutoKill(FALSE), mAutoKillTickDecVigour(AUTO_KILL_DEC_VIGOUR_TICK),m_dwForbidTalkStart(0), m_dwForbidTalkEnd(0),b_check_safe_code(FALSE),bReservationPvP(FALSE),
		  dw_reservation_tick(20000),dw_reservation_id(INVALID_VALUE),m_nGraduates(0),m_dwDestoryEquipCount(0),m_dwUseKillbadgeCD(0),b_leave_prictice(FALSE),dw_gm_instance_id(INVALID_VALUE),mnCirleRefresh(0),mnCirleRefreshMax(0),nCurEquip(INVALID_VALUE), mCarryTypeID(0),
		  n_shop_exploits_limit(0),m_n32_active_num(0),b_will_kick(FALSE),n_delay_num(0),b_in_guild_war(false),b_defender(true)/*,mIsPurpureDec(FALSE)*/,m_CirclePerDayNum(0), mCoolDownReviveCD(1000000), mCoolDownReviveTick(0),dw_shihun(0),mPerDayHangGetExpTimeMS(0),mGetExpTickUpdate(GETEXP_TIME_TICK),
		  n_strength_delay_num(0),n_delay_record_num(0),n16_pet_xiulian_size(0),m_n32_guild_active_num(0),m_PerdayGetVigourTotal(0),n_ce_check_num(0),m_CoolDownReviveTick2(0),m_PracticePartner(INVALID_VALUE),m_PracticingLevel(0),m_PracticeTick(0),m_nInstanceData(0),m_nShaodangIndex(INVALID_VALUE),m_VIP_Level(0),m_VIP_RefreshTick(0),
		  m_nBaseRating(0),m_dwShituAskID(INVALID_VALUE)
{
	m_stKeyInfo.hpID = INVALID_VALUE;
	m_stKeyInfo.traID = INVALID_VALUE;
	for (int i = 0; i < 7; i++)
	{
		m_stKeyInfo.open[i] = 1;
	}


	InitDuel(); InitFishing();
	list_query_paimai.clear();
	ZeroMemory(&st_EquipTeamInfo, sizeof(st_EquipTeamInfo));
	ZeroMemory(&EquipAttitional, sizeof(EquipAttitional));
	Init(pData);

	dw_check_delay_time = g_world.GetWorldTime();
	dw_strength_delay_time = g_world.GetWorldTime();

}

//-------------------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------------------
Role::~Role()
{
	// 完成任务销毁
	tagQuestDoneSave* pDoneQuest = NULL;
	QuestDoneMap::map_iter it = done_quest_.begin();
	while( done_quest_.find_next(it, pDoneQuest) )
	{
		SAFE_DELETE(pDoneQuest);
	}
	done_quest_.clear();

	SAFE_DELETE(m_pStall);
	m_pScript = NULL;

	SAFE_DELETE(m_pTitleMgr);

	SAFE_DELETE(m_pPetPocket);

	DealMountSkill(FALSE);

	MapLimitMap::map_iter iter = m_mapMapLimit.begin();
	s_enter_map_limit* p_enter_map_limit = NULL;
	while(m_mapMapLimit.find_next(iter, p_enter_map_limit))
	{
		if(VALID_POINT(p_enter_map_limit))
		{
			SAFE_DELETE(p_enter_map_limit);
		}
	}
	m_mapMapLimit.clear();

	LIST_INST_PRO::list_iter iter_pro = list_inst_process.begin();
	s_inst_process* p_inst_proecss = NULL;
	while(list_inst_process.find_next(iter_pro, p_inst_proecss))
	{
		if(VALID_POINT(p_inst_proecss))
		{
			SAFE_DELETE(p_inst_proecss);
		}
	}
	list_inst_process.clear();

};
EPowers	Role::GetPowerType()
{
	EPowers ep = EPower_Mana;

	switch(m_eClass)
	{
	case EV_Warrior:
		ep = EPower_Rage;
		break;
	//case EV_Blader:
	//	ep = EPower_Point;
	//	break;
	case EV_Taoist:
		ep = Epower_Focus;
		break;
	case EV_Mage:
		ep = EPower_Energy;
		break;
	//case EV_Astrologer:
	//	ep = EPower_Mana;
	//	break;
	default:
		break;
	}
	return ep;
}

BOOL Role::IsInCombat()
{
	return IsInRoleState(ERS_Combat);	
}
//--------------------------------------------------------------------------------
// 玩家的更新函数
//--------------------------------------------------------------------------------
VOID Role::Update()
{
	DWORD dw_time = timeGetTime();
	DWORD dw_new_time = timeGetTime();

	//if(is_leave_pricitice())
	//{
	//	dw_time = timeGetTime();
	//	m_PracticeMgr.Update();
	//	dw_new_time = timeGetTime();
	//	if(dw_new_time - dw_time > 30)
	//	{
	//		g_world.get_log()->write_log(_T("LeavePractice.Update() time %d\r\n"), dw_new_time - dw_time);
	//	}
	//}
	//else
	//{
		updata_delay();
		UpdateState();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateState time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		GetMoveData().Update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("GetMoveData().Update() time %d\r\n"), dw_new_time - dw_time);
		}

        // add by zhaopeng 2012.05.03
        // 检测玩家加速移动累积值 超过某值时踢下线
        //FLOAT fval;
        //GetMoveData().GetCheatAccumulate( fval );
        //get_window()->watch_info(_T("cheat_accumulate_val:"),fval);
   //     if ( GetMoveData().CheckSpeedCheat() )
   //     {
   //         //print_message(_T("Kick cheat player[%u]\r\n"), GetID());
			//logKick(elci_kick_role_move_fast);
			//GetMoveData().ResetSpeedCheat();
			//
			//if (AttRes::GetInstance()->GetVariableLen().n_kick_fast_move && !b_will_kick)
			//{
			//	NET_SIS_will_kick send;
			//	send.dwTime = 10 * 1000;
			//	SendMessage(&send, send.dw_size);

			//	g_mapCreator.role_will_kick(GetID(), 10 * TICK_PER_SECOND);
			//	b_will_kick = TRUE;
			//	/*g_worldSession.Kick(GetSession()->GetInternalIndex());
			//	GetSession()->SetKicked();*/
			//}

   //     }
        // end by zhaopeng

		dw_time = timeGetTime();
		GetCombatHandler().Update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("GetCombatHandler().Update() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdateSkill();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateSkill time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdateBuff();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateBuff time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdatePK();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdatePK time %d\r\n"), dw_new_time - dw_time);
		}

		//UpdateCoolDownRevive( );
		//UpdateHangGetExp( );

		dw_time = timeGetTime();
		UpdatePVP();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdatePVP time %d\r\n"), dw_new_time - dw_time);
		}
		//update_time_issue();
		dw_time = timeGetTime();
		RegenerAll();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("RegenerAll time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		GetItemMgr().Update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("GetItemMgr().Update() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		m_pStall->update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("m_pStall->update() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		m_CurMgr.Update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("m_CurMgr.Update() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		GetPetPocket()->Update();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("GetPetPocket()->Update() time %d\r\n"), dw_new_time - dw_time);
		}

		--m_nNeedSave2DBCountDown;

		dw_time = timeGetTime();
		UpdateTalkToWorld();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateTalkToWorld() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		update_talk_to_map();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("update_talk_to_map() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdateAddFreindValue();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateAddFreindValue() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdateMotionInviteState();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateMotionInviteState() time %d\r\n"), dw_new_time - dw_time);
		}

		update_ride( );
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("update_ride() time %d\r\n"), dw_new_time - dw_time);
		}

		//dw_time = timeGetTime();
		//m_PracticeMgr.Update();
		//dw_new_time = timeGetTime();
		//if(dw_new_time - dw_time > 30)
		//{
		//	g_world.get_log()->write_log(_T("m_PracticeMgr.Update() time %d\r\n"), dw_new_time - dw_time);
		//}

		dw_time = timeGetTime();
		UpdateHang();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateHang() time %d\r\n"), dw_new_time - dw_time);
		}

		dw_time = timeGetTime();
		UpdateNewRoleGift();
		dw_new_time = timeGetTime();
		if(dw_new_time - dw_time > 30)
		{
			g_world.get_log()->write_log(_T("UpdateNewRoleGift() time %d\r\n"), dw_new_time - dw_time);
		}

		//dw_time = timeGetTime();
		//update_vigour();
		//dw_new_time = timeGetTime();
		//if(dw_new_time - dw_time > 30)
		//{
		//	g_world.get_log()->write_log(_T("update_vigour() time %d\r\n\n\n"), dw_new_time - dw_time);
		//}

		//dw_time = timeGetTime();
		//UpdateDuel();
		//dw_new_time = timeGetTime();
		//if(dw_new_time - dw_time > 30)
		//{
		//	g_world.get_log()->write_log(_T("UpdateDuel() time %d\r\n\n\n"), dw_new_time - dw_time);
		//}

		//dw_time = timeGetTime();
		//UpdateFishing( );
		//dw_new_time = timeGetTime();
		//if(dw_new_time - dw_time > 30)
		//{
		//	g_world.get_log()->write_log(_T("UpdateFishing() time %d\r\n\n\n"), dw_new_time - dw_time);
		//}

		// 更新计时类成就
		//dw_time = timeGetTime();
		//GetAchievementMgr().DoFailedTimedAchievementCriterias();
		//dw_new_time = timeGetTime();
		//if(dw_new_time - dw_time > 30)
		//{
		//	g_world.get_log()->write_log(_T("UpdateFishing() time %d\r\n\n\n"), dw_new_time - dw_time);
		//}

		UpdateAutoKill( );

		m_pTitleMgr->Updata();

		update_Reservation();

		UpdateComPracticeExp();//gx add 2013.6.28
		
		UpdateVIPTime();//gx add 2013.8.14
	//}
	
	// 更新验证码系统
	//if (VALID_POINT(m_pSession) && !m_pSession->IsKicked())
	//{
	//	m_pSession->GetVerification().update();
	//}
	
}

VOID Role::RegenerAll()
{
	if( IsDead() ) return;
	
	//回血
	RegenerateHealth();
	//回能量
	Regenerate(EPower_Mana);

	// 回灵气
	if( --m_nInjuryRegainTickCountDown <= 0 )
	{
		m_nInjuryRegainTickCountDown = INJURY_REGAIN_INTER_TICK;
		ChangeLinqi(INJURY_INCREASE_VAL);	
	}

}

VOID Role::Regenerate(EPowers power)
{
	switch(power)
	{
	case EPower_Mana:
		{
			if( --m_nHPMPRegainTickCountDown <= 0 )
			{
				m_nHPMPRegainTickCountDown = HP_MP_REGAIN_INTER_TICK;

				if( GetAttValue(ERA_MPRegainRate) != 0 )
				{
					ChangeMP(GetAttValue(ERA_MPRegainRate));
				}
			}
		}
		break;
	case EPower_Rage:
		{
			if (!IsInCombat())
			{
				if (--m_nRageRegainTickCountDown <= 0 )
				{
					m_nRageRegainTickCountDown = RAGE_GEGAIN_INTER_TICK;
					ChangeMP(AttRes::GetInstance()->GetVariableLen().nRageLoss);
				}
			}
			
		}
		break;
	case EPower_Point:
		{
			if (!IsInCombat()) 
			{
				if (--m_nPointRegainTickCountDown <= 0 )
				{
					m_nPointRegainTickCountDown = POINT_GEGAIN_INTER_TICK;
					ChangeMP(AttRes::GetInstance()->GetVariableLen().nPointLoss);
				}
			}
			
		}
		break;
	case EPower_Energy:
		{
			if (!IsInCombat())
			{
				if (--m_nEnergyRegainTickCountDown <= 0 )
				{
					m_nEnergyRegainTickCountDown = ENERGY_GEGAIN_INTER_TICK;
					ChangeMP(AttRes::GetInstance()->GetVariableLen().nEnergy);
				}
			}
			else
			{
				if (--m_nEnergyRegainTickCountDown <= 0 )
				{
					m_nEnergyRegainTickCountDown = ENERGY_GEGAIN_INTER_TICK;
					ChangeMP(AttRes::GetInstance()->GetVariableLen().nEnergyCom);
				}
			}
			
		}
		break;
	case Epower_Focus:
		if (!IsInCombat())
		{
			if (--m_nFocusRegainTickCountDown <= 0 )
			{
				m_nFocusRegainTickCountDown = FOCUS_GEGAIN_INTER_TICK;
				ChangeMP(AttRes::GetInstance()->GetVariableLen().nFocusLoss);
			}
		}
		break;
	default:
		break;
	}
}

VOID Role::RegenerateHealth()
{
	// 回血
	if( --m_nHPMPRegainTickCountDown <= 0 )
	{
		m_nHPMPRegainTickCountDown = HP_MP_REGAIN_INTER_TICK;

		if( GetAttValue(ERA_HPRegainRate) != 0 )
		{
			ChangeHP(GetAttValue(ERA_HPRegainRate), this);
		}
	}
}
//-------------------------------------------------------------------------------------------
// 将玩家加入到游戏世界中
//-------------------------------------------------------------------------------------------
BOOL Role::AddToWorld(BOOL bFirst)
{
	BOOL bRet = g_mapCreator.take_over_role_when_online(this);

	if( bRet && VALID_POINT(m_pScript) )
	{
		if( bFirst )		// 第一次进入游戏世界
		{
			m_pScript->OnRoleFirstIntoWorld(this);
		}

		//if(IsPurpureName())
		//{
		//	Vector3 pos = GetCurPos( );
		//	HearSayHelper::SendMessage(EHST_PURPUREPOS, GetID(), pos.x, pos.y, pos.z, GetMapID());
		//}

		m_pScript->OnRoleIntoWorld(this);	// 进入游戏世界

		m_pScript->OnRecharge(this, 0, GetTotalRecharge());

		if(CalcTimeDiff(GetCurrentDWORDTime(), m_LogoutTime) >= 3600 * 24 * 30 && get_level() >= 40 && AttRes::GetInstance()->GetVariableLen().n_LeaveTimeReward)
			m_pScript->OnLeaveTimeReward(this);

		tagDWORDTime* pTime = &m_LoginTempTime;
		m_pScript->OnLineCompensate(this, pTime->year, pTime->month, pTime->day);
	}

	return bRet;
}

//-----------------------------------------------------------------------
// 检查是否在对应职能NPC附近
//-----------------------------------------------------------------------
DWORD Role::CheckFuncNPC(DWORD dwNPCID, EFunctionNPCType eNPCType,
						 OUT Creature **ppNPC/* = NULL*/, OUT Map **ppMap/* = NULL*/)
{
	// 获得map
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC))
	{
		return E_NPC_NotFound;
	}

	if(!pNPC->IsFunctionNPC(eNPCType))
	{
		return E_NPC_NotValid;
	}

	if(!pNPC->CheckNPCTalkDistance(this))
	{
		return E_NPC_TooFar;
	}

	if(ppNPC != NULL)
	{
		*ppNPC = pNPC;
	}

	if(ppMap != NULL)
	{
		*ppMap = pMap;
	}

	return E_Success;
}

//----------------------------------------------------------------------------------------
// 初始化物品装备 -- 发给客户端
//----------------------------------------------------------------------------------------
VOID Role::SendInitStateItem()
{
	GetItemMgr().SendInitStateItem();
	GetItemMgr().SendInitStateItemCDTime();
}

//----------------------------------------------------------------------------------------
// 初始套装
//----------------------------------------------------------------------------------------
VOID Role::SendInitStateSuit()
{
	INT nSuitNum = m_Suit.GetSuitNum();
	if(0 == nSuitNum)
	{
		return;
	}

	INT nSzMsg = sizeof(NET_SIS_get_role_init_state_suit) - 1 + sizeof(tagSuitInit) * nSuitNum;

	CREATE_MSG(pSend, nSzMsg, NET_SIS_get_role_init_state_suit);
	if(!VALID_POINT(pSend))
	{
		ASSERT(VALID_POINT(pSend));
		return;
	}

	m_Suit.InitSendInitState(pSend->byData);
	pSend->nSuitNum = nSuitNum;
	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}


//---------------------------------------------------------------------------
// 发送人物初始基本属性给客户端
//---------------------------------------------------------------------------
VOID Role::SendInitStateAtt()
{
	INT nBuffNum = GetBuffNum();
	DWORD dw_size = sizeof(NET_SIS_get_role_init_state_att) + ((nBuffNum > 0) ? (nBuffNum - 1)*sizeof(tagBuffMsgInfo) : 0);

	CREATE_MSG(pSend, dw_size, NET_SIS_get_role_init_state_att);

	pSend->Avatar = m_Avatar;
	pSend->AvatarEquip = GetAvatarEquip();
	pSend->DisplaySet = GetDisplaySet();
	pSend->nLevel = m_nLevel;
	pSend->nCurLevelExp = m_nCurLevelExp;
	//pSend->nCredit = m_nCredit;
	//pSend->nIdentity = m_nIdentity;
	pSend->nVIPPoint = m_nVIPPoint;
	pSend->dwState = GetState();
	pSend->n64RoleState = m_RoleState.GetState();
	pSend->dwRoleStateEx = m_RoleStateEx.GetState();
	pSend->ePKState = m_ePKState;
	pSend->iPKValue = GetClientPKValue();
	//pSend->bIsPurpureDec = mIsPurpureDec;
	//pSend->dw_destory_equip_count = m_dwDestoryEquipCount;
	//pSend->dwRebornMapID = m_dwRebornMapID;
	//pSend->vRebornPoint = g_mapCreator.get_reborn_point(m_dwRebornMapID);
	//pSend->nRebornX = (int)g_mapCreator.get_reborn_point(m_dwRebornMapID).x;
	//pSend->nRebornY = (int)g_mapCreator.get_reborn_point(m_dwRebornMapID).z;
	pSend->dwGuildID = GetGuildID();
	pSend->u16ActTitleID[0] = GetTitleMgr()->GetActiviteTitle(0);
	pSend->u16ActTitleID[1] = GetTitleMgr()->GetActiviteTitle(1);
	pSend->u16ActTitleID[2] = GetTitleMgr()->GetActiviteTitle(2);
	//pSend->bHasLeftMsg = m_bHasLeftMsg;
	pSend->eClassType = m_eClass;
	//pSend->eClassTypeEx = m_eClassEx;
	//pSend->dwMasterID = m_dwMasterID;
	//pSend->n16PetPocketNum = m_pPetPocket->GetSize();
	//pSend->nTotalMasterMoral = m_nTotalMasterMoral;
	pSend->nKillNum = m_nKillNum;
	//pSend->e_role_camp = e_role_camp;
	//pSend->e_temp_role_camp = e_temp_role_camp;
	//pSend->n16_exbag_step = n16_exbag_step;
	//pSend->n16_exware_step = n16_exware_step;
	//pSend->n_achievement_point = m_nAchievementPoint;
	pSend->n32CurExploits = m_CurMgr.GetExploits();//荣誉点
	pSend->nEquipRating = st_EquipTeamInfo.n32_Rating;
	//pSend->nCoolDownReviveCD = mCoolDownReviveCD;
	//pSend->n16PetXiulianSize = n16_pet_xiulian_size;
	pSend->n_sign_level = m_nSignLevel;
	pSend->dwSpouseID = GetSpouseID();//gx add 2013.7.3
	pSend->nVIPLevel = GetVIPLevel();//gx add
	pSend->dwVIPDeadline = GetVIPDeatLine();//gx add
	pSend->nMeiLi = get_shihun();//魅力值
	pSend->nInstanceData = m_nInstanceData;//副本数据
	pSend->dwRedZuiFlag = GetScriptData(REDZUI_FLAG_INDEX);//红唇标记
	if (VALID_POINT(GetSession()))
	{
		pSend->n_total_recharge = GetSession()->GetTotalRecharge();
		//pSend->nVIPLevel = GET_VIP_EXTVAL(GetTotalRecharge(), VIP_LEVEL, INT);
	}
	
	get_fast_code()->memory_copy(pSend->byRoleHelp, m_byRoleHelp, sizeof(m_byRoleHelp));

	get_fast_code()->memory_copy(pSend->nAtt, m_nAtt, sizeof(m_nAtt));
	//get_fast_code()->memory_copy(pSend->nAttPointAdd, m_nAttPointAdd, sizeof(m_nAttPointAdd));

	s_role_info* p_role_info = g_roleMgr.get_role_info(this->GetID( ));
	if(VALID_POINT(p_role_info))
		get_fast_code()->memory_copy(pSend->sz_role_name, p_role_info->sz_role_name, sizeof(pSend->sz_role_name));

	pSend->nBuffNum = nBuffNum;

	if( nBuffNum > 0 )
	{
		GetAllBuffMsgInfo(pSend->Buff, nBuffNum);
	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

//----------------------------------------------------------------------------
// 发送人物初始技能给客户端
//----------------------------------------------------------------------------
VOID Role::SendInitStateSkill()
{
	INT n_num = m_mapSkill.size();

	package_map<DWORD, Skill*>::map_iter it = m_mapSkill.begin();
	Skill* pSkill = NULL;

	if( n_num <= 0 )
	{
		NET_SIS_get_role_init_state_skill send;
		send.n_num = 0;

		SendMessage(&send, send.dw_size);
	}

	else
	{
		DWORD dw_size = sizeof(NET_SIS_get_role_init_state_skill) + sizeof(tagSkillMsgInfo) * (n_num-1);
		CREATE_MSG(pSend, dw_size, NET_SIS_get_role_init_state_skill);

		pSend->dw_size = dw_size;
		pSend->n_num = n_num;

		INT nIndex = 0;
		while( m_mapSkill.find_next(it, pSkill) )
		{
			pSkill->GenerateMsgInfo(&pSend->Skill[nIndex]);
			nIndex++;
		}

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}


}
//获得称号
VOID Role::SetTitle(DWORD dwID) 
{
	m_pTitleMgr->SetTitle(dwID); 
}
BOOL Role::HasTitle(DWORD dwID)
{
	return m_pTitleMgr->HasTitle(dwID);
}

//----------------------------------------------------------------------------
// 发送人物完成任务列表给客户端
//----------------------------------------------------------------------------
VOID Role::send_init_complete_quest()
{
	// 得到完成任务的个数
	INT n_complete = GetCompleteQuestCount();
	
	DWORD dw_size = sizeof(NET_SIS_get_role_init_state_complete_quest);
	if( n_complete > 0 )
	{
		dw_size += (n_complete-1) * sizeof(tagCompleteQuestMsgInfo);
	}
	
	CREATE_MSG(p_send, dw_size, NET_SIS_get_role_init_state_complete_quest);

	p_send->n_num = n_complete;

	INT n_index = 0;
	tagQuestDoneSave* p_done = NULL;
	QuestDoneMap::map_iter it = done_quest_.begin();

	while( done_quest_.find_next(it, p_done) )
	{
		p_send->completeQuest[n_index].u16QuestID = p_done->u16QuestID;
		p_send->completeQuest[n_index].dwStartTime = p_done->dwStartTime;
		p_send->completeQuest[n_index].nTimes = p_done->nTimes;
		++n_index;

		if( n_index >= n_complete ) break;
	}

	SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}

//----------------------------------------------------------------------------
// 发送人物当前任务列表给客户端
//----------------------------------------------------------------------------
VOID Role::send_init_incomplete_quest()
{
	//得到当前任务的个数
	INT n_number = get_current_quest_count();

	DWORD dw_msg_size = sizeof(NET_SIS_get_role_init_state_incomplete_quest);
	if( n_number > 0 )
	{
		dw_msg_size += (n_number-1) * sizeof(tagIncompleteQuestMsgInfo);
	}

	CREATE_MSG(p_send, dw_msg_size, NET_SIS_get_role_init_state_incomplete_quest);

	p_send->n_num = n_number;

	INT		n_index = 0;
	BYTE*	p_temp = (BYTE*)p_send->incompleteQuest;	
	for(INT n = 0; n < QUEST_MAX_COUNT; ++n)
	{
		if( !quests_[n].valid() ) continue;

		DWORD dw_quest_size = 0;

		quests_[n].make_msg_info((tagIncompleteQuestMsgInfo*)p_temp, dw_msg_size, dw_quest_size);
		++n_index;
		p_temp += dw_quest_size;
		

		if( n_index >= n_number ) break;
	}
	p_send->dw_size = dw_msg_size;
	SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}

//----------------------------------------------------------------------------
//  发送人物金钱给客户端
//----------------------------------------------------------------------------
VOID Role::SendInitStateMoney()
{
	NET_SIS_get_role_init_state_money send;

	CurrencyMgr &CurMgr		= GetCurMgr();
	send.nBagYuanBao		= CurMgr.GetBagYuanBao();
	send.n64BagSilver		= CurMgr.GetBagSilver();
	send.n64BagBindSilver	= CurMgr.GetBagBindSilver();
	send.n64WareSilver		= CurMgr.GetWareSilver();
	send.nBaiBaoYuanBao		= CurMgr.GetBaiBaoYuanBao();
	send.nExchangeVolume	= CurMgr.GetExchangeVolume();
	//send.nExchangeVolume	= GetSession()->GetTotalRecharge();//gx add 2013.9.9
	send.n32Exploit			= CurMgr.GetExploits();

	/*for (INT nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		send.nCurClanCon[nClanType] = CurMgr.GetClanCon(static_cast<ECLanType>(nClanType));
	}*/

	SendMessage(&send, send.dw_size);
}

//----------------------------------------------------------------------------
//  发送人物当前声望数据给客户端
//----------------------------------------------------------------------------
VOID Role::SendInitStateReputation()
{
	NET_SIS_get_role_init_state_reputation send;

	ClanData& clanData = GetClanData();

	for (INT32 clantype = ECLT_BEGIN; clantype < ECLT_END; ++clantype)
	{
		send.nReputation[clantype]	= clanData.RepGetVal((ECLanType)clantype);
		send.nActiveCount[clantype]	= clanData.ActCountGetVal((ECLanType)clantype);
		send.bisFame[clantype]		= clanData.IsFame((ECLanType)clantype);
	}

	SendMessage(&send, send.dw_size);
}

//----------------------------------------------------------------------------
//  发送人物所在帮派属性数据给客户端
//----------------------------------------------------------------------------
VOID Role::SendInitStateGuild()
{
	NET_SIS_get_role_init_state_guild send;

	g_guild_manager.send_role_guild_info(this, send.sGuildBase, send.sGuildMember);

	SendMessage(&send, send.dw_size);
}

//----------------------------------------------------------------------------
//  发送防沉迷数据给客户端
//----------------------------------------------------------------------------
VOID Role::SendFatigueGuardInfo( BYTE byCode )
{
	NET_SIS_enthrallment_limit send;
	send.byLimitCode = byCode;
	SendMessage(&send, send.dw_size);
}
//----------------------------------------------------------------------------
//  发送人物好友
//----------------------------------------------------------------------------
VOID Role::SendFriend()
{
	//得到好友个数
	INT nFriendNum = GetFriendCount();
	INT nIndex = 0;
	s_role_info* pRoleInfo = (s_role_info*)INVALID_VALUE;

	DWORD dw_size = sizeof(NET_SIS_send_friend_list);
	if( nFriendNum > 0 )
	{
		dw_size += (nFriendNum-1) * sizeof(tagFriendInfo);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_send_friend_list);
	//get_fast_code()->memory_copy(pSend->dwRolesID, m_dwBlackList, sizeof(DWORD) * MAX_BLACKLIST);
	//get_fast_code()->memory_copy(pSend->dwEnemyID, m_dwEnemyList, sizeof(DWORD) * MAX_ENEMYNUM);

	pSend->n_num = nFriendNum;

	for(INT n = 0; n < MAX_FRIENDNUM; ++n)
	{
		if( INVALID_VALUE == m_Friend[n].dwFriendID ) continue;

		pSend->FriendInfo[nIndex].dwFriendID = m_Friend[n].dwFriendID;
		pSend->FriendInfo[nIndex].dwFriVal = m_Friend[n].dwFriVal;
		pSend->FriendInfo[nIndex].byGroup = m_Friend[n].byGroup;
		
		pRoleInfo = g_roleMgr.get_role_info(m_Friend[n].dwFriendID);
		if(VALID_POINT(pRoleInfo))
		{
			pSend->FriendInfo[nIndex].bOnline = pRoleInfo->b_online_;
			pSend->FriendInfo[nIndex].nLevel = pRoleInfo->by_level;
			pSend->FriendInfo[nIndex].eClassType = pRoleInfo->e_class_type_;
			pSend->FriendInfo[nIndex].bySex = pRoleInfo->by_sex_;
		}

		++nIndex;
	}
	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);	
}

//----------------------------------------------------------------------------
//  发送人物黑名单
//----------------------------------------------------------------------------
VOID Role::SendBlack()
{
	INT nBlackNum = GetBlackCount();
	INT nIndex = 0;
	s_role_info* pRoleInfo = (s_role_info*)INVALID_VALUE;

	DWORD dw_size = sizeof(NET_SIS_send_black_list);
	if(nBlackNum > 0)
	{
		dw_size += (nBlackNum-1) * sizeof(tagBlackInfo);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_send_black_list);

	pSend->n_num = nBlackNum;

	for(INT i = 0; i < MAX_BLACKLIST; i++)
	{
		if(INVALID_VALUE == m_dwBlackList[nIndex]) continue;

		pSend->BlackInfo[nIndex].dwBlackID = m_dwBlackList[i];
		
		pRoleInfo = g_roleMgr.get_role_info(m_dwBlackList[i]);
		if(VALID_POINT(pRoleInfo))
		{
			pSend->BlackInfo[nIndex].nLevel = pRoleInfo->by_level;
			pSend->BlackInfo[nIndex].eClassType = pRoleInfo->e_class_type_;
			pSend->BlackInfo[nIndex].bySex = pRoleInfo->by_sex_;
		}
		++nIndex;
	}
	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}
//-------------------------------------------------------------------------------------
// 得到仇人个数
//-------------------------------------------------------------------------------------
INT Role::GetEnemyCount()
{
	INT nCount = 0; 
	for(INT i = 0; i < MAX_ENEMYNUM; i++)
	{
		if(m_dwEnemyList[i] == INVALID_VALUE)
			continue;
		// 改成从数据库操作，这里没必要判断
		/*if ( !VALID_POINT(g_roleMgr.get_role_info(m_dwEnemyList[i])))
		{
			m_dwEnemyList[i] = INVALID_VALUE;
			continue;
		}*/
		nCount++;
	}
	return nCount;
}
//----------------------------------------------------------------------------
//  发送人物仇人
//----------------------------------------------------------------------------
VOID Role::SendEnemy()
{
	INT nEnemyNum = GetEnemyCount();
	INT nIndex = 0;
	s_role_info* pRoleInfo = (s_role_info*)INVALID_VALUE;

	DWORD dw_size = sizeof(NET_SIS_send_enemy_list);
	if(nEnemyNum > 0)
	{
		dw_size += (nEnemyNum-1) * sizeof(tagEnemyInfo);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_send_enemy_list);

	pSend->n_num = nEnemyNum;

	for(INT i = 0; i < MAX_ENEMYNUM; i++)
	{
		if(INVALID_VALUE == m_dwEnemyList[i]) continue;

		pSend->EnemyInfo[nIndex].dwEnemyID = m_dwEnemyList[i];

		pRoleInfo = g_roleMgr.get_role_info(m_dwEnemyList[i]);
		if(VALID_POINT(pRoleInfo))
		{
			pSend->EnemyInfo[nIndex].nLevel = pRoleInfo->by_level;
			pSend->EnemyInfo[nIndex].eClassType = pRoleInfo->e_class_type_;
			pSend->EnemyInfo[nIndex].bySex = pRoleInfo->by_sex_;
		}
		++nIndex;
	}
	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

//----------------------------------------------------------------------------------------
// 地图触发器
//----------------------------------------------------------------------------------------
BOOL Role::MapTrigger(DWORD dwTriggerID, DWORD dwMisc)
{
	Map* pMap = get_map();

	if( !VALID_POINT(pMap) ) return FALSE;

	const tag_map_info* pInfo = pMap->get_map_info();

	// 看是否存在该Trigger
	tag_map_trigger_info* pTrigger = pInfo->map_trigger.find(dwTriggerID);
	if( !VALID_POINT(pTrigger) ) return FALSE;

	// 检查玩家当前位置是不是在trigger范围之内
	if( !pMap->is_in_trigger(this, dwTriggerID) ) return FALSE;

	// 查看pTrigger的类型

	switch(pTrigger->e_type)
	{
		// 切换地图
	case EMT_GotoNewMap:
		{
			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(pTrigger->dw_map_id);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pTrigger->dw_way_point_id);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);

			Vector3 vFace;
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

			// 生成出生坐标和出生朝向
			
			// todo: 将来随机一个点，目前选取中心点

			// 移动到新地图
			if( FALSE == GotoNewMap(pTrigger->dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z, dwMisc))
				return FALSE;
		}
		break;
	case EMT_GotoSBK:
		{
			if (!g_guild_manager.is_door_open())
			{
				return FALSE;
			}
			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(pTrigger->dw_map_id);
			if( !VALID_POINT(pMapInfo) ) return FALSE;

			const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pTrigger->dw_way_point_id);
			if( !VALID_POINT(pList) ) return FALSE;

			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);

			Vector3 vFace;
			vFace.y = 0;
			vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
			vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

			// 移动到新地图
			if( FALSE == GotoNewMap(pTrigger->dw_map_id, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z, dwMisc))
				return FALSE;
		}
		break;
		// 任务触发器
	case EMT_Script:
		{
			// 得到该trigger对应的任务序列号
			if( !VALID_POINT(pTrigger->dw_quest_id) ) return FALSE;
			on_quest_event(EQE_Trigger, dwTriggerID);
		}
		break;

		// 什么都没有，则调用脚本
	case EMT_Null:
		{
			pMap->on_enter_trigger(this, pTrigger);
			GetAchievementMgr().UpdateAchievementCriteria(ete_into_area, pTrigger->dw_att_id, 1);
		}
		break;

	default:
		break;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------
// 通知队友进入副本
//--------------------------------------------------------------------------------------
DWORD Role::InstanceNotify(BOOL bNotify)
{
	if(bNotify == FALSE)	return INVALID_VALUE;

	// 找到玩家小队
	const Team* pTeam = g_groupMgr.GetTeamPtr(GetTeamID());
	if( !VALID_POINT(pTeam) ) return INVALID_VALUE;

	// 检查队伍是否创建了副本
	Map* pMap = g_mapCreator.get_map(pTeam->get_own_instance_mapid(), pTeam->get_own_instanceid());
	if( !VALID_POINT(pMap) || EMT_Instance != pMap->get_map_type() ) return INVALID_VALUE;

	map_instance_normal* pInstance = static_cast<map_instance_normal*>(pMap);
	if( !VALID_POINT(pInstance) ) return INVALID_VALUE;

	const tagInstance* pInstanceProto = pInstance->get_instance_proto();
	if( !VALID_POINT(pInstanceProto) ) return INVALID_VALUE;	

	// 副本是否允许通知队友
	if( !pInstanceProto->bNoticeTeamate ) return INVALID_VALUE;

	// 通知队友
	NET_SIS_instance_agree	 send;
	send.dwInviterID	=	m_dwID;
	send.dwMapID		=	pInstance->get_map_id();
	g_groupMgr.SendTeamInstanceNotify(this, GetTeamID(), &send, send.dw_size);

	return E_Success;
}

//--------------------------------------------------------------------------------------
// 玩家是否同意其它玩家进入副本的邀请
//--------------------------------------------------------------------------------------
DWORD Role::InstanceAgree(BOOL bAgree)
{
	if(bAgree == FALSE)		return INVALID_VALUE;

	if (IsInRoleStateAny(ERS_Commerce | ERS_PrisonArea))	return INVALID_VALUE;
			
	// 找到玩家小队
	const Team* pTeam = g_groupMgr.GetTeamPtr(GetTeamID());
	if( !VALID_POINT(pTeam) ) return INVALID_VALUE;

	// 检查队伍是否创建了副本
	Map* pMap = g_mapCreator.get_map(pTeam->get_own_instance_mapid(), pTeam->get_own_instanceid());
	if( !VALID_POINT(pMap) || EMT_Instance != pMap->get_map_type() ) return INVALID_VALUE;

	map_instance_normal* pInstance = static_cast<map_instance_normal*>(pMap);
	if( !VALID_POINT(pInstance) ) return INVALID_VALUE;

	const tagInstance* pInstanceProto = pInstance->get_instance_proto();
	if( !VALID_POINT(pInstanceProto) ) return INVALID_VALUE;

	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(pInstance->get_map_id());
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;

	const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pInstanceProto->dwEnterWayPoint);
	if( !VALID_POINT(pList) ) return INVALID_VALUE;

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);


	SetMyOwnInstanceID(pTeam->get_own_instanceid());
	SetMyOwnInstanceMapID(pTeam->get_own_instance_mapid());
	SetMyOwnInstanceCreateTime(pInstance->get_create_time());

	GotoNewMap(pInstance->get_map_id(), wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z, pInstance->get_instance_hard(), FALSE);

	return E_Success;
}

//---------------------------------------------------------------------------------
// 玩家请求离开副本
//---------------------------------------------------------------------------------
DWORD Role::LeaveInstance()
{
	const tag_map_info* pInfo = get_map()->get_map_info();
	if( !VALID_POINT(pInfo) ) return INVALID_VALUE;

	if( EMT_Normal == pInfo->e_type )
		return INVALID_VALUE;

	MapMgr* pMapMgr = g_mapCreator.get_map_manager(pInfo->dw_id);
	if( !VALID_POINT(pMapMgr) ) return INVALID_VALUE;

	pMapMgr->RoleInstanceOut(this);

	return E_Success;
}

//---------------------------------------------------------------------------------
// 计算pvp费用
//---------------------------------------------------------------------------------
INT32 Role::GetPVPMoney(INT nLevel)
{
	if(nLevel >=30 && nLevel <= 39)
		return 100;
	else if(nLevel >= 40 && nLevel <= 49)
		return 200;
	else if(nLevel >= 50 && nLevel <=59)
		return 300;
	else if(nLevel >= 60 && nLevel <= 65)
		return 400;
	else if(nLevel >= 66 && nLevel <= 70)
		return 500;
	else
		return 0;
}

//---------------------------------------------------------------------------------
// 玩家请求进入乱战副本
//---------------------------------------------------------------------------------
DWORD Role::EnterPVP()
{
	if(get_level() < 30)
		return E_Instance_pvp_level_limit;

	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pMap->get_map_info()->e_type == EMT_Instance)
		return E_Instance_pvp_map_limit;

	if(IsInRoleState(ERS_Stall) || IsDuel() || GetAutoKill() || IsInRoleState(ERS_PrisonArea) || IsInRoleState(ERS_Combat))
		return E_Instance_pvp_state_limit;
	
	if (is_in_guild_war())
		return E_Instance_pvp_in_guild_war;

	INT nMoney = GetPVPMoney(get_level());

	if(GetCurMgr().GetBagSilver() < nMoney)
		return E_Instance_pvp_money_limit;

	if(IsInRoleState(ERS_Prictice))
	{
		PracticeEnd();
	}

	if(IsInRoleState(ERS_Fishing))
	{
		StopFishing();
	}

	if(IsInRoleState(ERS_Hang))
	{
		CancelHang();
	}

	DWORD dwPVPMapID = get_tool()->crc32(_T("j01"));
	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dwPVPMapID);
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;

	// 得到目标地图的导航点
	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwPVPMapID);
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;

	const tag_map_way_point_info_list* pList = NULL;
	pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
	if( !VALID_POINT(pList) ) return INVALID_VALUE;


	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	GotoNewMap(dwPVPMapID, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

	GetCurMgr().DecBagSilver(nMoney, elcid_pvp);

	return E_Success;
}

DWORD Role::EnterBattle()
{

	// 等级不够
	if(get_level() < BATTLE_ROLE_LEVEL)
		return E_Instance_battle_not_level;

	// 没到时间
	if (BattleGround::get_singleton().getState() != EBS_BEGIN)
	{
		return E_Instance_battle_not_start;
	}


	// 人数满了不能进入
	if (BattleGround::get_singleton().getRoleNum() >= BATTLE_MAX_ROLE_NUMBER)
		return E_Instance_battle_max_num;


	if(IsInRoleState(ERS_Stall))
		return INVALID_VALUE;


	DWORD dwPVPMapID = get_tool()->crc32(_T("d200"));
	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dwPVPMapID);
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;


	// 得到目标地图的导航点
	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwPVPMapID);
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;


	EBATTLENTEAM eBt = BattleGround::get_singleton().getTeam();


	const tag_map_way_point_info_list* pList = NULL;

	ECamp ec;
	if(eBt == EBT_A)
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnemyEnterPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
		ec = ECA_Battle_a;
	}
	else
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
		ec = ECA_Battle_b;
	}


	set_temp_role_camp(ec);
	//BattleGround::get_singleton().addRole(GetID(), eBt);

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	GotoNewMap(dwPVPMapID, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z);

	return E_Success;
}
//--------------------------------------------------------------------------------------
// 得到所属的副本ID（如果有队伍获取队伍的，如果没有队伍获取自己的）
//--------------------------------------------------------------------------------------     
DWORD Role::GetOwnInstanceID() const
{
	DWORD dwTeamID = GetTeamID();
	
	// 玩家有小队
	if( dwTeamID != INVALID_VALUE )
	{
		const Team *pTeam = g_groupMgr.GetTeamPtr(dwTeamID);
		if( VALID_POINT(pTeam) )
		{
			return pTeam->get_own_instanceid();
		}
		else
		{
			return INVALID_VALUE;
		}
	}
	else // 玩家没有小队
	{
		return INVALID_VALUE;
	}
}

//--------------------------------------------------------------------------------------
// 得到所属的副本地图ID（如果有队伍获取队伍的，没有队伍获取自己的）
//--------------------------------------------------------------------------------------
DWORD Role::GetOwnInstanceMapID() const
{
	DWORD dwTeamID = GetTeamID();

	// 玩家有小队
	if( dwTeamID != INVALID_VALUE )
	{
		const Team *pTeam = g_groupMgr.GetTeamPtr(dwTeamID);
		if( VALID_POINT(pTeam) )
		{
			return pTeam->get_own_instance_mapid();
		}
		else
		{
			return INVALID_VALUE;
		}
	}
	else // 玩家没有小队
	{
		return m_dwOwnInstanceMapID;
	}
}

//--------------------------------------------------------------------------------------
// 去一个新地图
//--------------------------------------------------------------------------------------
BOOL Role::GotoNewMap(DWORD dwDestMapID, FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fFaceX,FLOAT fFaceY, FLOAT fFaceZ, DWORD dwMisc, BOOL bSameInstace)
{
	if( fX < 0.0f || fZ < 0.0f )
	{
		return FALSE;
	}

	// 检查一下地图的大小
	const tag_map_info* pInfo = g_mapCreator.get_map_info(dwDestMapID);
	if( !VALID_POINT(pInfo) ) return FALSE;

	// 检查一下坐标
	if( fX < 0.0f || fX >= FLOAT(pInfo->n_width * TILE_SCALE) ||
		fZ < 0.0f || fZ >= FLOAT(pInfo->n_height * TILE_SCALE) )
		return FALSE;

	// 首先检查是不是本地图
	if( GetMapID() == dwDestMapID && bSameInstace )
	{
		// 同地图内传送
		Vector3 vNewPos(fX, fY, fZ);
		m_MoveData.ForceTeleport(vNewPos);

		// 得到新的九宫格
		get_map()->send_goto_new_map_to_player(this);

		return TRUE;
	}

	else
	{
		// 是新地图， 首先找到MapMgr
		MapMgr* pMapMgr = g_mapCreator.get_map_manager(dwDestMapID);
		if( !VALID_POINT(pMapMgr) ) return FALSE;

		// 没有找到MapMgr，地图ID非法
		Map* pMap = get_map();
		if( !VALID_POINT(pMap) ) return FALSE;

		// 尝试切换地图
		Vector3 vDestPos;
		vDestPos.x = fX;
		vDestPos.y = 0;
		vDestPos.z = fZ;

		Vector3 vFace;
		vFace.x = fFaceX;
		vFace.y = 0;
		vFace.z = fFaceZ;

		g_mapCreator.role_change_map(m_dwID, dwDestMapID, vDestPos, vFace, dwMisc);

		return TRUE;
	}
}

//----------------------------------------------------------------------------------------------------
// 回城
//----------------------------------------------------------------------------------------------------
VOID Role::ReturnCity()
{
	if( !VALID_POINT(m_dwRebornMapID) ) return;

	const tag_map_info* pInfo = g_mapCreator.get_map_info(m_dwRebornMapID);
	if( !VALID_POINT(pInfo) ) return;

	GotoNewMap(m_dwRebornMapID, pInfo->v_reborn_positon.x, pInfo->v_reborn_positon.y, pInfo->v_reborn_positon.z);
}

//-------------------------------------------------------------------------------------
// 学习一个技能
//-------------------------------------------------------------------------------------
INT Role::LearnSkill(DWORD dwSkillID, DWORD dwNPCID, INT64 n64ItemID)
{


	//INT nTalentIndex = INVALID_VALUE;			// 如果是天资技能且该系天资已投，则记录该系天资的位置
	//INT nBlankTalentIndex = INVALID_VALUE;		// 如果是天资技能但该系天资未投，则记录该系天资可插入的位置

	// 如果是天资技能，则检测该天资在人物身上是否已经存在
	//if( ETT_Null != pProto->eTalentType )
	//{
	//	nTalentIndex = FindTalentIndex(pProto->eTalentType);

	//	if( INVALID_VALUE != nTalentIndex )
	//	{
	//		// 检测人物该系天资所投的天资点是不是满足技能可学条件
	//		if( GetTalentPoint(nTalentIndex) < pProto->nNeedTalentPoint )
	//			return E_LearnSkill_NeedMoreTalentPoint;
	//	}
	//	else
	//	{
	//		// 人物身上没有该系天资，则判断是否还可以再学习新的天资
	//		nBlankTalentIndex = FindBlankTalentIndex(pProto->eTalentType);
	//		if( INVALID_VALUE == nBlankTalentIndex )
	//			return E_LearnSkill_ExceedTalentMaxNum;
	//	}
	//}
	
	//// NPC方式学习
	//if( INVALID_VALUE != dwNPCID )
	//{
	//	Creature* pNPC = get_map()->find_creature(dwNPCID);
	//	if( !VALID_POINT(pNPC) ) return E_LearnSkill_NPCNotEixst;

	//	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
	//		return E_LearnSkill_NPCTooFar;

	//	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Teacher) )
	//		return E_LevelUpSkill_NPCCanNotTeach;
	//	
	//	// todo: 还要检测NPC可不可以教授该种技能
	//	const tagLearnSkill* pLearnSkill = AttRes::GetInstance()->GetLearnSkillProto(pNPC->GetProto()->uFunctionID.dwCommonID);
	//	if (!VALID_POINT(pLearnSkill))
	//		return E_LevelUpSkill_NPCCanNotTeach;
	//	
	//	if (pLearnSkill->setLearnSkill.find(dwSkillID) == pLearnSkill->setLearnSkill.end())
	//		return E_LevelUpSkill_NPCCanNotTeach;

	//	// 检测金钱是否满足
	//	if (GetCurMgr().GetBagAllSilver() < pProto->dwMoneyConsume)
	//		return E_LevelUpSkill_NPCNotMoney;

	//	// 金钱消耗
	//	GetCurMgr().DecBagSilverEx(pProto->dwMoneyConsume, elcid_learn_skill);

	//}
	// 技能书学习
	
	tagItem *pItem = GetItemMgr().GetBagItem(n64ItemID);
	if( !VALID_POINT(pItem) )
		return E_Item_NotFound;

	// 使用的等级判断
	if( pItem->IsLevelLimit(get_level()) )
		return E_Item_LevelLimit;
	
	// 检测特殊效果类型
	if( pItem->pProtoType->eSpecFunc != EISF_LearnSkill 
		/*|| Skill::GetIDFromTypeID(pItem->pProtoType->nSpecFuncVal1) != dwSkillID*/)
		return E_Item_Used_NotValid;

	dwSkillID = Skill::GetIDFromTypeID(pItem->pProtoType->nSpecFuncVal1);
	
	
	// 检查该技能是否已经存在
	Skill* pSkill = GetSkill(dwSkillID);
	if( VALID_POINT(pSkill) ) return E_LearnSkill_Existed;

	// 首先找到该技能对应的静态属性（默认一级）
	DWORD dwSkillTypeID = Skill::CreateTypeID(dwSkillID, 1);
	const tagSkillProto* pProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	if( !VALID_POINT(pProto) ) return E_LearnSkill_ProtoNotFound;

	// 检测人物等级
	if( m_nLevel < pProto->nNeedRoleLevel )
		return E_LearnSkill_NeedMoreLevel;

	// 检测职业限制
	//if (!(((m_eClassEx << 9) + m_eClass ) & pProto->dwLearnVocationLimit))
	//	E_LearnSkill_ClassLimit;
	
	if (m_eClass != pProto->dwLearnVocationLimit)
		return E_LearnSkill_ClassLimit;

	// 检测前置技能
	if( INVALID_VALUE != pProto->dwPreLevelSkillID )
	{
		DWORD dwPreSkillID = Skill::GetIDFromTypeID(pProto->dwPreLevelSkillID);
		INT nPreSkillLevel = Skill::GetLevelFromTypeID(pProto->dwPreLevelSkillID);
		Skill* pPreSkill = GetSkill(dwPreSkillID);
		if( !VALID_POINT(pPreSkill) || pPreSkill->get_level() < nPreSkillLevel )
			return E_LearnSkill_NoPreSkill;

	}



	// 一切判断通过，则给人物添加技能
	INT nSelfLevel = 1;
	INT nLearnLevel = 0;


	// 添加技能
	pSkill = new Skill(dwSkillID, nSelfLevel, nLearnLevel, 0, 0, 0);
	AddSkill(pSkill);

	GetItemMgr().DelFromBag(pItem->n64_serial, elcid_learn_skill, 1);

	// 重新计算属性
	if( NeedRecalAtt() )
		RecalAtt();

	if( VALID_POINT(m_pScript) ) m_pScript->OnLearnSkill( this, dwSkillID, nLearnLevel );

	return E_Success;

}


//---------------------------------------------------------------------------------------
// 升级一个技能
//---------------------------------------------------------------------------------------
INT Role::LevelUpSkill(DWORD dwSkillID, DWORD dwNPCID, INT64 n64ItemID)
{
	// 检查该技能是否已经存在
	Skill* pSkill = GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return E_LevelUpSkill_NotExist;

	// 检测技能是否已经到了最高等级
	if( pSkill->get_level() >= pSkill->GetMaxLevel() )
		return E_LevelUpSkill_ExceedMaxLevel;
	
	// 得到升级后的静态属性
	INT nLevel = pSkill->get_level() + 1;

	// 首先找到该技能对应的静态属性
	DWORD dwSkillTypeID = Skill::CreateTypeID(dwSkillID, nLevel);
	const tagSkillProto* pProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	if( !VALID_POINT(pProto) ) return E_LevelUpSkill_ProtoNotFound;

	// 检测人物等级
	if( m_nLevel < pProto->nNeedRoleLevel )
		return E_LearnSkill_NeedMoreLevel;

	int nSkillPoint = GetAttValue(ERA_TalentPoint);
	if (nSkillPoint <= 0)
		return E_LevelUpSkill_NoTalentPoint;

	// 检测前置技能
	DWORD dwPreSkillID = Skill::GetIDFromTypeID(pProto->dwPreLevelSkillID);
	INT nPreSkillLevel = Skill::GetLevelFromTypeID(pProto->dwPreLevelSkillID);
	Skill* pPreSkill = GetSkill(dwPreSkillID);
	if( !VALID_POINT(pPreSkill) || pPreSkill->get_level() < nPreSkillLevel )
		return E_LearnSkill_NoPreSkill;

	INT nRemainProficency = pSkill->GetLevelUpExp() - pSkill->GetProficiency();
	
	int nCastPoint = min(nSkillPoint, nRemainProficency);
	nCastPoint = max(nCastPoint, 0);
	ModAttValue(ERA_TalentPoint, -nCastPoint);
	ChangeSkillExp(pSkill, nCastPoint);

	return E_Success;

}

//---------------------------------------------------------------------------------
// 遗忘技能
//---------------------------------------------------------------------------------
INT Role::ForgetSkill(DWORD dwSkillID, DWORD dwNPCID)
{
	// 检测玩家身上是否有该技能
	Skill* pSkill = GetSkill(dwSkillID);
	if( !VALID_POINT(pSkill) ) return E_ForgetSkill_NotExist;

	// 检测技能合法性
	if( ETT_Null != pSkill->GetTalentType() )
		return E_ForgetSkill_CanNotBeForgotten;

	// 检查NPC合法性
	Creature* pNPC = get_map()->find_creature(dwNPCID);
	if( !VALID_POINT(pNPC) ||
		FALSE == pNPC->IsFunctionNPC(EFNPCT_Teacher) ||
		FALSE == pNPC->CheckNPCTalkDistance(this))
		return E_ForgetSkill_NPCNotValid;

	// todo：检测NPC是否拥有该技能

	// 检查通过，删除技能
	RemoveSkill(dwSkillID);

	// 重新计算属性
	if( NeedRecalAtt() )
		RecalAtt();

	return E_Success;
}

//-----------------------------------------------------------------------------------
// 洗点
//-----------------------------------------------------------------------------------
INT Role::ClearTalent(INT64 n64ItemID)
{
	// 判断物品本身是不是洗点道具
	tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
	if( !VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EISF_RemoveTalentPt )
		return E_ClearAttPoint_ItemNotValid;

	// 使用的等级判断
	if( pItem->IsLevelLimit(get_level()) )
		return E_Item_LevelLimit;

	// 判断通过，删除物品
	GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_clear_talent);

	// 检测是否有天资
	for(INT n = 0; n < X_MAX_TALENT_PER_ROLE; ++n)
	{
		if( ETT_Null == m_Talent[n].eType )
			continue;

		// 删除所有该系的天资技能（对map这样操作，可能会有问题）
		package_map<DWORD, Skill*>::map_iter it = m_mapSkill.begin();
		Skill* pSkill = NULL;
		while( m_mapSkill.find_next(it, pSkill) )
		{
			if( pSkill->GetTalentType() == m_Talent[n].eType )
				RemoveSkill(pSkill->GetID());
		}

		// 将点数返回给玩家
		ModAttValue(ERA_TalentPoint, GetTalentPoint(n));
		// 清空该系天资
		RemoveTalent(n);
		// 保存天资到数据库
		SaveTalentPoint2DB(n);
	}

	// 重新计算人物属性
	if( NeedRecalAtt() )
		RecalAtt();

	return E_Success;

}
//----------------------------------------------------------------------------------
//设置pk状态条件判定
//----------------------------------------------------------------------------------
DWORD Role::SetPKStateLimit(ERolePKState ePKState)
{
	Map* pMap = get_map( );
	if(VALID_POINT(pMap) && VALID_POINT(pMap->get_map_info()) 
		&& pMap->get_map_info()->e_type == EMT_Guild){
			return E_PK_AreaLimit;
	}
	if(ePKState == GetPKState())
		return E_SafeGuard_Already;

	// 20级限制
	//if(get_level() < PKLEVELMIN)
	//	return E_PK_LevelOutOf20;

	// 家族判断
	if(ePKState == ERolePK_Guild && (GetGuildID() == INVALID_VALUE))
		return E_PK_GuildLimit;

	// 区域判断
	//if( IsInRoleStateAny(ERS_RealSafeArea | ERS_Arena | ERS_GuildBattle) )
	//	return E_PK_AreaLimit;

	// 安全区内白名玩家不可改变
	//if( IsInRoleState(ERS_SafeArea) && !HasPKValue( ) )
	//	return E_PK_AreaLimit;

	// 判断是否是红名
	//if( HasPKValue( ) && ePKState == ERolePK_Peace)
	//	return E_PK_RedNameLimit;

	// 判断战斗状态
	//if(IsInRoleState(ERS_Combat) || IsDuel( ))
	//	return E_PK_StateLimit;

	return E_Success;
}

//----------------------------------------------------------------------------------
// 更新行凶状态
//----------------------------------------------------------------------------------
VOID Role::UpdatePK()
{
	//追杀令cd
	//m_dwUseKillbadgeCD > 0 ? --m_dwUseKillbadgeCD : 0; 

	if( !m_mapLastAttacker.empty( ))
	{//删除超时
		std::vector<DWORD> Deletes;

		DWORD dwValue = 0, dwKey = 0, dwCurTime = GetCurrentDWORDTime( );
		m_mapLastAttacker.reset_iterator( );
		while( m_mapLastAttacker.find_next( dwKey, dwValue))
			if( dwValue <= dwCurTime ) Deletes.push_back( dwKey );

		for( size_t n = 0; n < Deletes.size( ); ++n)
			m_mapLastAttacker.erase( Deletes[n] );
	}


	//DWORD dwPrisionID = g_mapCreator.get_prison_map_id();
	if( GetPKValue( ) )
	{
		DWORD dwTimeDiff = CalcTimeDiff(GetCurrentDWORDTime( ), m_dwPKValueDecTime);

		Map* pMap = get_map( );
		if(VALID_POINT( pMap ))
		{
			
			if( dwTimeDiff >= PK_DEC_INTERVALOF_PRISION )
			{
				SetPKValueMod( PK_DEC_ITENRVALOF_VALUE );
				m_dwPKValueDecTime = GetCurrentDWORDTime( );

				NET_SIS_change_pk_value send;
				send.dw_role_id = GetID();
				send.iPKValue = GetClientPKValue();
				pMap->send_big_visible_tile_message(this, &send, send.dw_size);
			}
		}
	}

	//if(GetPKValue( ) == 0 && this->GetMapID( ) == dwPrisionID && IsInRoleState(ERS_PrisonArea))
	//{
	//	Vector3 outpos = g_mapCreator.get_putout_prison_point();
	//	this->GotoNewMap(dwPrisionID,outpos.x, outpos.y, outpos.z);
	//}
}

//----------------------------------------------------------------------------------
// 开启PVP状态
//----------------------------------------------------------------------------------
VOID Role::SetPVP()
{
	if( !IsInRoleState(ERS_PVP) )
	{
		SetRoleState(ERS_PVP);
	}
	
	m_nUnSetPVPTickCountDown = UNSET_PVP_TICK;
}

//----------------------------------------------------------------------------------
// 更新PVP状态
//-----------------------------------------------------------------------------------
VOID Role::UpdatePVP()
{
	if( !IsInRoleState(ERS_PVP) ) return;

	if(--m_nUnSetPVPTickCountDown <= 0)
	{
		UnsetRoleState(ERS_PVP);
	}
}

//----------------------------------------------------------------------------------
// 计算PK状态
//----------------------------------------------------------------------------------
VOID Role::CalPKState(BOOL bSendMsg)
{
}


//----------------------------------------------------------------------------------
// 经验改变
//----------------------------------------------------------------------------------
VOID Role::ExpChange(INT nValue, BOOL bKill/* =FALSE */, BOOL bForce/* =FALSE */,INT nSpecial /* = 0 */)
{
	if( 0 == nValue ) return;

	// 减少经验
	if( nValue < 0 )
	{
		INT nTemp = m_nCurLevelExp;
		m_nCurLevelExp += nValue;
		if( m_nCurLevelExp < 0 ) m_nCurLevelExp = 0;

		NET_SIS_change_role_exp send;
		send.nExp = m_nCurLevelExp;
		send.nChange = m_nCurLevelExp - nTemp;
		send.bKill = bKill;
		send.nSpecial = nSpecial;//gx add
		SendMessage(&send, send.dw_size);

		if (!bKill)
		{
			SaveExp2DB();
		}

		return;
	}

	// 如果是增加经验
	INT32 nID = m_eClass*1000 + m_nLevel;
	const s_level_up_effect* pEffect = AttRes::GetInstance()->GetLevelUpEffect(nID);
	ASSERT(VALID_POINT(pEffect));
	
	if (!VALID_POINT(pEffect))
		return;

	INT nLevelUpExpRemain = pEffect->n_exp_level_up_ - m_nCurLevelExp;
	ASSERT( nLevelUpExpRemain >= 0 );

	// 不会升级
	if( nValue < nLevelUpExpRemain )
	{
		// 检测是否到了10%的倍数，如果到了，则给玩家回满血回满蓝
		/*INT nPhaseExp = pEffect->n_exp_level_up_ / 10;
		INT nOldPhase = m_nCurLevelExp / nPhaseExp;
		INT nNewPhase = (m_nCurLevelExp + nValue) / nPhaseExp;
		if( nOldPhase != nNewPhase )
		{
			SetAttValue(ERA_HP, GetAttValue(ERA_MaxHP));
			SetAttValue(ERA_MP, GetAttValue(ERA_MaxMP));
		}*/

		// 累加上新的经验
		m_nCurLevelExp += nValue;

		NET_SIS_change_role_exp send;
		send.nExp = m_nCurLevelExp;
		send.nChange = nValue;
		send.bKill = bKill;
		send.nSpecial = nSpecial;//gx add
		SendMessage(&send, send.dw_size);
	}

	// 需要升级
	else
	{
		// 根据是否强制升级来决定技能的等级上限
		INT nMaxLevel = ROLE_LEVEL_LIMIT;
		if( bForce )
		{
			nMaxLevel = MAX_ROLE_LEVEL;
		}

		m_nCurLevelExp += nLevelUpExpRemain;

		// 已经到达上限
		if( m_nLevel == nMaxLevel )
		{
			if( nLevelUpExpRemain != 0 )
			{
				NET_SIS_change_role_exp send;
				send.nExp = m_nCurLevelExp;
				send.nChange = nLevelUpExpRemain;
				send.bKill = bKill;
				send.nSpecial = nSpecial;//gx add
				SendMessage(&send, send.dw_size);
			}
		}

		// 可以升级
		else
		{
			INT nExpRealAdd = nLevelUpExpRemain;
			nValue -= nLevelUpExpRemain;
			INT nNextLevel = m_nLevel + 1;
			INT nAttPointAdd = 0;
			INT nTalentPointAdd = 0;

			// 看人物能升几级
			for(; nNextLevel <= nMaxLevel; nNextLevel++)
			{
				INT32 nID = m_eClass*1000 + nNextLevel;
				pEffect = AttRes::GetInstance()->GetLevelUpEffect(nID);
				ASSERT(VALID_POINT(pEffect));

				// 还可以再继续升级
				if( pEffect->n_exp_level_up_ <= nValue )
				{
					nValue -= pEffect->n_exp_level_up_;
					nExpRealAdd += pEffect->n_exp_level_up_;
					//nAttPointAdd += pEffect->n16RoleAttAvail;
					nTalentPointAdd += pEffect->n_talent_avail_;
				}
				// 不可以再升级了
				else
				{
					//nAttPointAdd += pEffect->n16RoleAttAvail;
					nTalentPointAdd += pEffect->n_talent_avail_;
					break;
				}
			}

			// 给人物改变等级
			if( nNextLevel > nMaxLevel )
			{
				nNextLevel = nMaxLevel;
				// 经验修正一下
				if( nValue > pEffect->n_exp_level_up_ )
				{
					nValue = pEffect->n_exp_level_up_;
				}
			}

			LevelChange(nNextLevel, bKill, bForce);

			m_nCurLevelExp = nValue;
			nExpRealAdd += nValue;

			// 给玩家增加属性点和天资点
			ModAttValue(ERA_AttPoint, nAttPointAdd);
			ModAttValue(ERA_TalentPoint, nTalentPointAdd);

			// 发送经验改变
			NET_SIS_change_role_exp send;
			send.nExp = m_nCurLevelExp;
			send.nChange = nExpRealAdd;
			send.bKill = bKill;
			send.nSpecial = nSpecial;//gx add
			SendMessage(&send, send.dw_size);

			SaveAttPoint2DB();
			SaveTalentPoint2DB(0);
			//m_SaveRoleLock.Acquire();
			//SaveToDB();
			//m_SaveRoleLock.Release();
		}
	}
	if (!bKill)
	{
		SaveExp2DB();
	}
}

//----------------------------------------------------------------------------
// 改变等级
//----------------------------------------------------------------------------
VOID Role::LevelChange(INT nValue, BOOL bKill, BOOL bForce)
{
	// 根据是否强制升级来决定最高等级
	INT nMaxLevel = ROLE_LEVEL_LIMIT;
	if( bForce ) nMaxLevel = MAX_ROLE_LEVEL;

	if( nValue < 0 || nValue > nMaxLevel )
		return;

	if( m_nLevel == nValue ) return;

	m_nLevel = nValue;
	m_nCurLevelExp = 0;

	GetAchievementMgr().UpdateAchievementCriteria(ete_role_level);

	// 发送等级改变
	NET_SIS_change_role_level send;
	send.dw_role_id = GetID();
	send.nLevel = m_nLevel;
	send.bKill = bKill;

	SaveLevel2DB();

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}

	// 玩家在小队中,发送等级改变事件
	if(GetTeamID() != INVALID_VALUE)
		g_groupMgr.AddEvent(GetID(), EVT_ChangeLevel, send.dw_size, &send);

	// 同步tagRoleInfo
	g_socialMgr.AddEvent(GetID(), EVT_SynRoleLevel, send.dw_size, &send);

	//gx modify 2013.12.12
	g_MasterPrenticeMgr.AddEvent(GetID(), EVT_SynRoleLevel, send.dw_size, &send);
	//end

	//设置基础属性
	SetBaseAttByLevel(m_eClass, m_nLevel);

	RecalAtt();

	// 回满血
	SetAttValue(ERA_HP, GetAttValue(ERA_MaxHP));
	SendAttChange(ERA_MaxHP);
	
	SetAttValue(ERA_MP, GetAttValue(ERA_MaxMP));
	SendAttChange(ERA_MaxMP);
	

	//SetRating();
	//SendTeamEquipInfo();


	// 调用升级的脚本函数
	if( VALID_POINT(m_pScript))
	{
		m_pScript->OnRoleLevelChange(this);
	}
}

VOID Role::SetBaseAttByLevel(EClassType eClassType, INT nLevel)
{
	// 通过人物等级计算对应的一级属性
	INT32 nID = eClassType*1000 + nLevel;
	const s_level_up_effect* pLevelUp = AttRes::GetInstance()->GetLevelUpEffect(nID);
	ASSERT(VALID_POINT(pLevelUp));
	//SetBaseAttValue(ERA_Physique, pLevelUp->n_role_att_[ERA_Physique]);
	//SetBaseAttValue(ERA_Strength, pLevelUp->n_role_att_[ERA_Strength]);
	//SetBaseAttValue(ERA_Pneuma, pLevelUp->n_role_att_[ERA_Pneuma]);
	//SetBaseAttValue(ERA_InnerForce, pLevelUp->n_role_att_[ERA_InnerForce]);
	//SetBaseAttValue(ERA_Technique, pLevelUp->n_role_att_[ERA_Technique]);
	//SetBaseAttValue(ERA_Agility, pLevelUp->n_role_att_[ERA_Agility]);

	// 设置某些二级属性
	SetBaseAttValue(ERA_MaxHP, pLevelUp->n_hp_);
	SetBaseAttValue(ERA_MaxMP, pLevelUp->n_mp_);
	SetBaseAttValue(ERA_ExAttackMin, pLevelUp->n_role_att_[0]);
	SetBaseAttValue(ERA_ExAttackMax, pLevelUp->n_role_att_[1]);
	SetBaseAttValue(ERA_InAttackMin, pLevelUp->n_role_att_[2]);
	SetBaseAttValue(ERA_InAttackMax, pLevelUp->n_role_att_[3]);
	SetBaseAttValue(ERA_ArmorEx,	 pLevelUp->n_role_att_[4]);
	SetBaseAttValue(ERA_ArmorIn,	 pLevelUp->n_role_att_[5]);
	SetBaseAttValue(ERA_ExAttack,	 pLevelUp->n_role_att_[6]);
	SetBaseAttValue(ERA_ExDefense,	 pLevelUp->n_role_att_[7]);
	SetBaseAttValue(ERA_InAttack,	 pLevelUp->n_role_att_[8]);
	SetBaseAttValue(ERA_InDefense,	 pLevelUp->n_role_att_[9]);

	SetRating(-m_nBaseRating);
	m_nBaseRating = pLevelUp->n_rating;
	SetRating(m_nBaseRating);
	//SetBaseAttValue(ERA_MaxBrotherhood, pLevelUp->n_brotherhood_);
}

BOOL Role::IsAllEquipLevelHas(DWORD dwLevel, BOOL bAttack)
{
	for (int i = EEP_Head; i <= EEP_Shipin1; ++i)
	{
		if (bAttack && !MIsAttackEquip(i))
		{
			continue;
		}
		if (!bAttack && !MIsDefEquip(i))
		{
			continue;
		}

		tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)i);

		if (!VALID_POINT(pEquip) || pEquip->equipSpec.byConsolidateLevel < dwLevel || pEquip->GetEquipNewness() <= 0)
		{
			return FALSE;
		}

	}
	return TRUE;
}
//-------------------------------------------------------------------------------
// 投点
//-------------------------------------------------------------------------------
INT Role::BidAttPoint(const INT nAttPointAdd[X_ERA_ATTA_NUM])
{
	// 检测点数是否合法
	INT nSum = 0;
	for(INT n = 0; n < X_ERA_ATTA_NUM; n++)
	{
		if( nAttPointAdd[n] < 0 )
			return E_BidAttPoint_PointInvalid;

		nSum += nAttPointAdd[n];
	}

	// 如果总点数为0，则直接返回
	if( nSum <= 0 ) return E_BidAttPoint_PointInvalid;

	// 检测玩家当前身上的可投放属性点够不够
	if( GetAttValue(ERA_AttPoint) < nSum )
		return E_BidAttPoint_NoEnoughAttPoint;

	// 先扣除属性点
	ModAttValue(ERA_AttPoint, -nSum);

	// 再依次对应每个属性进行加成
	for(INT n = 0; n < X_ERA_ATTA_NUM; n++)
	{
		if( 0 == nAttPointAdd[n] ) continue;

		m_nAttPointAdd[n] += nAttPointAdd[n];
		ModBaseAttValue(MTransIndex2ERAATTA(n), nAttPointAdd[n]);
	}

	// 重算人物属性
	RecalAtt();

	// 发送每个属性投放量当前值给客户端
	NET_SIS_add_change_role_att_point send;
	get_fast_code()->memory_copy(send.nAttPointAdd, m_nAttPointAdd, sizeof(m_nAttPointAdd));
	SendMessage(&send, send.dw_size);

	SaveAttPoint2DB();

	return E_Success;
}

//----------------------------------------------------------------------------
// 洗属性点
//----------------------------------------------------------------------------
INT Role::ClearAttPoint(INT64& n64ItemID)
{
	// 判断物品本身是不是洗点道具
	tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
	if( !VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EISF_RemoveAttPt )
		return E_ClearAttPoint_ItemNotValid;

	// 使用的等级判断
	if( pItem->IsLevelLimit(get_level()) )
		return E_Item_LevelLimit;

	// 判断通过，删除物品
	GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_clear_att);

	// 清掉玩家所有一级属性投放
	INT nSum = 0;

	for(INT n = 0; n < X_ERA_ATTA_NUM; n++)
	{
		if( 0 == m_nAttPointAdd[n] ) continue;

		ModBaseAttValue(MTransIndex2ERAATTA(n), -m_nAttPointAdd[n]);
		nSum += m_nAttPointAdd[n];
		m_nAttPointAdd[n] = 0;
	}

	// 重算人物属性
	RecalAtt();

	// 更新当前可投放属性点
	ModAttValue(ERA_AttPoint, nSum);

	// 发送每个属性投放量当前值给客户端
	NET_SIS_add_change_role_att_point send;
	get_fast_code()->memory_copy(send.nAttPointAdd, m_nAttPointAdd, sizeof(m_nAttPointAdd));
	SendMessage(&send, send.dw_size);

	// 属性点存数据库
	SaveAttPoint2DB();

	return E_Success;
}

//-------------------------------------------------------------------------------
// 被攻击
//-------------------------------------------------------------------------------
//VOID Role::OnBeAttackedCalPK( Role* pAttacker )
//{
//	//自杀不增加PK值
//	if( pAttacker->GetID( ) == this->GetID( ) )return;
//
//	// 切磋不增加PK值
//	if( pAttacker->GetDuelTarget() == this->GetID() &&
//		pAttacker->GetID( ) == this->GetDuelTarget()) return;
//
//	if(pAttacker->IsInRoleState(ERS_PVPArea) && this->IsInRoleState(ERS_PVPArea)) return;
//
//	if(pAttacker->IsInRoleState(ERS_NoPunishArea) && this->IsInRoleState(ERS_NoPunishArea)) return;
//
//	//我是红名
//	if( HasPKValue( ) ) return;
//	
//	//对方还击
//	if( pAttacker->m_mapLastAttacker.is_exist( GetID( ) ) ) return;
//
//	if( m_mapLastAttacker.is_exist( pAttacker->GetID( ) ) )
//	{
//		this->SetLastAttacker( pAttacker->GetID( ) );
//		return;
//	}
//	//攻击者在脱离战斗6分钟内
//// 	DWORD dwTimeDiff = CalcTimeDiff(GetCurrentDWORDTime( ),pAttacker->GetLeaveCombatTime( ) );
//// 	if( m_mapLastAttacker.IsExist( pAttacker->GetID( ) ) )
//// 	{
//// 		if( dwTimeDiff > ATTACK_INCPK_INTERVAL ) return;
//// 	}
//	Map* pMap = pAttacker->get_map( );
//	if( !VALID_POINT(pMap) ) return;
//	if (!pMap->changePkValue()) return;
//
//	pAttacker->SetPKValueMod( ATTACK_WHITENAME_INCPK );
//	pAttacker->PKPenalty( TRUE );
//
//	
//	NET_SIS_change_pk_value send;
//	send.dw_role_id = pAttacker->GetID();
//	send.iPKValue = pAttacker->GetPKValue( );
//	pMap->send_big_visible_tile_message(this, &send, send.dw_size);
//	
//
//	this->SetLastAttacker( pAttacker->GetID( ) );
//}
VOID Role::OnBeAttacked(Unit* pSrc, Skill* pSkill, BOOL bHited, BOOL bBlock, BOOL bCrited)
{
	ASSERT( VALID_POINT(pSkill) && VALID_POINT(pSrc) );

	// 判断技能的敌我目标类型
	if( !pSkill->IsHostile() && !pSkill->IsFriendly() ) return;

	DWORD dwFriendEnemyFlag = pSrc->FriendEnemy(this);

	if( pSkill->IsHostile() && this != pSrc )
	{
		if( pSrc->IsRole() && (ETFE_Hostile & dwFriendEnemyFlag) )
		{
			Role* pAttacker = static_cast<Role*>(pSrc);
			//this->OnBeAttackedCalPK( pAttacker );
			GetCombatHandler().InterruptRide();
			if( AttackProbStopMount( ) )  BreakMount( );
		}
	}

	// 如果该技能可以对敌方使用，并且双方包含敌方关系
	if( pSkill->IsHostile() && this != pSrc && (ETFE_Hostile & dwFriendEnemyFlag) )
	{
		// 如果攻击方也是玩家，则设置对方是否进入灰名
		// 如果自己是红名玩家不计仇人
		// 如果在切磋不计仇人
		if( pSrc->IsRole() && !HasPKValue() && ( !IsDuel() || pSrc->GetID() != GetDuelTarget() ))
		{
			// 移到死亡处理 LC

			//Role* pAttacker = static_cast<Role*>(pSrc);
			//DWORD dwAttackerID = pAttacker->GetID( );
			//if(!IsEnemyList(dwAttackerID) && !IsFriend(dwAttackerID))
			//{// 加入仇人列表	
			//	for(INT i = 0; i < MAX_ENEMYNUM; i++)
			//	{
			//		if(GetEnemyList(i) == INVALID_VALUE)
			//		{
			//			
			//			SetEnemyList(i, dwAttackerID);

			//			NET_DB2C_insert_enemy send;
			//			send.dw_role_id = GetID();
			//			send.dw_enemy_id_ = dwAttackerID;
			//			g_dbSession.Send(&send, send.dw_size);

			//			NET_SIS_add_enemy AddSend;
			//			AddSend.dwDestRoleID = dwAttackerID;
			//			AddSend.bOnline = TRUE;
			//			AddSend.nLevel = pAttacker->get_level();
			//			AddSend.eClassType = pAttacker->GetClass();
			//			AddSend.by_sex = pAttacker->GetSex();
			//			SendMessage(&AddSend, AddSend.dw_size);
			//			break;
			//		}
			//	}
			//}	
		}
		else if( pSrc->IsCreature() )
		{
			// 怪物攻击玩家，增加一点仇恨
			Creature* pSrcCreature = static_cast<Creature*>(pSrc);
			if( VALID_POINT(pSrcCreature->GetAI()) )
				pSrcCreature->GetAI()->AddEnmity(this, 1);
		}

		// 检测身上的被动技能和装备的Buff触发
		if( bHited )
		{
			OnPassiveSkillBuffTrigger(pSrc, ETEE_Hited, 0, 0);
			OnEquipmentBuffTrigger(NULL, ETEE_Hited);
			OnBuffTrigger(pSrc, ETEE_Hited, 0, 0);
			//OnPetBuffTrigger(NULL, ETEE_Hited);
			GetItemMgr().ProcArmorNewness();//计算防具耐久度
			if( bBlock )
			{
				OnPassiveSkillBuffTrigger(pSrc, ETEE_Block, 0, 0);
				OnEquipmentBuffTrigger(NULL, ETEE_Block);
				OnBuffTrigger(pSrc, ETEE_Block, 0, 0);
			}
			if( bCrited )
			{
				OnPassiveSkillBuffTrigger(pSrc, ETEE_Crited, 0, 0);
				OnEquipmentBuffTrigger(NULL, ETEE_Crited);
				OnBuffTrigger(pSrc, ETEE_Crited, 0, 0);
			}

			// 检测当前技能的打断
			GetCombatHandler().InterruptPrepare(CombatHandler::EIT_Skill, ESSTE_Default == pSkill->GetTypeEx());

			// 检测某些被攻击打断的buff
			OnInterruptBuffEvent(EBIF_BeAttacked);
		}
		else
		{
			OnPassiveSkillBuffTrigger(pSrc, ETEE_Dodge, 0, 0);
			OnEquipmentBuffTrigger(NULL, ETEE_Dodge);
			OnBuffTrigger(pSrc, ETEE_Dodge, 0, 0);
		}
		OnPassiveSkillBuffTrigger(pSrc, ETEE_BeAttacked, 0, 0);
		OnEquipmentBuffTrigger(NULL, ETEE_BeAttacked);
		OnBuffTrigger(pSrc, ETEE_BeAttacked, 0, 0);
	}

	// 如果该技能可以对友方使用，并且双方包含友方关系
	if( pSkill->IsFriendly() && (ETFE_Friendly & dwFriendEnemyFlag) )
	{
		// 仇恨值
		INT	nValue = pSkill->GetEnmity() + pSrc->GetAttValue(ERA_Enmity_Degree);

		// 玩家身上被动仇恨所属的怪物全部增加仇恨
		DWORD dwCreatureID = INVALID_VALUE;
		Creature* pCreature = (Creature*)INVALID_VALUE;
		m_mapEnmityCreature.reset_iterator();
		while( m_mapEnmityCreature.find_next(dwCreatureID) )
		{
			pCreature = get_map()->find_creature(dwCreatureID);
			if( !VALID_POINT(pCreature) ) continue;

			if( VALID_POINT(pCreature->GetAI()) )
				pCreature->GetAI()->AddEnmity(pSrc, nValue);
		}
	}
}

//-------------------------------------------------------------------------------
// 死亡
//-------------------------------------------------------------------------------
VOID Role::OnDead(Unit* pSrc, Skill* pSkill/*=NULL*/, const tagBuffProto* pBuff/*=NULL*/, DWORD dwSerial, DWORD dwMisc, BOOL bCrit)
{
	// 如果本身已经死亡，则直接返回
	if( IsInState(ES_Dead) ) return;

	// 如果处于切磋状态不走死亡流程
	if(WhenDeadDuelDeal(pSrc)) return;

	if(PVPBiWuDead(pSrc)) return;

	//if(PVPDead(pSrc)) return;

	if(Dead1v1(pSrc)) return;

	// 死亡时触发
	OnPassiveSkillBuffTrigger(pSrc, ETEE_Die);
	OnEquipmentBuffTrigger(pSrc, ETEE_Die);
	OnBuffTrigger(pSrc, ETEE_Die);

	// 设置为死亡状态
	SetState(ES_Dead);

	// 
	StopMount( );
		
	removeFlowCreature();
	// 停止移动
	m_MoveData.StopMoveForce();

	// 停止当前正在释放的技能
	m_CombatHandler.End();
	
	if (VALID_POINT(pSrc))
	{

		DWORD dwFriendEnemyFlag = pSrc->FriendEnemy(this);

		if( this != pSrc && (ETFE_Hostile & dwFriendEnemyFlag) )
		{
			// 如果攻击方也是玩家，则设置对方是否进入灰名
			// 如果自己是红名玩家不计仇人
			// 如果在切磋不计仇人
			if( pSrc->IsRole() && !HasPKValue() && ( !IsDuel() || pSrc->GetID() != GetDuelTarget() ) && !is_in_guild_war())
			{
				Role* pAttacker = static_cast<Role*>(pSrc);
				DWORD dwAttackerID = pAttacker->GetID( );
				if(!IsEnemyList(dwAttackerID) && !IsFriend(dwAttackerID))
				{
					//如果在黑名单中则删除
					for (INT i = 0; i < MAX_BLACKLIST; i++)
					{
						if (GetBlackList(i) == dwAttackerID)
						{
							SetBlackList(i, INVALID_VALUE);
							
							NET_DB2C_delete_black send;
							send.dw_role_id = GetID();
							send.dw_black_id_ = dwAttackerID;
							g_dbSession.Send(&send, send.dw_size);
							
							NET_SIS_delete_black_list delSend;
							delSend.dwDestRoleID = dwAttackerID;
							delSend.dw_error_code = E_Success;
							SendMessage(&delSend, delSend.dw_size);
							break;
						}
					}
					// 加入仇人列表	
					for(INT i = 0; i < MAX_ENEMYNUM; i++)
					{
						if(GetEnemyList(i) == INVALID_VALUE)
						{

							SetEnemyList(i, dwAttackerID);

							NET_DB2C_insert_enemy send;
							send.dw_role_id = GetID();
							send.dw_enemy_id_ = dwAttackerID;
							g_dbSession.Send(&send, send.dw_size);

							NET_SIS_add_enemy AddSend;
							AddSend.dwDestRoleID = dwAttackerID;
							AddSend.bOnline = TRUE;
							AddSend.nLevel = pAttacker->get_level();
							AddSend.eClassType = pAttacker->GetClass();
							AddSend.by_sex = pAttacker->GetSex();
							SendMessage(&AddSend, AddSend.dw_size);
							break;
						}
					}
				}	
			}
		}
	}
	// 移除身上所有死亡后消失的buff
	OnInterruptBuffEvent(EBIF_Die);

	// 清空仇恨列表
	ClearEnmityCreature();

	BOOL bDeadPenalty = IsDeadPenalty();
	////死亡惩罚
	if(bDeadPenalty) 
		DeadPenalty(pSrc);

	//红名惩罚
	//if( HasPKValue( ) ) 
	PKPenalty( );

	// 击杀者
	if( VALID_POINT(pSrc) )
	{
		pSrc->OnKill(this);
	}
	
	//GetAchievementMgr().UpdateAchievementCriteria(ete_role_die, 1);	

	// 调用地图事件
	if( VALID_POINT(get_map()) )
	{
		get_map()->on_role_die(this, pSrc);
	}

	//GuildWarAchievement(pSrc);

	// 发送角色死亡消息
	NET_SIS_role_dead send;
	send.dw_role_id = GetID();
	send.dwSrcRoleID = (VALID_POINT(pSrc) ? pSrc->GetID() : INVALID_VALUE);
	send.eSuggestRevive = ERRT_None;
	send.bCrit = bCrit;
	//bHitFly改成怪物的typeID gx modify 2013.8.22
	send.bHitFly = INVALID_VALUE;
	if (VALID_POINT(pSrc))
	{
		if (pSrc->IsCreature())
		{
			Creature* pKillRole = (Creature*)pSrc;
			send.bHitFly = (int)pKillRole->GetTypeID();
		}
	}
	if( VALID_POINT(pSkill) )
	{
		send.eCause = ERDC_Skill;
		send.dwMisc = pSkill->GetTypeID();
		//send.bHitFly = (get_tool()->tool_rand() % 10000) < pSkill->GetProto()->nHitFlyPro ? TRUE: FALSE;
	}
	else if( VALID_POINT(pBuff) )
	{
		send.eCause = ERDC_Buff;
		send.dwMisc = pBuff->dwID;
	}
	else
	{
		send.eCause = ERDC_Other;
		send.dwMisc = INVALID_VALUE;
	}

	send.nCoolDownRevive = GetCoolDownReviveCD( );

	/*if( IsPurpureName() )
	//{// 牢狱复活
	//	send.eSuggestRevive = ERRT_Prison;	
	//}
	else if( HasPKValue( ) ) send.eSuggestRevive = ERRT_None;*/

	send.dwMisc2 = dwMisc;
	send.dwSerial = dwSerial;

	if( VALID_POINT(get_map()) )
	{
		//if(get_map()->get_map_info( )->b_CoolDownRevive)
		//	send.eSuggestRevive = ERRT_CoolDown;

		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}
}

//---------------------------------------------------------------------------------
// 击杀目标
//---------------------------------------------------------------------------------
VOID Role::OnKill(Unit* pTarget)
{
	ASSERT( VALID_POINT(pTarget) );
	if( pTarget->IsRole() )
	{//击杀的是玩家
		Role* pRole = static_cast<Role*>(pTarget);
		if( VALID_POINT(pRole) )
		{ 
			OnKillCalPKVal( pRole );
			//OnKillCalJustice( pRole );
			this->PKPenalty( TRUE );
			//if(pRole->IsPurpureName()){
			//	HearSayHelper::SendMessage(EHST_PURPUREBEKILLED, pRole->GetID(), GetID());
			//}
		}

		//触发被动技能和装备
		OnPassiveSkillBuffTrigger(pTarget, ETEE_Kill);
		OnEquipmentBuffTrigger(pTarget, ETEE_Kill);
		OnBuffTrigger(pTarget, ETEE_Kill);

		OnPassiveSkillBuffTrigger(pTarget, ETEE_Kill_Player);
		OnEquipmentBuffTrigger(pTarget, ETEE_Kill_Player);
		OnBuffTrigger(pTarget, ETEE_Kill_Player);

 		guild* pGuild = g_guild_manager.get_guild(this->GetGuildID());
 		if(VALID_POINT(pGuild) && VALID_VALUE(pRole->GetGuildID()))
 		{
 			// 是否是敌对帮派
 			if(pGuild->is_enemy(pRole->GetGuildID()))
 			{
 				// 增加成员帮派DKP
 				pGuild->change_member_dkp(this->GetID(),10); // 具体数值需要在确定
 			}
 		}
		
		// 帮会战时发公告
		if (get_map()->get_map_type() == EMT_Guild)
		{
			if (is_in_guild_war() && GetID() != pRole->GetID())
			{
				HearSayHelper::SendMessage(EHST_GuildMap_Kill, this->GetID(), pRole->GetID());
			}
		}
		
		if( VALID_POINT(pRole) && !is_in_guild_war() && GetID() != pRole->GetID())
		{ 
			guild* pDeadGuild = g_guild_manager.get_guild(pRole->GetGuildID());
			if(VALID_POINT(pDeadGuild))
			{
				NET_SIS_GuildMap_kill_role send;
				send.dwKillerID = GetID();
				send.dwDeadID = pRole->GetID();
				send.dwMapID = pRole->GetMapID();
				send.x = pRole->GetCurPos().x;
				send.z = pRole->GetCurPos().z;
				pDeadGuild->send_guild_message(&send, send.dw_size);
			}
		}



		// 为不同阵营
		// 增加荣誉值			
		
		GetAchievementMgr().UpdateAchievementCriteria(ete_kill_role, 1);	
		GetAchievementMgr().UpdateAchievementCriteria(ete_kill_role_level, pTarget->get_level(), 1);
		GetAchievementMgr().UpdateAchievementCriteria(ete_kill_role_level_sub, get_level() - pTarget->get_level(), 1);
		GetAchievementMgr().UpdateAchievementCriteria(ete_kill_role_class, ((Role*)pTarget)->GetClass(), 1);
		((Role*)pTarget)->GetAchievementMgr().UpdateAchievementCriteria(ete_role_skilled_by_role, 1);

// 		// 死者非全体模式
// 		if(!(pRole->GetPKState() == ERolePK_All))
// 		{
// 			DWORD dwGuildID = pRole->GetGuildID();
// 			if(dwGuildID == INVALID_VALUE) return;
// 
// 			guild* pGuild = g_guild_manager.get_guild(dwGuildID);
// 			if(!VALID_POINT(pGuild)) return;
// 
// 			// 帮派DKP
// 			pGuild->change_member_dkp(pRole->GetID(), 10); // 具体数值需在确定
// 			// 帮派贡献
// 			pGuild->change_member_contribution(pRole->GetID(), 20, FALSE); 
// 
// 		}

	}
	else if( pTarget->IsCreature() )
	{// 如果击杀的是怪物
		Creature* pCreature = dynamic_cast<Creature*>(pTarget);
		if (!VALID_POINT(pCreature))
			return;

		// 触发被动技能和装备
		if( !pCreature->IsGameObject() )
		{
			OnPassiveSkillBuffTrigger(pTarget, ETEE_Kill);
			OnEquipmentBuffTrigger(pTarget, ETEE_Kill);
			OnBuffTrigger(pTarget, ETEE_Kill);
		}

		// 称号相关
		
		GetAchievementMgr().UpdateAchievementCriteria(ete_kill_monster, pCreature->GetTypeID(), 1);
		
		
	}
}

//--------------------------------------------------------------------
// 死亡PK值计算
//--------------------------------------------------------------------
BOOL Role::OnKillCalPKVal( Role* pTarget )
{
	//自杀不增加PK值
	if( pTarget->GetID( ) == this->GetID( ) )return FALSE;
	if( pTarget->IsInRoleStateAny(ERS_PVPArea|ERS_PVP)) return FALSE;
	if (this->IsInRoleState(ERS_NoPunishArea))return FALSE;

	Map* pMap = get_map();
	if(!VALID_POINT(pMap)) return FALSE;
	if(!pMap->changePkValue()) return FALSE;


	//杀红名玩家，不增加PK值
	if( pTarget->IsRedName( ) ) 
	{
		//GetAchievementMgr().UpdateAchievementCriteria(ete_kill_red_role, 1);
		return FALSE;
	}
	// 还击时杀死别人不计算PK值
	if( m_mapLastAttacker.is_exist( pTarget->GetID( ) ) ) return FALSE;


	this->SetPKValueMod(ATTACK_WHITENAME_INCPK-1);

	this->IncKillNum();

	// 武器降幸运
	tagEquip* pWeapon = (tagEquip*)GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	if (VALID_POINT(pWeapon))
	{
		int nProto = get_tool()->tool_rand() % 100;
		if (nProto < 50)
		{

			EquipEffect(pWeapon, FALSE);

			pWeapon->equipSpec.byLuck -= 1;
			pWeapon->equipSpec.byLuck = max(-9, pWeapon->equipSpec.byLuck);

			EquipEffect(pWeapon, TRUE);

			GetItemMgr().UpdateEquipSpec(*pWeapon);

			//SetRating();
			//SendTeamEquipInfo();

		}
	}


	NET_SIS_change_pk_value send;
	send.dw_role_id = GetID( );
	send.iPKValue = GetClientPKValue();
	pMap->send_big_visible_tile_message(this, &send, send.dw_size);
	
	return TRUE;
}
VOID Role::OnKillCalJustice( Role* pTarget )
{
	if( pTarget->GetID( ) == this->GetID( ) )
		return  ;

	if(HasPKValue() || !pTarget->HasPKValue())
		return ;
	
	INT nJustice = 0;
	if(pTarget->IsRedName())
		nJustice = 1;
	//else if(pTarget->IsPurpureName())
	//	nJustice = 2;

	if(pTarget->IsHaveBuff(Buff::GetIDFromTypeID(KILLBUFFID)))
		nJustice += 1;

	add_justict_value(nJustice);
}
//--------------------------------------------------------------------
// PK惩罚
//--------------------------------------------------------------------
VOID Role::PKPenalty( BOOL bIgnoreLoot )
{
	//INT iPKVal = this->GetPKValue( );
	//if( iPKVal <= 0 ) return;
	//if( !IsPKPenalty( )) return;

	//掉落物品
	if( !bIgnoreLoot )
	{
		FLOAT fUsePerct = 0.1f;
		//if(IsYellowName()) fUsePerct = 0.1f;
		//else if(IsRedName()) fUsePerct = 0.15f;
		//else fUsePerct = 0.2f;

		//if(IsHaveBuff(Buff::GetIDFromTypeID(KILLBUFFID)))
		//	fUsePerct += 0.05f;

		for(INT16 n16Index = EEP_RightHand; n16Index <= EEP_Equip_End; ++n16Index)
		{
			tagEquip* pEquip = GetItemMgr().GetEquipBarEquip(n16Index);
			if(VALID_POINT(pEquip)) 
			{
				INT nNewness =(INT)(pEquip->GetMaxNewess() * fUsePerct);
				//gx modify 2013.8.30
				/*if(pEquip->GetEquipNewness( )<= nNewness)
					nNewness = pEquip->GetEquipNewness( ) - 1;
				
				if(nNewness > 0)*/

				INT16 nOldNewness = pEquip->GetEquipNewness();

				pEquip->ChangeNewness( 0 - nNewness);
				
				INT16 nCurNewness = pEquip->GetEquipNewness();
				if (nCurNewness <= 0 && nOldNewness > 0)
				{
					EquipEffect(pEquip, FALSE);

					//SetRating();
					//SendTeamEquipInfo();

				}

				NET_SIS_newess_change send;
				send.n64EquipSerial = pEquip->n64_serial;
				send.nAttackTimes = pEquip->nUseTimes;
				SendMessage(&send, send.dw_size);
			}	
		}
	}
}
//---------------------------------------------------------------------------------
// 是否进行死亡惩罚
//---------------------------------------------------------------------------------
BOOL Role::IsDeadPenalty()
{
	if(VALID_POINT(m_pScript))
		return m_pScript->IsDeadPenalty(this);

	return TRUE;
}

//---------------------------------------------------------------------------------
// 帮会战分数计算
//---------------------------------------------------------------------------------
VOID Role::GuildWarAchievement(Unit* pKill)
{
	if(!VALID_POINT(pKill))
		return;

	guild* pGuild = g_guild_manager.get_guild(GetGuildID());
	if(!VALID_POINT(pGuild))
		return ;

	tagGuildMember* pGuildMember = pGuild->get_member(GetID());
	if(!VALID_POINT(pGuildMember))
		return;

	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_WAR)
		return;

	if(pKill->IsRole())
	{
		Role* pKillRole = (Role*)pKill;

		if(pKillRole->GetGuildID() != pGuild->get_guild_war().get_war_guild_id())
			return;

		SetWarAchievement(pGuildMember->eGuildPos, pGuild);
	}

	if(pKill->IsCreature())
	{
		Creature* pKillRole = (Creature*)pKill;

		if(pKillRole->GetGuildID() != pGuild->get_guild_war().get_war_guild_id())
			return;

		SetWarAchievement(pGuildMember->eGuildPos, pGuild);
	}
}

//---------------------------------------------------------------------------------
// 帮会战分数设置
//---------------------------------------------------------------------------------
VOID Role::SetWarAchievement(EGuildMemberPos	eGuildPos, guild* pGuild)
{
	if(!VALID_POINT(pGuild))
		return;

	guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
	if(!VALID_POINT(pEnemyGuild))
		return;

	switch(eGuildPos)
	{
	case EGMP_BangZhu:
		{
			pEnemyGuild->get_guild_war().set_achievement(15);
			pGuild->get_guild_war().set_enemy_achievement(15);
			break;
		}
	case EGMP_FuBangZhu:
		{
			pEnemyGuild->get_guild_war().set_achievement(10);
			pGuild->get_guild_war().set_enemy_achievement(10);
			break;
		}
	default:
		{
			pEnemyGuild->get_guild_war().set_achievement(1);
			pGuild->get_guild_war().set_enemy_achievement(1);
			break;
		}
	}
}

//---------------------------------------------------------------------------------
// 死亡惩罚
//---------------------------------------------------------------------------------
VOID Role::DeadPenalty(Unit* pSrc)
{
	// 如果玩家是级别小于强制PK保护的等级，则不受任何死亡惩罚
	if( get_level() < SAFE_GUARD_FORCE_LEVEL ) return;


	// 如果人物在PVP区死亡，则不会受到任何死亡惩罚
	if ( IsInRoleState(ERS_PVPArea) ) return;
	//if (IsInRoleState(ERS_NoPunishArea)) return;

	// 经验惩罚（只有邪恶和通缉状态的玩家才会损失经验）
	//if( ERPKS_Wicked == m_ePKState || ERPKS_Wanted == m_ePKState )
	//{
	//	// 只有被玩家击杀的情况下才会损失经验
	//	if( VALID_POINT(pSrc) && pSrc->IsRole() )
	//	{
	//		// 损失的经验：[(400-角色等级×2)/20000] %×升级经验
	//		const s_level_up_effect* pEffect = AttRes::GetInstance()->GetLevelUpEffect(get_level());
	//		ASSERT( VALID_POINT(pEffect) );
	//		INT nExp = INT((200.0f - FLOAT(get_level())) / 10000.0f * FLOAT(pEffect->n_exp_level_up_));
	//		ExpChange(-nExp);
	//	}
	//}

	//// 装备掉落惩罚，PK保护玩家不受惩罚
	if( !IsInRoleState(ERS_Safeguard) )
	{
		INT nLootProp = 10000;//LOOT_RATE_PER_PK_STATE[m_ePKState];
		if( nLootProp > 0 &&  get_tool()->tool_rand() % 10000 <= nLootProp )
		{
			// 80%的几率掉落1件装备或道具
			// 15%的几率掉落2件装备或道具
			// 4%的几率掉落3件装备或道具
			// 0.8%的几率掉落4件装备或道具
			// 0.2%的几率掉落5件装备或道具
/*			INT n_num = 0;
			INT nProp = get_tool()->tool_rand() % 10000;

			if( nProp < 8000 )			n_num = 1;
			else if( nProp < 9500 )		n_num = 2;
			else if( nProp < 9900 )		n_num = 3;
			else if( nProp < 9980 )		n_num = 4;
			else						n_num = 5;*/

			// 根据个数从背包或者装备栏中掉落物品
			package_list<tagItem*> listLootItems;
			GetItemMgr().LootFromBag(listLootItems, elcid_dead_penalty);

			// 掉落装备栏的装备
			package_list<DWORD> listGemID;
			GetItemMgr().LootFromEquipBar(listLootItems, listGemID, elcid_dead_penalty);

			int r_index = 0;
			// 生成地面物品
			tagItem* pItem = NULL;
			package_list<tagItem*>::list_iter iter = listLootItems.begin();
			while (listLootItems.find_next(iter, pItem))
			{
				if (!VALID_POINT(pItem))
				{
					continue;
				}

				if( get_map() )
				{
					Vector3 vPos;
					if( !g_drop_mgr.get_drop_pos(this, vPos, r_index) )
					{
						::Destroy(pItem);
						continue;
					}


					tag_ground_item* pGroundItem = new tag_ground_item(get_map()->get_ground_item_id(), pItem->dw_data_id, 
						pItem->n16Num, pItem, vPos, INVALID_VALUE, 
						INVALID_VALUE, 0, GetID());

					ASSERT(VALID_POINT(pGroundItem));

					get_map()->add_ground_item(pGroundItem);
					r_index++;
				}
				else
				{
					Destroy(pItem);
				}
			}

			DWORD dwGemID = INVALID_VALUE;
			package_list<DWORD>::list_iter it = listGemID.begin();
			while (listGemID.find_next(it, dwGemID))
			{
				if( get_map() )
				{
					// 生成物品
					tagItem* pItem = ItemCreator::CreateEx(EICM_Loot, GetID(), dwGemID, 1);
					if( !VALID_POINT(pItem) ) 
						continue ;

					Vector3 vPos;
					if( !g_drop_mgr.get_drop_pos(this, vPos, r_index) )
					{
						::Destroy(pItem);
						continue;
					}


					tag_ground_item* pGroundItem = new tag_ground_item(get_map()->get_ground_item_id(), pItem->dw_data_id, 
						pItem->n16Num, pItem, vPos, INVALID_VALUE, 
						INVALID_VALUE, 0, GetID());

					ASSERT(VALID_POINT(pGroundItem));

					get_map()->add_ground_item(pGroundItem);
					r_index++;
				}
			}
		}
	}

	// 内伤惩罚
	// 29%几率不受内伤
	// 30%几率内伤+1
	// 20%几率内伤+2
	// 15%几率内伤+3
	// 5%几率内伤+5
	// 1%几率内伤+10
	//INT nInjury = 0;
	//INT nProp = get_tool()->tool_rand() % 100;

	//if( nProp < 29 )			nInjury = 0;
	//else if( nProp < 59 )		nInjury = 1;
	//else if( nProp < 79 )		nInjury = 2;
	//else if( nProp < 94 )		nInjury = 3;
	//else if( nProp < 99 )		nInjury = 5;
	//else						nInjury = 10;

	////if( nInjury != 0 )
	////	ModAttValue(ERA_Injury, nInjury);

	//// 跑商商银惩罚
	//guild* pGuild = g_guild_manager.get_guild(GetGuildID());
	//if (!VALID_POINT(pGuild))				return;
	//if (!IsInRoleState(ERS_Commerce))	return;
	//guild_commodity* pCommodity = pGuild->get_guild_commerce().get_commodity(GetID());
	//if (!VALID_POINT(pCommodity))			return;
	//pCommodity->dead_penalty();
}

//---------------------------------------------------------------------------------
// 目标的类型标志
//---------------------------------------------------------------------------------
DWORD Role::TargetTypeFlag(Unit* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;

	if( pTarget->IsRole() )
		return TargetTypeFlag(static_cast<Role*>(pTarget));
	else if( pTarget->IsCreature() )
		return TargetTypeFlag(static_cast<Creature*>(pTarget));
	else
		return 0;
}

DWORD Role::TargetTypeFlag(Role* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;

	// 如果目标就是自己，则返回自身
	if( this == pTarget ) return ETF_Self;

	DWORD dwFlag = 0;

	// todo：检测玩家跟自己是不是某些社会关系，如果是，则设置上相关的某些字段

	// 小队队友
	if( IsTeamMate(pTarget) )
	{
		dwFlag |= ETF_Teammate;
	}

	// 当然属于某些社会关系的人也属于玩家
	dwFlag |= ETF_Player;

	return dwFlag;
}

DWORD Role::TargetTypeFlag(Creature* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;

	DWORD dwFlag = 0;

	// 怪物
	if( pTarget->IsMonster() )
	{
		// 普通怪物
		if( pTarget->IsNormalMonster() )
		{
			if( !pTarget->IsBoss() )
				dwFlag |= ETF_NormalMonster;
			else
				dwFlag |= ETF_Boss;
		}

		// 巢穴
		else if( pTarget->IsNest())
		{
			dwFlag |= ETF_Nest;
		}
	}

	// NPC
	else if( pTarget->IsNPC() )
	{
		dwFlag |= ETF_NPC;
	}

	// 宠物
	else if( pTarget->IsPet() )
	{
		dwFlag |= ETF_Pet;
	}

	// 资源
	else if( pTarget->IsRes() )
	{
		if ( pTarget->IsManRes() || pTarget->IsGuildManRes())
			dwFlag |= ETF_ManRes;
		else if ( pTarget->IsNatuRes() || pTarget->IsGuildNatuRes())
			dwFlag |= ETF_NatuRes;
		else
			ASSERT(0);
	}

	// 可调查地物
	else if( pTarget->IsInves() || pTarget->IsInves_two())
	{
		dwFlag |= ETF_InvesGameObject;
	}
	// todo: 门和建筑再看看放到哪个子类里去

	return dwFlag;
}

//---------------------------------------------------------------------------------
// 与目标的敌友中立判断
//---------------------------------------------------------------------------------
DWORD Role::FriendEnemy(Unit* pTarget)
{
	if( !VALID_POINT(pTarget) ) return 0;
	if( !VALID_POINT(get_map()) || get_map() != pTarget->get_map() ) return 0;

	// 自身的话，则均满足
	if( this == pTarget )
	{
		return ETFE_Friendly | ETFE_Hostile | ETFE_Independent;
	}

	// 取所在地图的敌我关系判定
	BOOL bIgnore = FALSE;
	DWORD dwMapFlag = get_map()->get_friend_enemy_flag(this, pTarget, bIgnore);

	// 如果还需要判断单体自身的，则判断自身
	if( !bIgnore )
	{
		DWORD dwSelfFlag = 0;
		if( pTarget->IsRole() )
			dwSelfFlag = FriendEnemy(static_cast<Role*>(pTarget));
		else if( pTarget->IsCreature() )
			dwSelfFlag = FriendEnemy(static_cast<Creature*>(pTarget));
		else
			dwSelfFlag = 0;

		return (dwMapFlag | dwSelfFlag);
	}
	else
	{
		return dwMapFlag;
	}
}


DWORD Role::FriendEnemy(Role* pTarget)
{
	DWORD dwFlag = 0;

	if( !IsInRoleState(ERS_PVPArea) && !pTarget->IsInRoleState(ERS_PVPArea) )
	{
		if(pTarget->get_temp_role_camp() != ECA_Null)
		{
			const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_temp_role_camp());
			if(!VALID_POINT(pCreatureCamp))
				return dwFlag;
			
			switch(pCreatureCamp->eCampConnection[pTarget->get_temp_role_camp()])
			{
			case ECAC_Null:
			case ECAC_Enemy:
				{
					dwFlag |= ETFE_Hostile;
					goto Exit;
				}
				break;
			case ECAC_Friend:
				dwFlag |= ETFE_Friendly;
				break;
			case ECAC_Neutralism:
				dwFlag = ETFE_Independent;
				break;
			}
		}
		else
		{
			const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_role_camp());
			if(!VALID_POINT(pCreatureCamp))
				return dwFlag;

			switch(pCreatureCamp->eCampConnection[pTarget->get_role_camp()])
			{
			case ECAC_Null:
			case ECAC_Enemy:
				{
					dwFlag |= ETFE_Hostile;
					goto Exit;
				}
				break;
			case ECAC_Friend:
				dwFlag |= ETFE_Friendly;
				break;
			case ECAC_Neutralism:
				dwFlag = ETFE_Independent;
				break;
			}
		}
	}
	
	// mwh 2011-08-03 在对方同意切磋时才能设置
	if( VALID_POINT(GetDuelTarget()) && 
		(GetDuelTarget() == pTarget->GetID()) &&
		IsDuelOperate( ) && pTarget->IsDuelOperate( ) )
	{
		dwFlag |=  ETFE_Hostile;
	}

	// 和平模式则非白名为敌人
	//if(GetPKState() == ERolePK_Peace)
	//{
	//	// mwh 2011-08-03 非队友的红名玩家
	//	if( pTarget->HasPKValue( ) && !IsTeamMate(pTarget) )
	//	{
	//		dwFlag |=  ETFE_Hostile;
	//	}
	//	else
	//	{
	//		dwFlag |= ETFE_Friendly;
	//	}
	//}

	if(GetPKState() == ERolePK_Guild)
	{
		if(IsGuildMate(pTarget))
		{
			dwFlag |=  ETFE_Friendly;
			return dwFlag;
		}
		else
		{
			dwFlag |= ETFE_Hostile;
		}
	}

	//if(GetPKState() == ERolePK_Team)
	//{
	//	if(IsTeamMate(pTarget))
	//	{
	//		dwFlag |=  ETFE_Friendly;
	//	}
	//	else
	//	{
	//		dwFlag |= ETFE_Hostile;
	//	}
	//}

	if(GetPKState() == ERolePK_All)
	{
		if(IsTeamMate(pTarget))
		{
			dwFlag |=  ETFE_Friendly;
		}
		else
		{
			dwFlag |= ETFE_Hostile;
		}
	}

	// 先通过源方与目标方之间的关系确定敌我关系
	//if( IsFriendlySocial(pTarget) )
	//{
	//	if( !pTarget->IsInRoleState(ERS_PK) && !this->IsInRoleState(ERS_PK) )
	//	{
	//		// 双方符合某些社会关系，而且双方都不处在行凶状态，是友方
	//		dwFlag |= ETFE_Friendly;
	//	}
	//	else if( pTarget->IsInRoleState(ERS_PK) && this->IsInRoleState(ERS_PK) )
	//	{
	//		// 双方符合某些社会关系，而且双方都处在行凶状态，是友方
	//		dwFlag |= ETFE_Friendly;
	//	}
	//}
	//else
	//{
	//	// 如果自己或者目标设置成为行凶状态，则为敌方
	//	if( IsInRoleState(ERS_PK) || pTarget->IsInRoleState(ERS_PK) )
	//	{
	//		dwFlag |= ETFE_Hostile;
	//	}
	//	// 否则为友方
	//	else
	//	{
	//		dwFlag |= ETFE_Friendly;
	//	}
	//}

	//// 再考虑一下PK保护相关的内容
	//if( pTarget->IsInRoleState(ERS_Safeguard) )
	//{
	//	// 对开了PK保护的玩家不能进行PK
	//	if( dwFlag & ETFE_Hostile )
	//	{
	//		dwFlag ^= ETFE_Hostile;
	//	}
	//}
	//if( IsInRoleState(ERS_Safeguard) )
	//{
	//	// 自己开了PK保护，不能对他人进行PK
	//	if( dwFlag & ETFE_Hostile )
	//	{
	//		dwFlag ^= ETFE_Hostile;
	//	}

	//	// 不能对当前正在PVP状态下的友方玩家进行有益招式
	//	if( (dwFlag & ETFE_Friendly) && pTarget->IsInRoleState(ERS_PVP) )
	//	{
	//		dwFlag ^= ETFE_Friendly;
	//	}
	//}

Exit:

	if(pTarget->IsRedName() /*|| pTarget->IsPurpureName()*/)
	{
		if(dwFlag & ETFE_Friendly) dwFlag ^= ETFE_Friendly;
		dwFlag |= ETFE_Hostile;
	}

	// 安全区
	if( IsInRoleState(ERS_SafeArea) || pTarget->IsInRoleState(ERS_SafeArea))
	{
		//不是红名
// 		if(!(pTarget->GetPKValue() > 0 || this->GetPKValue() > 0) && (dwFlag & ETFE_Hostile))
// 		{
// 			dwFlag ^= ETFE_Hostile;
// 		}
		// mwh - 2011-08-01 安全区内不能战斗
		if(dwFlag & ETFE_Hostile) dwFlag ^= ETFE_Hostile;
	}
	// 绝对安全区
	if( IsInRoleState(ERS_RealSafeArea) || pTarget->IsInRoleState(ERS_RealSafeArea) )
	{
		if( dwFlag & ETFE_Hostile )
		{
			dwFlag ^= ETFE_Hostile;
		}
	}


	// PVP区域
	if( IsInRoleState(ERS_PVPArea) && pTarget->IsInRoleState(ERS_PVPArea) )
	{
		dwFlag |= ETFE_Hostile;
	}

	if(IsTeamMate(pTarget))
	{
		if(dwFlag & ETFE_Hostile) dwFlag ^= ETFE_Hostile;
		dwFlag |= ETFE_Friendly;
	}

	// mwh 2011-08-18 PK保护
	if(pTarget->get_level() < PKLEVELMIN && 
		GetDuelTarget() != pTarget->GetID( ) && !pTarget->is_in_guild_war())
	{
		// dwFlag |= ETFE_Friendly;
		if(dwFlag & ETFE_Hostile) dwFlag ^= ETFE_Hostile;
	}

	/**逻辑优先级最高*/
	if(pTarget->IsInRoleState(ERS_PrisonArea))
		if(dwFlag & ETFE_Hostile) dwFlag ^= ETFE_Hostile;

	return dwFlag;
}

DWORD Role::FriendEnemy(Creature* pCreature)
{
	DWORD dwFlag = 0;

	// 怪物
	if( pCreature->IsMonster() )
	{
		// 宠物的敌我判断
		Role* pMarster = pCreature->GetMarster();
		if (VALID_POINT(pMarster))
		{
			if (pMarster == this)
				return ETFE_Friendly;
			else
				return FriendEnemy(pMarster);
		}
		if(this->GetGuildID() != INVALID_VALUE && pCreature->GetGuildID() != INVALID_VALUE)
		{
			if(this->GetGuildID() == pCreature->GetGuildID())
			{
				dwFlag |= ETFE_Friendly;
			}
			else
			{
				if(get_temp_role_camp() != ECA_Null)
				{
					const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_temp_role_camp());
					if(!VALID_POINT(pCreatureCamp))
						return dwFlag;

					switch(pCreatureCamp->eCampConnection[pCreature->GetProto()->eCamp])
					{
					case ECAC_Null:
					case ECAC_Enemy:
						{
							dwFlag |= ETFE_Hostile;
						}
						break;
					case ECAC_Friend:
						dwFlag |= ETFE_Friendly;
						break;
					case ECAC_Neutralism:
						dwFlag = ETFE_Independent;
						break;
					}
				}
				else
				{
					const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_role_camp());
					if(!VALID_POINT(pCreatureCamp))
						return dwFlag;

					switch(pCreatureCamp->eCampConnection[pCreature->GetProto()->eCamp])
					{
					case ECAC_Null:
					case ECAC_Enemy:
						{
							dwFlag |= ETFE_Hostile;
						}
						break;
					case ECAC_Friend:
						dwFlag |= ETFE_Friendly;
						break;
					case ECAC_Neutralism:
						dwFlag = ETFE_Independent;
						break;
					}
				}
			}
		}
		else
		{
			if(get_temp_role_camp() != ECA_Null)
			{
				const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_temp_role_camp());
				if(!VALID_POINT(pCreatureCamp))
					return dwFlag;

				switch(pCreatureCamp->eCampConnection[pCreature->GetProto()->eCamp])
				{
				case ECAC_Null:
				case ECAC_Enemy:
					{
						dwFlag |= ETFE_Hostile;
					}
					break;
				case ECAC_Friend:
					dwFlag |= ETFE_Friendly;
					break;
				case ECAC_Neutralism:
					dwFlag = ETFE_Independent;
					break;
				}
			}
			else
			{
				const tagCreatureCamp* pCreatureCamp = AttRes::GetInstance()->GetCreatureCamp(get_role_camp());
				if(!VALID_POINT(pCreatureCamp))
					return dwFlag;

				switch(pCreatureCamp->eCampConnection[pCreature->GetProto()->eCamp])
				{
				case ECAC_Null:
				case ECAC_Enemy:
					{
						dwFlag |= ETFE_Hostile;
					}
					break;
				case ECAC_Friend:
					dwFlag |= ETFE_Friendly;
					break;
				case ECAC_Neutralism:
					dwFlag = ETFE_Independent;
					break;
				}
			}
		}
	}

	// NPC
	else if( pCreature->IsNPC() )
	{
		//if( pCreature->IsCanBeAttack() )
		//{
		//	dwFlag |= ETFE_Hostile;
		//}
		//else
		//{
			dwFlag |= ETFE_Friendly;
		//}

		//if( IsInRoleState(ERS_PK) )
		//{
		//	dwFlag |= ETFE_Hostile;
		//}
		//else
		//{
		//	dwFlag |= ETFE_Friendly;
		//}
	}

	// 地物
	else if( pCreature->IsGameObject() )
	{
		if(pCreature->IsGuildInves())
		{
			if(this->GetGuildID() != INVALID_VALUE && pCreature->GetGuildID() != INVALID_VALUE)
			{
				if(this->GetGuildID() != pCreature->GetGuildID())
				{
					dwFlag |= ETFE_Independent;
				}
			}
		}
		else if (pCreature->IsRes() || pCreature->IsInves() || pCreature->IsInves_two())
		{
			dwFlag |= ETFE_Independent;
		}
		else 
		{
			// todo:剩下的一些地物目前先不考虑，属于无法攻击型的
		}
	}



	// 宠物
	else if( pCreature->IsPet() )
	{

	}

	return dwFlag;
}

//------------------------------------------------------------------------------
// 注册触发器技能关联表
//------------------------------------------------------------------------------
VOID Role::RegisterTriggerSkillSet(ETriggerEventType eEvent, DWORD dwSkillID)
{
	EPassiveSkillAndEquipTrigger eTriggerType = TriggerTypeToPassiveSkillAndEquipTriggerType(eEvent);
	if( EPSAET_Null == eTriggerType ) return;

	std::set<DWORD>& skillSet = m_setPassiveSkillTrigger[eTriggerType];

	skillSet.insert(dwSkillID);
}

//----------------------------------------------------------------------------
// 反注册触发器技能关联表
//----------------------------------------------------------------------------
VOID Role::UnRegisterTriggerSkillSet(ETriggerEventType eEvent, DWORD dwSkillID)
{
	EPassiveSkillAndEquipTrigger eTriggerType = TriggerTypeToPassiveSkillAndEquipTriggerType(eEvent);
	if( EPSAET_Null == eTriggerType ) return;

	std::set<DWORD>& skillSet = m_setPassiveSkillTrigger[eTriggerType];

	skillSet.erase(dwSkillID);
}

//----------------------------------------------------------------------------------
// 装备相关buff预处理，并获取tigger类型
//----------------------------------------------------------------------------------
EPassiveSkillAndEquipTrigger Role::PreRegisterTriggerEquipSet(DWORD dwTriggerID, DWORD dwBuffID, BOOL bEquip)
{
	const tagTriggerProto* pProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
	if( !VALID_POINT(pProto) )
	{
		m_att_res_caution(_T("trigger proto"), _T("TriggerID"), dwTriggerID);
		return EPSAET_Null;
	}

	// 是否是装备类trigger
	if( ETEE_Equip == pProto->eEventType )
	{
		ProcEquipBuffTrigger(dwBuffID, bEquip);
		return EPSAET_Null;
	}

	return TriggerTypeToPassiveSkillAndEquipTriggerType(pProto->eEventType);
}

//----------------------------------------------------------------------------
// 被动技能触发器触发
//----------------------------------------------------------------------------
BOOL Role::OnPassiveSkillBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, 
		DWORD dwEventMisc1/* =INVALID_VALUE */, DWORD dwEventMisc2/* =INVALID_VALUE */)
{
	// 首先根据事件类型判断出被动技能对应的触发器类型
	EPassiveSkillAndEquipTrigger eTriggerType = TriggerTypeToPassiveSkillAndEquipTriggerType(eEvent);
	if( EPSAET_Null == eTriggerType ) return FALSE;

	// 找到是否有被动技能对该trigger类型进行了注册
	if( m_setPassiveSkillTrigger[eTriggerType].empty() ) return FALSE;

	// 一个技能一个技能的轮询
	std::set<DWORD>& skillSet = m_setPassiveSkillTrigger[eTriggerType];
	for(std::set<DWORD>::iterator it = skillSet.begin(); it != skillSet.end(); it++)
	{
		// 找到技能
		DWORD dwSkillID = *it;
		Skill* pSkill = GetSkill(dwSkillID);
		if( !VALID_POINT(pSkill) ) continue;

		// todo: 这个地方还要判断被动技能的限制，combathandler里面再加一个判断被动技能能否使用的判断

		BOOL bSelfUseble = FALSE;			// 自身能否作用
		BOOL bTargetUseble = FALSE;			// 目标能否作用

		if( E_Success == m_CombatHandler.CheckTargetLogicLimit(pSkill, pTarget) )
		{
			bTargetUseble = TRUE;
		}
		if( E_Success == m_CombatHandler.CheckTargetLogicLimit(pSkill, this) )
		{
			bSelfUseble = TRUE;
		}

		// 自身和目标都不能作用，则直接返回
		if( FALSE == bSelfUseble && bTargetUseble == FALSE )
			continue;

		// 检查技能是用skillbuff还是用属性表中的buffid
		for(INT n = 0; n < MAX_BUFF_PER_SKILL; n++)
		{
			DWORD dwBuffTypeID = pSkill->GetBuffTypeID(n);
			DWORD dwTriggerID = pSkill->GetTriggerID(n);
			if( !VALID_POINT(dwBuffTypeID) || !VALID_POINT(dwTriggerID) ) continue;

			const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
			const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
			if( !VALID_POINT(pBuffProto) || !VALID_POINT(pTriggerProto) ) continue;

			// 查看trigger类型是否相同，主要是为了优化
			if( ETEE_Null != pTriggerProto->eEventType && eEvent != pTriggerProto->eEventType )
				continue;

			// 如果目标可以作用，则作用于目标
			if( bTargetUseble )
				pTarget->TryAddBuff(this, pBuffProto, pTriggerProto, pSkill, NULL, TRUE, dwEventMisc1, dwEventMisc2);

			// 如果自身可以作用，则作用于自身
			if( bSelfUseble )
				TryAddBuff(this, pBuffProto, pTriggerProto, pSkill, NULL, TRUE, dwEventMisc1, dwEventMisc2);
		}
		m_CombatHandler.CheckInCombat(pTarget);
	}

	return TRUE;
}

//------------------------------------------------------------------------------
// 使用物品状态触发
//------------------------------------------------------------------------------
BOOL Role::OnActiveItemBuffTrigger(Unit* pTarget, tagItem* pItem, ETriggerEventType eEvent, 
								   DWORD dwEventMisc1/*=INVALID_VALUE*/, DWORD dwEventMisc2/*=INVALID_VALUE*/)
{
	// 目标是否存在
	if ( !VALID_POINT( pTarget ) ) return FALSE;

	// 物品是否存在
	if( !VALID_POINT(pItem)) return FALSE;

	for(INT n = 0; n < MAX_BUFF_PER_ITEM; ++n)
	{
		DWORD dwBuffTypeID = pItem->GetBuffID(n);
		DWORD dwTriggerID = pItem->GetTriggerID(n);
		if( !VALID_POINT(dwBuffTypeID) || !VALID_POINT(dwTriggerID) ) continue;

		const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
		const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
		if( !VALID_POINT(pBuffProto) || !VALID_POINT(pTriggerProto) ) continue;

		// 查看trigger类型是否相同，主要是为了优化
		if( ETEE_Null != pTriggerProto->eEventType && eEvent != pTriggerProto->eEventType )
			continue;

		// 给自己是否满足
		if (!pTarget->TryAddBuff( this, pBuffProto, pTriggerProto, NULL, pItem ))
			TryAddBuff( this, pBuffProto, pTriggerProto, NULL, pItem );

	}

	return TRUE;
}
//----------------------------------------------------------------------------------
// 装备相关被动trigger触发处理
//----------------------------------------------------------------------------------
VOID Role::OnEquipmentBuffTriggerCommon(Unit* pTarget, ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType)
{
	// 装备上普通buff处理 -- 轮循装备栏
	BitSetEquipPos& equipSet = m_bitsetEquipTrigger[eTriggerType];
	for(INT16 i=0; i<equipSet.size(); ++i)
	{
		if(!equipSet.test(i))
		{
			continue;
		}

		// 找到装备
		tagEquip *pEquip = GetItemMgr().GetEquipBarEquip(i);
		if(!VALID_POINT(pEquip))
		{
			// 执行到该处，说明装备buff，trigger处理有问题
			ASSERT(0);
			print_message(_T("\n\n\nCaution:\n"));
			print_message(_T("\tTrigger&Buff of equip process maybe have problem! Please tell programmer to check!"));
			continue;
		}

		// 找到装备的buff，尝试添加
		for(INT32 i=0; i<MAX_EQUIP_BUFF_NUM; ++i)
		{
			DWORD	dwBuffTypeID	=	INVALID_VALUE;
			DWORD	dwTriggerID		=	INVALID_VALUE;
			pEquip->GetTriggerIDBuffID(dwTriggerID, dwBuffTypeID, i);
			if(INVALID_VALUE == dwTriggerID || INVALID_VALUE == dwBuffTypeID)
			{
				continue;
			}

			const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(dwTriggerID);
			if(!VALID_POINT(pTriggerProto))
			{
				m_att_res_caution(_T("equip trigger or buff"), _T("equip typeid"), pEquip->dw_data_id);
				continue;
			}

			// 查看trigger类型是否相同，主要是为了优化
			if(ETEE_Null == pTriggerProto->eEventType || eEvent != pTriggerProto->eEventType)
			{
				continue;
			}

			const tagBuffProto*	pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffTypeID);
			if(!VALID_POINT(pBuffProto))
			{
				m_att_res_caution(_T("equip buff"), _T("equip typeid"), pEquip->dw_data_id);
				continue;
			}

			// 给自己是否满足
			if (!TryAddBuff(this, pBuffProto, pTriggerProto, NULL, pEquip))
			{
				if (VALID_POINT(pTarget))
				{
					pTarget->TryAddBuff(this, pBuffProto, pTriggerProto, NULL, pEquip);
				}
			}

		
		}
	}
}

//----------------------------------------------------------------------------------
// 装备相关被动trigger触发处理
//----------------------------------------------------------------------------------
VOID Role::OnEquipmentBuffTriggerSuit(Unit* pTarget, ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType)
{
	SetSuitTrigger& suitSet = m_setSuitTrigger[eTriggerType];

	for(SetSuitTrigger::iterator it = suitSet.begin(); it != suitSet.end(); ++it)
	{
		// 获取套装件数，遍历已激活buff
		INT32 nSuitEquipNum = m_Suit.GetSuitEquipNum(*it);
		ASSERT(nSuitEquipNum >= MIN_SUIT_EQUIP_NUM);

		// 套装原型
		const tagSuitProto *pSuitProto = AttRes::GetInstance()->GetSuitProto(*it);
		if(!VALID_POINT(pSuitProto))
		{
			ASSERT(0);
			m_att_res_caution(_T("suit"), _T("SuitID"), *it);
			continue;
		}

		if(nSuitEquipNum < pSuitProto->n8ActiveNum[0])
		{
			ASSERT(0);
			print_message(_T("\n\n\tSuit trigger reg/unreg maybe have problems! Please check!!!\n\n"));
			continue;
		}
		
		// 套装的触发器次数已经用尽
		if (m_Suit.IsNeedTriggerCount(pSuitProto) && m_Suit.GetSuitTriggerCount(pSuitProto) <= 0)
		{
			continue;
		}

		for(INT32 i=0; i<MAX_SUIT_ATT_NUM; ++i)
		{
			if(nSuitEquipNum < pSuitProto->n8ActiveNum[i])
			{
				break;
			}

			if(INVALID_VALUE == pSuitProto->dwTriggerID[i] || INVALID_VALUE == pSuitProto->dwBuffID[i])
			{
				break;
			}

			const tagTriggerProto* pTriggerProto = AttRes::GetInstance()->GetTriggerProto(pSuitProto->dwTriggerID[i]);
			if(!VALID_POINT(pTriggerProto))
			{
				m_att_res_caution(_T("suit trigger"), _T("suitid"), *it);
				continue;
			}

			// 查看trigger类型是否相同，主要是为了优化
			if(ETEE_Null == pTriggerProto->eEventType || eEvent != pTriggerProto->eEventType)
			{
				continue;
			}

			const tagBuffProto*	pBuffProto = AttRes::GetInstance()->GetBuffProto(pSuitProto->dwBuffID[i]);
			if(!VALID_POINT(pBuffProto))
			{
				m_att_res_caution(_T("suit buff"), _T("suitid"), *it);
				continue;
			}

			// 给自己是否满足
			if (TryAddBuff(this, pBuffProto, pTriggerProto, NULL, NULL))
			{
				// 消耗一次
				m_Suit.ConsumeTriggerCount(pSuitProto);
			}
		}
	}
}

//----------------------------------------------------------------------------
// 装备状态触发
//----------------------------------------------------------------------------
BOOL Role::OnEquipmentBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, 
								  DWORD dwEventMisc1/* =INVALID_VALUE */, DWORD dwEventMisc2/* =INVALID_VALUE */)
{
	// 首先根据事件类型判断出被动技能对应的触发器类型
	EPassiveSkillAndEquipTrigger eTriggerType = TriggerTypeToPassiveSkillAndEquipTriggerType(eEvent);
	if( EPSAET_Null == eTriggerType ) return FALSE;

	// 装备上普通buff处理
	if( m_bitsetEquipTrigger[eTriggerType].any() )
	{
		OnEquipmentBuffTriggerCommon(pTarget, eEvent, eTriggerType);
	}
	// 套装
	if(m_setSuitTrigger[eTriggerType].size() > 0)
	{
		OnEquipmentBuffTriggerSuit(pTarget, eEvent, eTriggerType);
	}

	return TRUE;
}

BOOL Role::OnPetBuffTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1/*=INVALID_VALUE*/, DWORD dwEventMisc2/*=INVALID_VALUE*/)
{
	// 首先根据事件类型判断出被动技能对应的触发器类型
	EPassiveSkillAndEquipTrigger eTriggerType = TriggerTypeToPassiveSkillAndEquipTriggerType(eEvent);
	if( EPSAET_Null == eTriggerType ) return FALSE;

	PetPocket* pPetPock = GetPetPocket();
	if(!VALID_POINT(pPetPock))
		return FALSE;

	// 触发宠物buff
	PetSoul* pPetSoul = GetPetPocket()->GetCalledPetSoul();
	if(!VALID_POINT(pPetSoul))
		return FALSE;
	
	pPetSoul->OnPetSkillBuffTrigger(eEvent, eTriggerType);

	return TRUE;
}

//-----------------------------------------------------------------------------
// 检查地图区域
//-----------------------------------------------------------------------------
VOID Role::CheckMapArea()
{
	INT64 areaState = 0;
	if( get_map() )
		areaState = get_map()->check_area(this);

#define DEAL_ROLE_STATE(state)\
	if(areaState & state){\
		if( !IsInRoleState(state) ) SetRoleState(state);\
	} else {\
		if( IsInRoleState(state) ) UnsetRoleState(state);\
	}

	// 安全区
	DEAL_ROLE_STATE(ERS_SafeArea);
	// PVP区
	DEAL_ROLE_STATE(ERS_PVPArea);
	// 摆摊区
	DEAL_ROLE_STATE(ERS_StallArea);
	// 监狱区
	DEAL_ROLE_STATE(ERS_PrisonArea);
	// 普通区
	DEAL_ROLE_STATE(ERS_CommonArea);
	// 绝对安全区
	DEAL_ROLE_STATE(ERS_RealSafeArea);
	// 帮会战区
	DEAL_ROLE_STATE(ERS_GuildBattle);
	// 无惩罚区
	DEAL_ROLE_STATE(ERS_NoPunishArea);
	// 神悟
	//DEAL_ROLE_STATE(ERS_ComprehendArea); 
	// 跳舞
	DEAL_ROLE_STATE(ERS_DancingArea);
	// 练武
	DEAL_ROLE_STATE(ERS_KongFuArea);
}

//------------------------------------------------------------------------------
// 复活
//------------------------------------------------------------------------------
DWORD Role::Revive(ERoleReviveType eType, INT64 n64ItemID, BOOL bNeedItem)
{
	if( !IsInState(ES_Dead) )
		return E_Revive_NotDead;	// 已经不是死亡状态了

	DWORD dw_error_code = E_Success;
	
	// 判断复活类型
	switch(eType)
	{
	case ERRT_Normal:
		{
			//if(IsRedName( ) /*|| IsPurpureName( )*/ )
			//{
				//dw_error_code = YamunRevive( );
				//if(dw_error_code != E_Success) 
					dw_error_code = NormalRevive();
			//}
			//else
			//{
			//	dw_error_code = NormalRevive();
			//}
			break;
		}
	case ERRT_Perfection:
		{
			dw_error_code = PerfectionRevive();
			break;
		}
	case ERRT_ReturnCity:
		{
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(GetMapID());
			if(pMapInfo->e_type == EMT_Guild) // 如果是家族副本地图
			{
				dw_error_code = GuildRevive();
				if(dw_error_code != E_Success)
				{
					dw_error_code = CityRevive();
				}
			}
			// 战场复活
			if (pMapInfo->e_type == EMT_Battle)
			{
				dw_error_code = BattleRevive();
				if(dw_error_code != E_Success)
				{
					dw_error_code = CityRevive();
				}
			}
			else if( IsRedName( ) /*|| IsPurpureName( )*/ )
			{
				dw_error_code = YamunRevive( );
				if( dw_error_code != E_Success ) 
					dw_error_code = CityRevive( );
			}
			else dw_error_code = CityRevive();
		}
		break;
	case ERRT_Locus:
		dw_error_code = LocusRevive(n64ItemID, bNeedItem);
		break;
	case ERRT_Accept:
		dw_error_code = AcceptRevive();
		break;
	case ERRT_Prison:
		dw_error_code = PrisonRevive();
		break;
	case ERRT_Instance:
		dw_error_code = InstRevive();
		break;
	case ERRT_Yamun:
		dw_error_code = YamunRevive( );
		if( dw_error_code != E_Success ) 
			dw_error_code = NormalRevive();
		break;
	case ERRT_GuildWar:
		dw_error_code = GuildWarRevive();
		break;
	default:
		// 不会执行到这里(上层已做类型判断)
		ASSERT(0);
		dw_error_code = E_Revive_Unknown;
	}

	get_map()->on_revive(this, eType, m_Revive.nHP, m_Revive.nMP, m_Revive.fX, m_Revive.fY, m_Revive.fZ, m_Revive.dwMapID);

	// 复活
	if(E_Success == dw_error_code)
	{
		SetAttValue(ERA_HP, m_Revive.nHP);
		SetAttValue(ERA_MP, m_Revive.nMP);
		UnsetState(ES_Dead);
//		mwh 2011-08-03 取消红名无敌BUFF 
// 		//添加一个无敌BUFF
// 		if( IsRedName( ) ) 
// 			TryAddBuff( this, AttRes::GetInstance()->GetBuffProto(INVINCIBLE_BUFF_ID), NULL, NULL, NULL);
	}

	// 清空复活相关记录信息
	m_Revive.nHP = 0;
	m_Revive.nMP = 0;

	return dw_error_code;
}

BOOL Role::IsInNormalMap()
{
	Map* p_map = get_map( );
	if( !VALID_POINT(p_map) ) return FALSE;
	if( !VALID_POINT(p_map->get_map_info( )) )return FALSE;
	
	if( p_map->get_map_info( )->e_type == EMT_Normal ||
		p_map->get_map_info( )->e_type == EMT_System)
		return TRUE;

	return FALSE;
}

DWORD Role::NormalRevive()
{
	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	if(pMapInfo->e_type == EMT_Normal)
	{
		m_Revive.fX = pMapInfo->v_reborn_positon.x;
		m_Revive.fY = pMapInfo->v_reborn_positon.y;
		m_Revive.fZ = pMapInfo->v_reborn_positon.z;
		m_Revive.dwMapID = GetMapID();
	}
	else
	{
		const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
		if(!VALID_POINT(pInstance))
		{
			return E_Revive_MapNotFound;
		}

		pMapInfo = g_mapCreator.get_map_info(pInstance->dwExportMapID);
		if(!VALID_POINT(pMapInfo))
		{
			ASSERT(VALID_POINT(pMapInfo));
			return E_Revive_MapNotFound;
		}

		const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pInstance->dwExportWayPoint);
		if( !VALID_POINT(pList) ) return E_Revive_MapNotFound;

		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		pList->list.rand_find(wayPoint);

		m_Revive.fX = wayPoint.v_pos.x;
		m_Revive.fY = wayPoint.v_pos.y;
		m_Revive.fZ = wayPoint.v_pos.z;
		m_Revive.dwMapID = pInstance->dwExportMapID;
	}

	//ChangeBrotherhood(-get_level()*50);

	//INT32 nID = m_eClass*1000 + m_nLevel;
	//const s_level_up_effect* pEffect = AttRes::GetInstance()->GetLevelUpEffect(nID);
	//ASSERT(VALID_POINT(pEffect));

	//ExpChange(-pEffect->n_exp_level_up_*0.01);

	m_Revive.nHP = GetAttValue(ERA_MaxHP)/2;
	SetReviveMp(2);
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 3, 1);

	return E_Success;

}

DWORD Role::CityRevive()
{
	//INT64 n_Money = FormulaHelper::GetRoleCityReviveSivler(get_level());

	//if(GetCurMgr().GetBagAllSilver() < n_Money)
	//	return E_Revive_BindMoney_Not_Enough;

	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	//INT n_mul = 1;

	//if(pMapInfo->e_normal_map_type == ENMT_Dragon)
	//	n_mul = 2;

	//n_Money *= n_mul;
	

	if (g_mapCreator.is_sbk_map(GetMapID()))
	{
		// 沙巴克行会在本地图复活点复活
		if(g_guild_manager.get_SBK_guild() != INVALID_VALUE &&
			g_guild_manager.get_SBK_guild() == GetGuildID())
		{
			// 得到目标地图的导航点
			const tag_map_info* pMapInfo = g_mapCreator.get_map_info(get_tool()->crc32(szGuildMapName));
			if( !VALID_POINT(pMapInfo) ) return E_Revive_MapNotFound;

			const tag_map_way_point_info_list* pList = NULL;
			pList = pMapInfo->map_way_point_list.find(get_tool()->crc32(_T("sbk1")));
			if( !VALID_POINT(pList) ) return E_Revive_MapNotFound;


			// 从目标导航点列表中任取一个导航点
			tag_way_point_info wayPoint;
			pList->list.rand_find(wayPoint);

			m_Revive.fX = wayPoint.v_pos.x;
			m_Revive.fY = 0;
			m_Revive.fZ = wayPoint.v_pos.z;
			m_Revive.dwMapID = pMapInfo->dw_id;
		}
		else// 其他行会的复活点
		{
			m_Revive.fX = pMapInfo->v_out_map_position.x;
			m_Revive.fY = pMapInfo->v_out_map_position.y;
			m_Revive.fZ = pMapInfo->v_out_map_position.z;
			m_Revive.dwMapID = pMapInfo->dw_out_map_id;
		}
	}
	else if(pMapInfo->e_type == EMT_Normal ||pMapInfo->e_type == EMT_System)
	{
		m_Revive.fX = pMapInfo->v_out_map_position.x;
		m_Revive.fY = pMapInfo->v_out_map_position.y;
		m_Revive.fZ = pMapInfo->v_out_map_position.z;
		m_Revive.dwMapID = pMapInfo->dw_out_map_id;
	}
	else
	{
		const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
		if(!VALID_POINT(pInstance))
		{
			return E_Revive_MapNotFound;
		}

		pMapInfo = g_mapCreator.get_map_info(pInstance->dwExportMapID);
		if(!VALID_POINT(pMapInfo))
		{
			ASSERT(VALID_POINT(pMapInfo));
			return E_Revive_MapNotFound;
		}

		const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pInstance->dwExportWayPoint);
		if( !VALID_POINT(pList) ) return E_Revive_MapNotFound;

		// 从目标导航点列表中任取一个导航点
		tag_way_point_info wayPoint;
		pList->list.rand_find(wayPoint);

		m_Revive.fX = wayPoint.v_pos.x;
		m_Revive.fY = wayPoint.v_pos.y;
		m_Revive.fZ = wayPoint.v_pos.z;
		m_Revive.dwMapID = pInstance->dwExportMapID;
	}

	//GetCurMgr().DecBagSilverEx(n_Money, elcid_revive_return_city, GetID());

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 2, 1);

	return E_Success;
}

DWORD Role::InstRevive()
{
	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	if(pMapInfo->e_type != EMT_Instance)
		return E_Revive_MapNotFound;

	map_instance_normal* pMap = (map_instance_normal*)get_map();
	if(!VALID_POINT(pMap))
		return E_Revive_MapNotFound;

	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
	if(!VALID_POINT(pInstance))
		return E_Revive_MapNotFound;

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP)*0.1;
	SetReviveMp();

	const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pInstance->dwExportWayPoint);
	if( !VALID_POINT(pList) ) return E_Revive_MapNotFound;;

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);
	
	m_Revive.fX = wayPoint.v_pos.x;
	m_Revive.fY = wayPoint.v_pos.y;
	m_Revive.fZ = wayPoint.v_pos.z;
	m_Revive.dwMapID = pInstance->dwExportMapID;

	return E_Success;
}

VOID  Role::SetReviveMp(INT n)
{
	
	m_Revive.nMP = GetAttValue(ERA_MaxMP)/n;

}
DWORD Role::GuildRevive()
{
	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	map_instance_guild* pMap = (map_instance_guild*)get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	Vector3 vRevive;
	const tag_map_way_point_info_list* pList = NULL;
	if(GetGuildID() != pMap->get_guild_id())
	{
		// 得到目标地图的导航点
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnemyEnterPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}
	else
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	vRevive = wayPoint.v_pos;
	
	FLOAT fDistSQ = 0.0f;
	FLOAT fTmpDistSQ = 0.0f;
	for(INT i = 0; i < MAX_GUILD_RELIVE_NUM; i++)
	{
		DWORD dwCreatureID = pGuild->get_guild_war().get_relive_id(i);
		if(dwCreatureID != INVALID_VALUE)
		{
			Creature* pCreature = pMap->find_creature(dwCreatureID);
			if(!VALID_POINT(pCreature))
				continue;

			fTmpDistSQ = Vec3Dist(GetCurPos(), pCreature->GetCurPos());

			if(fTmpDistSQ < fDistSQ)
			{
				fDistSQ = fTmpDistSQ;
				vRevive = pCreature->GetCurPos();
			}
		}
	}

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	m_Revive.fX = vRevive.x + 1000;
	m_Revive.fY = vRevive.y;
	m_Revive.fZ = vRevive.z - 1000;
	m_Revive.dwMapID = GetMapID();

	return E_Success;
}

DWORD Role::BattleRevive()
{
	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	map_instance_guild* pMap = (map_instance_guild*)get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;


	Vector3 vRevive;
	const tag_map_way_point_info_list* pList = NULL;
	if(get_temp_role_camp() == ECA_Battle_a)
	{
		// 得到目标地图的导航点
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnemyEnterPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}
	else
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}

	// 从目标导航点列表中任取一个导航点
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	vRevive = wayPoint.v_pos;

	
	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	m_Revive.fX = vRevive.x + 1000;
	m_Revive.fY = vRevive.y;
	m_Revive.fZ = vRevive.z - 1000;
	m_Revive.dwMapID = GetMapID();

	return E_Success;
}
DWORD Role::LocusRevive(INT64 n64ItemID, BOOL bNeedItem)
{
	// 获得物品
	//if (bNeedItem)
	//{
	//	tagItem *pItem = GetItemMgr().GetBagItem(n64ItemID);
	//	if(!VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EISF_RevieveItem)
	//	{
	//		return E_Revive_ItemLimit;
	//	}
	//
	//	// 删除物品
	//	DWORD dw_error_code = GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_revive_locus);
	//	if(dw_error_code != E_Success)
	//	{
	//		return dw_error_code;
	//	}

	//	// 设置复活点及血量和真气量
	//	m_Revive.nHP = GetAttValue(ERA_MaxHP) / 4;
	//	SetReviveMp();
	//}
	//else
	//{
	//	// 设置复活点及血量和真气量
	//	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	//	SetReviveMp();
	//}

	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	INT n_mul = 1;

	if(pMapInfo->e_normal_map_type == ENMT_Dragon)
		n_mul = 2;

	INT32 n_Money = FormulaHelper::GetRoleLocusReviveSivler(get_level());
	n_Money *= n_mul;

	if(GetCurMgr().GetBagSilver() < n_Money)
		return E_Revive_Money_Not_Enough;

	GetCurMgr().DecBagSilver(n_Money, elcid_revive_locus, GetID());
	
	m_Revive.nHP = GetAttValue(ERA_MaxHP)/2;
	SetReviveMp(2);

	m_Revive.fX = m_MoveData.m_vPos.x;
	m_Revive.fY = m_MoveData.m_vPos.y;
	m_Revive.fZ = m_MoveData.m_vPos.z;
	m_Revive.dwMapID = GetMapID();
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 1, 1);

	return E_Success;
}

DWORD Role::AcceptRevive()
{
	// 检查HP
	if(m_Revive.nHP <= 0)
	{
		return INVALID_VALUE;
	}
	
	return E_Success;
}

DWORD Role::PrisonRevive()
{
	DWORD	dwPrisonMapID	= g_mapCreator.get_prison_map_id();
	Vector3	PutInPoint		= g_mapCreator.get_putin_prison_point();

	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(dwPrisonMapID);
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	m_Revive.fX = PutInPoint.x;
	m_Revive.fY = PutInPoint.y;
	m_Revive.fZ = PutInPoint.z;
	m_Revive.dwMapID = dwPrisonMapID;

	// mwh 2011-09-06 监狱复活才广播
	if(GetPKValue()>= PK_VALUE_MAX)
		HearSayHelper::SendMessage(EHST_PKVALUE, GetID(), PK_VALUE_MAX);

	GetAchievementMgr().UpdateAchievementCriteria(ete_into_prison, 1);

	return E_Success;
}
//衙门复活
DWORD Role::YamunRevive()
{
	DWORD	dwYamunMapID	= g_mapCreator.get_yamun_map_id();
	Vector3	PutInPoint		= g_mapCreator.get_putin_yamun_point();

	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(dwYamunMapID);
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		return E_Revive_MapNotFound;
	}

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	m_Revive.fX = PutInPoint.x;
	m_Revive.fY = PutInPoint.y;
	m_Revive.fZ = PutInPoint.z;
	m_Revive.dwMapID = dwYamunMapID;

	return E_Success;
}

// 完美复活
DWORD Role::PerfectionRevive()
{
	// 沙巴克和皇宫不能原地复活
	if (g_mapCreator.is_sbk_map(GetMapID()))
	{
		return E_Revive_Not_guild_war;
	}

	int nNeedNumber = 1;//GetDayClearData(ERDCT_WANMEI_REBORN);
	DWORD rect = GetItemMgr().RemoveFromRole(1400043, nNeedNumber, elcid_revive_perfection);
	if (rect != E_Success)
	{
		INT32 n32_yuanbao = nNeedNumber * 10;
		if (GetCurMgr().GetBagYuanBao() >= n32_yuanbao)
		{
			GetCurMgr().DecBagYuanBao(n32_yuanbao, elcid_revive_perfection);
		}
		else if(GetCurMgr().GetBaiBaoYuanBao() >= n32_yuanbao)
		{
			GetCurMgr().DecBaiBaoYuanBao(n32_yuanbao, elcid_revive_perfection);
		}
		else
		{
			return E_Revive_Yuanbao_Not_Enough;
		}
	}

	//INT32 n32_yuanbao = FormulaHelper::GetRolePerfectionReviveYuanbao(get_level());

	//if(GetCurMgr().GetBaiBaoYuanBao() < n32_yuanbao)
	//	return E_Revive_Yuanbao_Not_Enough;

	//GetCurMgr().DecBaiBaoYuanBao(n32_yuanbao, elcid_revive_perfection);

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	m_Revive.nMP = GetAttValue(ERA_MaxMP);
	SetReviveMp();
	m_Revive.fX = GetCurPos().x;
	m_Revive.fY = GetCurPos().y;
	m_Revive.fZ = GetCurPos().z;
	m_Revive.dwMapID = GetMapID();
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 0, 1);

	ModRoleDayClearDate(ERDCT_WANMEI_REBORN);

	return E_Success;
}

DWORD Role::CoolDownRevive()
{
	// 设置复活点及血量和真气量
	NET_SIS_role_revive send;
	send.dw_role_id		= GetID();
	send.dw_error_code = E_Success;

	const tag_map_info *pMapInfo = g_mapCreator.get_map_info(GetMapID());
	if(!VALID_POINT(pMapInfo))
	{
		ASSERT(VALID_POINT(pMapInfo));
		send.dw_error_code =  E_Revive_MapNotFound;
		goto end_cooldown;
	}
	if(pMapInfo->e_type == EMT_Normal)
	{
		m_Revive.fX = pMapInfo->v_reborn_positon.x;
		m_Revive.fY = pMapInfo->v_reborn_positon.y;
		m_Revive.fZ = pMapInfo->v_reborn_positon.z;
	}
	else
	{
		const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(GetMapID());
		if(!VALID_POINT(pInstance))
		{
			send.dw_error_code =  E_Revive_MapNotFound;
			goto end_cooldown;
		}

		const tag_map_way_point_info_list* pList = pMapInfo->map_way_point_list.find(pInstance->coolDownReviveID);
		if( !VALID_POINT(pList) )
		{
			send.dw_error_code =  E_Revive_MapNotFound;
			goto end_cooldown;
		}

		if( VALID_POINT(pList) && pList->list.size() > 0 )
		{
			tag_way_point_info info = pList->list.front();
			m_Revive.fX = info.v_pos.x;
			m_Revive.fY = info.v_pos.y;
			m_Revive.fZ = info.v_pos.z;
			
		} else {
			send.dw_error_code =  E_Revive_MapNotFound;
			goto end_cooldown;
		}
	}
	m_Revive.dwMapID = GetMapID();
	m_Revive.nHP = GetAttValue(ERA_MaxHP)/2;
	SetReviveMp(2);

end_cooldown:

	if(send.dw_error_code == E_Success){

		GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 4, 1);
		get_map()->on_revive(this, ERRT_CoolDown, m_Revive.nHP, m_Revive.nMP, m_Revive.fX, m_Revive.fY, m_Revive.fZ, m_Revive.dwMapID);
		SetAttValue(ERA_HP, m_Revive.nHP);
		UnsetState(ES_Dead);
		GotoNewMap(m_Revive.dwMapID,  m_Revive.fX, m_Revive.fY, m_Revive.fZ);
		// 清空复活相关记录信息
		m_Revive.nHP = 0;m_Revive.nMP = 0;
	}

	if( VALID_POINT(get_map()) )
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}


	return send.dw_error_code;
}

DWORD Role::GuildWarRevive()
{
	if (!is_in_guild_war())
		return E_Revive_Not_guild_war;

	guild* pGuild = g_guild_manager.get_guild(GetGuildID());
	if (!VALID_POINT(pGuild))
		return E_Revive_Not_guild_war;

	if (pGuild->get_guild_war().getRebornTime() <= 0)
		return E_Revive_Time_not;

	pGuild->get_guild_war().decRebornTime();
	pGuild->get_guild_war().send_reborn_time_to_guild();

	// 设置复活点及血量和真气量
	m_Revive.nHP = GetAttValue(ERA_MaxHP);
	SetReviveMp();
	m_Revive.fX = GetCurPos().x;
	m_Revive.fY = GetCurPos().y;
	m_Revive.fZ = GetCurPos().z;
	m_Revive.dwMapID = GetMapID();

	//if (VALID_POINT(m_pScript))
	//{
	//	m_pScript->OnLocusRevive(this, get_map()->get_map_id(), get_map()->get_instance_id());
	//}

	GetAchievementMgr().UpdateAchievementCriteria(eta_roborn, 0, 1);

	return E_Success;
}
//-----------------------------------------------------------------------------------------
// 被复活
//-----------------------------------------------------------------------------------------
VOID Role::BeRevived(INT nHP, INT nMP, Unit* pSrc)
{
	if( nHP <= 0 ) return;

	m_Revive.nHP = nHP;
	m_Revive.nMP = nMP;

	if( VALID_POINT(pSrc) && pSrc->GetMapID() == GetMapID() )
	{
		m_Revive.dwMapID = GetMapID();
		m_Revive.fX = pSrc->GetCurPos().x;
		m_Revive.fY = pSrc->GetCurPos().y;
		m_Revive.fZ = pSrc->GetCurPos().z;
	}
	else
	{
		m_Revive.dwMapID = GetMapID();
		m_Revive.fX = GetCurPos().x;
		m_Revive.fY = GetCurPos().y;
		m_Revive.fZ = GetCurPos().z;
	}


	// 发送给客户端
	NET_SIS_role_revive_notify send;
	SendMessage(&send, send.dw_size);
}

DWORD Role::SendCloseStall()
{
	if(!IsInRoleStateAny(ERS_Stall | ERS_StallSet))
	{
		return E_Success;
	}
	
	NET_SIS_stall_close send;
	send.dw_error_code	= CloseStall();
	send.dwStallRoleID	= GetID();
	if(VALID_POINT(get_map()))
	{
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
	}

	return send.dw_error_code;
}

DWORD Role::UpdateFriOnline()
{
	INT				nFriendNum = GetFriendCount();
	DWORD			dwFriendID = INVALID_VALUE;
	s_role_info*	pRoleInfo = (s_role_info*)INVALID_VALUE;
	tagFriend*		pFriend = (tagFriend*)INVALID_VALUE;
	INT				nIndex = 0;
	DWORD			dw_size = sizeof(NET_SIS_update_friend_state);
		
	if( nFriendNum > 0 )
	{
		dw_size += (nFriendNum-1) * sizeof(tagFriUpdate);
	}
	
	CREATE_MSG(pSend, dw_size, NET_SIS_update_friend_state);
		
	m_mapFriend.reset_iterator();
	while(m_mapFriend.find_next(dwFriendID, pFriend))
	{
		pRoleInfo = g_roleMgr.get_role_info(dwFriendID);
		
		if(VALID_POINT(pRoleInfo))
		{
			pSend->FriUpdate[nIndex].dw_role_id = dwFriendID;
			pSend->FriUpdate[nIndex].bOnline = pRoleInfo->b_online_;
			pSend->FriUpdate[nIndex].nLevel = pRoleInfo->by_level;
			++nIndex;
		}
		else
		{	
			NET_SIS_role_cancel_friend send;
			send.dwDestRoleID = dwFriendID;
			send.byGroup = pFriend->byGroup;
			send.dw_error_code = E_Success;
			SendMessage(&send, send.dw_size);

			pFriend->dwFriendID = INVALID_VALUE;
			pFriend->dwFriVal = 0;
			pFriend->byGroup = 1;
			m_mapFriend.erase(dwFriendID);
		}
	}

	pSend->n_num = nIndex;
	if( nIndex > 0) 
	{
		pSend->dw_size = (nIndex - 1) * sizeof(tagFriUpdate) + sizeof(NET_SIS_update_friend_state);
	}
	else
		pSend->dw_size = sizeof(NET_SIS_update_friend_state);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return 0;
}
//--------------------------------------------------------------------------------
// 通过一级属性计算二级属性
//--------------------------------------------------------------------------------
VOID Role::AccountAtt(ERoleAttribute eRAtt)
{
	const s_role_att_change* pAttChange1 = AttRes::GetInstance()->GetRoleAttChange(m_eClass, 1);
	const s_role_att_change* pAttChange2 = AttRes::GetInstance()->GetRoleAttChange(m_eClass, 2);
	const s_role_att_change* pAttChange3 = AttRes::GetInstance()->GetRoleAttChange(m_eClass, 3);
	const s_role_att_change* pAttChange4 = AttRes::GetInstance()->GetRoleAttChange(m_eClass, 4);
	
	ASSERT( VALID_POINT(pAttChange1) && VALID_POINT(pAttChange2) && VALID_POINT(pAttChange3) && VALID_POINT(pAttChange4));

	e_role_att_to_change erac = ERA2ERAC(eRAtt);
	if (erac == erac_null) return;

	m_nAtt[eRAtt] = pAttChange1->n_att_change_[erac] * m_nAtt[ERA_Physique] + 
		pAttChange2->n_att_change_[erac] * m_nAtt[ERA_Strength] + 
		pAttChange3->n_att_change_[erac] * m_nAtt[ERA_InnerForce] + 
		pAttChange4->n_att_change_[erac] * m_nAtt[ERA_Agility] + m_nBaseAtt[eRAtt];

	m_nAtt[eRAtt] = CalAttMod(m_nAtt[eRAtt], eRAtt);
	m_bAttRecalFlag[eRAtt] = true;


	//switch(eRAtt)
	//{
	//	// 最大体力
	//case ERA_MaxHP:
	//	{
	//		m_nAtt[ERA_MaxHP] = m_nAtt[ERA_Physique] * 10 + m_nBaseAtt[ERA_MaxHP];
	//		m_nAtt[ERA_MaxHP] = CalAttMod(m_nAtt[ERA_MaxHP], ERA_MaxHP);
	//		m_bAttRecalFlag[ERA_MaxHP] = true;
	//	}
	//	break;

	//	// 最大法力
	//case ERA_MaxMP:
	//	{
	//		if (m_eClass == EV_Astrologer) //药师 法力= 智力*15
	//		{
	//			m_nAtt[ERA_MaxMP] = m_nAtt[ERA_InnerForce] * 15 + m_nBaseAtt[ERA_MaxMP];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_MaxMP] = m_nBaseAtt[ERA_MaxMP];
	//		}
	//		m_nAtt[ERA_MaxMP] = CalAttMod(m_nAtt[ERA_MaxMP], ERA_MaxMP);
	//		m_bAttRecalFlag[ERA_MaxMP] = true;
	//	}
	//	break;

	//	// 攻击
	//case ERA_ExAttack:
	//	{
	//		switch (m_eClass)
	//		{
	//		case EV_Warrior://猛将 攻击 = 力量*2
	//			m_nAtt[ERA_ExAttack] = m_nAtt[ERA_Strength]*2;
	//			break;
	//		case EV_Mage://少保 攻击 = 力量+敏捷
	//			m_nAtt[ERA_ExAttack] = m_nAtt[ERA_Strength]+m_nAtt[ERA_Agility];
	//			break;
	//		case EV_Hunter://游侠 攻击 = 敏捷+智力
	//			m_nAtt[ERA_ExAttack] = m_nAtt[ERA_Agility]+m_nAtt[ERA_InnerForce];
	//			break;
	//		case EV_Blader://刺客 攻击 = 敏捷*2
	//			m_nAtt[ERA_ExAttack] = m_nAtt[ERA_Agility]*2;
	//			break;
	//		case EV_Astrologer://药师 攻击 = 智力*2
	//			m_nAtt[ERA_ExAttack] = m_nAtt[ERA_InnerForce]*2;
	//			break;
	//		default:
	//			m_nAtt[ERA_ExAttack] = 0;
	//			break;
	//		}
	//		m_nAtt[ERA_ExAttack] += m_nBaseAtt[ERA_ExAttack];
	//		m_nAtt[ERA_ExAttack] = CalAttMod(m_nAtt[ERA_ExAttack], ERA_ExAttack);
	//		m_bAttRecalFlag[ERA_ExAttack] = true;
	//	}
	//	break;

	//	// 防御
	//case ERA_ExDefense:
	//	{
	//		if (m_eClass == EV_Warrior)//猛将 防御 = 力量
	//		{
	//			m_nAtt[ERA_ExDefense] = m_nAtt[ERA_Strength];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_ExDefense] = 0;
	//		}
	//		m_nAtt[ERA_ExDefense] += m_nBaseAtt[ERA_ExDefense];
	//		m_nAtt[ERA_ExDefense] = CalAttMod(m_nAtt[ERA_ExDefense], ERA_ExDefense);
	//		m_bAttRecalFlag[ERA_ExDefense] = true;
	//	}
	//	break;
	//	//命中
	//case ERA_HitRate:
	//	{
	//		switch(m_eClass)
	//		{
	//			//猛将 少保 游侠 刺客 命中 = 敏捷
	//			case EV_Warrior:
	//			case EV_Mage:
	//			case EV_Hunter:
	//			case EV_Blader:
	//				m_nAtt[ERA_HitRate]	= m_nAtt[ERA_Agility] + m_nBaseAtt[ERA_HitRate];
	//				break;
	//			default:
	//				m_nAtt[ERA_HitRate] = m_nBaseAtt[ERA_HitRate];
	//				break;
	//		}
	//		m_nAtt[ERA_HitRate] = CalAttMod(m_nAtt[ERA_HitRate], ERA_HitRate);
	//		m_bAttRecalFlag[ERA_HitRate] = true;
	//	}
	//	break;
	//	//闪避
	//case ERA_Dodge:
	//	{
	//		switch(m_eClass)
	//		{
	//			case EV_Blader:	//刺客 闪避 = 智力
	//				m_nAtt[ERA_Dodge]	= m_nAtt[ERA_InnerForce] + m_nBaseAtt[ERA_Dodge];
	//				break;
	//			case EV_Astrologer: //药师 闪避=敏捷
	//				m_nAtt[ERA_Dodge]	= m_nAtt[ERA_Agility] + m_nBaseAtt[ERA_Dodge];
	//				break;
	//			default:
	//				m_nAtt[ERA_Dodge] = m_nBaseAtt[ERA_Dodge];
	//				break;
	//		}
	//		m_nAtt[ERA_Dodge] = CalAttMod(m_nAtt[ERA_Dodge], ERA_Dodge);
	//		m_bAttRecalFlag[ERA_Dodge] = true;
	//	}
	//	break;
	//	//招架
	//case ERA_Block_Rate:
	//	{
	//		switch(m_eClass)
	//		{
	//		case EV_Warrior:	
	//		case EV_Mage: //猛将 少保 招架=力量
	//			m_nAtt[ERA_Block_Rate]	= m_nAtt[ERA_Strength] + m_nBaseAtt[ERA_Block_Rate];
	//			break;
	//		default:
	//			m_nAtt[ERA_Block_Rate] = m_nBaseAtt[ERA_Block_Rate];
	//			break;
	//		}
	//		m_nAtt[ERA_Block_Rate] = CalAttMod(m_nAtt[ERA_Block_Rate], ERA_Block_Rate);
	//		m_bAttRecalFlag[ERA_Block_Rate] = true;
	//	}
	//	break;
	//	//暴击
	//case ERA_Crit_Rate:
	//	{
	//		switch(m_eClass)
	//		{
	//		case EV_Hunter:	
	//		case EV_Blader: //游侠 刺客 暴击=力量
	//			m_nAtt[ERA_Crit_Rate]	= m_nAtt[ERA_Strength] + m_nBaseAtt[ERA_Crit_Rate];
	//			break;
	//		default:
	//			m_nAtt[ERA_Crit_Rate] = m_nBaseAtt[ERA_Crit_Rate];
	//			break;
	//		}
	//		m_nAtt[ERA_Crit_Rate] = CalAttMod(m_nAtt[ERA_Crit_Rate], ERA_Crit_Rate);
	//		m_bAttRecalFlag[ERA_Crit_Rate] = true;
	//	}
	//	break;
	//	//暴伤
	//case ERA_Crit_Amount:
	//	{
	//		if (m_eClass == EV_Blader)//刺客 暴伤=力量
	//		{
	//			m_nAtt[ERA_Crit_Amount]	= m_nAtt[ERA_Strength] + m_nBaseAtt[ERA_Crit_Amount];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_Crit_Amount]	= m_nBaseAtt[ERA_Crit_Amount];
	//		}
	//		m_nAtt[ERA_Crit_Amount] = CalAttMod(m_nAtt[ERA_Crit_Amount], ERA_Crit_Amount);
	//		m_bAttRecalFlag[ERA_Crit_Amount] = true;
	//	}
	//	break;
	//	//反暴击
	//case ERA_UnCrit_Rate:
	//	{
	//		if (m_eClass == EV_Astrologer)//药师 反暴击=力量
	//		{
	//			m_nAtt[ERA_UnCrit_Rate]	= m_nAtt[ERA_Strength] + m_nBaseAtt[ERA_UnCrit_Rate];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_UnCrit_Rate]	= m_nBaseAtt[ERA_UnCrit_Rate];
	//		}
	//		m_nAtt[ERA_UnCrit_Rate] = CalAttMod(m_nAtt[ERA_UnCrit_Rate], ERA_UnCrit_Rate);
	//		m_bAttRecalFlag[ERA_UnCrit_Rate] = true;
	//	}

	//	break;
	//	//反暴伤
	//case ERA_UnCrit_Amount:
	//	{
	//		if (m_eClass == EV_Astrologer)//药师 反暴伤=力量
	//		{
	//			m_nAtt[ERA_UnCrit_Amount]	= m_nAtt[ERA_Strength] + m_nBaseAtt[ERA_UnCrit_Amount];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_UnCrit_Amount]	= m_nBaseAtt[ERA_UnCrit_Amount];
	//		}
	//		m_nAtt[ERA_UnCrit_Amount] = CalAttMod(m_nAtt[ERA_UnCrit_Amount], ERA_UnCrit_Amount);
	//		m_bAttRecalFlag[ERA_UnCrit_Amount] = true;
	//	}
	//	break;
	//	//生命恢复
	//case ERA_HPRegainRate:
	//	{
	//		switch(m_eClass)
	//		{
	//			case EV_Warrior:	
	//			case EV_Mage: //猛将 少保 生命恢复=智力
	//				m_nAtt[ERA_HPRegainRate]	= m_nAtt[ERA_InnerForce] + m_nBaseAtt[ERA_HPRegainRate];
	//				break;
	//			default:
	//				m_nAtt[ERA_HPRegainRate] = m_nBaseAtt[ERA_HPRegainRate];
	//				break;
	//		}
	//		m_nAtt[ERA_HPRegainRate] = CalAttMod(m_nAtt[ERA_HPRegainRate], ERA_HPRegainRate);
	//		m_bAttRecalFlag[ERA_HPRegainRate] = true;
	//	}
	//	break;
	//	//法力恢复
	//case ERA_MPRegainRate:
	//	{
	//		if (m_eClass == EV_Astrologer)//药师 法力恢复 = 智力
	//		{
	//			m_nAtt[ERA_MPRegainRate]	= m_nAtt[ERA_InnerForce] + m_nBaseAtt[ERA_MPRegainRate];
	//		}
	//		else
	//		{
	//			m_nAtt[ERA_MPRegainRate]	= m_nBaseAtt[ERA_MPRegainRate];
	//		}
	//		m_nAtt[ERA_MPRegainRate] = CalAttMod(m_nAtt[ERA_MPRegainRate], ERA_MPRegainRate);
	//		m_bAttRecalFlag[ERA_MPRegainRate] = true;
	//	}
	//	break;
	//default:
	//	break;
	//}
}
VOID Role::ReAccAtt()
{
	Unit::ReAccAtt();

	bool bAttARecal[X_ERA_ATTA_NUM] = {false};		// 一级属性是否进行了重算
	bool bAttBRecal[X_ERA_AttB_NUM] = {false};		// 二级属性是否进行了重算

	// 开始计算
	for(INT n = 0; n < ERA_End; n++)
	{
		if( false == GetAttRecalFlag(n) )
			continue;

		if( n >= ERA_AttA_Start && n <= ERA_AttA_End )
		{
			bAttARecal[MTransERAAttA2Index(n)] = true;
		}
		else if( n >= ERA_AttB_Start && n < ERA_AttB_End )
		{
			bAttBRecal[MTransERAAttB2Index(n)] = true;
		}
	}

	// 判断一级属性和二级属性之间的关联

	// 首先查看是否有一级属性进行了重新计算
	for(INT n = 0; n < X_ERA_ATTA_NUM; ++n)
	{
		if( !bAttARecal[n] ) continue;

		// 得到是哪个一级属性
		ERoleAttribute eType = (ERoleAttribute)MTransIndex2ERAATTA(n);
		
		const s_role_att_change* pAttChange = AttRes::GetInstance()->GetRoleAttChange(m_eClass, (DWORD)eType+1);
		//ASSERT(VALID_POINT(pAttChange));
		if(!VALID_POINT(pAttChange))
			return;

		//for (int i = 0; i < earc_num; i++)
		//{
		//	if (pAttChange->n_att_change_[i] != 0)
		//	{
		//		ERoleAttribute erab = ERAC2ERA((e_role_att_to_change)i);
		//		AccountAtt(erab);
		//		bAttBRecal[MTransERAAttB2Index(erab)] = false;
		//	}	
		//}
		//switch(eType)
		//{
		//	// 体质，则二级属性的最大体力和外功防御要重新计算
		//case ERA_Physique:
		//	{
		//		// 计算最大体力
		//		AccountAtt(ERA_MaxHP);

		//		// 已经重新计算过的二级属性下面就不需要重新计算了
		//		bAttBRecal[MTransERAAttB2Index(ERA_MaxHP)] = false;

		//	}
		//	break;

		//	// 力量，则二级属性的外功攻击，外功防御和持久力需要重新计算
		//case ERA_Strength:
		//	{
		//		// 计算攻击
		//		AccountAtt(ERA_ExAttack);
		//		// 计算防御
		//		AccountAtt(ERA_ExDefense);
		//		// 计算招架
		//		AccountAtt(ERA_Block_Rate);
		//		// 计算暴击
		//		AccountAtt(ERA_Crit_Rate);
		//		// 计算暴伤
		//		AccountAtt(ERA_Crit_Amount);
		//		// 计算反暴击
		//		AccountAtt(ERA_UnCrit_Rate);
		//		// 计算反暴伤
		//		AccountAtt(ERA_UnCrit_Amount);
		//		// 已经重新计算过的二级属性下面就不需要重新计算了
		//		bAttBRecal[MTransERAAttB2Index(ERA_ExAttack)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_ExDefense)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_Block_Rate)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_Crit_Rate)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_Crit_Amount)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_UnCrit_Rate)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_UnCrit_Amount)] = false;
		//	}
		//	break;
		//	// 智力，则二级属性的攻击，闪避和魔法上限要重新计算
		//case ERA_InnerForce:
		//	{
		//		// 计算攻击
		//		AccountAtt(ERA_ExAttack);
		//		// 计算闪避
		//		AccountAtt(ERA_Dodge);
		//		// 计算魔法上限
		//		AccountAtt(ERA_MaxMP);
		//		// 计算生命恢复
		//		AccountAtt(ERA_HPRegainRate);
		//		// 计算魔法恢复
		//		AccountAtt(ERA_MPRegainRate);
		//		// 已经重新计算过的二级属性下面就不需要重新计算了
		//		bAttBRecal[MTransERAAttB2Index(ERA_InAttack)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_Dodge)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_MaxMP)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_HPRegainRate)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_MPRegainRate)] = false;

		//	}
		//	break;

		//	// 敏捷，则二级属性中的防御技巧和闪避需要重新计算
		//case ERA_Agility:
		//	{
		//		// 计算攻击
		//		AccountAtt(ERA_ExAttack);

		//		// 计算闪避
		//		AccountAtt(ERA_Dodge);

		//		// 计算命中
		//		AccountAtt(ERA_HitRate);

		//		// 已经重新计算过的二级属性下面就不需要重新计算了
		//		bAttBRecal[MTransERAAttB2Index(ERA_ExAttack)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_Dodge)] = false;
		//		bAttBRecal[MTransERAAttB2Index(ERA_HitRate)] = false;

		//	}
		//	break;

		//default:
		//	break;
		//}

	}

	// 再判断计算过后是否还有二级属性需要计算
	//for(INT n = 0; n < X_ERA_AttB_NUM; ++n)
	//{
	//	if( !bAttBRecal[n] ) continue;

	//	// 得到是哪个二级属性
	//	ERoleAttribute eType = (ERoleAttribute)MTransIndex2ERAATTB(n);

	//	AccountAtt(eType);
	//}

}
//--------------------------------------------------------------------------------
// 属性改变引起的其它内容改变
//--------------------------------------------------------------------------------
VOID Role::OnAttChange(INT nIndex)
{
	switch(nIndex)
	{
		// 血量
	case ERA_HP:
		m_nAtt[ERA_HP] = min(m_nAtt[ERA_HP], m_nAtt[ERA_MaxHP]);
		break;

		// 真气
	case ERA_MP:
		m_nAtt[ERA_MP] = min(m_nAtt[ERA_MP], m_nAtt[ERA_MaxMP]);
		break;
	
		// 义气
	case ERA_Brotherhood:
		m_nAtt[ERA_Brotherhood] = min(m_nAtt[ERA_Brotherhood], m_nAtt[ERA_MaxBrotherhood]);
		break;

	case ERA_Fortune:
		m_nAtt[ERA_Fortune] = max(0, min(m_nAtt[ERA_Fortune], VigourMax(m_nLevel)));
		break;

		// 武魂
	case ERA_Wuhuen:
		m_nAtt[ERA_Wuhuen] = min(m_nAtt[ERA_Wuhuen], m_nAtt[ERA_MaxBrotherhood]);
		break;

	//case ERA_Endurance:
	//	m_nAtt[ERA_Endurance] = min(m_nAtt[ERA_Endurance], m_nAtt[ERA_MaxEndurance]);
	//	break;

		// 道德
	case ERA_Morality:
		{
			CalPKState();
		}
		break;

	default:
		break;
	}
}

VOID Role::OnAttChange(bool bRecalFlag[ERA_End])
{
	// XZ方向速度
	if( bRecalFlag[ERA_Speed_XZ] )
	{
		m_fXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_XZ]) / 10000.0f);
		if( E_Success == m_MoveData.StopMove() )
		{
			// 发送给自己速度改变
			NET_SIS_move_speed_change send;
			send.dw_role_id = m_dwID;
			send.curPos = m_MoveData.m_vPos;
			send.faceTo = m_MoveData.m_vFace;
			SendMessage(&send, send.dw_size);
		}
	}

	// Y方向速度
	if( bRecalFlag[ERA_Speed_Y] )
	{
		m_fYSpeed = X_DEF_Y_SPEED * (FLOAT(m_nAtt[ERA_Speed_Y]) / 10000.0f);
	}

	// 游泳速度
	if( bRecalFlag[ERA_Speed_Swim] )
	{
		m_fSwimXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_Swim]) / 10000.0f);
		if( E_Success == m_MoveData.StopMove() )
		{
			// 发送给自己速度改变
			NET_SIS_move_speed_change send;
			send.dw_role_id = m_dwID;
			send.curPos = m_MoveData.m_vPos;
			send.faceTo = m_MoveData.m_vFace;
			SendMessage(&send, send.dw_size);
		}
	}

	// 骑乘速度
	if( bRecalFlag[ERA_Speed_Mount] )
	{
		m_fMountXZSpeed = X_DEF_XZ_SPEED * (FLOAT(m_nAtt[ERA_Speed_Mount]) / 10000.0f);
		if( E_Success == m_MoveData.StopMove() )
		{
			// 发送给自己速度改变
			NET_SIS_move_speed_change send;
			send.dw_role_id = m_dwID;
			send.curPos = m_MoveData.m_vPos;
			send.faceTo = m_MoveData.m_vFace;
			SendMessage(&send, send.dw_size);
		}
	}

	// 体型
	if( bRecalFlag[ERA_Shape] )
	{
		FLOAT fCosf = FLOAT(m_nAtt[ERA_Shape]) / 10000.0f;
		m_Size.x = X_DEF_ROLE_SIZE_X * fCosf;
		m_Size.y = X_DEF_ROLE_SIZE_Y * fCosf;
		m_Size.z = X_DEF_ROLE_SIZE_Z * fCosf;
	}

	// 队长统御力改变
	if(GetTeamID() != INVALID_VALUE && bRecalFlag[ERA_Rein])
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(GetTeamID());
		if( VALID_POINT(pTeam) && pTeam->is_leader(GetID()) )
		{
			tagLeaderRein	LeaderRein;
			LeaderRein.nLeaderRein = GetAttValue(ERA_Rein);
			g_groupMgr.AddEvent(GetID(), EVT_ChangeRein, sizeof(tagLeaderRein), &LeaderRein);
		}
	}
}

VOID Role::OnAttChange(INT nIndex, INT nDelta)
{
	if(nIndex == ERA_Fortune){
		if( nDelta < 0 ) {
			set_history_vigour_cost_ex(abs(nDelta));
		} 
	}
}
VOID Role::OnLeaveMap()
{
	// 切换地图消失的buff
	OnInterruptBuffEvent(EBIF_ChangeMap);

	// 清空仇恨列表
	ClearEnmityCreature();

	// 收起摆摊
	SendCloseStall();
	
	// 收起召唤宠物
	GetPetPocket()->CalledPetEnterPocket();

	// 停止修炼
	if(!is_leave_pricitice())
		PracticeEnd();

	// 使用传送结束钓鱼
	this->StopFishing();
	
	// 清除光环效果
	ClearTargetRingBuff();
		
	// 清除火墙类技能
	GetCombatHandler().ClearPilotList();

	m_mapLastAttacker.clear( );
}

VOID Role::OnEnterMap(Map* pScrMap)
{
	if(VALID_POINT(m_pScript))
		m_pScript->OnRoleEnterMap(this);

	Map* pMap = get_map( );
	if( VALID_POINT(pMap) )
	{
		if(ride_limit(NULL))
				StopMount();

		//NND，让你回来,弄死你
		DWORD dwPrisionID = g_mapCreator.get_prison_map_id();
		if( pMap->get_map_id( ) != dwPrisionID )
			this->PKPenalty( TRUE );
	}

	if (VALID_POINT(pScrMap))
	{

		// 跟随的怪从原地图删除
		Creature* pFlow = pScrMap->find_creature(m_dwFlowUnit);
		if (VALID_POINT(pFlow))
		{
			pScrMap->remove_flow_creature(pFlow);

			// 加到当前地图中
			if (VALID_POINT(pMap))
			{
				DWORD dwID = pMap->getNewCreatureID();
				pFlow->SetID(dwID);
				pFlow->SetMapID(pMap->get_map_id());
				SetFlowUnit(dwID);
				Vector3 pos = GetCurPos();
				pFlow->GetMoveData().Reset(pos.x, pos.y, pos.z, pos.x, pos.y, pos.z);
				pMap->add_creature(pFlow);
			}

		}
		else if(m_dwFlowUnit != INVALID_VALUE)
		{
			SI_LOG->write_log(_T("cant find flow unit!!"));
		}

	}
}

//--------------------------------------------------------------------------------
// 设置帮派属性
//--------------------------------------------------------------------------------
VOID Role::SetGuildID(DWORD dwGuildID)
{
	m_dwGuildID = dwGuildID;

	NET_DB2C_change_role_guild send;
	send.dw_role_id	= GetID();
	send.dw_guild_id	= dwGuildID;
	g_dbSession.Send(&send, send.dw_size);
}

//--------------------------------------------------------------------------------
// 设置离开帮派时间
//--------------------------------------------------------------------------------
VOID Role::SetLeaveGuildTime()
{
	m_LeaveGuildTime = g_world.GetWorldTime();
	NET_DB2C_change_role_leave_guild_time send;
	send.dw_role_id = GetID();
	send.dw_leave_guild_time_ = m_LeaveGuildTime;
	g_dbSession.Send(&send, send.dw_size);
}

//--------------------------------------------------------------------------------
// 世界喊话
//--------------------------------------------------------------------------------
BOOL Role::TalkToWorld()
{
	if (m_nWorldTalkCounter < 0)
	{
		m_nWorldTalkCounter = WORLD_CHANNEL_INTERVAL;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//--------------------------------------------------------------------------------
// 更新世界喊话计时
//--------------------------------------------------------------------------------
VOID Role::UpdateTalkToWorld()
{
	if (m_nWorldTalkCounter >= 0)
	{
		--m_nWorldTalkCounter;
	}
}

BOOL Role::talk_to_map()
{
	if( map_talk_counter_ < 0 )
	{
		map_talk_counter_ = WORLD_CHANNEL_INTERVAL;
		return TRUE;
	}

	return FALSE;
}


VOID Role::update_talk_to_map()
{
	if( map_talk_counter_ >= 0) 
		--map_talk_counter_;
}

//--------------------------------------------------------------------------------
// 更新人物好友值
//--------------------------------------------------------------------------------
VOID Role::UpdateAddFreindValue()
{
	if(GetTeamID() == INVALID_VALUE)
		return;

	if(m_nAddFriendValueCounter >= 0)
	{
		m_nAddFriendValueCounter--;
	}
	else
	{
		const Team* pTeam = g_groupMgr.GetTeamPtr(GetTeamID());
		if(VALID_POINT(pTeam))
		{
			for(INT i = 0; i < pTeam->get_member_number(); i++)
			{
				Role* pRole = pTeam->get_member(i);
				if(VALID_POINT(pRole))
				{
					tagFriend* pFriend = pRole->GetFriendPtr(GetID());
					if(VALID_POINT(pFriend))
					{
						tagFriend* pOwnFriend = GetFriendPtr(pRole->GetID());
						if(VALID_POINT(pOwnFriend))
						{
							pFriend->dwFriVal++;
							pFriend->dwFriVal = (pFriend->dwFriVal > MAX_FRIENDVAL) ? MAX_FRIENDVAL : pFriend->dwFriVal;

							NET_DB2C_update_friend_value sendDB;
							sendDB.dw_role_id = pRole->GetID();
							sendDB.s_friendship_save_.dw_friend_id_ = GetID();
							sendDB.s_friendship_save_.n_frival_ = pFriend->dwFriVal;
							g_dbSession.Send(&sendDB, sendDB.dw_size);

							NET_SIS_update_friend_value sendD;
							sendD.dw_role_id = GetID();
							sendD.nFriVal = pFriend->dwFriVal;
							pRole->SendMessage(&sendD, sendD.dw_size);
						}
					}
				}
			}
		}
		m_nAddFriendValueCounter = ADD_FRIENDVALUE_COUNT;
	}
}

//--------------------------------------------------------------------------------
//计算好友经验提升百分比
//--------------------------------------------------------------------------------
FLOAT Role::CalFriendExpPer(const Team* pTeam)
{
	FLOAT fExp = 1.0f;
	if(!VALID_POINT(pTeam))
		return fExp;

	for(INT i = 0; i < pTeam->get_member_number(); i++)
	{
		Role* pRole = pTeam->get_member(i);
		if(!VALID_POINT(pRole))
			continue;

		tagFriend* pFriend = pRole->GetFriendPtr(GetID());
		if(!VALID_POINT(pFriend))
			continue;

		if(VALID_POINT(m_pScript))
		{
			fExp += m_pScript->OnCalFriendExp(this, pFriend->dwFriVal);
		}
	}

	return fExp;
}

//--------------------------------------------------------------------------------
// 拾取物品
//--------------------------------------------------------------------------------
DWORD Role::PickUpItem( INT64 n64GroundID )
{
	// mwh-2011-07-12 骑马也可以拾取
	// StopMount();

	M_trans_else_ret(pMap, get_map(), Map, E_Loot_Map_Not_Found);
	M_trans_else_ret(pGroundItem, pMap->get_ground_item(n64GroundID), tag_ground_item, E_Loot_Item_Not_Found);

	if(pGroundItem->dw_team_id == INVALID_VALUE && pGroundItem->dw_owner_id != INVALID_VALUE && GetID() != pGroundItem->dw_owner_id)
		return E_Loot_Belong_To_You;

	if(pGroundItem->dw_team_id != INVALID_VALUE && GetTeamID() != pGroundItem->dw_team_id)
		return E_Loot_Belong_To_Team;

	if(pGroundItem->dw_team_id != INVALID_VALUE && pGroundItem->dw_owner_id != INVALID_VALUE && GetID() != pGroundItem->dw_owner_id)
		return E_Loot_Belong_To_Teammate;

	//gx modify 2013.8.6 解决挂机拾取异常
	/*if( pGroundItem->dw_team_id != INVALID_VALUE && pGroundItem->dw_owner_id == INVALID_VALUE )
	{
		if( pGroundItem->is_notice( ) )
		{
			tagAssignNotice stAssignNotice;
			stAssignNotice.dwMapID = pMap->get_map_id( );
			stAssignNotice.dwTeamID = pGroundItem->dw_team_id;
			stAssignNotice.ePickMode = pGroundItem->e_pick_mode;
			stAssignNotice.n64GroundID = n64GroundID;
			g_groupMgr.AddEvent( GetID( ), EVT_NoticeAssign, sizeof(stAssignNotice), &stAssignNotice );
			return E_Loot_WaitForAssign;
		}
		if( pGroundItem->is_wait_for_assign( ) )return  E_Loot_WaitForAssign;
	}*/

	//检查玩家和物品的距离 拾取范围和自动拾取范围是不一样的
	if ( abs(pGroundItem->v_pos.x - GetCurPos().x) > X_DEF_AUTO_PICKUP_DIST 
		|| abs(pGroundItem->v_pos.z - GetCurPos().z) > X_DEF_AUTO_PICKUP_DIST )
		return E_Loot_Pick_Up_Too_Far;

	//背包是否有空间
	if ( TYPE_ID_MONEY != pGroundItem->dw_type_id && GetItemMgr().GetBagFreeSize() <= 0)
		return E_Loot_BAG_NOT_ENOUGH_SPACE;

	DWORD dwRtv = E_Success;

	//注意把掉落物品拾取到背包里,这里tag_ground_item中的pItem指针会被销毁,所以放进背包应该是最后
	if (pGroundItem->dw_type_id == TYPE_ID_MONEY && 
		FALSE == GetCurMgr().IncBagSilver(pGroundItem->n_num, elcid_pickup_money)
		)
		return E_Loot_Add_Bag_Not_Success;
	//注意把掉落物品拾取到背包里,这里tag_ground_item中的pItem指针会被销毁,所以放进背包应该是最后
	else if (pGroundItem->dw_type_id != TYPE_ID_MONEY && 
		E_Success != (dwRtv = GetItemMgr().Add2Bag(pGroundItem->p_item, elcid_pickup_item, TRUE))
		)
		return dwRtv;
	else
	{
// 		//发送物品消失消息
// 		NET_SIS_role_ground_item_disappear disappear;
// 		disappear.n64_serial[0] = n64GroundID;
// 		pMap->send_big_visible_tile_message(this, &disappear, disappear.dw_size);
		if( HearSayHelper::IsItemBroadcast(pGroundItem->get_boss_id(), pGroundItem->p_item))
		{
			HearSayHelper::SendMessage(EHST_KILLBOSSGETITEM, GetID(),pGroundItem->p_item->dw_data_id, 
				 pGroundItem->get_boss_id(),  INVALID_VALUE, INVALID_VALUE, pGroundItem->p_item);
		}
		
		//从Map中删除掉落物品
		//将掉落物品从地面删除
		pMap->remove_ground_item(pGroundItem);

		//将指针恢复为NULL
		pGroundItem->p_item=NULL;
		SAFE_DELETE(pGroundItem);
	}

	return E_Success;
}

//--------------------------------------------------------------------------------
// 放下物品
//--------------------------------------------------------------------------------
DWORD Role::putdown_item( INT64 n64_serial, BYTE by_type)
{

	M_trans_else_ret(pMap, get_map(), Map, INVALID_VALUE);

	DWORD dwRet = E_Success;
	tagItem *pItem = NULL;

	const tag_map_info* pInfo = get_map()->get_map_info();
	if (VALID_POINT(pInfo))
	{
		// 矿洞不让丢东西
		if (pInfo->e_normal_map_type == ENMT_Activity)
		{		
			return E_Destory_Equip_Error;
		}
	}
	
	if( E_Success != (dwRet = GetItemMgr().DiscardFromBag(n64_serial, elcid_role_discard_item, pItem, by_type)))
	{
		return dwRet;
	}

	// todo：这个地方修正一下高度，像个更好一点的办法
	// 直接删掉物品,不在地图上出现
	//if(VALID_POINT(pItem))
	//{
	//	tag_ground_item* pGroundItem = new tag_ground_item(pMap->get_ground_item_id(), 
	//												pItem->dw_data_id, pItem->n16Num, pItem, 
	//												GetCurPos(), INVALID_VALUE, INVALID_VALUE, 0, GetID());

	//	ASSERT(pGroundItem->valid());

	//	pMap->add_ground_item(pGroundItem);
	//}
	return E_Success;
}
FLOAT Role::GetVNBExpRate()
{
	return VALID_POINT(GetSession()) ? GetSession()->GetVNBExpRate() / 10000.0f : 1.0f;
}

FLOAT Role::GetVNBLootRate()
{
	return VALID_POINT(GetSession()) ? GetSession()->GetVNBLootRate() / 10000.0f : 1.0f;
}

//--------------------------------------------------------------------------------
// 增加角色开启宝箱数
//--------------------------------------------------------------------------------
VOID Role::IncTreasureSum()
{
	++m_nTreasureSum;
	if (m_nTreasureSum > ROLE_CHSET_RATE)
		m_nTreasureSum = 1;

	NET_DB2C_update_treasure_sum send;
	send.dw_role_id	= GetID();
	send.n_sum_	= m_nTreasureSum;
	g_dbSession.Send(&send, send.dw_size);
	
}

//--------------------------------------------------------------------------------
// 设置角色开启宝箱数
//--------------------------------------------------------------------------------
VOID Role::SetTreasureSum(INT nSum)
{
	if (nSum > ROLE_CHSET_RATE)
		nSum = 1;

	m_nTreasureSum = nSum;
	NET_DB2C_update_treasure_sum send;
	send.dw_role_id	= GetID();
	send.n_sum_	= m_nTreasureSum;
	g_dbSession.Send(&send, send.dw_size);
}

VOID Role::StopMount()
{

	if (IsInRoleState(ERS_Mount))
	{
		NET_SIS_cancel_ride send;
		send.dwRoleID = GetID();
		send.dwError	= GetRaidMgr().CancelRaid( );

		if (send.dwError == E_Success)
		{
			if(VALID_POINT(get_map()))
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
		}
		else
		{
			SendMessage(&send, send.dw_size );
		}
	}
}

void Role::BreakMount()
{
	if (IsInRoleStateAny(ERS_Mount | ERS_Mount2))
	{
		StopMount();
	}
}

//----------------------------------------------------------------------------------------
// 添加指定套装的所有装备到背包(GM命令调用)
//----------------------------------------------------------------------------------------
DWORD Role::AddSuit(DWORD dwSuitID, INT nQlty)
{
	package_map<DWORD, tagEquipProto*> mapEquip = AttRes::GetInstance()->GetEquipMap();
	mapEquip.reset_iterator();
	
	DWORD dwEquipID = 0;
	tagEquipProto* pEquipProto = NULL;

	while ( mapEquip.find_next(dwEquipID, pEquipProto) )
	{
		for(INT i=0; i<MAX_PEREQUIP_SUIT_NUM; ++i)
		{
			if(dwSuitID == pEquipProto->dwSuitID[i])
			{
				tagItem *pItem = ItemCreator::Create(EICM_GM, NULL, dwEquipID, 1);
				if(!VALID_POINT(pItem)) return INVALID_VALUE;
				ASSERT( MIsEquipment(pItem->dw_data_id) );

				ItemCreator::IdentifyEquip((tagEquip*)pItem, (EItemQuality)nQlty);
				GetItemMgr().Add2Bag(pItem, elcid_gm_create_item, TRUE);

				//m_Suit.Add((tagEquip*)pItem, pItem->n16Index);
			}
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------------------------
// 添加指定级别的武器到背包(GM命令调用) nType 0:武器 1:其他
//----------------------------------------------------------------------------------------
DWORD Role::AddEquip(INT nType, INT nLevel, INT nQlty)
{
	package_map<DWORD, tagEquipProto*> mapEquip = AttRes::GetInstance()->GetEquipMap();
	mapEquip.reset_iterator();

	DWORD dwEquipID = 0;
	tagEquipProto* pEquipProto = NULL;

	while ( mapEquip.find_next(dwEquipID, pEquipProto) )
	{
		if(0 == nType)
		{
			if (pEquipProto->eType != 7) continue;
		}
		else
		{
			if (!(pEquipProto->eType == 8 || pEquipProto->eType == 9 || pEquipProto->eType == 10 || pEquipProto->eType == 11)) continue;
		}

		if(pEquipProto->byMinUseLevel == nLevel || pEquipProto->byMinUseLevel+5 == nLevel)
		{
			tagItem *pItem = ItemCreator::Create(EICM_GM, NULL, dwEquipID, 1);
			if(!VALID_POINT(pItem)) return INVALID_VALUE;

			ASSERT( MIsEquipment(pItem->dw_data_id) );

			ItemCreator::IdentifyEquip((tagEquip*)pItem, (EItemQuality)nQlty);
			GetItemMgr().Add2Bag(pItem, elcid_gm_create_item, TRUE);
		}
	}

	return TRUE;
}

MoveData::EMoveRet Role::MoveAction( PFMoveAction2P pAction, Vector3& v1, Vector3& v2 )
{
	MoveData::EMoveRet emr = (GetMoveData().*pAction)(v1, v2);
	return emr;
}
MoveData::EMoveRet Role::MoveAction( PFMoveAction1P pAction, Vector3& v )
{
	MoveData::EMoveRet emr = (GetMoveData().*pAction)(v);
	return emr;
}

DWORD Role::CanCastMotion( Unit* pDest, DWORD dwActionID )
{
	if (!VALID_POINT(pDest))
	{
		return E_Motion_CanotActive;
	}

	if(!GetMotionInviteStatus(pDest->GetID()))
		return E_Motion_CanotActive;

	const tagMotionProto* pMotion = AttRes::GetInstance()->GetMotionProto(dwActionID);
	if (!VALID_POINT(pMotion))
	{
		return E_Motion_NotValid;
	}

	// 系统判断目标是否为角色（而非怪物、NPC等任何非角色的可选中目标），若不符合，则提示：“无法进行交互”，若符合则进入下一步。
	if (!pDest->IsRole())
	{
		return E_Motion_CanotActive;
	}
	Role* pDestRole = static_cast<Role*>(pDest);
	// 系统判断目标是否为异性，若不符合，则提示：“无法进行交互”，若符合则进入下一步。
	if(pDestRole->GetSex() && GetSex() || !pDestRole->GetSex() && !GetSex())
	{
		return E_Motion_CanotActive;
	}
	// 系统判断目标是否处于非持械状态，若不符合，则提示：“无法进行交互”，若符合则进入下一步。
	if (pDestRole->IsInRoleStateAny(ERS_Combat | ERS_PK  | ERS_Arena) )
	{
		return E_Motion_CanotActive;
	}
	// 系统判断双方的友好度是否满足
	tagFriend* pFriend = GetFriendPtr(pDestRole->GetID());
	if (!VALID_POINT(pFriend) || pFriend->dwFriVal < pMotion->dwFriendVal)
	{
		return E_Motion_FriendValNotEnough;
	}

	// 若两者直线距离大于X个格子，则距离过远，交互动作失败，被取消。
	if (!IsInCombatDistance(*pDest, MAX_MOTION_DIST))
	{
		return E_Motion_DistanceNotEnough;
	}

	return E_Motion_Success;
	// 目标角色的屏幕内弹出确认提示框：“####（发起角色姓名）请求与你###（个性动作名称）？——接受/拒绝”
	// 点击“拒绝”，则发起者提示：“对方拒绝了您的请求。”
	// 若30秒内未做响应，则自动判断为拒绝。
	// 点击“接受”，则进入交互动作播放流程：
	// 发起者向接受者开始自动移动。
	// 若在自动移动过程中接受者移动，则交互动作失败，被取消。
	// 若在自动移动过程中发起者主动移动，则交互动作失败，被取消。
	// 若两者直线距离大于X个格子，则距离过远，交互动作失败，被取消。
	// 若发起者在向接受者自动移动过程中，被阻挡（如高差、不可行走区域），则交互动作失败，被取消。
	// 当上述一切判断、流程都通过后，发起者移动至接受者同一坐标同一高度位置，然后系统自动将朝向置为同一朝向，同时开始播放交互个性动作。
	// 若在交互动作播放流程过程中，接受其他角色或被其他角色接受另外一个已发起的交互动作申请，则之前的动作播放流程被打断。

}

DWORD Role::CastMotion( Unit* pDest, DWORD dwActionID )
{
	Role* pDestRole = static_cast<Role*>(pDest);
	
	NET_SIS_role_style_action send;
	send.dwActionID = dwActionID;

	send.dw_role_id = GetID();
	send.dwDstRoleID = pDestRole->GetID();
	get_map()->send_big_visible_tile_message(this, &send, send.dw_size);

	send.dw_role_id = pDestRole->GetID();
	send.dwDstRoleID = GetID();
	get_map()->send_big_visible_tile_message(this, &send, send.dw_size);

	return E_Motion_Success;
}

BOOL Role::GetMotionInviteStatus(DWORD dw_role_id)
{
	// 是伙伴的请求，则同意
	if (dw_role_id == m_dwPartnerID)
		return TRUE;
	// 没有伙伴，并且超时
	else if (!VALID_VALUE(m_dwPartnerID) && !VALID_VALUE(m_nMotionCounter))
		return TRUE;
	// 不是伙伴，不超时
	else
		return FALSE;
}

VOID Role::SetMotionInviteStatus(BOOL bSet, DWORD dw_role_id)
{
	if (VALID_VALUE(m_dwPartnerID) && dw_role_id != m_dwPartnerID)
		return ;

	if (bSet)
	{
		m_nMotionCounter = TICK_PER_SECOND * 60 * 30;
		m_dwPartnerID = dw_role_id;
	}
	else
	{
		m_nMotionCounter = INVALID_VALUE;
		m_dwPartnerID = INVALID_VALUE;
	}
}

VOID Role::UpdateMotionInviteState()
{
	if (m_nMotionCounter >= 0)
	{
		--m_nMotionCounter;
	}
	// time out, reset the partner
	else if (!VALID_VALUE(m_nMotionCounter))
	{
		m_dwPartnerID = INVALID_VALUE;
	}
}

INT Role::CanGather(Creature* pRes)
{
	// 获得资源和角色
	Creature* pCreature	= pRes;	
	Role* pRole	= this;	

	if ( !VALID_POINT(pCreature) || !VALID_POINT(pRole) )
		return INVALID_VALUE;

	// 判断是否是战斗状态
	//if(pRole->IsInState(ERS_Combat))
	//	return E_Master_StateLimit;

	// 判断资源并获得技能
	Skill* pGatherSkill = NULL;
	if ( pCreature->IsNatuRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_MINING);
	else if ( pCreature->IsManRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_HARVEST);
	else if ( pCreature->IsGuildNatuRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_GMINING);
	else if ( pCreature->IsGuildManRes() )
		pGatherSkill = pRole->GetSkill(GATHER_SKILL_GHARVEST);
	if ( !VALID_POINT(pGatherSkill) )
		return INVALID_VALUE;

	// 若角色背包已满				
	ItemMgr& itemMgr = pRole->GetItemMgr();	
	if (itemMgr.GetBagFreeSize() <= 0)
		return E_UseItem_NotEnoughSpace;

	// 若资源距离够近 
	if (!pRole->IsInCombatDistance(*pCreature, pGatherSkill->GetOPDist()))
		return E_UseSkill_DistLimit;	

	// 资源已被占有
	if( pCreature->IsTagged() )
		return E_UseSkill_TargetLimit;

	return E_Success;
}

VOID Role::SaveExp2DB()
{
	NET_DB2C_update_role_att send;

	send.dw_role_id = GetID();
	send.n_type_ = ertsa_exp;
	send.n_value_ = m_nCurLevelExp;

	g_dbSession.Send(&send, send.dw_size);
}

VOID Role::SaveAttPoint2DB()
{
	NET_DB2C_update_role_att_point send;

	send.dw_role_id = GetID();
	send.n_att_point_left_ = GetAttValue(ERA_AttPoint);
	get_fast_code()->memory_copy(send.n_att_point_add_, m_nAttPointAdd, sizeof(send.n_att_point_add_));

	g_dbSession.Send(&send, send.dw_size);
}

VOID Role::SaveTalentPoint2DB(INT nIndex)
{
	NET_DB2C_update_role_talent_point send;

	send.dw_role_id = GetID();
	send.n_talent_point_ = GetAttValue(ERA_TalentPoint);
	send.s_talent_ = m_Talent[nIndex];
	send.n_index_ = nIndex;
	g_dbSession.Send(&send, send.dw_size);
}

VOID Role::SaveLevel2DB()
{
	NET_DB2C_update_role_att send;

	send.dw_role_id = GetID();
	send.n_type_ = ertsa_level;
	send.n_value_ = m_nLevel;
	g_dbSession.Send(&send, send.dw_size);
}

// 保存义气到db
VOID	Role::SaveBrotherhood2DB()
{
	NET_DB2C_update_role_att send;

	send.dw_role_id = GetID();
	send.n_type_ = ertsa_brotherhood;
	send.n_value_ = GetAttValue(ERA_Brotherhood);
	g_dbSession.Send(&send, send.dw_size);
}

// 保存爱心值
VOID Role::SaveLove2DB()
{
	NET_DB2C_update_role_att send;

	send.dw_role_id = GetID();
	send.n_type_ = ertsa_love;
	send.n_value_ = GetAttValue(ERA_Love);
	g_dbSession.Send(&send, send.dw_size);
}

VOID Role::SaveWuhuen2DB()
{
	NET_DB2C_update_role_att send;

	send.dw_role_id = GetID();
	send.n_type_ = ertsa_wuhuen;
	send.n_value_ = GetAttValue(ERA_Wuhuen);
	g_dbSession.Send(&send, send.dw_size);
}

// 增加邮件发送数量
VOID Role::AddSendMailNum()
{
	m_nSendMailNum++;
}

// 获取背包或装备栏中的装备
tagItem* Role::GetItem(INT64 n64_serial)
{
	tagItem* pItem = GetItemMgr().GetEquipBarEquip(n64_serial);
	if (!VALID_POINT(pItem))
	{
		 pItem = GetItemMgr().GetBagItem(n64_serial);
	}

	return pItem;
}

e_role_att_to_change Role::ERA2ERAC(ERoleAttribute erac)
{
	switch(erac)
	{
	case ERA_ExAttack:
		return erac_attack;
	case ERA_ExDefense:
		return earc_defense;
	case ERA_MaxHP:
		return earc_maxhp;
	case ERA_MaxMP:
		return earc_maxmp;
	case ERA_HitRate:
		return earc_hit;
	case ERA_Dodge:
		return earc_dodge;
	case ERA_ShenAttack:
		return earc_block;
	case ERA_Crit_Rate:
		return earc_crit;
	case ERA_Crit_Amount:
		return earc_critvalue;
	case ERA_UnCrit_Rate:
		return earc_fancrit;
	case ERA_UnCrit_Amount:
		return earc_fancritvalue;
	case ERA_HPRegainRate:
		return earc_hp_reborn;
	case ERA_MPRegainRate:
		return earc_mp_reborn;
	}
	return erac_null;
}

ERoleAttribute Role::ERAC2ERA(e_role_att_to_change erac)
{
	switch(erac)
	{
	case erac_attack:
		return ERA_ExAttack;
	case earc_defense:
		return ERA_ExDefense;
	case earc_maxhp:
		return ERA_MaxHP;
	case earc_maxmp:
		return ERA_MaxMP;
	case earc_hit:
		return ERA_HitRate;
	case earc_dodge:
		return ERA_Dodge;
	case earc_block:
		return ERA_ShenAttack;
	case earc_crit:
		return ERA_Crit_Rate;
	case earc_critvalue:
		return ERA_Crit_Amount;
	case earc_fancrit:
		return ERA_UnCrit_Rate;
	case earc_fancritvalue:
		return ERA_UnCrit_Amount;
	case earc_hp_reborn:
		return ERA_HPRegainRate;
	case earc_mp_reborn:
		return ERA_MPRegainRate;
	}
	return ERA_Null;
}

VOID Role::set_master_prentice_forbid_time( )
{
	//m_dwMasterPrenticeForbidTime = 0;
	//gx modify 2013.12.10 师徒决裂加入惩罚机制
	tagDWORDTime curTime = GetCurrentDWORDTime();
	m_dwMasterPrenticeForbidTime = IncreaseTime(curTime,MASTERPRENTICEFORBIDTIME);
}

VOID Role::set_master_id( DWORD dwID )
{
	m_dwMasterID = dwID;
	//gx modify 暂不用发该消息
	/*Map* pMap = get_map();
	if( !VALID_POINT(pMap) )
		return;

	NET_SIS_role_master send;
	send.dwMaster  = m_dwMasterID;
	send.dwPrentice = GetID( );
	pMap->send_big_visible_tile_message( this, &send, send.dw_size );*/
}

// 在线挂机
VOID Role::UpdateHang()
{
	if(IsInRoleState(ERS_Hang)/* && IsInRoleState(ERS_HangArea)*/)
	{
		if(!IsCanHang())
		{
			if(IsHaveBuff(30003))
			{
				RemoveBuff(30003, TRUE);
			}

			UnsetRoleState(ERS_Hang);
		}

		m_dwHangTime -= TICK_TIME;

		if(m_dwHangTime <= 0/*CalcTimeDiff(GetCurrentDWORDTime(), m_dwHangBeginTime) >= ROLE_HANG_TIME*/)
		{
			m_dwHangBeginTime = GetCurrentDWORDTime();

			// 如果没有装备不计算
			tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
			if(!VALID_POINT(pEquip))
				return;

			INT32 n32_num = GetCurMgr().GetBaiBaoYuanBao();
			if(n32_num > 0)
			{
				INT n_exp = (150)*(1 + (FLOAT)(pEquip->equipSpec.byConsolidateLevel/10.0))*(1 + (FLOAT)(pEquip->equipSpec.byQuality/10.0));
				//ExpChange(n_exp);
				GetCurMgr().DecBaiBaoYuanBao(1, elcid_hang);
			}
			else
			{
				if(IsHaveBuff(30003))
				{
					RemoveBuff(30003, TRUE);
				}

				UnsetRoleState(ERS_Hang);
			}

			m_dwHangTime = HANG_TIME;
			/*INT32 nItem = GetItemMgr().GetBagSameItemCount(7001001);
			if(nItem > 0)
			{
				CalLineHang(pEquip, 7001001);
			}
			else
			{
				if(IsHaveBuff(30000))
				{
					RemoveBuff(30000, TRUE);
				}

				UnsetRoleState(ERS_Hang);
			}*/

			/*INT32 nItem = GetItemMgr().GetBagSameItemCount(7001002);
			if(nItem > 0)
			{
				CalLineHang(pEquip, 7001002);
			}
			else
			{
				if(IsHaveBuff(30000))
				{
					RemoveBuff(30000, TRUE);
				}
				
				nItem = GetItemMgr().GetBagSameItemCount(7001001);
				if(nItem <= 0)
				{
					UnsetRoleState(ERS_Hang);
					return;
				}

				CalLineHang(pEquip, 7001001);
			}*/
		}
	}
}

// 计算在线挂机经验
VOID Role::CalLineHang(tagEquip* pEquip, DWORD dw_data_id)
{
	INT nExp = 0;
	INT nBrotherhood = 0;
	package_list<tagItem*> ItemList;
	GetItemMgr().GetBagSameItemList(ItemList, dw_data_id, 1);
	if(!ItemList.empty())
	{
		package_list<tagItem*>::list_iter iter = ItemList.begin();
		tagItem* pItem = *iter;
		if(VALID_POINT(pItem))
		{
			nExp = (INT)((240 + get_level())*(((FLOAT)pItem->pProtoType->nSpecFuncVal1)/10));
			nBrotherhood = (INT)(((FLOAT)get_level()/2)*(((FLOAT)pItem->pProtoType->nSpecFuncVal2)/10));
			//ExpChange(nExp);
			ChangeBrotherhood(nBrotherhood);
			GetItemMgr().DelBagSameItem(ItemList, 1, elcid_leave_hang);

			//if(dw_data_id == 7001002)
			{
				if(!IsHaveBuff(30003))
				{
					// 添加无敌buff
					const tagBuffProto* pBuff = AttRes::GetInstance()->GetBuffProto(3000301);
					if(VALID_POINT(pBuff))
					{
						TryAddBuff((Unit*)this, pBuff, NULL, NULL, NULL);
					}	
				}
			}
		}
	}
}

// 计算离线挂机经验
VOID Role::CalLeaveLineHang(DWORD dw_time)
{
	INT nHour = dw_time / 3600;
	if(nHour <= 0)
		return;

	/*INT nPneuma = m_nAtt[ERA_Fortune];
	nPneuma = nPneuma / 15;
	if(nPneuma <= 0)
		return;*/

	INT nExp = 0;
	INT nBrother = 0;
	INT nExpItem = 0;
	INT nBrotherItem = 0;

	nHour = min(nHour, 24);

	nExp = nHour * 2000 * get_level();//gx modify 2013.12.20

	// 改变元气值，每小时10点
	//ModAttValue(ERA_Fortune, -nHour*15);
	//RecalAtt();

	/*nExp += ((240+get_level())/10)*nHour*60;
	nBrother += (INT)((((FLOAT)get_level()/2)/10)*nHour*60);*/

	// 使用经验丹
	/*if(m_bExp)
	{
		nExpItem = SetLeaveHangExp(7001003, nHour, nExp);
	}*/

	// 使用义气丹
	/*if(m_bBrotherhood)
	{
		nBrotherItem = SetLeaveHangExp(7001004, nHour, nBrother);
	}*/

	m_nLeaveExp += nExp;
	if(m_nLeaveExp > (24 * 2000 * get_level()))//gx modify 2013.12.20
		m_nLeaveExp = (24 * 2000 * get_level());

	//m_nLeaveBrotherHood += nBrother;
	//if(m_nLeaveBrotherHood > MAX_LEAVE_BROTHER)
	//	m_nLeaveBrotherHood = MAX_LEAVE_EXP;

	//NET_SIS_leave_exp_clueon send;
	//send.nBrother = nBrother;
	//send.nExp = nExp;
	//send.nBrotherItem = nBrotherItem;
	//send.nExpItem = nExpItem;
	//send.dw_logout_time = m_LogoutTime;
	//SendMessage(&send, send.dw_size);
}
// 计算离线时间
//存在问题：人物等级在逐步上升，这样计算的离线时间不准备，肯定是小于等于实际值
INT Role::CalLeaveTimeByExp(INT nLeaveExp)
{
	if (nLeaveExp <= 0)
		return 0;
	if (nLeaveExp >= 24 * 2000 * get_level())
		return 24;
	INT hours = nLeaveExp / (2000 * get_level());
	return min(hours,24);
}
// 计算离线灵气
VOID Role::CalLeaveLingqi(DWORD dw_time)
{
	INT nLingqi = dw_time / 60 * INJURY_INCREASE_VAL;
	ChangeLinqi(nLingqi);
}
// 设置离线经验
INT Role::SetLeaveHangExp(DWORD dw_data_id, INT nHour, INT& nExp)
{
	INT n_num = GetItemMgr().GetBagSameItemCount(dw_data_id);
	if(n_num > 0)
	{
		n_num = min(nHour, n_num);
		package_list<tagItem*> ItemList;
		GetItemMgr().GetBagSameItemList(ItemList, dw_data_id, n_num);
		if(!ItemList.empty())
		{
			package_list<tagItem*>::list_iter iter = ItemList.begin();
			tagItem* pItem = *iter;
			INT nTemp = 0;
			if(VALID_POINT(pItem))
			{
				if(dw_data_id == 7001003)
				{
					nTemp = (240 + get_level())*n_num*6*(pItem->pProtoType->nSpecFuncVal1-1);
				}
				
				/*if(dw_data_id == 7001004)
				{
					nTemp = (INT)(((FLOAT)get_level()/2)*n_num*6*(pItem->pProtoType->nSpecFuncVal1-1));
				}*/
			}
			GetItemMgr().DelBagSameItem(ItemList, n_num, elcid_leave_hang);
			nExp += nTemp;
		}
	}

	return n_num;
}

// 清除挂机次数
VOID Role::CleanHangNum()
{
	ZeroHangNum();
}

// 清除地图限制
VOID Role::DelMapLimit(INT nType)
{
	MapLimitMap::map_iter iter = m_mapMapLimit.begin();
	s_enter_map_limit* pEnterMapLimit = NULL;
	while(m_mapMapLimit.find_next(iter, pEnterMapLimit))
	{
		if(VALID_POINT(pEnterMapLimit))
		{
			if(pEnterMapLimit->e_enter_limit_ == nType)
			{
				pEnterMapLimit->dw_enter_num_ = 0;
			}
		}
	}
}

// 清除地图副本进度
VOID Role::DelMapProcess(INT nType)
{
	package_list<s_inst_process*>::list_iter iter = list_inst_process.begin();
	s_inst_process* p_process = NULL;
	while(list_inst_process.find_next(iter, p_process))
	{
		if(VALID_POINT(p_process))
		{
			if(p_process->n_type == nType)
			{
				list_inst_process.erase(p_process);
				SAFE_DELETE(p_process);
			}
		}
	}
}

// 开始挂机
DWORD Role::StartHang()
{
	if(IsInRoleState(ERS_Combat))
		return E_Hang_State_Limit;

	// 已经进入挂机状态
	if(IsInRoleState(ERS_Hang))
		return E_JoinHang_State;

	// 不在挂机区域
	/*if(IsInRoleState(ERS_HangArea))
		return E_Hang_Area_Limit;*/

	// 挂机次数已满
	//if(GetHangNum() >= 100)
	//	return E_Hang_Num_Limit;

	if(get_level() < 20)
		return E_Hang_Level_No_Enough;

	tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	if(!VALID_POINT(pEquip))
		return E_Hang_Equip_No_Exist;

	// 挂机物品不存在
	//INT32 nItem1 = GetItemMgr().GetBagSameItemCount(7001001);
	//INT32 nItem2 = GetItemMgr().GetBagSameItemCount(7001002);
	//if(!VALID_POINT(nItem1)/* && !VALID_POINT(nItem2)*/)
	//{
	//	return E_Hang_Item_No_Exist;
	//}

	if(GetCurMgr().GetBaiBaoYuanBao() <= 0)
		return E_Hang_Yuanbao_No_Enough;

	if(!IsHaveBuff(30003))
	{
		// 添加无敌buff
		const tagBuffProto* pBuff = AttRes::GetInstance()->GetBuffProto(3000301);
		if(VALID_POINT(pBuff))
		{
			TryAddBuff((Unit*)this, pBuff, NULL, NULL, NULL);
		}	
	}

	SetRoleState(ERS_Hang);
	//IncreaseHangNum();
	//SetHangBeginTime();

	/*NET_SIS_change_hang_num send;
	send.n16HangNum = GetHangNum();
	SendMessage(&send, send.dw_size);*/

	return E_Success;

}

// 是否可以继续挂机
BOOL Role::IsCanHang()
{
	BOOL bCannotHang = IsInState(ES_Dead) || 
		IsInState(ES_Dizzy) || 
		IsInState(ES_Tie) ||
		IsInState(ES_Spor) ||
		IsInRoleStateAny(ERS_Combat | ERS_Prictice | ERS_Stall);
	return !bCannotHang;
}

// 取消挂机
DWORD Role::CancelHang()
{
	if(!IsInRoleState(ERS_Hang))
		return E_Hang_NoHang_State;

	UnsetRoleState(ERS_Hang);

	RemoveBuff(30003, TRUE);

	return E_Success;
}

// 发送帮助数据
VOID Role::SendRoleHelp()
{
	NET_SIS_role_help send;
	get_fast_code()->memory_copy(send.byRoleHelp, m_byRoleHelp, sizeof(m_byRoleHelp));
	SendMessage(&send, send.dw_size);
}

// 修改帮助数据
VOID Role::SetRoleHelp(BYTE byIndex)
{
	m_byRoleHelp[byIndex] = 1;
}

// 发送人物对话数据
VOID Role::SendRoleTalk()
{
	NET_SIS_role_talk send;
	get_fast_code()->memory_copy(send.byRoleTalk, m_byTalkData, sizeof(m_byTalkData));
	SendMessage(&send, send.dw_size);
}

// 修改人物对话数据
VOID Role::SetRoleTalk(BYTE byIndex, BYTE byState)
{
	m_byTalkData[byIndex] = byState;
}

// 发送快捷键数据
VOID Role::SendKeyInfo()
{
	NET_SIS_key_info send;
	get_fast_code()->memory_copy(&send.stKeyInfo, &m_stKeyInfo, sizeof(m_stKeyInfo));
	SendMessage(&send, send.dw_size);
}

// 修改快捷键数据
VOID Role::SetKeyInfo(roleOnlineState* pKeyInfo)
{
	if(!VALID_POINT(pKeyInfo))
		return;

	get_fast_code()->memory_copy(&m_stKeyInfo, pKeyInfo, sizeof(m_stKeyInfo));
}

VOID Role::SendRoleDayClear()
{
	NET_SIS_role_day_claer send;
	get_fast_code()->memory_copy(send.byDayClear, m_byDayClear, sizeof(m_byDayClear));
	SendMessage(&send, send.dw_size);
}

VOID Role::SetRoleDayClearData(BYTE byIndex, BYTE byData)
{
	if (byIndex < 0 || byIndex >= ROLE_DAY_CLEAR_NUM)
		return;

	m_byDayClear[byIndex] = byData;
}

VOID Role::ModRoleDayClearDate(BYTE byIndex, BYTE byTime, BOOL bSnd)
{
	if (byIndex < 0 || byIndex >= ROLE_DAY_CLEAR_NUM)
		return;

	m_byDayClear[byIndex]+=byTime;
	if(bSnd) SendRoleDayClear( );
}

// 重置新手奖励
VOID Role::ResetNewRoleGift()
{
	if(m_stNewRoleGift.dw_gift_id_ == INVALID_VALUE)
		return;

	if(m_stNewRoleGift.n_step_-1 < 0)
		return;

	const s_new_role_gift_proto* pNewRoleGift = AttRes::GetInstance()->GetNewRoleGift(m_stNewRoleGift.n_id_);
	ASSERT(pNewRoleGift);

	if(!VALID_POINT(pNewRoleGift))
		return;

	m_stNewRoleGift.b_get_ = FALSE;
	m_stNewRoleGift.dw_leave_time_ = pNewRoleGift->dw_time_[m_stNewRoleGift.n_step_-1];
}

//发送领奖标志
VOID Role::SendReceiveType()
{
	if(VALID_POINT(GetSession()))
	{
		NET_SIS_send_receive_account_reward_ex send;
		send.dw_receive_type = GetSession()->GetReceiveTypeEx();
		SendMessage(&send, send.dw_size);
	}
}

// 发送1v1积分信息
VOID Role::Send1v1ScoreInfo()
{
	NET_SIS_1v1_score_info send;
	send.bAward = (get_1v1_score().n16_score_award > 0) ? TRUE : FALSE;
	send.nJoinNum = get_1v1_score().n_day_scroe_num;
	send.nMaxScore = get_1v1_score().n_day_max_score;
	send.nScore = get_1v1_score().n_cur_score;
	SendMessage(&send, send.dw_size);
}

// 发送活跃度数据
VOID Role::SendActiveInfo()
{
	NET_SIS_get_active_info send;
	send.n32_active_num = m_n32_active_num;
	memcpy(send.n32_active_data, m_n32_active_data, sizeof(m_n32_active_data));
	memcpy(send.b_active_receive, m_b_active_receive, sizeof(m_b_active_receive));
	SendMessage(&send, send.dw_size);
}

// 发送帮会活跃度数据
VOID Role::SendGuildActiveInfo()
{
	NET_SIS_get_guild_active_info send;
	send.n32_active_num = m_n32_guild_active_num;
	memcpy(send.n32_active_data, m_n32_guild_active_data, sizeof(m_n32_guild_active_data));
	memcpy(send.b_active_receive, m_b_guild_active_receive, sizeof(m_b_guild_active_receive));
	SendMessage(&send, send.dw_size);
}

VOID Role::SendConsumeReward()
{
	NET_SIS_get_ConsumeReward send;
	send.dwConsumeRewardConut = GetScriptData(65);
	send.dwConsumeRewardData = GetScriptData(66);
	SendMessage(&send, send.dw_size);
}
// 领取活跃度奖励
DWORD Role::ActiveReceive(INT nIndex)
{
	if(nIndex < 0 || nIndex > MAX_ACTIVE_RECEIVE)
		return INVALID_VALUE;

	if(!VALID_POINT(GetScript()))
		return INVALID_VALUE;

	return GetScript()->OnActiveReceive(this, nIndex);
}

DWORD Role::ActiveDone(INT nIndex,INT nBeishu)
{
	if(nIndex < 0 || nIndex > MAX_ACTIVE_RECEIVE)
		return INVALID_VALUE;
	if (nBeishu != 1 && nBeishu != 2)//只限双倍完成
		return INVALID_VALUE;

	if(!VALID_POINT(GetScript()))
		return INVALID_VALUE;

	return GetScript()->OnActiveDone(this, nIndex,nBeishu);
}
//gx add
VOID Role::DailyActTransmit(INT nIndex)
{
	if(nIndex < 0 || nIndex > MAX_DAILY_ACT_NUM)
		return;
	if(!VALID_POINT(GetScript()))
		return;
	GetScript()->OnDailyActTransmit(this,nIndex);
}
DWORD Role::GuildActiveReceive(INT nIndex)
{
	if(nIndex < 0 || nIndex > MAX_GUILD_ACTIVE_RECEIVE)
		return INVALID_VALUE;

	if(!VALID_POINT(GetScript()))
		return INVALID_VALUE;

	return GetScript()->OnGuildActiveReceive(this, nIndex);
}

// 重置活跃度数据
VOID Role::ResetActive()
{
	m_n32_active_num = 0;
	ZeroMemory(m_n32_active_data, sizeof(m_n32_active_data));
	ZeroMemory(m_b_active_receive, sizeof(m_b_active_receive));

	SendActiveInfo();
}

VOID Role::ResetGuildActive()
{
	m_n32_guild_active_num = 0;
	ZeroMemory(m_n32_guild_active_data, sizeof(m_n32_guild_active_data));
	ZeroMemory(m_b_guild_active_receive, sizeof(m_b_guild_active_receive));

	SendGuildActiveInfo();
}

VOID Role::initRoleGift(INT nStep, DWORD dwLeftTime)
{
	const s_new_role_gift_proto* pGift_Proto = AttRes::GetInstance()->GetNewRoleGift(1);
	ASSERT(pGift_Proto);
	if(!VALID_POINT(pGift_Proto))
		return;

	m_stNewRoleGift.n_id_ = 1;
	m_stNewRoleGift.n_step_ = nStep;
	m_stNewRoleGift.dw_gift_id_ = INVALID_VALUE;
	m_stNewRoleGift.dw_leave_time_ = 0;
	m_stNewRoleGift.b_get_ = false;

	if (nStep < 0 || nStep >= MAX_GIFT_NUM)
		return;

	m_stNewRoleGift.dw_gift_id_ = pGift_Proto->dw_gift_id_[nStep];
	m_stNewRoleGift.dw_leave_time_ = min(dwLeftTime, pGift_Proto->dw_time_[nStep]);
	m_stNewRoleGift.b_get_ = false;
}
// 创建新手奖励
VOID Role::CreateNewRoleGift()
{
	const s_new_role_gift_proto* pGift_Proto = AttRes::GetInstance()->GetNewRoleGift(1);
	ASSERT(pGift_Proto);
	if(!VALID_POINT(pGift_Proto))
		return;

	m_stNewRoleGift.n_id_ = pGift_Proto->n_id_;
	m_stNewRoleGift.n_step_ = 0;
	m_stNewRoleGift.dw_gift_id_ = pGift_Proto->dw_gift_id_[0];
	m_stNewRoleGift.dw_leave_time_ = pGift_Proto->dw_time_[0];
}

// 通知新手奖励数据
VOID Role::SendGiftInfo()
{
	const s_new_role_gift_proto* pGift_Proto = AttRes::GetInstance()->GetNewRoleGift(1);
	ASSERT(pGift_Proto);
	if(!VALID_POINT(pGift_Proto))
		return;

	if (m_stNewRoleGift.n_step_ < 0 || m_stNewRoleGift.n_step_ >= MAX_GIFT_NUM)
	{
		NET_SIS_new_role_gift send;
		send.dwGiftID = INVALID_VALUE;
		send.dwLeavingTime = 0;
		send.dwNumber = 0;
		send.bCal =  FALSE;
		SendMessage(&send, send.dw_size);

		return;
	}

	NET_SIS_new_role_gift send;
	send.dwGiftID = m_stNewRoleGift.dw_gift_id_;
	send.dwLeavingTime = m_stNewRoleGift.dw_leave_time_;
	send.dwNumber = pGift_Proto->dw_gift_num[m_stNewRoleGift.n_step_];
	send.bCal = (m_stNewRoleGift.n_step_-1) >= 0 ? TRUE : FALSE;
	SendMessage(&send, send.dw_size);
}

// 更新新手奖励
VOID Role::UpdateNewRoleGift()
{
	if(m_stNewRoleGift.n_step_ < 0 || m_stNewRoleGift.n_step_ > MAX_GIFT_NUM || m_stNewRoleGift.b_get_ || !m_stNewRoleGift.b_begin_time_)
		return;

	if (m_stNewRoleGift.dw_leave_time_ == 0)
	{
		m_stNewRoleGift.b_get_ = TRUE;
		return;
	}

	m_stNewRoleGift.dw_leave_time_ -= TICK_TIME;

	if(m_stNewRoleGift.dw_leave_time_ <= 30000)
	{
		m_stNewRoleGift.dw_leave_time_ = 0;
		m_stNewRoleGift.b_get_ = TRUE;
	}
}

// 获取新手在线奖励
DWORD Role::GetNewRoleGift()
{
	if(GetItemMgr().GetBagFreeSize() <= 0)
		return E_Gift_BagNoEnough;

	if(!m_stNewRoleGift.b_get_)
		return E_Gift_NotGet;

	if(m_stNewRoleGift.n_step_ >= MAX_GIFT_NUM)
		return INVALID_VALUE;

	const s_new_role_gift_proto* pNewRoleGift = AttRes::GetInstance()->GetNewRoleGift(m_stNewRoleGift.n_id_);
	ASSERT(pNewRoleGift);

	if(!VALID_POINT(pNewRoleGift))
		return INVALID_VALUE;

	tagItem* pItem = ItemCreator::Create(EICM_NewRoleGift, GetID(), m_stNewRoleGift.dw_gift_id_, pNewRoleGift->dw_gift_num[m_stNewRoleGift.n_step_], TRUE);
	if(!VALID_POINT(pItem))
		return INVALID_VALUE;

	GetItemMgr().Add2Bag(pItem, 1, elcid_new_role_gift);

	m_stNewRoleGift.n_step_++;


	if(m_stNewRoleGift.n_step_ >= MAX_GIFT_NUM)
	{
		m_stNewRoleGift.b_get_ = FALSE;
		m_stNewRoleGift.dw_gift_id_ = INVALID_VALUE;
		m_stNewRoleGift.dw_leave_time_ = 0;
		m_stNewRoleGift.b_begin_time_ = FALSE;
	}
	else
	{
		m_stNewRoleGift.b_get_ = FALSE;
		m_stNewRoleGift.b_begin_time_ = FALSE;
		m_stNewRoleGift.dw_gift_id_ = pNewRoleGift->dw_gift_id_[m_stNewRoleGift.n_step_];
		m_stNewRoleGift.dw_leave_time_ = pNewRoleGift->dw_time_[m_stNewRoleGift.n_step_];
	}


	

	SendGiftInfo();
	
	return E_Success;
}

tagAvatarEquip	Role::GetAvatarEquip() const
{
	// 不是时装模式直接返回装备
	if (!GetDisplaySet().bFashionDisplay)
	{
		return m_AvatarEquipEquip;
	}
	
	// 时装模式
	tagAvatarEquip avatarEquip = m_AvatarEquipEquip;
	
	for (int i = 0; i < X_AVATAR_ELEMENT_NUM; i++)
	{
		if (!m_AvatarEquipFashion.IsNull(i))
		{
			avatarEquip.AvatarEquip[i] = m_AvatarEquipFashion.AvatarEquip[i];
		}
	}
	return avatarEquip; 
}

// 发送角色的拍卖信息
VOID Role::send_paimai_info()
{
	INT n_num = 0;

	package_list<DWORD> list_paimai;

	g_paimai.get_role_paimai_num(GetID(), n_num, list_paimai);

	if(n_num == 0)
	{
		NET_SIS_send_own_paimai_info send;
		send.n_duty = AttRes::GetInstance()->GetVariableLen().n_paimai_duty;
		send.n_num = 0;
		SendMessage(&send, send.dw_size);
		return;
	}

	INT n_message_size = sizeof(NET_SIS_send_own_paimai_info) + (n_num-1)*sizeof(tag_own_paimai_info);
	CREATE_MSG(p_send, n_message_size, NET_SIS_send_own_paimai_info);

	INT n_temp_num = 0;

	package_list<DWORD>::list_iter iter = list_paimai.begin();
	DWORD dw_paimai_id = 0;
	while(list_paimai.find_next(iter, dw_paimai_id))
	{
		paimai* p_paimai = g_paimai.get_paimai_map().find(dw_paimai_id);

		if(!VALID_POINT(p_paimai))
		{
			print_message(_T("role paimai account not find %s %d\r\n"), _T(__FUNCTION__), __LINE__);
			continue;
		}

		if(!VALID_POINT(p_paimai->get_item()))
		{
			print_message(_T("role paimai item not find， id：%d, %s, %d\r\n"), dw_paimai_id, _T(__FUNCTION__), __LINE__);
			continue;
		}

		p_send->st_own_paimai[n_temp_num].dw_paimai_id = p_paimai->get_att().dw_paimai_id;
		p_send->st_own_paimai[n_temp_num].dw_begin_time = p_paimai->get_att().dw_beigin_time;
		p_send->st_own_paimai[n_temp_num].dw_bidup_price = p_paimai->get_att().dw_bidup;
		p_send->st_own_paimai[n_temp_num].dw_chaw_price = p_paimai->get_att().dw_chaw;
		p_send->st_own_paimai[n_temp_num].dw_sell_id = p_paimai->get_att().dw_sell_id;
		p_send->st_own_paimai[n_temp_num].by_time_type = p_paimai->get_att().by_time_type;

		if(MIsEquipment(p_paimai->get_item()->pProtoType->dw_data_id))
		{
			tagEquip* pEquip = (tagEquip*)p_paimai->get_item();
			p_send->st_own_paimai[n_temp_num].st_info.dw_data_id = pEquip->dw_data_id;
			p_send->st_own_paimai[n_temp_num].st_info.byBind = pEquip->byBind;
			p_send->st_own_paimai[n_temp_num].st_info.byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
			//p_send->st_own_paimai[n_temp_num].st_info.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
			p_send->st_own_paimai[n_temp_num].st_info.nUseTimes = pEquip->nUseTimes;
			p_send->st_own_paimai[n_temp_num].st_info.byHoldNum = pEquip->equipSpec.byHoleNum;
			//p_send->st_own_paimai[n_temp_num].st_info.n16MinDmg = pEquip->equipSpec.n16MinDmg;
			//p_send->st_own_paimai[n_temp_num].st_info.n16MaxDmg = pEquip->equipSpec.n16MaxDmg;
			//p_send->st_own_paimai[n_temp_num].st_info.n16Armor = pEquip->equipSpec.n16Armor;
			memcpy(p_send->st_own_paimai[n_temp_num].st_info.EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
			memcpy(p_send->st_own_paimai[n_temp_num].st_info.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);
		}
		else
		{
			p_send->st_own_paimai[n_temp_num].st_info.dw_data_id = p_paimai->get_item()->pProtoType->dw_data_id;
			p_send->st_own_paimai[n_temp_num].st_info.n_num = p_paimai->get_item()->n16Num;
		}
		n_temp_num++;
	}

	p_send->n_num = n_temp_num;
	p_send->n_duty = AttRes::GetInstance()->GetVariableLen().n_paimai_duty;
	p_send->dw_size = sizeof(NET_SIS_send_own_paimai_info) + (n_temp_num-1)*sizeof(tag_own_paimai_info);
	SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);
}

// 发送角色竞拍信息
VOID Role::send_jingpai_info()
{
	INT n_num = 0;

	package_list<DWORD> list_paimai;

	g_paimai.get_role_jingpai_num(GetID(), n_num, list_paimai);

	if(n_num == 0)
	{
		NET_SIS_send_own_jingpai_info send;
		send.n_num = 0;
		SendMessage(&send, send.dw_size);
		return;
	}

	INT n_message_size = sizeof(NET_SIS_send_own_jingpai_info) + (n_num-1)*sizeof(tag_own_paimai_info);
	CREATE_MSG(p_send, n_message_size, NET_SIS_send_own_jingpai_info);

	INT n_temp_num = 0;

	package_list<DWORD>::list_iter iter = list_paimai.begin();
	DWORD dw_paimai_id = 0;
	while(list_paimai.find_next(iter, dw_paimai_id))
	{
		paimai* p_paimai = g_paimai.get_paimai_map().find(dw_paimai_id);

		if(!VALID_POINT(p_paimai))
		{
			print_message(_T("role paimai account not find %s, %d\r\n"), _T(__FUNCTION__), __LINE__);
			continue;
		}

		if(!VALID_POINT(p_paimai->get_item()))
		{
			print_message(_T("role paimai item not find， id：%d, %s, %d\r\n"), dw_paimai_id, _T(__FUNCTION__), __LINE__);
			continue;
		}

		p_send->st_own_paimai[n_temp_num].dw_paimai_id = p_paimai->get_att().dw_paimai_id;
		p_send->st_own_paimai[n_temp_num].dw_begin_time = p_paimai->get_att().dw_beigin_time;
		p_send->st_own_paimai[n_temp_num].dw_bidup_price = p_paimai->get_att().dw_bidup;
		p_send->st_own_paimai[n_temp_num].dw_chaw_price = p_paimai->get_att().dw_chaw;
		p_send->st_own_paimai[n_temp_num].dw_sell_id = p_paimai->get_att().dw_sell_id;
		p_send->st_own_paimai[n_temp_num].by_time_type = p_paimai->get_att().by_time_type;
		p_send->st_own_paimai[n_temp_num].b_show_name = p_paimai->get_att().b_show_name;

		if(MIsEquipment(p_paimai->get_item()->pProtoType->dw_data_id))
		{
			tagEquip* pEquip = (tagEquip*)p_paimai->get_item();
			p_send->st_own_paimai[n_temp_num].st_info.dw_data_id = pEquip->dw_data_id;
			p_send->st_own_paimai[n_temp_num].st_info.byBind = pEquip->byBind;
			p_send->st_own_paimai[n_temp_num].st_info.byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
			//p_send->st_own_paimai[n_temp_num].st_info.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
			p_send->st_own_paimai[n_temp_num].st_info.nUseTimes = pEquip->nUseTimes;
			p_send->st_own_paimai[n_temp_num].st_info.byHoldNum = pEquip->equipSpec.byHoleNum;
			memcpy(p_send->st_own_paimai[n_temp_num].st_info.EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
			memcpy(p_send->st_own_paimai[n_temp_num].st_info.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);
		}
		else
		{
			p_send->st_own_paimai[n_temp_num].st_info.dw_data_id = p_paimai->get_item()->pProtoType->dw_data_id;
			p_send->st_own_paimai[n_temp_num].st_info.n_num = p_paimai->get_item()->n16Num;
		}
		n_temp_num++;
	}

	p_send->n_num = n_temp_num;
	p_send->dw_size = sizeof(NET_SIS_send_own_jingpai_info) + (n_temp_num-1)*sizeof(tag_own_paimai_info);
	SendMessage(p_send, p_send->dw_size);
	MDEL_MSG(p_send);
}

// 发送角色钱庄拍卖信息
VOID Role::send_bank_paimai_info()
{
	package_list<DWORD> list_bank;
	g_bankmgr.get_role_bank_paimai_list(GetID(), list_bank);

	INT n_size = list_bank.size();

	if(n_size <= 0)
	{
		NET_SIS_send_role_bank_paimai_info send;
		send.n_num = 0;
		SendMessage(&send, send.dw_size);
		return;
	}

	INT n_message_size = sizeof(NET_SIS_send_role_bank_paimai_info) + (n_size-1)*sizeof(tag_bank);

	CREATE_MSG(p_send, n_message_size, NET_SIS_send_role_bank_paimai_info);

	M_trans_pointer(p, p_send->st_bank, tag_bank);

	package_list<DWORD>::list_iter iter = list_bank.begin();
	DWORD dw_id = INVALID_VALUE;
	while(list_bank.find_next(iter, dw_id))
	{
		tag_bank* p_bank = g_bankmgr.get_bank_map().find(dw_id);
		if(!VALID_POINT(p_bank))
			continue;

		get_fast_code()->memory_copy(&p[p_send->n_num], p_bank, sizeof(tag_bank));

		p_send->n_num++;
	}

	p_send->n_duty = AttRes::GetInstance()->GetVariableLen().n_paimai_duty;
	p_send->dw_size = sizeof(NET_SIS_send_role_bank_paimai_info) + (p_send->n_num-1)*sizeof(tag_bank);
	SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}

// 是否是在hour后的第一次登陆
BOOL Role::is_today_first_login_after(INT hour) const
{
	tagDWORDTime current_time = GetCurrentDWORDTime();

	if(m_LogoutTime.year == 0 ) return FALSE;

	if(m_LogoutTime.year >= current_time.year &&
	   m_LogoutTime.month >= current_time.month &&
	   m_LogoutTime.day >= current_time.day &&
	   m_LogoutTime.hour >= current_time.hour) return FALSE;
	
	return TRUE;
}

BOOL Role::is_effect_sign_login() const
{
	//if (m_LoginTime.year == m_LoginTempTime.year && 
	//	m_LoginTime.month == m_LoginTempTime.month &&
	//	m_LoginTime.day == m_LoginTempTime.day)
	//	return false;
	
	tagDWORDTime nextDay = AddDay(m_LoginTempTime);

	if (m_LoginTime.year == nextDay.year && 
		m_LoginTime.month == nextDay.month &&
		m_LoginTime.day == nextDay.day)
		return true;


	return false;
}

VOID Role::AccountSignLevel()
{
	// 如果是同一天的,则没变化
	if (m_LoginTime.year == m_LoginTempTime.year && 
		m_LoginTime.month == m_LoginTempTime.month &&
		m_LoginTime.day == m_LoginTempTime.day)
		return;

	// 如果登出时间和当前登入时间未同一天，则没变化 gx add 2013.10.30 
	if (m_LoginTime.year == m_LogoutTime.year &&
		m_LoginTime.month == m_LogoutTime.month &&
		m_LoginTime.day == m_LogoutTime.day)
		return;

	// 隔天的话,等级增加
	if (is_effect_sign_login())
	{
		m_nSignLevel++;
		m_nSignLevel = min(6, m_nSignLevel);
	}	
	else//否则等级变成0
	{
		m_nSignLevel = 0;
	}

}
// 元气值奖励
VOID Role::send_vigour_reward(BOOL bFirstLogIn)
{
	if(!is_today_first_login_after(3)) return;

	DWORD dw = get_history_vigour_cost();  
	set_history_vigour_cost(0);

	//(发邮件)给奖励 FIXME::让脚本去做
	//mwh-2011-07-25
// 	if(VALID_POINT(m_pScript) && dw) 
// 	{
// 		BOOL b_send = m_pScript->OnVigourReward(this, dw, bFirstLogIn);
// 		NET_SIS_vigour_reward send;
// 		send.dwVigourCost = dw;
// 		if(!bFirstLogIn) send.e_code = b_send ? EVRC_Online_Reward : EVRC_Online_not_enough;
// 		else send.e_code = b_send ? EVRC_FirstLogin_Reward : EVRC_FirstLogin_not_enough;
// 		SendMessage(&send, send.dw_size);
// 	}
}

// 元气值重置
VOID Role::reset_vigour()
{
	SetAttValue(ERA_Fortune, VigourMin(m_nLevel));
	m_PerdayGetVigourTotal = 0;
}

// 元气值恢复
VOID Role::update_vigour()
{

//// 每天上线前2个小时以每12分钟增加10点元气值的速度增加（2个小时增加100点元气）							
//// 每天上线第3小时至第5小时每12分钟增加5点元气值（3个小时增加75点元气值）							
//// 每天上线5小时以上时每12分钟增加2点元气值（19个小时增加190点）							
//
//#define ONEMINUTETICK 60*TICK_PER_SECOND
//#define ONEHOURTICK 60*60*TICK_PER_SECOND
	inc_today_online_tick();
//	if( get_today_online_tick() <= 2 * ONEHOURTICK )
//	{
//		if( (get_today_online_tick() % (12*ONEMINUTETICK)) == 0 )
//			ModAttValue(ERA_Fortune, 10); // 增加 ERA_Vigour
//	}
//	else if(get_today_online_tick() <= 5 * ONEHOURTICK)
//	{
//		if( (get_today_online_tick() % (12*ONEMINUTETICK)) == 0 )
//			ModAttValue(ERA_Fortune, 5); // 增加 ERA_Vigour
//	}else {
//		if( (get_today_online_tick() % (12*ONEMINUTETICK)) == 0 )
//			ModAttValue(ERA_Fortune, 2); // 增加 ERA_Vigour
//	}

	/*	当C小于等于100时，每1分钟精力值增加1点			
	当C大于100小于等于200时，每3分钟增长1点			
	当C大于200小于250时，每8分钟增长1点			
	当C大于250时，每12分钟增长1点*/		

	if(GetAttValue(ERA_Fortune) >= VigourMax(m_nLevel))
		return ;

	BOOL addVigour = FALSE;
	if( m_PerdayGetVigourTotal <= 100 ){
		if(!(get_today_online_tick( ) % (TICK_PER_SECOND * 60))) addVigour = TRUE;
	} else if( m_PerdayGetVigourTotal <= 200 ){
		if(!(get_today_online_tick( ) % (TICK_PER_SECOND * 3 * 60)))  addVigour = TRUE;
	} else if( m_PerdayGetVigourTotal <= 250){
		if(!(get_today_online_tick( ) % (TICK_PER_SECOND * 8 * 60)))  addVigour = TRUE;
	} else {
		if(!(get_today_online_tick( ) % (TICK_PER_SECOND * 12 * 60)))  addVigour = TRUE;
	}

	if(addVigour){
		ModAttValue(ERA_Fortune, 1); 
		m_PerdayGetVigourTotal += 1;
	}
}

VOID Role::UpdateAutoKill()
{
	if(!mIsAutoKill)  return;
	if(--mAutoKillTickDecVigour > 0)return;

	ModAttValue(ERA_Fortune, AUTO_DEC_VIGOUR);
	mAutoKillTickDecVigour = AUTO_KILL_DEC_VIGOUR_TICK;
	
	DWORD tRet = CanAutoKill( );
	if(E_Success != tRet)
	{
		SetAutoKill(FALSE);
		NET_SIS_Auto_Kill_End send;
		send.dwErrorCode = tRet;
		this->SendMessage(&send, send.dw_size);
	}
}

DWORD Role::CanAutoKill()
{
	//gx modify 2013.6.20
	/*if(get_level() < AUTO_KILL_LEVEL_MIN) 
		return EAutoKill_OutOfLevel40;

	
	if(GetAttValue(ERA_Fortune) <= 0) 
		return EAutoKill_OutOfVigour;*/

	return E_Success;
}

// 重置副本进度
VOID Role::reset_inst_process(DWORD dw_map_id, INT n_mode)
{
	if(list_inst_process.size() <= 0)
		return;

	LIST_INST_PRO::list_iter iter = list_inst_process.begin();
	s_inst_process* p = NULL;
	while(list_inst_process.find_next(iter, p))
	{
		if(!VALID_POINT(p))
			continue;

		if(p->dw_map_id != dw_map_id)
			continue;

		if(p->n_mode != n_mode)
			continue;

		list_inst_process.erase(p);

		SAFE_DELETE(p);
	}
}

BOOL Role::is_have_inst_process(DWORD dw_map_id, INT n_mode)
{
	if(list_inst_process.size() <= 0)
		return FALSE;

	LIST_INST_PRO::list_iter iter = list_inst_process.begin();
	s_inst_process* p = NULL;
	while(list_inst_process.find_next(iter, p))
	{
		if(!VALID_POINT(p))
			continue;

		if(p->dw_map_id != dw_map_id)
			continue;

		if(p->n_mode != n_mode)
			continue;

		return TRUE;
	}

	return FALSE;
}

// 重置副本权值
DWORD Role::reset_instance_limit(DWORD dw_map_id)
{
	const tagInstance* pInst = AttRes::GetInstance()->get_instance_proto(dw_map_id);
	if(!VALID_POINT(pInst))
		return INVALID_VALUE;

	if(pInst->eInstanceEnterLimit != EEL_Week)
		return INVALID_VALUE;

	s_enter_map_limit* pEnterMapLimit = GetMapLimitMap().find(dw_map_id);
	if(!VALID_POINT(pEnterMapLimit))
		return E_Instance_limit_not_exists;

	if(GetCurMgr().GetBaiBaoYuanBao() < pInst->nResetLimitYB)
		return E_Instance_limit_yb_limit;

	pEnterMapLimit->dw_enter_num_ = 0;

	GetCurMgr().DecBaiBaoYuanBao(pInst->nResetLimitYB, elci_inst_limit);

	return E_Success;
}

//----------------------------------------------------------------------------------
// 得到周围怪物的ID，并返回数量
//----------------------------------------------------------------------------------
INT Role::GetAroundCreature(std::vector<DWORD> &vecCreature, FLOAT fOPRadius, FLOAT fHigh)
{
	// 如果范围为0，则直接返回
	if( 0.0f == fOPRadius )
		return 0;

	INT nCreatureNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到范围内的vistile列表
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Creature*	pCreature	= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 检测生物
		package_map<DWORD, Creature*>& mapCreature = pVisTile[n]->map_creature;
		package_map<DWORD, Creature*>::map_iter it2 = mapCreature.begin();

		while( mapCreature.find_next(it2, pCreature) )
		{
			if(pCreature->IsDead()) continue;

			// 将怪物加入到列表中
			if(abs(pCreature->GetCurPos().y - GetCurPos().y) <= fHigh)
			{
				vecCreature.push_back(pCreature->GetID());
				nCreatureNum++;
			}
		}
	}

	return nCreatureNum;
}

//----------------------------------------------------------------------------------
// 得到周围玩家的ID，并返回数量
//----------------------------------------------------------------------------------
INT	Role::GetAroundRole(std::vector<DWORD> &vecRole, FLOAT fOPRadius, FLOAT fHigh)
{
	// 如果攻击范围为0，则直接返回
	if( 0.0f == fOPRadius )
		return 0;

	INT nRoleNum = 0;
	tagVisTile* pVisTile[EUD_end] = {0};

	// 得到范围内的vistile列表
	get_map()->get_visible_tile(GetCurPos(), fOPRadius, pVisTile);
	Role*		pRole		= NULL;

	for(INT n = EUD_center; n < EUD_end; n++)
	{
		if( !VALID_POINT(pVisTile[n]) ) continue;

		// 检测人物
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		package_map<DWORD, Role*>::map_iter it = mapRole.begin();

		while( mapRole.find_next(it, pRole) )
		{
			if(pRole->IsDead()) continue;

			// 将玩家加入到列表中
			if(abs(pRole->GetCurPos().y - GetCurPos().y) <= fHigh)
			{
				vecRole.push_back(pRole->GetID());
				nRoleNum++;
			}
		}
	}

	return nRoleNum;
}


// 重置副本
DWORD Role::reset_instance(DWORD dw_map_id, INT n_mode)
{
	if(n_mode < EIHM_Normal || n_mode > EIHM_Devil)
		return INVALID_VALUE;
	
	// 如果没有队伍
	if(GetTeamID() == INVALID_VALUE)
	{
		if(!is_have_inst_process(dw_map_id, n_mode))
			return E_Instance_not_process;

		const tagInstance* pInst = AttRes::GetInstance()->get_instance_proto(dw_map_id);
		if(!VALID_POINT(pInst))
			return INVALID_VALUE;

		if(pInst->eInstanceEnterLimit == EEL_Week)
		{
			if(GetCurMgr().GetBaiBaoYuanBao() < pInst->nProcessYB)
				return E_Instance_process_yuanbao_limit;
		}

		reset_inst_process(dw_map_id, n_mode);

		if(pInst->eInstanceEnterLimit == EEL_Week)
		{
			GetCurMgr().DecBaiBaoYuanBao(pInst->nProcessYB, elci_process);
		}
	}
	else
	{
		const Team* p_team = g_groupMgr.GetTeamPtr(GetTeamID());
		if(!VALID_POINT(p_team))
			return INVALID_VALUE;

		if(p_team->is_have_leave_role())
			return E_Instance_have_online_team;

		if(p_team->get_own_instanceid() == INVALID_VALUE && p_team->get_own_instance_mapid() == INVALID_VALUE)
		{
			// 不是队长不能重置副本
			if(!p_team->is_leader(GetID()))
				return E_Instance_not_teamleader;

			if(!is_have_inst_process(dw_map_id, n_mode))
				return E_Instance_not_process;

			const tagInstance* pInst = AttRes::GetInstance()->get_instance_proto(dw_map_id);
			if(!VALID_POINT(pInst))
				return INVALID_VALUE;

			if(pInst->eInstanceEnterLimit == EEL_Week)
			{
				if(GetCurMgr().GetBaiBaoYuanBao() < pInst->nProcessYB)
					return E_Instance_process_yuanbao_limit;
			}

			reset_inst_process(dw_map_id, n_mode);

			if(pInst->eInstanceEnterLimit == EEL_Week)
			{
				GetCurMgr().DecBaiBaoYuanBao(pInst->nProcessYB, elci_process);
			}

			NET_SIS_reset_instance send;
			send.dw_error = E_Success;
			g_groupMgr.SendTeamateMessage(p_team->get_team_id(), GetID(), &send, send.dw_size);
		}
		else
		{
			return E_Instance_have_role;
		}
	}

	return E_Success;
}

// 检查是否可以扩充包裹或仓库
DWORD Role::exbag_check(DWORD dw_type, INT n_level)
{
	if(dw_type == 0) // 扩充包裹
	{
		if(n_level < n16_exbag_step)
			return E_Con_exstep_low;
		if(n_level > n16_exbag_step)
			return E_Con_exstep_up;

		return E_Success;
	}

	if(dw_type == 1) // 扩充仓库
	{
		if(n_level < n16_exware_step)
			return E_Con_exstep_low;
		if(n_level > n16_exware_step)
			return E_Con_exstep_up;

		return E_Success;
	}

	return INVALID_VALUE;
}

BOOL Role::HasCallPet() 
{ 
	return VALID_POINT(m_pPetPocket->GetCalledPetSoul());
}

VOID Role::SendPetSNSInfo()
{
	// 自己派出的宠物
	std::vector<tagPetSNSInfo*> vecMasterInfo;
	g_petSnsMgr.getPetSNSinfoByMasterID(vecMasterInfo, GetID());
	// 被人拍到自己这的宠物
	std::vector<tagPetSNSInfo*> vecFriendInfo;
	g_petSnsMgr.getPetSNSinfoByFriendID(vecMasterInfo, GetID());


	INT nNum = vecMasterInfo.size() + vecFriendInfo.size();
	tagPetSNSInfo* pRoleInfo = (tagPetSNSInfo*)INVALID_VALUE;

	DWORD dw_size = sizeof(NET_SIS_pet_SNS_info);

	if( nNum <= 0 )
		return;

	dw_size += (nNum-1) * sizeof(tagPetSNSInfo);
	

	CREATE_MSG(pSend, dw_size, NET_SIS_pet_SNS_info);

	pSend->n_num = nNum;
	
	INT nIndex = 0;
	for(INT i = 0; i < vecMasterInfo.size(); ++i)
	{
		if( INVALID_VALUE == vecMasterInfo[i]->dw_pet_id ) continue;

		pSend->petSNSInfo[nIndex].dw_master_id = vecMasterInfo[i]->dw_master_id;
		pSend->petSNSInfo[nIndex].dw_friend_id = vecMasterInfo[i]->dw_friend_id;
		pSend->petSNSInfo[nIndex].dw_pet_id = vecMasterInfo[i]->dw_pet_id;
		pSend->petSNSInfo[nIndex].dw_begin_time = vecMasterInfo[i]->dw_begin_time;
		++nIndex;
	}
	for(INT i = 0; i < vecFriendInfo.size(); ++i)
	{
		if( INVALID_VALUE == vecFriendInfo[i]->dw_pet_id ) continue;

		pSend->petSNSInfo[nIndex].dw_master_id = vecFriendInfo[i]->dw_master_id;
		pSend->petSNSInfo[nIndex].dw_friend_id = vecFriendInfo[i]->dw_friend_id;
		pSend->petSNSInfo[nIndex].dw_pet_id = vecFriendInfo[i]->dw_pet_id;
		pSend->petSNSInfo[nIndex].dw_begin_time = vecFriendInfo[i]->dw_begin_time;
		++nIndex;
	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

VOID Role::SetEquipTeamInfo(tagEquip* pEquip, BOOL bAdd)
{
	if(bAdd)
	{
		st_EquipTeamInfo.n32_ConsolidateNum += pEquip->equipSpec.byConsolidateLevel;
		INT n_num = 0;
		for(INT i = 0; i < MAX_EQUIPHOLE_NUM; i++)
		{
			if(pEquip->equipSpec.dwHoleGemID[i] != INVALID_VALUE && pEquip->equipSpec.dwHoleGemID[i] != 0)
				n_num++;
		}
		st_EquipTeamInfo.n32_InlayNum += n_num;
		//st_EquipTeamInfo.n32_Rating += pEquip->equipSpec.nRating;
	}
	else
	{
		st_EquipTeamInfo.n32_ConsolidateNum -= pEquip->equipSpec.byConsolidateLevel;
		INT n_num = 0;
		for(INT i = 0; i < MAX_EQUIPHOLE_NUM; i++)
		{
			if(pEquip->equipSpec.dwHoleGemID[i] != INVALID_VALUE && pEquip->equipSpec.dwHoleGemID[i] != 0)
				n_num++;
		}
		st_EquipTeamInfo.n32_InlayNum -= n_num;
		//st_EquipTeamInfo.n32_Rating -= pEquip->equipSpec.nRating;
	}
	
	//SetRating();

}

VOID Role::SetRating(INT32 nRating)
{

	st_EquipTeamInfo.n32_Rating += nRating;
	SendTeamEquipInfo();

	//st_EquipTeamInfo.n32_Rating = 0;
	//int nCur = 0;
	//// 附加属性
	//for (int i = 0; i < ERA_End; i++)
	//{
	//	EquipAddAtt eaa = ItemCreator::ERA2EAA((ERoleAttribute)i);
	//	if (eaa == EAA_NULL) continue;
	//	nCur = G_RATING_ATT[GetClass() - 1][eaa] * m_nAtt[i];
	//	st_EquipTeamInfo.n32_Rating += nCur;
	//}
	//
	//st_EquipTeamInfo.n32_Rating /= 1000;

}

VOID Role::SendTeamEquipInfo()
{
	NET_SIS_Team_Equip_Info send;
	send.dw_RoleID = GetID();
	memcpy(&send.st_EquipTeamInfo, &st_EquipTeamInfo, sizeof(st_EquipTeamInfo));
	g_groupMgr.SendTeamateMessage(GetTeamID(), GetID(), &send, send.dw_size);

	SendMessage(&send, send.dw_size);

	Team* pTeam = const_cast<Team*>(g_groupMgr.GetTeamPtr(GetTeamID()));
	if(VALID_POINT(pTeam))
	{
		pTeam->change_member_datas(this);
	}
	
}

VOID Role::ModAchievementPoint(INT nAdd, BOOL bSend)
{
	m_nAchievementPoint += nAdd;

	NET_SIS_achievement_point_change send;
	send.nNewValue = m_nAchievementPoint;
	SendMessage(&send, send.dw_size);
}

VOID Role::update_Reservation()
{
	if(!IsReservationPvP())
		return;

	dw_reservation_tick -= TICK_TIME;

	if(dw_reservation_tick <= 0)
	{
		dw_reservation_tick = 20000;
		SetReservationPvP(FALSE);
		SetReservationID(INVALID_VALUE);

		NET_SIS_reservation_timeout send;
		SendMessage(&send, send.dw_size);
	}
}

INT	Role::get_reservation_yuanbao_num(INT nLevel)
{
	INT nYuanBao = 0;
	if(nLevel >= 20 && nLevel <= 29)
	{
		nYuanBao = 5;
	}
	else if(nLevel >= 30 && nLevel <= 39)
	{
		nYuanBao = 10;
	}
	else if(nLevel >= 40 && nLevel <= 49)
	{
		nYuanBao = 15;
	}
	else if(nLevel >= 50 && nLevel <= 59)
	{
		nYuanBao = 20;
	}
	else if(nLevel >= 60 && nLevel <= 65)
	{
		nYuanBao = 25;
	}
	else if(nLevel >= 66 && nLevel <= 70)
	{
		nYuanBao = 30;
	}

	return nYuanBao * GOLD2SILVER;
}

// 约战申请
DWORD Role::reservation_apply(DWORD dw_role_id)
{
	if(dw_role_id == GetID())
		return INVALID_VALUE;

	if(IsReservationPvP())
		return E_Instance_have_reservation;

	if(get_level() < 30)
		return E_Instance_reservation_level_limit;

	if(IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Prictice) || IsDuel() || GetAutoKill() || IsDead())
		return E_Instance_reservation_state_limit;

	if(get_role_pvp().dw_pvp_id != INVALID_VALUE)
		return E_Instance_1v1_have_apply;

	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pMap->get_map_type() == EMT_Instance || pMap->get_map_type() == EMT_PVP || pMap->get_map_type() == EMT_1v1)
		return E_Instance_reservation_map_limit;

	Role* pReservation = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pReservation))
		return E_Instance_beservation_no_line;

	if(pReservation->get_role_pvp().dw_pvp_id != INVALID_VALUE)
		return E_Instance_bereservation_have_1v1_apply;

	if(abs(get_level()-pReservation->get_level()) > 10)
		return E_Instance_reservation_level_limit;

	INT n_yuanbao = get_reservation_yuanbao_num(pReservation->get_level());

	if(GetCurMgr().GetBagSilver() < n_yuanbao)
		return E_Instance_reservation_yuanbao_limit;

	if(pReservation->IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Prictice) || pReservation->IsDuel() || pReservation->GetAutoKill())
		return E_Instance_bereservation_state_limit;

	Map* pReservationMap = get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pReservationMap->get_map_type() == EMT_Instance || pReservationMap->get_map_type() == EMT_PVP || pReservationMap->get_map_type() == EMT_1v1)
		return E_Instance_bereservation_map_limit;

	if(pReservation->GetCurMgr().GetBagSilver() < n_yuanbao)
		return E_Instance_bereservation_yuanbao_limit;
	
	if (is_in_guild_war() || pReservation->is_in_guild_war())
	{
		return 	E_Instance_is_guild_war;
	}

	SetReservationPvP(TRUE);
	SetReservationID(pReservation->GetID());

	pReservation->SetReservationPvP(TRUE);
	pReservation->SetReservationID(GetID());

	NET_SIS_reservation_apply_notify send;
	send.dw_role_id = GetID();
	send.eClass = GetClass();
	send.nLevel = get_level();
	send.n_Equip_Scroe = GetEquipTeamInfo().n32_Rating;
	pReservation->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 约战回复
DWORD Role::reservation_result(DWORD	dw_role_id, BOOL b_ok)
{
	DWORD dw_error = E_Success;

	if(b_ok)
	{
		if(!IsReservationPvP())
			return E_Instance_reservation_time_limit;

		if(GetReservationID() != dw_role_id)
			return E_Instance_reservation_role_error;

		if(IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Prictice) || IsDuel() || GetAutoKill() ||IsDead())
			return E_Instance_reservation_state_limit;

		Map* pMap = get_map();
		if(!VALID_POINT(pMap))
			return INVALID_VALUE;

		if(pMap->get_map_type() == EMT_Instance || pMap->get_map_type() == EMT_PVP || pMap->get_map_type() == EMT_1v1)
			return E_Instance_reservation_map_limit;

		Role* pReservation = g_roleMgr.get_role(dw_role_id);
		if(!VALID_POINT(pReservation))
			return E_Instance_beservation_no_line;

		if(pReservation->GetReservationID() != GetID())
			return E_Instance_reservation_role_error;

		if(pReservation->IsInRoleStateAny(ERS_Stall | ERS_PrisonArea | ERS_Prictice) || pReservation->IsDuel() || pReservation->GetAutoKill() || pReservation->IsDead())
			return E_Instance_bereservation_state_limit;

		Map* pReservationMap = get_map();
		if(!VALID_POINT(pMap))
			return INVALID_VALUE;

		if(pReservationMap->get_map_type() == EMT_Instance || pReservationMap->get_map_type() == EMT_PVP || pReservationMap->get_map_type() == EMT_1v1)
			return E_Instance_bereservation_map_limit;

		INT n_yuanbao = get_reservation_yuanbao_num(get_level());

		if(GetCurMgr().GetBagSilver() < n_yuanbao)
			return E_Instance_bereservation_yuanbao_limit;

		if(pReservation->GetCurMgr().GetBagSilver() < n_yuanbao)
			return E_Instance_reservation_yuanbao_limit;

		dw_error = g_pvp_mgr.pair_reservation(this, pReservation, n_yuanbao);
	}
	else
	{
		NET_SIS_reservation_cancel send;
		//SendMessage(&send, send.dw_size);
		SetReservationPvP(FALSE);
		SetReservationID(INVALID_VALUE);

		Role* pReservation = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pReservation))
		{
			pReservation->SendMessage(&send, send.dw_size);
			pReservation->SetReservationPvP(FALSE);
			pReservation->SetReservationID(INVALID_VALUE);
		}
	}

	return dw_error;
}


VOID Role::logKick(DWORD dwCmdID, INT nParam)
{
	//NET_DB2C_log_kick send;
	//send.s_log_kick.dw_account_id = GetSession()->GetSessionID();
	//send.s_log_kick.dw_role_id = GetID();
	//send.s_log_kick.dw_cmd_id = dwCmdID;

	//g_dbSession.Send(&send, send.dw_size);

	NET_W2L_kick_log send;
	send.dw_account_id	= GetSession()->GetSessionID();
	send.dw_kick_time	= g_world.GetWorldTime();
	send.u_16err_code	= dwCmdID;
	send.by_seal		= nParam;			
	send.dw_role_level  = get_level();
	
	s_role_info* pRoleInfo = g_roleMgr.get_role_info(GetID()); 
	if (VALID_POINT(pRoleInfo))
	{
		get_tool()->unicode_to_unicode8(pRoleInfo->sz_role_name, send.szRoleName);
	}

	get_tool()->unicode_to_unicode8(g_world.GetWorldName(), send.szWorldName);


	g_loginSession.Send(&send, send.dw_size);
}

VOID Role::updata_delay()
{
	if (AttRes::GetInstance()->GetVariableLen().n_kick_fast_move)
	{
		if(CalcTimeDiff(g_world.GetWorldTime(), dw_check_delay_time) >= g_world.GetWeakCheckTime())
		{
			if(n_delay_num > g_world.GetWeakCheckNum())
			{
				logKick(elci_kick_role_move_fast, n_delay_num);

				//SI_LOG->write_log(_T("kick_move_fast:%d \r\n"), n_delay_num);
				//g_worldSession.Kick(GetSession()->GetInternalIndex());
				//GetSession()->SetKicked();

				if(VALID_POINT(GetScript()))
				{
					GetScript()->OnFastCheck(this, elci_kick_role_move_fast);
				}
			}
			n_delay_num = 0;
			dw_check_delay_time = g_world.GetWorldTime();
		}

		if(CalcTimeDiff(g_world.GetWorldTime(), dw_strength_delay_time) >= g_world.GetStrengthCheckTime())
		{
			if(n_strength_delay_num > g_world.GetStrengthCheckNum())
			{
				logKick(elci_kick_role_move_fast1, n_strength_delay_num);

				//SI_LOG->write_log(_T("kick_move_fast1:%d \r\n"), n_strength_delay_num);

				//g_worldSession.Kick(GetSession()->GetInternalIndex());
				//GetSession()->SetKicked();

				if(VALID_POINT(GetScript()))
				{
					GetScript()->OnFastCheck(this, elci_kick_role_move_fast1);
				}

				n_delay_record_num = 0;
			}

			if(n_ce_check_num > g_world.GetCeCheckNum())
			{
				logKick(elci_kick_role_move_fast2, n_ce_check_num);
				
				//SI_LOG->write_log(_T("kick_move_fast2:%d \r\n"), n_ce_check_num);

				//g_worldSession.Kick(GetSession()->GetInternalIndex());
				//GetSession()->SetKicked();

				if(VALID_POINT(GetScript()))
				{
					GetScript()->OnFastCheck(this, elci_kick_role_move_fast2);
				}

				n_delay_record_num = 0;
			}

			n_strength_delay_num = 0;
			n_ce_check_num = 0;
			n_delay_record_num++;
			dw_strength_delay_time = g_world.GetWorldTime();
		}
	}

}

VOID Role::SetPKValueMod(INT nValue)
{
	if(m_iPKValue <= 0) m_dwPKValueDecTime = g_world.GetWorldTime();
	m_iPKValue += nValue;
	if( m_iPKValue > PK_VALUE_MAX ) m_iPKValue = PK_VALUE_MAX;
	if( m_iPKValue < 0 ) m_iPKValue = 0;

	//if(nValue > 0 && m_iPKValue == PK_VALUE_MAX) 
	//	mIsPurpureDec = TRUE;
	//else if(nValue < 0 && m_iPKValue < 85)
	//	mIsPurpureDec = FALSE;

	//GetAchievementMgr().UpdateAchievementCriteria(ete_PKValue, m_iPKValue);

	if(VALID_POINT(get_map()))
	{
		NET_SIS_change_pk_value send;
		send.dw_role_id = GetID();
		send.iPKValue = GetClientPKValue();
		//send.bIsPurpureDec = mIsPurpureDec;
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);

		if(/*IsPurpureName( ) &&*/ m_iPKValue >= PK_VALUE_MAX){
			Vector3 pos = GetCurPos( );
			HearSayHelper::SendMessage(EHST_PURPUREPOS, GetID(), pos.x, pos.y, pos.z, GetMapID());
		}
	}
}

// 获取角色签到数据
DWORD Role::get_sign_data()
{
	if(!g_sign_mgr.is_open())
		return SIGN_NOT_OPEN;

	const tagSignProto* pSignProto = AttRes::GetInstance()->GetSignProto(g_sign_mgr.get_act_id());
	if(!VALID_POINT(pSignProto))
		return INVALID_VALUE;

	if(st_role_sign_data.st_sign_info[0].st_time.year == 0)
	{
		for(INT i = 0; i < pSignProto->n_keep_time; i++)
		{
			if(i == 0)
			{
				tagDWORDTime st_time(0, 0, 0, g_sign_mgr.get_begin_time().day, g_sign_mgr.get_begin_time().month, g_sign_mgr.get_begin_time().year);

				st_role_sign_data.st_sign_info[i].st_time = st_time;
			}
			else
			{
				st_role_sign_data.st_sign_info[i].st_time = AddDay(st_role_sign_data.st_sign_info[i-1].st_time);
			}
		}
	}

	NET_SIS_get_sign_data send;
	memcpy(&send.st_role_sign_data, &st_role_sign_data, sizeof(tagRoleSignData));
	send.dw_error = E_Success;

	SendMessage(&send, send.dw_size);

	return E_Success;
}

// 签到
DWORD Role::role_sign(BOOL b_buqian, DWORD dwDate)
{
	if(!g_sign_mgr.is_open())
		return SIGN_NOT_OPEN;

	const tagSignProto* pSignProto = AttRes::GetInstance()->GetSignProto(g_sign_mgr.get_act_id());
	if(!VALID_POINT(pSignProto))
		return INVALID_VALUE;
	
	if (get_level() < AttRes::GetInstance()->GetVariableLen().nSignLevel)
		return SIGN_LEVEL_NOT;

	INT n_index = 0;
	BOOL bFind = FALSE;

	if(b_buqian)
	{
		if (st_role_sign_data.n16_mianqian_time >= MAX_VIP_MIANQIAN_TIME)
		{
			return SIGN_BUQIAN_NUM;
		}
		if (GET_VIP_EXTVAL(GetTotalRecharge(), VIP_LEVEL, INT) <= 0)
		{
			return SIGN_BUQIAN_VIP;
		}
		tagDWORDTime st_time = dwDate;

		if (CalcTimeDiff(g_world.GetWorldTime(), st_time) < 0)
		{
			return SIGN_BUQIANERROR;
		}

		for(INT i = 0; i < pSignProto->n_keep_time; i++)
		{
			if(st_role_sign_data.st_sign_info[i].st_time.year == st_time.year &&
				st_role_sign_data.st_sign_info[i].st_time.day == st_time.day &&
				st_role_sign_data.st_sign_info[i].st_time.month == st_time.month)
			{
				if(st_role_sign_data.st_sign_info[i].b_sign)
				{
					return SIGN_HAVE_SIGN;
				}
				st_role_sign_data.st_sign_info[i].b_sign = TRUE;
				n_index = i;
				bFind = TRUE;
				st_role_sign_data.n16_mianqian_time++;
				break;
			}
		}
		
		if(!bFind)
			return SIGN_SYSTEM_ERROR;

	}
	else
	{
		tagDWORDTime st_cur_time = g_world.GetWorldTime();

		for(INT i = 0; i < pSignProto->n_keep_time; i++)
		{
			if(st_role_sign_data.st_sign_info[i].st_time.year == st_cur_time.year &&
				st_role_sign_data.st_sign_info[i].st_time.day == st_cur_time.day &&
				st_role_sign_data.st_sign_info[i].st_time.month == st_cur_time.month)
			{
				if(st_role_sign_data.st_sign_info[i].b_sign)
				{
					return SIGN_HAVE_SIGN;
				}
				st_role_sign_data.st_sign_info[i].b_sign = TRUE;
				n_index = i;
				bFind = TRUE;
				break;
			}
		}

		if(!bFind)
			return SIGN_SYSTEM_ERROR;
	}

	NET_SIS_sign send;
	send.st_sign_time = st_role_sign_data.st_sign_info[n_index].st_time;
	send.dw_error = E_Success;
	SendMessage(&send, send.dw_size);
	
	if (VALID_POINT(m_pScript))
	{
		m_pScript->OnSign(this);
	}

	return E_Success;
}

// 领取签到奖励
DWORD Role::role_sign_reward(INT n_index)
{
	if(!g_sign_mgr.is_open())
		return SIGN_NOT_OPEN;

	const tagSignProto* pSignProto = AttRes::GetInstance()->GetSignProto(g_sign_mgr.get_act_id());
	if(!VALID_POINT(pSignProto))
		return INVALID_VALUE;

	if(n_index < 0 || n_index >= pSignProto->n_reward_num)
		return INVALID_VALUE;

	if(st_role_sign_data.b_reward[n_index])
	{
		return SIGN_PEPEAT_REWARD;
	}

	INT n_condition = pSignProto->st_reward_data[n_index].n_condition;
	INT n_sign_num = 0;
	for(INT i = 0; i < pSignProto->n_keep_time; i++)
	{
		if(st_role_sign_data.st_sign_info[i].b_sign)
		{
			n_sign_num++;
		}
	}

	if(n_sign_num >= n_condition)
	{
		if(GetItemMgr().GetBagFreeSize() <= 0)
		{
			return SIGN_BAG_FULL;
		}

		tagItem* pItem = ItemCreator::Create(EICM_Sign,  m_dwID, pSignProto->st_reward_data[n_index].dw_reward_id, 1, pSignProto->st_reward_data[n_index].bBind, m_dwID);

		if( VALID_POINT(pItem) )
		{
			GetItemMgr().Add2Bag(pItem, elci_sign, TRUE);
		}

		st_role_sign_data.b_reward[n_index] = TRUE;

		return E_Success;

	}
	else
	{
		return INVALID_VALUE;
	}

	
}

VOID Role::UpdateCoolDownRevive( )
{
	if(!IsInState(ES_Dead))
		return;

	if(mCoolDownReviveTick <= 0 && m_CoolDownReviveTick2 <= 0)
		return;

	if( mCoolDownReviveTick > 0 ){
		--mCoolDownReviveTick;
		if(mCoolDownReviveTick > 0) return;
	}

	if( m_CoolDownReviveTick2 > 0 ){
		--m_CoolDownReviveTick2;
		if(m_CoolDownReviveTick2 > 0) return;
	}

//	if(mCoolDownReviveTick == 0)
	{
		const tag_map_info* pMapInfo = g_mapCreator.get_map_info(GetMapID());
		if(pMapInfo->e_type == EMT_Guild) // 如果是家族副本地图
		{
			NET_SIS_role_revive send;
			send.dw_role_id		= GetID();
			send.dw_error_code	= Revive(ERRT_ReturnCity, INVALID_VALUE, FALSE);

			if( VALID_POINT(get_map()) )
			{
				get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
			}

			if(E_Success == send.dw_error_code)
			{
				// 复活到指定地点
				GotoNewMap(m_Revive.dwMapID,m_Revive.fX, m_Revive.fY, m_Revive.fZ);
			}	
		}
		else
		{
			if( CoolDownRevive( ) != E_Success)
				Revive(ERRT_Normal, INVALID_VALUE, FALSE);
		}
		
	}
}

VOID Role::StartCoolDownRevive(  )
{
	if(!IsInState(ES_Dead))
		return;
	
	if( get_map()->get_map_info()->e_type == EMT_Guild ) {
		m_CoolDownReviveTick2 = GUILD_WAR_CD_REBORN_TIME * TICK_PER_SECOND;
	} else if( !get_map()->get_map_info()->b_AddCooldownTime ){
		m_CoolDownReviveTick2 = 25 * TICK_PER_SECOND;
	} else {
		mCoolDownReviveTick = mCoolDownReviveCD * TICK_PER_SECOND;
		mCoolDownReviveCD += 5;
	}
	
}

INT Role::GetCoolDownReviveCD( )
{
	if( VALID_POINT( get_map() ) && VALID_POINT(get_map()->get_map_info()))
	{	
		if( get_map()->get_map_info()->e_type == EMT_Guild )
			return GUILD_WAR_CD_REBORN_TIME;
		else if( !get_map()->get_map_info()->b_AddCooldownTime )
			return 25;
	}

	return mCoolDownReviveCD;
}

/* 挂机长经验*/
DWORD Role::StartHangGetExp(INT type)
{
	if(get_level() < GETEXP_MIN_LEVEL)
		return E_HangGetExp_OutOfLevel;


	if(IsInRoleStateAny(ERS_Prictice | ERS_Combat | ERS_Carry | ERS_Mount | ERS_Mount2) )
		return E_HangGetExp_StateLimit;

	if(IsInRoleStateAny(ERS_KongFu | ERS_Comprehend | ERS_Dancing))
		return E_HangGetExp_StateLimit;

	switch(type){
		case 1:
		case 2:{
			if(type == 1 && !IsInRoleStateAny(ERS_KongFuArea)) return E_HangGetExp_StateLimit;
			if(GetAttValue(ERA_Fortune) < HUANG_DEC_VIGOUR_VAL) return E_HangGetExp_OutOfVigour;
			m_HangDecVigourTick = HUANG_DEC_VIGOUR_TICK;
			ModAttValue(ERA_Fortune, 0 - HUANG_DEC_VIGOUR_VAL);
			SetRoleState(type == 1 ? ERS_KongFu : ERS_Comprehend);
			break;
		}
		case 3: {
			if( IsInRoleState(ERS_DancingArea) ){
				if(mPerDayHangGetExpTimeMS <= 0) {
					return E_HangGetExp_OutOfTime;
				}
				SetRoleState(ERS_Dancing);		
				SendDacingLeftTime( );
			} else {
				return E_HangGetExp_StateLimit;
			}
			break;
		}
		default: {
			return E_HangGetExp_StateLimit; 
			break;
		}
	}


	mGetExpTickUpdate =	GETEXP_TIME_TICK;
	if(VALID_POINT(m_pScript)) m_pScript->OnHang(this, type);
	return E_Success;
}
DWORD Role::StopHangGetExp( )
{
	if(!IsInRoleStateAny(ERS_KongFu | ERS_Comprehend | ERS_Dancing))
		return INVALID_VALUE;

	if( IsInRoleState(ERS_KongFu) )		UnsetRoleState(ERS_KongFu);
	if( IsInRoleState(ERS_Comprehend))  UnsetRoleState(ERS_Comprehend);
	if( IsInRoleState(ERS_Dancing)) { UnsetRoleState(ERS_Dancing); SendDacingLeftTime( ); }
		
	return E_Success;
}

VOID Role::UpdateHangGetExp( )
{
	if(!IsInRoleStateAny(ERS_KongFu | ERS_Comprehend | ERS_Dancing))
		return ;


	--mGetExpTickUpdate;
	--m_HangDecVigourTick;
	if(mGetExpTickUpdate > 0) return;

	mGetExpTickUpdate = GETEXP_TIME_TICK;

	if( IsInRoleState(ERS_KongFu) )
	{
		if(!IsInRoleStateAny(ERS_KongFuArea) || GetAttValue(ERA_Fortune) < HUANG_DEC_VIGOUR_VAL){
			StopHangGetExp( );
			return ;
		}
		/* 经验=15+(角色等级/2)[取整]	*/
		INT addExp = 15 + get_level() / 2;
		//ExpChange(addExp);
		UpdateHangVigour( );
	}
	
	if( IsInRoleState(ERS_Comprehend))
	{
		if( GetAttValue(ERA_Fortune) < HUANG_DEC_VIGOUR_VAL) {
			StopHangGetExp( );
			return ;
		}
		// 神识=5+（角色等级/3）[取整]	
		INT addBrotherHood = 5 + get_level() / 3;
		ChangeBrotherhood(addBrotherHood);
		UpdateHangVigour( );
	}

	if( IsInRoleState(ERS_Dancing))
	{
		if(!IsInRoleState(ERS_DancingArea)){
			StopHangGetExp( );
			return ;
		}

		// 经验=1+(角色等级/3)[取整]	
		// 神识=1+（角色等级/10）[取整]	
		FLOAT rate = GetDancingRate( );
		INT addExp = 1 + get_level() / 3;
		addExp = (INT)(addExp * rate);
		//ExpChange(addExp);

		INT addBrotherHood = 1 + get_level() / 10;
		addBrotherHood = (INT)(addBrotherHood * rate);
		ChangeBrotherhood(addBrotherHood);

		mPerDayHangGetExpTimeMS -= (GETEXP_TIME_TICK * TICK_TIME);
		if(mPerDayHangGetExpTimeMS <= 0){
			mPerDayHangGetExpTimeMS = 0;
			StopHangGetExp( );
		} 
	}
}

FLOAT Role::GetDancingRate( )
{	
	FLOAT rate = 1.0f;

	if( IsHaveBuff( 51006 ) ){
		// 额外100%
		rate += 1.0f;
	}

	if( IsHaveBuff( 51007 ) ){
		// 额外200%
		rate += 2.0f;
	}

	return rate * g_GMPolicy.GetDancingRate( );
}

VOID Role::SendDacingLeftTime( )
{
	NET_SIS_start_hang_time_left send;
	send.dwLeftMilliseconds = mPerDayHangGetExpTimeMS;
	SendMessage(&send, send.dw_size);
}


VOID Role::UpdateHangVigour( )
{
	if(m_HangDecVigourTick <= 0){
		ModAttValue(ERA_Fortune, 0 - HUANG_DEC_VIGOUR_VAL);
		m_HangDecVigourTick = HUANG_DEC_VIGOUR_TICK;
	}
}


DWORD Role::GodLevelUp()
{
	//if (getGodLevel() <= 0)
	//	return E_GOD_LEVEL_UP_IS_ZERO;
	//
	////判断条件是否满足
	//const tagGodLevelProto* pProto = AttRes::GetInstance()->GetGodLevelProto(m_nGodLevel+1);
	//if (!VALID_POINT(pProto))
	//	return E_GOD_LEVEL_NOT_PROTO;

	////角色等级
	//if (get_level() < pProto->nCondition[EGLC_ROLE_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;
	//
	////武器等级
	//tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	//if(!VALID_POINT(pEquip) && pProto->nCondition[EGLC_WEAPON_LEVEL] != 0)
	//	return E_GOD_LEVEL_UP_NOT_CON;

	//if (VALID_POINT(pEquip) && pEquip->equipSpec.nLevel < pProto->nCondition[EGLC_WEAPON_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;

	////徽章等级
	//pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Shipin1);
	//if(!VALID_POINT(pEquip) && pProto->nCondition[EGLC_HUIZHANG_LEVEL] != 0)
	//	return E_GOD_LEVEL_UP_NOT_CON;

	//if (VALID_POINT(pEquip) && pEquip->equipSpec.nLevel < pProto->nCondition[EGLC_HUIZHANG_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;
	//
	////腰佩等级
	//pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Shipin1);
	//if(!VALID_POINT(pEquip) && pProto->nCondition[EGLC_YAPPEI_LEVEL] != 0)
	//	return E_GOD_LEVEL_UP_NOT_CON;

	//if (VALID_POINT(pEquip) && pEquip->equipSpec.nLevel < pProto->nCondition[EGLC_YAPPEI_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;
	//
	////宠物等级
	//if (GetPetPocket()->GetMaxPetLevel() < pProto->nCondition[EGLC_PET_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;

	////魂练次数

	////拥有魂精等级
	//if (getMaxHunJingLevel() < pProto->nCondition[EGLC_HUENJIN_LEVEL])
	//	return E_GOD_LEVEL_UP_NOT_CON;
	//
	//for (int i = 0; i < 2; i++)
	//{
	//	DWORD dwSkillID =  pProto->dwSkill[GetClass() - 1][i];
	//	const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillID*100 + 1);
	//	if (VALID_POINT(pSkillProto))
	//	{
	//		Skill* pSkill = new Skill(dwSkillID, 0, 1, 0, 0);
	//		AddSkill(pSkill);
	//		if (VALID_POINT(GetScript()))
	//		{
	//			GetScript()->OnLearnGodSkill(this, dwSkillID);
	//		}
	//	}
	//}



	//m_nGodLevel++;

	return E_Success;
}

INT	Role::getMaxHunJingLevel()
{
	INT nMax = 0;
	for (int i = 0; i < MAX_ROLE_HUENJING_SIZE; i++)
	{
		INT nLevel = st_role_huenjing_data.s_huenjing_role_level[i].nLevel;
		if (nLevel > nMax)
		{
			nMax = nLevel;
		}
		
	}

	for (int i = 0; i < MAX_ROLE_HUENJING_SIZE; i++)
	{
		INT nLevel = st_role_huenjing_data.s_huenjing_role_titel[i].nLevel;
		if (nLevel > nMax)
		{
			nMax = nLevel;
		}
	}
	
	return nMax;
}

VOID Role::removeFlowCreature()
{
	if (VALID_POINT(get_map()))
	{
		Creature* pFlow = get_map()->find_creature(GetFlowUnit());
		if (VALID_POINT(pFlow))
		{
			SetFlowUnit(INVALID_VALUE);
			if (VALID_POINT(pFlow->GetAI()) && VALID_POINT(pFlow->GetAI()->GetTracker()))
			{
				pFlow->GetAI()->GetTracker()->SetTarget(NULL);
				pFlow->GetAI()->GetTracker()->SetTargetID(INVALID_VALUE);
			}
			
			pFlow->OnDisappear();
		}
		else if(GetFlowUnit() != INVALID_VALUE)
		{
			SI_LOG->write_log(_T("cant delete flow unit!!"));
		}
	}

}

VOID Role::SetFlowUnit(DWORD dwID) 
{ 
	m_dwFlowUnit = dwID;
	NET_SIS_follow_creature_change send;
	send.n64ID = dwID;
	SendMessage(&send, send.dw_size);
}

// 累计上线天数奖励
DWORD Role::role_get_sign_reward()
{
	if (GetDayClearData(ERDCT_Sign_Nubmer) >= 1)
		return INVALID_VALUE;
	
	DWORD dwItem[] = {1500001,1500002,1500003,1500004,1500005,1500006,1500007};
	const tagItemProto* pProto = AttRes::GetInstance()->GetItemProto(dwItem[getGodLevel()]);
	if (!VALID_POINT(pProto))
		return false;

	// 生成物品
	tagItem* pItem = ItemCreator::CreateEx(EICM_Activity, INVALID_VALUE, pProto->dw_data_id, 1, EIQ_Quality0);
	if( !VALID_POINT(pItem) ) return false;

	DWORD dwRtv = GetItemMgr().Add2Bag(pItem, elcid_wu_se_shi, TRUE);

	if(E_Success != dwRtv)
	{
		SAFE_DELETE(pItem);
		return false;
	}

	ModRoleDayClearDate(ERDCT_Sign_Nubmer);
	return E_Success;
}
//玩家每日分时段免费抽奖计数
DWORD Role::role_get_free_gamble()
{
	//分3个时间段：6-12,12-18,18-23
	// 当前的时间
	tagDWORDTime cur_time = GetCurrentDWORDTime();
	//6-12
	if (cur_time.hour >= 6 && cur_time.hour < 12)
	{
		if (!(m_byDayClear[ERDCT_FreeGamble_Number] & 0x01))
		{
			m_byDayClear[ERDCT_FreeGamble_Number] |= 0x01;
			return E_Success;
		}
		return INVALID_VALUE;
	}
	//12-18
	else if (cur_time.hour >= 12 && cur_time.hour < 18)
	{
		if (!(m_byDayClear[ERDCT_FreeGamble_Number] & 0x02))
		{
			m_byDayClear[ERDCT_FreeGamble_Number] |= 0x02;
			return E_Success;
		}
		return INVALID_VALUE;
	}
	//18-23
	else if (cur_time.hour >= 18 && cur_time.hour < 23)
	{
		if (!(m_byDayClear[ERDCT_FreeGamble_Number] & 0x04))
		{
			m_byDayClear[ERDCT_FreeGamble_Number] |= 0x04;
			return E_Success;
		}
		return INVALID_VALUE;
	}
	else
	{
		return INVALID_VALUE;
	}
}
//玩家当日付费抽奖计数
DWORD Role::role_get_normal_gamble()
{
	//获取当前等级下玩家总共可抽将的次数
	//普通玩家可抽奖5次，银贵vip10次，白金vip12次，至尊vip15次
	const INT normalPlayerGambleTimes = 10;
	INT gambleTimesPerday = normalPlayerGambleTimes+GET_VIP_EXTVAL(GetVIPLevel(),TIANMING_ADD,INT);//后续增加VIP接口
	if (GetDayClearData(ERDCT_FinishedGamble_Number) >= gambleTimesPerday)
		return INVALID_VALUE;
	ModRoleDayClearDate(ERDCT_FinishedGamble_Number,1,FALSE);
	return E_Success;
}
//魔物狩猎
DWORD Role::role_get_daily_quest_mowu()
{
	INT quest_times = DAILYQUESTMOWU+GET_VIP_EXTVAL(GetVIPLevel(),MOWUSHOULIE_ADD,INT);
	if (GetDayClearData(ERDCT_MoWuShouLie_Number) >= quest_times)
		return INVALID_VALUE;
	
	return E_Success;
}
VOID Role::role_set_daily_quest_mowu()
{
	ModRoleDayClearDate(ERDCT_MoWuShouLie_Number,1,FALSE);
}
//修罗试炼
DWORD Role::role_get_daily_quest_xlsl()
{
	INT quest_times = DAILYQUESTXIULU+GET_VIP_EXTVAL(GetVIPLevel(),XIULUOSHILIAN_ADD,INT);
	if (GetDayClearData(ERDCT_QuestRefresh) >= quest_times)
		return INVALID_VALUE;
	
	return E_Success;
}
VOID Role::role_set_daily_quest_xlsl()
{
	ModRoleDayClearDate(ERDCT_QuestRefresh,1,FALSE);
}
//双修给经验，gx add 2013.6.28
VOID Role::UpdateComPracticeExp()
{
	//不在双修中，直接退出
	if (!IsInRoleState(ERS_ComPractice))
		return;
	if (m_PracticeTick >= COMPRACTICE_WHOLE_TICKS)
	{
		UnsetRoleState(ERS_ComPractice);
		m_PracticeTick = 0;
		SetComPracticePartner(INVALID_VALUE);
		return;
	}
	m_PracticeTick++;
	//每隔5秒给经验
	if (m_PracticeTick % COMPRACTICE_EXP_TICKS == 0)
	{
		DWORD comPracticePartner = GetComPracticePartner();
		Role* pComPracticePartner = g_roleMgr.get_role(comPracticePartner);
		if( !VALID_POINT(pComPracticePartner) )
		{
			UnsetRoleState(ERS_ComPractice);
			m_PracticeTick = 0;
			SetComPracticePartner(INVALID_VALUE);
			return;
		}

		if (!pComPracticePartner->IsInRoleState(ERS_ComPractice))
		{
			UnsetRoleState(ERS_ComPractice);
			m_PracticeTick = 0;
			SetComPracticePartner(INVALID_VALUE);
			return;
		}
		INT nValue = ((abs(GetComPracticeLevel() - pComPracticePartner->GetComPracticeLevel())*0.01+1)*GetComPracticeLevel()*5000)/36;
		//夫妻双修加成 gx add 2013.10.24
		FLOAT fCoe = 1.0f;
		if (GetSpouseID() != (DWORD)INVALID_VALUE)
		{
			if (GetSpouseID() == comPracticePartner)
			{
				fCoe = 1.3f;
			}
		}
		nValue = (INT)(nValue*fCoe);
		//end
		ExpChange(nValue);
		
	}
}
VOID Role::UpdateVIPTime()
{
	if (GetVIPLevel() <= 0)//不是VIP玩家
		return;
	m_VIP_RefreshTick++;
	//每隔1分钟更新一次
	if (0 == m_VIP_RefreshTick % 300)
	{
		DWORD dwtime = CalcTimeDiff(GetVIPDeatLine(),GetCurrentDWORDTime());
		if (dwtime <= 0)//若VIP到期
		{
			SetVIPLevel(0);
			m_VIP_DeadLine.Clear();
			//向数据库发消息
			NET_C2DB_clear_role_VIP_info send;
			send.dw_role_id = GetID();
			g_dbSession.Send(&send, send.dw_size);
			//向客户端发消息
			NET_SIS_unset_vip_level cmd;
			cmd.dw_role_id = GetID();
			SendMessage(&cmd,cmd.dw_size);
		}
	}
	return;
}
VOID Role::CheckVip_OffLine()
{
	if (GetVIPLevel() <= 0)//不是VIP玩家
		return;
	DWORD dwtime = CalcTimeDiff(GetVIPDeatLine(),GetCurrentDWORDTime());
	if (dwtime <= 0)//若VIP到期
	{
		SetVIPLevel(0);
		m_VIP_DeadLine.Clear();
		//向数据库发消息
		NET_C2DB_clear_role_VIP_info send;
		send.dw_role_id = GetID();
		g_dbSession.Send(&send, send.dw_size);
		//向客户端发消息
		NET_SIS_unset_vip_level cmd;
		cmd.dw_role_id = GetID();
		SendMessage(&cmd,cmd.dw_size);
	}
	return;
}
VOID Role::day_clear_circle_quest_done_times()
{
	tagQuestDoneSave* p_done = NULL;
	QuestDoneMap::map_iter it = done_quest_.begin();
	while( done_quest_.find_next(it, p_done) )
	{	
		if (!VALID_POINT(p_done))
			continue;
		const tagQuestProto* p_proto = g_questMgr.get_protocol(p_done->u16QuestID);
		if( !VALID_POINT(p_proto) )	
			continue;
		//若是每日任务
		if (p_proto->period && EQuestPeriodic_DAY == p_proto->period_type)
		{
			if (p_done->nTimes > 0)
			{
				//已完成的每日任务则直接删除
				done_quest_.erase(p_done->u16QuestID);
				SAFE_DELETE(p_done);
				//p_done->nTimes = 0;
				// 发给数据库保存
				/*NET_DB2C_complete_quest send;
				send.dw_role_id = GetID();
				send.quest_done_.u16QuestID = p_done->u16QuestID;
				send.quest_done_.dwStartTime = GetCurrentDWORDTime();
				send.quest_done_.nTimes = p_done->nTimes;
				g_dbSession.Send(&send, send.dw_size);*/
			}
		}
	}
	//未完成的每日任务也要清空
	quest* p_get = NULL;
	QuestMap::map_iter iter_get = current_quest_.begin();
	while (current_quest_.find_next(iter_get,p_get))
	{
		if(!VALID_POINT(p_get))
			continue;
		const tagQuestProto* p_proto = g_questMgr.get_protocol(p_get->get_id());
		if(!VALID_POINT(p_proto))	
			continue;
		//若是每日任务
		if (p_proto->period && EQuestPeriodic_DAY == p_proto->period_type)
		{
			INT n_ret = delete_quest(p_get->get_id());
			//向客户端发消息
			NET_SIS_delete_quest send;
			send.u16QuestID = p_proto->id;
			send.dw_error_code = E_Success;
			SendMessage(&send, send.dw_size);
		}
	}
	//统一向客户端发送更新全部任务的消息
	NET_SIS_delete_quest send;
	send.u16QuestID = 0;
	send.dw_error_code = E_Success;
	SendMessage(&send, send.dw_size);
}

INT Role::GetClientPKValue() const
{
	if (GetPKValue() <= 0)
		return 0;
	else 
		return GetPKValue() / ATTACK_WHITENAME_INCPK + 1;
}
