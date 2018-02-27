/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		role_mgr
 *	@author		mwh
 *	@date		2011/03/10	initial
 *	@version	0.0.1.0
 *	@brief		���������
*/

#include "StdAfx.h"
#include "role_mgr.h"
#include "role.h"
#include "player_session.h"
#include "group_mgr.h"
#include "social_mgr.h"
#include "famehall.h"
#include "TradeYuanBao.h"
#include "vip_stall.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "../common/ServerDefine/quest_server_define.h"
#include "../../common/WorldDefine/script_protocol.h"
#include "../../common/WorldDefine/mail_define.h"
#include "mail_mgr.h"
#include "pet_heti.h"
#include "pvp_mgr.h"
#include "map_creator.h"
#include "master_prentice_mgr.h"

// ������������
VOID local_role_mgr::reset()
{
	free_index_	= 0;
	max_used_index_	= -1;
	last_save_index_ = 0;
	free_number_ = MAX_PLAYER_NUM;
	
	for(INT i=0; i<MAX_PLAYER_NUM - 1; ++i)
	{
		role_nodes_[i].next_free = i + 1;
		role_nodes_[i].p_role = NULL;
	}

	role_nodes_[MAX_PLAYER_NUM - 1].next_free = INVALID_VALUE;
	role_nodes_[MAX_PLAYER_NUM - 1].p_role = NULL;
}

// ȡ����Ҫ�����Role
Role* local_role_mgr::get_save_role()
{
	if(max_used_index_ < 0)
		return NULL;
	
	INT save_index = last_save_index_;
	while(true)
	{
		if(save_index >= max_used_index_) save_index = 0;
		else ++save_index;

		if(role_nodes_[save_index].p_role != NULL)
		{
			last_save_index_ = save_index;
			break;
		}

		if(save_index == last_save_index_)
		{
			max_used_index_	= -1;
			last_save_index_ = 0;
			
			// �߼��д�
			if(free_number_ != MAX_PLAYER_NUM)
				print_message(_T("\n\n\n\n\n fatal error in local_role_mgr!!!!!\n\n"));

			return NULL;
		}
	}

	if(!role_nodes_[last_save_index_].p_role->IsNeedSave2DB())
		return NULL;

	return role_nodes_[last_save_index_].p_role;
}

//************************************
//  role_mgr::~role_mgr
//************************************
role_mgr::~role_mgr()
{
	destroy();
}

// ��ʼ��
BOOL role_mgr::init()
{
	local_role_mgr_.reset();
	return TRUE;
}

// ����
VOID role_mgr::destroy()
{
	// ��ס
	role_mutex_.Acquire();

	Role* p_role = NULL; INT index = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, index) )
	{
		p_role = local_role_mgr_.remove(index);
		Role::Delete(p_role);
	}
	role_id_ref_index_.clear();
	local_role_mgr_.reset();

	// ����
	role_mutex_.Release();

	//ɾ������role_info
	erase_all_role_info();
}

// ����һ��role_info
BOOL role_mgr::insert_role_info(const s_role_info* pInfo)
{
	role_info_mutex_.Acquire();
	if (!role_infos_.is_exist(pInfo->dw_role_id))
	{
		s_role_info* p_new = new s_role_info;
		if( !VALID_POINT(p_new) ) return FALSE;

		get_fast_code()->memory_copy(p_new, pInfo, sizeof(s_role_info));
		p_new->b_online_ = false;
		role_infos_.add(p_new->dw_role_id, p_new);
		namecrc_ref_role_id_.add(p_new->dw_role_name_crc, p_new->dw_role_id);
	}
	role_info_mutex_.Release();

	return TRUE;
}

