#include "StdAfx.h"
#include "paimai.h"
#include "world.h"
#include "db_session.h"
#include "att_res.h"
#include "script_mgr.h"
#include "mail_mgr.h"
#include "role_mgr.h"
#include "role.h"

#include "../../common/WorldDefine/ItemDefine.h"
#include "../common/ServerDefine/paimai_server_define.h"
#include "../../common/WorldDefine/mail_define.h"

const DWORD	n_time[3] = {21600, 43200, 86400}; // 6, 12, 24小时

paimai::paimai(void)
:p_item(NULL)
{
	
}

paimai::~paimai(void)
{
	
}

BOOL paimai::update()
{
	if(!VALID_POINT(p_item))
		return FALSE;

	if(CalcTimeDiff(g_world.GetWorldTime(), m_att.dw_beigin_time) >= n_time[m_att.by_time_type])
	{
		TCHAR sz_role_name[X_SHORT_NAME];
		TCHAR sz_context[PAIMAI_CONTEXT];
		ZeroMemory(sz_context, sizeof(sz_context));
		ZeroMemory(sz_role_name, sizeof(sz_role_name));

		BYTE	by_quality = 0;
		if(MIsEquipment(p_item->dw_data_id))
			by_quality = ((tagEquip*)p_item)->GetQuality();
		else
			by_quality = p_item->GetQuality();
		
		Role* p_sell_role = g_roleMgr.get_role(m_att.dw_sell_id);
		// 如果没有竞拍者
		if(m_att.dw_bidup_id == INVALID_VALUE)
		{
			tagMailBase st_mail_base;
			ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			st_mail_base.dwSendRoleID = INVALID_VALUE;
			st_mail_base.dwRecvRoleID = m_att.dw_sell_id;
			tagItem* pItem[1] = {p_item};

			ZeroMemory(sz_context, sizeof(sz_context));
			_stprintf(sz_context, _T("%u_%d_%d"), p_item->dw_data_id, p_item->n16Num, p_item->GetQuality());
			g_mailmgr.CreateMail(st_mail_base, _T("&VenTimeOut&"), sz_context, pItem);
			
			if(VALID_POINT(p_sell_role))
			{
				p_sell_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_sell_fail, 1);
			}


		}
		else
		{
			tagMailBase st_mail_base;
			ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			st_mail_base.dwSendRoleID = INVALID_VALUE;
			st_mail_base.dwRecvRoleID = m_att.dw_sell_id;
			st_mail_base.dwGiveMoney = m_att.dw_bidup * (1-((FLOAT)AttRes::GetInstance()->GetVariableLen().n_paimai_duty/100));
			get_role_name(m_att.dw_bidup_id, sz_role_name, TRUE);
			_stprintf(sz_context, _T("%u_%d_%s_%u_1_%d"), p_item->dw_data_id, p_item->n16Num, sz_role_name, m_att.dw_bidup, p_item->GetQuality());
			g_mailmgr.CreateMail(st_mail_base, _T("&VenSell&"), sz_context);

			ZeroMemory(&st_mail_base, sizeof(st_mail_base));
			st_mail_base.dwSendRoleID = INVALID_VALUE;
			st_mail_base.dwRecvRoleID = m_att.dw_bidup_id;
			tagItem* pItem[1] = {p_item};
			ZeroMemory(sz_context, sizeof(sz_context));
			get_role_name(m_att.dw_sell_id, sz_role_name);
			_stprintf(sz_context, _T("%u_%d_%s_%u_1_%d"), p_item->dw_data_id, p_item->n16Num, sz_role_name, m_att.dw_bidup, p_item->GetQuality());
			g_mailmgr.CreateMail(st_mail_base, _T("&VenSuccess&"), sz_context, pItem);

			Role* p_buy_role = g_roleMgr.get_role(m_att.dw_bidup_id);
			if(VALID_POINT(p_buy_role))
			{
				NET_SIS_delete_jingpai send;
				send.dw_paimai_id = m_att.dw_paimai_id;
				p_buy_role->SendMessage(&send, send.dw_size);

				p_buy_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_buy_success, 1);
			}
			if(VALID_POINT(p_sell_role))
			{
				p_sell_role->GetAchievementMgr().UpdateAchievementCriteria(eta_paimai_sell_success, 1);
			}

			send_paimai_log(m_att.dw_bidup_id);

		}

		// 拍卖结束通知删除拍卖信息
		if(VALID_POINT(p_sell_role))
		{
			NET_SIS_delete_paimai send;
			send.dw_paimai_id = m_att.dw_paimai_id;
			p_sell_role->SendMessage(&send, send.dw_size);
		}
		return TRUE;
	}

	return FALSE;
}

