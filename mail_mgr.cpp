#include "StdAfx.h"
#include "mail_mgr.h"
#include "role.h"
#include "../../common/WorldDefine/filter.h"
#include "../../common/WorldDefine/mail_define.h"
#include "../../common/WorldDefine/mail_protocol.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../common/ServerDefine/log_server_define.h"
#include "map.h"
#include "item_mgr.h"
#include "creature.h"
#include "role_mgr.h"

mail_mgr g_mailmgr;

mail_mgr::mail_mgr(void)
:m_dwMaxSerialNum(0),m_bInitOK(FALSE)
{
}

mail_mgr::~mail_mgr(void)
{
	
}

VOID mail_mgr::Destroy()
{
	DestoryAllMail();
}

//  生成邮件编号
VOID mail_mgr::CreateMailSerialNum(DWORD& dwSerialNum)
{
	m_Mutex.Acquire();
	dwSerialNum = ++m_dwMaxSerialNum;
	m_Mutex.Release();
}

mail* mail_mgr::GetMail(DWORD dwMailSerial)
{
	//return m_mapMail.find(dwMailSerial);
	return NULL;
}

mail* mail_mgr::GetMail(DWORD dw_role_id, DWORD dwMailSerial)
{
	package_map<DWORD, mail*>* map_mail = m_mapNewMail.find(dw_role_id);
	if(VALID_POINT(map_mail))
	{
		mail* pMail = map_mail->find(dwMailSerial);
		if(VALID_POINT(pMail))
		{
			return pMail;
		}
	}

	return NULL;
}

VOID mail_mgr::Update()
{
	/*MAP_MAIL::map_iter iter = m_mapMail.Begin();
	mail* pMail = NULL;
	while(m_mapMail.find_next(iter, pMail))
	{
		if(VALID_POINT(pMail))
		{
			if(pMail->IsDel())
			{
				m_mapMail.Erase(pMail->GetMailAtt().dwID);
				SAFE_DELETE(pMail);
				continue;
			}
			pMail->Update();
		}
	}*/

	if(!m_bInitOK)
		return;

	MAP_NEWMAIL::map_iter iter = m_mapNewMail.begin();
	package_map<DWORD, mail*>* map_mail = NULL;
	while(m_mapNewMail.find_next(iter, map_mail))
	{
		if(VALID_POINT(map_mail))
		{
			mail* pMail = NULL;
			package_map<DWORD, mail*>::map_iter TIter = map_mail->begin();
			while(map_mail->find_next(TIter, pMail))
			{
				if(VALID_POINT(pMail))
				{
					// 如果是要删除邮件
					if(pMail->IsDel())
					{
						map_mail->erase(pMail->GetMailAtt().dwID);
						SAFE_DELETE(pMail);
						continue;
					}
					
					// 如果是退信
					if(pMail->IsReturn())
					{
						AddRoleMail(pMail->GetMailAtt().dwRecvRoleID, pMail);
						map_mail->erase(pMail->GetMailAtt().dwID);
						pMail->SetReturn(FALSE);

						Role* pRole = g_roleMgr.get_role(pMail->GetMailAtt().dwRecvRoleID);
						if(VALID_POINT(pRole))
						{
							NET_SIS_inform_new_mail cmd;
							cmd.dwMailID = pMail->GetMailAtt().dwID;
							pRole->SendMessage(&cmd, cmd.dw_size);  
						}
						continue;
					}
					pMail->Update();
				}
			}
		}
	}
}

VOID mail_mgr::InitDBLoadMail(NET_DB2S_load_all_mail* pLoadAllMail)
{
	for(INT32 i = 0; i < pLoadAllMail->n32_num_; i++)
	{
		M_trans_pointer(p, pLoadAllMail->s_load_mail_, s_load_mail);

		mail* pMail = new mail;
		if(VALID_POINT(pMail))
		{
			pMail->InitMail(&p[i]);
		}

		//m_mapMail.Add(p[i].stMailBase.dwID, pMail);
		AddRoleMail(pMail->GetMailAtt().dwRecvRoleID, pMail);
	}
}

