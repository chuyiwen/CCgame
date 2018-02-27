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
#include "clan_treasury.h"
#include "role.h"
#include "currency.h"
#include "item_creator.h"

#include "../../common/WorldDefine/shop_protocol.h"

//*********************************** ItemClanTreasury **********************************

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
ItemClanTreasury::ItemClanTreasury(DWORD dwNPCID, const tagShopProto *pShopProto)
: Shop(dwNPCID, pShopProto)
{
	ZeroMemory(m_n16ActNum, sizeof(m_n16ActNum));
	ZeroMemory(m_n16ActRareNum, sizeof(m_n16ActRareNum));
	m_bInitOK = Init(pShopProto);
}

ItemClanTreasury::~ItemClanTreasury()
{
}

//-----------------------------------------------------------------------------
// 初始化
//-----------------------------------------------------------------------------
BOOL ItemClanTreasury::Init(const tagShopProto *pShopProto)
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

		tagTreasuryItem *pItem = new tagTreasuryItem;
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
		//是否被激活
		pItem->SetAct(INVALID_VALUE);

		// 放到所属货架
		ASSERT(pItemProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pItemProto->byShelf].Add(pItem, n16ItemNum[pItemProto->byShelf]++);
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

		tagTreasuryItem *pItem = new tagTreasuryItem;
		if(!VALID_POINT(pItem))
		{
			ASSERT(VALID_POINT(pItem));
			return FALSE;
		}

		pItem->dw_data_id			= pRareProto->dw_data_id;
		pItem->n16RemainNum		= pRareProto->byRefreshNum;
		pItem->n16Index			= INVALID_VALUE;
		pItem->byProtoIndex		= j;
		pItem->dwRefreshTime	= dwCurTime;
		pItem->pRareProto		= pRareProto;
		//是否被激活
		pItem->SetAct(INVALID_VALUE);

		// 放到所属货架
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pItem, n16ItemNum[pRareProto->byShelf]++);
	}

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
VOID ItemClanTreasury::Update()
{
	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();

	// 记录各页商品下标
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	// 检查限量物品是否需要更新
	tagTreasuryItem *pItem = NULL;
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

		// 未激活的不更新
		if (!pItem->IsAct()) continue;

		if(CalcTimeDiff(dwCurTime, pItem->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			pItem->n16RemainNum	= pRareProto->byRefreshNum;
			pItem->dwRefreshTime = dwCurTime;
		}
	}
}

//-----------------------------------------------------------------------------
// 获取刷新物品
//-----------------------------------------------------------------------------
VOID ItemClanTreasury::GetRareItems(OUT tagMsgShopItem* pRareItems, INT16 n16RareItemNum, BYTE byShelf)
{
	ASSERT(GetRareGoodsNum(byShelf) == n16RareItemNum);

	tagTreasuryItem *pShopItem = NULL;
	for(INT16 i=m_n16RareIndexStart[byShelf]; i<m_Shelf[byShelf].GetCurSpaceSize(); ++i)
	{
		pShopItem = m_Shelf[byShelf].GetItem(i);
		ASSERT(VALID_POINT(pShopItem));
		
		// 未激活的不发送
		if (!pShopItem->IsAct()) continue;

		pRareItems[i].dw_data_id		= pShopItem->dw_data_id;
		pRareItems[i].n16RemainNum	= pShopItem->n16RemainNum;
		pRareItems[i].n16Index		= pShopItem->n16Index;
		pRareItems[i].byProtoIndex	= pShopItem->byProtoIndex;
	}
}

