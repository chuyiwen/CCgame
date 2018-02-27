#include "stdafx.h"
#include "Timer.h"
#include <MMSystem.h>
#pragma comment( lib, "winmm.lib" )


void Timer::Init()
{
	m_dwLastTime=timeGetTime();
	m_dwCurrentTime=timeGetTime();
}

void Timer::Update()
{
	m_dwCurrentTime=timeGetTime();

	m_dwDelta=m_dwCurrentTime-m_dwLastTime;
	m_dwElapse+=m_dwDelta;

	m_fElapse=m_dwElapse*0.001f;
	m_fDelta=m_dwDelta*0.001f;

	//--
	m_dwLastTime=m_dwCurrentTime;
}

Timer::Timer(void)
{
	m_dwLastTime=0;		
	m_dwCurrentTime=0;
	m_dwElapse=0;		
	m_dwDelta=0;		

	m_fElapse=0.0f;		
	m_fDelta=0.0f;		
}

Timer::~Timer(void)
{
}
	