// ����һ���������
Role* role_mgr::insert_online_role(DWORD dw_role_id, const s_role_data_load* p_data, PlayerSession* p_session, BOOL& first_in)
{
	Role* p_new = Role::Create(dw_role_id, p_data, p_session);
	if( !VALID_POINT(p_new) ) return NULL;

	Map* pMap = g_mapCreator.get_map(p_new->GetMapID(), INVALID_VALUE);
	if(VALID_POINT(pMap))
	{
		Role* pRole = pMap->stop_leave_prictice(dw_role_id);
		if(VALID_POINT(pRole))
		{
			if(VALID_POINT(pRole->GetScript()))
			{
				pRole->GetScript()->OnLeavePractice(pRole, pRole->GetPractice().GetLeavePraciticeTotalTime(), pRole->GetPractice().GetLeavePraciticeTotalLove(), 4);
			}
			pMap->remove_leave_pricitce_visible_tile(pRole);
			g_roleMgr.delete_role(pRole->GetID());
		}
	}

	role_mutex_.Acquire();

	INT index = local_role_mgr_.add(p_new);
	if(INVALID_VALUE == index)
	{
		print_message(_T("\n\n\n\n\n there is a fatal error in local_role_mgr!!!!!\n\n"));
		ASSERT(0);
		Role::Delete(p_new);
		return NULL;
	}

	role_id_ref_index_.add(p_new->GetID(), index);

	role_mutex_.Release();

	// �½�ɫ��һ�ε�½
	first_in = FALSE;
	if( INVALID_VALUE == p_new->GetOnlineTime() )
		first_in = TRUE;

	role_online(p_new, first_in);

	return p_new;
}

// (ɾ�����ʱ)ɾ��һ�����߽�ɫ��Ϣ
BOOL role_mgr::delete_role_info(const DWORD dw_role_id)
{
	role_info_mutex_.Acquire();

	s_role_info* p_info = role_infos_.find(dw_role_id);
	if( VALID_POINT(p_info) )
	{
		delete_friends(dw_role_id);
		g_VIPStall.RemoveRoleVIPStall(dw_role_id);
		role_infos_.erase(dw_role_id);
		namecrc_ref_role_id_.erase(p_info->dw_role_name_crc);
		SAFE_DELETE(p_info);
	}

	role_info_mutex_.Release();

	return TRUE;
}

// (����/����ѡ��)ɾ��һ���������
BOOL role_mgr::delete_role(const DWORD dw_role_id)
{
	role_mutex_.Acquire();

	INT nIndex = role_id_ref_index_.find(dw_role_id);
	Role* pRole = local_role_mgr_.get_role(nIndex);
	if( VALID_POINT(pRole) )
	{
		role_offline(pRole);
		local_role_mgr_.remove(nIndex);
		role_id_ref_index_.erase(dw_role_id);
		Role::Delete(pRole);
	}
	role_mutex_.Release();

	return TRUE;
}

// ÿ��tick����һ���������
VOID role_mgr::save_one_to_db()
{
	if(role_id_ref_index_.empty()) return;

	Role *p_role = local_role_mgr_.get_save_role();
	if(VALID_POINT(p_role)) p_role->SaveToDB();
}

// �ط�ʱ���������������
VOID role_mgr::save_all_role_to_db()
{
	Role* p_role = NULL; INT index = INVALID_VALUE;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, index) )
	{
		p_role = local_role_mgr_.get_role(index);
		if( VALID_POINT(p_role) )
		{
			p_role->SaveToDB();

			if(p_role->is_leave_pricitice() && !VALID_POINT(p_role->GetSession()))
			{
				p_role->GetScript()->OnLeavePractice(p_role, p_role->GetPractice().GetLeavePraciticeTotalTime(), p_role->GetPractice().GetLeavePraciticeTotalLove(), 5);
			}
		}
	}
}

// �������CRCֵ->���ID
DWORD role_mgr::get_role_id(const DWORD name_crc)
{
	return namecrc_ref_role_id_.find(name_crc);
}

// �������ID�õ��������
VOID role_mgr::get_role_name(const DWORD dw_role_id, LPTSTR sz_name)
{
	s_role_info* pRoleInfo = role_infos_.find(dw_role_id);
	if( VALID_POINT(pRoleInfo) )
		_tcsncpy(sz_name, pRoleInfo->sz_role_name, X_SHORT_NAME);
	else
		ZeroMemory(sz_name, X_SHORT_NAME * sizeof(TCHAR));
}

