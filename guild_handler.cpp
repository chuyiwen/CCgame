
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//���ɴ���
#include "StdAfx.h"

#include "player_session.h"
#include "role.h"
#include "role_mgr.h"
#include "guild.h"
#include "guild_manager.h"
#include "guild_chamber.h"
#include "../../common/WorldDefine/guild_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "map_creator.h"
//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCreateGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_create);
	NET_SIC_guild_create * p_receive = ( NET_SIC_guild_create * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = INVALID_VALUE;

	// NPC���
	//dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Guild);
	//if(E_Success == dw_error_code)
	//{
		INT32 nNameTCHAR = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_guild_create, szGuildName)) / sizeof(TCHAR);
		dw_error_code = g_guild_manager.create_guild(pRole, p_receive->szGuildName, nNameTCHAR, p_receive->n64ItemID);
	//}

	// ��ͻ��˷���
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ɢ����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDismissGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_dismiss);
	NET_SIC_guild_dismiss * p_receive = ( NET_SIC_guild_dismiss * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = INVALID_VALUE;

	// NPC���
	dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Guild);
	if(E_Success == dw_error_code)
	{
		dw_error_code = g_guild_manager.dismiss_guild(pRole, pRole->GetGuildID());
	}

	// ��ͻ��˷���
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleJoinGuildReq(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_join_request);
	NET_SIC_guild_join_request * p_receive = ( NET_SIC_guild_join_request * ) pCmd ;  	
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �Ƿ���Լ�����
	if(pRole->GetID() == p_receive->dwDstRoleID)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	Role *pInvitee = NULL;
	DWORD dw_error_code = pGuild->can_invite_join(pRole->GetID(), p_receive->dwDstRoleID, &pInvitee);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		ASSERT(VALID_POINT(pInvitee));

		NET_SIS_guild_join_request send;
		send.dwSrcRoleID	= pRole->GetID();
		send.dwGuildID		= pRole->GetGuildID();
		pInvitee->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �������,��RoleName
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleJoinGuildReqByName(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_join_request_by_name);

	NET_SIC_guild_join_request_by_name * p_receive = ( NET_SIC_guild_join_request_by_name * ) pCmd ;  
	DWORD dw_error_code = INVALID_VALUE;
	DWORD dwInviteeRoleID = g_roleMgr.get_role_id(p_receive->dwNameCrc);
	if(INVALID_VALUE ==dwInviteeRoleID )
	{
		dw_error_code = E_Guild_RoleNotFind;
		g_guild_manager.send_guild_failed_to_client(this,dw_error_code );
		return dw_error_code;
	}

	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �Ƿ���Լ�����
	if(pRole->GetID() == dwInviteeRoleID)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	Role *pInvitee = NULL;
	dw_error_code = pGuild->can_invite_join(pRole->GetID(), dwInviteeRoleID, &pInvitee,TRUE);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		ASSERT(VALID_POINT(pInvitee));

		NET_SIS_guild_join_request send;
		send.dwSrcRoleID	= pRole->GetID();
		send.dwGuildID		= pRole->GetGuildID();
		pInvitee->SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}


