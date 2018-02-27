/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//商店

#include "StdAfx.h"
#include "../../common/WorldDefine/time.h"
#include "../../common/WorldDefine/shop_protocol.h"
#include "../common/ServerDefine/base_server_define.h"
#include "shop.h"
#include "role.h"
#include "att_res.h"
#include "item_creator.h"
#include "clan_treasury.h"
#include "guild_manager.h"
#include "guild.h"

//*********************************** Shop **************************************

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
Shop::Shop(DWORD dwNPCID, const tagShopProto *pShopProto)
{
	ASSERT(VALID_POINT(pShopProto));

	// 检查商品个数
	INT16 n16TotalNum = 0;
	for(INT32 i=0; i<MAX_SHOP_SHELF_NUM; ++i)
	{
		ASSERT(pShopProto->n16Num[i] >= 0);
		n16TotalNum += pShopProto->n16Num[i];
	}

	ASSERT(n16TotalNum <= MAX_SHOP_TOTAL_ITEM);
	
	// 初始化
	m_bInitOK		= FALSE;
	m_dwNPCID		= dwNPCID;
	m_pShopProto	= pShopProto;

	ZeroMemory(m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	m_init_time = g_world.GetWorldTime().day;
}

Shop::~Shop()
{
}

//-----------------------------------------------------------------------------
// 创建商店
//-----------------------------------------------------------------------------
Shop* Shop::Create(DWORD dwNPCID, DWORD dwShopID)
{
	const tagShopProto *pShopProto = AttRes::GetInstance()->GetShopProto(dwShopID);
	if(!VALID_POINT(pShopProto))
	{
		ASSERT(VALID_POINT(pShopProto));
		return NULL;
	}

	Shop *pShop = NULL;

	if (pShopProto->bClanTreasury)
	{
		if (pShopProto->bEquip)
		{
			pShop = new EquipClanTreasury(dwNPCID, pShopProto);
		}
		else
		{
			pShop = new ItemClanTreasury(dwNPCID, pShopProto);
		}
	}
	else
	{
		if(pShopProto->bEquip)
		{
			pShop = new EquipShop(dwNPCID, pShopProto);
		}
		else
		{
			pShop = new ItemShop(dwNPCID, pShopProto);
		}
	}

	
	if(!pShop->is_init_ok())
	{
		ASSERT(pShop->is_init_ok());
		SAFE_DELETE(pShop);
		return NULL;
	}

	return pShop;
}

//-----------------------------------------------------------------------------
// 检查特殊消耗
//-----------------------------------------------------------------------------
DWORD Shop::CheckSpecCost(Role *pRole, ECurCostType eCostType, DWORD nSpecCost, 
			  DWORD dwCostItemID, BYTE byRepLevelLimit, OUT package_list<tagItem*> &listItem)
{
	ASSERT(VALID_POINT(pRole));
	
	if(nSpecCost < 0)
	{
		ASSERT(nSpecCost >= 0);
		return INVALID_VALUE;
	}
	
	listItem.clear();
	
	switch(eCostType)
	{
	case ECCT_Null:			// 没有特殊消耗
		break;
	case ECCT_BagSilver:	// 消耗背包金钱有单独字段
		if(!pRole->GetCurMgr().IsEnough(eCostType, nSpecCost))
		{
			return E_Shop_NotEnough_SpecCost;
		}
		break;
	case ECCT_ItemExchange:	// 物品兑换
		if(pRole->GetItemMgr().GetBagSameItemList(listItem, dwCostItemID, nSpecCost) < nSpecCost)
		{
			return E_Item_NotEnough;
		}
		break;
	case ECCT_BaiBaoYuanBao:	// 元宝
		{
			if(!pRole->GetCurMgr().IsEnough(eCostType, nSpecCost))
			{
				return E_Shop_NotEnough_SpecCost;
			}
			break;
		}
	case ECCT_GuildContribe:	// 帮贡
		{
			guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
			if(VALID_POINT(pGuild))
			{
				tagGuildMember* pMember = pGuild->get_member(pRole->GetID());
				if(VALID_POINT(pMember))
				{
					if(pMember->nContribution < nSpecCost)
						return E_Shop_NotEnough_SpecCost;
				}
				else
				{
					return E_Shop_NotEnough_SpecCost;
				}
			}
			else
			{
				return E_Shop_NotEnough_SpecCost;
			}
			break;
		}
	case ECCT_Exploits:
		{
			if(!pRole->GetCurMgr().IsEnough(eCostType, nSpecCost))
			{
				return E_Shop_NotEnough_SpecCost;
			}

			if(pRole->get_shop_exploits() > SHOP_EXPLOITS_LIMIT)
			{
				return E_Shop_Exploits_Limit;
			}
			break;
		}
	default:				// 声望等其他类型
		ECLanType eClanType = MTRANS_ECCT2ECLT(eCostType);
		if( JDG_VALID(ECLT, eClanType) && byRepLevelLimit > pRole->GetClanData().RepGetLvl(eClanType))
		{
			return E_Shop_RepLevel_Low;
		}

		// 特殊消耗品数量 -- 使用通用接口(策划调整后，程序不用修改)
		if(!pRole->GetCurMgr().IsEnough(eCostType, nSpecCost))
		{
			return E_Shop_NotEnough_SpecCost;
		}
		break;
	}
	
	return E_Success;
}


//*********************************** ItemShop **********************************

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
ItemShop::ItemShop(DWORD dwNPCID, const tagShopProto *pShopProto)
					: Shop(dwNPCID, pShopProto)
{
	m_bInitOK = Init(pShopProto);
}

ItemShop::~ItemShop()
{
}

//-----------------------------------------------------------------------------
// 初始化
//-----------------------------------------------------------------------------
BOOL ItemShop::Init(const tagShopProto *pShopProto)
{
	ASSERT(VALID_POINT(pShopProto));
	ASSERT(!pShopProto->bEquip);

	// 初始化货架
	for(INT32 n=0; n<MAX_SHOP_SHELF_NUM; ++n)
	{
		m_Shelf[n].Init(EICT_Shop, pShopProto->n16Num[n], pShopProto->n16Num[n]);
	}

	// 记录各页商品个数
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];
	ZeroMemory(n16ItemNum, sizeof(n16ItemNum));

	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();
	
	// 初始化普通物品
	INT32 i,j;
	const tagShopItemProto *pItemProto = NULL;
	for(i=0; i<MAX_SHOP_COMMON_ITEM; ++i)
	{
		pItemProto = &pShopProto->Item[i];

		if(INVALID_VALUE == pItemProto->dw_data_id)
		{
			break;
		}
		
		tagShopItem *pItem = new tagShopItem;
		if(!VALID_POINT(pItem))
		{
			ASSERT(VALID_POINT(pItem));
			return FALSE;
		}

		pItem->dw_data_id			= pItemProto->dw_data_id;
		pItem->n16RemainNum		= INVALID_VALUE;
		pItem->n16Index			= INVALID_VALUE;
		pItem->byProtoIndex		= i;
		pItem->dwRefreshTime	= dwCurTime;
		pItem->pProto			= pItemProto;

		// 放到所属货架
		ASSERT(pItemProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pItemProto->byShelf].Add(pItem, n16ItemNum[pItemProto->byShelf]++);
	}

	// 初始化限量物品起始下标
	get_fast_code()->memory_copy(m_n16RareIndexStart, n16ItemNum, sizeof(m_n16RareIndexStart));

	// 初始化限量物品
	package_map<INT, tagShopRareItemProto*> map_rare;
	tagShopRareItemProto* pRareProto = NULL;
	for(j = 0; j < MAX_SHOP_RARE_ITEM; j++)
	{
		pRareProto = const_cast<tagShopRareItemProto*>(&pShopProto->RareItem[j]);
		if(INVALID_VALUE == pRareProto->dw_data_id)
		{
			continue;
		}

		map_rare.add(j, pRareProto);
	}

	for(INT k = 0; k < pShopProto->n16Num[0]; k++)
	{
		INT nIndex = 0;
		tagShopRareItemProto* pRareProto = NULL;
		map_rare.rand_find(nIndex, pRareProto);

		if(!VALID_POINT(pRareProto))
			break;
	
		tagShopItem *pItem = new tagShopItem;
		if(!VALID_POINT(pItem))
		{
			ASSERT(VALID_POINT(pItem));
			return FALSE;
		}

		pItem->dw_data_id			= pRareProto->dw_data_id;
		pItem->n16RemainNum		= pRareProto->byRefreshNum;
		pItem->n16Index			= INVALID_VALUE;
		pItem->byProtoIndex		= nIndex;
		pItem->dwRefreshTime	= dwCurTime;
		pItem->pRareProto		= pRareProto;

		// 放到所属货架
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pItem, n16ItemNum[pRareProto->byShelf]++);

		map_rare.erase(nIndex);
	}
	//const tagShopRareItemProto *pRareProto = NULL;
	//for(j=0; j<MAX_SHOP_RARE_ITEM; ++i, ++j)
	//{
	//	pRareProto = &pShopProto->RareItem[j];

	//	if(INVALID_VALUE == pRareProto->dw_data_id)
	//	{
	//		break;
	//	}

	//	tagShopItem *pItem = new tagShopItem;
	//	if(!VALID_POINT(pItem))
	//	{
	//		ASSERT(VALID_POINT(pItem));
	//		return FALSE;
	//	}
	//	
	//	pItem->dw_data_id			= pRareProto->dw_data_id;
	//	pItem->n16RemainNum		= pRareProto->byRefreshNum;
	//	pItem->n16Index			= INVALID_VALUE;
	//	pItem->byProtoIndex		= j;
	//	pItem->dwRefreshTime	= dwCurTime;
	//	pItem->pRareProto		= pRareProto;

	//	// 放到所属货架
	//	ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
	//	m_Shelf[pRareProto->byShelf].Add(pItem, n16ItemNum[pRareProto->byShelf]++);
	//}

	// 检查商店资源是否有问题
	for(INT32 n=0; n<MAX_SHOP_SHELF_NUM; ++n)
	{
		if(pShopProto->n16Num[n] != n16ItemNum[n])
		{
			ASSERT(0);
			print_message(_T("Shop proto mybe has problem, please check<shopid:%u, shelf:%d>\n"), pShopProto->dwID, n);
			return FALSE;
		}
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
VOID ItemShop::Update()
{
	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();

	// 记录各页商品下标
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	if(m_pShopProto->eCostType == ECCT_Exploits)
	{
		if(m_init_time != g_world.GetWorldTime().day)
		{
			for(INT16 i = 0; i < m_Shelf[0].GetCurSpaceSize(); i++)
			{
				tagShopItem* pShopItem = m_Shelf->Remove(i);
				SAFE_DELETE(pShopItem);
			}

			package_map<INT, tagShopRareItemProto*> map_rare;
			tagShopRareItemProto* pRareProto = NULL;
			for(INT j = 0; j < MAX_SHOP_RARE_ITEM; j++)
			{
				pRareProto = const_cast<tagShopRareItemProto*>(&m_pShopProto->RareItem[j]);
				if(INVALID_VALUE == pRareProto->dw_data_id)
				{
					continue;
				}

				map_rare.add(j, pRareProto);
			}

			for(INT k = 0; k < m_pShopProto->n16Num[0]; k++)
			{
				INT nIndex = 0;
				tagShopRareItemProto* pRareProto = NULL;
				map_rare.rand_find(nIndex, pRareProto);

				if(!VALID_POINT(pRareProto))
					return;

				tagShopItem *pItem = new tagShopItem;
				if(!VALID_POINT(pItem))
				{
					ASSERT(VALID_POINT(pItem));
					return;
				}

				pItem->dw_data_id			= pRareProto->dw_data_id;
				pItem->n16RemainNum		= pRareProto->byRefreshNum;
				pItem->n16Index			= INVALID_VALUE;
				pItem->byProtoIndex		= nIndex;
				pItem->dwRefreshTime	= dwCurTime;
				pItem->pRareProto		= pRareProto;

				// 放到所属货架
				ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
				m_Shelf[pRareProto->byShelf].Add(pItem, n16ItemNum[pRareProto->byShelf]++);

				map_rare.erase(nIndex);
			}

			m_init_time = g_world.GetWorldTime().day;
		}
	}

	// 检查限量物品是否需要更新
	/*tagShopItem *pItem = NULL;
	const tagShopRareItemProto *pRareProto = NULL;
	for(INT32 i=0; i<MAX_SHOP_RARE_ITEM; ++i)
	{
		pRareProto = &m_pShopProto->RareItem[i];
		if(INVALID_VALUE == pRareProto->dw_data_id)
		{
			break;
		}

		pItem = m_Shelf[pRareProto->byShelf].GetItem(n16ItemNum[pRareProto->byShelf]++);
		ASSERT(VALID_POINT(pItem) && pItem->dw_data_id == pRareProto->dw_data_id);

		if(CalcTimeDiff(dwCurTime, pItem->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			pItem->n16RemainNum	= pRareProto->byRefreshNum;
			pItem->dwRefreshTime = dwCurTime;
		}
	}*/
}

//-----------------------------------------------------------------------------
// 获取刷新物品
//-----------------------------------------------------------------------------
VOID ItemShop::GetRareItems(OUT tagMsgShopItem* pRareItems, INT16 n16RareItemNum, BYTE byShelf)
{
	ASSERT(GetRareGoodsNum(byShelf) == n16RareItemNum);

	tagShopItem *pShopItem = NULL;
	for(INT16 i=0; i<n16RareItemNum; ++i)
	{
		pShopItem = m_Shelf[byShelf].GetItem((INT16)(m_n16RareIndexStart[byShelf] + i));
		ASSERT(VALID_POINT(pShopItem));

		pRareItems[i].dw_data_id		= pShopItem->dw_data_id;
		pRareItems[i].n16RemainNum	= pShopItem->n16RemainNum;
		pRareItems[i].n16Index		= pShopItem->n16Index;
		pRareItems[i].byProtoIndex	= pShopItem->byProtoIndex;
	}
}

//-----------------------------------------------------------------------------
// 出售物品
//-----------------------------------------------------------------------------
DWORD ItemShop::sell_goods(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT16 n16BuyNum, OUT tagSellItem &sell_goods)
{
	ASSERT(VALID_POINT(m_pShopProto));
	ASSERT(VALID_POINT(pRole));

	// 查找物品
	tagShopItem *pShopItem = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopItem))
	{
		// 执行到这里，意味着消息中的dw_data_id非法
		ASSERT(VALID_POINT(pShopItem));
		return E_Shop_ItemNotFind;
	}

	// 检查物品个数
	if(pShopItem->n16RemainNum != INVALID_VALUE && pShopItem->n16RemainNum < n16BuyNum)
	{
		sell_goods.n16RemainNum = pShopItem->n16RemainNum;
		return E_Shop_ItemNotEnough;
	}

	if(m_pShopProto->eCostType == ECCT_Exploits && n16BuyNum > 1)
	{
		return E_Shop_ItemNotEnough;
	}

	// 背包中金钱 -- 不使用通用接口
	INT nSilverCost = pShopItem->pProto->nSilver * n16BuyNum;
	if(nSilverCost > pRole->GetCurMgr().GetBagSilver())
	{
		return E_Shop_NotEnough_SilverInBag;
	}

	// 特殊消耗品数量 -- 使用通用接口(策划调整后，程序不用修改)
	DWORD nSpecCost = pShopItem->pProto->nCostNum * n16BuyNum;
	DWORD dw_error_code = CheckSpecCost(pRole, m_pShopProto->eCostType, nSpecCost, 
		m_pShopProto->dwItemTypeID, pShopItem->pProto->byRepLevel, sell_goods.listItem);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 创建物品
	tagItem *pItemSell = ItemCreator::Create(EICM_ShopNPC, m_dwNPCID, dw_data_id, n16BuyNum, m_pShopProto->bBind);
	if(!VALID_POINT(pItemSell))
	{
		ASSERT(VALID_POINT(pItemSell));
		return E_Shop_CreateItem_Failed;
	}

	if(m_pShopProto->eCostType == ECCT_Exploits)
	{
		pRole->inc_shop_exploits();
	}

	// 更新商店中物品个数
	/*if(pShopItem->n16RemainNum != INVALID_VALUE)
	{
		pShopItem->n16RemainNum -= n16BuyNum;
	}*/

	// 设置传出参数
	sell_goods.pItemSell		= pItemSell;
	sell_goods.n16RemainNum	= pShopItem->n16RemainNum;
	sell_goods.nSilverCost	= nSilverCost;
	sell_goods.eCostType		= m_pShopProto->eCostType;
	sell_goods.nSpecCost		= nSpecCost;
	sell_goods.byIndex		= (BYTE)pShopItem->n16Index;

	return E_Success;
}


