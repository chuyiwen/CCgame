/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//商店处理

#include "StdAfx.h"
#include "../../common/WorldDefine/shop_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "role.h"
#include "creature.h"
#include "shop.h"
#include "creature_ai.h"
#include "guild_manager.h"
#include "guild.h"

//-----------------------------------------------------------------------------
// 获取物品(非装备)店中刷新商品列表
//-----------------------------------------------------------------------------
DWORD Role::GetShopItems(DWORD dwNPCID, BYTE byShelf)
{
	// 获得地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC)
		|| !pNPC->IsFunctionNPC(EFNPCT_Shop) 
		|| !pNPC->CheckNPCTalkDistance(this))
	{
		return E_Shop_NPCNotValid;
	}

	// 找到商店
	Shop *pShop = pMap->get_shop(pNPC->GetTypeID());
	if(!VALID_POINT(pShop) || pShop->IsEquipShop())
	{
		return E_Shop_NotValid;
	}

	INT16 n16RareItemNum = pShop->GetRareGoodsNum(byShelf);
	
	// 没有刷新物品
	if(0 == n16RareItemNum)
	{
		NET_SIS_get_shop_item Send;
		Send.dwNPCID	= dwNPCID;
		Send.byShelf	= byShelf;
		Send.dwShopID	= pNPC->GetShopID();
		Send.n16RareItemNum = 0;

		SendMessage(&Send, Send.dw_size);
		
		return E_Success;
	}

	// 有刷新物品
	INT32 nSzMsg = sizeof(NET_SIS_get_shop_item) - 1 + sizeof(tagMsgShopItem) * n16RareItemNum;

	CREATE_MSG(pSend, nSzMsg, NET_SIS_get_shop_item);
	pSend->nShopExploitsNum = n_shop_exploits_limit;
	pSend->dw_size	= nSzMsg;
	pSend->dwNPCID	= dwNPCID;
	pSend->byShelf	= byShelf;
	pSend->dwShopID	= pNPC->GetShopID();
	pSend->n16RareItemNum = n16RareItemNum;

	pShop->GetRareItems((tagMsgShopItem*)pSend->byData, n16RareItemNum, byShelf);
	
	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获取所有装备
