/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����װ��

#include "StdAfx.h"
#include "pet_equip.h"

#include "../../common/WorldDefine/pet_protocol.h"
#include "../../common/WorldDefine/pet_equip_protocol.h"

#include "../common/ServerDefine/item_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "pet_skill_server_define.h"

#include "pet_soul.h"
#include "pet_skill.h"
#include "pet_att.h"
#include "role.h"
#include "item_mgr.h"

//----------------------------------------------------------------------------------------------------
// ͨ��64λID��װ����Ʒ����Ʒ����ұ�����δ���Ƴ���
//----------------------------------------------------------------------------------------------------
DWORD PetEquipBar::Equip( INT64 n64ItemID, INT8 n8DstPos, BOOL bSend /*= FALSE*/ )
{
	INT16 n16DstPos = (INT16)n8DstPos;

 	M_trans_else_ret(pMaster, m_pSoul->GetMaster(), Role, E_Pets_PetEquip_Master_NotExist);

	// ��ұ����л�ȡ��Ʒ
	M_trans_else_ret(p2Equip, GetFromRoleBag(n64ItemID), tagItem, E_Pets_PetEquip_PetEquipNotInBag);

	// ���ԭ��
	M_trans_else_ret(p2EquipPetEquipProto, GetPetEquipProto(p2Equip), const tagPetEquipProto, E_Pets_PetEquip_UnknownEquipProto)

	// �������Ƴ�
	pMaster->GetItemMgr().TakeOutFromBag(n64ItemID, elcid_pet_item_pet_equip, TRUE);

	//����װ����ÿ��λ��װ��
	for (INT16 i=0; i<PetEquipContainer::GetCurSpaceSize(); ++i)
	{
		tagItem* pCurItem = PetEquipContainer::GetItem(i);
		if (!VALID_POINT(pCurItem)) 
			continue;

		const tagPetEquipProto* pCurProto = GetPetEquipProto(pCurItem);
		if (!VALID_POINT(pCurProto)) 
			continue;
		//����װ��ͬ����Ψһ�����λ��װ��������ͬ����װ��Ψһ�����λ��װ��typeid��ͬ
		if (p2EquipPetEquipProto->bTypeUnique && p2EquipPetEquipProto->nType == pCurProto->nType ||
			p2EquipPetEquipProto->bUnique && p2Equip->dw_data_id == pCurItem->dw_data_id)
		{
			//ж�ظ�λ��װ��
			UnEquip(pCurItem->GetKey(), INVALID_VALUE, TRUE);
		}
	}
		
	//��ָ��λ��
	if (VALID_VALUE(n16DstPos))
	{
		//Ŀ��λ�ò�����
		if (!PetEquipContainer::IsOnePlaceFree(n16DstPos))
		{
			//Ŀ��λ��жװ
			tagItem* pItem = PetEquipContainer::GetItem(n16DstPos);
			if (VALID_POINT(pItem))
			{
				UnEquip(pItem->GetKey(), INVALID_VALUE, TRUE);
			}
		}
	}
	//����ָ��λ��
	else
	{
		//Ŀ��λ������Ϊ�ҵ�һ������λ��
		if (PetEquipContainer::GetFreeSpaceSize() != 0)
			for (INT16 i=0; i<PetEquipContainer::GetCurSpaceSize(); ++i)
			{
				if (!PetEquipContainer::IsOnePlaceFree(i)) continue;
				n16DstPos = i;
				break;
			}
		else
		{
			n16DstPos = 0;
			tagItem* pCurItem = PetEquipContainer::GetItem(n16DstPos);
			ASSERT(VALID_POINT(pCurItem)) ;
			DWORD dwRtv = UnEquip(pCurItem->GetKey(), INVALID_VALUE, TRUE);
			ASSERT(dwRtv == E_Success);
		}
	}

	// ��׼��
	if (n16DstPos > SIZE_PET_EQUIP_BAR || n16DstPos < 0)
	{
		n16DstPos = 0;
	}
	
	AddEquip(p2Equip, n16DstPos);

	// ���뵽���ݿ�
	InsertDB(p2Equip);
	p2Equip->SetUpdate(EUDBS_Null);

	if (bSend)
	{
		// ͬ���ͻ���
		NET_SIS_pet_equip send;
		send.dwErrCode = E_Success;
		send.dwPetID = m_pSoul->GetID();	
		FillClientEquipData(&send.itemData, p2Equip);
		pMaster->SendMessage(&send, send.dw_size);
	}
 	return E_Pets_Success;
}