//*********************************** EquipShop **********************************

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
EquipShop::EquipShop(DWORD dwNPCID, const tagShopProto *pShopProto)
					: Shop(dwNPCID, pShopProto)
{
	m_bInitOK = Init(pShopProto);
}

EquipShop::~EquipShop()
{
	// 删除已生成的装备
	tagShopEquip *pShopEquip = NULL;
	for(INT32 i=0; i<MAX_SHOP_SHELF_NUM; ++i)
	{
		for(INT16 j=0; j<m_pShopProto->n16Num[i]; ++j)
		{
			pShopEquip = m_Shelf[i].GetItem(j);
			if(!VALID_POINT(pShopEquip))
			{
				break;
			}

			SAFE_DELETE(pShopEquip->pEquip);
		}
	}
}

//-----------------------------------------------------------------------------
// 初始化
//-----------------------------------------------------------------------------
BOOL EquipShop::Init(const tagShopProto *pShopProto)
{
	ASSERT(VALID_POINT(pShopProto));
	ASSERT(pShopProto->bEquip);

	// 初始化货架
	for(INT32 n=0; n<MAX_SHOP_SHELF_NUM; ++n)
	{
		m_Shelf[n].Init(EICT_Shop, pShopProto->n16Num[n], pShopProto->n16Num[n]);
	}

	// 记录各页商品个数
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];
	ZeroMemory(n16ItemNum, sizeof(n16ItemNum));

	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();

	// 初始化普通物品
	INT32 i,j;
	const tagShopItemProto *pEquipProto = NULL;
	for(i=0; i<MAX_SHOP_COMMON_ITEM; ++i)
	{
		pEquipProto = &pShopProto->Item[i];
		if(INVALID_VALUE == pEquipProto->dw_data_id)
		{
			break;
		}

		tagShopEquip *pEquip = new tagShopEquip;
		if(!VALID_POINT(pEquip))
		{
			ASSERT(VALID_POINT(pEquip));
			return FALSE;
		}


		pEquip->n16RemainNum	= INVALID_VALUE;
		pEquip->n16Index		= INVALID_VALUE;
		pEquip->byProtoIndex	= i;
		pEquip->dwRefreshTime	= dwCurTime;
		pEquip->pProto			= pEquipProto;
		pEquip->pEquip			= NULL;

		// 是否是装备
		if(!MIsEquipment(pEquipProto->dw_data_id))
		{
			return FALSE;
		}

		// 生成装备
		pEquip->pEquip = (tagEquip*)ItemCreator::Create(EICM_ShopNPC, m_dwNPCID, pEquipProto->dw_data_id);
		if(!VALID_POINT(pEquip->pEquip))
		{
			ASSERT(VALID_POINT(pEquip->pEquip));
			return FALSE;
		}

		// 鉴定为白色装备
		ItemCreator::IdentifyEquip(pEquip->pEquip, EIQ_Quality0);

		// 放到所属货架
		ASSERT(pEquipProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pEquipProto->byShelf].Add(pEquip, n16ItemNum[pEquipProto->byShelf]++);
	}

	// 初始化限量物品起始下标
	get_fast_code()->memory_copy(m_n16RareIndexStart, n16ItemNum, sizeof(m_n16RareIndexStart));

	// 初始化限量物品
	const tagShopRareItemProto *pRareProto = NULL;
	for(j=0; j<MAX_SHOP_RARE_ITEM; ++i, ++j)
	{
		pRareProto = &pShopProto->RareItem[j];
		if(INVALID_VALUE == pRareProto->dw_data_id)
		{
			break;
		}

		tagShopEquip *pEquip = new tagShopEquip;
		if(!VALID_POINT(pEquip))
		{
			ASSERT(VALID_POINT(pEquip));
			return FALSE;
		}

		pEquip->n16RemainNum	= pRareProto->byRefreshNum;
		pEquip->n16Index		= INVALID_VALUE;
		pEquip->byProtoIndex	= j;
		pEquip->dwRefreshTime	= dwCurTime;
		pEquip->pRareProto		= pRareProto;
		pEquip->pEquip			= NULL;

		// 是否是装备
		if(!MIsEquipment(pRareProto->dw_data_id))
		{
			return FALSE;
		}

		// 生成装备
		pEquip->pEquip = (tagEquip*)ItemCreator::Create(EICM_ShopNPC, m_dwNPCID, pRareProto->dw_data_id);
		if(!VALID_POINT(pEquip->pEquip))
		{
			ASSERT(VALID_POINT(pEquip->pEquip));
			return FALSE;
		}

		// 鉴定为指定品级装备
		ItemCreator::IdentifyEquip(pEquip->pEquip, (EItemQuality)pRareProto->byQuality);
		
		// 放到所属货架
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pEquip, n16ItemNum[pRareProto->byShelf]++);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
VOID EquipShop::Update()
{
	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();

	// 记录各页商品下标
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	// 检查限量物品是否需要更新
	tagShopEquip *pEquip = NULL;
	const tagShopRareItemProto *pRareProto = NULL;
	for(INT32 i=0; i<MAX_SHOP_RARE_ITEM; ++i)
	{
		pRareProto = &m_pShopProto->RareItem[i];
		if(INVALID_VALUE == pRareProto->dw_data_id)
		{
			break;
		}

		pEquip = m_Shelf[pRareProto->byShelf].GetItem(n16ItemNum[pRareProto->byShelf]++);
		ASSERT(VALID_POINT(pEquip));

		if(CalcTimeDiff(dwCurTime, pEquip->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			if(VALID_POINT(pEquip->pEquip))
			{
				ASSERT(pEquip->pEquip->dw_data_id == pRareProto->dw_data_id);

				SAFE_DELETE(pEquip->pEquip);
			}

			// 生成装备
			pEquip->pEquip = (tagEquip*)ItemCreator::Create(EICM_ShopNPC, m_dwNPCID, pRareProto->dw_data_id);
			//ASSERT(VALID_POINT(pEquip->pEquip));

			// 鉴定为指定品级装备
			ItemCreator::IdentifyEquip(pEquip->pEquip, (EItemQuality)pRareProto->byQuality);
			
			pEquip->n16RemainNum	= pRareProto->byRefreshNum;
			pEquip->dwRefreshTime	= dwCurTime;
		}
	}
}

//-----------------------------------------------------------------------------
// 获取所有装备
//-----------------------------------------------------------------------------
VOID EquipShop::GetRareEquips(OUT tagMsgShopEquip* pEquips, BYTE byShelf, INT16 &n16RareEquipNum)
{
	ASSERT(GetRareGoodsNum(byShelf) == n16RareEquipNum);

	tagShopEquip *pShopEquip = NULL;
	INT16 n16RealNum = 0;
	for(INT16 i=0; i<n16RareEquipNum; ++i)
	{
		pShopEquip = m_Shelf[byShelf].GetItem((INT16)(i + m_n16RareIndexStart[byShelf]));
		ASSERT(VALID_POINT(pShopEquip));

		if(VALID_POINT(pShopEquip->pEquip))
		{
			pEquips[n16RealNum].n16RemainNum		= pShopEquip->n16RemainNum;
			pEquips[n16RealNum].n16Index			= pShopEquip->n16Index;
			pEquips[n16RealNum].byProtoIndex		= pShopEquip->byProtoIndex;
			
			get_fast_code()->memory_copy(&pEquips[n16RealNum].Equip, pShopEquip->pEquip, SIZE_EQUIP);
			++n16RealNum;
		}
	}

	n16RareEquipNum = n16RealNum;
}

//-----------------------------------------------------------------------------
// 出售装备
//-----------------------------------------------------------------------------
DWORD EquipShop::SellEquip(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial, OUT tagSellEquip &SellEquip)
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(VALID_POINT(m_pShopProto));
	
	// 查找装备
	tagShopEquip *pShopEquip = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopEquip))
	{
		// 执行到这里，意味着消息中的dw_data_id非法
		ASSERT(VALID_POINT(pShopEquip));
		return E_Shop_ItemNotFind;
	}

	// 如果是稀有装备检查指定装备是否已经被出售
	if(pShopEquip->n16RemainNum >= 0)
	{
		if(!VALID_POINT(pShopEquip->pEquip) || pShopEquip->pEquip->n64_serial != n64_serial)
		{
			// 设置传出参数
			SellEquip.pEquipNew		= pShopEquip->pEquip;
			SellEquip.n16RemainNum	= pShopEquip->n16RemainNum;

			return E_Shop_Equip_Sold;
		}
	}

	// 背包中金钱
	if(pShopEquip->pProto->nSilver > pRole->GetCurMgr().GetBagSilver())
	{
		return E_Shop_NotEnough_SilverInBag;
	}
	
	// 声望阶段是否满足
	DWORD dw_error_code = CheckSpecCost(pRole, m_pShopProto->eCostType, 
									pShopEquip->pProto->nCostNum, m_pShopProto->dwItemTypeID, 
									pShopEquip->pProto->byRepLevel, SellEquip.listItem);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 检查64位id是否合法，非法的话，则重新生成一个
	if(pShopEquip->pEquip->n64_serial < MIN_ITEM_SERIAL_INTERNAL)
	{
		ItemCreator::CreateItemSerial(pShopEquip->pEquip->n64_serial);
	}
	
	// 设置传出参数
	SellEquip.pEquipSell	= pShopEquip->pEquip;
	SellEquip.nSilverCost	= pShopEquip->pProto->nSilver;
	SellEquip.eCostType		= m_pShopProto->eCostType;
	SellEquip.nSpecCost		= pShopEquip->pProto->nCostNum;

	// 更新商店中物品个数
	if(pShopEquip->n16RemainNum != INVALID_VALUE)
	{
		--pShopEquip->n16RemainNum;
	}

	// 创建新装备
	if(pShopEquip->n16RemainNum != 0)
	{
		pShopEquip->pEquip = (tagEquip*)ItemCreator::Create(EICM_ShopNPC, m_dwNPCID, dw_data_id, 1, m_pShopProto->bBind);
		if(!VALID_POINT(pShopEquip->pEquip))
		{
			// 执行到这里,应该是内存申请失败.(原型肯定可以找到)
			ASSERT(VALID_POINT(pShopEquip->pEquip));
			// return E_Shop_CreateEquip_Failed;
			return E_Success;
		}

		// 鉴定为指定品级装备
		ItemCreator::IdentifyEquip(pShopEquip->pEquip, (EItemQuality)SellEquip.pEquipSell->equipSpec.byQuality, m_pShopProto->bRandAtt);
	}
	else
	{
		pShopEquip->pEquip = NULL;
	}

	// 设置传出参数
	SellEquip.pEquipNew		= pShopEquip->pEquip;
	SellEquip.n16RemainNum	= pShopEquip->n16RemainNum;
	SellEquip.byIndex		= (BYTE)pShopEquip->n16Index;

	return E_Success;
}