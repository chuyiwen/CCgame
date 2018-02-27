/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once

#include "../../common/WorldDefine/mall_define.h"
#include "../../common/WorldDefine/currency_define.h"
#include "currency.h"
#include <set>

//--------------------------------------------------------------------------------------------
// ���Ԫ�����׵Ľ����˻�
//--------------------------------------------------------------------------------------------
struct tagYuanBaoAccount
{
	DWORD				dw_role_id;				//���ID
	Currency<INT>		YuanBao;				//Ԫ������
	Currency<INT64>		Silver;					//��Ǯ����
	BOOL				bSellOrder;				//�Ƿ��г��۶���
	BOOL				bBuyOrder;				//�Ƿ����չ�����
	DWORD				dwQuestTick;			//��ѯ����ʱ�䣨��ֹ�ͻ���Ƶ����ѯ���ܶ�����
	tagYuanBaoAccount(DWORD dwID):YuanBao(0, MAX_YB_NUM), Silver(0, MAX_SILVER_NUM)
	{
		dw_role_id = dwID;
		bSellOrder = FALSE;
		bBuyOrder = FALSE;
		dwQuestTick = 0;
	}
	tagYuanBaoAccount(DWORD dwID, INT32 nYuanBao, INT32 nGold, INT32 nSilver, BOOL bSell, BOOL bBuy):YuanBao(nYuanBao, MAX_YB_NUM),
					Silver((MGold2Silver(nGold)+(INT64)nSilver), MAX_SILVER_NUM)
	{
		dw_role_id = dwID;
		bSellOrder = bSell;
		bBuyOrder = bBuy;
		dwQuestTick = 0;
	}

	// ��ƿͻ��˲�ѯʱ��
	VOID			SetQuestTick(DWORD dwTick) { dwQuestTick = dwTick; }
	DWORD			GetQuestTick() { return dwQuestTick; }

	// �����Ƿ��ύ������
	VOID			SetSellOrder(BOOL bSubmit);
	VOID			SetBuyOrder(BOOL bSubmit);

	// �õ��˻���ǰ��Ǯ��
	inline INT		GetAccountYB() { return YuanBao.GetCur(); }
	inline INT64	GetAccountSilver() { return Silver.GetCur(); }
	// ���˻���Ǯ
	BOOL			IncAccountYB(INT nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	BOOL			IncAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	// ���˻�ȡǮ
	BOOL			DecAccountYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	BOOL			DecAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	// ���Դ���Ľ�Ǯ��Ŀ
	INT64			GetCanIncAccountSilver()	{ return Silver.GetMax() - GetAccountSilver(); }
	INT				GetCanIncAccountYB()	    { return YuanBao.GetMax() - GetAccountYB(); }
};

//--------------------------------------------------------------------------------------------
// Ԫ�����۶�������
//--------------------------------------------------------------------------------------------
struct SellOrderCmp
{
	bool operator()(tagYuanBaoOrder* pOrder1, tagYuanBaoOrder* pOrder2) const
	{
		if(pOrder1->nPrice < pOrder2->nPrice)
			return TRUE;
		else if(pOrder1->nPrice == pOrder2->nPrice)
		{
			if(pOrder1->dwID < pOrder2->dwID)
				return TRUE;
		}
		return FALSE;
	}
};

//--------------------------------------------------------------------------------------------
// Ԫ���չ���������
//--------------------------------------------------------------------------------------------
struct BuyOrderCmp
{
	bool operator()(tagYuanBaoOrder* pOrder1, tagYuanBaoOrder* pOrder2) const
	{
		if(pOrder1->dwID < pOrder2->dwID)
			return TRUE;
		return FALSE;
	}
};

//--------------------------------------------------------------------------------------------
// ���Ԫ��������
//--------------------------------------------------------------------------------------------
class CTradeYB
{
public:
	CTradeYB() {}
	~CTradeYB();
	BOOL	Init();
	VOID	Destroy();
	VOID	OnHour();

