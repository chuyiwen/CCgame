
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//角色名贴命令处理器

#include "stdafx.h"

#include "player_session.h"
#include "role_mgr.h"
#include "role.h"
#include "vcard.h"
#include "../../common/WorldDefine/role_card_protocol.h"
#include "../common/ServerDefine/vcard_server_define.h"

//-----------------------------------------------------------------------------
// 获得角色名贴
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetVCard(tag_net_message* pCmd)
{

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_role_card);
	NET_SIC_get_role_card * p_receive = ( NET_SIC_get_role_card * ) pCmd ;  
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	// 本玩家
	if(p_receive->dw_role_id == GetRole()->GetID())
		GetRole()->GetVCard().SendVCard2Client(GetRole()->GetID());
	// 其他玩家
	else
	{
		Role* pRole = g_roleMgr.get_role(p_receive->dw_role_id);
		// 在线玩家
		if (VALID_POINT(pRole) )
		{
			tagRoleVCard* pVCard = &pRole->GetVCard();
			// 名帖可见
			//if(pVCard->customVCard.bVisibility)
				pVCard->SendVCard2Client(GetRole()->GetID());
			// 名帖不可见
			//else
			//	GetRole()->GetVCard().NotifyClientGetVCard(p_receive->dw_role_id, E_VCard_NotVisible);
		}
		// 离线玩家
		else
			GetRole()->GetVCard().SendLoadOffLineVCard(p_receive->dw_role_id, GetRole()->GetID());
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 设置角色名贴自定义信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetVCard(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_set_role_card);
	NET_SIC_set_role_card * p_receive = ( NET_SIC_set_role_card * ) pCmd ;  
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;
	
	if( p_receive->dw_role_id != GetRole()->GetID())
	{	// 无改变权限
		GetRole()->GetVCard().NotifyClientSetVCard(p_receive->dw_role_id, E_VCard_NoPrivilege);
		return INVALID_VALUE;
	}

	tagRoleVCard* pVCard = &GetRole()->GetVCard();

	// 更新数据
	pVCard->SetCustomData(&p_receive->customVCardData);

	// 更新数据库
	pVCard->SendSaveDB();
		
	// 通知客户端
	pVCard->NotifyClientSetVCard(p_receive->dw_role_id, E_VCard_Success);

	return 0;
}

//-----------------------------------------------------------------------------
// 获取在线玩家的头像url
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetHeadPicUrl(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_role_head_picture);
	NET_SIC_get_role_head_picture * p_receive = ( NET_SIC_get_role_head_picture * ) pCmd ;  
	if( !VALID_POINT(GetRole()) ) return INVALID_VALUE;

	Role*pRole = g_roleMgr.get_role(p_receive->dw_role_id);
	if (!VALID_POINT(pRole))
		GetRole()->GetVCard().SendNullUrlToMe(p_receive->dw_role_id);
	//else
	//	pRole->GetVCard().SendHeadUrlTo(GetRole()->GetID());
	
	return 0;
}