
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//装备强化
#include "StdAfx.h"
#include "player_session.h"
#include "../common/ServerDefine/consolidate_server_define.h"
#include "../../common/WorldDefine/compose_protocol.h"
#include "../../common/WorldDefine/compose_define.h"
#include "map.h"
#include "role.h"



//------------------------------------------------------------------------
// 物品强化（铭纹）
//------------------------------------------------------------------------
//DWORD	PlayerSession::HandleRolePosyEquip(tag_net_message* pCmd)
//{
//	// 接收网络消息
//	MGET_MSG(p_receive, pCmd, NET_SIC_consolidate_posy);
//
//	// 获取角色
//	Role* pRole = GetRole();
//	if(!VALID_POINT(pRole))
//	{
//		return INVALID_VALUE;
//	}
//
//	// 输入材料个数
//	INT nStuffNum = 1 + ((INT)p_receive->dw_size - sizeof(NET_SIC_consolidate_posy)) / sizeof(INT64);
//
//	DWORD	dw_error_code = pRole->PosyEquip(p_receive->dwNPCID, p_receive->dwFormulaID, p_receive->n64ItemID, \
//												p_receive->n64IMID, p_receive->n64StuffID, nStuffNum, p_receive->dwID);
//
//	if(INVALID_VALUE == dw_error_code)
//	{
//		return dw_error_code;
//	}
//
//	NET_SIS_consolidate_posy	send;
//	send.dw_error_code = dw_error_code;
//	SendMessage(&send, send.dw_size);
//
//	return dw_error_code;
//}

//------------------------------------------------------------------------
// 物品强化（镌刻）
//------------------------------------------------------------------------
//DWORD PlayerSession::HandleRoleEngraveEquip(tag_net_message* pCmd)
//{
//	// 接收网络消息
//	MGET_MSG(p_receive, pCmd, NET_SIC_consolidate_engrave);
//
//	// 获取角色
//	Role* pRole = GetRole();
//	if(!VALID_POINT(pRole))
//	{
//		return INVALID_VALUE;
//	}
//
//	// 输入材料个数
//	INT nStuffNum = 1 + ((INT)p_receive->dw_size - sizeof(NET_SIC_consolidate_engrave)) / sizeof(INT64);
//
//	DWORD	dw_error_code = pRole->EngraveEquip(p_receive->dwNPCID, p_receive->dwFormulaID, p_receive->n64ItemID, \
//													p_receive->n64IMID, p_receive->n64StuffID, nStuffNum, p_receive->dwID);
//
//	if(INVALID_VALUE == dw_error_code)
//	{
//		return dw_error_code;
//	}
//
//	NET_SIS_consolidate_engrave	send;
//	send.dw_error_code = dw_error_code;
//	SendMessage(&send, send.dw_size);
//
//	return dw_error_code;
//}

//------------------------------------------------------------------------
// 物品强化（镶嵌）
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleInlayEquip(tag_net_message* pCmd)
{
	// 接收网络消息
	NET_SIC_inlay* p_receive = (NET_SIC_inlay*)pCmd;

	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD	dw_error_code = pRole->InlayEquip(p_receive->n64DstItemID, p_receive->n64SrcItemID);

	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_inlay	send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}


//------------------------------------------------------------------------
// 生产合成物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleProduceItem(tag_net_message* pCmd)
{
	// 接收网络消息
	NET_SIC_produce* p_receive = (NET_SIC_produce*)pCmd;

	// 获取角色
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

	// 找到合成配方数据
	const s_produce_proto_ser *pProduceProto = AttRes::GetInstance()->GetProduceProto(p_receive->dwFormulaID);
	if(!VALID_POINT(pProduceProto))
		return INVALID_VALUE;

	// 输入材料个数
	//INT nStuffNum = 1 + ((INT)p_receive->dw_size - sizeof(NET_SIC_produce)) / sizeof(INT64);
	
	DWORD	dw_error_code = INVALID_VALUE;
	//const tagEquipProto* pEquipProto = AttRes::GetInstance()->GetEquipProto(pProduceProto->dw_pro_item_data_id);

	//BOOL bFashion = FALSE;
	//if (VALID_POINT(pEquipProto) && MIsFashion(pEquipProto->eEquipPos))
	//{
	//	bFashion = TRUE;
	//}

	dw_error_code = pRole->ProduceItem(p_receive->dwNPCID, 0, 0, p_receive->dwFormulaID,\
		0, p_receive->dw_message_id, 0, 0);
	// 时装走物品的
	//if(VALID_POINT(pEquipProto) && !MIsFashion(pEquipProto->eEquipPos))
	//{
	//	dw_error_code = pRole->ProduceEquip(p_receive->dwNPCID, Skill::GetIDFromTypeID(p_receive->dwSkillID), p_receive->n64ItemID, p_receive->dwFormulaID,\
	//		p_receive->bBind, p_receive->byUseUpQualityItem, p_receive->byQualityNum);
	//}
	//else
	//{
	//	dw_error_code = pRole->ProduceItem(p_receive->dwNPCID, Skill::GetIDFromTypeID(p_receive->dwSkillID), p_receive->n64ItemID, p_receive->dwFormulaID,\
	//		p_receive->bBind, p_receive->dw_message_id, p_receive->byYuanBao, bFashion);
	//}


	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_produce	send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}

