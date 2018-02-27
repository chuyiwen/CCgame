
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ϸ�����������ݿ��������ͨ��
#include "Stdafx.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/yuanbao_server_define.h"
#include "../common/ServerDefine/famehall_server_define.h"
#include "../common/ServerDefine/guild_server_define.h"
#include "../../common/WorldDefine/select_role_protocol.h"
#include "../../common/WorldDefine/mall_protocol.h"
#include "../common/ServerDefine/pet_server_define.h"
#include "../common/ServerDefine/mall_server_define.h"
#include "../common/ServerDefine/vcard_server_define.h"
#include "../common/ServerDefine/vipstall_server_define.h"
#include "../common/ServerDefine/vip_netbar_server_define.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "../../common/WorldDefine/role_card_protocol.h"
#include "../common/ServerDefine/activity_server_define.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/quest_server_define.h"
#include "../common/ServerDefine/master_apprentice_server_define.h"
#include "../common/ServerDefine/rank_server_define.h"
#include "../../common/WorldDefine/mail_protocol.h"
#include "../common/ServerDefine/paimai_server_define.h"
#include "../common/ServerDefine/bank_server_define.h"
#include "../common/ServerDefine/guerdon_quest_protocol.h"
#include "../common/ServerDefine/common_server_define.h"
#include "db_session.h"
#include "world.h"
#include "world_session.h"
#include "role.h"
#include "role_mgr.h"
#include "att_res.h"
#include "item_creator.h"
#include "TradeYuanBao.h"
#include "famehall.h"
#include "guild.h"
#include "guild_manager.h"
#include "pet_pocket.h"
#include "pet_soul.h"
#include "mall.h"
#include "vip_stall.h"
#include "activity_mgr.h"
#include "vip_netbar.h"
#include "chat_mgr.h"
#include "mail_mgr.h"
#include "master_prentice_mgr.h"
#include "RankMgr.h"
#include "mail_mgr.h"
#include "paimai_manager.h"
#include "bank_manager.h"
#include "guerdon_quest_mgr.h"
#include "pet_sns_mgr.h"
#include "center_session.h"
#include "auto_paimai.h"
//-----------------------------------------------------------------------------
// ���캯��
//-----------------------------------------------------------------------------
DBSession g_dbSession;

