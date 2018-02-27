/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/
//帮会团购


#include "StdAfx.h"

#include "../WorldDefine/mall_define.h"
#include "../WorldDefine/msg_mall.h"
#include "../WorldDefine/ItemDefine.h"

#include "../ServerDefine/log_server_define.h"
#include "../ServerDefine/mall_server_define.h"
#include "../ServerDefine/log_server_define.h"

#include "guild_purchase.h"
#include "guild.h"
#include "att_res.h"
#include "item_creator.h"
#include "mall.h"
#include "role_mgr.h"
#include "currency.h"
#include "role.h"
#include "guild_manager.h"

//-------------------------------------------------------------------------------------------------------
// 构造与析构
//-------------------------------------------------------------------------------------------------------
GuildPurchase::GuildPurchase()
{
	m_pGuild = NULL;
	m_mapGPInfo.clear();
}

GuildPurchase::~GuildPurchase()
{
	Destory();
}

//-------------------------------------------------------------------------------------------------------
// 初始化、更新、销毁
//-------------------------------------------------------------------------------------------------------
BOOL GuildPurchase::Init( DWORD dwGuildID )
{
	m_mapGPInfo.clear();

	// 上层需保证pGuild的合法性
	ASSERT(VALID_VALUE(dwGuildID));
	m_pGuild = g_guild_manager.get_guild(dwGuildID);
	if (!VALID_POINT(m_pGuild))
	{
		return FALSE;
	}

	return TRUE;
}

VOID GuildPurchase::Update(DWORD dw_time)
{
	INT64 nMapKey = INVALID_VALUE;
	tagGroupPurchase* pGPInfo = NULL;

	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, nMapKey, pGPInfo))
	{
		// 超过24小时的团购失败
		pGPInfo->dwRemainTime -= dw_time;
		if ((INT32)(pGPInfo->dwRemainTime) <= 0)
		{
			RemoveGroupPurchaseInfo(pGPInfo, FALSE);
		}
	}

	// 上层判断map是否为空
}

VOID GuildPurchase::Destory()
{
	tagGroupPurchase* pGPInfo = NULL;
	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, pGPInfo))
	{
		SAFE_DELETE(pGPInfo);
	}
	m_mapGPInfo.clear();
}

//-------------------------------------------------------------------------------------------------------
// 团购信息管理
//-------------------------------------------------------------------------------------------------------
BOOL GuildPurchase::Add( tagGroupPurchase* pGPInfo, BOOL bNotify2DB /*= TRUE*/ )
{
	ASSERT(pGPInfo);
	if (!VALID_POINT(pGPInfo))
	{
		return FALSE;
	}

	// 验证帮派
	if (pGPInfo->dwGuildID != m_pGuild->get_guild_att().dwID)
	{
		return FALSE;
	}

	// 插入
	INT64 nKey = GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID);
	if (m_mapGPInfo.is_exist(nKey))
	{
		return FALSE;
	}

	m_mapGPInfo.add(nKey, pGPInfo);

	// 通知数据库
	if (bNotify2DB)
	{
		AddGPInfo2DB(pGPInfo);
	}

	return TRUE;
}

BOOL GuildPurchase::Remove( tagGroupPurchase* pGPInfo, BOOL bNotify2DB /*= TRUE*/ )
{
	ASSERT(pGPInfo);
	if (!VALID_POINT(pGPInfo))
	{
		return FALSE;
	}

	// 验证帮派
	if (pGPInfo->dwGuildID != m_pGuild->get_guild_att().dwID)
	{
		return FALSE;
	}

	// 插入
	BOOL bRet = m_mapGPInfo.erase(GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID));

	// 通知数据库
	if (bRet && bNotify2DB)
	{
		RemoveGPInfo2DB(pGPInfo);
	}

	return bRet;
}

