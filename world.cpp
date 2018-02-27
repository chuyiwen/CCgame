
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//游戏世界管理

#include "StdAfx.h"

#include "../../common/WorldDefine/svn_revision.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "vip_netbar.h"
#include "world.h"
#include "world_session.h"
#include "login_session.h"
#include "db_session.h"
#include "bill_session.h"
//#include "montioring_session.h"
#include "GMSession.h"
#include "map_creator.h"
#include "map_creator.h"
#include "role.h"
#include "creature.h"
#include "role_mgr.h"
#include "att_res.h"
#include "quest_mgr.h"
#include "script_mgr.h"
#include "loot_mgr.h"
#include "social_mgr.h"
#include "group_mgr.h"
#include "buff_effect.h"
#include "world_net_cmd_mgr.h"
#include "mall.h"
#include "TradeYuanBao.h"
#include "game_guarder.h"
#include "famehall.h"
#include "vip_stall.h"
#include "activity_mgr.h"
#include "ps_bomb.h"
#include "ps_ipchecker.h"
#include "guild_manager.h"
#include "MsgInfoMgr.h"
#include "game_guarder.h"
#include "gm_policy.h"
#include "NPCTeam_mgr.h"
#include "TreasureChest_mgr.h"
#include "mail_mgr.h"
#include "master_prentice_mgr.h"
#include "RankMgr.h"
#include "center_session.h"
#include "paimai_manager.h"
#include "bank_manager.h"
#include "guerdon_quest_mgr.h"
#include "pet_sns_mgr.h"
#include "pvp_mgr.h"
#include "TeamRandShareMgr.h"
#include "auto_paimai.h"
#include "lottery.h"
#include "BattleGround.h"
//#include "mem_map.h"
//#define VLD_FORCE_ENABLE
//#include "vld.h"

file_container* World::p_var = NULL;
thread_manager*			World::p_thread = NULL;

////========>
struct UpdateTimeCalcAux
{
	VOID Start()
	{
		dwStartTime = timeGetTime();
	}
	DWORD elapse()
	{
		DWORD dwDiff = timeGetTime() - dwStartTime;
		dwStartTime = timeGetTime();
		return dwDiff;
	}
	DWORD dwStartTime;
};


////<========
//-----------------------------------------------------------------------------
// construction
//-----------------------------------------------------------------------------
World::World()
{
	m_bTerminateUpdate = FALSE;
	m_bShutingdown = FALSE;
	m_bOverloadAlert = FALSE;

	m_dwWorldTick		= 0;
	m_dwLastSaveTick	= m_dwWorldTick;

	m_dwMaxMsgSize = 0;

	b_ShutDown_Time = FALSE;
	n_ShutDown_Time = 0;

	// 初始化服务器时间
	m_CurrentTime = GetCurrentDWORDTime();
	World::p_var = new file_container;
	m_pLog = new log_file;
	pVFS = new file_system;
	World::p_thread = new thread_manager;

	dw_leave_role_num = 0;
}

//-----------------------------------------------------------------------------
// destruction
//-----------------------------------------------------------------------------
World::~World()
{
	SAFE_DELETE(World::p_var);
	SAFE_DELETE(m_pLog);
	SAFE_DELETE(pVFS);
	SAFE_DELETE(World::p_thread);
}

