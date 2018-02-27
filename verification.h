
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
#pragma once

#include "../../common/WorldDefine/time.h"
#include "../../common/WorldDefine/base_define.h"

#define MINVERIFICATIONTIME 10 * 60 * TICK_PER_SECOND	// 需要验证的最小时间
#define MAXVERIFICATIONTIME 30 * 60 * TICK_PER_SECOND	// 需要验证的最大时间

#define RELAYFAILVERIFICATIONTIME 180			// 验证准备限制时间
#define FAILVERIFICATIONTIME 120				// 验证限制时间

#define VIREIFCATIONCOUNT	3			// 验证失败次数


enum E_Verification_State
{
	E_VER_NULL,		//无验证
	E_VER_RELAY,	//预备验证
	E_VER_ING,		//验证中
};

class Role; 


//验证码生成策略
class VerificationStrategy
{
public:
	VerificationStrategy (){}
	virtual ~VerificationStrategy() {}

	virtual VOID productVerify(TCHAR* str, DWORD& dwParam, DWORD& dwAnswer) = 0;
private:

};

class VerificationStrategyOne : public VerificationStrategy
{
public:
	VerificationStrategyOne (){}
	virtual ~VerificationStrategyOne() {}

	virtual VOID productVerify(TCHAR* str, DWORD& dwParam, DWORD& dwAnswer);
};


class Verification
{
public:
	Verification(PlayerSession* pPlayerSession)
		:m_bVerification(TRUE), 
		m_pPlayerSession(pPlayerSession),
		m_dwCodeCrc(INVALID_VALUE),
		m_nTickCountDown(0),
		m_nFailNum(0),
		m_bIsInVerState(FALSE),
		m_eState(E_VER_NULL),
		//m_dwIndex(0),
		m_nTickRelayCountDown(0)
	{
		ASSERT( VALID_POINT(m_pPlayerSession) );
		//ZeroMemory(m_strCode, sizeof(m_strCode));
		m_pStrategy = new VerificationStrategyOne;

		//resetVerificationCode();
	}
	~Verification() { SAFE_DELETE(m_pStrategy); }


	//DWORD getKey() { return m_dwKey;}
	//VOID setIndex(DWORD dwParam) { m_dwIndex = dwParam; }
	//DWORD getIndex() { return m_dwIndex; }

	//BOOL getCryptString(VOID* pData);
	//VOID getString(TCHAR* str)
	//{
		//memcpy(str, m_strCode, sizeof(TCHAR)*4);
	//}
	BOOL	isSuccess(DWORD dwCode)
	{
		if (dwCode == m_dwCodeCrc)
			return TRUE;

		return FALSE;
	}

	VOID	resetVerificationCode();
	VOID	update();
	VOID	setNeedVerTime();
	BOOL	isNeedVerification();
	BOOL	isNeedVerification(Role* pRole, BOOL bCheck);

	DWORD	receiveVer(DWORD dwCodeCrc, Role* pRole);

	VOID	beginVerification(Role* pRole, BOOL bCheck);

	VOID	SetVeringState();

	VOID	reset();

	VOID	setAnswer(DWORD dwAnswer) { m_dwCodeCrc = dwAnswer; }
private:
	PlayerSession*	m_pPlayerSession;					// 当前 PlayerSession
	BOOL			m_bVerification;					// 是否需要验证
	//DWORD			m_dwKey;							// 当前验证码的key
	//DWORD			m_dwIndex;							// 验证码的索引
	//TCHAR			m_strCode[4];						// 当前验证码
	DWORD			m_dwCodeCrc;						// 验证码的crc

	INT				m_nFailNum;							// 验证错误次数
	INT				m_nTickCountDown;					// 验证倒计时
	
	BOOL			m_bIsInVerState;					// 是否正在验证状态中
	INT				m_nTickFailCountDown;				// 验证限制时间计时
	INT				m_nTickRelayCountDown;				// 准备验证时间计时

	E_Verification_State	m_eState;					// 验证状态

	VerificationStrategy*	m_pStrategy;				// 生成策略
};