/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����

#pragma once

#include "../../common/WorldDefine/cost_type_define.h"

class Role;
//-----------------------------------------------------------------------------
// log����������ö��
//-----------------------------------------------------------------------------
enum ELogConType
{
	ELCT_Null		= 0,

	ELCT_Bag		= 1,	// ����
	ELCT_RoleWare	= 2,	// ��ɫ�ֿ�
	ELCT_BaiBao		= 3,	// �ٱ���
	ELCT_BagBind	= 4,	// ������
};

//-----------------------------------------------------------------------------
// ģ��
//-----------------------------------------------------------------------------
template<class T>
class Currency
{
public:
	Currency(T nMoney, T nMaxMoney);

public:
	T	Gain(T nMoney);
	T	Spend(T nMoney);

public:
	T	GetCur() const { return m_nMoney; }
	T	GetMax() const { return m_nMaxMoney; }

private:
	T	m_nMoney;
	T	m_nMaxMoney;
};
class ClanData;
class ItemMgr;
//-----------------------------------------------------------------------------
// ������ -- �����ֿ��еĽ�ǮԪ��
//-----------------------------------------------------------------------------
class CurrencyMgr
{
friend class ItemMgr;

public:
	CurrencyMgr(Role *pRole, INT32 nBagGold, INT32 nBagSilver, INT32 nBagCopper, 
				INT32 nBagBindGold, INT32 nBagBindSilver, INT32 nBagBindCopper,
				INT32 nBagYuanBao, INT32 nWareGold, INT32 nWareSilver, INT32 nWareCopper, INT32 nBaiBaoYuanBao, INT32 nExVolume,
				INT32 n32_exploits);

	VOID Update();

public:
	// ��ǰ�����Ƿ�����Ҫ��
	BOOL IsEnough(ECurCostType eCurType, DWORD nNeed);
	// �۳�ָ������
	BOOL DecCurrency(ECurCostType eCurType, DWORD nNeed, DWORD dw_cmd_id);

public:
	// ��õ�ǰ��Ǯ��	
	INT64 GetBagSilver()		const { return m_BagSilver.GetCur(); }
	INT64 GetBagBindSilver()	const { return m_BagBindSilver.GetCur(); }
	INT32 GetBagYuanBao()		const { return m_BagYuanBao.GetCur(); }
	INT64 GetWareSilver()		const { return m_WareSilver.GetCur(); }
	INT32 GetBaiBaoYuanBao()	const { return m_BaiBaoYuanBao.GetCur(); }
	INT32 GetExchangeVolume()	const { return m_ExchangeVolume.GetCur(); }
	INT32 GetExploits()			const { return m_exploits.GetCur();	}
	
	INT64 GetBagAllSilver()		const { return m_BagSilver.GetCur() + m_BagBindSilver.GetCur(); }
	// ��ÿɴ洢����Ǯ��
	INT64 GetMaxBagSilver()			const { return m_BagSilver.GetMax(); }
	INT64 GetMaxBagBindSilver()		const { return m_BagBindSilver.GetMax(); }
	INT32 GetMaxBagYuanBao()		const { return m_BagYuanBao.GetMax(); }
	INT64 GetMaxWareSilver()		const { return m_WareSilver.GetMax(); }
	INT32 GetMaxBaiBaoYuanBao()		const { return m_BaiBaoYuanBao.GetMax(); }
	INT32 GetMaxExchangeVolume()	const { return m_ExchangeVolume.GetMax(); }
	INT32 GetMaxExploits()			const { return m_exploits.GetMax(); }

	// �����Դ���Ľ�Ǯ��Ŀ
	INT64 GetCanIncBagSilver()		const { return GetMaxBagSilver() - GetBagSilver(); }
	INT64 GetCanIncBagBindSilver()	const { return GetMaxBagBindSilver() - GetBagBindSilver(); }
	INT32 GetCanIncBagYuanBao()		const { return GetMaxBagYuanBao() - GetBagYuanBao(); }
	INT64 GetCanIncWareSilver()		const { return GetMaxWareSilver() - GetWareSilver(); }
	INT32 GetCanIncBaiBaoYuanBao()	const { return GetMaxBaiBaoYuanBao() - GetBaiBaoYuanBao(); }
	INT32 GetCanIncExchangeVolume()	const { return GetMaxExchangeVolume() - GetExchangeVolume(); }

