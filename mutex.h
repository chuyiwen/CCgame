#pragma once

#include "StdAfx.h"

//------------------------------------------------------------------------------------------------------
// 临界区简单实现类
//------------------------------------------------------------------------------------------------------
class Mutex
{
public:
	Mutex()		{ if(FALSE == InitializeCriticalSectionAndSpinCount(&cs, 4000)) { abort(); } nCishu = 0; }
	~Mutex()	{ nCishu; DeleteCriticalSection(&cs); }

	VOID Acquire() { EnterCriticalSection(&cs); Interlocked_Exchange_Add((LPLONG)&nCishu, 1); }
	VOID Release() { LeaveCriticalSection(&cs); Interlocked_Exchange_Add((LPLONG)&nCishu, -1); }
	BOOL TryAcquire() 
	{
	#if(_WIN32_WINNT >= 0x0400)
		return TryEnterCriticalSection(&cs); 
	#else
		return FALSE;
	#endif
	}

private:
	CRITICAL_SECTION cs;
	volatile INT nCishu;

};

//-------------------------------------------------------------------------------------------------------
// 事件简单实现类
//-------------------------------------------------------------------------------------------------------
class Event
{
public:
	Event() { m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); ASSERT(m_hEvent != NULL); }
	Event(BOOL bManual, BOOL bInitState) { m_hEvent = CreateEvent(NULL, bManual, bInitState, NULL); ASSERT(m_hEvent != NULL); }
	~Event() { CloseHandle(m_hEvent); }

	VOID Set() { SetEvent(m_hEvent); }
	VOID ReSet() { ResetEvent(m_hEvent); }
	VOID Pulse() { PulseEvent(m_hEvent); }
	VOID Wait()	{ WaitForSingleObject(m_hEvent, INFINITE); }

private:
	HANDLE	m_hEvent;
};