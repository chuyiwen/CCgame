/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��Ʒ��װ��������

#include "StdAfx.h"

#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/RoleDefine.h"
#include "../common/ServerDefine/base_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "pet_server_define.h"

#include "att_res.h"
#include "item_creator.h"
#include "world.h"
#include "script_mgr.h"
#include "../../common/WorldDefine/ride_define.h"

INT64			ItemCreator::m_n64MaxSerial = 0;
INT64			ItemCreator::m_n64MinSerial = 0;
Mutex			ItemCreator::m_Mutex;

//-------------------------------------------------------------------------------------------------------
// ���캯��
//-------------------------------------------------------------------------------------------------------
ItemCreator::ItemCreator()
{}

ItemCreator::~ItemCreator()
{}

//-------------------------------------------------------------------------------------------------------
// ����������Ʒ�����µĶ���Ʒ
//-------------------------------------------------------------------------------------------------------
tagItem* ItemCreator::Create(const tagItem &item, INT16 n16Num)
{
	if(n16Num > item.pProtoType->n16MaxLapNum)
	{
		ASSERT(n16Num <= item.pProtoType->n16MaxLapNum);
		return NULL;
	}
	
	tagItem *pNewItem;
	if(MIsEquipment(item.dw_data_id))
	{
		pNewItem = new tagEquip;
		get_fast_code()->memory_copy(pNewItem, &item, SIZE_EQUIP);
	}
	else
	{
		pNewItem = new tagItem;
		get_fast_code()->memory_copy(pNewItem, &item, SIZE_ITEM);
	}

	CreateItemSerial(pNewItem->n64_serial);
	pNewItem->n16Num = n16Num;

	return pNewItem;
}

//-------------------------------------------------------------------------------------------------------
// ������Ʒ&װ��
//-------------------------------------------------------------------------------------------------------
tagItem* ItemCreator::Create(EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, 
							 INT16 n16Num/* = 1*/, BOOL bBind/* = FALSE*/, DWORD dwCreator/* = INVALID_VALUE*/,
							 INT16 n16QltyModPct/* = 0*/, INT16 n16QltyModPctEx/* = 0*/, INT16 n16PotValPct/* = 10000*/)
{
	if(n16Num <= 0)
	{
		//ASSERT(n16Num > 0);
		return NULL;
	}
	
	tagItem *pRetItem;
	if(MIsEquipment(dw_data_id))
	{
		pRetItem = new tagEquip;
		if(NULL == pRetItem)
		{
			ASSERT(pRetItem != NULL);
			return NULL;
		}

		pRetItem->pEquipProto = AttRes::GetInstance()->GetEquipProto(dw_data_id);
		ASSERT(VALID_POINT(pRetItem->pEquipProto));
		InitEquipSpec(((tagEquip*)pRetItem)->equipSpec, pRetItem->pEquipProto, EICM_Loot==eCreateMode);
	}
	else
	{		
		pRetItem = new tagItem;
		if(NULL == pRetItem)
		{
			ASSERT(pRetItem != NULL);
			return NULL;
		}

		pRetItem->pProtoType = AttRes::GetInstance()->GetItemProto(dw_data_id);
	}

	if(!VALID_POINT(pRetItem->pProtoType)/* || pRetItem->pProtoType->n16MaxLapNum < n16Num*/)
	{
		m_att_res_caution(_T("item or equip proto"), _T("typeid"), dw_data_id);
		//ASSERT(VALID_POINT(pRetItem->pProtoType));
		//ASSERT(VALID_POINT(pRetItem->pProtoType) && pRetItem->pProtoType->n16MaxLapNum >= n16Num);
		::Destroy(pRetItem);
		return NULL;
	}
	
	if (pRetItem->pProtoType->n16MaxLapNum < n16Num)
	{
		n16Num = pRetItem->pProtoType->n16MaxLapNum;
	}

	INT64 n64_serial;
	CreateItemSerial(n64_serial);

	InitItem(*pRetItem, eCreateMode, pRetItem->pProtoType, dwCreateID, n64_serial, n16Num, dwCreator, g_world.GetWorldTime());
	
	//if (eCreateMode == EICM_Quest || 
	//	eCreateMode == EICM_AccQuest || 
	//	eCreateMode == EICM_NewRoleGift || 
	//	eCreateMode == EICM_Mail)
	//{
	//	pRetItem->byBind = EBS_SYSTEM_Bind;
	//}
	if (bBind)
	{
		pRetItem->byBind = EBS_Bind;
		pRetItem->bCreateBind = bBind;
	}
	else
	{
		pRetItem->byBind = pRetItem->pEquipProto->byBindType;
	}
	// װ��
	if(MIsEquipment(pRetItem->dw_data_id))
	{
		// ����Ƿ�Ϊ����������ɺ�ʱ����
		/*if((pRetItem->pEquipProto->bIdentifyLoot && EICM_Loot == eCreateMode)
			|| (pRetItem->pEquipProto->bIdentifyProduct && EICM_Product == eCreateMode))*/
		//if ( EICM_Loot == eCreateMode || EICM_Product == eCreateMode)
		//{
		//	ItemCreator::IdentifyEquip((tagEquip*)pRetItem);
		//}
		if(MEquipIsRide( pRetItem->pProtoType ))  
		{
			const tagPetProto* pPetProto = AttRes::GetInstance()->GetPetProto( pRetItem->pEquipProto->dwPetID );
			//((tagEquip*)pRetItem)->equipSpec.dwSpeed = VALID_POINT( pPetProto ) ? pPetProto->nMountSpeed : 0;
			((tagEquip*)pRetItem)->equipSpec.byHoleNum = MAX_RIDEHOLE_NUM;
		}
	}

	return pRetItem;
}

//-------------------------------------------------------------------------------------------------------
// �������ݿ��ȡ�����ݴ�����Ʒ
//-------------------------------------------------------------------------------------------------------
tagItem* ItemCreator::CreateItemByData( PVOID pData )
{
	M_trans_pointer(pItem, pData, tagItem);
	tagItem* pNewItem = new tagItem;

	get_fast_code()->memory_copy(pNewItem, pItem, sizeof(tagItem));
	pNewItem->pProtoType = AttRes::GetInstance()->GetItemProto(pItem->dw_data_id);

	if (IsGMItemNoInit(pNewItem))
	{
		InitItem(*pNewItem, pItem->eCreateMode, pNewItem->pProtoType, pNewItem->dwCreateID, pNewItem->n64_serial, pNewItem->n16Num, pNewItem->dwCreatorID, pNewItem->dwCreateTime);
	}

	return pNewItem;
}