// 初始化拍卖属性
VOID paimai::init_att(NET_SIC_begin_paimai* p_recv, DWORD dw_sell_id, DWORD dw_paimai_id, tagItem* p_item, DWORD dw_auto_paimai_id/* = INVALID_VALUE*/)
{
	m_att.dw_paimai_id = dw_paimai_id;
	m_att.dw_sell_id = dw_sell_id;
	m_att.dw_bidup = p_recv->dw_bidup_price;
	m_att.dw_chaw = p_recv->dw_chaw_price;
	m_att.dw_beigin_time = g_world.GetWorldTime();
	m_att.by_time_type = p_recv->by_time_type;
	m_att.b_show_name = p_recv->b_show_name;
	m_att.dw_auto_index = dw_auto_paimai_id;
	this->p_item = p_item;

	this->p_item->SetOwner(m_att.dw_paimai_id, INVALID_VALUE);
	this->p_item->eConType = EICT_PaiMai;

	save_update_item_to_db(this->p_item);

	insert_paimai_to_db();
}

VOID paimai::init_att(tag_paimai* p_paimai)
{
	get_fast_code()->memory_copy(&m_att, p_paimai, sizeof(m_att));

	/*NET_S2DB_load_paimai_item send;
	send.dw_paimai_id = m_att.dw_paimai_id;
	g_dbSession.Send(&send, send.dw_size);*/
}

// 更新物品信息
VOID paimai::save_update_item_to_db(tagItem* p_item)
{
	NET_S2DB_update_paimai_item send;
	send.st_item.by_conType		= EICT_PaiMai;
	send.st_item.dw_owner_id	= p_item->dwOwnerID;
	send.st_item.dw_account_id	= p_item->dw_account_id;
	send.st_item.n16_index		= 0;
	send.st_item.n16_num		= p_item->n16Num;
	send.st_item.n64_serial		= p_item->n64_serial;
	send.st_item.n_use_times	= p_item->nUseTimes;
	send.st_item.by_bind		= p_item->byBind;
	send.st_item.dw_bind_time	= p_item->dwBindTime;

	g_dbSession.Send(&send, send.dw_size);
}

// 保存拍卖信息
VOID paimai::insert_paimai_to_db()
{
	NET_S2DB_insert_paimai send;
	get_fast_code()->memory_copy(&send.st_paimai, &m_att, sizeof(m_att));
	g_dbSession.Send(&send, send.dw_size);
}

// 删除拍卖纪录
VOID paimai::delete_paimai_to_db()
{
	NET_S2DB_delete_paimai send;
	send.dw_paimai_id = m_att.dw_paimai_id;
	g_dbSession.Send(&send, send.dw_size);
}

// 更新拍卖纪录
VOID paimai::update_paimai_to_db()
{
	NET_S2DB_update_paimai send;
	get_fast_code()->memory_copy(&send.st_paimai, &m_att, sizeof(m_att));
	g_dbSession.Send(&send, send.dw_size);
}

// 读取拍卖物品
VOID paimai::load_paimai_item(tagItem* pItem)
{
	p_item = pItem;
}

VOID paimai::send_paimai_log(DWORD dw_buy_id)
{
	NET_DB2C_log_paimai send;

	send.s_log_paimai.dw_bidup = m_att.dw_bidup;
	send.s_log_paimai.dw_bidup_id = m_att.dw_bidup_id;
	send.s_log_paimai.dw_buy_id = dw_buy_id;
	send.s_log_paimai.dw_chaw = m_att.dw_chaw;
	send.s_log_paimai.dw_sell_id = m_att.dw_sell_id;
	if(VALID_POINT(p_item))
	{
		send.s_log_paimai.n64_serial = p_item->n64_serial;
	}
	
	g_dbSession.Send(&send, send.dw_size);
}

VOID paimai::get_role_name(DWORD dw_role_id, LPTSTR sz_role_name, BOOL b_get)
{
	if(b_get)
	{
		g_roleMgr.get_role_name(dw_role_id, sz_role_name);
	}
	else
	{
		if(m_att.b_show_name)
		{
			g_roleMgr.get_role_name(dw_role_id, sz_role_name);
		}
		else
		{
			_tcsncpy(sz_role_name, _T("匿名"), 4);
		}
	}
	
}