//-----------------------------------------------------------------------------
// init
//-----------------------------------------------------------------------------
BOOL World::Init(HINSTANCE hInst)
{

	///* 定时关服务器
	tagDWORDTime dwShutDownTime;
	ZeroMemory(&dwShutDownTime, sizeof(dwShutDownTime));
	dwShutDownTime.year = SHUT_DOWN_YEAY;
	dwShutDownTime.month = SHUT_DOWN_MONTH;
	
	DWORD dwCurTime = GetCurrentDWORDTime();

	if (dwCurTime > dwShutDownTime)
		return false;
	//*/



	
	// 生成log
	m_pLog->create_log();

	// 初始化字典
	//g_ipDict.Init();
	
	// 炸弹初始化
	g_pSGuarder.Init();

	// 加载配置文件
	TCHAR t_sz_path[MAX_PATH];
	ZeroMemory(t_sz_path, sizeof(t_sz_path));

	if (!get_file_io_mgr()->get_ini_path(t_sz_path, _T("server_config/world/world"))||
		!World::p_var->load(g_world.get_virtual_filesys(), t_sz_path))
	{
		ERROR_CLUE_ON(_T("配置文件未找到"));
		return FALSE;
	}

	//World::p_var->load(g_world.get_virtual_filesys(), _T("server_config/world/world.ini"));

	m_dwViewWidth = (DWORD)World::p_var->get_dword(_T("width"), _T("window"));
	m_dwViewHeight = (DWORD)World::p_var->get_dword(_T("height"), _T("window"));
	m_bySaveOneNeedTicks = (BYTE)World::p_var->get_dword(_T("save_num_per_tick"), _T("world"));
	m_dwGoldenCode = (DWORD)World::p_var->get_dword(_T("golden_code"), _T("server"));
	m_strWorldName = World::p_var->get_string(_T("name"), _T("world"));
	b_Join_World = World::p_var->get_int(_T("joinworld"), _T("isjoinworld"));

	// 作弊检测
	m_fLaxCheatDistanceSQ = World::p_var->get_float(_T("lax_cheat_distance"), _T("cheat")) * FLOAT(TILE_SCALE);
	m_fLaxCheatDistanceSQ = m_fLaxCheatDistanceSQ * m_fLaxCheatDistanceSQ;
	m_fStrictCheatDistanceSQ = World::p_var->get_float(_T("strict_cheat_distance"), _T("cheat")) * FLOAT(TILE_SCALE);
	m_fStrictCheatDistanceSQ = m_fStrictCheatDistanceSQ * m_fStrictCheatDistanceSQ;
	m_fLaxJumpCheatDistanceSQ = World::p_var->get_float(_T("lax_jump_cheat_distance"), _T("cheat")) * FLOAT(TILE_SCALE);
	m_fLaxJumpCheatDistanceSQ = m_fLaxJumpCheatDistanceSQ * m_fLaxJumpCheatDistanceSQ;
	m_fStrictJumpCheatDistanceSQ = World::p_var->get_float(_T("strict_jump_cheat_distance"), _T("cheat")) * FLOAT(TILE_SCALE);
	m_fStrictJumpCheatDistanceSQ = m_fStrictJumpCheatDistanceSQ * m_fStrictJumpCheatDistanceSQ;

	m_strength_check_time = World::p_var->get_int(_T("strength_check_time"), _T("cheat"));
	m_strength_check_num = World::p_var->get_int(_T("strength_check_num"), _T("cheat"));
	m_weak_check_time = World::p_var->get_int(_T("weak_check_time"), _T("cheat"));
	m_weak_check_num = World::p_var->get_int(_T("weak_check_num"), _T("cheat"));
	m_ce_check_num = World::p_var->get_int(_T("cd_check_num"), _T("cheat"));

	// 创建窗口
	get_window()->init(m_dwViewWidth, m_dwViewHeight, TRUE);

	tstring strWindowName = _T("GameServer") + m_strWorldName;
	get_window()->create_window(strWindowName.c_str(), hInst);

	// 脚本初始化
	if( FALSE == g_ScriptMgr.Init() )
		return FALSE;

	// 资源初始化
	if( FALSE == AttRes::GetInstance()->Init() )
		return FALSE;
	
	// 状态特殊类初始化
	BuffEffect::Init();

	// 帮派
	if( FALSE == g_guild_manager.init() )
		return FALSE;

	// 社会管理器初始化
	if( FALSE == g_socialMgr.init() )
		return FALSE;

	// 商城管理器初始化
	if( FALSE == g_mall.Init() )
		return FALSE;
	//gx modify

	// 元宝交易管理类初始化
	if( FALSE == g_tradeYB.Init())
		return FALSE;

	//团队管理器初始化
	if( FALSE == g_groupMgr.Init())
		return FALSE;
	
	// 名人堂初始化
	//if (FALSE == g_fameHall.Init())
	//	return FALSE;

	// 怪物小队队形初始化
	if(FALSE == NPCTeamMgr::LoadNPCOrder())
		return FALSE;

	// VIP摊位初始化
	if (FALSE == g_VIPStall.Init())
		return FALSE;

	// 创建所有地图
	if( FALSE == g_mapCreator.init() )
		return FALSE;

	mInstanceRoleNum.resize(g_mapCreator.GetInstanceNames().size(), 0);

	// 所有在地图上层处理的消息管理器初始化
	if( FALSE == WorldNetCmdMgr::GetInstance()->Init() )
		return FALSE;

	// 人物管理器初始化
	if( FALSE == g_roleMgr.init() )
		return FALSE;

	//任务管理器初始化
	if (FALSE == g_questMgr.init())
		return FALSE;

	// 脚本通用消息流管理类
	if( FALSE == g_MsgInfoMgr.Init())
		return FALSE;

	//掉落
	new drop_mgr;	//注意 此处 new
	if( FALSE == g_drop_mgr.init() )
		return FALSE;

	// 创建所有session
	if( FALSE == g_worldSession.Init() )
		return FALSE;

	if( FALSE == g_dbSession.Init() )
		return FALSE;

	if( FALSE == g_loginSession.Init() )
		return FALSE;

	if (FALSE == g_billSession.Init())
		return FALSE;

	if( FALSE == g_rtSession.Init() )
		return FALSE;

	if(FALSE == g_center_session.Init())
		return FALSE;

	// 移动初始化
	MoveData::m_Timer.Init();

	activity_mgr::GetInstance()->start();

	g_MasterPrenticeMgr.init( );

	g_GuerdonQuestMgr.init();
	
	g_lottery.Init();
	//sTeamShareMgr.Init();

	// 宝箱开启物品初始化
	//g_TreasureChestMgr.Init();

	// todo: gmsession
	// todo: 这些session是否也可以做成单个线程呢
	
	// 记录运行计时
	m_dwLastMinute = timeGetTime();
	m_dwLastHour = m_dwLastMinute;
	m_dwTotalRunMinute = 0;

	SetDancingGMPolicy( );
	// 系统参数
	m_nCpuNum = get_tool()->get_cup_num();

	// 启动update线程
	if( FALSE == World::p_thread->create_thread(_T("thread_update"),
		&World::static_thread_update, this) )
		return FALSE;

	// 注册服务器控制台命令(测试专用:正式版本需注释掉)
	//RegisterTestCmd(m_pConsole);

	return TRUE;
}

