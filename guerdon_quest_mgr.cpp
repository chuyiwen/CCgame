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
*	@file		guerdon_quest_mgr
*	@author		mwh
*	@date		2011/04/18	initial
*	@version	0.0.1.0
*	@brief		悬赏任务
*/
#include "StdAfx.h"
#include "../../common/WorldDefine/QuestDef.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/mail_define.h"
#include "../common/ServerDefine/quest_server_define.h"
#include "../../common/WorldDefine/guerdon_quest_protocol.h"
#include "../common/ServerDefine/guerdon_quest_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "role.h"
#include "quest.h"
#include "role_mgr.h"
#include "mail_mgr.h"
#include "quest_mgr.h"
#include "db_session.h"
#include "title_mgr.h"
#include "guerdon_quest_mgr.h"

guerdon_quest_mgr g_GuerdonQuestMgr;

VOID FillDataWithQuest(guerdon_quest_data* pData, const guerdon_quest* pQuest)
{
	ASSERT( pData != 0); ASSERT(pQuest != 0);

	pData->n64Serial = pQuest->n64Serial;
	pData->dwSender = pQuest->dwSender;
	pData->dwReciver = pQuest->dwReciver;
	pData->u16QuestID = pQuest->u16QuestID;
	pData->bGuildFix = pQuest->bGuildFix;
	pData->dwEndTime = pQuest->dwEndTime;
	pData->u16YuanBao = pQuest->u16YuanBao;

	if(VALID_POINT(pQuest->pItem0)) 
	{
		pData->dwItem0 = pQuest->pItem0->dw_data_id;
		pData->n16Number0 = pQuest->pItem0->n16Num;
	}
	else { pData->dwItem0 = 0; pData->n16Number0 = 0;}

	if(VALID_POINT(pQuest->pItem1)) 
	{
		pData->dwItem1 = pQuest->pItem1->dw_data_id;
		pData->n16Number1 = pQuest->pItem1->n16Num;
	}
	else { pData->dwItem1 = 0; pData->n16Number1 = 0;}
}

//--------------------------------------------------------------------
// 主线程调用
//--------------------------------------------------------------------
BOOL guerdon_quest_mgr::init()
{
	RegisterEvent();
	return TRUE;
}
VOID guerdon_quest_mgr::destroy()
{
	guerdon_quest* p = 0;
	mQuests.reset_iterator();
	while(mQuests.find_next(p))
	{
		::Destroy(p->pItem0);
		::Destroy(p->pItem1);
		SAFE_DELETE(p);
	}

	IDRefHelper* pIDRef = 0;
	mGets.reset_iterator();
	while(mGets.find_next(pIDRef))
	{
		SAFE_DELETE(pIDRef);
	}

	mPuts.reset_iterator();
	while(mPuts.find_next(pIDRef))
	{
		SAFE_DELETE(pIDRef);
	}

	mQuests.clear();
	mGets.clear();
	mPuts.clear();
}
VOID guerdon_quest_mgr::update()
{
	Super::Update();

	// 1分钟一次
	static DWORD _S_Tick = 0;
	if((++_S_Tick) >= TICK_PER_SECOND * 60)
	{
		tagDWORDTime dwCurTime = GetCurrentDWORDTime();
		guerdon_quest* p = 0; mQuests.reset_iterator();
		while(mQuests.find_next(p))
		{	
			if(p->dwEndTime <= dwCurTime)
			{
				QuestTimeout(p);
				DeleteQuestDBAndMem(p);
			}
		}
		_S_Tick = 0;
	}
}

VOID guerdon_quest_mgr::init(const VOID* pData, INT number)
{
	// 服务器启动时，初始化所有悬赏
	guerdon_quest* pGuerdonQuest = (guerdon_quest*)	pData;
	for(INT n = 0; n < number; ++n, ++pGuerdonQuest)
	{
		guerdon_quest* p = new guerdon_quest;
		p->dwReciver = pGuerdonQuest->dwReciver;
		p->n64Serial = pGuerdonQuest->n64Serial;
		p->dwSender = pGuerdonQuest->dwSender;
		p->eState	= pGuerdonQuest->eState;
		p->u16QuestID = pGuerdonQuest->u16QuestID;
		p->bGuildFix = pGuerdonQuest->bGuildFix;
		p->dwEndTime = pGuerdonQuest->dwEndTime;
		p->u16YuanBao = pGuerdonQuest->u16YuanBao;
		p->pItem0 = p->pItem1 = 0;

		ASSERT(p->Assert());
		ASSERT(!mQuests.is_exist(pGuerdonQuest->n64Serial));
		if(!p->Assert()|| mQuests.is_exist(pGuerdonQuest->n64Serial)){ delete p; continue; }

		if(VALID_POINT(p->dwReciver))
			AddGets(p->dwReciver, p->u16QuestID, p->n64Serial);

		if(VALID_POINT(p->dwSender))
			AddPuts(p->dwSender, p->u16QuestID, p->n64Serial);

		mQuests.add(p->n64Serial, p);
	}
}

