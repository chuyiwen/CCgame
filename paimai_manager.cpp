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
*	@file		paimai_manager.cpp
*	@author		lc
*	@date		2011/03/16	initial
*	@version	0.0.1.0
*	@brief		拍卖管理器
*/

#include "StdAfx.h"
#include "paimai_manager.h"
#include "../../common/WorldDefine/paimai_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/paimai_server_define.h"
#include "../../common/WorldDefine/mail_define.h"
#include "../../common/WorldDefine/bank_protocol.h"

#include "role.h"
#include "paimai.h"
#include "mail_mgr.h"
#include "role_mgr.h"
#include "auto_paimai.h"

#define	PAIMAI_UPDATE_TIME  60000;
const INT32 QUERY_NUM		=   12;

paimai_manager::paimai_manager(void)
:dw_max_id(0),b_init_ok(FALSE)
{
	dw_update_time = PAIMAI_UPDATE_TIME;
}

paimai_manager::~paimai_manager(void)
{
}

VOID paimai_manager::update()
{
	if(!b_init_ok)
		return;

	dw_update_time -= TICK_TIME;

	if(dw_update_time <= 0)
	{
		MAP_PAIMAI::map_iter iter = map_paimai.begin();
		paimai* p_paimai = NULL;
		while(map_paimai.find_next(iter, p_paimai))
		{
			if(!VALID_POINT(p_paimai))
				continue;

			BOOL b = p_paimai->update();
			if(b)
			{
				p_paimai->delete_paimai_to_db();
				map_paimai.erase(p_paimai->get_att().dw_paimai_id);
				if(p_paimai->get_att().dw_auto_index != INVALID_VALUE)
				{
					g_auto_paimai.check_is_paimai(p_paimai->get_att().dw_auto_index);
				}
				SAFE_DELETE(p_paimai);
			}
		}

		dw_update_time = PAIMAI_UPDATE_TIME;
	}
}

VOID  paimai_manager::destroy()
{
	MAP_PAIMAI::map_iter iter = map_paimai.begin();
	paimai* p_paimai = NULL;
	while(map_paimai.find_next(iter, p_paimai))
	{
		tagItem* pItem = p_paimai->get_item();
		map_paimai.erase(p_paimai->get_att().dw_paimai_id);
		SAFE_DELETE(pItem);
		SAFE_DELETE(p_paimai);
	}
}

// 创建拍卖编号
VOID paimai_manager::create_paimai_id(DWORD&	dw_paimai_id)
{
	m_mutex.Acquire();
	dw_paimai_id = ++dw_max_id;
	m_mutex.Release();
}

