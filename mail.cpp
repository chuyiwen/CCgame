#include "StdAfx.h"
#include "mail.h"
#include "role_mgr.h"
#include "role.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../../common/WorldDefine/mail_protocol.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "mail_mgr.h"
#include "item_creator.h"

#define MAIL_UPDATE_TIME 60000

mail::mail(void)
{
	dwSendMailLastTime = GetCurrentDWORDTime();
	dwWithdrawalLastTime = GetCurrentDWORDTime();
	dwDelMailLastTime = GetCurrentDWORDTime();
	bDel = FALSE;
	bWithdrawal = FALSE;

	dw_update_time = MAIL_UPDATE_TIME;
}

mail::~mail(void)
{
}

const tagMail& mail::GetMailAtt() const
{
	return m_Att;
}

VOID  mail::SetSendMailTime()
{
	m_Att.dwSendTime = GetCurrentDWORDTime();
}

VOID mail::InitContent2DB()
{
	//NET_DB2C_load_mail_content send;
	//send.dw_mail_serial_ = m_Att.dwID;
	//g_dbSession.Send(&send, send.dw_size);
}

VOID mail::InitItem2DB()
{
	/*NET_DB2C_load_mail_item send;
	send.dw_mail_serial = m_Att.dwID;
	g_dbSession.Send(&send, send.dw_size);*/
}

VOID mail::InitContent(tag_mail_content* p_mail_content)
{
	if(!VALID_POINT(p_mail_content))
		return;
	m_Att.strContent = p_mail_content->sz_content_;
}

VOID mail::InitItem(tagItem* pItem)
{
	for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
	{
		tagItem* pTmpItem = m_MailItem.GetItem(i);
		if(VALID_POINT(pTmpItem))
			continue;

		m_MailItem.Add(pItem, i);
		break;
	}
}

VOID mail::InitMail(s_load_mail* pLoadMail)
{
	if(!VALID_POINT(pLoadMail))
		return;

	get_fast_code()->memory_copy((tagMailBase*)&m_Att, &pLoadMail->s_mail_base_, sizeof(tagMailBase));
	m_Att.strName = pLoadMail->str_name;

	InitContent2DB();

	InitItem2DB();
}

// 创建邮件
VOID mail::CreateMail(DWORD dwSerial, tstring& stName, tstring& stContent, tagMailBase& stMailBase, DWORD* dwItemTypeID, INT16* n16Qlty, INT* n_num, INT nItemNum, BOOL bBind)
{
	m_Att.Init(dwSerial, stName, stContent, stMailBase);

	if(stMailBase.dwSendRoleID != INVALID_VALUE)
	{
		for(INT i = 0; i < Max_Item_Num; i++)
		{
			if(stMailBase.n64ItemSerial[i] > 0)
			{
				add_item(stMailBase.dwSendRoleID, stMailBase.n64ItemSerial[i]);
			}
		}
	}
	else
	{
		if(dwItemTypeID && nItemNum > 0 && n16Qlty)
		{
			for(INT i = 0; i < nItemNum; i++)
			{
				tagItem* pItem = ItemCreator::Create(EICM_Mail, NULL, dwItemTypeID[i], 1, bBind);
				if(VALID_POINT(pItem))
				{
					if(MIsEquipment(pItem->dw_data_id))
					{
						ItemCreator::IdentifyEquip((tagEquip*)pItem, (EItemQuality)n16Qlty[i]);
					}

					pItem->n16Num = n_num[i];
					m_Att.n64ItemSerial[i] = pItem->n64_serial;	
					add_item(pItem);
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
				}
			}
		}
	}
	
	m_Att.dwSendTime = GetCurrentDWORDTime();

	/*INT nItemNum = 0;
	GetSendItemNum(nItemNum);*/
	/*if( nItemNum || m_Att.dwGiveMoney > 0)*/
	m_Att.bAtOnce = TRUE;

	save_update_item_to_db();

	SaveMail2DB();

	Role* pSendRole = g_roleMgr.get_role(m_Att.dwSendRoleID);
	if(VALID_POINT(pSendRole))
	{
		pSendRole->AddSendMailNum();
	}

	// 如果不是系统邮件，更新人物发件数量
	if(m_Att.dwSendRoleID != INVALID_VALUE)
	{
		NET_DB2C_update_role_send_num send;
		send.dw_role_id = m_Att.dwSendRoleID;
		g_dbSession.Send(&send, send.dw_size);
	}
}

