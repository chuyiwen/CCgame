
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 选人界面相关

#include "StdAfx.h"

#include "../../common/WorldDefine/select_role_protocol.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/common_server_define.h"
#include "../common/ServerDefine/paimai_define.h"
#include "../../common/WorldDefine/time_protocol.h"
#include "../../common/WorldDefine/filter.h"
#include "../../common/WorldDefine/verification_protocol.h"
#include "../../common/WorldDefine/activity_protocol.h"

#include "player_session.h"
#include "db_session.h"
#include "role.h"
#include "role_mgr.h"
#include "map_creator.h"
#include "map.h"
#include "world_session.h"
#include "guild_manager.h"
#include "master_prentice_mgr.h"
#include "center_session.h"
#include "RankMgr.h"
#include "guild.h"

//--------------------------------------------------------------------------
// 进入游戏
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleJoinGame(tag_net_message* pCmd)
{
	// 发送返回消息
	NET_SIS_join_game send;
	send.dw_error_code = 0;

	SendMessage(&send, send.dw_size);

	return 0;
}

//--------------------------------------------------------------------------
// 申请角色列表
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleEnum(tag_net_message* pCmd)
{
	NET_SIC_enum_role* p_receive = (NET_SIC_enum_role*)pCmd;

	DWORD dw_error_code = E_Success;

	if( m_bRoleEnumDone )
	{
		dw_error_code = E_EnumRole_RoleEnumed;
	}
	else if( m_bRoleEnuming )
	{
		dw_error_code = E_EnumRole_RoleEnuming;
	}
	else if( m_bRoleInWorld || m_pRole )
	{
		dw_error_code = E_EnumRole_InWorld;
	}

	// 如果检查不合法，则返回错误码
	if( E_Success != dw_error_code )
	{
		NET_SIS_enum_role send;
		send.dw_error_code = dw_error_code;
		SendMessage(&send, send.dw_size);

		return INVALID_VALUE;
	}
	else
	{
		// 同步给客户端服务器时间
		NET_SIS_synchronize_time sendClient;
		sendClient.dw_time = GetCurrentDWORDTime();
		SendMessage(&sendClient, sendClient.dw_size);

		NET_DB2C_load_sim_role send;
		send.dw_account_id = m_dwAccountID;
		get_fast_code()->memory_copy(send.sz_account, GetAccount(), sizeof(send.sz_account));;
		g_dbSession.Send(&send, send.dw_size);

		GetVerification().reset();
		// 请求验证码信息
		//if (AttRes::GetInstance()->GetVariableLen().n_verification == 1 && 
		//	!IsPrivilegeEnough(6) && 
		//	g_center_session.IsWell())
		//{
		//	SendVerifyCodeMessage();
		//}

		m_bRoleEnuming = true;
		
		return 0;
	}

}

//--------------------------------------------------------------------------
// 检查角色名称
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCheckName(tag_net_message* pCmd)
{
	NET_SIC_check_name* p_receive = (NET_SIC_check_name*)pCmd;

	DWORD dwError = E_Success;

	TCHAR	buf[X_SHORT_NAME] = {0};
	_tcsncpy(buf, p_receive->szRoleName, X_SHORT_NAME);
	buf[X_SHORT_NAME-1] = _T('\0');
	_tcslwr(buf);
	DWORD dwNameCrc = get_tool()->crc32(buf);
	if (g_roleMgr.get_role_id(dwNameCrc) != (DWORD)INVALID_VALUE)
	{
		//RoleName已经存在
		dwError = E_CreateRole_NameExist;
	}
	else
	{
		// 检查名字长度,合法性由客户端保证
		dwError = Filter::CheckName(buf, AttRes::GetInstance()->GetVariableLen().nRoleNameMax, 
			AttRes::GetInstance()->GetVariableLen().nRoleNameMin, AttRes::GetInstance()->GetNameFilterWords());
	}

	NET_SIS_check_name send;
	send.dwError = dwError;
	SendMessage(&send, send.dw_size);

	return INVALID_VALUE;
}

