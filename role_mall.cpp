/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//商城处理

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
// 获取商城中所有商品
//-----------------------------------------------------------------------------
DWORD Role::GetMallAll(OUT DWORD &dwMallTime)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 获取服务器商城加载时间
	dwMallTime = g_mall.GetMallTime();

	INT nGoodsNum, nSzMsg;

	// 普通商品
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

	// 礼品包
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
	
	// 免费领取商品(只有1个)
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
// 更新商城中有出售个数限制的所有商品个数
//-----------------------------------------------------------------------------
DWORD Role::UpdateMallAll(OUT DWORD &dwNewMallTime, IN DWORD dwOldMallTime)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 获取服务器商城加载时间
	dwNewMallTime = g_mall.GetMallTime();

	//-- 检查客户端的商城静态属性是否与服务器相同

	// 1.不同：重新发送商店原型信息
	if(dwNewMallTime != dwOldMallTime)
	{
		return GetMallAll(dwNewMallTime);
	}

	// 2.相同：只刷新有个数限制的商品个数信息
	INT nGoodsNum, nSzMsg;

	// 普通商品
	nGoodsNum = g_mall.GetItemNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_item);
		g_mall.UpdateAllItems(pSend->byData, pSend->nItemNum);

		// 重新计算消息大小
		if(pSend->nItemNum > 0)
		{
			pSend->dw_size = sizeof(NET_SIS_mall_update_item) - 1 + sizeof(tagMallUpdate) * pSend->nItemNum;
			SendMessage(pSend, pSend->dw_size);
		}

		MDEL_MSG(pSend);
	}

	// 礼品包
	nGoodsNum = g_mall.GetPackNum();
	if(nGoodsNum > 0)
	{
		nSzMsg = sizeof(NET_SIS_mall_update_pack) - 1 + sizeof(tagMallUpdate) * nGoodsNum;
		CREATE_MSG(pSend, nSzMsg, NET_SIS_mall_update_pack);
		g_mall.UpdateAllPacks(pSend->byData, pSend->nItemNum);

		// 重新计算消息大小
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
// 购买商城物品
//-----------------------------------------------------------------------------
DWORD Role::BuyMallItem(DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	//if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	//{
	//	return E_Con_PswNotPass;
	//}
	
	// 预检查背包中是否有空位
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.sell_goods(this, GetID(), elcid_mall_buy_item, 
										dwID, byIndex, nUnitPrice, n16BuyNum, sItemSell);

	// 处理结果
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// 元宝已在商城中扣除
		
		// 将物品放到玩家背包中
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_buy_item, TRUE);
		
		// 如果有赠品，则放到百宝贷中
		if(VALID_POINT(sItemSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, 
							GetNameID(), INVALID_VALUE, EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_buy_item_add);
		}

		// 回馈玩家赠点
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_buy_item);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 购买商城礼品包
//-----------------------------------------------------------------------------
DWORD Role::BuyMallPack(DWORD dwID, INT nUnitPrice, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// 预检查背包中是否有空位
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallPackSell sPackSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.SellPack(this, GetID(), elcid_mall_buy_pack, 
										dwID, byIndex, nUnitPrice, sPackSell, TRUE);

	// 处理结果
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{
		// 将物品放到玩家背包中
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			GetItemMgr().Add2Bag(sPackSell.pItem[i], elcid_mall_buy_pack, TRUE);
		}

		// 如果有赠品，则放到百宝贷中
		if(VALID_POINT(sPackSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
								GetNameID(), INVALID_VALUE, EBBRT_Mall, sPackSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_buy_pack_add);
		}

		// 回馈玩家赠点
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_buy_pack);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 赠送商城物品给好友
//-----------------------------------------------------------------------------
DWORD Role::BuyMallItem(DWORD dwTgtRoleID, LPCTSTR szLeaveWord, 
						DWORD dwID, INT nUnitPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// 检查好友ID在游戏世界中是否存在
	if(!g_roleMgr.is_this_world_role(dwTgtRoleID))
	{
		return E_Role_Not_ExistIn_World;
	}

	tagMallItemSell sItemSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.sell_goods(this, dwTgtRoleID, elcid_mall_present_item, 
										dwID, byIndex, nUnitPrice, n16BuyNum, sItemSell);

	// 处理结果
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// 百宝袋历史记录
		GetItemMgr().ProcBaiBaoRecord(sItemSell.pItem->dw_data_id, dwTgtRoleID, 
							GetNameID(), EBBRT_Friend, sItemSell.pItem->dw1stGainTime, szLeaveWord);

		// 将物品放到好友百宝袋中
		Role *pFriend = g_roleMgr.get_role(dwTgtRoleID);
		if(VALID_POINT(pFriend))
		{
			pFriend->GetItemMgr().Add2BaiBao(sItemSell.pItem, elcid_mall_present_item, GetID());
		}
		else
		{
			// 存储到item_baibao表中
			ItemMgr::InsertBaiBao2DB(sItemSell.pItem, dwTgtRoleID, elcid_mall_present_item);
			
			// 删除物品
			::Destroy(sItemSell.pItem);
		}

		// 如果有赠品，则放到好友百宝袋中
		if(VALID_POINT(sItemSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, dwTgtRoleID, 
										GetNameID(), EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_present_item_add);
			}
			else
			{
				// 存储到item_baibao表中
				ItemMgr::InsertBaiBao2DB(sItemSell.pPresent, dwTgtRoleID, elcid_mall_present_item_add);

				// 删除物品
				::Destroy(sItemSell.pPresent);
			}
		}
		
		// 向买家回馈赠点
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_present_item);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 赠送商品礼品包给好友
//-----------------------------------------------------------------------------
DWORD Role::BuyMallPack(DWORD dwTgtRoleID, LPCTSTR szLeaveWord, 
						DWORD dwID, INT nUnitPrice, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// 检查好友ID在游戏世界中是否存在
	if(!g_roleMgr.is_this_world_role(dwTgtRoleID))
	{
		return E_Role_Not_ExistIn_World;
	}

	tagMallPackSell sPackSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.SellPack(this, dwTgtRoleID, elcid_mall_present_pack, 
										dwID, byIndex, nUnitPrice, sPackSell, FALSE);

	// 处理结果
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{		
		Role *pFriend = g_roleMgr.get_role(dwTgtRoleID);

		// 将物品放到好友百宝袋中 -- item_baibao表
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			// 记录到第一个物品上
			if(0 == i)
			{
				// 百宝袋历史记录
				GetItemMgr().ProcBaiBaoRecord(sPackSell.pItem[i]->dw_data_id, 
					dwTgtRoleID, GetNameID(), EBBRT_Friend, sPackSell.pItem[i]->dw1stGainTime, szLeaveWord);
			}
			else
			{
				// 百宝袋历史记录(无留言)
				GetItemMgr().ProcBaiBaoRecord(sPackSell.pItem[i]->dw_data_id, 
					dwTgtRoleID, GetNameID(), EBBRT_Friend, sPackSell.pItem[i]->dw1stGainTime);
			}

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sPackSell.pItem[i], elcid_mall_present_pack, GetID());
			}
			else
			{
				// 存储到item_baibao表中
				ItemMgr::InsertBaiBao2DB(sPackSell.pItem[i], dwTgtRoleID, elcid_mall_present_pack);

				// 删除物品
				::Destroy(sPackSell.pItem[i]);
			}
		}

		// 如果有赠品，则放到好友百宝贷中
		if(VALID_POINT(sPackSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
							dwTgtRoleID, GetNameID(), EBBRT_Mall, sPackSell.pPresent->dw1stGainTime);

			if(VALID_POINT(pFriend))
			{
				pFriend->GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_present_pack_add, GetID());
			}
			else
			{
				// 存储到item_baibao表中
				ItemMgr::InsertBaiBao2DB(sPackSell.pPresent, dwTgtRoleID, elcid_mall_present_pack_add);

				// 删除物品
				::Destroy(sPackSell.pPresent);
			}
		}

		// 向买家回馈赠点
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_present_pack);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 索取商城免费物品
//-----------------------------------------------------------------------------
DWORD Role::GetMallFreeItem(DWORD dwID)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}
	
	// 预检查背包中是否有空位
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.GrantFreeItem(this, dwID, sItemSell);

	// 处理结果
	if(E_Success == dw_error_code && VALID_POINT(sItemSell.pItem))
	{
		// 将物品放到背包中
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_free_item, TRUE);
	}
	
	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 兑换商城物品