//-------------------------------------------------------------------------------------------------------
// 获取团购信息
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::GetAllSimGPInfo( INT &nGPInfoNum, tagSimGPInfo* pData )
{
	if (m_mapGPInfo.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* pGPInfo = NULL;
	nGPInfoNum = 0;

	MapGPInfo::map_iter iter = m_mapGPInfo.begin();
	while (m_mapGPInfo.find_next(iter, pGPInfo))
	{
		if (!VALID_POINT(pGPInfo))
		{
			// 数据损坏
			ASSERT(pGPInfo);
			print_message(_T("\nthere is a error GP Info in map.\n"));
			continue;
		}
		pData[nGPInfoNum].dwGuildID			= pGPInfo->dwGuildID;
		pData[nGPInfoNum].dw_role_id			= pGPInfo->dw_role_id;
		pData[nGPInfoNum].dwMallID			= pGPInfo->dwMallID;
		pData[nGPInfoNum].nPrice			= pGPInfo->nPrice;
		pData[nGPInfoNum].nParticipatorNum	= pGPInfo->nParticipatorNum;
		pData[nGPInfoNum].nRequireNum		= pGPInfo->nRequireNum;
		pData[nGPInfoNum].dwRemainTime		= pGPInfo->dwRemainTime;

		nGPInfoNum++;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 获取指定团购的响应者列表
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::GetParticipators( DWORD dwID, DWORD dw_role_id, DWORD *pData )
{
	if (m_mapGPInfo.empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	tagGroupPurchase* pGPInfo = NULL;
	INT64 nMapKey = GetKey(dw_role_id, dwID);

	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
	{
		return E_GroupPurchase_NoInfo;
	}

	// 没有响应者的团购数据，不应该存在
	if (pGPInfo->listParticipators->empty())
	{
		return E_GroupPurchase_NoInfo;
	}

	INT i = 0;
	pGPInfo->listParticipators->reset_iterator();
	while (pGPInfo->listParticipators->find_next(pData[i++]));

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 获取团购信息数量
//-------------------------------------------------------------------------------------------------------
INT GuildPurchase::GetGroupPurchaseInfoNum()
{
	return m_mapGPInfo.size();
}

//-------------------------------------------------------------------------------------------------------
// 获取响应者数量
//-------------------------------------------------------------------------------------------------------
INT GuildPurchase::GetParticipatorNum( DWORD dwID, DWORD dw_role_id )
{
	if (m_mapGPInfo.empty())
	{
		return 0;
	}

	tagGroupPurchase* pGPInfo = NULL;
	INT64 nMapKey = GetKey(dw_role_id, dwID);

	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
	{
		return 0;
	}

	return pGPInfo->listParticipators->size();
}

//-------------------------------------------------------------------------------------------------------
// 发起团购
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::LaunchGroupPurchase( Role *pRole, DWORD dwID, BYTE byScope, BYTE byIndex, INT nUnitPrice, OUT tagGroupPurchase* &pGPInfo, OUT DWORD& dwItemTypeID )
{
	ASSERT(VALID_POINT(pRole));
	ASSERT(g_mall.is_init_ok());
	ASSERT(nUnitPrice > 0);

	DWORD dw_error_code = E_Success;

	// 检查索引合法性
	const tagMallItemProto *pProto = g_mall.GetMallItem(byIndex, EMIT_Item)->pMallItem;
	if (!VALID_POINT(pProto))
	{
		return INVALID_VALUE;
	}

	// 检查该物品是否可以团购
	switch(byScope)
	{
	case EGPS_SMALL:
		if (pProto->bySmallGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	case EGPS_MEDIUM:
		if (pProto->byMediumGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	case EGPS_BIG:
		if (pProto->byBigGroupDiscount == (BYTE)INVALID_VALUE)
		{
			return E_GroupPurchase_ItemNotAllowable;
		}
		break;

	default:
		return INVALID_VALUE;
		break;
	}

	// 上层负责检查人物帮派
	INT32 nGroupPurchase = m_pGuild->get_guild_att().nGroupPurchase;

	// id
	if(pProto->dwID != dwID)
	{
		return E_Mall_ID_Error;
	}

	// 检查是否已经发起过这个物品的团购
	INT64 nMapKey = GetKey(pRole->GetID(), dwID);
	if (m_mapGPInfo.is_exist(nMapKey))
		return E_GroupPurchase_AlreadyInitiate;

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

	// 计算团购价格
	nPrice *= pProto->byGroupPurchaseAmount;

	switch(byScope)
	{
	case EGPS_SMALL:
		nPrice = (INT)(nPrice * (pProto->bySmallGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;

	case EGPS_MEDIUM:
		nPrice = (INT)(nPrice * (pProto->byMediumGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;

	case EGPS_BIG:
		nPrice = (INT)(nPrice * (pProto->byBigGroupDiscount/100.0f - 0.2f*nGroupPurchase/100000));
		break;
	}

	// 检查玩家元宝是否足够
	if(nPrice > pRole->GetCurMgr().GetBagYuanBao() || nPrice <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	// 设置传出参数
	pGPInfo = new tagGroupPurchase;
	if (!VALID_POINT(pGPInfo))
	{
		ASSERT(pGPInfo);
		return INVALID_VALUE;
	}

	pGPInfo->dwGuildID		= pRole->GetGuildID();
	pGPInfo->dw_role_id		= pRole->GetID();
	pGPInfo->dwMallID		= dwID;
	pGPInfo->nPrice			= nPrice;
	pGPInfo->dwRemainTime	= pProto->dwPersistTime * 60 * 60; // 以秒为单位

	switch (byScope)
	{
	case EGPS_SMALL:
		pGPInfo->nRequireNum = pProto->bySmallGroupHeadcount;
		break;

	case EGPS_MEDIUM:
		pGPInfo->nRequireNum = pProto->byMediumGroupHeadcount;
		break;

	case EGPS_BIG:
		pGPInfo->nRequireNum = pProto->byBigGroupHeadcount;
		break;
	}
	pGPInfo->nParticipatorNum = 1;
	pGPInfo->listParticipators = new package_list<DWORD>;
	if (!VALID_POINT(pGPInfo->listParticipators))
	{
		ASSERT(pGPInfo->listParticipators);
		SAFE_DELETE(pGPInfo);
		return INVALID_VALUE;
	}
	pGPInfo->listParticipators->push_back(pGPInfo->dw_role_id);

	// 将该团购信息加入管理表
	Add(pGPInfo);

	// 传出物品TypeID
	dwItemTypeID = pProto->dw_data_id;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 响应团购
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::RespondGroupPurchase( Role *pRole, DWORD dwID, DWORD dw_role_id, INT nPrice, OUT tagGroupPurchase* &pGPInfo )
{
	// 检验帮派信息
	if (!VALID_VALUE(pRole->GetGuildID()))
		return E_GroupPurchase_NotInGuild;

	if (!VALID_POINT(m_pGuild->get_member(pRole->GetID())))
		return E_GroupPurchase_NotMember;

	// 检验是否是发起人
	if (dw_role_id == pRole->GetID())
	{
		return E_GroupPurchase_IsInitiate;
	}

	// 检验指定的团购是否已经结束
	INT64 nMapKey = GetKey(dw_role_id, dwID);
	pGPInfo = m_mapGPInfo.find(nMapKey);

	if (!VALID_POINT(pGPInfo) || (INT32)(pGPInfo->dwRemainTime) <= 0)
		return E_GroupPurchase_AlreadyEnd;

	// 非法信息应该删除
	if (!VALID_POINT(pGPInfo->listParticipators))
		return E_GroupPurchase_AlreadyEnd;

	// 检验是否已经响应过这个团购
	DWORD dwTmpRoleID = pRole->GetID();
	if (pGPInfo->listParticipators->is_exist(dwTmpRoleID))
	{
		return E_GroupPurchase_AlreadyInitiate;
	}

	// 金钱
	if(nPrice != pGPInfo->nPrice)
	{
		return E_Mall_YuanBao_Error;
	}

	// 检查玩家元宝是否足够
	if(nPrice > pRole->GetCurMgr().GetBagYuanBao() || nPrice <= 0)
	{
		return E_BagYuanBao_NotEnough;
	}

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 生成团购物品
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::CreateGPItems( DWORD dwID, OUT tagMallItemSell &itemSell )
{
	ASSERT(VALID_VALUE(dwID));

	//验证商品的合法性
	const tagMallItemProto *pProto = g_attRes.GetMallItemProto(dwID);

	if (!VALID_POINT(pProto))
	{
		return E_Mall_ID_Error;
	}

	// 创建物品
	tagItem *pItemNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, pProto->dw_data_id, pProto->byGroupPurchaseAmount, EIQ_Quality0);
	if(!VALID_POINT(pItemNew))
	{
		ASSERT(VALID_POINT(pItemNew));
		return E_Mall_CreateItem_Failed;
	}

	// 如果有赠品，则生成赠品
	tagItem *pPresentNew = NULL;
	if(pProto->dwPresentID != INVALID_VALUE)
	{
		pPresentNew = ItemCreator::CreateEx(EICM_Mall, INVALID_VALUE, 
			pProto->dwPresentID, (INT16)pProto->byPresentNum * pProto->byGroupPurchaseAmount, EIQ_Quality0);
		if(!VALID_POINT(pPresentNew))
		{
			::Destroy(pItemNew);
			ASSERT(VALID_POINT(pPresentNew));
			return E_Mall_CreatePres_Failed;
		}
	}

	// 回馈赠点
	if (pProto->byExAssign >= 0)
	{
		itemSell.nExVolumeAssign = pProto->byExAssign;
	}

	// 设置传出参数
	itemSell.pItem			= pItemNew;
	itemSell.pPresent		= pPresentNew;
	itemSell.nYuanBaoNeed	= 0;				// 已经预付费
	itemSell.byRemainNum	= INVALID_VALUE;

	return E_Success;
}

//-------------------------------------------------------------------------------------------------------
// 删除指定团购信息
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::RemoveGroupPurchaseInfo( tagGroupPurchase* &pGPInfo, BOOL bSuccess /*= TRUE*/ )
{
	ASSERT(pGPInfo);

	if (!VALID_POINT(pGPInfo))
		return;

	// 删除管理器中的信息
	INT64 nMapKey = GetKey(pGPInfo->dw_role_id, pGPInfo->dwMallID);
	m_mapGPInfo.erase(nMapKey);

	// 若是团购失败则返还玩家元宝
	if (!bSuccess)
	{
		ReturnCost2Participator(pGPInfo);

		m_pGuild->modify_group_exponent(FALSE);

		// 广播团购失败消息
		const tagMallItemProto *pProto = g_attRes.GetMallItemProto(pGPInfo->dwMallID);

		if (!VALID_POINT(pProto))
		{
			// 这里不会发生吧，以防万一
			ASSERT(pProto);
			return;
		}

		tagNS_RespondBroadCast send;
		send.eType = ERespondBroadCast_Lose;
		send.dw_role_id = pGPInfo->dw_role_id;
		send.dw_data_id = pProto->dw_data_id;
		m_pGuild->send_guild_message(&send, send.dw_size);
	}

	// 删除数据库中的信息
	RemoveGPInfo2DB(pGPInfo);

	// 删除信息本身
	SAFE_DELETE(pGPInfo);

	// 上层检查是否删除this对象
}

//-------------------------------------------------------------------------------------------------------
// 删除该帮派所有团购信息(帮派解散)
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::RemoveGroupPurchaseInfo()
{
	// 该帮派所有团购失败
	tagGroupPurchase *pGPInfo = NULL;
	if (!m_mapGPInfo.empty())
	{
		MapGPInfo::map_iter iter = m_mapGPInfo.begin();
		while (m_mapGPInfo.find_next(iter, pGPInfo))
		{
			ReturnCost2Participator(pGPInfo);
			SAFE_DELETE(pGPInfo);
		}
	}
	m_mapGPInfo.clear();

	// 删除数据库中该帮派的团购信息
	RemoveGuildGPInfo2DB();

	// 上层检测是否需要删除this对象
}

//-------------------------------------------------------------------------------------------------------
// 返还玩家元宝(团购失败)
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::ReturnCost2Participator( tagGroupPurchase* pGPInfo )
{
	if (!VALID_POINT(pGPInfo) || !VALID_POINT(pGPInfo->listParticipators))
		return;

	DWORD tmpRoleID = INVALID_VALUE;

	pGPInfo->listParticipators->reset_iterator();
	while(pGPInfo->listParticipators->find_next(tmpRoleID))
	{
		// 检查玩家是够在线
		Role* pRole = g_roleMgr.GetRolePtrByID(tmpRoleID);
		if (!VALID_POINT(pRole))
		{
			// 向离线玩家发送元宝
			CurrencyMgr::ModifyBaiBaoYuanBao(tmpRoleID, pGPInfo->nPrice, elcid_group_purchase_faild);
		}
		else
		{
			// 向在线玩家发送元宝
			pRole->GetCurMgr().IncBaiBaoYuanBao(pGPInfo->nPrice, elcid_group_purchase_faild);
		}
	}
}

//-------------------------------------------------------------------------------------------------------
// 数据库操作
//-------------------------------------------------------------------------------------------------------
VOID GuildPurchase::AddGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_AddGPInfo send;

	send.gp_info.dw_guild_id			= pGPInfo->dwGuildID;
	send.gp_info.dw_role_id			= pGPInfo->dw_role_id;
	send.gp_info.dw_mall_id			= pGPInfo->dwMallID;
	send.gp_info.n_price				= pGPInfo->nPrice;
	send.gp_info.n_participator_num	= pGPInfo->nParticipatorNum;
	send.gp_info.n_require_num			= pGPInfo->nRequireNum;
	send.gp_info.n_remain_time			= pGPInfo->dwRemainTime;
	send.gp_info.dw_participators[0]	= pGPInfo->dw_role_id;

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::UpdateGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_UpdateGPInfo send;

	send.gp_info_key_.dw_guild_id	= pGPInfo->dwGuildID;
	send.gp_info_key_.dw_role_id		= pGPInfo->dw_role_id;
	send.gp_info_key_.dw_mall_id		= pGPInfo->dwMallID;
	send.dw_new_participator_		= pGPInfo->listParticipators->get_list().back();

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::RemoveGPInfo2DB( tagGroupPurchase* pGPInfo )
{
	ASSERT(VALID_POINT(pGPInfo));
	if (!VALID_POINT(pGPInfo))
		return;

	tagNDBC_RemoveGPInfo send;

	send.gp_info_key_.dw_guild_id	= pGPInfo->dwGuildID;
	send.gp_info_key_.dw_role_id		= pGPInfo->dw_role_id;
	send.gp_info_key_.dw_mall_id		= pGPInfo->dwMallID;

	g_dbSession.Send(&send, send.dw_size);
}

VOID GuildPurchase::RemoveGuildGPInfo2DB()
{
	ASSERT(VALID_POINT(m_pGuild));
	if (!VALID_POINT(m_pGuild))
	{
		return;
	}

	tagNDBC_RemoveGuildGPInfo send;

	send.dw_guild_id = m_pGuild->get_guild_att().dwID;

	g_dbSession.Send(&send, send.dw_size);
}

//-------------------------------------------------------------------------------------------------------
// 生成键值
//-------------------------------------------------------------------------------------------------------
INT64 GuildPurchase::GetKey( DWORD dw_role_id, DWORD dwID )
{
	ASSERT(VALID_VALUE(dw_role_id) && VALID_VALUE(dwID));
	if (!VALID_VALUE(dw_role_id) || !VALID_VALUE(dwID))
		return INVALID_VALUE;

	INT64 n64Key = dw_role_id;

	n64Key = (n64Key << 32) | dwID;

	return n64Key;
}

//-------------------------------------------------------------------------------------------------------
// 团购成功处理
//-------------------------------------------------------------------------------------------------------
DWORD GuildPurchase::DoSuccessStuff( tagGroupPurchase* pGPInfo )
{
	// 做一下简单验证
	if (m_pGuild->get_guild_att().dwID != pGPInfo->dwGuildID)
	{
		return INVALID_VALUE;
	}

	DWORD dwItemTypeID		= INVALID_VALUE;
	DWORD dwLaunchRoleID	= pGPInfo->dw_role_id;

	// 向所有列表中的玩家发送物品
	DWORD tmpRoleID = INVALID_VALUE;
	package_list<DWORD>::list_iter iter = pGPInfo->listParticipators->begin();	// 上层已经保证了列表的合法性
	
	while(pGPInfo->listParticipators->find_next(iter, tmpRoleID))
	{
		// 生成物品
		tagMallItemSell	sItemSell;
		DWORD dw_error_code = CreateGPItems(pGPInfo->dwMallID, sItemSell);
		if (dw_error_code != E_Success)
		{
			return dw_error_code;
		}
		else if (VALID_POINT(sItemSell.pItem))
		{
			dwItemTypeID = sItemSell.pItem->dw_data_id;
		}
		else
		{
			return INVALID_VALUE;
		}

		// 处理结果
		if(VALID_POINT(sItemSell.pItem))
		{		
			INT64 n64_serial = sItemSell.pItem->n64_serial;
			DWORD dwFstGainTime = g_world.GetWorldTime();
			INT16 n16BuyNum = sItemSell.pItem->n16Num;

			// 百宝袋历史记录
			ItemMgr::ProcBaiBaoRecord(sItemSell.pItem->dw_data_id, tmpRoleID, INVALID_VALUE, EBBRT_GroupPurchase, dwFstGainTime);

			// log
			g_mall.LogMallSell(tmpRoleID, INVALID_VALUE, *sItemSell.pItem, n64_serial, n16BuyNum, 
				dwFstGainTime, sItemSell.nYuanBaoNeed, 0, elcid_group_purchase_buy_item);

			// 将物品放到百宝袋中
			Role *pTmpRole = g_roleMgr.GetRolePtrByID(tmpRoleID);
			if(VALID_POINT(pTmpRole))
			{
				pTmpRole->GetItemMgr().Add2BaiBao(sItemSell.pItem, elcid_group_purchase_buy_item);
			}
			else
			{
				// 存储到item_baibao表中
				ItemMgr::InsertBaiBao2DB(sItemSell.pItem, tmpRoleID, elcid_group_purchase_buy_item);

				// 删除物品
				::Destroy(sItemSell.pItem);
			}

			// 如果有赠品，则放到百宝袋中
			if(VALID_POINT(sItemSell.pPresent))
			{
				// 百宝袋历史记录
				ItemMgr::ProcBaiBaoRecord(sItemSell.pPresent->dw_data_id, tmpRoleID, INVALID_VALUE, EBBRT_Mall, dwFstGainTime);

				if(VALID_POINT(pTmpRole))
				{
					pTmpRole->GetItemMgr().Add2BaiBao(sItemSell.pPresent, elcid_group_purchase_buy_item_add);
				}
				else
				{
					// 存储到item_baibao表中
					ItemMgr::InsertBaiBao2DB(sItemSell.pPresent, tmpRoleID, elcid_group_purchase_buy_item_add);

					// 删除物品
					::Destroy(sItemSell.pItem);
				}
			}

			// 回馈赠点
			if (VALID_POINT(pTmpRole) && sItemSell.nExVolumeAssign > 0)
			{
				pTmpRole->GetCurMgr().IncExchangeVolume(sItemSell.nExVolumeAssign, elcid_group_purchase_buy_item);
			}
		}
	}

	// 帮派团购指数+1
	m_pGuild->modify_group_exponent(TRUE);

	// 删除该条团购消息记录
	RemoveGroupPurchaseInfo(pGPInfo);

	// 帮派内广播成功消息
	tagNS_RespondBroadCast send;
	send.eType = ERespondBroadCast_Success;
	send.dw_role_id = dwLaunchRoleID;
	send.dw_data_id = dwItemTypeID;
	m_pGuild->send_guild_message(&send, send.dw_size);

	return E_Success;
}