VOID World::SetDancingGMPolicy( )
{
	tagDWORDTime dwCurTime = GetCurrentDWORDTime( );
	if(dwCurTime.hour < AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_start_time)
		return ;
	if(dwCurTime.hour >= AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_end_time)
		return;

	INT nPersist = AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_end_time - dwCurTime.hour;
	g_GMPolicy.SetRate(EGMDT_Dacning, 
		AttRes::GetInstance()->GetVariableLen().dancing_factor,
		(DWORD)GetCurrentDWORDTime(), nPersist * 3600);
}

//-----------------------------------------------------------------------------
// destroy
//-----------------------------------------------------------------------------
VOID World::Destroy()
{
	// 关闭主循环线程
	Interlocked_Exchange((LONG*)&m_bTerminateUpdate, TRUE);
	World::p_thread->waitfor_thread_destroy(_T("thread_update"), INFINITE);

	// 关闭所有的地图线程
	g_mapCreator.stop_all_map_manager();

	// 保存所有玩家的信息
	g_roleMgr.save_all_role_to_db();
	
	UpdateOnlineNumber(0);

	// 保存其余信息到数据库
	SaveWorldInfoToDB();

	// 等待db处理完消息
	WaitDBTerminate();

	// 得到当前时间
	SYSTEMTIME sys_time;
	GetLocalTime(&sys_time);

	// 记录到log文件
	m_pLog->write_log(_T("shutdown at %02d/%02d/%04d %02d:%02d:%02d\r\n"),
		sys_time.wMonth, sys_time.wDay, sys_time.wYear,
		sys_time.wHour, sys_time.wMinute, sys_time.wSecond);

	//掉落
	delete g_drop_mgr.getSingletonPtr();

	RankMgr::Destory();
	activity_mgr::Destory();
	g_groupMgr.Destroy();
	g_roleMgr.destroy();
	g_mapCreator.destroy();
	g_rtSession.Destroy();
	g_billSession.Destroy();
	g_center_session.Destroy();
	g_loginSession.Destroy();
	g_worldSession.Destroy();
	g_dbSession.Destroy();
	//AttRes::GetInstance()->Destroy();
	AttRes::Destory();
	g_guild_manager.destroy();
	g_questMgr.destroy();
	g_mall.Destroy();
	g_paimai.destroy();
	g_bankmgr.destroy();
	g_ScriptMgr.Destroy();
	g_TreasureChestMgr.Destroy();
	g_mailmgr.Destroy();
	g_MasterPrenticeMgr.destroy( );
	g_petSnsMgr.destory();
	g_GuerdonQuestMgr.destroy();
	g_pvp_mgr.destroy();
	NPCTeamMgr::DestoryNPCOrder();
	//sTeamShareMgr.Destroy();
	WorldNetCmdMgr::Destroy();

	// 销毁窗口及GUI等系统
	SAFE_DELETE(g_pwindow);
	//get_window()->destroy();
	m_pLog->close_file();



	
}

//---------------------------------------------------------------------------------
// 保存游戏世界的信息
//---------------------------------------------------------------------------------
VOID World::SaveWorldInfoToDB()
{
	g_guild_manager.send_all_guild_to_db();
	activity_mgr::GetInstance()->save_all_to_db();
	g_MasterPrenticeMgr.force_save_all( ); 
	//sTeamShareMgr.ForceAllDelDB( );
}

