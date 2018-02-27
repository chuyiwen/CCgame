/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��װ

#include "StdAfx.h"

#include "../common/ServerDefine/base_server_define.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/suit_define.h"
#include "../../common/WorldDefine/item_protocol.h"

#include "suit.h"
#include "role.h"
//--------------------------------------------------------------------------------------------
// ����&����
//--------------------------------------------------------------------------------------------
Suit::Suit(Role *pRole)
{
	m_pRole = pRole;
}

Suit::~Suit()
{
	m_pRole = NULL;
}

//--------------------------------------------------------------------------------------------
// ��װ��
//--------------------------------------------------------------------------------------------
VOID Suit::Add(const tagEquip *pEquip, INT16 n16EquipPos, BOOL bSend2Client/* = TRUE*/)
{
	ASSERT(pEquip->n16Index == n16EquipPos);
	const tagEquipProto *pEquipProto = pEquip->pEquipProto;

	for(INT i=0; i<MAX_PEREQUIP_SUIT_NUM; ++i)
	{
		if(INVALID_VALUE == pEquipProto->dwSuitID[i])
		{
			break;
		}

		// �ж��Ƿ�Ϊ��װ����
		if(!IsSuitPart(pEquip, pEquipProto, i, n16EquipPos))
		{
			continue;
		}

		// ��ӵ�map��
		m_mapSuitNum.modify_value(pEquipProto->dwSuitID[i], 1);

		// ����Ч��
		INT nCnt = m_mapSuitNum.find(pEquipProto->dwSuitID[i]);
		if(bSend2Client)
		{
			// ���µ���װ�������͵��ͻ���
			NET_SIS_suit_num sendSuitNum;
			sendSuitNum.dwSuitID	= pEquipProto->dwSuitID[i];
			sendSuitNum.n8Num		= nCnt;
			m_pRole->SendMessage(&sendSuitNum, sendSuitNum.dw_size);
		}

		if(nCnt < MIN_SUIT_EQUIP_NUM)
		{
			continue;
		}

		const tagSuitProto *pSuitProto = AttRes::GetInstance()->GetSuitProto(pEquipProto->dwSuitID[i]);
		if(!VALID_POINT(pSuitProto))
		{
			ASSERT(VALID_POINT(pSuitProto));
			continue;
		}

		for(INT n=0; n<MAX_SUIT_ATT_NUM; ++n)
		{
			if(nCnt < pSuitProto->n8ActiveNum[n])
			{
				break;
			}
			else if(pSuitProto->n8ActiveNum[n] == nCnt)
			{
				// �����������
				m_pRole->RegisterTriggerSuitSet(pSuitProto->dwTriggerID[n], 
								pSuitProto->dwBuffID[n], pEquipProto->dwSuitID[i]);
			}
		}

		// ��Ч
		if(pSuitProto->n8SpecEffectNum == nCnt)
		{
			m_pRole->SetSuitEffect(pEquipProto->dwSuitID[i]);

			if(bSend2Client && VALID_POINT(m_pRole->get_map()))
			{
				// ����ЧID���͸��ͻ���
				NET_SIS_suit_effect sendSuitEffect;
				sendSuitEffect.dw_role_id			= m_pRole->GetID();
				sendSuitEffect.dwSuitEffectID	= pEquipProto->dwSuitID[i];

				m_pRole->get_map()->send_big_visible_tile_message(m_pRole, &sendSuitEffect, sendSuitEffect.dw_size);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
// ��װ��
//--------------------------------------------------------------------------------------------
VOID Suit::Remove(const tagEquip *pEquip, INT16 n16OldIndex)
{
	const tagEquipProto *pEquipProto = pEquip->pEquipProto;

	for(INT i=0; i<MAX_PEREQUIP_SUIT_NUM; ++i)
	{
		if(INVALID_VALUE == pEquipProto->dwSuitID[i])
		{
			break;
		}

		// �ж��Ƿ�Ϊ��װ����
		if(!IsSuitPart(pEquip, pEquipProto, i, n16OldIndex))
		{
			continue;
		}

		// ��ӵ�map��
		m_mapSuitNum.modify_value(pEquipProto->dwSuitID[i], -1);

		// ����Ч��
		INT nCnt = m_mapSuitNum.find(pEquipProto->dwSuitID[i]);

		// ���µ���װ�������͵��ͻ���
		NET_SIS_suit_num sendSuitNum;
		sendSuitNum.dwSuitID	= pEquipProto->dwSuitID[i];
		sendSuitNum.n8Num		= nCnt;
		m_pRole->SendMessage(&sendSuitNum, sendSuitNum.dw_size);

		INT nOrgCnt = nCnt + 1;
		if(1 == nOrgCnt)
		{
			m_mapSuitNum.erase(pEquipProto->dwSuitID[i]);
			continue;
		}

		const tagSuitProto *pSuitProto = AttRes::GetInstance()->GetSuitProto(pEquipProto->dwSuitID[i]);
		if(!VALID_POINT(pSuitProto))
		{
			ASSERT(VALID_POINT(pSuitProto));
			continue;
		}

		// ��ȥ�����������������
		for(INT n=0; n<MAX_SUIT_ATT_NUM; ++n)
		{
			if(nOrgCnt < pSuitProto->n8ActiveNum[n])
			{
				break;
			}

			m_pRole->UnRegisterTriggerSuitSet(pSuitProto->dwTriggerID[n], 
							pSuitProto->dwBuffID[n], pEquipProto->dwSuitID[i]);
		}

		// ������������������
		for(INT n=0; n<MAX_SUIT_ATT_NUM; ++n)
		{
			if(nCnt < pSuitProto->n8ActiveNum[n])
			{
				break;
			}

			m_pRole->RegisterTriggerSuitSet(pSuitProto->dwTriggerID[n], 
							pSuitProto->dwBuffID[n], pEquipProto->dwSuitID[i]);
		}


		// ��Ч
		if(pSuitProto->n8SpecEffectNum == nOrgCnt)
		{
			m_pRole->SetSuitEffect(INVALID_VALUE);

			if(VALID_POINT(m_pRole->get_map()))
			{
				// ����ЧID���͸��ͻ���
				NET_SIS_suit_effect sendSuitEffect;
				sendSuitEffect.dw_role_id			= m_pRole->GetID();
				sendSuitEffect.dwSuitEffectID	= INVALID_VALUE;

				m_pRole->get_map()->send_big_visible_tile_message(m_pRole, &sendSuitEffect, sendSuitEffect.dw_size);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------
// ��ʼ�����͵��ͻ��˵���Ϣ����
//--------------------------------------------------------------------------------------------
VOID Suit::InitSendInitState(BYTE *pData)
{
	M_trans_pointer(p, pData, tagSuitInit);

	INT nEquipNum, i=0;
	DWORD dwSuitID;
	package_map<DWORD, INT>::map_iter iter = m_mapSuitNum.begin();
	while(m_mapSuitNum.find_next(iter, dwSuitID, nEquipNum))
	{
		p[i].dwSuitID	= dwSuitID;
		p[i].nEquipNum	= nEquipNum;

		++i;
	}
}

//--------------------------------------------------------------------------------------------
// �жϸü���װ�����Ƿ���Ҫͳ��
//--------------------------------------------------------------------------------------------
BOOL Suit::IsNeedCount(const tagEquip *pEquip, const tagEquipProto *pEquipProto, 
				 INT nSuitIndex, INT16 n16IndexOther)
{
	const tagEquip *pEquipOther = m_pRole->GetItemMgr().GetEquipBarEquip(n16IndexOther);
	if(VALID_POINT(pEquipOther) 
		&& pEquipOther->dw_data_id == pEquip->dw_data_id
		&& pEquipProto->bySuitMinQlty[nSuitIndex] <= pEquipOther->equipSpec.byQuality)
	{
		return FALSE;
	}

	return TRUE;
}

// ��ȡ��װ��������ʣ�����
INT Suit::GetSuitTriggerCount(const tagSuitProto* pSuitProto)
{
	//tagEquip* pEquip = m_pRole->GetItemMgr().GetEquipBarEquip((INT16)pSuitProto->nEffectConutPos);
	//if (!VALID_POINT(pEquip))
	//{
		return 0;
	//}
	//return pEquip->equipSpec.byTriggerCount;
}

// ���Ķ�Ӧ�Ĵ���������
VOID Suit::ConsumeTriggerCount(const tagSuitProto* pSuitProto)
{
	//tagEquip* pEquip = m_pRole->GetItemMgr().GetEquipBarEquip((INT16)pSuitProto->nEffectConutPos);
	//if (!VALID_POINT(pEquip))
	//{
	//	return;
	//}

	//pEquip->equipSpec.byTriggerCount--;
	//m_pRole->GetItemMgr().UpdateEquipSpec(*pEquip);
}