//-----------------------------------------------------------------------------
// 出售物品
//-----------------------------------------------------------------------------
DWORD ItemClanTreasury::sell_goods(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT16 n16BuyNum, OUT tagSellItem &sell_goods)
{
	ASSERT(VALID_POINT(m_pShopProto));
	ASSERT(VALID_POINT(pRole));

	// 查找物品
	tagTreasuryItem *pShopItem = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopItem))
	{
		// 执行到这里，意味着消息中的dw_data_id非法
		ASSERT(VALID_POINT(pShopItem));
		return E_Shop_ItemNotFind;
	}

	// 是否激活
	if(!pShopItem->IsAct())
	{
		return E_Shop_ItemNotActived;
	}

	// 检查物品个数
	if(pShopItem->n16RemainNum != INVALID_VALUE && pShopItem->n16RemainNum < n16BuyNum)
	{
		sell_goods.n16RemainNum = pShopItem->n16RemainNum;
		return E_Shop_ItemNotEnough;
	}

	// 背包中金钱 -- 不使用通用接口
	INT nSilverCost = pShopItem->pProto->nSilver * n16BuyNum;
	if(nSilverCost > pRole->GetCurMgr().GetBagSilver())
	{
		return E_Shop_NotEnough_SilverInBag;
	}

	// 特殊消耗品数量 -- 使用通用接口(策划调整后，程序不用修改)
	INT nSpecCost = pShopItem->pProto->nCostNum * n16BuyNum;
	DWORD dw_error_code = CheckSpecCost(pRole, m_pShopProto->eCostType, nSpecCost, 
		m_pShopProto->dwItemTypeID, pShopItem->pProto->byRepLevel, sell_goods.listItem);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// 创建物品
	ASSERT(pShopItem->IsAct());
	tagItem *pItemSell = ItemCreator::CreateTreasure(pShopItem->dwNameID, EICM_ShopNPC, m_dwNPCID, dw_data_id, n16BuyNum);
	if(!VALID_POINT(pItemSell))
	{
		ASSERT(VALID_POINT(pItemSell));
		return E_Shop_CreateItem_Failed;
	}

	// 更新商店中物品个数
	if(pShopItem->n16RemainNum != INVALID_VALUE)
	{
		pShopItem->n16RemainNum -= n16BuyNum;
	}

	// 设置传出参数
	sell_goods.pItemSell		= pItemSell;
	sell_goods.n16RemainNum	= pShopItem->n16RemainNum;
	sell_goods.nSilverCost	= nSilverCost;
	sell_goods.eCostType		= m_pShopProto->eCostType;
	sell_goods.nSpecCost		= nSpecCost;
	sell_goods.byIndex		= (BYTE)pShopItem->n16Index;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 激活珍宝
//-----------------------------------------------------------------------------
BOOL ItemClanTreasury::ActiveTreasure( DWORD dw_data_id, DWORD dwNameID )
{
	ASSERT(VALID_VALUE(dwNameID));

	tagTreasuryItem* pToAct = NULL;
	for (INT i=0; i<MAX_SHOP_SHELF_NUM; ++i)
	{
		pToAct = m_Shelf[i].GetItem(dw_data_id);
		if (VALID_POINT(pToAct))
		{
			pToAct->SetAct(dwNameID);
			m_n16ActNum[i]++;
			if (VALID_VALUE(pToAct->n16RemainNum))
			{
				// 稀有珍宝
				m_n16ActRareNum[i]++;
			}
			return TRUE;
		}
	}
	// 资源有问题
	ASSERT(0);
	return FALSE;
}

//*********************************** EquipClanTreasury **********************************

//-----------------------------------------------------------------------------
// 构造&析构函数
//-----------------------------------------------------------------------------
EquipClanTreasury::EquipClanTreasury(DWORD dwNPCID, const tagShopProto *pShopProto)
: Shop(dwNPCID, pShopProto)
{
	ZeroMemory(m_n16ActNum, sizeof(m_n16ActNum));
	ZeroMemory(m_n16ActRareNum, sizeof(m_n16ActRareNum));
	m_bInitOK = Init(pShopProto);
}

EquipClanTreasury::~EquipClanTreasury()
{
	// 删除已生成的装备
	tagTreasuryEquip *pShopEquip = NULL;
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
BOOL EquipClanTreasury::Init(const tagShopProto *pShopProto)
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

		tagTreasuryEquip *pEquip = new tagTreasuryEquip;
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
		
		pEquip->SetAct(INVALID_VALUE);


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

		tagTreasuryEquip *pEquip = new tagTreasuryEquip;
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
		
		pEquip->SetAct(INVALID_VALUE);

		// 放到所属货架
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pEquip, n16ItemNum[pRareProto->byShelf]++);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
VOID EquipClanTreasury::Update()
{
	// 获取当前时间
	DWORD dwCurTime = g_world.GetWorldTime();

	// 记录各页商品下标
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	// 检查限量物品是否需要更新
	tagTreasuryEquip *pEquip = NULL;
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

		ASSERT(VALID_VALUE(pEquip->n16RemainNum));

		// 未激活的不更新
		if (!pEquip->IsAct()) continue;
		
		if(CalcTimeDiff(dwCurTime, pEquip->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			if(VALID_POINT(pEquip->pEquip))
			{
				ASSERT(pEquip->pEquip->dw_data_id == pRareProto->dw_data_id);

				SAFE_DELETE(pEquip->pEquip);
			}

			// 生成装备
			ASSERT(pEquip->IsAct());
			pEquip->pEquip = (tagEquip*)ItemCreator::CreateTreasureEx(pEquip->dwNameID, EICM_ShopNPC, m_dwNPCID, pRareProto->dw_data_id, 1, (EItemQuality)pRareProto->byQuality);

			pEquip->n16RemainNum	= pRareProto->byRefreshNum;
			pEquip->dwRefreshTime	= dwCurTime;
		}
	}
}

