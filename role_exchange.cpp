/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��ҽ���

#include "StdAfx.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/exchange_protocol.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../common/ServerDefine/log_server_define.h"

//------------------------------------------------------------------------
// ������
//------------------------------------------------------------------------
DWORD Role::ProcExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID)
{
	// �Ƿ��Լ����Լ�����
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}
	
	// ��ȡ��ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// �������Ƿ���Է�����
	if(!CanExchange())
	{
		return E_Exchange_SelfBusy;
	}
	
	//������������
	if (!IsInRoleState(ERS_SafeArea))
	{
		return E_Exchange_NotInSafearea;
	}
	// �������߼��
	pTarget = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pTarget))
	{
		// �������߲�����,���������߲���ͬһ�ŵ�ͼ��
		return E_Exchange_NotInSame_Map;
	}

	if(pTarget->IsDuel())
	{
		return E_Exchange_Trage_InDuel;
	}

	// ���������Ƿ�ɽ���
	if(!pTarget->CanExchange())
	{
		return E_Exchange_RoleBusy;
	}
	
	//��������������
	if (!pTarget->IsInRoleState(ERS_SafeArea))
	{
		return E_Exchange_NotInSafearea;
	}
	// ���׾���
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		return E_Exchange_OutOfRange;
	}
	
	// ���������߽���״̬
	BeginExchange(dwTgtRoleID);

	// ���ñ�������״̬
	pTarget->GetExchMgr().SetTgtRoleID(GetID());

	return E_Success;
}

//------------------------------------------------------------------------
// ��Ҷ������׵ķ���
//------------------------------------------------------------------------
DWORD Role::ProcExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code)
{
	// �Ƿ��Լ����Լ�����
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	// �жϽ��׶����Ƿ���ȷ
	if(GetExchMgr().GetTgtRoleID() != dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	//// �ٴ��ж��Ƿ�æ
	//if(!CanExchange())
	//{
	//	return E_Exchange_RoleBusy;
	//}

	// ��ȡ��ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// ���������߼��
	pApplicant = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pApplicant))
	{
		// ���������ߣ����߸�����ͼ
		return E_Exchange_NotInSame_Map;
	}

	// �����ߵĽ���Ŀ���Ƿ�仯
	ExchangeMgr &exchMgrApp = pApplicant->GetExchMgr();
	if(pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	//�������������ٴμ�� gx add
	if (!IsInRoleState(ERS_SafeArea))
	{
		if (pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() == GetID())
		{
			// �������׷����ߵĽ���״̬
			pApplicant->EndExchange();
		}
		return E_Exchange_NotInSafearea;
	}

	// ��齻�׷�����
	if(dw_error_code != E_Exchange_Accept)
	{
		// �������׷����ߵĽ���״̬
		pApplicant->EndExchange();
		return dw_error_code;
	}

	// �����߽���״̬���
	if(!pApplicant->IsExchanging())
	{
		exchMgrApp.SetTgtRoleID(INVALID_VALUE);
		//return INVALID_VALUE;
		return E_Exchange_NotAnswer;//gx modify Ϊ�˸��ͻ�����ʾ
	}

	// ���׾���
	if(!IsInDistance(*pApplicant, MAX_EXCHANGE_DISTANCE))
	{
		return E_Exchange_OutOfRange;
	}
	// ������������
	if (!pApplicant->IsInRoleState(ERS_SafeArea))
	{
		if (pApplicant->IsExchanging() && exchMgrApp.GetTgtRoleID() == GetID())
		{
			// �������׷����ߵĽ���״̬
			pApplicant->EndExchange();
		}
		return E_Exchange_NotInSafearea;
	}

	// ���������߽���״̬
	BeginExchange(dwTgtRoleID);

	return E_Exchange_Accept;
}

