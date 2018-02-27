
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//声望消息处理过程

#include "stdafx.h"
#include "player_session.h"
#include "role_mgr.h"
#include "role.h"
#include "../../common/WorldDefine/reputation_protocol.h"

DWORD PlayerSession::HandleGetRoleClanData(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_reputation);
	NET_SIC_get_reputation * p_receive = ( NET_SIC_get_reputation * ) pCmd ;  	
	Role* pRole = GetOtherInMap(p_receive->dw_role_id);
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_get_reputation send;
	send.dw_role_id = p_receive->dw_role_id;
	for (INT nClanType = ECLT_BEGIN; nClanType < ECLT_END; ++nClanType)
	{
		send.ReputeData.nrValue[nClanType]		= pRole->GetClanData().RepGetVal((ECLanType)nClanType);
		send.ReputeData.ncValue[nClanType]		= pRole->GetClanData().ClanConGetVal((ECLanType)nClanType);
		send.ReputeData.nActiveCount[nClanType]	= pRole->GetClanData().ActCountGetVal((ECLanType)nClanType);
		send.ReputeData.bisFame[nClanType]		= pRole->GetClanData().IsFame((ECLanType)nClanType);
	}
	SendMessage(&send, send.dw_size);

	return 0;
}
