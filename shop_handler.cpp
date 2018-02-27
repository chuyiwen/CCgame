
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�̵���Ϣ����
#include "StdAfx.h"
#include "../../common/WorldDefine/shop_define.h"
#include "../../common/WorldDefine/shop_protocol.h"
#include "player_session.h"
#include "role.h"

//-----------------------------------------------------------------------------
// ��ȡ��Ʒ(��װ��)����ˢ����Ʒ�б�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetShopItems(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_shop_item);
	NET_SIC_get_shop_item * p_receive = ( NET_SIC_get_shop_item * ) pCmd ;  
	// ��Ϣ�Ϸ�����֤
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// ��ȡ����
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->GetShopItems(p_receive->dwNPCID, p_receive->byShelf);

	// ����������
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);
	
	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡװ����װ���б�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetShopEquips(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_shop_equip);
	NET_SIC_get_shop_equip * p_receive = ( NET_SIC_get_shop_equip * ) pCmd ;  
	// ��Ϣ�Ϸ�����֤
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// ��ȡ����
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->GetShopEquips(p_receive->dwNPCID, p_receive->byShelf);

	// ����������
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ������Ʒ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBuyShopItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_buy_shop_item);
	NET_SIC_buy_shop_item * p_receive = ( NET_SIC_buy_shop_item * ) pCmd ;  
	// ��Ϣ�Ϸ�����֤
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM
		|| p_receive->n16ItemNum < 0 || MIsEquipment(p_receive->dw_data_id))
	{
		return INVALID_VALUE;
	}

	// ��ȡ����
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

	// ����������
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ����װ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBuyShopEquip(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_buy_shop_equip);
	NET_SIC_buy_shop_equip * p_receive = ( NET_SIC_buy_shop_equip * ) pCmd ;  
	// ��Ϣ�Ϸ�����֤
	if(p_receive->byShelf >= MAX_SHOP_SHELF_NUM)
	{
		return INVALID_VALUE;
	}

	// ��ȡ����
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

	// ����������
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ������Ʒ&װ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSellToShop(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_sell_to_shop);
	NET_SIC_sell_to_shop * p_receive = ( NET_SIC_sell_to_shop * ) pCmd ;  
	// ��Ϣ�Ϸ�����֤: ��

	// ��ȡ����
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

	// ����������
	pRole->SendShopFeedbackMsg(dw_error_code, p_receive->dwNPCID);

	return dw_error_code;
}