//--------------------------------------------------------------------------
// 创建角色
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCreate(tag_net_message* pCmd)
{
	NET_SIC_create_role* p_receive = (NET_SIC_create_role*)pCmd;

	DWORD dw_error_code = E_Success;

	// 检查玩家当前的状态
	if( !m_bRoleEnumDone )
	{
		dw_error_code = E_CreateRole_RoleEnumNotDone;
	}
	else if( m_bRoleCreating )
	{
		dw_error_code = E_CreateRole_RoleCreating;
	}
	else if (!m_bRoleEnumSuccess)
	{
		dw_error_code = E_CreateRole_RoleEnumNotSuccess;
	}
	else if( m_bRoleDeleting )
	{
		dw_error_code = E_CreateRole_RoleDeleting;
	}
	else if( m_bRoleLoading )
	{
		dw_error_code = E_CreateRole_RoleLoading;
	}
	else if( m_bRoleChangeNameing )
	{
		dw_error_code = E_CreateRole_RoleChangNaming;
	}
	else if( m_bRoleDelGuardCanceling )
	{
		dw_error_code = E_CreateRole_RoleCancelDelGuardTime;
	}
	else if( m_SessionCommonData.GetSize() >= MAX_ROLECREATENUM_ONEACCOUNT )
	{
		dw_error_code = E_CreateRole_RoleNumFull;
	}
	else if( m_bRoleInWorld || m_pRole )
	{
		dw_error_code = E_CreateRole_InWorld;
	}
	else if(p_receive->eClassType <= EV_Null || p_receive->eClassType >= EV_End )
	{
		dw_error_code = INVALID_VALUE;
	}
	else
	{
		// Todo：要检查安全码的合法性，同时要将rolename
		// 是不是还要检查一下Avatar中的各个字段值
		//检查玩家要创建的RoleName是否已经存在
		//将RoleName转成小写计算CRC32
		TCHAR	buf[X_SHORT_NAME] = {0};
		_tcsncpy(buf, p_receive->szRoleName, X_SHORT_NAME);
		buf[X_SHORT_NAME-1] = _T('\0');
		_tcslwr(buf);
		DWORD dwNameCrc = get_tool()->crc32(buf);
		if (g_roleMgr.get_role_id(dwNameCrc) != (DWORD)INVALID_VALUE)
		{
			//RoleName已经存在
			dw_error_code = E_CreateRole_NameExist;
		}
		else
		{
			// 检查名字长度,合法性由客户端保证
			dw_error_code = Filter::CheckName(buf, AttRes::GetInstance()->GetVariableLen().nRoleNameMax, 
							AttRes::GetInstance()->GetVariableLen().nRoleNameMin, AttRes::GetInstance()->GetNameFilterWords());
		}	
	}

	if( E_Success != dw_error_code )
	{
		NET_SIS_create_role send;
		send.dw_error_code = dw_error_code;

		SendMessage(&send, send.dw_size);

		return INVALID_VALUE;
	}
	else
	{
		NET_DB2C_create_role send;
		send.dw_account_id = m_dwAccountID;

		// Todo: 得到帐号名字
		send.s_role_info_.e_class_type_ = p_receive->eClassType;
		
		get_fast_code()->memory_copy(&send.s_role_info_.avatar, &p_receive->AvatarAtt, sizeof(tagAvatarAtt));
		send.s_role_info_.avatar.byClass = (BYTE)p_receive->eClassType;
		get_fast_code()->memory_copy(send.s_role_info_.sz_role_name, p_receive->szRoleName, sizeof(p_receive->szRoleName));

		send.s_role_info_.by_level_ = 1;
		send.s_role_info_.dw_map_id_ = g_mapCreator.get_born_map_id();
		
		send.s_role_info_.create_time_ = g_world.GetWorldTime();

		// 临时填写，到后期会从该地图的导航点中寻找到一个随机坐标点
		send.s_role_info_.fX = 0.0f;
		send.s_role_info_.fY = 0.0f;
		send.s_role_info_.fZ = 0.0f;
		send.s_role_info_.f_face_x_ = 0.0f;
		send.s_role_info_.f_face_y_ = 0.0f;
		send.s_role_info_.f_face_z_ = 0.0f;

		g_dbSession.Send(&send, send.dw_size);

		m_bRoleCreating = true;

		return 0;
	}
}