VOID mail_mgr::InitDBLoadMailContent(NET_DB2S_load_mail_content* pLeadMailContent)
{
	/*mail* pMail = m_mapMail.find(pLeadMailContent->dwMainSerial);
	if(VALID_POINT(pMail))
	{
		pMail->InitContent(pLeadMailContent);
	}*/

	for(INT i = 0; i < pLeadMailContent->n_num; i++)
	{
		BOOL bHave = FALSE;
		MAP_NEWMAIL::map_iter iter = m_mapNewMail.begin();
		package_map<DWORD, mail*>* map_mail = NULL;
		while(m_mapNewMail.find_next(iter, map_mail))
		{
			if(VALID_POINT(map_mail))
			{
				mail* pMail = map_mail->find(pLeadMailContent->st_mail_content[i].dw_main_serial);
				if(VALID_POINT(pMail))
				{
					pMail->InitContent(&pLeadMailContent->st_mail_content[i]);
					bHave = TRUE;
					break;
				}
			}
		}
	}

}

VOID mail_mgr::InitDBLoadMailItem(LPVOID pData, INT& n_num)
{
	/*mail* pMail = m_mapMail.find(dwMailSerial);
	if(VALID_POINT(pMail))
	{
		pMail->InitItem(pData, n_num);
	}*/

	INT32 nItemSize		= sizeof(tagItem);
	INT32 nEquipSize	= sizeof(tagEquip);

	DWORD dw_error_code = INVALID_VALUE;
	const tagItem	*pTmpItem	= NULL;
	tagItem			*pNewItem	= NULL;

	pTmpItem = (const tagItem *)pData;

	for(INT i = 0; i < n_num; i++)
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
			/*if (pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Null && 
				pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Physique)
			{
				pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
				pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
			}

			if (pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt == ERA_Physique)
			{
				if (pEquip->equipSpec.byAddTalentPoint == 0 || 
					pEquip->equipSpec.EquipAttitionalAtt[7].nValue < 0 || 
					pEquip->equipSpec.EquipAttitionalAtt[7].nValue > 30)
				{
					pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
					pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
				}
			}*/

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

		BOOL bHave = FALSE;
		MAP_NEWMAIL::map_iter iter = m_mapNewMail.begin();
		package_map<DWORD, mail*>* map_mail = NULL;
		while(m_mapNewMail.find_next(iter, map_mail))
		{
			if(VALID_POINT(map_mail))
			{
				mail* pMail = map_mail->find(pNewItem->dwOwnerID);
				if(VALID_POINT(pMail))
				{
					pMail->InitItem(pNewItem);
					bHave = TRUE;
					break;
				}
			}
		}
		if(!bHave)
			::Destroy(pNewItem);
	}
}

VOID mail_mgr::InitDBMaxMailSerial(DWORD& dwSerialNum)
{
	m_dwMaxSerialNum = dwSerialNum;
}

