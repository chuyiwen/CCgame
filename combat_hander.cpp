
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//玩家间交易消息处理
#include "StdAfx.h"

#include "../../common/WorldDefine/combat_protocol.h"
#include "../../common/WorldDefine/map_protocol.h"

#include "role_mgr.h"
#include "player_session.h"
#include "map.h"
#include "unit.h"
#include "role.h"

//--------------------------------------------------------------------------------
// 进入战斗状态
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEnterCombat(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	 //设置人物现在处于战斗状态
	//pRole->SetRoleState(ERS_EquipTrans);gx modify 2013.6.27
	return 0;
}

//--------------------------------------------------------------------------------
// 离开战斗状态
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLeaveCombat(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	// 设置人物现在处于战斗状态
	//pRole->UnsetRoleState(ERS_EquipTrans);gx modify 2013.6.27

	// 如果是灰名，并且是灰名后第一次离开战斗，则设置灰名离开战斗状态时间
	/*if(pRole->IsGrayName() && pRole->GetGNFirstLeavePK())
	{
		pRole->SetLeavePKStateTime();
		pRole->SetGNFirstLeavePK(FALSE);
	}*/
	return 0;
}

//--------------------------------------------------------------------------------
// 使用技能
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSkill(tag_net_message* pCmd)
{
	NET_SIC_skill* p_receive = (NET_SIC_skill*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) ) return INVALID_VALUE;

	// 看能否同步到发招地点(外挂警告)
 	//if(p_receive->dwMapID == pMap->get_map_id())
	//float nDis = Vec3Dist(pRole->GetCurPos(), p_receive->vSrcPos);
	//print_message(_T("dis:%f"), nDis);
 	//	pRole->GetMoveData().ForceTeleport(p_receive->vSrcPos,TRUE);

	// 使用技能
	INT nRet = pRole->GetCombatHandler().UseSkill(Skill::GetIDFromTypeID(p_receive->dwSkillID), p_receive->dwTargetRoleID, p_receive->dwSerial);

	if( E_Success != nRet )
	{
		NET_SIS_skill send;
		send.dwSrcRoleID = pRole->GetID();
		send.dwTarRoleID = p_receive->dwTargetRoleID;
		send.dwSkillID = p_receive->dwSkillID;
		send.dwSerial = p_receive->dwSerial;
		send.dw_error_code = DWORD(nRet);

		SendMessage(&send, send.dw_size);
	}

	else
	{
		// 看能否同步到发招地点
		if(p_receive->dwMapID == pMap->get_map_id())
			pRole->GetMoveData().DropDownStandPoint();

		pRole->BreakMount( );

		NET_SIS_skill send;
		send.dwSrcRoleID = pRole->GetID();
		send.dwTarRoleID = p_receive->dwTargetRoleID;
		send.dwSkillID = p_receive->dwSkillID;
		send.dwSerial = p_receive->dwSerial;
		send.nSpellTime = pRole->GetCombatHandler().GetSkillPrepareCountDown();
		send.dw_error_code = E_Success;

		if( send.nSpellTime > TICK_TIME )
		{
			send.nSpellTime -= TICK_TIME;
		}

		if( VALID_POINT(pRole->get_map()) )
		{
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
		}
	}

	return 0;
}

//--------------------------------------------------------------------------------
// 打断技能
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleInterruptSkill(tag_net_message* pCmd)
{
	NET_SIC_skill_interrupt* p_receive = (NET_SIC_skill_interrupt*)pCmd;

	Role *pRole = GetRole();
	if( !VALID_POINT(pRole)) return INVALID_VALUE;

	pRole->GetCombatHandler().CancelSkillUse(p_receive->dwSkillID);

	return 0;	
}

