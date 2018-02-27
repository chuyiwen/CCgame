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
#include "TradeYuanBao.h"
#include "world.h"
#include "role_mgr.h"
#include "role.h"
#include "db_session.h"

#include "../../common/WorldDefine/mall_protocol.h"
#include "../common/ServerDefine/yuanbao_server_define.h"

CTradeYB g_tradeYB;

VOID tagYuanBaoAccount::SetSellOrder(BOOL bSubmit) 
{ 
	bSellOrder = bSubmit; 

	// ͬ�����ݿ�
	NET_DB2C_syn_submit_sell	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.b_sell_order = (bool)bSellOrder;
	g_dbSession.Send(&sendDB, sendDB.dw_size);
}

VOID tagYuanBaoAccount::SetBuyOrder(BOOL bSubmit) 
{
	bBuyOrder = bSubmit;

	// ͬ�����ݿ�
	NET_DB2C_syn_submit_buy	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.b_buy_order = (bool)bBuyOrder;
	g_dbSession.Send(&sendDB, sendDB.dw_size);
}

BOOL tagYuanBaoAccount::IncAccountYB(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient)
{
	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT32 nInc = YuanBao.Gain(nYuanBao);

	// ��¼log//??
	
	// ��ͻ��˷���Ϣ
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_account_yuanbao	send;
			send.nYuanBao = GetAccountYB();
			pRole->SendMessage(&send, sizeof(send));
		}
	}

	// ͬ�����ݿ�
	NET_DB2C_syn_account_yanbao	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.n_yuanbao = GetAccountYB();
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return TRUE;
}

BOOL tagYuanBaoAccount::IncAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient)
{
	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Inc = Silver.Gain(n64Silver);

	// ��¼log//??

	// ��ͻ��˷���Ϣ
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_account_silver	send;
			send.nSilver = GetAccountSilver();
			pRole->SendMessage(&send, sizeof(send));
		}
	}

	// ͬ�����ݿ�
	NET_DB2C_syn_account_silver	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.money.nGold = MSilver2DBGold(GetAccountSilver());
	sendDB.money.nSilver = MSilver2DBSilver(GetAccountSilver());
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return TRUE;
}

BOOL tagYuanBaoAccount::DecAccountYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient)
{
	if(nYuanBao <= 0)
	{
		return FALSE;
	}

	INT64 n64Dec = YuanBao.Spend(nYuanBao);

	// ��¼log//??

	// ��ͻ��˷���Ϣ
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_account_yuanbao	send;
			send.nYuanBao = GetAccountYB();
			pRole->SendMessage(&send, sizeof(send));
		}
	}

	// ͬ�����ݿ�
	NET_DB2C_syn_account_yanbao	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.n_yuanbao = GetAccountYB();
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return TRUE;
}

BOOL tagYuanBaoAccount::DecAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient)
{
	if(n64Silver <= 0)
	{
		return FALSE;
	}

	INT64 n64Dec = Silver.Spend(n64Silver);

	// ��¼log//??

	// ��ͻ��˷���Ϣ
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_account_silver	send;
			send.nSilver = GetAccountSilver();
			pRole->SendMessage(&send, sizeof(send));
		}
	}

	// ͬ�����ݿ�
	NET_DB2C_syn_account_silver	sendDB;
	sendDB.dw_role_id = dw_role_id;
	sendDB.money.nGold = MSilver2DBGold(GetAccountSilver());
	sendDB.money.nSilver = MSilver2DBSilver(GetAccountSilver());
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return TRUE;
}


BOOL CTradeYB::Init()
{
	m_mapYBAccount.clear();
	m_setYBBuy.clear();
	m_setYBSell.clear();
	m_mapYBBuyList.clear();
	m_mapYBSellList.clear();
	m_mapOrder2Role.clear();

	m_dwMaxOrderID	= 0;

	return TRUE;
}

CTradeYB::~CTradeYB()
{
	Destroy();
}

