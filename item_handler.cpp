
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 物品/装备相关
#include "StdAfx.h"

#include "../../common/WorldDefine/item_protocol.h"
#include "../../common/WorldDefine/drop_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/common_server_define.h"

#include "player_session.h"
#include "world_session.h"
#include "role.h"
#include "world.h"
#include "guild_manager.h"
#include "role_mgr.h"

//-----------------------------------------------------------------------------
// 穿上装备
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEquip(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_equip);
	NET_SIC_equip * p_receive = ( NET_SIC_equip * ) pCmd ;  	
	// 消息合法性验证
	if(p_receive->ePosDst <= EEP_Start || p_receive->ePosDst >= EEP_End)
	{
		print_message(_T("The equip position is invalid!"));
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 换装
	DWORD dw_error_code = pRole->Equip(p_receive->n64_serial, p_receive->ePosDst);

	// 反馈给客户端换装结果
	NET_SIS_equip	send;
	send.n64_serial = p_receive->n64_serial;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);


	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 脱下装备
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUnequip(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_unequip);
	NET_SIC_unequip * p_receive = ( NET_SIC_unequip * ) pCmd ;  	
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 换装
	DWORD dw_error_code = pRole->Unequip(p_receive->n64_serial, p_receive->n16PosDst);
	//if(E_Success == dw_error_code)
	//{
	//	return E_Success;
	//}

	NET_SIS_unequip send;
	send.n64_serial = p_receive->n64_serial;
	send.dw_error_code = dw_error_code;

	// 反馈给客户端换装结果
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 主副手对换
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSwapWeapon(tag_net_message* pCmd)
{
	//MGET_MSG(p_receive, pCmd, NET_SIC_swap_weapon);

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 换装
	DWORD dw_error_code = E_Success;//pRole->SwapWeapon();
	if(E_Success == dw_error_code)
	{
		return E_Success;
	}

	NET_SIS_swap_weapon send;
	send.dw_error_code = dw_error_code;

	// 反馈给客户端换装结果
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 鉴定装备
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleIdentifyEquip(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_identify_equip);
	NET_SIC_identify_equip * p_receive = ( NET_SIC_identify_equip * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->IdentifyEquip(p_receive->n64SerialReel, 
												p_receive->n64SerialEquip);

	NET_SIS_identify_equip send;
	send.n64_serial = p_receive->n64SerialEquip;
	send.dw_error_code = dw_error_code;

	// 反馈给客户端换装结果
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 装备维修
//-----------------------------------------------------------------------------
DWORD	PlayerSession::HandleRoleEquipRepair(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_equip_repair);
	NET_SIC_equip_repair * p_receive = ( NET_SIC_equip_repair * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->EquipRepair(INVALID_VALUE, p_receive->dwNPCID);

	NET_SIS_equip_repair send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

// 装备绑定
DWORD PlayerSession::HandleRoleEquipBind(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_equip_bind);
	NET_SIC_equip_bind * p_receive = ( NET_SIC_equip_bind * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	EBindStatus eBindType = EBS_Unbind;
	DWORD dw_error_code = pRole->EquipBind(p_receive->n64EquipID, p_receive->n64ItemID, p_receive->dwNPCID, eBindType);

	NET_SIS_equip_bind send;
	send.n64EquipID = p_receive->n64EquipID;
	send.dw_error_code = dw_error_code;
	send.eBindType = eBindType;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

// 装备解绑
DWORD PlayerSession::HandleRoleEquipUnBind(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_equip_unbind);
	NET_SIC_equip_unbind * p_receive = ( NET_SIC_equip_unbind * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	DWORD dw_error_code = pRole->EquipUnBind(p_receive->n64EquipID, p_receive->n64ItemID);

	NET_SIS_equip_unbind send ;
	send.n64EquipID = p_receive->n64EquipID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
	
}

// 销毁装备
DWORD PlayerSession::HandleEquipDestroy(tag_net_message* pCmd)
{
	NET_SIC_equip_destroy* p_receive = (NET_SIC_equip_destroy*)pCmd;

	DWORD	dw_error = E_Success;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(p_receive->dw_item_serial);
	if(!VALID_POINT(pItem))
		return INVALID_VALUE;

	if(!MIsEquipment(pItem->dw_data_id))
		dw_error = INVALID_VALUE;

	tagEquip* pEquip = (tagEquip*)pItem;

	//if(pEquip->equipSpec.nCurLevelExp > 0 || pEquip->equipSpec.nLevel > 1)
	//	dw_error = INVALID_VALUE;

	if(pEquip->byBind != EBS_Bind)
		dw_error = INVALID_VALUE;

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{
			dw_error = E_BagPsd_Error;
		}
	
		if(dw_error == E_Success)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}
	
	// 不能再摧毁了
	if (pRole->GetDestoryEquipCount() > 0)
	{
		dw_error = E_Destory_Equip_Error;
	}

	if(dw_error == E_Success)
	{
		pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_delete_item , 1);
		pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_delete_item_level , pEquip->pEquipProto->byMinUseLevel, 1);
		pRole->GetScript()->OnEquipDestroy(pRole);
		pRole->GetItemMgr().DelFromBag(p_receive->dw_item_serial, elcid_equip_destroy, 1);	
		pRole->ModDestoryEquipCount(1);
	}

	NET_SIS_equip_destroy send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return dw_error;
}


// 重置装备属性
DWORD PlayerSession::HandleRoleEquipReatt(tag_net_message* pCmd)
{
	NET_SIC_equip_reatt * p_receive = ( NET_SIC_equip_reatt * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	DWORD dw_error_code = pRole->EquipReAtt(p_receive->dwNPCID, p_receive->n64EquipID, p_receive->n64ItemID, p_receive->byIndex, p_receive->byType);

	NET_SIS_equip_reatt send ;
	send.n64EquipID = p_receive->n64EquipID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;

}

// 武器开光
DWORD PlayerSession::HandleRoleEquipKaiguang(tag_net_message* pCmd)
{
	NET_SIC_equip_kaiguang * p_receive = ( NET_SIC_equip_kaiguang * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	DWORD dw_error_code = pRole->EquipKaiguang(0, p_receive->n64EquipID);

	NET_SIS_equip_kaiguang send;
	send.n64EquipID = p_receive->n64EquipID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}
//-----------------------------------------------------------------------------
// 同一容器内移动物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChangeItemPos(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_item_position_change);
	NET_SIC_item_position_change * p_receive = ( NET_SIC_item_position_change * ) pCmd ;  
	if(p_receive->n16Num < 0)
	{
		return INVALID_VALUE;
	}
	
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code;
	switch(p_receive->eConType)
	{
	case EICT_Equip:
		return pRole->MoveRing(p_receive->n64_serial, p_receive->n16PosDst);
		break;
	case EICT_RoleWare:
		dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
		if(dw_error_code != E_Success)
		{
			return dw_error_code;
		}
		break;

	case EICT_GuildWare:
		dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildWare);
		if(dw_error_code == E_Success)
		{
			g_guild_manager.add_guild_ware_event(pRole->GetID(), EVT_GuildWareMoveTo, pCmd->dw_size, p_receive);
		}
		return dw_error_code;
		break;
	}

	return pRole->GetItemMgr().Move(p_receive->eConType, p_receive->n64_serial, 
								p_receive->n16Num, p_receive->n16PosDst, elcid_item_move);
}

//-----------------------------------------------------------------------------
// 容器间移动物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChangeItemPosEx(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_item_position_change_extend);
	NET_SIC_item_position_change_extend * p_receive = ( NET_SIC_item_position_change_extend * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 如果和角色仓库有关，则需判断NPC
	if(EICT_RoleWare == p_receive->eConTypeSrc || EICT_RoleWare == p_receive->eConTypeDst)
	{
		DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
		if(dw_error_code != E_Success)
		{
			return dw_error_code;
		}
	}

	// 如果和帮派仓库有关，则需判断NPC
	if(EICT_GuildWare == p_receive->eConTypeSrc || EICT_GuildWare == p_receive->eConTypeDst)
	{
		DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildWare);
		if(dw_error_code == E_Success)
		{
			g_guild_manager.add_guild_ware_event(pRole->GetID(), EVT_GuildWareMove2Other, pCmd->dw_size, p_receive);
		}
		return dw_error_code;
	}

	//if(EICT_RoleWare == p_receive->eConTypeSrc || EICT_Bag == p_receive->eConTypeSrc)
	//{
		//if(!pRole->get_check_safe_code())
		//{
		//	if(GetBagPsd() != p_receive->dw_safe_code)
		//	{
		//		NET_SIS_item_change_safe_code send;
		//		pRole->SendMessage(&send, send.dw_size);
		//		return INVALID_VALUE;
		//	}

		//	NET_SIS_code_check_ok send_check;
		//	send_check.bSuccess = TRUE;
		//	pRole->SendMessage(&send_check, send_check.dw_size);

		//	pRole->set_check_safe_code();
		//}
	//}

	return pRole->GetItemMgr().move_to_other(p_receive->eConTypeSrc, p_receive->n64Serial1, 
									p_receive->eConTypeDst, p_receive->n16PosDst, elcid_item_move_to_other);
}

