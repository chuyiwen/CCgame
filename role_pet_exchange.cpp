
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#include "StdAfx.h"
#include "role.h"
#include "map.h"
#include "../../common/WorldDefine/pet_exchange_protocot.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "pet_pocket.h"
#include "pet_soul.h"
#include "pet_server_define.h"
//------------------------------------------------------------------------
// ������
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeReq(OUT Role* &pTarget, DWORD dwTgtRoleID)
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
	if(!CanPetExchange())
	{
		return E_Pet_Exchange_SelfBusy;
	}

	// �������߼��
	pTarget = pMap->find_role(dwTgtRoleID);
	if(!VALID_POINT(pTarget))
	{
		// �������߲�����,���������߲���ͬһ�ŵ�ͼ��
		return E_Pet_Exchange_NotInSame_Map;
	}

	// ���������Ƿ�ɽ���
	if(!pTarget->CanPetExchange())
	{
		return E_Pet_Exchange_RoleBusy;
	}

	// ���׾���
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		return E_Pet_Exchange_OutOfRange;
	}

	// ���������߽���״̬
	BeginPetExchange(dwTgtRoleID);

	// ���ñ�������״̬
	pTarget->GetPetExchMgr().SetTgtRoleID(GetID());

	return E_Pets_Success;
}

//------------------------------------------------------------------------
// ��Ҷ������׵ķ���
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeReqRes(OUT Role* &pApplicant, DWORD dwTgtRoleID, DWORD dw_error_code)
{
	// �Ƿ��Լ����Լ�����
	if(GetID() == dwTgtRoleID)
	{
		return INVALID_VALUE;
	}

	// �жϽ��׶����Ƿ���ȷ
	if(GetPetExchMgr().GetTgtRoleID() != dwTgtRoleID)
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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// �����ߵĽ���Ŀ���Ƿ�仯
	PetExchangeMgr &exchMgrApp = pApplicant->GetPetExchMgr();
	if(pApplicant->IsPetExchanging() && exchMgrApp.GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	// ��齻�׷�����
	if(dw_error_code != E_Pet_Exchange_Accept)
	{
		// �������׷����ߵĽ���״̬
		pApplicant->EndPetExchange();
		return dw_error_code;
	}

	// �����߽���״̬���
	if(!pApplicant->IsPetExchanging())
	{
		exchMgrApp.SetTgtRoleID(INVALID_VALUE);
		return INVALID_VALUE;
	}

	// ���׾���
	if(!IsInDistance(*pApplicant, MAX_EXCHANGE_DISTANCE))
	{
		return E_Pet_Exchange_OutOfRange;
	}

	// ���������߽���״̬
	BeginPetExchange(dwTgtRoleID);

	return E_Pet_Exchange_Accept;
}

//------------------------------------------------------------------------
// ��ӽ�����Ʒ
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeAdd( OUT Role* &pTarget, DWORD dwPetID )
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ȡ�ý�����Ʒ
	PetSoul* pSoul = GetPetPocket()->GetPetSoul(dwPetID);
	if(!VALID_POINT(pSoul))
	{
		ASSERT(VALID_POINT(pSoul));
		return INVALID_VALUE;
	}

	// ��Ʒ�Ƿ�ɱ���
	DWORD dwRtv = GetPetPocket()->CanExchange(dwPetID);
	if(dwRtv != E_Success)
	{
		return dwRtv;
	}

	if( pSoul->GetProto()->nRoleLvlLim > pTarget->get_level())
	{	
		return E_Pet_Exchange_RoleLvlNotEnough;
	}

	// ���뵽��������Ʒ��
	dw_error_code = GetPetExchMgr().AddPet(dwPetID);
	if(E_Success != dw_error_code)
	{
		// ���ڽ����б��У�������Ʒ�����Ѵﵽ����
		return INVALID_VALUE;
	}

	// ���Ŀ������״̬
	pTarget->GetPetExchMgr().Unlock();

	return E_Success;
}

//------------------------------------------------------------------------
// ȡ��������Ʒ
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeDec( OUT Role* &pTarget, DWORD dwPetID )
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ���Ŀ������״̬
	pTarget->GetPetExchMgr().Unlock();

	return GetPetExchMgr().DecPet(dwPetID);
}