//-------------------------------------------------------------------------------------------------------
// ÿСʱִ��һ��
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::OnHour()
{
	// �������׶���
	tagYuanBaoOrder*	pYBOrder = NULL;
	tagDWORDTime		cur_time = GetCurrentDWORDTime();
	DWORD				dwCloseTime = DecreaseTime(cur_time, 2 * 24 * 60 * 60);

	// �����ύ��������ʱ�䣬��Ϊ�ر�״̬
	std::set<tagYuanBaoOrder*, BuyOrderCmp>::iterator itOrderBuy = m_setYBBuy.begin();
	while(itOrderBuy != m_setYBBuy.end())
	{
		pYBOrder = *itOrderBuy;
		itOrderBuy++;

		if(dwCloseTime >= pYBOrder->dwStartTime)
			DeleteYBOrder(pYBOrder, EYBOM_Close);
	}

	std::set<tagYuanBaoOrder*, SellOrderCmp>::iterator itOrderSell = m_setYBSell.begin();
	while(itOrderSell != m_setYBSell.end())
	{
		pYBOrder = *itOrderSell;
		itOrderSell++;

		if(dwCloseTime >= pYBOrder->dwStartTime)
			DeleteYBOrder(pYBOrder, EYBOM_Close);
	}

	// ɾ�����ݿ���ʱ�䳬����������ж���
	DWORD	dwDeleteTime = DecreaseTime(cur_time, 7 * 24 * 60 * 60);
	NET_DB2C_delete_yuanbao_order	sendDB;
	sendDB.dw_delete_time = dwDeleteTime;
	g_dbSession.Send(&sendDB, sendDB.dw_size);
}

