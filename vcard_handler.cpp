
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ɫ�����������

#include "stdafx.h"

#include "player_session.h"
#include "role_mgr.h"
#include "role.h"
#include "vcard.h"
#include "../../common/WorldDefine/role_card_protocol.h"
#include "../common/ServerDefine/vcard_server_define.h"

//-----------------------------------------------------------------------------
// ��ý�ɫ����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGetVCard(tag_net_message* pCmd)
{

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_role_card);
	NET_SIC_get_role_card * p_receive = ( NET_SIC_get_role_card * ) pCmd ;  
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	// �����
	if(p_receive->dw_role_id == GetRole()->GetID())
		GetRole()->GetVCard().SendVCard2Client(GetRole()->GetID());
	// �������
	else
	{
		Role* pRole = g_roleMgr.get_role(p_receive->dw_role_id);
		// �������
		if (VALID_POINT(pRole) )
		{
			tagRoleVCard* pVCard = &pRole->GetVCard();
			// �����ɼ�
			//if(pVCard->customVCard.bVisibility)
				pVCard->SendVCard2Client(GetRole()->GetID());
			// �������ɼ�
			//else
			//	GetRole()->GetVCard().NotifyClientGetVCard(p_receive->dw_role_id, E_VCard_NotVisible);
		}
		// �������
		else
			GetRole()->GetVCard().SendLoadOffLineVCard(p_receive->dw_role_id, GetRole()->GetID());
	}

	return 0;
}

//-----------------------------------------------------------------------------
// ���ý�ɫ�����Զ�����Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetVCard(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_set_role_card);
	NET_SIC_set_role_card * p_receive = ( NET_SIC_set_role_card * ) pCmd ;  
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;
	
	if( p_receive->dw_role_id != GetRole()->GetID())
	{	// �޸ı�Ȩ��
		GetRole()->GetVCard().NotifyClientSetVCard(p_receive->dw_role_id, E_VCard_NoPrivilege);
		return INVALID_VALUE;
	}

	tagRoleVCard* pVCard = &GetRole()->GetVCard();

	// ��������
	pVCard->SetCustomData(&p_receive->customVCardData);

	// �������ݿ�
	pVCard->SendSaveDB();
		
	// ֪ͨ�ͻ���
	pVCard->NotifyClientSetVCard(p_receive->dw_role_id, E_VCard_Success);

	return 0;
}

//-----------------------------------------------------------------------------
// ��ȡ������ҵ�ͷ��url
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