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
*	@file		bank_manager.cpp
*	@author		lc
*	@date		2011/03/21	initial
*	@version	0.0.1.0
*	@brief		钱庄
*/

#include "StdAfx.h"
#include "bank_manager.h"
#include "mail_mgr.h"
#include "role_mgr.h"
#include "role.h"
#include "../common/ServerDefine/bank_server_define.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/mail_define.h"

bank_manager g_bankmgr;

#define BANK_UPDATE_TIME 60000;
const DWORD	n_time[3] = {21600, 43200, 86400}; // 6, 12, 24小时

#define BANK_QUERY_NUM	10
#define BANK_CONTEXT	100

bank_manager::bank_manager(void)
:dw_max_id(0)
{
	dw_update_time = BANK_UPDATE_TIME;
	dw_ratio = 0;
	dw_total_yuanbao = 0;
	dw_total_money = 0;
}

bank_manager::~bank_manager(void)
{
}

VOID bank_manager::update()
{
	dw_update_time -= TICK_TIME;

	if(dw_update_time <= 0)
	{
		TCHAR	sz_context[BANK_CONTEXT];
		TCHAR	sz_role_name[X_SHORT_NAME];
		ZeroMemory(sz_role_name, sizeof(sz_role_name));
		ZeroMemory(sz_context, sizeof(sz_context));

		MAP_BANK::map_iter iter = map_bank.begin();
		tag_bank* p_bank = NULL;
		while(map_bank.find_next(iter, p_bank))
		{
			if(CalcTimeDiff(g_world.GetWorldTime(), p_bank->dw_begin_time) >= n_time[p_bank->by_time_type])
			{
				if(p_bank->dw_bidup_id == INVALID_VALUE)
				{
					tagMailBase st_mail_base;
					ZeroMemory(&st_mail_base, sizeof(st_mail_base));
					st_mail_base.dwSendRoleID = INVALID_VALUE;
					st_mail_base.dwRecvRoleID = p_bank->dw_sell_id;
					st_mail_base.byType = p_bank->by_type;
					st_mail_base.dwGiveMoney = p_bank->dw_sell;

					ZeroMemory(sz_context, sizeof(sz_context));
					_stprintf(sz_context, _T("%u_%d"), p_bank->dw_sell, p_bank->by_type);
					g_mailmgr.CreateMail(st_mail_base, _T("&FundFail&"), sz_context);
				}
				else
				{
					tagMailBase st_mail_base;
					ZeroMemory(&st_mail_base, sizeof(st_mail_base));
					st_mail_base.dwSendRoleID = INVALID_VALUE;
					st_mail_base.dwRecvRoleID = p_bank->dw_bidup_id;
					st_mail_base.byType = p_bank->by_type;
					st_mail_base.dwGiveMoney = p_bank->dw_sell;

					ZeroMemory(sz_context, sizeof(sz_context));
					INT	nType = 1;
					g_roleMgr.get_role_name(p_bank->dw_sell_id, sz_role_name);
					_stprintf(sz_context, _T("%u_%d_%d_%s_%u"), p_bank->dw_sell, p_bank->by_type, nType, sz_role_name, p_bank->dw_bidup);
					g_mailmgr.CreateMail(st_mail_base, _T("&FundBuy&"), sz_context);

					Role* p_bidup_role = g_roleMgr.get_role(p_bank->dw_bidup_id);
					if(VALID_POINT(p_bidup_role))
					{
						NET_SIS_bank_exchange_info send;
						memcpy(&send.st_bank, p_bank, sizeof(tag_bank));
						send.by_type = 0;
						p_bidup_role->SendMessage(&send, send.dw_size);
						p_bidup_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_buy_success, p_bank->by_type, p_bank->dw_sell);
					}

					ZeroMemory(&st_mail_base, sizeof(st_mail_base));
					st_mail_base.dwSendRoleID = INVALID_VALUE;
					st_mail_base.dwRecvRoleID = p_bank->dw_sell_id;
					st_mail_base.byType = !p_bank->by_type;
					st_mail_base.dwGiveMoney = p_bank->dw_bidup*(1-(FLOAT)AttRes::GetInstance()->GetVariableLen().n_paimai_duty/100);

					ZeroMemory(sz_context, sizeof(sz_context));
					g_roleMgr.get_role_name(p_bank->dw_bidup_id, sz_role_name);
					_stprintf(sz_context, _T("%u_%d_%s_%u_1"), p_bank->dw_sell, p_bank->by_type, sz_role_name, p_bank->dw_bidup);
					g_mailmgr.CreateMail(st_mail_base, _T("&FundSale&"), sz_context);

					send_bank_log(p_bank, p_bank->dw_bidup_id);

					if((p_bank->dw_sell < (p_bank->dw_bidup/10000)) && p_bank->by_type == 1)
					{
						dw_total_yuanbao += p_bank->dw_sell;
						dw_total_money += p_bank->dw_bidup/10000;
					}
				}

				Role* p_sell_role = g_roleMgr.get_role(p_bank->dw_id);
				if(VALID_POINT(p_sell_role))
				{
					NET_SIS_delete_role_bank_paimai send;
					send.dw_bank_id = p_bank->dw_id;
					p_sell_role->SendMessage(&send, send.dw_size);

					NET_SIS_bank_exchange_info send_info;
					memcpy(&send_info.st_bank, p_bank, sizeof(tag_bank));
					send_info.by_type = 1;
					p_sell_role->SendMessage(&send_info, send_info.dw_size);

					p_sell_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_sell_success, p_bank->by_type, p_bank->dw_sell);
				}

				NET_S2DB_delete_bank send;
				send.dw_bank_id = p_bank->dw_id;
				g_dbSession.Send(&send, send.dw_size);

				map_bank.erase(p_bank->dw_id);
				SAFE_DELETE(p_bank);
			}
		}

		dw_update_time = BANK_UPDATE_TIME;
	}
	
}

