/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�̳Ǵ���

#include "StdAfx.h"
#include "../../common/WorldDefine/mall_protocol.h"
#include "../../common/WorldDefine/mall_define.h"
#include "../common/ServerDefine/yuanbao_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "role.h"
#include "mall.h"
#include "role_mgr.h"
#include "TradeYuanBao.h"
#include "guild.h"
#include "guild_manager.h"
#include "world.h"

//-----------------------------------------------------------------------------
// ��ȡ�̳���������Ʒ
//-----------------------------------------------------------------------------
DWORD Role::GetMallAll(OUT DWORD &dwMallTime)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// ��ȡ�������̳Ǽ���ʱ��
	dwMallTime = g_mall.GetMallTime();

	INT nGoodsNum, nSzMsg;

	// ��ͨ��Ʒ
	nGoodsNum = g_mall.GetItemNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_item) - 1 + sizeof(tagMallItemProto) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_item);
		pSend->nItemNum = nGoodsNum;
		g_mall.GetAllItems(pSend->byData);

		SendMessage(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	// ��Ʒ��
	nGoodsNum = g_mall.GetPackNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_pack) - 1 + sizeof(tagMallPackProto) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_pack);
		pSend->nPackNum = nGoodsNum;
		g_mall.GetAllPacks(pSend->byData);

		SendMessage(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}
	
	// �����ȡ��Ʒ(ֻ��1��)
	nGoodsNum = g_mall.GetFreeItemNum();
	if(nGoodsNum > 0)
	{
		NET_SIS_mall_free_item send;
		g_mall.GetFreeItem(&send.freeItem);
		SendMessage(&send, send.dw_size);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����̳����г��۸������Ƶ�������Ʒ����
//-----------------------------------------------------------------------------
DWORD Role::UpdateMallAll(OUT DWORD &dwNewMallTime, IN DWORD dwOldMallTime)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// ��ȡ�������̳Ǽ���ʱ��
	dwNewMallTime = g_mall.GetMallTime();

	//-- ���ͻ��˵��̳Ǿ�̬�����Ƿ����������ͬ

	// 1.��ͬ�����·����̵�ԭ����Ϣ
	if(dwNewMallTime != dwOldMallTime)
	{
		return GetMallAll(dwNewMallTime);
	}

	// 2.��ͬ��ֻˢ���и������Ƶ���Ʒ������Ϣ
	INT nGoodsNum, nSzMsg;

	// ��ͨ��Ʒ
	nGoodsNum = g_mall.GetItemNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_item);
		g_mall.UpdateAllItems(pSend->byData, pSend->nItemNum);

		// ���¼�����Ϣ��С
		if(pSend->nItemNum > 0)
		{
			pSend->dw_size = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate) * pSend->nItemNum;
			SendMessage(pSend, pSend->dw_size);
		}

		MDEL_MSG(pSend);
	}

	// ��Ʒ��
	nGoodsNum = g_mall.GetPackNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_pack);
		g_mall.UpdateAllPacks(pSend->byData, pSend->nItemNum);

		// ���¼�����Ϣ��С
		if(pSend->nItemNum > 0)
		{
			pSend->dw_size = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate) * pSend->nItemNum;
			SendMessage(pSend, pSend->dw_size);
		}

		MDEL_MSG(pSend);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����̳���Ʒ
//-----------------------------------------------------------------------------
DWORD Role::BuyMallItem(DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	//if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	//{
	//	return E_Con_PswNotPass;
	//}
	
	// Ԥ��鱳�����Ƿ��п�λ
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.sell_goods(this, GetID(), elcid_mall_buy_item, 
										dwID, byIndex, nUnitPrice, n16BuyNum, sItemSell);

	// ������
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// Ԫ�������̳��п۳�
		
		// ����Ʒ�ŵ���ұ�����
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_buy_item, TRUE);
		
		// �������Ʒ����ŵ��ٱ�����
		if(VALID_POINT(sItemSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, 
							GetNameID(), INVALID_VALUE, EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_buy_item_add);
		}

		// �����������
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_buy_item);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Item_NotEnough == dw_error_code) 
		&& sItemSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_item);
		
		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sItemSell.byRemainNum;
		p->dwID			= dwID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �����̳���Ʒ��
