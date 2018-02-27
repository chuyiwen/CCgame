/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"
#include "role.h"
#include "player_session.h"
#include "../../common/WorldDefine/master_prentice_protocol.h"
#include "master_prentice_mgr.h"

DWORD PlayerSession::HandleMakeMaster( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.make_master( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleMakeMasterEx( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.make_master_ex( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleMakePrentice( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.make_prentice( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleMakePrenticeEx( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.make_prentice_ex( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleMasterPrenticeBreak( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.master_prentice_break( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleGetMasterPlacard( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.get_master_placard( pRole->GetID(), pCmd);
	return 0;
}
DWORD PlayerSession::HandleShowInMasterPlacard( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.show_in_master_placard( pRole->GetID(),  pCmd);
	return 0;
}

DWORD PlayerSession::HandleCallInMaster( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;
	
	g_MasterPrenticeMgr.call_in_master( pRole->GetID( ), pCmd );

	return 0;
}
DWORD PlayerSession::HandlePrenticeCallIn( tag_net_message* pCmd )
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.prentice_call_in( pRole->GetID( ), pCmd );

	return 0;
}
DWORD PlayerSession::HandleMasterTeachPrentice(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;
	g_MasterPrenticeMgr.master_teach_prentice(pRole->GetID(),pCmd);
	return 0;
}
DWORD PlayerSession::HandleJoinMasterRecruit(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.join_master_recruit(pRole);

	return 0;
}

DWORD PlayerSession::HandleLeaveMasterRecruit(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	g_MasterPrenticeMgr.leave_master_recruit(pRole);

	return 0;
}

DWORD PlayerSession::HandleQueryPageMasterRecruit(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;
	
	NET_SIC_query_page_master_recruit* p = (NET_SIC_query_page_master_recruit*)pCmd;
	g_MasterPrenticeMgr.query_page_master_recruit(pRole, p->n_page);

	return 0;
}

DWORD PlayerSession::HandleSayGoodByteToMaster(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	NET_SIC_say_goodbye_to_master* p = (NET_SIC_say_goodbye_to_master*)pCmd;
	//首先进行出师条件预判断
	//师徒二人必须先组队
	DWORD dwTeamID = pRole->GetTeamID();
	if (INVALID_VALUE == dwTeamID)
	{
		return INVALID_VALUE;
	}
	const Team* pTeam = g_groupMgr.GetTeamPtr(dwTeamID);
	if(!VALID_POINT(pTeam))
		return INVALID_VALUE;
	BOOL bFind = FALSE;
	for (int i = 0; i < MAX_TEAM_NUM;i++)
	{
		if (pRole->get_master_id() == pTeam->get_member_id(i))
		{
			bFind = TRUE;
			break;
		}
	}
	if (!bFind)
		return INVALID_VALUE;
	g_MasterPrenticeMgr.say_goodbye_to_master(pRole, p->byAck);
}