// 创建拍卖物品
DWORD paimai_manager::create_paimai(NET_SIC_begin_paimai* p_recv, Role* p_sell_role)
{
	if(!VALID_POINT(p_sell_role) || !VALID_POINT(p_recv))
		return INVALID_VALUE;

	tagItem* pItem = p_sell_role->GetItemMgr().GetBagItem(p_recv->n64_item_id);
	if(!VALID_POINT(pItem))
		return E_PaiMai_Item_NotExist;

	if(p_recv->by_time_type < 0 || p_recv->by_time_type > 2)
		return E_PaiMai_TimeType_Limit;

	// 绑定物品不能出售
	if(pItem->IsBind())
		return E_PaiMai_Item_IsBind;

	// 物品使用过不能拍卖
	if(!MIsEquipment(pItem->dw_data_id))
	{
		if(pItem->nUseTimes > 0)
			return E_Item_BeUse;
	}
	
	// 等级不足
	if(p_sell_role->get_level() < 20)
		return E_PaiMai_LevelNotEnough;

	// 一口价出价超过极限
	if(p_recv->dw_chaw_price > MAX_PAIMAI_VOLUME_NUM)
		return E_PaiMai_Chaw_Limit;

	// 一口价非法
	if((p_recv->dw_chaw_price > 0 && p_recv->dw_chaw_price < 100) || p_recv->dw_chaw_price < 0)
		return E_PaiMai_chaw_limit;

	// 起拍价格非法
	if(p_recv->dw_bidup_price < 100)
		return E_PaiMai_bidup_limit;


	// 竞拍价格超过一口价
	if(p_recv->dw_chaw_price > 100 && p_recv->dw_bidup_price >= p_recv->dw_chaw_price)
		return E_PaiMai_bidup_Limit;


	// 保管费不足
	INT32 n32_keeping = 0;
	n32_keeping = n_keeping[p_recv->by_time_type]+p_recv->dw_bidup_price*f_keeping[p_recv->by_time_type];
	/*if(MIsEquipment(pItem->dw_data_id))
	{
		tagEquip* pEquip = (tagEquip*)pItem;
		n32_keeping = pEquip->pEquipProto->n_keeping * EquipQualityChange(pEquip->equipSpec.byQuality);
	}
	else
	{
		n32_keeping = pItem->pProtoType->n_keeping;
	}*/
	if(p_sell_role->GetCurMgr().GetBagSilver() < n32_keeping)
		return E_PaiMai_Keeping_NotEnough;

	// 拍卖次数限制
	if(p_sell_role->get_paimai_limit() >= 20)
		return E_PaiMai_Num_Limit;

	DWORD dw_paimai_id = 0;
	create_paimai_id(dw_paimai_id);

	paimai* p_paimai = new paimai;
	if(!VALID_POINT(p_paimai))
		return INVALID_VALUE;

	// 从包裹中拿出物品
	p_sell_role->GetItemMgr().TakeOutFromBag(p_recv->n64_item_id, elcid_paimai_item, FALSE);

	// 扣除保管费
	p_sell_role->GetCurMgr().DecBagSilver(n32_keeping, elcid_paimai_item, dw_paimai_id);


	p_paimai->init_att(p_recv, p_sell_role->GetID(), dw_paimai_id, pItem);

	map_paimai.add(p_paimai->get_att().dw_paimai_id, p_paimai);

	// 发送添加拍卖纪录消息
	send_add_paimai_to_client(p_sell_role, p_paimai);

	p_sell_role->inc_paimai_limit();

	return E_Success;
}

// 取消拍卖
DWORD paimai_manager::cancel_paimai(NET_SIC_cancel_paimai* p_recv, Role* p_sell_role)
{
	if(!VALID_POINT(p_recv) || !VALID_POINT(p_sell_role))
		return INVALID_VALUE;

	paimai* p_paimai = g_paimai.get_paimai_map().find(p_recv->dw_paimai_id);
	if(!VALID_POINT(p_paimai))
		return INVALID_VALUE;

	tagItem* p_item = p_paimai->get_item();
	if(!VALID_POINT(p_item))
		return INVALID_VALUE;

	if(p_sell_role->GetID() != p_paimai->get_att().dw_sell_id)
		return E_PaiMai_Not_paimai_Role;

	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_sell_role->GetID();
	tagItem* pItem[1] = {p_item};

	TCHAR sz_context[PAIMAI_CONTEXT];
	ZeroMemory(sz_context, sizeof(sz_context));

	BYTE	by_quality = 0;
	if(MIsEquipment(p_item->dw_data_id))
		by_quality = ((tagEquip*)p_item)->GetQuality();
	else
		by_quality = p_item->GetQuality();

	_stprintf(sz_context, _T("%u_%d_%d"), p_item->dw_data_id,p_item->n16Num, p_item->GetQuality());
	g_mailmgr.CreateMail(st_mail_base, _T("&VenItemback&"), sz_context, pItem);

	if(p_paimai->get_att().dw_bidup_id != INVALID_VALUE)
	{
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = p_paimai->get_att().dw_bidup_id;
		st_mail_base.dwGiveMoney = p_paimai->get_att().dw_bidup;
		ZeroMemory(sz_context, sizeof(sz_context));
		_stprintf(sz_context, _T("%u_%d_%u_%d"), p_item->dw_data_id,p_item->n16Num, p_paimai->get_att().dw_bidup, p_item->GetQuality());
		g_mailmgr.CreateMail(st_mail_base, _T("&VenMoneyback&"), sz_context);

		Role* pBidUp = g_roleMgr.get_role(p_paimai->get_att().dw_bidup_id);
		if(VALID_POINT(pBidUp))
		{
			NET_SIS_send_cancel_paimai send;
			send.dw_paimai_id = p_recv->dw_paimai_id;
			pBidUp->SendMessage(&send, send.dw_size);
		}
	}

	p_paimai->delete_paimai_to_db();
	g_paimai.get_paimai_map().erase(p_recv->dw_paimai_id);
	SAFE_DELETE(p_paimai);

	return E_Success;
}

