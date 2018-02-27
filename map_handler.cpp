
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端地图消息处理类

#include "StdAfx.h"

#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/battle_ground_protocol.h"

#include "player_session.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "item_mgr.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "map_instance_pvp.h"
#include "pvp_mgr.h"
#include "map_instance_1v1.h"


//------------------------------------------------------------------------------
// 玩家踩到地图触发器
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMapTrigger(tag_net_message* pCmd)
{
	NET_SIC_role_map_trigger* p_receive = (NET_SIC_role_map_trigger*)pCmd;

	Role* pRole = GetRole();

	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	// 玩家踩到触发器
	pRole->MapTrigger(p_receive->dwTriggerID, p_receive->dwMisc);

	return 0;
}

//------------------------------------------------------------------------------
// 通知队友进入
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleInstanceNotify(tag_net_message* pCmd)
{
	NET_SIC_instance_notify* p_receive = (NET_SIC_instance_notify*)pCmd;

	Role* pRole = GetRole();

	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	// 通知队友
	DWORD dw_error_code = pRole->InstanceNotify(p_receive->bNotify);

	if(dw_error_code == INVALID_VALUE)
	{
		return INVALID_VALUE;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
// 玩家是否同意其它玩家进入副本的邀请
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleInstanceAgree(tag_net_message* pCmd)
{
	NET_SIC_instance_agree* p_receive = (NET_SIC_instance_agree*)pCmd;

	Role* pRole = GetRole();

	if( !VALID_POINT(pRole) ) return INVALID_VALUE;

	pRole->InstanceAgree(p_receive->bAgree);

	return 0;
}

//------------------------------------------------------------------------------
// 玩家请求离开副本
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLeaveInstance(tag_net_message* pCmd)
{
	NET_SIC_leave_instance* p_receive = (NET_SIC_leave_instance*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))	return INVALID_VALUE;

	pRole->LeaveInstance();

	return 0;
}

//------------------------------------------------------------------------------
// 进入乱战副本
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleEnterPVPInstance(tag_net_message* pCmd)
{
	NET_SIC_enter_pvp_instance* p_recive = (NET_SIC_enter_pvp_instance*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))	return INVALID_VALUE;

	DWORD dw_error = pRole->EnterPVP();

	NET_SIS_enter_pvp_instance send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}


// 进入战场
DWORD PlayerSession::HandleEnterBattle(tag_net_message* pCmd)
{
	NET_SIC_enter_battle_instance* pRecv = (NET_SIC_enter_battle_instance*)pCmd;

	Role* pRole = GetRole();

	if (!VALID_POINT(pRole)) return INVALID_VALUE;

	DWORD dw_error = pRole->EnterBattle();

	NET_SIS_enter_battle_instance send;
	send.dwErrorCode = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
//------------------------------------------------------------------------------
// 退出乱战副本
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleLeavePVPInstance(tag_net_message* pCmd)
{
	NET_SIC_leave_pvp_instance* p_recive = (NET_SIC_leave_pvp_instance*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))	return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pMap->get_map_info()->e_type != EMT_PVP)
		return INVALID_VALUE;

	map_instance_pvp* p_instance_pvp = (map_instance_pvp*)pMap;
	if(!VALID_POINT(p_instance_pvp))
		return INVALID_VALUE;

	if(pRole->IsInRoleState(ERS_Combat))
		return INVALID_VALUE;

	p_instance_pvp->on_leave_instance(pRole->GetID());

	return 0;
}

//------------------------------------------------------------------------------
// 离开1v1副本
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleLeave1v1(tag_net_message* pCmd)
{
	Role* pRole = GetRole();

	if(!VALID_POINT(pRole)) return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pMap->get_map_type() != EMT_1v1)
		return INVALID_VALUE;

	map_instance_1v1* p1v1 = (map_instance_1v1*)pMap;
	if(!VALID_POINT(p1v1))
		return INVALID_VALUE;

	p1v1->on_leave_instance(pRole->GetID());

	return 0;
}