	// ��ý�Ǯ��Ԫ��
	BOOL IncBagSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
	BOOL IncBagBindSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
	BOOL IncBagYuanBao(INT32 nYuanBao, DWORD dw_cmd_id);
	BOOL IncWareSilver(INT64 n64Silver, DWORD dw_cmd_id);
	BOOL IncBaiBaoYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bSaveDB = TRUE, DWORD dwRoleIDRel = INVALID_VALUE);
	BOOL IncExchangeVolume(INT32 nExVolume, DWORD dw_cmd_id);
	BOOL IncExploits(INT32 nExVolume,	DWORD dw_cmd_id);

	// ʧȥ��Ǯ��Ԫ��
	BOOL DecBagSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE, BOOL bSendDB = FALSE, DWORD	dw_role_id = INVALID_VALUE);
	BOOL DecBagBindSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
	BOOL DecBagYuanBao(INT32 nYuanBao, DWORD dw_cmd_id);
	BOOL DecWareSilver(INT64 n64Silver, DWORD dw_cmd_id);
	BOOL DecBaiBaoYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
	BOOL DecExchangeVolume(INT32 nExVolume, DWORD dw_cmd_id);
	BOOL DecExploits(INT32 nExVolume,	DWORD dw_cmd_id);
	
	// ����ʧȥ�󶨽��,���������ķǰ󶨽��
	BOOL DecBagSilverEx(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel = INVALID_VALUE);
public:
	// ���������ҵĽӿ�
	static BOOL ModifyBaiBaoYuanBao(DWORD dw_account_id, INT32 nYuanBao, DWORD dw_cmd_id);
	static BOOL ModifyExchangeVolume(DWORD dw_account_id, INT nVolume, DWORD dw_cmd_id);
	static BOOL ModifyWareSilver(DWORD dw_role_id, INT64 n64Silver, DWORD dw_cmd_id);
private:
	static VOID SendBaiBaoYB2DB(DWORD dw_account_id, INT nBaiBaoYuanBao);
	static VOID LogBaiBaoYuanBao(const DWORD dw_account_id, const INT n_num, const DWORD dw_cmd_id);
	static VOID SendWareSilver2DB(DWORD dw_account_id, INT64 n64WareSilver);
	static VOID LogWareSilver(const DWORD dw_account_id, const INT64 n64Num, const DWORD dw_cmd_id);
	static VOID	SendExchangeVolume2DB(DWORD dw_account_id, INT nVolume);
	static VOID LogNoLineExchange(const DWORD dw_account_id, const INT nVolume, const DWORD dw_cmd_id);
	static VOID SendBagSilver2DB(DWORD	dw_role_id, INT64 n64Silver);

public:
	// ���幱������
	BOOL DecClanCon(INT32 nClanCon, DWORD dw_cmd_id, ECLanType eClanType);
	BOOL IncClanCon(INT32 nIncr, DWORD dw_cmd_id, ECLanType eClanType);
	INT32 GetCanIncClanCon(ECLanType eClanType)	const;
	INT32 GetMaxClanCon(ECLanType eClanType) const;
	INT32 GetClanCon(ECLanType eClanType) const;

private:
	// ��ͻ��˷�����Ϣ
	VOID SendMessage(LPVOID p_message, DWORD dw_size);

	// ��¼log����STDB����Ϣ
	VOID LogSilver(const ELogConType eLogConType, const INT64 n64Num, 
				  const INT64 n64TotalNum, const DWORD dw_cmd_id, const DWORD dwRoleIDRel = INVALID_VALUE);
	VOID LogLeaveSilver(const ELogConType eLogConType, const INT64 n64Num, 
		const INT64 n64TotalNum, const DWORD dw_cmd_id, const DWORD	dw_role_id, const DWORD dwRoleIDRel = INVALID_VALUE);
	VOID LogYuanBao(const ELogConType eLogConType, const INT n_num, 
					const INT nTotalNum, const DWORD dw_cmd_id, const DWORD dwRoleIDRel = INVALID_VALUE);
	VOID LogExVolume(const INT n_num, const INT nTotalNum, const DWORD dw_cmd_id);
	VOID LogExploits(const INT32 n_num, const INT32 nTotalNum, const DWORD dw_cmd_id);
	VOID LogTimeStat();