//-----------------------------------------------------------------------------
// Main loop
//-----------------------------------------------------------------------------
VOID World::Mainloop()
{
	DWORD dwMsg=0, dwParam1=0, dwParam2=0;
	static DWORD dwTimeKeepr = timeGetTime();

	INT idx = 0;

	const std::vector<tstring> &SVR_MAPS = g_mapCreator.GetMapNames( );
	const std::vector<tstring> &SVR_INTS = g_mapCreator.GetInstanceNames( );

	while( FALSE == get_window()->message_loop() && FALSE == IsShutingdown() )
	{
		if( FALSE == get_window()->is_window_active() )
		{
			Sleep(30);
			continue;
		}

		while( get_window()->peek_window_message(dwMsg, dwParam1, dwParam2) )
		{
			if( dwMsg == WM_QUIT )
			{
				return;
			}
			if(dwMsg == WM_MYSELF_USER)
			{
				if(dwParam1 == 1)
				{
					get_window()->print_list();
				}
			}
		}

		UpdateMemoryUsage();

		m_dwTotalRunMinute = timeGetTime() - dwTimeKeepr;

		INT nHour = m_dwTotalRunMinute / 3600000;
		INT nMin = (m_dwTotalRunMinute % 3600000) / 60000;
		INT nSec = ((m_dwTotalRunMinute % 3600000) % 60000) / 1000;

		// 加入要观察的实时变量
		get_window()->watch_info(_T("version"), STWORLD_BUILD_REVISION);

		get_window()->watch_info(_T("Recvaddnum"), g_worldSession.GetRecvAddNum());
		get_window()->watch_info(_T("Recvdropnum"), g_worldSession.get_recv_drop_num());
		get_window()->watch_info(_T("addnum"), g_worldSession.GetAddNum());
		get_window()->watch_info(_T("dropnum"), g_worldSession.GetDropNum());

		get_window()->watch_info(_T("cpu_num:"), m_nCpuNum);
		get_window()->watch_info(_T("mem_total:"), m_dwTotalPhys/1024/1024);
		get_window()->watch_info(_T("mem_avail:"), m_dwAvailPhys/1024/1024);
		get_window()->watch_info(_T("non_paged_pool_usage:"), m_nQuotaNonPagedPoolUsage);
		get_window()->watch_info(_T("run_minute"), m_dwTotalRunMinute);
		get_window()->watch_info(_T("sec: "),					nSec);
		get_window()->watch_info(_T("min: "),					nMin);
		get_window()->watch_info(_T("hour: "),					nHour);
		get_window()->watch_info(_T("world_tick"), GetWorldTick());
		get_window()->watch_info(_T("idle_time"), GetIdleTime());
		get_window()->watch_info(_T("free_client_num: "), g_worldSession.GetFreeClientNum());
		get_window()->watch_info(_T("accept_num: "), g_worldSession.GetAcceptNum());
		get_window()->watch_info(_T("LeaveRole: "), dw_leave_role_num);
		get_window()->watch_info(_T("online_num"), g_worldSession.GetPlayerNumCurrent());
		get_window()->watch_info(_T("online_peek"), g_worldSession.GetPlayerNumPeek());
		get_window()->watch_info(_T("online_limit"), g_worldSession.GetPlayerNumLimit());
		get_window()->watch_info(_T("login_times"), g_worldSession.GetPlayerLoginTimes());
		get_window()->watch_info(_T("logout_times"), g_worldSession.GetPlayerLogoutTimes());
		get_window()->watch_info(_T("max_msg"), GetMaxMsgSize());
		get_window()->watch_info(_T("send_cast"), g_worldSession.GetMsgSendCast());
		get_window()->watch_info(_T("io_pending"), g_worldSession.get_io_pending());
		get_window()->watch_info(_T("io_pending_id"), g_worldSession.get_io_pending_id());
		get_window()->watch_info(_T("db_recv"), g_dbSession.GetReceivedPackageNum());
		get_window()->watch_info(_T("db_unsend"), g_dbSession.GetUnsendPackageNum());
		get_window()->watch_info(_T("send_size"), g_worldSession.GetTotalSendSize());
		get_window()->watch_info(_T("real_send"), g_worldSession.GetTotalRealSendSize());
		get_window()->watch_info(_T("sec_real_send"), g_worldSession.get_sec_send_size());
		get_window()->watch_info(_T("com_ratio"), g_worldSession.GetSendComRatio());
		get_window()->watch_info(_T("instance_nolimit"), g_mapCreator.get_instance_num(EMIG_NOLIMIT));
		get_window()->watch_info(_T("instance_512"), g_mapCreator.get_instance_num(EMIG_512));
		get_window()->watch_info(_T("instance_1024"), g_mapCreator.get_instance_num(EMIG_1024));
		get_window()->watch_info(_T("instance_2048"), g_mapCreator.get_instance_num(EMIG_2048));
		get_window()->watch_info(_T("instance_3072"), g_mapCreator.get_instance_num(EMIG_3072));
		get_window()->watch_info(_T("instance_coefnum"), g_mapCreator.get_instance_coef_num());
		get_window()->watch_info(_T("instance_coeflimit"), g_mapCreator.get_instance_coef_num_limit());


		get_window()->watch_info(_T("=== Instance's Number ="), SVR_INTS.size() );
		for(idx = 0; idx < SVR_INTS.size(); idx++){
			get_window()->watch_info( SVR_INTS[idx].c_str(), mInstanceRoleNum[idx]);
		}

		get_window()->watch_info(_T("=== Map's Number ="), SVR_MAPS.size());
		for(idx = 0; idx < SVR_MAPS.size(); idx++){
			get_window()->watch_info( SVR_MAPS[idx].c_str(), 
			g_mapCreator.get_map_role_num(get_tool()->crc32(SVR_MAPS[idx].c_str())),
			g_mapCreator.get_update_time(get_tool()->crc32(SVR_MAPS[idx].c_str())));
		}

		get_window()->watch_info(_T("==== Logic Update  ===="), 1);
		get_window()->watch_info(_T("MoveData: "), mUpdateGather.dwMoveData);
		get_window()->watch_info(_T("GMPolicy: "), mUpdateGather.dwGMPolicy);
		get_window()->watch_info(_T("WordSession: "), mUpdateGather.dwWordSession);
		get_window()->watch_info(_T("LoginSession: "), mUpdateGather.dwLoginSession);
		get_window()->watch_info(_T("DBSession: "), mUpdateGather.dwDBSession);
		get_window()->watch_info(_T("BillSession: "), mUpdateGather.dwBillSession);
		get_window()->watch_info(_T("ExchangeSession: "), mUpdateGather.dwExchangeSession);
		get_window()->watch_info(_T("RtSession: "), mUpdateGather.dwRtSession);
		get_window()->watch_info(_T("map_creator: "), mUpdateGather.dwMapCreator);
		get_window()->watch_info(_T("SocialMgr: "), mUpdateGather.dwSocialMgr);
		get_window()->watch_info(_T("FameHall: "), mUpdateGather.dwFameHall);
		get_window()->watch_info(_T("WorldNetCmdMgr: "), mUpdateGather.dwWorldNetCmdMgr);
		get_window()->watch_info(_T("GroupMgr: "), mUpdateGather.dwGroupMgr);
		get_window()->watch_info(_T("ActivityMgr: "), mUpdateGather.dwActivityMgr);
		get_window()->watch_info(_T("PSGuarder: "), mUpdateGather.dwPSSafeGuarder);
		get_window()->watch_info(_T("GuildMgr: "), mUpdateGather.dwGuildMgr);
		get_window()->watch_info(_T("Mall: "), mUpdateGather.dwMall);
		get_window()->watch_info(_T("PaiMai: "), mUpdateGather.dwPaiMai);
		get_window()->watch_info(_T("Bank: "), mUpdateGather.dwBank);
		get_window()->watch_info(_T("VIPStall: "), mUpdateGather.dwVIPStall);
		get_window()->watch_info(_T("ScriptMgr: "), mUpdateGather.dwScriptMgr);
		get_window()->watch_info(_T("GameGuarder: "), mUpdateGather.dwGameGuarder);
		get_window()->watch_info(_T("MailMgr: "), mUpdateGather.dwMailMgr);
		get_window()->watch_info(_T("XSQuestMgr: "), mUpdateGather.dwXSQuestMgr);
		get_window()->watch_info(_T("MasterPrenticeMgr: "), mUpdateGather.dwMasterPrenticeMgr);
		get_window()->watch_info(_T("PVPMgr: "), mUpdateGather.dwPVPMgr);
		get_window()->watch_info(_T("AutoPaimai: "), mUpdateGather.dwAutoPaimai);
		get_window()->watch_info(_T("Word::OnMinute: "), mUpdateGather.dwOnMinute);
		get_window()->watch_info(_T("World:SaveRolePerTick: "), mUpdateGather.dwRoleMgrSavePerTick);
		get_window()->watch_info(_T("lua_memory: "), g_ScriptMgr.GetMemcory());

		Sleep(50);
	}
	SI_LOG->write_log(_T("shut down::%d"), IsShutingdown());	
}