// 竞拍
DWORD paimai_manager::jingpai(NET_SIC_jingpai* p_recv, Role* p_jing_role)
{
	if(!VALID_POINT(p_recv) || !VALID_POINT(p_jing_role))
		return INVALID_VALUE;

	paimai* p_paimai = g_paimai.get_paimai_map().find(p_recv->dw_paimai_id);
	if(!VALID_POINT(p_paimai))
		return INVALID_VALUE;

	tagItem* pItem = p_paimai->get_item();
	if(!VALID_POINT(pItem))
		return INVALID_VALUE;

	if(p_jing_role->GetID() == p_paimai->get_att().dw_sell_id)
		return INVALID_VALUE;

	if(p_jing_role->GetID() == p_paimai->get_att().dw_bidup_id)
		return E_PaiMai_bidup_repeat;

	DWORD dw_old_bidup_id = p_paimai->get_att().dw_bidup_id;
	DWORD dw_old_bidup = p_paimai->get_att().dw_bidup;

	// 计算最新竞拍价格
	DWORD dw_new_bidup = p_paimai->get_att().dw_bidup * 1.05;

	// 竞价款不足
	if(p_jing_role->GetCurMgr().GetBagSilver() < dw_new_bidup)
		return E_PaiMai_Money_Not_Enough;

	p_paimai->get_att().dw_bidup = dw_new_bidup;
	p_paimai->get_att().dw_bidup_id = p_jing_role->GetID();

	// 给拍卖者发送竞价变更消息
	Role* p_sell_role = g_roleMgr.get_role(p_paimai->get_att().dw_sell_id);
	if(VALID_POINT(p_sell_role))
	{
		NET_SIS_update_jingpai send;
		send.dw_bidup = dw_new_bidup;
		send.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
		p_sell_role->SendMessage(&send, send.dw_size);
	}

	if(dw_old_bidup_id != INVALID_VALUE)
	{
		Role* p_old_bidup_role = g_roleMgr.get_role(dw_old_bidup_id);
		if(VALID_POINT(p_old_bidup_role))
		{
			NET_SIS_delete_jingpai send;
			send.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
			p_old_bidup_role->SendMessage(&send, send.dw_size);
		}
		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = dw_old_bidup_id;
		st_mail_base.dwGiveMoney = dw_old_bidup;

		TCHAR sz_context[PAIMAI_CONTEXT];
		TCHAR sz_role_name[X_SHORT_NAME];
		ZeroMemory(sz_context, sizeof(sz_context));
		ZeroMemory(sz_role_name, sizeof(sz_role_name));

		BYTE	by_quality = 0;
		if(MIsEquipment(pItem->dw_data_id))
			by_quality = ((tagEquip*)pItem)->GetQuality();
		else
			by_quality = pItem->GetQuality();

		p_paimai->get_role_name(p_jing_role->GetID(), sz_role_name, TRUE);
		_stprintf(sz_context, _T("%u_%d_%s_%u_%d"), pItem->dw_data_id, pItem->n16Num, sz_role_name, dw_old_bidup, pItem->GetQuality());
		g_mailmgr.CreateMail(st_mail_base, _T("&VenBeyond&"), sz_context);
	}

	// 扣除竞价款
	p_jing_role->GetCurMgr().DecBagSilver(dw_new_bidup, elcid_paimai_item, p_paimai->get_att().dw_paimai_id);

	NET_SIS_add_jingpai send;
	send.st_paimai_info.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
	send.st_paimai_info.dw_begin_time = p_paimai->get_att().dw_beigin_time;
	send.st_paimai_info.dw_bidup_price = p_paimai->get_att().dw_bidup;
	send.st_paimai_info.dw_chaw_price = p_paimai->get_att().dw_chaw;
	send.st_paimai_info.dw_sell_id = p_paimai->get_att().dw_sell_id;
	send.st_paimai_info.b_show_name = p_paimai->get_att().b_show_name;

	if(MIsEquipment(pItem->pProtoType->dw_data_id))
	{
		tagEquip* pEquip = (tagEquip*)pItem;
		send.st_paimai_info.st_info.dw_data_id = pEquip->dw_data_id;
		send.st_paimai_info.st_info.byBind = pEquip->byBind;
		send.st_paimai_info.st_info.byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
		//send.st_paimai_info.st_info.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
		send.st_paimai_info.st_info.nUseTimes = pEquip->nUseTimes;
		send.st_paimai_info.st_info.byHoldNum = pEquip->equipSpec.byHoleNum;
		//send.st_paimai_info.st_info.n16MinDmg = pEquip->equipSpec.n16MinDmg;
		//send.st_paimai_info.st_info.n16MaxDmg = pEquip->equipSpec.n16MaxDmg;
		//send.st_paimai_info.st_info.n16Armor = pEquip->equipSpec.n16Armor;
		memcpy(send.st_paimai_info.st_info.EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
		memcpy(send.st_paimai_info.st_info.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);
	}
	else
	{
		send.st_paimai_info.st_info.dw_data_id = pItem->pProtoType->dw_data_id;
		send.st_paimai_info.st_info.n_num = pItem->n16Num;
	}

	p_jing_role->SendMessage(&send, send.dw_size);

	p_paimai->update_paimai_to_db();
	
	p_jing_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_jingpai, 1);

	return E_Success;
}

