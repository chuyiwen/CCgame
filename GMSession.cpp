/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		GMSession
 *	@author		mwh
 *	@date		2011/04/07	initial
 *	@version	0.0.1.0
 *	@brief		连接GM服务器
*/

#include "StdAfx.h"
#include "../common/ServerDefine/GMServerMessage.h"
#include "../../common/WorldDefine/chat_define.h"
#include "../../common/WorldDefine/chat_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"

#include "world.h"
#include "world_session.h"
#include "GMSession.h"
#include "role.h"
#include "role_mgr.h"
#include "item_creator.h"
#include "gm_policy.h"
#include "mail_mgr.h"
#include "db_session.h"

#define GMSESSIONCONNECTTHREADNAME _T("GMSessionConnectGMServer")
#define UniCode2Unicode8(Str) get_tool()->unicode_to_unicode8(Str)

GMSession g_rtSession;

GMSession::GMSession()
:mPort(0),mWordID(0),mSectionID(0),mSendStatusTick(STATUSREPORTINTERVAL)
,mTerminateConnect(FALSE),mInit(FALSE),mNetSession(0)
{
}

GMSession::~GMSession()
{

}

BOOL GMSession::Init()
{
	// 初始化成员属性
	mIP	= World::p_var->get_string(_T("ip gm_server"));
	mPort =	World::p_var->get_dword(_T("port gm_server"));
	mWordID	= World::p_var->get_dword(_T("id world"));
	mSectionID = World::p_var->get_dword(_T("section_id world"));

	mNetSession = new few_connect_client;
	ASSERT(VALID_POINT(mNetSession));
	if(!VALID_POINT(mNetSession))
	{
		ERROR_CLUE_ON(_T("\n\nfatal error on create net session!!\n\n"));
		return FALSE;
	}

	this->RegisterCmd();

	if(!mNetSession->init()) return FALSE;
	if(!CreateConnectThread()) return FALSE;

	return TRUE;
}

VOID GMSession::Destroy()
{
	
	Interlocked_Exchange((LONG*)&mTerminateConnect, TRUE);
	World::p_thread->waitfor_thread_destroy(GMSESSIONCONNECTTHREADNAME, INFINITE);

	mNetSession->destory();
	SAFE_DELETE(mNetSession);

	// 注销消息管理
	UnregisterCmd();
}

VOID GMSession::Update()
{
	this->UpdateSession();
	this->SendServerInfo();
}

VOID GMSession::UpdateSession()
{
	if(!VALID_POINT(mNetSession))
		return;

	if(!mNetSession->is_connect()&&
	   !World::p_thread->is_thread_active(GMSESSIONCONNECTTHREADNAME))
	{
		ReConnect(); return;
	}

	if(mNetSession->is_connect())
		HandleMessage();
}

UINT GMSession::ThreadConnect()
{
	ASSERT(VALID_POINT(mNetSession));

	THROW_EXCEPTION_START

	while(!mTerminateConnect && !mNetSession->is_connect())
	{
		if( !mNetSession->is_trying_create_connect() )
		{
			mNetSession->try_create_connect(UniCode2Unicode8(mIP.c_str()), mPort);
		}
		Sleep(100); 
		continue;
	}
	
	if(mNetSession->is_connect())
	{
		print_message(_T("connected to GMServer %s:%4d\r\n"), mIP.c_str(), mPort);

		NET_S2GMS_SeverLogin send;
		send.dw_crc = get_tool()->crc32(g_world.GetWorldName());
		send.e_type = EST_GAMESERVER;
		mNetSession->send_msg(&send, send.dw_size);
	}

	THROW_EXCEPTION_END
	
	::_endthreadex(0);

	return 0;
}

UINT GMSession::ThreadCall(LPVOID lpVoid)
{
	ASSERT(VALID_POINT(lpVoid));
	return static_cast<GMSession*>(lpVoid)->ThreadConnect();
}