DBSession::DBSession()
{
	m_dwDBPort	= INVALID_VALUE;
	m_dwGoldenCode	= INVALID_VALUE;
	m_bTermConnect	= FALSE;

	m_bInitOK = FALSE;

	ZeroMemory(m_szDBIP, sizeof(m_szDBIP));
	//ZeroMemory(m_szServerName, sizeof(m_szServerName));
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DBSession::~DBSession()
{
}

//-----------------------------------------------------------------------------
// ��ʼ������
//-----------------------------------------------------------------------------
BOOL DBSession::Init()
{
	// �ò��ֳ�Ա���¸�ֵ����Ϊ���������һ��ȫ�ֱ�����
	

	// ����net_command_manager
	m_pTran = new few_connect_client;
	if(!VALID_POINT(m_pTran))
	{
		ERROR_CLUE_ON(_T("Create ToLoonDB(few_connect_client) obj failed!"));
		return FALSE;
	}
	m_pTran->init();

	// ��ȡ�ļ�, ��ʼ����Ա
	if(!InitConfig())
	{
		ERROR_CLUE_ON(_T("Init File Read Failed! Please Check......"));
		return FALSE;
	}

	// ע��������������
	RegisterAllDBCommand();

	// ���������߳�
	if(!World::p_thread->create_thread(_T("ConnectSTDB"), 
		&DBSession::static_thread_connect, this))
	{
		return FALSE;
	}

	while(!World::p_thread->is_thread_active(_T("ConnectSTDB")))
	{
		continue;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��ȡ�ļ�, ��ʼ����Ա
//-----------------------------------------------------------------------------
BOOL DBSession::InitConfig()
{
	if(!VALID_POINT(World::p_var))
	{
		ERROR_CLUE_ON(_T("Create STDBVar(VarContainer) obj failed!"));
		return FALSE;
	}

	// ��ȡ���ݿ�������Ķ˿ںźͽ�ɫ����
	m_dwDBPort	= World::p_var->get_dword(_T("port db_server"));
	m_dwGoldenCode	= World::p_var->get_dword(_T("golden_code db_server"));

	// ��ȡ������Ϸ����Ļ�����
	/*CHAR szName[LONG_STRING];
	strncpy(szName, m_pTran->GetHostName(), LONG_STRING);*/
	//_tcsncpy(m_szServerName, get_tool()->unicode8_to_unicode(szName), cal_tchar_array_num(m_szServerName) - 1);

	// ��ȡ���ݷ�����IP��ַ
	TCHAR szIP[X_IP_LEN];
	_tcsncpy(szIP, World::p_var->get_string(_T("ip db_server")), cal_tchar_array_num(szIP) - 1);
	get_fast_code()->memory_copy(m_szDBIP, get_tool()->unicode_to_unicode8(szIP), sizeof(m_szDBIP) - 1);


	return TRUE;
}

//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID DBSession::Destroy()
{
	// �ȴ������߳̽���
	Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
	World::p_thread->waitfor_thread_destroy(_T("ConnectSTDB"), INFINITE);

	m_pTran->destory();
	SAFE_DELETE(m_pTran);

	// ���ע�����Ϣ
	UnRegisterAllDBCommand();
	
	
}

//-----------------------------------------------------------------------------
// update
//-----------------------------------------------------------------------------
VOID DBSession::Update()
{
	if( NULL == m_pTran )
		return;

	if(!m_pTran->is_connect() && !World::p_thread->is_thread_active(_T("ConnectSTDB")))
	{
		Interlocked_Exchange((LONG*)&m_bTermConnect, TRUE);
		m_pTran->disconnect();

		World::p_thread->waitfor_thread_destroy(_T("ConnectSTDB"), INFINITE);

		// �����������ݿ�����������߳�
		Interlocked_Exchange((LONG*)&m_bTermConnect, FALSE);
		World::p_thread->create_thread(_T("ConnectSTDB"), &DBSession::static_thread_connect, this);

		while(FALSE == World::p_thread->is_thread_active(_T("ConnectSTDB")))
		{
			continue;
		}

		return;
	}

	// �����������ݿ����������Ϣ
	while(m_pTran->is_connect())
	{
		DWORD dw_size = 0;
		LPBYTE p_receive = m_pTran->recv_msg(dw_size);

		if( !VALID_POINT(p_receive) )
			return;

		DWORD dw_message_id = ((tag_net_message*)p_receive)->dw_message_id;

		// ������Ϣ
		if (!serverframe::net_command_manager::get_singleton().handle_message((tag_net_message*)p_receive, dw_size, INVALID_VALUE))
			print_message(_T("dbSession unknown msg"));

		// ������Դ
		m_pTran->free_recv_msg(p_receive);
	}
}

//-----------------------------------------------------------------------------
// �����߳�(�������ݿ������)
//-----------------------------------------------------------------------------
UINT DBSession::thread_connect()
{
//#ifdef _DEBUG
	THROW_EXCEPTION_START;
//#endif

	while(FALSE == m_bTermConnect)
	{
		if( !m_pTran->is_connect() )
		{
			if( !m_pTran->is_trying_create_connect() )
			{
				m_pTran->try_create_connect(m_szDBIP, m_dwDBPort);
			}

			Sleep(100);
			continue;	// ���¼������
		}

		print_message(_T("Contected to DBServer Server at %s: %d\r\n"), get_tool()->unicode8_to_unicode(m_szDBIP), m_dwDBPort);

		// ���͵�½��֤��Ϣ����Ϸ��������ֺ�������Ϸ����Ļ�����
		NET_retification	sendCheck;
		sendCheck.dw_golden_code_ = g_world.GetGoldenCode();
		_tcsncpy(sendCheck.sz_world_name_, g_world.GetWorldName(),X_SHORT_NAME - 1);
		//_tcsncpy(sendCheck.szServerName, m_szServerName, X_SHORT_NAME - 1);

		m_pTran->send_msg(&sendCheck, sendCheck.dw_size);

		break;
	}

//#ifdef _DEBUG
	THROW_EXCEPTION_END;
//#endif
	return 0;
}

UINT DBSession::static_thread_connect(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	DBSession* p_this = (DBSession*)p_data;
	return p_this->thread_connect();
}

//----------------------------------------------------------------------------------------------
// ע�����е���������
//----------------------------------------------------------------------------------------------
VOID DBSession::RegisterAllDBCommand()
{
	REGISTER_NET_MSG("NET_heartbeat",			DBSession,	HandleHeartBeat,			_T("Heart Beat"));

	REGISTER_NET_MSG("NET_retification",		DBSession,HandleCertification,		_T("DB Server Certification Reply"));
	REGISTER_NET_MSG("NET_DB2S_world_init_ok",	DBSession,HandleSTWorldInitOK,   _T("LoongWorld Init OK Reply"));
	REGISTER_NET_MSG("NET_DB2S_get_item_info",	DBSession,	HandleItemInfo,			_T("Load Item Max&Min Serial"));
	REGISTER_NET_MSG("NET_DB2S_load_item_need_log",DBSession,	HandleItemNeedLog,		_T("Load Item TypeID Which Need Log"));
	REGISTER_NET_MSG("NET_DB2S_load_all_role_info",	DBSession,HandleLoadAllRoleInfo,	_T("Load All Role Info"));

	REGISTER_NET_MSG("NET_DB2S_load_simrole",	DBSession,	HandleRoleEnum,			_T("Load simple Role"));
	REGISTER_NET_MSG("NET_DB2S_create_role",		DBSession,HandleRoleCreate,			_T("Create Role"));
	REGISTER_NET_MSG("NET_DB2S_delete_role",		DBSession,	HandleRoleDelete,			_T("Delete Role"));
	REGISTER_NET_MSG("NDBS_ResumeRole",		DBSession,HandleRoleResume,			_T("Resume Role"));
	REGISTER_NET_MSG("NET_DB2S_load_role",	DBSession,		HandleRoleLoad,			_T("Load Role all data"));
	REGISTER_NET_MSG("NET_DB2S_delete_role_guard_time_set",	DBSession,		HandleRoleDeleteGuardSet, _T("set role delete guard time"));
	REGISTER_NET_MSG("NET_DB2S_change_role_name",	DBSession,		HandleRoleChangeName, _T("role change name"));
	REGISTER_NET_MSG("NET_DB2S_cancel_role_del_guard_time",	DBSession,		HandleCancelRoleDelGuardTime, _T("restor delete role"));
	REGISTER_NET_MSG("NET_DB2S_load_serial_reward",		DBSession,			HandleLoadSerialReward, _T(""));

	REGISTER_NET_MSG("NET_DB2S_load_baibao",	DBSession,	HandleBaiBaoLoad,			_T("Load New Item Neet Put Into BaiBao Bag"));
	REGISTER_NET_MSG("NET_DB2S_load_baobao_yuanbao",DBSession,HandleBaiBaoYuanBaoLoad,	_T("Reload Yuanbao Put Into BaiBao Bag"));
	REGISTER_NET_MSG("NET_DB2S_load_baibao_log",	DBSession,HandleBaiBaoLoadLog,		_T("Load BaiBao History Record"));
	REGISTER_NET_MSG("NET_DB2S_load_web_receive",	DBSession, HandleLoadWebReceive,	_T(""));

	REGISTER_NET_MSG("NET_DB2S_load_all_yuanbao_account", DBSession,HandleLoadAllYBAccount,	_T("Load All Yuan Bao Trade Account"));
	REGISTER_NET_MSG("NET_DB2S_load_all_order",		DBSession,HandleLoadAllYBOrder,		_T("Load All Yuan Bao Order"));
	REGISTER_NET_MSG("NET_DB2S_get_role_yuanbao_order",DBSession,	HandleRoleGetYBOrder,		_T("Load Role Yuan Bao Order"));

	REGISTER_NET_MSG("NET_DB2S_get_act_treasure_list",	DBSession,HandleGetActTreasureList,		_T("Load Clan Treasure"));
	REGISTER_NET_MSG("NET_DB2S_get_fame_hall_enter_snap",	DBSession,HandleGetFameHallEnterSnap,	_T("Load FameHall EnterSnap"));
	REGISTER_NET_MSG("NET_DB2S_get_rep_reset_times_tamp",	DBSession,HandleRepRstTimeStamp,		_T("Load Reputation RstTimeSamp"));
	REGISTER_NET_MSG("NET_DB2S_get_rep_rank_list",		DBSession,HandleRepRankLoad,		_T("Load Reputation Rank"));

	// �������
	REGISTER_NET_MSG("NET_DB2S_load_all_guild",	DBSession,		HandleLoadGuild,				_T("load guild"));
	REGISTER_NET_MSG("NET_DB2S_load_all_guild_member",	DBSession,HandleLoadGuildMember,		_T("load guild member"));
	REGISTER_NET_MSG("NET_DB2S_load_guild_ware_items",DBSession,	HandleLoadGuildWareItems,		_T("load guild warehouse items"));
	REGISTER_NET_MSG("NET_DB2S_load_facilities_info",	DBSession,HandleLoadGuildUpgradeInfo,	_T("load guild upgrade info"));
	REGISTER_NET_MSG("NET_DB2S_load_guild_skill_info",	DBSession,HandleLoadGuildSkillInfo,		_T("load guild skill info"));
	REGISTER_NET_MSG("NET_DB2S_load_commodity",		DBSession,HandleLoadGuildCommerceInfo,	_T("load guild commerce info"));
	REGISTER_NET_MSG("NET_DB2S_load_commerce_rank",	DBSession,	HandleLoadCommerceRankInfo,	_T("load commerce rank info"));
	REGISTER_NET_MSG("NET_DB2S_guild_init_ok",		DBSession,	HandleGuildInitOK,			_T("load guild skill info"));
	REGISTER_NET_MSG("NET_DB2S_load_guild_delate",	DBSession,	HandleLoadGuildDelate,		_T("load guild delate"));
	REGISTER_NET_MSG("NET_DB2S_load_pvp_data",		DBSession,	HandleLoadPvPData,			_T("load pvp data"));
	REGISTER_NET_MSG("NET_DB2C_load_guild_recruit",	DBSession,  HandleLoadGuildRecruit,		_T("load guild recruit"));
	REGISTER_NET_MSG("NET_DB2C_load_guild_skill_boss", DBSession,	HandleLoadGuildSkillBoss,  _T("load guild kill boss"));
	REGISTER_NET_MSG("NET_DB2C_load_guild_war_history", DBSession,	HandleLoadGuildWarHistory,  _T("load guild war history"));
	REGISTER_NET_MSG("NET_DB2C_load_guild_plant_data", DBSession,	HandleLoadGuildPlantData,  _T("load guild plant data"));
	REGISTER_NET_MSG("NET_DB2C_load_sbk",				DBSession,	HandleLoadSBKData,			_T("load sbk guild"));
	// �������
	REGISTER_NET_MSG("NET_DB2S_create_pet_soul",	DBSession,	HandleCreatePetSoul,			_T("create pet soul"));
	REGISTER_NET_MSG("NET_DB2S_load_pet_sns",		DBSession,	HandleLoadPetSns,				_T("load pet sns"));
	// �̳����
	REGISTER_NET_MSG("NET_DB2S_get_all_gp_info",		DBSession,	HandleLoadAllGPInfo,			_T("load all group purchase info"));

	// �������
	REGISTER_NET_MSG("NET_DB2S_get_off_line_vcard",	DBSession,	HandleLoadRoleVCard,			_T("load db vcard"));

	// VIP̯λ
	REGISTER_NET_MSG("NET_DB2S_get_all_vip_stall_info",	DBSession,HandleLoadVIPStallInfo,		_T("load all vip stall info"));

	// �̶��
	REGISTER_NET_MSG("NET_DB2S_load_activity_data",	DBSession,	HandleLoadActivityData,		_T("load activity data"));

	// VIP̯λ
	REGISTER_NET_MSG("NET_DB2S_get_vnb_data",		DBSession,	HandleLoadVNBData,			_T("load vip netbar data"));

	// ��ȡ����
	REGISTER_NET_MSG("NET_DB2S_load_left_message",		DBSession,	HandleLoadLeftMsg,			_T("load left msg"));

	// �ʼ����
	REGISTER_NET_MSG("NET_DB2S_get_mail_max_serial",	DBSession,	HandleGetMailMaxSerial,		_T("load mail max serial"));
	REGISTER_NET_MSG("NET_DB2S_load_all_mail",		DBSession,	HandleLoadAllMail,			_T("load all mail"));
	REGISTER_NET_MSG("NET_DB2S_load_all_mail_end",	DBSession,	HandleLoadAllMailEnd,		_T("load all mail end"));
	REGISTER_NET_MSG("NET_DB2S_load_mail_content",	DBSession,	HandleLoadMailContent,		_T("load mail content"));
	REGISTER_NET_MSG("NET_DB2S_load_mail_content_end",	DBSession,  HandleLoadMailContentEnd,	_T("load mail content end"));
	REGISTER_NET_MSG("NET_DB2S_load_mail_item",		DBSession,	HandleLoadMailItem,			_T("load mail item"));
	REGISTER_NET_MSG("NET_DB2S_load_mail_item_end",	DBSession,	HandleLoadMailItemEnd,		_T("load mail item end"));

	//ʦͽ���
	REGISTER_NET_MSG("NET_DB2S_load_all_master_prentice",DBSession,	HandleLoadAllMasterPrentices,	_T("master prentice system"));
	//REGISTER_NET_MSG("NET_DB2S_load_all_master_recruit",DBSession,	HandleLoadAllMasterrecruit,	_T("master recruit system"));

	//���а�
	REGISTER_NET_MSG("NET_DB2S_load_level_rank",	DBSession,	HandleLoadLevelRank,		_T("load Level Rank"));
	REGISTER_NET_MSG("NET_DB2S_load_equip_rank",	DBSession,	HandleEquipRank,			_T("load equip rank"));
	REGISTER_NET_MSG("NET_DB2S_load_guild_rank",	DBSession,	HandleLoadGuildRank,		_T("load guild rank"));
	REGISTER_NET_MSG("NET_DB2S_load_kill_rank",	DBSession,		HandleLoadKillRank,		_T("load kill rank"));
	REGISTER_NET_MSG("NET_DB2S_load_justice_rank",	DBSession,	HandleLoadJusticeRank,	_T(""));
	REGISTER_NET_MSG("NET_DB2S_load_1v1_score_rank",	DBSession, HandleLoad1v1ScoreRank, _T("load 1v1 score rank"));
	REGISTER_NET_MSG("NET_DB2S_load_shihun_rank",		DBSession,	HandleLoadShihunRank,	_T(""));
	REGISTER_NET_MSG("NET_DB2S_load_achievement_point_rank",		DBSession,	HandleLoadAchPointRank,	_T(""));
	REGISTER_NET_MSG("NET_DB2S_load_achievement_number_rank",		DBSession,	HandleLoadAchNumberRank,	_T(""));
	REGISTER_NET_MSG("NET_DB2S_load_MasterGraduate_rank",		DBSession,	HandleLoadMasterRank, _T(""));//ʦͽ gx add 2013.12.06
	REGISTER_NET_MSG("NET_DB2S_load_mounts_rank",		DBSession,	HandleLoadMountsRank,	_T(""));
	REGISTER_NET_MSG("NET_DB2S_load_reach_rank",		DBSession,	HandleLoadReachRank,	_T(""));


	// �������
	REGISTER_NET_MSG("NET_DB2S_load_paimai_max_id", DBSession, HandleLoadPaiMaiMaxID,	_T("load paimai max id"));
	REGISTER_NET_MSG("NET_DB2S_load_all_paimai",	DBSession, handle_load_all_paimai,	_T("load all paimai"));
	REGISTER_NET_MSG("NET_DB2S_load_all_paimai_end",	DBSession, handle_load_all_paimai_end, _T("load all paimai end"));
	REGISTER_NET_MSG("NET_DB2S_load_paimai_item",	DBSession, handle_load_paimai_item,	_T("load paimai item"));
	REGISTER_NET_MSG("NET_DB2S_load_paimai_item_end", DBSession, handle_load_paimai_item_end, _T("load paimai item end"));

	// �Զ�����
	REGISTER_NET_MSG("NET_DB2S_load_auto_paimai",	DBSession, handle_load_auto_paimai,	_T("load auto paimai"));
	REGISTER_NET_MSG("NET_DB2S_load_auto_paimai_end", DBSession, handle_load_auto_paimai_end,	_T("load auto paimai end"));
	REGISTER_NET_MSG("NET_DB2S_check_is_paimai",		DBSession, handle_check_is_paimai,		_T("check is paimai"));
	REGISTER_NET_MSG("NET_DB2S_auto_paimai_init_ok",	DBSession, handle_auto_paimai_init_ok,	_T("auto paimai init ok"));

	// Ǯׯ���
	REGISTER_NET_MSG("NET_DB2S_get_max_bank_id", DBSession, handle_get_max_bank_id,		_T("get max bank id"));
	REGISTER_NET_MSG("NET_DB2S_load_all_bank",	DBSession,  handle_load_all_bank,		_T("get all bank"));

	// ��������
	REGISTER_NET_MSG("NET_DB2C_LoadAllGuerdonQuest",	DBSession,  handle_load_all_guerdonquest,		_T("get all bank"));
	REGISTER_NET_MSG("NET_DB2C_LoadAllGuerdonReward",	DBSession,  handle_load_all_guerdonquest_reward,		_T("NET_DB2C_LoadAllGuerdonReward"));
}

VOID DBSession::UnRegisterAllDBCommand()
{
	UNREGISTER_NET_MSG("NET_heartbeat",			DBSession,	HandleHeartBeat);

	UNREGISTER_NET_MSG("NET_retification",		DBSession,HandleCertification);
	UNREGISTER_NET_MSG("NET_DB2S_world_init_ok",	DBSession,HandleSTWorldInitOK);
	UNREGISTER_NET_MSG("NET_DB2S_get_item_info",	DBSession,	HandleItemInfo);
	UNREGISTER_NET_MSG("NET_DB2S_load_item_need_log",DBSession,	HandleItemNeedLog);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_role_info",	DBSession,HandleLoadAllRoleInfo);

	UNREGISTER_NET_MSG("NET_DB2S_load_simrole",	DBSession,	HandleRoleEnum);
	UNREGISTER_NET_MSG("NET_DB2S_create_role",		DBSession,HandleRoleCreate);
	UNREGISTER_NET_MSG("NET_DB2S_delete_role",		DBSession,	HandleRoleDelete);
	UNREGISTER_NET_MSG("NDBS_ResumeRole",		DBSession,HandleRoleResume);
	UNREGISTER_NET_MSG("NET_DB2S_load_role",	DBSession,		HandleRoleLoad);
	UNREGISTER_NET_MSG("NET_DB2S_delete_role_guard_time_set",	DBSession,		HandleRoleDeleteGuardSet);
	UNREGISTER_NET_MSG("NET_DB2S_change_role_name",	DBSession,		HandleRoleChangeName);
	UNREGISTER_NET_MSG("NET_DB2S_cancel_role_del_guard_time",	DBSession,		HandleCancelRoleDelGuardTime);

	UNREGISTER_NET_MSG("NET_DB2S_load_baibao",	DBSession,	HandleBaiBaoLoad);
	UNREGISTER_NET_MSG("NET_DB2S_load_baobao_yuanbao",DBSession,HandleBaiBaoYuanBaoLoad);
	UNREGISTER_NET_MSG("NET_DB2S_load_baibao_log",	DBSession,HandleBaiBaoLoadLog);
	UNREGISTER_NET_MSG("NET_DB2S_load_web_receive",	DBSession, HandleLoadWebReceive);

	UNREGISTER_NET_MSG("NET_DB2S_load_all_yuanbao_account", DBSession,HandleLoadAllYBAccount);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_order",		DBSession,HandleLoadAllYBOrder);
	UNREGISTER_NET_MSG("NET_DB2S_get_role_yuanbao_order",DBSession,	HandleRoleGetYBOrder);

	UNREGISTER_NET_MSG("NET_DB2S_get_act_treasure_list",	DBSession,HandleGetActTreasureList);
	UNREGISTER_NET_MSG("NET_DB2S_get_fame_hall_enter_snap",	DBSession,HandleGetFameHallEnterSnap);
	UNREGISTER_NET_MSG("NET_DB2S_get_rep_reset_times_tamp",	DBSession,HandleRepRstTimeStamp);
	UNREGISTER_NET_MSG("NET_DB2S_get_rep_rank_list",		DBSession,HandleRepRankLoad);

	// �������
	UNREGISTER_NET_MSG("NET_DB2S_load_all_guild",	DBSession,		HandleLoadGuild);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_guild_member",	DBSession,HandleLoadGuildMember);
	UNREGISTER_NET_MSG("NET_DB2S_load_guild_ware_items",DBSession,	HandleLoadGuildWareItems);
	UNREGISTER_NET_MSG("NET_DB2S_load_facilities_info",	DBSession,HandleLoadGuildUpgradeInfo);
	UNREGISTER_NET_MSG("NET_DB2S_load_guild_skill_info",	DBSession,HandleLoadGuildSkillInfo);
	UNREGISTER_NET_MSG("NET_DB2S_load_commodity",		DBSession,HandleLoadGuildCommerceInfo);
	UNREGISTER_NET_MSG("NET_DB2S_load_commerce_rank",	DBSession,	HandleLoadCommerceRankInfo);
	UNREGISTER_NET_MSG("NET_DB2S_guild_init_ok",		DBSession,	HandleGuildInitOK);
	UNREGISTER_NET_MSG("NET_DB2S_load_guild_delate",	DBSession,	HandleLoadGuildDelate);
	UNREGISTER_NET_MSG("NET_DB2S_load_pvp_data",		DBSession,	HandleLoadPvPData);
	UNREGISTER_NET_MSG("NET_DB2C_load_guild_recruit",	DBSession,  HandleLoadGuildRecruit);
	UNREGISTER_NET_MSG("NET_DB2C_load_guild_skill_boss", DBSession,	HandleLoadGuildSkillBoss);
	UNREGISTER_NET_MSG("NET_DB2C_load_guild_war_history", DBSession,	HandleLoadGuildWarHistory);
	UNREGISTER_NET_MSG("NET_DB2C_load_guild_plant_data", DBSession,	HandleLoadGuildPlantData);

	// �������
	UNREGISTER_NET_MSG("NET_DB2S_create_pet_soul",	DBSession,	HandleCreatePetSoul);
	UNREGISTER_NET_MSG("NET_DB2S_load_pet_sns",		DBSession,	HandleLoadPetSns);
	// �̳����
	UNREGISTER_NET_MSG("NET_DB2S_get_all_gp_info",		DBSession,	HandleLoadAllGPInfo);

	// �������
	UNREGISTER_NET_MSG("NET_DB2S_get_off_line_vcard",	DBSession,	HandleLoadRoleVCard);

	// VIP̯λ
	UNREGISTER_NET_MSG("NET_DB2S_get_all_vip_stall_info",	DBSession,HandleLoadVIPStallInfo);

	// �̶��
	UNREGISTER_NET_MSG("NET_DB2S_load_activity_data",	DBSession,	HandleLoadActivityData);

	// VIP̯λ
	UNREGISTER_NET_MSG("NET_DB2S_get_vnb_data",		DBSession,	HandleLoadVNBData);

	// ��ȡ����
	UNREGISTER_NET_MSG("NET_DB2S_load_left_message",		DBSession,	HandleLoadLeftMsg);

	// �ʼ����
	UNREGISTER_NET_MSG("NET_DB2S_get_mail_max_serial",	DBSession,	HandleGetMailMaxSerial);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_mail",		DBSession,	HandleLoadAllMail);
	UNREGISTER_NET_MSG("NET_DB2S_load_mail_content",	DBSession,	HandleLoadMailContent);
	UNREGISTER_NET_MSG("NET_DB2S_load_mail_item",		DBSession,	HandleLoadMailItem);

	//ʦͽ���
	UNREGISTER_NET_MSG("NET_DB2S_load_all_master_prentice",DBSession,	HandleLoadAllMasterPrentices);

	//���а�
	UNREGISTER_NET_MSG("NET_DB2S_load_level_rank",	DBSession,	HandleLoadLevelRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_equip_rank",	DBSession,	HandleEquipRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_guild_rank",	DBSession,	HandleLoadGuildRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_kill_rank",	DBSession,		HandleLoadKillRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_1v1_score_rank",	DBSession, HandleLoad1v1ScoreRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_MasterGraduate_rank",DBSession,HandleLoadMasterRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_mounts_rank",		DBSession,	HandleLoadMountsRank);
	UNREGISTER_NET_MSG("NET_DB2S_load_reach_rank",		DBSession,	HandleLoadReachRank);



	// ����
	UNREGISTER_NET_MSG("NET_DB2S_load_paimai_max_id", DBSession, HandleLoadPaiMaiMaxID);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_paimai",	DBSession, handle_load_all_paimai);
	UNREGISTER_NET_MSG("NET_DB2S_load_paimai_item",	DBSession, handle_load_paimai_item);
	UNREGISTER_NET_MSG("NET_DB2S_get_max_bank_id", DBSession, handle_get_max_bank_id);
	UNREGISTER_NET_MSG("NET_DB2S_load_all_bank",	DBSession,  handle_load_all_bank);
}

//-------------------------------------------------------------------------------------------------
// ��֤��Ϣ
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleCertification(tag_net_message* p_message, DWORD)
{
	NET_retification* p_receive = (NET_retification*)p_message;

	/*if( p_receive->dw_golden_code_ == m_dwGoldenCode
		&& 0 == _tcsncmp(p_receive->szServerName, m_szServerName, cal_tchar_array_num(p_receive->szServerName))
		&& 0 == _tcsncmp(p_receive->sz_world_name_, g_world.GetWorldName(), cal_tchar_array_num(p_receive->sz_world_name_))
	)
	{*/
	if (m_bInitOK)
		return 0;

	// ��ʼ����������Ϣ
	//g_fameHall.SendLoadDBData();

	// ��ȡ��Ʒ��Ϣ
	NET_DB2C_get_item_info itemInfo;
	Send(&itemInfo, itemInfo.dw_size);
	
	// ���ͻ�ȡ��Ʒ�Ƿ񱣴���Ϣ
	NET_DB2C_load_item_need_log itemNeedLog;
	Send(&itemNeedLog, itemNeedLog.dw_size);	
	
	// ���ͻ�ȡ���н�ɫ��Ϣ����Ϣ
	NET_DB2C_load_all_role_info send;
	Send(&send, send.dw_size);

	// ���ͻ���������Ԫ�������˻���Ϣ
	NET_DB2C_load_all_yuanbao_account YBAccount;
	Send(&YBAccount, YBAccount.dw_size);

	NET_DB2C_load_all_order YBOrder;
	Send(&YBOrder, YBOrder.dw_size);

	// ��ȡ���ɼ����ɳ�Ա��Ϣ
	NET_DB2C_load_all_guild loadGuild;
	Send(&loadGuild, loadGuild.dw_size);

	NET_DB2C_load_all_guild_member loadGuildMember;
	Send(&loadGuildMember, loadGuildMember.dw_size);

	//NET_DB2C_load_pvp_data loadPvP;
	//Send(&loadPvP, loadPvP.dw_size);

	NET_C2DB_load_guild_recruit load_recruit;
	Send(&load_recruit, load_recruit.dw_size);

	//NET_C2DB_load_guild_skill_boss load_guild_kill_boss;
	//Send(&load_guild_kill_boss, load_guild_kill_boss.dw_size);
	
	//NET_C2DB_load_guild_war_history load_guild_war_history;
	//Send(&load_guild_war_history, load_guild_war_history.dw_size);
	
	//NET_C2DB_load_guild_plant_data load_guild_plant_data;
	//Send(&load_guild_plant_data, load_guild_plant_data.dw_size);

	NET_C2DB_load_sbk load_sbk;
	Send(&load_sbk, load_sbk.dw_size);

	// ��DB�����ʼ���Ź���Ϣ
	//NET_DB2C_get_all_gp_info GPInfo;
	//g_dbSession.Send(&GPInfo, GPInfo.dw_size);

	// ��ȡVIP̯λ��Ϣ
	//NET_DB2C_get_all_vip_stall_info VIPStallInfo;
	//g_dbSession.Send(&VIPStallInfo, VIPStallInfo.dw_size);

	//NET_DB2C_get_mail_max_serial GetMailSerial;
	//g_dbSession.Send(&GetMailSerial, GetMailSerial.dw_size);

	//NET_DB2C_load_all_mail LoadMail;
	//g_dbSession.Send(&LoadMail, LoadMail.dw_size);

	// ��ȡ���������
	//NET_S2DB_load_paimai_max_id get_paimai_max_id;
	//g_dbSession.Send(&get_paimai_max_id, get_paimai_max_id.dw_size);

	// ��ȡ����������Ϣ
	//NET_S2DB_load_all_paimai get_all_paimai_info;
	//g_dbSession.Send(&get_all_paimai_info, get_all_paimai_info.dw_size);

	// ��ȡǮׯ�����
	//NET_S2DB_get_max_bank_id get_bank_max_id;
	//g_dbSession.Send(&get_bank_max_id, get_bank_max_id.dw_size);

	// ��ȡ����Ǯׯ��Ϣ
	//NET_S2DB_load_all_bank get_all_bank;
	//g_dbSession.Send(&get_all_bank, get_all_bank.dw_size);

	// ��ȡ��ű�����
	NET_DB2C_load_activity_data ActivityData;		 
	g_dbSession.Send(&ActivityData, ActivityData.dw_size);

	// ��ȡvip��������
	//tagDWORDTime dwToday = GetCurrentDWORDTime();
	//dwToday.sec = 0;
	//dwToday.min = 0;
	//dwToday.hour = 0;
	//
	//NET_DB2C_get_vnb_data VIPNetBar;
	//VIPNetBar.dw_date = dwToday;
	//g_dbSession.Send(&VIPNetBar, VIPNetBar.dw_size);

	//��ȡʦͽ����
	NET_DB2C_load_all_master_prentice LoadAllMasterPrentices;
	g_dbSession.Send(&LoadAllMasterPrentices, LoadAllMasterPrentices.dw_size);

	// ��ȡ��ʦ��
	//NET_DB2C_load_all_master_recruit LoadMasterRecruit;
	//g_dbSession.Send(&LoadMasterRecruit, LoadMasterRecruit.dw_size);
	
	// ��ȡװ�����а�
	//NET_DB2C_load_equip_rank LoadEquipRank;
	//g_dbSession.Send(&LoadEquipRank, LoadEquipRank.dw_size);

	// ��ȡ������а�
	NET_DB2C_load_guild_rank LoadGuildRank;
	g_dbSession.Send(&LoadGuildRank, LoadGuildRank.dw_size);

	// ��ȡ���˰�
	//NET_DB2C_load_kill_rank LoadKillRank;
	//g_dbSession.Send(&LoadKillRank, LoadKillRank.dw_size);

	// ��ȡ�����
	NET_DB2C_load_justice_rank LoadJusticeRank;
	g_dbSession.Send(&LoadJusticeRank, LoadJusticeRank.dw_size);

	//NET_DB2C_load_1v1_score_rank Load1v1ScoreRank;
	//g_dbSession.Send(&Load1v1ScoreRank, Load1v1ScoreRank.dw_size);

	// ��ȡ�ȼ����а�
	NET_DB2C_load_level_randk LoadLevelRank;
	g_dbSession.Send(&LoadLevelRank, LoadLevelRank.dw_size);

	NET_DB2C_load_shihun_rank LoadShihunRank;
	g_dbSession.Send(&LoadShihunRank, LoadShihunRank.dw_size);

	// ��ȡʦͽ���а� gx add 2013.12.06
	NET_DB2C_load_MasterGraduate_randk LoadMasterRank;
	g_dbSession.Send(&LoadMasterRank,LoadMasterRank.dw_size);
	// end

	// ��ȡ�������а�
	NET_DB2C_load_mounts_rank LoadmountsRank;
	g_dbSession.Send(&LoadmountsRank, LoadmountsRank.dw_size);

	// ��ȡ��ֵ���а�
	NET_DB2C_load_reach_rank LoadreachRank;
	g_dbSession.Send(&LoadreachRank, LoadreachRank.dw_size);

	// ��ȡ�ɾ͵����а�
	//NET_DB2C_load_achievement_point_rank LoadAchPointRank;
	//g_dbSession.Send(&LoadAchPointRank, LoadAchPointRank.dw_size);

	//// ��ȡ�ɾ��������а�
	//NET_DB2C_load_achievement_number_rank LoadAchNumberRank;
	//g_dbSession.Send(&LoadAchNumberRank, LoadAchNumberRank.dw_size);

	// ��ȡ��������
	//NET_DB2S_LoadAllGuerdonQuest LoadGuerdonQuest;
	//g_dbSession.Send(&LoadGuerdonQuest, LoadGuerdonQuest.dw_size);

	//NET_DB2S_LoadAllGuerdonReward LoadGuerdonQuestReward;
	//g_dbSession.Send(&LoadGuerdonQuestReward, LoadGuerdonQuestReward.dw_size);
	
	/// ��ȡ����sns
	//NET_DB2C_load_pet_sns LoadPetSns;
	//g_dbSession.Send(&LoadPetSns, LoadPetSns.dw_size);

	// ��ȡ�Զ�����
	//NET_S2DB_load_auto_paimai send_auto;
	//g_dbSession.Send(&send_auto, send_auto.dw_size);

	// GameServer��DB���͵ĳ�ʼ����ɵ�ȷ����Ϣ����֤���һ�����ͣ�
	NET_DB2C_world_init_ok	sendInitOk;	
	Send(&sendInitOk, sendInitOk.dw_size);
	/*}
	else
	{
		ASSERT(0);	
	}*/

	return 0;
}

//-------------------------------------------------------------------------------------------------
// ����
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleHeartBeat(tag_net_message* p_message, DWORD)
{
	return 0;
}

//------------------------------------------------------------------------------------------------
// GameServer��ʼ����ɺ�DB���ص�ȷ����Ϣ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleSTWorldInitOK(tag_net_message* p_message, DWORD)
{
	// �Ƿ���Ҫ�ȼ���Ƿ�������Ҫ���ݶ���ʼ���ɹ�//?? ����Ӹ���ʼ��״̬��־λ -- ʵ�ַ�ʽ������
	
	// ��ʼ�����
	Interlocked_Exchange((LPLONG)&m_bInitOK, TRUE);
	return 0;
}

//-------------------------------------------------------------------------------------------------
// �������ݿ����������������Ϣ
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadAllRoleInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_role_info* p_receive = (NET_DB2S_load_all_role_info*)p_message;

	if( E_Success != p_receive->dw_error_code )
	{
		return INVALID_VALUE;
	}
	
	// ɾ�����о�̬��ɫ
	//g_roleMgr.EraseAllRoleInfo();

	// ���õ�ǰ����ɫID
//	g_roleMgr.SetMaxRoleID(p_receive->dwMaxRoleID);

	// ��ʼ���ؽ�ɫ
	s_role_info* pInfo = &p_receive->role_info_[0];
	for(INT n = 0; n < p_receive->n_count; n++)
	{
		g_roleMgr.insert_role_info(pInfo);
		pInfo++;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// �����ɫ�ͻ��˼�����Ϣ
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleItemInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_item_info* p_receive = (NET_DB2S_get_item_info*)p_message;

	// ������Ʒ����������
	ItemCreator::SetItemSerial(p_receive->n64_max_serial, p_receive->n64_min_serial);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// �����ɫ�ͻ��˼�����Ϣ
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleItemNeedLog(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_item_need_log* p_receive = (NET_DB2S_load_item_need_log*)p_message;

	if( E_Success != p_receive->dw_error_code )
	{
		return INVALID_VALUE;
	}

	// ������Դ����������
	AttRes::GetInstance()->ResetItemLog(p_receive->sNeedLogItem, p_receive->n_item_num);

	return 0;
}


//-------------------------------------------------------------------------------------------------
// �����ɫ�ͻ��˼�����Ϣ
//-------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleEnum(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_simrole* p_receive = (NET_DB2S_load_simrole*)p_message;

	// ���ȸ���AccountID�ҵ�session
	PlayerSession* pSession = g_worldSession.FindGlobalSession(p_receive->dw_account_id);

	// Ԥ�����dbsession�����ʱ�򲻴���ͻ��˵���Ϣ�������Ͳ������̻߳���������
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;
	if( !pSession->IsRoleEnuming() ) return INVALID_VALUE;

/*
	// ��ʼ��
	for(INT i = 0; i < MAX_ROLENUM_ONEACCOUNT; ++i)
	{
		pSession->m_dwRoleID[i] = INVALID_VALUE;
	}
*/

	pSession->m_n8RoleNum = 0;
	pSession->m_SessionCommonData.Initialize();
	if( E_Success == p_receive->dw_error_code )
	{
		for(INT n = 0; n < p_receive->n_count; n++)
		{
		//	pSession->m_dwRoleID[n] = p_receive->sim_role_[n].dw_role_id;

			PlayerSessionCommonData data;
			data.dwRoleID = p_receive->sim_role_[n].dw_role_id;
			data.dwChangeNameTime = p_receive->sim_role_[n].dwChangeNameTime;
			data.dwDelGuardTime = p_receive->sim_role_[n].dwDelGuardTime;
			pSession->m_SessionCommonData.Add(data);
		}

		// �˺�ͨ����Ϣ
		pSession->m_sAccountCommon = p_receive->s_account_common_;

		// Ԫ���Զ�����
		if(pSession->m_sAccountCommon.n_yuanbao_recharge > 0)
		{
			CurrencyMgr::ModifyBaiBaoYuanBao(p_receive->dw_account_id, pSession->m_sAccountCommon.n_yuanbao_recharge, elcid_baibao_bill_yuanbao);
			//CurrencyMgr::ModifyExchangeVolume(p_receive->dw_account_id, pSession->m_sAccountCommon.n_yuanbao_recharge/10, elcid_baibao_bill_yuanbao);

			NET_DB2C_update_yuanbao_recharge send;
			send.dw_account_id = p_receive->dw_account_id;
			send.n32_yuanbao = pSession->m_sAccountCommon.n_yuanbao_recharge;
			g_dbSession.Send(&send, send.dw_size);

			pSession->SetTotalRecharge(pSession->m_sAccountCommon.n_yuanbao_recharge);
			NET_DB2C_update_total_recharge send_total;
			send_total.dw_account_id = p_receive->dw_account_id;
			send_total.n32_yuanbao = pSession->m_sAccountCommon.n_yuanbao_recharge;
			g_dbSession.Send(&send_total, send_total.dw_size);

			pSession->m_sAccountCommon.n_yuanbao_recharge = 0;
		}

		// �޸��콱��־
		if(pSession->m_sAccountCommon.dw_web_type > 0)
		{
			pSession->m_sAccountCommon.dw_receive_type |= pSession->m_sAccountCommon.dw_web_type;
			pSession->m_sAccountCommon.dw_web_type = 0;

			NET_DB2C_update_receive_ex send;
			send.dw_account_id = p_receive->dw_account_id;
			send.dw_receive_type = pSession->m_sAccountCommon.dw_receive_type;
			g_dbSession.Send(&send, send.dw_size);

			NET_DB2C_update_web_recieve send_web;
			send_web.dw_account_id = p_receive->dw_account_id;
			g_dbSession.Send(&send_web, send_web.dw_size);

		}

		// ��ɫ����
		pSession->m_n8RoleNum = p_receive->n_count;
	}

	pSession->m_bRoleEnuming = false;
	pSession->m_bRoleEnumDone = true;
	pSession->m_bRoleEnumSuccess = (E_Success == p_receive->dw_error_code ? true : false);

	// ���ظ��ͻ���
	if( E_Success != p_receive->dw_error_code || p_receive->n_count <= 0 )
	{
		NET_SIS_enum_role send;
		send.dw_error_code = p_receive->dw_error_code;
		
		send.dwSafeCodeCrc = p_receive->s_account_common_.s_safe_code_.dw_safe_code_crc;
		send.dwTimeReset = p_receive->s_account_common_.s_safe_code_.dw_reset_time;
		send.n_num = 0;

		send.dwTimeLastLogin = pSession->GetPreLoginTime();
		send.dwIPLast = pSession->GetPreLoginIP();
		send.dw_ip = pSession->GetCurLoginIP();
		send.dwYuanBao = p_receive->s_account_common_.n_baibao_yuanbao_;
		if (pSession->GetVerification().isNeedVerification())
		{
			send.b_need_verify = true;
		}
		/*else
		{
			pSession->GetVerification().getCryptString(send.byStrCode);
			send.by_verificationIndex = pSession->GetVerification().getIndex();
		}*/
		
		pSession->SendMessage(&send, send.dw_size);

		return INVALID_VALUE;
	}
	else
	{
		DWORD dw_size = sizeof(NET_SIS_enum_role) - 1 + sizeof(tagSimRole) *  p_receive->n_count;
		PBYTE pNew = new BYTE[dw_size];
		if( !VALID_POINT(pNew) ) return INVALID_VALUE;

		NET_SIS_enum_role* pSend = (NET_SIS_enum_role*)pNew;

		pSend->dw_message_id = get_tool()->crc32("NET_SIS_enum_role");
		pSend->dw_size = dw_size;
		pSend->dw_error_code = p_receive->dw_error_code;
		pSend->dwSafeCodeCrc = p_receive->s_account_common_.s_safe_code_.dw_safe_code_crc;
		pSend->dwTimeReset = p_receive->s_account_common_.s_safe_code_.dw_reset_time;
		pSend->dwYuanBao = p_receive->s_account_common_.n_baibao_yuanbao_;
		pSend->n_num = p_receive->n_count;
		pSend->b_need_verify = false;
		pSend->dwTimeLastLogin = pSession->GetPreLoginTime();
		pSend->dwIPLast = pSession->GetPreLoginIP();
		pSend->dw_ip = pSession->GetCurLoginIP();
		if (pSession->GetVerification().isNeedVerification())
		{
			pSend->b_need_verify = true;
		}
		/*else
		{
			pSession->GetVerification().getCryptString(pSend->byStrCode);
			pSend->by_verificationIndex = pSession->GetVerification().getIndex();
		}*/

		get_fast_code()->memory_copy(pSend->bySimpleInfo, p_receive->sim_role_, sizeof(tagSimRole) *  p_receive->n_count);

		pSession->SendMessage(pSend, pSend->dw_size);

		SAFE_DELETE_ARRAY(pNew);

		return 0;
	}
}

//--------------------------------------------------------------------------------------------------------
// ������ɫ
//--------------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleCreate(tag_net_message* p_message, DWORD)
{
	NET_DB2S_create_role* p_receive = (NET_DB2S_create_role*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;
	if( !pSession->IsRoleCreating() ) return INVALID_VALUE;

	// ��������ɹ�
	if( E_Success == p_receive->dw_error_code )
	{
		ASSERT( VALID_POINT(p_receive->s_sim_role_.dw_role_id) );

		// ���뵽session�Ľ�ɫ�б���
		pSession->AddRole(p_receive->s_sim_role_.dw_role_id);

		// ����һ���µ��������
		g_roleMgr.insert_role_info(&p_receive->s_role_info_);

	}
	pSession->m_bRoleCreating = false;

	// ���ظ��ͻ���
	if( E_Success != p_receive->dw_error_code )
	{
		NET_SIS_create_role send;
		send.dw_error_code = E_CreateRole_NameExist;

		pSession->SendMessage(&send, send.dw_size);

		return INVALID_VALUE;	
	}

	else
	{
		DWORD dw_size = sizeof(NET_SIS_create_role) - 1 + sizeof(tagSimRole);
		PBYTE pNew = new BYTE[dw_size];
		if( !VALID_POINT(pNew) ) return INVALID_VALUE;

		NET_SIS_create_role* pSend = (NET_SIS_create_role*)pNew;

		pSend->dw_message_id = get_tool()->crc32("NET_SIS_create_role");
		pSend->dw_size = dw_size;
		pSend->dw_error_code = p_receive->dw_error_code;
		get_fast_code()->memory_copy(pSend->bySimRoleInfo, &p_receive->s_sim_role_, sizeof(tagSimRole));

		pSession->SendMessage(pSend, pSend->dw_size);

		SAFE_DELETE_ARRAY(pNew);

		return 0;
	}
}

//---------------------------------------------------------------------------------------------
// ɾ����ɫ
//---------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleDelete(tag_net_message* p_message, DWORD)
{
	NET_DB2S_delete_role* p_receive = (NET_DB2S_delete_role*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;
	if( !pSession->IsRoleDeleting() ) return INVALID_VALUE;
	
	if( E_Success == p_receive->dw_error_code )
	{
		// ����ҵĽ�ɫ�б���ɾ��
		pSession->RemoveRole(p_receive->dw_role_id);

		// ɾ��һ���������
		g_roleMgr.delete_role_info(p_receive->dw_role_id);
	}
	pSession->m_bRoleDeleting = false;

	// ���͸��ͻ���
	NET_SIS_delete_role send;
	send.dw_error_code = p_receive->dw_error_code;
	send.dw_role_id = p_receive->dw_role_id;

	pSession->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD DBSession::HandleRoleDeleteGuardSet(tag_net_message* p_message, DWORD)
{
	NET_DB2S_delete_role_guard_time_set* p = (NET_DB2S_delete_role_guard_time_set*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p->dw_account_id);
	if(!VALID_POINT(pSession)) return INVALID_VALUE;
	if(!pSession->IsRoleDeleting()) return INVALID_VALUE;

	if( E_Success == p->dw_error_code )
		pSession->SetRoleDelGuardTime(p->dw_role_id, p->dw_delGuardTime);
	

	pSession->m_bRoleDeleting = false;

	NET_SIS_delete_role_guard_set send;
	send.dw_error_code = p->dw_error_code;
	send.dw_role_id = p->dw_role_id;
	send.dw_delGuardTime = p->dw_delGuardTime;
	pSession->SendMessage(&send, send.dw_size);

	return 0;
}	

//-----------------------------------------------------------------------------------------------
// �ָ���ɫ
//-----------------------------------------------------------------------------------------------
DWORD DBSession::HandleCancelRoleDelGuardTime(tag_net_message* p_message, DWORD)
{
	NET_DB2S_cancel_role_del_guard_time* p = (NET_DB2S_cancel_role_del_guard_time*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p->dw_account_id);
	if(!VALID_POINT(pSession)) return INVALID_VALUE;
	if(!pSession->IsRoleDelGuardCanceling()) return INVALID_VALUE;

	pSession->m_bRoleDelGuardCanceling = false;

	if(E_Success == p->dw_error)
		pSession->SetRoleDelGuardTime(p->dw_role_id, INVALID_VALUE);

	NET_SIS_delete_role_guard_Cancel send;
	send.dw_error = p->dw_error;
	send.dw_role_id = p->dw_role_id;
	pSession->SendMessage(&send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------------------------
// ��ɫ����
//-----------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleChangeName(tag_net_message* p_message, DWORD)
{	
	NET_DB2S_change_role_name* p = (NET_DB2S_change_role_name*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p->dw_account_id);
	if(!VALID_POINT(pSession)) return INVALID_VALUE;
	if(!pSession->IsRoleChangeNaming()) return INVALID_VALUE;


	if(E_Success == p->dw_error_code)
	{
		s_role_info* p_info = g_roleMgr.get_role_info(p->dw_role_id);
		if(VALID_POINT(p_info))
		{
			TCHAR nameTmp[X_SHORT_NAME] = {0};
			_tcsncpy(nameTmp, p_info->sz_role_name, X_SHORT_NAME);
			nameTmp[X_SHORT_NAME-1] = _T('\0'); _tcslwr(nameTmp);
			DWORD dwNameCrc = get_tool()->crc32(nameTmp);

			p_info->dw_role_name_crc = p->dw_name_crc;
			_tcsncpy(p_info->sz_role_name, p->sz_new_role_name, X_SHORT_NAME);
			p_info->sz_role_name[X_SHORT_NAME-1] = 0;
			g_roleMgr.get_name_crc_map().erase(dwNameCrc);
			g_roleMgr.get_name_crc_map().add(p->dw_name_crc, p->dw_role_id);
		}
		else
		{
			print_message(_T("��ɫ��������[role:%d, new name:%s, name crc:%u]"), 
				p->dw_role_id, p->sz_new_role_name, p->dw_name_crc);
		}


		//! ͨ����Ϳ�ʼ��Ǯ
		{
			NET_DB2C_baibao_yuanbao_update send;
			send.dw_account_id	= p->dw_account_id;
			send.nBaiBaoYuanBao	= 0 - CHANGE_NAME_COST_YUANBAO;
			Send(&send, send.dw_size);

			pSession->m_sAccountCommon.n_baibao_yuanbao_ -= CHANGE_NAME_COST_YUANBAO;
		}

		//! ��¼log
		{
			NET_DB2C_log_yuanbao send;
			send.s_log_yuanbao_.dw_role_id		= p->dw_role_id;
			send.s_log_yuanbao_.dw_account_id	= p->dw_account_id;
			send.s_log_yuanbao_.dw_cmd_id		= elcid_changename;
			send.s_log_yuanbao_.dw_role_id_rel	= INVALID_VALUE;
			send.s_log_yuanbao_.n8_log_con_type	= ELCT_BaiBao;
			send.s_log_yuanbao_.n_yuanbao		= 0 - CHANGE_NAME_COST_YUANBAO;
			send.s_log_yuanbao_.n_total_yuanbao	= pSession->m_sAccountCommon.n_baibao_yuanbao_;

			Send(&send, send.dw_size);
		}
	}

	pSession->m_bRoleChangeNameing = false;

	NET_SIS_change_role_name send;
	send.dw_error_code = p->dw_error_code == e_change_role_name_failed ? E_ChangeRoleName_NameExist : p->dw_error_code;
	send.dw_role_id = p->dw_role_id;
	send.dw_change_time = p->dw_change_time;
	_tcsncpy(send.sz_new_role_name, p->sz_new_role_name, X_SHORT_NAME);
	pSession->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD DBSession::HandleLoadSerialReward(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_serial_reward* p = (NET_DB2S_load_serial_reward*)p_message;

	Role* pRole = g_roleMgr.get_role(p->dw_role_id);
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(pRole->is_leave_pricitice())
		return INVALID_VALUE;

	string str(p->sz_serial);


	if (!p->b_exists)
	{
		NET_SIS_receive_serial_reward send;
		send.dw_error = E_SerialReward_SerialNotExists;
		pRole->SendMessage(&send, send.dw_size);
		return 0;
	}

	DWORD dwSR = pRole->GetScriptData(0);

	// ��������Ѿ���ȡ
	if (dwSR & (1 << p->n_type))
	{
		NET_SIS_receive_serial_reward send;
		send.dw_error = E_SerialReward_rep_receive;
		pRole->SendMessage(&send, send.dw_size);
		return 0;
	}
	
	DWORD dw_error = E_Success;

	if(pRole->GetScript())
	{
		pRole->GetScript()->OnReceiveSerialReward(pRole, p->n_type, dw_error);
	}

	if(dw_error == E_Success)
	{
		dwSR |= (1 << p->n_type);
		pRole->SetScriptData(0, dwSR);
		NET_DB2C_delete_serial_reward send;
		strncpy(send.sz_serial, p->sz_serial, X_SHORT_NAME);
		g_dbSession.Send(&send, send.dw_size);

		NET_DB2C_log_serial_reward log_send;
		log_send.s_log_serial_reward.dw_account_id = INVALID_VALUE;
		log_send.s_log_serial_reward.n_type = p->n_type;
		strncpy(log_send.s_log_serial_reward.sz_serial, p->sz_serial, X_SHORT_NAME);
		g_dbSession.Send(&log_send, log_send.dw_size);
	}
	


	NET_SIS_receive_serial_reward send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	/*PlayerSession* pSession = g_worldSession.FindSession(p_recv->dw_account_id);
	if(!VALID_POINT(pSession)) return INVALID_VALUE;

	Role* pRole = pSession->GetRole();
	if(!VALID_POINT(pRole))
	return INVALID_VALUE;

	s_serial_reward* p_data = (s_serial_reward*)p_recv->by_data_;

	for(INT i = 0; i < p_recv->n_num; i++)
	{
	s_serial_reward* p = new s_serial_reward;
	ZeroMemory(p, sizeof(*p));
	memcpy(p, p_data, sizeof(s_serial_reward));

	string str(p->sz_serial);

	pRole->get_map_serial_reward().add(str, p);

	p_data++;
	}*/

	return 0;
}

//------------------------------------------------------------------------------------------------
// ѡ���ɫ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleLoad(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_role* p_receive = (NET_DB2S_load_role*)p_message;

	PlayerSession* pSession = g_worldSession.FindGlobalSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;
	if( !pSession->IsRoleLoading() ) return INVALID_VALUE;

	if(p_receive->RoleData.role_data_save_.e_class_ <= EV_Null || p_receive->RoleData.role_data_save_.e_class_ >= EV_End)
		return INVALID_VALUE;

	// ���ͻ��˷��ͷ�����Ϣ
	NET_SIS_select_role send;
	send.dw_error_code = p_receive->dw_error_code;
	send.dw_role_id = p_receive->dw_role_id;
	pSession->SendMessage(&send, send.dw_size);

	if( E_Success == p_receive->dw_error_code )
	{
		ASSERT( NULL == pSession->GetRole() );
		
		// todo: ���һ��roledata�Ĵ�С���Ϸ���
		
		// ����һ���µ�role
		BOOL bFirst = FALSE;
		Role* pRole = g_roleMgr.insert_online_role(p_receive->dw_role_id, &p_receive->RoleData, pSession, bFirst);
		if( NULL == pRole ) return INVALID_VALUE;

		s_role_info* p_role_info = g_roleMgr.get_role_info(pRole->GetID());
		if(!VALID_POINT(p_role_info))
		{
			p_role_info = new s_role_info;
			if(VALID_POINT(p_role_info))
			{
				ZeroMemory(p_role_info, sizeof(s_role_info));
				memcpy(p_role_info->sz_role_name, 
					p_receive->RoleData.role_data_const_.sz_role_name, sizeof(p_role_info->sz_role_name));
				p_role_info->dw_role_id = p_receive->dw_role_id;
				p_role_info->dw_account_id = p_receive->dw_account_id;
				p_role_info->dw_role_name_crc = p_receive->RoleData.role_data_const_.dw_role_name_crc;
				p_role_info->dw_map_id_ = p_receive->RoleData.role_data_save_.dw_map_id_;
				p_role_info->by_level = p_receive->RoleData.role_data_save_.n_level_;
				p_role_info->by_sex_ = p_receive->RoleData.role_data_const_.avatar.bySex;
				p_role_info->e_class_type_ = p_receive->RoleData.role_data_save_.e_class_;
				p_role_info->b_online_ = TRUE;
				p_role_info->dw_amends_flag = 1;
				g_roleMgr.insert_role_info(p_role_info);
			}
		}
		
		// Todo: ��ɫ���ɺ����������Ҳдһ������

		if( !pSession->FullLogin(pRole, bFirst) )
		{
			g_roleMgr.delete_role(pRole->GetID());
		}
		else
		{
			// ����ٱ������п�λ�����������Ƿ���δ�������Ʒ
			/*if(pRole->GetItemMgr().GetBaiBaoFreeSize() > 0)
			{
				NET_DB2C_load_baibao send;
				send.dw_account_id = p_receive->dw_account_id;
				send.n64_serial = INVALID_VALUE;
				send.n_free_space_size_ = pRole->GetItemMgr().GetBaiBaoFreeSize();

				g_dbSession.Send(&send, send.dw_size);
			}*/

			// �������߹һ�����
			DWORD dwCalTime = CalcTimeDiff(GetCurrentDWORDTime(), pRole->GetLogoutTime());
			if(dwCalTime > 0)
			{
				if(pRole->get_level() >= 30)
				{
					pRole->CalLeaveLineHang(dwCalTime);
				}
				pRole->CalLeaveLingqi(dwCalTime);
			}

			// ֪ͨ��ɫ��δ���ʼ�
			if(g_mailmgr.IsNoReadMail(pRole))
			{
				NET_SIS_inform_noread_mail send;
				pRole->SendMessage(&send, send.dw_size);
			}

			if(pRole->get_level() >= 25)
			{
				// �жϰ�ȫ��
				if(pSession->GetBagPsd() != INVALID_VALUE)
				{
					if(pSession->GetBagPsd() == 111111)
					{
						NET_SIS_change_code send;
						pSession->SendMessage(&send, send.dw_size);
					}
				}
			}

			// ��֤�����
			//pSession->GetVerification().resetVerificationCode();
			pSession->GetVerification().setNeedVerTime();
			
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
// ��ٱ��������µ���Ʒ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleBaiBaoLoad(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_baibao* p_receive = (NET_DB2S_load_baibao*)p_message;

	if(p_receive->dw_error_code != E_Success || 0 == p_receive->n_ret_num_)
	{
		return p_receive->dw_error_code;
	}
	
	PlayerSession* pSession = g_worldSession.FindSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;
	
	Role *pRole = pSession->GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	INT32 nItemSize		= sizeof(tagItem);
	INT32 nEquipSize	= sizeof(tagEquip);

	tagItem	*pTmpItem	= NULL;
	tagItem	*pNewItem	= NULL;

	NET_DB2C_delete_baobao_item	sendItem;
	NET_DB2C_delete_baibao_equip	sendEquip;

	pTmpItem = (tagItem*)p_receive->by_data_;
	for(INT32 i=0; i<p_receive->n_ret_num_; ++i)
	{
		if(!MIsEquipment(pTmpItem->dw_data_id))
		{
			pNewItem = ItemCreator::CreateItemByData(pTmpItem);

			pTmpItem = (tagItem*)((BYTE*)pTmpItem + nItemSize);

			// �ȷ���Ϣ,֪ͨDB��item_baibao����ɾ����Ʒ�����������崻��Ȳ����ĸ�������
			if(VALID_POINT(pNewItem->pProtoType))
			{
				sendItem.n64_serial = pNewItem->n64_serial;
				g_dbSession.Send(&sendItem, sendItem.dw_size);

				// ��¼log //??
			}
		}
		else
		{
			pNewItem = ItemCreator::CreateEquipByData(pTmpItem);

			pTmpItem = (tagEquip*)((BYTE*)pTmpItem + nEquipSize);

			// �ȷ���Ϣ,֪ͨDB��item_baibao����ɾ����Ʒ�����������崻��Ȳ����ĸ�������
			if(VALID_POINT(pNewItem->pProtoType))
			{
				sendEquip.n64_serial = pNewItem->n64_serial;
				g_dbSession.Send(&sendEquip, sendEquip.dw_size);

				// ��¼log //??
			}
		}

		if(!VALID_POINT(pNewItem->pProtoType))
		{
			ASSERT(VALID_POINT(pNewItem->pProtoType));
			m_att_res_caution(_T("item/equip"), _T("typeid"), pTmpItem->dw_data_id);
			print_message(_T("The item(SerialNum: %lld) hasn't found proto type!"), pTmpItem->n64_serial);			
			
			::Destroy(pNewItem);
			continue;
		}

		DWORD dw_error_code = pRole->GetItemMgr().Add2BaiBao(pNewItem, elcid_baibao_load_from_db, TRUE);
		if(dw_error_code != E_Success)
		{
			// ��¼ʧ�ܲ���log���Ա��֤ //??

			::Destroy(pNewItem);
		}
	}

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ���°ٱ���Ԫ������
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleBaiBaoYuanBaoLoad(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_baobao_yuanbao* p_receive = (NET_DB2S_load_baobao_yuanbao*)p_message;

	// �ҵ�ָ���˺ŵ��������
	PlayerSession* pPlayerSession = g_worldSession.FindSession(p_receive->dw_account_id);

	// �˺Ų�����
	if (!VALID_POINT(pPlayerSession))
		return INVALID_VALUE;

	pPlayerSession->SetTotalRecharge(p_receive->n_baibao_yaunbao_);

	// �������Ƿ���Ϸ��
	Role* pRole = pPlayerSession->GetRole();
	if (VALID_POINT(pRole))
	{
		// ����������Ұٱ�Ԫ��������(����ֻ���������Ԫ������)
		//INT32 nCurYuanBao = pRole->GetCurMgr().GetBaiBaoYuanBao();
		//ASSERT(p_receive->n_baibao_yaunbao_ >= nCurYuanBao);
		pRole->GetCurMgr().IncBaiBaoYuanBao(p_receive->n_baibao_yaunbao_, elcid_baibao_bill_yuanbao, TRUE);
		//pRole->GetCurMgr().IncExchangeVolume(p_receive->n_baibao_yaunbao_/10, elcid_baibao_bill_yuanbao);

		if (pRole->GetScript())
		{
			pRole->GetScript()->OnRecharge(pRole, p_receive->n_baibao_yaunbao_, pRole->GetTotalRecharge());
		}
		pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_load_baobao_yuanbao, p_receive->n_baibao_yaunbao_);
	}
	// �Ƿ���ѡ�˽���
	else if (VALID_POINT(g_worldSession.FindGlobalSession(p_receive->dw_account_id)))
	{
		//ASSERT(p_receive->n_baibao_yaunbao_ >= 0);
		//pPlayerSession->SetBaiBaoYB(p_receive->n_baibao_yaunbao_);
		CurrencyMgr::ModifyBaiBaoYuanBao(p_receive->dw_account_id, p_receive->n_baibao_yaunbao_, elcid_baibao_bill_yuanbao);
		//CurrencyMgr::ModifyExchangeVolume(p_receive->dw_account_id, p_receive->n_baibao_yaunbao_/10, elcid_baibao_bill_yuanbao);
	}

	NET_DB2C_update_yuanbao_recharge send;
	send.dw_account_id = p_receive->dw_account_id;
	send.n32_yuanbao = p_receive->n_baibao_yaunbao_;
	g_dbSession.Send(&send, send.dw_size);


	NET_DB2C_update_total_recharge send_total;
	send_total.dw_account_id = p_receive->dw_account_id;
	send_total.n32_yuanbao = p_receive->n_baibao_yaunbao_;
	g_dbSession.Send(&send_total, send_total.dw_size);


	return E_Success;
}

// �����콱��־
DWORD DBSession::HandleLoadWebReceive(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_web_receive* p_recv = (NET_DB2S_load_web_receive*)p_message;

	// �ҵ�ָ���˺ŵ��������
	PlayerSession* pPlayerSession = g_worldSession.FindSession(p_recv->dw_account_id);

	// �˺Ų�����
	if (!VALID_POINT(pPlayerSession))
		return INVALID_VALUE;

	pPlayerSession->SetReceiveType(p_recv->dw_web_receive);

	Role* pRole = pPlayerSession->GetRole();
	if(VALID_POINT(pRole))
	{
		NET_SIS_send_receive_account_reward_ex send_receive;
		send_receive.dw_receive_type = pPlayerSession->GetReceiveTypeEx();
		pRole->SendMessage(&send_receive, send_receive.dw_size);
	}


	NET_DB2C_update_receive_ex send;
	send.dw_account_id = p_recv->dw_account_id;
	send.dw_receive_type = pPlayerSession->GetReceiveTypeEx();
	g_dbSession.Send(&send, send.dw_size);

	NET_DB2C_update_web_recieve send_web;
	send_web.dw_account_id = p_recv->dw_account_id;
	g_dbSession.Send(&send_web, send_web.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������������������
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRepRankLoad(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_rep_rank_list* p_receive = (NET_DB2S_get_rep_rank_list*)p_message;

	if(p_receive->dw_error_code != E_Success || 0 == p_receive->nRecNum)
	{
		return p_receive->dw_error_code;
	}
	g_fameHall.HandleUpdateRepRank(p_message);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ��ȡ������ҵ�Ԫ�������˻�
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadAllYBAccount(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_yuanbao_account* p_receive = (NET_DB2S_load_all_yuanbao_account*)p_message;

	// ��ʼ���ؽ����˻�
	tagYBAccount* pYBAccount = &p_receive->account[0];
	for(INT n = 0; n < p_receive->n_count; n++)
	{
		g_tradeYB.CreateTradeAccount(pYBAccount);
		pYBAccount++;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------
// ��ȡ������ҵ�Ԫ�����׶���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadAllYBOrder(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_order* p_receive = (NET_DB2S_load_all_order*)p_message;

	// ��ʼ���ؽ����˻�
	g_tradeYB.SetMaxOrderIndex(p_receive->dw_max_order_id + 1);
	tagYuanBaoOrder* pYBOrder = &p_receive->yuanbao_order[0];
	for(INT n = 0; n < p_receive->n_count; n++)
	{
		g_tradeYB.LoadYOOrder(pYBOrder);
		pYBOrder++;
	}

	return 0;	
}

//------------------------------------------------------------------------------------------------
// ��ȡһ����ҵ�Ԫ�����׶���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRoleGetYBOrder(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_role_yuanbao_order* p_receive = (NET_DB2S_get_role_yuanbao_order*)p_message;

	INT			n_num = p_receive->n_count;
	DWORD		dw_size = sizeof(NET_SIS_get_yuanbao_order);
	tagYuanBaoOrder* pNDBSOrder = &p_receive->yuanbao_order[0];
		
	Role *pRole = g_roleMgr.get_role(pNDBSOrder->dw_role_id);
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if( n_num > 0 )
	{
		dw_size += (n_num-1) * sizeof(tagYuanBaoOrder);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_get_yuanbao_order);

	// ��ʼ���ؽ����˻�
	tagYuanBaoOrder* pNCOrder = &pSend->Orders[0];
	for(INT n = 0; n < p_receive->n_count; n++)
	{
		*pNCOrder = *pNDBSOrder;
		pNDBSOrder++;
		pNCOrder++;
	}
	pSend->n_num = n_num;
	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return 0;
}

//------------------------------------------------------------------------------------------------
// ��ȡָ���˻��ٱ�����ʷ��¼
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleBaiBaoLoadLog(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_baibao_log* p_receive = (NET_DB2S_load_baibao_log*)p_message;

	if (p_receive->dw_error_code != E_Success)
		return INVALID_VALUE;

	PlayerSession* pSession = g_worldSession.FindSession(p_receive->dw_account_id);
	if( !VALID_POINT(pSession) ) return INVALID_VALUE;

	// ���ͻ��˷��ͷ�����Ϣ
	INT nSize = p_receive->dw_size - sizeof(NET_DB2S_load_baibao_log) + sizeof(NET_SIS_init_baibao_record);
	CREATE_MSG(pSend, nSize, NET_SIS_init_baibao_record);

	pSend->n16Num = p_receive->n_log_num_;
	get_fast_code()->memory_copy(pSend->byData, p_receive->by_data_, p_receive->dw_size - sizeof(NET_DB2S_load_baibao_log) + sizeof(p_receive->by_data_));

	pSession->SendMessage(pSend, pSend->dw_size);
	MDEL_MSG(pSend);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ��������ʱ���ȡ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleRepRstTimeStamp(tag_net_message* p_message, DWORD)
{
	g_fameHall.HandleInitRepRstTimeStamp(p_message);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ��ȡ�����ý������
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleGetFameHallEnterSnap(tag_net_message* p_message, DWORD)
{
	g_fameHall.HandleInitFameHallTop50(p_message);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ��ȡ�Ѽ��������䱦
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleGetActTreasureList(tag_net_message* p_message, DWORD)
{
	g_fameHall.HandleInitActTreasureList(p_message);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ����������Է���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuild(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_guild* p = (NET_DB2S_load_all_guild*)p_message;

	g_guild_manager.init_db_guild(&p->s_guild_load_);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������ɵ�������
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildDelate(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_guild_delate* p = (NET_DB2S_load_guild_delate*)p_message;

	if(p->s_guild_delate_load_.dwGuildID == INVALID_VALUE)
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(p->s_guild_delate_load_.dwGuildID);
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->get_delate().load_guild_delate(&p->s_guild_delate_load_);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������ɳ�Ա���Է���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildMember(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_guild_member* p = (NET_DB2S_load_all_guild_member*)p_message;

	if(p->n_all_guild_num <= 0)
	{
		if(g_guild_manager.get_guild_num() > 0)
		{
			ASSERT(0);
			SI_LOG->write_log(_T("guild members load failed! Please check db!\n"));
			g_world.ShutDown();
			return INVALID_VALUE;
		}

		return E_Success;
	}
	else
	{
		if(g_guild_manager.get_guild_num() <= 0)
		{
			ASSERT(0);
			SI_LOG->write_log(_T("guild load failed! Please check db!\n"));
			g_world.ShutDown();
			return INVALID_VALUE;
		}
	}

	g_guild_manager.init_db_guild_member(p->guild_member_load_, p->n_all_guild_num);

	// ���Ͱ��ɳ�ʼ�����������Ϣ
	NET_DB2C_guild_init_ok send;
	Send(&send, send.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// �������PVP���ݷ���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadPvPData(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_pvp_data* p_receive = (NET_DB2S_load_pvp_data*)p_message;

	if(p_receive->n_count <= 0)
		return INVALID_VALUE;

	g_guild_manager.init_pvp_data(p_receive->st_load_guild_pvp_data, p_receive->n_count);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ���������ļ�����ݷ���
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildRecruit(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_guild_recruit* p_receive = (NET_DB2C_load_guild_recruit*)p_message;

	if(p_receive->n_num <= 0)
		return INVALID_VALUE;

	g_guild_manager.load_guild_recruit(p_receive->p_data, p_receive->n_num);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// �������BOSS��ɱ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildSkillBoss(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_guild_skill_boss* p_recv = (NET_DB2C_load_guild_skill_boss*)p_message;

	g_guild_manager.load_guild_kill_boss(p_recv->st_guild_kill_boss, p_recv->n_num);

	return E_Success;
}

DWORD DBSession::HandleLoadGuildWarHistory(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_guild_war_history* p_recv = (NET_DB2C_load_guild_war_history*)p_message;

	g_guild_manager.load_guild_war_history(p_recv->st_guild_war_history, p_recv->n_num);

	return E_Success;
}

DWORD DBSession::HandleLoadGuildPlantData(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_guild_plant_data* p_recv = (NET_DB2C_load_guild_plant_data*)p_message;

	g_guild_manager.load_guild_plant(p_recv->s_plant_data.dw_guild_id, p_recv->s_plant_data.s_data, MAX_PLANT_NUMBER);

	return E_Success;
}

DWORD DBSession::HandleLoadSBKData(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_sbk* p_recv = (NET_DB2C_load_sbk*)p_message;

	g_guild_manager.set_SBK_gulid(p_recv->dw_guild_id, false);

	return E_Success;
}
//------------------------------------------------------------------------------------------------
// ������ɲֿ���Ʒ����
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildWareItems(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_guild_ware_items* p_receive = (NET_DB2S_load_guild_ware_items*)p_message;

	// ����ID��֤
	if (!VALID_VALUE(p_receive->dw_guild_id))
		return INVALID_VALUE;

	// ��Ʒ��Ϣ����
	if (p_receive->n_item_num < 0)
		return INVALID_VALUE;

	// ȡ�ð���
	guild* pGuild = g_guild_manager.get_guild(p_receive->dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		// ���ɲ�����
		return INVALID_VALUE;
	}

	pGuild->get_guild_warehouse().load_warehouse_items(p_receive->by_data, p_receive->n_item_num);

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������ɵ���ʩ������Ϣ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildUpgradeInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_facilities_info* p_receive = (NET_DB2S_load_facilities_info*)p_message;

	if (!VALID_VALUE(p_receive->dw_guild_id) || p_receive->n_info_num < 0)
	{
		return INVALID_VALUE;
	}

	// ȡ�ð���
	guild* pGuild = g_guild_manager.get_guild(p_receive->dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	if (!pGuild->get_guild_facilities().LoadFacilitiesInfo(p_receive->s_facilities_info_, p_receive->n_info_num))
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������ɼ�����Ϣ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildSkillInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_guild_skill_info* p_receive = (NET_DB2S_load_guild_skill_info*)p_message;

	if (!VALID_VALUE(p_receive->dw_guild_id) || p_receive->n_info_num_ <= 0)
	{
		return INVALID_VALUE;
	}

	// ȡ�ð���
	guild* pGuild = g_guild_manager.get_guild(p_receive->dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	if (!pGuild->get_guild_skill().load_guild_skill_info(p_receive->s_guild_skill_info_, p_receive->n_info_num_))
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// �������������Ϣ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildCommerceInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_commodity* p_receive = (NET_DB2S_load_commodity*)p_message;

	if (!VALID_VALUE(p_receive->dw_guild_id) || p_receive->n_commodity_num < 0)
	{
		return INVALID_VALUE;
	}

	// ȡ�ð���
	guild* pGuild = g_guild_manager.get_guild(p_receive->dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	s_guild_commerce_info* pInfo = NULL;
	if (p_receive->n_commodity_num > 0)
	{
		pInfo = p_receive->s_commerce_info_;
	}
	if (!pGuild->get_guild_commerce().load_commerce_info(pInfo, p_receive->n_commodity_num))
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// �����������������Ϣ
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadCommerceRankInfo(serverframe::tag_net_message *p_message, DWORD)
{
	NET_DB2S_load_commerce_rank* p_receive = (NET_DB2S_load_commerce_rank*)p_message;

	if (!VALID_VALUE(p_receive->dw_guild_id) || p_receive->n_rank_num < 0)
	{
		return INVALID_VALUE;
	}

	// ȡ�ð���
	guild* pGuild = g_guild_manager.get_guild(p_receive->dw_guild_id);
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	tagCommerceRank* pInfo = NULL;
	if (p_receive->n_rank_num > 0)
	{
		pInfo = p_receive->s_rank_info;
	}
	if (!pGuild->get_guild_commerce().load_commerce_rank_info(pInfo, p_receive->n_rank_num))
	{
		return INVALID_VALUE;
	}

	return E_Success;
}

//------------------------------------------------------------------------------------------------
// ������ɳ�ʼ�����
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleGuildInitOK(tag_net_message* p_message, DWORD)
{
	if (!g_guild_manager.is_guild_init_ok())
	{
		// �رշ�����
		ASSERT(0);
		SI_LOG->write_log(_T("guild init failed! Please check db!\n"));
		g_world.ShutDown();
		return INVALID_VALUE;
	}

	return E_Success;
}

DWORD DBSession::HandleCreatePetSoul( tag_net_message* p_message, DWORD )
{
	NET_DB2S_create_pet_soul* p_receive = (NET_DB2S_create_pet_soul*)p_message;

	DWORD dwRtv = E_Success;

	if (E_Success == p_receive->dw_error_code)
	{
		M_trans_else_ret(pRole, g_roleMgr.get_role(p_receive->pet_data_load.pet_att.dw_master_id_), Role, INVALID_VALUE);

		const BYTE* pData = (BYTE*)(&p_receive->pet_data_load);
		M_trans_else_ret(pSoulLoad, PetSoul::CreateSoulByDBData(pData, TRUE), PetSoul, INVALID_VALUE);

		dwRtv = pRole->GetPetPocket()->PutIn(pSoulLoad);
		if (E_Success !=dwRtv)
		{
			PetSoul::DeleteSoul(pSoulLoad, TRUE);
		}
		else
		{
			pRole->GetPetPocket()->InitRandomSkill(pSoulLoad->GetID());

			//dwRtv = pRole->GetPetPocket()->CallPet(pSoulLoad->GetID());
		}
		
		ASSERT(E_Success == dwRtv);
	}

	return dwRtv;
}

DWORD DBSession::HandleLoadPetSns(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_pet_sns* p_receive = (NET_DB2S_load_pet_sns*)p_message;

	DWORD dwRtv = E_Success;
	if (E_Success == p_receive->dw_error_code)
	{
		tagPetSNSInfo* pInfo = p_receive->pet_sns_info;
	
		g_petSnsMgr.initList(pInfo, p_receive->n_count);
	}

	return dwRtv;
}

DWORD DBSession::HandleLoadAllGPInfo( tag_net_message* p_message, DWORD )
{
	NET_DB2S_get_all_gp_info* p_receive = (NET_DB2S_get_all_gp_info*)p_message;

	if (p_receive->dw_error_code != E_Success)
		return p_receive->dw_error_code;

	return g_mall.LoadAllGPInfo(p_receive->n_gp_info_num_, (LPVOID)(p_receive->gp_info_data_));
}

DWORD DBSession::HandleLoadRoleVCard( tag_net_message* p_message, DWORD )
{
	NET_DB2S_get_off_line_vcard* p_receive = (NET_DB2S_get_off_line_vcard*)p_message;

	if (E_Success != p_receive->dw_error_code)
	{
		return E_Success;
	}

	Role* pQuery = g_roleMgr.get_role(p_receive->dw_query_id);
	if (!VALID_POINT(pQuery))
	{
		return E_Success;
	}

	tagRoleVCard tmpVCard;

	const BYTE* pByte1 = p_receive->by_data;
	tmpVCard.Init(pByte1, NULL);

	tmpVCard.SendVCard2Client(p_receive->dw_query_id);

	return E_Success;
}

DWORD DBSession::HandleLoadVIPStallInfo(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_all_vip_stall_info* p_receive = (NET_DB2S_get_all_vip_stall_info*)p_message;

	if (p_receive->dw_error_code != E_Success)
		return p_receive->dw_error_code;

	return g_VIPStall.LoadAllVIPStallInfo(p_receive->vip_stall_info);
}

//------------------------------------------------------------------------------------------------
// ��ȡ�̶������
//------------------------------------------------------------------------------------------------
DWORD DBSession::HandleLoadActivityData(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_activity_data* p_receive = (NET_DB2S_load_activity_data*)p_message;

	if(p_receive->n_count == 0) return 0;

	// ��ʼ�̶���ű�����
	s_activity_data* pActivityData = &p_receive->activity[0];
	for(INT n = 0; n < p_receive->n_count; n++)
	{
		activity_fix* pActivity = activity_mgr::GetInstance()->get_activity(pActivityData->dw_id);
		if(VALID_POINT(pActivity))
			pActivity->init_script_data(pActivityData);

		pActivityData++;
	}

	return 0;
}

DWORD DBSession::HandleLoadVNBData( tag_net_message* p_message, DWORD )
{
	NET_DB2S_get_vnb_data* pData = (NET_DB2S_get_vnb_data*)p_message;
	g_VipNetBarMgr.InitData(&pData->players);	

	return E_Success;
}

DWORD DBSession::HandleLoadLeftMsg( tag_net_message* p_message, DWORD )
{
	g_msgMgr.deal_load_offline_msg(p_message);

	return E_Success;
}

DWORD DBSession::HandleLoadAllMail(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_mail* p_receive = (NET_DB2S_load_all_mail*)p_message;

	g_mailmgr.InitDBLoadMail(p_receive);
	return E_Success;
}

DWORD DBSession::HandleLoadAllMailEnd(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_mail_end* p_receive = (NET_DB2S_load_all_mail_end*)p_message;

	NET_DB2C_load_mail_content send;
	g_dbSession.Send(&send, send.dw_size);

	print_message(_T("init all mail base info e_success!!!\r\n"));

	return E_Success;
}

DWORD DBSession::HandleLoadMailContent(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_mail_content* p_receive = (NET_DB2S_load_mail_content*)p_message;

	g_mailmgr.InitDBLoadMailContent(p_receive);

	return E_Success;
}

DWORD DBSession::HandleLoadMailContentEnd(tag_net_message* p_message, DWORD)
{
	NET_DB2C_load_mail_item send;
	g_dbSession.Send(&send, send.dw_size);

	print_message(_T("init all mail content e_success!!!\r\n"));

	return E_Success;
}

DWORD DBSession::HandleLoadMailItem(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_mail_item* p_receive = (NET_DB2S_load_mail_item*)p_message;

	g_mailmgr.InitDBLoadMailItem(p_receive->by_data_, p_receive->n_count);

	return E_Success;
}

DWORD DBSession::HandleLoadMailItemEnd(tag_net_message* p_message, DWORD)
{
	g_mailmgr.SetInitOK(TRUE);
	print_message(_T("init all mail item e_success!!!\r\n"));

	// ��һ��ִ�нű��¼�
	static bool bFirst = TRUE;
	if (bFirst)
	{
		const RankScript* pScript = g_ScriptMgr.GetRankScript();
		if (VALID_POINT(pScript)  )
		{
			pScript->OnInitRoleLevel();
		}
		bFirst = FALSE;
	}

	return E_Success;
}

DWORD DBSession::HandleGetMailMaxSerial(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_mail_max_serial* p_receive = (NET_DB2S_get_mail_max_serial*)p_message;

	g_mailmgr.InitDBMaxMailSerial(p_receive->dw_max_mail_serial_); 

	return E_Success;
}
//--------------------------------------------------------------------
// ʦͽ
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadAllMasterPrentices( tag_net_message* p_message, DWORD )
{
	NET_DB2S_load_all_master_prentice* p = (NET_DB2S_load_all_master_prentice*)p_message;
	if( p->u32_number )
		g_MasterPrenticeMgr.init_placard( p_message );
	return E_Success;
}

DWORD DBSession::HandleLoadAllMasterrecruit( tag_net_message* p_message, DWORD )
{
	NET_DB2S_load_all_master_recruit* p = (NET_DB2S_load_all_master_recruit*)p_message;
	//g_MasterPrenticeMgr.init_recruit(p->dwRoleID, p->u32_number);
	return E_Success;
}

//--------------------------------------------------------------------
// �ȼ����а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadLevelRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_level_rank* p_receive = (NET_DB2S_load_level_rank*)p_message;

	RankMgr::GetInstance()->InitLevelRank(p_receive);

	return E_Success;
}

//--------------------------------------------------------------------
// 1V1�������а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoad1v1ScoreRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_1v1_score_rank* p_receive = (NET_DB2S_load_1v1_score_rank*)p_message;

	RankMgr::GetInstance()->Init1v1ScoreRank(p_receive);

	return E_Success;
}

//--------------------------------------------------------------------
// �������а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadKillRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_kill_rank* p_receive = (NET_DB2S_load_kill_rank*)p_message;

	RankMgr::GetInstance()->InitKillRank(p_receive);

	return E_Success;
}

//--------------------------------------------------------------------
// �������а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadJusticeRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_justice_rank* p_receive = (NET_DB2S_load_justice_rank*)p_message;

	RankMgr::GetInstance()->InitJustice(p_receive);

	return E_Success;
}

//--------------------------------------------------------------------
// �ɻ����а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadShihunRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_shihun_rank* p_receive = (NET_DB2S_load_shihun_rank*)p_message;

	RankMgr::GetInstance()->InitShihunRank(p_receive);

	return E_Success;
}
//ʦͽ��
DWORD DBSession::HandleLoadMasterRank(tag_net_message* p_message,DWORD)
{
	NET_DB2S_load_MasterGraduate_rank* p_receive = (NET_DB2S_load_MasterGraduate_rank*)p_message;
	RankMgr::GetInstance()->InitMasterRank(p_receive);
	return E_Success;
}
DWORD DBSession::HandleLoadAchPointRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_achievement_point_rank* p_receive = (NET_DB2S_load_achievement_point_rank*)p_message;

	RankMgr::GetInstance()->InitAchPointRank(p_receive);

	return E_Success;
}
DWORD DBSession::HandleLoadAchNumberRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_achievement_number_rank* p_receive = (NET_DB2S_load_achievement_number_rank*)p_message;

	RankMgr::GetInstance()->InitAchNumberRank(p_receive);

	return E_Success;
}

DWORD DBSession::HandleLoadMountsRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_mounts_rank* p_receive = (NET_DB2S_load_mounts_rank*)p_message;

	RankMgr::GetInstance()->InitMountsRank(p_receive);

	
	return E_Success;
}

DWORD DBSession::HandleLoadReachRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_reach_rank* p_receive = (NET_DB2S_load_reach_rank*)p_message;

	RankMgr::GetInstance()->InitreachRank(p_receive);


	return E_Success;
}

//--------------------------------------------------------------------
// װ�����а�
//--------------------------------------------------------------------
DWORD DBSession::HandleEquipRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_equip_rank* p_receive = (NET_DB2S_load_equip_rank*)p_message;

	RankMgr::GetInstance()->InitEquipRank(&p_receive->s_equip);

	return E_Success;
}

//--------------------------------------------------------------------
// ������а�
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadGuildRank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_guild_rank* p_receive = (NET_DB2S_load_guild_rank*)p_message;

	RankMgr::GetInstance()->InitGuildRank(p_receive);

	return E_Success;
}

//--------------------------------------------------------------------
// ��ȡ���������
//--------------------------------------------------------------------
DWORD DBSession::HandleLoadPaiMaiMaxID(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_paimai_max_id* p_receive = (NET_DB2S_load_paimai_max_id*)p_message;

	g_paimai.set_max_id(p_receive->dw_max_id);

	return E_Success;
}

//--------------------------------------------------------------------
// ��ȡ����������Ϣ
//--------------------------------------------------------------------
DWORD DBSession::handle_load_all_paimai(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_paimai* p_recv = (NET_DB2S_load_all_paimai*)p_message;

	g_paimai.load_all_paimai_from_db(p_recv);

	return E_Success;
}

//--------------------------------------------------------------------
// ��ȡ������Ʒ��Ϣ
//--------------------------------------------------------------------
DWORD DBSession::handle_load_paimai_item(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_paimai_item* p_recv = (NET_DB2S_load_paimai_item*)p_message;

	g_paimai.load_paimai_item(p_recv);

	return E_Success;
}

DWORD DBSession::handle_load_all_paimai_end(tag_net_message* p_message, DWORD)
{
	NET_S2DB_load_paimai_item send;
	g_dbSession.Send(&send, send.dw_size);

	print_message(_T("init paimai base info ok!!!\r\n"));
	return E_Success;
}

DWORD DBSession::handle_load_paimai_item_end(tag_net_message* p_message, DWORD)
{
	g_paimai.set_init_ok(TRUE);
	print_message(_T("init paimai item ok!!!\r\n"));

	return E_Success;
}

//--------------------------------------------------------------------
// ��ȡǮׯ�����
//--------------------------------------------------------------------
DWORD DBSession::handle_get_max_bank_id(tag_net_message* p_message, DWORD)
{
	NET_DB2S_get_max_bank_id* p_recv = (NET_DB2S_get_max_bank_id*)p_message;

	g_bankmgr.set_max_id(p_recv->dw_max_id);

	return E_Success;
}

//--------------------------------------------------------------------
// ��ȡǮׯ������Ϣ
//--------------------------------------------------------------------
DWORD DBSession::handle_load_all_bank(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_all_bank* p_recv = (NET_DB2S_load_all_bank*)p_message;

	g_bankmgr.load_all_bank_from_db(p_recv);

	return E_Success;
}
//--------------------------------------------------------------------
// ��ȡ������������
//--------------------------------------------------------------------
DWORD DBSession::handle_load_all_guerdonquest(tag_net_message* p_message, DWORD)
{
	NET_DB2C_LoadAllGuerdonQuest* p_recv = (NET_DB2C_LoadAllGuerdonQuest*)p_message;
	g_GuerdonQuestMgr.init(p_recv->stData, p_recv->nNumber);
	return E_Success;
}
//--------------------------------------------------------------------
// ��ȡ��������������
//--------------------------------------------------------------------
DWORD DBSession::handle_load_all_guerdonquest_reward(tag_net_message* p_message, DWORD)
{
	NET_DB2C_LoadAllGuerdonReward* p_recv = (NET_DB2C_LoadAllGuerdonReward*)p_message;
	g_GuerdonQuestMgr.init_item(p_recv->byData, p_recv->nNumber);
	return E_Success;
}

// ��ȡ�����Զ�����
DWORD DBSession::handle_load_auto_paimai(tag_net_message* p_message, DWORD)
{
	NET_DB2S_load_auto_paimai* p_recv = (NET_DB2S_load_auto_paimai*)p_message;
	g_auto_paimai.load_all_auto_paimai(p_recv);
	return E_Success;
}

// �����Զ��������ݳ�ʼ�����
DWORD DBSession::handle_load_auto_paimai_end(tag_net_message* p_message, DWORD)
{
	g_auto_paimai.check_is_paimai();
	return E_Success;
}

// ����Ƿ��Ѿ�����
DWORD DBSession::handle_check_is_paimai(tag_net_message* p_message, DWORD)
{
	NET_DB2S_check_is_paimai* p_recv = (NET_DB2S_check_is_paimai*)p_message;

	g_auto_paimai.set_is_paimai(p_recv->dw_auto_paimai_id, p_recv->b_have);

	return E_Success;
}

// �Զ�������ʼ�����
DWORD DBSession::handle_auto_paimai_init_ok(tag_net_message* p_message, DWORD)
{
	g_auto_paimai.set_init_ok(TRUE);
	return E_Success;
}
