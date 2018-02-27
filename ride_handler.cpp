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
 *	@file		ride_handler
 *	@author		mawenhong
 *	@date		2010/11/22	initial
 *	@version	0.0.1.0
 *	@brief		坐骑
*/

#include "StdAfx.h"
#include "../../common/WorldDefine/ride_protocol.h"
#include "player_session.h"
#include "role.h"

//--------------------------------------------------------------------
// 强化骑乘
//--------------------------------------------------------------------
DWORD PlayerSession::HandleUpgRide( tag_net_message* pCmd )
{
	NET_SIC_upgrade_ride* p = (NET_SIC_upgrade_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_upgrade_ride sisUpgRide;
	sisUpgRide.dwNpcID = p->dwNpcID;
	sisUpgRide.dwError = pRole->upgrade_ride(p->dwNpcID, 
											p->n64RideSerial, 
											p->byGodStoneNumber, 
											p->bUseBind);
	SendMessage(&sisUpgRide, sisUpgRide.dw_size );
	return 0;
}
//--------------------------------------------------------------------
// 上装备
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRideInlay( tag_net_message* pCmd )
{
	NET_SIC_ride_inlay* p = (NET_SIC_ride_inlay*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_ride_inlay sisRideInlay;
	sisRideInlay.dwError = pRole->ride_inlay(p->n64RideSerial, 
											p->n64ItemSerial, MAX_RIDEHOLE_NUM);	
	SendMessage(&sisRideInlay, sisRideInlay.dw_size );

	return 0;
}
//--------------------------------------------------------------------
// 移除装备
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRemoveRideInlay( tag_net_message* pCmd )
{
// 	NET_SIC_remove_ride_inlay* p = (NET_SIC_remove_ride_inlay*)pCmd;
// 
// 	Role* pRole = GetRole( );
// 	if( FALSE == VALID_POINT(pRole))return INVALID_VALUE;
// 
// 	NET_SIS_remove_ride_inlay sisRemoveInlay;
// 	sisRemoveInlay.dwError = pRole->remove_ride_inlay(p->dwNpcID,
// 													p->n64RideSerial,
// 													p->n64CrushStone,
// 													p->byIndex );
// 	SendMessage(&sisRemoveInlay, sisRemoveInlay.dw_size );
	return 0;
}
//--------------------------------------------------------------------
// 开始骑乘
//--------------------------------------------------------------------
DWORD PlayerSession::HandleBeginRide( tag_net_message* pCmd )
{
	NET_SIC_begin_ride* p = (NET_SIC_begin_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_begin_ride send;
	send.dwError = pRole->GetRaidMgr().BeginRaid();
	send.dwRoleID = pRole->GetID();
	send.dwTypeID = pRole->GetRaidMgr().getRaidMode();
	send.nLevel = pRole->GetRaidMgr().getLevel();

	if (send.dwError == E_Success)
	{
		if(VALID_POINT(pRole->get_map()))
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
	}
	else
	{
		pRole->SendMessage(&send, send.dw_size);
	}

	return 0;
}
//--------------------------------------------------------------------
// 取消骑乘
//--------------------------------------------------------------------
DWORD PlayerSession::HandleCancelRide( tag_net_message* pCmd )
{
	NET_SIC_cancel_ride* p = (NET_SIC_cancel_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_cancel_ride send;
	send.dwRoleID = pRole->GetID();
	send.dwError		= pRole->GetRaidMgr().CancelRaid();
	if (send.dwError == E_Success)
	{
		if(VALID_POINT(pRole->get_map()))
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);

	}
	else
	{
		SendMessage(&send, send.dw_size );
	}
	
	return 0;
}
//---------------------------------------------------------------------
// 打断坐骑
//---------------------------------------------------------------------
DWORD PlayerSession::HandleInterruptRide(tag_net_message* pCmd)
{
	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	pRole->InterruptRide();

	return 0;
}
//---------------------------------------------------------------------
// 装备坐骑
//---------------------------------------------------------------------
DWORD PlayerSession::HandleEquipRide(tag_net_message* pCmd)
{
	NET_SIC_Equip_ride* p = (NET_SIC_Equip_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_Equip_ride send;
	send.dwError = pRole->EquipRide(p->n64RideSerial);
	send.n64RideSerial = p->n64RideSerial;
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}

//---------------------------------------------------------------------
// 卸下坐骑
//---------------------------------------------------------------------
DWORD PlayerSession::HandleUnEquipRide(tag_net_message* pCmd)
{
	NET_SIC_UnEquip_ride* p = (NET_SIC_UnEquip_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;
	
	NET_SIS_UnEquip_ride send;
	send.dwError = pRole->UnEquipRide();
	send.n64RideSerial = p->n64RideSerial;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

// 培养坐骑
DWORD PlayerSession::HandletogRide(tag_net_message* pCmd)
{
	NET_SIC_tog_ride* p = (NET_SIC_tog_ride*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	NET_SIS_tog_ride send;
	send.nCritNum = 0;
	send.nExp = 0;
	send.dw_error_code = pRole->GetRaidMgr().Tog(p->byType, send.nCritNum, send.nExp);
	pRole->SendMessage(&send, send.dw_size);


	return 0;
}

DWORD PlayerSession::HandleGetRideAtt(tag_net_message* pCmd)
{
	NET_SIC_get_raid_att* p = (NET_SIC_get_raid_att*)pCmd;

	Role* pRole = GetRole( );
	if(!VALID_POINT(pRole))return INVALID_VALUE;

	pRole->GetRaidMgr().sendRaidData();

	return 0;
}