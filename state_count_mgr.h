
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���������ܵ�״̬��������״̬��һ��tagDWORDFlagArray��������������飬״̬�������ܳ���DWORD��λ����״̬�������ᳬ��127
#pragma once

#include "../../common/WorldDefine/dword_flag.h"

//---------------------------------------------------------------------------------
// ����������״̬������
//---------------------------------------------------------------------------------
template<INT nSize>
class CountStateMgr
{
public:
	//-----------------------------------------------------------------------------
	// CONSTRUCT
	//-----------------------------------------------------------------------------
	CountStateMgr() { ZeroMemory(m_sCount, sizeof(m_sCount)); }

	//-----------------------------------------------------------------------------
	// ��ʼ��
	//-----------------------------------------------------------------------------
	VOID Init(DWORD dwStateFlags);

	//-----------------------------------------------------------------------------
	// ����
	//-----------------------------------------------------------------------------
	VOID Reset();

	//-----------------------------------------------------------------------------
	// ���úͳ���״̬
	//-----------------------------------------------------------------------------
	VOID SetState(INT nIndex);
	VOID UnsetState(INT nIndex);

	//------------------------------------------------------------------------------
	// ��ѯ״̬
	//------------------------------------------------------------------------------
	BOOL IsInState(INT nIndex);

	//------------------------------------------------------------------------------
	// �õ�״̬��־λ����
	//------------------------------------------------------------------------------
	DWORD GetStateFlags();

private:
	tagDWORDFlagArray<nSize>		m_FlagsArray;			// ״̬��־λ����
	SHORT							m_sCount[nSize];		// ��������
};

//----------------------------------------------------------------------------------
// ��ʼ��
//----------------------------------------------------------------------------------
template<INT nSize>
inline VOID CountStateMgr<nSize>::Init(DWORD dwStateFlags)
{
	// ���ñ�־λ����
	m_FlagsArray.Init(dwStateFlags);

	// ��մ���
	ZeroMemory(m_sCount, sizeof(m_sCount));

	// ����ĳЩ�����־λ��������
	for(INT n =0; n < nSize; ++n)
	{
		if( m_FlagsArray.IsSet(n) ) ++m_sCount[n];
	}
}

//-----------------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------------
template<INT nSize>
inline VOID CountStateMgr<nSize>::Reset()
{
	m_FlagsArray.Init(0);

	ZeroMemory(m_sCount, sizeof(m_sCount));
}

//-----------------------------------------------------------------------------------
// ����״̬
//-----------------------------------------------------------------------------------
template<INT nSize>
inline VOID CountStateMgr<nSize>::SetState(INT nIndex)
{
	if( nIndex < 0 || nIndex >= nSize ) return;

	// �鿴���ü���
	if( m_sCount[nIndex] > 0 )
	{
		++m_sCount[nIndex];
	}
	else
	{
		m_FlagsArray.SetFlag(nIndex);
		m_sCount[nIndex] = 1;
	}
}

//-------------------------------------------------------------------------------------
// ����״̬
//-------------------------------------------------------------------------------------
template<INT nSize>
inline VOID CountStateMgr<nSize>::UnsetState(INT nIndex)
{
	if( nIndex < 0 || nIndex >= nSize ) return;

	if( m_sCount[nIndex] > 0 )
	{
		if( --m_sCount[nIndex] <= 0 )
		{
			m_FlagsArray.UnsetFlag(nIndex);
		}
	}
}

//---------------------------------------------------------------------------------------
// �Ƿ���ĳ��״̬��
//---------------------------------------------------------------------------------------
template<INT nSize>
inline BOOL CountStateMgr<nSize>::IsInState(INT nIndex)
{
	if( nIndex < 0 || nIndex >= nSize ) return FALSE;

	return m_FlagsArray.IsSet(nIndex);
}

//-----------------------------------------------------------------------------------------
// �õ�״̬��־λ����
//-----------------------------------------------------------------------------------------
template<INT nSize>
inline DWORD CountStateMgr<nSize>::GetStateFlags()
{
	return m_FlagsArray.GetFlags();
}