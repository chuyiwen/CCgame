/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "stdafx.h"
#include "db_session.h"
#include "role.h"
#include "role_mgr.h"
#include "../common/ServerDefine/master_apprentice_server_define.h"
#include "../../common/WorldDefine/master_prentice_define.h"
#include "../../common/WorldDefine/master_prentice_protocol.h"
#include "../../common/WorldDefine/creature_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "creature.h"
#include "master_prentice_mgr.h"
#include "../../common/WorldDefine/social_protocol.h"
#include "social_mgr.h"
//--------------------------------------------------------------------
// G L O B A L   V A R I A B L E
//--------------------------------------------------------------------
master_prentice_mgr g_MasterPrenticeMgr;
//--------------------------------------------------------------------
// 主线程调用
//--------------------------------------------------------------------
VOID master_prentice_mgr::init( )
{
	Super::Init( );
	this->register_event( );
}

VOID master_prentice_mgr::init_placard( const VOID* p_data )
{
	NET_DB2S_load_all_master_prentice* p = (NET_DB2S_load_all_master_prentice*)p_data;
	if( !VALID_POINT(p->u32_number) )
		return ; 

	s_master_prentice_load* p_masterplacard_load = p->master_prentice;
	for( UINT32 n = 0; n < p->u32_number; ++n)
	{
		s_master_placard* p_placard = new s_master_placard;
		p_placard->dw_master = p_masterplacard_load->dw_master;
		p_placard->dw_graduates = p_masterplacard_load->dw_graduates;
		p_placard->dw_master_moral = p_masterplacard_load->dw_master_moral;
		p_placard->b_show_in_panel = p_masterplacard_load->b_show_in_panel;
		GetReputionOfGraduates(p_placard->dw_graduates, p_placard->by_master_repution);
		
		ZeroMemory(p_placard->dw_prenices, sizeof(p_placard->dw_prenices));
		ASSERT(p_masterplacard_load->by_number<=PRENTICE_MAX_COUNT);
		p_placard->by_number = min(PRENTICE_MAX_COUNT, p_masterplacard_load->by_number);
		if(p_placard->by_number)
			get_fast_code()->memory_copy(p_placard->dw_prenices, p_masterplacard_load->dw_prentices, sizeof(DWORD) * p_placard->by_number );

		if( !master_placards_.is_exist( p_placard->dw_master ) )
			master_placards_.add(p_placard->dw_master, p_placard);
		else
		{
			ASSERT(FALSE);
			SAFE_DELETE(p_placard); //逻辑错误
		}
			
		p_masterplacard_load = (s_master_prentice_load*)((char*)p_masterplacard_load + MasterPrenticeLoadRealSize(p_masterplacard_load->by_number));
	}
}

VOID master_prentice_mgr::init_recruit(const VOID* p_data, INT iNumber)
{
	//master_recruit_.clear();
	if(iNumber>0)
	{
		DWORD* pRoleID = (DWORD*)p_data;
		for(INT n  = 0; n < iNumber; ++n, ++pRoleID)
			master_recruit_.push_back(*pRoleID);
	}
}
VOID master_prentice_mgr::update( )
{
	Super::Update( );

	static DWORD ls_last_seconds = timeGetTime( );
	if( (timeGetTime() - ls_last_seconds) >= 60*1000 )
	{//60秒存一次数据库
		ls_last_seconds += 60*1000;
		save_to_db( );
	}
}
VOID master_prentice_mgr::force_save_all( )
{
	this->save_to_db( );
}
VOID master_prentice_mgr::save_to_db( )
{
	if( save_list_.size( ) <= 0) return ;

	UINT32 u_list_size = save_list_.size( );
	UINT32 u32_msg_size = sizeof(NET_DB2C_save_master_prentice) + u_list_size * MasterPrenticeSaveRealSize(PRENTICE_MAX_COUNT);

	CREATE_MSG(p_send, u32_msg_size, NET_DB2C_save_master_prentice);
	p_send->dw_size -= sizeof(s_master_prentice_save);
	p_send->u32_number = 0;

	DWORD dw_master = save_list_.pop_front( );
	s_master_prentice_save* p_save = p_send->master_prentice;
	while(VALID_POINT(dw_master) && u_list_size)
	{
		s_master_placard* p_master_placard = master_placards_.find( dw_master );
		if( VALID_POINT(p_master_placard) )
		{
			p_save->dw_master = p_master_placard->dw_master;
			p_save->dw_graduates = p_master_placard->dw_graduates;
			p_save->dw_master_moral = p_master_placard->dw_master_moral;
			p_save->b_show_in_panel = p_master_placard->b_show_in_panel;

			ASSERT(p_save->by_number<=PRENTICE_MAX_COUNT);
			p_save->by_number = min(PRENTICE_MAX_COUNT, p_master_placard->by_number);
			if(p_save->by_number)
				get_fast_code()->memory_copy(p_save->dw_prentices, p_master_placard->dw_prenices, sizeof(DWORD)*p_save->by_number);
			p_send->dw_size += MasterPrenticeSaveRealSize(p_save->by_number);
			++p_send->u32_number; ++p_save;
		}
		--u_list_size; dw_master = save_list_.pop_front( );
	}
	
	if( p_send->u32_number ) g_dbSession.Send(p_send, p_send->dw_size);
}

VOID master_prentice_mgr::register_event( )
{
	Super::RegisterEventFunc(EVT_SynRoleLevel,	   &master_prentice_mgr::syn_role_levelup );
}
//--------------------------------------------------------------------
// 客户端消息事件
//--------------------------------------------------------------------
VOID master_prentice_mgr::syn_role_levelup(DWORD dw_sender, VOID* p_message )
{
	NET_SIS_change_role_level* p = (NET_SIS_change_role_level*)p_message;

	Role* p_role = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT(p_role) || p->nLevel < MASTER_MIN_LEVEL)
		return;

	BOOL bSendDBMsg = FALSE;

	//出师方式修改为，徒弟手拉手师傅，肩并肩找个npc主动出师
	//if( p_role->get_master_id( ) )
	//{//如果有师傅，出师
	//	/*if(p_role->get_level() == 30 || 
	//		p_role->get_level() == 40 )
	//	{
	//		NET_SIS_say_goodbye_to_master send;
	//		p_role->SendMessage(&send, send.dw_size);
	//	}
	//	else if(p_role->get_level()>=PRENTICE_MAX_LEVEL)
	//	{
	//		say_goodbye_to_master(p_role);
	//		bSendDBMsg = TRUE;
	//	}*/
	//	//gx modify 2013.12.05
	//	if (p_role->get_level()>=PRENTICE_MAX_LEVEL)
	//	{
	//		say_goodbye_to_master(p_role);
	//		bSendDBMsg = TRUE;
	//	}
	//}

	//if(p->nLevel > MASTER_MIN_LEVEL)
	//{
	if( !master_placards_.is_exist(dw_sender) )
	{
		{//添加到师徒表
			NET_DB2C_insert_master send;
			send.dw_master = dw_sender;
			g_dbSession.Send(&send, send.dw_size);
		}

		s_master_placard* p_master_placard = new s_master_placard;
		ZeroMemory( p_master_placard, sizeof(s_master_placard));
		p_master_placard->dw_master = dw_sender;
		GetReputionOfGraduates(p_master_placard->dw_graduates, p_master_placard->by_master_repution );
		master_placards_.add( dw_sender, p_master_placard );

		{
			//gx modify 2013.12.10
			/*NET_SIS_master_moral_and_gradates send;
			send.dwMasterMoral = p_master_placard->dw_master_moral;
			send.dwGraduates   = p_master_placard->dw_graduates;
			p_role->SendMessage(&send,send.dw_size);*/
		}
		bSendDBMsg = TRUE;
	}
	//}
	//gx modify 2013.12.12
	/*if(p->nLevel > FUK_PRENTIC_CHECK_MAX && find_role_in_recruit(dw_sender)) bSendDBMsg = TRUE;
	if(bSendDBMsg) remove_role_in_recruit(dw_sender, bSendDBMsg);*/
}