//------------------------------------------------------------------------
// �޸Ľ��׽�Ǯ
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeMoney(OUT Role* &pTarget, INT64 n64Silver)
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ���Ŀ������״̬
	pTarget->GetPetExchMgr().Unlock();

	// �������Ƿ����㹻�Ľ�Ǯ
	CurrencyMgr &CurMgr = GetCurMgr();
	if(n64Silver > CurMgr.GetBagSilver())
	{
		return E_Pet_Exchange_NotEnough_Money;
	}

	GetPetExchMgr().ResetMoney(n64Silver);

	return E_Success;
}

//------------------------------------------------------------------------
// ��������
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeLock(OUT Role* &pTarget)
{
	DWORD dw_error_code = ProcPreparePetExchange(pTarget);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	GetPetExchMgr().Lock();

	return E_Success;
}

//------------------------------------------------------------------------
// ���ȡ������
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeCancel(OUT Role* &pTarget)
{
	// �Ƿ��ڽ���״̬
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// ��ý��׹������ͽ��׶���id
	PetExchangeMgr &exchMgr = GetPetExchMgr();
	DWORD dwTargetRoleID = exchMgr.GetTgtRoleID();

	// ȡ������
	EndPetExchange();

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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	pTarget->EndPetExchange();

	return E_Success;
}

//------------------------------------------------------------------------
// ȷ�Ͻ���
//------------------------------------------------------------------------
DWORD Role::ProcPetExchangeVerify(OUT Role* &pTarget, OUT DWORD &dwFailedRoleID)
{
	DWORD dw_error_code = E_Success;

	// �Ƿ��ڽ���״̬
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// ��ý��׹�����
	PetExchangeMgr &exchMgr = GetPetExchMgr();

	// ���������Ƿ�������״̬
	if(!exchMgr.IsLock())
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
		dwFailedRoleID	= exchMgr.GetTgtRoleID();
		dw_error_code		= E_Pet_Exchange_NotInSame_Map;
		goto Exit;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		//return E_Exchange_ApplicantTgt_Change;
		return INVALID_VALUE;
	}

	// Ŀ���Ƿ�������״̬
	if(!pTarget->GetPetExchMgr().IsLock())
	{
		return INVALID_VALUE;
	}

	exchMgr.Verify();
	if(!pTarget->GetPetExchMgr().IsVerify())
	{
		return INVALID_VALUE;
	}

	// ���׾���ȷ��
	if(!IsInDistance(*pTarget, MAX_EXCHANGE_DISTANCE))
	{
		dw_error_code = E_Pet_Exchange_OutOfRange;
		goto Exit;
	}

	// �����ռ��� -- �����ռ䲻��ʱ���������������
	BYTE byMyPetNum	= exchMgr.GetPetNum();
	BYTE byTgtPetNum	= pTarget->GetPetExchMgr().GetPetNum();
	if(byMyPetNum > byTgtPetNum)
	{
		// ���Է�����
		if(byMyPetNum - byTgtPetNum > pTarget->GetPetPocket()->GetFreeSize())
		{
			// ����
			exchMgr.Unlock();
			pTarget->GetPetExchMgr().Unlock();

			dwFailedRoleID = pTarget->GetID();
			return E_Pet_Exchange_NotEnough_Space;
		}
	}
	else
	{
		// ����Լ�����
		if(byTgtPetNum - byMyPetNum >GetPetPocket()->GetFreeSize())
		{
			// ����
			exchMgr.Unlock();
			pTarget->GetPetExchMgr().Unlock();

			dwFailedRoleID = GetID();
			return E_Pet_Exchange_NotEnough_Space;
		}
	}

	/*************************************************
	* ������Ʒ����
	*************************************************/

	PetSoul *pSrcItem[MAX_EXCHANGE_ITEM];
	PetSoul *pDstItem[MAX_EXCHANGE_ITEM];
	ZeroMemory(pSrcItem, sizeof(PetSoul*) * MAX_EXCHANGE_ITEM);
	ZeroMemory(pDstItem, sizeof(PetSoul*) * MAX_EXCHANGE_ITEM);

	// ������Ʒ���
	dw_error_code = VerifyPetExchangeData();
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = GetID();
		goto Exit;
	}

	dw_error_code = pTarget->VerifyPetExchangeData();
	if(dw_error_code != E_Success)
	{
		dwFailedRoleID = pTarget->GetID();
		goto Exit;
	}
 
 	// 4.�ƶ���Ʒ

	// 4.1 ��Ǯ -- �ȼ���Ǯ,������ʧ
	GetCurMgr().DecBagSilver(exchMgr.GetMoney(), elcid_exchange_verify, pTarget->GetID());
	pTarget->GetCurMgr().DecBagSilver(pTarget->GetPetExchMgr().GetMoney(), elcid_exchange_verify, GetID());

	GetCurMgr().IncBagSilver(
		min(GetCurMgr().GetCanIncBagSilver(), pTarget->GetPetExchMgr().GetMoney()), 
		elcid_exchange_verify, pTarget->GetID());

	pTarget->GetCurMgr().IncBagSilver(
		min(pTarget->GetCurMgr().GetCanIncBagSilver(), exchMgr.GetMoney()), 
		elcid_exchange_verify, GetID());

	// 4.2 ��Ʒ

	// 4.2.1 �ȴ��������ɾ��������Ʒ -- ����ɶѵ���Ʒ�������
	GetPetPocket()->TakeFromPocket(pSrcItem, MAX_EXCHANGE_ITEM, exchMgr.GetPetIDs(), exchMgr.GetPetNum());
	pTarget->GetPetPocket()->TakeFromPocket(pDstItem, MAX_EXCHANGE_ITEM, pTarget->GetPetExchMgr().GetPetIDs(), pTarget->GetPetExchMgr().GetPetNum());

	// 4.2.2 ��������Ʒ�ŵ��������
	GetPetPocket()->PutInPocket(pDstItem, pTarget->GetPetExchMgr().GetPetNum());
	pTarget->GetPetPocket()->PutInPocket(pSrcItem, exchMgr.GetPetNum());