VOID mail::CreateMail(DWORD dwSerial, tstring& stName, tstring& stContent, tagMailBase& stMailBase, tagItem* pItem[], INT nItemNum)
{
	for(INT i = 0; i < nItemNum; i++)
	{
		if(!VALID_POINT(pItem[i]))
			return;
	}
	

	m_Att.Init(dwSerial, stName, stContent, stMailBase);

	for(INT i = 0; i < nItemNum; i++)
	{
		m_Att.n64ItemSerial[i] = pItem[i]->n64_serial;	
		add_item(pItem[i]);
	}
	
	m_Att.dwSendTime = GetCurrentDWORDTime();

	m_Att.bAtOnce = TRUE;

	save_update_item_to_db();

	SaveMail2DB();
}

VOID mail::Update()
{
	// 发送邮件
	//CheckSendMail();

	dw_update_time -= TICK_TIME;

	if(dw_update_time <= 0)
	{
		// 检查退件
		CheckWithdrawalMail();

		// 检查要删除的邮件
		CheckDelMail();

		dw_update_time = MAIL_UPDATE_TIME;
	}
	
}

// 保存邮件
VOID mail::SaveMail2DB()
{
	INT32 dw_size = sizeof(NET_DB2C_save_mail) + (m_Att.strContent.size()+1)*sizeof(TCHAR);
	CREATE_MSG(pSend, dw_size, NET_DB2C_save_mail);

	wcsncpy(pSend->str_name_, m_Att.strName.c_str(), m_Att.strName.size()+1);
	INT nSize = m_Att.strName.size();
	pSend->str_name_[nSize] = _T('\0');

	get_fast_code()->memory_copy(&pSend->s_mail_base_, (tagMailBase*)&m_Att, sizeof(tagMailBase));

	wcsncpy(pSend->sz_content_, m_Att.strContent.c_str(), m_Att.strContent.size()+1);
	nSize = m_Att.strContent.size();
	pSend->sz_content_[nSize] = _T('\0');

	g_dbSession.Send(pSend, pSend->dw_size);
	MDEL_MSG(pSend);
}

// 更新邮件
VOID mail::SaveUpdateMail2DB()
{
	NET_DB2C_update_mail send;
	get_fast_code()->memory_copy(&send.s_mail_base, (tagMailBase*)&m_Att, sizeof(tagMailBase));
	g_dbSession.Send(&send, send.dw_size);
}

// 删除邮件
VOID mail::DelMail2DB()
{
	NET_DB2C_delete_mail send;
	send.dw_mail_id = m_Att.dwID;
	g_dbSession.Send(&send, send.dw_size);
}

//更新物品基础信息
VOID mail::save_update_item_to_db()
{
	DWORD dw_size = sizeof(NET_DB2C_save_mail_item) + (m_MailItem.GetCurSpaceSize())*sizeof(s_item_update);
	CREATE_MSG(pSend, dw_size, NET_DB2C_save_mail_item);
	M_trans_pointer(pItemUpdate, pSend->by_buffer_, s_item_update);

	pSend->n_count = 0;

	for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
	{
		tagItem* pTemp = m_MailItem.GetItem(i);
		if(VALID_POINT(pTemp) && pTemp->eStatus != EUDBS_Null)
		{
			pItemUpdate[pSend->n_count].by_conType	= pTemp->eConType;
			pItemUpdate[pSend->n_count].dw_owner_id	= pTemp->dwOwnerID;
			pItemUpdate[pSend->n_count].dw_account_id= pTemp->dw_account_id;
			pItemUpdate[pSend->n_count].n16_index	= pTemp->n16Index;
			pItemUpdate[pSend->n_count].n16_num		= pTemp->n16Num;
			pItemUpdate[pSend->n_count].n64_serial	= pTemp->n64_serial;
			pItemUpdate[pSend->n_count].n_use_times	= pTemp->nUseTimes;
			pItemUpdate[pSend->n_count].by_bind		= pTemp->byBind;
			pItemUpdate[pSend->n_count].dw_bind_time = pTemp->dwBindTime;
			memcpy(pItemUpdate[pSend->n_count].dw_script_data, pTemp->dw_script_data, sizeof(DWORD)*2);
			pTemp->SetUpdate(EUDBS_Null);

			pSend->n_count++;
		}
	}

	if (pSend->n_count > 0)
	{
		// 重新设置消息大小
		pSend->dw_size = sizeof(NET_DB2C_save_mail_item) + pSend->n_count * sizeof(s_item_update);

		// 发送消息
		g_dbSession.Send(pSend, pSend->dw_size);
	}
	MDEL_MSG(pSend);
}

