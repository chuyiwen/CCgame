
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家间交易消息处理
#include "StdAfx.h"
#include "player_session.h"
#include "../../common/WorldDefine/exchange_define.h"
#include "../../common/WorldDefine/exchange_protocol.h"
#include "item_mgr.h"
#include "map.h"
#include "role.h"

//------------------------------------------------------------------------
// 请求交易
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeReq(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_request);
	NET_SIC_exchange_request * p_receive = ( NET_SIC_exchange_request * ) pCmd ;  
	if(INVALID_VALUE == p_receive->dwDstID)
	{
		return INVALID_VALUE;
	}
	
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	Role* pTarget = NULL;
	DWORD dw_error_code = pRole->ProcExchangeReq(pTarget, p_receive->dwDstID);
	if(E_Success == dw_error_code)
	{
		if(VALID_POINT(pTarget))
		{
			// 向被申请者发消息
			NET_SIS_exchange_request send;
			send.dwSrcID = pRole->GetID();
			pTarget->SendMessage(&send, send.dw_size);
			//向邀请者发消息，gx add 2013.8.20
			SendMessage(&send, send.dw_size);
		}
		else
		{
			// 永远不会执行到此
			ASSERT(VALID_POINT(pTarget));
		}
	}
	else if(dw_error_code != INVALID_VALUE)
	{
		//// 向申请者反馈
		//NET_SIS_exchange_finish send;
		//send.dw_error_code	= dw_error_code;
		//send.dwFailedRoleID	= p_receive->dwDstID;
		//SendMessage(&send, send.dw_size);

		NET_SIS_exchange_request_result send;
		send.dw_error_code	= dw_error_code;
		send.dwSrcID		= pRole->GetID();
		send.dwDstID		= p_receive->dwDstID;
		SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 玩家对请求交易的反馈
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeReqRes(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_request_result);
	NET_SIC_exchange_request_result * p_receive = ( NET_SIC_exchange_request_result * ) pCmd ;  
	if(INVALID_VALUE == p_receive->dwDstID)
	{
		return INVALID_VALUE;
	}
	
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pApplicant = NULL;
	DWORD dw_error_code = pRole->ProcExchangeReqRes(pApplicant, p_receive->dwDstID, p_receive->dw_error_code);

	// 检查是否需要重置交易状态
	if(dw_error_code != E_Exchange_Accept)
	{
		pRole->GetExchMgr().SetTgtRoleID(INVALID_VALUE);
	}
	
	// 检查是否需要发反馈消息
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	// 为简化服务器端逻辑，所以情况均向双方发送消息，由客户端处理
	NET_SIS_exchange_request_result send;
	send.dwSrcID = pRole->GetID();
	send.dwDstID = p_receive->dwDstID;
	send.dw_error_code = dw_error_code;

	if(E_Exchange_Accept == p_receive->dw_error_code)
	{
		if (E_Exchange_NotAnswer != dw_error_code)
		{
			SendMessage(&send, send.dw_size);
		}
	}
	//gx add 2013.9.4
	if (E_Exchange_NotAnswer == dw_error_code)
	{
		SendMessage(&send, send.dw_size);
	}
	if(VALID_POINT(pApplicant))
	{
		if (E_Exchange_NotAnswer != dw_error_code)
		{
			pApplicant->SendMessage(&send, send.dw_size);
		}
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 添加交易物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeAdd(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_add_exchange);
	NET_SIC_add_exchange * p_receive = ( NET_SIC_add_exchange * ) pCmd ;  
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role	*pTarget	= NULL;
	tagItem *pItem		= NULL;
	INT32	nInsIndex	= INVALID_VALUE;
	DWORD	dw_error_code = pRole->ProcExchangeAdd(pTarget, pItem, nInsIndex, p_receive->n64_serial);

	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}
	
	if(E_Success == dw_error_code)
	{
		NET_SIS_add_exchange_to_dest send2Dst;
		send2Dst.dwSrcID	= pRole->GetID();
		send2Dst.nIndex		= nInsIndex;

		// 判断交易的是装备还是物品
		if(MIsEquipment(pItem->dw_data_id))
		{
			get_fast_code()->memory_copy(send2Dst.byData, pItem, SIZE_EQUIP);
		}
		else
		{
			get_fast_code()->memory_copy(send2Dst.byData, pItem, SIZE_ITEM);
			send2Dst.dw_size = send2Dst.dw_size - SIZE_EQUIP + SIZE_ITEM;
		}

		pTarget->SendMessage(&send2Dst, send2Dst.dw_size);
	}

	NET_SIS_add_exchange_to_src send2Src;
	send2Src.dwDstID		= VALID_POINT(pTarget) ? pTarget->GetID() : INVALID_VALUE;
	send2Src.n64_serial		= p_receive->n64_serial;
	send2Src.dw_error_code	= dw_error_code;
	send2Src.nIndex			= nInsIndex;
	
	SendMessage(&send2Src, send2Src.dw_size);

	return dw_error_code;
}