// ����RoleID�õ����ָ��
Role* role_mgr::get_role(const DWORD dw_role_id)
{
	return local_role_mgr_.get_role(role_id_ref_index_.find(dw_role_id));
}

// ���ȡ��һ��(force_one: ����õ�һ��)
Role* role_mgr::get_rand_role( BOOL force_one /*= FALSE*/ )
{
	if (force_one)
	{
		if (role_id_ref_index_.size() <= 0)
		{
			return NULL;
		}

		return local_role_mgr_.get_role(role_id_ref_index_.begin()->second);
	}
	INT	nMaxUsedIndex = local_role_mgr_.get_max_used_index();

	if(nMaxUsedIndex == 0)
		return (Role*)INVALID_VALUE;

	INT nRandIndex = get_tool()->tool_rand() % nMaxUsedIndex;

	return local_role_mgr_.get_role(nRandIndex);
}

struct __SendOp
{
public:
	__SendOp(PVOID p_message, DWORD dw_size)
	:msg_(p_message), size_(dw_size){}

public:
	VOID operator()(Unit* p_unit)
	{
		if (VALID_POINT(p_unit) && p_unit->IsRole())
		{
			Role* pRole = dynamic_cast<Role*>(p_unit);
			if (VALID_POINT(pRole)) pRole->SendMessage(msg_, size_);			
		}
	}

public:
	PVOID msg_;
	DWORD size_;
};

// �����������е�ͼ�ڵ���ҷ���Ϣ
VOID role_mgr::send_world_msg(LPVOID p_message, DWORD dw_size)
{
	this->for_each(__SendOp(p_message, dw_size));
}

// �ɵõ����������н�ɫ�ļ���Ϣ
s_role_info* role_mgr::get_role_info(const DWORD dw_role_id)
{
	s_role_info* pRoleInfo = role_infos_.find(dw_role_id);
	return pRoleInfo;	
}

// ɾ������role_info
VOID role_mgr::erase_all_role_info()
{
	role_info_mutex_.Acquire();

	s_role_info* p_info = NULL;
	RoleInfoMap::map_iter it2 = role_infos_.begin();
	while( role_infos_.find_next(it2, p_info) )
		SAFE_DELETE(p_info);

	role_infos_.clear();

	role_info_mutex_.Release();
}

// �����������ֵ
VOID role_mgr::reset_role_reputation( ECLanType clan_type, EReputationLevel reputation_level, tagDWORDTime dw_time )
{
	Role* pRole = NULL;	INT nIndex = INVALID_VALUE;
	
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) g_fameHall.RoleRepUpdate(pRole, clan_type);
	}
}

//ͨ������id��ȡ����
VOID role_mgr::get_role_name_by_name_id( const DWORD name_id, LPTSTR sz_name )
{
	// FIXME:: RoleID��NameID
	get_role_name(name_id, sz_name);
}

VOID role_mgr::get_role_signature_name(const DWORD dw_role_id, LPTSTR sz_signature_name)
{
	Role* pRole = get_role(dw_role_id);
	if (!VALID_POINT(pRole)) return;
	pRole->GetSignature(sz_signature_name);
}
// ���������ҵ��췢������
VOID role_mgr::clear_send_mail_number()
{
	Role* p_role = NULL; INT index = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, index) )
	{
		p_role = local_role_mgr_.get_role(index);
		if( VALID_POINT(p_role) ) p_role->ClearSendMainNum();
	}
}

// �����Ҹ�������
VOID role_mgr::delete_role_map_limit(INT type)
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) )
		{
			pRole->DelMapLimit(type);
			pRole->DelMapProcess(type);
		}
	}

	NET_DB2C_delete_role_map_limit send;
	send.n_type = type;
	g_dbSession.Send(&send, send.dw_size);

	NET_DB2C_delete_role_map_process send_process;
	send_process.n_type = type;
	g_dbSession.Send(&send_process, send_process.dw_size);
}

// �����ҹһ�����
VOID role_mgr::clear_role_hang_number()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) pRole->CleanHangNum();
	}

	NET_DB2C_clean_role_hang_num send;
	g_dbSession.Send(&send, send.dw_size);

}