//-----------------------------------------------------------------------------
// Update线程
//-----------------------------------------------------------------------------
UINT World::thread_update()
{
	INT nGarbage = 0x7fffffff;
	INT nLastTimeGarbage = 0x7fffffff;
	INT nIncreaseCounter = 0;

	//modify mmz at 2010.9.17 release也记dump
//#ifdef _DEBUG
	_set_se_translator(serverdump::si_translation);

	try
	{
//#endif
		UpdateTimeCalcAux __timeAux;
		while( FALSE == m_bTerminateUpdate )
		{
			LARGE_INTEGER m_liPerfFreq;
			LARGE_INTEGER m_liPerfStart;
			QueryPerformanceFrequency(&m_liPerfFreq);
			QueryPerformanceCounter(&m_liPerfStart);

			// 锁住网络
			LockNetWork();

			SetInstanceRoleNum();
			//SetLeaveRoleNum();

			__timeAux.Start();
			// 移动update
			MoveData::m_Timer.Update();
			mUpdateGather.dwMoveData = __timeAux.elapse();

			// g_GMPolicy
			g_GMPolicy.Update();
			mUpdateGather.dwGMPolicy = __timeAux.elapse();

			// g_worldSession
			g_worldSession.Update();
			mUpdateGather.dwWordSession = __timeAux.elapse();

			// g_loginSession
			g_loginSession.Update();
			mUpdateGather.dwLoginSession = __timeAux.elapse();

			// g_dbSession
			g_dbSession.Update();
			mUpdateGather.dwDBSession = __timeAux.elapse();

			// g_billSession
			g_billSession.Update();
			mUpdateGather.dwBillSession = __timeAux.elapse();

			// g_center_session
			g_center_session.Update();
			mUpdateGather.dwExchangeSession = __timeAux.elapse();

			// g_rtSession
			g_rtSession.Update();
			mUpdateGather.dwRtSession = __timeAux.elapse();

			// g_mapCreator
			g_mapCreator.update();
			mUpdateGather.dwMapCreator = __timeAux.elapse();

			// g_socialmgr
			g_socialMgr.update();
			mUpdateGather.dwSocialMgr = __timeAux.elapse();

			// g_fameHall
			//g_fameHall.Update();
			//mUpdateGather.dwFameHall = __timeAux.elapse();

			// g_worldNetCmdMgr
			WorldNetCmdMgr::GetInstance()->Update();
			mUpdateGather.dwWorldNetCmdMgr = __timeAux.elapse();

			// g_groupmgr
			g_groupMgr.Update();
			mUpdateGather.dwGroupMgr = __timeAux.elapse();

			// g_activityMgr
			activity_mgr::GetInstance()->update();
			mUpdateGather.dwActivityMgr = __timeAux.elapse();

			// g_pSGuarder
			g_pSGuarder.Update();
			mUpdateGather.dwPSSafeGuarder = __timeAux.elapse();

			// g_guild_manager
			g_guild_manager.update();
			mUpdateGather.dwGuildMgr = __timeAux.elapse();

			// g_mall
			g_mall.Update();
			mUpdateGather.dwMall = __timeAux.elapse();

			// g_paimai
			//g_paimai.update();
			//mUpdateGather.dwPaiMai = __timeAux.elapse();

			// g_bankmgr
			//g_bankmgr.update();
			//mUpdateGather.dwBank = __timeAux.elapse();

			// g_VIPStall			
			g_VIPStall.Update();
			mUpdateGather.dwVIPStall = __timeAux.elapse();

			// g_scriptMgr			
			g_ScriptMgr.Update();
			mUpdateGather.dwScriptMgr = __timeAux.elapse();

			// g_gameGuarder
			g_gameGuarder.Update();
			mUpdateGather.dwGameGuarder = __timeAux.elapse();

			//g_mailmgr.Update();
			//mUpdateGather.dwMailMgr = __timeAux.elapse();

			//g_GuerdonQuestMgr.update();
			//mUpdateGather.dwXSQuestMgr = __timeAux.elapse();

			//gx modify 2013.12.12
			g_MasterPrenticeMgr.update( );
			mUpdateGather.dwMasterPrenticeMgr = __timeAux.elapse();

			//g_pvp_mgr.update();
			//mUpdateGather.dwPVPMgr			= __timeAux.elapse();

			//sTeamShareMgr.Update( );

			//g_auto_paimai.Update();
			//mUpdateGather.dwAutoPaimai		= __timeAux.elapse();

			/*if( timeGetTime() - m_dwLastMinute >= 1000 )
			{
				g_roleMgr.create_system_mail();
			}*/

			// 基本周期运算，每分钟执行一次
			if( timeGetTime() - m_dwLastMinute >= 60*1000 )
			{ 
				m_dwLastMinute += 60*1000;
				OnMinute();
			}
			mUpdateGather.dwOnMinute = __timeAux.elapse();

			// 保存玩家
			SaveRolePerTick();
			mUpdateGather.dwRoleMgrSavePerTick = __timeAux.elapse();

			// 做最后的处理，这个必须要发到所有update之后
			g_worldSession.DoHouseKeeping();

			// 服务器关闭倒计时检测
			OnShutDown();

			// 解锁
			UnlockNetWork();

			// 计算sleep时间
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			__int64 totalTime = (liPerfNow.QuadPart- m_liPerfStart.QuadPart) * 1000000 / m_liPerfFreq.QuadPart;
			/*PRINT_MESSAGE(_T("Use Tick %ld\r\n"), totalTime);*/

			DWORD dw_time = (DWORD)(totalTime / 1000);
			if( dw_time < TICK_TIME )
			{
				m_dwIdleTime = TICK_TIME - dw_time;
				Sleep(TICK_TIME - dw_time);
			}
			else
			{
				m_dwIdleTime = 0;
			}

			SetWorldTickInc();
			UpdateWorldTime();
		}
//#ifdef _DEBUG
	} 
	catch(serverdump::throw_exception)
	{
		if( get_tool()->is_debug_present() )
			throw;
		else
		{
			SI_LOG->write_log(_T("world::thread_update shut down"));	
			// 设置ShutDown
			ShutDown();
		}
	}
//#endif
	_endthreadex(0);

	return 0;
}