//-----------------------------------------------------------------------------
DWORD Role::BuyMallPack(DWORD dwID, INT nUnitPrice, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// Ԥ��鱳�����Ƿ��п�λ
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallPackSell sPackSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.SellPack(this, GetID(), elcid_mall_buy_pack, 
										dwID, byIndex, nUnitPrice, sPackSell, TRUE);

	// ������
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{
		// ����Ʒ�ŵ���ұ�����
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			GetItemMgr().Add2Bag(sPackSell.pItem[i], elcid_mall_buy_pack, TRUE);
		}

		// �������Ʒ����ŵ��ٱ�����
		if(VALID_POINT(sPackSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
								GetNameID(), INVALID_VALUE, EBBRT_Mall, sPackSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_buy_pack_add);
		}

		// �����������
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_buy_pack);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Pack_NotEnough == dw_error_code) 
		&& sPackSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_pack);

		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sPackSell.byRemainNum;
		p->dwID			= dwID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �����̳���Ʒ������
//-----------------------------------------------------------------------------
DWORD Role::BuyMallItem(DWORD dwTgtRoleID, LPCTSTR szLeaveWord, 
						DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// ������ID����Ϸ�������Ƿ����
	if(!g_roleMgr.is_this_world_role(dwTgtRoleID))
	{
		return E_Role_Not_ExistIn_World;
	}

	tagMallItemSell sItemSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.sell_goods(this, dwTgtRoleID, elcid_mall_present_item, 
										dwID, byIndex, nUnitPrice, n16BuyNum, sItemSell);

	// ������
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// �ٱ�����ʷ��¼
		GetItemMgr().ProcBaiBaoRecord(sItemSell.pItem->dw_data_id, dwTgtRoleID, 
							GetNameID(), EBBRT_Friend, sItemSell.pItem->dw1stGainTime, szLeaveWord);

		// ����Ʒ�ŵ����Ѱٱ�����
		Role *pFriend = g_roleMgr.get_role(dwTgtRoleID);
		if(VALID_POINT(pFriend))
		{
			pFriend->GetItemMgr().Add2BaiBao(sItemSell.pItem, elcid_mall_present_item, GetID());
		}
		else
		{
			// �洢��item_baibao����
			ItemMgr::InsertBaiBao2DB(sItemSell.pItem, dwTgtRoleID, elcid_mall_present_item);
			
			// ɾ����Ʒ
			::Destroy(sItemSell.pItem);
		}

		// �������Ʒ����ŵ����Ѱٱ�����
		if(VALID_POINT(sItemSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, dwTgtRoleID, 
										GetNameID(), EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_present_item_add);
			}
			else
			{
				// �洢��item_baibao����
				ItemMgr::InsertBaiBao2DB(sItemSell.pPresent, dwTgtRoleID, elcid_mall_present_item_add);

				// ɾ����Ʒ
				::Destroy(sItemSell.pPresent);
			}
		}
		
		// ����һ�������
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_present_item);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Item_NotEnough == dw_error_code) 
		&& sItemSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_item);

		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sItemSell.byRemainNum;
		p->dwID			= dwID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ������Ʒ��Ʒ��������
