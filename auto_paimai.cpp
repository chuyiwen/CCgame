#include "StdAfx.h"
#include "auto_paimai.h"
#include "db_session.h"
#include "paimai_manager.h"
#include "paimai.h"
#include "item_creator.h"
#include "role_mgr.h"

#include "../common/ServerDefine/role_data_server_define.h"

auto_paimai g_auto_paimai;

auto_paimai::auto_paimai(void)
{
	b_init_ok = FALSE;

	map_auto_paimai.clear();

	dw_update_time = AUTO_PAIMAI_TIME;
}

auto_paimai::~auto_paimai(void)
{
	MAP_AUTO_PAIMAI::map_iter iter = map_auto_paimai.begin();
	tagAutoPaimai* pAutoPaimai = NULL;
	while(map_auto_paimai.find_next(iter, pAutoPaimai))
	{
		if(VALID_POINT(pAutoPaimai))
		{
			SAFE_DELETE(pAutoPaimai);
		}
	}
	map_auto_paimai.clear();
}

VOID auto_paimai::reload_auto_paimai()
{
	b_init_ok = FALSE;

	MAP_AUTO_PAIMAI::map_iter iter = map_auto_paimai.begin();
	tagAutoPaimai* pAutoPaimai = NULL;
	while(map_auto_paimai.find_next(iter, pAutoPaimai))
	{
		if(VALID_POINT(pAutoPaimai))
		{
			SAFE_DELETE(pAutoPaimai);
		}
	}
	map_auto_paimai.clear();

	NET_S2DB_load_auto_paimai send;
	g_dbSession.Send(&send, send.dw_size);
}

VOID auto_paimai::Update()
{
	if(!b_init_ok)
		return;

	dw_update_time--;

	if(dw_update_time <= 0)
	{
		MAP_AUTO_PAIMAI::map_iter iter = map_auto_paimai.begin();
		tagAutoPaimai* pAutoPaimai = NULL;
		while(map_auto_paimai.find_next(iter, pAutoPaimai))
		{
			if(!VALID_POINT(pAutoPaimai))
				continue;

			if(pAutoPaimai->bSell)
				continue;
			
			if(pAutoPaimai->dwInventory <= 0)
				continue;

			if(!pAutoPaimai->bOnline)
				continue;

			create_auto_paimai(pAutoPaimai);
		}
		dw_update_time = AUTO_PAIMAI_TIME;
	}
}


// 获取自动拍卖数据
VOID auto_paimai::load_all_auto_paimai(NET_DB2S_load_auto_paimai* p_recv)
{
	if(p_recv->n_num <= 0)
		return;

	for(INT i = 0; i < p_recv->n_num; i++)
	{
		tagAutoPaimai* p = new tagAutoPaimai;
		memcpy(p, &p_recv->st_auto_paimai[i], sizeof(tagAutoPaimai));
		map_auto_paimai.add(p->dwID, p);
	}
}

// 检查是否已经在拍卖行中出售
VOID auto_paimai::check_is_paimai()
{
	MAP_AUTO_PAIMAI::map_iter iter = map_auto_paimai.begin();
	tagAutoPaimai* pAutoPaimai = NULL;
	while(map_auto_paimai.find_next(iter, pAutoPaimai))
	{
		if(!VALID_POINT(pAutoPaimai))
			continue;

		NET_S2DB_check_is_paimai send;
		send.dw_auto_paimai_id = pAutoPaimai->dwID;
		g_dbSession.Send(&send, send.dw_size);
	}

	NET_S2DB_auto_paimai_init_ok send_end;
	g_dbSession.Send(&send_end, send_end.dw_size);
}

// 检查是否已经在拍卖行中出售
VOID auto_paimai::check_is_paimai(DWORD dw_auto_paimai_id)
{
	tagAutoPaimai* pAutoPaimai = map_auto_paimai.find(dw_auto_paimai_id);
	if(!VALID_POINT(pAutoPaimai))
		return;

	NET_S2DB_check_is_paimai send;
	send.dw_auto_paimai_id = pAutoPaimai->dwID;
	g_dbSession.Send(&send, send.dw_size);
}

// 设置是否拍卖
VOID auto_paimai::set_is_paimai(DWORD dw_auto_paimai_id, BOOL b_have)
{
	tagAutoPaimai* pAutoPaimai = map_auto_paimai.find(dw_auto_paimai_id);
	if(!VALID_POINT(pAutoPaimai))
		return;

	pAutoPaimai->bSell = b_have;

	update_auto_paimai_to_db(pAutoPaimai);
}

// 创建拍卖物品
VOID auto_paimai::create_auto_paimai(tagAutoPaimai* p)
{
	s_role_info* pRoleInfo = g_roleMgr.get_role_info(p->dwSellRoleID);
	if(!VALID_POINT(pRoleInfo))
		return;

	DWORD dw_paimai_id = 0;
	g_paimai.create_paimai_id(dw_paimai_id);

	paimai* p_paimai = new paimai;
	if(!VALID_POINT(p_paimai))
		return;

	tagItem* pItem = ItemCreator::Create(EICM_Mail, NULL, p->dwItemID, 1, TRUE);
	if(VALID_POINT(pItem))
	{
		pItem->n16Num = p->dwSellNum;
		pItem->SetOwner(dw_paimai_id, INVALID_VALUE);
		pItem->eConType = EICT_PaiMai;
		pItem->n16Index = 0;
		if(p->bBind)
		{
			pItem->byBind = EBS_Bind;
		}
		if(MIsEquipment(pItem->dw_data_id))
		{
			NET_DB2C_new_equip send;
			get_fast_code()->memory_copy(&send.equip, pItem, SIZE_EQUIP);
			g_dbSession.Send(&send, send.dw_size);
		}
		else
		{
			NET_DB2C_new_item send;
			get_fast_code()->memory_copy(&send.item, pItem, SIZE_ITEM);
			g_dbSession.Send(&send, send.dw_size);
		}
			
		NET_SIC_begin_paimai st_begin_paimai;
		st_begin_paimai.dw_bidup_price = p->dwBidup;
		st_begin_paimai.dw_chaw_price = p->dwChaw;
		st_begin_paimai.by_time_type = p->byTimeType;
		st_begin_paimai.b_show_name = FALSE;

		p_paimai->init_att(&st_begin_paimai, p->dwSellRoleID, dw_paimai_id, pItem, p->dwID);

		g_paimai.get_paimai_map().add(p_paimai->get_att().dw_paimai_id, p_paimai);

		p->bSell = TRUE;
		p->dwInventory--;

		update_auto_paimai_to_db(p);
	}
}

VOID auto_paimai::update_auto_paimai_to_db(tagAutoPaimai* p)
{
	NET_S2DB_update_auto_paimai send;
	memcpy(&send.st_auto_paimai, p, sizeof(tagAutoPaimai));
	g_dbSession.Send(&send, send.dw_size);
}

VOID auto_paimai::reset_auto_paimai_inventory()
{
	MAP_AUTO_PAIMAI::map_iter iter = map_auto_paimai.begin();
	tagAutoPaimai* pAutoPaimai = NULL;
	while(map_auto_paimai.find_next(iter, pAutoPaimai))
	{
		if(!VALID_POINT(pAutoPaimai))
			continue;

		pAutoPaimai->dwInventory = pAutoPaimai->dwComplement;

		update_auto_paimai_to_db(pAutoPaimai);
	}
}