BOOL GMSession::ReConnect()
{
	Interlocked_Exchange((LONG*)&mTerminateConnect, TRUE);

	mNetSession->disconnect();
	World::p_thread->waitfor_thread_destroy(GMSESSIONCONNECTTHREADNAME, INFINITE);

	// 重新启动登陆服务器连接线程
	Interlocked_Exchange((LONG*)&mTerminateConnect, FALSE);
	if(!this->CreateConnectThread()) return FALSE;
	return TRUE;
}

BOOL GMSession::CreateConnectThread()
{
	if(!World::p_thread->create_thread(GMSESSIONCONNECTTHREADNAME, &GMSession::ThreadCall, this))
		return FALSE;

	while(!World::p_thread->is_thread_active(GMSESSIONCONNECTTHREADNAME))
		continue;

	return TRUE;
}

VOID GMSession::HandleMessage()
{
	DWORD dwSize = 0;
	net_command_manager& netMan = serverframe::net_command_manager::get_singleton();
	LPBYTE lpMsg = mNetSession->recv_msg(dwSize);
	while(VALID_POINT(lpMsg))
	{
		// 处理消息
		if (!netMan.handle_message((tag_net_message*)lpMsg,dwSize, INVALID_VALUE))
			print_message(_T("gmSession unkown msg"));

		// 回收资源
		mNetSession->free_recv_msg(lpMsg);

		lpMsg = mNetSession->recv_msg(dwSize);
	}
}

VOID GMSession::SendServerInfo()
{
	if(!mNetSession->is_connect() || !mInit)
		return;

	--mSendStatusTick;
	if(mSendStatusTick > 0) return;

	mSendStatusTick = STATUSREPORTINTERVAL;

	NET_S2GMS_ServerInfo send;
	send.e_status = g_world.IsWell() ? ews_well : ews_system_error;
	send.n_max_online_num = g_worldSession.GetPlayerNumLimit();
	send.n_online_num = g_worldSession.GetPlayerNumCurrent();
	send.e_db_status = g_dbSession.IsWell() ? ews_well : ews_system_error;
	send.is_database_ok = TRUE; //!g_world.is_db_connect( );
	mNetSession->send_msg(&send, send.dw_size);
}
//=================================================
//	消息处理注册
//=================================================
VOID GMSession::RegisterCmd()
{
	REGISTER_NET_MSG("NET_GMS2S_SeverLogin",	GMSession,	HandleServerLogin,	_T("NET_GMS2S_SeverLogin"));
	REGISTER_NET_MSG("NET_GMS2S_forbid_Talk",	GMSession,	HandleForbidTalk,	_T("NET_GMS2S_forbid_Talk"));
	REGISTER_NET_MSG("NET_GMS2S_Affiche",		GMSession,	HandleAfficheMsg,	_T("NET_GMS2S_Affiche"));
	REGISTER_NET_MSG("NET_GMS2S_Kick",			GMSession,	HandleGMKick,		_T("NET_GMS2S_Kick"));
	REGISTER_NET_MSG("NET_GMS2S_ShutDown",		GMSession,  HandleShutDown,		_T("NET_GMS2S_ShutDown"));
	REGISTER_NET_MSG("NET_GMS2S_Add_Item",		GMSession,	HandleAddItem,	_T("NET_GMS2S_Add_Item"));
	REGISTER_NET_MSG("NET_GMS2S_SetDoublePolicy",		GMSession,	HandleSetDoublePolicy,	_T("NET_GMS2S_SetDoublePolicy"));
	REGISTER_NET_MSG("NET_GMS2S_SetMaxPlayerNumber",		GMSession,	HandleSetMaxPlayerNumber, _T("set world's max player"));
	REGISTER_NET_MSG("NET_GMS2S_GMCreateEquip",		GMSession,	HandleGMToolCreateEquip, _T("NET_GMS2S_GMCreateEquip"));
	REGISTER_NET_MSG("NET_GMS2S_GMCreateItem",		GMSession,	HandleGMToolCreateItem, _T("NET_GMS2S_GMCreateItem"));
	REGISTER_NET_MSG("NET_GMS2S_GMGiveMoney",		GMSession,	HandleGMToolGiveMoney, _T("NET_GMS2S_GMGiveMoney"));
	REGISTER_NET_MSG("NET_GMS2S_validate",		GMSession,	HandleGMValidate, _T("NET_GMS2S_validate"));
}
VOID GMSession::UnregisterCmd()
{
	UNREGISTER_NET_MSG("NET_GMS2S_SeverLogin",	GMSession,	HandleServerLogin);
	UNREGISTER_NET_MSG("NET_GMS2S_forbid_Talk", GMSession,	HandleForbidTalk);
	UNREGISTER_NET_MSG("NET_GMS2S_Affiche",		GMSession,	HandleAfficheMsg);
	UNREGISTER_NET_MSG("NET_GMS2S_Kick",		GMSession,	HandleGMKick);
	UNREGISTER_NET_MSG("NET_GMS2S_ShutDown",		GMSession,  HandleShutDown);
	UNREGISTER_NET_MSG("NET_GMS2S_Add_Item",		GMSession,	HandleAddItem);
	UNREGISTER_NET_MSG("NET_GMS2S_SetDoublePolicy",		GMSession,	HandleSetDoublePolicy);
	UNREGISTER_NET_MSG("NET_GMS2S_SetMaxPlayerNumber",		GMSession,	HandleSetMaxPlayerNumber);
	UNREGISTER_NET_MSG("NET_GMS2S_GMCreateEquip",		GMSession,	HandleGMToolCreateEquip);
	UNREGISTER_NET_MSG("NET_GMS2S_GMCreateItem",		GMSession,	HandleGMToolCreateItem);
	UNREGISTER_NET_MSG("NET_GMS2S_GMGiveMoney",		GMSession,	HandleGMToolGiveMoney);
	UNREGISTER_NET_MSG("NET_GMS2S_validate",		GMSession,	HandleGMValidate);
}