//-----------------------------------------------------------------------------
// ��������ҷ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleJoinGuildReqRes(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_join_request_result);
	NET_SIC_guild_join_request_result * p_receive = ( NET_SIC_guild_join_request_result * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(p_receive->dwDstGuildID);
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	if(p_receive->dw_error_code != E_Success)
	{
		// �������������Ϣ
		pGuild->erase_invite_join(pRole->GetID());	

		// ����������Ƿ�������
		Role *pInviter = g_roleMgr.get_role(p_receive->dwDstRoleID);
		if(VALID_POINT(pInviter))
		{
			// �������߷���Ϣ
			NET_SIS_guild_join_request_result	send;
			send.dw_error_code = E_Guild_Join_BeRefused;
			send.dwInviterID = p_receive->dwDstRoleID;
			send.dwInviteeID = pRole->GetID();
			pInviter->SendMessage(&send, send.dw_size);
		}

		return E_Success;
	}

	// ����
	DWORD dw_error_code = pGuild->invite_join(p_receive->dwDstRoleID, pRole->GetID());
	if(dw_error_code != E_Success)
	{		
		// ��˫������Ϣ
		NET_SIS_guild_join_request_result	send;
		send.dw_error_code = dw_error_code;
		send.dwInviterID = p_receive->dwDstRoleID;
		send.dwInviteeID = pRole->GetID();
		SendMessage(&send, send.dw_size);

		// ����������Ƿ�������
		Role *pInviter = g_roleMgr.get_role(p_receive->dwDstRoleID);
		if(VALID_POINT(pInviter))
		{
			pInviter->SendMessage(&send, send.dw_size);
		}
	}
	else
	{
		// ���ɹ㲥
		s_role_info *pRoleInfo = g_roleMgr.get_role_info(pRole->GetID());
		if (VALID_POINT(pRoleInfo))
		{
			INT32 nRoleNameCnt = _tcsclen(pRoleInfo->sz_role_name);
			INT32 nMsgSz = sizeof(NET_SIS_guild_join_broad) + nRoleNameCnt * sizeof(TCHAR);

			CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_join_broad);
			pSend->dw_role_id		= pRole->GetID();
			pSend->dwRoleNameID	= pRole->GetNameID();

			memcpy(pSend->szRoleName, pRoleInfo->sz_role_name, (nRoleNameCnt + 1) * sizeof(TCHAR));

			pGuild->send_guild_message(pSend, pSend->dw_size);
			MDEL_MSG(pSend);
		}

		// ͬ������Χ���
		tagGuildMember* pMember = pGuild->get_member(pRole->GetID());
		Map*			pMap	= pRole->get_map();
		if (VALID_POINT(pMember) && VALID_POINT(pMap))
		{
			NET_SIS_remote_role_guild_info_change send;
			send.dwGuildID		= p_receive->dwDstGuildID;
			send.dw_role_id		= pRole->GetID();
			send.n8GuildPos		= pMember->eGuildPos;

			pMap->send_big_visible_tile_message(pRole, &send, send.dw_size);
		}

		//������ҽű�
		if(VALID_POINT( pRole->GetScript( ) ) ) 
		{
			pRole->GetScript( )->OnJoinGuild( pRole, p_receive->dwDstGuildID );
		}
	
		pRole->GetAchievementMgr().UpdateAchievementCriteria(ete_join_guild, 1);

		// �����������
		if(g_guild_manager.is_have_recruit(pRole->GetID()))
		{
			g_guild_manager.delete_role_from_guild_recruit(pRole->GetID());
			NET_C2DB_delete_guild_recruit send;
			send.dw_role_id = pRole->GetID();
			g_dbSession.Send(&send, send.dw_size);

			Role *pInviter = g_roleMgr.get_role(p_receive->dwDstRoleID);
			if(VALID_POINT(pInviter))
			{
				g_guild_manager.send_query_result(pInviter, 0, pInviter->get_list_guild_recruit());
			}
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �뿪����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleLeaveGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_leave);
	NET_SIC_guild_leave * p_receive = ( NET_SIC_guild_leave * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->leave_guild(pRole);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		// ���ɹ㲥
		NET_SIS_guild_leave_broad send;
		send.dwSrcRoleID = pRole->GetID();
		pGuild->send_guild_message(&send, send.dw_size);

		// �뿪��
		SendMessage(&send, send.dw_size);

		// ͬ������Χ���
		Map* pMap	= pRole->get_map();
		if (VALID_POINT(pMap))
		{
			NET_SIS_remote_role_guild_info_change send_broad;
			send_broad.dwGuildID	= INVALID_VALUE;
			send_broad.dw_role_id		= pRole->GetID();
			send_broad.n8GuildPos	= EGMP_Null;

			pMap->send_big_visible_tile_message(pRole, &send_broad, send_broad.dw_size);
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �Ӱ������߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleKickFromGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_kick);
	NET_SIC_guild_kick * p_receive = ( NET_SIC_guild_kick * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �Ƿ���Լ�����
	if(pRole->GetID() == p_receive->dw_role_id)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->kick_member(pRole->GetID(), p_receive->dw_role_id);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_guild_kick_broad send;
		send.dwDstRoleID = p_receive->dw_role_id;
		pGuild->send_guild_message(&send, send.dw_size);

		// ��鱻�����Ƿ�����
		Role *pKicked = g_roleMgr.get_role(p_receive->dw_role_id);
		if(VALID_POINT(pKicked))
		{
			pKicked->SendMessage(&send, send.dw_size);

			// ͬ������Χ���
			Map* pMap	= pKicked->get_map();
			if (VALID_POINT(pMap))
			{
				NET_SIS_remote_role_guild_info_change send_broad;
				send_broad.dwGuildID	= INVALID_VALUE;
				send_broad.dw_role_id		= pKicked->GetID();
				send_broad.n8GuildPos	= EGMP_Null;

				pMap->send_big_visible_tile_message(pKicked, &send_broad, send_broad.dw_size);
			}
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �ƽ�����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleTurnoverGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_trunover);
	NET_SIC_guild_trunover * p_receive = ( NET_SIC_guild_trunover * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	
	//if(!pRole->get_check_safe_code())
	//{
	//	if(GetBagPsd() != p_receive->dw_safe_code)
	//	{

	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = FALSE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		return INVALID_VALUE;
	//	}

	//	else 
	//	{
	//		NET_SIS_code_check_ok send_check;
	//		send_check.bSuccess = TRUE;
	//		pRole->SendMessage(&send_check, send_check.dw_size);

	//		pRole->set_check_safe_code();
	//	}
	//}

	// �Ƿ���Լ�����
	if(pRole->GetID() == p_receive->dw_role_id)
	{
		return INVALID_VALUE;
	}

	Role* pNewLeader = g_roleMgr.get_role(p_receive->dw_role_id);
	if(!VALID_POINT(pNewLeader))
		return INVALID_VALUE;

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->change_leader(pRole->GetID(), p_receive->dw_role_id);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_guild_trunover_broad send;
		send.dwSrcRoleID	= pRole->GetID();
		send.dwDstRoleID	= p_receive->dw_role_id;
		pGuild->send_guild_message(&send, send.dw_size);

		// ͬ������Χ���(ԭ�������Ȱ���)
		tagGuildMember* pMember = pGuild->get_member(pRole->GetID());
		Map*			pMap	= pRole->get_map();
		if (VALID_POINT(pMember) && VALID_POINT(pMap))
		{
			NET_SIS_remote_role_guild_info_change send_broad;
			send_broad.dwGuildID	= pRole->GetGuildID();
			send_broad.dw_role_id		= pRole->GetID();
			send_broad.n8GuildPos	= pMember->eGuildPos;

			pMap->send_big_visible_tile_message(pRole, &send_broad, send_broad.dw_size);
		}

		// �ж��Ȱ����Ƿ�����
		Role* pNewLeader = g_roleMgr.get_role(p_receive->dw_role_id);
		if (VALID_POINT(pNewLeader))
		{
			pMember = pGuild->get_member(pNewLeader->GetID());
			pMap	= pNewLeader->get_map();
			if (VALID_POINT(pMember) && VALID_POINT(pMap))
			{
				NET_SIS_remote_role_guild_info_change send_broad;
				send_broad.dwGuildID	= pNewLeader->GetGuildID();
				send_broad.dw_role_id		= pNewLeader->GetID();
				send_broad.n8GuildPos	= pMember->eGuildPos;

				pMap->send_big_visible_tile_message(pNewLeader, &send_broad, send_broad.dw_size);
			}

			if(VALID_POINT(pNewLeader->GetScript()))
			{
				pNewLeader->GetScript()->OnGuildTrunOver(pRole, pNewLeader);
			}
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ְ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDemissFromGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_demiss);
	NET_SIC_guild_demiss * p_receive = ( NET_SIC_guild_demiss * ) pCmd ;  
	// ��ȡ��ɫ	
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	INT8 n8OldGuildPos = EGMP_BangZhong;
	DWORD dw_error_code = pGuild->demiss_postion(pRole->GetID(), n8OldGuildPos);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_guild_demiss_broad send;
		send.dw_role_id		= pRole->GetID();
		send.n8OldGuildPos	= n8OldGuildPos;
		pGuild->send_guild_message(&send, send.dw_size);

		// ͬ������Χ���
		Map* pMap	= pRole->get_map();
		if (VALID_POINT(pMap))
		{
			NET_SIS_remote_role_guild_info_change send_broad;
			send_broad.dwGuildID	= pRole->GetGuildID();
			send_broad.dw_role_id		= pRole->GetID();
			send_broad.n8GuildPos	= n8OldGuildPos;

			pMap->send_big_visible_tile_message(pRole, &send_broad, send_broad.dw_size);
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ����ְλ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleAppointForGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_appoint);
	NET_SIC_guild_appoint * p_receive = ( NET_SIC_guild_appoint * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pAppointor = GetRole();
	if(!VALID_POINT(pAppointor))
	{
		return INVALID_VALUE;
	}

	// �Ƿ���Լ�����
	if(pAppointor->GetID() == p_receive->dw_role_id)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pAppointor->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->appoint(pAppointor->GetID(), p_receive->dw_role_id, p_receive->ePos);
	if(dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_guild_appoint_broad send;
		send.dwSrcRoleID	= pAppointor->GetID();
		send.dwDstRoleID	= p_receive->dw_role_id;
		send.ePos			= p_receive->ePos;
		pGuild->send_guild_message(&send, send.dw_size);

		// ͬ������Χ���
		Role* pDstRole = g_roleMgr.get_role(p_receive->dw_role_id);
		if( VALID_POINT(pDstRole) && VALID_POINT(pDstRole->get_map()) )
		{
			NET_SIS_remote_role_guild_info_change send_broad;
			send_broad.dwGuildID	= pDstRole->GetGuildID();
			send_broad.dw_role_id		= p_receive->dw_role_id;
			send_broad.n8GuildPos	= p_receive->ePos;

			pDstRole->get_map()->send_big_visible_tile_message(pDstRole, &send_broad, send_broad.dw_size);
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �޸İ�����ּ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleChangeGuildTenet(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_tenet_change);
	NET_SIC_guild_tenet_change * p_receive = ( NET_SIC_guild_tenet_change * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	INT32 nTenetCnt = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_guild_tenet_change, szTenet)) / sizeof(TCHAR);
	DWORD dw_error_code = pGuild->modify_tenet(pRole, p_receive->szTenet, nTenetCnt);
	if(dw_error_code != E_Success)
	{
		// �ж��Ƿ���Ҫ����
		if (dw_error_code != E_Return_NotNeedFeed)
		{
			g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
		}
	}
	else
	{
		INT32 nMsgSz = sizeof(NET_SIS_guild_tenet_change_broad) + nTenetCnt * sizeof(TCHAR);
		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_tenet_change_broad);
		pSend->dw_role_id	= pRole->GetID();
		memcpy(pSend->szTenet, p_receive->szTenet, (nTenetCnt + 1) * sizeof(TCHAR));
		pSend->szTenet[nTenetCnt] = _T('\0');
		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �޸İ��ɱ�־
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleChangeGuildSymbol(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_symbol);
	NET_SIC_guild_symbol * p_receive = ( NET_SIC_guild_symbol * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	INT32 nSymbolCnt = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_guild_symbol, szText)) / sizeof(TCHAR);
	DWORD	dwError = pGuild->modify_symbol(pRole->GetID(), p_receive->szText, nSymbolCnt);

	if(dwError != E_Success)
	{
		// �ж��Ƿ���Ҫ����
		if (dwError != E_Return_NotNeedFeed)
		{
			g_guild_manager.send_guild_failed_to_client(this, dwError);
		}
	}
	else
	{
		//NET_SIS_guild_symbol_broad send;
		//send.dw_role_id = pRole->GetID();
		//send.dwValue = p_receive->dwValue;
		//memcpy(send.szText, p_receive->szText, sizeof(send.szText));
		//pGuild->send_guild_message(&send, send.dw_size);
		/*INT32 nMsgSz = sizeof(NET_SIS_guild_symbol_broad) + (nSymbolCnt-1) * sizeof(BYTE);
		MCREATE_MSG(pSend, nMsgSz, NET_SIS_guild_symbol_broad);
		pSend->dw_role_id	= pRole->GetID();
		pSend->dwValue = p_receive->dwValue;
		memcpy(pSend->bySymbol, p_receive->bySymbol, nSymbolCnt * sizeof(BYTE));
		pSend->nSize = nSymbolCnt;
		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);*/
		INT32 nMsgSz = sizeof(NET_SIS_guild_symbol_broad) + nSymbolCnt * sizeof(TCHAR);
		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_symbol_broad);
		pSend->dw_role_id	= pRole->GetID();
		memcpy(pSend->szText, p_receive->szText, (nSymbolCnt + 1) * sizeof(TCHAR));
		pSend->szText[nSymbolCnt] = _T('\0');
		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);

	}

	return dwError;
}

//-----------------------------------------------------------------------------
// �޸ļ���ְλ����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildPosNameChange(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_position_name_change);
	NET_SIC_guild_position_name_change * p_receive = ( NET_SIC_guild_position_name_change * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->modify_postion_name(pRole->GetID(), &p_receive->szPosName[0][0]);
	if(dw_error_code != E_Success)
	{
		// �ж��Ƿ���Ҫ����
		if (dw_error_code != E_Return_NotNeedFeed)
		{
			g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
		}
	}
	else
	{
		INT32 nMsgSz = sizeof(NET_SIS_guild_position_name_change);
		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_position_name_change);
		pSend->dwGuildID	= pRole->GetGuildID();
		memcpy(pSend->szPosName, p_receive->szPosName, sizeof(pSend->szPosName));
		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	return dw_error_code;
}


//-----------------------------------------------------------------------------
// �޸ļ���ְλȨ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildPosPowerChange(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_position_power_change);
	NET_SIC_guild_position_power_change * p_receive = ( NET_SIC_guild_position_power_change * ) pCmd ;  
	// ��ȡ��ɫ
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// ����
	DWORD dw_error_code = pGuild->modify_postion_power(pRole->GetID(), &p_receive->dwPosPower[0]);
	if(dw_error_code != E_Success)
	{
		// �ж��Ƿ���Ҫ����
		if (dw_error_code != E_Return_NotNeedFeed)
		{
			g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
		}
	}
	else
	{
		INT32 nMsgSz = sizeof(NET_SIS_guild_position_power_change_broad);
		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_position_power_change_broad);
		pSend->dwGuildID	= pRole->GetGuildID();
		memcpy(&pSend->dwPosPower[0], &p_receive->dwPosPower[0], sizeof(pSend->dwPosPower));
		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ������ɵ�ͼ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleEnterGuildMap(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_enter_guild_map);
	NET_SIC_enter_guild_map * p_receive = ( NET_SIC_enter_guild_map * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;
	
	INT nRet = E_Success;
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(VALID_POINT(pGuild))
	{
		//if(pGuild->get_guild_war().get_guild_war_state() == EGWS_WAR)
		//{
		//	nRet = E_Instance_Act_NoBegin;
		//}
		if( E_Success != nRet )		// ���������������룬���͸������Ϣ
		{
			NET_SIS_enter_instance send;
			send.dwTimeLimit = INVALID_VALUE;
			send.dw_error_code = nRet;

			pRole->SendMessage(&send, send.dw_size);

			return INVALID_VALUE;
		}
	}
	tagItem* pItem = NULL;

	if(E_Success != pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildMgr))
	{
		pItem = pRole->GetItemMgr().GetBagItem(p_receive->n64_ItemSerial);
		if(!VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EIST_GuildTrans)
			return INVALID_VALUE;

	}

	DWORD dwGuildMapID = get_tool()->crc32(szGuildMapName);
	const tagInstance* pInstance = AttRes::GetInstance()->get_instance_proto(dwGuildMapID);
	if(!VALID_POINT(pInstance))
		return INVALID_VALUE;

	// �õ�Ŀ���ͼ�ĵ�����
	const tag_map_info* pMapInfo = g_mapCreator.get_map_info(dwGuildMapID);
	if( !VALID_POINT(pMapInfo) ) return INVALID_VALUE;

	const tag_map_way_point_info_list* pList = NULL;

	if(p_receive->dwParam == 1)
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnemyEnterPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}
	else
	{
		pList = pMapInfo->map_way_point_list.find(pInstance->dwEnterWayPoint);
		if( !VALID_POINT(pList) ) return INVALID_VALUE;
	}

	// ��Ŀ�굼�����б�����ȡһ��������
	tag_way_point_info wayPoint;
	pList->list.rand_find(wayPoint);

	Vector3 vFace;
	vFace.y = 0;
	vFace.x = cosf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);
	vFace.z = sinf((270-wayPoint.f_yaw) * 3.1415927f / 180.0f);

	pRole->GotoNewMap(dwGuildMapID, wayPoint.v_pos.x, wayPoint.v_pos.y, wayPoint.v_pos.z, vFace.x, vFace.y, vFace.z, p_receive->dwParam);

	if(VALID_POINT(pItem))
	{
		pRole->GetItemMgr().ItemUsedFromBag(p_receive->n64_ItemSerial, 1, elcid_guild_trans);
	}
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// ���õж԰��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSetEnemyGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_set_enemy_guild);
	NET_SIC_set_enemy_guild * p_receive = ( NET_SIC_set_enemy_guild * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->set_enemy_guild(pRole, p_receive->dwEnemyGuildID);

	NET_SIS_set_enemy_guild send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ɾ���ж԰��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDelEnemyGuild(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_delete_enemy_guild);
	NET_SIC_delete_enemy_guild * p_receive = ( NET_SIC_delete_enemy_guild * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;	

	DWORD dwError = pGuild->delete_enemy_guild(pRole, p_receive->dwEnemyGuildID);

	NET_SIS_delete_enemy_guild send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ��ȡ�ж԰������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetEnemyData(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_enemy_data);
	NET_SIC_get_enemy_data * p_receive = ( NET_SIC_get_enemy_data * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->get_enemy_data(pRole);

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����ս
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildDeclareWar(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_declare_war);
	NET_SIC_guild_declare_war * p_receive = ( NET_SIC_guild_declare_war * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->declare_war(pRole, p_receive->dwGuildID);

	NET_SIS_guild_declare_war send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����ս�ظ�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildDeclareWarRes(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_declare_war_result);
	NET_SIC_guild_declare_war_result * p_receive = ( NET_SIC_guild_declare_war_result * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->declare_war_res(pRole, p_receive->dwGuildID, p_receive->bAccept);

	NET_SIS_guild_declare_war_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}
//������ս�ʸ�
DWORD PlayerSession::HandleGuildQualifyWar(tag_net_message* pCmd)
{
	NET_SIC_guild_war_qualify * p_receive = ( NET_SIC_guild_war_qualify * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;
	
	DWORD dwError = pGuild->guild_qualify_war(pRole, p_receive->bJoin);

	NET_SIS_guild_war_qualify send;
	send.dw_error_code = dwError;
	pRole->SendMessage(&send, send.dw_size);
	
	if (dwError == E_Success)
	{
		NET_SIS_guild_war_member_change send;
		send.dw_role_id = pRole->GetID();
		send.m_bAdd = p_receive->bJoin;
		pGuild->send_guild_message(&send, send.dw_size);
	}

	return dwError;
}
//ȡ����Ա���ս�ʸ�
DWORD PlayerSession::HandleGuildDisqualifyWar(tag_net_message* pCmd)
{
	NET_SIC_guild_war_dis_qualify * p_receive = ( NET_SIC_guild_war_dis_qualify * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	//Role* pmember = g_roleMgr.get_role(p_receive->dw_role_id);
	//if (!VALID_POINT(pRole))
	//	return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->guild_qualify_war_dis(pRole, p_receive->dw_role_id);

	NET_SIS_guild_war_dis_qualify send;
	send.dw_error_code = dwError;
	pRole->SendMessage(&send, send.dw_size);
	
	if (dwError == E_Success)
	{
		NET_SIS_guild_war_member_change send;
		send.dw_role_id = p_receive->dw_role_id;
		send.m_bAdd = FALSE;
		pGuild->send_guild_message(&send, send.dw_size);
	}

	return dwError;
}

// ��������ս����
DWORD PlayerSession::HandleGuildWarNum(tag_net_message* pCmd)
{
	NET_SIC_guild_war_num * p_receive = ( NET_SIC_guild_war_num * ) pCmd ;  

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = E_Success;
	if (pGuild->get_guild_att().dwLeaderRoleID != pRole->GetID())
	{
		dwError = E_Guild_War_Not_Leader;
		NET_SIS_guild_war_num send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return E_Guild_War_Not_Leader;
	}
	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_Prepare)
	{
		dwError = E_Guild_War_Not_Relay;
		NET_SIS_guild_war_num send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return E_Guild_War_Not_Relay;
	}
	if (p_receive->dwNum != 10 && p_receive->dwNum != 20 && p_receive->dwNum != 40 && p_receive->dwNum != 60 && p_receive->dwNum != 80)
	{
		dwError = E_Guild_war_Num_Cant;
		NET_SIS_guild_war_num send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return E_Guild_war_Num_Cant;
	}

	if (p_receive->dwNum < pGuild->get_guild_war().m_dw_baoming_num)
	{
		dwError = E_Guild_BaoMin_Full;
		NET_SIS_guild_war_num send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return E_Guild_war_Num_Cant;
	}

	if (dwError == E_Success)
	{
		pGuild->get_guild_war().set_max_number(p_receive->dwNum);

		NET_SIS_guild_war_maxnum_change send;
		send.dwNumber = p_receive->dwNum;
		pGuild->send_guild_message(&send, send.dw_size);
	}
	
	
	NET_SIS_guild_war_num send;
	send.dw_error_code = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// ���÷���ϵ��
DWORD PlayerSession::HandleGuildWarMoneyParam(tag_net_message* pCmd)
{
	NET_SIC_guild_war_money_param * p_receive = ( NET_SIC_guild_war_money_param * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;
	
	DWORD dwError = E_Success;
	if (pGuild->get_guild_att().dwLeaderRoleID != pRole->GetID())
	{
		dwError = E_Guild_War_Not_Leader;
		goto Exit;
	}
	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_Prepare)
	{
		dwError = E_Guild_War_Not_Relay;
		goto Exit;
	}
	// ֻ����0 0.2 0.4 0.6 0.8
	if ((p_receive->fParam > 0.01 && p_receive->fParam < 0.19) || (p_receive->fParam > 0.21 && p_receive->fParam < 0.39)
		|| (p_receive->fParam > 0.41 && p_receive->fParam < 0.59) 
		|| (p_receive->fParam > 0.61 && p_receive->fParam < 0.79)
		|| p_receive->fParam > 0.81)
	{
		dwError = E_Guild_war_Num_Cant;
		goto Exit;
	}
	
	if (dwError == E_Success)
	{
		pGuild->get_guild_war().set_param(p_receive->fParam);
		
		NET_SIS_guild_war_money_param_change send;
		send.fParam = p_receive->fParam;
		pGuild->send_guild_message(&send, send.dw_size);

	}
	
Exit:
	NET_SIS_guild_war_money_param send;
	send.dw_error_code = dwError;
	pRole->SendMessage(&send, send.dw_size);


}

DWORD PlayerSession::HandleGuildWarRelay(tag_net_message* pCmd)
{
	NET_SIC_Guild_War_Relay* p_receive = (NET_SIC_Guild_War_Relay*)pCmd;
	
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = E_Success;
	if (pGuild->get_guild_att().dwLeaderRoleID != pRole->GetID())
	{
		dwError = E_Guild_War_Not_Leader;
		NET_SIS_Guild_War_Relay send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return dwError ;
	}
	if(pGuild->get_guild_war().get_guild_war_state() != EGWS_Prepare)
	{
		dwError = E_Guild_War_Not_Relay;
		NET_SIS_Guild_War_Relay send;
		send.dw_error_code = dwError;
		pRole->SendMessage(&send, send.dw_size);
		return dwError ;
	}

	if (dwError == E_Success)
	{
		pGuild->get_guild_war().setRelay(TRUE);
		NET_SIS_Guild_War_Relay send;
		send.dw_error_code = dwError;
		pGuild->send_guild_message(&send, send.dw_size);

		DWORD dwEnemyGuild = pGuild->get_guild_war().get_war_guild_id();
		guild* pEnemyGuild = g_guild_manager.get_guild(dwEnemyGuild);
		if (VALID_POINT(pEnemyGuild))
		{
			Role* pEnemyLeader = g_roleMgr.get_role(pEnemyGuild->get_guild_att().dwLeaderRoleID);
			if (VALID_POINT(pEnemyLeader))
			{
				NET_SIS_Guild_War_Relay send2;
				send2.dw_error_code = dwError;
				send2.bEenemy = TRUE;
				pEnemyLeader->SendMessage(&send2, send2.dw_size);
			}
		}

	}

}

// �����ʽ�
DWORD PlayerSession::HandleGuildIncreaseFund(tag_net_message* pCmd)
{
	NET_SIC_Guild_increase_fund* p_receive = (NET_SIC_Guild_increase_fund*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = E_Success;
	if (pGuild->get_guild_att().dwLeaderRoleID != pRole->GetID())
	{
		dwError = E_Guild_War_Not_Leader;
		goto Exit;
	}
	if (pRole->GetCurMgr().GetBagSilver() < p_receive->nFund)
	{
		dwError = E_Guild_Role_Silver_Not_Enough;
		goto Exit;
	}
	INT32 n_max_fund = pGuild->get_upgrade().GetMaxFund();
	
	if (pGuild->get_guild_att().nFund + p_receive->nFund > n_max_fund)
	{
		dwError = E_Guild_Role_Silver_Full;
		goto Exit;
	}

	if (dwError == E_Success)
	{
		pRole->GetCurMgr().DecBagSilver(p_receive->nFund, elcid_guild_inc_fund);
		pGuild->increase_guild_fund(pRole->GetID(), p_receive->nFund, elcid_guild_inc_fund);
	}

Exit:
	NET_SIS_Guild_increase_fund send;
	send.dw_error_code = dwError;
	send.nCurFund = pGuild->get_guild_att().nFund;
	pRole->SendMessage(&send, send.dw_size);
	
	return dwError;
}

DWORD PlayerSession::HandleGuildMaterialReceive(tag_net_message* pCmd)
{
	NET_SIC_Material_receive* p_receive = (NET_SIC_Material_receive*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	NET_SIS_Material_receive send;
	send.dw_error_code = pGuild->materialReceive(pRole, p_receive->dw_item_id, p_receive->n_number);
	pRole->SendMessage(&send, send.dw_size);
	
	return 0;
}
DWORD PlayerSession::HandleGuilddonateFund(tag_net_message* pCmd)
{
	NET_SIC_Guild_donate_fund* p_receive = (NET_SIC_Guild_donate_fund*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;
	
	NET_SIS_Guild_donate_fund send;
	send.dw_error_code = pGuild->donateFund(pRole, p_receive->byType);
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleGetAllGuildInfo(tag_net_message* pCmd)
{
	NET_SIC_get_all_guild_info* p_receive = (NET_SIC_get_all_guild_info*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	g_guild_manager.send_all_guild_info(pRole, p_receive->bSBKList);

	NET_SIS_join_guild_recruit send;
	send.dw_guild_id = g_guild_manager.get_recruit_guild(pRole->GetID());
	send.dw_error = E_Success;
	pRole->SendMessage(&send, send.dw_size);

}

DWORD PlayerSession::HandleGuildMianzhan(tag_net_message* pCmd)
{
	NET_SIC_guild_mianzhan* p_receive = (NET_SIC_guild_mianzhan*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	NET_SIS_guild_mianzhan send;
	send.dw_error_code = pGuild->useMianzhan(pRole, p_receive->n64ItemID);
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}

DWORD PlayerSession::HandleGuildModifyApplyLevel(tag_net_message* pCmd)
{
	NET_SIC_Modify_ApplyLevel* p_reveive = (NET_SIC_Modify_ApplyLevel*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;

	NET_SIS_Modify_ApplyLevel send;
	send.nLevel = p_reveive->nLevel;
	send.dwErrorCode = pGuild->ModifyApplyLevel(pRole, p_reveive->nLevel);
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}
DWORD PlayerSession::HandleSignUpAttack(tag_net_message* pCmd)
{
	NET_SIC_sign_up_attack* p_reveive = (NET_SIC_sign_up_attack*)pCmd;

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;

	NET_SIS_sign_up_attack send;
	send.dw_error_code = pGuild->SignUpAttact(pRole);
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}
DWORD PlayerSession::HandleGetSBKData(tag_net_message* pCmd)
{
	NET_SIC_get_SBK_data* p_recv = (NET_SIC_get_SBK_data*)pCmd;
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;
	//gx modify 2013.10.22
	//δ�����л�����ҲӦ�õõ�ɳ�Ϳ��л����Ϣ
	/*guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;*/
	
	NET_SIS_get_SBK_data send;
	g_guild_manager.getSBKData(send.s_sbk_data);
	pRole->SendMessage(&send, send.dw_size);
	return 0;
}

DWORD PlayerSession::HandleGetSBKReward(tag_net_message* pCmd)
{
	NET_SIC_get_SBK_reward* p_recv = (NET_SIC_get_SBK_reward*)pCmd;
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
		return INVALID_VALUE;
	
	if (g_guild_manager.is_begin_SBK())
		return INVALID_VALUE;

	// ����ɳ�Ϳ˹��᲻����ȡ
	if (pGuild->get_guild_att().dwID != g_guild_manager.get_SBK_guild())
		return INVALID_VALUE;

	NET_SIS_get_SBK_reward send;
	send.dw_error_code = pGuild->getGuildSBKReward(pRole);
	pRole->SendMessage(&send, send.dw_size);

	return 0;
}
//-----------------------------------------------------------------------------
// ��Ὺʼ����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleStartGuildPractice(tag_net_message* pCmd)
{
	NET_SIC_Start_Guild_Practice* p_receive = (NET_SIC_Start_Guild_Practice*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->get_upgrade().StartGuildPractice(pRole);

	NET_SIS_Start_Guild_Practice send;
	send.dw_error = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ��ȡ��ᶦ��Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildTripodInfo(tag_net_message* pCmd)
{
	NET_SIC_Guild_Tripod_Info* p_recv = (NET_SIC_Guild_Tripod_Info*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->send_tripod_message();

	return 0;
}

// ��ȡ����ʽ�
DWORD PlayerSession::HandleGuildGetFund(tag_net_message* pCmd)
{
	NET_SIC_Get_Guild_Fund* p_recv = (NET_SIC_Get_Guild_Fund*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(p_recv->dw_guild_id);
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	Role* pLeader = g_roleMgr.get_role(pGuild->get_guild_att().dwLeaderRoleID);
	
	INT32 nOnlineNum = pGuild->get_guild_member().get_online_num();

	NET_SIS_Get_Guild_Fund send;
	send.dw_error_code = E_Success;
	send.dw_guild_id = p_recv->dw_guild_id;
	if (VALID_POINT(pLeader)) send.b_online = true;
	send.dw_fund = pGuild->get_guild_att().nFund;
	send.nOnlineNum = nOnlineNum;
	send.dw_leader_id = pGuild->get_guild_att().dwLeaderRoleID;
	send.dw_guild_level = pGuild->get_guild_att().byLevel;
	pRole->SendMessage(&send, send.dw_size);

	return 0;

}

// ��ȡ���ս��
DWORD PlayerSession::HandleGuildWarHistory(tag_net_message* pCmd)
{
	NET_SIC_Guild_War_History* p_recv = (NET_SIC_Guild_War_History*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	
	BYTE* p = new BYTE[1024];
	ZeroMemory(p, sizeof(*p));

	NET_SIS_Guild_War_History* p_send = (NET_SIS_Guild_War_History*)p;
	M_msg_init(p_send, NET_SIS_Guild_War_History);
	
	std::vector<tagGuildWarHistory> vecHistory = pGuild->get_war_history();

	INT n_num = vecHistory.size();
	p_send->n_num = n_num;

	tagGuildWarHistory* p_data = p_send->st_guild_war_history;
	
	for (int i = 0; i < n_num; i++)
	{
		p_data[i] = vecHistory[i];
	}

	p_send->dw_size = sizeof(NET_SIS_Guild_War_History) + (p_send->n_num-1)*sizeof(NET_SIS_Guild_War_History);
	pRole->SendMessage(p_send, p_send->dw_size);

	SAFE_DELETE_ARRAY(p);

	return 0;
}
//-----------------------------------------------------------------------------
// ����ȡDKP����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetDKP(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_dkp);
	NET_SIC_get_dkp * p_receive = ( NET_SIC_get_dkp * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	NET_SIS_get_dkp send;
	memcpy(send.n16DKP, pGuild->get_guild_att().n16DKP, sizeof(send.n16DKP));
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
// �������DKP����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandldSetDKP(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_set_dkp);
	NET_SIC_set_dkp * p_receive = ( NET_SIC_set_dkp * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->set_guild_dkp(pRole, p_receive->n16DKP);

	NET_SIS_set_dkp send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ���DKPȷ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDKPAffirmance(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_dkp_affirmance);
	NET_SIC_dkp_affirmance * p_receive = ( NET_SIC_dkp_affirmance * ) pCmd ;  
	Role* pRole = GetRole();	
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->set_dkp_affirmance(pRole, p_receive->n16DKP);

	NET_SIS_dkp_affirmance send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleInviteLeague(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_invite_league);
	NET_SIC_invite_league * p_receive = ( NET_SIC_invite_league * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->invite_league(pRole, p_receive->dwLeagueGuildID);

	NET_SIS_invite_league send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// �������˻ظ�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleInviteLeagueRes(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_invite_league_result);
	NET_SIC_invite_league_result * p_receive = ( NET_SIC_invite_league_result * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->invite_league_res(pRole, p_receive->dwInviteGuildID, p_receive->bAgree);

	NET_SIS_invite_league_result send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// �������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandelRelieveLeague(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_relieve_league);
	NET_SIC_relieve_league * p_receive = ( NET_SIC_relieve_league * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->relieve_league(pRole, p_receive->dwGuildID);

	NET_SIS_relieve_league send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// �������ǩ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleInviteSign(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_invite_sign);
	NET_SIC_invite_sign * p_receive = ( NET_SIC_invite_sign * ) pCmd ;  
	Role* pInviteRole = g_roleMgr.get_role(p_receive->dwInviteID);
	if(!VALID_POINT(pInviteRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pInviteRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	Role* pBeInviteRole = g_roleMgr.get_role(p_receive->dwBeInviteID);
	if(!VALID_POINT(pBeInviteRole))
		return INVALID_VALUE;

	DWORD dwError = pGuild->invite_sign(pInviteRole, pBeInviteRole, p_receive->n64ItemID);

	if(dwError != E_Success)
	{
		NET_SIS_invite_sign send;
		send.dwError = dwError;
		pInviteRole->SendMessage(&send, send.dw_size);
	}
	else
	{
		NET_SIS_invite_sign_data send;
		send.dwInviteID = pInviteRole->GetID();
		send.st_GuildSignData.dwGuildID = pInviteRole->GetGuildID();
		memcpy(send.st_GuildSignData.dwSignRoleID, pGuild->get_guild_att().dwSignRoleID, sizeof(pGuild->get_guild_att().dwSignRoleID));
		send.st_GuildSignData.dwCreateGuildTime = pGuild->get_guild_att().dwCreateTime;
		pBeInviteRole->SendMessage(&send, send.dw_size);
	}
	return dwError;
}

//-----------------------------------------------------------------------------
// ȡ������ǩ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCancelSign(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_cancel_sign);
	NET_SIC_cancel_sign * p_receive = ( NET_SIC_cancel_sign * ) pCmd ;  
	Role* pCancelRole = GetRole();
	if(!VALID_POINT(pCancelRole))
		return INVALID_VALUE;

	Map* pMap = pCancelRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	DWORD dwBeCancelID = pCancelRole->GetSignID();
	pCancelRole->SetSignID(INVALID_VALUE);

	Role* pBeCancelRole = g_roleMgr.get_role(dwBeCancelID);
	if(!VALID_POINT(pBeCancelRole))
		return INVALID_VALUE;

	pBeCancelRole->SetSignID(INVALID_VALUE);

	NET_SIS_cancel_sign send;
	pBeCancelRole->SendMessage(&send, send.dw_size);
 
	return E_Success;
}

//-----------------------------------------------------------------------------
// ȷ�ϰ���ǩ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleAffirmanceSign(tag_net_message* pCmd)
{
	Role* pAffirmRole = GetRole();
	if(!VALID_POINT(pAffirmRole))
		return INVALID_VALUE;

	Map* pMap = pAffirmRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pAffirmRole->GetSignID() == INVALID_VALUE)
		return INVALID_VALUE;

	Role* pLeaderRole = g_roleMgr.get_role(pAffirmRole->GetSignID());
	if(!VALID_POINT(pLeaderRole))
	{
		pAffirmRole->SetSignID(INVALID_VALUE);
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pLeaderRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->affirmance_sign(pAffirmRole, pLeaderRole);

	return E_Success;
}

//-----------------------------------------------------------------------------
// �ύ����ǩ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleReferSign(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_refer_sign);
	NET_SIC_refer_sign * p_receive = ( NET_SIC_refer_sign * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	Map* pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(E_Success != pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_Guild))
		return INVALID_VALUE;
	
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->refer_sign(pRole, p_receive->dwNPCID, p_receive->n64ItemID);

	NET_SIS_refer_sign send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ��ȡ��ᵯ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildDelateData(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_delate_data);
	NET_SIC_get_guild_delate_data * p_receive = ( NET_SIC_get_guild_delate_data * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->get_delate().get_guild_delate_data(pRole);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��ȡ��ᵯ������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildDelateContent(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_delate_content);
	NET_SIC_get_guild_delate_content * p_receive = ( NET_SIC_get_guild_delate_content * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	pGuild->get_delate().get_guild_delate_content(pRole);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDelateLeader(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_delate_leader);
	NET_SIC_delate_leader * p_receive = ( NET_SIC_delate_leader * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	INT32 nContentCnt = (p_receive->dw_size - FIELD_OFFSET(NET_SIC_delate_leader, szContent)) / sizeof(TCHAR);
	DWORD dwError = pGuild->get_delate().delate_leader(pRole, p_receive->n64ItemID, p_receive->szContent, nContentCnt);

	NET_SIS_delate_leader send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}

//-----------------------------------------------------------------------------
// ����ͶƱ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleDelateBallot(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_delate_ballot);
	NET_SIC_delate_ballot * p_receive = ( NET_SIC_delate_ballot * ) pCmd ;  
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
		return INVALID_VALUE;

	DWORD dwError = pGuild->get_delate().delate_ballot(pRole, p_receive->bAgree);

	NET_SIS_delate_ballot send;
	send.dwError = dwError;
	pRole->SendMessage(&send, send.dw_size);

	return dwError;
}


//********************** ��ȡ���������Ϣ��Ϣ���� **********************//

//-----------------------------------------------------------------------------
// ��ȡ�������г�Ա
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildMembers(tag_net_message* pCmd)
{
	//MGET_MSG(p, pCmd, NET_SIC_guild_get_all_member);

	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!pRole->IsInGuild())
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		// ִ�е��˴���˵�����ɹ��������� -- ������û�г�Աʱ�ſ���ɾ������
		ASSERT(0);
		return INVALID_VALUE;
	}

	pGuild->send_all_members_to_client(pRole);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��ȡ����ָ����Ա��չ��Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildMemberEx(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_guild_get_member_extend);
	NET_SIC_guild_get_member_extend * p = ( NET_SIC_guild_get_member_extend * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!pRole->IsInGuild())
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		// ִ�е��˴���˵�����ɹ��������� -- ������û�г�Աʱ�ſ���ɾ������
		ASSERT(0);
		return INVALID_VALUE;
	}

	return pGuild->send_special_memberex_to_client(pRole, p->dw_role_id);
}

//-----------------------------------------------------------------------------
// ˢ�°���ָ����Ա��Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRefreshGuildMember(tag_net_message* pCmd)
{
	//GET_MESSAGE(p, pCmd, NET_SIC_guild_refresh_member);
	NET_SIC_guild_refresh_member * p = ( NET_SIC_guild_refresh_member * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!pRole->IsInGuild())
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		// ִ�е��˴���˵�����ɹ��������� -- ������û�г�Աʱ�ſ���ɾ������
		ASSERT(0);
		return INVALID_VALUE;
	}

	return pGuild->send_special_member_to_client(pRole, p->dw_role_id);
}

//-----------------------------------------------------------------------------
// ��ȡ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildName(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_name);
	NET_SIC_get_guild_name * p_receive = ( NET_SIC_get_guild_name * ) pCmd ;  
	guild *pGuild = g_guild_manager.get_guild(p_receive->dwGuildID);
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	return pGuild->send_guild_name_to_client(this);
}

//-----------------------------------------------------------------------------
// ��ȡ������ּ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildTenet(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_tenet);
	NET_SIC_get_guild_tenet * p_receive = ( NET_SIC_get_guild_tenet * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	return pGuild->send_guild_tenet_to_client(this);
}

//-----------------------------------------------------------------------------
// ��ȡ���ɱ�־
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildSymbol(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_symbol);
	NET_SIC_get_guild_symbol * p_receive = ( NET_SIC_get_guild_symbol * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(p_receive->dwGuildID);
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	return pGuild->send_guild_symbol_to_client(this);
}

//-----------------------------------------------------------------------------
// ��ȡ���ɲֿ�����Ʒ��Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildWareItems(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_ware);
	NET_SIC_get_guild_ware * p_receive = ( NET_SIC_get_guild_ware * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// NPC���
	DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildWare);
	if(E_Success != dw_error_code)
	{
		return INVALID_VALUE;
	}

	// �����㹻��Ļ�����
	CREATE_MSG(pSend, TEMP_GUILD_BUFF_SIZE, NET_SIS_get_guild_ware);
	pSend->dwLastUpdateTime = p_receive->dwLastUpdateTime;

	INT32 nItemSize = 0;
	pSend->dw_error_code = pGuild->get_guild_warehouse().get_guild_ware_info(pSend->byData, pSend->nItemNum, pSend->dwLastUpdateTime, pSend->nSzGuildWare, nItemSize);

	// ����������Ϣ��С
	pSend->dw_size = sizeof(NET_SIS_get_guild_ware) + nItemSize;

	// ���͸��ͻ���
	SendMessage(pSend, pSend->dw_size);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡ���ɲֿ����Ȩ���б�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildWarePriList(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_ware_power_list);
	NET_SIC_get_guild_ware_power_list * p_receive = ( NET_SIC_get_guild_ware_power_list * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// NPC���
	DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildWare);
	if(E_Success == dw_error_code)
	{
		dw_error_code = pGuild->get_guild_warehouse().send_guild_ware_power_list_to_client(pRole);
	}

	if (VALID_VALUE(dw_error_code) && (E_Success != dw_error_code))
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return INVALID_VALUE;
}

//-----------------------------------------------------------------------------
// ���ָ����Ա�İ��ɲֿ����Ȩ��
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGuildWarePrivilege(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_change_guild_ware_power);
	NET_SIC_change_guild_ware_power * p_receive = ( NET_SIC_change_guild_ware_power * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// NPC���
	DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildWare);
	if(E_Success == dw_error_code)
	{
		dw_error_code = pGuild->set_guild_ware_use_privilege(pRole, p_receive->dw_role_id, p_receive->bCanUse);
	}
	
	if (!VALID_VALUE(dw_error_code))
	{
		return INVALID_VALUE;
	}

	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		// ֪ͨ�����߸����б�
		NET_SIS_change_guild_ware_power send;
		send.dw_role_id	= p_receive->dw_role_id;
		send.bCanUse	= p_receive->bCanUse;
		SendMessage(&send, send.dw_size);

		// ֪ͨ��������Ȩ�ޱ��
		Role* pMember = g_roleMgr.get_role(p_receive->dw_role_id);
		if (VALID_POINT(pMember))
		{
			pMember->SendMessage(&send, send.dw_size);
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡ������ʩ������Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildFacilitiesInfo(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_guild_upgreae_info);
	NET_SIC_get_guild_upgreae_info * p_receive = ( NET_SIC_get_guild_upgreae_info * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	// NPC���
	/*DWORD dw_error_code = pRole->CheckFuncNPC(p_receive->dwNPCID, EFNPCT_GuildMgr);
	if(E_Success == dw_error_code)
	{
		dw_error_code = pGuild->send_facilities_info_to_client(pRole);
	}*/
	DWORD dw_error_code = pGuild->send_facilities_info_to_client(pRole, p_receive->eFacType);

	if (VALID_VALUE(dw_error_code) && (E_Success != dw_error_code))
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ����Ͻɰ�����ʩ����������Ʒ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleHandInItems(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_upgrede_level);
	NET_SIC_upgrede_level * p_receive = ( NET_SIC_upgrede_level * ) pCmd ;  
	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}
	
	DWORD dw_error_code = E_Success;
	//if (p_receive->eType == EFT_Null)
	//{
		dw_error_code = pGuild->handleLevelUp(pRole);

		NET_SIS_synchronize_guild_info send;
		send.sGuildInfo = (tagGuildBase)pGuild->get_guild_att();
		pRole->SendMessage(&send, send.dw_size);
	//}
	//else
	//{
	//	dw_error_code = pGuild->get_guild_facilities().HandInItems(pRole, p_receive->eType);
	//}

	if (VALID_VALUE(dw_error_code))
	{
		if (E_Success != dw_error_code)
		{
			g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
		}
		else
		{
			// ���ر仯�����ʩ��Ϣ
			/*NET_SIS_update_facilities_info send;
			pGuild->get_guild_facilities().GetGuildFacilitiesInfo(&send.sFacilitiesInfo, p_receive->eType);
			pRole->SendMessage(&send, send.dw_size);*/
		}
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ������������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSpreadGuildAffair(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_issuance_guild_affair);
	NET_SIC_issuance_guild_affair * p_receive = ( NET_SIC_issuance_guild_affair * ) pCmd ;  
	if (!VALID_VALUE(p_receive->dwBuffID))
	{
		return INVALID_VALUE;
	}

	Role *pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_affair().issuance_guild_affair(pRole, p_receive->dwBuffID);

	if (VALID_VALUE(dw_error_code) && (dw_error_code != E_Success))
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ȡ�ð��ɼ�����Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetGuildSkillInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	CREATE_MSG(pSend, TEMP_GUILD_BUFF_SIZE, NET_SIS_get_guild_skill_info);
	DWORD dw_error_code = pGuild->get_guild_skill().get_guild_skill_info(pSend->dwCurSkillID, pSend->n16Progress,
		pSend->nSkillNum, pSend->dwSkillInfo);

	// ���¼�����Ϣ��С
	if (dw_error_code == E_Success)
	{
		pSend->dw_size = sizeof(NET_SIS_get_guild_skill_info) + (pSend->nSkillNum - 1) * sizeof(DWORD);
		SendMessage(pSend, pSend->dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �Ͻɰ��ɼ��ܵ伮
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpgradeGuildSkill(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_guild_skill_upgrade);
	NET_SIC_guild_skill_upgrade * p_receive = ( NET_SIC_guild_skill_upgrade * ) pCmd ;  
	if (!VALID_VALUE(p_receive->n64ItemSerial))
	{
		return INVALID_VALUE;
	}

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	NET_SIS_guild_skill_upgrade send;
	DWORD dw_error_code = pGuild->get_guild_skill().skill_upgrade_turn_in(pRole, p_receive->n64ItemSerial, send.dwSkillID, send.n16Progress);
	if (E_Success == dw_error_code)
	{
		SendMessage(&send, send.dw_size);
	}
	else if (VALID_VALUE(dw_error_code))
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ѧϰ���ɼ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleLearnGuildSkill(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_learn_guild_skill);
	NET_SIC_learn_guild_skill * p_receive = ( NET_SIC_learn_guild_skill * ) pCmd ;  
	if (!VALID_VALUE(p_receive->dwSkillID))
	{
		return INVALID_VALUE;
	}

	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	NET_SIS_learn_guild_skill send;
	BOOL bLearnError = FALSE;
	INT nLevel = 0;
	DWORD dw_error_code = pGuild->get_guild_skill().learn_guild_skill(pRole, p_receive->dwSkillID, bLearnError, nLevel);

	if (VALID_VALUE(dw_error_code))
	{
		if (bLearnError)
		{
			send.dw_error_code	= dw_error_code;
			send.dwSkillID		= p_receive->dwSkillID * 100 + nLevel;
			SendMessage(&send, send.dw_size);
		}
		else if (dw_error_code != E_Success)
		{
			g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
		}
	}
	
	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ���õ�ǰ�о����ɼ���
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSetResearchSkill(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_research_skill);
	NET_SIC_research_skill * p_receive = ( NET_SIC_research_skill * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	NET_SIS_set_research_skill send;
	INT nLevel = 0;
	DWORD dw_error_code = pGuild->get_guild_skill().set_current_upgrade_guild_skill(pRole->GetID(), p_receive->dwSkillID, nLevel, send.n16Progress);
	if (!VALID_VALUE(dw_error_code))
	{
		return INVALID_VALUE;
	}

	if (dw_error_code != E_Success)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		if (VALID_VALUE(p_receive->dwSkillID))
		{
			send.dwSkillID	= p_receive->dwSkillID * 100 + nLevel;
		}
		pGuild->send_guild_message(&send, send.dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ͬ�����ɻ�����Ϣ
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSyncGuildInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	NET_SIS_synchronize_guild_info send;
	send.sGuildInfo = (tagGuildBase)pGuild->get_guild_att();
	SendMessage(&send, send.dw_size);

	return E_Success;
}

DWORD PlayerSession::HandleSyncGuildWarInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}
	
	guild* pEnemyGuild = g_guild_manager.get_guild(pGuild->get_guild_war().get_war_guild_id());
	if (!VALID_POINT(pEnemyGuild))
		return INVALID_VALUE;


	NET_SIS_synchronize_guild_war_info send;
	send.dwMaxWarNum = pGuild->get_guild_war().get_max_number();
	send.dwCurWarNum = pGuild->get_guild_war().m_dw_baoming_num;
	send.fMoneyParam = pGuild->get_guild_war().get_param();
	send.eSatae = pGuild->get_guild_war().get_guild_war_state();
	send.dwBeginTime = (DWORD)pGuild->get_guild_war().get_begin_time();
	send.dwGulldID = pGuild->get_guild_war().get_war_guild_id();

	_tcsncpy(send.szGuildName, pEnemyGuild->get_guild_att().str_name.c_str(), pEnemyGuild->get_guild_att().str_name.size());
	send.byEnemyLevel = pEnemyGuild->get_guild_att().byLevel;
	send.dwSymbolValue = pEnemyGuild->get_guild_att().dwValue;
	send.bDefenter = (pGuild->get_guild_war().get_declare_guild_id() != INVALID_VALUE);
	send.bRelay = pGuild->get_guild_war().isRelay();
	memcpy(send.szText, pEnemyGuild->get_guild_att().szText, sizeof(send.szText));

	SendMessage(&send, send.dw_size);
	
	return E_Success;
}
//-----------------------------------------------------------------------------
// ��ȡ�����̻���Ϣ	-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetCofCInfo(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_chamber_goods_info);
	NET_SIC_get_chamber_goods_info * p_receive = ( NET_SIC_get_chamber_goods_info * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	guild_chamber* pCofC = pMap->get_chamber(p_receive->dwNPCID);
	if (!VALID_POINT(pCofC))
	{
		// ����Ҫ֪ͨ�ͻ���
		return INVALID_VALUE;
	}

	return pCofC->send_commerce_goods_to_role(pRole);
}

//-----------------------------------------------------------------------------
// �ر��̻����		-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCloseCofC(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_colse_chamber);
	NET_SIC_colse_chamber * p_receive = ( NET_SIC_colse_chamber * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	Map* pMap = pRole->get_map();
	if (!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	guild_chamber* pCofC = pMap->get_chamber(p_receive->dwNPCID);
	if (!VALID_POINT(pCofC))
	{
		return INVALID_VALUE;
	}

	// �Ƴ��۲����б�
	pCofC->remove_observer(pRole->GetID());

	return E_Success;
}

//-----------------------------------------------------------------------------
// �����̻��̻�		-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleBuyCofCGoods(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_buy_chamber_goods);
	NET_SIC_buy_chamber_goods * p_receive = ( NET_SIC_buy_chamber_goods * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_commerce().buy_goods(pRole, p_receive->dwNPCID, p_receive->dwGoodID, p_receive->byBuyNum);
	if ((E_Success != dw_error_code) && VALID_VALUE(dw_error_code))
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// �����̻����̻�	-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSellCofCGoods(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_sell_chamber_goods);
	NET_SIC_sell_chamber_goods * p_receive = ( NET_SIC_sell_chamber_goods * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_commerce().sell_goods(pRole, p_receive->dwNPCID, p_receive->dwGoodID, p_receive->bySellNum);
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡ����������Ϣ	-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetCommodityInfo(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_get_commodity_info);
	NET_SIC_get_commodity_info * p_receive = ( NET_SIC_get_commodity_info * ) pCmd ;  
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	INT nMsgSize = sizeof(NET_SIS_get_commodity_info) + sizeof(tagCommerceGoodInfo) * (MAX_COMMODITY_CAPACITY - 1);
	CREATE_MSG(pSend, nMsgSize, NET_SIS_get_commodity_info);
	// ��ȡ�̻���Ϣ
	DWORD dw_error_code = pGuild->get_guild_commerce().get_commodity_goods_info(pRole, pSend->sGoodInfo, pSend->nGoodNum, pSend->nCurTael, pSend->nLevel);

	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		// ������Ϣ��С
		pSend->dw_size = sizeof(NET_SIS_get_commodity_info) + sizeof(tagCommerceGoodInfo) * (pSend->nGoodNum - 1);
		SendMessage(pSend, pSend->dw_size);
	}

	MDEL_MSG(pSend);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��ȡ���̳�ʼ��Ϣ	-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetTaelInfo(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	NET_SIS_get_tael_info send;
	tagTaelInfo sTaelInfo;
	DWORD dw_error_code = pGuild->get_guild_commerce().get_commerce_init_info(pRole->GetID(), send.nLevel, sTaelInfo);
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		SendMessage(&send, send.dw_size);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// ��ȡ������������	-- ��ͼ�߳�
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetCommerceRank(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	INT nMsgSize = sizeof(NET_SIS_get_commerce_rank) + sizeof(tagCommerceRank) * (MAX_COMMERCE_RANK_INFO - 1);
	CREATE_MSG(pSend, nMsgSize, NET_SIS_get_commerce_rank);
	// ��ȡ������Ϣ
	DWORD dw_error_code = pGuild->get_guild_commerce().get_commerce_rank_info(pSend->sRankInfo, pSend->nRankNum, pSend->bCommend);

	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		// ������Ϣ��С
		pSend->dw_size = sizeof(NET_SIS_get_commerce_rank) + sizeof(tagCommerceRank) * (pSend->nRankNum - 1);
		SendMessage(pSend, pSend->dw_size);
	}

	MDEL_MSG(pSend);

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleAcceptCommerce(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_commerce().accept_commerce(pRole);
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		INT nLevel;
		tagTaelInfo sTaelInfo;
		NET_SIS_accept_commerce send;
		dw_error_code = pGuild->get_guild_commerce().get_commerce_init_info(pRole->GetID(), nLevel, sTaelInfo);
		if (dw_error_code == E_Success)
		{
			send.nBeginningTael = sTaelInfo.nBeginningTael;
			SendMessage(&send, send.dw_size);
		}
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// �Ͻ�����
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleCompleteCommerce(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	INT32 nFund = 0;
	DWORD dw_error_code = pGuild->get_guild_commerce().complete_commerce(pRole, nFund);
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_complete_commerce send;
		send.dw_role_id	= pRole->GetID();
		send.nFund		= nFund;
		pGuild->send_guild_message(&send, send.dw_size);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleAbandonCommerce(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_commerce().abandon_commerce(pRole->GetID());
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	
	return E_Success;
}

//-----------------------------------------------------------------------------
// �������̼ν�״̬
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSwitchCommendation(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_switch_commendation);
	NET_SIC_switch_commendation * p_receive = ( NET_SIC_switch_commendation * ) pCmd ;  
	Role* pRole = GetRole();	
	if (!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// ȡ�ð�����Ϣ
	guild* pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if (!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->get_guild_commerce().switch_commendation(pRole->GetID(), p_receive->bSwitchON);
	if (E_Success != dw_error_code)
	{
		g_guild_manager.send_guild_failed_to_client(this, dw_error_code);
	}
	else
	{
		NET_SIS_switch_commendation send;
		send.dw_role_id	= pRole->GetID();
		send.bSwitchON	= p_receive->bSwitchON;
		pGuild->send_guild_message(&send, send.dw_size);
	}

	return E_Success;
}

// ��������ļ��
DWORD PlayerSession::HandleJoinGuildRecruit(tag_net_message* pCmd)
{
	NET_SIC_join_guild_recruit* p_msg = (NET_SIC_join_guild_recruit*)pCmd;

	Role* pRole = g_roleMgr.get_role(p_msg->dw_role_id);
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	

	DWORD dw_error = g_guild_manager.join_guild_recruit(pRole, p_msg->dw_guild_id);

	NET_SIS_join_guild_recruit send;
	send.dw_guild_id = (dw_error == E_Success ? p_msg->dw_guild_id : INVALID_VALUE);
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// �뿪�����ļ��
DWORD PlayerSession::HandleLeaveGuildRecruit(tag_net_message* pCmd)
{
	NET_SIC_leave_guild_recruit* p_msg = (NET_SIC_leave_guild_recruit*)pCmd;

	Role* pRole = g_roleMgr.get_role(p_msg->dw_role_id);
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	DWORD dw_error = g_guild_manager.leave_guild_recruit(pRole);

	NET_SIS_leave_guild_recruit send;
	send.dw_error = dw_error;
	pRole->SendMessage(&send, send.dw_size);

	return E_Success;
}

// ��ѯ�����ļ����Ϣ
DWORD PlayerSession::HandleQueryGuildRecruit(tag_net_message* pCmd)
{
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_guild_manager.query_guild_recruit(pRole);

	return E_Success;
}

// ��ѯĳҳ�����ļ����Ϣ
DWORD PlayerSession::HandleQueryPageGuildRecruit(tag_net_message* pCmd)
{
	NET_SIC_query_page_guild_recruit* p_msg = (NET_SIC_query_page_guild_recruit*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	g_guild_manager.send_query_result(pRole, p_msg->n_page, pRole->get_list_guild_recruit());

	return E_Success;
}

// ͬ�����
DWORD PlayerSession::HandleAgreeJoin(tag_net_message* pCmd)
{
	NET_SIC_agree_join* p_msg = (NET_SIC_agree_join*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;
	
	Role* pTarget = g_roleMgr.get_role(p_msg->dwRoleID);
	if (!VALID_POINT(pTarget))//gx modify 2013.10.30 �����ߵ���Ҹ��ͻ�����ʾ
	{
		NET_SIS_agree_join	send;
		send.dwRoleID = p_msg->dwRoleID;
		send.dwErrorCode = E_Role_Not_Online;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	// �Ƿ���Լ�����
	if(pRole->GetID() == p_msg->dwRoleID)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}
	
	if (!g_guild_manager.is_have_recruit(p_msg->dwRoleID, pGuild->get_guild_att().dwID))
	{
		NET_SIS_agree_join	send;// gx add 2013.8.30
		send.dwRoleID = p_msg->dwRoleID;
		send.dwErrorCode = E_Guild_recruit_nohave_exist;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}

	DWORD dw_error_code = pGuild->invite_join(pRole->GetID(), p_msg->dwRoleID);
	

	// ���ɹ㲥
	s_role_info *pRoleInfo = g_roleMgr.get_role_info(pTarget->GetID());
	if (VALID_POINT(pRoleInfo))
	{
		INT32 nRoleNameCnt = _tcsclen(pRoleInfo->sz_role_name);
		INT32 nMsgSz = sizeof(NET_SIS_guild_join_broad) + nRoleNameCnt * sizeof(TCHAR);

		CREATE_MSG(pSend, nMsgSz, NET_SIS_guild_join_broad);
		pSend->dw_role_id		= pTarget->GetID();
		pSend->dwRoleNameID	= pTarget->GetNameID();

		memcpy(pSend->szRoleName, pRoleInfo->sz_role_name, (nRoleNameCnt + 1) * sizeof(TCHAR));

		pGuild->send_guild_message(pSend, pSend->dw_size);
		MDEL_MSG(pSend);
	}

	// ͬ������Χ���
	tagGuildMember* pMember = pGuild->get_member(pTarget->GetID());
	Map*			pMap	= pTarget->get_map();
	if (VALID_POINT(pMember) && VALID_POINT(pMap))
	{
		NET_SIS_remote_role_guild_info_change send;
		send.dwGuildID		= pRole->GetID();
		send.dw_role_id		= pTarget->GetID();
		send.n8GuildPos		= pMember->eGuildPos;

		pMap->send_big_visible_tile_message(pTarget, &send, send.dw_size);
	}

	//������ҽű�
	if(VALID_POINT( pTarget->GetScript( ) ) ) 
	{
		pTarget->GetScript( )->OnJoinGuild( pTarget, pGuild->get_guild_att().dwID );
	}

	pTarget->GetAchievementMgr().UpdateAchievementCriteria(ete_join_guild, 1);


	g_guild_manager.delete_role_from_guild_recruit(pTarget->GetID());

	
	NET_SIS_agree_join	send;
	send.dwRoleID = p_msg->dwRoleID;
	send.dwErrorCode = dw_error_code;
	SendMessage(&send, send.dw_size);
	
	if (E_Success == dw_error_code)
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	NET_SIS_leave_guild_recruit sendrecruit;
	sendrecruit.dw_error = E_Success;
	pTarget->SendMessage(&sendrecruit, sendrecruit.dw_size);

	return E_Success;
}

// �ܾ�����
//gx modify 2013.10.30 �����ߵ����Ҳ���Խ���ܾ�
DWORD PlayerSession::HandlenoAgreeJoin(tag_net_message* pCmd)
{	
	NET_SIC_noagree_join* p_msg = (NET_SIC_noagree_join*)pCmd;

	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	// �Ƿ���Լ�����
	if(pRole->GetID() == p_msg->dwRoleID)
	{
		return INVALID_VALUE;
	}

	// �õ�����
	guild *pGuild = g_guild_manager.get_guild(pRole->GetGuildID());
	if(!VALID_POINT(pGuild))
	{
		return INVALID_VALUE;
	}
	if (!g_guild_manager.is_have_recruit(p_msg->dwRoleID, pGuild->get_guild_att().dwID))//gx add
	{
		NET_SIS_noagree_join	send;
		send.dwRoleID = p_msg->dwRoleID;
		send.dwErrorCode = E_Guild_recruit_nohave_exist;
		SendMessage(&send, send.dw_size);
		return INVALID_VALUE;
	}
	
	g_guild_manager.delete_role_from_guild_recruit(p_msg->dwRoleID);
		
	NET_SIS_noagree_join	send;
	send.dwRoleID = p_msg->dwRoleID;
	send.dwErrorCode = E_Success;
	SendMessage(&send, send.dw_size);//�᳤����ˢ��UI��

	Role* pTarget = g_roleMgr.get_role(p_msg->dwRoleID);
	if (VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send,send.dw_size);//gx modify 2013.8.29
		
		NET_SIS_leave_guild_recruit sendrecruit;
		sendrecruit.dw_error = E_Success;
		pTarget->SendMessage(&sendrecruit, sendrecruit.dw_size);
	}
	return E_Success;
}	