//-------------------------------------------------------------------------------------------------------
// �������ݿ��ȡ�����ݴ���װ��
//-------------------------------------------------------------------------------------------------------
tagEquip* ItemCreator::CreateEquipByData( PVOID pData )
{
	M_trans_pointer(pEquip, pData, tagEquip);
	tagEquip* pNewEquip = new tagEquip;

	get_fast_code()->memory_copy(pNewEquip, pEquip, sizeof(tagEquip));
	pNewEquip->pEquipProto = AttRes::GetInstance()->GetEquipProto(pEquip->dw_data_id);

	if (IsGMItemNoInit(pEquip))
	{
		InitItem(*pNewEquip, pNewEquip->eCreateMode, pNewEquip->pProtoType, pNewEquip->dwCreateID, pNewEquip->n64_serial, pNewEquip->n16Num, pNewEquip->dwCreatorID, pNewEquip->dwCreateTime);

		EItemQuality eQuality = (EItemQuality)pNewEquip->equipSpec.byQuality;
		InitEquipSpec(pNewEquip->equipSpec, pNewEquip->pEquipProto);
		IdentifyEquip(pNewEquip, eQuality);
	}

	return pNewEquip;
}

//-------------------------------------------------------------------------------------------------------
// ��������Ψһ��(ע��Ҫ����������)
//-------------------------------------------------------------------------------------------------------
VOID ItemCreator::CreateItemSerial(INT64 &n64NewSerial)
{
	m_Mutex.Acquire();
	n64NewSerial = ++m_n64MaxSerial;
	m_Mutex.Release();
}

//-------------------------------------------------------------------------------------------------------
// ������Ʒ
//-------------------------------------------------------------------------------------------------------
VOID ItemCreator::InitItem( tagItem &item, EItemCreateMode eCreateMode, const tagItemProto *pProto, DWORD dwCreateID, INT64 n64_serial, INT16 n16Num, DWORD dwCreator, DWORD dwCreateTime )
{
	ZeroMemory(&item, SIZE_ITEM);

	item.pProtoType		= pProto;
	
	item.n64_serial		= n64_serial;
	item.n16Num			= n16Num;
	item.dw_data_id		= pProto->dw_data_id;

	item.byBind			= EBS_Unknown;
	item.bLock			= FALSE;

	item.eCreateMode	= eCreateMode;
	item.dwCreateID		= dwCreateID;	// ���ɸ���Ʒ��ID,��: QuestID,GMID��
	
	item.dwCreatorID	= dwCreator;	/* ��ΪRoleID, ��ĳλ���������������Ʒ,��λ���ܱ�ʾΪ����Ҵ���;
											������Ʒͨ������ϵͳ����ʱ,���λͬdwCreateID*/
	item.dwNameID		= INVALID_VALUE;
	item.dwCreateTime	= dwCreateTime;

	item.dwOwnerID		= INVALID_VALUE;
	item.dw_account_id	= INVALID_VALUE;
	item.eConType		= EICT_Null;
	item.n16Index		= INVALID_VALUE;

	item.eStatus		= EUDBS_Insert;
	
	item.pScript		= g_ScriptMgr.GetItemScript(pProto->dw_data_id);
}

//-------------------------------------------------------------------------------------------------------
// ����δ����װ��(װ��ר�ò�������)
//-------------------------------------------------------------------------------------------------------
VOID ItemCreator::InitEquipSpec(tagEquipSpec &equipSpec, const tagEquipProto* pProto, BOOL bHoldNum/* = TRUE*/)
{
	ZeroMemory(&equipSpec, SIZE_EQUIPSPEC);

	//equipSpec.nLevel		= 1;
	equipSpec.byQuality		= EIQ_Null;
	//equipSpec.bCanCut		= TRUE;

	//for(INT32 i=0; i<MAX_ROLEATT_POSY_EFFECT; ++i)
	//{
	//	equipSpec.PosyEffect[i].eRoleAtt = ERA_Null;
	//}
	
	for (INT32 i = 0; i < MAX_ADDITIONAL_EFFECT; ++i)
	{
		equipSpec.EquipAttitionalAtt[i].eRoleAtt = ERA_Null;
	}


	if (!VALID_POINT(pProto))
		return;

	for (int i = 0; i < MAX_BASE_ATT; i++)
	{
		// �������
		int nRand = get_tool()->tool_rand() % 100;
		if (pProto->RandEffect[i].eRoleAtt != ERA_Null && nRand <= pProto->dwRandEffectPro[i])
		{	
			equipSpec.EquipAttitionalAtt[i].eRoleAtt = pProto->RandEffect[i].eRoleAtt;
			equipSpec.EquipAttitionalAtt[i].nValue = pProto->RandEffect[i].nValue;
		}
	}

	equipSpec.byLuck = pProto->byLuck;

	// ʱװ���û�ÿ�
	if (pProto->eEquipPos == EEP_Fashion || 
		pProto->eEquipPos == EEP_Body1 ||
		pProto->eEquipPos == EEP_Ride)
		return ;

	// ��׺�ǿ��ͬ��,�����װ������
	if (bHoldNum)
	{
		int nPro = get_tool()->tool_rand() % 10000;
		// �жϴ����
		if (nPro <= 5000)
		{
			equipSpec.byHoleNum = 1;
		}
		else if ( nPro < 6000)
		{
			equipSpec.byHoleNum = 2;
		}
		else if ( nPro < 6100)
		{
			equipSpec.byHoleNum = 3;
		}
		else if ( nPro < 6110)
		{
			equipSpec.byHoleNum = 4;
		}
		else
		{
			equipSpec.byHoleNum = 0;
		}

		int nStar = 0;
		// �ж�ǿ��
		if (nPro < 100)
		{
			nStar = get_tool()->tool_rand() % 2;
			equipSpec.byConsolidateLevel = nStar + 6;
		}
		else if (nPro < 1400)
		{
			nStar = get_tool()->tool_rand() % 2;
			equipSpec.byConsolidateLevel = nStar + 4;
		}
		else if (nPro < 3500)
		{
			nStar = get_tool()->tool_rand() % 3;
			equipSpec.byConsolidateLevel = nStar + 1;
		}
	}



	//equipSpec.bySpecAtt			= 0;
	//equipSpec.n8ColorID			= EC_Null;

	//equipSpec.n16QltyModPct		= n16QltyModPct;
	//equipSpec.n16QltyModPctEx	= n16QltyModPctEx;

	//equipSpec.n16PotValModPct	= n16PotValPct;
}