Role* GetRole(const TCHAR* pRoleName)
{
	TCHAR name[X_SHORT_NAME]={0};
	_tcscpy_s(name, _countof(name), pRoleName);
	_tcslwr(name);
	DWORD dwRoleID = g_roleMgr.get_role_id( get_tool()->crc32(name));
	return g_roleMgr.get_role(dwRoleID);
}
//=================================================
//	以下是逻辑处理
//=================================================
DWORD GMSession::HandleServerLogin(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_SeverLogin* xPorotocol = (NET_GMS2S_SeverLogin*)lpMsg;
	Interlocked_Exchange((LONG*)&mInit, TRUE);
	return 0;
}

DWORD GMSession::HandleForbidTalk(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_forbid_Talk* xPorotocol = (NET_GMS2S_forbid_Talk*)lpMsg;

	Role* pRole = GetRole(xPorotocol->szRoleName);
	if(VALID_POINT(pRole))pRole->SetSpeakOff(xPorotocol->b_forbid, xPorotocol->dwSecond);

	NET_S2GMS_forbid_Talk send;
	send.dw_client_id = xPorotocol->dw_client_id;
	_tcsncpy(send.szRoleName,xPorotocol->szRoleName, X_SHORT_NAME);
	send.dw_error_code = VALID_POINT(pRole) ? E_Success : EGMForbidTalk_RoleNotExist;
	mNetSession->send_msg(&send, send.dw_size);
	return 0;
}
DWORD GMSession::HandleAfficheMsg(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_Affiche* xProtocol = (NET_GMS2S_Affiche*)lpMsg;

	INT nAffLen = _tcslen(xProtocol->szAfficheMsg);
	INT nMsgSize = sizeof(NET_SIS_role_char) + nAffLen * sizeof(TCHAR);

	CREATE_MSG(pSend, nMsgSize, NET_SIS_role_char);
	NET_SIS_role_char temp; 
	pSend->dw_message_id = temp.dw_message_id;
	pSend->byAutoReply = 0; 
	pSend->byChannel = ESCC_Affiche;
	pSend->dw_error_code = E_Success;
	pSend->dwSrcRoleID = xProtocol->dwSecond;
	_tcscpy(pSend->szMsg, xProtocol->szAfficheMsg);
	pSend->szMsg[nAffLen] = 0;

	g_roleMgr.send_world_msg(pSend, pSend->dw_size);

	return 0;
}

