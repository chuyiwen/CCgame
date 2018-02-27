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
#include "player_session.h"
#include "role.h"
#include "creature.h"
#include "map.h"
#include "script_mgr.h"

#include "../../common/WorldDefine/role_att_protocol.h"
#include "creature_ai.h"

DWORD PlayerSession::HandleTalkToNPC(tag_net_message* pCmd)
{
	M_trans_pointer(p_receive, pCmd, NET_SIC_npc_talk);

	M_trans_else_ret(pRole,		GetRole(),							Role,				INVALID_VALUE);
	M_trans_else_ret(pMap,		pRole->get_map(),					Map,				INVALID_VALUE);
	M_trans_else_ret(pNpc,		pMap->find_creature(p_receive->dwNPCId), Creature,			INVALID_VALUE);
	M_trans_else_ret(pScript,	pNpc->GetScript(),					CreatureScript,		INVALID_VALUE);
	
	// 如果在巡逻则停下一段时间
	if (pNpc->GetMoveData().GetCurMoveState() == EMS_CreaturePatrol)
	{
		pNpc->GetAI()->PausePatrol(pRole->GetCurPos()-pNpc->GetCurPos(), CREATURE_ONTALE_PAUSE_TIME);
	}

	// 触发谈话
	pScript->OnTalk(pNpc, pRole, -1);

	return 0;
}