// 创建邮件
DWORD mail_mgr::CreateMail(DWORD dwNPCID, Role* pRole, tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent)
{
	if(!VALID_POINT(pRole))
		return INVALID_VALUE;

	//// 获得地图
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	//tagItem* pItem = pRole->GetItemMgr().GetBagItem((DWORD)(11111111));
	//if(!VALID_POINT(pItem))
	//{
	//	// 找到NPC，并检查合法性
	//	Creature* pNPC = pMap->find_creature(dwNPCID);
	//	if(!VALID_POINT(pNPC)
	//		|| !pNPC->IsFunctionNPC(EFNPCT_Mail) 
	//		|| !pNPC->CheckNPCTalkDistance(pRole))
	//	{
	//		return E_Mail_Condition_NotEnough;
	//	}
	//}
	
	//人物不到10级不能发送邮件
	if(pRole->get_level() < 10)
		return E_Mail_Role_Level_NotEnough;

	if(stMailBase.dwRecvRoleID == stMailBase.dwSendRoleID)
		return E_Mail_NoCan_SendOwn;

	// 判断人物发送邮件上限
	if(pRole->GetSendMainNum() >= Max_Role_SendMailNum)
		return E_Mail_MaxSendNum;

	if(stMailBase.dwGiveMoney > Max_Money_Limit || stMailBase.dwSolve > Max_Money_Limit)
		return E_Mail_Money_Limit;

	// 判断邮费是否足够
	BOOL bItem = FALSE;
	INT nMailMoney = AttRes::GetInstance()->GetVariableLen().nMailMoney;
	for(INT i = 0; i < Max_Item_Num; i++)
	{
		if(stMailBase.n64ItemSerial[i] > 0)
		{
			tagItem* pItem = pRole->GetItemMgr().GetBagItem(stMailBase.n64ItemSerial[i]);
			if(VALID_POINT(pItem))
			{
				nMailMoney += nMailMoney;
				bItem = TRUE;

				if(!MIsEquipment(pItem->dw_data_id))
				{
					if(pItem->nUseTimes > 0)
						return E_Item_BeUse;
				}
			}
			else
			{
				return E_Mail_Item_NoExist;
			}
		}
	}

	// 判断人物身上的要邮寄的钱是否足够(包括邮费)
	if(pRole->GetCurMgr().GetBagSilver() < stMailBase.dwGiveMoney+nMailMoney)
		return E_Mail_SendMoney_NotEnough;

	if(!bItem && stMailBase.dwSolve > 0)
		return E_Mail_NoCan_Solve;


	// 判断邮件名称是否合法
	DWORD dwError = INVALID_VALUE;
	dwError = Filter::CheckName(szName, Max_Mail_Name, Min_Mail_Name/*, AttRes::GetInstance()->GetNameFilterWords()*/);
	if(dwError != E_Success)
		return dwError;

	tstring stTmpName(szName);

	tstring stTmpContent;
	if(_tcslen(szContent) > 0)
	{
		//判断邮件内容是否合法
		dwError = Filter::CheckContent(szContent, Max_Mail_Content, Min_Mail_Content, AttRes::GetInstance()->GetNameFilterWords());
		if(dwError != E_Success)
			return dwError;

		stTmpContent = szContent;
	}
	else
	{
		stTmpContent = _T("\0");
	}
	
	/*tstring stTmpContent(szContent);*/

	DWORD dwSerial = INVALID_VALUE;
	CreateMailSerialNum(dwSerial);

	mail* pMail = new mail;
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->CreateMail(dwSerial, stTmpName, stTmpContent, stMailBase);

	//m_mapMail.Add(dwSerial, pMail);
	AddRoleMail(pMail->GetMailAtt().dwRecvRoleID, pMail);

	pRole->GetCurMgr().DecBagSilver(stMailBase.dwGiveMoney+nMailMoney, elcid_npc_mail, pMail->GetMailAtt().dwRecvRoleID);
	
	/*if(VALID_POINT(pItem))
	{
		pRole->GetItemMgr().DelFromBag(pItem->n64_serial, elcid_npc_mail, 1);
	}*/
	pMail->SendMail();
	
	return E_Success;
}

// 创建邮件
DWORD mail_mgr::CreateMail(tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent, DWORD* dwItemTypeID, INT16* n16Qlty, INT* n_num, INT nItemNum, BOOL bBind)
{
	// 判断邮件名称是否合法
	DWORD dwError = INVALID_VALUE;
	dwError = Filter::CheckContent(szName, Max_Mail_Name, Min_Mail_Name, AttRes::GetInstance()->GetNameFilterWords());
	if(dwError != E_Success)
		return dwError;

	tstring stTmpName(szName);

	//判断邮件内容是否合法
	dwError = Filter::CheckContent(szContent, Max_Mail_Content, Min_Mail_Content, AttRes::GetInstance()->GetNameFilterWords());
	if(dwError != E_Success)
		return dwError;
	

	tstring stTmpContent(szContent);

	DWORD dwSerial = INVALID_VALUE;
	CreateMailSerialNum(dwSerial);

	mail* pMail = new mail;
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->CreateMail(dwSerial, stTmpName, stTmpContent, stMailBase, dwItemTypeID, n16Qlty, n_num, nItemNum, bBind);
	//m_mapMail.Add(dwSerial, pMail);
	AddRoleMail(pMail->GetMailAtt().dwRecvRoleID, pMail);

	pMail->SendMail();
	
	return E_Success;
}

