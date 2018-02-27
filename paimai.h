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
*	@file		paimai.h
*	@author		lc
*	@date		2011/03/17	initial
*	@version	0.0.1.0
*	@brief		�����˻�
*/

#ifndef PAIMAI
#define PAIMAI

#include "../../common/WorldDefine/paimai_protocol.h"

struct tagItem;
struct NET_SIC_begin_paimai;
struct NET_DB2S_load_paimai_item;

class paimai
{
public:
	paimai(void);
	~paimai(void);

	BOOL	update();

	// ��ʼ����������
	VOID init_att(NET_SIC_begin_paimai* p_recv, DWORD dw_sell_id, DWORD dw_paimai_id, tagItem* p_item, DWORD dw_auto_paimai_id = INVALID_VALUE);

	VOID init_att(tag_paimai* p_paimai);

	// ��ȡ������Ʒ
	VOID	load_paimai_item(tagItem* pItem);

	tag_paimai&		get_att() { return m_att; }

	tagItem*		get_item() { return p_item; }

	// ������Ʒ��Ϣ
	VOID save_update_item_to_db(tagItem* p_item);

	// ����������Ϣ
	VOID insert_paimai_to_db();

	// ɾ��������¼
	VOID delete_paimai_to_db();

	// ����������¼
	VOID update_paimai_to_db();

	VOID send_paimai_log(DWORD dw_buy_id);

	VOID get_role_name(DWORD dw_role_id, LPTSTR sz_role_name, BOOL b_get = FALSE);

private:

	tag_paimai	m_att;

	tagItem*	p_item;
};
#endif
