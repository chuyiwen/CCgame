/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"

#include "gm_policy.h"

GMPolicy g_GMPolicy;

//-----------------------------------------------------------------------------
// ˫������
//-----------------------------------------------------------------------------
GMDoublePolicy::GMDoublePolicy()
{
	for (INT i=0; i<EGMDT_NUMBER; ++i)
	{
		m_fCurDouble[i] = 1.0f;
		m_bDouble[i]	= FALSE;
	}
}

VOID GMDoublePolicy::SetRate( EGMDoubleType eDoubleType, DWORD dwRate, DWORD dwStart, DWORD dwPersistSeconds )
{
	if (!EDT_VALID(eDoubleType))
		return;

	m_bDouble[eDoubleType] = TRUE;

	// ���ñ���
	m_fCurDouble[eDoubleType] = dwRate / 100.0f;

	// ��¼�ر�ʱ��
	m_dwEndTime[eDoubleType] = IncreaseTime(dwStart, dwPersistSeconds);
}

VOID GMDoublePolicy::Update()
{
	for (INT i=0; i<EGMDT_NUMBER; ++i)
	{
		if (m_bDouble[i] == TRUE)
		{
			// ���Ŀǰ�����˶౶�ʣ���ô����Ƿ�ʱ
			tagDWORDTime dwNow = GetCurrentDWORDTime();

			if(dwNow >= m_dwEndTime[i])
			{
				// �౶��ʱ���ѵ�
				m_fCurDouble[i] = 1.0f;
				m_bDouble[i] = FALSE;
			}
		}
	}
}