//------------------------------------------------------------------------
// 按指定顺序排列行囊中物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleReorderItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_item_reorder);
	NET_SIC_item_reorder * p_receive = ( NET_SIC_item_reorder * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 消息合法性弱检测
	if(p_receive->n16ItemNum <= 0 
		|| (p_receive->n16ItemNum > SPACE_ALL_BAG && p_receive->n16ItemNum > SPACE_ALL_WARE))
	{
		return INVALID_VALUE;
	}

	// 创建消息
	INT nSzMsg = sizeof(NET_SIS_item_reorder) + (p_receive->n16ItemNum - 1) * sizeof(INT16);
	CREATE_MSG(pSend, nSzMsg, NET_SIS_item_reorder);

	// 处理
	DWORD dw_error_code = E_Success;
	if(INVALID_VALUE == p_receive->dwNPCID)	// 背包
	{
		if(p_receive->by_type == 1)
		{
			pSend->eConType = EICT_Quest;
			dw_error_code = pRole->GetItemMgr().ReorderQuest(p_receive->n16Index, pSend->n16OldIndex, p_receive->n16ItemNum);
		}
		else
		{
			pSend->eConType = EICT_Bag;
			dw_error_code = pRole->GetItemMgr().ReorderBag(p_receive->n16Index, pSend->n16OldIndex, p_receive->n16ItemNum);
		}
	}
	else	// 仓库
	{
		pSend->eConType = EICT_RoleWare;
		dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
		if(E_Success == dw_error_code)
		{
			dw_error_code = pRole->GetItemMgr().ReorderRoleWare(p_receive->n16Index, pSend->n16OldIndex, p_receive->n16ItemNum);
		}
	}

	// 设置消息
	pSend->n16ItemNum = p_receive->n16ItemNum;
	pSend->dw_error_code = dw_error_code;

	if(dw_error_code != E_Success)
	{
		pSend->dw_size = sizeof(NET_SIS_item_reorder);
	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return dw_error_code;
}

DWORD PlayerSession::HandleRoleReorderItemEx(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_item_reorder_extend);
	NET_SIC_item_reorder_extend * p_receive = ( NET_SIC_item_reorder_extend * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 消息合法性弱检测
	if(p_receive->n16ItemNum <= 0 
		|| (p_receive->n16ItemNum > SPACE_ALL_BAG && p_receive->n16ItemNum > SPACE_ALL_WARE))
	{
		return INVALID_VALUE;
	}

	// 创建消息
	INT max_bag_size = max(SPACE_ALL_BAG, SPACE_ALL_WARE);
	INT nSzMsg = sizeof(NET_SIS_item_reorder_extend) + max_bag_size * sizeof(tagItemOrder) - sizeof(BYTE);
	CREATE_MSG(pSend, nSzMsg, NET_SIS_item_reorder_extend);

	// 处理
	DWORD dw_error_code = E_Success;
	if(INVALID_VALUE == p_receive->dwNPCID)	// 背包
	{
		pSend->eConType = EICT_Bag;
		dw_error_code = pRole->GetItemMgr().ReorderBagEx(p_receive->byData, 
							pSend->byData, pSend->n16ItemNum, p_receive->n16ItemNum);
	}
	else	// 仓库
	{
		pSend->eConType = EICT_RoleWare;
		dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
		if(E_Success == dw_error_code)
		{
			dw_error_code = pRole->GetItemMgr().ReorderRoleWareEx(p_receive->byData, 
							pSend->byData, pSend->n16ItemNum, p_receive->n16ItemNum);
		}
	}

	// 设置消息
	pSend->dw_error_code = dw_error_code;

// 	if(pSend->n16ItemNum != p_receive->n16ItemNum)
// 	{
// 		print_message(_T("\n\nCaution:\n\tRole<id: %u> maybe modified bag order msg!\n"), pRole->GetID());
// 		ASSERT(pSend->n16ItemNum == p_receive->n16ItemNum);

		pSend->dw_size = sizeof(NET_SIS_item_reorder_extend) + pSend->n16ItemNum * sizeof(tagItemOrder) - sizeof(BYTE);
//	}

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return dw_error_code;
}

DWORD	PlayerSession::HandleRoleStackItem(tag_net_message* pCmd)
{
	NET_SIC_stack_item * p_receive = ( NET_SIC_stack_item * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	pRole->GetItemMgr().Stack(p_receive->eConType);

	NET_SIS_stack_item send;
	send.eConType = p_receive->eConType;
	SendMessage(&send, send.dw_size);

	return 0;
}
//------------------------------------------------------------------------
// 拾取物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePickUpItem(tag_net_message* pCmd)
{
	NET_SIC_role_pickup_item* p_receive = (NET_SIC_role_pickup_item*)pCmd;

	if( !VALID_POINT(GetRole()) )
		return INVALID_VALUE;

	NET_SIS_role_pickup_item send;
	send.dw_role_id		= GetRole()->GetID();
	send.n64_serial		= p_receive->n64_serial;
	send.dw_error_code	= GetRole()->PickUpItem(p_receive->n64_serial);
	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------
// 扔掉物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePutDownItem(tag_net_message* pCmd)
{
	NET_SIC_role_putdown_item* p_receive = (NET_SIC_role_putdown_item*)pCmd;

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

	NET_SIS_role_putdown_item send;
	send.dw_error_code = GetRole()->putdown_item(p_receive->n64_serial, p_receive->by_type);
	send.n64_serial = p_receive->n64_serial;
	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------
// 行囊加密
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetBagPsd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_set_bag_password);
	NET_SIC_set_bag_password * p = ( NET_SIC_set_bag_password * ) pCmd ;  	
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_bag_password send;
	
	if(IsHaveBagPsd())	// 已设置
	{
		send.dw_error_code = E_BagPsd_Exist;
	}
	else	// 未设置，设置
	{
		SetBagPsd(p->dwBagPsdCrc);
		//send.dwRoleStateEx	= pRole->m_RoleStateEx.GetState();
		send.dw_error_code	= E_Success;
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 取消行囊加密
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUnsetBagPsd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_unset_bag_password);
	NET_SIC_unset_bag_password * p = ( NET_SIC_unset_bag_password * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_bag_password send;

	if(!IsHaveBagPsd())
	{
		send.dw_error_code = E_BagPsd_NoExist;
	}
	else
	{
		if(GetSafeCode() != p->dwSafeCodeCrc)
		{
			send.dw_error_code = E_BagPsd_SafeCode_Error;
		}
		else
		{
			SetBagPsd(INVALID_VALUE);
			//send.dwRoleStateEx	= pRole->m_RoleStateEx.GetState();
			send.dw_error_code = E_Success;
		}
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 验证行囊密码
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCheckBagPsd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_old_bag_password);
	NET_SIC_old_bag_password * p = ( NET_SIC_old_bag_password * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_bag_password send;
	//send.dwRoleStateEx	= pRole->m_RoleStateEx.GetState();
	send.dw_error_code	= E_BagPsd_OK;

	if(GetBagPsd() != p->dwOldBagPsdCrc)
	{
		send.dw_error_code = E_BagPsd_Error;
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 修改行囊密码
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleResetBagPsd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_reset_bag_password);
	NET_SIC_reset_bag_password * p = ( NET_SIC_reset_bag_password * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_bag_password send;

	if(!IsHaveBagPsd())
	{
		send.dw_error_code = E_BagPsd_NoExist;
	}
	else
	{
		if(p->dwNewBagPsdCrc > 999999999)
		{
			send.dw_error_code = INVALID_VALUE;
		}
		else if(GetBagPsd() != p->dwOldBagPsdCrc)
		{
			send.dw_error_code = E_BagPsd_Error;
		}
		else if(p->dwOldBagPsdCrc == p->dwNewBagPsdCrc)
		{
			send.dw_error_code = INVALID_VALUE;
		}
		else
		{
			SetBagPsd(p->dwNewBagPsdCrc);
			//send.dwRoleStateEx	= pRole->m_RoleStateEx.GetState();
			send.dw_error_code = E_Success;
		}
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 打开背包等需要发送密码
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleOpenBagPsd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_open_bag_password);
	NET_SIC_open_bag_password * p = ( NET_SIC_open_bag_password * ) pCmd ;  
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_bag_password send;

	if(IsHaveBagPsd() && GetBagPsd() != p->dwBagPsdCrc)
	{
		send.dw_error_code = E_BagPsd_Error;
	}
	else
	{
		pRole->SetRoleStateEx(ERSE_BagPsdPass, TRUE);
		//send.dwRoleStateEx	= pRole->m_RoleStateEx.GetState();
		send.dw_error_code	= E_Success;
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 设置行囊密码及发送相关消息
//------------------------------------------------------------------------
VOID PlayerSession::SetBagPsd(DWORD dwNewPsdCrc)
{
	m_sAccountCommon.dw_bag_password_crc_ = dwNewPsdCrc;

	// 向dbserver发消息
	NET_DB2C_change_bag_password send;
	send.dw_account_id = GetSessionID();
	send.dw_bag_password = dwNewPsdCrc;
	g_dbSession.Send(&send, send.dw_size);

	// 成就
	Role *pRole = GetRole();
	if(VALID_POINT(pRole))
	{
		pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_set_bag_password, 1);
	}
	//
	//// 根据新密码设置角色状态
	////pRole->m_RoleStateEx.SetState(ERSE_BagPsdPass);
	//pRole->m_RoleStateEx.SetState(ERSE_BagPsdExist);

	//if(INVALID_VALUE == dwNewPsdCrc)
	//{
	//	pRole->m_RoleStateEx.UnsetState(ERSE_BagPsdExist);
	//}
}

//------------------------------------------------------------------------
// 获得其他玩家的装备信息
//------------------------------------------------------------------------
DWORD PlayerSession::HandleGetSomeoneEquip(tag_net_message* pCmd)
{
	NET_SIC_get_remote_role_equip_info * p_receive = ( NET_SIC_get_remote_role_equip_info * ) pCmd ;  
	DWORD dw_role_id = p_receive->dw_role_id;
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole)) 
	{
		NET_SIS_get_remote_role_equip_info_offline send;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	} 

	//! mwh 2011-08-22 [EEP_RightHand=0, EEP_Ride=15]
	//! 调整代码

	INT nIndex = 0;
	INT nMaxIndex = EEP_Ride + 1; 
	
	INT nMaxSize = sizeof(tagEquipViewInfo)* nMaxIndex  + sizeof(NET_SIS_get_remote_role_equip_info);
	CREATE_MSG(pSend, nMaxSize,NET_SIS_get_remote_role_equip_info);

	pSend->nEquipNum = 0;
	pSend->dw_role_id = p_receive->dw_role_id;

	//gx add 2013.6.5 新增玩家基本属性描述信息
	pSend->nLevel = pRole->get_level();
	//玩家属性
	pSend->nAtt[ERRA_MaxHP]		= pRole->GetAttValue(ERA_MaxHP);
	pSend->nAtt[ERRA_HP]			= pRole->GetAttValue(ERA_HP);
	pSend->nAtt[ERRA_MaxMP]		= pRole->GetAttValue(ERA_MaxMP);
	pSend->nAtt[ERRA_MP]			= pRole->GetAttValue(ERA_MP);
	pSend->nAtt[ERRA_Rage]			= pRole->GetAttValue(ERA_Love);
	pSend->nAtt[ERRA_Speed_XZ]		= pRole->GetAttValue(ERA_Speed_XZ);
	pSend->nAtt[ERRA_Speed_Y]		= pRole->GetAttValue(ERA_Speed_Y);
	pSend->nAtt[ERRA_Speed_Swim]	= pRole->GetAttValue(ERA_Speed_Swim);
	pSend->nAtt[ERRA_Speed_Mount]	= pRole->GetAttValue(ERA_Speed_Mount);
	pSend->nAtt[ERRA_Shape]		= pRole->GetAttValue(ERA_Shape);

	pSend->nAtt[ERRA_HitRate] = pRole->GetAttValue(ERA_HitRate);
	pSend->nAtt[ERRA_Dodge] = pRole->GetAttValue(ERA_Dodge);
	pSend->nAtt[ERRA_Crit_Rate] = pRole->GetAttValue(ERA_Crit_Rate);
	pSend->nAtt[ERRA_UnCrit_Rate] = pRole->GetAttValue(ERA_UnCrit_Rate);
	pSend->nAtt[ERRA_ExAttackMin] = pRole->GetAttValue(ERA_ExAttackMin);
	pSend->nAtt[ERRA_ExAttackMax] = pRole->GetAttValue(ERA_ExAttackMax);
	pSend->nAtt[ERRA_InAttackMin] = pRole->GetAttValue(ERA_InAttackMin);
	pSend->nAtt[ERRA_InAttackMax] = pRole->GetAttValue(ERA_InAttackMax);
	pSend->nAtt[ERRA_ArmorEx] = pRole->GetAttValue(ERA_ArmorEx);
	pSend->nAtt[ERRA_ArmorIn] = pRole->GetAttValue(ERA_ArmorIn);
	pSend->nAtt[ERRA_ExAttack] = pRole->GetAttValue(ERA_ExAttack);
	pSend->nAtt[ERRA_ExDefense] = pRole->GetAttValue(ERA_ExDefense);
	pSend->nAtt[ERRA_InAttack] = pRole->GetAttValue(ERA_InAttack);
	pSend->nAtt[ERRA_InDefense] = pRole->GetAttValue(ERA_InDefense);
	pSend->nAtt[ERRA_Luck] = pRole->GetAttValue(ERA_Luck);
	pSend->nAtt[ERRA_ShengW] = pRole->GetAttValue(ERA_Knowledge);
	pSend->dwGuildID = pRole->GetGuildID();
	pSend->dwSpouseID = pRole->GetSpouseID();//gx add 2013.7.4
	pSend->byClass = (EClassType)pRole->GetClass();
	pSend->bySex = pRole->GetSex();
	pSend->nFightLi = pRole->GetEquipTeamInfo().n32_Rating;//战斗力
	pSend->dwMeili = pRole->get_shihun();//魅力值
	pSend->nRongyu = pRole->GetCurExploits();//荣誉点
	pSend->nPKvalue = pRole->GetClientPKValue();//pk值
	get_fast_code()->memory_copy(&pSend->AvatarEquip, &pRole->GetAvatarEquip(), sizeof(tagAvatarEquip));
	//end
	tagEquipViewInfo* pStart = (tagEquipViewInfo*)pSend->byEquip;
	for(; nIndex < nMaxIndex; ++nIndex)
	{	
		tagEquip* pEquipInfo = pRole->GetItemMgr().GetEquipBarEquip((INT16)nIndex);
		if (!VALID_POINT(pEquipInfo)) continue;

		pStart->dw_data_id = pEquipInfo->dw_data_id;
		pStart->byBind = pEquipInfo->byBind;
		pStart->nLevel = pEquipInfo->equipSpec.nRating;//战斗力 gx modify 2013.8.8
		pStart->byConsolidateLevel = pEquipInfo->equipSpec.byConsolidateLevel;
		pStart->byConsolidateLevelStar = pEquipInfo->equipSpec.byQuality;//品质 gx modify 2013.8.8
		pStart->nUseTimes = pEquipInfo->nUseTimes;
		pStart->byHoldNum = pEquipInfo->equipSpec.byHoleNum;
		pStart->n16MinDmg = pEquipInfo->equipSpec.byLuck;//该装备的幸运gx modify 2013.9.5
		//pStart->n16MaxDmg = pEquipInfo->equipSpec.n16MaxDmg;
		//pStart->n16Armor = pEquipInfo->equipSpec.nRating;
		memcpy(pStart->EquipAttitionalAtt, pEquipInfo->equipSpec.EquipAttitionalAtt, sizeof(pStart->EquipAttitionalAtt));
		memcpy(pStart->dwHoleGemID, pEquipInfo->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);

		++pStart; ++pSend->nEquipNum;
	}
	

	//! 重新计算大小
	pSend->dw_size = 0;
	pSend->dw_size +=  pSend->nEquipNum * sizeof(tagEquipViewInfo);
	pSend->dw_size += sizeof(NET_SIS_get_remote_role_equip_info) - sizeof(BYTE);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
	return E_Success;
}


DWORD	PlayerSession::HandleRoleEquipXili(tag_net_message* pCmd)
{
	NET_SIC_equip_xili * p_receive = ( NET_SIC_equip_xili * ) pCmd ;  
	
	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_xili send;
	send.dw_error_code = pRole->EquipXili(INVALID_VALUE, p_receive->n64EquipID, p_receive->dwType);
	send.n64EquipID = p_receive->n64EquipID;
	memcpy(send.EquipAttitional, pRole->EquipAttitional, sizeof(send.EquipAttitional));

	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD	PlayerSession::HandleRoleEquipXiliChange(tag_net_message* pCmd)
{
	NET_SIS_equip_xili_change * p_receive = ( NET_SIS_equip_xili_change * ) pCmd ;  

	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_xili_change send;
	send.dw_error_code = pRole->EquipXiliChange(p_receive->n64EquipID);
	send.n64EquipID = p_receive->n64EquipID;
	SendMessage(&send, send.dw_size);

	return E_Success;

}

DWORD	PlayerSession::HandleRoleEquipGetWuhuen(tag_net_message* pCmd)
{
	NET_SIC_equip_get_wuhuen* p_receive = (NET_SIC_equip_get_wuhuen*)pCmd;

	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_get_wuhuen send;
	send.dw_error_code = pRole->EquipGetWuhuen(p_receive->n64EquipID, p_receive->dwItemID, p_receive->nNumber);
	send.n64EquipID = p_receive->n64EquipID;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD	PlayerSession::HandleRoleEquipRongLian(tag_net_message* pCmd)
{
	NET_SIC_equip_ronglian* p_receive = (NET_SIC_equip_ronglian*)pCmd;

	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_ronglian send;
	send.dw_error_code = pRole->EquipRonglian(p_receive->n64EquipID);
	send.n64EquipID = p_receive->n64EquipID;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD	PlayerSession::HandleShiPinFumo(tag_net_message* pCmd)
{

	NET_SIC_equip_fumo* p_receive = (NET_SIC_equip_fumo*)pCmd;

	// 获取角色
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_fumo send;
	send.dw_error_code = pRole->Equipfumo(p_receive->n64EquipID, p_receive->dwItemID, p_receive->nNumber);
	send.n64EquipID = p_receive->n64EquipID;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD PlayerSession::HandleEquipChange(tag_net_message* pCmd)
{
	NET_SIC_equip_type_change* p_recv = (NET_SIC_equip_type_change*)pCmd;
	
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_equip_type_change send;
	send.dw_error_code = pRole->EquipChange(p_recv->n64EquipID, p_recv->n64Item1, p_recv->n64Item2, p_recv->n64Item3, send.n64EquipID);

	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD PlayerSession::HandleEquipLuckYou(tag_net_message* pCmd)
{
	NET_SIC_use_luck_you* p_recv = (NET_SIC_use_luck_you*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_use_luck_you send;
	send.dw_error_code = pRole->EquipUseLuckYou(p_recv->n64Item,send.ch_CurLuck,send.ch_ChangeLuck);
	SendMessage(&send, send.dw_size);

	return E_Success;
}