//------------------------------------------------------------------------
// ��ӽ�����Ʒ
//------------------------------------------------------------------------
DWORD Role::ProcExchangeAdd(OUT Role* &pTarget, OUT tagItem* &pItem, INT32 &nInsIndex, INT64 n64_serial)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ȡ�ý�����Ʒ
	pItem = GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		ASSERT(VALID_POINT(pItem));
		return INVALID_VALUE;
	}

	// ��Ʒ�Ƿ�ɱ���
	if(!GetItemMgr().CanExchange(*pItem))
	{	
		return E_Exchange_ItemCanNot_Exchange;
	}

	if(!MIsEquipment(pItem->dw_data_id))
	{
		if(!VALID_POINT(pItem->pProtoType) || pItem->nUseTimes)
		{
			return E_Exchange_ItemCanNot_Exchange;
		}
	}
	//else
	//{
	//	tagEquip* pEquip = (tagEquip*)pItem;
	//	if (pEquip->equipSpec.nCurLevelExp > 0 || pEquip->equipSpec.nLevel > 1)
	//	{
	//		return E_Exchange_ItemCanNot_Exchange;
	//	}
	//}
	// ���뵽��������Ʒ��
	nInsIndex = GetExchMgr().add_item(n64_serial, pItem->n16Num);
	if(INVALID_VALUE == nInsIndex)
	{
		// ���ڽ����б��У�������Ʒ�����Ѵﵽ����
		return INVALID_VALUE;
	}

	// ���Ŀ������״̬
	pTarget->GetExchMgr().Unlock();

	return E_Success;
}

//------------------------------------------------------------------------
// ȡ��������Ʒ
//------------------------------------------------------------------------
DWORD Role::ProcExchangeDec(OUT Role* &pTarget, INT64 n64_serial)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ���Ŀ������״̬
	pTarget->GetExchMgr().Unlock();
	
	return GetExchMgr().DecItem(n64_serial);
}

//------------------------------------------------------------------------
// �޸Ľ��׽�Ǯ
//------------------------------------------------------------------------
DWORD Role::ProcExchangeMoney(OUT Role* &pTarget, INT64 n64Silver)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ���Ŀ������״̬
	pTarget->GetExchMgr().Unlock();

	// �������Ƿ����㹻�Ľ�Ǯ
	CurrencyMgr &CurMgr = GetCurMgr();
	if(n64Silver > CurMgr.GetBagSilver())
	{
		return E_Exchange_NotEnough_Money;
	}

	GetExchMgr().ResetMoney(n64Silver);

	return E_Success;
}

//------------------------------------------------------------------------
// ��������
//------------------------------------------------------------------------
DWORD Role::ProcExchangeLock(OUT Role* &pTarget)
{
	DWORD dw_error_code = ProcPrepareExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	GetExchMgr().Lock();

	return E_Success;
}