//--------------------------------------------------------------------------------
// 使用物品
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUseItem(tag_net_message *pCmd)
{
	NET_SIC_use_item* p_receive = (NET_SIC_use_item*)pCmd;

	Role *pRole = GetRole();
	if( !VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if( !VALID_POINT(pMap) ) return INVALID_VALUE;

	DWORD dw_data_id = INVALID_VALUE;
	bool bImmediate = false;

	// 使用物品
	INT nRet = pRole->GetCombatHandler().UseItem(p_receive->n64ItemID, p_receive->dwTargetRoleID, p_receive->dwSerial, dw_data_id, bImmediate);

	if( E_Success != nRet)
	{
		NET_SIS_use_item send;
		send.dwSrcRoleID = pRole->GetID();
		send.dwTarRoleID = p_receive->dwTargetRoleID;
		send.n64ItemID = p_receive->n64ItemID;
		send.dw_data_id = dw_data_id;
		send.dwSerial = p_receive->dwSerial;
		send.dw_error_code = DWORD(nRet);
		send.bInmmediate = bImmediate;

		SendMessage(&send, send.dw_size);
	}
	else
	{
		if( !bImmediate )
		{
			// 如果不是立即可以使用的物品，则需要停下来
			pRole->GetMoveData().DropDownStandPoint();
		}

		NET_SIS_use_item send;
		send.dwSrcRoleID = pRole->GetID();
		send.dwTarRoleID = p_receive->dwTargetRoleID;
		send.n64ItemID = p_receive->n64ItemID;
		send.dw_data_id = dw_data_id;
		send.dwSerial = p_receive->dwSerial;
		send.nSpellTime = pRole->GetCombatHandler().GetItemPrepareCountDown();
		send.dw_error_code = E_Success;
		send.bInmmediate = bImmediate;

		if( send.nSpellTime > TICK_TIME )
		{
			send.nSpellTime -= TICK_TIME;
		}

		if( VALID_POINT(pRole->get_map()) )
		{
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
		}
	}

	return 0;
}

//--------------------------------------------------------------------------------
// 打断使用物品
//--------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleInterruptUseItem(tag_net_message* pCmd)
{
	NET_SIC_use_item_interrupt* p_receive = (NET_SIC_use_item_interrupt*)pCmd;

	Role *pRole = GetRole();
	if( !VALID_POINT(pRole)) return INVALID_VALUE;

	pRole->GetCombatHandler().CancelItemUse(p_receive->n64ItemID);

	return 0;	
}

//------------------------------------------------------------------------
// 绑定复活地图
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleBindRebornMap(tag_net_message* pCmd)
{
	NET_SIC_bind_reborn_map* p = (NET_SIC_bind_reborn_map*)pCmd;
	

	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_bind_reborn_map send;
	send.dw_error_code = pRole->SetRebornMap(p->dwNPCID, send.dwBornMapID, send.vBornPos);

	SendMessage(&send, send.dw_size);

	return send.dw_error_code;
}

//------------------------------------------------------------------------
// 复活
//------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleRevive(tag_net_message* pCmd)
{
	NET_SIC_role_revive* p = (NET_SIC_role_revive*)pCmd;

	// 检查消息合法性
	if(p->eType < ERRT_Start || p->eType > ERRT_End)
	{
		return INVALID_VALUE;
	}
	
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//if (p->eType != ERRT_Normal && p->eType != ERRT_Prison && p->eType != ERRT_CoolDown)
	//{
	//	if(!pRole->get_check_safe_code())
	//	{
	//		if(GetBagPsd() != p->dw_safe_code)
	//		{

	//			NET_SIS_code_check_ok send_check;
	//			send_check.bSuccess = FALSE;
	//			pRole->SendMessage(&send_check, send_check.dw_size);

	//			return INVALID_VALUE;
	//		}

	//		else 
	//		{
	//			NET_SIS_code_check_ok send_check;
	//			send_check.bSuccess = TRUE;
	//			pRole->SendMessage(&send_check, send_check.dw_size);

	//			pRole->set_check_safe_code();
	//		}
	//	}
	//} 
	
	
	//if( p->eType == ERRT_CoolDown && VALID_POINT(pRole->get_map()) &&
	//	VALID_POINT(pRole->get_map()->get_map_info()) ){
	//	if( pRole->get_map()->get_map_info()->b_CoolDownRevive) {
	//		pRole->StartCoolDownRevive( );
	//		return E_Success;
	//	}  else {
	//		p->eType = ERRT_ReturnCity;
	//	}
	//}

	{
		NET_SIS_role_revive send;
		send.dw_role_id		= pRole->GetID();
		send.dw_error_code	= pRole->Revive(p->eType, p->n64ItemID);

		if( VALID_POINT(pRole->get_map()) )
		{
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
		}

		if(E_Success == send.dw_error_code)
		{
			// 复活到指定地点
			pRole->GotoNewMap(pRole->m_Revive.dwMapID, 
				pRole->m_Revive.fX, pRole->m_Revive.fY, pRole->m_Revive.fZ);
		}

		return send.dw_error_code;
	}

}

//------------------------------------------------------------------------------
// 玩家点掉Buff
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCancelBuff(tag_net_message* pCmd)
{
	NET_SIC_cancel_buffer* p_receive = (NET_SIC_cancel_buffer*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	// 取消Buff
	pRole->CancelBuff(p_receive->dwBuffID);

	return 0;
}


DWORD	PlayerSession::HandleTargetChange(tag_net_message* pCmd)
{
	NET_SIC_target_change* p_receive = (NET_SIC_target_change*)pCmd;

	Role* pRole = GetRole();
	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	pRole->SetTargetID(p_receive->dw_target_id);

	return 0;

}