//-----------------------------------------------------------------------------
// ����װ��(û��Ʒ����Ʒ���Ƿ��������¼���Ʒ��)
//-----------------------------------------------------------------------------
EItemQuality ItemCreator::IdentifyEquip(IN OUT tagEquip *pEquip, 
								EItemQuality eQlty/* = EIQ_Null*/,
								BOOL bRandAtt/* = TRUE*/)
{
	ASSERT(VALID_POINT(pEquip));
	ASSERT(VALID_POINT(pEquip->pEquipProto));
	//ASSERT(!M_is_identified(pEquip));

	//// ����Ƿ����Ѽ�����װ��
	//if(M_is_identified(pEquip))
	//{
	//	return EIQ_Quality0;
	//}

	//����װ��������
	//if(VALID_POINT(pEquip->pEquipProto) && MEquipIsRide(pEquip->pEquipProto))
	//{
	//	pEquip->equipSpec.byQuality = (BYTE)EIQ_Quality0;
	//	return EIQ_Quality0;
	//}
	//

	pEquip->equipSpec.nRating = pEquip->GetRating();


	// ʱװ��򲻼���
	if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
		pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
		pEquip->pEquipProto->eEquipPos == EEP_Ride)
	{
		pEquip->equipSpec.byQuality = eQlty;
		return eQlty;
	}
		
	INT32 nQuality = eQlty;
	
	// û��Ʒ����Ʒ���Ƿ��������¼���Ʒ��
	if(eQlty <= EIQ_Start || eQlty >= EIQ_End)
	{
		// ����װ��Ʒ�����ɼ�������Ʒ��
		nQuality = GenBaseEquipQlty(pEquip->dw_data_id);

		// ��������Ʒ��������������Ʒ��
		//nQuality = ModEquipQltyByProduce(pEquip, nQuality);
	}

	// ��ʼ��Ʒ��
	pEquip->equipSpec.byQuality = (BYTE)nQuality;
	
	if (nQuality != EIQ_Null)
	{
		CreateEquipQltyRel(pEquip, pEquip->pEquipProto, (EItemQuality)nQuality, bRandAtt);
	}
	// ��ʼ���ȼ�����
	//pEquip->equipSpec.byMinUseLevel = pEquip->pEquipProto->byMinUseLevel;	// �ȼ�����
	//pEquip->equipSpec.byMaxUseLevel = pEquip->pEquipProto->byMaxUseLevel;	// �ȼ�����

	// ����Ʒ�������������
	//if(!M_is_fashion(pEquip))
	//{
		
	//}
	//else
	//{
	//	CreateFashionQltyRel(pEquip, pEquip->pEquipProto, (EItemQuality)nQuality, pIMEffect);
	//}
	
	return (EItemQuality)nQuality;
}

//-----------------------------------------------------------------------------
// ����װ��Ʒ�����ɼ�������Ʒ��
//-----------------------------------------------------------------------------
INT32 ItemCreator::GenBaseEquipQlty(DWORD dw_data_id)
{
	tagEquipQltyPct *pEquipQltyPct = AttRes::GetInstance()->GetEquipQltyPct(dw_data_id);
	if(!VALID_POINT(pEquipQltyPct))
	{
		ASSERT(VALID_POINT(pEquipQltyPct));
		return EIQ_Null;
	}

	INT32 nEquipQltySumPct = 0;
	INT32 nRandPct = get_tool()->tool_rand() % 10000;
	for(INT32 i=EIQ_End - 1; i>EIQ_Start; --i)
	{
		nEquipQltySumPct += pEquipQltyPct->nEquipQltyPct[i];
		if(nRandPct < nEquipQltySumPct)
		{
			return i;
		}
	}

	return EIQ_Null;
}

//-----------------------------------------------------------------------------
// ����װ��Ʒ�����ɼ�������Ʒ��
//-----------------------------------------------------------------------------
INT32 ItemCreator::GenEquipQlty(DWORD dw_data_id, BYTE byQuality, INT nAddPro, INT nSkillLevel)
{
	tagEquipQltyPct *pEquipQltyPct = AttRes::GetInstance()->GetEquipQltyPct(dw_data_id);
	if(!VALID_POINT(pEquipQltyPct))
	{
		ASSERT(VALID_POINT(pEquipQltyPct));
		return EIQ_Quality0;
	}
	

	
	tagEquipQltyPct eqpct;
	memcpy(&eqpct, pEquipQltyPct, sizeof(tagEquipQltyPct));

	for (int i = EIQ_Quality0; i < EIQ_End; i++)
	{
		eqpct.nEquipQltyPct[i] += ComposeHelper::GetProduceSkillLevelParam(i) * (nSkillLevel - 1);
	}
	eqpct.nEquipQltyPct[EIQ_Quality1] -=nAddPro;
	eqpct.nEquipQltyPct[byQuality] +=nAddPro;

	INT32 nEquipQltySumPct = 0;
	INT32 nRandPct = get_tool()->tool_rand() % 10000;
	for(INT32 i=EIQ_End - 1; i>EIQ_Start; --i)
	{
		nEquipQltySumPct += eqpct.nEquipQltyPct[i];
		if(nRandPct < nEquipQltySumPct)
		{
			return i;
		}
	}
}
//-----------------------------------------------------------------------------
// ��������Ʒ��������������Ʒ��
//-----------------------------------------------------------------------------
//INT32 ItemCreator::ModEquipQltyByProduce(const tagEquip *pEquip, INT32 nQuality)
//{
//	if(pEquip->equipSpec.n16QltyModPctEx != 0
//		&& (get_tool()->tool_rand() % 10000) < pEquip->equipSpec.n16QltyModPctEx)	// ��������
//	{
//		return min(nQuality + 2, EIQ_End - 1);
//	}
//	else	// һ������
//	{
//		if(pEquip->equipSpec.n16QltyModPct > 0)
//		{
//			if((get_tool()->tool_rand() % 10000) < pEquip->equipSpec.n16QltyModPct)
//			{
//				return min(nQuality + 1, EIQ_End - 1);
//			}
//		}
//		else if(pEquip->equipSpec.n16QltyModPct < 0)
//		{
//			if((get_tool()->tool_rand() % 10000) < -pEquip->equipSpec.n16QltyModPct)
//			{
//				return max(nQuality - 1, EIQ_Quality0);
//			}
//		}
//	}
//
//	return nQuality;
//}