//------------------------------------------------------------------------
// 点化,装备合成
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDeCompose(tag_net_message* pCmd)
{
	// 接收网络消息
	NET_SIC_decomposition* p_receive = (NET_SIC_decomposition*)pCmd;

	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 找到合成配方数据
	const s_decompose_proto_ser *pComposeProto = AttRes::GetInstance()->GetDeComposeProto(p_receive->dwFormulaID);
	if(!VALID_POINT(pComposeProto))
		return INVALID_VALUE;

	DWORD	dw_error_code = INVALID_VALUE;
	INT		nStuffNum = 0;

	TCHAR szMsg[1024] = {0};
	NET_SIS_decomposition* send = (NET_SIS_decomposition*)szMsg;
	send->dw_message_id		= get_tool()->crc32("NET_SIS_decomposition");
	
	
	DWORD dwStartTime = timeGetTime();

	dw_error_code = pRole->DeComposeItem(p_receive->dwNPCID, p_receive->dwSkillID, p_receive->n64ItemID, p_receive->dwFormulaID,\
			p_receive->n64IMID, p_receive->n64Item, p_receive->dw_message_id, send->nStuffIndex, nStuffNum, p_receive->bPetDec);
	
	DWORD dwEndTime = timeGetTime();

	DWORD dwMaxPcallTime = dwEndTime - dwStartTime;
	if(dwMaxPcallTime > 0)
	{
		g_world.get_log()->write_log(_T("DeComposeItem useing %d millisecond.\r\n"), dwMaxPcallTime);
	}
	

	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}


	send->dw_error_code = dw_error_code;
	send->dw_size = sizeof(NET_SIS_decomposition) + sizeof(INT)*(nStuffNum - 1);
	SendMessage(send, send->dw_size);

	return dw_error_code;
}

//------------------------------------------------------------------------
// 开凿
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChisel(tag_net_message* pCmd)
{
	// 接收网络消息
	NET_SIC_chisel* p_receive = (NET_SIC_chisel*)pCmd;

	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD	dw_error_code = INVALID_VALUE;

	dw_error_code = pRole->ChiselEquip(p_receive->n64SrcItemID, p_receive->dwNPCID, p_receive->n64StuffID);

	NET_SIS_chisel	send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}


//------------------------------------------------------------------------
// 物品升星
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEquipShengXing(tag_net_message* pCmd)
{
	NET_SIC_shengxing* p_receive = (NET_SIC_shengxing*)pCmd;

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

	DWORD dw_error_code = pRole->ConsolidateEquip(p_receive->dwNPCID, p_receive->n64SerialEquip, p_receive->n64ShengXingItem, p_receive->n64BaohuItem, p_receive->dwStuffID, p_receive->nStuffNum, p_receive->bBind);

	NET_SIS_shengxing send;
	send.n64SerialEquip = p_receive->n64SerialEquip;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;

}

//------------------------------------------------------------------------
// 物品拆嵌
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUnbeset(tag_net_message* pCmd)
{
	NET_SIC_unbeset* p_receive = (NET_SIC_unbeset*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->Unbeset(p_receive->n64SerialEquip, p_receive->dwNPCID, p_receive->byUnBesetPos);

	NET_SIS_unbeset send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}
//-------------------------------------------------------------------------
// 原石打磨
//-------------------------------------------------------------------------
DWORD	PlayerSession::HandleRoleDamo(tag_net_message* pCmd)
{
	NET_SIC_damo* p_receive = (NET_SIC_damo*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->damo(p_receive->dwItemID, p_receive->dwNumber, p_receive->dwNPCID);

	NET_SIS_damo send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return  dw_error_code;
}

//-------------------------------------------------------------------------
// 开始修炼武器
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePracticeBegin(tag_net_message* pCmd)
{
	NET_SIC_practice_begin* p_receive = (NET_SIC_practice_begin*)pCmd;
	
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	DWORD dw_error_code = pRole->PracticeBegin(p_receive->n64SerialEquip, p_receive->dw_speed_item_data_id, p_receive->dw_speed_item_number);

	NET_SIS_practice_begin send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}
//-------------------------------------------------------------------------
// 结束修炼武器
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePracticeEnd(tag_net_message* pCmd)
{
	NET_SIC_parctice_end* p_receive = (NET_SIC_parctice_end*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->PracticeEnd();
	//NET_SIS_parctice_end send;
	//send.dw_error_code = dw_error_code;
	//SendMessage(&send, send.dw_size);

	return dw_error_code;
}

//-------------------------------------------------------------------------
// 武器融合
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleFusion(tag_net_message* pCmd)
{
	NET_SIC_fusion* p_receive = (NET_SIC_fusion*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error_code = pRole->FusionEquip(p_receive->nEquip1, p_receive->nEquip2, p_receive->n64ItemID);

	NET_SIS_fusion send;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;

}

//-------------------------------------------------------------------------
// 武器洗天赋点
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleWeaponClearTalent(tag_net_message* pCmd)
{
	NET_SIC_weapon_clear_talent* p_receive = (NET_SIC_weapon_clear_talent*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pRole->WeaponClearTalent(p_receive->n64ItemID);

	NET_SIS_weapon_clear_talent send;
	//send.n64EquipID = p_receive->n64EquipID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return dw_error_code;
}

//-------------------------------------------------------------------------
// 离线修炼
//-------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLeavePracitice(tag_net_message* pCmd)
{
	NET_SIC_leave_parcitice* p_recv = (NET_SIC_leave_parcitice*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD	dw_error = E_Success;

	dw_error = pRole->SetLeaveParcitice(p_recv->byTimeType, p_recv->byMulType);

	NET_SIS_leave_parcitice send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}