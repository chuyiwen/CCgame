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
*	@file		bank_manager.h
*	@author		lc
*	@date		2011/03/21	initial
*	@version	0.0.1.0
*	@brief		钱庄
*/


#ifndef BANK_MANAGER
#define BANK_MANAGER

#include "mutex.h"
#include "role.h"
#include "../../common/WorldDefine/bank_protocol.h"

struct NET_DB2S_load_all_bank;

typedef package_map<DWORD, tag_bank*> MAP_BANK;

class bank_manager
{
public:
	bank_manager(void);
	~bank_manager(void);

public:
	VOID	update();

	VOID	destroy();
	// 设置钱庄最大编号
	VOID	set_max_id(DWORD dw_max_id_) { dw_max_id = dw_max_id_; }

	// 创建钱庄编号
	VOID	create_paimai_id(DWORD&	dw_bank_id);

	// 初始化钱庄信息
	VOID	load_all_bank_from_db(NET_DB2S_load_all_bank* p_recv);

	// 获取角色钱庄拍卖列表
	VOID	get_role_bank_paimai_list(DWORD dw_role_id, package_list<DWORD>& list_bank);

	// 开始钱庄竞拍
	DWORD	begin_bank_paimai(Role* p_sell_role, NET_SIC_begin_bank_paimai* p_recv);

	// 取消钱庄拍卖
	DWORD	cancel_bank_paimai(Role* p_sell_role, NET_SIC_cancel_role_bank_paimai* p_recv);

	// 开始竞拍
	DWORD	begin_bank_jing(Role* p_jing_role, NET_SIC_begin_bank_jing* p_recv);

	// 一口价购买
	DWORD	chaw_buy(Role* p_buy_role, NET_SIC_bank_chaw_buy* p_recv);

	// 查询钱庄信息
	DWORD	query_bank(Role* p_role, NET_SIC_query_bank* p_recv);

	// 返回查询结果
	VOID	send_query_result(Role* p_role, INT n_page_num,	BYTE by_type, package_list<DWORD>& list_query_bank);

	// 计算页数
	VOID	cal_page_num(INT n_size, INT& n_page);

	// 插入拍卖信息
	VOID	insert_bank_paimai(tag_bank* p_bank);

	MAP_BANK&	get_bank_map() { return map_bank;}

	// 元宝兑换
	DWORD	yuanbao_exchange(Role* pRole, BYTE byType);

	VOID	send_bank_log(tag_bank* pBank, DWORD dw_buy_id);

	DWORD	get_radio() { return dw_ratio; }

	VOID	cal_radio() { dw_ratio = ((FLOAT)dw_total_money/dw_total_yuanbao)*100;}

private:

	DWORD	dw_max_id;

	Mutex	m_mutex;

	MAP_BANK	map_bank;

	DWORD		dw_update_time;

	DWORD		dw_ratio;
	DWORD		dw_total_yuanbao;
	DWORD		dw_total_money;
};

extern bank_manager g_bankmgr;

#endif
