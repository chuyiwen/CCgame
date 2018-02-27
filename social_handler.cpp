
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�����ύ����Ϣ������

#include "stdafx.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/group_define.h"
#include "../../common/WorldDefine/SocialDef.h"
#include "../../common/WorldDefine/team_protocol.h"
#include "../../common/WorldDefine/social_protocol.h"

#include "player_session.h"
#include "unit.h"
#include "role.h"
#include "group_mgr.h"
#include "social_mgr.h"

//-----------------------------------------------------------------------------
// ��Ӻ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleMakeFriend(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_role_make_friend* p_receive = (NET_SIC_role_make_friend*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_MakeFriend, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ɾ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleCancelFriend(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_role_cancel_friend* p_receive = (NET_SIC_role_cancel_friend*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_CancelFriend, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ɾ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpdateFriendGroup(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_update_friend_group* p_receive = (NET_SIC_update_friend_group*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_FriendGrp, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ��������������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleMoveBlackList(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_move_to_black_list* p_receive = (NET_SIC_move_to_black_list*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_InsertBlkList, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ������������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleAddEnemy(tag_net_message* pCmd)
{
	if (!VALID_POINT(m_pRole)) return INVALID_VALUE;
	NET_SIC_add_enemy* p_receive = (NET_SIC_add_enemy*)pCmd;
	
	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_AddEmList, pCmd->dw_size, p_receive);

	return 0;
}
//-----------------------------------------------------------------------------
// ������Ƴ�������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDeleteBlackList(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_delete_black_list* p_receive = (NET_SIC_delete_black_list*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_DeleteBlkList, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ������Ƴ������б�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDeleteEnemyList(tag_net_message* pCmd)
{
	if(!VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_delete_enemy_list* p_receive = (NET_SIC_delete_enemy_list*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_DeleteEmList, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ��ȡ����λ��
//-----------------------------------------------------------------------------
DWORD   PlayerSession::HandleGetEnemyPos(tag_net_message* pCmd)
{
	if(!VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_get_enemy_position* p_receive = (NET_SIC_get_enemy_position*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_GetEnemyPos, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSendGift(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_send_gift* p_receive = (NET_SIC_send_gift*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_SendGift, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �յ������ظ�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSendGiftReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_send_gift_reply* p_receive = (NET_SIC_send_gift_reply*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_SendGiftRpy, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �������С��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleApplyJoinTeam(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_apply_join_team* p_receive = (NET_SIC_apply_join_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_ApplyJoin, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �������С�ӷ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleApplyJoinTeamReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_apply_join_team_reply* p_receive = (NET_SIC_apply_join_team_reply*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_ApplyJoinReply, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �Դ�������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleOwnCreateTeam(tag_net_message* pCmd)
{
	if(!VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_own_create_team* p_receive = (NET_SIC_own_create_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_OwnCreate, pCmd->dw_size, p_receive);

	return 0;
}
//-----------------------------------------------------------------------------
// ��ɢ����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDismissTeam(tag_net_message* pCmd)
{
	if(!VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_dismiss_team* p_receive = (NET_SIC_dismiss_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_Dismiss, pCmd->dw_size, p_receive);
	
	return 0;
}
//-----------------------------------------------------------------------------
// ��������б�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCleanApply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_clean_apply* p_receive = (NET_SIC_clean_apply*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_CleanApply, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ��Ա������Ҽ���С��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleMemInviteJoinTeam(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_member_invite_join_team* p_receive = (NET_SIC_member_invite_join_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_MemJoinTeam, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ��Ա������Ҽ���С�ӻظ�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleMemInviteJoinTeamReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_member_invite_join_team_replay* p_receive = (NET_SIC_member_invite_join_team_replay*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_MemJoinTeamReply, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �ı���鹫��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleChangeTeamPlacard(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_change_team_placard* p_receive = (NET_SIC_change_team_placard*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_ChangeTeamPlacard, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ȡ�����ڵ�ͼ������С����Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetMapTeamInfo(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole)) return INVALID_VALUE;

	NET_SIC_get_map_team_info* p_receive = (NET_SIC_get_map_team_info*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_GetMapTeamInfo, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ������Ҽ���С��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleJoinTeam(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_invite_join_team* p_receive = (NET_SIC_invite_join_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_JoinTeam, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ��������Ƿ�ͬ�����С��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleJoinTeamReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_invite_reply* p_receive = (NET_SIC_invite_reply*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_JoinTeamRepley, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ����ҳ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleKickMember(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_kick_member* p_receive = (NET_SIC_kick_member*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_KickMember, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ����뿪С��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLeaveTeam(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_leave_team* p_receive = (NET_SIC_leave_team*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_LeaveTeam, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// ����С��ʰȡģʽ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetPickMol(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;

	NET_SIC_set_pick_mode* p_receive = (NET_SIC_set_pick_mode*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_SetPickMol, pCmd->dw_size, p_receive);

	return 0;
}

//-----------------------------------------------------------------------------
// �ı�С�Ӷӳ�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleChangeLeader(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;

	NET_SIC_change_leader* p_receive = (NET_SIC_change_leader*)pCmd;

	g_groupMgr.AddEvent(m_pRole->GetID(), EVT_ChangeLeader, pCmd->dw_size, p_receive);

	return 0;
}


//-----------------------------------------------------------------------------
// ͬ����������Ƿ�����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpdateFriOnline(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;

	NET_SIC_update_friend_state* p_receive = (NET_SIC_update_friend_state*)pCmd;

	m_pRole->UpdateFriOnline();
	return 0;
}


//--------------------------------------------------------------------
// ���÷�����Ʒ�ȼ�
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRoleSetAssignQuality( tag_net_message* pCmd )
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;
	g_groupMgr.AddEvent(m_pRole->GetID( ), EVT_SetAssignQuality, pCmd->dw_size, pCmd );
	return 0;
}
//--------------------------------------------------------------------
// �ӳ�����
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRoleLeaderAssign( tag_net_message* pCmd )
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;
	g_groupMgr.AddEvent(m_pRole->GetID( ), EVT_LeaderAssign, pCmd->dw_size, pCmd );
	return 0;
}
//--------------------------------------------------------------------
// ������
//--------------------------------------------------------------------
DWORD PlayerSession::HandleRoleTeamAssignSice( tag_net_message* pCmd )
{
	if( !VALID_POINT(m_pRole))	return INVALID_VALUE;
	g_groupMgr.AddEvent(m_pRole->GetID( ), EVT_TeamMemberSice, pCmd->dw_size, pCmd );
	return 0;
}

//----------------------------------------------------------------------
//mwh 2011-09-06
//----------------------------------------------------------------------
DWORD PlayerSession::HandleRoleTeamSetShareLeaderCircleQuest( tag_net_message* pCmd )
{
	if(!VALID_POINT(m_pRole))	return INVALID_VALUE;
	g_groupMgr.AddEvent(m_pRole->GetID( ), EVT_SetShareLeaderCircleQuest, pCmd->dw_size, pCmd );
	return 0;
}

//-----------------------------------------------------------------------------
// ������ҽ���˫��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleInviteRoleComPractice(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_invite_practice* p_receive = (NET_SIC_invite_practice*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_ComPractice, pCmd->dw_size, p_receive);

	return 0;
}
DWORD PlayerSession::HandleInviteRoleCompracticeReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_invite_practice_reply* p_receive = (NET_SIC_invite_practice_reply*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_ComPracticeReply, pCmd->dw_size, p_receive);

	return 0;
}
DWORD PlayerSession::HandleRoleCancelCompractice(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_cancel_practice* p_receive = (NET_SIC_cancel_practice*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_CancelPractice, pCmd->dw_size, p_receive);

	return 0;
}
//-----------------------------------------------------------------------------
// ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleMalePropose(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_male_propose* p_receive = (NET_SIC_male_propose*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_Propose, pCmd->dw_size, p_receive);

	return 0;
}
DWORD PlayerSession::HandleProposeFemaleReply(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_propose_reply* p_receive = (NET_SIC_propose_reply*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_ProposeReply, pCmd->dw_size, p_receive);

	return 0;
}
DWORD PlayerSession::HandleRoleDivorce(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_role_divorce* p_receive = (NET_SIC_role_divorce*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_Divorce, pCmd->dw_size, p_receive);

	return 0;
}
DWORD PlayerSession::HandleGetQbjjReward(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_SIC_get_qbjj_reward* p_receive = (NET_SIC_get_qbjj_reward*)pCmd;

	g_socialMgr.AddEvent(m_pRole->GetID(), EVT_QbjjReward, pCmd->dw_size, p_receive);

	return 0;
}