DWORD mail_mgr::CreateMail(tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent, tagItem* pItem[], INT nItemNum)
{
	if(nItemNum > Max_Item_Num)
		return INVALID_VALUE;

	for(INT i = 0; i < nItemNum; i++)
	{
		if(!VALID_POINT(pItem[i]))
			return INVALID_VALUE;
	}
	
	// 判断邮件名称是否合法
	DWORD dwError = INVALID_VALUE;
	dwError = Filter::CheckContent(szName, Max_Mail_Name, Min_Mail_Name, AttRes::GetInstance()->GetNameFilterWords());
	if(dwError != E_Success)
		return dwError;

	tstring stTmpName(szName);

	//判断邮件内容是否合法
	dwError = Filter::CheckContent(szContent, Max_Mail_Content, Min_Mail_Content, AttRes::GetInstance()->GetNameFilterWords());
	if(dwError != E_Success)
		return dwError;


	tstring stTmpContent(szContent);

	DWORD dwSerial = INVALID_VALUE;
	CreateMailSerialNum(dwSerial);

	mail* pMail = new mail;
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->CreateMail(dwSerial, stTmpName, stTmpContent, stMailBase, pItem, nItemNum);

	//m_mapMail.Add(dwSerial, pMail);
	AddRoleMail(pMail->GetMailAtt().dwRecvRoleID, pMail);

	pMail->SendMail();

	return E_Success;
}

// 清除所有邮件
VOID mail_mgr::DestoryAllMail()
{
	/*MAP_MAIL::map_iter iter = m_mapMail.Begin();
	mail* pMail = NULL;
	while(m_mapMail.find_next(iter, pMail))
	{
		if(VALID_POINT(pMail))
		{
			SAFE_DELETE(pMail);
		}
	}
	m_mapMail.Clear();*/

	MAP_NEWMAIL::map_iter iter = m_mapNewMail.begin();
	package_map<DWORD, mail*>* map_mail = NULL;
	DWORD	dwKey = INVALID_VALUE;
	while(m_mapNewMail.find_next(iter, dwKey, map_mail))
	{
		DWORD dw_role_id = dwKey;
		if(VALID_POINT(map_mail))
		{
			mail* pMail = NULL;
			package_map<DWORD, mail*>::map_iter TIter = map_mail->begin();
			while(map_mail->find_next(TIter, pMail))
			{
				if(VALID_POINT(pMail))
				{
					for(INT16 i = 0; i < pMail->GetMailItem().GetCurSpaceSize(); i++)
					{
						tagItem* pItem = pMail->GetMailItem().Remove(i);
						if(VALID_POINT(pItem))
						{
							SAFE_DELETE(pItem);
						}
					}
					map_mail->erase(pMail->GetMailAtt().dwID);
					SAFE_DELETE(pMail);
				}
			}

			SAFE_DELETE(map_mail);
			m_mapNewMail.erase(dw_role_id);
		}
	}

	m_mapNewMail.clear();
}

// 获取玩家身上的邮件数量
VOID mail_mgr::GetRoleMailNum(INT32& n32Num, BOOL& bRead, DWORD dw_role_id)
{
	/*MAP_MAIL::map_iter iter = m_mapMail.Begin();
	mail* pMail = NULL;
	while(m_mapMail.find_next(iter, pMail))
	{
		if(VALID_POINT(pMail))
		{
			if(pMail->GetMailAtt().bSend && pMail->GetMailAtt().dwRecvRoleID == dw_role_id)
			{
				n32Num++;
				if(pMail->GetMailAtt().bReed == FALSE)
					bRead = TRUE;
			}
		}
	}*/

	package_map<DWORD, mail*>* map_mail = m_mapNewMail.find(dw_role_id);
	if(VALID_POINT(map_mail))
	{
		mail* pMail = NULL;
		package_map<DWORD, mail*>::map_iter TIter = map_mail->begin();
		while(map_mail->find_next(TIter, pMail))
		{
			if(VALID_POINT(pMail))
			{
				if(pMail->GetMailAtt().bSend && pMail->GetMailAtt().dwRecvRoleID == dw_role_id && !pMail->IsDel() && !pMail->IsReturn())
				{
					n32Num++;
					if(pMail->GetMailAtt().bReed == FALSE)
						bRead = TRUE;
				}
			}
		}
	}
}

