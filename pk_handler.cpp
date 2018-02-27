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
*	@file		Sample.h
*	@author		mmz
*	@date		2010/10/06	initial
*	@version	0.0.1.0
*/


#include "StdAfx.h"
#include "../../common/WorldDefine/pk_protocol.h"
#include "../common/ServerDefine/log_server_define.h"

#include "player_session.h"
#include "role.h"
#include "map.h"
#include "role_mgr.h"
#include "hearSay_helper.h"
//----------------------------------------------------------------------------
// 设置pk状态
//----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChangePKState(tag_net_message* pCmd)
{
	NET_SIC_change_pk_value* p_receive = (NET_SIC_change_pk_value*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = pRole->SetPKStateLimit(p_receive->eState);

	NET_SIS_change_pk_state send;
	if(dwError == E_Success)
	{
		pRole->SetPKState(p_receive->eState);
		Map* pMap = pRole->get_map();
		if(VALID_POINT(pMap))
		{
			send.dw_role_id = pRole->GetID();
			send.ePKState = p_receive->eState;
			send.dwError = E_Success;
			pRole->SendMessage(&send, send.dw_size);
		}
	}
	else
	{
		send.dw_role_id = pRole->GetID();
		send.ePKState = pRole->GetPKState();
		send.dwError = dwError;
		pRole->SendMessage(&send, send.dw_size);
	}
	return 0;
}

//--------------------------------------------------------------------
// 对某人使用追杀令，M_REGISTER_WORLD_RECV_CMD
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUseKillBadage( tag_net_message* pCmd )
{
	//GET_MESSAGE(p,pCmd,NET_SIC_use_skill_badge_item);
	NET_SIC_use_skill_badge_item * p = ( NET_SIC_use_skill_badge_item * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;

// 	if(!pRole->CanUseKillbadge())
// 	{
// 		NET_SIS_use_skill_badge_item send;
// 		send.dwError = E_UseKillbadgeItem_CDLimited;
// 		pRole->SendMessage( &send, send.dw_size );
// 		return 0;
// 	}

	Role* pTarget = g_roleMgr.get_role( p->dwTarget );
	if( !VALID_POINT(pTarget) )
	{
		NET_SIS_use_skill_badge_item send;
		send.dwError = E_UseKillbadgeItem_TargetOffLine;
		pRole->SendMessage( &send, send.dw_size );
		return 0;
	}

	Map* pMap = pTarget->get_map( );
	if( !VALID_POINT(pMap) )
	{
		NET_SIS_use_skill_badge_item send;
		send.dwError = E_UseKillbadgeItem_TargetOffLine;
		pRole->SendMessage( &send, send.dw_size );
		return 0;
	}

	if(pRole->GetCurMgr().GetBagSilver() < KILLNOTICEUSEMONEY)
		return 0;

 	const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto( KILLBUFFID );
 	if( !VALID_POINT( pBuffProto ) )return	0;

	HearSayHelper::SendMessage(EHST_USEKILLBAGAGE, pRole->GetID(), pTarget->GetID());
	pRole->GetCurMgr().DecBagSilver(KILLNOTICEUSEMONEY, elci_KillBadage);
	pTarget->TryAddBuff( pRole, pBuffProto, NULL, NULL, NULL, FALSE);

// 
//  	if( pTarget->IsInRoleState( ERS_PrisonArea ) )
//  	{
//  		NET_SIS_use_skill_badge_item send;
//  		send.dwError = E_UseKillbadgeItem_TargetInPrision;
//  		pRole->SendMessage( &send, send.dw_size );
//  		return 0;
//  	}
// 
// 	if( !pTarget->HasPKValue() /*&& !pRole->IsEnemyList(pTarget->GetID())*/)
// 	{
// 		NET_SIS_use_skill_badge_item send;
// 		send.dwError = E_UseKillbadgeItem_TargetNotRedName;
// 		pRole->SendMessage( &send, send.dw_size );
// 		return 0;
// 	}
// 
// 	tagItem* pItem = pRole->GetItemMgr( ).GetBag( ).GetItem(  p->n64ItemSerial );
// 	if( !VALID_POINT( pItem )|| !VALID_POINT(pItem->pProtoType) || pItem->pProtoType->eSpecFunc != EIST_Killbadge ) return 0;
// 
// 	const tagBuffProto* pBuffProto = AttRes::GetInstance()->GetBuffProto( pItem->GetBuffID(0) );
// 	if( !VALID_POINT( pBuffProto ) )return	0;
// 
// 	HearSayHelper::SendMessage(EHST_USEKILLBAGAGE, pRole->GetID(), pTarget->GetID(), pItem->dw_data_id);
// 
// 	NET_SIS_use_skill_badge_item send;
// 	send.dwError = pRole->GetItemMgr( ).DelFromBag( p->n64ItemSerial, elcid_item_use, 1 );
// 	pRole->SendMessage( &send, send.dw_size );
// 	pTarget->TryAddBuff( pRole, pBuffProto, NULL, NULL, NULL, FALSE);

//	pRole->ResetUseKillbadgeCD( );
	return 0;
}