VOID bank_manager::destroy()
{
	MAP_BANK::map_iter iter = map_bank.begin();
	tag_bank* p_bank = NULL;
	while(map_bank.find_next(iter, p_bank))
	{
		if(!VALID_POINT(p_bank))
			continue;
		
		SAFE_DELETE(p_bank);
	}
	map_bank.clear();
}

// 创建钱庄编号
VOID bank_manager::create_paimai_id(DWORD&	dw_bank_id)
{
	m_mutex.Acquire();
	dw_bank_id = ++dw_max_id;
	m_mutex.Release();
}

// 初始化钱庄信息
VOID bank_manager::load_all_bank_from_db(NET_DB2S_load_all_bank* p_recv)
{
	for(INT i = 0; i < p_recv->n_num; i++)
	{
		M_trans_pointer(p, p_recv->st_bank, tag_bank);

		tag_bank* p_bank = new tag_bank;
		if(VALID_POINT(p_bank))
		{
			get_fast_code()->memory_copy(p_bank, &p[i], sizeof(tag_bank));
		}

		map_bank.add(p_bank->dw_id, p_bank);
	}
}

// 插入拍卖信息
VOID bank_manager::insert_bank_paimai(tag_bank* p_bank)
{
	NET_S2DB_insert_bank send;
	get_fast_code()->memory_copy(&send.st_bank, p_bank, sizeof(tag_bank));
	g_dbSession.Send(&send, send.dw_size);
}

// 获取角色钱庄拍卖列表
VOID bank_manager::get_role_bank_paimai_list(DWORD dw_role_id, package_list<DWORD>& list_bank)
{
	MAP_BANK::map_iter iter = map_bank.begin();
	tag_bank* p_bank = NULL;
	while(map_bank.find_next(iter, p_bank))
	{
		if(!VALID_POINT(p_bank))
			continue;

		if(p_bank->dw_sell_id == dw_role_id)
		{
			list_bank.push_back(p_bank->dw_id);
		}
	}
}