VOID guerdon_quest_mgr::init_item(const VOID* pData, INT number)
{
	// 初始化所有任务奖励
	if(number == 0) return;

	const INT ITEMSIZE = sizeof( tagItem );
	const INT EQUIPSIZE = sizeof( tagEquip );
	const tagItem* pItem = (const tagItem*)pData;

	for(INT n = 0; n < number; ++n)
	{
		if(MIsEquipment(pItem->dw_data_id))
		{
			ASSERT(FALSE);
			print_message(_T("The gdquest reward(SerialNum: %lld, %u) is Equipment!\n"), pItem->n64_serial, pItem->dw_data_id);
			pItem = (const tagItem*)((char*)pItem + EQUIPSIZE);
			continue;
		}

		tagItem* pNewItem = new tagItem;
		get_fast_code()->memory_copy(pNewItem, pItem, ITEMSIZE);
		pNewItem->pProtoType = AttRes::GetInstance()->GetItemProto(pItem->dw_data_id);
		pItem = (const tagItem*)((char*)pItem + ITEMSIZE);

		if(!VALID_POINT(pNewItem->pProtoType))
		{
			ASSERT(VALID_POINT(pNewItem->pProtoType));
			m_att_res_caution(_T("item/equip"), _T("typeid"), pNewItem->dw_data_id);
			print_message(_T("The item(SerialNum: %lld) hasn't found proto type!\n"), pNewItem->n64_serial);
			::Destroy(pNewItem);
			continue;
		}

		pNewItem->n16Index = n;
		pNewItem->eStatus = EUDBS_Null;
		pNewItem->pScript = g_ScriptMgr.GetItemScript( pNewItem->dw_data_id);

		INT64 dwQuestSerial = guerdon_quest::MakeSerial(pNewItem->dwOwnerID, (UINT16)pNewItem->dw_account_id);
		guerdon_quest* pQuest = GetQuest(dwQuestSerial);
		if(!VALID_POINT(pQuest))
		{
			print_message(_T("\nThe item(SerialNum: %lld,QuestSerial: %lld) error gdquest reward!!\n"), pNewItem->n64_serial, dwQuestSerial);
			::Destroy(pNewItem);
			continue;
		}
		
		if(!VALID_POINT(pQuest->pItem0))
			pQuest->pItem0 = pNewItem;
		else if(!VALID_POINT(pQuest->pItem1))
			pQuest->pItem1 = pNewItem;	
		else ASSERT(FALSE);
	}
}

