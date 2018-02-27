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
// ����&��������
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
// ��ʼ��
//-----------------------------------------------------------------------------
BOOL ItemClanTreasury::Init(const tagShopProto *pShopProto)
{
	ASSERT(VALID_POINT(pShopProto));
	ASSERT(!pShopProto->bEquip);

	// ��ʼ������
	for(INT32 n=0; n<MAX_SHOP_SHELF_NUM; ++n)
	{
		m_Shelf[n].Init(EICT_Shop, pShopProto->n16Num[n], pShopProto->n16Num[n]);
	}

	// ��¼��ҳ��Ʒ����
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];
	ZeroMemory(n16ItemNum, sizeof(n16ItemNum));

	// ��ȡ��ǰʱ��
	DWORD dwCurTime = g_world.GetWorldTime();

	// ��ʼ����ͨ��Ʒ
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
		//�Ƿ񱻼���
		pItem->SetAct(INVALID_VALUE);

		// �ŵ���������
		ASSERT(pItemProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pItemProto->byShelf].Add(pItem, n16ItemNum[pItemProto->byShelf]++);
	}

	// ��ʼ��������Ʒ��ʼ�±�
	get_fast_code()->memory_copy(m_n16RareIndexStart, n16ItemNum, sizeof(m_n16RareIndexStart));

	// ��ʼ��������Ʒ
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
		//�Ƿ񱻼���
		pItem->SetAct(INVALID_VALUE);

		// �ŵ���������
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pItem, n16ItemNum[pRareProto->byShelf]++);
	}

	// ����̵���Դ�Ƿ�������
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
// ����
//-----------------------------------------------------------------------------
VOID ItemClanTreasury::Update()
{
	// ��ȡ��ǰʱ��
	DWORD dwCurTime = g_world.GetWorldTime();

	// ��¼��ҳ��Ʒ�±�
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	// ���������Ʒ�Ƿ���Ҫ����
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

		// δ����Ĳ�����
		if (!pItem->IsAct()) continue;

		if(CalcTimeDiff(dwCurTime, pItem->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			pItem->n16RemainNum	= pRareProto->byRefreshNum;
			pItem->dwRefreshTime = dwCurTime;
		}
	}
}

//-----------------------------------------------------------------------------
// ��ȡˢ����Ʒ
//-----------------------------------------------------------------------------
VOID ItemClanTreasury::GetRareItems(OUT tagMsgShopItem* pRareItems, INT16 n16RareItemNum, BYTE byShelf)
{
	ASSERT(GetRareGoodsNum(byShelf) == n16RareItemNum);

	tagTreasuryItem *pShopItem = NULL;
	for(INT16 i=m_n16RareIndexStart[byShelf]; i<m_Shelf[byShelf].GetCurSpaceSize(); ++i)
	{
		pShopItem = m_Shelf[byShelf].GetItem(i);
		ASSERT(VALID_POINT(pShopItem));
		
		// δ����Ĳ�����
		if (!pShopItem->IsAct()) continue;

		pRareItems[i].dw_data_id		= pShopItem->dw_data_id;
		pRareItems[i].n16RemainNum	= pShopItem->n16RemainNum;
		pRareItems[i].n16Index		= pShopItem->n16Index;
		pRareItems[i].byProtoIndex	= pShopItem->byProtoIndex;
	}
}

