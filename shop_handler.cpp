
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//商店消息处理
#include "StdAfx.h"
#include "../../common/WorldDefine/shop_define.h"
#include "../../common/WorldDefine/shop_protocol.h"
#include "player_session.h"
#include "role.h"

//-----------------------------------------------------------------------------
// 获取物品(非装备)店中刷新商品列表
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetShopItems(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_shop_item);
	NET_SIC_get_shop_item * p_receive = ( NET_SIC_get_shop_item * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->GetShopItems(p_receive->dwNPCID, p_receive->byShelf);

	// 处理结果反馈
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);
	
	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取装备店装备列表
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetShopEquips(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_shop_equip);
	NET_SIC_get_shop_equip * p_receive = ( NET_SIC_get_shop_equip * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->GetShopEquips(p_receive->dwNPCID, p_receive->byShelf);

	// 处理结果反馈
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 购买物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBuyShopItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_buy_shop_item);
	NET_SIC_buy_shop_item * p_receive = ( NET_SIC_buy_shop_item * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM
		|| p_receive->n16ItemNum < 0 || MIsEquipment(p_receive->dw_data_id))
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	//if(!pRole->get_check_safe_code())
	//{
	//	if(GetBagPsd() != p_receive->dw_safe_code)
	//	{

	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = FALSE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		return INVALID_VALUE;
	//	}

	//	else 
	//	{
	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = TRUE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		pRole->set_check_safe_code();
	//	}
	//}

	DWORD dw_error_code =	pRole->BuyShopItem(p_receive->dwNPCID, p_receive->byShelf, 
										p_receive->dw_data_id, p_receive->n16ItemNum);

	// 处理结果反馈
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 购买装备
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBuyShopEquip(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_buy_shop_equip);
	NET_SIC_buy_shop_equip * p_receive = ( NET_SIC_buy_shop_equip * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	//if(!pRole->get_check_safe_code())
	//{
	//	if(GetBagPsd() != p_receive->dw_safe_code)
	//	{

	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = FALSE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		return INVALID_VALUE;
	//	}

	//	else 
	//	{
	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = TRUE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		pRole->set_check_safe_code();
	//	}
	//}


	DWORD dw_error_code = pRole->BuyShopEquip(p_receive->dwNPCID, p_receive->byShelf, 
										p_receive->dw_data_id, p_receive->n64_serial);

	// 处理结果反馈
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 出售物品&装备
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSellToShop(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_sell_to_shop);
	NET_SIC_sell_to_shop * p_receive = ( NET_SIC_sell_to_shop * ) pCmd ;  
	// 消息合法性验证: 无

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	//if(!pRole->get_check_safe_code())
	//{
	//	if(GetBagPsd() != p_receive->dw_safe_code)
	//	{

	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = FALSE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		return INVALID_VALUE;
	//	}

	//	else 
	//	{
	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = TRUE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		pRole->set_check_safe_code();
	//	}
	//}


	DWORD dw_error_code = pRole->SellToShop(p_receive->dwNPCID, p_receive->nNumber, p_receive->n64_serial);

	// 处理结果反馈
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}