DWORD GMSession::HandleGMKick(tag_net_message* lpMsg,	DWORD)
{
	NET_GMS2S_Kick* xProtocol = (NET_GMS2S_Kick*)lpMsg;

	Role* pRole = GetRole(xProtocol->szRoleName);
	if(VALID_POINT(pRole))
	{
		PlayerSession *pSession = pRole->GetSession();
		if(VALID_POINT(pSession))
		{
			g_worldSession.Kick(pSession->GetInternalIndex());
			pSession->SetKicked();
		}
	}

	return 0;

}

DWORD GMSession::HandleShutDown(tag_net_message* lpMsg,	DWORD)
{
	NET_GMS2S_ShutDown* xProtocol = (NET_GMS2S_ShutDown*)lpMsg;

	if(g_world.get_shutdown_time())
		return 0;

	g_world.set_shutdown_time(xProtocol->nTime);

	return 0;
}

DWORD GMSession::HandleAddItem(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_Add_Item* xProtocol = (NET_GMS2S_Add_Item*)lpMsg;

	Role* pRole = GetRole(xProtocol->szRoleName);
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_data_id	= xProtocol->dw_item_data_id;
	INT16 n16Num	= xProtocol->n16Num;
	INT32 nQlty		= xProtocol->nQlty;
	BOOL bBind = xProtocol->bBind;

	if(n16Num <= 0)
	{
		n16Num = 1;
	}

	tagItem *pNewItem = ItemCreator::Create(EICM_GM, pRole->GetID(), dw_data_id, n16Num, bBind);
	if(!VALID_POINT(pNewItem))
	{
		return INVALID_VALUE;
	}

	if(MIsEquipment(pNewItem->dw_data_id))
	{
		if(nQlty == INVALID_VALUE)
		{
			// 不鉴定	
		}
		else
		{
			ItemCreator::IdentifyEquip((tagEquip*)pNewItem, (EItemQuality)nQlty);
		}
	}


	pRole->GetItemMgr().Add2Bag(pNewItem, elcid_gm_create_item, TRUE);

	return 0;
}


DWORD GMSession::HandleSetDoublePolicy(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_SetDoublePolicy* xProtocol = (NET_GMS2S_SetDoublePolicy*)lpMsg;

	g_GMPolicy.SetRate(xProtocol->eType, xProtocol->dwValue,
			xProtocol->dwStartTime, xProtocol->dwPersistSeconds);
	return 0;
}

DWORD GMSession::HandleSetMaxPlayerNumber(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_SetMaxPlayerNumber* xProtocol = (NET_GMS2S_SetMaxPlayerNumber*)lpMsg;
	g_worldSession.SetPlayerNumLimit(xProtocol->nMaxPlayerNumber);
	return 0;
}