//-----------------------------------------------------------------------------
// ����ָ��Ʒ������װ���������
//-----------------------------------------------------------------------------
VOID ItemCreator::CreateEquipQltyRel(OUT tagEquip *pEquip, const tagEquipProto *pEquipProto, EItemQuality eQlty, BOOL bRandAtt)
{
	ASSERT(eQlty > EIQ_Start && eQlty < EIQ_End);
	
	// �õ�ָ��Ʒ��װ�����Բ���
	const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(eQlty);
	if(!VALID_POINT(pEquipQltyEffect))
	{
		ASSERT(VALID_POINT(pEquipQltyEffect));
		return;
	}
	
	//pEquip->equipSpec.byTriggerCount = pEquipProto->bySuitTriggerCount;

	// ����ϵ��
	//DOUBLE fValueModFactor;
	
	//if (pEquipProto->eEquipPos != EEP_Shipin1 && pEquipProto->eEquipPos != EEP_Shipin1)
	//{
	//	// װ���������� -- "������ԭʼ�༭ֵ; ���ߣ�ԭʼ�༭ֵ"
	//	fValueModFactor = pEquipQltyEffect->fWeaponFactor;

	//	//pEquip->equipSpec.n16MinDmg	= (INT16)(pEquipProto->n16MinDmg * fValueModFactor);	// ������С�˺�
	//	//pEquip->equipSpec.n16MaxDmg	= (INT16)(pEquipProto->n16MaxDmg * fValueModFactor);	// ��������˺�

	//	fValueModFactor = pEquipQltyEffect->fArmorFactor;
	//	//pEquip->equipSpec.n16Armor	= (INT16)(pEquipProto->n16Armor * fValueModFactor);		// ���߻���
	//}
	//else
	//{
	//	if (bRandAtt)
	//	{
	//		if (pEquipProto->n16MinDmg != 0)
	//		{
	//			pEquip->equipSpec.n16MinDmg = get_tool()->fluctuate(pEquipProto->n16MinDmg, 50, 50);
	//		}
	//		if (pEquipProto->n16MaxDmg != 0)
	//		{
	//			pEquip->equipSpec.n16MaxDmg = get_tool()->fluctuate(pEquipProto->n16MaxDmg, 50, 50);
	//		}
	//		
	//		if (pEquipProto->n16Armor != 0)
	//		{
	//			pEquip->equipSpec.n16Armor	= get_tool()->fluctuate(pEquipProto->n16Armor, 50, 50);		// ���߻���
	//		}
	//	}
	//	else
	//	{
	//		//pEquip->equipSpec.n16MinDmg	= (INT16)(pEquipProto->n16MinDmg );	// ������С�˺�
	//		//pEquip->equipSpec.n16MaxDmg	= (INT16)(pEquipProto->n16MaxDmg );	// ��������˺�

	//		//pEquip->equipSpec.n16Armor	= (INT16)(pEquipProto->n16Armor);	// ���߻���
	//	}
	//	
	//}
	

	//const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(pEquip->equipSpec.nLevel);
	//ASSERT(VALID_POINT(pEffect));
	//pEquip->equipSpec.byMaxTalentPoint = pEffect->n16TalentAvail;
							
	//ZeroMemory(pEquip->equipSpec.nRoleAttEffect, sizeof(pEquip->equipSpec.nRoleAttEffect));
	
	//fValueModFactor = pEquipQltyEffect->fAttAFactor;

	
	// װ����������
	const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquipProto->byLevel);
	if(!VALID_POINT(pEquipLevelPctProto))
	{
		ASSERT(VALID_POINT(pEquipLevelPctProto));
		return;
	}

	
	
	FLOAT fPct = pEquipLevelPctProto->fLevelPct * pEquipLevelPctProto->fPosPct[pEquipProto->eEquipPos];
	//! ��ȥ������
	//DWORD dwAttNum = min(pEquipQltyEffect->nAttANumEffect, MAX_RAND_ATT);
	
	// �Ƿ�ʹ�ù̶��������
	//if (!bRandAtt)
	//{
		//for (int i = 0; i < MAX_BASE_ATT; i++)
		//{
		//	// �������
		//	int nRand = get_tool() % 100;
		//	if (pEquip->pEquipProto->RandEffect[i].eRoleAtt != ERA_Null && nRand <= 10)
		//	{	
		//		pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = pEquip->pEquipProto->RandEffect[i].eRoleAtt;
		//		pEquip->equipSpec.EquipAttitionalAtt[i].nValue = pEquip->pEquipProto->RandEffect[i].nValue;
		//	}
		//}
		//return;
	//}

	//memset(&pEquip->equipSpec.EquipAttitionalAtt[MAX_BASE_ATT], 0, sizeof(tagRoleAttEffect) * 5);
		
	for (INT32 i = MAX_BASE_ATT; i < MAX_ADDITIONAL_EFFECT; i++)
	{
		pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = ERA_Null;
		pEquip->equipSpec.EquipAttitionalAtt[i].nValue = 0;
	}	

	INT16 n16Rand = 0;	
	std::vector<int> vecAtt;//����Щ������ȡ
	for (int i = 0; i < ADDATT_TYPE_NUM; i++)
	{
		vecAtt.push_back(i);
	}
	int nAttIndex = 0;
	int nAttNumber = min(pEquipQltyEffect->nAttANumEffect, MAX_RAND_ATT);
	for (INT32 i = MAX_BASE_ATT; i < MAX_BASE_ATT + nAttNumber; i++)
	{
		nAttIndex = get_tool()->tool_rand() % (vecAtt.size());
		n16Rand = vecAtt[nAttIndex];
		//! ÿ�������������
		//vecRand[n16Rand]++;
		//if (vecRand[n16Rand] >= 2)
		//{
			vecAtt.erase(vecAtt.begin()+nAttIndex);
		//}
		pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = EAA2ERA((EquipAddAtt)n16Rand);
		//EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
		pEquip->equipSpec.EquipAttitionalAtt[i].nValue = max(1, (INT32)(get_tool()->fluctuate((pEquipQltyEffect->EquipAddAtt[n16Rand]*fPct), pEquipQltyEffect->nAttAFactor, pEquipQltyEffect->nAttAFactor)));
	}


}

//-----------------------------------------------------------------------------
// �����������Զ�װ�����Ե�Ӱ��
//-----------------------------------------------------------------------------
//VOID ItemCreator::ProcEquipAttBySpecAtt(tagEquip *pEquip)
//{
//	tagEquipSpec &equipSpec = pEquip->equipSpec;
//
//	switch(equipSpec.bySpecAtt)
//	{
//	case EESA_LevelLim_Simple:
//		// ����:��װ���ȼ�����-5����Ϳɼ�����0
//		equipSpec.byMinUseLevel = (equipSpec.byMinUseLevel > 5 ? equipSpec.byMinUseLevel - 5 : 0);
//		break;
//	case EESA_LevelLim_Fine:
//		// ����		��װ���ȼ�����-10����Ϳɼ�����0
//		equipSpec.byMinUseLevel = (equipSpec.byMinUseLevel > 10 ? equipSpec.byMinUseLevel - 10 : 0);
//		break;
//	case EESA_LevelLim_None:
//		// �޼���	��װ���޵ȼ�����
//		equipSpec.byMinUseLevel = 0;
//		break;
//
//	case EESA_AttALim_Simple:
//		// ���		��װ���������Ƽ���10%��ȡ��
//		equipSpec.n16AttALimModPct = -1000;
//		break;
//	case EESA_AttALim_Comfort:
//		// ����		��װ���������Ƽ���25%��ȡ��
//		equipSpec.n16AttALimModPct = -2500;
//		break;
//	case EESA_AttALim_Light:
//		// ��ӯ		��װ���������Ƽ���50%��ȡ��
//		equipSpec.n16AttALimModPct = -5000;
//		break;
//
//	case EESA_Potential_YinFeng:
//		// ����		��װ���ĳ�ʼǱ��ֵ+200
//		equipSpec.nPotVal = min(equipSpec.nPotVal + 200, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_YinHuang:
//		// ����		��װ���ĳ�ʼǱ��ֵ+400
//		equipSpec.nPotVal = min(equipSpec.nPotVal + 400, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_FeiFeng:
//		// �ɷ�		��װ���ĳ�ʼǱ��ֵ+800
//		equipSpec.nPotVal = min(equipSpec.nPotVal + 800, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_MingHuang:
//		// ����		��װ���ĳ�ʼǱ��ֵ+1200
//		equipSpec.nPotVal = min(equipSpec.nPotVal + 1200, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_WoLong:
//		// ����		װ���ĳ�ʼǱ��ֵ���5%
//		equipSpec.nPotVal = (INT32)min(equipSpec.nPotVal * 1.05, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_CangLong:
//		// ����		װ���ĳ�ʼǱ��ֵ���10%
//		equipSpec.nPotVal = (INT32)min(equipSpec.nPotVal * 1.1, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_FuLong:
//		// ����		װ���ĳ�ʼǱ��ֵ���20%
//		equipSpec.nPotVal = (INT32)min(equipSpec.nPotVal * 1.2, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	case EESA_Potential_ShengLong:
//		// ����		װ���ĳ�ʼǱ��ֵ���30%
//		equipSpec.nPotVal = (INT32)min(equipSpec.nPotVal * 1.3, pEquip->pEquipProto->nMaxPotVal);
//		break;
//	}
//}

