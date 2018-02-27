/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//商城

#include "StdAfx.h"
#include "../../common/WorldDefine/mall_define.h"
#include "../../common/WorldDefine/mall_protocol.h"
#include "../../common/WorldDefine/GMDefine.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/mall_server_define.h"
#include "../common/ServerDefine/log_server_define.h"

#include "mall.h"
#include "att_res.h"
#include "world.h"
#include "role.h"
#include "role_mgr.h"
#include "item_creator.h"
#include "guild.h"
#include "guild_manager.h"
#include "guild_buy.h"

Mall g_mall;
//-----------------------------------------------------------------------------
// 构造&析构
//-----------------------------------------------------------------------------
Mall::Mall()
{
	m_bInitOK		= FALSE;
	m_dwLoadTime	= INVALID_VALUE;
	m_dwTimeKeeper	= INVALID_VALUE;
	m_byMinuteTime	= INVALID_VALUE;

	m_nItemNum		= 0;
	m_nPackNum		= 0;
	m_pMallItem		= NULL;
	m_pMallPack		= NULL;
	m_pMallFreeItem = new tagMallGoods;
	m_mapGuildPurchase.clear();
}

Mall::~Mall()
{
	Destroy();

	SAFE_DELETE(m_pMallFreeItem);
}

//-----------------------------------------------------------------------------
// 初始化数据
//-----------------------------------------------------------------------------
BOOL Mall::Init()
{
	if(is_init_ok())
	{
		return TRUE;
	}
	
	m_dwLoadTime	= g_world.GetWorldTime();
	m_dwTimeKeeper	= g_world.GetWorldTime();
	m_byMinuteTime	= 0;

	m_nItemNum		= AttRes::GetInstance()->GetMallItemNum();
	m_nPackNum		= AttRes::GetInstance()->GetMallPackNum();

	if(m_nItemNum <= 0 && m_nPackNum <= 0)
	{
		ASSERT(0);
		print_message(_T("\nCaution: \n\tMall Res Load Failed! Please Check!\n\n"));
		return FALSE;
	}
	
	if(m_nItemNum > 0)
	{
		m_pMallItem	= new tagMallGoods[m_nItemNum];
		if(!VALID_POINT(m_pMallItem))
		{
			return FALSE;
		}

		InitItem();
	}
	
	if(m_nPackNum > 0)
	{
		m_pMallPack	= new tagMallGoods[m_nPackNum];
		if(!VALID_POINT(m_pMallPack))
		{
			Destroy();
			return FALSE;
		}

		if(!CheckPack())
		{
			print_message(_T("\n\nCaution:\n\tMall pack proto have problem, please check and reopen mall!\n\n\n"));
			Destroy();
			return FALSE;
		}

		InitPack();
	}

	// 初始化免费领取物品
	m_pMallFreeItem->pMallFreeItem = AttRes::GetInstance()->GetMallFreeProto();

	m_bInitOK		= TRUE;

	return m_bInitOK;
}

//-----------------------------------------------------------------------------
// 销毁数据
//-----------------------------------------------------------------------------
VOID Mall::Destroy()
{
	m_bInitOK		= FALSE;
	m_dwLoadTime	= INVALID_VALUE;
	m_dwTimeKeeper	= INVALID_VALUE;
	m_byMinuteTime	= INVALID_VALUE;

	m_nItemNum		= 0;
	m_nPackNum		= 0;
	
	SAFE_DELETE_ARRAY(m_pMallItem);
	SAFE_DELETE_ARRAY(m_pMallPack);
}

