
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ϸ�������

#pragma once

#include "mutex.h"
//#include "mem_map.h"

#define INST_NUM		11
//-----------------------------------------------------------------------------
// ����������������
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
	// crc32������װ -- ��ת��ΪСд�ټ���
	DWORD	LowerCrc32(LPCTSTR str);

public:
	static file_container*	p_var;
	static thread_manager*			p_thread;

private:
	// ����������Ϸ������������
	VOID	SaveWorldInfoToDB();
	VOID	WaitDBTerminate();

	VOID	SetInstanceRoleNum();
	VOID	SetLeaveRoleNum();
private:
	

	file_system*			pVFS;
	
	log_file*					m_pLog;

	volatile BOOL			m_bShutingdown;						// �������Ƿ����ڹر�
	volatile BOOL			m_bTerminateUpdate;					// �������߳�ֹͣλ
	volatile BOOL			m_bOverloadAlert;					// ���������ؾ���
	volatile DWORD			m_dwIdleTime;						// �ϴη�������ѭ������ʱ��
	volatile BYTE			m_bySaveOneNeedTicks;				// ����Tick����һ�����

	volatile DWORD			m_dwMaxMsgSize;

	DWORD					m_dwViewWidth;
	DWORD					m_dwViewHeight;

	INT						m_nCpuNum;
	DWORD					m_dwTotalPhys;
	DWORD					m_dwAvailPhys;
	INT						m_nQuotaNonPagedPoolUsage;

	tstring					m_strWorldName;						// ��Ϸ��������
	DWORD					m_dwGoldenCode;						// �����������golden_code

	DWORD					m_dwLastMinute;
	DWORD					m_dwLastHour;
	DWORD					m_dwTotalRunMinute;
	volatile DWORD			m_dwWorldTick;						// ��Ϸ��������
	tagDWORDTime			m_CurrentTime;						// ��ǰϵͳʱ��
	DWORD					m_dwLastSaveTick;					// ��һ�α�����Ҽ�¼����

	FLOAT					m_fLaxCheatDistanceSQ;				// ���������µľ���ƽ��
	FLOAT					m_fStrictCheatDistanceSQ;			// ǿ�������µľ���ƽ��
	FLOAT					m_fLaxJumpCheatDistanceSQ;
	FLOAT					m_fStrictJumpCheatDistanceSQ;

	INT						m_strength_check_time;
	INT						m_strength_check_num;
	INT						m_weak_check_time;
	INT						m_weak_check_num;
	INT						m_ce_check_num;

	Mutex					m_MutexNetwork;						// ���������������ײ��logincallback��logoutcallback���л���

	volatile BOOL			b_ShutDown_Time;					// �Ƿ������رյ���ʱ
	INT						n_ShutDown_Time;					// �رյ���ʱʱ��
	DWORD					dw_ShutDown_Begin;					// �ر���ʼʱ��

	BOOL					b_Join_World;						// �Ƿ����������Ϸ����

private:
	// Update�߳�
	UINT thread_update();
	static UINT WINAPI static_thread_update(LPVOID p_data);

	// ��¼���������ߵ��ö�ջ��Ϣ
//	DWORD RecordStack(LPCSTR);
	// �޸Ķ��ٸ�tick����һ���������
	DWORD ChangeSaveTicks(LPCSTR szTicks);
	// ÿ����ִ��һ��
	VOID OnMinute();
	// ÿСʱִ��һ��
	VOID OnHour();
	// ÿ����ִ��һ��
	VOID OnClock(BYTE byHour);
	// ���������Ϣ
	VOID SaveRolePerTick();

	// ������������
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