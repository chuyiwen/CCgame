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

#include "StdAfx.h"
#include "currency.h"
#include "role.h"
#include "role_mgr.h"
#include "../../common/WorldDefine/currency_define.h"
#include "../../common/WorldDefine/currency_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/common_server_define.h"

//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
CurrencyMgr::CurrencyMgr(Role *pRole, INT32 nBagGold, INT32 nBagSilver, INT32 nBagCopper, 
						 INT32 nBagBindGold, INT32 nBagBindSilver, INT32 nBagBindCopper,
						 INT32 nBagYuanBao, INT32 nWareGold, INT32 nWareSilver, INT32 nWareCopper, INT32 nBaiBaoYuanBao, INT32 nExVolume,INT32 n32_exploits)
						: m_BagSilver(MGold2Silver(nBagGold) + MSilver2Copper(nBagSilver) + nBagCopper/*(INT64)nBagSilver*/, MAX_BAG_SILVER_NUM),
						  m_BagBindSilver(MGold2Silver(nBagBindGold) + MSilver2Copper(nBagBindSilver) + nBagBindCopper, MAX_BAG_SILVER_NUM),
						  m_BagYuanBao(nBagYuanBao, MAX_BAG_YUANBAO_NUM),
						  m_WareSilver(MGold2Silver(nWareGold) + MSilver2Copper(nWareSilver) + nWareCopper, MAX_WARE_SILVER_NUM),
						  m_BaiBaoYuanBao(nBaiBaoYuanBao, MAX_BAIBAO_YUANBAO_NUM),
						  m_pRoleClanData( &( pRole->GetClanData() ) ),
						  m_ExchangeVolume(nExVolume, MAX_EXCHANGE_VOLUME_NUM),
						  m_exploits(n32_exploits, MAX_EXPLOITS)
{
	m_pRole = pRole;
	m_nTimeStatCountDown = ROLE_TIME_STAT_INTERVAL;
}

//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
VOID CurrencyMgr::Update()
{
	if(--m_nTimeStatCountDown <= 0)
	{
		m_nTimeStatCountDown = ROLE_TIME_STAT_INTERVAL;
		LogTimeStat();
	}
}

//-----------------------------------------------------------------------------
// ��ͻ��˷�����Ϣ
//-----------------------------------------------------------------------------
VOID CurrencyMgr::SendMessage(LPVOID p_message, DWORD dw_size)
{
	ASSERT(VALID_POINT(m_pRole));
	ASSERT(VALID_POINT(m_pRole->GetSession()));

	if(VALID_POINT(m_pRole) && VALID_POINT(m_pRole->GetSession()))
		m_pRole->GetSession()->SendMessage(p_message, dw_size);
}

