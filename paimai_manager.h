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
*	@file		paimai_manager.h
*	@author		lc
*	@date		2011/03/16	initial
*	@version	0.0.1.0
*	@brief		拍卖管理器
*/

#ifndef PAIMAI_MANAGER
#define PAIMAI_MANAGER

#include "mutex.h"

class paimai;
struct NET_SIC_begin_paimai;
struct NET_DB2S_load_all_paimai;
struct NET_DB2S_load_paimai_item;
struct NET_SIC_cancel_paimai;
struct NET_SIC_jingpai;
struct NET_SIC_chaw_buy;
struct NET_SIC_paimai_query;

typedef package_map<DWORD, paimai*> MAP_PAIMAI;

class paimai_manager
{

public:
	paimai_manager(void);
	~paimai_manager(void);

public:

	VOID update();

	VOID destroy();

	// 设置拍卖最大编号
	VOID	set_max_id(DWORD dw_max_id_) { dw_max_id = dw_max_id_; }

	// 读取所有拍卖信息
	VOID	load_all_paimai_from_db(NET_DB2S_load_all_paimai* p_recv);

	//  读取拍卖物品
	VOID	load_paimai_item(NET_DB2S_load_paimai_item* p_recv);

	// 创建拍卖编号
	VOID	create_paimai_id(DWORD&	dw_paimai_id);

	// 创建拍卖物品
	DWORD	create_paimai(NET_SIC_begin_paimai* p_recv, Role* p_sell_role);

	// 取消拍卖
	DWORD	cancel_paimai(NET_SIC_cancel_paimai* p_recv, Role* p_sell_role);

	// 竞拍
	DWORD	jingpai(NET_SIC_jingpai* p_recv, Role* p_jing_role);

	// 一口价购买
	DWORD   chaw_buy(NET_SIC_chaw_buy* p_recv, Role* p_buy_role);

	// 通知客户端添加拍卖物品
	VOID	send_add_paimai_to_client(Role* p_sell_role, paimai* p_paimai);

	// 获取角色拍卖信息数量
	VOID	get_role_paimai_num(DWORD dw_role_id, INT& n_num, package_list<DWORD>& list);

	// 获取角色竞拍信息数量
	VOID	get_role_jingpai_num(DWORD dw_role_id, INT& n_num, package_list<DWORD>& list);

	// 拍卖信息查询
	VOID	query_paimai(Role* p_role, NET_SIC_paimai_query* p_recv);

	// 返回查询结果
	VOID	send_query_result(Role* p_role, INT n_page_num,	package_list<DWORD>& list_query_paimai);

	// 计算页数
	VOID	cal_page_num(INT n_size, INT& n_page);

	// 获取拍卖列表
	MAP_PAIMAI&		get_paimai_map() { return map_paimai; }

	VOID	set_init_ok(BOOL b) { b_init_ok = b; }

	FLOAT	EquipQualityChange(BYTE by_quality);

private:

	DWORD	dw_max_id;

	Mutex	m_mutex;

	MAP_PAIMAI	map_paimai;

	DWORD		dw_update_time;

	BOOL		b_init_ok;
};

extern paimai_manager g_paimai;

#endif