//-----------------------------------------------------------------------------
DWORD Role::GetShopEquips(DWORD dwNPCID, BYTE byShelf)
{
	// 获得地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC)
		|| !pNPC->IsFunctionNPC(EFNPCT_Shop) 
		|| !pNPC->CheckNPCTalkDistance(this))
	{
		return E_Shop_NPCNotValid;
	}

	// 找到商店
	Shop *pShop = pMap->get_shop(pNPC->GetTypeID());
	if(!VALID_POINT(pShop) || !pShop->IsEquipShop())
	{
		return E_Shop_NotValid;
	}

	INT16 n16RareEquipNum = pShop->GetRareGoodsNum(byShelf);

	// 没有刷新物品
	if(0 == n16RareEquipNum)
	{
		NET_SIS_get_shop_equip Send;
		Send.dwNPCID		= dwNPCID;
		Send.byShelf		= byShelf;
		Send.dwShopID		= pNPC->GetShopID();
		Send.n16EquipNum	= 0;

		SendMessage(&Send, Send.dw_size);
		
		return E_Success;
	}

	INT32 nSzMsg = sizeof(NET_SIS_get_shop_equip) - 1 + sizeof(tagMsgShopEquip) * n16RareEquipNum;

	CREATE_MSG(pSend, nSzMsg, NET_SIS_get_shop_equip);
	pSend->dw_size	= nSzMsg;
	pSend->dwNPCID	= dwNPCID;
	pSend->byShelf	= byShelf;
	pSend->dwShopID	= pNPC->GetShopID();
	pSend->n16EquipNum = n16RareEquipNum;

	pShop->GetRareEquips((tagMsgShopEquip*)pSend->byData, byShelf, pSend->n16EquipNum);

	// 重新计算大小
	if(pSend->n16EquipNum != n16RareEquipNum)
	{
		pSend->dw_size -= (n16RareEquipNum - pSend->n16EquipNum) * sizeof(tagMsgShopEquip);
	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 购买物品(非装备)
//-----------------------------------------------------------------------------
DWORD Role::BuyShopItem(DWORD dwNPCID, BYTE byShelf, DWORD dw_data_id, INT16 n16ItemNum)
{
	// 行囊是否解锁
	/*if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}*/
	
	// 获得地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC)
		|| !pNPC->IsFunctionNPC(EFNPCT_Shop) 
		|| !pNPC->CheckNPCTalkDistance(this))
	{
		return E_Shop_NPCNotValid;
	}

	// 找到商店
	Shop *pShop = pMap->get_shop(pNPC->GetTypeID());
	if(!VALID_POINT(pShop) || pShop->IsEquipShop())
	{
		return E_Shop_NotValid;
	}
	
	// 检查玩家背包是否有地方
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Shop_NotEnough_SpaceInBag;
	}

	// 同类物品数量达到上限
	if (!GetItemMgr().CanAddMaxHoldItem(dw_data_id, n16ItemNum))
	{
		return E_Shop_ItemMaxHold;
	}

	tagSellItem sell_goods;
	DWORD dw_error_code = pShop->sell_goods(this, byShelf, dw_data_id, n16ItemNum, sell_goods);
	if(dw_error_code != E_Success
		&& dw_error_code != E_Shop_ItemNotEnough)
	{
		return dw_error_code;
	}

	// 从玩家背包中扣除金钱
	if(sell_goods.nSilverCost > 0)
	{
		GetCurMgr().DecBagSilver(sell_goods.nSilverCost, elcid_shop_buy_item);
	}

	// 扣除特殊消耗
	if(sell_goods.nSpecCost > 0)
	{
		if(ECCT_ItemExchange == sell_goods.eCostType)
		{
			GetItemMgr().DelBagSameItem(sell_goods.listItem, sell_goods.nSpecCost, elcid_shop_buy_item);
		}
		else if(sell_goods.eCostType == ECCT_BaiBaoYuanBao)
		{
			GetCurMgr().DecBaiBaoYuanBao(sell_goods.nSpecCost, elcid_shop_buy_item);
		}
		else if(sell_goods.eCostType == ECCT_GuildContribe)
		{
			guild* pGuild = g_guild_manager.get_guild(GetGuildID());
			if(VALID_POINT(pGuild))
			{
				pGuild->change_member_contribution(GetID(), sell_goods.nSpecCost, FALSE);
			}
		}
		else if(sell_goods.eCostType != ECCT_Null)
		{
			GetCurMgr().DecCurrency(sell_goods.eCostType, sell_goods.nSpecCost, elcid_shop_buy_equip);
		}
	}

	// 将装备放到玩家背包中
	if(VALID_POINT(sell_goods.pItemSell))
	{
		GetItemMgr().Add2Bag(sell_goods.pItemSell, elcid_shop_buy_item, TRUE);
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
	if(sell_goods.n16RemainNum != INVALID_VALUE)
	{
		NET_SIS_buy_shop_item Send;
		Send.dwNPCID		= dwNPCID;
		Send.byShelf		= byShelf;
		Send.n16RemainNum	= sell_goods.n16RemainNum;
		Send.dw_data_id		= dw_data_id;
		Send.byIndex		= sell_goods.byIndex;

		SendMessage(&Send, Send.dw_size);
	}
	

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 买装备
//-----------------------------------------------------------------------------
DWORD Role::BuyShopEquip(DWORD dwNPCID, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial)
{
	// 行囊是否解锁
	/*if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}*/
	
	// 获得地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC)
		|| !pNPC->IsFunctionNPC(EFNPCT_Shop) 
		|| !pNPC->CheckNPCTalkDistance(this))
	{
		return E_Shop_NPCNotValid;
	}

	// 找到商店
	Shop *pShop = pMap->get_shop(pNPC->GetTypeID());
	if(!VALID_POINT(pShop) || !pShop->IsEquipShop())
	{
		return E_Shop_NotValid;
	}

	// 检查玩家背包是否有地方
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Shop_NotEnough_SpaceInBag;
	}

	// 同类物品数量达到上限
	if (!GetItemMgr().CanAddMaxHoldItem(dw_data_id, 1))
	{
		return E_Shop_ItemMaxHold;
	}

	tagSellEquip SellEquip;
	DWORD dw_error_code = pShop->SellEquip(this, byShelf, dw_data_id, n64_serial, SellEquip);
	if(dw_error_code != E_Success
		&& dw_error_code != E_Shop_Equip_Sold
		&& dw_error_code != E_Shop_CreateEquip_Failed)
	{
		return dw_error_code;
	}

	// 从玩家背包中扣除金钱
	if(SellEquip.nSilverCost > 0)
	{
		GetCurMgr().DecBagSilver(SellEquip.nSilverCost, elcid_shop_buy_equip);
	}

	// 扣除特殊消耗
	if(SellEquip.nSpecCost > 0)
	{
		if(ECCT_ItemExchange == SellEquip.eCostType)
		{
			GetItemMgr().DelBagSameItem(SellEquip.listItem, SellEquip.nSpecCost, elcid_shop_buy_equip);
		}
		else if(SellEquip.eCostType == ECCT_BaiBaoYuanBao)
		{
			GetCurMgr().DecBaiBaoYuanBao(SellEquip.nSpecCost, elcid_shop_buy_item);
		}
		else if(SellEquip.eCostType == ECCT_GuildContribe)
		{
			guild* pGuild = g_guild_manager.get_guild(GetGuildID());
			if(VALID_POINT(pGuild))
			{
				pGuild->change_member_contribution(GetID(), SellEquip.nSpecCost, FALSE);
			}
		}
		else if(SellEquip.eCostType != ECCT_Null)
		{
			GetCurMgr().DecCurrency(SellEquip.eCostType, SellEquip.nSpecCost, elcid_shop_buy_equip);
		}
	}

	// 将装备放到玩家背包中
	if(VALID_POINT(SellEquip.pEquipSell))
	{
		tagItem *pItem = SellEquip.pEquipSell;
		GetItemMgr().Add2Bag(pItem, elcid_shop_buy_equip, TRUE);
	}

	// 卖出的为普通武器
	if(INVALID_VALUE == SellEquip.n16RemainNum)
	{
		return dw_error_code;
	}
	
	// 发送更新后商店物品 -- 只更新稀有武器
	if(VALID_POINT(SellEquip.pEquipNew))
	{
		INT32 nMsgSz = sizeof(NET_SIS_buy_shop_equip) - 1 + SIZE_EQUIP;

		CREATE_MSG(pSend, nMsgSz, NET_SIS_buy_shop_equip);
		pSend->dwNPCID		= dwNPCID;
		pSend->byShelf		= byShelf;
		pSend->n16RemainNum	= SellEquip.n16RemainNum;
		pSend->dw_size		= nMsgSz;
		pSend->byIndex		= SellEquip.byIndex;
		
		get_fast_code()->memory_copy(pSend->byData, SellEquip.pEquipNew, SIZE_EQUIP);
		
		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
	else
	{
		// 刷新装备，且没有待售装备，更新个数
		NET_SIS_buy_shop_equip Send;
		Send.dwNPCID		= dwNPCID;
		Send.byShelf		= byShelf;
		Send.n16RemainNum	= SellEquip.n16RemainNum;
		Send.byIndex		= SellEquip.byIndex;

		ASSERT(0 == SellEquip.n16RemainNum);

		SendMessage(&Send, Send.dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 向商店出售物品&装备
//-----------------------------------------------------------------------------
DWORD Role::SellToShop(DWORD dwNPCID, int nNUmber, INT64* n64_serial)
{
	// 行囊是否解锁
	/*if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}*/
	
	// 获得地图
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// 找到NPC，并检查合法性
	Creature* pNPC = pMap->find_creature(dwNPCID);
	if(!VALID_POINT(pNPC)
		|| !pNPC->IsFunctionNPC(EFNPCT_Shop) 
		|| !pNPC->CheckNPCTalkDistance(this))
	{
		return E_Shop_NPCNotValid;
	}

	// 找到商店
	Shop *pShop = pMap->get_shop(pNPC->GetTypeID());
	if(!VALID_POINT(pShop))
	{
		return E_Shop_NotValid;
	}

	package_list<tagItem*> itemList;
	INT64 n64Price = 0;
	for (int i = 0; i < nNUmber; i++)
	{
		// 找到待售物品
		tagItem *pItem = GetItemMgr().GetBagItem(n64_serial[i]);
		if(!VALID_POINT(pItem))
			continue;

		// 是否可出售
		if(!GetItemMgr().CanSell(*pItem))
			continue;

		// 修炼过的武器不能卖
		//if(MIsEquipment(pItem->dw_data_id))
		//{
		//	tagEquip* pEquip = (tagEquip*)pItem;

		//	if(pEquip->equipSpec.nCurLevelExp > 0 || pEquip->equipSpec.nLevel > 1)
		//		return E_Shop_ItemCannotSellXiulian;
		//}



		// 计算售价//??没有计算税收影响
		n64Price += pItem->pProtoType->nBasePrice * pItem->n16Num;
		//if(MIsEquipment(pItem->dw_data_id))
		//{
		//	FLOAT fFactor;
		//	MCalPriceFactor(fFactor, ((tagEquip*)pItem)->equipSpec.byQuality);
		//	n64Price = (INT64)(n64Price * fFactor);
		//	if(n64Price < 0)
		//	{
		//		ASSERT(0);
		//		n64Price = 0;
		//	}
		//}

		itemList.push_back(pItem);
		// 从玩家身上删除物品
		DWORD dw_error_code = GetItemMgr().DelFromBag(n64_serial[i], elcid_shop_sell, INVALID_VALUE, TRUE);
		if(dw_error_code != E_Success)
			continue;


	}

	
	// 防沉迷
	float fEarnRate = GetEarnRate() / 10000.0f;
	n64Price = (INT64)(n64Price * fEarnRate);

	// 玩家获得金钱
	GetCurMgr().IncBagSilver(n64Price, elcid_shop_sell);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 商店消息处理结构反馈给客户端
//-----------------------------------------------------------------------------
VOID Role::SendShopFeedbackMsg(DWORD dw_error_code, DWORD dwNPCID)
{
	// 成功也会返回，客户端需要做音效处理
	NET_SIS_feedback_from_shop Send;
	Send.dw_error_code	= dw_error_code;
	Send.dwNPCID		= dwNPCID;

	SendMessage(&Send, Send.dw_size);
}