// ����ֵ����
//VOID role_mgr::clear_role_luck()
//{
//	Role* pRole = NULL; INT nIndex = -1;
//	RoleMap::map_iter it = role_id_ref_index_.begin();
//	while( role_id_ref_index_.find_next(it, nIndex) )
//	{
//		pRole = local_role_mgr_.get_role(nIndex);
//		if( VALID_POINT(pRole) ) pRole->SetAttValue(ERA_Luck, 0);
//	}
//	
//	NET_C2DB_clean_role_luck send3;
//	g_dbSession.Send(&send3, send3.dw_size);
//
//}

VOID role_mgr::inc_circle_refresh_number()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ){
			pRole->IncCirleRefresh(CIRCLEQUESTFRESHNUMBERDELTA);
			pRole->SetCirlePerdayNumber(CIRCLEQUESTPERDAYNUMBER);
			pRole->SetCirleRefreshMax(CIRCLEQUEST_FRESH_NUMBERMAXDAY);
		}
	}

	NET_C2DB_UpdateCircleRefreshNumberAndQuestNum send;
	send.nDelta = CIRCLEQUESTFRESHNUMBERDELTA;
	send.nPerDayNum = CIRCLEQUESTPERDAYNUMBER;
	send.nRefreshMax = CIRCLEQUEST_FRESH_NUMBERMAXDAY;

	g_dbSession.Send(&send, send.dw_size);
}

// 0�����õ�����
VOID role_mgr::reset_role_data()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			//pRole->CleanHangNum();
			//pRole->zero_paimai_limit();
			//pRole->zero_bank_limit();
			//pRole->SetBaseAttValue(ERA_Luck, 0);
			//pRole->SetAttValue(ERA_Luck, 0);
			//pRole->ClearYuanBaoExchangeNum();
			//pRole->SetDestoryEquipCount(0);
			//pRole->zero_shop_exploits();
			pRole->ResetActive();
			//pRole->ResetGuildActive();
			pRole->ResetDayClearData();
			//pRole->ResetPerDayHangGetExpTime( );
			//pRole->ResetCooldownCD();
			pRole->CreateNewRoleGift();
			
		}
	}
	

	NET_DB2C_clean_role_hang_num send1;
	g_dbSession.Send(&send1, send1.dw_size);

	//NET_C2DB_CleanRolePaiMaiNum send2;
	//g_dbSession.Send(&send2, send2.dw_size);

	//NET_C2DB_clean_role_luck send3;
	//g_dbSession.Send(&send3, send3.dw_size);

	//NET_DB2C_clean_role_yuanbao_exchange_num send4;
	//g_dbSession.Send(&send4, send4.dw_size);

	//NET_C2DB_clean_role_bank_num send5;
	//g_dbSession.Send(&send5, send5.dw_size);

	//NET_C2DB_clean_role_destroy_equip_count send6;
	//g_dbSession.Send(&send6, send6.dw_size);

	//NET_C2DB_clean_role_exploits_num send7;
	//g_dbSession.Send(&send7, send7.dw_size);

	NET_C2DB_clean_role_active_data send8;
	g_dbSession.Send(&send8, send8.dw_size);

	NET_C2DB_clean_role_day send9;
	g_dbSession.Send(&send9, send9.dw_size);

	//NET_C2DB_reset_role_value_at_0 send10;
	//send10.nPerDayHangGetExpTimeMS = DAY_HANG_GETEXP_TIME_MS;
	//send10.nCoolDownReviveCD = COOLDOWN_REVIVE_CD;
	//g_dbSession.Send(&send10, send10.dw_size);
}
VOID role_mgr::reset_role_data_six()
{
	Role* pRole = NULL; INT nIndex = -1;
	//�ȿ������ߵ��û�
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->day_clear_circle_quest_done_times();//gx add 
		}
	}
	//�����ݿⷢ��Ϣֱ�����quest��quest_done���е�ÿ������
	NET_DB2C_clear_day_quest send;
	g_dbSession.Send(&send, send.dw_size);
}
//���ǩ����������
VOID role_mgr::update_role_sign_level()
{
	Role* pRole = NULL; INT nIndex = -1;
	//�ȿ������ߵ��û�
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->updateGodLevel();
		}
	}
	return;
}
// �����ҵ�����������
VOID role_mgr::clear_role_paimai_number()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) pRole->zero_paimai_limit();
	}

	NET_C2DB_CleanRolePaiMaiNum send;
	g_dbSession.Send(&send, send.dw_size);

}

