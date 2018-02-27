
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 任务相关
#include "StdAfx.h"

#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/quest_protocol.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../../common/WorldDefine/script_protocol.h"
#include "../../common/WorldDefine/ScriptMsgInfo.h"
#include "../../common/WorldDefine/guerdon_quest_protocol.h"
#include "../../common/WorldDefine/creature_define.h"
#include "../../common/WorldDefine/TeamRandShareProtocol.h"
#include "../../common/WorldDefine/RoleCarryDefine.h"

#include "player_session.h"
#include "role.h"
#include "role_mgr.h"
#include "quest.h"
#include "quest_mgr.h"
#include "script_mgr.h"
#include "creature.h"
#include "activity_mgr.h"
#include "guerdon_quest_mgr.h"
#include "TeamRandShareMgr.h"

//------------------------------------------------------------------------------------
// 接取任务
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleNPCAcceptQuest(tag_net_message* p_cmd)
{
	if (VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	NET_SIC_npc_accept_quest* p_receive = (NET_SIC_npc_accept_quest*)p_cmd;
	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	INT n_ret = p_role->accept_quest_from_npc(p_receive->u16QuestID, p_receive->dwNPCID);

	NET_SIS_accept_quest send;
	send.u16QuestID = p_receive->u16QuestID;
	send.dw_error_code = n_ret;
	SendMessage(&send, send.dw_size);

	return 0;
}

//--------------------------------------------------------------------------------------
// 通过触发器接任务
//-------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleTriggerAcceptQuest(tag_net_message* p_cmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	NET_SIC_trigger_accept_quest* p_receive = (NET_SIC_trigger_accept_quest*)p_cmd;

	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	INT n_ret = p_role->accept_quest_from_trigger(p_receive->u16QuestID, p_receive->dwTriggerID);

	NET_SIS_accept_quest send;
	send.u16QuestID = p_receive->u16QuestID;
	send.dw_error_code = n_ret;
	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------------
// 是否可以完成此任务
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCompleteQuest(tag_net_message* p_cmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	NET_SIC_complete_quest* p_receive = (NET_SIC_complete_quest*)p_cmd;

	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	quest* pQuest = p_role->get_quest(p_receive->u16QuestID);
	// 超级密码验证
	if (VALID_POINT(pQuest))
	{
		const tagQuestProto* pQuestProto = pQuest->get_protocol();
		if (VALID_POINT(pQuestProto) && pQuestProto->complete_req_bag_password)
		{
			if(!p_role->get_check_safe_code())
			{
				if(GetBagPsd() != p_receive->dw_safe_code)
				{
					NET_SIS_code_check_ok send_check;
					send_check.bSuccess = FALSE;
					p_role->SendMessage(&send_check, send_check.dw_size);

					return INVALID_VALUE;
				}
				else
				{
					NET_SIS_code_check_ok send_check;
					send_check.bSuccess = TRUE;
					p_role->SendMessage(&send_check, send_check.dw_size);

					p_role->set_check_safe_code();
				}
			}
		}
	}
	

	if(QuestIDHelper::SpecialID(p_receive->u16QuestID) ||
		(VALID_POINT(pQuest)  && pQuest->get_quest_flag().dwQuestBeGuerdon) )
	{
		NET_SIC_CompleteGDQuest send;
		send.dwNPCID = p_receive->dwNPCID;
		send.u16QuestID = p_receive->u16QuestID;
		send.choice_index = p_receive->nRewChoicesItemIndex;
		g_GuerdonQuestMgr.AddEvent(p_role->GetID(), EVT_CompGDQuest, send.dw_size, &send);
		return 0;
	}

	UINT16 next_quest = 0;

	INT n_ret = p_role->complete_quest(p_receive->u16QuestID, p_receive->dwNPCID, 
									  p_receive->nRewChoicesItemIndex, next_quest);
	// 发送返回
	NET_SIS_complete_quest send;
	send.u16QuestID = p_receive->u16QuestID;
	send.dw_error_code = n_ret;
	SendMessage(&send, send.dw_size);

	// 检测是否有后续自动接取的任务
	if(E_Success == n_ret && next_quest != 0)
		p_role->accept_quest_from_npc(next_quest, INVALID_VALUE);

	return 0;
}

//------------------------------------------------------------------------------------
// 是否可以删除此任务
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDeleteQuest(tag_net_message* p_cmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	NET_SIC_delete_quest* p_receive = (NET_SIC_delete_quest*)p_cmd;

	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	INT n_ret = E_Success;
	quest* pQuest = p_role->get_quest(p_receive->u16QuestID);
	if(QuestIDHelper::SpecialID(p_receive->u16QuestID) ||
		(VALID_POINT(pQuest) && pQuest->get_quest_flag().dwQuestBeGuerdon))
	{
		n_ret = E_FAILED_QUEST_FORBID_DELETE;
	}
	else  n_ret = p_role->delete_quest(p_receive->u16QuestID);

	// 发送返回消息
	NET_SIS_delete_quest send;
	send.u16QuestID = p_receive->u16QuestID;
	send.dw_error_code = n_ret;
	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------------
// 更新NPC对话任务状态
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleUpdateQuestNPCTalk(tag_net_message* p_cmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	NET_SIC_update_quest_npc_talk* p_receive = (NET_SIC_update_quest_npc_talk*)p_cmd;

	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;

	p_role->update_quest_npc_talk(p_receive->u16QuestID, p_receive->dwNPCID);

	return 0;
}

//------------------------------------------------------------------------------
// 设置任务追踪
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleTrackQuest(tag_net_message* p_cmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	// 检查Role是否合法
	Role* p_role = GetRole();
	if( !VALID_POINT(p_role) ) return INVALID_VALUE;
	
	NET_SIC_set_quest_track* p_recive = (NET_SIC_set_quest_track*)p_cmd;

	NET_SIS_set_quest_track send;
	send.u16QuestID = p_recive->u16QuestID;
	send.dwCode = p_role->set_track_quest(p_recive->u16QuestID, p_recive->byTrack);
	SendMessage(&send, send.dw_size);
	
	return 0;
}

//------------------------------------------------------------------------------
// 环随机任务
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleCircleQuestList(tag_net_message* pCmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	// 检查Role是否合法
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	if(!VALID_POINT(p_role->get_map())) return INVALID_VALUE;

	NET_SIC_CircleQuestList* pProtocol = (NET_SIC_CircleQuestList*)pCmd;
	Creature* pNpc = p_role->get_map()->find_creature(pProtocol->dwNpcID);
	if(!VALID_POINT(pNpc) || !pNpc->IsFunctionNPC(EFNPCT_CircleQuest))
		return INVALID_VALUE;


	p_role->rand_circle_quest();
	p_role->SendCirleQuest( );
	return 0;
}

DWORD PlayerSession::HandleRefreshCircleQuest(tag_net_message* pCmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	// 检查Role是否合法
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	if(!VALID_POINT(p_role->get_map())) return INVALID_VALUE;

	NET_SIC_RefreshCircleQuest* pProtocol = (NET_SIC_RefreshCircleQuest*)pCmd;
	Creature* pNpc = p_role->get_map()->find_creature(pProtocol->dwNpcID);
	if(!VALID_POINT(pNpc) || !pNpc->IsFunctionNPC(EFNPCT_CircleQuest))
		return INVALID_VALUE;


	if(!p_role->get_circle_quest_man().Exist(pProtocol->u16QuestID))
		return INVALID_VALUE;

	quest* pQuest = p_role->get_quest(pProtocol->u16QuestID);

	if(VALID_POINT(pQuest))
		return INVALID_VALUE;

	INT nDeltaRefresh = GET_VIP_EXTVAL(GetTotalRecharge(), QUEST_REFRESH, INT);
	if(p_role->GetCircleRefrshNumber() <= 0 && p_role->GetDayClearData(ERDCT_QuestRefresh) >= nDeltaRefresh )
		return INVALID_VALUE;

	if(p_role->GetCircleRefrshNumber())p_role->DecCirleRefresh( );
	else p_role->ModRoleDayClearDate(ERDCT_QuestRefresh);
	p_role->auto_rand_circle_quest_to_index(pProtocol->u16QuestID);
	
	p_role->GetAchievementMgr().UpdateAchievementCriteria(eta_refresh_circle_quest, 1);
	return 0;
}

//-----------------------------------------------------------------------------
// 客户端发给服务的对话框缺省消息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDlgDefaultMsg(tag_net_message* p_cmd)
{
	//GET_MESSAGE(p_receive, p_cmd, NET_SIC_dialog_default_message);
	NET_SIC_dialog_default_message * p_receive = ( NET_SIC_dialog_default_message * ) p_cmd ;  
	Role* p_role = GetRole();

	if(!VALID_POINT(p_role)) return INVALID_VALUE;

	switch (p_receive->eDlgTarget)
	{
	case EMUT_DlgTarget_Quest:
		break;
	case EMUT_DlgTarget_Item:
		break;
	case EMUT_DlgTarget_Creature:
		{
			DWORD			dw_npc		= p_receive->dwTargetID;
			EDlgOption		option		= p_receive->eDlgOption;
			M_trans_else_ret(p_map,	p_role->get_map(),				Map,			INVALID_VALUE);
			M_trans_else_ret(p_npc,	p_map->find_creature(dw_npc),	Creature,		INVALID_VALUE);
			M_trans_else_ret(p_script,p_npc->GetScript(),			CreatureScript,	INVALID_VALUE);
			p_script->OnTalk(p_npc, p_role, (INT)option);
		}
		break;
	default:
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 客户端触发服务器脚本的缺省消息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDefaultRequest(tag_net_message* p_cmd)
{
	//GET_MESSAGE(p_receive, p_cmd, NET_SIC_default_request);
	NET_SIC_default_request * p_receive = ( NET_SIC_default_request * ) p_cmd ;  
	Role* p_role = GetRole();

	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	
	switch (p_receive->eDlgTarget)
	{
		case EMUT_Request_Activity:
			{
				activity_fix* p_activity = activity_mgr::GetInstance()->get_activity(p_receive->dwTargetID);
				if( !VALID_POINT(p_activity) ) return INVALID_VALUE;

				const ActScript* p_script = p_activity->get_script();
				if( !VALID_POINT(p_activity) ) return INVALID_VALUE;

				p_script->OnDefaultRequest(p_receive->dwTargetID, p_role, p_receive->dwEventType);
			}
			break;

		default:
			break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 发布悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandlePutQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	
	NET_SIC_PutGDQuest* p_receive = (NET_SIC_PutGDQuest*)p_cmd;
	
	if(!p_role->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			p_role->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			p_role->SendMessage(&send_check, send_check.dw_size);

			p_role->set_check_safe_code();
		}
	}

	g_GuerdonQuestMgr.EvtPutQuest(p_role->GetID(), p_cmd);
	return 0;
}
//-----------------------------------------------------------------------------
// 接取悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;

	g_GuerdonQuestMgr.EvtGetQuest(p_role->GetID(), p_cmd);
	return 0;
}
////-----------------------------------------------------------------------------
//// 放弃悬赏
////-----------------------------------------------------------------------------
//DWORD PlayerSession::HandleGiveUpQuest(tag_net_message* p_cmd)
//{
//	Role* p_role = GetRole();
//	if(!VALID_POINT(p_role)) return INVALID_VALUE;
//
//	return 0;
//}
//-----------------------------------------------------------------------------
// 获取所有悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetOnePageGDQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	g_GuerdonQuestMgr.EvtGetOnePageGDQuest(p_role->GetID(), p_cmd);
	return 0;
}
//-----------------------------------------------------------------------------
// 更新我发布的悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpdateMyPutGDQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;

	g_GuerdonQuestMgr.EvtUpdateMyPutGDQuest(p_role->GetID(), p_cmd);
	return 0;
}

//-----------------------------------------------------------------------------
// 更新我接取的悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpdateMyGetGDQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;

	g_GuerdonQuestMgr.EvtUpdateMyGetGDQuest(p_role->GetID(), p_cmd);
	return 0;
}
//-----------------------------------------------------------------------------
// 取消发布悬赏
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCancelPutGDQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	g_GuerdonQuestMgr.EvtCancelPutQuest(p_role->GetID(), p_cmd);
	return 0;
}

