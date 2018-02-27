/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/RoleCarryDefine.h"

VOID Role::OnInvestCarryNPC(DWORD dwCarryID)
{
	if(!IsInRoleState(ERS_Carry) && dwCarryID)
	{
		SetCarrryID(dwCarryID);
		SetRoleState(ERS_Carry);
	}
}

VOID Role::OnSetRoleState(const RoleState& oldState, ERoleState eState)
{
	if(oldState.IsInState(eState))
		return;

	if(eState == ERS_Carry)
	{
		NET_SIS_CarrySomething send;
		send.dwRoleID = GetID();
		send.dwTypeID = GetCarryID( );
		Map *pMap = get_map( );
		if(VALID_POINT(pMap))
			pMap->send_big_visible_tile_message(this, &send, send.dw_size);
	}

	if(eState == ERS_Mount) DealMountSkill(TRUE);
}


VOID Role::OnUnsetRoleState(const RoleState& oldState, ERoleState eState)
{
	if(!oldState.IsInState(eState))
		return;

	if(eState == ERS_Carry && !IsInRoleState(ERS_Carry))
		mCarryTypeID = 0;

	if(eState == ERS_Mount && !IsInRoleState(ERS_Mount))
		DealMountSkill(FALSE);

}

VOID Role::SetCarrryID(DWORD dwID)
{
		mCarryTypeID = dwID;
}

DWORD Role::GetCarryID( ) const
{
	return mCarryTypeID;
}