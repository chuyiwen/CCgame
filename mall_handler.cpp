
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
#include "player_session.h"
#include "role.h"
#include "mall.h"
#include "world_net_cmd_mgr.h"

//-----------------------------------------------------------------------------
// 常量
//-----------------------------------------------------------------------------
const UINT32 MAX_REQUEST_GPINFO_PER_TICK = 100;

//-----------------------------------------------------------------------------
// 获取商场商品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallGet(tag_net_message* pCmd)
{
	//MGET_MSG(p_receive, pCmd, NET_SIC_mall_get);

	// 消息合法性验证: 无

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_open_mall send;
	send.dw_error_code = pRole->GetMallAll(send.dwMallTime);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 更新商城
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallUpdate(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_update);
	NET_SIC_mall_update * p_receive = ( NET_SIC_mall_update * ) pCmd ;  
	// 消息合法性验证
	if(INVALID_VALUE == p_receive->dwMallTime)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//NET_SIS_mall_update send;
	//send.dwMallTime = INVALID_VALUE;
	//pRole->UpdateMallAll(send.dwMallTime, p_receive->dwMallTime);
	//SendMessage(&send, send.dw_size);

	NET_SIS_open_mall send;
	send.dw_error_code = E_Success;
	pRole->UpdateMallAll(send.dwMallTime, p_receive->dwMallTime);
	SendMessage(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 购买物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallBuyItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_buy_item);
	NET_SIC_mall_buy_item * p_receive = ( NET_SIC_mall_buy_item * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n16BuyNum <= 0 || p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_mall_buy_item send;
	send.dw_error_code = pRole->BuyMallItem(p_receive->dw_data_id, p_receive->nPrice, p_receive->n16BuyNum, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 购买礼品包
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallBuyPack(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_buy_pack);
	NET_SIC_mall_buy_pack * p_receive = ( NET_SIC_mall_buy_pack * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n16BuyNum != 1 || p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_mall_buy_pack send;
	send.dw_error_code = pRole->BuyMallPack(p_receive->dw_data_id, p_receive->nPrice, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向好友赠送物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallPresentItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_present_item);
	NET_SIC_mall_present_item * p_receive = ( NET_SIC_mall_present_item * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n16BuyNum <= 0 || p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//GET_MESSAGE_STRING(strLeave, p_receive->szLeave, p_receive->dw_size, NET_SIC_mall_present_item, szLeave);
	INT32 tmpStrSz = ( p_receive->dw_size - FIELD_OFFSET ( NET_SIC_mall_present_item , szLeave ) ) / sizeof ( TCHAR ) ;  
	if ( tmpStrSz < 0 ) tmpStrSz = 0 ;  
	tstring strLeave ( p_receive->szLeave , tmpStrSz ) ;  

	NET_SIS_mall_present_item send;
	send.dw_error_code = pRole->BuyMallItem(p_receive->dwRoleTgtID, strLeave.c_str(), 
								p_receive->dw_data_id, p_receive->nPrice, p_receive->n16BuyNum, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向好友赠送礼品包
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallPresentPack(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_present_pack);
	NET_SIC_mall_present_pack * p_receive = ( NET_SIC_mall_present_pack * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n16BuyNum <= 0 || p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//GET_MESSAGE_STRING(strLeave, p_receive->szLeave, p_receive->dw_size, NET_SIC_mall_present_pack, szLeave);
	INT32 tmpStrSz = ( p_receive->dw_size - FIELD_OFFSET ( NET_SIC_mall_present_pack , szLeave ) ) / sizeof ( TCHAR ) ;   
	if ( tmpStrSz < 0 ) tmpStrSz = 0 ;   
	tstring strLeave ( p_receive->szLeave , tmpStrSz ) ;  

	NET_SIS_mall_present_pack send;
	send.dw_error_code = pRole->BuyMallPack(p_receive->dwRoleTgtID, strLeave.c_str(), 
								p_receive->dw_data_id, p_receive->nPrice, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取免费物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallFreeGetItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_free_get_item);
	NET_SIC_mall_free_get_item * p_receive = ( NET_SIC_mall_free_get_item * ) pCmd ;  
	// 消息合法性验证: 无

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_mall_free_get_item send;
	send.dw_error_code = pRole->GetMallFreeItem(p_receive->dw_data_id);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 发起团购
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallLaunchGroupPurchase(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_launch_group_purchase);
	NET_SIC_launch_group_purchase * p_receive = ( NET_SIC_launch_group_purchase * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_launch_group_purchase send;
	send.dw_error_code = g_mall.lauch_guild_buy(pRole, p_receive->dw_data_id, p_receive->byScope, p_receive->byIndexInServer, p_receive->nPrice);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 响应团购
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallRespondGroupPurchase(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_respond_group_purchase);
	NET_SIC_respond_group_purchase * p_receive = ( NET_SIC_respond_group_purchase * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_respond_group_purchase send;
	send.nGroupPurchaseKey = p_receive->dw_role_id;
	send.nGroupPurchaseKey = (send.nGroupPurchaseKey << 32) | p_receive->dw_data_id;

	//send.dw_error_code = g_mall.respond_guild_buy(pRole, p_receive->dwGuildID, p_receive->dw_data_id, p_receive->dw_role_id, p_receive->nPrice);

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取玩家所在帮派的团购信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallGetGroupPurchaseInfo(tag_net_message* pCmd)
{
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 判断服务器是否繁忙
	UINT32 nCount = WorldNetCmdMgr::GetInstance()->GetRunTimesPerTick(pCmd->dw_message_id);
	if (nCount < 0)
	{
		print_message(_T("Msg runtimes status error!\r\n"));
		return INVALID_VALUE;
	}

	if (nCount > MAX_REQUEST_GPINFO_PER_TICK)
	{
		NET_SIS_guild_buy_info send;
		send.dw_error_code = E_GroupPurchase_ServerBusy;
		SendMessage(&send, send.dw_size);
		return send.dw_error_code;
	}

	return g_mall.get_all_guild_buy_info(pRole);
}

//-----------------------------------------------------------------------------
// 获取指定团购信息中的响应者列表
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallGetParticipators(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_participators);
	NET_SIC_get_participators * p_receive = ( NET_SIC_get_participators * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	return g_mall.get_response(pRole, p_receive->dwGuildID, p_receive->dw_data_id, p_receive->dw_role_id);
}

//-----------------------------------------------------------------------------
// 商城物品兑换
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallItemExchange(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_item_exchange);
	NET_SIC_mall_item_exchange * p_receive = ( NET_SIC_mall_item_exchange * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n16BuyNum <= 0 || p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_mall_item_exchange send;
	send.dw_error_code = pRole->MallItemExchange(p_receive->dwMallID, p_receive->nPrice, p_receive->n16BuyNum, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 商城打包物品兑换
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMallPackExchange(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_mall_pack_exchange);
	NET_SIC_mall_pack_exchange * p_receive = ( NET_SIC_mall_pack_exchange * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->nPrice <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_mall_pack_exchange send;
	send.dw_error_code = pRole->MallPackExchange(p_receive->dwMallID, p_receive->nPrice, p_receive->byIndexInServer);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向元宝交易账户存元宝
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSaveYB2Account(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_save_yuanbao_to_account);
	NET_SIC_save_yuanbao_to_account * p_receive = ( NET_SIC_save_yuanbao_to_account * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->n_num <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_save_yuanbao_to_account send;
	send.dw_error_code = pRole->SaveYB2Account(pRole->GetID(), p_receive->n_num);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向元宝交易账户存金钱
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSaveSilver2Account(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_save_silver_to_account);
	NET_SIC_save_silver_to_account * p_receive = ( NET_SIC_save_silver_to_account * ) pCmd ;  
	if(p_receive->n_num <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_save_silver_to_account send;
	send.dw_error_code = pRole->SaveSilver2Account(pRole->GetID(), p_receive->n_num);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向元宝交易账户取元宝
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDepositYBAccount(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_deposit_account_yuanbao);
	NET_SIC_deposit_account_yuanbao * p_receive = ( NET_SIC_deposit_account_yuanbao * ) pCmd ;  
	if(p_receive->n_num <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_deposit_account_yuanbao send;
	send.dw_error_code = pRole->DepositYBAccout(pRole->GetID(), p_receive->n_num);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 向元宝交易账户取金钱
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDepositSilver(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_deposit_account_silver);
	NET_SIC_deposit_account_silver * p_receive = ( NET_SIC_deposit_account_silver * ) pCmd ;  
	if(p_receive->n_num <= 0)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_deposit_account_silver send;
	send.dw_error_code = pRole->DepositSilverAccount(pRole->GetID(), p_receive->n_num);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 同步元宝交易初始化信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetYBTradeInfo(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_synchronize_yuanbao_trade_info);
	NET_SIC_synchronize_yuanbao_trade_info * p_receive = ( NET_SIC_synchronize_yuanbao_trade_info * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	pRole->GetYBTradeInfo();

	return E_Success;
}

//-----------------------------------------------------------------------------
// 玩家提交出售元宝订单
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSubmitSellOrder(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_submit_sell_order);
	NET_SIC_submit_sell_order * p_receive = ( NET_SIC_submit_sell_order * ) pCmd ;  
	// 获取人物
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_submit_sell_order	send;
	send.dw_error_code = pRole->SubmitSellOrder(pRole->GetID(), p_receive->n_num, p_receive->nPrice);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 玩家提交收购元宝订单
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSubmitBuyOrder(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_submit_buy_order);
	NET_SIC_submit_buy_order * p_receive = ( NET_SIC_submit_buy_order * ) pCmd ;  
	// 获取人物
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_submit_buy_order	send;
	send.dw_error_code = pRole->SubmitBuyOrder(pRole->GetID(), p_receive->n_num, p_receive->nPrice);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 玩家删除提交的订单
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDeleteOrder(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_delete_order);
	NET_SIC_delete_order * p_receive = ( NET_SIC_delete_order * ) pCmd ;  
	// 获取人物
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_delete_order	send;
	send.dw_error_code = pRole->DeleteOrder(pRole->GetID(), p_receive->dwOrderID, p_receive->eYBOType);
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 玩家获取一周内的交易订单
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetYBOrder(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_yuanbao_order);
	NET_SIC_get_yuanbao_order * p_receive = ( NET_SIC_get_yuanbao_order * ) pCmd ;  
	// 获取人物
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	pRole->GetYBOrder(pRole->GetID());

	return E_Success;
}