// 一口价购买
DWORD paimai_manager::chaw_buy(NET_SIC_chaw_buy* p_recv, Role* p_buy_role)
{
	if(!VALID_POINT(p_recv) || !VALID_POINT(p_buy_role))
		return INVALID_VALUE;

	paimai* p_paimai = map_paimai.find(p_recv->dw_paimai_id);
	if(!VALID_POINT(p_paimai))
		return INVALID_VALUE;

	tagItem* p_item = p_paimai->get_item();
	if(!VALID_POINT(p_item))
		return INVALID_VALUE;

	if(p_buy_role->GetID() == p_paimai->get_att().dw_sell_id)
		return INVALID_VALUE;

	if(p_paimai->get_att().dw_chaw == 0)
		return E_PaiMai_Only_JingPai;

	if(p_paimai->get_att().dw_bidup > p_paimai->get_att().dw_chaw)
		return E_PaiMai_chaw_buy_limit;

	if(p_buy_role->GetCurMgr().GetBagSilver() < p_paimai->get_att().dw_chaw)
		return E_PaiMai_Money_Not_Enough;

	// 通知拍卖者删除拍卖信息
	Role* p_sell_role = g_roleMgr.get_role(p_paimai->get_att().dw_sell_id);
	if(VALID_POINT(p_sell_role))
	{
		NET_SIS_delete_paimai send;
		send.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
		p_sell_role->SendMessage(&send, send.dw_size);

		p_sell_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_sell_success,1);
	}

	tagMailBase st_mail_base;
	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_paimai->get_att().dw_sell_id;
	st_mail_base.dwGiveMoney = p_paimai->get_att().dw_chaw * (1-((FLOAT)AttRes::GetInstance()->GetVariableLen().n_paimai_duty/100));

	TCHAR sz_role_name[X_SHORT_NAME];
	TCHAR sz_context[PAIMAI_CONTEXT];
	ZeroMemory(sz_context, sizeof(sz_context));
	ZeroMemory(sz_role_name, sizeof(sz_role_name));

	BYTE	by_quality = 0;
	if(MIsEquipment(p_item->dw_data_id))
		by_quality = ((tagEquip*)p_item)->GetQuality();
	else
		by_quality = p_item->GetQuality();

	p_paimai->get_role_name(p_buy_role->GetID(), sz_role_name, TRUE);
	_stprintf(sz_context, _T("%u_%d_%s_%u_0_%d"), p_item->dw_data_id, p_item->n16Num, sz_role_name, p_paimai->get_att().dw_chaw, p_item->GetQuality());

	g_mailmgr.CreateMail(st_mail_base, _T("&VenSell&"), sz_context);

	// 通知竞拍者删除竞拍信息
	if(p_paimai->get_att().dw_bidup_id != INVALID_VALUE)
	{
		Role* p_jing_role = g_roleMgr.get_role(p_paimai->get_att().dw_bidup_id);
		if(VALID_POINT(p_jing_role))
		{
			NET_SIS_delete_jingpai send;
			send.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
			p_jing_role->SendMessage(&send, send.dw_size);
		}

		tagMailBase st_mail_base;
		ZeroMemory(&st_mail_base, sizeof(st_mail_base));
		st_mail_base.dwSendRoleID = INVALID_VALUE;
		st_mail_base.dwRecvRoleID = p_paimai->get_att().dw_bidup_id;
		st_mail_base.dwGiveMoney = p_paimai->get_att().dw_bidup;

		ZeroMemory(sz_context, sizeof(sz_context));
		p_paimai->get_role_name(p_buy_role->GetID(), sz_role_name, TRUE);
		_stprintf(sz_context, _T("%u_%d_%s_%u_%d"), p_item->dw_data_id, p_item->n16Num, sz_role_name, p_paimai->get_att().dw_bidup, p_item->GetQuality());
		g_mailmgr.CreateMail(st_mail_base, _T("&VenRefund&"), sz_context);
	}
	
	p_buy_role->GetCurMgr().DecBagSilver(p_paimai->get_att().dw_chaw, elcid_paimai_item, p_paimai->get_att().dw_paimai_id);
	
	p_buy_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_buy_success,1);

	ZeroMemory(&st_mail_base, sizeof(st_mail_base));
	st_mail_base.dwSendRoleID = INVALID_VALUE;
	st_mail_base.dwRecvRoleID = p_buy_role->GetID();
	tagItem* pItem[1] = {p_item};

	ZeroMemory(sz_context, sizeof(sz_context));
	p_paimai->get_role_name(p_paimai->get_att().dw_sell_id, sz_role_name);
	_stprintf(sz_context, _T("%u_%d_%s_%u_0_%d"), p_item->dw_data_id, p_item->n16Num, sz_role_name, p_paimai->get_att().dw_chaw, p_item->GetQuality());
	g_mailmgr.CreateMail(st_mail_base, _T("&VenSuccess&"), sz_context, pItem);

	p_paimai->send_paimai_log(p_buy_role->GetID());

	p_paimai->delete_paimai_to_db();
	map_paimai.erase(p_paimai->get_att().dw_paimai_id);
	if(p_paimai->get_att().dw_auto_index != INVALID_VALUE)
	{
		g_auto_paimai.check_is_paimai(p_paimai->get_att().dw_auto_index);
	}
	SAFE_DELETE(p_paimai);

	return E_Success;
}