//------------------------------------------------------------------------------
// 开始挂机
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleStartHang(tag_net_message* pCmd)
{
	NET_SIC_start_hang* p_receive = (NET_SIC_start_hang*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole)) return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}
	DWORD dwError = pRole->StartHang();

	NET_SIS_start_hang send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 取消挂机
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleCancelHang(tag_net_message* pCmd)
{
	NET_SIC_cancel_hang* p_receive = (NET_SIC_cancel_hang*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwError = pRole->CancelHang();

	NET_SIS_cancel_hang send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 设置离线挂机
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleSetLeaveLineHang(tag_net_message* pCmd)
{
	NET_SIC_set_leave_line_hang* p_receive = (NET_SIC_set_leave_line_hang*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	pRole->SetUseLeaveExp(TRUE/*p_receive->bExp*/);
	pRole->SetUseLeaveBrother(TRUE/*p_receive->bBrotherhood*/);

	return 0;
}

//------------------------------------------------------------------------------
// 获取离线经验信息
// gx modify 2013.12.17
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetLeaveExp(tag_net_message* pCmd)
{
	NET_SIC_get_leave_exp* p_receive = (NET_SIC_get_leave_exp*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	//计算离线时间，单位是秒
	INT nHour = pRole->CalLeaveTimeByExp(pRole->GetLeaveExp());
	if(nHour <= 0)
		return INVALID_VALUE;
	NET_SIS_leave_exp_clueon send;
	send.nExp = pRole->GetLeaveExp();
	send.dw_logout_time = nHour;
	pRole->SendMessage(&send,send.dw_size);
	/*NET_SIS_get_leave_exp send;
	send.nExp = pRole->GetLeaveExp();
	//send.nBrotherHand = pRole->GetLeaveBrother();
	//send.n16HangNum = pRole->GetHangNum();
	pRole->ExpChange(pRole->GetLeaveExp());
	pRole->CleanLeaveExp();
	pRole->SendMessage(&send, send.dw_size);*/

	return 0;
}

//------------------------------------------------------------------------------
// 提取离线经验
//------------------------------------------------------------------------------
DWORD PlayerSession::HandlePickLeaveExp(tag_net_message* pCmd)
{
	NET_SIC_pickup_leave_exp* p_receive = (NET_SIC_pickup_leave_exp*)pCmd;

	Role* pRole = GetRole();

	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	/*if (VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		return INVALID_VALUE;
	}
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	if (VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		return INVALID_VALUE;
	}*/

	if(p_receive->n16Type < 1 || p_receive->n16Type > 3)
		return INVALID_VALUE;

	if(pRole->GetLeaveExp() <= 0)
		return INVALID_VALUE;

	INT leaveHours = pRole->CalLeaveTimeByExp(pRole->GetLeaveExp());
	if(leaveHours <= 0)
		return INVALID_VALUE;
	//防止异常
	if (leaveHours != p_receive->nLeaveTime)//离线时间客户端也要做相应的判断
		return INVALID_VALUE;
	INT32 n32_yuanbao_num = 0;//翻倍领取经验所花元宝数，6元宝，9元宝
	INT	  n_mul = 0;//领取离线经验的倍数，1,2,3
	switch(p_receive->n16Type)
	{
	/*case 0:
		{
			n32_yuanbao_num = 0;
			n_mul = 1;
			break;
		}*/
	case 1:
		{
			n32_yuanbao_num = 0;
			n_mul = 1;
			break;
		}
	case 2:
		{
			n32_yuanbao_num = leaveHours * 6;
			n_mul = 2;
			break;
		}
	case 3:
		{
			n32_yuanbao_num = leaveHours * 9;
			n_mul = 3;
			break;
		}
	}

	if(n32_yuanbao_num > 0)
	{
		if(pRole->GetCurMgr().GetBaiBaoYuanBao() < n32_yuanbao_num)
		{
			NET_SIS_pickup_leave_exp send;
			send.dwError = E_Hang_Yuanbao_No_Enough;
			pRole->SendMessage(&send, send.dw_size);
			return 0;
		}
	}

	INT n_exp = 0;
	if(n32_yuanbao_num > 0)
	{
		pRole->GetCurMgr().DecBaiBaoYuanBao(n32_yuanbao_num, elcid_hang);
	}
	pRole->ExpChange(pRole->GetLeaveExp()*n_mul);

	INT nTmpExp = pRole->GetLeaveExp()*n_mul;//临时存储，用于给客户端发送提示
	pRole->CleanLeaveExp();
	
	// 提取离线经验经验
	/*if(p_receive->n16Type == 1)
	{
		if(pRole->GetLeaveExp() <= 0)
			return INVALID_VALUE;

		pRole->ExpChange(pRole->GetLeaveExp());
		pRole->CleanLeaveExp();
		NET_DB2C_clean_role_leave_exp send;
		send.dw_role_id = pRole->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}*/

	// 提取义气值
	/*if(p_receive->n16Type == 2)
	{
		if(pRole->GetLeaveBrother() <= 0)
			return INVALID_VALUE;

		pRole->ChangeBrotherhood(pRole->GetLeaveBrother());
		pRole->ReAccAtt();
		pRole->CleanLeaveBrother();
		NET_DB2C_clean_role_leave_brother send;
		send.dw_role_id = pRole->GetID();
		g_dbSession.Send(&send, send.dw_size);
	}*/

	NET_SIS_pickup_leave_exp send;
	send.nExp = nTmpExp;
	send.dwError = E_Success;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 角色加载完成
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleLoadComplete(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	// 计算玩家所在的可视地砖格子
	INT nVisIndex = pMap->world_position_to_visible_index(pRole->GetCurPos());

	// 得到九宫格
	tagVisTile* pVisTile[EUD_end] = {0};
	pMap->get_visible_tile(nVisIndex, pVisTile);

	// 同步给加入到客户端的玩家和生物
	pMap->synchronize_add_units(pRole, pVisTile);

	if(pMap->GetWeather() != INVALID_VALUE){
		NET_SIS_change_Weather send;
		send.dwMapID = pMap->get_map_id();
		send.dwInstanceID = pMap->get_instance_id();
		send.dwWeather = pMap->GetWeather();
		SendMessage(&send, send.dw_size);
	}

	return 0;
}

//------------------------------------------------------------------------------
// 重置副本
//------------------------------------------------------------------------------
DWORD PlayerSession::handle_reset_instance(tag_net_message* pCmd)
{
	NET_SIC_reset_instance* p_recv = (NET_SIC_reset_instance*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = pRole->reset_instance(p_recv->dw_map_id, p_recv->n_mode);

	NET_SIS_reset_instance send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);
	
	return 0;
}

//------------------------------------------------------------------------------
// 重置副本权值
//------------------------------------------------------------------------------
DWORD PlayerSession::handle_reset_inst_limit(tag_net_message* pCmd)
{
	NET_SIC_reset_instance_limit* p_recv = (NET_SIC_reset_instance_limit*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dw_error = pRole->reset_instance_limit(p_recv->dw_map_id);

	NET_SIS_reset_instance_limit send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 进入1v1竞技场申请
//------------------------------------------------------------------------------
DWORD PlayerSession::Handle1v1Apply(tag_net_message* pCmd)
{
	NET_SIC_1v1_apply* p_recv = (NET_SIC_1v1_apply*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	
	dw_error = g_pvp_mgr.apply_1v1(pRole);

	NET_SIS_1v1_apply send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::Handle1v1LeaveQueue(tag_net_message* pCmd)
{
	NET_SIC_1v1_leave_queue* p_recv = (NET_SIC_1v1_leave_queue*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;

	dw_error = g_pvp_mgr.role_leave_queue(pRole);

	NET_SIS_1v1_leave_queue send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);


	return 0;
}

DWORD PlayerSession::HandleGet1v1ScoreAward(tag_net_message* pCmd)
{
	NET_SIC_get_1v1_score_award* p_recv = (NET_SIC_get_1v1_score_award*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;

	dw_error = g_pvp_mgr.get_1v1_award(pRole);

	NET_SIS_get_1v1_score_award send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 约战申请
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleReservationApply(tag_net_message* pCmd)
{
	NET_SIC_reservation_apply* p_recv = (NET_SIC_reservation_apply*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD dw_error = pRole->reservation_apply(p_recv->dw_role_id);

	NET_SIS_reservation_apply send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 获取约战人物信息
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetReservationInfo(tag_net_message* pCmd)
{
	NET_SIC_get_reservation_info* p_recv = (NET_SIC_get_reservation_info*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;

	Role* pReservation = g_roleMgr.get_role(p_recv->dw_role_id);
	if(!VALID_POINT(pReservation))
		dw_error = E_Instance_beservation_no_line;

	NET_SIS_get_reservation_info send;
	if(dw_error == E_Success)
	{
		send.dw_error = dw_error;
		send.eClass = pReservation->GetClass();
		send.nLevel = pReservation->get_level();
		send.nEquipScore = pReservation->GetEquipTeamInfo().n32_Rating;
		pRole->SendMessage(&send, send.dw_size);
	}
	else
	{
		send.dw_error = dw_error;
		pRole->SendMessage(&send, send.dw_size);
	}

	return 0;
}

DWORD PlayerSession::HandleReservationResult(tag_net_message* pCmd)
{
	NET_SIC_reservation_result* p_recv = (NET_SIC_reservation_result*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	if(!pRole->get_check_safe_code() && p_recv->b_ok)
	{
		if(GetBagPsd() != p_recv->dw_safe_code)
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;

		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}

	DWORD	dw_error = pRole->reservation_result(p_recv->dw_role_id, p_recv->b_ok);

	NET_SIS_reservation_result send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleStartHangGetExp(tag_net_message *pCmd)
{
	NET_SIC_start_hang_get_exp* p_recv = (NET_SIC_start_hang_get_exp*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;

	NET_SIS_start_hang_get_exp send;
	send.dwError = pRole->StartHangGetExp( p_recv->type );
	SendMessage(&send, send.dw_size);

	return E_Success;
}
DWORD PlayerSession::HandleStopHangGetExp(tag_net_message *pCmd)
{
	NET_SIC_stop_hang_get_exp* p_recv = (NET_SIC_stop_hang_get_exp*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole)) return INVALID_VALUE;
	pRole->StopHangGetExp( );
	return E_Success;
}