//------------------------------------------------------------------------
// 取消交易物品
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeDec(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_cancel_exchange_item);
	NET_SIC_cancel_exchange_item * p_receive = ( NET_SIC_cancel_exchange_item * ) pCmd ;  
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcExchangeDec(pTarget, p_receive->n64_serial);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}
	
	NET_SIS_cancel_exchange_item send;
	send.dwSrcID	= pRole->GetID();
	send.dwDstID	= VALID_POINT(pTarget) ? pTarget->GetID() : INVALID_VALUE;
	send.n64_serial	= p_receive->n64_serial;

	// 从交易清单中删除
	send.dw_error_code = dw_error_code;

	SendMessage(&send, send.dw_size);

	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 修改交易金钱
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeMoney(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_money);
	NET_SIC_exchange_money * p_receive = ( NET_SIC_exchange_money * ) pCmd ;  
	if(p_receive->n64Money < 0)
	{
		return INVALID_VALUE;
	}

	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcExchangeMoney(pTarget, p_receive->n64Money);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_exchange_money send;
	send.dwSrcID	= pRole->GetID();
	send.n64Money	= p_receive->n64Money;
	send.dw_error_code= dw_error_code;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 锁定交易
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeLock(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_item_lock);
	NET_SIC_exchange_item_lock * p_receive = ( NET_SIC_exchange_item_lock * ) pCmd ;  
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcExchangeLock(pTarget);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_exchange_item_lock send;
	send.dwSrcID = pRole->GetID();
	send.dw_error_code = dw_error_code;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// 玩家取消交易
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeCancel(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_cancel);
	NET_SIC_exchange_cancel * p_receive = ( NET_SIC_exchange_cancel * ) pCmd ;  
	// 获取角色	
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcExchangeCancel(pTarget);

	NET_SIS_exchange_cancel send;
	send.dwSrcID = pRole->GetID();

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget) && E_Success == dw_error_code)
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return E_Success;
}

//------------------------------------------------------------------------
// 确认交易
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleExchangeVerify(tag_net_message* pCmd)
{
	// 接收网络消息
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_exchange_verify);
	NET_SIC_exchange_verify * p_receive = ( NET_SIC_exchange_verify * ) pCmd ;  
	// 获取角色
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	//gx modify 2013.7.1
	/*if(!pRole->get_check_safe_code())
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
	}*/
	
	Role *pTarget = NULL;
	DWORD dwFailedRoleID = INVALID_VALUE;
	DWORD dw_error_code = pRole->ProcExchangeVerify(pTarget, dwFailedRoleID);


	if(INVALID_VALUE == dw_error_code)
	{
		// 无需向客户端返回
		return dw_error_code;
	}

	NET_SIS_exchange_finish send;
	send.dw_error_code	= dw_error_code;
	send.dwFailedRoleID	= dwFailedRoleID;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}