// 开始钱庄竞拍
DWORD bank_manager::begin_bank_paimai(Role* p_sell_role, NET_SIC_begin_bank_paimai* p_recv)
{
	if(!VALID_POINT(p_sell_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	// 时间类型非法
	if(p_recv->by_time_type < 0 || p_recv->by_time_type > 2)
		return E_bank_timetype_limit;

	// 等级不足
	if(p_sell_role->get_level() < 15)
		return E_bank_LevelNotEnough;

	if(p_sell_role->get_bank_limit() > 20)
		return E_bank_num_limit;

	// 卖出游戏币
	if(!p_recv->by_type)
	{
		if(p_recv->dw_sell < MIN_BANK_MONEY || p_recv->dw_sell > MAX_BANK_MONEY)
			return E_bank_sell_num_limit;

		// 一口价出价超过极限
		if((p_recv->dw_chaw > 0 && p_recv->dw_chaw < MIN_BANK_YUANBAO) || p_recv->dw_chaw < 0 || p_recv->dw_chaw > MAX_BANK_YUANBAO)
			return E_bank_Chaw_Limit;

		// 竞拍价格超过一口价
		if(p_recv->dw_chaw > 0 && (p_recv->dw_bidup < MIN_BANK_YUANBAO || p_recv->dw_bidup > p_recv->dw_chaw))
			return E_bank_bidup_Limit;

		// 保管费不足
		if(p_sell_role->GetCurMgr().GetBagSilver() < n_keeping[p_recv->by_time_type] + p_recv->dw_sell)
			return E_bank_Keeping_NotEnough;
	}
	else	// 卖出元宝
	{
		if(p_recv->dw_sell < MIN_BANK_YUANBAO || p_recv->dw_sell > MAX_BANK_YUANBAO)
			return E_bank_sell_num_limit;

		if((p_recv->dw_chaw > 0 &&p_recv->dw_chaw < MIN_BANK_MONEY) || p_recv->dw_chaw < 0 || p_recv->dw_chaw > MAX_BANK_MONEY)
			return E_bank_Chaw_Limit;

		// 竞拍价格超过一口价
		if(p_recv->dw_chaw > 0 && (p_recv->dw_bidup < MIN_BANK_MONEY || p_recv->dw_bidup > p_recv->dw_chaw))
			return E_bank_bidup_Limit;

		// 保管费不足
		if(p_sell_role->GetCurMgr().GetBagSilver() < n_keeping[p_recv->by_time_type])
			return E_bank_Keeping_NotEnough;
		// 元宝不足
		if(p_sell_role->GetCurMgr().GetBaiBaoYuanBao() < p_recv->dw_sell)
			return E_bank_yuanbao_not_enough;

	}

	DWORD dw_bank_id = 0;
	create_paimai_id(dw_bank_id);

	tag_bank* p_bank = new tag_bank;
	if(!VALID_POINT(p_bank))
		return INVALID_VALUE;

	p_bank->dw_id = dw_bank_id;
	p_bank->by_time_type = p_recv->by_time_type;
	p_bank->by_type = p_recv->by_type;
	p_bank->dw_begin_time = g_world.GetWorldTime();
	p_bank->dw_bidup = p_recv->dw_bidup;
	p_bank->dw_chaw = p_recv->dw_chaw;
	p_bank->dw_sell = p_recv->dw_sell;
	p_bank->dw_sell_id = p_sell_role->GetID();

	// 卖出游戏币
	if(!p_bank->by_type)
	{
		// 扣除保管费+卖出游戏币
		p_sell_role->GetCurMgr().DecBagSilver(n_keeping[p_recv->by_time_type] + p_recv->dw_sell, elcid_bank, p_bank->dw_id);
	}
	else	// 卖出元宝
	{
		// 扣除保管费
		p_sell_role->GetCurMgr().DecBagSilver(n_keeping[p_recv->by_time_type], elcid_bank, p_bank->dw_id);
		// 扣除卖出元宝
		p_sell_role->GetCurMgr().DecBaiBaoYuanBao(p_recv->dw_sell, elcid_bank, p_bank->dw_id);
	}

	map_bank.add(p_bank->dw_id, p_bank);

	insert_bank_paimai(p_bank);

	p_sell_role->inc_bank_limit();

	NET_SIS_add_role_bank_paimai send;
	get_fast_code()->memory_copy(&send.st_bank, p_bank, sizeof(tag_bank));
	p_sell_role->SendMessage(&send, send.dw_size);

	return E_Success;
}

// 取消钱庄拍卖
DWORD bank_manager::cancel_bank_paimai(Role* p_sell_role, NET_SIC_cancel_role_bank_paimai* p_recv)
{
	if(!VALID_POINT(p_sell_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	tag_bank* p_bank = map_bank.find(p_recv->dw_bank_id);
	if(!VALID_POINT(p_bank))
		return INVALID_VALUE;

	if(p_sell_role->GetID() != p_bank->dw_sell_id)
		return E_bank_not_sell_role;

	TCHAR	sz_context[BANK_CONTEXT];
	TCHAR	sz_role_name[X_SHORT_NAME];
	ZeroMemory(sz_context, sizeof(sz_context));
	ZeroMemory(sz_role_name, sizeof(sz_role_name));

	if(p_bank->dw_bidup_id != INVALID_VALUE)
	{
		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = p_bank->dw_bidup_id;
		st_mail_base.byType = !p_bank->by_type;
		st_mail_base.dwGiveMoney = p_bank->dw_bidup;

		_stprintf(sz_context, _T("%u_%d_%u"), p_bank->dw_sell, p_bank->by_type, p_bank->dw_bidup);
		g_mailmgr.CreateMail(st_mail_base, _T("&FundRefund&"), sz_context);
	}

	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_bank->dw_sell_id;
	st_mail_base.byType = p_bank->by_type;
	st_mail_base.dwGiveMoney = p_bank->dw_sell;

	ZeroMemory(sz_context, sizeof(sz_context));
	_stprintf(sz_context, _T("%u_%d"), p_bank->dw_sell, p_bank->by_type);
	g_mailmgr.CreateMail(st_mail_base, _T("&FundCancel&"), sz_context);

	NET_SIS_delete_role_bank_paimai send;
	send.dw_bank_id = p_bank->dw_id;
	p_sell_role->SendMessage(&send, send.dw_size);

	NET_S2DB_delete_bank senddb;
	senddb.dw_bank_id = p_bank->dw_id;
	g_dbSession.Send(&senddb, senddb.dw_size);

	map_bank.erase(p_bank->dw_id);
	SAFE_DELETE(p_bank);

	return E_Success;
}

// 开始竞拍
DWORD bank_manager::begin_bank_jing(Role* p_jing_role, NET_SIC_begin_bank_jing* p_recv)
{
	if(!VALID_POINT(p_jing_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	tag_bank* p_bank = map_bank.find(p_recv->dw_bank_id);
	if(!VALID_POINT(p_bank))
		return INVALID_VALUE;

	if(p_jing_role->GetID() == p_bank->dw_bidup_id)
		return E_bank_bidup_repeat;

	if(p_jing_role->GetID() == p_bank->dw_sell_id)
		return E_bank_jing_limit;

	DWORD	dw_old_bidup = p_bank->dw_bidup;
	DWORD	dw_old_bidup_role = p_bank->dw_bidup_id;

	DWORD	dw_new_bidup = p_bank->dw_bidup * 1.05;

	TCHAR sz_context[BANK_CONTEXT];
	TCHAR sz_role_name[X_SHORT_NAME];
	ZeroMemory(sz_context, sizeof(sz_context));
	ZeroMemory(sz_role_name, sizeof(sz_role_name));

	// 卖出游戏币
	if(!p_bank->by_type)
	{
		if(p_jing_role->GetCurMgr().GetBaiBaoYuanBao() < dw_new_bidup)
			return E_bank_yuanbao_not_enough;
		p_jing_role->GetCurMgr().DecBaiBaoYuanBao(dw_new_bidup, elcid_bank, p_bank->dw_id);
	}
	else	// 卖出元宝
	{
		if(p_jing_role->GetCurMgr().GetBagSilver() < dw_new_bidup)
			return E_bank_money_not_enough;
		p_jing_role->GetCurMgr().DecBagSilver(dw_new_bidup, elcid_bank, p_bank->dw_id);
	}

	// 竞价失败退还竞价款
	if(dw_old_bidup_role != INVALID_VALUE)
	{
		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = dw_old_bidup_role;
		st_mail_base.byType = !p_bank->by_type;
		st_mail_base.dwGiveMoney = p_bank->dw_bidup;

		g_roleMgr.get_role_name(p_jing_role->GetID(), sz_role_name);
		_stprintf(sz_context, _T("%u_%d_%s_%u"), p_bank->dw_sell, p_bank->by_type, sz_role_name, p_bank->dw_bidup);
		g_mailmgr.CreateMail(st_mail_base, _T("&FundBeyond&"), sz_context);
	}

	p_bank->dw_bidup = dw_new_bidup;
	p_bank->dw_bidup_id = p_jing_role->GetID();


	NET_S2DB_update_bank send;
	get_fast_code()->memory_copy(&send.st_bank, p_bank, sizeof(tag_bank));
	g_dbSession.Send(&send, send.dw_size);

	Role* p_sell_role = g_roleMgr.get_role(p_bank->dw_sell_id);
	if(VALID_POINT(p_sell_role))
	{
		NET_SIS_update_bank_jing send;
		send.dw_new_bidup = p_bank->dw_bidup;
		send.dw_bank_id = p_bank->dw_id;
		p_sell_role->SendMessage(&send, send.dw_size);
	}
	
	p_jing_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_jingpai, 1);
	return E_Success;
}

// 一口价购买
DWORD bank_manager::chaw_buy(Role* p_buy_role, NET_SIC_bank_chaw_buy* p_recv)
{
	if(!VALID_POINT(p_buy_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	tag_bank* p_bank = map_bank.find(p_recv->dw_bank_id);
	if(!VALID_POINT(p_bank))
		return INVALID_VALUE;

	if(p_buy_role->GetID() == p_bank->dw_sell_id)
		return INVALID_VALUE;

	if(p_bank->dw_chaw == 0)
		return E_bank_only_jingpai;

	if(p_bank->dw_bidup > p_bank->dw_chaw)
		return E_bank_chaw_buy_limit;

	TCHAR	sz_context[BANK_CONTEXT];
	TCHAR	sz_role_name[X_SHORT_NAME];
	ZeroMemory(sz_context, sizeof(sz_context));
	ZeroMemory(sz_role_name, sizeof(sz_role_name));

	// 卖出游戏币
	if(!p_bank->by_type)
	{
		if(p_buy_role->GetCurMgr().GetBaiBaoYuanBao() < p_bank->dw_chaw)
			return E_bank_yuanbao_not_enough;

		p_buy_role->GetCurMgr().DecBaiBaoYuanBao(p_bank->dw_chaw, elcid_bank);

		p_buy_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_buy_success, p_bank->by_type, p_bank->dw_sell);
	}
	else	// 卖出元宝
	{
		if(p_buy_role->GetCurMgr().GetBagSilver() < p_bank->dw_chaw)
			return E_bank_money_not_enough;
		p_buy_role->GetCurMgr().DecBagSilver(p_bank->dw_chaw, elcid_bank);

		p_buy_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_buy_success, p_bank->by_type, p_bank->dw_sell);

		if(p_bank->dw_sell < (p_bank->dw_chaw/10000) && p_bank->by_type == 1)
		{
			dw_total_yuanbao += p_bank->dw_sell;
			dw_total_money += p_bank->dw_chaw/10000;
		}
	}

	if(p_bank->dw_bidup_id != INVALID_VALUE)
	{
		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = p_bank->dw_bidup_id;
		st_mail_base.byType = !p_bank->by_type;
		st_mail_base.dwGiveMoney = p_bank->dw_bidup;

		g_roleMgr.get_role_name(p_buy_role->GetID(), sz_role_name);
		_stprintf(sz_context, _T("%u_%d_%s_%u"), p_bank->dw_sell, p_bank->by_type, sz_role_name, p_bank->dw_bidup);
		g_mailmgr.CreateMail(st_mail_base, _T("&FundKill&"), sz_context);
	}

	Role* p_sell_role = g_roleMgr.get_role(p_bank->dw_sell_id);
	if(VALID_POINT(p_sell_role))
	{
		NET_SIS_delete_role_bank_paimai send;
		send.dw_bank_id = p_bank->dw_id;
		p_sell_role->SendMessage(&send, send.dw_size);

		NET_SIS_bank_exchange_info send_info;
		memcpy(&send_info.st_bank, p_bank, sizeof(tag_bank));
		send_info.by_type = 1;
		p_sell_role->SendMessage(&send_info, send_info.dw_size);

		p_sell_role->GetAchievementMgr().UpdateAchievementCriteria(eta_bank_sell_success, p_bank->by_type, p_bank->dw_sell);
	}

	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_bank->dw_sell_id;
	st_mail_base.byType = !p_bank->by_type;
	st_mail_base.dwGiveMoney = p_bank->dw_chaw*(1-((FLOAT)AttRes::GetInstance()->GetVariableLen().n_paimai_duty/100));

	ZeroMemory(sz_context, sizeof(sz_context));
	g_roleMgr.get_role_name(p_buy_role->GetID(), sz_role_name);
	_stprintf(sz_context, _T("%u_%d_%s_%u_0"), p_bank->dw_sell, p_bank->by_type, sz_role_name, p_bank->dw_chaw);
	g_mailmgr.CreateMail(st_mail_base, _T("&FundSale&"), sz_context);


	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_buy_role->GetID();
	st_mail_base.byType = p_bank->by_type;
	st_mail_base.dwGiveMoney = p_bank->dw_sell;

	ZeroMemory(sz_context, sizeof(sz_context));
	INT	nType = 0;
	g_roleMgr.get_role_name(p_bank->dw_sell_id, sz_role_name);
	_stprintf(sz_context, _T("%u_%d_%d_%s_%u"), p_bank->dw_sell, p_bank->by_type, nType, sz_role_name, p_bank->dw_chaw);
	g_mailmgr.CreateMail(st_mail_base, _T("&FundBuy&"), sz_context);

	
	NET_SIS_bank_exchange_info send;
	memcpy(&send.st_bank, p_bank, sizeof(tag_bank));
	send.by_type = 1;
	p_buy_role->SendMessage(&send, send.dw_size);


	NET_S2DB_delete_bank send1;
	send1.dw_bank_id = p_bank->dw_id;
	g_dbSession.Send(&send1, send1.dw_size);

	send_bank_log(p_bank, p_buy_role->GetID());

	map_bank.erase(p_bank->dw_id);
	SAFE_DELETE(p_bank);

	return E_Success;
}

// 查询钱庄信息
DWORD bank_manager::query_bank(Role* p_role, NET_SIC_query_bank* p_recv)
{
	if(!VALID_POINT(p_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	if(!p_recv->by_type)
		p_role->get_query_bank().clear();
	else
		p_role->get_query_bank_ex().clear();

	MAP_BANK::map_iter iter = map_bank.begin();
	tag_bank* p_bank = NULL;
	while(map_bank.find_next(iter, p_bank))
	{
		if(!VALID_POINT(p_bank))
			continue;

		if(p_role->GetID() == p_bank->dw_sell_id)
			continue;

		if(p_bank->by_type != p_recv->by_type)
			continue;

		if(!p_recv->by_type)
			p_role->get_query_bank().push_back(p_bank->dw_id);
		else
			p_role->get_query_bank_ex().push_back(p_bank->dw_id);
	}

	if(!p_recv->by_type)
		send_query_result(p_role, 0, p_recv->by_type, p_role->get_query_bank());
	else
		send_query_result(p_role, 0, p_recv->by_type, p_role->get_query_bank_ex());

	return E_Success;
}

// 返回查询结果
VOID bank_manager::send_query_result(Role* p_role, INT n_page_num,	BYTE by_type, package_list<DWORD>& list_query_bank)
{
	INT n_page = 0;

	cal_page_num(list_query_bank.size(), n_page);

	if(n_page_num < 0 || n_page_num > n_page)
		return;

	INT	n_message_size = sizeof(NET_SIS_query_bank) + (BANK_QUERY_NUM-1)*sizeof(tag_bank);
	CREATE_MSG(p_send, n_message_size, NET_SIS_query_bank);

	p_send->n_page_num = n_page;
	p_send->n_num = 0;
	p_send->by_type = by_type;

	package_list<DWORD>::list_iter iter = list_query_bank.begin();
	DWORD	dw_bank_id = INVALID_VALUE;
	INT		n_temp = 0;

	while(list_query_bank.find_next(iter, dw_bank_id))
	{
		if(n_page > 1)
		{
			if(n_temp < n_page_num*BANK_QUERY_NUM)
			{
				n_temp++;
				continue;
			}
		}

		tag_bank* p_bank = map_bank.find(dw_bank_id);
		if(!VALID_POINT(p_bank))
			continue;

		get_fast_code()->memory_copy(&p_send->st_bank[p_send->n_num], p_bank, sizeof(tag_bank));

		p_send->n_num++;

		if(p_send->n_num > BANK_QUERY_NUM)
			break;
	}

	if(p_send->n_num > 0)
	{
		p_send->dw_size = sizeof(NET_SIS_query_bank) + (p_send->n_num-1)*sizeof(tag_bank);
	}
	else
	{
		p_send->dw_size = sizeof(NET_SIS_query_bank);
	}
	p_role->SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
}

// 计算页数
VOID bank_manager::cal_page_num(INT n_size, INT& n_page)
{
	if(n_size <= 0)
	{
		n_page = 0;
		return;
	}

	n_page = n_size / BANK_QUERY_NUM;
	if(n_page <= 0)
	{
		if((n_size % BANK_QUERY_NUM) > 0)
		{
			n_page += 1;
			return;
		}
	}
	else
	{
		if((n_size % (n_page*BANK_QUERY_NUM)) > 0)
		{
			n_page += 1;
			return;
		}
	}
}

// 元宝兑换
DWORD bank_manager::yuanbao_exchange(Role* pRole, BYTE byType)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	if(byType < 0 && byType >= 4)
		return INVALID_VALUE;

	if(pRole->GetYuanbaoExchangeNum() >= 100)
		return E_bank_exchange_num_limit;

	if(pRole->GetCurMgr().GetBaiBaoYuanBao() < n_exchange[byType])
		return E_bank_exchange_not_enough;

	INT32 n32_num = n_exchange[byType] * 10000;

	pRole->GetCurMgr().DecBaiBaoYuanBao(n_exchange[byType], elcid_bank_exchange);
	pRole->GetCurMgr().IncBagSilver(n32_num, elcid_bank_exchange);

	//pRole->GetCurMgr().IncExchangeVolume(n_exchange[byType]/10, elcid_bank_exchange);

	pRole->GetYuanbaoExchangeNum()++;

	return E_Success;
}

VOID bank_manager::send_bank_log(tag_bank* pBank, DWORD dw_buy_id)
{
	NET_DB2C_log_bank send;

	send.s_log_bank.by_type = pBank->by_type;
	send.s_log_bank.dw_bidup = pBank->dw_bidup;
	send.s_log_bank.dw_bidup_id = pBank->dw_bidup_id;
	send.s_log_bank.dw_buy_id = dw_buy_id;
	send.s_log_bank.dw_chaw = pBank->dw_chaw;
	send.s_log_bank.dw_sell = pBank->dw_chaw;
	send.s_log_bank.dw_sell_id = pBank->dw_sell_id;

	g_dbSession.Send(&send, send.dw_size);
}