//-----------------------------------------------------------------------------
// 实时更新商城
//-----------------------------------------------------------------------------
DWORD Mall::ReInit()
{
	// 关闭商城
	if(is_init_ok())
	{
		Destroy();
	}

	// 重新读取资源文件
	if(!AttRes::GetInstance()->ReloadMallProto())
	{
		m_att_res_caution(_T("mall proto"), _T("NULL"), INVALID_VALUE);
		return EGMMall_LoadProto_Failed;
	}

	// 初始化商城
	if(!Init())
	{
		return EGMMall_Init_Failed;
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 初始化普通商品
//-----------------------------------------------------------------------------
VOID Mall::InitItem()
{
	INT i = 0;
	tagMallItemProto *pProto = NULL;
	package_map<DWORD, tagMallItemProto*> mapMallItem = AttRes::GetInstance()->GetMallItem();
	package_map<DWORD, tagMallItemProto*>::map_iter it = mapMallItem.begin();
	while(mapMallItem.find_next(it, pProto))
	{
		m_pMallItem[i].pMallItem	= pProto;
		m_pMallItem[i].byCurNum		= pProto->byNum;

		++i;
	}

	ASSERT(m_nItemNum == i);
}

//-----------------------------------------------------------------------------
// 初始化礼品包
//-----------------------------------------------------------------------------
VOID Mall::InitPack()
{
	INT i = 0;
	tagMallPackProto *pProto = NULL;
	package_map<DWORD, tagMallPackProto*> mapMallPack = AttRes::GetInstance()->GetMallPack();
	package_map<DWORD, tagMallPackProto*>::map_iter it = mapMallPack.begin();
	while(mapMallPack.find_next(it, pProto))
	{
		m_pMallPack[i].pMallPack	= pProto;
		m_pMallPack[i].byCurNum		= pProto->byNum;

		++i;
	}

	ASSERT(m_nPackNum == i);
}

//-----------------------------------------------------------------------------
// 检查礼品包静态属性是否正确
//-----------------------------------------------------------------------------
BOOL Mall::CheckPack()
{
	tagMallPackProto *pProto = NULL;
	package_map<DWORD, tagMallPackProto*> mapMallPack = AttRes::GetInstance()->GetMallPack();
	package_map<DWORD, tagMallPackProto*>::map_iter it = mapMallPack.begin();
	while(mapMallPack.find_next(it, pProto))
	{
		INT nTotalPrice = 0;
		for(INT i=0; i<MALL_PACK_ITEM_NUM; ++i)
		{
			nTotalPrice += pProto->nItemPrice[i];
		}

		if(nTotalPrice != pProto->nPrice)
		{
			return FALSE;
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// 获取所有物品
//-----------------------------------------------------------------------------
DWORD Mall::GetAllItems(OUT LPVOID pData)
{
	ASSERT(is_init_ok());
	
	M_trans_pointer(p, pData, tagMallItemProto);

	for(INT i=0; i<GetItemNum(); ++i)
	{
		//gx modify 2013.11.22服务器商城的全部静态数据都要同步给客户端
		//// 还没开始
		//if(m_pMallItem[i].pMallItem->dwSaleBegin != INVALID_VALUE && g_world.GetWorldTime() < m_pMallItem[i].pMallItem->dwSaleBegin)
		//{
		//	continue;
		//}

		//// 已经结束
		//if(m_pMallItem[i].pMallItem->dwSaleEnd != INVALID_VALUE && g_world.GetWorldTime() > m_pMallItem[i].pMallItem->dwSaleEnd)
		//{
		//	continue;
		//}

		
		memcpy(&p[i], m_pMallItem[i].pMallItem, sizeof(tagMallItemProto));

		// 将商品在服务器的下标及当前个数放到proto中
		p[i].byIndexInServer = i;
		p[i].byNum = m_pMallItem[i].byCurNum;
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// 获取所有礼品包
//-----------------------------------------------------------------------------
DWORD Mall::GetAllPacks(OUT LPVOID pData)
{
	ASSERT(is_init_ok());
	
	M_trans_pointer(p, pData, tagMallPackProto);

	for(INT i=0; i<GetPackNum(); ++i)
	{
		memcpy(&p[i], m_pMallPack[i].pMallPack, sizeof(tagMallPackProto));

		// 将商品在服务器的下标及当前个数放到proto中
		p[i].byIndexInServer = i;
		p[i].byNum = m_pMallPack[i].byCurNum;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 获取免费物品
//-----------------------------------------------------------------------------
DWORD Mall::GetFreeItem(OUT LPVOID pData)
{
	ASSERT(is_init_ok());
	ASSERT(m_pMallFreeItem != NULL);
	ASSERT(m_pMallFreeItem->pMallFreeItem != NULL);

	M_trans_pointer(p, pData, tagMallFreeItem);

	p->byNum		= m_pMallFreeItem->pMallFreeItem->byNum;
	p->dw_data_id		= m_pMallFreeItem->pMallFreeItem->dw_data_id;
	p->nUnitPrice	= m_pMallFreeItem->pMallFreeItem->nUnitPrice;

	return 0;
}

//-----------------------------------------------------------------------------
// 更新所有物品
//-----------------------------------------------------------------------------
DWORD Mall::UpdateAllItems(OUT LPVOID pData, OUT INT &nRefreshNum)
{
	ASSERT(is_init_ok());

	M_trans_pointer(p, pData, tagMallUpdate);

	nRefreshNum = 0;
	for(INT i=0; i<GetItemNum(); ++i)
	{
		if(!VALID_POINT(m_pMallItem[i].pMallItem))
		{
			// 不应该执行到该处
			ASSERT(0);
			return INVALID_VALUE;
		}
		
		if((BYTE)INVALID_VALUE == m_pMallItem[i].pMallItem->byNum
			|| m_pMallItem[i].byCurNum == m_pMallItem[i].pMallItem->byNum)
		{
			continue;
		}

		p[nRefreshNum].dwID = m_pMallItem[i].pMallItem->dwID;
		p[nRefreshNum].byRemainNum = m_pMallItem[i].byCurNum;
		++nRefreshNum;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 更新所有礼品包
//-----------------------------------------------------------------------------
DWORD Mall::UpdateAllPacks(OUT LPVOID pData, OUT INT &nRefreshNum)
{
	ASSERT(is_init_ok());

	M_trans_pointer(p, pData, tagMallUpdate);

	nRefreshNum = 0;
	for(INT i=0; i<GetPackNum(); ++i)
	{
		if(!VALID_POINT(m_pMallPack[i].pMallPack))
		{
			ASSERT(0);
			return INVALID_VALUE;
		}
		
		if((BYTE)INVALID_VALUE == m_pMallPack[i].pMallPack->byNum
			|| m_pMallPack[i].byCurNum == m_pMallPack[i].pMallPack->byNum)
		{
			continue;
		}

		p[nRefreshNum].dwID = m_pMallPack[i].pMallPack->dwID;
		p[nRefreshNum].byRemainNum = m_pMallPack[i].byCurNum;
		++nRefreshNum;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 出售物品
//-----------------------------------------------------------------------------
DWORD Mall::sell_goods(Role *pRole, DWORD dwToRoleID, DWORD dw_cmd_id, 
					DWORD dwID, BYTE byIndex, INT nUnitPrice, INT16 n16BuyNum, 
					OUT tagMallItemSell &itemSell)
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(is_init_ok());
	ASSERT(nUnitPrice > 0 && n16BuyNum > 0);

	// 检查索引合法性
	if(byIndex >= GetItemNum())
	{
		return INVALID_VALUE;
	}
	const tagMallItemProto *pProto = m_pMallItem[byIndex].pMallItem;
	
	if (!VALID_POINT(pProto))
		return INVALID_VALUE;//非法
	DWORD dw_CurWorldTime = g_world.GetWorldTime();//获取当前时间
	// 还没开始
	if(pProto->dwSaleBegin != INVALID_VALUE && dw_CurWorldTime < pProto->dwSaleBegin)
	{
		return E_MALL_SALE_START;//非法
	}

	// 已经结束
	if(pProto->dwSaleEnd != INVALID_VALUE && dw_CurWorldTime > pProto->dwSaleEnd)
	{
		return E_MALL_SALE_END;//非法
	}
	//判断购买数是否超过了该物品的最大堆叠数 gx add
	tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(pProto->dw_data_id);
	if (!VALID_POINT(pItemProto))//不是物品，则判断是否是装备
	{
		tagEquipProto* pEquipProto = AttRes::GetInstance()->GetEquipProto(pProto->dw_data_id);
		if (!VALID_POINT(pEquipProto))
		{
			return INVALID_VALUE;//非法
		}
		else
		{
			if (n16BuyNum != 1)
			{
				return INVALID_VALUE;//装备只能买一个
			}
		}
	}
	if (VALID_POINT(pItemProto))
	{
		if (n16BuyNum > pItemProto->n16MaxLapNum)
			return INVALID_VALUE;//客户端已做判断，若走到这里，必是非法
	}
	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// 个数
	if(m_pMallItem[byIndex].byCurNum != (BYTE)INVALID_VALUE && m_pMallItem[byIndex].byCurNum < n16BuyNum)
	{
		itemSell.byRemainNum = m_pMallItem[byIndex].byCurNum;
		return E_Mall_Item_NotEnough;
	}
	
	INT nPrice = 0;
	
	// 单价
	if((pProto->dwTimeSaleEnd != INVALID_VALUE && pProto->dwTimeSaleStart != INVALID_VALUE) && (dw_CurWorldTime > pProto->dwTimeSaleStart && dw_CurWorldTime < pProto->dwTimeSaleEnd))
	{
		// 促销
		nPrice = pProto->nSalePrice;
	}
	else
	{
		// 正常
		nPrice = pProto->nPrice;
	}

	// 金钱
	if(nPrice != nUnitPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	nPrice *= n16BuyNum;
	
	//判断是元宝购买还是荣誉点兑换，通过商品所属分类区分
	if (Exchange_Goods_Index != pProto->n8Kind)
	{
		// 检查玩家元宝是否足够
		if(nPrice > pRole->GetCurMgr().GetBaiBaoYuanBao() || nPrice <= 0)
		{
			return E_BagYuanBao_NotEnough;
		}
	}
	else
	{
		//检查玩家荣誉点是否足够
		if (nPrice > pRole->GetCurMgr().GetExploits() || nPrice <= 0)
		{
			return E_BagSilver_NotEnough;
		}
	}

	// 创建物品
	tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id, n16BuyNum, EIQ_Quality0);
	if(!VALID_POINT(pItemNew))
	{
		ASSERT(VALID_POINT(pItemNew));
		return E_Mall_CreateItem_Failed;
	}

	pItemNew->dw1stGainTime	 = g_world.GetWorldTime();

	// 如果有赠品，则生成赠品
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
							pProto->dwPresentID, (INT16)pProto->byPresentNum * n16BuyNum, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			::Destroy(pItemNew);
			ASSERT(VALID_POINT(pPresentNew));
			return E_Mall_CreatePres_Failed;
		}

		pPresentNew->dw1stGainTime	= g_world.GetWorldTime();
	}

	// 回馈玩家赠点
	if (pProto->byExAssign >= 0)
	{
		itemSell.nExVolumeAssign = pProto->byExAssign * n16BuyNum;
	}

	// 更新商店中物品个数
	if(m_pMallItem[byIndex].byCurNum != (BYTE)INVALID_VALUE)
	{
		m_pMallItem[byIndex].byCurNum -= n16BuyNum;
	}

	if (Exchange_Goods_Index != pProto->n8Kind)
	{
		// 从玩家背包中扣除元宝
		pRole->GetCurMgr().DecBaiBaoYuanBao(nPrice, dw_cmd_id);

		// log
		LogMallSell(pRole->GetID(), dwToRoleID, *pItemNew, 
			pItemNew->n64_serial, n16BuyNum, pItemNew->dw1stGainTime, nPrice, 0, dw_cmd_id);
	}
	else
	{
		//扣除相应的荣誉点
		pRole->GetCurMgr().DecExploits(nPrice,dw_cmd_id);
		// log
		LogMallSell(pRole->GetID(), dwToRoleID, *pItemNew, 
			pItemNew->n64_serial, n16BuyNum, pItemNew->dw1stGainTime, 0, nPrice, dw_cmd_id);
	}

	// 设置传出参数
	itemSell.pItem			= pItemNew;
	itemSell.pPresent		= pPresentNew;
	itemSell.nYuanBaoNeed	= nPrice;
	itemSell.byRemainNum	= m_pMallItem[byIndex].byCurNum;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 出售礼品包
//-----------------------------------------------------------------------------
DWORD Mall::SellPack(Role *pRole, DWORD dwToRoleID, DWORD dw_cmd_id, DWORD dwID, 
					BYTE byIndex, INT nUnitPrice,  OUT tagMallPackSell &packSell, 
					BOOL bNeedCheckBagSpace)
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(is_init_ok());
	ASSERT(nUnitPrice > 0);

	// 检查索引合法性
	if(byIndex >= GetPackNum())
	{
		return INVALID_VALUE;
	}

	const tagMallPackProto *pProto = m_pMallPack[byIndex].pMallPack;

	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// 个数
	if(m_pMallPack[byIndex].byCurNum != (BYTE)INVALID_VALUE && m_pMallPack[byIndex].byCurNum < 1)
	{
		return E_Mall_Pack_NotEnough;
	}

	INT nPrice = 0;

	// 单价
	if(pProto->dwTimeSaleEnd != INVALID_VALUE && g_world.GetWorldTime() < pProto->dwTimeSaleEnd)
	{
		// 促销
		nPrice = pProto->nSalePrice;
	}
	else
	{
		// 正常
		nPrice = pProto->nPrice;
	}

	// 金钱
	if(nPrice != nUnitPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	// 检查玩家元宝是否足够
	if(nPrice > pRole->GetCurMgr().GetBaiBaoYuanBao())
	{
		return E_BagYuanBao_NotEnough;
	}

	// 检查玩家背包空间
	if(bNeedCheckBagSpace && pProto->n8ItemKind > pRole->GetItemMgr().GetBagFreeSize())
	{
		return E_Bag_NotEnoughSpace;
	}

	// 如果有赠品，则先生成赠品 -- 方便生成物品失败时，内存释放处理
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
							pProto->dwPresentID, pProto->byPresentNum, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			ASSERT(VALID_POINT(pPresentNew));
			print_message(_T("\n\nCaution:\n"));
			print_message(_T("\tThere is critical error in proto of mall pack or item&equip!!!!!!!!!\n\n"));
			return E_Mall_CreatePres_Failed;
		}

		pPresentNew->dw1stGainTime = g_world.GetWorldTime();
	}
	
	// 创建物品
	INT i = 0;
	for(; i<pProto->n8ItemKind; ++i)
	{
		tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id[i], pProto->byItemNum[i], EIQ_Quality0);
		if(!VALID_POINT(pItemNew))
		{
			ASSERT(VALID_POINT(pItemNew));
			print_message(_T("\n\nCaution:\n"));
			print_message(_T("\tThere is critical error in proto of mall pack or item&equip!!!!!!!!!\n\n"));
			break;
		}
		else
		{
			pItemNew->dw1stGainTime = g_world.GetWorldTime();
			packSell.pItem[i] = pItemNew;
		}
	}

	// 检查物品是否均创建成功
	if(i != pProto->n8ItemKind)
	{
		// 是否已创建成功物品内存
		for(INT j=0; j<i; ++j)
		{
			::Destroy(packSell.pItem[j]);
		}

		// 赠品
		if(VALID_POINT(pPresentNew))
		{
			::Destroy(pPresentNew);
		}

		return E_Mall_CreateItem_Failed;
	}

	// 回馈玩家赠点
	if (pProto->byExAssign >= 0)
	{
		packSell.nExVolumeAssign = pProto->byExAssign;
	}

	// 更新商店中物品个数
	if(m_pMallPack[byIndex].byCurNum != (BYTE)INVALID_VALUE)
	{
		--m_pMallPack[byIndex].byCurNum;
	}

	// 从玩家背包中扣除元宝
	pRole->GetCurMgr().DecBaiBaoYuanBao(nPrice, dw_cmd_id);

	// 礼品包log
	LogMallSellPack(dwID, pRole->GetID(), dwToRoleID, nPrice);

	// 礼品包内物品log
	for(i=0; i<pProto->n8ItemKind; ++i)
	{
		LogMallSell(pRole->GetID(), dwToRoleID, *packSell.pItem[i], 
			packSell.pItem[i]->n64_serial, packSell.pItem[i]->n16Num, 
			packSell.pItem[i]->dw1stGainTime, pProto->nItemPrice[i], 0, dw_cmd_id);
	}

	// 设置传出参数
	packSell.pPresent		= pPresentNew;
	packSell.nYuanBaoNeed	= nPrice;
	packSell.byRemainNum	= m_pMallPack[byIndex].byCurNum;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 免费发放
//-----------------------------------------------------------------------------
DWORD Mall::GrantFreeItem(Role *pRole, DWORD dwID, OUT tagMallItemSell &itemSell)
{
	// 是否有免费物品
	if(GetFreeItemNum() <= 0)
	{
		return E_Mall_Free_NotExist;
	}

	// 检查玩家当天是否已经领取过了
	tagDWORDTime dwCurDay = g_world.GetWorldTime();
	dwCurDay.hour	= 0;
	dwCurDay.min	= 0;
	dwCurDay.sec	= 0;

	if(pRole->GetLastMallFreeTime() >= dwCurDay)
	{
		return E_Mall_Free_AlreadyGet;
	}

	// 生成免费物品
	tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
							m_pMallFreeItem->pMallFreeItem->dw_data_id, 
							m_pMallFreeItem->pMallFreeItem->byNum, EIQ_Quality0);
	if(!VALID_POINT(pItemNew))
	{
		ASSERT(VALID_POINT(pItemNew));
		print_message(_T("\n\nCaution:\n"));
		print_message(_T("\tThere is critical error in proto of mall free item!!!!!!!!!\n\n"));
		return E_Mall_CreateItem_Failed;
	}

	// 设置领取时间
	pRole->SetLastGetMallFreeTime(dwCurDay);

	// 设置传出参数
	itemSell.pItem = pItemNew;

	return E_Success;
}
//-----------------------------------------------------------------------------
// 载入所有团购信息
//-----------------------------------------------------------------------------
DWORD Mall::LoadAllGPInfo( INT nGPInfoNum, LPVOID pData )
{
	M_trans_pointer(pGPInfo, pData, s_gp_info);

	for (int i=0; i<nGPInfoNum; i++)
	{
		guild_buy* pMapGPInfo = m_mapGuildPurchase.find(pGPInfo->dw_guild_id);
		if (!VALID_POINT(pMapGPInfo))
		{
			pMapGPInfo = new guild_buy;
			if (!VALID_POINT(pMapGPInfo) || !pMapGPInfo->init(pGPInfo->dw_guild_id))
			{
				ASSERT(VALID_POINT(pMapGPInfo));
				print_message(_T("\npMapGPInfo Create Faild in LoadAllGPInfo.\n"));
				SAFE_DELETE(pMapGPInfo);
				goto Next_GPInfo;
			}
			else
			{
				m_mapGuildPurchase.add(pGPInfo->dw_guild_id, pMapGPInfo);
			}
		}

		// 建立新的团购信息
		tagGroupPurchase* pTmpGPInfo	= new tagGroupPurchase;
		pTmpGPInfo->dwGuildID			= pGPInfo->dw_guild_id;
		pTmpGPInfo->dw_role_id			= pGPInfo->dw_role_id;
		pTmpGPInfo->dwMallID			= pGPInfo->dw_mall_id;
		pTmpGPInfo->nPrice				= pGPInfo->n_price;
		pTmpGPInfo->nParticipatorNum	= pGPInfo->n_participator_num;
		pTmpGPInfo->nRequireNum			= pGPInfo->n_require_num;
		pTmpGPInfo->dwRemainTime		= pGPInfo->n_remain_time;

		pTmpGPInfo->listParticipators	= new package_list<DWORD>;
		if (!VALID_POINT(pTmpGPInfo->listParticipators))
		{
			ASSERT(VALID_POINT(pTmpGPInfo->listParticipators));
			print_message(_T("\nlistParticipators Create Faild in LoadAllGPInfo.\n"));
			SAFE_DELETE(pTmpGPInfo);
			goto Next_GPInfo;
		}

		for (int j=0; j<pGPInfo->n_participator_num; j++)
		{
			pTmpGPInfo->listParticipators->push_back(pGPInfo->dw_participators[j]);
		}
		
		// 加入信息到管理表
		if (!pMapGPInfo->add(pTmpGPInfo, FALSE))
		{
			SAFE_DELETE(pTmpGPInfo);
			goto Next_GPInfo;
		}

Next_GPInfo:
		// 将pGPInfo进行位移
		pGPInfo = (s_gp_info*)((BYTE*)pGPInfo + sizeof(s_gp_info) + (pGPInfo->n_participator_num-1)*sizeof(DWORD));
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 发起团购
//-----------------------------------------------------------------------------
DWORD Mall::lauch_guild_buy(Role *pRole, DWORD dwID, BYTE byScope,
								BYTE byIndex, INT nUnitPrice)
{
	// 判断商城是否开放
	if(!is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if (!pRole->GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	tagGroupPurchase* pGPInfo = NULL;
	DWORD dwItemTypeID = INVALID_VALUE;

	// 取得帮派团购信息
	DWORD dwGuildID = pRole->GetGuildID();
	if (!VALID_VALUE(dwGuildID))
	{
		return E_GroupPurchase_NotInGuild;
	}

	guild_buy* pMapGPInfo = m_mapGuildPurchase.find(dwGuildID);
	if (!VALID_POINT(pMapGPInfo))
	{
		// 该帮派还没有团购信息
		pMapGPInfo = new guild_buy;
		if (!VALID_POINT(pMapGPInfo) || !pMapGPInfo->init(dwGuildID))
		{
			SAFE_DELETE(pMapGPInfo);
			return E_GroupPurchase_NotMember;
		}

		// 加入到管理表中
		m_mapGuildPurchase.add(dwGuildID, pMapGPInfo);
	}

	// 检查数据合法性并管理团购信息
	DWORD dw_error_code = pMapGPInfo->lauch_guild_buy(pRole, dwID, byScope, byIndex, nUnitPrice, pGPInfo, dwItemTypeID);

	// 处理结果
	if(E_Success == dw_error_code && pGPInfo->nPrice > 0)
	{
		// 从玩家背包中扣除元宝
		pRole->GetCurMgr().DecBaiBaoYuanBao(pGPInfo->nPrice, elcid_group_purchase_buy_item);

		// 帮派内广播团购发起成功消息
		guild* tmpGuild = g_guild_manager.get_guild(pGPInfo->dwGuildID);
		if (!VALID_POINT(tmpGuild)) //以防万一，再判断一下
		{
			ASSERT(tmpGuild);

			// 有错误，就不发了
			return dw_error_code;
		}

		NET_SIS_respond_broad send;
		send.eType = ERespondBroadCast_Launch;
		send.dw_role_id = pGPInfo->dw_role_id;
		send.dw_data_id = dwItemTypeID;

		tmpGuild->send_guild_message(&send, send.dw_size);
	}

	// 团购所需人数应该不是一个人，所以在此处不做达成条件判断

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 响应团购
//-----------------------------------------------------------------------------
DWORD Mall::respond_guild_buy(Role *pRole, DWORD dwGuildID, DWORD dwID, DWORD dw_role_id, INT nPrice)
{
	// 判断商城是否开放
	if(!is_init_ok())
	{
		return E_Mall_Close;
	}

	// 行囊是否解锁
	if(!pRole->GetRoleStateEx().IsInState(ERSE_BagPsdPass))
	{
		return E_Con_PswNotPass;
	}

	tagGroupPurchase* pGPInfo = NULL;

	// 检查数据合法性并管理团购信息
	guild_buy* pMapGPInfo = m_mapGuildPurchase.find(dwGuildID);
	if (!VALID_POINT(pMapGPInfo))
	{
		return E_GroupPurchase_NoInfo;
	}

	DWORD dw_error_code = pMapGPInfo->respond_guild_buy(pRole, dwID, dw_role_id, nPrice, pGPInfo);

	BOOL bCompelete = FALSE;

	// 处理结果
	if(E_Success == dw_error_code && pGPInfo->nPrice > 0)
	{
		// 从玩家背包中扣除元宝
		pRole->GetCurMgr().DecBaiBaoYuanBao(pGPInfo->nPrice, elcid_group_purchase_buy_item);

		// 更新响应者列表
		pGPInfo->listParticipators->push_back(pRole->GetID());
		pGPInfo->nParticipatorNum++;

		if (pGPInfo->nParticipatorNum == pGPInfo->nRequireNum)
		{
			bCompelete = TRUE;
		}
		else
		{
			pMapGPInfo->update_respond_info_to_db(pGPInfo);
		}
	}

	// 团购成功
	if (bCompelete)
	{
		pMapGPInfo->guild_buy_success(pGPInfo);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取指定帮派的团购简易信息
//-----------------------------------------------------------------------------
DWORD Mall::get_all_guild_buy_info(Role* pRole)
{
	ASSERT(VALID_POINT(pRole));
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if (!is_init_ok())
	{
		return E_Mall_Close;
	}

	DWORD dw_error_code = E_Success;
	DWORD dwGuildID = pRole->GetGuildID();

	// 检验帮派的合法性
	if (!VALID_VALUE(dwGuildID))
	{
		dw_error_code = E_GroupPurchase_NotInGuild;
	}

	guild* tmpGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(tmpGuild))
	{
		dw_error_code = E_GroupPurchase_NotInGuild;
	}

	guild_buy* pMapGPInfo = m_mapGuildPurchase.find(dwGuildID);
	if (!VALID_POINT(pMapGPInfo))
	{
		dw_error_code = E_GroupPurchase_NoInfo;
	}

	if (dw_error_code == E_Success)
	{
		// 读取该帮派的团购信息
		INT nGPInfoNum = pMapGPInfo->get_guild_buy_info_num();
		INT nMsgSize = sizeof(NET_SIS_guild_buy_info)+(nGPInfoNum-1)*sizeof(tagSimGPInfo);
		CREATE_MSG(pSend, nMsgSize, NET_SIS_guild_buy_info);

		pSend->nGroupPurchase	= tmpGuild->get_guild_att().nGroupPurchase;
		pSend->nGPInfoNum		= nGPInfoNum;		

		if (nGPInfoNum > 0)
		{
			pSend->dw_error_code = dw_error_code = pMapGPInfo->get_all_guild_buy_info(nGPInfoNum, pSend->simData);

			if (pSend->nGPInfoNum != nGPInfoNum)
			{
				// 重新计算大小
				pSend->dw_size		= sizeof(NET_SIS_guild_buy_info) + (nGPInfoNum - 1) * sizeof(tagSimGPInfo);
				pSend->nGPInfoNum	= nGPInfoNum;
			}
		}

		pRole->SendMessage(pSend, pSend->dw_size);

		MDEL_MSG(pSend);
	}
	else
	{
		NET_SIS_guild_buy_info send;
		if ((dw_error_code == E_GroupPurchase_NoInfo) && VALID_POINT(tmpGuild))
		{
			send.dw_error_code	= E_Success;
			send.nGroupPurchase	= tmpGuild->get_guild_att().nGroupPurchase;
			send.nGPInfoNum		= 0;
		}
		else
		{
			send.dw_error_code = dw_error_code;
		}
		pRole->SendMessage(&send, send.dw_size);
	}


	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取指定团购信息的响应者列表
//-----------------------------------------------------------------------------
DWORD Mall::get_response(Role* pRole, DWORD dwGuildID, DWORD dwID, DWORD dw_role_id)
{
	ASSERT(VALID_POINT(pRole));
	if (!VALID_POINT(pRole) || pRole->GetGuildID() != dwGuildID)
	{
		return INVALID_VALUE;
	}

	if (!is_init_ok())
	{
		return E_Mall_Close;
	}

	DWORD dw_error_code = E_Success;

	if (!VALID_VALUE(dwGuildID) || !VALID_VALUE(dw_role_id))
	{
		dw_error_code = E_GroupPurchase_NoInfo;
	}

	// 读取响应者
	if (dw_error_code == E_Success)
	{
		// 确定帮派
		guild_buy* pMapGPInfo = m_mapGuildPurchase.find(dwGuildID);
		INT nParticipatorNum = 0;
		if (VALID_POINT(pMapGPInfo))
		{
			nParticipatorNum = pMapGPInfo->get_response_num(dwID, dw_role_id);
		}

		CREATE_MSG(pSend, sizeof(NET_SIS_get_participators)+(nParticipatorNum-1)*sizeof(DWORD), NET_SIS_get_participators);

		pSend->nGroupPurchaseKey = dw_role_id;
		pSend->nGroupPurchaseKey = (pSend->nGroupPurchaseKey << 32) | dwID;
		pSend->nParticipatorNum = nParticipatorNum;

		if (nParticipatorNum > 0)
		{
			// 此时也保证了pMapGPInfo的合法性
			pSend->dw_error_code = dw_error_code = pMapGPInfo->get_response(dwID, dw_role_id, pSend->dwParticipators);
		}
		else
		{
			pSend->dw_error_code = dw_error_code = E_GroupPurchase_NoInfo;
		}

		pRole->SendMessage(pSend, pSend->dw_size);
	}
	else
	{
		NET_SIS_get_participators send;
		send.dw_error_code = dw_error_code;
		send.nParticipatorNum = 0;
		send.nGroupPurchaseKey = dw_role_id;
		send.nGroupPurchaseKey = (send.nGroupPurchaseKey << 32) | dwID;
		pRole->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 更新团购信息
//-----------------------------------------------------------------------------
VOID Mall::Update()
{
	// 商城关闭时停止计时
	if(!is_init_ok())
	{
		return;
	}

	// 计时更新(每秒更新)
	if (m_dwTimeKeeper == g_world.GetWorldTime())
		return;

	DWORD dwSecTime = g_world.GetWorldTime() - m_dwTimeKeeper;
	m_dwTimeKeeper = g_world.GetWorldTime();

	DWORD dwGuildID = INVALID_VALUE;
	guild_buy* pMapGPInfo = NULL;

	INT64 nMapKey = INVALID_VALUE;
	tagGroupPurchase *pGPInfo = NULL;

	package_map<DWORD, guild_buy*>::map_iter iter = m_mapGuildPurchase.begin();
	while (m_mapGuildPurchase.find_next(iter, dwGuildID, pMapGPInfo))
	{
		if (!VALID_POINT(pMapGPInfo))
		{
			m_mapGuildPurchase.erase(dwGuildID);
			continue;
		}

		// 每个帮派更新
		pMapGPInfo->update(dwSecTime);

		// 帮派团购信息无效
		if (pMapGPInfo->is_empty())
		{
			m_mapGuildPurchase.erase(dwGuildID);
			SAFE_DELETE(pMapGPInfo);
			continue;
		}
	}

	// 通知数据库更新所有数据的剩余时间(每分钟更新)
	// World和DB之间有最大一分钟的误差，即数据库中可能存在剩余时间小于0的信息
	// 此类信息如果读入World将在下一个Tick删除
	if (++m_byMinuteTime > 60)
	{
		m_byMinuteTime = 0;

		NET_DB2C_update_gp_time send;
		g_dbSession.Send(&send, send.dw_size);
	}
}

//-----------------------------------------------------------------------------
// 兑换商城物品
//-----------------------------------------------------------------------------
DWORD Mall::ExchangeItem( Role *pRole, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
						 INT nPrice, INT16 n16BuyNum, OUT tagMallItemSell &itemSell )
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(is_init_ok());
	ASSERT(nPrice > 0 && n16BuyNum > 0);

	// 检查索引合法性
	if(byIndex >= GetItemNum())
	{
		return INVALID_VALUE;
	}

	const tagMallItemProto *pProto = m_pMallItem[byIndex].pMallItem;

	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// 个数
	if(m_pMallItem[byIndex].byCurNum != (BYTE)INVALID_VALUE && m_pMallItem[byIndex].byCurNum < n16BuyNum)
	{
		// 返回当前剩余数量
		itemSell.byRemainNum = m_pMallItem[byIndex].byCurNum;
		return E_Mall_Item_NotEnough;
	}

	BYTE nExVolume = pProto->byExNum;

	if (nExVolume == (BYTE)INVALID_VALUE)
	{
		// 该商品不可兑换
		return E_Mall_Exchange_NotAllowable;
	}

	// 赠点
	if(nPrice != nExVolume)
	{
		return E_Mall_ExVolume_Error;
	}

	// 计算总价
	nExVolume *= n16BuyNum;

	// 检查玩家赠点是否足够
	if(nExVolume > pRole->GetCurMgr().GetExchangeVolume() || nExVolume <= 0)
	{
		return E_ExVolume_NotEnough;
	}

	// 创建物品
	tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id, n16BuyNum, EIQ_Quality0);
	if(!VALID_POINT(pItemNew))
	{
		ASSERT(VALID_POINT(pItemNew));
		return E_Mall_CreateItem_Failed;
	}

	pItemNew->dw1stGainTime = g_world.GetWorldTime();

	// 如果有赠品，则生成赠品
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
			pProto->dwPresentID, (INT16)pProto->byPresentNum*n16BuyNum, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			::Destroy(pItemNew);
			ASSERT(VALID_POINT(pPresentNew));
			return E_Mall_CreatePres_Failed;
		}

		pPresentNew->dw1stGainTime = g_world.GetWorldTime();
	}

	// 回馈玩家赠点
	if (pProto->byExAssign >= 0)
	{
		itemSell.nExVolumeAssign = pProto->byExAssign * n16BuyNum;
	}

	// 更新商店中物品个数
	if(m_pMallItem[byIndex].byCurNum != (BYTE)INVALID_VALUE)
	{
		m_pMallItem[byIndex].byCurNum -= n16BuyNum;
	}

	// 扣除玩家的赠点
	pRole->GetCurMgr().DecExchangeVolume(nExVolume, dw_cmd_id);

	// log
	LogMallSell(pRole->GetID(), pRole->GetID(), *pItemNew, 
		pItemNew->n64_serial, n16BuyNum, pItemNew->dw1stGainTime, 0, nExVolume, dw_cmd_id);

	// 设置传出参数
	itemSell.pItem			= pItemNew;
	itemSell.pPresent		= pPresentNew;
	itemSell.nYuanBaoNeed	= nExVolume;
	itemSell.byRemainNum	= m_pMallItem[byIndex].byCurNum;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 兑换商城打包物品
//-----------------------------------------------------------------------------
DWORD Mall::ExchangePack( Role *pRole, DWORD dw_cmd_id, DWORD dwID, BYTE byIndex, 
						 INT nPrice, OUT tagMallPackSell &packSell )
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(is_init_ok());
	ASSERT(nPrice > 0);

	// 检查索引合法性
	if(byIndex >= GetPackNum())
	{
		return INVALID_VALUE;
	}

	const tagMallPackProto *pProto = m_pMallPack[byIndex].pMallPack;

	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// 个数
	if(m_pMallPack[byIndex].byCurNum != (BYTE)INVALID_VALUE && m_pMallPack[byIndex].byCurNum < 1)
	{
		return E_Mall_Pack_NotEnough;
	}

	INT nExVolume = pProto->byExNum;
	if (nExVolume == (BYTE)INVALID_VALUE)
	{
		return E_Mall_Exchange_NotAllowable;
	}

	// 赠点
	if(nPrice != nExVolume)
	{
		return E_Mall_ExVolume_Error;
	}

	// 检查玩家赠点是否足够
	if(nExVolume > pRole->GetCurMgr().GetExchangeVolume() || nExVolume <= 0)
	{
		return E_ExVolume_NotEnough;
	}

	// 检查玩家背包空间
	if(pProto->n8ItemKind > pRole->GetItemMgr().GetBagFreeSize())
	{
		return E_Bag_NotEnoughSpace;
	}

	// 如果有赠品，则先生成赠品 -- 方便生成物品失败时，内存释放处理
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
			pProto->dwPresentID, pProto->byPresentNum, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			ASSERT(VALID_POINT(pPresentNew));
			print_message(_T("\n\nCaution:\n"));
			print_message(_T("\tThere is critical error in proto of mall pack or item&equip!!!!!!!!!\n\n"));
			return E_Mall_CreatePres_Failed;
		}
	}

	// 创建物品
	INT i = 0;
	for(; i<pProto->n8ItemKind; ++i)
	{
		tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id[i], pProto->byItemNum[i], EIQ_Quality0);
		if(!VALID_POINT(pItemNew))
		{
			ASSERT(VALID_POINT(pItemNew));
			print_message(_T("\n\nCaution:\n"));
			print_message(_T("\tThere is critical error in proto of mall pack or item&equip!!!!!!!!!\n\n"));
			break;
		}
		else
		{
			packSell.pItem[i] = pItemNew;
		}
	}

	// 检查物品是否均创建成功
	if(i != pProto->n8ItemKind)
	{
		// 是否已创建成功物品内存
		for(INT j=0; j<i; ++j)
		{
			::Destroy(packSell.pItem[j]);
		}

		// 赠品
		if(VALID_POINT(pPresentNew))
		{
			::Destroy(pPresentNew);
		}

		return E_Mall_CreateItem_Failed;
	}

	// 回馈玩家赠点
	if (pProto->byExAssign >= 0)
	{
		packSell.nExVolumeAssign = pProto->byExAssign;
	}

	// 更新商店中物品个数
	if(m_pMallPack[byIndex].byCurNum != (BYTE)INVALID_VALUE)
	{
		--m_pMallPack[byIndex].byCurNum;
	}

	// 扣除玩家赠点
	pRole->GetCurMgr().DecExchangeVolume(nExVolume, dw_cmd_id);

	// 记录到第一个物品上
	LogMallSell(pRole->GetID(), pRole->GetID(), *packSell.pItem[0], 
				packSell.pItem[0]->n64_serial, packSell.pItem[0]->n16Num, 
				packSell.pItem[0]->dw1stGainTime, 0, nExVolume, dw_cmd_id);

	// 设置传出参数
	packSell.pPresent		= pPresentNew;
	packSell.nYuanBaoNeed	= nExVolume;
	packSell.byRemainNum	= m_pMallPack[byIndex].byCurNum;

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获取商城物品信息
//-----------------------------------------------------------------------------
const tagMallGoods* Mall::GetMallItem( BYTE byIndex, EMallItemType eType /*= EMIT_Item*/ )
{
	tagMallGoods* pMallGoods = NULL;

	switch (eType)
	{
	case EMIT_Item:
		if (byIndex < GetItemNum())
		{
			pMallGoods = &m_pMallItem[byIndex];
		}
		break;

	case EMIT_Pack:
		if (byIndex < GetPackNum())
		{
			pMallGoods = &m_pMallPack[byIndex];
		}
		break;

	case EMIT_FreeItem:
		if (GetFreeItemNum() != 0)
		{
			pMallGoods = m_pMallFreeItem;
		}
		break;
	}

	return pMallGoods;
}

//-----------------------------------------------------------------------------
// 删除帮派团购信息(帮派解散)
//-----------------------------------------------------------------------------
VOID Mall::RemoveGuildPurchaseInfo( DWORD dwGuildID )
{
	if (!VALID_VALUE(dwGuildID))
	{
		return;
	}

	guild_buy* pGPInfo = m_mapGuildPurchase.find(dwGuildID);
	if (!VALID_POINT(pGPInfo))
	{
		return;
	}

	pGPInfo->remove_guild_buy_info();
}

//-----------------------------------------------------------------------------
// 向db发消息，要求记录log
//-----------------------------------------------------------------------------
VOID Mall::LogMallSell(DWORD dwBuyRoleID, DWORD dwToRoleID, const tagItem& item, 
					   INT64 n64_serial, INT16 n16Num, DWORD dwFstGainTime, 
					   INT nCostYuanBao, INT nCostExVolume, DWORD dw_cmd_id)
{
	NET_DB2C_log_mall_sell send;

	send.s_log_mall_sell_.dw_data_id		= item.dw_data_id;
	send.s_log_mall_sell_.dw_exist_item	= item.pProtoType->dwTimeLimit;
	send.s_log_mall_sell_.n_max_use_times	= item.pProtoType->nMaxUseTimes;

	send.s_log_mall_sell_.dw_role_id		= dwBuyRoleID;
	send.s_log_mall_sell_.dw_to_role_id	= dwToRoleID;
	send.s_log_mall_sell_.n64_serial		= n64_serial;
	send.s_log_mall_sell_.dw_first_gain_time	= dwFstGainTime;
	send.s_log_mall_sell_.n_cost_yuanbao	= nCostYuanBao;
	send.s_log_mall_sell_.n_cost_ex_volume	= nCostExVolume;
	send.s_log_mall_sell_.dw_cmd_id		= dw_cmd_id;
	send.s_log_mall_sell_.n16_num_sell	= n16Num;

	g_dbSession.Send(&send, send.dw_size);
}

//-----------------------------------------------------------------------------
// 向db发消息，要求记录礼品包log
//-----------------------------------------------------------------------------
VOID Mall::LogMallSellPack(DWORD dwPackID, DWORD dwBuyRoleID, DWORD dwToRoleID, INT nCostYuanBao)
{
	NET_DB2C_log_mall_sell_pack send;
	send.s_log_mall_sell_pack_.dw_pack_id		= dwPackID;
	send.s_log_mall_sell_pack_.dw_buy_role_id	= dwBuyRoleID;
	send.s_log_mall_sell_pack_.dw_to_role_id	= dwToRoleID;
	send.s_log_mall_sell_pack_.n_cost_yuanbao	= nCostYuanBao;

	g_dbSession.Send(&send, send.dw_size);
}