VOID role_mgr::reset_sign_data()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->reset_sign_data();
			pRole->resetMianqianTime();
		}
	}

	NET_C2DB_clean_role_sign_data send;
	g_dbSession.Send(&send, send.dw_size);
}

// �����ҵ���Ǯׯ������
VOID role_mgr::clear_role_bank_number()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) pRole->zero_bank_limit();
	}

	NET_C2DB_clean_role_bank_num send;
	g_dbSession.Send(&send, send.dw_size);
}

// �������ÿ��
VOID role_mgr::update_1v1_day_score()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->get_1v1_score().n_day_max_score += 30;
			pRole->get_1v1_score().n_day_scroe_num = 0;
			pRole->Send1v1ScoreInfo();
		}
	}

	NET_C2DB_update_day_1v1_score send;
	g_dbSession.Send(&send, send.dw_size);
}

// �������1v1�ܻ���
VOID role_mgr::update_week_score()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->get_1v1_score().n_day_max_score = 80;
			pRole->get_1v1_score().n_day_scroe_num = 0;
			pRole->get_1v1_score().n_cur_score = 50;
			pRole->get_1v1_score().n16_score_award = 0;
			pRole->Send1v1ScoreInfo();
		}
	}

	NET_C2DB_update_week_1v1_score send;
	g_dbSession.Send(&send, send.dw_size);
}

// ��������ɻ�
VOID role_mgr::update_shihun()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->reset_shihun();
		}
	}

	NET_C2DB_update_shihun send;
	g_dbSession.Send(&send, send.dw_size);
}

// ���·���Ҽ��ʱ��
VOID role_mgr::reset_delay_time()
{
	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) ) 
		{
			pRole->reset_check_delay_time();
		}
	}
}

// �������
VOID role_mgr::role_online(Role *p_role, BOOL first_in)
{
	// ����ǵ�һ�����ߣ�����ó������ߺ���
	p_role->Online(first_in);

	// ����������ߵ��¼�
	g_socialMgr.send_login_to_friend(p_role);

	// �ѽ���֪ͨ��ż���� gx add 2013.10.29
	g_socialMgr.send_login_to_spouse(p_role);
	//ʦͽ�������� gx add 2013.12.13
	g_MasterPrenticeMgr.send_login_to_fellow(p_role);

	s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_role->GetID());

	if(VALID_POINT(pRoleInfo))
		pRoleInfo->b_online_ = true;
	//gx add vip ���
	p_role->CheckVip_OffLine();
}

// �������
VOID role_mgr::role_offline(Role* p_role)
{
	// ����Ϣ���罻ϵͳ
	g_socialMgr.send_logout_to_friend(p_role);

	// �ѽ���֪ͨ��ż���� gx add 2013.10.29
	g_socialMgr.send_loginout_to_spouse(p_role);

	// ����ʦͽ��ϵ��֪ͨ���� gx add 2013.12.13
	g_MasterPrenticeMgr.send_logout_to_fellow(p_role);
	
	// ȡ��������
	PetHeti::cancelHeti(p_role);
	PetHeti::cancelBeHeTiPet(p_role);

	g_pvp_mgr.role_offline(p_role);

	// �˳�����
	DWORD dwTeamID = p_role->GetTeamID();
	if( INVALID_VALUE != dwTeamID )
		g_groupMgr.RoleOutline(p_role);
	
	//���������˫��״̬�У�Ӧ��֪ͨ����˫�޵���ң�ȡ��˫��
	if (p_role->IsInRoleState(ERS_ComPractice))
	{
		p_role->UnsetRoleState(ERS_ComPractice,FALSE);
		DWORD dwPartner = p_role->GetComPracticePartner();
		Role* pPartnerRole = g_roleMgr.get_role(dwPartner);
		if(!VALID_POINT(pPartnerRole))
			return;
		p_role->SetComPracticePartner(INVALID_VALUE);
		pPartnerRole->UnsetRoleState(ERS_ComPractice);
		pPartnerRole->SetComPracticePartner(INVALID_VALUE);
	}
	//�������ʱ��Ӧ���һ��VIP��Ϣ gx add
	p_role->CheckVip_OffLine();
	s_role_info* pRoleInfo = g_roleMgr.get_role_info(p_role->GetID());

	if(VALID_POINT(pRoleInfo))
	{
		pRoleInfo->b_online_ = false;
		pRoleInfo->dw_time_last_logout_ = p_role->GetLogoutTime();
	}
}