//-----------------------------------------------------------------------------
// ����ָ��Ʒ������ʱװ�������
//-----------------------------------------------------------------------------
BOOL ItemCreator::CreateFashionQltyRel(OUT tagEquip *pEquip, const tagEquipProto *pEquipProto, 
									EItemQuality eQlty, const s_ime_effect *pIMEffect/* = NULL*/)
{
	ASSERT(eQlty > EIQ_Start && eQlty < EIQ_End);

	// ��ȡʱװ���������Դ
	const tagFashionGen *pGen = AttRes::GetInstance()->GetFashionQltyEffect(eQlty);
	if(!VALID_POINT(pGen))
	{
		m_att_res_caution(_T("fashion_qlty_effect"), _T("Quality"), (INT)eQlty);
		return FALSE;
	}

	const tagFashionColorPct *pColor = AttRes::GetInstance()->GetFashionColorPct(eQlty);
	if(!VALID_POINT(pColor))
	{
		m_att_res_caution(_T("fashion_color_pct"), _T("Quality"), (INT)eQlty);
		return FALSE;
	}

	// ��װû�����Լӳ�
	if(EIQ_Quality0 == eQlty)
	{
		return TRUE;
	}

	// ʱװ�����ɿ���
	//pEquip->equipSpec.bCanCut = FALSE;

	return TRUE;
}

tagItem* ItemCreator::CreateTreasure(DWORD dwNameID, EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, INT16 n16Num /*= 1*/, DWORD dwCreator /*= INVALID_VALUE*/, INT16 n16QltyModPct /*= 0*/, INT16 n16QltyModPctEx /*= 0*/ )
{
	ASSERT(VALID_VALUE(dwNameID));
	tagItem* pNew = Create(eCreateMode, dwCreateID, dw_data_id, n16Num, FALSE, dwCreator, n16QltyModPct, n16QltyModPctEx);
	if (VALID_POINT(pNew))
	{
		pNew->dwNameID = dwNameID;
	}	
	return pNew;
}

tagItem* ItemCreator::CreateTreasureEx(DWORD dwNameID, EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, 
								 INT16 n16Num/* = 1*/, EItemQuality eQlty/* = EIQ_Null*/, DWORD dwCreator/* = INVALID_VALUE*/, 
								 const s_ime_effect *pIMEffect/* = NULL*/)
{
	ASSERT(VALID_VALUE(dwNameID));
	tagItem* pNew = CreateEx(eCreateMode, dwCreateID, dw_data_id, n16Num,eQlty,  FALSE, dwCreator, pIMEffect);
	if (VALID_POINT(pNew))
	{
		pNew->dwNameID = dwNameID;
	}
	return pNew;
}

BOOL ItemCreator::IsGMItemNoInit(tagItem* pTmpItem)
{
	ASSERT(VALID_POINT(pTmpItem));

	if (EICT_Baibao == pTmpItem->eConType 
		&& !VALID_VALUE(pTmpItem->dwCreateID)
		&& !VALID_VALUE(pTmpItem->dwCreatorID))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL ItemCreator::InitGMItem(tagItem* pTmpItem)
{
	// ��ʼ����Ʒ����

	// ��ʼ��װ������
	if (MIsEquipment(pTmpItem->dw_data_id))
	{
		M_trans_pointer(pEquip, pTmpItem, tagEquip);
		ItemCreator::IdentifyEquip(pEquip, EItemQuality(pEquip->equipSpec.byQuality));
	}

	return TRUE;
}

//���ɰ�����
VOID ItemCreator::CreateEquipBindAtt(tagEquip* pEquip, FLOAT fParam)
{
	ASSERT(VALID_POINT(pEquip));
	if (!VALID_POINT(pEquip))
		return;
	
	// �õ�ָ��Ʒ��װ�����Բ���
	const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
	if(!VALID_POINT(pEquipQltyEffect))
	{
		ASSERT(VALID_POINT(pEquipQltyEffect));
		return;
	}

	const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
	if(!VALID_POINT(pEquipLevelPctProto))
	{
		ASSERT(VALID_POINT(pEquipLevelPctProto));
		return;
	}
	INT16 n16Rand = 0;
	FLOAT fPct = pEquipLevelPctProto->fLevelPct /** pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos]*/;

	if(pEquip->pEquipProto->eEquipPos == EEP_RightHand)
	{
		n16Rand = get_tool()->tool_rand() % (WRA_NUM);
		pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = WRA2ERA((WeaponRoleAtt)n16Rand);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt);
		pEquip->equipSpec.EquipAttitionalAtt[5].nValue = (INT32)(pEquipQltyEffect->EquipAddAtt[eProtoRoleAtt]*fPct*fParam + 0.5f);
	}	

	if(pEquip->pEquipProto->eEquipPos >= EEP_Neck && pEquip->pEquipProto->eEquipPos <= EEP_Shipin1)
	{
		n16Rand = get_tool()->tool_rand() % DRA_NUM;
		pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = DRA2ERA((DecorationRoleAtt)n16Rand);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt);
		pEquip->equipSpec.EquipAttitionalAtt[5].nValue = (INT32)(pEquipQltyEffect->EquipAddAtt[eProtoRoleAtt]*fPct*fParam+ 0.5f);
	}

	if(pEquip->pEquipProto->eEquipPos >= EEP_Head && pEquip->pEquipProto->eEquipPos < EEP_Dress_End)
	{
		n16Rand = get_tool()->tool_rand() % (ARA_Armor_NUM);
		pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = ARA2ERA((ArmorRoleAtt)n16Rand);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt);
		pEquip->equipSpec.EquipAttitionalAtt[5].nValue = (INT32)(pEquipQltyEffect->EquipAddAtt[eProtoRoleAtt]*fPct*fParam+ 0.5f);
	}
	
	if ( pEquip->equipSpec.EquipAttitionalAtt[5].nValue <= 0 )
	{
		pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = ERA_Null;
	}
}

VOID ItemCreator::SetWeaponAtt(std::vector<int>& vec)
{
	vec.push_back(WRA_ExAttack);
	vec.push_back(WRA_Stamina);
	vec.push_back(WRA_Cirt);
	vec.push_back(WRA_Potence);
	vec.push_back(WRA_Potence);
	vec.push_back(WRA_Agility);
	vec.push_back(WRA_Agility);
	vec.push_back(WRA_Brains);
	vec.push_back(WRA_Brains);
	vec.push_back(WRA_Ren);
	vec.push_back(WRA_Ren);
	vec.push_back(WRA_Ren);
	vec.push_back(WRA_CirtNum);
	vec.push_back(WRA_CirtNum);
	vec.push_back(WRA_CirtNum);
	vec.push_back(WRA_CirtNum);
	vec.push_back(WRA_HitRate);
	vec.push_back(WRA_HitRate);
	vec.push_back(WRA_HitRate);
	vec.push_back(WRA_HitRate);
	vec.push_back(WRA_HitRate);
}