	tagYuanBaoAccount*		GetYBAccount(DWORD dw_role_id) { return m_mapYBAccount.find(dw_role_id); }
	tagYuanBaoOrder*		GetYBSellOrder(DWORD dw_role_id);
	tagYuanBaoOrder*		GetYBBuyOrder(DWORD dw_role_id);
	tagYuanBaoAccount*		CreateTradeAccount(DWORD dw_role_id);
	tagYuanBaoAccount*		CreateTradeAccount(const tagYBAccount *pYBAccount);
	tagYuanBaoOrder*		CreateYBOrder(DWORD dw_role_id, EYBOTYPE eYBOType, INT nPrice, INT n_num);
	VOID					LoadYOOrder(tagYuanBaoOrder* pYBOrder);
	VOID					RefreshYBOrder(tagYuanBaoOrder* pYBOrder, DWORD dw_role_id, INT nPrice, INT n_num, BOOL bNoticeClient);
	VOID					DeleteYBOrder(tagYuanBaoOrder* pYBOrder, EYBOMODE eYBOMode);
	VOID					DealYBSell(tagYuanBaoOrder* pYBOrder);
	VOID					DealYBBuy(tagYuanBaoOrder* pYBBuyOrder);
	VOID					DealYBTrade(tagYuanBaoOrder* pYBOrder, EYBOTYPE eYBOType);
	DWORD					IsCanTradeYB();

	VOID					SynSellPriceList(Role *pRole);
	VOID					SynBuyPriceList(Role *pRole);
	VOID					SynYBAccount(Role *pRole);

	VOID					SetMaxOrderIndex(DWORD dwIndex)	{ m_dwMaxOrderID = dwIndex; };

private:
	VOID					CalYBSellList();
	VOID					CalYBBuyList();
	VOID					InserYBSellList(INT nPrice, INT n_num);
	VOID					InserYBBuyList(INT nPrice, INT n_num);
	VOID					RemoveYBBuyList(INT nPrice, INT n_num, DWORD dw_role_id, BOOL bNoticeClient=FALSE);
	VOID					RemoveYBSellList(INT nPrice, INT n_num, DWORD dw_role_id, BOOL bNoticeClient=FALSE);

private:
	DWORD										m_dwMaxOrderID;					// ��󶩵�����
	std::set<tagYuanBaoOrder*, SellOrderCmp>	m_setYBSell;					// Ԫ�����۶���
	std::set<tagYuanBaoOrder*, BuyOrderCmp>		m_setYBBuy;						// Ԫ���չ�����
	package_map<DWORD, tagYuanBaoAccount*>				m_mapYBAccount;					// ���Ԫ�������˻�
	package_map<DWORD,	DWORD>							m_mapOrder2Role;				// ����ID�����ID�Ķ�Ӧ����
	package_map<INT, INT>								m_mapYBSellList;				// Ԫ�������б�����ͬ���ͻ��� ���� ������
	package_map<INT, INT>								m_mapYBBuyList;					// Ԫ���չ��б�����ͬ���ͻ��� ���� ������
};

inline tagYuanBaoOrder* CTradeYB::GetYBSellOrder(DWORD dw_role_id)
{
	tagYuanBaoOrder* pSellOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	std::set<tagYuanBaoOrder*, SellOrderCmp>::iterator it = m_setYBSell.begin();
	while(it != m_setYBSell.end())
	{
		pSellOrder = *it;
		if(pSellOrder->dw_role_id == dw_role_id)
		{
			return pSellOrder;
		}
		++it;
	}

	return (tagYuanBaoOrder*)INVALID_VALUE;
}

inline tagYuanBaoOrder* CTradeYB::GetYBBuyOrder(DWORD dw_role_id)
{
	tagYuanBaoOrder* pBuyOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	std::set<tagYuanBaoOrder*, BuyOrderCmp>::iterator it = m_setYBBuy.begin();
	while(it != m_setYBBuy.end())
	{
		pBuyOrder = *it;
		if(pBuyOrder->dw_role_id == dw_role_id)
		{
			return pBuyOrder;
		}
		++it;
	}

	return (tagYuanBaoOrder*)INVALID_VALUE;
}

extern CTradeYB g_tradeYB;