VOID master_prentice_mgr::role_deleted( DWORD dw_sender, VOID* p_message )
{
	s_master_placard* p_placard = this->get_master_placard( dw_sender );
	if( VALID_POINT( p_placard ) )
	{//师傅删除角色
		{//通知在线弟子，师傅去也
			NET_SIS_master_delete_role send;
			this->send_for_each( p_placard, &send, send.dw_size);
		}

		{// 将在线弟子的师傅ID设置为0
			for( INT n = 0; n < p_placard->by_number; ++n )
			{//所有徒弟
				Role* p_member = g_roleMgr.get_role( p_placard->dw_prenices[n]);
				if( VALID_POINT(p_member ) )  p_member->set_master_id(INVALID_MASTER);
			}
		}

		{//更新数据库
			{
				NET_DB2C_update_master_id_and_forbid_time send;
				send.dw_forbid_time = 0;
				send.dw_master = INVALID_MASTER;

				for( INT n = 0 ; n < p_placard->by_number; ++n )
				{
					send.dw_role_id = p_placard->dw_prenices[n];
					g_dbSession.Send(&send, send.dw_size);
				}
			}

			{
				NET_DB2C_master_delete_role send;
				send.dw_role_id = dw_sender;
				g_dbSession.Send(&send, send.dw_size);
			}

			this->delete_one_master_placard( dw_sender );
		}
	}

	{//非师傅删角色
		master_placards_.reset_iterator( );
		while( master_placards_.find_next( p_placard ) )
		{
			if(p_placard->has_prentice(dw_sender)) break;
			p_placard = NULL;
		}

		if( VALID_POINT(p_placard) )
		{	
			p_placard->remove_prentice( dw_sender );

			//通知师门所有人
			NET_SIS_prentice_delete_role send;
			send.dwPrentice = dw_sender;
			this->send_for_each( p_placard, &send, send.dw_size );
			
			save_list_.push_back( p_placard->dw_master );
		}
		// 通知数据库
		remove_role_in_recruit(dw_sender, TRUE);
	}

}
VOID master_prentice_mgr::make_master(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_make_master* p = (NET_SIC_make_master*)p_message;

	Role* p_prentice = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT( p_prentice ) ) return;
	if( p_prentice->GetID( ) == p->dwMaster ) return;

	if( p_prentice->get_master_prentice_forbid_time( )> (DWORD)GetCurrentDWORDTime( ) )
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_You_ForbidOp;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	if(p_prentice->is_graduates_from_master( ))
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Master_Has_Graduates;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	if( p_prentice->get_master_id( ) )
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_You_HasMaster;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	//拜师等级[10-60]
	if( p_prentice->get_level( ) > PRENTICE_MAX_LEVEL || p_prentice->get_level() < PRENTICE_MIN_LEVEL)
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Your_OutOfLevel;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	//师傅等级
	Role* p_master = g_roleMgr.get_role( p->dwMaster );
	if( !VALID_POINT(p_master)) return ;

	//师徒两人必须在安全区内
	if (!p_prentice->IsInRoleState(ERS_SafeArea) || !p_master->IsInRoleState(ERS_SafeArea))
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MasterPrentice_OutSafeArea;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	if(p_master->get_level( ) <= p_prentice->get_level( ))
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Master_MinOfLevel;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	if(p_master->get_master_prentice_forbid_time( ) > (DWORD)GetCurrentDWORDTime( ) )
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Master_ForbidOp;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}

	//>=60
	s_master_placard* p_master_placard = this->get_master_placard( p->dwMaster );
	if( !VALID_POINT(p_master_placard) )
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Master_OutOfLevel;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}
	//拜师的对象不能还是别人的徒弟
	if (p_master->get_master_id())
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_MasterIsPrentice;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}
	if( p_master_placard->has_prentice( dw_sender))
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_You_HasMaster;
		p_prentice->SendMessage( &send, send.dw_size );
		return;
	}

	//师傅徒弟数[1...n]
	//if( p_master_placard->by_number >= g_PrenticeMaxNumber[p_master_placard->by_master_repution])
	if (p_master_placard->by_number >= MAX_PRENTICE)//gx modify 2013.12.05
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_Master_MaxPrentices;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}
	//判断师傅是否正在被邀请
	if (p_master->GetShituAskID() != INVALID_VALUE)
	{
		NET_SIS_make_master send;
		send.dwMaster = p->dwMaster;
		send.dwError = E_MakeMaster_MasterBusy;
		p_prentice->SendMessage( &send, send.dw_size );
		return ;
	}
	p_master->SetShituAskID(dw_sender);
	//给徒弟一个信儿 gx add 2013.12.19
	NET_SIS_make_master send_prentice;
	send_prentice.dwMaster = p->dwMaster;
	send_prentice.dwError = E_MakeMaster_ToConfirm;
	p_prentice->SendMessage( &send_prentice, send_prentice.dw_size );

	//转发师傅
	NET_SIS_make_master_extend send;
	send.dwPrentice = dw_sender;
	p_master->SendMessage(&send, send.dw_size);
}
VOID master_prentice_mgr::make_master_ex(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_make_master_extend* p = (NET_SIC_make_master_extend*)p_message;

	Role* p_sender = g_roleMgr.get_role(dw_sender);
	if(!VALID_POINT(p_sender)) return;

	Role* p_prentice = g_roleMgr.get_role( p->dwPrentice );
	if( !VALID_POINT( p_prentice ) ) return ;
	if(VALID_POINT(p_prentice->get_master_id())) return;

	p_sender->SetShituAskID(INVALID_VALUE);
	//转发回应给徒弟
	if( p->byAck != EMASTER_ACCEPT )
	{
		NET_SIS_make_master send;
		send.dwMaster = dw_sender;
		send.dwError = E_MakeMaster_Master_Refuse;
		p_prentice->SendMessage( &send, send.dw_size );
		return;
	}

	//添加新人
	BOOL bAddSuccess = add_new_prentice( dw_sender, p->dwPrentice);

	// 无论成功与否，都给徒弟发消息
	{
		NET_SIS_make_master send;
		send.dwMaster = dw_sender;
		send.dwError = bAddSuccess ? E_MakeMaster_Master_Suceessful : E_MakeMaster_Master_MaxPrentices;
		p_prentice->SendMessage(&send, send.dw_size);
	}
	
	if(!bAddSuccess) return;

	// 真正拜师成功，给师傅发消息
	{
		NET_SIS_make_master_result_to_master send;
		send.dwPrentice = p->dwPrentice;
		p_sender->SendMessage(&send, send.dw_size);
	}


	//放入更新列表
	save_list_.push_back( dw_sender );

	//gx add 2013.12.05
	//师徒关系建立后双方互相关注
	NET_SIC_role_make_friend send_friend;
	send_friend.dwDestRoleID = p->dwPrentice;
	g_socialMgr.AddEvent(dw_sender, EVT_MakeFriend, send_friend.dw_size, &send_friend);//师傅关注徒弟
	send_friend.dwDestRoleID = dw_sender;
	g_socialMgr.AddEvent(p->dwPrentice, EVT_MakeFriend, send_friend.dw_size, &send_friend);//徒弟关注师傅
	//end
	NET_DB2C_update_master_id_and_forbid_time send;
	send.dw_role_id = p->dwPrentice;
	send.dw_master = dw_sender;
	send.dw_forbid_time = 0;
	g_dbSession.Send(&send, send.dw_size );

	if( VALID_POINT(p_prentice->GetScript( )) )
		p_prentice->GetScript( )->OnMakeMaster( p_prentice, dw_sender, p->dwPrentice );

	p_prentice->GetAchievementMgr().UpdateAchievementCriteria(ete_make_master, 1);

}
VOID master_prentice_mgr::make_prentice(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_make_prentice* p = (NET_SIC_make_prentice*)p_message;

	Role* p_master = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT( p_master ) ) return;
	if( p_master->GetID( ) == p->dwPrentice ) return;

	if( p_master->get_master_prentice_forbid_time( ) > (DWORD)GetCurrentDWORDTime( ) )
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_You_ForbidOp;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	//达到收徒条件
	s_master_placard* p_master_placard = get_master_placard( dw_sender );
	if( !VALID_POINT(p_master_placard) )
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Your_OutOfLevel;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}
	//该玩家是否是师傅
	if (p_master->get_master_id())
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_NotMaster;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}
	if( p_master_placard->has_prentice(p->dwPrentice ))
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_HasMaster;
		p_master->SendMessage( &send, send.dw_size );
		return;	
	}

	//徒弟数量
	//if( p_master_placard->by_number >= g_PrenticeMaxNumber[p_master_placard->by_master_repution])
	if(p_master_placard->by_number >= MAX_PRENTICE)
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Your_MaxPrentices;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	//徒弟等级[10,60]
	Role* p_prentice = g_roleMgr.get_role( p->dwPrentice );
	if( !VALID_POINT(p_prentice) || p_prentice->get_level( )> PRENTICE_MAX_LEVEL || p_prentice->get_level() < PRENTICE_MIN_LEVEL)
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_OutOfLevel;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	//师徒两人必须在安全区内
	if (!p_prentice->IsInRoleState(ERS_SafeArea) || !p_master->IsInRoleState(ERS_SafeArea))
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MasterPrentice_OutSafeArea;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	if(p_master->get_level() <= p_prentice->get_level( ))
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_MaxOfLevel;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	if( p_prentice->is_graduates_from_master())
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_Has_Graduates;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	if( p_prentice->get_master_prentice_forbid_time( ) > (DWORD)GetCurrentDWORDTime( ) )
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_ForbidOp;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	if( p_prentice->get_master_id( ) )
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_Prentice_HasMaster;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}
	//徒弟正在处理收徒的请求
	if (p_prentice->GetShituAskID() != INVALID_VALUE)
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = p->dwPrentice;
		send.dwError = E_MakePrentice_PrenticeBusy;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}
	p_prentice->SetShituAskID(dw_sender);

	//给师傅一个信儿 gx add 2013.12.19
	NET_SIS_make_prentice send_master;
	send_master.dwPrentice = p->dwPrentice;
	send_master.dwError = E_MakePrentice_ToConfirm;
	p_master->SendMessage( &send_master, send_master.dw_size );

	NET_SIS_make_prentice_extend send;
	send.dwMaster = dw_sender;
	p_prentice->SendMessage( &send, send.dw_size );
}
VOID master_prentice_mgr::make_prentice_ex(DWORD dw_sender, VOID* p_message)
{
	NET_SIC_make_prentice_extend* p = (NET_SIC_make_prentice_extend*)p_message;

	Role* p_master = g_roleMgr.get_role( p->dwMaster );
	if( !VALID_POINT(p_master) ) return;

	Role* p_sender = g_roleMgr.get_role(dw_sender);
	if(!VALID_POINT(p_sender)) return;

	p_sender->SetShituAskID(INVALID_VALUE);
	
	if( p->byAck != EMASTER_ACCEPT )
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = dw_sender;
		send.dwError =  E_MakePrentice_Prentice_Refuse;
		p_master->SendMessage( &send, send.dw_size );
		return ;
	}

	if(VALID_POINT(p_sender->get_master_id()))
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = dw_sender;
		send.dwError = E_MakePrentice_Prentice_HasMaster;
		p_master->SendMessage( &send, send.dw_size );
		return;
	}

	//添加新人
	BOOL bAddSuccess = add_new_prentice( p->dwMaster, dw_sender );

	// 不论结果与否都给师傅发消息
	{
		NET_SIS_make_prentice send;
		send.dwPrentice = dw_sender;
		send.dwError = bAddSuccess ? E_MakePrentice_Prentice_Successful : E_MakePrentice_Your_MaxPrentices;
		p_master->SendMessage( &send, send.dw_size );
	}

	if(!bAddSuccess) return;

	// 真正收徒成功, 给徒弟发消息
	{
		NET_SIS_make_prentice_result_to_prentice send;
		send.dwMaster = p->dwMaster;
		p_sender->SendMessage(&send, send.dw_size);
	}

	//gx add 2013.12.05
	//师徒关系建立后双方互相关注
	NET_SIC_role_make_friend send_friend;
	send_friend.dwDestRoleID = p->dwMaster;
	g_socialMgr.AddEvent(dw_sender, EVT_MakeFriend, send_friend.dw_size, &send_friend);//师傅关注徒弟
	send_friend.dwDestRoleID = dw_sender;
	g_socialMgr.AddEvent(p->dwMaster, EVT_MakeFriend, send_friend.dw_size, &send_friend);//徒弟关注师傅
	//end

	//放入更新列表
	save_list_.push_back( p->dwMaster );

	NET_DB2C_update_master_id_and_forbid_time send;
	send.dw_role_id = dw_sender;
	send.dw_master = p->dwMaster;
	send.dw_forbid_time = 0;
	g_dbSession.Send(&send, send.dw_size );

	if( VALID_POINT(p_master->GetScript( )) )
		p_master->GetScript( )->OnMakeMaster( p_master, p->dwMaster, dw_sender );

	p_master->GetAchievementMgr().UpdateAchievementCriteria(ete_make_prentice, 1);

}