//--------------------------------------------------------------------------
// 删除角色
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleDelete(tag_net_message* pCmd)
{
	NET_SIC_delete_role* p_receive = (NET_SIC_delete_role*)pCmd;

	DWORD dw_error_code = E_Success;
	// 检查人物当前状态
	if( !m_bRoleEnumDone )
	{
		dw_error_code = E_DelRole_RoleEnumNotDone;
	}
	else if( m_bRoleDeleting )
	{
		dw_error_code = E_DelRole_RoleDeleting;
	}
	else if( m_bRoleCreating )
	{
		dw_error_code = E_DelRole_RoleCreating;
	}
	else if( m_bRoleLoading )
	{
		dw_error_code = E_DelRole_RoleLoading;
	}
	else if(m_bRoleChangeNameing)
	{
		dw_error_code = E_DelRole_RoleChangeNaming;
	}
	else if( m_bRoleDelGuardCanceling )
	{
		dw_error_code = E_DelRole_RoleCancelDelGuardTime;
	}
	else if( m_bRoleInWorld || m_pRole )
	{
		dw_error_code = E_DelRole_InWorld;
	}
	else if( FALSE == IsRoleExist(p_receive->dw_role_id) )
	{
		dw_error_code = E_DelRole_RoleNotExist;
	}
	else if(g_guild_manager.clear_role_remove(p_receive->dw_role_id) != E_Success)
	{
		// 帮派处理
		dw_error_code = E_DelRole_LeaveGuild1st;
	}

	//! 角色删除保护
	INT role_index = m_SessionCommonData.Find(PSCSearchPred(p_receive->dw_role_id));
	PlayerSessionCommonData* pPSCD = m_SessionCommonData.GetPtr(role_index);
	if(!VALID_POINT(pPSCD)){
		print_message(_T("delete role guard logic error[invalid role:%d]"), p_receive->dw_role_id);
		return INVALID_VALUE;
	}

	if(VALID_POINT(pPSCD->dwDelGuardTime))
	{
		if(pPSCD->dwDelGuardTime > GetCurrentDWORDTime())
			dw_error_code = E_DelRole_InDelGuardTime;
		else if(p_receive->dwSafeCodeCrc != m_sAccountCommon.dw_bag_password_crc_)
			dw_error_code = E_DelRole_SafeCodeIncorrect;
	}

	s_role_info* info = g_roleMgr.get_role_info(p_receive->dw_role_id);
	if(VALID_POINT(info) && info->by_level < 0)
	{
		if(p_receive->dwSafeCodeCrc != m_sAccountCommon.dw_bag_password_crc_)
			dw_error_code = E_DelRole_SafeCodeIncorrect;
	}
	//! 离线修炼不能删除角色
	Role* pRole = g_roleMgr.get_role(p_receive->dw_role_id);
	if(VALID_POINT(pRole) && pRole->is_leave_pricitice( ))
		dw_error_code = E_DelRole_RoleInLeavePractice;

	if( E_Success != dw_error_code )
	{
		NET_SIS_delete_role send;
		send.dw_error_code	= dw_error_code;
		send.dw_role_id		= p_receive->dw_role_id;

		SendMessage(&send, send.dw_size);

		return INVALID_VALUE;
	}

	// 下面两种情况都需要等数据库返回
	m_bRoleDeleting = true;

	// 还未处于删除保护阶段，不从数据库删除，只是设置删除保护时间
	if(!VALID_POINT(pPSCD->dwDelGuardTime) && VALID_POINT(info) && info->by_level >= 20){

		NET_DB2C_delete_role_guard_time_set tSendGuardTimeSet;
		tSendGuardTimeSet.dw_account_id = m_dwAccountID;
		tSendGuardTimeSet.dw_role_id = p_receive->dw_role_id;
		tSendGuardTimeSet.dw_delGuardTime = IncreaseTime(GetCurrentDWORDTime(), DELETE_ROLE_GUARD_SECOND);
		g_dbSession.Send(&tSendGuardTimeSet, tSendGuardTimeSet.dw_size);
	}
	// 从数据库删除
	else 
	{
		//师徒
		g_MasterPrenticeMgr.role_deleted( p_receive->dw_role_id, p_receive );

		// 设置删除标志位
		NET_DB2C_delete_role send;
		send.dw_account_id = m_dwAccountID;
		send.dw_role_id = p_receive->dw_role_id;
		send.dw_time = g_world.GetWorldTime();

		CHAR szIP[X_IP_LEN] = {0};
		strncpy(szIP, inet_ntoa((*(in_addr*)&m_dwIP)), X_IP_LEN);
		get_fast_code()->memory_copy(send.sz_ip, szIP, sizeof(szIP));

		g_roleMgr.get_role_name(p_receive->dw_role_id, send.sz_role_name);

		g_dbSession.Send(&send, send.dw_size);

		// 清除地图限制
		NET_DB2C_remove_role_map_limit resend;
		resend.dw_role_id = p_receive->dw_role_id;
		g_dbSession.Send(&resend, resend.dw_size);

		// 清除帮会榜数据
		if(g_guild_manager.is_have_recruit(p_receive->dw_role_id))
		{
			g_guild_manager.delete_role_from_guild_recruit(p_receive->dw_role_id);
			NET_C2DB_delete_guild_recruit send;
			send.dw_role_id = p_receive->dw_role_id;
			g_dbSession.Send(&send, send.dw_size);
		}
	}

	return 0;
}

//--------------------------------------------------------------------------
// 恢复删除角色
//--------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleGuardCancel(tag_net_message* pCmd)
{
	NET_SIC_delete_role_guard_Cancel* p_receive = (NET_SIC_delete_role_guard_Cancel*)pCmd;

	DWORD dw_error_code = E_Success;
	// 检查人物当前状态
	if( !m_bRoleEnumDone )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleEnumNotDone;
	}
	else if( m_bRoleDeleting )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleDeleting;
	}
	else if( m_bRoleCreating )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleCreating;
	}
	else if( m_bRoleLoading )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleLoading;
	}
	else if(m_bRoleChangeNameing)
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleChangNaming;
	}
	else if( m_bRoleDelGuardCanceling )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleCancelDelGuardTime;
	}
	else if( m_bRoleInWorld || m_pRole )
	{
		dw_error_code = E_CancelRoleDelGuardTime_InWorld;
	}
	else if( FALSE == IsRoleExist(p_receive->dw_role_id) )
	{
		dw_error_code = E_CancelRoleDelGuardTime_RoleNotExist;
	}
	else if(!VALID_POINT(GetRoleDelGuardTime(p_receive->dw_role_id)))
	{
		dw_error_code = E_CancelRoleDelGuardTime_NotInDelGuard;
	}

	if(dw_error_code != E_Success)
	{
		NET_SIS_delete_role_guard_Cancel send;
		send.dw_error = dw_error_code;
		send.dw_role_id = p_receive->dw_role_id;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	m_bRoleDelGuardCanceling = true;

	//! 重置
	{
		NET_DB2C_cancel_role_del_guard_time send;
		send.dw_account_id = m_dwAccountID;
		send.dw_role_id = p_receive->dw_role_id;
		g_dbSession.Send(&send, send.dw_size);
	}

	return 0;
}
//------------------------------------------------------------------------------------
// 选择角色
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSelect(tag_net_message* pCmd)
{
	NET_SIC_select_role* p_receive = (NET_SIC_select_role*)pCmd;

	DWORD dw_error_code = E_Success;

	if(!g_world.is_join_world())
	{
		dw_error_code = E_SelectRole_Not_Join_Game;
	}
	else if( !m_bRoleEnumDone )
	{
		dw_error_code = E_SelectRole_RoleEnumNotDone;
	}
	else if( m_bRoleCreating )
	{
		dw_error_code = E_SelectRole_RoleCreating;
	}
	else if( m_bRoleDeleting )
	{
		dw_error_code = E_SelectRole_RoleDeleting;
	}
	else if( m_bRoleLoading )
	{
		dw_error_code = E_SelectRole_RoleLoading;
	}
	else if( m_bRoleChangeNameing )
	{
		dw_error_code = E_SelectRole_RoleChangNaming;
	}
	else if( m_bRoleInWorld )
	{
		dw_error_code = E_SelectRole_InWorld;
	}
	else if( FALSE == IsRoleExist(p_receive->dw_role_id) )
	{
		dw_error_code = E_SelectRole_RoleNotExist;
	}
	else if(VALID_POINT(GetRoleDelGuardTime(p_receive->dw_role_id)))
	{
		dw_error_code = E_SelectRole_RoleInDelGuard;
	}
	else if( FALSE == GetVerification().isSuccess(p_receive->dw_verification_code_crc) &&
		GetVerification().isNeedVerification())
	{
		dw_error_code = E_SelectRole_VerificationError;
		// 重新生产一个验证码
		//GetVerification().resetVerificationCode();
		SendVerifyCodeMessage();
		//NET_SIS_reset_verification_code send;
		//GetVerification().getCryptString(send.byStrCode);
		//send.byVerificationCodeIndex = GetVerification().getIndex();
		//SendMessage(&send, send.dw_size);
	}

	if( E_Success != dw_error_code )
	{
		NET_SIS_select_role send;
		send.dw_error_code = dw_error_code;

		SendMessage(&send, send.dw_size);
	}
	else
	{
		NET_DB2C_load_role send;
		send.dw_account_id = m_dwAccountID;
		send.dw_role_id = p_receive->dw_role_id;

		g_dbSession.Send(&send, send.dw_size);

		m_bRoleLoading = true;
	}

	return 0;
}