// 获取邮件列表
VOID  mail_mgr::GetMailInfo(Role* pRole)
{
	
	INT32 n32Num = 0;
	BOOL bRead = FALSE;
	GetRoleMailNum(n32Num, bRead, pRole->GetID());
	if(n32Num <= 0)
		return;

	INT32 n32MemSize = 0;
	if(n32Num > Max_Mail_Size)
	{
		n32MemSize = Max_Mail_Size;
	}
	else
	{
		n32MemSize = n32Num;
	}

	DWORD dw_size = sizeof(NET_SIS_get_own_mail) + sizeof(tagMailInfo)*(n32MemSize-1);

	CREATE_MSG(pSend, dw_size, NET_SIS_get_own_mail);

	M_trans_pointer(p, pSend->stMailInfo, tagMailInfo);

	INT32 n32Count = 0;
	/*MAP_MAIL::map_iter iter = m_mapMail.Begin();
	mail* pMail = NULL;
	while(m_mapMail.find_next(iter, pMail) && n32Count < n32MemSize)
	{
		if(VALID_POINT(pMail))
		{
			if(pMail->GetMailAtt().bSend && pMail->GetMailAtt().dwRecvRoleID == pRole->GetID())
			{
				swprintf(pSend->stMailInfo[n32Count].szName, pMail->GetMailAtt().strName.c_str(), Max_Mail_Name*sizeof(TCHAR));
				get_fast_code()->memory_copy(&pSend->stMailInfo[n32Count].stMailBase, &pMail->GetMailAtt(), sizeof(tagMailBase));
				tagItem* pItem = pMail->GetSpaceItem(0);
				if(VALID_POINT(pItem))
				{
					pSend->stMailInfo[n32Count].dwType = pItem->dw_data_id;
				}
				else
				{
					pSend->stMailInfo[n32Count].dwType = INVALID_VALUE;
				}
			}
			n32Count++;
		}
	}*/

	package_map<DWORD, mail*>* map_mail = m_mapNewMail.find(pRole->GetID());
	if(VALID_POINT(map_mail))
	{
		mail* pMail = NULL;
		package_map<DWORD, mail*>::map_iter TIter = map_mail->begin();
		while(map_mail->find_next(TIter, pMail) && n32Count < n32MemSize)
		{
			if(VALID_POINT(pMail))
			{
				if(pMail->GetMailAtt().bSend && pMail->GetMailAtt().dwRecvRoleID == pRole->GetID() && !pMail->IsDel() && !pMail->IsReturn())
				{
					wcsncpy(pSend->stMailInfo[n32Count].szName, pMail->GetMailAtt().strName.c_str(), Max_Mail_Name);
					pSend->stMailInfo[n32Count].szName[Max_Mail_Name+1] = _T('\0');
					//get_fast_code()->memory_copy(&pSend->stMailInfo[n32Count].stMailBase, &pMail->GetMailAtt(), sizeof(tagMailBase));
					pSend->stMailInfo[n32Count].stMailInfo.dwID = pMail->GetMailAtt().dwID;
					pSend->stMailInfo[n32Count].stMailInfo.bReed = pMail->GetMailAtt().bReed;
					pSend->stMailInfo[n32Count].stMailInfo.bWithdrawal = pMail->GetMailAtt().bWithdrawal;
					pSend->stMailInfo[n32Count].stMailInfo.dwSendRoleID = pMail->GetMailAtt().dwSendRoleID;
					pSend->stMailInfo[n32Count].stMailInfo.dwSolve = pMail->GetMailAtt().dwSolve;
					pSend->stMailInfo[n32Count].stMailInfo.dwGiveMoney = pMail->GetMailAtt().dwGiveMoney;
					pSend->stMailInfo[n32Count].stMailInfo.byType = pMail->GetMailAtt().byType;
					tagItem* pItem = pMail->GetSpaceItem(0);
					if(VALID_POINT(pItem))
					{
						pSend->stMailInfo[n32Count].dwType = pItem->dw_data_id;
					}
					else
					{
						pSend->stMailInfo[n32Count].dwType = INVALID_VALUE;
					}
					tagDWORDTime dwTmpTime;
					ZeroMemory(&dwTmpTime, sizeof(tagDWORDTime));
					if(pSend->stMailInfo[n32Count].stMailInfo.bWithdrawal || pSend->stMailInfo[n32Count].stMailInfo.dwSendRoleID == INVALID_VALUE)
					{
						dwTmpTime = IncreaseTime(pMail->GetMailAtt().dwRecvTime, 3600 * 24 * 3);
					}
					else
					{
						dwTmpTime = IncreaseTime(pMail->GetMailAtt().dwRecvTime, 3600 * 24);
					}
					pSend->stMailInfo[n32Count].dwFreeTime = CalcTimeDiff(dwTmpTime, GetCurrentDWORDTime());
				}
				n32Count++;
			}
		}
	}

	pSend->n_num = n32Num;
	pRole->SendMessage(pSend, pSend->dw_size);


}