//-----------------------------------------------------------------------------
DWORD Role::MallItemExchange(DWORD dwMallID, INT nPrice, INT16 n16BuyNum, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// 预检查背包中是否有空位
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallItemSell sItemSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.ExchangeItem(this, elcid_mall_exchange_item, dwMallID, byIndex, nPrice, n16BuyNum, sItemSell);

	// 处理结果
	if(E_Success == dw_error_code	&& sItemSell.nYuanBaoNeed > 0 && VALID_POINT(sItemSell.pItem))
	{
		// 将物品放到玩家背包中
		GetItemMgr().Add2Bag(sItemSell.pItem, elcid_mall_exchange_item, TRUE);

		// 如果有赠品，则放到百宝贷中
		if(VALID_POINT(sItemSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, 
				GetNameID(), INVALID_VALUE, EBBRT_Mall, sItemSell.pPresent->dw1stGainTime);

			GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_mall_exchange_item_add);
		}

		// 回馈玩家赠点
		if (sItemSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_mall_exchange_item);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 兑换商城打包物品
//-----------------------------------------------------------------------------
DWORD Role::MallPackExchange(DWORD dwMallID, INT nPrice, BYTE byIndex)
{
	// 判断商城是否开放
	if(!g_mall.is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	// 预检查背包中是否有空位
	if(GetItemMgr().GetBagFreeSize() < 1)
	{
		return E_Bag_NotEnoughSpace;
	}

	tagMallPackSell sPackSell;

	// 商城中商品相关检查
	DWORD dw_error_code = g_mall.ExchangePack(this, elcid_mall_exchange_pack, dwMallID, byIndex, nPrice, sPackSell);

	// 处理结果
	if(E_Success == dw_error_code	&& sPackSell.nYuanBaoNeed > 0 && VALID_POINT(sPackSell.pItem[0]))
	{
		INT64 n64_serial = sPackSell.pItem[0]->n64_serial;
		INT16 n16BuyNum = sPackSell.pItem[0]->n16Num;
		DWORD dwFstGainTime = g_world.GetWorldTime();

		// 将物品放到玩家背包中
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			if(!VALID_POINT(sPackSell.pItem[i]))
			{
				break;
			}

			GetItemMgr().Add2Bag(sPackSell.pItem[i], elcid_mall_exchange_pack, TRUE);
		}

		// 如果有赠品，则放到百宝贷中
		if(VALID_POINT(sPackSell.pPresent))
		{
			// 百宝袋历史记录
			GetItemMgr().ProcBaiBaoRecord(sPackSell.pPresent->dw_data_id, 
				GetNameID(), INVALID_VALUE, EBBRT_Mall, dwFstGainTime);

			GetItemMgr().Add2BaiBao(sPackSell.pPresent, elcid_mall_exchange_pack_add);
		}

		// 回馈玩家赠点
		if (sPackSell.nExVolumeAssign > 0)
		{
			GetCurMgr().IncExchangeVolume(sPackSell.nExVolumeAssign, elcid_mall_exchange_pack);
		}
	}

	// 发送更新后商店物品 -- 只有刷新物品要更新物品个数
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
// 玩家向元宝交易账户存元宝
//-----------------------------------------------------------------------------
DWORD Role::SaveYB2Account(DWORD dwID, INT n_num)
{
	// 检查玩家背包元宝数量
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
// 玩家向元宝交易账户存金钱
//-----------------------------------------------------------------------------
DWORD Role::SaveSilver2Account(DWORD dwID, INT64 n_num)
{
	// 检查玩家背包金钱数量
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
// 玩家向元宝交易账户取元宝
//-----------------------------------------------------------------------------
DWORD Role::DepositYBAccout(DWORD dwID, INT n_num)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	// 检查账户元宝数量
	if(pYBAccount->GetAccountYB() < n_num)
		return E_Trade_AccountYB_NotEnough;

	// 检查玩家是否提交过出售订单
	tagYuanBaoOrder *pYBOrder = g_tradeYB.GetYBSellOrder(dwID);
	if(VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	pYBAccount->DecAccountYuanBao(n_num, (DWORD)elcid_trade_depossit_yuanbao, TRUE);
	GetCurMgr().IncBagYuanBao(n_num, (DWORD)elcid_trade_depossit_yuanbao);
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// 玩家向元宝交易账户取金钱
//-----------------------------------------------------------------------------
DWORD Role::DepositSilverAccount(DWORD dwID, INT64 n_num)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dwID);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	// 检查账户金钱数量
	if(pYBAccount->GetAccountSilver() < n_num)
		return E_Trade_AccountSilver_NotEnough;

	// 检查玩家是否提交过收购订单
	tagYuanBaoOrder *pYBOrder = g_tradeYB.GetYBBuyOrder(dwID);
	if(VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	pYBAccount->DecAccountSilver(n_num, (DWORD)elcid_trade_deposit_silver, TRUE);
	GetCurMgr().IncBagSilver(n_num, (DWORD)elcid_trade_deposit_silver);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 同步元宝交易初始化信息
//-----------------------------------------------------------------------------
DWORD Role::GetYBTradeInfo()
{
	g_tradeYB.SynBuyPriceList(this);
	g_tradeYB.SynSellPriceList(this);
	g_tradeYB.SynYBAccount(this);
	return E_Success;
}

//-----------------------------------------------------------------------------
// 玩家提交元宝出售订单
//-----------------------------------------------------------------------------
DWORD Role::SubmitSellOrder(DWORD dw_role_id, INT n_num, INT nPrice)
{
	tagYuanBaoAccount* pYBAccount = g_tradeYB.GetYBAccount(dw_role_id);
	if(!VALID_POINT(pYBAccount))
		return INVALID_VALUE;

	if(n_num <= 0 || nPrice <= 0)
		return INVALID_VALUE;
		
	// 是否已经提交过出售订单
	tagYuanBaoOrder* pSellOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	pSellOrder = g_tradeYB.GetYBSellOrder(dw_role_id);
	if(VALID_POINT(pSellOrder))
		return E_Trade_SellOrder_Exit;

	// 交易账户元宝是否足够
	if(pYBAccount->GetAccountYB() < n_num)
		return E_Trade_AccountYB_NotEnough;

	// 交易手续费为总价的2%
	INT nTax = nPrice * n_num * 2 / 100;
	if(nTax < 1)    nTax = 1;

	// 玩家手续费是否足够
	if (GetCurMgr().GetBagSilver() < nTax)
		return E_Trade_Tax_NotEnough;


	tagYuanBaoOrder * pYBOrder = g_tradeYB.CreateYBOrder(dw_role_id, EYBOT_SELL, nPrice, n_num);
	if(!VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	// 设置账户中订单的提交状态
	pYBAccount->SetSellOrder(TRUE);

	// 扣除手续费 
	GetCurMgr().DecBagSilver(nTax, elcid_trade_tax);

	// 出售元宝
	g_tradeYB.DealYBSell(pYBOrder);
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// 玩家提交元宝收购订单
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

	// 是否已近提交过订单
	tagYuanBaoOrder* pBuyOrder = (tagYuanBaoOrder*)INVALID_VALUE;
	pBuyOrder = g_tradeYB.GetYBBuyOrder(dw_role_id);
	if(VALID_POINT(pBuyOrder))
		return E_Trade_BuyOrder_Exit;

	// 交易账户金钱是否足够
	if(pYBAccount->GetAccountSilver() < n_num * nPrice)
		return E_Trade_AccountSilver_NotEnough;

	// 交易手续费为总价的2%
	INT nTax = (FLOAT)(nPrice * n_num) * 0.02f;
	if(nTax < 1)    nTax = 1;

	// 玩家手续费是否足够
	if (GetCurMgr().GetBagSilver() < nTax)
		return E_Trade_Tax_NotEnough;

	tagYuanBaoOrder *pYBOrder = g_tradeYB.CreateYBOrder(dw_role_id, EYBOT_BUY, nPrice, n_num);
	if(!VALID_POINT(pYBOrder))
		return INVALID_VALUE;

	// 设置账户中订单的提交状态
	pYBAccount->SetBuyOrder(TRUE);

	// 扣除手续费 
	GetCurMgr().DecBagSilver(nTax, elcid_trade_tax);

	// 购买元宝
	g_tradeYB.DealYBBuy(pYBOrder);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 删除订单
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
// 查询一周内该玩家的元宝交易订单
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

	// 向数据库发送查询消息
	NET_DB2C_get_role_yuanbao_order	sendDB;
	sendDB.dw_role_id = dw_role_id;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return E_Success;
}