//-----------------------------------------------------------------------------
DWORD Role::BuyMallPack(DWORD dwTgtRoleID, LPCTSTR szLeaveWord, 
						DWORD dwID, INT nUnitPrice, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// ������ID����Ϸ�������Ƿ����
	if(!g_roleMgr.is_this_world_role(dwTgtRoleID))
	{
		return E_Role_Not_ExistIn_World;
	}

	tagMallPackSell sPackSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.SellPack(this, dwTgtRoleID, elcid_mall_present_pack, 
										dwID, byIndex, nUnitPrice, sPackSell, FALSE);

	// ������
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{		
		Role *pFriend = g_roleMgr.get_role(dwTgtRoleID);

		// ����Ʒ�ŵ����Ѱٱ����� -- item_baibao��
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			// ��¼����һ����Ʒ��
			if(0 == i)
			{
				// �ٱ�����ʷ��¼
				GetItemMgr().ProcBaiBaoRecord(sPackSell.pItem[i]->dw_data_id, 
					dwTgtRoleID, GetNameID(), EBBRT_Friend, sPackSell.pItem[i]->dw1stGainTime, szLeaveWord);
			}
			else
			{
				// �ٱ�����ʷ��¼(������)
				GetItemMgr().ProcBaiBaoRecord(sPackSell.pItem[i]->dw_data_id, 
					dwTgtRoleID, GetNameID(), EBBRT_Friend, sPackSell.pItem[i]->dw1stGainTime);
			}

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sPackSell.pItem[i], elcid_mall_present_pack, GetID());
			}
			else
			{
				// �洢��item_baibao����
				ItemMgr::InsertBaiBao2DB(sPackSell.pItem[i], dwTgtRoleID, elcid_mall_present_pack);

				// ɾ����Ʒ
				::Destroy(sPackSell.pItem[i]);
			}
		}

		// �������Ʒ����ŵ����Ѱٱ�����
		if(VALID_POINT(sPackSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
							dwTgtRoleID, GetNameID(), EBBRT_Mall, sPackSell.pPresent->dw1stGainTime);

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_present_pack_add, GetID());
			}
			else
			{
				// �洢��item_baibao����
				ItemMgr::InsertBaiBao2DB(sPackSell.pPresent, dwTgtRoleID, elcid_mall_present_pack_add);

				// ɾ����Ʒ
				::Destroy(sPackSell.pPresent);
			}
		}

		// ����һ�������
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_present_pack);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Pack_NotEnough == dw_error_code) 
		&& sPackSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_pack);

		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sPackSell.byRemainNum;
		p->dwID			= dwID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡ�̳������Ʒ
//-----------------------------------------------------------------------------
DWORD Role::GetMallFreeItem(DWORD dwID)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}
	
	// Ԥ��鱳�����Ƿ��п�λ
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.GrantFreeItem(this, dwID, sItemSell);

	// ������
	if(E_Success == dw_error_code && VALID_POINT(sItemSell.pItem))
	{
		// ����Ʒ�ŵ�������
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_free_item, TRUE);
	}
	
	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �һ��̳���Ʒ
//-----------------------------------------------------------------------------
DWORD Role::MallItemExchange(DWORD dwMallID, INT nPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// Ԥ��鱳�����Ƿ��п�λ
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.ExchangeItem(this, elcid_mall_exchange_item, dwMallID, byIndex, nPrice, n16BuyNum, sItemSell);

	// ������
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// ����Ʒ�ŵ���ұ�����
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_exchange_item, TRUE);

		// �������Ʒ����ŵ��ٱ�����
		if(VALID_POINT(sItemSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, 
				GetNameID(), INVALID_VALUE, EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_exchange_item_add);
		}

		// �����������
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_exchange_item);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Item_NotEnough == dw_error_code) 
		&& sItemSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_item);

		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sItemSell.byRemainNum;
		p->dwID			= dwMallID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;

}

//-----------------------------------------------------------------------------
// �һ��̳Ǵ����Ʒ
//-----------------------------------------------------------------------------
DWORD Role::MallPackExchange(DWORD dwMallID, INT nPrice, BYTE byIndex)
{
	// �ж��̳��Ƿ񿪷�
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// �����Ƿ����
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// Ԥ��鱳�����Ƿ��п�λ
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallPackSell sPackSell;

	// �̳�����Ʒ��ؼ��
	DWORD dw_error_code = g_mall.ExchangePack(this, elcid_mall_exchange_pack, dwMallID, byIndex, nPrice, sPackSell);

	// ������
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{
		INT64 n64_serial = sPackSell.pItem[0]->n64_serial;
		INT16 n16BuyNum = sPackSell.pItem[0]->n16Num;
		DWORD dwFstGainTime = g_world.GetWorldTime();

		// ����Ʒ�ŵ���ұ�����
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			GetItemMgr().Add2Bag(sPackSell.pItem[i], elcid_mall_exchange_pack, TRUE);
		}

		// �������Ʒ����ŵ��ٱ�����
		if(VALID_POINT(sPackSell.pPresent))
		{
			// �ٱ�����ʷ��¼
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
				GetNameID(), INVALID_VALUE, EBBRT_Mall, dwFstGainTime);

			GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_exchange_pack_add);
		}

		// �����������
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_exchange_pack);
		}
	}

	// ���͸��º��̵���Ʒ -- ֻ��ˢ����ƷҪ������Ʒ����
	if((E_Success == dw_error_code || E_Mall_Pack_NotEnough == dw_error_code) 
		&& sPackSell.byRemainNum != (BYTE)INVALID_VALUE)
	{
		INT nSzMsg = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate);
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_pack);

		pSend->nItemNum = 1;
		M_trans_pointer(p, pSend->byData, tagMallUpdate);
		p->byRemainNum	= sPackSell.byRemainNum;
		p->dwID			= dwMallID;

		SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �����Ԫ�������˻���Ԫ��
