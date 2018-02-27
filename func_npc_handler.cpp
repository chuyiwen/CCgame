
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//部分职能NPC消息处理
#include "StdAfx.h"
#include "../../common/WorldDefine/func_npc_define.h"
#include "../../common/WorldDefine/function_npc_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/common_server_define.h"
#include "role.h"
#include "creature.h"

//-----------------------------------------------------------------------------
// 驿站(乾坤石)消息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDak(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_posthouse);
	NET_SIC_posthouse * p_receive = ( NET_SIC_posthouse * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->nIndex < 0 || p_receive->nIndex >= MAX_DAK_SITE_NUM)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_posthouse	send;
	send.dw_error_code	= pRole->ProcDak(p_receive->dwNPCID, p_receive->nIndex, p_receive->dwMapID, p_receive->by_type, p_receive->dwPosID);

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

// 副本扫荡
DWORD	PlayerSession::HandleRoleInstanceSaodang(tag_net_message* pCmd)
{
	NET_SIC_instance_saodang * p_receive = ( NET_SIC_instance_saodang * ) pCmd ;  
	// 消息合法性验证
	if(p_receive->nIndex < 0 || p_receive->nIndex >= MAX_DAK_SITE_NUM)
	{
		return INVALID_VALUE;
	}

	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_instance_saodang	send;
	send.dw_error_code	= pRole->InstanceSaodang(p_receive->nIndex);
	send.nIndex = p_receive->nIndex;
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;

}

DWORD	PlayerSession::HandleRoleSaodangOver(tag_net_message* pCmd)
{	
	NET_SIC_saodang_over * p_receive = ( NET_SIC_saodang_over * ) pCmd ;  


	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_saodang_over	send;
	send.dw_error_code	= pRole->InstanceSaodangOver();
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}
//-----------------------------------------------------------------------------
// 打开仓库
//-----------------------------------------------------------------------------
//DWORD PlayerSession::HandleRoleWareOpen(tag_net_message* pCmd)
//{
//	MGET_MSG(p_receive, pCmd, NC_);
//
//	// 获取人物
//	Role* pRole = GetRole();
//	if(!VALID_POINT(pRole))
//	{
//		return INVALID_VALUE;
//	}
//
//	tagNS_	send;
//	send.dw_error_code = pRole->(p_receive);
//
//	SendMessage(&send, send.dw_size);
//
//	return send.dw_error_code;
//}

//-----------------------------------------------------------------------------
// 仓库扩容
////-----------------------------------------------------------------------------
//DWORD PlayerSession::HandleRoleWareExtend(tag_net_message* pCmd)
//{
//	MGET_MSG(p_receive, pCmd, NET_SIC_ware_extend);
//
//	// 获取人物
//	Role* pRole = GetRole();
//	if(!VALID_POINT(pRole))
//	{
//		return INVALID_VALUE;
//	}
//
//	NET_SIS_ware_extend send;
//	send.dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
//	if(E_Success == send.dw_error_code)
//	{
//		send.dw_error_code = pRole->GetItemMgr().ExtendRoleWare(p_receive->bUseSilver);
//		send.n16WareNum = pRole->GetItemMgr().GetWareCurSize();
//	}
//
//	SendMessage(&send, send.dw_size);
//
//	if(E_Success == send.dw_error_code)
//	{
//		tagNDBC_WareSizeUpdate sendDB;
//		sendDB.dw_account_id	= GetSessionID();
//		sendDB.n16WareSize	= pRole->GetItemMgr().GetWareCurSize();
//		g_dbSession.Send(&sendDB, sendDB.dw_size);
//	}
//
//	return send.dw_error_code;
//}

