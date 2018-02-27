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

#include "shop.h"


//-----------------------------------------------------------------------------
// ���屦�⣨��װ����
//-----------------------------------------------------------------------------
class ItemClanTreasury: public Shop
{
	friend class Shop;

	typedef Container<tagTreasuryItem, DWORD>	TreasuryItemCon;
public:
	virtual BOOL	ActiveTreasure(DWORD dw_data_id, DWORD dwNameID);

private:
	ItemClanTreasury(DWORD dwNPCID, const tagShopProto *pShopProto);
	virtual ~ItemClanTreasury();

private:
	virtual VOID Update();

	virtual INT16 get_goods_num(BYTE byShelf) const
	{ ASSERT(byShelf < MAX_SHOP_SHELF_NUM); return m_n16ActNum[byShelf]; }

	virtual INT16	GetRareGoodsNum(BYTE byShelf) const
	{ return m_n16ActRareNum[byShelf]; }

	virtual VOID	GetRareItems(OUT tagMsgShopItem* pRareItems, INT16 n16RareItemNum, BYTE byShelf);

	virtual DWORD	sell_goods(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT16 n16BuyNum, OUT tagSellItem &sell_goods);

private:
	BOOL Init(const tagShopProto *pShopProto);

private:
	TreasuryItemCon		m_Shelf[MAX_SHOP_SHELF_NUM];		// ��Ʒ��ҳ
	INT16				m_n16ActNum[MAX_SHOP_SHELF_NUM];		// �����䱦��Ŀ
	INT16				m_n16ActRareNum[MAX_SHOP_SHELF_NUM];	// ����ϡ���䱦��Ŀ
};

//-----------------------------------------------------------------------------
// ���屦�⣨װ����
//-----------------------------------------------------------------------------
class EquipClanTreasury: public Shop
{
	friend class Shop;

	typedef Container<tagTreasuryEquip, DWORD>	TreasuryEquipCon;
public:
	virtual BOOL	ActiveTreasure(DWORD dw_data_id, DWORD dwNameID);

private:
	EquipClanTreasury(DWORD dwNPCID, const tagShopProto *pShopProto);
	virtual ~EquipClanTreasury();

private:
	virtual VOID Update();

	virtual INT16 get_goods_num(BYTE byShelf) const
	{ ASSERT(byShelf < MAX_SHOP_SHELF_NUM); return m_n16ActNum[byShelf]; }

	virtual INT16	GetRareGoodsNum(BYTE byShelf) const
	{ return m_n16ActRareNum[byShelf]; }

	virtual VOID	GetRareEquips(OUT tagMsgShopEquip* pEquips, BYTE byShelf, INT16 &n16RareEquipNum);

	virtual DWORD	SellEquip(Role *pRole, BYTE byShelf, DWORD dw_data_id, INT64 n64_serial, OUT tagSellEquip &SellEquip);

private:
	BOOL Init(const tagShopProto *pShopProto);

private:
	TreasuryEquipCon	m_Shelf[MAX_SHOP_SHELF_NUM];			// ��Ʒ��ҳ
	INT16				m_n16ActNum[MAX_SHOP_SHELF_NUM];		// �����䱦��Ŀ
	INT16				m_n16ActRareNum[MAX_SHOP_SHELF_NUM];	// ����ϡ���䱦��Ŀ
};