//----------------------------------------------------------------------------------------------------
// ͨ��64λID��жװ����Ʒ���뵽���񱳰�
//----------------------------------------------------------------------------------------------------
DWORD PetEquipBar::UnEquip( INT64 n64ItemID, INT16 n16DstPos, BOOL bSend /*= FALSE*/ )
{
	M_trans_else_ret(pMaster, m_pSoul->GetMaster(), Role, E_Pets_PetEquip_Master_NotExist);

	//��ָ��λ��
	if (VALID_VALUE(n16DstPos))
	{
		//Ŀ��λ�ò�����
		if (!pMaster->GetItemMgr().IsBagOneSpaceFree(n16DstPos))
		{
			//���ش���
			return E_Pets_PetEquip_BagIndexNotFree;
		}
	}
	//����ָ��λ��
	else
	{
		//�ҵ�����Ŀ��λ��
		n16DstPos = m_pSoul->GetMaster()->GetItemMgr().GetBagOneFreeSpace();
		//���޿���λ��
		if (!VALID_VALUE(n16DstPos))
		{
			//���ش���	
			return E_Pets_PetEquip_BagIndexNotFree;
		}
	}

	// �ӳ���װ�����Ƴ�
	tagItem* p2UnEquip = PetEquipContainer::Remove(n64ItemID);

	//��ʱ�޹������Ƴ�
	if (p2UnEquip->is_time_limit())
	{
		m_TimeLimitMgr.RemoveFromTimeLeftList(p2UnEquip->n64_serial);
	}	
	
	// �����ݿ��Ƴ�
	RemoveDB(p2UnEquip);
	p2UnEquip->SetUpdate(EUDBS_Null);

	// ����Ч��
	CalcEffect(m_pSoul, p2UnEquip->pProtoType->nSpecFuncVal1, FALSE, TRUE);

	DWORD dwRtv = pMaster->GetItemMgr().Add2BagByIndexAndInsertDB(p2UnEquip, elcid_pet_item_pet_equip, n16DstPos);
	if (E_Pets_Success != dwRtv)
	{
		::Destroy(p2UnEquip);
	}

	// ͬ���ͻ���
	if (E_Success == dwRtv && bSend)
	{
		NET_SIS_pet_unequip send;
		send.n64ItemID	= p2UnEquip->GetKey();
		send.dwErrCode = E_Success;
		pMaster->SendMessage(&send, send.dw_size);
	}

	return dwRtv ;
}

//----------------------------------------------------------------------------------------------------
// ����װ��Ч��
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::CalcEffect(PetSoul* pSoul, DWORD dwPetEquipTypeID, BOOL bEquip, BOOL bSend)
{
	const tagPetEquipProto* pProto = AttRes::GetInstance()->GetPetEquipProto(dwPetEquipTypeID);
	if (!VALID_POINT(pProto) || !VALID_POINT(pSoul))
	{
		ASSERT(0);
		return ;
	}

	// �Ƿ�жװ
	INT nCoef = bEquip ? 1 : -1;

	std::list<PetSkill*> listSkills;

	// ����Ч��
	for (INT i=0; i<MAX_PET_EQUIP_MOD_ATT && VALID_VALUE(pProto->nPetAtt[i]); ++i)
	{
		INT nPetAtt = pProto->nPetAtt[i];
		listSkills.clear();
		m_pSoul->ExportSpecSkill(nPetAtt, listSkills);

		DisablePetSkill(listSkills);
		pSoul->GetPetAtt().ModAttVal(nPetAtt, nCoef * pProto->nPetAttMod[i], bSend);
		EnablePetSkill(listSkills);
	}
}

//----------------------------------------------------------------------------------------------------
// ���캯��
//----------------------------------------------------------------------------------------------------
PetEquipBar::PetEquipBar(PetSoul* pSoul) 
	:PetEquipContainer(EICT_PetEquip, SIZE_PET_EQUIP_BAR, SIZE_PET_EQUIP_BAR), m_TimeLimitMgr(PET_EQUIP_UPDATE_TIME), m_pSoul(pSoul)
{
	ASSERT(VALID_POINT(m_pSoul));
}

//----------------------------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::Update()
{
	m_TimeLimitMgr.Update();
	package_list<INT64>& list2Del = m_TimeLimitMgr.GetNeedDelList();
	while (!list2Del.empty())
	{
		// ����Ʒ���ڣ���򵥵İ��Ƴ������������ﱳ��
		INT64 n64ItemID = list2Del.pop_front();
		if(IsEquipExist(n64ItemID))
			UnEquip(n64ItemID, INVALID_VALUE, TRUE);
	}
}

//----------------------------------------------------------------------------------------------------
// �����ﱳ���л�ȡ����װ��
//----------------------------------------------------------------------------------------------------
tagItem* PetEquipBar::GetFromRoleBag( INT64 n64ItemID )
{
	M_trans_else_ret(pMaster, m_pSoul->GetMaster(), Role, NULL);
	tagItem* pToGet = pMaster->GetItemMgr().GetBagItem(n64ItemID);
	if (!VALID_POINT(pToGet))
	{
		ASSERT(0);
	}
	return pToGet;
}

