
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

//������֤ϵͳ
#pragma once

#include "../../common/WorldDefine/time.h"
#include "../../common/WorldDefine/base_define.h"

#define MINVERIFICATIONTIME 10 * 60 * TICK_PER_SECOND	// ��Ҫ��֤����Сʱ��
#define MAXVERIFICATIONTIME 30 * 60 * TICK_PER_SECOND	// ��Ҫ��֤�����ʱ��

#define RELAYFAILVERIFICATIONTIME 180			// ��֤׼������ʱ��
#define FAILVERIFICATIONTIME 120				// ��֤����ʱ��

#define VIREIFCATIONCOUNT	3			// ��֤ʧ�ܴ���


enum E_Verification_State
{
	E_VER_NULL,		//����֤
	E_VER_RELAY,	//Ԥ����֤
	E_VER_ING,		//��֤��
};

class Role; 


//��֤�����ɲ���
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
	PlayerSession*	m_pPlayerSession;					// ��ǰ PlayerSession
	BOOL			m_bVerification;					// �Ƿ���Ҫ��֤
	//DWORD			m_dwKey;							// ��ǰ��֤���key
	//DWORD			m_dwIndex;							// ��֤�������
	//TCHAR			m_strCode[4];						// ��ǰ��֤��
	DWORD			m_dwCodeCrc;						// ��֤���crc

	INT				m_nFailNum;							// ��֤�������
	INT				m_nTickCountDown;					// ��֤����ʱ
	
	BOOL			m_bIsInVerState;					// �Ƿ�������֤״̬��
	INT				m_nTickFailCountDown;				// ��֤����ʱ���ʱ
	INT				m_nTickRelayCountDown;				// ׼����֤ʱ���ʱ

	E_Verification_State	m_eState;					// ��֤״̬

	VerificationStrategy*	m_pStrategy;				// ���ɲ���
};