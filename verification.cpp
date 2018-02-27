/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//在线验证系统

#include "StdAfx.h"
#include "verification.h"
#include "player_session.h"
#include "att_res.h"
#include "role.h"
#include "../../common/WorldDefine/verification_protocol.h"
#include "center_session.h"

VOID VerificationStrategyOne::productVerify(TCHAR* str, DWORD& dwParam, DWORD& dwAnswer)
{
	srand(unsigned(time(0)));

	for (int i = 0; i < 4; i++)
	{
		INT32 nType = get_tool()->tool_rand()%3;
		if (nType == 0)
		{
			str[i] = get_tool()->rand_in_range(_T('a'), _T('z'));
		}
		else if(nType == 1)
		{
			str[i] = get_tool()->rand_in_range(_T('A'), _T('Z'));
		}
		else
		{
			str[i] = get_tool()->rand_in_range(_T('1'), _T('9'));
		}
	}

	{
		dwParam = get_tool()->tool_rand()%4;
		TCHAR sz[2] = _T(""); 
		sz[0] = str[dwParam];
		dwAnswer = g_world.LowerCrc32(sz);
	}	
}
// 重新生成验证码
VOID Verification::resetVerificationCode()
{
	//m_pStrategy->productVerify(m_strCode, m_dwIndex, m_dwCodeCrc);
}

VOID Verification::reset()
{
	m_eState = E_VER_NULL;
	m_bIsInVerState = FALSE;
	m_bVerification = FALSE;
	m_nTickRelayCountDown = 0;
	m_nTickFailCountDown = 0;
	m_nTickCountDown = 0;
	m_nFailNum = 0;
}

VOID Verification::update()
{
	if (!m_bVerification)
	{
		if( --m_nTickCountDown <= 0 )
		{
			m_bVerification = TRUE;
		}
	}
	
	// 准备验证中!...
	if (m_eState == E_VER_RELAY)
	{
		if (--m_nTickRelayCountDown <= 0)
		{
			SetVeringState();
		}
	}
	
	// 验证中!...
	if(m_eState == E_VER_ING)
	{
		if (--m_nTickFailCountDown <= 0)
		{
			// 时间到了,踢下线
			if(VALID_POINT(m_pPlayerSession))
			{
				if (VALID_POINT(m_pPlayerSession->GetRole()))
				{
					m_pPlayerSession->GetRole()->logKick(elci_kick_role_vercation, 0);
				}
				
				g_worldSession.Kick(m_pPlayerSession->GetInternalIndex());
				m_pPlayerSession->SetKicked();
			}

		}
	}
}

// 设置验证码周期
VOID	Verification::setNeedVerTime()
{
	INT nMin = AttRes::GetInstance()->GetVariableLen().n_min_verification_time * TICK_PER_SECOND;
	INT nMax = AttRes::GetInstance()->GetVariableLen().n_max_verification_time * TICK_PER_SECOND;
	m_nTickCountDown = get_tool()->rand_in_range(nMin, nMax);
	m_bVerification = FALSE;
}


// 判断是否需要验证
BOOL	Verification::isNeedVerification()
{
	if (AttRes::GetInstance()->GetVariableLen().n_verification != 0 && 
		!m_pPlayerSession->IsPrivilegeEnough(6) && 
		g_center_session.IsWell())
		return true;

	return false;
}

BOOL	Verification::isNeedVerification(Role* pRole, BOOL bCheck)
{
	if (!VALID_POINT(pRole))
		return FALSE;

	if (m_bIsInVerState)
		return FALSE;
	
	if (!m_bVerification)
		return FALSE;

	if (!pRole->IsInNormalMap() && bCheck)
		return FALSE;

	if (pRole->get_level() < 20 && bCheck)
		return FALSE;

	if (pRole->GetAutoKill())
		return FALSE;

	return TRUE;
}


// 看是否通过验证
DWORD	Verification::receiveVer(DWORD dwCodeCrc, Role* pRole)
{
	if (m_dwCodeCrc == INVALID_VALUE)
		return INVALID_VALUE;

	if (!isSuccess(dwCodeCrc))
	{
		m_nFailNum++;

		//resetVerificationCode();
		
		if (m_nFailNum >= VIREIFCATIONCOUNT)
		{
			// 次数到了,踢下线
			if(VALID_POINT(m_pPlayerSession))
			{
				if (VALID_POINT(m_pPlayerSession->GetRole()))
				{
					m_pPlayerSession->GetRole()->logKick(elci_kick_role_vercation, 0);
				}
				g_worldSession.Kick(m_pPlayerSession->GetInternalIndex());
				m_pPlayerSession->SetKicked();
			}
		}
		return E_VERIFICATION_ERROR;
	}
	
	m_nFailNum = 0;
	setNeedVerTime();
	//resetVerificationCode();
	m_bIsInVerState = FALSE;
	m_eState = E_VER_NULL;
	m_dwCodeCrc = INVALID_VALUE;

	// 获得奖励
	//pRole->ExpChange(pRole->get_level()*50);
	pRole->ChangeBrotherhood(pRole->get_level()*10);

	return E_VERIFICATION_OK;
}


// 开始进行验证
VOID	Verification::beginVerification(Role* pRole, BOOL bCheck)
{
	if (!isNeedVerification())
		return;
	
	if (AttRes::GetInstance()->GetVariableLen().n_verification == 2 && bCheck)
		return;

	if (!isNeedVerification(pRole, bCheck))
		return;

	m_bIsInVerState = TRUE;
	m_nTickRelayCountDown = RELAYFAILVERIFICATIONTIME * TICK_PER_SECOND;

	m_eState = E_VER_RELAY;
	NET_SIS_relay_verification send;
	send.dwTime = RELAYFAILVERIFICATIONTIME * 1000;
	m_pPlayerSession->SendMessage(&send, send.dw_size);

}

VOID Verification::SetVeringState()
{
	m_eState = E_VER_ING;

	m_nTickFailCountDown = FAILVERIFICATIONTIME * TICK_PER_SECOND;

	m_pPlayerSession->SendVerifyCodeMessage();

	NET_SIS_need_verification send;
	//getCryptString(send.byStrCode);
	//send.byVerificationCodeIndex = m_dwIndex;
	send.byCount = VIREIFCATIONCOUNT;
	send.dwTime = FAILVERIFICATIONTIME * 1000;
	m_pPlayerSession->SendMessage(&send, send.dw_size);
}

//BOOL Verification::getCryptString(VOID* pData)
//{
//	//byte data[8];
//	//ZeroMemory(data, sizeof(data));
//	//memcpy(data, m_strCode, 8);
//	//DWORD* pKey = (DWORD*)data;
//	//DWORD* pIndex = (DWORD*)(data+4);
//	//
//	//*pKey = m_dwKey;
//	//*pIndex = m_dwIndex;
//
//	return get_tool()->encrypt((VOID*)m_strCode, pData, 8, 13, 1);
//}