//--------------------------------------------------------------------
// 内部辅助函数
//--------------------------------------------------------------------
VOID guerdon_quest_mgr::PutQuest(Role* pRole, VOID* pMsg)
{
	NET_SIC_PutGDQuest* pProtocol = (NET_SIC_PutGDQuest*)pMsg;

	quest* pQuest = pRole->get_quest(pProtocol->u16QuestID);
	if(!VALID_POINT(pQuest)) return;
	if(!VALID_POINT(pQuest->get_protocol()))return;
	if(!macroIsXSQuest(pQuest->get_protocol())) return;

	// 接取的悬赏不可在悬赏
	if(pQuest->get_quest_flag().dwTakeFromGuerdon)
	{
		NET_SIS_PutGDQuest send;
		send.dwError = EGDE_Put_Cant;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	// 已经悬赏
	INT64 n64Serial = guerdon_quest::MakeSerial(pRole->GetID(), pProtocol->u16QuestID);
	if(mQuests.is_exist(n64Serial)) 
	{
		NET_SIS_PutGDQuest send;
		send.dwError = EGDE_Put_Already;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	// 悬赏上限
	if(!CanPuts(pRole->GetID()))
	{
		NET_SIS_PutGDQuest send;
		send.dwError = EGDE_Put_OutOfNumber;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	// 手续费
	if(pRole->GetCurMgr().GetBagSilver() < PUTOUTSHANDLECHARGE)
	{
		NET_SIS_PutGDQuest send;
		send.dwError = EGDE_Put_OutOfMoney;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	if(pRole->GetCurMgr().GetBaiBaoYuanBao() < pProtocol->byYuanBao)
	{
		NET_SIS_PutGDQuest send;
		send.dwError = EGDE_Put_OutOfYuanBao;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	// 额外奖励
	tagItem* pItem0 = 0, *pItem1 = 0;
	if(VALID_POINT(pProtocol->n64Item0))
	{
		pItem0 = pRole->GetItemMgr().GetBagItem(pProtocol->n64Item0);
		if(!VALID_POINT(pItem0) || !RewardCheck(pItem0))
		{
			NET_SIS_PutGDQuest send;
			send.dwError = EGDE_Put_CheckItem;
			pRole->SendMessage(&send, send.dw_size);
			return ;
		}
	}

	if(VALID_POINT(pProtocol->n64Item1))
	{
		pItem1 = pRole->GetItemMgr().GetBagItem(pProtocol->n64Item1);
		if(!VALID_POINT(pItem1) || !RewardCheck(pItem1))
		{
			NET_SIS_PutGDQuest send;
			send.dwError = EGDE_Put_CheckItem;
			pRole->SendMessage(&send, send.dw_size);
			return ;
		}
	}

	// 开始发布
	if(VALID_POINT(pItem0))
	{// 从玩家背包删除奖励道具1
		NET_DB2S_InsertQuestRewardItem send;
		send.byContainerType = EICT_GDQuest;
		send.dwSender = pRole->GetID();
		send.u16QuestID = pProtocol->u16QuestID;
		send.n64Serial = pItem0->n64_serial;
		g_dbSession.Send(&send, send.dw_size);

		pItem0->eConType = EICT_GDQuest;
		pRole->GetItemMgr().TakeOutFromBag(pItem0->n64_serial,elcid_gdquest_putout, FALSE);
	}

	if(VALID_POINT(pItem1))
	{// 从玩家背包删除奖励道具2
		NET_DB2S_InsertQuestRewardItem send;
		send.byContainerType = EICT_GDQuest;
		send.dwSender = pRole->GetID();
		send.u16QuestID = pProtocol->u16QuestID;
		send.n64Serial = pItem1->n64_serial;
		g_dbSession.Send(&send, send.dw_size);

		pItem1->eConType = EICT_GDQuest;
		pRole->GetItemMgr().TakeOutFromBag(pItem1->n64_serial,elcid_gdquest_putout, FALSE);
	}

	// 扣钱
	pRole->GetCurMgr().DecBagSilver(PUTOUTSHANDLECHARGE, elcid_gdquest_putout);
	pRole->GetCurMgr().DecBaiBaoYuanBao(pProtocol->byYuanBao, elcid_gdquest_putout);
	pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_quest_xuanshan_put, 1);
	// 设置标志位
	pQuest->get_quest_flag().dwQuestBeGuerdon = 1;
	{
		NET_SIS_UpdateQuestFlag send;
		send.u16QuestID = pQuest->get_id();
		send.dwFlag = pQuest->get_quest_flag();
		pRole->SendMessage(&send, send.dw_size);
	}

	// 初始化数据
	guerdon_quest* pGuerdonQuest = new guerdon_quest;
	pGuerdonQuest->dwReciver = 0;
	pGuerdonQuest->n64Serial = n64Serial;
	pGuerdonQuest->dwSender = pRole->GetID();
	pGuerdonQuest->eState = EGQS_WaitForTake;
	pGuerdonQuest->u16QuestID = pQuest->get_id();
	pGuerdonQuest->bGuildFix = pProtocol->bGuildFix;
	pGuerdonQuest->dwEndTime = IncreaseTime(GetCurrentDWORDTime(), (pProtocol->byHour * 3600));
	pGuerdonQuest->u16YuanBao = pProtocol->byYuanBao;
	pGuerdonQuest->pItem0 = pItem0;  pGuerdonQuest->pItem1 = pItem1;
	mQuests.add(n64Serial, pGuerdonQuest);
	AddPuts(pRole->GetID(), pGuerdonQuest->u16QuestID, pGuerdonQuest->n64Serial);

	{// 通知结果	
		NET_SIS_PutGDQuest send;
		send.dwError = E_Success;
		send.u16QuestID = pProtocol->u16QuestID;
		pRole->SendMessage(&send, send.dw_size);
	}

	{// 数据库保存
		NET_DB2S_AddNewGuerdonQuest send;
		ASSERT(sizeof(send.stData) == sizeof(*pGuerdonQuest));
		memcpy(&send.stData, pGuerdonQuest, sizeof(guerdon_quest));
		g_dbSession.Send(&send, send.dw_size);
	}
}
VOID guerdon_quest_mgr::GetQuest(Role* pRole, VOID* pMsg)
{
	INT64 n64Serial = ((NET_SIC_GetGDQuest*)pMsg)->n64Serial;

	if(pRole->get_level() < TAKEGDQUESTMINLEVEL)
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_OutOfLevel25;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	// 上限判断
	if(!CanGets(pRole->GetID()))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_OutOfNumber;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 任务检查
	guerdon_quest* pGuerdonQuest = mQuests.find(n64Serial);
	if(!VALID_POINT(pGuerdonQuest))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_NotExist;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	const tagQuestProto* pProto = g_questMgr.get_protocol(pGuerdonQuest->u16QuestID);
	if(!VALID_POINT(pProto))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_NotExist;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 状态检查
	if(VALID_POINT(pGuerdonQuest->dwReciver))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_BeTaked;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 不可自己接取
	if(pGuerdonQuest->dwSender == pRole->GetID())
		return;

	// 不可重复接取
	if(pRole->is_have_quest(QuestIDHelper::GenerateID(pGuerdonQuest->u16QuestID, TRUE)))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_Already;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 手续费与押金
	if(pRole->GetCurMgr().GetBagSilver() < (HANDLINGCHARGESILVER + TAKEQUESTFOREGIFTSILVER))
	{
		NET_SIS_GetGDQuest send;
		send.n64Serial = n64Serial;
		send.dwError = EGDE_Get_OutOfMoney;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 普通任务检查
	INT nIndex = INVALID_VALUE;
	INT nRet = pRole->accept_quest_check(pGuerdonQuest->u16QuestID, nIndex, NULL, TRUE);
	if( nRet == E_Success)
	{
		pGuerdonQuest->dwReciver = pRole->GetID();
		pGuerdonQuest->eState = EGQS_WaitForFinish;
		AddGets(pRole->GetID(), pGuerdonQuest->u16QuestID, pGuerdonQuest->n64Serial);
		pRole->GetCurMgr().DecBagSilver((HANDLINGCHARGESILVER  + TAKEQUESTFOREGIFTSILVER),elcid_xsquest_accept);
		pRole->add_quest(pProto, nIndex, TRUE);
		quest* pQuest = pRole->get_quest_by_index(nIndex);
		if(!VALID_POINT(pQuest))  pQuest->on_accept(NULL);

		pRole->GetAchievementMgr().UpdateAchievementCriteria(eta_quest_xuanshan_get, 1);
	}

	NET_SIS_GetGDQuest send;
	send.dwError = nRet;
	send.n64Serial = n64Serial;
	pRole->SendMessage(&send, send.dw_size);


	UpdateQuest2DB(pGuerdonQuest);
}
VOID guerdon_quest_mgr::CompGDQuest(Role* pRole, VOID* pMsg)
{
	UINT16 u16 = ((NET_SIC_CompleteGDQuest*)pMsg)->u16QuestID;
	INT choice_index = ((NET_SIC_CompleteGDQuest*)pMsg)->choice_index;

	IDRefHelper* pHelper = mGets.find(pRole->GetID());
	if(!VALID_POINT(pHelper))
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = 0;
		send.dwError = EGDE_NotBelongYou;
		pRole->SendMessage(&send, send.dw_size);
		print_message(_T("Complete GuerdonQuest fatal error[IDRefHelper]:r:%u,quest:%u\n"),
			pRole->GetID(), u16);
		return;
	}

	INT64 n64 = pHelper->Get64ID(QuestIDHelper::RestoreID(u16));
	if(!VALID_POINT(n64))
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = EGDE_NotBelongYou;
		pRole->SendMessage(&send, send.dw_size);
		print_message(_T("Complete GuerdonQuest fatal error[IDRefHelper.Get64ID]:r:%u,quest:%u\n"),
			pRole->GetID(), u16);
		return;
	}

	guerdon_quest* p = mQuests.find(n64);
	if(!VALID_POINT(p))
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = EGDE_NotBelongYou;
		pRole->SendMessage(&send, send.dw_size);
		print_message(_T("Complete GuerdonQuest fatal error[IDRefHelper.find]:r:%u,quest:%u,serial:%I64d\n"),
			pRole->GetID(), u16, n64);
		return;
	}

	// 状态判断
	if(p->dwSender != pRole->GetID() && p->dwReciver != pRole->GetID())
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = EGDE_NotBelongYou;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 只能接取者完成
	if(p->dwReciver != pRole->GetID())
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = EGDE_OnlyReciverComplete;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 任务判断
	quest* pQuest = pRole->get_quest(u16);
	if(!VALID_POINT(pQuest) || pQuest->get_quest_flag().dwTakeFromGuerdon == 0)
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = EGDE_NotBelongYou;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 普通任务检查
	if(!pQuest->complete_check(choice_index, NULL, TRUE))
	{
		NET_SIS_CompleteGDQuest send;
		send.n64Serial = n64;
		send.dwError = E_CanCompleteQuest_FAILED_MISSING_Creature;
		pRole->SendMessage(&send, send.dw_size);
		return;
	}

	// 处理完成数据
	Role* pSender = g_roleMgr.get_role(p->dwSender);
	if(VALID_POINT(pSender) && !pSender->is_leave_pricitice())
	{// 发布者在线
		quest* pBeGDQuest = pSender->get_quest(p->u16QuestID);
		if(VALID_POINT(pBeGDQuest) && pBeGDQuest->get_quest_flag().dwQuestBeGuerdon)
		{
			pBeGDQuest->get_quest_flag().dwQuestBeGuerdon = 1;
			pBeGDQuest->get_quest_flag().dwCompleteByReciver = 1;

			NET_SIS_UpdateQuestFlag send;
			send.u16QuestID = pBeGDQuest->get_id();
			send.dwFlag = pBeGDQuest->get_quest_flag();
			pSender->SendMessage(&send, send.dw_size);
		}else{
			print_message(_T("Complete GerdonQuest fatal Error[Flag]: r:%u,s:%u quest:%d,flag:%u\n"),
				p->dwReciver, p->dwSender, VALID_POINT(pBeGDQuest) ? p->u16QuestID : -1,
				VALID_POINT(pBeGDQuest) ? pBeGDQuest->get_quest_flag() : -1 );
		}

		NET_SIS_GDQuestCompleteByReciver send;
		send.u16QuestID = p->u16QuestID;
		send.n64Serial = n64;
		pSender->SendMessage(&send, send.dw_size);
	}
	else
	{// 不在线
		tagDWORDQuestFlag flag(0), flagEx(-1);
		flag.dwQuestBeGuerdon = 1; flagEx.dwQuestBeGuerdon = 0;
		flag.dwCompleteByReciver = 1; flagEx.dwCompleteByReciver = 0;
		NET_DB2S_UpdateRoleQuestFlag send1;
		send1.dwRoleID = p->dwSender;
		send1.u16QuestID = p->u16QuestID;
		send1.dwFlag = flag; send1.dwFlagEx = flagEx;
		g_dbSession.Send(&send1, send1.dw_size);
	}

	// 处理奖励
	if(VALID_POINT(p->dwReciver))
	{
		//将道具通过发邮件发给接单者
		QuestRewardDeal(p, EROT_Complete);
	}

	// 接取者删除映射
	pHelper->Remove(QuestIDHelper::RestoreID(u16));

	// 发布者者删除映射
	pHelper = mPuts.find(p->dwSender);
	if(VALID_POINT(pHelper)) pHelper->Remove(QuestIDHelper::RestoreID(u16));

	// 删除
	DeleteQuestDBAndMem(p);

	{// 普通任务完成
		BOOL bHasSameCommQuest = VALID_POINT( pRole->get_quest(QuestIDHelper::RestoreID(u16)) );
		pQuest->set_complete_flag(TRUE);
		pRole->remove_quest(u16, TRUE, bHasSameCommQuest);

		NET_SIS_complete_quest send;
		send.u16QuestID = u16;
		send.dw_error_code = E_Success;
		pRole->SendMessage(&send, send.dw_size);
	}


	NET_SIS_CompleteGDQuest send;
	send.dwError = E_Success;
	send.n64Serial = n64;
	pRole->SendMessage(&send, send.dw_size);
}

VOID guerdon_quest_mgr::ComplBeGDQuest(Role* pRole, VOID* pMsg)
{
	NET_SIC_CompleteGDQuest* pProtocol = (NET_SIC_CompleteGDQuest*)pMsg;

	NET_SIS_complete_quest send;
	send.u16QuestID = pProtocol->u16QuestID;
	send.dw_error_code = pRole->complete_quest_ex(pProtocol->u16QuestID, 
																						pProtocol->dwNPCID,
																						pProtocol->choice_index );
	pRole->SendMessage(&send, send.dw_size);

	//! mwh 2011-08-22 换随机任务被完成后，继续刷
	if(E_Success == send.dw_error_code)
		pRole->auto_rand_circle_quest_after_complete_quest(pProtocol->u16QuestID);
}
VOID guerdon_quest_mgr::QuestTimeout(guerdon_quest* p)
{
	// 发布者处理
	Role* pSender = g_roleMgr.get_role(p->dwSender);
	if(VALID_POINT(pSender) && !pSender->is_leave_pricitice())
	{
		NET_SIS_GDQuestTimeOut send;
		send.u16QuestID = p->u16QuestID;
		send.n64Serial = p->n64Serial;
		pSender->SendMessage(&send, send.dw_size);

		quest* pQuest = pSender->get_quest(p->u16QuestID);
		if(VALID_POINT(pQuest))
		{
			pQuest->get_quest_flag().dwQuestBeGuerdon = 0;
			NET_SIS_UpdateQuestFlag send;
			send.u16QuestID = pQuest->get_id();
			send.dwFlag = pQuest->get_quest_flag();
			pSender->SendMessage(&send, send.dw_size);
		}
	}
	else if(VALID_POINT(p->dwSender))
	{
		tagDWORDQuestFlag flag(0), flagEx(-1);
		flag.dwQuestBeGuerdon = 0; flagEx.dwQuestBeGuerdon = 0;
		NET_DB2S_UpdateRoleQuestFlag send;
		send.dwRoleID = p->dwSender;
		send.u16QuestID = p->u16QuestID;
		send.dwFlag = flag; send.dwFlagEx = flagEx;
		g_dbSession.Send(&send, send.dw_size);
	}
	else{ ASSERT(FALSE);}
	
	//删除发布映射
	{
		IDRefHelper* pHelper = mPuts.find(p->dwSender);
		if(VALID_POINT(pHelper)) pHelper->Remove(p->u16QuestID);
	}


	// 接取人处理
	Role* pReciver = g_roleMgr.get_role(p->dwReciver);
	if(VALID_POINT(pReciver) && !pReciver->is_leave_pricitice())
	{	
		NET_SIS_GDQuestTimeOut send;
		send.u16QuestID = QuestIDHelper::GenerateID(p->u16QuestID, TRUE);
		send.n64Serial = p->n64Serial;
		pReciver->SendMessage(&send, send.dw_size);

		// 删除任务
		NET_SIS_delete_quest send2;
		send2.u16QuestID = send.u16QuestID;
		send2.dw_error_code = E_Success;
		pReciver->SendMessage(&send2, send2.dw_size);

		pReciver->remove_quest(QuestIDHelper::GenerateID(p->u16QuestID, TRUE), FALSE);
	}
	else if(VALID_POINT(p->dwReciver))
	{
		// 发数据库删除
		NET_DB2C_discard_quest send;
		send.dw_role_id = p->dwReciver;
		send.u16_quest_id_ = QuestIDHelper::GenerateID(p->u16QuestID, TRUE);
		g_dbSession.Send(&send, send.dw_size);
	}

	//删除接取映射
	{
		IDRefHelper* pHelper = mGets.find(p->dwReciver);
		if(VALID_POINT(pHelper)) pHelper->Remove(p->u16QuestID);
	}


	QuestRewardDeal(p, EROT_Timeout);
	// 让上层从管理器和数据库里删除悬赏
}
VOID guerdon_quest_mgr::CancelPutGDQuest(Role* pRole, VOID* pMsg)
{
	NET_SIC_CancelPutGDQuest* pProtocol = (NET_SIC_CancelPutGDQuest*)pMsg;

	guerdon_quest* p = mQuests.find(pProtocol->n64Serial);
	if(!VALID_POINT(p)) return;

	NET_SIS_CancelPutGDQuest send;
	send.n64Serial = pProtocol->n64Serial;
	send.u16QuestID = p->u16QuestID;
	if(p->dwSender != pRole->GetID() || VALID_POINT(p->dwReciver))
	{
		send.dwError = EGDE_CantCancelPut;
		pRole->SendMessage(&send, send.dw_size);
		return ;
	}

	{// 善后

		IDRefHelper* pHelper = mPuts.find(pRole->GetID());
		if(VALID_POINT(pHelper)) pHelper->Remove(p->u16QuestID);

		quest* pQuest = pRole->get_quest(p->u16QuestID);
		if(VALID_POINT(pQuest))
		{
			pQuest->get_quest_flag().dwQuestBeGuerdon = 0;

			// 同步状态标志
			NET_SIS_UpdateQuestFlag send;
			send.u16QuestID = pQuest->get_id();
			send.dwFlag = pQuest->get_quest_flag();
			pRole->SendMessage(&send, send.dw_size);
		}

		// 将删除奖励物品
		QuestRewardDeal(p, EROT_Cancel, pRole->GetID());
		DeleteQuestDBAndMem(p);
	}

	send.dwError = E_Success;
	pRole->SendMessage(&send, send.dw_size);
}
VOID guerdon_quest_mgr::SendOnePageToRole(Role* pRole, BYTE byPage)
{
#define ONCESENDGDQUESTMAX 20 // 未压缩≈30*20 bytes
#define CalcMsgSize(Number) \
	(sizeof(NET_SIS_GetOnePageGuerdonQuest)+(Number-1)*sizeof(guerdon_quest_data))

	NET_SIS_GetOnePageGuerdonQuest temp; 
	temp.nNumber = 0;  temp.bHasNext = FALSE; temp.byPage = 0;
	if(mQuests.empty())
	{
		temp.dw_size -= sizeof(guerdon_quest_data);
		pRole->SendMessage(&temp,temp.dw_size);
		return ;
	}

	INT tBeginIndex = ONCESENDGDQUESTMAX * byPage;
	if(tBeginIndex >= mQuests.size())
	{
		INT tTotalPage = mQuests.size() / ONCESENDGDQUESTMAX;
		if((mQuests.size() % ONCESENDGDQUESTMAX)) ++tTotalPage;
		temp.byPage = tTotalPage - 1;
		tBeginIndex = temp.byPage * ONCESENDGDQUESTMAX;
	} else temp.byPage = byPage;

	INT32 nMsgSize = CalcMsgSize(ONCESENDGDQUESTMAX);
	CREATE_MSG( pSend, nMsgSize, NET_SIS_GetOnePageGuerdonQuest);
	pSend->dw_message_id = temp.dw_message_id;
	pSend->nNumber = 0; pSend->byPage = temp.byPage;

	guerdon_quest_data* pData = pSend->stData;
	guerdon_quest* pGuerdonQuest = 0; mQuests.reset_iterator();
	for(INT n = 0; n < tBeginIndex; ++n) mQuests.find_next(pGuerdonQuest);

	BOOL bSnd = FALSE;
	while(mQuests.find_next(pGuerdonQuest))
	{
		if(pSend->nNumber >= ONCESENDGDQUESTMAX)
		{
			ASSERT(pSend->nNumber<=ONCESENDGDQUESTMAX);
			pSend->bHasNext = TRUE;
			pSend->dw_size = CalcMsgSize(pSend->nNumber);
			pRole->SendMessage(pSend, pSend->dw_size);
			pSend->nNumber = 0; pData = pSend->stData;
			bSnd = TRUE;
			break;
		}

		//if(pGuerdonQuest->dwSender == pRole->GetID() || 
		//	!VALID_POINT(pGuerdonQuest->dwReciver) )
		{
			FillDataWithQuest(pData, pGuerdonQuest);
			++pData; ++pSend->nNumber;
		}
	}

	if(pSend->nNumber > 0 || !bSnd)
	{
		pSend->bHasNext = pSend->nNumber ? mQuests.find_next(pGuerdonQuest) : FALSE;
		pSend->dw_size = CalcMsgSize(pSend->nNumber);
		pRole->SendMessage(pSend, pSend->dw_size);
	}

#undef  CalcMsgSize
}

VOID guerdon_quest_mgr::UpdateMyPutGDQuest(Role* pRole)
{
	NET_SIS_UpdateMyPutGuerdonQuest send;
	send.byNumber = 0; send.dw_size -= sizeof(send.stData);

	IDRefHelper* pHelper = mPuts.find(pRole->GetID());
	if(VALID_POINT(pHelper))
	{
		for(INT n = 0; n < pHelper->nNumber; ++n)
		{
			guerdon_quest* pGDQuest = mQuests.find(pHelper->n64ID[n]);
			if(VALID_POINT(pGDQuest))
			{
				FillDataWithQuest(&send.stData[send.byNumber], pGDQuest);
				++send.byNumber;
			}
		}
	}

	send.dw_size += send.byNumber * sizeof(guerdon_quest_data);
	pRole->SendMessage(&send, send.dw_size);
}
VOID guerdon_quest_mgr::UpdateMyGetGDQuest(Role* pRole)
{
	NET_SIS_UpdateMyGetGuerdonQuest send;
	send.byNumber = 0; send.dw_size -= sizeof(send.stData);

	IDRefHelper* pHelper = mGets.find(pRole->GetID());
	if(VALID_POINT(pHelper))
	{
		for(INT n = 0; n < pHelper->nNumber; ++n)
		{
			guerdon_quest* pGDQuest = mQuests.find(pHelper->n64ID[n]);
			if(VALID_POINT(pGDQuest))
			{
				FillDataWithQuest(&send.stData[send.byNumber], pGDQuest);
				++send.byNumber;
			}
		}
	}

	send.dw_size += send.byNumber * sizeof(guerdon_quest_data);
	pRole->SendMessage(&send, send.dw_size);
}
VOID guerdon_quest_mgr::UpdateQuest2DB(guerdon_quest* p)
{
	NET_DB2S_UpdateGuerdonQuest send;
	send.n64Serial = p->n64Serial;
	send.dwReciver = p->dwReciver;
	send.eState = p->eState;
	g_dbSession.Send(&send, send.dw_size);
}

VOID guerdon_quest_mgr::DeleteQuestDBAndMem(guerdon_quest* p)
{
	NET_DB2S_DelGuerdonQuest send;
	send.n64Serial = p->n64Serial;
	g_dbSession.Send(&send, send.dw_size);

	mQuests.erase(p->n64Serial);
	SAFE_DELETE(p);
}

VOID guerdon_quest_mgr::QuestRewardDeal(guerdon_quest* p, ERwardOpType eOp, DWORD dwParam1)
{
	s_role_info* s_sender_info = g_roleMgr.get_role_info(p->dwSender);
	if( ((eOp == EROT_Timeout) && !VALID_POINT(s_sender_info)) )// 发布删除账号
	{
		// 将删除奖励物品
		if(VALID_POINT(p->pItem0))
		{
			NET_DB2S_DeleteQuestRewardItem send;
			send.byContainerType = EICT_GDQuest;
			send.dwAccountID = p->u16QuestID;
			send.dwOwner = p->dwSender;
			send.n64Serial = p->pItem0->n64_serial;
			g_dbSession.Send(&send, send.dw_size);
			::Destroy(p->pItem0); p->pItem0 = 0;
		}

		if(VALID_POINT(p->pItem1))
		{
			NET_DB2S_DeleteQuestRewardItem send;
			send.byContainerType = EICT_GDQuest;
			send.dwAccountID = p->u16QuestID;
			send.dwOwner = p->dwSender;
			send.n64Serial = p->pItem1->n64_serial;
			g_dbSession.Send(&send, send.dw_size);
			::Destroy(p->pItem1); p->pItem1 = 0;
		}
		return ;
	}
	
	TCHAR szContent[128]={0};
	TCHAR szTitle[64] = {0}; 
	const tagQuestProto* pProto = g_questMgr.get_protocol(p->u16QuestID);

	ERewardMailOP eMailOP = GetMailTitleAndContent(p, eOp);
	_stprintf(szTitle, _T("&xuanshang%d&"), eMailOP);
	_stprintf(szContent, _T("%d"), VALID_POINT(pProto) ?  pProto->id : 0);

	tagMailBase stMailBase;
	ZeroMemory(&stMailBase, sizeof(stMailBase));
	stMailBase.dwSendRoleID = INVALID_VALUE;
	stMailBase.dwRecvRoleID = INVALID_VALUE;
	stMailBase.byType = 0; stMailBase.dwGiveMoney = 0;

	switch(eOp)
	{
	case EROT_Complete:
		stMailBase.dwRecvRoleID = p->dwReciver;
		stMailBase.byType = 1;
		stMailBase.dwGiveMoney =  p->u16YuanBao;
		break;
// 	case EROT_GiveUp:
// 		stMailBase.dwRecvRoleID = p->dwSender;
// 		stMailBase.byType = 1;
// 		stMailBase.dwGiveMoney =  p->u16YuanBao;
//			break;
	case EROT_Cancel:
		stMailBase.dwRecvRoleID = p->dwSender;
		stMailBase.byType = 1;
		stMailBase.dwGiveMoney =  p->u16YuanBao;
		break;
	case EROT_Timeout:
		stMailBase.dwRecvRoleID = p->dwSender;
		stMailBase.byType = 1;
		stMailBase.dwGiveMoney =  p->u16YuanBao;
		break;
	default: ASSERT(FALSE); return;
	}

	tagItem* pItemArray[2] = {0}; INT nNumber = 0;
	if(VALID_POINT(p->pItem0)){pItemArray[nNumber] = p->pItem0; ++nNumber;}
	if(VALID_POINT(p->pItem1)){pItemArray[nNumber] = p->pItem1; ++nNumber;}

	if(nNumber == 0) g_mailmgr.CreateMail(stMailBase, szTitle, szContent); 
	else g_mailmgr.CreateMail(stMailBase, szTitle, szContent, pItemArray, nNumber);

	if(VALID_POINT(p->dwReciver))
	{
		DWORD dwRewardGetter = stMailBase.dwRecvRoleID;
		ZeroMemory(&stMailBase, sizeof(stMailBase));
		stMailBase.dwSendRoleID = INVALID_VALUE;
		stMailBase.dwRecvRoleID = dwRewardGetter;
		stMailBase.dwGiveMoney = TAKEQUESTFOREGIFTSILVER;

		eMailOP = GetMailTitleAndContent(p, eOp, TRUE);
		_stprintf(szTitle, _T("&xuanshang%d&"), eMailOP);
		_stprintf(szContent, _T("%d"), VALID_POINT(pProto) ?  pProto->id : 0);

		g_mailmgr.CreateMail(stMailBase, szTitle, szContent);
	}
}
BOOL guerdon_quest_mgr::RewardCheck(const tagItem* pItem)
{
	if(MIsEquipment(pItem->dw_data_id)) return FALSE;
	if(!VALID_POINT(pItem->pProtoType)) return FALSE;
	if(pItem->nUseTimes) return FALSE;
	// 检查指定物品ID
	return GDQuestRewardCheck(pItem->dw_data_id);
}
ERewardMailOP guerdon_quest_mgr::GetMailTitleAndContent(const guerdon_quest* p, ERwardOpType eOp, BOOL dwGoldFlag)
{
	if(dwGoldFlag && VALID_POINT(p->dwReciver))
	{
		if(eOp == EROT_Complete)
				return ERMOP_CompleteGold;
		return ERMOP_FailedGold;
	}

	switch (eOp)
	{
	case EROT_Cancel:
		return ERMOP_Cancel;
	case EROT_Complete:
		return ERMOP_Complete;
	case EROT_Timeout:
		{
			if(VALID_POINT(p->dwReciver)) return ERMOP_Failed;
			return ERMOP_Timeout;
		}
	}

	ASSERT(FALSE);
	return ERMOP_Cancel;
}
VOID guerdon_quest_mgr::RegisterEvent()
{
	Super::RegisterEventFunc(EVT_CompGDQuest,	&guerdon_quest_mgr::EvtCompGDQuest);
}

//--------------------------------------------------------------------
// 需通过AddEvet来处理
//--------------------------------------------------------------------
VOID guerdon_quest_mgr::EvtCompGDQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(!VALID_POINT(pRole)) return;

	UINT16 u16 = ((NET_SIC_CompleteGDQuest*)pMsg)->u16QuestID;
	quest* pQuest = pRole->get_quest(u16);
	if(!VALID_POINT(pQuest)) return;

	if(QuestIDHelper::SpecialID(u16) ||
		!pQuest->get_quest_flag().dwCompleteByReciver)
	{
		CompGDQuest(pRole, pMsg);
	}
	else if(pQuest->get_quest_flag().dwCompleteByReciver &&
			pQuest->get_quest_flag().dwQuestBeGuerdon )
	{
		ComplBeGDQuest(pRole, pMsg);
	}	
}
//--------------------------------------------------------------------
// 主线程调用
//--------------------------------------------------------------------
VOID guerdon_quest_mgr::EvtPutQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(VALID_POINT(pRole)) this->PutQuest(pRole, pMsg);
}

VOID guerdon_quest_mgr::EvtGetQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(VALID_POINT(pRole)) this->GetQuest(pRole, pMsg);
}
VOID guerdon_quest_mgr::EvtGetOnePageGDQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	NET_SIC_GetOnePageGuerdonQuest* pProtocol = (NET_SIC_GetOnePageGuerdonQuest*)pMsg;

	if(VALID_POINT(pRole)) this->SendOnePageToRole(pRole, pProtocol->byPage);
}

VOID guerdon_quest_mgr::EvtUpdateMyPutGDQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(VALID_POINT(pRole)) this->UpdateMyPutGDQuest(pRole);
}
VOID guerdon_quest_mgr::EvtUpdateMyGetGDQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(VALID_POINT(pRole)) this->UpdateMyGetGDQuest(pRole);
}
VOID guerdon_quest_mgr::EvtCancelPutQuest(DWORD dwRoleID, VOID* pMsg)
{
	Role* pRole = g_roleMgr.get_role(dwRoleID);
	if(VALID_POINT(pRole)) this->CancelPutGDQuest(pRole, pMsg);
}