VOID master_prentice_mgr::master_prentice_break(DWORD dw_sender, VOID* p_message )
{
	//MGET_MSG(p,p_message,NET_SIC_master_prentice_break);
	NET_SIC_master_prentice_break* p = (NET_SIC_master_prentice_break*)p_message;

	//gx modify 2013.12.05
	/*Role* p_role = g_roleMgr.get_role( dw_sender );
	if( VALID_POINT( p_role ) && VALID_POINT( p_role->get_map( ) ) )
	{
		Creature *pNpc = p_role->get_map( )->find_creature( p->dwNpcID );
		if( !VALID_POINT( pNpc ) || !pNpc->IsFunctionNPC( EFNPCT_Master ))
		{
			NET_SIS_master_prentice_break_error send;
			send.dwNpcID = p->dwNpcID; send.dwError = E_MasterPrenticeBreak_Error_Npc;
			p_role->SendMessage( &send, send.dw_size );
			return ;
		}
	}*/

	//徒弟脱离师门
	if(!VALID_POINT(p->dw_role_id))
		prentice_breakwith_master( dw_sender );	
	
	//师傅撵走徒弟
	if( VALID_POINT(p->dw_role_id) )
		master_fire_prentice( dw_sender, p->dw_role_id );
}
VOID master_prentice_mgr::show_in_master_placard(DWORD dw_sender, VOID* p_message )
{
/*
 	NET_SIC_show_in_master_placard* p = (NET_SIC_show_in_master_placard*)p_message;
 
 	Role* p_role = g_roleMgr.get_role( dw_sender );
 	if( !VALID_POINT(p_role) ) return;
   
 	NET_SIS_show_in_master_placard send;
 	s_master_placard* pPlacard = get_master_placard( dw_sender );
 	if( !VALID_POINT(pPlacard) || pPlacard->dw_master != dw_sender )
 	{
 		send.dwError =  p->byFlag == EMASTER_SHOW ? E_ShowInPanel_Your_OutOfLevel : E_HideInPanel_You_NotRegister;
 		p_role->SendMessage(&send, send.dw_size);
 		return;
 	}
 
 	if( p->byFlag == pPlacard->b_show_in_panel )
 	{
 		send.dwError = pPlacard->b_show_in_panel == EMASTER_SHOW ?  E_ShowInPanel_Successful : E_HideInPanel_Successful;
 		p_role->SendMessage(&send, send.dw_size);
 		return;
 	}
 
 	pPlacard->b_show_in_panel = p->byFlag ;
 
 	send.dwError = pPlacard->b_show_in_panel == EMASTER_SHOW ?  E_ShowInPanel_Successful : E_HideInPanel_Successful;
 	p_role->SendMessage(&send, send.dw_size);
 
 	save_list_.push_back( dw_sender );
 
 	if(VALID_POINT(p_role->GetScript()))
 		p_role->GetScript()->OnJoinMasterRecruit(p_role);
*/
}
VOID master_prentice_mgr::get_master_placard(DWORD dw_sender, VOID* p_message )
{
/*
#define CalcRealSendSize( Number )\
	( sizeof(NET_SIS_get_master_placard)+sizeof(tagMasterPalcardSim)*((Number)-1) )

	Role* p_role = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT(p_role) )
		return;

	CREATE_MSG( p_send, MASTERPRENTICEPLACARDBUFF + sizeof(NET_SIS_get_master_placard), NET_SIS_get_master_placard);

	p_send->u16Number = 0;
	tagMasterPalcardSim* p_placard_sim =  p_send->stMPlacardSim;

	UINT32 u32_send_num = 0;
	s_master_placard* p_placard = NULL;
	master_placards_.reset_iterator( );
	while( master_placards_.find_next(p_placard))
	{
		if( p_send->u16Number < MASTERPRENTICEONTIMECOUNT)
		{
			if( p_placard->b_show_in_panel != EMASTER_SHOW ) continue;

			s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_placard->dw_master);
			if( !VALID_POINT(pRoleInfo))
			{
				ASSERT(FALSE);
				continue;
			}

			p_placard_sim->dwMaster = p_placard->dw_master;
			p_placard_sim->dwGraduates = p_placard->dw_graduates;
			p_placard_sim->eReputation = (EMasterReputation)p_placard->by_master_repution;

			p_placard_sim->bOnline = pRoleInfo->b_online_;
			p_placard_sim->eClassType = pRoleInfo->e_class_type_;
			p_placard_sim->byLevel = pRoleInfo->by_level;

			++p_send->u16Number; ++p_placard_sim; ++u32_send_num;
		}
		else
		{
			p_send->dw_size = CalcRealSendSize( p_send->u16Number );
			p_role->SendMessage( p_send, p_send->dw_size );

			p_send->u16Number = 0;
			p_placard_sim =  p_send->stMPlacardSim;

			if( !p_placard->b_show_in_panel ) continue;

			s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_placard->dw_master);
			if( !VALID_POINT(pRoleInfo))
			{
				ASSERT(FALSE);
				continue;
			}

			p_placard_sim->dwMaster = p_placard->dw_master;
			p_placard_sim->dwGraduates = p_placard->dw_graduates;
			p_placard_sim->eReputation = (EMasterReputation)p_placard->by_master_repution;

			p_placard_sim->bOnline = pRoleInfo->b_online_;
			p_placard_sim->eClassType = pRoleInfo->e_class_type_;
			p_placard_sim->byLevel = pRoleInfo->by_level;

			++p_send->u16Number; ++p_placard_sim; ++u32_send_num;
		}
	}

	if( p_send->u16Number || !u32_send_num )
	{
		p_send->dw_size = CalcRealSendSize( p_send->u16Number );
		p_role->SendMessage( p_send, p_send->dw_size );
	}
*/
}

