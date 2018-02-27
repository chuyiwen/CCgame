/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//邮件相关
#include "StdAfx.h"

#include "player_session.h"
#include "../../common/WorldDefine/mail_protocol.h"
#include "role.h"
#include "mail_mgr.h"
#include "role_mgr.h"

// 发送邮件
DWORD PlayerSession::HandleSendMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_send_mail);
	NET_SIC_send_mail * p_receive = ( NET_SIC_send_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
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

	DWORD dwError = INVALID_VALUE;

	INT32 nContentTCHAR = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_send_mail, szContent)) / sizeof(TCHAR);

	p_receive->stMail.dwSendRoleID = pRole->GetID();

	DWORD dwRecvRoleID = g_roleMgr.get_role_id(p_receive->dwRecvCrc);
	if(dwRecvRoleID != INVALID_VALUE)
	{
		p_receive->stMail.dwRecvRoleID = dwRecvRoleID;
		dwError = g_mailmgr.CreateMail(p_receive->dwNPCID, pRole, p_receive->stMail, p_receive->szName, p_receive->szContent);
	}
	else
	{
		dwError = E_Mail_RecvRole_NotExist;
	}
	
	if (dwError == E_Success)
	{
		pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_send_mail, 1);
	}
	NET_SIS_send_mail send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	/*if(E_Success ==dwError)
	{		
		Role * pRecvRole = g_roleMgr.GetRolePtrByID(dwRecvRoleID);
		if(VALID_POINT(pRecvRole))
		{
			NET_SIS_inform_new_mail cmd;
			pRecvRole->SendMessage(&cmd, cmd.dw_size);             
		}

	}*/
	return dwError;
}

// 获取邮件列表
DWORD PlayerSession::HandleGetOwnMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_own_mail);
	NET_SIC_get_own_mail * p_receive = ( NET_SIC_get_own_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_mailmgr.GetMailInfo(pRole);

	return E_Success;
}

// 获取邮件简单数据
DWORD PlayerSession::HandleGetOneMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_one_mail);
	NET_SIC_get_one_mail * p_receive = ( NET_SIC_get_one_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_mailmgr.GetOneMailInfo(pRole, p_receive->dwMailID);

	return E_Success;

}

// 获取邮件内容
DWORD PlayerSession::HandleGetMailContent(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_mail_content);
	NET_SIC_get_mail_content * p_receive = ( NET_SIC_get_mail_content * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_mailmgr.GetMailContent(p_receive->dwMailID, pRole);

	return E_Success;
}

// 获取邮件物品
DWORD PlayerSession::HandleGetMailItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_mail_item);
	NET_SIC_get_mail_item * p_receive = ( NET_SIC_get_mail_item * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	g_mailmgr.GetMailItem(p_receive->dwMailID, pRole);

	return E_Success;
}

// 收取邮件附件
DWORD PlayerSession::HandleAcceptAccessoryMailItem(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_accept_accessory_mail);
	NET_SIC_accept_accessory_mail * p_receive = ( NET_SIC_accept_accessory_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	mail* pMail = g_mailmgr.GetMail(pRole->GetID(), p_receive->dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code() && pMail->GetSolve() > 0)
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

	DWORD dwError = pMail->AcceptAccessoryMail(pRole);

	NET_SIS_accept_mail send;
	send.dwMailID = pMail->GetMailAtt().dwID;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 读取邮件
DWORD PlayerSession::HandleReadMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_read_mail);
	NET_SIC_read_mail * p_receive = ( NET_SIC_read_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	mail* pMail = g_mailmgr.GetMail(pRole->GetID(), p_receive->dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->ReadMail();

	pMail->SendMailContent(pRole);

	pMail->SendMailItem(pRole);

	NET_SIS_read_mail send;
	send.dwMailID = pMail->GetMailAtt().dwID;
	send.dwError = E_Success;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 删除邮件
DWORD PlayerSession::HandleDeleteMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_delete_mail);
	NET_SIC_delete_mail * p_receive = ( NET_SIC_delete_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	mail* pMail = g_mailmgr.GetMail(pRole->GetID(), p_receive->dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	DWORD dwError = pMail->DeleteMail();

	NET_SIS_delete_mail send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 退回邮件
DWORD PlayerSession::HandleReturnMail(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_return_mail);
	NET_SIC_return_mail * p_receive = ( NET_SIC_return_mail * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	mail* pMail = g_mailmgr.GetMail(pRole->GetID(), p_receive->dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	if(pMail->GetMailAtt().bWithdrawal)
		return INVALID_VALUE;

	if(pMail->GetMailAtt().dwSendRoleID == INVALID_VALUE)
		return E_Mail_SystemMail_NoCan_Return;

	DWORD dwError = pMail->ReturnMail();

	/*if(dwError == E_Success)
	{
		Role* pRole = g_roleMgr.GetRolePtrByID(pMail->GetMailAtt().dwRecvRoleID);
		if(VALID_POINT(pRole))
		{
			INT32 n32MailNum = 0;
			BOOL bRead = FALSE;
			g_mailmgr.GetRoleMailNum(n32MailNum, bRead, pMail->GetMailAtt().dwRecvRoleID);

			NET_SIS_get_mail_num send;
			send.bRead = bRead;
			send.dwMainNum = n32MailNum;
			pRole->SendMessage(&send, send.dw_size);
		}
	}*/

	NET_SIS_return_mail send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);
	return E_Success;
}

// 获取邮件数量
DWORD PlayerSession::HandleGetMailNum(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_mail_num);
	NET_SIC_get_mail_num * p_receive = ( NET_SIC_get_mail_num * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	INT32 n32MailNum = 0;
	BOOL bRead = FALSE;
	g_mailmgr.GetRoleMailNum(n32MailNum, bRead, p_receive->dw_role_id);

	NET_SIS_get_mail_num send;
	send.bRead = bRead;
	send.dwMainNum = n32MailNum;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}