VOID CTradeYB::Destroy()
{
	tagYuanBaoAccount* pYBAccount = NULL;
	package_map<DWORD, tagYuanBaoAccount*>::map_iter itAccount = m_mapYBAccount.begin();
	while( m_mapYBAccount.find_next(itAccount, pYBAccount) )
	{
		SAFE_DELETE(pYBAccount);
	}

	tagYuanBaoOrder* pYBOrder = NULL;
	std::set<tagYuanBaoOrder*, BuyOrderCmp>::iterator itOrderBuy = m_setYBBuy.begin();
	while(itOrderBuy != m_setYBBuy.end())
	{
		SAFE_DELETE(*itOrderBuy);
		itOrderBuy++;
	}
	m_setYBBuy.clear();

	std::set<tagYuanBaoOrder*, SellOrderCmp>::iterator itOrderSell = m_setYBSell.begin();
	while(itOrderSell != m_setYBSell.end())
	{
		SAFE_DELETE(*itOrderSell);
		itOrderSell++;
	}
	m_setYBSell.clear();
	
	m_mapYBBuyList.clear();
	m_mapYBSellList.clear();
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�������˻�
//-------------------------------------------------------------------------------------------------------
tagYuanBaoAccount* CTradeYB::CreateTradeAccount(DWORD dw_role_id)
{
	tagYuanBaoAccount *pYBAccount = new tagYuanBaoAccount(dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return (tagYuanBaoAccount*)INVALID_VALUE;

	m_mapYBAccount.add(dw_role_id, pYBAccount);

	// insert to db
	NET_DB2C_insert_account	sendDB;
	sendDB.dw_role_id = dw_role_id;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return pYBAccount;
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�������˻�
//-------------------------------------------------------------------------------------------------------
tagYuanBaoAccount* CTradeYB::CreateTradeAccount(const tagYBAccount *pYBAccount)
{
	tagYuanBaoAccount *pYuanBaoAccount = new tagYuanBaoAccount(pYBAccount->dw_role_id, pYBAccount->nYuanBao,
						pYBAccount->nGold, pYBAccount->nSilver, pYBAccount->bSellOrder, pYBAccount->bBuyOrder);
	if(!VALID_POINT(pYuanBaoAccount))
		return (tagYuanBaoAccount*)INVALID_VALUE;

	m_mapYBAccount.add(pYBAccount->dw_role_id, pYuanBaoAccount);

	return pYuanBaoAccount;
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�������б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::CalYBSellList()
{
	tagYuanBaoOrder* pYBOrder = NULL;

	set<tagYuanBaoOrder*, SellOrderCmp>::iterator itOrderSell = m_setYBSell.begin();
	while( itOrderSell != m_setYBSell.end())
	{
		pYBOrder = *itOrderSell;
		InserYBSellList(pYBOrder->nPrice, pYBOrder->GetRemainNum());
		++itOrderSell;
	}
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ���չ��б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::CalYBBuyList()
{
	tagYuanBaoOrder* pYBOrder = NULL;

	set<tagYuanBaoOrder*, BuyOrderCmp>::iterator itOrderBuy = m_setYBBuy.begin();
	while( itOrderBuy != m_setYBBuy.end())
	{
		pYBOrder = *itOrderBuy;
		InserYBBuyList(pYBOrder->nPrice, pYBOrder->GetRemainNum());
		++itOrderBuy;
	}
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�������б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::InserYBSellList(INT nPrice, INT n_num)
{
	m_mapYBSellList.modify_value(nPrice, n_num);
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ���չ��б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::InserYBBuyList(INT nPrice, INT n_num)
{
	m_mapYBBuyList.modify_value(nPrice, n_num);
}

//-------------------------------------------------------------------------------------------------------
// ɾ��Ԫ�������б���ͬ�۸�Ķ���
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::RemoveYBSellList(INT nPrice, INT n_num, DWORD dw_role_id, BOOL bNoticeClient)
{
	m_mapYBSellList.modify_value(nPrice, -n_num);
	INT		nTotalNum = m_mapYBSellList.find(nPrice);

	if(nTotalNum == 0)
	{
		m_mapYBSellList.erase(nPrice);
	}

	if(nTotalNum < 0)
	{
		SI_LOG->write_log(_T("YuanBao Num in m_mapYBSellList < 0  %u\r\n"), nPrice);
	}

	// ͬ���ͻ���
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_price_list	send;
			send.nPrice = nPrice;
			send.n_num = nTotalNum;
			pRole->SendMessage(&send, send.dw_size);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// ɾ��Ԫ���չ��б���ͬ�۸�Ķ���(dw_role_id, ��Ҫͬ�������ID��
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::RemoveYBBuyList(INT nPrice, INT n_num, DWORD dw_role_id, BOOL bNoticeClient)
{
	m_mapYBBuyList.modify_value(nPrice, -n_num);
	INT		nTotalNum = m_mapYBBuyList.find(nPrice);

	if(nTotalNum == 0)
	{
		m_mapYBBuyList.erase(nPrice);
	}

	if(nTotalNum < 0)
	{
		SI_LOG->write_log(_T("YuanBao Num in m_mapYBBuyList < 0  %u\r\n"), nPrice);
	}

	// ͬ���ͻ���
	if(bNoticeClient == TRUE)
	{
		Role *pRole = g_roleMgr.get_role(dw_role_id);
		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_buy_price_list	send;
			send.nPrice = nPrice;
			send.n_num = nTotalNum;
			pRole->SendMessage(&send, send.dw_size);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�����׶���
//-------------------------------------------------------------------------------------------------------
tagYuanBaoOrder* CTradeYB::CreateYBOrder(DWORD dw_role_id, EYBOTYPE eYBOType, INT nPrice, INT n_num)
{
	tagYuanBaoOrder* pYBOrder = new tagYuanBaoOrder;
	if(!VALID_POINT(pYBOrder))		
		return (tagYuanBaoOrder*)INVALID_VALUE;

	if(eYBOType != 	EYBOT_BUY && eYBOType != EYBOT_SELL)
		return (tagYuanBaoOrder*)INVALID_VALUE;

	pYBOrder->dwID = m_dwMaxOrderID;
	pYBOrder->dw_role_id = dw_role_id;
	pYBOrder->eYBOType = eYBOType;
	pYBOrder->nPrice = nPrice;
	pYBOrder->n_count = n_num;
	pYBOrder->eYBOMode = EYBOM_Submit;
	pYBOrder->dwStartTime = g_world.GetWorldTime();

	if(eYBOType == 	EYBOT_BUY)
	{
		m_setYBBuy.insert(pYBOrder);
		InserYBBuyList(nPrice, n_num);
	}
	else
	{
		m_setYBSell.insert(pYBOrder);
		InserYBSellList(nPrice, n_num);
	}

	// insert to db
	NET_DB2C_insert_order		sendDB;
	sendDB.yb_order.dwID = pYBOrder->dwID;
	sendDB.yb_order.dw_role_id = pYBOrder->dw_role_id;
	sendDB.yb_order.nType = pYBOrder->eYBOType;
	sendDB.yb_order.nPrice = pYBOrder->nPrice;
	sendDB.yb_order.n_count = pYBOrder->n_count;
	sendDB.yb_order.nYBMode = pYBOrder->eYBOMode;
	sendDB.yb_order.dwStartTime = pYBOrder->dwStartTime;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	// ���붩��ID�����ID�Ķ�Ӧ����
	m_mapOrder2Role.add(m_dwMaxOrderID, dw_role_id);
	// ����������һ
	++m_dwMaxOrderID;

	return pYBOrder;
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�����׶���
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::LoadYOOrder(tagYuanBaoOrder* pYuanBaoOrder)
{
	if(!VALID_POINT(pYuanBaoOrder))
		return;

	tagYuanBaoOrder* pYBOrder = new tagYuanBaoOrder;
	if(!VALID_POINT(pYBOrder))		
		return;

	*pYBOrder = *pYuanBaoOrder;

	if(pYBOrder->eYBOType == EYBOT_BUY)
	{
		m_setYBBuy.insert(pYBOrder);
		InserYBBuyList(pYBOrder->nPrice, pYBOrder->GetRemainNum());
	}
	else
	{
		m_setYBSell.insert(pYBOrder);
		InserYBSellList(pYBOrder->nPrice, pYBOrder->GetRemainNum());
	}

	// ���붩��ID�����ID�Ķ�Ӧ����
	m_mapOrder2Role.add(pYBOrder->dwID, pYBOrder->dw_role_id);

	// �õ���󶩵�����
	//if(pYBOrder->dwID > m_dwMaxOrderID)
	//	m_dwMaxOrderID = pYBOrder->dwID +1;
}

//-------------------------------------------------------------------------------------------------------
// ������ر�Ԫ�����׶���
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::DeleteYBOrder(tagYuanBaoOrder* pYBOrder, EYBOMODE eYBOMode)
{
	if(eYBOMode != 	EYBOM_Cancel && eYBOMode != EYBOM_Close)
		return;

	BOOL bNoticeClient = FALSE;
	if(EYBOM_Cancel == eYBOMode)
		bNoticeClient = TRUE;

	// �õ���ҽ����˻�
	tagYuanBaoAccount* pYBAccount = m_mapYBAccount.find(pYBOrder->dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return;

	if(pYBOrder->eYBOType == EYBOT_BUY)
	{
		pYBOrder->eYBOMode = eYBOMode;
		pYBOrder->dwEndTime = g_world.GetWorldTime();

		RemoveYBBuyList(pYBOrder->nPrice, pYBOrder->GetRemainNum(), pYBOrder->dw_role_id, bNoticeClient);
		m_setYBBuy.erase(pYBOrder);
		pYBAccount->SetBuyOrder(FALSE);
	}
	else
	{
		pYBOrder->eYBOMode = eYBOMode;
		pYBOrder->dwEndTime = g_world.GetWorldTime();

		RemoveYBSellList(pYBOrder->nPrice, pYBOrder->GetRemainNum(), pYBOrder->dw_role_id, bNoticeClient);
		m_setYBSell.erase(pYBOrder);
		pYBAccount->SetSellOrder(FALSE);
	}

	// Update to db
	NET_DB2C_complete_order	sendDB;
	sendDB.complete_order.dwID = pYBOrder->dwID;
	sendDB.complete_order.nYBOMode = eYBOMode;
	sendDB.complete_order.dwEndTime = pYBOrder->dwEndTime;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	// ɾ������ID�����ID�Ķ�Ӧ����
	m_mapOrder2Role.erase(pYBOrder->dwID);
	SAFE_DELETE(pYBOrder);
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ�����׶���(���ض���ʣ��Ԫ������
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::RefreshYBOrder(tagYuanBaoOrder* pYBOrder, DWORD dw_role_id, INT nPrice, INT n_num, BOOL bNoticeClient)
{
	if(n_num == 0)
		return;

	if(!VALID_POINT(pYBOrder))
		return;

	tagYuanBaoAccount* pYBAccount = m_mapYBAccount.find(pYBOrder->dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return;

	if(pYBOrder->eYBOType == EYBOT_BUY)
	{
		pYBOrder->nAvgPrice = (pYBOrder->nAvgPrice * pYBOrder->nDealNum + nPrice * n_num) / (pYBOrder->nDealNum + n_num);
		pYBOrder->nDealNum += n_num;
		
		// �������Ԫ�������˻�
		pYBAccount->IncAccountYB(n_num, INVALID_VALUE, bNoticeClient);
		pYBAccount->DecAccountSilver(nPrice*n_num, INVALID_VALUE, bNoticeClient);

		// ɾ��Ԫ���չ��б���ͬ�۸�Ķ���
		RemoveYBBuyList(pYBOrder->nPrice, n_num, dw_role_id, TRUE);
	}
	else
	{
		pYBOrder->nAvgPrice = (pYBOrder->nAvgPrice * pYBOrder->nDealNum + nPrice * n_num) / (pYBOrder->nDealNum + n_num);
		pYBOrder->nDealNum += n_num;

		// �������Ԫ�������˻�
		pYBAccount->DecAccountYuanBao(n_num, INVALID_VALUE, bNoticeClient);
		pYBAccount->IncAccountSilver(nPrice*n_num, INVALID_VALUE, bNoticeClient);

		// ɾ��Ԫ�������б���ͬ�۸�Ķ���
		RemoveYBSellList(pYBOrder->nPrice, n_num, dw_role_id, TRUE);
	}
	
	// Update to db
	NET_DB2C_refresh_order	sendDB;
	sendDB.refresh_order.dwID = pYBOrder->dwID;
	sendDB.refresh_order.nAvgPrice = pYBOrder->nAvgPrice;
	sendDB.refresh_order.nDealNum = pYBOrder->nDealNum;
	g_dbSession.Send(&sendDB, sendDB.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// ����Ԫ��
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::DealYBSell(tagYuanBaoOrder* pYBSellOrder)
{
	BOOL	bNoticeClient = FALSE;	

	// �����չ�������Ѱ��ƥ��Ķ���
	tagYuanBaoOrder* pYBBuyOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	std::set<tagYuanBaoOrder*, BuyOrderCmp>::iterator itBuyOrder = m_setYBBuy.begin();
	while (itBuyOrder != m_setYBBuy.end())
	{
		pYBBuyOrder = (*itBuyOrder);
		++itBuyOrder;

		// ���׶����໥ƥ��
		if (pYBBuyOrder->nPrice >= pYBSellOrder->nPrice)
		{
			bNoticeClient = TRUE;
			INT		nRemainNum = 0;

			if(pYBSellOrder->GetRemainNum() > pYBBuyOrder->GetRemainNum())
			{
				nRemainNum = pYBBuyOrder->GetRemainNum();
				RefreshYBOrder(pYBBuyOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBSellOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBBuyOrder, EYBOM_Close);
			}
			else if(pYBSellOrder->GetRemainNum() < pYBBuyOrder->GetRemainNum())
			{
				nRemainNum = pYBSellOrder->GetRemainNum();
				RefreshYBOrder(pYBBuyOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBSellOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBSellOrder, EYBOM_Close);
				break;
			}
			else
			{
				nRemainNum = pYBBuyOrder->GetRemainNum();
				RefreshYBOrder(pYBBuyOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBSellOrder, pYBSellOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBBuyOrder, EYBOM_Close);
				DeleteYBOrder(pYBSellOrder, EYBOM_Close);
				break;
			}
		}
	}

	// û��ƥ��Ķ���ʱ��ͬ��һ�¼۸��б�
	if(bNoticeClient == FALSE)
	{
		INT		nTotalNum = m_mapYBSellList.find(pYBSellOrder->nPrice);
		Role	*pRole = g_roleMgr.get_role(pYBSellOrder->dw_role_id);

		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_price_list	send;
			send.nPrice = pYBSellOrder->nPrice;
			send.n_num = nTotalNum;
			pRole->SendMessage(&send, send.dw_size);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// �չ�Ԫ��
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::DealYBBuy(tagYuanBaoOrder* pYBBuyOrder)
{
	// �������۶�����Ѱ��ƥ��Ķ���
	BOOL	bNoticeClient = FALSE;

	tagYuanBaoOrder* pYBSellOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	std::set<tagYuanBaoOrder*, SellOrderCmp>::iterator itSellOrder = m_setYBSell.begin();
	while (itSellOrder != m_setYBSell.end())
	{
		pYBSellOrder = (*itSellOrder);
		++itSellOrder;

		// ���׶����໥ƥ��
		if(pYBBuyOrder->nPrice >= pYBSellOrder->nPrice)
		{
			bNoticeClient = TRUE;
			INT		nRemainNum = 0;

			if(pYBBuyOrder->GetRemainNum() > pYBSellOrder->GetRemainNum())
			{
				nRemainNum = pYBSellOrder->GetRemainNum();
				RefreshYBOrder(pYBSellOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBBuyOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBSellOrder, EYBOM_Close);
			}
			else if(pYBBuyOrder->GetRemainNum() < pYBSellOrder->GetRemainNum())
			{
				nRemainNum = pYBBuyOrder->GetRemainNum();
				RefreshYBOrder(pYBSellOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBBuyOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBBuyOrder, EYBOM_Close);
				break;
			}
			else
			{
				nRemainNum = pYBBuyOrder->GetRemainNum();
				RefreshYBOrder(pYBSellOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, FALSE);
				RefreshYBOrder(pYBBuyOrder, pYBBuyOrder->dw_role_id, pYBSellOrder->nPrice, nRemainNum, TRUE);
				DeleteYBOrder(pYBBuyOrder, EYBOM_Close);
				DeleteYBOrder(pYBSellOrder, EYBOM_Close);
				break;
			}
		}
		else
			break;
	}

	// û��ƥ��Ķ���ʱ��ͬ��һ�¼۸��б�
	if(bNoticeClient == FALSE)
	{
		INT		nTotalNum = m_mapYBBuyList.find(pYBBuyOrder->nPrice);
		Role	*pRole = g_roleMgr.get_role(pYBBuyOrder->dw_role_id);

		if(VALID_POINT(pRole))
		{
			NET_SIS_synchronize_buy_price_list	send;
			send.nPrice = pYBBuyOrder->nPrice;
			send.n_num = nTotalNum;
			pRole->SendMessage(&send, send.dw_size);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// ͬ���ͻ����������۵ļ۸��б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::SynSellPriceList(Role *pRole)
{
	INT		nListNum = m_mapYBSellList.size();
	DWORD	dw_size = sizeof(NET_SIS_get_sell_price_list);
	INT		nIndex = 0;
	INT		nPrice = 0;
	INT		n_num = 0;

	if(!VALID_POINT(pRole))
		return;

	if(nListNum >0)
	{
		dw_size += (nListNum - 1) * sizeof(tagYBPriceList);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_get_sell_price_list);

	m_mapYBSellList.reset_iterator();
	while(m_mapYBSellList.find_next(nPrice, n_num))
	{
		pSend->YBPriceList[nIndex].nPrice = nPrice;
		pSend->YBPriceList[nIndex].n_num = n_num;
		++nIndex;
	}

	pSend->nListNum = nListNum;
	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

//-------------------------------------------------------------------------------------------------------
// ͬ���ͻ��������չ��ļ۸��б�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::SynBuyPriceList(Role *pRole)
{
	INT		nListNum = m_mapYBBuyList.size();
	DWORD	dw_size = sizeof(NET_SIS_get_buy_price_list);
	INT		nIndex = 0;
	INT		nPrice = 0;
	INT		n_num = 0;

	if(!VALID_POINT(pRole))
		return;

	if(nListNum > 0)
	{
		dw_size += (nListNum - 1) * sizeof(tagYBPriceList);
	}

	CREATE_MSG(pSend, dw_size, NET_SIS_get_buy_price_list);

	m_mapYBBuyList.reset_iterator();
	while(m_mapYBBuyList.find_next(nPrice, n_num))
	{
		pSend->YBPriceList[nIndex].nPrice = nPrice;
		pSend->YBPriceList[nIndex].n_num = n_num;
		++nIndex;
	}

	pSend->nListNum = nListNum;
	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

//-------------------------------------------------------------------------------------------------------
// ͬ���ͻ���Ԫ�������˻�
//-------------------------------------------------------------------------------------------------------
VOID CTradeYB::SynYBAccount(Role *pRole)
{
	if(!VALID_POINT(pRole))
		return;

	tagYuanBaoAccount *pYBAccount = m_mapYBAccount.find(pRole->GetID());
	if(!VALID_POINT(pYBAccount))
		return;

	NET_SIS_get_yuanbao_account	send;
	send.dw_role_id = pYBAccount->dw_role_id;
	send.nYuanBao = pYBAccount->GetAccountYB();
	send.nSilver = pYBAccount->GetAccountSilver();
	send.bSellOrder = pYBAccount->bSellOrder;
	send.bBuyOrder = pYBAccount->bBuyOrder;
	pRole->SendMessage(&send, send.dw_size);
};