//-----------------------------------------------------------------------------
// ��STDB����Ϣ -- �ǰ󶨱�
//-----------------------------------------------------------------------------
VOID CurrencyMgr::SendBagSilver2DB(DWORD	dw_role_id, INT64 n64Silver)
{
	NET_DB2C_bag_money_update send;
	send.dw_role_id = dw_role_id;
	send.n_bag_gold_ = MSilver2DBGold(n64Silver);
	send.n_bag_silver_ = MSilver2DBSilver(n64Silver);
	send.n_bag_copper_ = MSilver2DBCopper(n64Silver);
	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��STDB����Ϣ -- �ֿ��Ǯ
//-----------------------------------------------------------------------------
VOID CurrencyMgr::SendWareSilver2DB(DWORD dw_account_id, INT64 n64WareSilver)
{
	NET_DB2C_ware_money_update send;
	send.dw_account_id		= dw_account_id;
	send.n64_ware_silver		= n64WareSilver;
	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��STDB����Ϣ -- �ٱ���Ԫ��
//-----------------------------------------------------------------------------
VOID CurrencyMgr::SendBaiBaoYB2DB(DWORD dw_account_id, INT nBaiBaoYuanBao)
{
	NET_DB2C_baibao_yuanbao_update send;
	send.dw_account_id	= dw_account_id;
	send.nBaiBaoYuanBao	= nBaiBaoYuanBao;
	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��STDB����Ϣ -- ����
//-----------------------------------------------------------------------------
VOID CurrencyMgr::SendExchangeVolume2DB(DWORD dw_account_id, INT nVolume)
{
	NET_DB2C_exchange_volume_update send;
	send.dw_account_id = dw_account_id;
	send.n_volume = nVolume;
	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��¼��Ǯlog����STDB����Ϣ
//-----------------------------------------------------------------------------
VOID CurrencyMgr::LogSilver(const ELogConType eLogConType, const INT64 n64Num, 
							const INT64 n64TotalNum, const DWORD dw_cmd_id, const DWORD dwRoleIDRel)
{

	if (!VALID_POINT(m_pRole->GetSession()))
		return;

	NET_DB2C_log_silver send;
	send.s_log_silver_.dw_role_id		= m_pRole->GetID();
	send.s_log_silver_.dw_account_id		= m_pRole->GetSession()->GetSessionID();
	send.s_log_silver_.dw_cmd_id			= dw_cmd_id;
	send.s_log_silver_.dw_role_id_rel		= dwRoleIDRel;
	send.s_log_silver_.n8_log_con_type	= (INT8)eLogConType;
	send.s_log_silver_.n64_silver		= n64Num;
	send.s_log_silver_.n64_total_silver	= n64TotalNum;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��¼���߽�Ǯlog����STDB����Ϣ
//-----------------------------------------------------------------------------
VOID  CurrencyMgr::LogLeaveSilver(const ELogConType eLogConType, const INT64 n64Num, 
					const INT64 n64TotalNum, const DWORD dw_cmd_id, const DWORD	dw_role_id, const DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	NET_DB2C_log_silver send;
	send.s_log_silver_.dw_role_id		= dw_role_id;
	send.s_log_silver_.dw_account_id		= INVALID_VALUE;
	send.s_log_silver_.dw_cmd_id			= dw_cmd_id;
	send.s_log_silver_.dw_role_id_rel		= dwRoleIDRel;
	send.s_log_silver_.n8_log_con_type	= (INT8)eLogConType;
	send.s_log_silver_.n64_silver		= n64Num;
	send.s_log_silver_.n64_total_silver	= n64TotalNum;

	g_dbSession.Send(&send, send.dw_size);
}

VOID CurrencyMgr::LogWareSilver( const DWORD dw_account_id, const INT64 n64Num, const DWORD dw_cmd_id )
{
	NET_DB2C_log_silver send;
	send.s_log_silver_.dw_role_id		= INVALID_VALUE;
	send.s_log_silver_.dw_account_id		= dw_account_id;
	send.s_log_silver_.dw_cmd_id			= dw_cmd_id;
	send.s_log_silver_.n8_log_con_type	= (INT8)ELCT_RoleWare;
	send.s_log_silver_.n64_silver		= n64Num;
	send.s_log_silver_.n64_total_silver	= INVALID_VALUE;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��¼Ԫ��log����STDB����Ϣ
//-----------------------------------------------------------------------------
VOID CurrencyMgr::LogYuanBao(const ELogConType eLogConType, const INT n_num, 
							 const INT nTotalNum, const DWORD dw_cmd_id, const DWORD dwRoleIDRel)
{
	if (!VALID_POINT(m_pRole->GetSession()))
		return;

	NET_DB2C_log_yuanbao send;
	send.s_log_yuanbao_.dw_role_id		= m_pRole->GetID();
	send.s_log_yuanbao_.dw_account_id	= m_pRole->GetSession()->GetSessionID();
	send.s_log_yuanbao_.dw_cmd_id		= dw_cmd_id;
	send.s_log_yuanbao_.dw_role_id_rel	= dwRoleIDRel;
	send.s_log_yuanbao_.n8_log_con_type	= (INT8)eLogConType;
	send.s_log_yuanbao_.n_yuanbao		= n_num;
	send.s_log_yuanbao_.n_total_yuanbao	= nTotalNum;

	g_dbSession.Send(&send, send.dw_size);
}

VOID CurrencyMgr::LogBaiBaoYuanBao( const DWORD dw_account_id, const INT n_num, const DWORD dw_cmd_id )
{
	NET_DB2C_log_yuanbao send;
	send.s_log_yuanbao_.dw_role_id		= INVALID_VALUE;
	send.s_log_yuanbao_.dw_account_id	= dw_account_id;
	send.s_log_yuanbao_.dw_cmd_id		= dw_cmd_id;
	send.s_log_yuanbao_.n8_log_con_type	= (INT8)ELCT_BaiBao;
	send.s_log_yuanbao_.n_yuanbao		= n_num;
	send.s_log_yuanbao_.n_total_yuanbao	= INVALID_VALUE;

	g_dbSession.Send(&send, send.dw_size);
}

VOID CurrencyMgr::LogNoLineExchange(const DWORD dw_account_id, const INT nVolume, const DWORD dw_cmd_id)
{
	NET_DB2C_log_exvolume send;
	send.s_log_ex_volume_.dw_account_id		= dw_account_id;
	send.s_log_ex_volume_.dw_role_id			= INVALID_VALUE;
	send.s_log_ex_volume_.dw_cmd_id			= dw_cmd_id;
	send.s_log_ex_volume_.n_ex_volume			= nVolume;
	send.s_log_ex_volume_.n_total_ex_volume	= INVALID_VALUE;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��¼����log����STDB����Ϣ
//-----------------------------------------------------------------------------
VOID CurrencyMgr::LogExVolume(const INT n_num, const INT nTotalNum, const DWORD dw_cmd_id)
{
	if (!VALID_POINT(m_pRole->GetSession()))
		return;

	NET_DB2C_log_exvolume send;
	send.s_log_ex_volume_.dw_account_id		= m_pRole->GetSession()->GetSessionID();
	send.s_log_ex_volume_.dw_role_id			= m_pRole->GetID();
	send.s_log_ex_volume_.dw_cmd_id			= dw_cmd_id;
	send.s_log_ex_volume_.n_ex_volume			= n_num;
	send.s_log_ex_volume_.n_total_ex_volume	= nTotalNum;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��¼ս��log
//-----------------------------------------------------------------------------
VOID CurrencyMgr::LogExploits(const INT32 n_num, const INT32 nTotalNum, const DWORD dw_cmd_id)
{
	if (!VALID_POINT(m_pRole->GetSession()))
		return;

	NET_DB2C_log_exploits send;
	send.s_log_exploits.dw_account_id = m_pRole->GetSession()->GetSessionID();
	send.s_log_exploits.dw_role_id = m_pRole->GetID();
	send.s_log_exploits.dw_cmd_id = dw_cmd_id;
	send.s_log_exploits.n_exploits = n_num;
	send.s_log_exploits.n_total_exploits = nTotalNum;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��ʱͳ����һ���
//-----------------------------------------------------------------------------
VOID CurrencyMgr::LogTimeStat()
{
	if (!VALID_POINT(m_pRole->GetSession()))
		return;

	NET_DB2C_log_time_stat send;
	send.s_log_times_stat_.dw_role_id			= m_pRole->GetID();
	send.s_log_times_stat_.dw_account_id		= m_pRole->GetSession()->GetSessionID();
	send.s_log_times_stat_.n64_bag_silver		= GetBagSilver();
	send.s_log_times_stat_.n64_ware_silver		= GetWareSilver();
	send.s_log_times_stat_.n_bag_yuanbao		= GetBagYuanBao();
	send.s_log_times_stat_.n_baibao_yuanbao	= GetBaiBaoYuanBao();
	send.s_log_times_stat_.n_ex_volume			= GetExchangeVolume();

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// ��ñ�����Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncBagSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Inc = m_BagSilver.Gain(n64Silver);

	// ��¼log
	LogSilver(ELCT_Bag, n64Silver, m_BagSilver.GetCur(), dw_cmd_id, dwRoleIDRel);
	
	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_incbagsilver, (DWORD)n64Inc);
	// ��ͻ��˷���Ϣ
	NET_SIS_bag_silver	send;
	send.n64CurSilver		= GetBagSilver();
	send.n64ChangeSilver	= n64Inc;
	send.bBind				= 0;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��ñ����󶨽�Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncBagBindSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Inc = m_BagBindSilver.Gain(n64Silver);

	// ��¼log
	LogSilver(ELCT_BagBind, n64Silver, m_BagBindSilver.GetCur(), dw_cmd_id, dwRoleIDRel);
	
	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_incbagbindsilver, (DWORD)n64Inc);

	// ��ͻ��˷���Ϣ
	NET_SIS_bag_silver	send;
	// mwh 2011/04/25 ȡ�ð󶨽�Ǯ
	send.n64CurSilver		= GetBagBindSilver();//GetBagSilver();
	send.n64ChangeSilver	= n64Inc;
	send.bBind				= 1;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��ñ���Ԫ��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncBagYuanBao(INT32 nYuanBao, DWORD dw_cmd_id)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT32 nInc = m_BagYuanBao.Gain(nYuanBao);

	// ���ݿ����//??
	
	// ��¼log
	LogYuanBao(ELCT_Bag, nYuanBao, m_BagYuanBao.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_bag_yuanbao send;
	send.nCurYuanBao	= GetBagYuanBao();
	send.nChangeYuanBao	= nInc;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��òֿ��Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncWareSilver(INT64 n64Silver, DWORD dw_cmd_id)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Inc = m_WareSilver.Gain(n64Silver);

	// ��������
	//SendWareSilver2DB(m_pRole->GetSession()->GetSessionID(), n64Inc);
	
	// ��¼log
	LogSilver(ELCT_RoleWare, n64Inc, m_WareSilver.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_ware_silver send;
	send.n64CurSilver		= GetWareSilver();
	send.n64ChangeSilver	= n64Inc;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ��ðٱ�Ԫ��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncBaiBaoYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bSaveDB /*= TRUE*/, DWORD dwRoleIDRel)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT32 nInc = m_BaiBaoYuanBao.Gain(nYuanBao);

	// ��������
	if (bSaveDB)
	{
		if (VALID_POINT(m_pRole->GetSession()))
		{
			SendBaiBaoYB2DB(m_pRole->GetSession()->GetSessionID(), nInc);
		}
	}
	
	// ��¼log
	LogYuanBao(ELCT_BaiBao, nInc, m_BaiBaoYuanBao.GetCur(), dw_cmd_id, dwRoleIDRel);

	// ��ͻ��˷���Ϣ
	NET_SIS_baibao_yuanbao send;
	send.nCurYuanBao	= GetBaiBaoYuanBao();
	send.nChangeYuanBao	= nInc;
	send.bBillYuanbao = (dw_cmd_id == elcid_baibao_bill_yuanbao);
	SendMessage(&send, send.dw_size);
	
	return TRUE;
}

//-----------------------------------------------------------------------------
// �ٱ�Ԫ�����(��Բ��������)
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::ModifyBaiBaoYuanBao(DWORD dw_account_id, INT32 nYuanBao, DWORD dw_cmd_id)
{
	if(nYuanBao == 0)
	{
		// ���0��û������
		return FALSE;
	}

	// ��������
	SendBaiBaoYB2DB(dw_account_id, nYuanBao);

	// ��¼log
	LogBaiBaoYuanBao(dw_account_id, nYuanBao, dw_cmd_id);

	// �Ƿ���ѡ�˽���
	PlayerSession* pSession = g_worldSession.FindGlobalSession(dw_account_id);
	if (VALID_POINT(pSession))
	{
		/*INT nBaiBaoYB = pSession->GetBaiBaoYB() + nYuanBao;
		if (nBaiBaoYB < 0)
		{
			nBaiBaoYB = 0;
		}*/
		pSession->SetBaiBaoYB(nYuanBao);
	}

	return TRUE;
}

BOOL CurrencyMgr::ModifyExchangeVolume(DWORD dw_account_id, INT nVolume, DWORD dw_cmd_id)
{
	if(nVolume == 0)
	{
		return FALSE;
	}

	SendExchangeVolume2DB(dw_account_id, nVolume);

	LogNoLineExchange(dw_account_id, nVolume, dw_cmd_id);

	PlayerSession* pSession = g_worldSession.FindGlobalSession(dw_account_id);
	if(VALID_POINT(pSession))
	{
		pSession->SetExchange(nVolume);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// �ֿ��Ǯ���(��Բ��������)
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::ModifyWareSilver( DWORD dw_role_id, INT64 n64Silver, DWORD dw_cmd_id )
{
	if(n64Silver == 0)
	{
		// ���0��û������
		return FALSE;
	}

	s_role_info* pRoleInfo = g_roleMgr.get_role_info(dw_role_id);

	// ��֤������Ƿ����
	if (!VALID_POINT(pRoleInfo))
	{
		return FALSE;
	}

	// ��������
	SendWareSilver2DB(pRoleInfo->dw_account_id, n64Silver);

	// ��¼log
	LogWareSilver(pRoleInfo->dw_account_id, n64Silver, dw_cmd_id);

	// �Ƿ���ѡ�˽���
	PlayerSession* pSession = g_worldSession.FindGlobalSession(pRoleInfo->dw_account_id);
	if (VALID_POINT(pSession))
	{
		INT64 n64WareSilver = pSession->GetWareSilver() + n64Silver;
		if (n64WareSilver < 0)
		{
			n64WareSilver = 0;
		}
		pSession->SetWareSilver(n64WareSilver);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// �������
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncExchangeVolume(INT32 nExVolume, DWORD dw_cmd_id)
{
	ASSERT(VALID_POINT(m_pRole));

	if(nExVolume <= 0)
	{
		return FALSE;
	}

	INT32 nInc = m_ExchangeVolume.Gain(nExVolume);

	if (!VALID_POINT(m_pRole->GetSession()))
		return FALSE;

	// ���ݿ����//??
	SendExchangeVolume2DB(m_pRole->GetSession()->GetSessionID(), nInc);

	// ��¼log
	LogExVolume(nInc, m_ExchangeVolume.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_present_point send;
	send.nCurExVolume		= GetExchangeVolume();
	send.nChangeExVolume	= nInc;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ���ս��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::IncExploits(INT32 nExVolume,	DWORD dw_cmd_id)
{
	ASSERT(VALID_POINT(m_pRole));

	if(nExVolume <= 0)
	{
		return FALSE;
	}

	INT32 n32Inc = m_exploits.Gain(nExVolume);

	// ��¼log
	LogExploits(nExVolume, m_exploits.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_exploits	send;
	send.n32CurExploits = m_exploits.GetCur();
	send.n32ChangeExploits = nExVolume;
	SendMessage(&send, send.dw_size);
	return TRUE;
}

BOOL CurrencyMgr::IncClanCon(INT32 nIncr, DWORD dw_cmd_id, ECLanType eClanType)
{
	if (nIncr <= 0)
	{
		return FALSE;
	}
	

	NET_SIS_gens_contribute send;
	send.byClanType		= eClanType;
	send.nChangeClanCon = m_pRoleClanData->ClanConInc(nIncr, eClanType);
	send.nCurClanCon	= GetClanCon(eClanType);	
	SendMessage(&send, send.dw_size);
	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥ������Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecBagSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel/* = INVALID_VALUE*/, BOOL bSendDB/* = FALSE*/, DWORD	dw_role_id/* = INVALID_VALUE*/)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Dec = m_BagSilver.Spend(n64Silver);

	if(bSendDB)
	{
		SendBagSilver2DB(dw_role_id, m_BagSilver.GetCur());
	}

	// ��¼log
	if(bSendDB)
	{
		LogLeaveSilver(ELCT_Bag, -n64Silver, m_BagSilver.GetCur(), dw_cmd_id, dw_role_id);
	}
	else
	{
		LogSilver(ELCT_Bag, -n64Silver, m_BagSilver.GetCur(), dw_cmd_id, dwRoleIDRel);
	}
	

	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_decbagsilver, (DWORD)n64Dec);

	if(!bSendDB)
	{
		// ��ͻ��˷���Ϣ
		NET_SIS_bag_silver	send;
		send.n64CurSilver		= GetBagSilver();
		send.n64ChangeSilver	= -n64Dec;
		send.bBind				= 0;
		SendMessage(&send, send.dw_size);
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥ�����󶨽�Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecBagBindSilver(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel/* = INVALID_VALUE*/)
{
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Dec = m_BagBindSilver.Spend(n64Silver);

	// ��¼log
	LogSilver(ELCT_BagBind, -n64Silver, m_BagBindSilver.GetCur(), dw_cmd_id, dwRoleIDRel);
	
	m_pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_decbagbindsilver, (DWORD)n64Dec);

	// ��ͻ��˷���Ϣ
	NET_SIS_bag_silver	send;
	send.n64CurSilver		= GetBagBindSilver();
	send.n64ChangeSilver	= -n64Dec;
	send.bBind				= 1;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

BOOL CurrencyMgr::DecBagSilverEx(INT64 n64Silver, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{	
	ASSERT(VALID_POINT(m_pRole));
	
	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64BindSilver = 0;
	INT64 n64NoBindSilver = 0;
	if (GetBagBindSilver() >= n64Silver)
	{
		n64BindSilver = n64Silver;
	}
	else
	{
		n64BindSilver = GetBagBindSilver();
		n64NoBindSilver = n64Silver - n64BindSilver;
	}
	
	DecBagSilver(n64NoBindSilver, dw_cmd_id, dwRoleIDRel);
	//DecBagBindSilver(n64BindSilver, dw_cmd_id, dwRoleIDRel);gx modify 2013.8.5 ����Ҫ���İ�Ԫ��
}
//-----------------------------------------------------------------------------
// ʧȥ����Ԫ��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecBagYuanBao(INT32 nYuanBao, DWORD dw_cmd_id)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT32 nDec = m_BagYuanBao.Spend(nYuanBao);

	// ���ݿ����//??
	
	// ��¼log
	LogYuanBao(ELCT_Bag, -nYuanBao, m_BagYuanBao.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_bag_yuanbao send;
	send.nCurYuanBao	= GetBagYuanBao();
	send.nChangeYuanBao	= -nDec;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥ�ֿ��Ǯ
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecWareSilver(INT64 n64Silver, DWORD dw_cmd_id)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Dec = m_WareSilver.Spend(n64Silver);

	// �������ݿ�
	//SendWareSilver2DB(m_pRole->GetSession()->GetSessionID(), -n64Dec);

	// ��¼log
	LogSilver(ELCT_RoleWare, -n64Silver, m_WareSilver.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_ware_silver	send;
	send.n64CurSilver		= GetWareSilver();
	send.n64ChangeSilver	= -n64Dec;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥ�ٱ�Ԫ��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecBaiBaoYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, DWORD dwRoleIDRel)
{ 
	ASSERT(VALID_POINT(m_pRole));

	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT32 nDec = m_BaiBaoYuanBao.Spend(nYuanBao);
	
	if (!VALID_POINT(m_pRole->GetSession()))
		return FALSE;

	// �������ݿ�
	SendBaiBaoYB2DB(m_pRole->GetSession()->GetSessionID(), -nDec);
	
	// ����Ԫ���¼�
	if (VALID_POINT(m_pRole->GetScript()))
	{
		m_pRole->GetScript()->OnConsumeYuanBao(m_pRole, nDec, dw_cmd_id);
	}
	
	// ���ݿ����//??

	// ��¼log
	LogYuanBao(ELCT_BaiBao, -nDec, m_BaiBaoYuanBao.GetCur(), dw_cmd_id, dwRoleIDRel);

	// ��ͻ��˷���Ϣ
	NET_SIS_baibao_yuanbao send;
	send.nCurYuanBao	= GetBaiBaoYuanBao();
	send.nChangeYuanBao	= -nDec;
	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥ����
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecExchangeVolume(INT32 nExVolume, DWORD dw_cmd_id)
{
	ASSERT(VALID_POINT(m_pRole));

	if(nExVolume <= 0)
	{
		return FALSE;
	}

	INT32 nDec = m_ExchangeVolume.Spend(nExVolume);
	
	if (!VALID_POINT(m_pRole->GetSession()))
		return FALSE;

	SendExchangeVolume2DB(m_pRole->GetSession()->GetSessionID(), -nDec);

	// ��¼log
	LogExVolume(-nDec, m_ExchangeVolume.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_present_point	send;
	send.nCurExVolume		= GetExchangeVolume();
	send.nChangeExVolume	= -nDec;

	SendMessage(&send, send.dw_size);

	return TRUE;
}

//-----------------------------------------------------------------------------
// ʧȥս��
//-----------------------------------------------------------------------------
BOOL CurrencyMgr::DecExploits(INT32 nExVolume,	DWORD dw_cmd_id)
{
	ASSERT(VALID_POINT(m_pRole));

	if(nExVolume <= 0)
	{
		return FALSE;
	}

	INT32 n32Dec = m_exploits.Spend(nExVolume);


	// ��¼log
	LogExploits(-nExVolume, m_exploits.GetCur(), dw_cmd_id);

	// ��ͻ��˷���Ϣ
	NET_SIS_exploits	send;
	send.n32CurExploits = m_exploits.GetCur();
	send.n32ChangeExploits = -nExVolume;
	SendMessage(&send, send.dw_size);
	return TRUE;
}

BOOL CurrencyMgr::DecClanCon(INT32 nClanCon, DWORD dw_cmd_id, ECLanType eClanType)
{
	if (nClanCon <= 0)
	{
		return FALSE;
	}

	NET_SIS_gens_contribute send;
	send.byClanType		= eClanType;
	send.nChangeClanCon = -m_pRoleClanData->ClanConDec(nClanCon, eClanType);
	send.nCurClanCon	= GetClanCon(eClanType);	
	SendMessage(&send, send.dw_size);
	return TRUE;
}

//------------------------------------------------------------------------------
// ��������Ƿ����㹻�Ľ�Ǯ��Ԫ�������׵�
//------------------------------------------------------------------------------
BOOL CurrencyMgr::IsEnough(ECurCostType eCurType, DWORD nNeed)
{
	ASSERT(nNeed > 0);
	
	switch(eCurType)
	{
	case ECCT_BagSilver:
		return (GetBagSilver() >= nNeed);
		break;
	case ECCT_BagYuanBao:
		return (GetBagYuanBao() >= nNeed);
		break;
	case ECCT_WareSilver:
		return (GetWareSilver() >= nNeed);
		break;
	case ECCT_BaiBaoYuanBao:
		return (GetBaiBaoYuanBao() >= nNeed);
		break;
	case ECCT_ExchangeVolume:
		return (GetExchangeVolume() >= nNeed);
		break;
	case ECCT_BagBindSilver:
		return (GetBagAllSilver() >= nNeed);
		break;
	case ECCT_Exploits:
		return (GetExploits() >= nNeed);
		break;

	case ECCT_ClanConXuanYuan:
	case ECCT_ClanConShenNong:
	case ECCT_ClanConFuXi:
	case ECCT_ClanConSanMiao:
	case ECCT_ClanConJiuLi:
	case ECCT_ClanConYueZhi:
	case ECCT_ClanConNvWa:
	case ECCT_ClanConGongGong:
		ECLanType eClanType = MTRANS_ECCT2ECLT(eCurType);
		return ( GetClanCon(eClanType) >= nNeed );
		break;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
// �۳�ָ������
//------------------------------------------------------------------------------
BOOL CurrencyMgr::DecCurrency(ECurCostType eCurType, DWORD nNeed, DWORD dw_cmd_id)
{
	ASSERT(nNeed > 0);

	switch(eCurType)
	{
	case ECCT_BagSilver:
		if(GetBagSilver() >= nNeed)
		{
			return DecBagSilver(nNeed, dw_cmd_id);
		}
		break;
	case ECCT_BagYuanBao:
		if(GetBagYuanBao() >= nNeed)
		{
			return DecBagYuanBao(nNeed, dw_cmd_id);
		}
		break;
	case ECCT_WareSilver:
		if(GetWareSilver() >= nNeed)
		{
			return DecWareSilver(nNeed, dw_cmd_id);
		}
		break;
	case ECCT_BaiBaoYuanBao:
		if(GetBaiBaoYuanBao() >= nNeed)
		{
			return DecBaiBaoYuanBao(nNeed, dw_cmd_id);
		}
		break;

	case ECCT_ExchangeVolume:
		if (GetExchangeVolume() >= nNeed)
		{
			return DecExchangeVolume(nNeed, dw_cmd_id);
		}
		break;
	case ECCT_BagBindSilver:
		{
			if(GetBagAllSilver() >= nNeed)
			{
				return DecBagSilverEx(nNeed, dw_cmd_id);
			}
			break;
		}
	case ECCT_Exploits:
		{
			if(GetExploits() >= nNeed)
			{
				return DecExploits(nNeed, dw_cmd_id);
			}
			break;
		}

	case ECCT_ClanConXuanYuan:
	case ECCT_ClanConShenNong:
	case ECCT_ClanConFuXi:
	case ECCT_ClanConSanMiao:
	case ECCT_ClanConJiuLi:
	case ECCT_ClanConYueZhi:
	case ECCT_ClanConNvWa:
	case ECCT_ClanConGongGong:
		ECLanType eClanType = MTRANS_ECCT2ECLT(eCurType);
		if (m_pRoleClanData->ClanConGetVal(eClanType) >= nNeed)
		{
			return DecClanCon(nNeed, dw_cmd_id, eClanType);
		}
		break;
	}

	return FALSE;
}
INT32 CurrencyMgr::GetClanCon(ECLanType eClanType) const 
{ 
	return m_pRoleClanData->ClanConGetVal(eClanType); 
}

INT32 CurrencyMgr::GetCanIncClanCon(ECLanType eClanType)	const 
{ 
	return GetMaxClanCon(eClanType) - GetClanCon(eClanType); 
}
INT32 CurrencyMgr::GetMaxClanCon(ECLanType eClanType)		const 
{
	return m_pRoleClanData->ClanConGetMaxVal(eClanType);
}