// 添加邮寄物品
VOID mail::add_item(DWORD dw_role_id, INT64& n64ItemSerial)
{
	Role* pRole = g_roleMgr.get_role(dw_role_id);
	if(!VALID_POINT(pRole))
		return;

	tagItem* pItem = pRole->GetItemMgr().GetBagItem(n64ItemSerial);
	if(!VALID_POINT(pItem))
		return;

	if(pItem->IsBind())
	{
		n64ItemSerial = INVALID_VALUE;
		return;
	}

	pRole->GetItemMgr().TakeOutFromBag(pItem->n64_serial, (DWORD)elcid_npc_mail, FALSE);
	pItem->SetOwner(m_Att.dwID, INVALID_VALUE);
	pItem->SetUpdate(EUDBS_Update);

	for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
	{
		tagItem* pTmpItem = m_MailItem.GetItem(i);
		if(VALID_POINT(pTmpItem))
			continue;

		m_MailItem.Add(pItem, i);
		break;
	}
}

// 添加邮寄物品
VOID mail::add_item(tagItem* pItem)
{
	if(!VALID_POINT(pItem))
		return;

	pItem->SetOwner(m_Att.dwID, INVALID_VALUE);
	pItem->SetUpdate(EUDBS_Update);

	for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
	{
		tagItem* pTmpItem = m_MailItem.GetItem(i);
		if(VALID_POINT(pTmpItem))
			continue;

		m_MailItem.Add(pItem, i);
		break;
	}
}

// 获取发送的物品数量
VOID mail::GetSendItemNum(INT& nItemNum)
{
	INT nTmpNum = 0;
	for(INT i = 0; i < Max_Item_Num; i++)
	{
		if(m_Att.n64ItemSerial[i] && m_Att.n64ItemSerial[i] != INVALID_VALUE)
		{
			nTmpNum++;
		}
	}

	nItemNum = nTmpNum;
}

VOID mail::DeleteItemID(INT64 n64_id)
{
	for(INT i = 0; i < Max_Item_Num; i++)
	{
		if(m_Att.n64ItemSerial[i] == n64_id)
			m_Att.n64ItemSerial[i] = 0;
	}
}

// 检查要发送的邮件
VOID mail::CheckSendMail()
{
	if(m_Att.dwID <= 0)
		return;

	if(m_Att.bSend)
		return;

	//if(CalcTimeDiff(g_world.GetWorldTime(), dwSendMailLastTime) >= 60)
	{
		// 立刻要发送的邮件
		if(m_Att.bAtOnce)
		{
			SendMail();
			//dwSendMailLastTime = GetCurrentDWORDTime();
			return;
		}

		// 延迟发送的邮件
		/*if(CalcTimeDiff(g_world.GetWorldTime(), m_Att.dwSendTime) >= 3600)
		{
			SendMail();
			dwSendMailLastTime = GetCurrentDWORDTime();
			return;
		}*/
		//dwSendMailLastTime = GetCurrentDWORDTime();
	}
}

// 检查退件
VOID mail::CheckWithdrawalMail()
{
	if(m_Att.dwID <= 0)
		return;

	// 如果已经为退信，不处理
	if(m_Att.bWithdrawal)
		return;

	//if(CalcTimeDiff(g_world.GetWorldTime(), dwWithdrawalLastTime) >= 3600)
	{
		if(CalcTimeDiff(g_world.GetWorldTime(), m_Att.dwRecvTime) >= 3600 * 24)
		{
			INT nItemNum = 0;
			GetSendItemNum(nItemNum);
			// 如果有附件就退信
			if((m_Att.dwGiveMoney > 0 || nItemNum > 0) && m_Att.dwSendRoleID != INVALID_VALUE)
			{
				ReturnMail();
			}
		}
		dwWithdrawalLastTime = g_world.GetWorldTime();
	}
}