//-----------------------------------------------------------------------------
// 获取所有装备
//-----------------------------------------------------------------------------
VOID EquipClanTreasury::GetRareEquips(OUT tagMsgShopEquip* pEquips, BYTE byShelf, INT16 &n16RareEquipNum)
{
	tagTreasuryEquip *pShopEquip = NULL;
	INT16 n16RealNum = 0;
	for(INT16 i=m_n16RareIndexStart[byShelf]; i< m_Shelf[byShelf].GetCurSpaceSize(); ++i)
	{
		pShopEquip = m_Shelf[byShelf].GetItem((INT16)(i));
		ASSERT(VALID_POINT(pShopEquip));

		if(	pShopEquip->IsAct() &&
			VALID_POINT(pShopEquip->pEquip))
		{
			pEquips[n16RealNum].n16RemainNum		= pShopEquip->n16RemainNum;
			pEquips[n16RealNum].n16Index			= pShopEquip->n16Index;
			pEquips[n16RealNum].byProtoIndex		= pShopEquip->byProtoIndex;

			get_fast_code()->memory_copy(&pEquips[n16RealNum].Equip, pShopEquip->pEquip, SIZE_EQUIP);
			++n16RealNum;
		}
	}
	ASSERT(n16RealNum == GetRareGoodsNum(byShelf));
	n16RareEquipNum = n16RealNum;
}

//-----------------------------------------------------------------------------
// 出售装备
//-----------------------------------------------------------------------------
DWORD EquipClanTreasury::SellEquip(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial, OUT tagSellEquip &SellEquip)
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(VALID_POINT(m_pShopProto));

	// 查找装备
	tagTreasuryEquip *pShopEquip = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopEquip))
	{
		// 执行到这里，意味着消息中的dw_data_id非法
		ASSERT(VALID_POINT(pShopEquip));
		return E_Shop_ItemNotFind;
	}

	// 是否被激活
	if (!pShopEquip->IsAct())
	{
		return E_Shop_ItemNotActived;
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
		ASSERT(pShopEquip->IsAct());
		pShopEquip->pEquip = (tagEquip*)ItemCreator::CreateTreasureEx(	pShopEquip->dwNameID, EICM_ShopNPC, m_dwNPCID, dw_data_id, 
																		1, (EItemQuality)SellEquip.pEquipSell->equipSpec.byQuality);
		if(!VALID_POINT(pShopEquip->pEquip))
		{
			// 执行到这里,应该是内存申请失败.(原型肯定可以找到)
			ASSERT(VALID_POINT(pShopEquip->pEquip));
			// return E_Shop_CreateEquip_Failed;
			return E_Success;
		}
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

//-----------------------------------------------------------------------------
// 激活珍宝
//-----------------------------------------------------------------------------
BOOL EquipClanTreasury::ActiveTreasure( DWORD dw_data_id, DWORD dwNameID )
{
	ASSERT(VALID_VALUE(dwNameID));
	BOOL bRtv = FALSE;
	tagTreasuryEquip* pToAct = NULL;

	for (INT i=0; i<MAX_SHOP_SHELF_NUM; ++i)
	{
		pToAct = m_Shelf->GetItem(dw_data_id);
		if (VALID_POINT(pToAct))
		{
			pToAct->SetAct(dwNameID);
			
			EItemQuality eQuality = EIQ_Quality0;
			if (VALID_VALUE(pToAct->n16RemainNum))
			{
				eQuality = (EItemQuality)pToAct->pRareProto->byQuality;
				m_n16ActRareNum[i]++;
			}
			m_n16ActNum[i]++;

			ASSERT(!VALID_POINT(pToAct->pEquip));
			pToAct->pEquip = (tagEquip *)ItemCreator::CreateTreasureEx(dwNameID, EICM_ShopNPC, m_dwNPCID, dw_data_id, 1, eQuality);
			
			bRtv = TRUE;
			break;
		}
	}
	return bRtv;
}