VOID master_prentice_mgr::get_master_prentices(DWORD dw_sender, VOID* p_message )
{
	Role* p_dest = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT(p_dest ) ) return ;

	s_master_placard* p_placard = this->get_master_placard( dw_sender );
	s_master_placard*	 p_placard_brother = this->get_master_placard( p_dest->get_master_id( ) );

	if( VALID_POINT( p_placard )  || VALID_POINT(p_placard_brother))
	{
		if(VALID_POINT(p_placard))
		{
			send_members_to_one( p_dest, p_placard, dw_sender );
			if( p_placard->dw_master == dw_sender )
			{
				//gx modify 2013.12.12收到这条消息表明玩家是师傅,还是需要判断
				NET_SIS_master_moral_and_gradates send;
				/*send.dwMasterMoral = p_placard->dw_master_moral;
				send.dwGraduates	= p_placard->dw_graduates;*/
				if (p_dest->get_master_id())//针对60级以上但是死活不出师的那种人
				{
					send.dwMasterID = 0;
				}
				else
				{
					if (p_placard->by_number > 0)
					{
						send.dwMasterID = dw_sender;
					}
					else
					{
						send.dwMasterID = 0;
					}
				}
				p_dest->SendMessage(&send,send.dw_size);
			}
		}

		if(VALID_POINT(p_placard_brother))
		{
			send_members_to_one_ex( p_dest, p_placard_brother, dw_sender );
		}
	}
	else
	{
		NET_SIS_master_and_prentices send;
		send.byNumber = 0;
		send.dw_size -= sizeof(tagMPMember);
		p_dest->SendMessage(&send,send.dw_size);
	}
}
VOID master_prentice_mgr::call_in_master( DWORD dw_sender, VOID* p_message )
{
	Role* p_prentice = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT( p_prentice ) ) return;

	INT32 n32_item_count = p_prentice->GetItemMgr( ).GetBagSameItemCount( CALLINMASTERITEMTYPE );
	if( n32_item_count <= 0 )
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_No_Item;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	Map* p_map = p_prentice->get_map( );
	if(!VALID_POINT(p_map)) return;

	if( p_map->get_map_type() != EMT_Normal)
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_In_Intance;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(!p_map->can_use_item(CALLINMASTERITEMTYPE, 0)){
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_MapLimit;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	DWORD dw_master = p_prentice->get_master_id( );
	Role* p_master = g_roleMgr.get_role( dw_master );
	if( !VALID_POINT(dw_master) || !VALID_POINT(p_master))
	{
		NET_SIS_call_in_master send;
		send.dwError = dw_master ? E_CallInMaster_Master_Offline : E_CallInMaster_No_Master;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(p_prentice->IsInRoleState(ERS_PrisonArea))
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_You_In_Prison;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(p_master->IsInRoleState(ERS_PrisonArea))
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_In_Prison;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if (p_master->is_in_guild_war())
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_In_guild_war;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	{//转发给师傅
		NET_SIS_prentice_call_in send;
		send.dwPrentice = dw_sender;
		p_master->SendMessage(&send, send.dw_size );
	}
}
VOID master_prentice_mgr::prentice_call_in(DWORD dw_sender, VOID* p_message )
{
	NET_SIC_prentice_call_in* p = (NET_SIC_prentice_call_in*)p_message;

	s_master_placard* p_placard = get_master_placard( dw_sender );
	if( !VALID_POINT(p_placard) ) return;
	if(!p_placard->has_prentice( p->dwPrentice ) ) return;

	Role* p_master = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT(p_master ) )return;

	Role* p_prentice = g_roleMgr.get_role( p->dwPrentice );
	if( !VALID_POINT(p_prentice) ) return;

	INT32 n32_item_count = p_prentice->GetItemMgr( ).GetBagSameItemCount( CALLINMASTERITEMTYPE );
	if( n32_item_count <= 0 )
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_No_Item;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	Map* p_map = p_prentice->get_map( );
	if(!VALID_POINT(p_map) ) return;

	if(p_map->get_map_type() != EMT_Normal)
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_In_Intance;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(!p_map->can_use_item(CALLINMASTERITEMTYPE, 0)){
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_MapLimit;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(p_prentice->IsInRoleState(ERS_PrisonArea))
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_You_In_Prison;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	if(p_master->IsInRoleState(ERS_PrisonArea))
	{
		NET_SIS_call_in_master send;
		send.dwError = E_CallInMaster_In_Prison;
		p_prentice->SendMessage(&send, send.dw_size );
		return;
	}

	{//转发给徒弟
		NET_SIS_call_in_master send;
		send.dwError = p->byAck == EMASTER_REFUSE ? E_CallInMaster_Master_Refuse : E_CallInMaster_Success;
		p_prentice->SendMessage(&send,send.dw_size);
	}

	if( p->byAck == EMASTER_ACCEPT)
	{
		Map* p_map = p_prentice->get_map( );
		if( !VALID_POINT(p_map) )return ;

		package_list<tagItem*> s_list;
		INT32 n32_item_count = p_prentice->GetItemMgr( ).GetBagSameItemList(s_list, CALLINMASTERITEMTYPE, 1 );
		if( n32_item_count <= 0 || s_list.empty( ) )return;

		tagItem* p_item = s_list.pop_front( );
		if(!VALID_POINT(p_item)) return;

		p_prentice->GetItemMgr( ).DelFromBag(p_item->n64_serial, elcid_call_in_master, (INT16)1, FALSE);
	

		Vector3 pos = p_prentice->GetCurPos( );

		p_master->GotoNewMap(p_map->get_map_id( ), pos.x, pos.y, pos.z );
	}
}
VOID master_prentice_mgr::join_master_recruit(Role* p_role)
{
	if(!VALID_POINT(p_role))
		return ;

	if(VALID_POINT(p_role->get_master_id()))
		return ;

	if(find_role_in_recruit( p_role->GetID() ))
	{
		NET_SIS_join_master_recruit send;
		send.dw_error = E_JoinRecruit_AlreadyIn;
		p_role->SendMessage(&send, send.dw_size);
		return ;
	}

	if(p_role->get_level() > FUK_PRENTIC_CHECK_MAX)
	{
		NET_SIS_join_master_recruit send;
		send.dw_error = E_JoinRecruit_OutOfLevel;
		p_role->SendMessage(&send, send.dw_size);
		return ;
	}

	// 更新数据库
	NET_DB2C_join_master_recruit send_db;
	send_db.dw_role_id = p_role->GetID();
	g_dbSession.Send(&send_db, send_db.dw_size);

	master_recruit_.push_front(p_role->GetID());

	// 通知玩家
	NET_SIS_join_master_recruit send;
	send.dw_error = E_Success;
	p_role->SendMessage(&send, send.dw_size);

	// 脚本
	if(VALID_POINT(p_role->GetScript()))
		p_role->GetScript()->OnJoinMasterRecruit(p_role);
}
VOID master_prentice_mgr::leave_master_recruit(Role* p_role)
{
	if(!VALID_POINT(p_role))
		return ;

	if(!find_role_in_recruit( p_role->GetID() ))
	{
		NET_SIS_leave_master_recruit send;
		send.dw_error = E_LeaveRecruit_NotIn;
		p_role->SendMessage(&send, send.dw_size);
		return ;
	}

	// 通知数据库
	remove_role_in_recruit(p_role->GetID(), TRUE);

	// 通知玩家
	NET_SIS_leave_master_recruit send;
	send.dw_error = E_Success;
	p_role->SendMessage(&send, send.dw_size);
}
VOID master_prentice_mgr::query_page_master_recruit(Role* p_role, INT n_cur_page)
{
#define  ONEPAGERECRUITSIZE 10
#define  CalcMsgSize(Number) \
	(sizeof(NET_SIS_query_page_master_recruit) + (Number - 1)*sizeof(tagMasterRecruitSim))

	if(!VALID_POINT(p_role)) return;

	NET_SIS_query_page_master_recruit temp; temp.n_cur = 0;
	temp.b_register = FALSE;  temp.n_page = 1;temp.n_num = 0;
	if(master_recruit_.empty())
	{
		temp.dw_size -= sizeof(tagMasterRecruitSim);
		p_role->SendMessage(&temp, temp.dw_size);
		return ;
	}

	INT n_total_page = master_recruit_.size() / ONEPAGERECRUITSIZE;
	if((master_recruit_.size() % ONEPAGERECRUITSIZE) != 0) ++n_total_page;

	INT n_start_index = n_cur_page * ONEPAGERECRUITSIZE;
	if(n_start_index >= master_recruit_.size())
	{
		temp.n_cur = n_total_page - 1;
		n_start_index = temp.n_cur * ONEPAGERECRUITSIZE;
	}else temp.n_cur = n_cur_page; temp.n_page = n_total_page;

	INT n_msg_size = CalcMsgSize(ONEPAGERECRUITSIZE);
	CREATE_MSG(p_send, n_msg_size, NET_SIS_query_page_master_recruit);
	if(!VALID_POINT(p_send)) return ;

	p_send->n_num = 0;
	p_send->n_cur = temp.n_cur; 
	p_send->n_page = temp.n_page;
	p_send->dw_message_id = temp.dw_message_id;
	p_send->b_register = find_role_in_recruit(p_role->GetID());

	tagMasterRecruitSim* p_start = p_send->p_data;
	DWORD dwID = 0;  master_recruit_.reset_iterator();
	for(INT n = 0; n < n_start_index; ++n) master_recruit_.find_next(dwID);
	while(master_recruit_.find_next(dwID))
	{
		if(p_send->n_num >= ONEPAGERECRUITSIZE)
		{
			ASSERT(p_send->n_num <= ONEPAGERECRUITSIZE);

			p_send->dw_size = CalcMsgSize(p_send->n_num);
			p_role->SendMessage(p_send, p_send->dw_size);
			p_send->n_num = 0; break;
		}

		s_role_info* s_info = g_roleMgr.get_role_info(dwID);
		if(VALID_POINT(s_info))
		{
			p_start->dwRoleID = dwID;
			p_start->nLevel = s_info->by_level;
			p_start->bOnline = s_info->b_online_;
			p_start->eClassType = s_info->e_class_type_;
			++p_send->n_num; ++p_start;
		}
	}

	if(p_send->n_num > 0)
	{
		p_send->dw_size = CalcMsgSize(p_send->n_num);
		p_role->SendMessage(p_send, p_send->dw_size);
	}
}
VOID master_prentice_mgr::say_goodbye_to_master(Role* p_role, BYTE byAck)
{
	if(VALID_POINT(p_role))
		say_goodbye_to_master(p_role, byAck == 1);
}
//--------------------------------------------------------------------
// 师徒所有东西是在主线程调用的所以在脚本里做只读访问，没有线程问题
// 只在脚本中使用
//--------------------------------------------------------------------
BOOL master_prentice_mgr::has_prentice( DWORD dw_role_id ) 
{
	s_master_placard* p_placard = this->get_master_placard( dw_role_id );
	if( !VALID_POINT( p_placard ) ) return FALSE;
	return p_placard->by_number ? TRUE : FALSE;
}

BYTE master_prentice_mgr::get_prentice_num(DWORD dw_role_id)
{
	s_master_placard* p_placard = this->get_master_placard( dw_role_id );
	if( !VALID_POINT( p_placard ) ) return 0;
	return p_placard->by_number;
}
//--------------------------------------------------------------------
// 内部辅助函数
//--------------------------------------------------------------------
BOOL master_prentice_mgr::add_new_prentice(DWORD dw_master, DWORD dw_prentice)
{
	Role* p_prentice = g_roleMgr.get_role( dw_prentice );
	if(!VALID_POINT( p_prentice ) ) return FALSE;
	if(p_prentice->get_level() > PRENTICE_MAX_LEVEL) return FALSE;

	//再次检查
	s_master_placard* p_master_placard = this->get_master_placard( dw_master );
	/*if( !VALID_POINT(p_master_placard) || 
		p_master_placard->by_number >= g_PrenticeMaxNumber[p_master_placard->by_master_repution])*/
	if( !VALID_POINT(p_master_placard) || 
		p_master_placard->by_number >= MAX_PRENTICE)//gx modify
	{
		return FALSE;
	}

	if( p_master_placard->has_prentice(dw_prentice ) ) return FALSE;

	s_role_info* p_role_info = g_roleMgr.get_role_info(dw_prentice);
	if( !VALID_POINT(p_role_info) || VALID_POINT(p_prentice->get_master_id()))
	{
		ASSERT(FALSE);//逻辑错误
		return FALSE;
	}

	p_prentice->set_master_id( dw_master );

	//通知
	NET_SIS_new_prentice send;
	send.stMember.bOnline = p_role_info->b_online_;
	send.stMember.byLevel = p_role_info->by_level;
	send.stMember.dw_role_id = p_role_info->dw_role_id;
	send.stMember.eClass = p_role_info->e_class_type_;
	send.stMember.bySex = p_role_info->by_sex_;
	send.stMember.eMemberType = EMPMT_PRENTICE;
//	this->send_for_each( p_master_placard, &send, send.dw_size );
 	
		//师傅
		Role* p_master = g_roleMgr.get_role(p_master_placard->dw_master);
		if( VALID_POINT(p_master) ) p_master->SendMessage(&send, send.dw_size);

		send.stMember.eMemberType = EMPMT_BROTHER;
 		for( INT n = 0; n < p_master_placard->by_number; ++n )
 		{//所有徒弟
 			Role* p_member = g_roleMgr.get_role( p_master_placard->dw_prenices[n]);
 			if( VALID_POINT(p_member ) ) 
 				p_member->SendMessage(&send, send.dw_size);
 		}
 	
	this->send_members_to_one_ex( p_prentice, p_master_placard );

	p_master_placard->add_prentice( dw_prentice );
	
	//拜师成功后，通知师傅自己已是师傅 gx add 2013.12.20
	NET_SIS_master_moral_and_gradates send_master;
	if (p_master_placard->by_number > 0)
	{
		send_master.dwMasterID = dw_master;
		if (VALID_POINT(p_master))
		{
			p_master->SendMessage(&send_master,send_master.dw_size);
		}
	}
	//end
	p_master->GetAchievementMgr().UpdateAchievementCriteria(ete_have_prentice, p_master_placard->by_number);

	//remove_role_in_recruit(dw_prentice, TRUE);

	return TRUE;
}
VOID master_prentice_mgr::prentice_graduate( s_master_placard* p_master_placard, DWORD dw_prentice )
{
	if( !VALID_POINT(p_master_placard) ) return;
	if( !p_master_placard->has_prentice(dw_prentice ) ) return;

	NET_SIS_prentice_graduate send;
	send.dwPrentice = dw_prentice;
	this->send_for_each( p_master_placard, &send, send.dw_size );

	p_master_placard->remove_prentice( dw_prentice );
	p_master_placard->inc_graduate( );
	GetReputionOfGraduates(p_master_placard->dw_graduates, p_master_placard->by_master_repution );

	Role* p_master = g_roleMgr.get_role( p_master_placard->dw_master );
	if( VALID_POINT(p_master) )
	{
		p_master->GetAchievementMgr().UpdateAchievementCriteria(ete_graduates_num ,p_master_placard->dw_graduates);

		//gx modify 2013.12.05
		/*NET_SIS_master_moral_and_gradates send;
		send.dwMasterMoral = p_master_placard->dw_master_moral;
		send.dwGraduates	= p_master_placard->dw_graduates;
		p_master->SendMessage(&send,send.dw_size);*/
	}
}
VOID master_prentice_mgr::send_for_each( const s_master_placard* p_master_placard, LPVOID p_message, DWORD dw_size )
{
	if( !VALID_POINT(p_master_placard) ) return;

	for( INT n = 0; n < p_master_placard->by_number; ++n )
	{//所有徒弟
		Role* p_member = g_roleMgr.get_role( p_master_placard->dw_prenices[n]);
		if( VALID_POINT(p_member ) ) 
		{
			p_member->SendMessage(p_message, dw_size);
		}
	}

	//师傅
	Role* p_master = g_roleMgr.get_role(p_master_placard->dw_master);
	if( VALID_POINT(p_master) ) p_master->SendMessage(p_message, dw_size);
}

VOID master_prentice_mgr::send_members_to_one(Role* p_dest, const s_master_placard* p_master_placard, DWORD dw_except)
{
	if( !VALID_POINT(p_dest) || !VALID_POINT(p_master_placard))
		return;

	//通知新人所有资料，注意重设dwSzie
	CREATE_MSG( p_Send, MASTERPRENTICEONEBUFF, NET_SIS_master_and_prentices);
	tagMPMember* p_member = p_Send->stMember;  p_Send->byNumber = 0;
	for( INT n = 0; n < p_master_placard->by_number; ++n )
	{//所有师兄
		s_role_info* p_role_info = g_roleMgr.get_role_info(p_master_placard->dw_prenices[n]);
		if( VALID_POINT(p_role_info) && p_role_info->dw_role_id != dw_except )
		{
			p_member->byLevel = p_role_info->by_level;
			p_member->bOnline = p_role_info->b_online_;
			p_member->dw_role_id = p_role_info->dw_role_id;
			p_member->eClass = p_role_info->e_class_type_;
			p_member->bySex = p_role_info->by_sex_;
			p_member->eMemberType = EMPMT_PRENTICE;
			++p_member;++p_Send->byNumber;
		}
	}

	s_role_info* p_role_info = g_roleMgr.get_role_info( p_master_placard->dw_master );
	if( VALID_POINT(p_role_info ) && p_role_info->dw_role_id != dw_except )
	{//师傅资料
		p_member->byLevel = p_role_info->by_level;
		p_member->bOnline = p_role_info->b_online_;
		p_member->dw_role_id = p_role_info->dw_role_id;
		p_member->eClass = p_role_info->e_class_type_;
		p_member->bySex = p_role_info->by_sex_;
		p_member->eMemberType = EMPMT_MASTER;
		++p_member;++p_Send->byNumber;
	}

	p_Send->dw_size = sizeof(NET_SIS_master_and_prentices);
	p_Send->dw_size +=(INT32) (p_Send->byNumber * sizeof(tagMPMember) - sizeof(tagMPMember) );

	ASSERT( p_Send->dw_size < MASTERPRENTICEONEBUFF );
	p_dest->SendMessage( p_Send, p_Send->dw_size);
}
VOID master_prentice_mgr::send_members_to_one_ex(Role* p_dest, const s_master_placard* p_master_placard, DWORD dw_except)
{
	if( !VALID_POINT(p_dest) || !VALID_POINT(p_master_placard))
		return;

	//通知新人所有资料，注意重设dwSzie
	CREATE_MSG( p_Send, MASTERPRENTICEONEBUFF, NET_SIS_master_and_prentices);
	tagMPMember* p_member = p_Send->stMember;  p_Send->byNumber = 0;
	for( INT n = 0; n < p_master_placard->by_number; ++n )
	{//所有师兄
		s_role_info* p_role_info = g_roleMgr.get_role_info(p_master_placard->dw_prenices[n]);
		if( VALID_POINT(p_role_info) && p_role_info->dw_role_id != dw_except )
		{
			p_member->byLevel = p_role_info->by_level;
			p_member->bOnline = p_role_info->b_online_;
			p_member->dw_role_id = p_role_info->dw_role_id;
			p_member->eClass = p_role_info->e_class_type_;
			p_member->bySex = p_role_info->by_sex_;
			p_member->eMemberType = EMPMT_BROTHER;
			++p_member;++p_Send->byNumber;
		}
	}

	s_role_info* p_role_info = g_roleMgr.get_role_info( p_master_placard->dw_master );
	if( VALID_POINT(p_role_info ) && p_role_info->dw_role_id != dw_except )
	{//师傅资料
		p_member->byLevel = p_role_info->by_level;
		p_member->bOnline = p_role_info->b_online_;
		p_member->dw_role_id = p_role_info->dw_role_id;
		p_member->eClass = p_role_info->e_class_type_;
		p_member->bySex = p_role_info->by_sex_;
		p_member->eMemberType = EMPMT_MASTER;
		++p_member;++p_Send->byNumber;
	}

	p_Send->dw_size = sizeof(NET_SIS_master_and_prentices);
	p_Send->dw_size +=(INT32) (p_Send->byNumber * sizeof(tagMPMember) - sizeof(tagMPMember) );

	ASSERT( p_Send->dw_size < MASTERPRENTICEONEBUFF );
	p_dest->SendMessage( p_Send, p_Send->dw_size);
}
VOID master_prentice_mgr::prentice_breakwith_master( DWORD dw_prentice )
{
	Role* p_prentice = g_roleMgr.get_role( dw_prentice );
	if( !VALID_POINT(p_prentice) ) return;

	DWORD dw_master = p_prentice->get_master_id( );
	s_master_placard* p_master_placard = this->get_master_placard( dw_master );
	if( !VALID_POINT(p_master_placard) ) return;
	if( !p_master_placard->has_prentice( dw_prentice) ) return;

	{//0.广播孽徒走了
		NET_SIS_master_prentice_break send;
		send.dw_role_id = dw_prentice; 
		send.byMasterOP = FALSE;
		send_for_each( p_master_placard, &send, send.dw_size );
	}

	//1.删除这个孽徒
	p_prentice->set_master_id( INVALID_MASTER );
	p_prentice->set_master_prentice_forbid_time( );
	p_master_placard->remove_prentice( dw_prentice );
	p_prentice->GetAchievementMgr().UpdateAchievementCriteria(eta_prentice_breakwith_master, 1);
	if(VALID_POINT(p_prentice->GetScript( )))
		p_prentice->GetScript( )->OnMasterPrenticeBreak(p_prentice, dw_master, dw_prentice);

	//2.师傅在线设置禁止时间
	//gx modify 2013.12.10徒弟单方面的操作不影响师傅
	/*Role* p_master = g_roleMgr.get_role( dw_master );
	if( VALID_POINT(p_master) ) p_master->set_master_prentice_forbid_time( );	*/

	{//更新师傅ID和禁止操作时间
	 //3.更新孽徒
		NET_DB2C_update_master_id_and_forbid_time send;
		send.dw_role_id = dw_prentice;
		send.dw_master = INVALID_MASTER;
		send.dw_forbid_time = p_prentice->get_master_prentice_forbid_time( );
		g_dbSession.Send(&send, send.dw_size );

// 	//4.更新师傅//mwh 2011-12-09 不能更新师傅，因为现在师傅也能成为徒弟
// 		send.dw_role_id = dw_master;//时间和师傅ID一样
// 		g_dbSession.Send(&send, send.dw_size);
	}

	save_list_.push_back( p_master_placard->dw_master );
}
VOID master_prentice_mgr::master_fire_prentice(DWORD dw_master, DWORD dw_prentice)
{
	Role* p_master = g_roleMgr.get_role( dw_master );
	if( !VALID_POINT(p_master ) ) return;

	s_master_placard* p_master_placard = this->get_master_placard( dw_master );
	if( !VALID_POINT(p_master_placard) )return;
	if( p_master_placard->dw_master != dw_master ) return;
	if( !p_master_placard->has_prentice( dw_prentice ) ) return;


	{//0.发给所有人，包括孽徒
		NET_SIS_master_prentice_break send;
		send.dw_role_id = dw_prentice;
		send.byMasterOP = TRUE;
		send_for_each( p_master_placard, &send, send.dw_size );
	}

	//1.清除数据
	p_master_placard->remove_prentice( dw_prentice );
	p_master->set_master_prentice_forbid_time( );

	p_master->GetAchievementMgr().UpdateAchievementCriteria(eta_master_fire_prentice, 1);

	if(VALID_POINT(p_master->GetScript( )))
		p_master->GetScript( )->OnMasterPrenticeBreak(p_master, dw_master, dw_prentice);

	//2.如果徒弟在线，设置禁止时间
	Role* pPrentice = g_roleMgr.get_role( dw_prentice );
	if( VALID_POINT(pPrentice) )
	{
		//pPrentice->set_master_prentice_forbid_time( );//gx modify 2013.12.10不冻结徒弟
		pPrentice->set_master_id( INVALID_MASTER );
	}

	{//更新师傅ID和禁止操作时间
	//3.更新师傅//mwh 2011-12-09 不能更新师傅，因为现在师傅也能成为徒弟
		//gx modify 2013.12.10
		NET_DB2C_update_master_id_and_forbid_time send;
		send.dw_role_id = dw_master;//
		send.dw_master = INVALID_MASTER;
		send.dw_forbid_time = p_master->get_master_prentice_forbid_time( );
		g_dbSession.Send(&send, send.dw_size );

	//4.更新孽徒
		send.dw_role_id = dw_prentice;//时间和师傅ID一样
		if (VALID_POINT(pPrentice))//若徒弟在线
		{
			send.dw_forbid_time = pPrentice->get_master_prentice_forbid_time();
		}
		else
		{
			send.dw_forbid_time = (DWORD)GetCurrentDWORDTime();
		}
		g_dbSession.Send(&send, send.dw_size);
	}

	save_list_.push_back( p_master_placard->dw_master );
}

VOID master_prentice_mgr::delete_one_master_placard( DWORD dw_master )
{
	s_master_placard* p_master_placard = this->get_master_placard( dw_master );
	if( VALID_POINT(p_master_placard) ) SAFE_DELETE(p_master_placard);
	master_placards_.erase( dw_master );
}
BOOL master_prentice_mgr::find_role_in_recruit(DWORD dwRoleID)
{
	DWORD dwValue = 0;
	MasterRecruitIterator iter = master_recruit_.begin();
	while (master_recruit_.find_next(iter, dwValue))
		if(dwValue == dwRoleID) return TRUE;
	return FALSE;
}

int master_prentice_mgr::fill_prentice_list(DWORD dw_role_id, DWORD	(&dw_prenices)[5])
{
	s_master_placard* p_placard = this->get_master_placard( dw_role_id );
	if(!VALID_POINT(p_placard)) return 0;

	for(int n = 0; n < p_placard->by_number; ++n)
		dw_prenices[n] = p_placard->dw_prenices[n];

	return p_placard->by_number;
}

VOID master_prentice_mgr::remove_role_in_recruit(DWORD dwRoleID, BOOL bSndDB)
{
	master_recruit_.erase(dwRoleID);

	if(bSndDB)
	{
		NET_DB2C_leave_master_recruit send;
		send.dw_role_id = dwRoleID;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID master_prentice_mgr::say_goodbye_to_master(Role* p_role, bool b)
{
	s_master_placard* p_placard = get_master_placard( p_role->get_master_id( ) );
	if( VALID_POINT( p_placard ))
	{
		if(b)
		{
			this->prentice_graduate( p_placard, p_role->GetID( ));

			NET_DB2C_update_master_id_and_forbid_time send;
			send.dw_role_id = p_role->GetID( );
			send.dw_master = INVALID_MASTER;
			send.dw_forbid_time = 0;
			g_dbSession.Send(&send, send.dw_size );

			if(VALID_POINT(p_role->GetScript()))
				p_role->GetScript( )->OnSayGoodbyeToMaster(p_role, p_role->get_master_id( ), TRUE);

			p_role->inc_graduates_from_master( );
			p_role->set_master_id( INVALID_MASTER );
			p_role->GetAchievementMgr().UpdateAchievementCriteria(ete_chushi, 1);
			save_list_.push_back( p_placard->dw_master );
		}
		else if(VALID_POINT(p_role->GetScript()))
		{
			p_role->GetScript( )->OnSayGoodbyeToMaster(p_role, p_role->get_master_id( ), FALSE);
		}
	}
}

VOID master_prentice_mgr::master_teach_prentice( DWORD dw_sender,VOID* p_message )
{
	NET_SIC_Master_teach_Prentice* p = (NET_SIC_Master_teach_Prentice*)p_message;

	s_master_placard* p_placard = get_master_placard( dw_sender );
	if( !VALID_POINT(p_placard) ) return;
	if(!p_placard->has_prentice( p->dwPrentice ) ) return;

	Role* p_master = g_roleMgr.get_role( dw_sender );
	if( !VALID_POINT(p_master ) )return;

	NET_SIS_Master_teach_Prentice send;
	send.dw_error = E_Success;
	send.dwPrentice = p->dwPrentice;
	Role* p_prentice = g_roleMgr.get_role( p->dwPrentice );
	if( !VALID_POINT(p_prentice) ) 
	{
		send.dw_error = E_MasterTeach_Prentice_offline;
		goto Exit;
	}
	//徒弟今日已被传功
	if (p_prentice->GetDayClearData(ERDCT_Master_Practice) >= 1)
	{
		send.dw_error = E_MasterTeach_Prentice_full;
		goto Exit;
	}
	//条件判断成功，师傅可以给徒弟传功啦
	p_prentice->ModRoleDayClearDate(ERDCT_Master_Practice,1,FALSE);
	//经验计算根据公式
	//徒弟给经验，师傅给声望
	INT nShengWang = 0;
	INT nExpAdd = 0;
	nShengWang = 2*p_prentice->get_level();
	nExpAdd = 100000*p_prentice->get_level();
	p_master->ModAttValue(ERA_Knowledge, nShengWang);
	p_prentice->ExpChange(nExpAdd);
	send.dwAddExp = nExpAdd;
	if (VALID_POINT(p_prentice))
	{
		p_prentice->SendMessage(&send,send.dw_size);
	}
Exit:
	//无论错误与否，都给师傅发消息
	if (VALID_POINT(p_master))
	{
		p_master->SendMessage(&send, send.dw_size);
	}
}
//师傅上线给徒弟发消息，徒弟上线给师傅发消息
VOID master_prentice_mgr::send_login_to_fellow( Role *p_role )
{
	if (!VALID_POINT(p_role))
		return;
	s_master_placard* p_placard = get_master_placard(p_role->GetID());
	s_master_placard* p_placard_master = get_master_placard(p_role->get_master_id());
	if (VALID_POINT(p_placard) || VALID_POINT(p_placard_master))
	{
		if(VALID_POINT(p_placard))
		{
			if (p_placard->by_number > 0)//是师傅上线给徒弟发
			{
				NET_SIS_Master_Login send;
				this->send_for_each( p_placard, &send, send.dw_size );
			}
		}
		if (VALID_POINT(p_placard_master))//是徒弟上线给师傅发
		{
			Role* pMaster = g_roleMgr.get_role( p_placard_master->dw_master );
			if (VALID_POINT(pMaster))
			{
				NET_SIS_Prentice_Login send;
				send.dwPrentice = p_role->GetID();
				pMaster->SendMessage(&send,send.dw_size);
			}
		}
	}
	return;
}
//师傅下线给徒弟发消息，徒弟下线给师傅发消息
VOID master_prentice_mgr::send_logout_to_fellow( Role *p_role )
{
	if (!VALID_POINT(p_role))
		return;
	s_master_placard* p_placard = get_master_placard(p_role->GetID());
	s_master_placard* p_placard_master = get_master_placard(p_role->get_master_id());
	if (VALID_POINT(p_placard) || VALID_POINT(p_placard_master))
	{
		if(VALID_POINT(p_placard))
		{
			if (p_placard->by_number > 0)//是师傅下线给徒弟发
			{
				NET_SIS_Master_Logout send;
				this->send_for_each( p_placard, &send, send.dw_size );
			}
		}
		if (VALID_POINT(p_placard_master))//是徒弟下线给师傅发
		{
			Role* pMaster = g_roleMgr.get_role( p_placard_master->dw_master );
			if (VALID_POINT(pMaster))
			{
				NET_SIS_Prentice_Logout send;
				send.dwPrentice = p_role->GetID();
				pMaster->SendMessage(&send,send.dw_size);
			}
		}
	}
	return;
}