UINT World::static_thread_update(LPVOID p_data)
{
	ASSERT(VALID_POINT(p_data));
	World* p_this = (World*)p_data;
	return p_this->thread_update();
}

//-----------------------------------------------------------------------------
// 每分钟执行一次
//-----------------------------------------------------------------------------
VOID World::OnMinute()
{
	// 基本周期运算，每小时执行一次
	DWORD dw = timeGetTime();
	// 当前的时间
	tagDWORDTime cur_time = GetCurrentDWORDTime();

	if( dw - m_dwLastHour >= 60*60*1000 )
	{ 
		m_dwLastHour += 60*60*1000;
		OnHour();
	}

	if (cur_time.min == 0)
	{
		OnClock(cur_time.hour);
	}

	activity_mgr::GetInstance()->on_minute(cur_time);
	
	BattleGround::get_singleton().update(cur_time);

	UpdateOnlineNumber(g_worldSession.GetPlayerNumCurrent());
	
}

VOID World::UpdateOnlineNumber(INT nNum)
{
	// 更新在线人数
	NET_DB2C_update_online_num send;
	get_tool()->unicode_to_unicode8(g_world.GetWorldName(), send.szName);
	send.nNum = nNum;
	g_dbSession.Send(&send, send.dw_size);
}

VOID World::OnShutDown()
{
	if(!b_ShutDown_Time)
		return;

	if(CalcTimeDiff(GetCurrentDWORDTime(), dw_ShutDown_Begin) >= 60)
	{
		if(n_ShutDown_Time == 0)
		{
			SI_LOG->write_log(_T("onshut down"));	
			ShutDown();
		}	
		--n_ShutDown_Time;

		if(n_ShutDown_Time > 0)
		{
			NET_SIS_ShutDown_Time send;
			send.nTime = n_ShutDown_Time;
			g_roleMgr.send_world_msg(&send, send.dw_size);
		}

		dw_ShutDown_Begin = GetCurrentDWORDTime();
	}
}