VOID ItemCreator::SetDecorationAtt(std::vector<int>& vec)
{
	vec.push_back(DRA_ExAttack);
	vec.push_back(DRA_Stamina);
	vec.push_back(DRA_Cirt);
	vec.push_back(DRA_Potence);
	vec.push_back(DRA_Potence);
	vec.push_back(DRA_Potence);
	vec.push_back(DRA_Agility);
	vec.push_back(DRA_Agility);
	vec.push_back(DRA_Agility);
	vec.push_back(DRA_Brains);
	vec.push_back(DRA_Brains);
	vec.push_back(DRA_Brains);
	vec.push_back(DRA_CirtNum);
	vec.push_back(DRA_CirtNum);
	vec.push_back(DRA_CirtNum);
	vec.push_back(DRA_CirtNum);
	vec.push_back(DRA_CirtNum);
	vec.push_back(DRA_HitRate);
	vec.push_back(DRA_HitRate);
	vec.push_back(DRA_HitRate);
	vec.push_back(DRA_HitRate);
	vec.push_back(DRA_HitRate);
}

VOID ItemCreator::SetArmorAtt(std::vector<int>& vec)
{
	vec.push_back(ARA_Potence);
	vec.push_back(ARA_Agility);
	vec.push_back(ARA_Brains);
	vec.push_back(ARA_Stamina);
	vec.push_back(ARA_Dodge);
	vec.push_back(ARA_Dodge);
	vec.push_back(ARA_Dodge);
	vec.push_back(ARA_Dodge);
	vec.push_back(ARA_ExDefense);
	vec.push_back(ARA_ExDefense);
	vec.push_back(ARA_ExDefense);
	vec.push_back(ARA_ExDefense);
	vec.push_back(ARA_FanCirt);
	vec.push_back(ARA_FanCirt);
	vec.push_back(ARA_FanCirt);
	vec.push_back(ARA_FanCirt);
	vec.push_back(ARA_FanCirtNum);
	vec.push_back(ARA_FanCirtNum);
	vec.push_back(ARA_FanCirtNum);
	vec.push_back(ARA_FanCirtNum);
	vec.push_back(ARA_FanCirtNum);
	vec.push_back(ARA_Ren);
	vec.push_back(ARA_Ren);
	vec.push_back(ARA_Ren);
	vec.push_back(ARA_Ren);
}
VOID ItemCreator::RemoveAtt(std::vector<int>& vec, int nType)
{
	std::vector<int>::iterator it = vec.begin();
	while (it != vec.end())
	{
		if (*it == nType)
		{
			vec.erase(it);
		}
		else
		{
			it++;
		}
	}
}
// ����һ������
VOID ItemCreator::ReattEquip(tagEquip* pEquip, BYTE byindex)
{
	ASSERT(VALID_POINT(pEquip));
	if (!VALID_POINT(pEquip))
		return;

	// �õ�ָ��Ʒ��װ�����Բ���
	const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
	if(!VALID_POINT(pEquipQltyEffect))
	{
		ASSERT(VALID_POINT(pEquipQltyEffect));
		return;
	}

	const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
	if(!VALID_POINT(pEquipLevelPctProto))
	{
		ASSERT(VALID_POINT(pEquipLevelPctProto));
		return;
	}
	INT16 n16Rand = 0;
	FLOAT fPct = pEquipLevelPctProto->fLevelPct * pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos];
	

	ERoleAttribute eCurAtt = pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt;
	INT nCurAttValue = pEquip->equipSpec.EquipAttitionalAtt[byindex].nValue;
	INT nCurAddAttValue = pEquip->equipSpec.EquipAttitional[byindex];

	INT nParam1 = GetAttParam(eCurAtt);
	INT nParam2 = 1;

	if(pEquip->pEquipProto->eEquipPos == EEP_RightHand)
	{
		std::vector<int> vecRand(WRA_NUM, 0);//ÿ�����Գ��ִ���
		std::vector<int> vecAtt;//����Щ������ȡ
		SetWeaponAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			if ( i == byindex)
				continue;
			
			INT nIndex = ERA2WRA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			if (nIndex >= 0 && nIndex< vecRand.size())
			{
				vecRand[nIndex]++;
				if (vecRand[nIndex] >= 2)
				{
					RemoveAtt(vecAtt, nIndex);	
				}
			}
		}
		n16Rand = get_tool()->tool_rand() % (vecAtt.size());
		pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt = WRA2ERA((WeaponRoleAtt)vecAtt[n16Rand]);
		nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);

		pEquip->equipSpec.EquipAttitionalAtt[byindex].nValue = nParam2*1.0/nParam1 * nCurAttValue;
		pEquip->equipSpec.EquipAttitional[byindex] = nParam2*1.0/nParam1 * nCurAddAttValue;
	}	

	if(pEquip->pEquipProto->eEquipPos >= EEP_Neck && pEquip->pEquipProto->eEquipPos <= EEP_Shipin1)
	{
		std::vector<int> vecRand(DRA_NUM, 0);//ÿ�����Գ��ִ���
		std::vector<int> vecAtt;//����Щ������ȡ
		SetDecorationAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			if ( i == byindex)
				continue;

			INT nIndex = ERA2DRA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			if (nIndex >= 0 && nIndex< vecRand.size())
			{
				vecRand[nIndex]++;
				if (vecRand[nIndex] >= 2)
				{
					RemoveAtt(vecAtt, nIndex);
				}
			}
		}
		n16Rand = get_tool()->tool_rand() % (vecAtt.size());
		pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt = DRA2ERA((DecorationRoleAtt)vecAtt[n16Rand]);
		nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);

		pEquip->equipSpec.EquipAttitionalAtt[byindex].nValue = nParam2*1.0/nParam1 * nCurAttValue;
		pEquip->equipSpec.EquipAttitional[byindex] = nParam2*1.0/nParam1 * nCurAddAttValue;
	}

	if(pEquip->pEquipProto->eEquipPos >= EEP_Head && pEquip->pEquipProto->eEquipPos < EEP_Dress_End)
	{
		std::vector<int> vecRand(ARA_Armor_NUM, 0);//ÿ�����Գ��ִ���
		std::vector<int> vecAtt;//����Щ������ȡ
		SetArmorAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			if ( i == byindex)
				continue;

			INT nIndex = ERA2ARA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			if (nIndex >= 0 && nIndex< vecRand.size())
			{
				vecRand[nIndex]++;
				if (vecRand[nIndex] >= 2)
				{
					RemoveAtt(vecAtt, nIndex);
				}
			}

		}
		n16Rand = get_tool()->tool_rand() % (vecAtt.size());
		pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt = ARA2ERA((ArmorRoleAtt)vecAtt[n16Rand]);
		nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);
		EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt);

		pEquip->equipSpec.EquipAttitionalAtt[byindex].nValue = nParam2*1.0/nParam1 * nCurAttValue;
		pEquip->equipSpec.EquipAttitional[byindex] = nParam2*1.0/nParam1 * nCurAddAttValue;
	}

	if ( pEquip->equipSpec.EquipAttitionalAtt[byindex].nValue <= 0 )
	{
		pEquip->equipSpec.EquipAttitionalAtt[byindex].eRoleAtt = ERA_Null;
	}
}
VOID ItemCreator::ReattEquip(tagEquip* pEquip)
{

	ASSERT(VALID_POINT(pEquip));
	if (!VALID_POINT(pEquip))
		return;

	// �õ�ָ��Ʒ��װ�����Բ���
	const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
	if(!VALID_POINT(pEquipQltyEffect))
	{
		ASSERT(VALID_POINT(pEquipQltyEffect));
		return;
	}

	const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
	if(!VALID_POINT(pEquipLevelPctProto))
	{
		ASSERT(VALID_POINT(pEquipLevelPctProto));
		return;
	}
	INT16 n16Rand = 0;
	FLOAT fPct = pEquipLevelPctProto->fLevelPct * pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos];

	//! ����
	if(pEquip->pEquipProto->eEquipPos == EEP_RightHand)
	{
		std::vector<int> vecRand(WRA_NUM, 0);//ÿ�����Գ��ִ���
		std::vector<int> vecAtt;//����Щ������ȡ
		SetWeaponAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			nAttIndex = get_tool()->tool_rand() % (vecAtt.size());
			n16Rand = vecAtt[nAttIndex];
			//! ÿ�������������
			vecRand[n16Rand]++;
			if (vecRand[n16Rand] >= 2)
			{
				RemoveAtt(vecAtt, n16Rand);
			}

			ERoleAttribute eCurAtt = pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt;
			INT nCurAttValue = pEquip->equipSpec.EquipAttitionalAtt[i].nValue;
			INT nCurAddAttValue = pEquip->equipSpec.EquipAttitional[i];

			INT nParam1 = GetAttParam(eCurAtt);
		
			pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = WRA2ERA((WeaponRoleAtt)n16Rand);

			INT nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			pEquip->equipSpec.EquipAttitionalAtt[i].nValue = nParam2*1.0/nParam1 * nCurAttValue;
			pEquip->equipSpec.EquipAttitional[i] = nParam2*1.0/nParam1 * nCurAddAttValue;
		}
	}
	//! ��Ʒ
	if(pEquip->pEquipProto->eEquipPos >= EEP_Neck && pEquip->pEquipProto->eEquipPos <= EEP_Shipin1)
	{
		std::vector<int> vecRand(DRA_NUM, 0);
		std::vector<int> vecAtt;//����Щ������ȡ
		SetDecorationAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			nAttIndex = get_tool()->tool_rand() % (vecAtt.size());
			n16Rand = vecAtt[nAttIndex];
			//! ÿ�������������
			vecRand[n16Rand]++;
			if (vecRand[n16Rand] >= 2)
			{
				RemoveAtt(vecAtt, n16Rand);
			}

			ERoleAttribute eCurAtt = pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt;
			INT nCurAttValue = pEquip->equipSpec.EquipAttitionalAtt[i].nValue;
			INT nCurAddAttValue = pEquip->equipSpec.EquipAttitional[i];

			INT nParam1 = GetAttParam(eCurAtt);

			pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = DRA2ERA((DecorationRoleAtt)n16Rand);

			INT nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);

			EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			pEquip->equipSpec.EquipAttitionalAtt[i].nValue = nParam2*1.0/nParam1 * nCurAttValue;
			pEquip->equipSpec.EquipAttitional[i] = nParam2*1.0/nParam1 * nCurAddAttValue;
		}
	}
	//! ����
	if(pEquip->pEquipProto->eEquipPos >= EEP_Head && pEquip->pEquipProto->eEquipPos < EEP_Dress_End)
	{
		std::vector<int> vecRand(ARA_Armor_NUM, 0);
		std::vector<int> vecAtt;//����Щ������ȡ
		SetArmorAtt(vecAtt);
		int nAttIndex = 0;
		for (INT32 i = 0; i < 3; i++)
		{
			nAttIndex = get_tool()->tool_rand() % (vecAtt.size());
			n16Rand = vecAtt[nAttIndex];
			//! ÿ�������������
			vecRand[n16Rand]++;
			if (vecRand[n16Rand] >= 2)
			{
				RemoveAtt(vecAtt, n16Rand);
			}

			ERoleAttribute eCurAtt = pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt;
			INT nCurAttValue = pEquip->equipSpec.EquipAttitionalAtt[i].nValue;
			INT nCurAddAttValue = pEquip->equipSpec.EquipAttitional[i];

			INT nParam1 = GetAttParam(eCurAtt);

			pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt = ARA2ERA((ArmorRoleAtt)n16Rand);
			INT nParam2 = GetAttParam(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);

			EquipAddAtt eProtoRoleAtt = ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
			pEquip->equipSpec.EquipAttitionalAtt[i].nValue = nParam2*1.0/nParam1 * nCurAttValue;
			pEquip->equipSpec.EquipAttitional[i] = nParam2*1.0/nParam1 * nCurAddAttValue;
		}
	}

}
// ɾ��������
VOID ItemCreator::RemoveEquipBindAtt(tagEquip* pEquip)
{
	pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = ERA_Null;
	pEquip->equipSpec.EquipAttitionalAtt[5].nValue = 0;
}
ERoleAttribute ItemCreator::WRA2ERA(WeaponRoleAtt eWeaponRoleAtt)
{
	switch(eWeaponRoleAtt)
	{
	case WRA_ExAttack:
		return ERA_ExAttack;
	case WRA_HitRate:
		return ERA_HitRate;
	case WRA_Ren:
		return ERA_ShenAttack;
	case WRA_Cirt:
		return ERA_Crit_Rate;
	case WRA_CirtNum:
		return ERA_Crit_Amount;
	case WRA_Potence:
		return ERA_Strength;
	case WRA_Agility:
		return ERA_Agility;
	case WRA_Brains:
		return ERA_InnerForce;
	case WRA_Stamina:
		return ERA_Physique;
	}
	return ERA_Null;
}

