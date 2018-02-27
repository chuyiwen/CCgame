
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//天资消息处理类

#include "stdafx.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/talent_define.h"
#include "../../common/WorldDefine/talent_protocol.h"

#include "player_session.h"
#include "unit.h"
#include "role.h"


//-----------------------------------------------------------------------------
// 学习技能
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLearnSkill(tag_net_message* pCmd)
{
	NET_SIC_learn_skill* p_receive = (NET_SIC_learn_skill*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->LearnSkill(p_receive->dwSkillID, p_receive->dwNPCID, p_receive->n64ItemID);

	// 发送返回消息
	NET_SIS_learn_skill send;
	send.dwSkillID = p_receive->dwSkillID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------
// 技能升级
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLevelUpSkill(tag_net_message* pCmd)
{
	NET_SIC_level_up_skill* p_receive = (NET_SIC_level_up_skill*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->LevelUpSkill(p_receive->dwSkillID, p_receive->dwNPCID, p_receive->n64ItemID);

	// 发送返回消息
	NET_SIS_level_up_skill send;
	send.dwSkillID = p_receive->dwSkillID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}

//-----------------------------------------------------------------------------
// 遗忘技能
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleForgetSkill(tag_net_message* pCmd)
{
	NET_SIC_forget_skill* p_receive = (NET_SIC_forget_skill*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->ForgetSkill(p_receive->dwSkillID, p_receive->dwNPCID);

	// 发送返回消息
	NET_SIS_forget_skill send;
	send.dwSkillID = p_receive->dwSkillID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 洗点
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleClearTalent(tag_net_message* pCmd)
{
	NET_SIC_clear_talent* p_receive = (NET_SIC_clear_talent*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->ClearTalent(p_receive->n64ItemID);

	// 发送返回消息
	NET_SIS_clear_talent send;
	send.eType = p_receive->eType;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}
//------------------------------------------------------------------------------
// 武器学习技能
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEquipLearnSkill(tag_net_message* pCmd)
{
	NET_SIC_equip_learn_skill* p_receive = (NET_SIC_equip_learn_skill*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->WeaponLearnSkill(p_receive->n64EquipID, p_receive->dwSkillID);

	// 发送返回消息
	NET_SIS_equip_learn_skill send;
	send.n64EquipID = p_receive->n64EquipID;
	send.dwSkillID  = p_receive->dwSkillID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}
//------------------------------------------------------------------------------
// 武器升级技能
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEquipLevelUpSkill(tag_net_message* pCmd)
{
	NET_SIC_equip_level_up_skill* p_receive = (NET_SIC_equip_level_up_skill*)pCmd;
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	DWORD dw_error_code = m_pRole->WeaponLevelUpSkill(p_receive->n64EquipID, p_receive->dwSkillID);

	// 发送返回消息
	NET_SIS_equip_level_up_skill send;
	send.n64EquipID = p_receive->n64EquipID;
	send.dwSkillID  = p_receive->dwSkillID;
	send.dw_error_code = dw_error_code;
	SendMessage(&send, send.dw_size);

	return 0;
}