// 获取一封邮件信息
VOID mail_mgr::GetOneMailInfo(Role* pRole, DWORD dwMailID)
{
	if(!VALID_POINT(pRole))
		return;

	NET_SIS_get_one_mail send;

	package_map<DWORD, mail*>* map_mail = m_mapNewMail.find(pRole->GetID());
	if(VALID_POINT(map_mail))
	{
		mail* pMail = NULL;
		
		pMail = map_mail->find(dwMailID);

		if(VALID_POINT(pMail))
		{
			if(pMail->GetMailAtt().dwID == dwMailID)
			{
				wcsncpy(send.stMailInfo.szName, pMail->GetMailAtt().strName.c_str(), Max_Mail_Name);
				send.stMailInfo.szName[Max_Mail_Name+1] = _T('\0');
				//get_fast_code()->memory_copy(&pSend->stMailInfo[n32Count].stMailBase, &pMail->GetMailAtt(), sizeof(tagMailBase));
				send.stMailInfo.stMailInfo.dwID = pMail->GetMailAtt().dwID;
				send.stMailInfo.stMailInfo.bReed = pMail->GetMailAtt().bReed;
				send.stMailInfo.stMailInfo.bWithdrawal = pMail->GetMailAtt().bWithdrawal;
				send.stMailInfo.stMailInfo.dwSendRoleID = pMail->GetMailAtt().dwSendRoleID;
				send.stMailInfo.stMailInfo.dwSolve = pMail->GetMailAtt().dwSolve;
				send.stMailInfo.stMailInfo.dwGiveMoney = pMail->GetMailAtt().dwGiveMoney;
				tagItem* pItem = pMail->GetSpaceItem(0);
				if(VALID_POINT(pItem))
				{
					send.stMailInfo.dwType = pItem->dw_data_id;
				}
				else
				{
					send.stMailInfo.dwType = INVALID_VALUE;
				}
				tagDWORDTime dwTmpTime = IncreaseTime(pMail->GetMailAtt().dwRecvTime, 3600 * 24 * 3);
				send.stMailInfo.dwFreeTime = CalcTimeDiff(dwTmpTime, GetCurrentDWORDTime());

				pRole->SendMessage(&send, send.dw_size);
			}
		}
	}
	
}

// 获取邮件内容
DWORD  mail_mgr::GetMailContent(DWORD dwMailID, Role* pRole)
{
	mail* pMail = GetMail(pRole->GetID(), dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->SendMailContent(pRole);

	return E_Success;
}

// 获取邮件物品
DWORD mail_mgr::GetMailItem(DWORD dwMailID, Role*pRole)
{
	mail* pMail = GetMail(pRole->GetID(), dwMailID);
	if(!VALID_POINT(pMail))
		return INVALID_VALUE;

	pMail->SendMailItem(pRole);
	return E_Success;
}

// 整点清除人物发件数量
VOID mail_mgr::ClearRoleSendMainNum()
{
	g_roleMgr.clear_send_mail_number();

	NET_DB2C_update_role_send_num send;
	send.dw_role_id = INVALID_VALUE;
	g_dbSession.Send(&send, send.dw_size);
}

// 添加人物邮件
VOID mail_mgr::AddRoleMail(DWORD dw_role_id, mail* pMail)
{
	package_map<DWORD, mail*>* map_mail = m_mapNewMail.find(dw_role_id);
	if(VALID_POINT(map_mail))
	{
		map_mail->add(pMail->GetMailAtt().dwID, pMail);
	}
	else
	{
		map_mail = new package_map<DWORD, mail*>;
		map_mail->add(pMail->GetMailAtt().dwID, pMail);
		m_mapNewMail.add(dw_role_id, map_mail);
	}
}

// 是否有未读邮件
BOOL mail_mgr::IsNoReadMail(Role* pRole)
{
	if(!VALID_POINT(pRole))
		return FALSE;

	INT32 n32MailNum = 0;
	BOOL  bRead = FALSE;

	GetRoleMailNum(n32MailNum, bRead, pRole->GetID());

	if(bRead)
		return TRUE;

	return FALSE;
}