//------------------------------------------------------------------------------------
// 角色更名
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChangeName(tag_net_message* pCmd)
{
	NET_SIC_change_role_name* p_receive = (NET_SIC_change_role_name*)pCmd;

	DWORD dw_error_code = E_Success;

	if( m_bRoleInWorld || m_pRole )
	{
		dw_error_code = E_ChangeRoleName_InWorld;
	}
	else if( !m_bRoleEnumDone )
	{
		dw_error_code = E_ChangeRoleName_RoleEnumNotDone;
	}
	else if( m_bRoleCreating )
	{
		dw_error_code = E_ChangeRoleName_RoleCreating;
	}
	else if( m_bRoleDeleting )
	{
		dw_error_code = E_ChangeRoleName_RoleDeleting;
	}
	else if( m_bRoleLoading )
	{
		dw_error_code = E_ChangeRoleName_RoleLoading;
	}
	else if( m_bRoleChangeNameing )
	{
		dw_error_code = E_ChangeRoleName_RoleChangNaming;
	}
	else if( m_bRoleDelGuardCanceling )
	{
		dw_error_code = E_ChangeRoleName_RoleCancelDelGuardTime;
	}
	else if( FALSE == IsRoleExist(p_receive->dw_role_id) )
	{
		dw_error_code = E_ChangeRoleName_RoleNotExist;
	}
	else if(VALID_POINT(GetRoleDelGuardTime(p_receive->dw_role_id)))
	{
		dw_error_code = E_ChangeRoleName_InDelGuard;
	}
	else if(!IsCanChangeName(p_receive->dw_role_id))
	{
		dw_error_code = E_ChangeRoleName_Cant60Days; 
	}
	else if( m_sAccountCommon.dw_bag_password_crc_ != p_receive->dw_safe_code)
	{
		dw_error_code = E_ChangeRoleName_BagPasswordError;
	}
	else if( m_sAccountCommon.n_baibao_yuanbao_ < CHANGE_NAME_COST_YUANBAO)
	{
		dw_error_code = E_ChangeRoleName_OutOfYuanBao;
	}

	if( E_Success != dw_error_code )
	{
		NET_SIS_change_role_name send;
		send.dw_error_code = dw_error_code;
		send.dw_role_id = p_receive->dw_role_id;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	TCHAR nameTmp[X_SHORT_NAME] = {0};
	_tcsncpy(nameTmp, p_receive->sz_new_role_name, X_SHORT_NAME);
	nameTmp[X_SHORT_NAME-1] = _T('\0'); _tcslwr(nameTmp);
	DWORD dwNameCrc = get_tool()->crc32(nameTmp);

	if (g_roleMgr.get_role_id(dwNameCrc) != (DWORD)INVALID_VALUE)
	{
		//RoleName已经存在
		dw_error_code = E_ChangeRoleName_NameExist;
	}
	else
	{
		// 检查名字长度,合法性由客户端保证
		dw_error_code = Filter::CheckName(nameTmp, AttRes::GetInstance()->GetVariableLen().nRoleNameMax, 
			AttRes::GetInstance()->GetVariableLen().nRoleNameMin, AttRes::GetInstance()->GetNameFilterWords());
	}	

	if( E_Success != dw_error_code )
	{
		NET_SIS_change_role_name send;
		send.dw_error_code = dw_error_code;
		send.dw_role_id = p_receive->dw_role_id;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}


	//!去更名
	{
		NET_DB2C_change_role_name send;
		send.dw_name_crc = dwNameCrc;
		send.dw_account_id = m_dwAccountID;
		send.dw_role_id = p_receive->dw_role_id;
		send.dw_change_time = IncreaseTime(GetCurrentDWORDTime( ), CHANGE_NAME_DIFF_SECOND);
		_tcsncpy(send.sz_new_role_name,p_receive->sz_new_role_name, X_SHORT_NAME);
		send.sz_new_role_name[X_SHORT_NAME-1] = _T('\0');
		g_dbSession.Send(&send, send.dw_size);
	}

	m_bRoleChangeNameing = true;

	return 0;
}
//------------------------------------------------------------------------------------
// 设置安全码
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetSafeCode(tag_net_message* pCmd)
{
	NET_SIC_safe_code* p_receive = (NET_SIC_safe_code*)pCmd;

	if(INVALID_VALUE == p_receive->dwSafeCode)
	{
		return INVALID_VALUE;
	}

	if(!CanSetSafeCode())
	{
		return INVALID_VALUE;
	}

	// 设置
	m_sAccountCommon.s_safe_code_.dw_safe_code_crc = p_receive->dwSafeCode;
	m_sAccountCommon.s_safe_code_.dw_reset_time = INVALID_VALUE;

	NET_DB2C_safe_code_set send2DB;
	send2DB.dw_account_id = GetSessionID();
	send2DB.dw_safe_code_crc = p_receive->dwSafeCode;
	g_dbSession.Send(&send2DB, send2DB.dw_size);

	NET_SIS_safe_code send;
	send.dw_error_code = E_Success;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------------------
// 重置安全码
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleResetSafeCode(tag_net_message* pCmd)
{
	//MGET_MSG(p_receive, pCmd, NET_SIC_reset_safe_code);

	if(!CanResetSafeCode())
	{
		return INVALID_VALUE;
	}

	// 重置
	m_sAccountCommon.s_safe_code_.dw_reset_time = g_world.GetWorldTime();

	NET_DB2C_safe_code_reset send2DB;
	send2DB.dw_account_id = GetSessionID();
	send2DB.dw_reset_time = m_sAccountCommon.s_safe_code_.dw_reset_time;
	g_dbSession.Send(&send2DB, send2DB.dw_size);

	NET_SIS_reset_safe_code send;
	send.dwTimeReset = m_sAccountCommon.s_safe_code_.dw_reset_time;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------------------
// 取消安全码重置
//------------------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCancelSafeCodeReset(tag_net_message* pCmd)
{
	//MGET_MSG(p_receive, pCmd, NET_SIC_cancel_safe_code_reset);

	if(!CanCancelSafeCodeReset())
	{
		return INVALID_VALUE;
	}

	// 取消
	m_sAccountCommon.s_safe_code_.dw_reset_time = INVALID_VALUE;

	NET_DB2C_safe_code_reset_cancel send2DB;
	send2DB.dw_account_id = GetSessionID();
	g_dbSession.Send(&send2DB, send2DB.dw_size);

	NET_SIS_cancel_safe_code_reset send;
	send.dw_error_code = E_Success;
	SendMessage(&send, send.dw_size);

	return E_Success;
}

// 重新得到一个验证码
DWORD	PlayerSession::HandleResetVerCode(tag_net_message* pCmd)
{
	if (!m_bRoleVerifying)
	{
		// 重新生产一个验证码
		//GetVerification().resetVerificationCode();
		SendVerifyCodeMessage();
	}

	//NET_SIS_reset_verification_code send;
	//GetVerification().getCryptString(send.byStrCode);
	//send.byVerificationCodeIndex = GetVerification().getIndex();
	//SendMessage(&send, send.dw_size);
	return E_Success;
}

// 验证
DWORD	PlayerSession::HandleNeedVerCode(tag_net_message* pCmd)
{
	NET_SIC_need_verification_return* p_recv = (NET_SIC_need_verification_return*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	DWORD dwErrorCode = GetVerification().receiveVer(p_recv->dwVerificationCodeCrc, pRole);
	
	if (dwErrorCode == E_VERIFICATION_ERROR)
	{
		SendVerifyCodeMessage();
	}

	NET_SIS_need_verification_return send;
	send.dw_error_code = dwErrorCode;
	SendMessage(&send, send.dw_size);
	
	return E_Success;
}

// 进入正式验证阶段
DWORD PlayerSession::HandleGotoVer(tag_net_message* pCmd)
{
	NET_SIC_goto_verification* p_recv = (NET_SIC_goto_verification*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	GetVerification().SetVeringState();

	return E_Success;
}
// 验证超级密码
DWORD PlayerSession::HandleRoleCodeCheck(tag_net_message* pCmd)
{

	NET_SIC_code_check_ok* p_recv = (NET_SIC_code_check_ok*)pCmd;

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

	return E_Success;
}

//------------------------------------------------------------------------------
// 修改帮助数据
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleModRoleHelp(tag_net_message* pCmd)
{
	NET_SIC_modify_role_help* p_receive = (NET_SIC_modify_role_help*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(p_receive->byIndex < 0 && p_receive->byIndex > (ROLE_HELP_NUM-1))
		return INVALID_VALUE;

	pRole->SetRoleHelp(p_receive->byIndex);
	return 0;
}

//------------------------------------------------------------------------------
// 修改人物对话数据
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleModRoleTalk(tag_net_message* pCmd)
{
	NET_SIC_modify_role_talk* p_receive = (NET_SIC_modify_role_talk*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(p_receive->byIndex < 0 && p_receive->byIndex > (TALK_DATA_NUM-1))
		return INVALID_VALUE;

	if(p_receive->byState < 0 && p_receive->byState > (TALK_DATA_NUM-1))
		return INVALID_VALUE;

	pRole->SetRoleTalk(p_receive->byIndex, p_receive->byState);

	return 0;
}

DWORD PlayerSession::HandlekeyInfo(tag_net_message* pCmd)
{
	NET_SIC_key_info* p_receive = (NET_SIC_key_info*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
		
	pRole->SendKeyInfo();

	return 0;
}

//------------------------------------------------------------------------------
// 修改快捷键数据
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleModKeyInfo(tag_net_message* pCmd)
{
	NET_SIC_modify_key_info* p_receive = (NET_SIC_modify_key_info*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	pRole->SetKeyInfo(&p_receive->stKeyInfo);

	return 0;
}

//--------------------------------------------------------------------
// 活跃度数据
//--------------------------------------------------------------------
DWORD PlayerSession::HandleGetActiveInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	pRole->SendActiveInfo();

	return 0;
}

DWORD PlayerSession::HandleGetGuildActiveInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	pRole->SendGuildActiveInfo();

	return 0;
}

//--------------------------------------------------------------------
// 领取活跃度奖励
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRoleActiveReceive(tag_net_message* pCmd)
{
	NET_SIC_role_active_receive* p_msg = (NET_SIC_role_active_receive*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = pRole->ActiveReceive(p_msg->nIndex);

	NET_SIS_role_active_receive send;
	send.nIndex = p_msg->nIndex;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleRoleActiveDone(tag_net_message* pCmd)
{
	NET_SIC_active_done* p_msg = (NET_SIC_active_done*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = pRole->ActiveDone(p_msg->nIndex,p_msg->nBeishu);

	NET_SIS_active_done send;
	send.nIndex = p_msg->nIndex;
	send.dw_erroe_code = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
//每日活动一键传送
DWORD PlayerSession::HandleDailyActTransmit(tag_net_message* pCmd)
{
	NET_SIC_daily_act_transmit* p_msg = (NET_SIC_daily_act_transmit*)pCmd;
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;
	pRole->DailyActTransmit(p_msg->nIndex); 
	return 0;
}
DWORD PlayerSession::HandleGuildActiveReceive(tag_net_message* pCmd)
{
	NET_SIC_guild_active_receive* p_msg = (NET_SIC_guild_active_receive*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dw_error = pRole->GuildActiveReceive(p_msg->nIndex);

	NET_SIS_guild_active_receive send;
	send.nIndex = p_msg->nIndex;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 启动新手在线奖励
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleStartNewRoleGift(tag_net_message* pCmd)
{
	NET_SIC_start_new_role_gift* p_receive = (NET_SIC_start_new_role_gift*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pRole->m_stNewRoleGift.n_step_-1 >= 0)
		return INVALID_VALUE;

	
	pRole->m_stNewRoleGift.n_step_++;

	pRole->m_stNewRoleGift.b_begin_time_ = TRUE;

	return E_Success;
}

//------------------------------------------------------------------------------
// 获取新手在线奖励
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleGetNewRoleGift(tag_net_message* pCmd)
{
	NET_SIC_get_new_role_gift* p_receive = (NET_SIC_get_new_role_gift*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dwError = pRole->GetNewRoleGift();

	NET_SIS_get_new_role_gift send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

//------------------------------------------------------------------------------
// 开始新手奖励计时
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleBeginRoleGiftTime(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	pRole->m_stNewRoleGift.b_begin_time_ = TRUE;

	return 0;
}

//------------------------------------------------------------------------------
// 领取账号奖励
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleReceiveAccountReward(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;

	if(m_sAccountCommon.n16_receive_type == 0 && m_sAccountCommon.b_receive)
		dw_error = E_Account_Reward_Receive_Limit;

	if(m_sAccountCommon.n16_receive_type != 0 && m_sAccountCommon.b_receive)
		dw_error = E_Account_Reward_Receive;

	if(dw_error == E_Success)
	{
		if(VALID_POINT(pRole->GetScript()))
		{
			pRole->GetScript()->OnReceiveAccountReward(pRole, pMap->get_map_id(), pMap->get_instance_id(), m_sAccountCommon.n16_receive_type);
		}

		m_sAccountCommon.b_receive = TRUE;

		NET_DB2C_update_receive send;
		send.dw_account_id = GetSessionID();
		g_dbSession.Send(&send, send.dw_size);
	}

	NET_SIS_receive_account_reward send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

//------------------------------------------------------------------------------
// 领取账号奖励
//------------------------------------------------------------------------------
DWORD PlayerSession::HandleReceiveAccountRewardEx(tag_net_message* pCmd)
{
	NET_SIC_receive_account_reward_ex* p_recv = (NET_SIC_receive_account_reward_ex*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD	dw_error = E_Success;

	if(m_sAccountCommon.dw_receive_type == 0)
		dw_error = E_Account_Reward_Receive_Limit;

	if(!(m_sAccountCommon.dw_receive_type & p_recv->eReceiveState))
		dw_error = E_Account_Reward_Receive;

	if(dw_error == E_Success)
	{
		if(VALID_POINT(pRole->GetScript()))
		{
			pRole->GetScript()->OnReceiveAccountRewardEx(pRole, pMap->get_map_id(), pMap->get_instance_id(), p_recv->eReceiveState);
		}

		m_sAccountCommon.dw_receive_type ^= p_recv->eReceiveState;

		NET_DB2C_update_receive_ex send;
		send.dw_account_id = GetSessionID();
		send.dw_receive_type = m_sAccountCommon.dw_receive_type;
		g_dbSession.Send(&send, send.dw_size);

		NET_SIS_send_receive_account_reward_ex send_receive;
		send_receive.dw_receive_type = m_sAccountCommon.dw_receive_type;;
		pRole->SendMessage(&send_receive, send_receive.dw_size);
	}

	NET_SIS_receive_account_reward_ex send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);


	return 0;
}

DWORD PlayerSession::HandleReceiveSerialReward(tag_net_message* pCmd)
{
	NET_SIC_receive_serial_reward* p_recv = (NET_SIC_receive_serial_reward*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	NET_DB2C_load_serial_reward send_reward;
	send_reward.dw_role_id = pRole->GetID();
	strncpy(send_reward.sz_serial, p_recv->sz_serial, X_SHORT_NAME);
	g_dbSession.Send(&send_reward, send_reward.dw_size);

	/*string str(p_recv->sz_serial);

	DWORD dw_error = E_Success;

	if(!pRole->is_serial_init())
	dw_error = E_SerialReward_not_init;

	s_serial_reward* p = pRole->get_map_serial_reward().find(str);
	if(!VALID_POINT(p))
	return E_SerialReward_SerialNotExists;

	if(dw_error == E_Success)
	{
	if(pRole->GetScript())
	{
	pRole->GetScript()->OnReceiveSerialReward(pRole, p->n_type);
	}

	NET_DB2C_delete_serial_reward send;
	send.dw_account_id = GetSessionID();
	strncpy(send.sz_serial, p->sz_serial, X_SHORT_NAME);
	g_dbSession.Send(&send, send.dw_size);

	NET_DB2C_log_serial_reward log_send;
	log_send.s_log_serial_reward.dw_account_id = GetSessionID();
	log_send.s_log_serial_reward.n_type = p->n_type;
	strncpy(log_send.s_log_serial_reward.sz_serial, p->sz_serial, X_SHORT_NAME);
	g_dbSession.Send(&log_send, log_send.dw_size);

	pRole->get_map_serial_reward().erase(str);
	SAFE_DELETE(p);
	}

	NET_SIS_receive_serial_reward send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);*/

	return 0;
}

//向验证码服务器索要验证码图片
VOID PlayerSession::SendVerifyCodeMessage()
{
	NET_W2C_verify_code send;
	send.dw_account_id = m_dwAccountID;
	g_center_session.Send(&send, send.dw_size);

	m_bRoleVerifying = true;
}

DWORD PlayerSession::HandleBuyLingqi(tag_net_message* p_cmd)
{
	NET_SIC_buy_lingqi_vaule* p_recv = (NET_SIC_buy_lingqi_vaule*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
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

	NET_SIS_buy_lingqi_vaule send;
	send.dwErrCode = pRole->buyLingqi();
	SendMessage(&send, send.dw_size);

	return 0;
}	

DWORD PlayerSession::HandleHuenLian(tag_net_message* p_cmd)
{
	NET_SIC_LianHun* p_recv = (NET_SIC_LianHun*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_LianHun send;
	send.dw_error = pRole->Huenlian(p_recv->byType);
	send.byCurArtisan = pRole->getCurArtisan();
	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleGetHuenJingData(tag_net_message* p_cmd)
{
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	pRole->send_huenjing_data();

	return 0;

}

DWORD PlayerSession::HandleHuenjingOpteron(tag_net_message* p_cmd)
{
	NET_SIC_huenjing_opteron* p_recv = (NET_SIC_huenjing_opteron*)p_cmd;


	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	NET_SIS_huenjing_opteron send;
	send.dw_error_code = pRole->HuenJingOpt(p_recv->nIndex, p_recv->byOpt, p_recv->byConType);
	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleHuenjingLevelUp(tag_net_message* p_cmd)
{
	NET_SIC_huenjing_UpLevel* p_recv = (NET_SIC_huenjing_UpLevel*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_huenjing_UpLevel send;
	send.dw_error_code = pRole->HuenJingLevelUp(p_recv->nIndex);
	SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleHuenjingInlay(tag_net_message* p_cmd)
{
	NET_SIC_huenjing_Inlay* p_recv = (NET_SIC_huenjing_Inlay*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_huenjing_Inlay send;
	send.dw_error_code = pRole->HuenJingInlay(p_recv->nSrcIndex, p_recv->nDesIndex, p_recv->byType, p_recv->byOptType);
	SendMessage(&send, send.dw_size);

	return 0;
}


DWORD PlayerSession::HandleGetRewardData(tag_net_message* p_cmd)
{
	NET_SIC_get_reward_data* p_recv = (NET_SIC_get_reward_data*)p_cmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	pRole->send_reward_data();

	return 0;
}

DWORD PlayerSession::HandleReceiveReward(tag_net_message* p_cmd)
{
	NET_SIC_receive_reward* p_recv = (NET_SIC_receive_reward*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_receive_reward send;
	send.byType = p_recv->byType;
	send.byIndex = p_recv->byIndex;
	send.dwErrorCode = pRole->receiveRewardItem(p_recv->byType);
	SendMessage(&send, send.dw_size);

	return 0;
}
// 获取开服活动数据
DWORD PlayerSession::HandleGetOpenActiveData(tag_net_message* p_cmd)
{
	NET_SIC_get_server_acitvity* p_recv = (NET_SIC_get_server_acitvity*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_get_server_acitvity send;
	send.nDay = g_world.get_open_server_day() + 1;
	
	if (VALID_POINT(pRole->GetSession()))
	{
		send.dwSelf[0] = RankMgr::GetInstance()->GetRankPosClass(ERT_LEVELRANK, pRole->GetID(), pRole->GetClass());
		send.dwSelf[1] = RankMgr::GetInstance()->GetRankPosClass(ERT_JUSTICERANK, pRole->GetID(), pRole->GetClass());
		send.dwSelf[2] = RankMgr::GetInstance()->GetRankPos(ERT_MOUNTS, pRole->GetID());
		send.dwSelf[3] = 0;
		send.dwSelf[4] = RankMgr::GetInstance()->GetRankPos(ERT_CHONGZHI, pRole->GetSession()->GetSessionID());
		send.dwSelf[5] = RankMgr::GetInstance()->GetRankPos(ERT_SHIHUNRANK, pRole->GetID());
		send.dwSelf[6] = RankMgr::GetInstance()->GetRankPos(ERT_JUSTICERANK, pRole->GetID());
	}

	
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}

// 获取开服活动奖励
DWORD PlayerSession::HandleGetOpenActiveReceive(tag_net_message* p_cmd)
{
	NET_SIC_get_server_acitvity_receive* p_recv = (NET_SIC_get_server_acitvity_receive*)p_cmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dwErrorCode = E_Success;
	INT nDay = g_world.get_open_server_day();
 
	// 奖励类型
	DWORD dwType = 0;

	// 职业
	int nClass = ((int)pRole->GetClass() - 1);

	// 领取标记
	DWORD dwRecv = pRole->GetScriptData(1);
	// 已经领取了
	if (dwRecv & (1 << p_recv->nDay))
	{
		dwErrorCode = E_Open_Server_Has_Recv;
		goto EXIT;
	}

	// 背包空间不足
	//if (pRole->GetItemMgr().GetBagFreeSize() <= 0)
	//{	
	//	dwErrorCode = E_Open_Server_Not_Free_Bag;
	//	goto EXIT;
	//}

	switch (p_recv->nDay)
	{

	// 第一天等级比拼
	case 1:
		{
			// 第6天领取
			if (nDay != 5)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}

			if (RankMgr::GetInstance()->IsClessOne(pRole->GetID(), nClass))
			{
				dwType = 2;
			}
			else if(pRole->get_level() >= 55)
			{
				dwType = 1;
			}
				
		}
		break;
	//第二天职业战力
	case 2:
		{
			// 第3天领取
			if (nDay != 2)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}
			if (RankMgr::GetInstance()->IsZhanLiClassOne(pRole->GetID(), nClass))
			{
				dwType = 2;
			}
			else if ( pRole->st_EquipTeamInfo.n32_Rating > 5000)
			{
				dwType = 1;
			}
		}
		break;
	//第三天坐骑等级
	case 3:
		{
			// 第4天领取
			if (nDay != 3)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}
			if (RankMgr::GetInstance()->IsMountsOne(pRole->GetID()))
			{
				dwType = 2;
			}
			else if (pRole->GetRaidMgr().GetStep() >= 2)
			{
				dwType = 1;
			}
		}
		break;
	// 第四天沙巴克
	case 4:
		{
			// 第5天领取
			if (nDay != 4)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}
			
			if (pRole->GetGuildID() == INVALID_VALUE)
			{
				dwErrorCode = INVALID_VALUE;
				goto EXIT;
			}

			if (g_guild_manager.get_SBK_guild() == pRole->GetGuildID())
			{
				dwType = 1;
				guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
				if ( VALID_POINT(pGuild) && 
					(pGuild->get_guild_att().b_hasOpenServerReceive == 0) &&
					(pGuild->get_guild_att().dwLeaderRoleID == pRole->GetID()) 
					)
				{
					dwType = 2;
					pGuild->set_guild_open_server(1);
				}
				
			}
			
		}
		break;
	// 第五天充值
	case 5:
		{
			// 第6天领取
			if (nDay != 5)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}

			if (RankMgr::GetInstance()->IsChongZhiOne(pRole->GetID()))
			{
				dwType = 2;
			}
			else if (pRole->GetTotalRecharge() >= 2000)
			{
				dwType = 1;
			}
		}
		break;
	// 第六天魅力
	case 6:
		{
			// 第7天领取
			if (nDay != 6)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}

			if (RankMgr::GetInstance()->IsMeiLiOne(pRole->GetID(), pRole->GetSex()))
			{
				dwType = 2;
			}
			else if (pRole->get_shihun() >= 50)
			{
				dwType = 1;
			}

		}
		break;
		// 第7天总战力
	case 7:
		{
			// 第7天领取
			if (nDay != 7)
			{
				dwErrorCode = E_Open_Server_Not_Today;
				goto EXIT;
			}

			if (RankMgr::GetInstance()->IsZhanliOne(pRole->GetID()))
			{
				dwType = 2;
			}
			else if (pRole->st_EquipTeamInfo.n32_Rating >= 10000)
			{
				dwType = 1;
			}

		}
		break;
	}

	// 没有领取资格
	if (dwType == 0)
	{
		dwErrorCode = E_Open_Server_Cant;
		goto EXIT;
	}

	// 设置领取标记
	dwRecv |= (1 << p_recv->nDay);
	pRole->SetScriptData(1, dwRecv);


	// 交与脚本处理奖励
	if(pRole->GetScript())
	{	
		pRole->GetScript()->OnGetOpenReceive(pRole, p_recv->nDay, dwType);
	}

EXIT:
	NET_SIS_get_server_acitvity_receive send;
	send.dw_error_code = dwErrorCode;
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}