//-----------------------------------------------------------------------------
// ������Ʒ
//-----------------------------------------------------------------------------
DWORD ItemClanTreasury::sell_goods(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT16 n16BuyNum, OUT tagSellItem &sell_goods)
{
	ASSERT(VALID_POINT(m_pShopProto));
	ASSERT(VALID_POINT(pRole));

	// ������Ʒ
	tagTreasuryItem *pShopItem = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopItem))
	{
		// ִ�е������ζ����Ϣ�е�dw_data_id�Ƿ�
		ASSERT(VALID_POINT(pShopItem));
		return E_Shop_ItemNotFind;
	}

	// �Ƿ񼤻�
	if(!pShopItem->IsAct())
	{
		return E_Shop_ItemNotActived;
	}

	// �����Ʒ����
	if(pShopItem->n16RemainNum != INVALID_VALUE && pShopItem->n16RemainNum < n16BuyNum)
	{
		sell_goods.n16RemainNum = pShopItem->n16RemainNum;
		return E_Shop_ItemNotEnough;
	}

	// �����н�Ǯ -- ��ʹ��ͨ�ýӿ�
	INT nSilverCost = pShopItem->pProto->nSilver * n16BuyNum;
	if(nSilverCost > pRole->GetCurMgr().GetBagSilver())
	{
		return E_Shop_NotEnough_SilverInBag;
	}

	// ��������Ʒ���� -- ʹ��ͨ�ýӿ�(�߻������󣬳������޸�)
	INT nSpecCost = pShopItem->pProto->nCostNum * n16BuyNum;
	DWORD dw_error_code = CheckSpecCost(pRole, m_pShopProto->eCostType, nSpecCost, 
		m_pShopProto->dwItemTypeID, pShopItem->pProto->byRepLevel, sell_goods.listItem);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ������Ʒ
	ASSERT(pShopItem->IsAct());
	tagItem *pItemSell = ItemCreator::CreateTreasure(pShopItem->dwNameID, EICM_ShopNPC, m_dwNPCID, dw_data_id, n16BuyNum);
	if(!VALID_POINT(pItemSell))
	{
		ASSERT(VALID_POINT(pItemSell));
		return E_Shop_CreateItem_Failed;
	}

	// �����̵�����Ʒ����
	if(pShopItem->n16RemainNum != INVALID_VALUE)
	{
		pShopItem->n16RemainNum -= n16BuyNum;
	}

	// ���ô�������
	sell_goods.pItemSell		= pItemSell;
	sell_goods.n16RemainNum	= pShopItem->n16RemainNum;
	sell_goods.nSilverCost	= nSilverCost;
	sell_goods.eCostType		= m_pShopProto->eCostType;
	sell_goods.nSpecCost		= nSpecCost;
	sell_goods.byIndex		= (BYTE)pShopItem->n16Index;

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����䱦
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
				// ϡ���䱦
				m_n16ActRareNum[i]++;
			}
			return TRUE;
		}
	}
	// ��Դ������
	ASSERT(0);
	return FALSE;
}

//*********************************** EquipClanTreasury **********************************

//-----------------------------------------------------------------------------
// ����&��������
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
	// ɾ�������ɵ�װ��
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
// ��ʼ��
//-----------------------------------------------------------------------------
BOOL EquipClanTreasury::Init(const tagShopProto *pShopProto)
{
	ASSERT(VALID_POINT(pShopProto));
	ASSERT(pShopProto->bEquip);

	// ��ʼ������
	for(INT32 n=0; n<MAX_SHOP_SHELF_NUM; ++n)
	{
		m_Shelf[n].Init(EICT_Shop, pShopProto->n16Num[n], pShopProto->n16Num[n]);
	}

	// ��¼��ҳ��Ʒ����
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];
	ZeroMemory(n16ItemNum, sizeof(n16ItemNum));

	// ��ȡ��ǰʱ��
	DWORD dwCurTime = g_world.GetWorldTime();

	// ��ʼ����ͨ��Ʒ
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


		// �ŵ���������
		ASSERT(pEquipProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pEquipProto->byShelf].Add(pEquip, n16ItemNum[pEquipProto->byShelf]++);
	}

	// ��ʼ��������Ʒ��ʼ�±�
	get_fast_code()->memory_copy(m_n16RareIndexStart, n16ItemNum, sizeof(m_n16RareIndexStart));

	// ��ʼ��������Ʒ
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

		// �ŵ���������
		ASSERT(pRareProto->byShelf < MAX_SHOP_SHELF_NUM);
		m_Shelf[pRareProto->byShelf].Add(pEquip, n16ItemNum[pRareProto->byShelf]++);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
VOID EquipClanTreasury::Update()
{
	// ��ȡ��ǰʱ��
	DWORD dwCurTime = g_world.GetWorldTime();

	// ��¼��ҳ��Ʒ�±�
	INT16	n16ItemNum[MAX_SHOP_SHELF_NUM];

	get_fast_code()->memory_copy(n16ItemNum, m_n16RareIndexStart, sizeof(m_n16RareIndexStart));

	// ���������Ʒ�Ƿ���Ҫ����
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

		// δ����Ĳ�����
		if (!pEquip->IsAct()) continue;
		
		if(CalcTimeDiff(dwCurTime, pEquip->dwRefreshTime) >= pRareProto->dwRefreshTime)
		{
			if(VALID_POINT(pEquip->pEquip))
			{
				ASSERT(pEquip->pEquip->dw_data_id == pRareProto->dw_data_id);

				SAFE_DELETE(pEquip->pEquip);
			}

			// ����װ��
			ASSERT(pEquip->IsAct());
			pEquip->pEquip = (tagEquip*)ItemCreator::CreateTreasureEx(pEquip->dwNameID, EICM_ShopNPC, m_dwNPCID, pRareProto->dw_data_id, 1, (EItemQuality)pRareProto->byQuality);

			pEquip->n16RemainNum	= pRareProto->byRefreshNum;
			pEquip->dwRefreshTime	= dwCurTime;
		}
	}
}

