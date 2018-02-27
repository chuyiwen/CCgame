/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�̳�

#pragma once

class Role;
class guild_buy;

struct tagItem;
//-----------------------------------------------------------------------------
// �����д�����������
//-----------------------------------------------------------------------------
struct tagMallItemSell
{
	tagItem		*pItem;
	tagItem		*pPresent;
	INT			nYuanBaoNeed;		//�һ�ʱ������������
	BYTE		nExVolumeAssign;
	BYTE		byRemainNum;
	
	tagMallItemSell() { ZeroMemory(this, sizeof(*this)); }
};

struct tagMallPackSell
{
	tagItem		*pItem[MALL_PACK_ITEM_NUM];
	tagItem		*pPresent;
	INT			nYuanBaoNeed;		//�һ�ʱ������������
	BYTE		nExVolumeAssign;
	BYTE		byRemainNum;
	
	tagMallPackSell() { ZeroMemory(this, sizeof(*this)); }
};

//-----------------------------------------------------------------------------
// �̳���Ʒ����
//----------------------------------------------------------------------------
enum EMallItemType
{
	EMIT_Item,			// ��Ʒ
	EMIT_Pack,			// �����Ʒ
	EMIT_FreeItem,		// �����Ʒ

	EMIT_End
};

//-----------------------------------------------------------------------------
// �̳���
//-----------------------------------------------------------------------------
class Mall
{
public:
	Mall();
	~Mall();

	// �򿪹ر��̳�
	BOOL Init();
	VOID Destroy();

	// ������Դ�������´��̳�
	DWORD ReInit();

	// �Ź���Ϣ����
	VOID Update();

public:
	BOOL is_init_ok()			const { return m_bInitOK; }
	INT  GetItemNum()		const { return m_nItemNum; }
	INT  GetPackNum()		const { return m_nPackNum; }
	INT	 GetFreeItemNum()	const { return (m_pMallFreeItem->pMallFreeItem->dw_data_id != INVALID_VALUE) ? 1 : 0; }

	DWORD GetMallTime()		const { return m_dwLoadTime; }

	const tagMallGoods* GetMallItem(BYTE byIndex, EMallItemType eType = EMIT_Item);

public:
	// ͬ��
	DWORD GetAllItems(OUT LPVOID pData);
	DWORD GetAllPacks(OUT LPVOID pData);
	DWORD GetFreeItem(OUT LPVOID pData);
	DWORD UpdateAllItems(OUT LPVOID pData, OUT INT &nRefreshNum);
	DWORD UpdateAllPacks(OUT LPVOID pData, OUT INT &nRefreshNum);
		
	// ����
	DWORD sell_goods(Role *pRole, DWORD dwToRoleID, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
				INT nUnitPrice, INT16 n16BuyNum, OUT tagMallItemSell &itemSell);
	DWORD SellPack(Role *pRole, DWORD dwToRoleID, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
				INT nUnitPrice, OUT tagMallPackSell &packSell, BOOL bNeedCheckBagSpace);

	// �һ�
	DWORD ExchangeItem(Role *pRole, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
		INT nPrice, INT16 n16BuyNum, OUT tagMallItemSell &itemSell);
	DWORD ExchangePack(Role *pRole, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
		INT nPrice, OUT tagMallPackSell &packSell);

	// ��ѷ���
	DWORD GrantFreeItem(Role *pRole, DWORD dwID, OUT tagMallItemSell &itemSell);

	// �����Ź���Ϣ����
	DWORD LoadAllGPInfo(INT nGPInfoNum, LPVOID pData);
	DWORD lauch_guild_buy(Role *pRole, DWORD dwID, BYTE byScope,
		BYTE byIndex, INT nUnitPrice);
	DWORD respond_guild_buy(Role *pRole, DWORD dwGuildID, DWORD dwID, DWORD dw_role_id,
		INT nPrice);
	DWORD get_all_guild_buy_info(Role* pRole);
	DWORD get_response(Role* pRole, DWORD dwGuildID, DWORD dwID, DWORD dw_role_id);
	VOID RemoveGuildPurchaseInfo(DWORD dwGuildID);

private:
	VOID InitItem();
	VOID InitPack();
	BOOL CheckPack();

public:
	// log
	VOID LogMallSell(DWORD dwBuyRoleID, DWORD dwToRoleID, 
					 const tagItem& item, INT64 n64_serial, INT16 n16Num, 
					 DWORD dwFstGainTime, INT nCostYuanBao, INT nCostExVolume, DWORD dw_cmd_id);
	VOID LogMallSellPack(DWORD dwPackID, DWORD dwBuyRoleID, DWORD dwToRoleID, INT nCostYuanBao);

private:
	BOOL			m_bInitOK;			// �̳ǿ���״̬
	DWORD			m_dwLoadTime;		// ��ȡ�̳���Ʒԭ��ʱ��(tagDwordTime)
	DWORD			m_dwTimeKeeper;		// ��ʱ��
	BYTE			m_byMinuteTime;		// ���Ӽ�ʱ

private:
	INT				m_nItemNum;			// ��Ʒ����
	INT				m_nPackNum;			// ��Ʒ������
	tagMallGoods	*m_pMallItem;		// ��Ʒ����ָ��
	tagMallGoods	*m_pMallPack;		// ��Ʒ������ָ��
	tagMallGoods	*m_pMallFreeItem;	// �����ȡ��Ʒ

private:
	// �����Ź���Ϣ����
	package_map<DWORD, guild_buy*>	m_mapGuildPurchase;

private:
	// ͳ��
	INT				m_nFreeGrantNum;
};

extern Mall g_mall;