DWORD GMSession::HandleGMToolCreateEquip(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_GMCreateEquip* xProtocol = (NET_GMS2S_GMCreateEquip*)lpMsg;
	
	NET_S2GMS_GMCreateEquip send;
	send.dwClientID = xProtocol->dwClientID;
	send.dwRoleID = xProtocol->dwRoleID;
	//! 制造装备
	

	DWORD dw_data_id	= xProtocol->stEquipCreate.dw_data_id;
	INT32 nQlty		= xProtocol->stEquipCreate.byQuality;


	s_role_info* p_role_info = g_roleMgr.get_role_info(xProtocol->dwRoleID);
	if(!VALID_POINT(p_role_info) || !MIsEquipment(dw_data_id))
	{
		send.dwError = E_SystemError;
		send.n64Serial = -1;	
		mNetSession->send_msg(&send, send.dw_size);
		return INVALID_VALUE;
	}

	tagItem *pNewItem = ItemCreator::Create(EICM_ReGet, p_role_info->dw_role_id, dw_data_id, 1);
	if(!VALID_POINT(pNewItem))
	{
		send.dwError = E_SystemError;
		send.n64Serial = -1;	
		mNetSession->send_msg(&send, send.dw_size);
		return INVALID_VALUE;
	}

	tagEquip* pNewEquip = (tagEquip*)pNewItem;

	ItemCreator::IdentifyEquip(pNewEquip, (EItemQuality)nQlty);
	
	//gx add 装备的拥有者
	pNewEquip->dwOwnerID = p_role_info->dw_role_id;
	pNewEquip->dwCreatorID = (DWORD)INVALID_VALUE;
	pNewEquip->dw_account_id = p_role_info->dw_account_id;
	pNewEquip->eConType = EICT_Bag;
	pNewEquip->n16Index = 0;
	pNewEquip->byBind = xProtocol->stEquipCreate.byBind;
	pNewEquip->bCreateBind = TRUE;
	if (pNewEquip->byBind == EBS_Unbind)
	{
		pNewEquip->bCreateBind = FALSE;
	}
	//pNewEquip->equipSpec.byFlareVal = xProtocol->stEquipCreate.byFlareVal;
	//pNewEquip->equipSpec.nLevel = xProtocol->stEquipCreate.nLevel;
	//pNewEquip->equipSpec.nCurLevelExp = xProtocol->stEquipCreate.nCurLevelExp;
	//pNewEquip->equipSpec.byTalentPoint = xProtocol->stEquipCreate.byTalentPoint;
	//pNewEquip->equipSpec.byMaxTalentPoint = xProtocol->stEquipCreate.byMaxTalentPoint;
	pNewEquip->equipSpec.byHoleNum = xProtocol->stEquipCreate.byHoleNum;
	pNewEquip->equipSpec.byConsolidateLevel = xProtocol->stEquipCreate.byConsolidateLevel;
	//pNewEquip->equipSpec.byConsolidateLevelStar = xProtocol->stEquipCreate.byConsolidateLevelStar;
	memcpy(pNewEquip->equipSpec.dwHoleGemID, xProtocol->stEquipCreate.dwHoleGemID, sizeof(DWORD) * MAX_EQUIPHOLE_NUM);
	memcpy(pNewEquip->equipSpec.EquipAttitional, xProtocol->stEquipCreate.EquipAttitional, sizeof(INT32) * MAX_RAND_ATT);
	memcpy(pNewEquip->equipSpec.EquipAttitionalAtt, xProtocol->stEquipCreate.EquipAttitionalAtt, sizeof(tagRoleAttEffect) * MAX_ADDITIONAL_EFFECT);
	
	{
		NET_DB2C_new_equip send;
		get_fast_code()->memory_copy(&send.equip, pNewEquip, SIZE_EQUIP);
		g_dbSession.Send(&send, send.dw_size);
	}

	/*tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = xProtocol->dwRoleID;

	TCHAR szPetMailContent[X_SHORT_NAME] = _T("GMCreate");
	g_mailmgr.CreateMail(st_mail_base, _T("&gmaddItem&"), szPetMailContent, &pNewItem, 1);*/


	send.dwError = E_Success;
	send.n64Serial = pNewEquip->n64_serial;	
	//! 回复GMServer
	mNetSession->send_msg(&send, send.dw_size);

	return 0;
}