//------------------------------------------------------------------------
// ���ȡ������
//------------------------------------------------------------------------
DWORD Role::ProcExchangeCancel(OUT Role* &pTarget)
{
	// �Ƿ��ڽ���״̬
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// ��ý��׹������ͽ��׶���id
	ExchangeMgr &exchMgr = GetExchMgr();
	DWORD dwTargetRoleID = exchMgr.GetTgtRoleID();
	
	// ȡ������
	EndExchange();

	//// ���������Ƿ�������״̬
	//if(exchMgr.IsLock())
	//{
	//	return INVALID_VALUE;
	//}

	// ��ȡ��ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(dwTargetRoleID);
	if(!VALID_POINT(pTarget))
	{
		// �������߲�����,���������߲���ͬһ�ŵ�ͼ��
		return E_Exchange_NotInSame_Map;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	pTarget->EndExchange();

	return E_Success;
}

//------------------------------------------------------------------------
// ȷ�Ͻ���
//------------------------------------------------------------------------
DWORD Role::ProcExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID)
{
	DWORD dw_error_code = E_Success;

	// ��ý��׹�����
	ExchangeMgr &exchMgr = GetExchMgr();

	// ��ȡ��ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(exchMgr.GetTgtRoleID());
	if(!VALID_POINT(pTarget))
	{
		// �������߲�����,���������߲���ͬһ�ŵ�ͼ��
		dwFailedRoleID	= exchMgr.GetTgtRoleID();
		dw_error_code		= E_Exchange_NotInSame_Map;
	}

	dw_error_code = ProcExchangeLock(pTarget);
	if(INVALID_VALUE == dw_error_code)
	{
		return dw_error_code;
	}

	NET_SIS_exchange_item_lock send;
	send.dwSrcID = GetID();
	send.dw_error_code = dw_error_code;

	SendMessage(&send, send.dw_size);
	if(VALID_POINT(pTarget))
	{
		pTarget->SendMessage(&send, send.dw_size);
	}

	if(dw_error_code != E_Success)
		return INVALID_VALUE;
	
	// �Ƿ��ڽ���״̬
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// ���������Ƿ�������״̬
	if(!exchMgr.IsLock())
	{
		return INVALID_VALUE;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		//return E_Exchange_ApplicantTgt_Change;
		return INVALID_VALUE;
	}

	// Ŀ���Ƿ�������״̬
	if(!pTarget->GetExchMgr().IsLock())
	{
		return INVALID_VALUE;
	}

	exchMgr.Verify();
	pTarget->GetExchMgr().Verify();

	// ���׾���ȷ��
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		dw_error_code = E_Exchange_OutOfRange;
		goto Exit;
	}
	//˫�������ٴμ�� gx add
	if (!IsInRoleState(ERS_SafeArea) || !pTarget->IsInRoleState(ERS_SafeArea))
	{
		dw_error_code = E_Exchange_NotInSafearea;
		goto Exit;
	}

	// �����ռ��� -- �����ռ䲻��ʱ���������������
	BYTE byExTypeNum	= exchMgr.GetItemTypeNum();
	BYTE byTgtExTypeNum	= pTarget->GetExchMgr().GetItemTypeNum();
	if(byExTypeNum > byTgtExTypeNum)
	{
		// ���Է�����
		if(byExTypeNum - byTgtExTypeNum > pTarget->GetItemMgr().GetBagFreeSize())
		{
			// ����
			exchMgr.Unlock();
			pTarget->GetExchMgr().Unlock();

			dwFailedRoleID = pTarget->GetID();
			return E_Exchange_NotEnough_BagSpace;
		}
	}
	else
	{
		// ����Լ�����
		if(byTgtExTypeNum - byExTypeNum > GetItemMgr().GetBagFreeSize())
		{
			// ����
			exchMgr.Unlock();
			pTarget->GetExchMgr().Unlock();

			dwFailedRoleID = GetID();
			return E_Exchange_NotEnough_BagSpace;
		}
	}

	/*************************************************
	* ������Ʒ����
	*************************************************/

	tagItem *pSrcItem[MAX_EXCHANGE_ITEM];
	tagItem *pDstItem[MAX_EXCHANGE_ITEM];
	ZeroMemory(pSrcItem, sizeof(tagItem*) * MAX_EXCHANGE_ITEM);
	ZeroMemory(pDstItem, sizeof(tagItem*) * MAX_EXCHANGE_ITEM);

	// ������Ʒ���
	dw_error_code = VerifyExchangeData(pSrcItem);
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = GetID();
		goto Exit;
	}

	dw_error_code = pTarget->VerifyExchangeData(pDstItem);
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = pTarget->GetID();
		goto Exit;
	}

	// 4.�ƶ���Ʒ

	// 4.1 ��Ǯ -- �ȼ���Ǯ,������ʧ
	GetCurMgr().DecBagSilver(exchMgr.GetMoney(), elcid_exchange_verify, pTarget->GetID());
	pTarget->GetCurMgr().DecBagSilver(pTarget->GetExchMgr().GetMoney(), elcid_exchange_verify, GetID());

	GetCurMgr().IncBagSilver(
		min(GetCurMgr().GetCanIncBagSilver(), pTarget->GetExchMgr().GetMoney()), 
		elcid_exchange_verify, pTarget->GetID());

	pTarget->GetCurMgr().IncBagSilver(
		min(pTarget->GetCurMgr().GetCanIncBagSilver(), exchMgr.GetMoney()), 
		elcid_exchange_verify, GetID());

	// 4.2 ��Ʒ

	// 4.2.1 �ȴ��������ɾ��������Ʒ -- ����ɶѵ���Ʒ�������
	GetItemMgr().RemoveFromBag(exchMgr.GetSerialArray(), MAX_EXCHANGE_ITEM, elcid_exchange_verify, pTarget->GetID());
	pTarget->GetItemMgr().RemoveFromBag(pTarget->GetExchMgr().GetSerialArray(), 
										MAX_EXCHANGE_ITEM, elcid_exchange_verify, GetID());

	// 4.2.2 ��������Ʒ�ŵ��������
	GetItemMgr().Add2Bag(pDstItem, MAX_EXCHANGE_ITEM, elcid_exchange_verify, pTarget->GetID());
	pTarget->GetItemMgr().Add2Bag(pSrcItem, MAX_EXCHANGE_ITEM, elcid_exchange_verify, GetID());


	// �ɾ�
	GetAchievementMgr().UpdateAchievementCriteria(eta_exchange, 1);
	pTarget->GetAchievementMgr().UpdateAchievementCriteria(eta_exchange, 1);