//----------------------------------------------------------------------------------------------------
// �ӳ������ϻ�ȡ����װ��
//----------------------------------------------------------------------------------------------------
tagItem* PetEquipBar::GetFromPet( INT64 n64ItemID )
{
	tagItem* pToGet = PetEquipContainer::GetItem(n64ItemID);
	if (!VALID_POINT(pToGet))
	{
		ASSERT(0);	
	}
	return pToGet;
}

//----------------------------------------------------------------------------------------------------
// ��ȡ�ͻ���װ���ṹ
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::FillClientEquipData( tagPetEquipMsgInfo* pMsgInfo, tagItem* pItem)
{
	if (VALID_POINT(pItem))
	{
		pMsgInfo->n64ItemID		= pItem->GetKey();
		pMsgInfo->dwItemProtoID	= pItem->dw_data_id;
		pMsgInfo->n8Pos			= (INT8)pItem->n16Index;
	}
}

//----------------------------------------------------------------------------------------------------
// ����װ��λ�ý���
//----------------------------------------------------------------------------------------------------
DWORD PetEquipBar::SwapEquipPos( INT64 n64ItemID, INT8 n8DstPos, BOOL bSend /*= FALSE*/ )
{
	
	INT16 n16DstPos = (INT16)n8DstPos;
	tagItem* pItem = GetFromPet( n64ItemID);
	if (!VALID_POINT(pItem))
	{
		return E_Pets_PetEquip_EquipIsNotOnPet;
	}else if (n16DstPos < 0 || n16DstPos >= PetEquipContainer::GetCurSpaceSize())
	{
		return E_Pets_PetEquip_InvalidDstPos;
	}
	else if (pItem->n16Index == n16DstPos)
	{
		return E_Pets_PetEquip_EquipAlreadyInPos;
	}

	INT16 n16SrcPos = pItem->n16Index;

	if (PetEquipContainer::IsOnePlaceFree(n16DstPos))
	{
		// ֱ�ӷŵ����λ��
		tagItem* pItem = PetEquipContainer::Remove(n64ItemID);
		ASSERT(VALID_POINT(pItem));
		PetEquipContainer::Add(pItem, n16DstPos);
	}
	else
	{
		// ����
		PetEquipContainer::Swap(pItem->n16Index, n16DstPos);
	}

	if (bSend)
	{
		NET_SIS_pet_equip_position_swap send;
		send.dwErrCode = E_Success;
		send.n64ItemID = pItem->GetKey();
		send.n8PosSrc = (INT8)n16SrcPos;
		send.n8PosDst = (INT8)pItem->n16Index;
		m_pSoul->GetMaster()->SendMessage(&send, send.dw_size);
	}

	return E_Pets_Success;
}

//----------------------------------------------------------------------------------------------------
// �����ݿ�
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::SaveToDB( OUT LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num )
{
	n_num = 0;

	M_trans_pointer(pCurPointer, pData, s_item_update);

	tagItem	*pTemp	= NULL;
	for(INT16 i=0; i<PetEquipContainer::GetCurSpaceSize(); ++i)
	{
		pTemp = PetEquipContainer::GetItem(i);
		if(VALID_POINT(pTemp) && pTemp->eStatus != EUDBS_Null)
		{
			pCurPointer[n_num].n64_serial		= pTemp->n64_serial;
			pCurPointer[n_num].dw_owner_id		= pTemp->dwOwnerID;
			pCurPointer[n_num].n_use_times		= pTemp->nUseTimes;
			pCurPointer[n_num].n16_num		= pTemp->n16Num;
			pCurPointer[n_num].n16_index		= pTemp->n16Index;

			pTemp->SetUpdate(EUDBS_Null);

			++n_num;
		}
	}

	pOutPointer = &pCurPointer[n_num];
}

//----------------------------------------------------------------------------------------------------
// �������ݿ�
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::InsertDB( tagItem* p2Equip )
{
	NET_DB2C_new_item send;
	get_fast_code()->memory_copy(&send.item, p2Equip, SIZE_ITEM);
	g_dbSession.Send(&send, send.dw_size);
}

//----------------------------------------------------------------------------------------------------
// �����ݿ��Ƴ�
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::RemoveDB( tagItem* p2UnEquip )
{
	NET_DB2C_delete_item send;
	send.n64_serial = p2UnEquip->n64_serial;
	g_dbSession.Send(&send, send.dw_size);
}

