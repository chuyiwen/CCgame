#pragma once

#include "mutex.h"

//----------------------------------------------------------------------------------------------------
// ����id������
//----------------------------------------------------------------------------------------------------
class IDGen
{
public:
	VOID	InitLastValidID(DWORD dwLastValidID);
	DWORD	GenValidID();

private:
	VOID	PetIDErr();
private:
	DWORD	m_LastValidID;
	Mutex	m_Lock;
};

extern IDGen g_petIDGen;