DWORD GMSession::HandleGMToolCreateItem(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_GMCreateItem* xProtocol = (NET_GMS2S_GMCreateItem*)lpMsg;

	NET_S2GMS_GMCreateItem send;
	send.dwClientID = xProtocol->dwClientID;
	send.dwRoleID = xProtocol->dwRoleID;
	send.dwTypeID = xProtocol->stItemCreate.dw_data_id;//gx add 2013.11.06
	//! 制造物品
	BYTE bybind = xProtocol->stItemCreate.byBind;
	INT16 n16Number = xProtocol->stItemCreate.n16Number;
	DWORD dw_data_id = xProtocol->stItemCreate.dw_data_id;
	tagItemProto* p_proto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	s_role_info* p_role_info = g_roleMgr.get_role_info(xProtocol->dwRoleID);

	if(!VALID_POINT(p_proto) ||  !VALID_POINT(p_role_info) ||
		MIsEquipment(dw_data_id) || p_proto->n16MaxLapNum < n16Number )
	{
		send.n64Serial = -1;
		send.dwError = E_SystemError;
		mNetSession->send_msg(&send, send.dw_size);
		return INVALID_VALUE;
	}

	tagItem* p_new = ItemCreator::Create(EICM_ReGet, p_role_info->dw_role_id, dw_data_id, n16Number, bybind);
	if(!VALID_POINT(p_new))
	{
		send.n64Serial = -1;
		send.dwError = E_SystemError;
		mNetSession->send_msg(&send, send.dw_size);
		return INVALID_VALUE;
	}
	p_new->n16Index = 0;
	//gx add 2013.7.19
	p_new->dwOwnerID = p_role_info->dw_role_id;
	p_new->dwCreatorID = (DWORD)INVALID_VALUE;
	p_new->dw_account_id = p_role_info->dw_account_id;
	p_new->eConType = EICT_Bag;

	{
		NET_DB2C_new_item send;
		get_fast_code()->memory_copy(&send.item, p_new, SIZE_ITEM);
		g_dbSession.Send(&send, send.dw_size);
	}

	/*tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = xProtocol->dwRoleID;

	TCHAR szPetMailContent[X_SHORT_NAME] = _T("GMCreate");
	g_mailmgr.CreateMail(st_mail_base, _T("&gmaddItem&"), szPetMailContent, &p_new, 1);*/


	send.dwError = E_Success;
	send.n64Serial = p_new->n64_serial;

	//! 回复GMServer
	mNetSession->send_msg(&send, send.dw_size);

	return E_Success;
}

DWORD GMSession::HandleGMToolGiveMoney(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_GMGiveMoney* xProtocol = (NET_GMS2S_GMGiveMoney*)lpMsg;

	NET_S2GMS_GMGiveMoney send;
	send.dwClientID = xProtocol->dwClientID;
	send.dwRoleID = xProtocol->dwRoleID;
	send.dwYuanBao = xProtocol->dwYuanBao;
	send.dwMoney = xProtocol->dwMoney;
	send.dwError = E_SystemError;
	
	s_role_info* p_role_info = g_roleMgr.get_role_info(xProtocol->dwRoleID);
	if(!VALID_POINT(p_role_info))
	{
		mNetSession->send_msg(&send, send.dw_size);
		return INVALID_VALUE;
	}

	tagMailBase st_mail_base;
	TCHAR szPetMailContent[X_SHORT_NAME] = _T("GMCreate");

	if(VALID_POINT(xProtocol->dwMoney))
	{
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = xProtocol->dwRoleID;
		st_mail_base.byType = 0;
		st_mail_base.dwGiveMoney = xProtocol->dwMoney;
		g_mailmgr.CreateMail(st_mail_base, _T("&gmaddMoney&"), szPetMailContent);
	}
	
	if(VALID_POINT(xProtocol->dwYuanBao))
	{
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = xProtocol->dwRoleID;
		st_mail_base.byType = 1;
		st_mail_base.dwGiveMoney = xProtocol->dwYuanBao;
		g_mailmgr.CreateMail(st_mail_base, _T("&gmaddYuaBao&"), szPetMailContent);
	}

	send.dwError = E_Success;
	mNetSession->send_msg(&send, send.dw_size);

	return E_Success;
}

DWORD GMSession::HandleGMValidate(tag_net_message* lpMsg, DWORD)
{
	NET_GMS2S_validate* xProtocol = (NET_GMS2S_validate*)lpMsg;
	AttRes::GetInstance()->GetVariableLen().n_verification = xProtocol->bOpen ? 1 : 0;

	NET_S2GMS_validate send;
	send.bOpen = xProtocol->bOpen;
	send.dwClientID = xProtocol->dwClientID;
	mNetSession->send_msg(&send, send.dw_size);
	
	return E_Success;
}