WeaponRoleAtt ItemCreator::ERA2WRA(ERoleAttribute eWeaponRoleAtt)
{
	switch(eWeaponRoleAtt)
	{
	case ERA_ExAttack:
		return WRA_ExAttack;
	case ERA_HitRate:
		return WRA_HitRate;
	case ERA_ShenAttack:
		return WRA_Ren;
	case ERA_Crit_Rate:
		return WRA_Cirt;
	case ERA_Crit_Amount:
		return WRA_CirtNum;
	case ERA_Strength:
		return WRA_Potence;
	case ERA_Agility:
		return WRA_Agility;
	case ERA_InnerForce:
		return WRA_Brains;
	case ERA_Physique:
		return WRA_Stamina;
	}
	return WRA_Null;
}

ERoleAttribute ItemCreator::ARA2ERA(ArmorRoleAtt eArmorRoleAtt)
{
	switch(eArmorRoleAtt)
	{
	case ARA_Dodge:
		return ERA_Dodge;
	case ARA_ExDefense:
		return ERA_ExDefense;
	case ARA_Potence:
		return ERA_Strength;
	case ARA_Agility:
		return ERA_Agility;
	case ARA_Brains:
		return ERA_InnerForce;
	case ARA_Stamina:
		return ERA_Physique;
	case ARA_FanCirt:
		return ERA_UnCrit_Rate;
	case ARA_FanCirtNum:
		return ERA_UnCrit_Amount;
	case ARA_Ren:
		return ERA_ShenAttack;
	}
	return ERA_Null;
}


