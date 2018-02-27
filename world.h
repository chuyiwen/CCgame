
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

#pragma once

#include "mutex.h"
//#include "mem_map.h"

#define INST_NUM		11
//-----------------------------------------------------------------------------
// 区域服务器主框架类
//-----------------------------------------------------------------------------
class World
{

public:
	World();
	~World();

	BOOL Init(HINSTANCE hInst);
	VOID Mainloop();
	VOID Destroy();

	BOOL	IsOverload() const			{ return m_bOverloadAlert; }
	DWORD	GetIdleTime() const			{ return m_dwIdleTime; }
	DWORD	GetTotalRunMinute() const	{ return m_dwTotalRunMinute; }
	BYTE	GetSaveOneTicks() const		{ return m_bySaveOneNeedTicks; }
	DWORD	GetGoldenCode() const		{ return m_dwGoldenCode; }
	LPCTSTR	GetWorldName() const		{ return m_strWorldName.c_str(); }
	INT		GetCpuNum()	const			{ return m_nCpuNum; }
	DWORD	GetTotalMemory() const		{ return m_dwTotalPhys; }
	DWORD	GetAvailMemory() const		{ return m_dwAvailPhys; }
	INT		GetQuotaNonPagedPoolUsage()	{ return m_nQuotaNonPagedPoolUsage; }

	VOID	ShutDown()					{ Interlocked_Exchange((LONG*)&m_bShutingdown, TRUE); }
	BOOL	IsShutingdown() const		{ return m_bShutingdown; }

	VOID	SwitchGameGuarder(BOOL bOn);
	BOOL	IsWell() const;
	VOID	UpdateMemoryUsage();	

	DWORD	GetWorldTick()				{ return m_dwWorldTick; }
	VOID	SetWorldTickInc()			{ Interlocked_Exchange_Add((LPLONG)&m_dwWorldTick, 1); }

	DWORD	GetMaxMsgSize()				{ return m_dwMaxMsgSize; }
	VOID	SetMaxMsgSize(DWORD dwMsgSize)	{ Interlocked_Exchange((LONG*)&m_dwMaxMsgSize, dwMsgSize); }

	tagDWORDTime GetWorldTime()			{ return m_CurrentTime; }
	VOID	UpdateWorldTime()			{ m_CurrentTime = GetCurrentDWORDTime(); }

	VOID	LockNetWork()				{ m_MutexNetwork.Acquire(); }
	VOID	UnlockNetWork()				{ m_MutexNetwork.Release(); }

	VOID	SetDancingGMPolicy( );
public:
	FLOAT	GetLaxCheatDistanceSQ()		{ return m_fLaxCheatDistanceSQ; }
	FLOAT	GetStrictCheatDistanceSQ()	{ return m_fStrictCheatDistanceSQ; }
	FLOAT	GetLaxJumpCheatDistanceSQ() { return m_fLaxJumpCheatDistanceSQ; }
	FLOAT	GetStrictJumpCheatDistanceSQ() { return m_fStrictJumpCheatDistanceSQ; }
	VOID	SetLaxCheatDistanceSQ(FLOAT fDistance) 
	{
		m_fLaxCheatDistanceSQ = fDistance * FLOAT(TILE_SCALE); 
		m_fLaxCheatDistanceSQ *= m_fLaxCheatDistanceSQ; 
	}
	VOID	SetLaxJumpCheatDistanceSQ(FLOAT fDistance) 
	{
		m_fLaxJumpCheatDistanceSQ = fDistance * FLOAT(TILE_SCALE); 
		m_fLaxJumpCheatDistanceSQ *= m_fLaxJumpCheatDistanceSQ;
	}
	VOID    SetCELimitNum(INT n_num)
	{
		m_ce_check_num = n_num;
	}

	INT		GetStrengthCheckTime() { return m_strength_check_time; }
	INT		GetStrengthCheckNum() { return m_strength_check_num; }
	INT		GetWeakCheckTime() { return m_weak_check_time; }
	INT		GetWeakCheckNum() { return m_weak_check_num; }
	INT		GetCeCheckNum() { return m_ce_check_num; }

	VOID	SetStrengthCheck(INT nTime, INT nNum)
	{
		m_strength_check_time = nTime;
		m_strength_check_num = nNum;
	}

	log_file*			get_log()			{ return m_pLog; }

	file_system* get_virtual_filesys() { return pVFS; }

	VOID	set_shutdown_time(INT n_Time);
	BOOL	get_shutdown_time() { return b_ShutDown_Time; }

	BOOL	is_join_world() { return b_Join_World; }