Exit:
	// �ͷŽ�����Դ
	EndExchange();

	if(VALID_POINT(pTarget))
	{
		pTarget->EndExchange();
	}
	else
	{
		// �������Ŀ����������ͼ���ô�Ҳ����ֱ�Ӳ���(���̵߳���)
	}

	return dw_error_code;
}

//------------------------------------------------------------------------
// ���������ϵĽ�����Ʒ�Ƿ���ȫ
//------------------------------------------------------------------------
DWORD Role::VerifyExchangeData(OUT tagItem* pItem[])
{
	// ��ý��׹�����
	ExchangeMgr &exchMgr = GetExchMgr();
	
	// 1.��Ǯ�Ƿ񻹹�
	if(GetCurMgr().GetBagSilver() < exchMgr.GetMoney())
	{
		return E_Exchange_NotEnough_Money;
	}

	//// 2.�����Ƿ����㹻�Ŀռ�
	//if(GetItemMgr().GetBagFreeSize() < exchMgr.GetItemTypeNum())
	//{
	//	return E_Exchange_NotEnough_BagSpace;
	//}

	// 3.��������Ʒ�Ƿ�������, ���ڣ��õ���Ʒָ��
	if(!GetItemMgr().CheckExistInBag(pItem, exchMgr.GetSerialArray(), exchMgr.GetNumArray(), MAX_EXCHANGE_ITEM))
	{
		return E_Exchange_ItemCanNot_Find;
	}

	return E_Success;
}

//------------------------------------------------------------------------
// �޸Ľ�������ǰ�ļ�飬���õ���ض���
//------------------------------------------------------------------------
DWORD Role::ProcPrepareExchange(OUT Role* &pTarget)
{
	// �Ƿ��ڽ���״̬
	if(!IsExchanging())
	{
		return INVALID_VALUE;
	}

	// ��ý��׹�����
	ExchangeMgr &exchMgr = GetExchMgr();

	// ���������Ƿ�������״̬
	if(exchMgr.IsLock())
	{
		return INVALID_VALUE;
	}

	// ��ȡ��ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	pTarget = pMap->find_role(exchMgr.GetTgtRoleID());
	if(!VALID_POINT(pTarget))
	{
		// �������߲�����,���������߲���ͬһ�ŵ�ͼ��
		return E_Exchange_NotInSame_Map;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsExchanging() || pTarget->GetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Exchange_ApplicantTgt_Change;
	}

	return E_Success;
}