//-----------------------------------------------------------------------------
// ��ȡ����װ��
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
// ����װ��
//-----------------------------------------------------------------------------
DWORD EquipClanTreasury::SellEquip(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial, OUT tagSellEquip &SellEquip)
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(VALID_POINT(m_pShopProto));

	// ����װ��
	tagTreasuryEquip *pShopEquip = m_Shelf[byShelf].GetItem(dw_data_id);
	if(!VALID_POINT(pShopEquip))
	{
		// ִ�е������ζ����Ϣ�е�dw_data_id�Ƿ�
		ASSERT(VALID_POINT(pShopEquip));
		return E_Shop_ItemNotFind;
	}

	// �Ƿ񱻼���
	if (!pShopEquip->IsAct())
	{
		return E_Shop_ItemNotActived;
	}

	// �����ϡ��װ�����ָ��װ���Ƿ��Ѿ�������
	if(pShopEquip->n16RemainNum >= 0)
	{
		if(!VALID_POINT(pShopEquip->pEquip) || pShopEquip->pEquip->n64_serial != n64_serial)
		{
			// ���ô�������
			SellEquip.pEquipNew		= pShopEquip->pEquip;
			SellEquip.n16RemainNum	= pShopEquip->n16RemainNum;

			return E_Shop_Equip_Sold;
		}
	}

	// �����н�Ǯ
	if(pShopEquip->pProto->nSilver > pRole->GetCurMgr().GetBagSilver())
	{
		return E_Shop_NotEnough_SilverInBag;
	}

	// �����׶��Ƿ�����
	DWORD dw_error_code = CheckSpecCost(pRole, m_pShopProto->eCostType, 
		pShopEquip->pProto->nCostNum, m_pShopProto->dwItemTypeID, 
		pShopEquip->pProto->byRepLevel, SellEquip.listItem);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ���64λid�Ƿ�Ϸ����Ƿ��Ļ�������������һ��
	if(pShopEquip->pEquip->n64_serial < MIN_ITEM_SERIAL_INTERNAL)
	{
		ItemCreator::CreateItemSerial(pShopEquip->pEquip->n64_serial);
	}

	// ���ô�������
	SellEquip.pEquipSell	= pShopEquip->pEquip;
	SellEquip.nSilverCost	= pShopEquip->pProto->nSilver;
	SellEquip.eCostType		= m_pShopProto->eCostType;
	SellEquip.nSpecCost		= pShopEquip->pProto->nCostNum;

	// �����̵�����Ʒ����
	if(pShopEquip->n16RemainNum != INVALID_VALUE)
	{
		--pShopEquip->n16RemainNum;
	}

	// ������װ��
	if(pShopEquip->n16RemainNum != 0)
	{
		ASSERT(pShopEquip->IsAct());
		pShopEquip->pEquip = (tagEquip*)ItemCreator::CreateTreasureEx(	pShopEquip->dwNameID, EICM_ShopNPC, m_dwNPCID, dw_data_id, 
																		1, (EItemQuality)SellEquip.pEquipSell->equipSpec.byQuality);
		if(!VALID_POINT(pShopEquip->pEquip))
		{
			// ִ�е�����,Ӧ�����ڴ�����ʧ��.(ԭ�Ϳ϶������ҵ�)
			ASSERT(VALID_POINT(pShopEquip->pEquip));
			// return E_Shop_CreateEquip_Failed;
			return E_Success;
		}
	}
	else
	{
		pShopEquip->pEquip = NULL;
	}

	// ���ô�������
	SellEquip.pEquipNew		= pShopEquip->pEquip;
	SellEquip.n16RemainNum	= pShopEquip->n16RemainNum;
	SellEquip.byIndex		= (BYTE)pShopEquip->n16Index;

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����䱦
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