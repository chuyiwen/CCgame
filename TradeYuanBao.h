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
// 玩家元宝交易的交易账户
//--------------------------------------------------------------------------------------------
struct tagYuanBaoAccount
{
	DWORD				dw_role_id;				//玩家ID
	Currency<INT>		YuanBao;				//元宝数量
	Currency<INT64>		Silver;					//金钱数量
	BOOL				bSellOrder;				//是否有出售订单
	BOOL				bBuyOrder;				//是否有收购订单
	DWORD				dwQuestTick;			//查询订单时间（防止客户端频繁查询本周订单）
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

	// 设计客户端查询时间
	VOID			SetQuestTick(DWORD dwTick) { dwQuestTick = dwTick; }
	DWORD			GetQuestTick() { return dwQuestTick; }

	// 设置是否提交过订单
	VOID			SetSellOrder(BOOL bSubmit);
	VOID			SetBuyOrder(BOOL bSubmit);

	// 得到账户当前金钱数
	inline INT		GetAccountYB() { return YuanBao.GetCur(); }
	inline INT64	GetAccountSilver() { return Silver.GetCur(); }
	// 向账户存钱
	BOOL			IncAccountYB(INT nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	BOOL			IncAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	// 向账户取钱
	BOOL			DecAccountYuanBao(INT32 nYuanBao, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	BOOL			DecAccountSilver(INT64 n64Silver, DWORD dw_cmd_id, BOOL bNoticeClient=FALSE);
	// 可以存入的金钱数目
	INT64			GetCanIncAccountSilver()	{ return Silver.GetMax() - GetAccountSilver(); }
	INT				GetCanIncAccountYB()	    { return YuanBao.GetMax() - GetAccountYB(); }
};

//--------------------------------------------------------------------------------------------
// 元宝寄售订单排序
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
// 元宝收购订单排序
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
// 玩家元宝交易类
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
	DWORD										m_dwMaxOrderID;					// 最大订单索引
	std::set<tagYuanBaoOrder*, SellOrderCmp>	m_setYBSell;					// 元宝寄售订单
	std::set<tagYuanBaoOrder*, BuyOrderCmp>		m_setYBBuy;						// 元宝收购订单
	package_map<DWORD, tagYuanBaoAccount*>				m_mapYBAccount;					// 玩家元宝交易账户
	package_map<DWORD,	DWORD>							m_mapOrder2Role;				// 订单ID与玩家ID的对应索引
	package_map<INT, INT>								m_mapYBSellList;				// 元宝寄售列表（方便同步客户端 单价 数量）
	package_map<INT, INT>								m_mapYBBuyList;					// 元宝收购列表（方便同步客户端 单价 数量）
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