// 检查要删除的邮件
VOID mail::CheckDelMail()
{
	if(m_Att.dwID <= 0)
		return;

	if(bDel)
		return;

	//if(CalcTimeDiff(g_world.GetWorldTime(), dwDelMailLastTime) >= 3600)
	{
		if(CalcTimeDiff(g_world.GetWorldTime(), m_Att.dwRecvTime) >= 3600* 24 * 3)
		{
			INT nItemNum = 0;
			GetSendItemNum(nItemNum);
			// 如果为退信或没有附件的邮件，直接删除
			if(m_Att.bWithdrawal || (m_Att.dwGiveMoney <= 0 && nItemNum <= 0) || m_Att.dwSendRoleID == INVALID_VALUE)
			{
				DelMail();
			}
		}
		dwDelMailLastTime = GetCurrentDWORDTime();
	}
}

// 删除邮件
VOID mail::DelMail()
{
	INT n_num = 0;
	GetSendItemNum(n_num);

	if(n_num > 0)
	{
		for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
		{
			tagItem* pItem = m_MailItem.Remove(i);
			if(VALID_POINT(pItem))
			{
				if(!MIsEquipment(pItem->dw_data_id))
				{
					NET_DB2C_delete_item send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
				else
				{
					NET_DB2C_delete_equip send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
			}

			SAFE_DELETE(pItem);
		}
	}

	DelMail2DB();

	CleanAccessory();

	NET_SIS_inform_delete_mail send;
	send.dwMailID = m_Att.dwID;
	Role* pRole = g_roleMgr.get_role(m_Att.dwRecvRoleID);
	if(VALID_POINT(pRole))
	{
		pRole->SendMessage(&send, send.dw_size);
	}

	bDel = TRUE;
}

// 发送邮件
VOID mail::SendMail()
{
	m_Att.bSend = TRUE;
	m_Att.dwRecvTime = GetCurrentDWORDTime();

	Role* pRole = g_roleMgr.get_role(m_Att.dwRecvRoleID);
	if(VALID_POINT(pRole))
	{
		INT32 n32MailNum = 0;
		BOOL bRead = FALSE;
		g_mailmgr.GetRoleMailNum(n32MailNum, bRead, m_Att.dwRecvRoleID);

		NET_SIS_get_mail_num send;
		send.bRead = bRead;
		send.dwMainNum = n32MailNum;
		pRole->SendMessage(&send, send.dw_size);

		NET_SIS_inform_new_mail cmd;
		cmd.dwMailID = m_Att.dwID;
		pRole->SendMessage(&cmd, cmd.dw_size); 
		
		pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_receive_mail, 1);
	}

	SaveUpdateMail2DB();
}

// 获取物品
tagItem* mail::GetSpaceItem(INT16 n16Space)
{
	return m_MailItem.GetItem(n16Space);
}

// 发送邮件内容
VOID mail::SendMailContent(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	DWORD dw_size = m_Att.strContent.size();
	if(dw_size <= 0)
		return;

	dw_size = sizeof(NET_SIS_get_mail_content) + m_Att.strContent.size()*sizeof(TCHAR);

	CREATE_MSG(pSend, dw_size, NET_SIS_get_mail_content);
	pSend->dwMailID = m_Att.dwID;
	wcsncpy(pSend->szContent, m_Att.strContent.c_str(), m_Att.strContent.size()+1);
	INT32 n32Content = m_Att.strContent.size();
	pSend->szContent[n32Content] = _T('\0');
	pRole->SendMessage(pSend, pSend->dw_size);
	MDEL_MSG(pSend);
}

// 发送邮件物品
VOID mail::SendMailItem(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return;

	INT32 n32ItemNum = m_MailItem.GetCurSpaceSize() - m_MailItem.GetFreeSpaceSize();
	if(n32ItemNum <= 0)
	{
		NET_SIS_get_mail_item send;
		send.dwMailID = m_Att.dwID;
		send.n32ItemNum = 0;
		send.dwGiveMoney = m_Att.dwGiveMoney;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	INT32 n32TmpItemNum = 0;

	INT32 n32MaxSize = SIZE_EQUIP * n32ItemNum + sizeof(NET_SIS_get_mail_item);
	CREATE_MSG(pSend, n32MaxSize, NET_SIS_get_mail_item);

	pSend->dwMailID = m_Att.dwID;
	BYTE* byTmpData = pSend->byData;

	for(INT16 i = 0; i < n32ItemNum; i++)
	{
		tagItem* pItem = m_MailItem.GetItem(i);
		if(VALID_POINT(pItem))
		{
			if(MIsEquipment(pItem->dw_data_id))
			{
				get_fast_code()->memory_copy(byTmpData, pItem, SIZE_EQUIP);
				//((tagEquip*)byTmpData)->equipSpec.n16QltyModPctEx = 0;	// 对客户端隐藏二级修正率
				byTmpData += SIZE_EQUIP;
			}
			else
			{
				get_fast_code()->memory_copy(byTmpData, pItem, SIZE_ITEM);
				byTmpData += SIZE_ITEM;
			}
			n32TmpItemNum++;
		}
	}

	pSend->dw_size = sizeof(NET_SIS_get_mail_item) - 1 + (byTmpData - pSend->byData);
	pSend->n32ItemNum = n32TmpItemNum;
	pSend->dwGiveMoney = GetMailAtt().dwGiveMoney;
	pRole->SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);
}

VOID mail::ChangeCmd(DWORD& dw_cmd_id)
{
	switch(m_Att.n_yuanbao_type)
	{
	case 0:
		dw_cmd_id = elcid_npc_mail;
		break;
	case 1:
		dw_cmd_id = elcid_compensate_yuanbao;
		break;
	case 2:
		dw_cmd_id = elcid_pack_yuanbao;
		break;
	default:
		dw_cmd_id = elcid_npc_mail;
		break;
	}
}

// 收取邮件附件
DWORD mail::AcceptAccessoryMail(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	INT nItemNum = 0;
	GetSendItemNum(nItemNum);
	if(m_Att.dwGiveMoney <= 0 && nItemNum <= 0 && m_Att.dwSolve <= 0)
		return E_Mail_No_Accessory;

	// 如果不是退件
	if(!m_Att.bWithdrawal)
	{
		// 判断要付的费用是否足够
		if(m_Att.dwSolve > 0)
		{
			if(pRole->GetCurMgr().GetBagSilver() < m_Att.dwSolve)
				return E_Mail_AcceptMoney_NotEnough;
		}

		// 判断包裹是否有足够位置
		if(nItemNum > 0)
		{
			if(pRole->GetItemMgr().GetBagFreeSize() < nItemNum)
				return E_Mail_Bag_NotEnough;
		}

		if(m_Att.dwGiveMoney > 0)
		{
			if(!m_Att.byType)
			{
				pRole->GetCurMgr().IncBagSilver(m_Att.dwGiveMoney, elcid_npc_mail, m_Att.dwSendRoleID);
			}
			else
			{
				DWORD dw_cmd_id = elcid_npc_mail;
				ChangeCmd(dw_cmd_id);
				pRole->GetCurMgr().IncBaiBaoYuanBao(m_Att.dwGiveMoney, dw_cmd_id, TRUE);
			}
		}

		pRole->GetCurMgr().DecBagSilver(m_Att.dwSolve, elcid_npc_mail, m_Att.dwSendRoleID);

		for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
		{
			tagItem* pItem = m_MailItem.Remove(i);
			if(VALID_POINT(pItem))
			{
				// 此处要删除数据库中的这个物品， 加入包裹时如果是可堆叠物品时无法清除数据库中旧的物品
				if(!MIsEquipment(pItem->dw_data_id))
				{
					NET_DB2C_delete_item send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
				else
				{
					NET_DB2C_delete_equip send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
				pRole->GetItemMgr().Add2Bag(pItem, elcid_npc_mail, TRUE);
				pItem->SetOwner(pRole->GetID(), pRole->GetSession()->GetSessionID());
				pItem->SetUpdate(EUDBS_Update);
			}
		}

		if(m_Att.dwSendRoleID != INVALID_VALUE)
		{
			// 创建反馈邮件
			tagMailBase st_MailBase;
			ZeroMemory(&st_MailBase, sizeof(st_MailBase));
			st_MailBase.dwGiveMoney = m_Att.dwSolve;
			st_MailBase.dwSendRoleID = pRole->GetID();
			st_MailBase.dwRecvRoleID = m_Att.dwSendRoleID;
			g_mailmgr.CreateMail(st_MailBase, _T("收件反馈邮件"), _T("已收取附件"));
		}
		
		//系统邮件
		/*ZeroMemory(&st_MailBase, sizeof(st_MailBase));
		st_MailBase.dwSendRoleID = INVALID_VALUE;
		st_MailBase.dwRecvRoleID = m_Att.dwSendRoleID;
		DWORD dwItemType[Max_Item_Num] = {INVALID_VALUE, INVALID_VALUE, INVALID_VALUE};
		dwItemType[0] = 1000005;
		dwItemType[1] = 9100003;
		g_mailmgr.CreateMail(st_MailBase, _T("邮件"), _T("一级菜鸟赏你个物品"), dwItemType, 2);*/
	}
	else
	{
		// 判断包裹是否有足够位置
		INT nItemNum = 0;
		GetSendItemNum(nItemNum);
		if(nItemNum > 0)
		{
			if(pRole->GetItemMgr().GetBagFreeSize() < nItemNum)
				return E_Mail_Bag_NotEnough;
		}

		for(INT16 i = 0; i < m_MailItem.GetCurSpaceSize(); i++)
		{
			tagItem* pItem = m_MailItem.Remove(i);
			if(VALID_POINT(pItem))
			{
				if(!MIsEquipment(pItem->dw_data_id))
				{
					NET_DB2C_delete_item send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
				else
				{
					NET_DB2C_delete_equip send;
					send.n64_serial = pItem->n64_serial;
					g_dbSession.Send(&send, send.dw_size);
				}
				pRole->GetItemMgr().Add2Bag(pItem, elcid_npc_mail, TRUE);
				pItem->SetOwner(pRole->GetID(), pRole->GetSession()->GetSessionID());
				pItem->SetUpdate(EUDBS_Update);
			}
		}

		if(m_Att.dwGiveMoney > 0)
		{
			pRole->GetCurMgr().IncBagSilver(m_Att.dwGiveMoney, elcid_npc_mail, m_Att.dwSendRoleID);
		}

	}

	CleanAccessory();

	SaveUpdateMail2DB();
	return E_Success;
}

// 清空附件
VOID  mail::CleanAccessory()
{
	m_Att.dwSolve = 0;
	m_Att.dwGiveMoney = 0;
	::ZeroMemory(m_Att.n64ItemSerial, sizeof(m_Att.n64ItemSerial));
}

// 读取邮件
VOID mail::ReadMail()
{
	if(!m_Att.bReed)
	{
		m_Att.bReed = TRUE;
		SaveUpdateMail2DB();
	}
}

// 删除邮件
DWORD mail::DeleteMail()
{
	/*INT nItemNum = 0;
	GetSendItemNum(nItemNum);*/
	if(m_Att.dwGiveMoney > 0 || m_MailItem.GetFreeSpaceSize() <  Max_Item_Num || m_Att.dwSolve > 0)
		return E_Mail_NoCan_Delete;

	DelMail();
	return E_Success;
}

// 退回邮件
DWORD mail::ReturnMail()
{
	m_Att.dwSolve = 0;
	m_Att.bSend = TRUE;
	m_Att.bWithdrawal = TRUE;
	m_Att.bReed = FALSE;
	DWORD dwTmpRoleID = m_Att.dwSendRoleID;
	m_Att.dwSendRoleID = m_Att.dwRecvRoleID;
	m_Att.dwRecvRoleID = dwTmpRoleID;
	m_Att.dwSendTime = GetCurrentDWORDTime();
	ZeroMemory(&m_Att.dwRecvTime, sizeof(tagDWORDTime));
	m_Att.dwRecvTime = GetCurrentDWORDTime();
	dwSendMailLastTime = GetCurrentDWORDTime();

	bWithdrawal = TRUE;

	SaveUpdateMail2DB();

	NET_SIS_inform_return_mail send;
	send.dwMailID = m_Att.dwID;
	Role* pRole = g_roleMgr.get_role(m_Att.dwSendRoleID);
	if(VALID_POINT(pRole))
	{
		pRole->SendMessage(&send, send.dw_size);
	}
	return E_Success;
}