// 通知客户端添加拍卖物品
VOID paimai_manager::send_add_paimai_to_client(Role* p_sell_role, paimai* p_paimai)
{
	if(!VALID_POINT(p_paimai))
		return; 

	tagItem* pItem = p_paimai->get_item();
	if(!VALID_POINT(pItem))
		return;

	NET_SIS_add_paimai_info send;
	send.dw_paimai_id = p_paimai->get_att().dw_paimai_id;
	send.dw_bidup_price = p_paimai->get_att().dw_bidup;
	send.dw_chaw_price = p_paimai->get_att().dw_chaw;
	send.dw_begin_time = p_paimai->get_att().dw_beigin_time;
	send.b_show_name = p_paimai->get_att().b_show_name;
	send.by_time_type = p_paimai->get_att().by_time_type;

	if(MIsEquipment(pItem->pProtoType->dw_data_id))
	{
		tagEquip* pEquip = (tagEquip*)pItem;
		send.st_info.dw_data_id = pEquip->dw_data_id;
		send.st_info.byBind = pEquip->byBind;
		send.st_info.byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
		//send.st_info.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
		send.st_info.nUseTimes = pEquip->nUseTimes;
		send.st_info.byHoldNum = pEquip->equipSpec.byHoleNum;
		//send.st_info.n16MinDmg = pEquip->equipSpec.n16MinDmg;
		//send.st_info.n16MaxDmg = pEquip->equipSpec.n16MaxDmg;
		//send.st_info.n16Armor = pEquip->equipSpec.n16Armor;
		memcpy(send.st_info.EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
		memcpy(send.st_info.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);
	}
	else
	{
		send.st_info.dw_data_id = pItem->pProtoType->dw_data_id;
		send.st_info.n_num = pItem->n16Num;
	}
	p_sell_role->SendMessage(&send, send.dw_size);
}

// 读取所有拍卖信息
VOID paimai_manager::load_all_paimai_from_db(NET_DB2S_load_all_paimai* p_recv)
{
	for(INT i = 0; i < p_recv->n_num; i++)
	{
		M_trans_pointer(p, p_recv->st_paimai, tag_paimai);

		paimai* p_paimai = new paimai;
		if(VALID_POINT(p_paimai))
		{
			p_paimai->init_att(&p[i]);
		}

		map_paimai.add(p_paimai->get_att().dw_paimai_id, p_paimai);
	}
}

//  读取拍卖物品
VOID paimai_manager::load_paimai_item(NET_DB2S_load_paimai_item* p_recv)
{
	INT32 nItemSize		= sizeof(tagItem);
	INT32 nEquipSize	= sizeof(tagEquip);

	DWORD dw_error_code = INVALID_VALUE;
	const tagItem	*pTmpItem	= NULL;
	tagItem			*pNewItem	= NULL;

	pTmpItem = (const tagItem *)p_recv->by_data_;

	for(INT i = 0; i < p_recv->n_count; i++)
	{
		if(!MIsEquipment(pTmpItem->dw_data_id))
		{
			pNewItem = new tagItem;
			get_fast_code()->memory_copy(pNewItem, pTmpItem, nItemSize);
			pNewItem->pProtoType = AttRes::GetInstance()->GetItemProto(pTmpItem->dw_data_id);

			pTmpItem = (const tagItem*)((BYTE*)pTmpItem + nItemSize);
		}
		else
		{
			pNewItem = new tagEquip;
			get_fast_code()->memory_copy(pNewItem, pTmpItem, nEquipSize);
			pNewItem->pProtoType = AttRes::GetInstance()->GetEquipProto(pTmpItem->dw_data_id);

			pTmpItem = (tagEquip*)((BYTE*)pTmpItem + nEquipSize);

			tagEquip* pEquip = (tagEquip*)pNewItem;

			//开光属性的异常数据恢复
			//if (pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Null && 
			//	pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Physique)
			//{
			//	pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
			//	pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
			//}

			//if (pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt == ERA_Physique)
			//{
			//	if (pEquip->equipSpec.byAddTalentPoint == 0 || 
			//		pEquip->equipSpec.EquipAttitionalAtt[7].nValue < 0 || 
			//		pEquip->equipSpec.EquipAttitionalAtt[7].nValue > 30)
			//	{
			//		pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
			//		pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
			//	}
			//}

		}

		if(!VALID_POINT(pNewItem->pProtoType))
		{
			ASSERT(VALID_POINT(pNewItem->pProtoType));
			m_att_res_caution(_T("item/equip"), _T("typeid"), pTmpItem->dw_data_id);
			print_message(_T("The item(SerialNum: %lld) hasn't found proto type!\n"), pTmpItem->n64_serial);
			::Destroy(pNewItem);
			continue;
		}

		pNewItem->eStatus = EUDBS_Null;
		pNewItem->pScript = g_ScriptMgr.GetItemScript( pNewItem->dw_data_id);

		paimai* p_paimai = map_paimai.find(pNewItem->dwOwnerID);
		if(!VALID_POINT(p_paimai))
		{
			print_message(_T("paimai item not find account， id：%d itemserial=%lld\r\n"), pNewItem->dwOwnerID, pNewItem->n64_serial);
			if(!MIsEquipment(pNewItem->dw_data_id))
			{
				NET_DB2C_delete_item send;
				send.n64_serial = pNewItem->n64_serial;
				g_dbSession.Send(&send, send.dw_size);
			}
			else
			{
				NET_DB2C_delete_equip send;
				send.n64_serial = pNewItem->n64_serial;
				g_dbSession.Send(&send, send.dw_size);
			}
			::Destroy(pNewItem);
			continue;
		}

		p_paimai->load_paimai_item(pNewItem);
	}
	
}

// 获取角色拍卖信息数量
VOID paimai_manager::get_role_paimai_num(DWORD dw_role_id, INT& n_num, package_list<DWORD>& list)
{
	MAP_PAIMAI::map_iter iter = map_paimai.begin();
	paimai* p_paimai = NULL;
	while(map_paimai.find_next(iter, p_paimai))
	{
		if(!VALID_POINT(p_paimai))
			continue;

		if(p_paimai->get_att().dw_sell_id == dw_role_id)
		{
			list.push_back(p_paimai->get_att().dw_paimai_id);
			n_num++;
		}
	}
}

// 获取角色竞拍信息数量
VOID paimai_manager::get_role_jingpai_num(DWORD dw_role_id, INT& n_num, package_list<DWORD>& list)
{
	MAP_PAIMAI::map_iter iter = map_paimai.begin();
	paimai* p_paimai = NULL;
	while(map_paimai.find_next(iter, p_paimai))
	{
		if(!VALID_POINT(p_paimai))
			continue;

		if(p_paimai->get_att().dw_bidup_id == dw_role_id)
		{
			list.push_back(p_paimai->get_att().dw_paimai_id);
			n_num++;
		}
	}
}	

// 拍卖信息查询
VOID paimai_manager::query_paimai(Role* p_role, NET_SIC_paimai_query* p_recv)
{
	if(!VALID_POINT(p_role) || !VALID_POINT(p_recv))
		return; 

	p_role->get_query_paimai().clear();

	MAP_PAIMAI::map_iter iter = map_paimai.begin();
	paimai* p_paimai = NULL;
	while(map_paimai.find_next(iter, p_paimai))
	{
		if(!VALID_POINT(p_paimai))
			continue;

		if(!VALID_POINT(p_paimai->get_item()))
			continue;

		if(p_role->GetID() == p_paimai->get_att().dw_sell_id)
			continue;

		switch(p_recv->st_paimai_query.e_query_type)
		{
		case EQT_ALL:		// 查询所有拍卖物品
			{
				p_role->get_query_paimai().push_back(p_paimai->get_att().dw_paimai_id);
				break;
			}
		case EQT_Name:		// 按物品名称查询
			{
				if(p_paimai->get_item()->pProtoType->dw_data_id == p_recv->st_paimai_query.dw_type_id)
				{
					p_role->get_query_paimai().push_back(p_paimai->get_att().dw_paimai_id);
				}
				break;
			}
		case EQT_Type:
			{
				if(p_paimai->get_item()->pProtoType->eType == (EItemType)p_recv->st_paimai_query.dw_item_type[0])
				{
					p_role->get_query_paimai().push_back(p_paimai->get_att().dw_paimai_id);
				}
				break;
			}
		case EQT_TypeEx:
			{
				if(p_paimai->get_item()->pProtoType->eType == (EItemType)p_recv->st_paimai_query.dw_item_type[0] && 
					p_paimai->get_item()->pProtoType->eTypeEx == (EItemTypeEx)p_recv->st_paimai_query.dw_item_type[1])
				{
					p_role->get_query_paimai().push_back(p_paimai->get_att().dw_paimai_id);
				}
				break;
			}
		case EQT_Reserved:
			{
				if(p_paimai->get_item()->pProtoType->eType == (EItemType)p_recv->st_paimai_query.dw_item_type[0] && 
					p_paimai->get_item()->pProtoType->eTypeEx == (EItemTypeEx)p_recv->st_paimai_query.dw_item_type[1] && 
					p_paimai->get_item()->pProtoType->eTypeReserved == (EItemTypeReserved)p_recv->st_paimai_query.dw_item_type[2])
				{
					p_role->get_query_paimai().push_back(p_paimai->get_att().dw_paimai_id);
				}
				break;
			}
		}
	}

	send_query_result(p_role, 0, p_role->get_query_paimai());
}

// 返回查询结果
VOID paimai_manager::send_query_result(Role* p_role, INT n_page_num, package_list<DWORD>& list_query_paimai)
{
	INT n_page = 0;

	cal_page_num(list_query_paimai.size(), n_page);

	if(n_page_num < 0 || n_page_num > n_page)
		return;

	INT n_message_size = sizeof(NET_SIS_paimai_query) + (QUERY_NUM-1) * sizeof(tag_own_paimai_info);
	CREATE_MSG(p_send, n_message_size, NET_SIS_paimai_query);

	p_send->n_page_num = n_page;
	p_send->n_num = 0;

	M_trans_pointer(p, p_send->st_own_paimai, tag_own_paimai_info);

	package_list<DWORD>::list_iter iter = list_query_paimai.begin();
	DWORD	dw_paimai_id = INVALID_VALUE;
	INT		n_temp = 0;
	
	while(list_query_paimai.find_next(iter, dw_paimai_id))
	{
		if(n_page > 1)
		{
			if(n_temp < n_page_num*QUERY_NUM)
			{
				n_temp++;
				continue;
			}
		}
		paimai* p_paimai = map_paimai.find(dw_paimai_id);
		if(!VALID_POINT(p_paimai))
			continue;

		tagItem* pItem = p_paimai->get_item();
		if(!VALID_POINT(pItem))
			continue;

		p_send->st_own_paimai[p_send->n_num].dw_paimai_id = p_paimai->get_att().dw_paimai_id;
		p_send->st_own_paimai[p_send->n_num].dw_bidup_price = p_paimai->get_att().dw_bidup;
		p_send->st_own_paimai[p_send->n_num].dw_chaw_price = p_paimai->get_att().dw_chaw;
		p_send->st_own_paimai[p_send->n_num].dw_begin_time = p_paimai->get_att().dw_beigin_time;
		p_send->st_own_paimai[p_send->n_num].by_time_type = p_paimai->get_att().by_time_type;
		p_send->st_own_paimai[p_send->n_num].b_show_name = p_paimai->get_att().b_show_name;
		p_send->st_own_paimai[p_send->n_num].dw_sell_id = p_paimai->get_att().dw_sell_id;

		if(MIsEquipment(pItem->pProtoType->dw_data_id))
		{
			tagEquip* pEquip = (tagEquip*)pItem;
			p_send->st_own_paimai[p_send->n_num].st_info.dw_data_id = pEquip->dw_data_id;
			//p_send->st_own_paimai[p_send->n_num].st_info.nLevel = pEquip->equipSpec.nLevel;
			p_send->st_own_paimai[p_send->n_num].st_info.byBind = pEquip->byBind;
			p_send->st_own_paimai[p_send->n_num].st_info.byConsolidateLevel = pEquip->equipSpec.byConsolidateLevel;
			//p_send->st_own_paimai[p_send->n_num].st_info.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevelStar;
			p_send->st_own_paimai[p_send->n_num].st_info.nUseTimes = pEquip->nUseTimes;
			p_send->st_own_paimai[p_send->n_num].st_info.byHoldNum = pEquip->equipSpec.byHoleNum;
			//p_send->st_own_paimai[p_send->n_num].st_info.n16MinDmg = pEquip->equipSpec.n16MinDmg;
			//p_send->st_own_paimai[p_send->n_num].st_info.n16MaxDmg = pEquip->equipSpec.n16MaxDmg;
			//p_send->st_own_paimai[p_send->n_num].st_info.n16Armor = pEquip->equipSpec.n16Armor;
			memcpy(p_send->st_own_paimai[p_send->n_num].st_info.EquipAttitionalAtt, pEquip->equipSpec.EquipAttitionalAtt, sizeof(pEquip->equipSpec.EquipAttitionalAtt));
			memcpy(p_send->st_own_paimai[p_send->n_num].st_info.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(DWORD)*MAX_EQUIPHOLE_NUM);
		}
		else
		{
			p_send->st_own_paimai[p_send->n_num].st_info.dw_data_id = pItem->pProtoType->dw_data_id;
			p_send->st_own_paimai[p_send->n_num].st_info.n_num = pItem->n16Num;
		}

		p_send->n_num++;

		if(p_send->n_num >= QUERY_NUM)
			break;
	}

	if(p_send->n_num > 0)
	{
		p_send->dw_size = sizeof(NET_SIS_paimai_query) + (p_send->n_num-1)*sizeof(tag_own_paimai_info);
	}
	else
	{
		p_send->dw_size = sizeof(NET_SIS_paimai_query);
	}

	p_role->SendMessage(p_send, p_send->dw_size);

	MDEL_MSG(p_send);
	
}

// 计算页数
VOID paimai_manager::cal_page_num(INT n_size, INT& n_page)
{
	if(n_size <= 0)
	{
		n_page = 0;
		return;
	}

	n_page = n_size / QUERY_NUM;
	if(n_page <= 0)
	{
		if((n_size % QUERY_NUM) > 0)
		{
			n_page += 1;
			return;
		}
	}
	else
	{
		if((n_size % (n_page*QUERY_NUM)) > 0)
		{
			n_page += 1;
			return;
		}
	}
}

FLOAT paimai_manager::EquipQualityChange(BYTE by_quality)
{
	switch(by_quality)
	{
	case EIQ_Quality0:
		return 1.2;
	case EIQ_Quality1:
		return 1.4;
	case EIQ_Quality2:
		return 1.8;
	case EIQ_Quality3:
		return 2.1;
	case EIQ_Quality4:
		return 2.5;
	default:
		return 1.2;
	}
}

paimai_manager g_paimai;