//-----------------------------------------------------------------------------
// 每小时执行一次
//-----------------------------------------------------------------------------
VOID World::OnHour()
{
	g_tradeYB.OnHour();
}

//-----------------------------------------------------------------------------
// 每整点执行一次
//-----------------------------------------------------------------------------
VOID World::OnClock(BYTE byHour)
{
	switch (byHour)
	{
	case 0:
		{
			//g_guild_manager.daily_guild_war_state_reset();
			//g_guild_manager.daily_guild_lobby_use();
			//g_guild_manager.guild_quest_reset( );
			
			// 清除循环及日副本限制
			g_roleMgr.delete_role_map_limit(2);
			g_roleMgr.delete_role_map_limit(0);

			// 清除周副本限制
			DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
			if(dwWeek == 6)
			{
				g_roleMgr.delete_role_map_limit(1);
			}

			//g_roleMgr.clear_role_hang_number();
			//g_roleMgr.clear_role_paimai_number();
			// 清除幸运值
			//g_roleMgr.clear_role_luck();
			// 0点重置的数据
			g_roleMgr.reset_role_data();
			g_roleMgr.inc_circle_refresh_number( );

			g_roleMgr.reset_role_data_six();//gx add
			g_roleMgr.update_role_sign_level();//gx add 2013.10.28

			// 更新排行榜
			RankMgr::GetInstance()->UpdateRank();
			break;
		}
	case 1:
		{	
			//
			break;
		}
	//case 2:
	//	{
	//		// 扣除帮派每日消耗并重置帮务状态
	//		//g_guild_manager.daily_guild_reset();
	//		DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
	//		if(dwWeek == 2)
	//		{
	//			g_roleMgr.update_week_score();
	//			RankMgr::GetInstance()->UpdateRank();
	//			g_guild_manager.week_clear_enemy();
	//		}
	//		else
	//		{
	//			RankMgr::GetInstance()->UpdateRank();
	//			g_roleMgr.update_1v1_day_score();
	//		}
	//		g_bankmgr.cal_radio();
	//		break;
	//	}
	//case 3:
	//	{
	//		DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
	//		if(dwWeek == 2)
	//		{
	//			g_guild_manager.clear_guild_holiness_num();
	//			g_guild_manager.clean_guild_recruit_to_db();
	//			RankMgr::GetInstance()->Role1v1GiveAward();
	//		}
	//	}
	//	break;
	//case 4:
	//	{
	//		// 整点清除玩家当天发件数量
	//		g_mailmgr.ClearRoleSendMainNum();

	//		g_auto_paimai.reset_auto_paimai_inventory();

	//		break;
	//	}
	case 6:
		{
			// 回收沙巴克
			DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
			if (dwWeek == 5)
			{
				g_guild_manager.resetSBKData();
			}
			//g_roleMgr.reset_role_data_six();//gx add 
			break;
		}
	//case 20:
	//	{
	//		RankMgr::GetInstance()->UpdateRank();
	//		break;
	//	}
	//case 21:
	//	{
	//		DWORD dwWeek = WhichWeekday(GetCurrentDWORDTime());
	//		if(dwWeek == 0)
	//		{
	//			RankMgr::GetInstance()->RoleShihunGiveAward();
	//			//g_roleMgr.update_shihun();魅力值不清空 gx modify 2013.8.23
	//		}
	//		break;
	//	}
	}

	//g_roleMgr.on_sharp_hour(byHour);
	// 所有整点动作
	//g_guild_manager.upgrade_guild_skill();
	g_mapCreator.onclock(byHour);
	//if((byHour == AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_start_time)){
	//	INT nPersist = AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_end_time - byHour;

	//	g_GMPolicy.SetRate(EGMDT_Dacning, 
	//		AttRes::GetInstance()->GetVariableLen().dancing_factor,
	//		(DWORD)GetCurrentDWORDTime(), nPersist * 3600);
	//} else if(byHour == AttRes::GetInstance()->GetVariableLen( ).dancing_multiple_end_time){
	//	g_GMPolicy.SetRate(EGMDT_Dacning, 100, (DWORD)GetCurrentDWORDTime(), 1);
	//}
}

//-----------------------------------------------------------------------------
// 保存玩家信息
//-----------------------------------------------------------------------------
VOID World::SaveRolePerTick()
{
	if(m_dwWorldTick - m_dwLastSaveTick < m_bySaveOneNeedTicks)
	{
		return;
	}

	m_dwLastSaveTick = m_dwWorldTick;

	g_roleMgr.save_one_to_db();
}