//-----------------------------------------------------------------------------
// 背包扩容
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBagExtand(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_bag_extend);
	NET_SIC_bag_extend * p_receive = ( NET_SIC_bag_extend * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_bag_extend send;
	
	send.dw_error_code = pRole->GetItemMgr().ExtendBag(p_receive->n64ItemSerial, pRole, p_receive->n32_type);
	send.n32_type = p_receive->n32_type;
	if(send.n32_type == 1)
	{
		send.n16BagNum = pRole->GetItemMgr().GetWareCurSize();
	}
	else
	{
		send.n16BagNum = pRole->GetItemMgr().GetBagCurSize();
	}
	

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 角色仓库中存金钱
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSaveSilver(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_save_silver);
	NET_SIC_save_silver * p_receive = ( NET_SIC_save_silver * ) pCmd ;  
	// 获取人物	
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 金钱合法性检查
	if(p_receive->n64SilverSave <= 0
		|| p_receive->n64SilverSave > pRole->GetCurMgr().GetBagSilver() 
		|| p_receive->n64SilverSave > pRole->GetCurMgr().GetCanIncWareSilver())
	{
		return INVALID_VALUE;
	}

	NET_SIS_save_silver send;
	send.dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
	if(E_Success == send.dw_error_code)
	{
		pRole->GetCurMgr().DecBagSilver(p_receive->n64SilverSave, elcid_ware_save_silver);
		pRole->GetCurMgr().IncWareSilver(p_receive->n64SilverSave, elcid_ware_save_silver);
	}

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 角色仓库中存取金钱&元宝
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetSilver(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_silver);
	NET_SIC_get_silver * p_receive = ( NET_SIC_get_silver * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 金钱合法性检查
	if(p_receive->n64SilverGet <= 0
		|| p_receive->n64SilverGet > pRole->GetCurMgr().GetWareSilver() 
		|| p_receive->n64SilverGet > pRole->GetCurMgr().GetCanIncBagSilver())
	{
		return INVALID_VALUE;
	}

	NET_SIS_get_silver send;
	send.dw_error_code = E_Success;

	//if(!pRole->get_check_safe_code())
	//{
	//	if(p_receive->dw_safe_code != GetBagPsd())
	//	{
	//		send.dw_error_code = E_Safe_Code_Error;
	//		goto exit;
	//	}

	//	NET_SIS_code_check_ok send_check;
	//	send_check.bSuccess = TRUE;
	//	pRole->SendMessage(&send_check, send_check.dw_size);

	//	pRole->set_check_safe_code();
	//}
	
	send.dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
	if(E_Success == send.dw_error_code)
	{
		pRole->GetCurMgr().DecWareSilver(p_receive->n64SilverGet, elcid_ware_get_silver);
		pRole->GetCurMgr().IncBagSilver(p_receive->n64SilverGet, elcid_ware_get_silver);
	}

exit:
	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 角色仓库中存元宝
//------------------------------------------------------------------------
//DWORD PlayerSession::HandleRoleSaveYuanBao(tag_net_message* pCmd)
//{
//	MGET_MSG(p_receive, pCmd, NC_SaveYuanBao);
//
//	// 获取人物
//	Role* pRole = GetRole();
//	if(!VALID_POINT(pRole))
//	{
//		return INVALID_VALUE;
//	}
//
//	// 金钱合法性检查
//	if(p_receive->nYuanBaoSave <= 0
//		|| p_receive->nYuanBaoSave > pRole->GetCurMgr().GetBagYuanBao() 
//		|| p_receive->nYuanBaoSave > pRole->GetCurMgr().GetCanIncWareYuanBao())
//	{
//		return INVALID_VALUE;
//	}
//
//	NET_SIS_save_yuan_bao send;
//	send.dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Ware);
//	if(E_Success == send.dw_error_code)
//	{
//		pRole->GetCurMgr().DecBagYuanBao(p_receive->nYuanBaoSave, ELCID_RoleWare_SaveYuanBao);
//		pRole->GetCurMgr().IncWareYuanBao(p_receive->nYuanBaoSave, ELCID_RoleWare_SaveYuanBao);
//	}
//
//	SendMessage(&send, send.dw_size);
//
//	return send.dw_error_code;
//}

//------------------------------------------------------------------------
// 角色仓库中存取金钱&元宝
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetYuanBao(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_yuan_bao);
	NET_SIC_get_yuan_bao * p_receive = ( NET_SIC_get_yuan_bao * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 金钱合法性检查
	if(p_receive->nYuanBaoGet <= 0
		|| p_receive->nYuanBaoGet > pRole->GetCurMgr().GetBaiBaoYuanBao() 
		|| p_receive->nYuanBaoGet > pRole->GetCurMgr().GetCanIncBagYuanBao())
	{
		return INVALID_VALUE;
	}

	pRole->GetCurMgr().DecBaiBaoYuanBao(p_receive->nYuanBaoGet, elcid_ware_get_yuanbao);
	pRole->GetCurMgr().IncBagYuanBao(p_receive->nYuanBaoGet, elcid_ware_get_yuanbao);

	//tagNS_GetYuanBao send;
	//send.dw_error_code = E_Success;
	//SendMessage(&send, send.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------
// 使用磨石
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleAbrase(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_abrase_stone);
	NET_SIC_abrase_stone * p_receive = ( NET_SIC_abrase_stone * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_abrase_stone send;
	send.dw_error_code = pRole->AbraseWeapon(p_receive->n64AbraserSerial);
	//send.n64WeaponSerial = p_receive->n64WeaponSerial;

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}