ArmorRoleAtt ItemCreator::ERA2ARA(ERoleAttribute eArmorRoleAtt)
{
	switch(eArmorRoleAtt)
	{
	case ERA_Dodge:
		return ARA_Dodge;
	case ERA_ExDefense:
		return ARA_ExDefense;
	case ERA_Strength:
		return ARA_Potence;
	case ERA_Agility:
		return ARA_Agility;
	case ERA_InnerForce:
		return ARA_Brains;
	case ERA_Physique:
		return ARA_Stamina;
	case ERA_UnCrit_Rate:
		return ARA_FanCirt;
	case ERA_UnCrit_Amount:
		return ARA_FanCirtNum;
	case ERA_ShenAttack:
		return ARA_Ren;
	}
	return ARA_Null;
}


ERoleAttribute ItemCreator::DRA2ERA(DecorationRoleAtt	eDecorationRoleAtt)
{
	switch(eDecorationRoleAtt)
	{
	case DRA_ExAttack:
		return ERA_ExAttack;
	case DRA_HitRate:
		return ERA_HitRate;
	case DRA_Cirt:
		return ERA_Crit_Rate;
	case DRA_CirtNum:
		return ERA_Crit_Amount;
	case DRA_Potence:
		return ERA_Strength;
	case DRA_Agility:
		return ERA_Agility;
	case DRA_Brains:
		return ERA_InnerForce;
	case DRA_Stamina:
		return ERA_Physique;
	}
	return ERA_Null;
}


DecorationRoleAtt ItemCreator::ERA2DRA(ERoleAttribute	eDecorationRoleAtt)
{
	switch(eDecorationRoleAtt)
	{
	case ERA_ExAttack:
		return DRA_ExAttack;
	case ERA_HitRate:
		return DRA_HitRate;
	case ERA_Crit_Rate:
		return DRA_Cirt;
	case ERA_Crit_Amount:
		return DRA_CirtNum;
	case ERA_Strength:
		return DRA_Potence;
	case ERA_Agility:
		return DRA_Agility;
	case ERA_InnerForce:
		return DRA_Brains;
	case ERA_Physique:
		return DRA_Stamina;
	}
	return DRA_Null;
}

ERoleAttribute ItemCreator::SPA2ERA(ShiPinRoleAtt	eShipinRoleAtt)
{
	switch(eShipinRoleAtt)
	{
	case SP_Dodge:
		return ERA_Dodge;
	case SP_HitRate:
		return ERA_HitRate;
	case SP_Cirt:
		return ERA_Crit_Rate;
	case SP_CirtNum:
		return ERA_Crit_Amount;
	case SP_Potence:
		return ERA_Strength;
	case SP_Agility:
		return ERA_Agility;
	case SP_Brains:
		return ERA_InnerForce;
	case SP_Ren:
		return ERA_ShenAttack;
	case SP_FanCirt:
		return ERA_UnCrit_Rate;
	case SP_FanCirtNum:
		return ERA_UnCrit_Amount;
	}
	return ERA_Null;
}

ShiPinRoleAtt ItemCreator::ERA2SPA(ERoleAttribute	eRoleAtt)
{
	switch(eRoleAtt)
	{
	case ERA_Dodge:
		return SP_Dodge;
	case ERA_HitRate:
		return SP_HitRate;
	case ERA_Crit_Rate:
		return SP_Cirt;
	case ERA_Crit_Amount:
		return SP_CirtNum;
	case ERA_Strength:
		return SP_Potence;
	case ERA_Agility:
		return SP_Agility;
	case ERA_InnerForce:
		return SP_Brains;
	case ERA_ShenAttack:
		return SP_Ren;
	case ERA_UnCrit_Rate:
		return SP_FanCirt;
	case ERA_UnCrit_Amount:
		return SP_FanCirtNum;
	}
	return SP_Null;
}

EquipAddAtt ItemCreator::ERA2EAA(ERoleAttribute eRoleAttribut)
{
	switch(eRoleAttribut)
	{
	case ERA_ExAttackMin:
		return EAA_ExAttackMin;
	case ERA_ExAttackMax:
		return EAA_ExAttackMax;
	case ERA_InAttackMin:
		return EAA_InAttackMin;
	case ERA_InAttackMax:
		return EAA_InAttackMax;
	case ERA_ArmorEx:
		return EAA_ArmorEx;
	case ERA_ArmorIn:
		return EAA_ArmorIn;
	case ERA_ExAttack:
		return EAA_ExAttack;
	case ERA_ExDefense:
		return EAA_ExDef;
	case ERA_InAttack:
		return EAA_InAttack;
	case ERA_InDefense:
		return EAA_InDefense;
	case ERA_HitRate:
		return EAA_HitRate;
	case ERA_Dodge:
		return EAA_Dodge;
	case ERA_Crit_Rate:
		return EAA_Crit_Rate;
	case ERA_UnCrit_Rate:
		return EAA_UnCrit_Rate;
	case ERA_MaxHP:
		return EAA_MaxHP;
	case ERA_MaxMP:
		return EAA_MaxMP;
	}
	return EAA_NULL;
}


ERoleAttribute ItemCreator::EAA2ERA(EquipAddAtt eRoleAttribut)
{
	switch(eRoleAttribut)
	{
	case EAA_ExAttackMin:
		return ERA_ExAttackMin;
	case EAA_ExAttackMax:
		return ERA_ExAttackMax;
	case EAA_InAttackMin:
		return ERA_InAttackMin;
	case EAA_InAttackMax:
		return ERA_InAttackMax;
	case EAA_ArmorEx:
		return ERA_ArmorEx;
	case EAA_ArmorIn:
		return ERA_ArmorIn;
	case EAA_ExAttack:
		return ERA_ExAttack;
	case EAA_ExDef:
		return ERA_ExDefense;
	case EAA_InAttack:
		return ERA_InAttack;
	case EAA_InDefense:
		return ERA_InDefense;
	case EAA_HitRate:
		return ERA_HitRate;
	case EAA_Dodge:
		return ERA_Dodge;
	case EAA_Crit_Rate:
		return ERA_Crit_Rate;
	case EAA_UnCrit_Rate:
		return ERA_UnCrit_Rate;
	case EAA_MaxHP:
		return ERA_MaxHP;
	case EAA_MaxMP:
		return ERA_MaxMP;
	}
	return ERA_Null;
}

INT ItemCreator::GetAttParam(ERoleAttribute eAtt)
{
	if (eAtt == ERA_ExAttack)
	{
		return 10;
	}
	
	if (eAtt == ERA_ExDefense)
	{
		return 4;
	}

	if (eAtt > ERA_Agility)
	{
		return 2;
	}

	return 1;
}