// ɾ����ɫʱ��ɾ����ɫ����ϵͳ����
VOID role_mgr::delete_friends(DWORD dw_role_id)
{
	{// �ҵĺ���
		NET_DB2C_delete_all_friend	send;
		send.dw_role_id = dw_role_id;
		g_dbSession.Send(&send, send.dw_size);
	}

	{// ����Ϊ���ѵ���
		NET_DB2C_delete_all_friend_value	send;
		send.dw_role_id = dw_role_id;
		g_dbSession.Send(&send, send.dw_size);
	}

	{// ������
		NET_DB2C_delete_black send;
		send.dw_role_id = dw_role_id;
		send.dw_black_id_ = INVALID_VALUE;
		g_dbSession.Send(&send, send.dw_size);
	}

	{// ����
		NET_DB2C_delete_enemy send;
		send.dw_role_id = dw_role_id;
		send.dw_enemy_id_ = INVALID_VALUE;
		g_dbSession.Send(&send, send.dw_size);
	}	
}
// ������������Ҽ�BUFF(!!!!!!!ֻ�������̵߳��� !!!!!)
VOID role_mgr::add_buff_to_all(DWORD dw_buff)
{
	const tagBuffProto* p_proto = AttRes::GetInstance()->GetBuffProto(dw_buff);
	if( !VALID_POINT(p_proto) ) return;

	Role* pRole = NULL; INT nIndex = -1;
	RoleMap::map_iter it = role_id_ref_index_.begin();
	while( role_id_ref_index_.find_next(it, nIndex) )
	{
		pRole = local_role_mgr_.get_role(nIndex);
		if( VALID_POINT(pRole) )
			pRole->TryAddBuff(pRole, p_proto, NULL, NULL, NULL);
	}
}
// 3�����Ԫ��ֵ������ʱ��
struct _vigour_op
{
	VOID operator()(Unit* pUnit)
	{
		Role* p_role = dynamic_cast<Role*>(pUnit);
		if (VALID_POINT(p_role)) {
			p_role->send_vigour_reward();
			p_role->reset_vigour();
			p_role->set_today_online_tick(0);
		}
	}
};
VOID role_mgr::on_sharp_hour(INT hour)
{
	if( hour == 5)
	{
		// ��������ҷ�������������Ӧֵ
		this->for_each(_vigour_op());
		
		// ���ݿ�����
		NET_DB2C_reset_vigour send;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID role_mgr::create_system_mail()
{
	/*Role* p_role = get_rand_role(TRUE);

	if(!VALID_POINT(p_role))
		return;

	tagMailBase st_MailBase;
	ZeroMemory(&st_MailBase, sizeof(st_MailBase));
	st_MailBase.dwSendRoleID = INVALID_VALUE;
	st_MailBase.dwRecvRoleID = p_role->GetID();
	st_MailBase.dwSolve = 0;
	st_MailBase.dwGiveMoney = 0;
	DWORD dwItemType[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
	dwItemType[0] = 1100000;
	dwItemType[1] = 8000104;
	dwItemType[2] = 8000104;

	INT16 dwQlty[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
	dwQlty[0] = 0;
	dwQlty[1] = 0;
	dwQlty[2] = 0;
	g_mailmgr.CreateMail(st_MailBase, _T("�ʼ�"), _T("ѹ������"), dwItemType, dwQlty, 3);*/
}

role_mgr g_roleMgr;