	DWORD	get_open_server_day();
public:
	// crc32方法封装 -- 先转换为小写再计算
	DWORD	LowerCrc32(LPCTSTR str);

public:
	static file_container*	p_var;
	static thread_manager*			p_thread;

private:
	// 保存所有游戏世界其他数据
	VOID	SaveWorldInfoToDB();
	VOID	WaitDBTerminate();

	VOID	SetInstanceRoleNum();
	VOID	SetLeaveRoleNum();
private:
	

	file_system*			pVFS;
	
	log_file*					m_pLog;

	volatile BOOL			m_bShutingdown;						// 服务器是否正在关闭
	volatile BOOL			m_bTerminateUpdate;					// 服务器线程停止位
	volatile BOOL			m_bOverloadAlert;					// 服务器超载警报
	volatile DWORD			m_dwIdleTime;						// 上次服务器主循环空闲时间
	volatile BYTE			m_bySaveOneNeedTicks;				// 多少Tick保存一个玩家

	volatile DWORD			m_dwMaxMsgSize;

	DWORD					m_dwViewWidth;
	DWORD					m_dwViewHeight;

	INT						m_nCpuNum;
	DWORD					m_dwTotalPhys;
	DWORD					m_dwAvailPhys;
	INT						m_nQuotaNonPagedPoolUsage;

	tstring					m_strWorldName;						// 游戏世界名称
	DWORD					m_dwGoldenCode;						// 区域服务器的golden_code

	DWORD					m_dwLastMinute;
	DWORD					m_dwLastHour;
	DWORD					m_dwTotalRunMinute;
	volatile DWORD			m_dwWorldTick;						// 游戏世界心跳
	tagDWORDTime			m_CurrentTime;						// 当前系统时间
	DWORD					m_dwLastSaveTick;					// 上一次保存玩家记录心跳

	FLOAT					m_fLaxCheatDistanceSQ;				// 弱检测情况下的距离平方
	FLOAT					m_fStrictCheatDistanceSQ;			// 强检测情况下的距离平方
	FLOAT					m_fLaxJumpCheatDistanceSQ;
	FLOAT					m_fStrictJumpCheatDistanceSQ;

	INT						m_strength_check_time;
	INT						m_strength_check_num;
	INT						m_weak_check_time;
	INT						m_weak_check_num;
	INT						m_ce_check_num;

	Mutex					m_MutexNetwork;						// 网络层锁，用于与底层的logincallback和logoutcallback进行互斥

	volatile BOOL			b_ShutDown_Time;					// 是否启动关闭倒计时
	INT						n_ShutDown_Time;					// 关闭倒计时时间
	DWORD					dw_ShutDown_Begin;					// 关闭起始时间

	BOOL					b_Join_World;						// 是否允许进入游戏世界

private:
	// Update线程
	UINT thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	// 记录服务器主线调用堆栈信息
//	DWORD RecordStack(LPCSTR);
	// 修改多少个tick保存一个玩家数据
	DWORD ChangeSaveTicks(LPCSTR szTicks);
	// 每分钟执行一次
	VOID OnMinute();
	// 每小时执行一次
	VOID OnHour();
	// 每整点执行一次
	VOID OnClock(BYTE byHour);
	// 保存玩家信息
	VOID SaveRolePerTick();

	// 更新在线人数
	VOID UpdateOnlineNumber(INT nNum);
	VOID OnShutDown();

private:
	std::vector<DWORD> mInstanceRoleNum;
	DWORD	dw_leave_role_num;
	struct UpdateTimeGather
	{
		DWORD dwMoveData;
		DWORD dwGMPolicy;
		DWORD dwWordSession;
		DWORD dwLoginSession;
		DWORD dwDBSession;
		DWORD dwBillSession;
		DWORD dwExchangeSession;
		DWORD dwRtSession;
		DWORD dwMapCreator;
		DWORD dwSocialMgr;
		DWORD dwFameHall;
		DWORD dwWorldNetCmdMgr;
		DWORD dwGroupMgr;
		DWORD dwActivityMgr;
		DWORD dwPSSafeGuarder;
		DWORD dwGuildMgr;
		DWORD dwMall;
		DWORD dwPaiMai;
		DWORD dwBank;
		DWORD dwVIPStall;
		DWORD dwScriptMgr;
		DWORD dwGameGuarder;
		DWORD dwMailMgr;
		DWORD dwXSQuestMgr;
		DWORD dwMasterPrenticeMgr;
		DWORD dwPVPMgr;
		DWORD dwAutoPaimai;
		DWORD dwOnMinute;
		DWORD dwRoleMgrSavePerTick;
		UpdateTimeGather(){ memset( this, 0, sizeof(*this));}
	};	

	UpdateTimeGather mUpdateGather;
};

extern World g_world;

#define SI_LOG		(g_world.get_log())