Exit:
	// �ͷŽ�����Դ
	EndPetExchange();

	if(VALID_POINT(pTarget))
	{
		pTarget->EndPetExchange();
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
DWORD Role::VerifyPetExchangeData()
{
	// ��ý��׹�����
	PetExchangeMgr &exchMgr = GetPetExchMgr();

	// 1.��Ǯ�Ƿ񻹹�
	if(GetCurMgr().GetBagSilver() < exchMgr.GetMoney())
	{
		return E_Pet_Exchange_NotEnough_Money;
	}

	//// 2.�����Ƿ����㹻�Ŀռ�
	//if(GetItemMgr().GetBagFreeSize() < exchMgr.GetItemTypeNum())
	//{
	//	return E_Exchange_NotEnough_BagSpace;
	//}

	// 3.��������Ʒ�Ƿ�������, ���ڣ��õ���Ʒָ��
	if(!GetPetPocket()->CheckExistInPocket(exchMgr.GetPetIDs(), MAX_EXCHANGE_ITEM))
	{
		return E_Pet_Exchange_PetCanNot_Find;
	}

	return E_Success;
}

//------------------------------------------------------------------------
// �޸Ľ�������ǰ�ļ�飬���õ���ض���
//------------------------------------------------------------------------
DWORD Role::ProcPreparePetExchange(OUT Role* &pTarget)
{
	// �Ƿ��ڽ���״̬
	if(!IsPetExchanging())
	{
		return INVALID_VALUE;
	}

	// ��ý��׹�����
	PetExchangeMgr &exchMgr = GetPetExchMgr();

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
		return E_Pet_Exchange_NotInSame_Map;
	}

	// Ŀ��û�д��ڽ���״̬�������ںͱ��˽���
	if(!pTarget->IsPetExchanging() || pTarget->GetPetExchMgr().GetTgtRoleID() != GetID())
	{
		return E_Pet_Exchange_ApplicantTgt_Change;
	}

	return E_Success;
}