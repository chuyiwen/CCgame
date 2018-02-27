
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ｻ��
#include "StdAfx.h"
#include "player_session.h"
#include "../../common/WorldDefine/exchange_define.h"
#include "../../common/WorldDefine/pet_exchange_protocot.h"
#include "map.h"
#include "role.h"

//------------------------------------------------------------------------
// ������
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeReq(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_request);
	NET_SIC_pet_exchange_request * p_receive = ( NET_SIC_pet_exchange_request * ) pCmd ;  
	if(INVALID_VALUE == p_receive->dwDstID)
	{
		return INVALID_VALUE;
	}

	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role* pTarget = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeReq(pTarget, p_receive->dwDstID);
	if(E_Success == dw_error_code)
	{
		if(VALID_POINT(pTarget))
		{
			// �������߷���Ϣ
			NET_SIS_pet_exchange_request send;
			send.dwSrcID = pRole->GetID();
			pTarget->SendMessage(&send, send.dw_size);
		}
		else
		{
			// ��Զ����ִ�е���
			ASSERT(VALID_POINT(pTarget));
		}
	}
	else if(dw_error_code != INVALID_VALUE)
	{
		//// �������߷���
		//NET_SIS_exchange_finish send;
		//send.dw_error_code	= dw_error_code;
		//send.dwFailedRoleID	= p_receive->dwDstID;
		//SendMessage(&send, send.dw_size);

		NET_SIS_pet_exchange_request_result send;
		send.dw_error_code	= dw_error_code;
		send.dwSrcID		= pRole->GetID();
		send.dwDstID		= p_receive->dwDstID;
		SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// ��Ҷ������׵ķ���
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeReqRes(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_request_result);
	NET_SIC_pet_exchange_request_result * p_receive = ( NET_SIC_pet_exchange_request_result * ) pCmd ;  
	if(INVALID_VALUE == p_receive->dwDstID)
	{
		return INVALID_VALUE;
	}

	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pApplicant = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeReqRes(pApplicant, p_receive->dwDstID, p_receive->dw_error_code);

	// ����Ƿ���Ҫ���ý���״̬
	if(dw_error_code != E_Pet_Exchange_Accept)
	{
		pRole->GetPetExchMgr().SetTgtRoleID(INVALID_VALUE);
	}

	// ����Ƿ���Ҫ��������Ϣ
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	// Ϊ�򻯷��������߼��������������˫��������Ϣ���ɿͻ��˴���
	NET_SIS_pet_exchange_request_result send;
	send.dwSrcID = pRole->GetID();
	send.dwDstID = p_receive->dwDstID;
	send.dw_error_code = dw_error_code;

	if(E_Pet_Exchange_Accept == p_receive->dw_error_code)
	{
		SendMessage(&send, send.dw_size);
	}

	if(VALID_POINT(pApplicant))
	{
		pApplicant->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// ��ӽ�����Ʒ
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeAdd(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_add);
	NET_SIC_pet_exchange_add * p_receive = ( NET_SIC_pet_exchange_add * ) pCmd ;  
	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role	*pTarget	= NULL;
	DWORD	dw_error_code = pRole->ProcPetExchangeAdd(pTarget, p_receive->dwPetID);

	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	if(E_Success == dw_error_code)
	{
		NET_SIS_exchange_add_to_dest send2Dst;
		send2Dst.dwSrcID	= pRole->GetID();
		send2Dst.dwPetID	= p_receive->dwPetID;

		pTarget->SendMessage(&send2Dst, send2Dst.dw_size);
	}

	NET_SIS_pet_exchange_add_to_src send2Src;
	send2Src.dwDstID		= VALID_POINT(pTarget) ? pTarget->GetID() : INVALID_VALUE;
	send2Src.dwPetID		= p_receive->dwPetID;
	send2Src.dw_error_code	= dw_error_code;

	SendMessage(&send2Src, send2Src.dw_size);

	return dw_error_code;
}

//------------------------------------------------------------------------
// ȡ��������Ʒ
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeDec(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_subtract);
	NET_SIC_pet_exchange_subtract * p_receive = ( NET_SIC_pet_exchange_subtract * ) pCmd ;  
	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeDec(pTarget, p_receive->dwPetID);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_pet_exchange_subtract send;
	send.dwSrcID	= pRole->GetID();
	send.dwDstID	= VALID_POINT(pTarget) ? pTarget->GetID() : INVALID_VALUE;
	send.dwPetID	= p_receive->dwPetID;

	// �ӽ����嵥��ɾ��
	send.dw_error_code = dw_error_code;

	SendMessage(&send, send.dw_size);

	if(VALID_POINT(pTarget))
	{
		NET_SIS_pet_exchange_subtract_to_dest send2Dst;
		send2Dst.dwSrcID	= pRole->GetID();
		send2Dst.dwPetID	= p_receive->dwPetID;

		pTarget->SendMessage(&send2Dst, send2Dst.dw_size);
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// �޸Ľ��׽�Ǯ
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeMoney(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_money);
	NET_SIC_pet_exchange_money * p_receive = ( NET_SIC_pet_exchange_money * ) pCmd ;  
	if(p_receive->n64Money < 0)
	{
		return INVALID_VALUE;
	}

	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeMoney(pTarget, p_receive->n64Money);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_pet_exchange_money send;
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
// ��������
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeLock(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_lock);
	NET_SIC_pet_exchange_lock * p_receive = ( NET_SIC_pet_exchange_lock * ) pCmd ;  
	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeLock(pTarget);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_pet_exchange_lock send;
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
// ���ȡ������
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeCancel(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_cancel);
	NET_SIC_pet_exchange_cancel * p_receive = ( NET_SIC_pet_exchange_cancel * ) pCmd ;  
	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dw_error_code = pRole->ProcPetExchangeCancel(pTarget);

	NET_SIS_pet_exchange_cancel send;
	send.dwSrcID = pRole->GetID();

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget) && E_Success == dw_error_code)
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return E_Success;
}

//------------------------------------------------------------------------
// ȷ�Ͻ���
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRolePetExchangeVerify(tag_net_message* pCmd)
{
	// ����������Ϣ
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_pet_exchange_verify);
	NET_SIC_pet_exchange_verify * p_receive = ( NET_SIC_pet_exchange_verify * ) pCmd ;  
	// ��ȡ��ɫ
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Role *pTarget = NULL;
	DWORD dwFailedRoleID = INVALID_VALUE;
	DWORD dw_error_code = pRole->ProcPetExchangeVerify(pTarget, dwFailedRoleID);
	if(INVALID_VALUE == dw_error_code)
	{
		// ������ͻ��˷���
		return dw_error_code;
	}

	NET_SIS_pet_exchange_finish send;
	send.dw_error_code	= dw_error_code;
	send.dwFailedRoleID	= dwFailedRoleID;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}