//-----------------------------------------------------------------------------
// crc32方法封装 -- 先转换为小写再计算
//-----------------------------------------------------------------------------
DWORD World::LowerCrc32(LPCTSTR str)
{
	tstring strTmp = str;
	transform(strTmp.begin(), strTmp.end(), strTmp.begin(), tolower);

	return get_tool()->crc32(strTmp.c_str());
}

//-----------------------------------------------------------------------------
// 更新内存的占用情况
//-----------------------------------------------------------------------------
VOID World::UpdateMemoryUsage()
{
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatus(&memStatus);

	m_dwTotalPhys = memStatus.dwTotalPhys;
	m_dwAvailPhys = memStatus.dwAvailPhys;

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

	m_nQuotaNonPagedPoolUsage = pmc.QuotaNonPagedPoolUsage;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
VOID World::SwitchGameGuarder( BOOL bOn )
{
	g_gameGuarder.Switch(bOn);
}

//-----------------------------------------------------------------------------
// 服务器状态是否良好
//-----------------------------------------------------------------------------
BOOL World::IsWell() const
{
	if( FALSE == g_dbSession.IsWell() )
		return FALSE;

	return TRUE;
}

//-----------------------------------------------------------------------------
// 等待db返回
//-----------------------------------------------------------------------------
VOID World::WaitDBTerminate()
{
	NET_DB2C_world_shutdown send;

	g_dbSession.Send(&send, send.dw_size);

	DWORD dw_size = 0;
	tag_net_message* p_receive = (tag_net_message*)g_dbSession.Recv(dw_size);
	NET_DB2S_world_shutdown RtMsg;

	while(!VALID_POINT(p_receive) || (VALID_POINT(p_receive) && RtMsg.dw_message_id != p_receive->dw_message_id))
	{
		Sleep(100);
		p_receive = (tag_net_message*)g_dbSession.Recv(dw_size);
	}

}

VOID World::set_shutdown_time(INT n_Time)
{
	Interlocked_Exchange((LPLONG)&b_ShutDown_Time, TRUE);
	n_ShutDown_Time = n_Time;
	dw_ShutDown_Begin = GetCurrentDWORDTime();

	NET_SIS_ShutDown_Time send;
	send.nTime = n_Time;
	g_roleMgr.send_world_msg(&send, send.dw_size);
}

DWORD	World::get_open_server_day()
{
	tagDWORDTime dwOpenTime;
	dwOpenTime.year = World::p_var->get_dword(_T("year open"));
	dwOpenTime.month = World::p_var->get_dword(_T("month open"));
	dwOpenTime.day = World::p_var->get_dword(_T("day open"));
	dwOpenTime.hour = 0;
	dwOpenTime.min = 2;
	dwOpenTime.sec = 0;

	DWORD dwDay = CalcTimeDiff(GetCurrentDWORDTime(), dwOpenTime) / (24 * 60 * 60);


	return dwDay;
}

VOID World::SetInstanceRoleNum()
{
	const std::vector<tstring>& insname = g_mapCreator.GetInstanceNames( );
	if(insname.size())
	{
		for(size_t t = 0; t < insname.size( ); t++)
			mInstanceRoleNum[t] = g_mapCreator.get_instance_role_num(get_tool()->crc32(insname[t].c_str()));
	}
	/*	
	dw_inst_num[0] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("j02")));
	dw_inst_num[1] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s02-a")));
	dw_inst_num[2] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s05-a")));
	dw_inst_num[3] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s06-a")));
	dw_inst_num[4] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s06-b")));
	dw_inst_num[5] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s07-a")));
	dw_inst_num[6] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s08-a")));
	dw_inst_num[7] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s09-a")));
	dw_inst_num[8] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s10-a")));
	dw_inst_num[9] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s11-a")));
	dw_inst_num[10] = g_mapCreator.get_instance_role_num(get_tool()->crc32(_T("s13-a")));
	*/
}

VOID World::SetLeaveRoleNum()
{
	dw_leave_role_num = 0;
	dw_leave_role_num += g_mapCreator.get_instance_leave_role_num(get_tool()->crc32(_T("s01")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s03")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s04")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s05")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s06")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s07")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s08")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s09")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s10")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s11")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s12")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s13")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s20")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s06-1")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s06-2")));
	dw_leave_role_num += g_mapCreator.get_map_leave_role_num(get_tool()->crc32(_T("s06-3")));
}

World g_world;

//-----------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------
INT APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, INT)
{
	// 设置进程优先级
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	
	// 禁止屏幕保护及电源管理
	::SystemParametersInfo(SPI_SETLOWPOWERTIMEOUT, 0, NULL, 0);
	::SystemParametersInfo(SPI_SETPOWEROFFTIMEOUT, 0, NULL, 0);
	::SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 0, NULL, 0);

	memorysystem::init_new_protect();
	serverbase::init_network();
	serverbase::init_serverbase();


//#ifdef _DEBUG
	THROW_EXCEPTION;
//#endif

	if( g_world.Init(hInst) )
	{
		g_world.Mainloop();
	}

	g_world.Destroy();
	
	//WriteMem();

	serverbase::destroy_serverbase();
	serverbase::destroy_network();

	

	return 0;
}