//-----------------------------------------------------------------------------
// WARNING: REGISTER_WORLD_RECV_COMMAND
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleTeamNextQuest(tag_net_message* p_cmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	NET_SIC_Get_Team_Share_Quest *pProtocol = (NET_SIC_Get_Team_Share_Quest*)p_cmd;
	sTeamShareMgr.MemberNext(pProtocol->dwNPCID, p_role->GetID());
	return 0;
}


//----------------------------------------------------------------------------
// 搬运
//----------------------------------------------------------------------------
DWORD PlayerSession::HandleStopCarrySomething(tag_net_message* pCmd)
{
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;


	NET_SIC_StopCarrySomething *pProtocol = (NET_SIC_StopCarrySomething*)pCmd;
	
	p_role->UnsetRoleState(ERS_Carry);

	return 0;
}

DWORD PlayerSession::HandleBuyRefreshCircleQuest(tag_net_message *pCmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	// 检查Role是否合法
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	if(!VALID_POINT(p_role->get_map())) return INVALID_VALUE;


	NET_SIC_BuyRefreshCircleQuest *pProtocol = (NET_SIC_BuyRefreshCircleQuest*)pCmd;

	Creature* pNpc = p_role->get_map()->find_creature(pProtocol->dwNpcID);
	if(!VALID_POINT(pNpc) || !pNpc->IsFunctionNPC(EFNPCT_CircleQuest))
		return INVALID_VALUE;

	NET_SIS_BuyRefreshCircleQuest send;
	send.dwError = p_role->BuyRefreshCircleQuest( );
	SendMessage(&send, send.dw_size);

	return 0;
}
DWORD PlayerSession::HandleBuyCircleQuestPerdayNumber(tag_net_message *pCmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Quest);
		return 0;
	}

	// 检查Role是否合法
	Role* p_role = GetRole();
	if(!VALID_POINT(p_role)) return INVALID_VALUE;
	if(!VALID_POINT(p_role->get_map())) return INVALID_VALUE;

	NET_SIC_BuyCircleQuestPerdayNumber *pProtocol = (NET_SIC_BuyCircleQuestPerdayNumber*)pCmd;

	Creature* pNpc = p_role->get_map()->find_creature(pProtocol->dwNpcID);
	if(!VALID_POINT(pNpc) || !pNpc->IsFunctionNPC(EFNPCT_CircleQuest))
		return INVALID_VALUE;

	p_role->BuyCircleQuestPerdayNumber( );

	return 0;
}