//----------------------------------------------------------------------------------------------------
// ��ȡ����װ��ԭ��
//----------------------------------------------------------------------------------------------------
const tagPetEquipProto* PetEquipBar::GetPetEquipProto( tagItem* pItem )
{
	if (!VALID_POINT(pItem)) return NULL;
	M_trans_else_ret(pItemProto, pItem->pProtoType, const tagItemProto, NULL);
	if (EISF_PetEquipment != pItemProto->eSpecFunc) return NULL;
	return AttRes::GetInstance()->GetPetEquipProto(pItem->pProtoType->nSpecFuncVal1);
}

//----------------------------------------------------------------------------------------------------
// ���װ��
//----------------------------------------------------------------------------------------------------
VOID PetEquipBar::AddEquip( tagItem* p2Equip, INT16 n16DstPos, BOOL bOnLoad)
{
	// ���ԭ��
	M_trans_else_ret(p2EquipPetEquipProto, GetPetEquipProto(p2Equip), const tagPetEquipProto, );

	//��ӵ�װ����
	PetEquipContainer::Add(p2Equip, n16DstPos);
	p2Equip->SetOwner(m_pSoul->GetID(), INVALID_VALUE);

	// ����ʱ����Ʒ
	if (p2Equip->is_time_limit())
	{
		m_TimeLimitMgr.Add2TimeLeftList(p2Equip->n64_serial, p2Equip->pProtoType->dwTimeLimit, p2Equip->dw1stGainTime, 0);
	}	

	// ������ƷЧ��
	CalcEffect(m_pSoul, p2EquipPetEquipProto->dw_data_id, TRUE, !bOnLoad);
}

BOOL PetEquipBar::HasEquip()
{
	return  PetEquipContainer::GetMaxSpaceSize() - PetEquipContainer::GetFreeSpaceSize() != 0;
}

VOID PetEquipBar::DisablePetSkill( std::list<PetSkill*> &listSkill)
{
	if (!m_pSoul->IsCalled())
	{
		return;
	}
	PetSkill* pSkill = NULL;
	std::list<PetSkill*>::iterator itr = listSkill.begin();
	while (itr != listSkill.end())
	{
		pSkill = *itr;
		if (pSkill->GetProto()->eCastType == EPCT_Passive)
		{
			PassiveSkill *pPassiveSkill = (PassiveSkill *)pSkill;
			pPassiveSkill->DeActive(m_pSoul->GetMaster());
		}
		//if (pSkill->GetProto()->eCastType == EPCT_Enhance)
		//{
		//	EnhanceSkill *pEnhanceSkill = (EnhanceSkill *)pSkill;
		//	pEnhanceSkill->Close();	
		//}
		//PassiveSkill* pPassive = dynamic_cast<PassiveSkill*>(*itr);
		//if (VALID_POINT(pPassive))
		//{
		//	pPassive->DeActive();
		//}
		//EnhanceSkill* pEnhance = dynamic_cast<EnhanceSkill*>(*itr);
		//if (VALID_POINT(pEnhance))
		//{
		//	pEnhance->Close();
		//}
		++itr;
	}
}

VOID PetEquipBar::EnablePetSkill( std::list<PetSkill*> &listSkill)
{
	if (!m_pSoul->IsCalled())
	{
		return;
	}
	PetSkill* pSkill = NULL;
	std::list<PetSkill*>::iterator itr = listSkill.begin();
	while (itr != listSkill.end())
	{
		pSkill = *itr;
		if (pSkill->GetProto()->eCastType == EPCT_Passive)
		{
			PassiveSkill *pPassiveSkill = (PassiveSkill *)pSkill;
			pPassiveSkill->Active(m_pSoul->GetMaster());
		}
		//if (pSkill->GetProto()->eCastType == EPCT_Enhance)
		//{
		//	EnhanceSkill *pEnhanceSkill = (EnhanceSkill *)pSkill;
		//	pEnhanceSkill->Open();	
		//}

		//PassiveSkill* pPassive = dynamic_cast<PassiveSkill*>(*itr);
		//if (VALID_POINT(pPassive))
		//{
		//	pPassive->Active();
		//}
		//EnhanceSkill* pEnhance = dynamic_cast<EnhanceSkill*>(*itr);
		//if (VALID_POINT(pEnhance))
		//{
		//	pEnhance->Open();
		//}
		++itr;
	}
}

BOOL PetEquipBar::IsEquipExist( INT64 n64Id )
{
	//����װ����ÿ��λ��װ��
	for (INT16 i=0; i<PetEquipContainer::GetCurSpaceSize(); ++i)
	{
		tagItem* pCurItem = PetEquipContainer::GetItem(i);
		if (VALID_POINT(pCurItem)) 
			return TRUE;
	}

	return FALSE;
}