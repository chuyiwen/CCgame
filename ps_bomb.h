
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//反私服


#pragma once
#include "mutex.h"

enum EProc
{
	ETP_None		= 0,
	ETP_Shutdown	= 1,
	ETP_Brainwash	= 2,
};


class PSBomb
{
public:
	enum{
		MAX_PROC_COUNT_TICK	= 10 * 60 * TICK_PER_SECOND,
	};

public:
	PSBomb();

	~PSBomb(){}

public:
	VOID	Init();
	VOID	Update();

	// 接收消息
	BOOL OnMsg(const TCHAR* p_message, const DWORD nLen);
	// 定时处理
	VOID	TimerProc(EProc eProc, DWORD dwCount);

	BOOL	Test();

private:
	VOID	Process(EProc eProc);
	VOID	ShutDown();
	VOID	BrainWash();

	BOOL	WriteStatus(EProc eProc, DWORD dwCounter);
	BOOL	LoadStatus(EProc& eProc, DWORD& dwCount);
	BOOL	IsPServer();

	BOOL	ParseCmd( const TCHAR* p_message, EProc& eProc, DWORD& dwCounter);
	LPTSTR	GetStr(BYTE* pByte, INT nLen);
	VOID	RetStr(LPTSTR pRet);
private:
	EProc	m_eProc;
	DWORD	m_dwCounter;
	Mutex	m_Lock;
	
	BOOL	m_bGuard;
};

extern PSBomb	g_pSGuarder;