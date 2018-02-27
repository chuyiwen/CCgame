
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//称号消息处理过程

#include "stdafx.h"

#include "../../common/WorldDefine/role_title_protocol.h"


#include "player_session.h"
#include "unit.h"
#include "role.h"
#include "role_mgr.h"
#include "title_mgr.h"
//-----------------------------------------------------------------------------
// 设置角色使用某称号
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleActiveTitle(tag_net_message* pCmd)
{
	NET_SIC_use_role_title* p_receive = (NET_SIC_use_role_title*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;
	
	// 只能设置本角色
	if (GetRole()->GetID() != p_receive->dw_role_id)
		return INVALID_VALUE;
	
	if (p_receive->nIndex <0 || p_receive->nIndex > 2)
		return INVALID_VALUE;


	// 激活称号
	TitleMgr* pTitleMgr = GetRole()->GetTitleMgr();
	DWORD dwRtv = pTitleMgr->setCurTitle(p_receive->u16TitleID, p_receive->nIndex);


	// 发送结果
	NET_SIS_use_role_title send;
	send.dw_role_id = GetRole()->GetID();
	send.u16TitleID = pTitleMgr->GetActiviteTitle(p_receive->nIndex);
	send.nIndex = p_receive->nIndex;
	send.dw_error_code = dwRtv;
	SendMessage(&send, send.dw_size);

	return 0;
}
//-----------------------------------------------------------------------------
// 客户端通知服务器是否显示激活的称号给其余玩家 gx add
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleShowActiveTitle(tag_net_message* pCmd)
{
	NET_SIC_show_active_title* p_receive = (NET_SIC_show_active_title*)pCmd;
	if( !VALID_POINT(m_pRole) ) 
		return INVALID_VALUE;
	// 激活称号
	TitleMgr* pTitleMgr = GetRole()->GetTitleMgr();
	for (INT i = 0;i < 3;i++)
	{
		pTitleMgr->setCurShowTitle(i,p_receive->bshow_title[i]);
	}
	return 0;
}
//-----------------------------------------------------------------------------
// 请求获得角色拥有的所有称号
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetTitles(tag_net_message* pCmd)
{
	NET_SIC_get_role_titles* p_receive = (NET_SIC_get_role_titles*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	Role* pRole = p_receive->dw_role_id == GetRole()->GetID() ? GetRole() : GetOtherInMap(p_receive->dw_role_id);
	if ( !VALID_POINT(pRole) )
		return 0;

	TitleMgr* pTitleMgr = pRole->GetTitleMgr();

// 	// 查看权限
// 	if ( pRole->GetID() != m_pRole->GetID() && !pTitleMgr.Visibility() )
// 	{	// 不可发送
// 		NET_SIS_get_role_titles send;
// 		send.dw_error_code = E_Title_NotVisible;
// 		send.dw_role_id = p_receive->dw_role_id;
// 		send.u16TitleNum = 0;
// 		SendMessage(&send, send.dw_size);
// 	}
// 	else
// 	{	// 可发送
		DWORD dwTitlesSize = pTitleMgr->GetObtainedTitlesNum() * sizeof(tagTitleData);
		DWORD dwMsgSize = sizeof(NET_SIS_get_role_titles) - 1 + (dwTitlesSize > 0 ? dwTitlesSize : 1);

		ASSERT(dwMsgSize >= sizeof(NET_SIS_get_role_titles));

		CREATE_MSG(pSend, dwMsgSize, NET_SIS_get_role_titles);
		//NET_SIS_get_role_titles* pSend;
		tagTitleData* pu16 = pSend->byData;
		DWORD dwRtv = pTitleMgr->GetObtainedTitleIDs(pu16, pSend->u16TitleNum);
		pSend->dw_error_code = dwRtv;
		pSend->dw_role_id = p_receive->dw_role_id;
		SendMessage(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
//	}

	return 0;
}


DWORD PlayerSession::HandleRoleTitleBuy(tag_net_message* pCmd)
{
	NET_SIC_role_title_buy* p_receive = (NET_SIC_role_title_buy*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;


	// 激活称
	TitleMgr* pTitleMgr = GetRole()->GetTitleMgr();
	DWORD dwRtv = pTitleMgr->BuyTitle(p_receive->dw_title_id);


	// 发送结果
	NET_SIS_role_title_buy send;
	send.dwErrorCode = dwRtv;
	SendMessage(&send, send.dw_size);
	return 0;
}

DWORD PlayerSession::HandleRoleTitleReturn(tag_net_message* pCmd)
{
	NET_SIC_role_title_return* p_receive = (NET_SIC_role_title_return*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;


	// 归还称号
	TitleMgr* pTitleMgr = GetRole()->GetTitleMgr();
	
	// 发送结果
	NET_SIS_role_title_return send;
	send.dwErrorCode = pTitleMgr->ReturnTitle(p_receive->dw_title_id);
	send.dw_title_id = p_receive->dw_title_id;
	SendMessage(&send, send.dw_size);
	return 0;
}