private:
	Role*				m_pRole;
	INT					m_nTimeStatCountDown;	// ��ʱͳ�Ƶ���ʱ

private:
	Currency<INT64>		m_BagSilver;
	Currency<INT64>		m_BagBindSilver;
	Currency<INT32>		m_BagYuanBao;
	Currency<INT64>		m_WareSilver;
	Currency<INT32>		m_BaiBaoYuanBao;
	Currency<INT32>		m_ExchangeVolume;
	Currency<INT32>		m_exploits;				// ս��
	ClanData*			m_pRoleClanData;		// ���幱��
};


//-----------------------------------------------------------------------------
// ģ����ʵ��
//-----------------------------------------------------------------------------
template<class T>
Currency<T>::Currency(T nMoney, T nMaxMoney)
{
	m_nMoney	= nMoney;
	m_nMaxMoney = nMaxMoney;

	if(nMoney > nMaxMoney)
	{
		ASSERT(nMoney <= nMaxMoney);
		nMoney = nMaxMoney;
	}

	if(nMoney < 0)
	{
		ASSERT(nMoney >= 0);
		nMoney = 0;
	}
}

template<class T>
T Currency<T>::Gain(T nMoney)
{
	if(nMoney <= 0)
	{
		return 0;
	}

	T nChange = m_nMaxMoney - m_nMoney;

	if(nChange < nMoney)
	{
		ASSERT(nChange >= nMoney);
		m_nMoney = m_nMaxMoney;
	}
	else
	{
		m_nMoney += nMoney;
		nChange = nMoney;
	}

	return nChange;
}

template<class T>
T Currency<T>::Spend(T nMoney)
{
	if(nMoney <= 0)
	{
		return 0;
	}

	T nChange = nMoney;

	if(m_nMoney < nMoney)
	{
		ASSERT(m_nMoney >= nMoney);
		nChange = m_nMoney;
		m_nMoney = 0;
	}
	else
	{
		m_nMoney -= nMoney;
	}

	return nChange;
}

////-----------------------------------------------------------------------------
//class Currency
//{
//public:
//	Currency();
//	~Currency();
//
//public:
//	// ��ʼ�������ݿ���������
//	VOID Init(INT32 nBagGold, INT32 nBagSilver, INT32 nBagYuanBao, 
//		INT32 nWareGold, INT32 nWareSilver, INT32 nBaiBaoYuanBao);
//
//	// ��ý�Ǯ��Ԫ��
//	INT64 IncBagSilver(INT64 n64Silver);
//	INT32 IncBagYuanBao(INT32 nYuanBao);
//	INT64 IncWareSilver(INT64 n64Silver);
//	INT32 IncBaiBaoYuanBao(INT32 nYuanBao);
//
//	// ʧȥ��Ǯ��Ԫ��
//	INT64 DecBagSilver(INT64 n64Silver);
//	INT32 DecBagYuanBao(INT32 nYuanBao);
//	INT64 DecWareSilver(INT64 n64Silver);
//	INT32 DecBaiBaoYuanBao(INT32 nYuanBao);
//
//
//public:
//	INT64 GetBagSilver()	const { return m_n64BagSilver; }
//	INT32 GetBagYuanBao()	const { return m_nBagYuanBao; }
//	INT64 GetWareSilver()	const { return m_n64WareSilver; }
//	INT32 GetBaiBaoYuanBao()	const { return m_nBaiBaoYuanBao; }
//
//private:
//	INT64	m_n64BagSilver;
//	INT64	m_n64WareSilver;
//	INT32	m_nBagYuanBao;
//	INT32	m_nBaiBaoYuanBao;
//};