//-----------------------------------------------------------------------------
DWORD Role::SaveYB2Account(DWORD dwID, INT n_num)
{
	// �����ұ���Ԫ������
	if(GetCurMgr().GetBagYuanBao() < n_num)
		return E_Trade_BagYB_NotEnough;

	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
	{
		pYBAccount = g_tradeYB.CreateTradeAccount(dwID);
		if(!VALID_POINT(pYBAccount))
			return INVALID_VALUE;
	}

	pYBAccount->IncAccountYB(n_num, (DWORD)elcid_trade_save_yuanbao, TRUE);
	GetCurMgr().DecBagYuanBao(n_num, (DWORD)elcid_trade_save_yuanbao);
		
	return E_Success;
}

//-----------------------------------------------------------------------------
// �����Ԫ�������˻����Ǯ
//-----------------------------------------------------------------------------
DWORD Role::SaveSilver2Account(DWORD dwID, INT64 n_num)
{
	// �����ұ�����Ǯ����
	if(GetCurMgr().GetBagSilver() < n_num)
		return E_Trade_BagSilver_NotEnough;

	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
	{
		pYBAccount = g_tradeYB.CreateTradeAccount(dwID);
		if(!VALID_POINT(pYBAccount))
			return INVALID_VALUE;
	}

	pYBAccount->IncAccountSilver(n_num, (DWORD)elcid_trade_save_silver, TRUE);
	GetCurMgr().DecBagSilver(n_num, (DWORD)elcid_trade_save_silver);

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����Ԫ�������˻�ȡԪ��
//-----------------------------------------------------------------------------
DWORD Role::DepositYBAccout(DWORD dwID, INT n_num)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	// ����˻�Ԫ������
	if(pYBAccount->GetAccountYB() < n_num)
		return E_Trade_AccountYB_NotEnough;

	// �������Ƿ��ύ�����۶���
	tagYuanBaoOrder *pYBOrder = g_tradeYB.GetYBSellOrder(dwID);
	if(VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	pYBAccount->DecAccountYuanBao(n_num, (DWORD)elcid_trade_depossit_yuanbao, TRUE);
	GetCurMgr().IncBagYuanBao(n_num, (DWORD)elcid_trade_depossit_yuanbao);
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// �����Ԫ�������˻�ȡ��Ǯ
//-----------------------------------------------------------------------------
DWORD Role::DepositSilverAccount(DWORD dwID, INT64 n_num)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	// ����˻���Ǯ����
	if(pYBAccount->GetAccountSilver() < n_num)
		return E_Trade_AccountSilver_NotEnough;

	// �������Ƿ��ύ���չ�����
	tagYuanBaoOrder *pYBOrder = g_tradeYB.GetYBBuyOrder(dwID);
	if(VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	pYBAccount->DecAccountSilver(n_num, (DWORD)elcid_trade_deposit_silver, TRUE);
	GetCurMgr().IncBagSilver(n_num, (DWORD)elcid_trade_deposit_silver);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ͬ��Ԫ�����׳�ʼ����Ϣ
//-----------------------------------------------------------------------------
DWORD Role::GetYBTradeInfo()
{
	g_tradeYB.SynBuyPriceList(this);
	g_tradeYB.SynSellPriceList(this);
	g_tradeYB.SynYBAccount(this);
	return E_Success;
}

//-----------------------------------------------------------------------------
// ����ύԪ�����۶���
//-----------------------------------------------------------------------------
DWORD Role::SubmitSellOrder(DWORD dw_role_id, INT n_num, INT nPrice)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	if(n_num <= 0 || nPrice <= 0)
		return INVALID_VALUE;
		
	// �Ƿ��Ѿ��ύ�����۶���
	tagYuanBaoOrder* pSellOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	pSellOrder = g_tradeYB.GetYBSellOrder(dw_role_id);
	if(VALID_POINT(pSellOrder))
		return E_Trade_SellOrder_Exit;

	// �����˻�Ԫ���Ƿ��㹻
	if(pYBAccount->GetAccountYB() < n_num)
		return E_Trade_AccountYB_NotEnough;

	// ����������Ϊ�ܼ۵�2%
	INT nTax = nPrice * n_num * 2 / 100;
	if(nTax < 1)    nTax = 1;

	// ����������Ƿ��㹻
	if (GetCurMgr().GetBagSilver() < nTax)
		return E_Trade_Tax_NotEnough;


	tagYuanBaoOrder * pYBOrder = g_tradeYB.CreateYBOrder(dw_role_id, EYBOT_SELL, nPrice, n_num);
	if(!VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	// �����˻��ж������ύ״̬
	pYBAccount->SetSellOrder(TRUE);

	// �۳������� 
	GetCurMgr().DecBagSilver(nTax, elcid_trade_tax);

	// ����Ԫ��
	g_tradeYB.DealYBSell(pYBOrder);
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// ����ύԪ���չ�����
//-----------------------------------------------------------------------------
DWORD Role::SubmitBuyOrder(DWORD dw_role_id, INT n_num, INT nPrice)
{
	tagYuanBaoAccount *pYBAccount = g_tradeYB.GetYBAccount(dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	if(n_num <= 0 || nPrice <= 0)
		return INVALID_VALUE;

	if(n_num * nPrice <= 0)
		return E_Trade_AccountSilver_NotEnough;

	// �Ƿ��ѽ��ύ������
	tagYuanBaoOrder* pBuyOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	pBuyOrder = g_tradeYB.GetYBBuyOrder(dw_role_id);
	if(VALID_POINT(pBuyOrder))
		return E_Trade_BuyOrder_Exit;

	// �����˻���Ǯ�Ƿ��㹻
	if(pYBAccount->GetAccountSilver() < n_num * nPrice)
		return E_Trade_AccountSilver_NotEnough;

	// ����������Ϊ�ܼ۵�2%
	INT nTax = (FLOAT)(nPrice * n_num) * 0.02f;
	if(nTax < 1)    nTax = 1;

	// ����������Ƿ��㹻
	if (GetCurMgr().GetBagSilver() < nTax)
		return E_Trade_Tax_NotEnough;

	tagYuanBaoOrder *pYBOrder = g_tradeYB.CreateYBOrder(dw_role_id, EYBOT_BUY, nPrice, n_num);
	if(!VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	// �����˻��ж������ύ״̬
	pYBAccount->SetBuyOrder(TRUE);

	// �۳������� 
	GetCurMgr().DecBagSilver(nTax, elcid_trade_tax);

	// ����Ԫ��
	g_tradeYB.DealYBBuy(pYBOrder);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ɾ������
//-----------------------------------------------------------------------------
DWORD Role::DeleteOrder(DWORD dw_role_id, DWORD dwOrderID, EYBOTYPE eYBOType)
{
	if(eYBOType != EYBOT_BUY && eYBOType != EYBOT_SELL)
		return INVALID_VALUE;

	tagYuanBaoOrder *pYBOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	if(eYBOType == EYBOT_BUY)
		pYBOrder = g_tradeYB.GetYBBuyOrder(dw_role_id);
	else
		pYBOrder = g_tradeYB.GetYBSellOrder(dw_role_id);

	if(!VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	if(pYBOrder->dwID != dwOrderID)
		return INVALID_VALUE;

	g_tradeYB.DeleteYBOrder(pYBOrder, EYBOM_Cancel);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��ѯһ���ڸ���ҵ�Ԫ�����׶���
//-----------------------------------------------------------------------------
DWORD Role::GetYBOrder(DWORD dw_role_id)
{
	tagYuanBaoAccount *pYBAccount = g_tradeYB.GetYBAccount(dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return  INVALID_VALUE;

	DWORD dwCurTick = g_world.GetWorldTick();
	if(dwCurTick - pYBAccount->GetQuestTick() > 50)
		pYBAccount->SetQuestTick(dwCurTick);
	else
		return INVALID_VALUE;

	// �����ݿⷢ�Ͳ�ѯ��Ϣ
	NET_DB2C_get_role_yuanbao_order	sendDB;
	sendDB.dw_role_id = dw_role_id;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return E_Success;
}