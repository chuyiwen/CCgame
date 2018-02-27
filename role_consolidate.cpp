/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//װ��ǿ��

#include "StdAfx.h"

#include "../../common/WorldDefine/compose_protocol.h"
#include "../../common/WorldDefine/compose_define.h"
#include "../../common/WorldDefine/formula_define.h"
#include "../common/ServerDefine/consolidate_server_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../common/ServerDefine/item_server_define.h"
#include "../../common/WorldDefine/vip_define.h"

#include "role.h"
#include "map.h"
#include "creature.h"
#include "item_creator.h"
#include "hearSay_helper.h"
#include "pet_pocket.h"
#include "pvp_mgr.h"
//-----------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------
//DWORD Role::PosyEquip(DWORD dwNPCID, DWORD dwFormulaID, INT64 n64ItemID, INT64 n64IMID, INT64 n64StuffID[], INT32 nArraySz, DWORD dw_cmd_id)
//{
//	// ��õ�ͼ
//	Map *pMap = get_map();
//	if(!VALID_POINT(pMap))
//	{
//		ASSERT(VALID_POINT(pMap));
//		return INVALID_VALUE;
//	}
//
//	Creature* pNPC = pMap->find_creature(dwNPCID);
//	if( !VALID_POINT(pNPC) ) 
//		return E_Compose_NPC_Not_Exist;
//
//	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
//		return E_Compose_NPC_Distance;
//
//	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Posy) )
//		return E_Consolidate_NPCCanNotPosy;
//
//	// �ҵ�ǿ������
//	const tagPosyProtoSer *pPosyProto = AttRes::GetInstance()->GetPosyProto(dwFormulaID);
//	if(!VALID_POINT(pPosyProto))
//		return E_Compose_Formula_Not_Exist;
//
//	// �ҵ���ǿ��װ��
//	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64ItemID);
//	if(!VALID_POINT(pEquip))
//		return E_Consolidate_Equip_Not_Exist;
//
//	if(!MIsEquipment(pEquip->dw_data_id))
//		return E_Consolidate_NotEquipment;
//
//	// ������ƴ���
//	if(pEquip->equipSpec.byPosyTimes >= MAX_ENGRAVE_TIMES)
//		return E_Consolidate_BeyondPosyTime;
//
//	// ���ǿ���䷽�Ƿ�Ϸ�
//	if((pEquip->equipSpec.byPosyTimes + 1 ) != pPosyProto->byPosyTimes)
//		return E_Consolidate_FormulaNotMatch;
//
//	// ����Ƿ���ǿ���䷽��Ӧ��װ������
//	if(!AttRes::GetInstance()->IsPosyPos(pPosyProto->ePosyAtt, pEquip->pEquipProto->eEquipPos))
//		return E_Consolidate_EquipCanNotPosy;
//
//	// ���װ��Ǳ��ֵ�Ƿ��㹻
//	if(pEquip->equipSpec.nPotVal < (INT)pPosyProto->nPotValConsume)
//		return E_Consolidate_ValConsume_Inadequate;
//
//	// �����ҽ�Ǯ�Ƿ��㹻
//	if(GetCurMgr().GetBagSilver() < pPosyProto->dw_money_consume)
//		return E_Consolidate_NotEnough_Money;
//
//	// �����Ҳ����Ƿ��㹻(˳������ܲ���������
//	INT nItemQualityNum[EIQ_End];
//	ZeroMemory(nItemQualityNum, sizeof(nItemQualityNum));
//
//	for(INT n = 0; n < MAX_CONSOLIDATE_STUFF_QUANTITY; ++n)
//	{
//		if(pPosyProto->ConsolidateStuff[n].dwStuffID == INVALID_VALUE 
//			&& pPosyProto->ConsolidateStuff[n].eStuffType == EST_Null)
//			break;
//
//		tagItem *pItemStuff = GetItemMgr().GetBagItem(n64StuffID[n]); 
//
//		if(!VALID_POINT(pItemStuff))
//			return E_Consolidate_NotEnough_Stuff;
//
//		// ������64λID�Ƿ��ظ�
//		for(INT i = 0; i < n; ++i)
//		{
//			if(n64StuffID[i] == n64StuffID[n])
//				return E_Consolidate_NotEnough_Stuff;
//		}
//
//		if(pItemStuff->dw_data_id != pPosyProto->ConsolidateStuff[n].dwStuffID 
//			&& pItemStuff->pProtoType->eStuffType != pPosyProto->ConsolidateStuff[n].eStuffType)
//			return E_Consolidate_NotEnough_Stuff;
//
//		if(pItemStuff->n16Num < (INT)pPosyProto->ConsolidateStuff[n].dwStuffNum)
//			return E_Consolidate_NotEnough_Stuff;
//
//		nItemQualityNum[pItemStuff->pProtoType->byQuality] += pPosyProto->ConsolidateStuff[n].dwStuffNum;
//	}
//
//	// ����IM���ߵ�Ӱ��
//	tagIMEffect			IMEffect;
//	tagPosyProtoSer *pProto = const_cast<tagPosyProtoSer*>(pPosyProto);		
//	CalIMEffect(ECTS_Posy, IMEffect, n64IMID, pProto,1);
//
//	// �����������
//	for(INT n = 0; n < nArraySz; ++n)
//	{
//		 GetItemMgr().DelFromBag(n64StuffID[n], (DWORD)ELCID_Compose_Posy, (INT16)pPosyProto->ConsolidateStuff[n].dwStuffNum);
//	}
//
//	// ��Ǯ����
//	GetCurMgr().DecBagSilver(pPosyProto->dw_money_consume, ELCID_Compose_Posy);
//
//	// ����ɹ���
//	// ���ճɹ���=�����ɹ���+��-10%����Ʒ��������/�ܲ�������+0%����Ʒ��������/�ܲ�������+5%����Ʒ��������
//	// /�ܲ�������+10%����Ʒ��������/�ܲ�������+20%����Ʒ��������/�ܲ�����������[1+��װ���ȼ�-75��/150]
//	// +����ɫ��Ե/1000��+IM���߼ӳ�                                      ���ճɹ��ʵ�ȡֵΪ0%-100%
//	FLOAT fProp = 0;
//	fProp = (FLOAT)pPosyProto->nSuccessRate + (((-0.1f * (FLOAT)nItemQualityNum[EIQ_Quality0] 
//		    + 0.05f * (FLOAT)nItemQualityNum[EIQ_Quality4] + 0.1f * (FLOAT)nItemQualityNum[EIQ_Quality1] 
//			+ 0.2f * (FLOAT)nItemQualityNum[EIQ_Quality3]) / (FLOAT)pPosyProto->nTotleStuff) 
//			* (1.0f + ((FLOAT)pEquip->pProtoType->byLevel - 75.0f) / 150.0f) + ((FLOAT) GetAttValue(ERA_Vigour) / 1000.0f)) * 10000;
//
//	// ����B�����ԶԳɹ���Ӱ��
//	fProp = CalSpecAttEffectSuc((EEquipSpecAtt)(pEquip->equipSpec.bySpecAtt), fProp, EESA_Guard_Posy);
//
//	// �������Ƿ�������߳ɹ��ʵ�IM
//	if( IMEffect.e_effect ==  eime_comadvance)
//	{
//		fProp += (FLOAT)IMEffect.dw_param2;
//	}
//
//	BOOL bResult = get_tool()->tool_rand() % 10000 <= (INT)fProp;
//
//	if(bResult)			// �ɹ�
//	{
//		// װ�����ƴ�����һ
//		pEquip->equipSpec.byPosyTimes++;
//
//		// ����װ����Ӧǿ��ֵ
//		// ��ֵ=��װ���ȼ���fcoefficientA + fcoefficientB��/ fcoefficientC ����1+����Ʒ�ʼӳɣ�
//		/* ����Ʒ�ʼӳ�=��-20%����Ʒ��������/�ܲ�������+0%����Ʒ��������/�ܲ�������+20%
//		   ����Ʒ��������/�ܲ�������+50%����Ʒ��������/�ܲ�������+100%����Ʒ��������/�ܲ���������
//		   ����Ʒ�ʼӳɵ�ȡֵΪ0%-100%  */
//
//		// �����ӳ�
//		FLOAT fBaseAttInc = ((FLOAT)pEquip->pProtoType->byLevel * pPosyProto->fcoefficientA 
//							+ pPosyProto->fcoefficientB) / pPosyProto->fcoefficientC;
//
//		// ����Ʒ�ʼӳ�
//		FLOAT fStuffAttInc = (-0.2f * (FLOAT)nItemQualityNum[EIQ_Quality0] + 0.2f * (FLOAT)nItemQualityNum[EIQ_Quality4] 
//							  + 0.5f * (FLOAT)nItemQualityNum[EIQ_Quality1] + 1.0f * (FLOAT)nItemQualityNum[EIQ_Quality3] )
//							  / (FLOAT)pPosyProto->nTotleStuff; 
//
//		if(fStuffAttInc < 0.0f)
//			fStuffAttInc = 0.0f;
//		if(fStuffAttInc > 1.0f)
//			fStuffAttInc = 1.0f;
//
//		// ����ת�� 
//		ERoleAttribute eRoleAtt = ERA_Null;
//		ConsolidatePosyAttConvert(eRoleAtt, pPosyProto->ePosyAtt);
//
//		for(INT n = 0; n < MAX_ROLEATT_POSY_EFFECT; ++n)
//		{
//			if(pEquip->equipSpec.PosyEffect[n].eRoleAtt == eRoleAtt)
//			{
//				pEquip->equipSpec.PosyEffect[n].nValue += (INT32)(fBaseAttInc * (1.0f + fStuffAttInc));
//				break;
//			}
//			else if(pEquip->equipSpec.PosyEffect[n].eRoleAtt == ERA_Null)
//			{
//				pEquip->equipSpec.PosyEffect[n].eRoleAtt = eRoleAtt;
//				pEquip->equipSpec.PosyEffect[n].nValue += (INT32)(fBaseAttInc * (1.0f + fStuffAttInc));
//				break;
//			}
//		}
//
//		// Ǳ��ֵ����
//		// ����������=�����ɹ��ʡ�1/20+��ɫ��Ե/1000
//		FLOAT fPefectProp = (fProp * 0.05f) / 10000.0f + (FLOAT)GetAttValue(ERA_Vigour) / 1000.0f;
//
//		// ����B�����Զ�������Ӱ��
//		fPefectProp = CalSpecAttEffectPef((EEquipSpecAtt)(pEquip->equipSpec.bySpecAtt), fPefectProp);
//
//		// ����װ������
//		CalEquipFlare(pEquip);
//
//		BOOL bPefect = get_tool()->tool_rand() % 10000 <= (fPefectProp * 10000);
//
//		if(bPefect)
//		{
//			// ����װ��Ǳ��ֵΪ���Ƴɹ���75%
//			pEquip->ChangePotVal(-pPosyProto->nPotValConsume * 75 / 100);
//			GetItemMgr().UpdateEquipSpec(*pEquip);
//			return E_Compose_Consolidate_Perfect;
//		}
//		else
//		{
//			pEquip->ChangePotVal(-pPosyProto->nPotValConsume);
//			GetItemMgr().UpdateEquipSpec(*pEquip);
//			return E_Compose_Consolidate_Success;
//		}
//	}
//	else				// ʧ��
//	{
//		// װ��Ǳ��ֵ����5��
//		if(IMEffect.e_effect !=  eime_protectsign)
//			pEquip->ChangePotVal(-5);
//
//		GetItemMgr().UpdateEquipSpec(*pEquip);
//		return E_Compose_Consolidate_Lose;
//	}
//}

//-----------------------------------------------------------------------------
// GM����
//-----------------------------------------------------------------------------
//DWORD Role::GMPosyEquip(DWORD dwFormulaID, INT16 n16ItemIndex)
//{
//	// �ҵ�ǿ������
//	const tagPosyProtoSer *pPosyProto = AttRes::GetInstance()->GetPosyProto(dwFormulaID);
//	if(!VALID_POINT(pPosyProto))
//		return E_Compose_Formula_Not_Exist;
//
//	// �ҵ���ǿ��װ��
//	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n16ItemIndex);
//	if(!VALID_POINT(pEquip))
//		return E_Consolidate_Equip_Not_Exist;
//
//	if(!MIsEquipment(pEquip->dw_data_id))
//		return E_Consolidate_NotEquipment;
//
//	// ������ƴ���
//	if(pEquip->equipSpec.byPosyTimes >= MAX_ENGRAVE_TIMES)
//		return E_Consolidate_BeyondPosyTime;
//
//	// ���ǿ���䷽�Ƿ�Ϸ�
//	if((pEquip->equipSpec.byPosyTimes + 1 ) != pPosyProto->byPosyTimes)
//		return E_Consolidate_FormulaNotMatch;
//
//	// ����Ƿ���ǿ���䷽��Ӧ��װ������
//	if(!AttRes::GetInstance()->IsPosyPos(pPosyProto->ePosyAtt, pEquip->pEquipProto->eEquipPos))
//		return E_Consolidate_EquipCanNotPosy;
//
//	// װ�����ƴ�����һ
//	pEquip->equipSpec.byPosyTimes++;
//
//	// ����װ����Ӧǿ��ֵ
//	// ��ֵ=��װ���ȼ���fcoefficientA + fcoefficientB��/ fcoefficientC ����1+����Ʒ�ʼӳɣ�
//
//	// �����ӳ�
//	FLOAT fBaseAttInc = ((FLOAT)pEquip->pProtoType->byLevel * pPosyProto->fcoefficientA 
//		+ pPosyProto->fcoefficientB) / pPosyProto->fcoefficientC;
//
//	// ����ת�� 
//	ERoleAttribute eRoleAtt = ERA_Null;
//	ConsolidatePosyAttConvert(eRoleAtt, pPosyProto->ePosyAtt);
//
//	for(INT n = 0; n < MAX_ROLEATT_POSY_EFFECT; ++n)
//	{
//		if(pEquip->equipSpec.PosyEffect[n].eRoleAtt == eRoleAtt)
//		{
//			pEquip->equipSpec.PosyEffect[n].nValue += (INT32)(fBaseAttInc * (1.0f + 0));
//			break;
//		}
//		else if(pEquip->equipSpec.PosyEffect[n].eRoleAtt == ERA_Null)
//		{
//			pEquip->equipSpec.PosyEffect[n].eRoleAtt = eRoleAtt;
//			pEquip->equipSpec.PosyEffect[n].nValue += (INT32)(fBaseAttInc * (1.0f + 0));
//			break;
//		}
//	}
//
//	// ����װ������
//	CalEquipFlare(pEquip);
//		
//	GetItemMgr().UpdateEquipSpec(*pEquip);
//	return E_Compose_Consolidate_Success;
//}

//-----------------------------------------------------------------------------
// �Կ�
//-----------------------------------------------------------------------------
//DWORD Role::EngraveEquip(DWORD dwNPCID, DWORD dwFormulaID, INT64 n64ItemID, INT64 n64IMID, INT64 n64StuffID[], INT32 nArraySz, DWORD dw_cmd_id)
//{
//	// ��õ�ͼ
//	Map *pMap = get_map();
//	if(!VALID_POINT(pMap))
//	{
//		ASSERT(VALID_POINT(pMap));
//		return INVALID_VALUE;
//	}
//
//	Creature* pNPC = pMap->find_creature(dwNPCID);
//	if( !VALID_POINT(pNPC) ) 
//		return E_Compose_NPC_Not_Exist;
//
//	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
//		return E_Compose_NPC_Distance;
//
//	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Engrave) )
//		return E_Compose_NPCCanNotEngrave;
//
//	// �ҵ�ǿ������
//	const tagEngraveProtoSer *pEngraveProto = AttRes::GetInstance()->GetEngraveProto(dwFormulaID);
//	if(!VALID_POINT(pEngraveProto))
//		return E_Compose_Formula_Not_Exist;
//
//	// �ҵ���ǿ��װ��
//	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64ItemID);
//	if(!VALID_POINT(pEquip))
//		return E_Consolidate_Equip_Not_Exist;
//
//	if(!MIsEquipment(pEquip->dw_data_id))
//		return E_Consolidate_NotEquipment;
//
//	// ����Կ̴���
//	if(pEquip->equipSpec.byEngraveTimes >= MAX_ENGRAVE_TIMES)
//		return E_Consolidate_BeyondEngraveTime;
//
//	// ���ǿ���䷽�Ƿ�Ϸ�
//	if((pEquip->equipSpec.byEngraveTimes + 1 ) != pEngraveProto->byEngraveTimes)
//		return E_Consolidate_FormulaNotMatch;
//
//	// ����Ƿ���ǿ���䷽��Ӧ��װ������
//	if(!AttRes::GetInstance()->IsEngravePos(pEngraveProto->eEngraveAtt, pEquip->pEquipProto->eEquipPos))
//		return E_Consolidate_EquipCanNotEngrave;
//
//	// ���װ��Ǳ��ֵ�Ƿ��㹻
//	if(pEquip->equipSpec.nPotVal < (INT)pEngraveProto->nPotValConsume)
//		return E_Consolidate_ValConsume_Inadequate;
//
//	// �����ҽ�Ǯ�Ƿ��㹻
//	if(GetCurMgr().GetBagSilver() < pEngraveProto->dw_money_consume)
//		return E_Consolidate_NotEnough_Money;
//
//	// �����Ҳ����Ƿ��㹻(˳������ܲ���������
//	INT nItemQualityNum[EIQ_End];
//	ZeroMemory(nItemQualityNum, sizeof(nItemQualityNum));
//
//	for(INT n = 0; n < MAX_CONSOLIDATE_STUFF_QUANTITY; ++n)
//	{
//		if(pEngraveProto->ConsolidateStuff[n].dwStuffID == INVALID_VALUE 
//			&& pEngraveProto->ConsolidateStuff[n].eStuffType == EST_Null)
//			break;
//
//		tagItem *pItemStuff = GetItemMgr().GetBagItem(n64StuffID[n]); 
//
//		if(!VALID_POINT(pItemStuff))
//			return E_Consolidate_NotEnough_Stuff;
//
//		// ������64λID�Ƿ��ظ�
//		for(INT i = 0; i < n; ++i)
//		{
//			if(n64StuffID[i] == n64StuffID[n])
//				return E_Consolidate_NotEnough_Stuff;
//		}
//
//		if(pItemStuff->dw_data_id != pEngraveProto->ConsolidateStuff[n].dwStuffID 
//			&& pItemStuff->pProtoType->eStuffType != pEngraveProto->ConsolidateStuff[n].eStuffType)
//			return E_Consolidate_NotEnough_Stuff;
//
//		if(pItemStuff->n16Num < (INT)pEngraveProto->ConsolidateStuff[n].dwStuffNum)
//			return E_Consolidate_NotEnough_Stuff;
//
//		nItemQualityNum[pItemStuff->pProtoType->byQuality] += pEngraveProto->ConsolidateStuff[n].dwStuffNum;
//	}
//
//	// ����IM���ߵ�Ӱ��
//	tagIMEffect			IMEffect;
//	tagEngraveProtoSer  *pProto = const_cast<tagEngraveProtoSer*>(pEngraveProto);
//	CalIMEffect(ECTS_Engrave, IMEffect, n64IMID, pProto,1);
//
//	// �����������
//	for(INT n = 0; n < nArraySz; ++n)
//	{
//		GetItemMgr().DelFromBag(n64StuffID[n], (DWORD)elcid_compose_engrave, (INT16)pEngraveProto->ConsolidateStuff[n].dwStuffNum);
//	}
//
//	// ��Ǯ����
//	GetCurMgr().DecBagSilver(pEngraveProto->dw_money_consume, elcid_compose_engrave);
//
//	// ����ɹ���
//	// ���ճɹ���=�����ɹ���+��-10%����Ʒ��������/�ܲ�������+0%����Ʒ��������/�ܲ�������+5%����Ʒ��������
//	// /�ܲ�������+10%����Ʒ��������/�ܲ�������+20%����Ʒ��������/�ܲ�����������[1+��װ���ȼ�-75��/150]
//	// +����ɫ��Ե/1000��+IM���߼ӳ�                                      ���ճɹ��ʵ�ȡֵΪ0%-100%
//	FLOAT fProp = 0;
//	fProp = (FLOAT)pEngraveProto->nSuccessRate + (((-0.1f * (FLOAT)nItemQualityNum[EIQ_Quality0] 
//		    + 0.05f * (FLOAT)nItemQualityNum[EIQ_Quality4] + 0.1f * (FLOAT)nItemQualityNum[EIQ_Quality1] 
//			+ 0.2f * (FLOAT)nItemQualityNum[EIQ_Quality3]) / (FLOAT)pEngraveProto->nTotleStuff) 
//			* (1.0f + ((FLOAT)pEquip->pProtoType->byLevel - 75.0f) / 150.0f) 
//			+ ((FLOAT) GetAttValue(ERA_Vigour) / 1000.0f)) * 10000;
//
//	// ����B�����ԶԳɹ���Ӱ��
//	fProp = CalSpecAttEffectSuc((EEquipSpecAtt)(pEquip->equipSpec.bySpecAtt), fProp, EESA_Guard_Engrave);
//
//	// �������Ƿ�������߳ɹ��ʵ�IM
//	if( IMEffect.e_effect ==  eime_comadvance)
//	{
//		fProp += (FLOAT)IMEffect.dw_param2;
//	}
//
//	BOOL bResult = get_tool()->tool_rand() % 10000 <= (INT)fProp;
//
//	if(bResult)			// �ɹ�
//	{
//		// װ�����ƴ�����һ
//		pEquip->equipSpec.byEngraveTimes++;
//
//		// ����װ����Ӧǿ��ֵ
//		// ��ֵ=��װ���ȼ���fcoefficientA + fcoefficientB��/ fcoefficientC ����1+����Ʒ�ʼӳɣ�
//		/* ����Ʒ�ʼӳ�=��-20%����Ʒ��������/�ܲ�������+0%����Ʒ��������/�ܲ�������+20%
//		����Ʒ��������/�ܲ�������+50%����Ʒ��������/�ܲ�������+100%����Ʒ��������/�ܲ���������
//		����Ʒ�ʼӳɵ�ȡֵΪ0%-50%  */
//
//		// �����ӳ�
//		FLOAT fBaseAttInc = ((FLOAT)pEquip->pProtoType->byLevel * pEngraveProto->fcoefficientA 
//			+ pEngraveProto->fcoefficientB) / pEngraveProto->fcoefficientC;
//
//		// ����Ʒ�ʼӳ�
//		FLOAT fStuffAttInc = (-0.2f * (FLOAT)nItemQualityNum[EIQ_Quality0] + 0.2f * (FLOAT)nItemQualityNum[EIQ_Quality4] 
//							  + 0.5f * (FLOAT)nItemQualityNum[EIQ_Quality1] + 1.0f * (FLOAT)nItemQualityNum[EIQ_Quality3] )
//							  / (FLOAT)pEngraveProto->nTotleStuff; 
//
//		if(fStuffAttInc < 0.0f)
//			fStuffAttInc = 0.0f;
//		if(fStuffAttInc > 1.0f)
//			fStuffAttInc = 1.0f;
//
//		// ���ռӳ�
//		INT nEngraveAttInc = (INT)(fBaseAttInc * (1.0f + fStuffAttInc));
//
//		switch(pEngraveProto->eEngraveAtt)
//		{
//		case EEngraveAtt_WeaponDmg:
//			pEquip->equipSpec.nEngraveAtt[0] += nEngraveAttInc;
//			pEquip->equipSpec.nEngraveAtt[1] += nEngraveAttInc;
//			break;
//		case EEngraveAtt_WeaponSoul:
//			pEquip->equipSpec.nEngraveAtt[2] += nEngraveAttInc;
//			break;
//		case EEngraveAtt_Armor:
//			pEquip->equipSpec.nEngraveAtt[3] += nEngraveAttInc;
//			break;
//		case EEngraveAtt_Deration:
//			pEquip->equipSpec.nEngraveAtt[3] += nEngraveAttInc;
//			break;
//		default:
//			break;
//		}
//
//		// Ǳ��ֵ����
//		// ����������=�����ɹ��ʡ�1/20+��ɫ��Ե/1000
//		FLOAT fPefectProp = (fProp * 0.05f) / 10000.0f + (FLOAT)GetAttValue(ERA_Vigour) / 1000.0f;
//
//		// ����B�����Զ�������Ӱ��
//		fPefectProp = CalSpecAttEffectPef((EEquipSpecAtt)(pEquip->equipSpec.bySpecAtt), fPefectProp);
//
//		// ����װ������
//		CalEquipFlare(pEquip);
//
//		BOOL bPefect = get_tool()->tool_rand() % 10000 <= (fPefectProp * 10000);
//
//		if(bPefect)
//		{
//			// ����װ��Ǳ��ֵΪ���Ƴɹ���75%
//			pEquip->ChangePotVal(-pEngraveProto->nPotValConsume * 75 / 100);
//			GetItemMgr().UpdateEquipSpec(*pEquip);
//			return E_Compose_Consolidate_Perfect;
//		}
//		else
//		{
//			pEquip->ChangePotVal(-pEngraveProto->nPotValConsume);
//			GetItemMgr().UpdateEquipSpec(*pEquip);
//			return E_Compose_Consolidate_Success;
//		}
//	}
//	else				// ʧ��
//	{
//		// װ��Ǳ��ֵ����5��
//		if(IMEffect.e_effect !=  eime_protectsign)
//			pEquip->ChangePotVal(-5);
//
//		GetItemMgr().UpdateEquipSpec(*pEquip);
//		return E_Compose_Consolidate_Lose;
//	}
//}

//-----------------------------------------------------------------------------
// GM �Կ�
//-----------------------------------------------------------------------------
//DWORD Role::GMEngraveEquip(DWORD dwFormulaID, INT16 n16ItemIndex)
//{
//	// �ҵ�ǿ������
//	const tagEngraveProtoSer *pEngraveProto = AttRes::GetInstance()->GetEngraveProto(dwFormulaID);
//	if(!VALID_POINT(pEngraveProto))
//		return E_Compose_Formula_Not_Exist;
//
//	// �ҵ���ǿ��װ��
//	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n16ItemIndex);
//	if(!VALID_POINT(pEquip))
//		return E_Consolidate_Equip_Not_Exist;
//
//	if(!MIsEquipment(pEquip->dw_data_id))
//		return E_Consolidate_NotEquipment;
//
//	// ����Կ̴���
//	if(pEquip->equipSpec.byEngraveTimes >= MAX_ENGRAVE_TIMES)
//		return E_Consolidate_BeyondEngraveTime;
//
//	// ���ǿ���䷽�Ƿ�Ϸ�
//	if((pEquip->equipSpec.byEngraveTimes + 1 ) != pEngraveProto->byEngraveTimes)
//		return E_Consolidate_FormulaNotMatch;
//
//	// ����Ƿ���ǿ���䷽��Ӧ��װ������
//	if(!AttRes::GetInstance()->IsEngravePos(pEngraveProto->eEngraveAtt, pEquip->pEquipProto->eEquipPos))
//		return E_Consolidate_EquipCanNotEngrave;
//
//	// װ�����ƴ�����һ
//	pEquip->equipSpec.byEngraveTimes++;
//
//	// ����װ����Ӧǿ��ֵ
//	// ��ֵ=��װ���ȼ���fcoefficientA + fcoefficientB��/ fcoefficientC ����1+����Ʒ�ʼӳɣ�
//
//	// �����ӳ�
//	FLOAT fBaseAttInc = ((FLOAT)pEquip->pProtoType->byLevel * pEngraveProto->fcoefficientA 
//		+ pEngraveProto->fcoefficientB) / pEngraveProto->fcoefficientC;
//
//	// ���ռӳ�
//	INT nEngraveAttInc = (INT)(fBaseAttInc * (1.0f + 0));
//
//	switch(pEngraveProto->eEngraveAtt)
//	{
//	case EEngraveAtt_WeaponDmg:
//		pEquip->equipSpec.nEngraveAtt[0] += nEngraveAttInc;
//		pEquip->equipSpec.nEngraveAtt[1] += nEngraveAttInc;
//		break;
//	case EEngraveAtt_WeaponSoul:
//		pEquip->equipSpec.nEngraveAtt[2] += nEngraveAttInc;
//		break;
//	case EEngraveAtt_Armor:
//		pEquip->equipSpec.nEngraveAtt[3] += nEngraveAttInc;
//		break;
//	case EEngraveAtt_Deration:
//		pEquip->equipSpec.nEngraveAtt[3] += nEngraveAttInc;
//		break;
//	default:
//		break;
//	}
//	
//	// ����װ������
//	CalEquipFlare(pEquip);
//
//	GetItemMgr().UpdateEquipSpec(*pEquip);
//	return E_Compose_Consolidate_Success;
//}
//ǿ������
DWORD Role::ConsolidateEquip(DWORD dwNPCID, INT64 n64ItemID, INT64 n64StuffID, INT64 n64BaohuID, DWORD n64IMID, INT n_num, BOOL bBind)
{
	// ��õ�ͼ
	Map *pMap = get_map();  
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// �ҵ���ǿ��װ��
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64ItemID);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	//Ҫ���ǵ���Ʒ����װ��
	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;

	//װ���Ѿ������������
	if(pEquip->equipSpec.byConsolidateLevel >= MAX_CONSOLIDATE_LEVEL)
		return E_ShengXing_CanNot;

	// ʱװ��򲻿���ǿ��
	if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
		pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
		pEquip->pEquipProto->eEquipPos == EEP_Ride)
		return E_ShengXing_CanNot;

	// �����������ߵȼ��Ƿ�����
	const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(n64IMID);
	if (!VALID_POINT(pItemProto))
	{	
		return E_ShengXingItem_Not_Exit;
	}

	if (n_num <= 0)
		return E_ShengXingItem_Not_Exit;

	if (pItemProto->eSpecFunc != EISF_ComposeAdvance)
		return E_ShengXing_Item_Error;

	//����ǿ15ǿ��ʯ
	if (pItemProto->nSpecFuncVal1 == 0 && pItemProto->nSpecFuncVal2 == 0)
	{
		INT32 nNum = GetItemMgr().GetBagSameItemCount(n64IMID);
		if (nNum < 1)
			return E_ShengXing_NumError;

		BOOL bBind = GetItemMgr().GetBagSameBindItemCount(n64IMID,TRUE) > 0;//��ð󶨵���Ʒ����
		
		GetItemMgr().RemoveFromRole(n64IMID,1,(DWORD)elcid_npc_shengxin,bBind);

		pEquip->equipSpec.byConsolidateLevel = MAX_CONSOLIDATE_LEVEL;


		GetItemMgr().UpdateEquipSpec(*pEquip);
		if( VALID_POINT(m_pScript) )
			m_pScript->OnEquipConsolidate( this, pEquip->equipSpec.byConsolidateLevel );
		//��ǿ���ȼ���15,10,6������Ҫ��ȫ������ gx add
		
		HearSayHelper::SendMessage(EHST_EQUIPSTRENGTH,pEquip->dwOwnerID,pEquip->dw_data_id,15);
		
		return E_Success;
	}

	if (pItemProto->nSpecFuncVal1 > pEquip->equipSpec.byConsolidateLevel ||
		pItemProto->nSpecFuncVal2 < pEquip->equipSpec.byConsolidateLevel)
		return E_ShengXing_Item_Error;
	

	//gx modify 2013.9.5 �������İ󶨵���Ʒ
	/*package_list<tagItem*> itemList;
	INT32 n32num = GetItemMgr().GetBag().GetSameItemList(itemList, n64IMID, n_num);
	if (n32num < n_num)
		return E_ShengXing_NumError;

	GetItemMgr().DelBagSameItem(itemList, n_num, elcid_npc_shengxin);*/
	INT32 n32num = GetItemMgr().GetBagSameItemCount(n64IMID);
	if (n32num < n_num)
		return E_ShengXing_NumError;
	INT32 n32Bindnum = GetItemMgr().GetBagSameBindItemCount(n64IMID,TRUE);//��ð󶨵���Ʒ����
	//���󶨵Ĺ�
	if (n32Bindnum >= n_num)
	{
		GetItemMgr().RemoveFromRole(n64IMID,n_num,(DWORD)elcid_npc_shengxin,1);
	}
	else//���󶨵Ĳ���
	{
		if (n32Bindnum > 0)//�����İ󶨵�
		{
			GetItemMgr().RemoveFromRole(n64IMID,n32Bindnum,(DWORD)elcid_npc_shengxin,1);
		}
		GetItemMgr().RemoveFromRole(n64IMID,(n_num-n32Bindnum),(DWORD)elcid_npc_shengxin,0);
	}
	//end
	GetAchievementMgr().UpdateAchievementCriteria(ete_consolidate_up_pro, n64IMID, n_num);



	INT fProp = 0;

	fProp = ItemHelper::GetUpstarsAddPro(n_num, pEquip->equipSpec.byConsolidateLevel);
	fProp += GET_VIP_EXTVAL(GetTotalRecharge(), CONSOLIDATE_EQUIP_PROB, INT);

	BOOL bResult = get_tool()->tool_rand() % 10000 <= fProp;

	if (bResult)
	{
	
		pEquip->equipSpec.byConsolidateLevel++;

		CalEquipFlare(pEquip);//�������ֵ
		GetItemMgr().UpdateEquipSpec(*pEquip);
		if( VALID_POINT(m_pScript) )
			m_pScript->OnEquipConsolidate( this, pEquip->equipSpec.byConsolidateLevel );
		//��ǿ���ȼ���15,10,6������Ҫ��ȫ������ gx add
		if (MAX_CONSOLIDATE_LEVEL == pEquip->equipSpec.byConsolidateLevel)
		{
			HearSayHelper::SendMessage(EHST_EQUIPSTRENGTH,pEquip->dwOwnerID,pEquip->dw_data_id,15);
		}
		else if (10 == pEquip->equipSpec.byConsolidateLevel)
		{
			HearSayHelper::SendMessage(EHST_EQUIPSTRENGTH,pEquip->dwOwnerID,pEquip->dw_data_id,10);
		}
		else if (6 == pEquip->equipSpec.byConsolidateLevel)
		{
			HearSayHelper::SendMessage(EHST_EQUIPSTRENGTH,pEquip->dwOwnerID,pEquip->dw_data_id,6);
		}
		else
		{
			//do nothing
		}
	}
	else
	{

		RecalAtt();

		CalEquipFlare(pEquip);

		GetItemMgr().UpdateEquipSpec(*pEquip);
	}
	

	return bResult ? E_Success : E_ShengXing_Abort;
}

//GMǿ��
DWORD	Role::GMConsolidateEquip(INT16 n16ItemIndex, INT level)
{
	// �ҵ���ǿ��װ��
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n16ItemIndex);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	//Ҫ���ǵ���Ʒ����װ��
	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;

	//װ���Ѿ������������
	if(pEquip->equipSpec.byConsolidateLevel >= MAX_CONSOLIDATE_LEVEL)
		return E_ShengXing_CanNot;


	pEquip->equipSpec.byConsolidateLevel = level;
	if (pEquip->equipSpec.byConsolidateLevel > MAX_CONSOLIDATE_LEVEL)
	{
		pEquip->equipSpec.byConsolidateLevel = MAX_CONSOLIDATE_LEVEL;
	}
	
	//pEquip->equipSpec.byConsolidateLevelStar = pEquip->equipSpec.byConsolidateLevel;

	CalEquipFlare(pEquip);//�������ֵ
	GetItemMgr().UpdateEquipSpec(*pEquip);

	return E_Success;
}

VOID Role::OnConsolidateFail(tagEquip* pEquip, BOOL bBind)
{
	ASSERT(VALID_POINT(pEquip));

	BYTE byLevel = pEquip->equipSpec.byConsolidateLevel;
	switch(byLevel)
	{
	case 0:
	case 1:
	case 2:
	case 6:
	case 7:
	case 8:
	case 9:
		pEquip->equipSpec.byConsolidateLevel = bBind ? pEquip->equipSpec.byConsolidateLevel:0 ;
		//pEquip->equipSpec.byConsolidateLevelStar = 0;
		break;
	case 3:
		pEquip->equipSpec.byConsolidateLevel = bBind ? pEquip->equipSpec.byConsolidateLevel:2;
		//pEquip->equipSpec.byConsolidateLevelStar = 2;
		break;
	case 4:
		pEquip->equipSpec.byConsolidateLevel = bBind ? pEquip->equipSpec.byConsolidateLevel:3;
		//pEquip->equipSpec.byConsolidateLevelStar = 3;
		break;
	case 5:
		pEquip->equipSpec.byConsolidateLevel = bBind ? pEquip->equipSpec.byConsolidateLevel:4;
		//pEquip->equipSpec.byConsolidateLevelStar = 4;
		break;
	default:
		break;
	}
}
//-----------------------------------------------------------------------------
// ��Ƕ
//-----------------------------------------------------------------------------
DWORD Role::InlayEquip(INT64 n64DstItemID, INT64 (&n64SrcItemID)[MAX_EQUIPHOLE_NUM])
{
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	// �ҵ���ǿ��װ��
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64DstItemID);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;

	if((INT)pEquip->equipSpec.byHoleNum == 0)
		return E_Consolidate_Gem_Not_Hole;

	// ʱװ��򲻿�����Ƕ
	if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
		pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
		pEquip->pEquipProto->eEquipPos == EEP_Ride)
		return E_Consolidate_Shipingqu_Not;

	// ��Ǯ�ж�
	//const tagFormulaParam* pFormula = AttRes::GetInstance()->GetFormulaParam();
	//if(!VALID_POINT(pFormula))
	//	return E_Consolidate_Param_Not_Find;

	//if(GetCurMgr().GetBagAllSilver() < pFormula->n16InlayBaseSilver)
	//	return E_Consolidate_NotEnough_Money;

	
	BOOL bSuc = FALSE;
	DWORD res = E_Success;
	// �ҵ���ʯ
	for (INT i = 0; i < (INT)pEquip->equipSpec.byHoleNum; i++)
	{
		if (n64SrcItemID[i] == INVALID_VALUE)
			continue;

		tagItem *pItemGem = GetItemMgr().GetBagItem(n64SrcItemID[i]);
		if(!VALID_POINT(pItemGem) || pItemGem->pProtoType->eSpecFunc != EISF_HoleGem )
		{
			res =  E_Consolidate_Gem_Not_Exit;
			continue;
		}
		
		if (pEquip->pProtoType->byLevel < pItemGem->pProtoType->nSpecFuncVal1 || 
			pEquip->pProtoType->byLevel > pItemGem->pProtoType->nSpecFuncVal2)
		{
			res = E_Consolidate_Gem_level_not;
			continue;
		}

		// �ҵ�ǿ������
		const tagConsolidateItem *pConsolidateProto = AttRes::GetInstance()->GetConsolidateProto(pItemGem->dw_data_id);
		if(!VALID_POINT(pConsolidateProto))
		{
			res =  E_Compose_Formula_Not_Exist;
			continue;
		}

		for (int j = 0; j < pEquip->equipSpec.byHoleNum; j++)
		{
			int nNewGemType = pItemGem->pProtoType->dw_data_id / 100 % 100;
			int nGemType = pEquip->equipSpec.dwHoleGemID[j] / 100 % 100;
			if (nNewGemType == nGemType)
			{
				res = E_Consolidate_EquipCanNotInlay;
				break;
			}
		}
		if (res == E_Consolidate_EquipCanNotInlay)
			continue;

		// ��Ƕ��ʯ
		//if (VALID_POINT(pEquip->equipSpec.dwHoleGemID[i]))
		//{
		//	GetAchievementMgr().UpdateAchievementCriteria(ete_inlay_repeat, 1);
		//}

		//GetAchievementMgr().UpdateAchievementCriteria(ete_inlay_level, pItemGem->pProtoType->nSpecFuncVal1 , 1);
		
		pEquip->equipSpec.dwHoleGemID[i] = pItemGem->dw_data_id;

		// ɾ������
		GetItemMgr().DelFromBag(n64SrcItemID[i], (DWORD)elcid_compose_enchase, 1);	

		bSuc = TRUE;

	}

	if (!bSuc)
	{
		return res;
	}
	// ����Ƿ�ʯ�Ѿ�����
	//if(pEquip->equipSpec.dwHoleGemID[pEquip->equipSpec.byHoleNum - 1] != INVALID_VALUE 
	//	&& pEquip->equipSpec.dwHoleGemID[pEquip->equipSpec.byHoleNum - 1] != 0 )
	//	return E_Consolidate_Gem_Full;
	
	//if(dwHoldIndex > pEquip->equipSpec.byHoleNum)
	//	return E_Consolidate_Gem_Full;


	// ��ⱦʯ�Ƿ�����Ƕ��װ����
	//BOOL bConsolidate = FALSE;
	//for(INT m = 0; m < MAX_CONSOLIDATE_POS_QUANTITY; ++m)
	//{
	//	if(pEquip->pEquipProto->eEquipPos != pConsolidateProto->ConsolidatePos[m].ePos)
	//		continue;
	//	else
	//	{
	//		if( 1 == pConsolidateProto->ConsolidatePos[m].byConsolidate)
	//		{
	//			bConsolidate = TRUE;
	//			break;
	//		}
	//	}
	//}

	//if(!bConsolidate)
	//	return E_Consolidate_EquipCanNotInlay;

	// ���װ��Ǳ��ֵ�Ƿ��㹻
	//if(pEquip->equipSpec.nPotVal < (INT)pConsolidateProto->dwPotValConsume)
	//	return E_Consolidate_ValConsume_Inadequate;

	// ��Ƕ��ʯ
	//for(INT i = 0; i < (INT)pEquip->equipSpec.byHoleNum; ++i)
	//{
	//	if(pEquip->equipSpec.dwHoleGemID[i] == INVALID_VALUE || pEquip->equipSpec.dwHoleGemID[i] == 0)
	//	{
	//		pEquip->equipSpec.dwHoleGemID[i] = n64SrcItemID[i];
	//		break;
	//	}
	//}

	// ����װ������
	CalEquipFlare(pEquip);

	// ����װ��Ǳ��ֵ
	//pEquip->ChangePotVal(-(INT)pConsolidateProto->dwPotValConsume);
	GetItemMgr().UpdateEquipSpec(*pEquip);
	
	if(VALID_POINT(GetScript()))
		GetScript()->OnInlayEquip(this);
	
	// ��Ǯ����
	//GetCurMgr().DecBagSilverEx(pFormula->n16InlayBaseSilver, elcid_npc_inlay);
	
	GetAchievementMgr().UpdateAchievementCriteria(eta_inlay, 1);

	return res;	
}
//-----------------------------------------------------------------------------
// ��Ƕ
//-----------------------------------------------------------------------------
DWORD Role::Unbeset(INT64 n64SerialEquip, DWORD dwNPCID, BYTE byUnBesetPos)
{
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	DWORD dwError = E_Success;

	//Creature* pNPC = pMap->find_creature(dwNPCID);
	//if( !VALID_POINT(pNPC) ) 
	//	return E_Compose_NPC_Not_Exist;

	//if( FALSE == pNPC->CheckNPCTalkDistance(this) )
	//	return E_Compose_NPC_Distance;

	/*if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Unbeset) )
	return E_Unbeset_NPCCanNot_Exit;*/

	if (byUnBesetPos < 0 || byUnBesetPos >= MAX_EQUIPHOLE_NUM)
		return INVALID_VALUE;

	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;
	
	//if (pEquip->byBind == EBS_Bind || pEquip->byBind == EBS_NPC_Bind)
	//{
	//	return INVALID_VALUE;
	//}

	// ��ȡҪ��Ǩ�ı�ʯ
	DWORD dwBesetTypeID = pEquip->equipSpec.dwHoleGemID[byUnBesetPos];
	if(dwBesetTypeID <= 0)
		return E_Unbeset_BesetItem_Not_Exit;


	//const tagConsolidateItem* pFormula = AttRes::GetInstance()->GetConsolidateProto(dwBesetTypeID);
	//if(!VALID_POINT(pFormula))
	//	return E_Unbeset_FormulaParam_Not_Exit;

	tagItemProto* pItemProto = (tagItemProto*)AttRes::GetInstance()->GetItemProto(dwBesetTypeID);
	if(!VALID_POINT(pItemProto))
		return E_Unbeset_Item_Not_Exit;
	
	// �����ռ��ж�
	if (GetItemMgr().GetBagFreeSize() <= 0)
		return E_Unbeset_Not_bag_free;			
	
	// ��Ǯ�ж�
	//if(GetCurMgr().GetBagAllSilver() < pFormula->nMoney)
	//	return E_Consolidate_NotEnough_Money;

	// ��Ǯ����
	//GetCurMgr().DecBagSilverEx(pFormula->nMoney, elcid_npc_unbeset);
	
	// �����Ƭ
	GetItemMgr().Add2Role(EICM_Unbeset, m_dwID, dwBesetTypeID, 1, EIQ_Null, elcid_npc_unbeset, 0);


	for(INT16 i = 0; i < (INT16)pEquip->equipSpec.byHoleNum; i++)
	{
		if(pEquip->equipSpec.dwHoleGemID[i] == dwBesetTypeID)
		{
			pEquip->equipSpec.dwHoleGemID[i] = 0;	
			break;
		}
	}
	

	//DWORD dwHoleGemID[MAX_EQUIPHOLE_NUM];
	//::ZeroMemory(dwHoleGemID, sizeof(dwHoleGemID));
	//BYTE  byCount = 0;
	//for(INT16 i = 0; i < (INT16)pEquip->equipSpec.byHoleNum; i++)
	//{
	//	if(pEquip->equipSpec.dwHoleGemID[i] > 0)
	//	{
	//		dwHoleGemID[byCount] = pEquip->equipSpec.dwHoleGemID[i];
	//		byCount++;
	//	}
	//}

	//memcpy(pEquip->equipSpec.dwHoleGemID, dwHoleGemID, sizeof(dwHoleGemID));
	GetItemMgr().UpdateEquipSpec(*pEquip);
	//GetAchievementMgr().UpdateAchievementCriteria(eta_unbeset, 1);
	//GetAchievementMgr().UpdateAchievementCriteria(eta_unbeset_level, pItemProto->nSpecFuncVal1 , 1);

	return dwError;
}

//-----------------------------------------------------------------------------
// ԭʯ��ĥ
//-----------------------------------------------------------------------------
DWORD	Role::damo(DWORD dwItemID, DWORD dwNumber, DWORD dwNPCID)
{
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	DWORD dwError = E_Success;

	Creature* pNPC = pMap->find_creature(dwNPCID);
	if( !VALID_POINT(pNPC) ) 
		return E_Compose_NPC_Not_Exist;

	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
		return E_Compose_NPC_Distance;

	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_DAMO) )
	return E_Role_yuanshi_damo_NPC_NOT;
	
	package_list<tagItem*> itemList;

	if (dwItemID != INVALID_VALUE && dwItemID != 0)
	{
		const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwItemID);
		if (!VALID_POINT(pItemProto))
			return INVALID_VALUE;
		
		if (pItemProto->eSpecFunc != EISF_HoleGem)
			return INVALID_VALUE;


		const tagConsolidateItem *pConsolidateProto = AttRes::GetInstance()->GetConsolidateProto(dwItemID);
		if(!VALID_POINT(pConsolidateProto))
			return INVALID_VALUE;
		
		// �Ƿ���ԭʯ
		if (pConsolidateProto->dwDamoItem == 0)
			return E_Role_yuanshi_damo_not_yuanshi;

		// npc�ȼ��ж�
		if (pItemProto->byLevel != pNPC->GetProto()->uFunctionID.dwCommonID)
			return E_Role_yuanshi_damo_NPC_NOT;

		INT32 nNum = min(GetItemMgr().GetBag().GetSameItemList(itemList, dwItemID, dwNumber), dwNumber);

		if (pConsolidateProto->nDamoMonery * nNum > GetCurMgr().GetBagSilver())
			return E_Role_yuanshi_damo_not_silver;

		// �õ���Ӧ��ʯ
		if (GetItemMgr().Add2Role(EICM_DAMO, m_dwID, pConsolidateProto->dwDamoItem, nNum, EIQ_Null, elcid_npc_daomo, 0))
		{
			GetItemMgr().DelBagSameItem(itemList, nNum, elcid_npc_daomo);

			// ��Ǯ����
			GetCurMgr().DecBagSilver(pConsolidateProto->nDamoMonery *nNum, elcid_npc_daomo);

		}
		else
		{
			return E_Role_yuanshi_damo_error;
		}

	}
	else
	{
		return INVALID_VALUE;
	}


	return E_Success;
}
//-----------------------------------------------------------------------------
// ���
//-----------------------------------------------------------------------------
DWORD Role::ChiselEquip(INT64 n64SrcItemID, DWORD dwNPCID, INT64 n64SuffID)
{
	// ��õ�ͼ
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	//Creature* pNPC = pMap->find_creature(dwNPCID);
	//if( !VALID_POINT(pNPC) ) 
	//	return E_Compose_NPC_Not_Exist;

	//if( FALSE == pNPC->CheckNPCTalkDistance(this) )
	//	return E_Compose_NPC_Distance;

	/*if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Hole) )
	return E_Hole_NPCCanNot_Exit;*/

	// �ҵ���ǿ��װ��
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SrcItemID);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;

	if(!M_is_identified(pEquip))
		return E_Consolidate_NotIdentified;
	
	// ʱװ��򲻿��Դ��
	if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
		pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
		pEquip->pEquipProto->eEquipPos == EEP_Ride)
		return E_Consolidate_Shipingqu_Not;

	// ��Ʒ��ɫ���ϲ���
	//if (pEquip->pEquipProto->eEquipPos == EEP_Shipin1)
	//{
	//	if (pEquip->equipSpec.byQuality < EIQ_Quality3)
	//	{
	//		return E_Consolidate_Shipingqu_Not;
	//	}
	//}
	//const tagFormulaParam* pFormula = AttRes::GetInstance()->GetFormulaParam();
	//if(!VALID_POINT(pFormula))
	//	return E_Hole_FormulaParam_Not_Exit;

	/*if(pEquip->equipSpec.bCanCut == false)
	return E_Consolidate_Equip_CanNot_Chisel;*/

	// ���װ����Ƕ����
	if(pEquip->equipSpec.byHoleNum >= MAX_EQUIPHOLE_NUM)
		return E_Consolidate_Equip_Hole_Full;


	if (INVALID_VALUE != n64SuffID)
	{
		// �ҵ����ʯ
		tagItem *pItemChisel = GetItemMgr().GetBagItem(n64SuffID);
		if(!VALID_POINT(pItemChisel) || pItemChisel->pProtoType->eSpecFunc != EISF_Chisel )
		{
			return E_Consolidate_Chisel_Not_Exit;
		}

		//// ���ʯ�ȼ�����
		//if (pEquip->pEquipProto->byMinUseLevel < pItemChisel->pProtoType->nSpecFuncVal2 )
		//{
		//	return E_Consolidate_EquipLevel_Not;
		//}

		////�����Ƕ�����Ͳ����Ƿ�ƥ��
		//if(pEquip->equipSpec.byHoleNum != pItemChisel->pProtoType->nSpecFuncVal1)
		//	return E_Consolidate_Chisel_Not_Match;

		if (pItemChisel->n16Num <  1 /*+ pEquip->equipSpec.byHoleNum*/ )
			return E_Consolidate_Chisel_Not_Exit;

		// ��������
		GetItemMgr().DelFromBag(n64SuffID, (DWORD)elcid_compose_chisel, pEquip->equipSpec.byHoleNum + 1);


	}
	else
	{
		//gx modify 2013.9.25 �������������ȯ������Ԫ��
		INT32 needdwYuanbao =  CHISEL_NEED_MONERY;// * (pEquip->equipSpec.byHoleNum + 1);
		//if (GetCurMgr().GetBagYuanBao() >= needdwYuanbao)//�ȼ����ȯ
		//{
		//	GetCurMgr().DecBagYuanBao(needdwYuanbao,elcid_npc_hole);
		//}
		if (GetCurMgr().GetBaiBaoYuanBao() >= needdwYuanbao)//�ټ��Ԫ��
		{
			GetCurMgr().DecBaiBaoYuanBao(needdwYuanbao, elcid_npc_hole);
		}
		else
		{
			return E_Hole_NotEnough_Money;
		}
		//end
	}






	
	//INT64 nConSilver = FormulaHelper::GetEquipChisePrice(pEquip);
	
	//if(GetCurMgr().GetBagAllSilver() < nConSilver)
	//	return E_Hole_NotEnough_Money;

	pEquip->equipSpec.byHoleNum++;
	
	// ���ж�
	//if (pItemChisel->IsBind() && !pEquip->IsBind())
	//{
	//	pEquip->SetBind(EBS_Equip_Bind);
	//	NET_SIS_bind_change send;
	//	send.n64EquipSerial = n64SrcItemID;
	//	send.byBind = pEquip->byBind;
	//	send.dwConType = pEquip->eConType;
	//	SendMessage(&send, send.dw_size);
	//}

	// ��Ǯ����
	//GetCurMgr().DecBagSilverEx(nConSilver, elcid_npc_hole);

	GetItemMgr().UpdateEquipSpec(*pEquip);

	GetAchievementMgr().UpdateAchievementCriteria(eta_chisel, pEquip->equipSpec.byHoleNum);

	return E_Success;
}
// gm���
DWORD	Role::GMChiselEquip(INT16 pos, INT nHoldNum)
{
	// �ҵ���ǿ��װ��
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(pos);
	if(!VALID_POINT(pEquip))
		return E_Consolidate_Equip_Not_Exist;

	//����װ��
	if(!MIsEquipment(pEquip->dw_data_id))
		return E_Consolidate_NotEquipment;


	pEquip->equipSpec.byHoleNum = min(nHoldNum, MAX_EQUIPHOLE_NUM);

	GetItemMgr().UpdateEquipSpec(*pEquip);
	return E_Success;
}


//--------------------------------------------------------------------------------
// �����ϳ���Ʒ
//---------------------------------------------------------------------------------
DWORD Role::ProduceItem(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID,
						BOOL bBind, DWORD dw_cmd_id, DWORD byYuanBao, BOOL bFahion)
{
	// �ҵ��ϳɱ�����
	const s_produce_proto_ser* pProduceProto = AttRes::GetInstance()->GetProduceProto(dwFormulaID);

	if(!VALID_POINT(pProduceProto))
		return E_Compose_Formula_Not_Exist;

	DWORD dw_error_code = CheckProduceAlchemy(dwNPCID, dwSkillID, n64ItemID, dwFormulaID, pProduceProto, bBind);


	if (dw_error_code != E_Success)
	{
		return dw_error_code;
	}


	INT nNum = pProduceProto->dw_pro_quan_tity;

	tagItem *pItem = NULL;

	pItem = CreateItem(EICM_Product,  m_dwID, 
			pProduceProto->dw_pro_item_data_id, nNum, m_dwID, pProduceProto->bBind);
	


	if( VALID_POINT(pItem) )
	{
		DWORD dwRtv = GetItemMgr().Add2Bag(pItem, elcid_compose_produce, TRUE);

		if(E_Success != dwRtv)
		{
			SAFE_DELETE(pItem);
			return E_Compose_Consolidate_Lose;

		}
	}

	//ExpChange(pProduceProto->dw_master_incre);


	package_list<tagItem*> itemList;

	// ���Ĳ���
	for (int i = 0; i < MAX_PRODUCE_STUFF_QUANTITY; i++)
	{
		DWORD dwItemTypeID = pProduceProto->produce_stuff[i].dwStuffID;
		if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
			continue;

		DWORD dwItemNum = pProduceProto->produce_stuff[i].dwStuffNum;
		itemList.clear();
		INT32 nNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItemTypeID, dwItemNum);

		GetItemMgr().DelBagSameItem(itemList, dwItemNum, elcid_compose_produce);
	}
	

	return E_Success;


}

//---------------------------------------------------------------------------------
// �����ϳ�װ��
//--------------------------------------------------------------------------------
DWORD Role::ProduceEquip(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, 
										BOOL bBind, BYTE byQualityType, BYTE byQualityNum)
{
	// �ҵ��ϳɱ�����
	const s_produce_proto_ser* pProduceProto = AttRes::GetInstance()->GetProduceProto(dwFormulaID);

	if(!VALID_POINT(pProduceProto))
		return E_Compose_Formula_Not_Exist;

	DWORD dw_error_code = E_Success;
	
	const tagEquipProto* pEquipProto = AttRes::GetInstance()->GetEquipProto(pProduceProto->dw_pro_item_data_id);
	if (!VALID_POINT(pEquipProto)) return E_Compose_Consolidate_Lose;
 
	dw_error_code = CheckProduceLimit(dwNPCID, dwSkillID, n64ItemID, dwFormulaID, pProduceProto, bBind, byQualityType, byQualityNum);

	if( dw_error_code != E_Success)
		return dw_error_code;
	
	package_list<tagItem*> itemList;

	// ���Ĳ���
	for (int i = 0; i < MAX_PRODUCE_STUFF_QUANTITY; i++)
	{
		DWORD dwItemTypeID = pProduceProto->produce_stuff[i].dwStuffID;
		if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
			continue;

		DWORD dwItemNum = pProduceProto->produce_stuff[i].dwStuffNum;
		itemList.clear();
		INT32 nNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItemTypeID, dwItemNum);
		
		GetItemMgr().DelBagSameItem(itemList, dwItemNum, elcid_compose_produce);
	}
	
	// ���Ӻϳɼ���������(ֻ�м��ܺϳɲŻ����Ӽ��������ȣ�
	Skill* pSkill = GetSkill(NPC_PRODUCE_SKILL);
	ASSERT(VALID_POINT(pSkill));
	
	INT nSkillLevel = pSkill->get_level();

	BOOL bFromBox = FALSE;
	if (pProduceProto->e_form_from == EFormulaFrom_NPC)
	{
		// ���챦��
		INT64 dwMoney = FormulaHelper::GetEquipProducePrice(pEquipProto);
		const tagItem* pBoxItem = GetItemMgr().GetBagItem(n64ItemID);
		if (VALID_POINT(pBoxItem))
		{
			nSkillLevel = min(nSkillLevel, pBoxItem->pProtoType->nSpecFuncVal1);
			GetItemMgr().ItemUsedFromBag(n64ItemID, 1, (DWORD)elcid_compose_produce);
			dwMoney = dwMoney * ComposeHelper::GeProduceBoxMoneyParam();
			bFromBox = TRUE;
		}

		// ��Ǯ����
		GetCurMgr().DecBagSilver(dwMoney, elcid_compose_produce);
	}
	
	EItemQuality eQuality = EIQ_Null;
	
	INT nAddAttValue = 0;
	
	// ���ļ�������
	if (byQualityType > EIQ_Null && byQualityType < EIQ_End)
	{
		DWORD dwQualityItem = ItemHelper::GetProduceQualityID(pEquipProto->byMinUseLevel, byQualityType);
		INT nAttValue = ItemHelper::GetProduceQualityAttValue(pEquipProto->byMinUseLevel, byQualityType);
		itemList.clear();
		INT nMinNum = min(pProduceProto->nQualiyNum[byQualityType], byQualityNum);
		if (nMinNum > 0)
		{
			//nAddAttValue = min(nAttValue * nMinNum, ItemHelper::GetProduceQualityAttValueMax(pEquipProto->byMinUseLevel));
			
			GetItemMgr().GetBag().GetSameItemList(itemList, dwQualityItem, nMinNum);

			GetItemMgr().DelBagSameItem(itemList, nMinNum, elcid_compose_produce);

			INT nProbAddVip = GET_VIP_EXTVAL(GetTotalRecharge(), MAKE_EQUIP_PROB, INT) * nMinNum;

			eQuality = (EItemQuality)ItemCreator::GenEquipQlty(pEquipProto->dw_data_id, byQualityType, nProbAddVip + pProduceProto->nQualiyPro[byQualityType] * nMinNum, nSkillLevel);
			
			GetAchievementMgr().UpdateAchievementCriteria(ete_produce_equip_up_quality, dwQualityItem, nMinNum);
		}
	}
	else
	{
		eQuality = (EItemQuality)ItemCreator::GenEquipQlty(pEquipProto->dw_data_id, 0, 0, nSkillLevel);
	}
	
	// ����ͼֽ
	BOOL bRandAtt = TRUE;
	BOOL bEquipBind = FALSE;
	if (pProduceProto->e_form_from == EFormulaFrom_Item )
	{
		const tagItem* pPicItem = GetItemMgr().GetBagItem(n64ItemID);
		if (!VALID_POINT(pPicItem))
		{
			return E_Compose_Item_NotFind;
		}

		// ��ɫһ�����ɹ̶��ĸ�������
		eQuality = (EItemQuality)pPicItem->pProtoType->nSpecFuncVal2;
		if (eQuality < EIQ_Quality4)
		{
			bRandAtt = FALSE;
		}
		bEquipBind = pPicItem->byBind != EBS_Unbind && pPicItem->byBind != EBS_Unknown;
		GetItemMgr().DelFromBag(n64ItemID, (DWORD)elcid_compose_produce, 1);
	}

	if(VALID_POINT(pSkill))
	{
		DWORD	dwSkillExp = 1;//(DWORD)((FLOAT)pProduceProto->dw_master_incre * (1.0f + (FLOAT)GetAttValue(ERA_Savvy) / 100.0f));
		ChangeSkillExp(pSkill, dwSkillExp);
	}
	
	EItemCreateMode eCreateMod = EICM_Product;
	if (pProduceProto->e_form_from == EFormulaFrom_Item )
	{
		eCreateMod	= EICM_Product_picture;
	}

	tagItem *pItem = ItemCreator::Create(eCreateMod,  m_dwID, 
		pProduceProto->dw_pro_item_data_id, (INT16)(pProduceProto->dw_pro_quan_tity), bEquipBind, m_dwID);
	
	if(VALID_POINT(pItem) )
	{
		eQuality = ItemCreator::IdentifyEquip((tagEquip*)pItem, eQuality, bRandAtt);
		GetItemMgr().Add2Bag(pItem, elcid_compose_produce, TRUE);
	}
	

	if( VALID_POINT(m_pScript)&& VALID_POINT(pItem))
		m_pScript->OnProduceEquip( this, pItem->dw_data_id, ((tagEquip*)pItem)->equipSpec.byQuality, bFromBox);

	if(VALID_POINT(pItem) && ((tagEquip*)pItem)->equipSpec.byQuality>= EIQ_Quality4)
	{
		HearSayHelper::SendMessage(EHST_EQUIPPRODUCE,
			this->GetID( ), pItem->dw_data_id, INVALID_VALUE, INVALID_VALUE, INVALID_VALUE, pItem);
	}
	
	// �ɾ�ϵͳ
	GetAchievementMgr().UpdateAchievementCriteria(ete_composite_equip_success, eQuality, 1);
	return E_Compose_Consolidate_Success;
}

//---------------------------------------------------------------------------------
// �㻯,װ���ֽ�
//---------------------------------------------------------------------------------
DWORD Role::DeComposeItem(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, 
								INT64 n64IMID, INT64 n64Item, DWORD dw_cmd_id, INT nOutStuffIndex[], INT& nStuffNum1, BOOL bPetDec)
{
	// �ҵ��ϳɱ�����
	const s_decompose_proto_ser* pDeComposeProto = AttRes::GetInstance()->GetDeComposeProto(dwFormulaID);

	if(!VALID_POINT(pDeComposeProto))
		return E_Compose_Formula_Not_Exist;

	// �ҵ��������
	tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64Item); 
	if (!VALID_POINT(pEquip))
		return E_Compose_Stuff_Not_Enough;

	DWORD dwEquipTypeID = pEquip->dw_data_id;

	DWORD dw_error_code = E_Success;

	dw_error_code = CheckDeComposeLimit(dwNPCID, dwSkillID, n64ItemID, dwFormulaID, pDeComposeProto, n64Item, pEquip);
	

	// �����ֵô��ų���
	if (bPetDec && !VALID_POINT(GetPetPocket()->GetCalledPetSoul()))
	{
		return E_Decomposition_Pet_NOt;
	}

	if( dw_error_code != E_Success)
		return dw_error_code;

	// �㻯ֻ���ڻ�ɫ������װ�����ֽ�����Ʒ��
	//if(pDeComposeProto->e_com_type == ECOMT_EquipdDecompose && pEquip->equipSpec.byQuality == (BYTE)EIQ_Quality0)
	//	return E_Compose_Quality_Not_Match;

	// ��߼�ʱװ���ɵ㻯��ֽ�
	if(M_is_fashion(pEquip) /*|| pEquip->pEquipProto->eEquipPos == EEP_Face*/)
		return E_Compose_Not_Fashion;

	// npc��,��ʯ�󶨲��ɷֽ�
	if (pEquip->byBind == EBS_Bind)
		return E_Compose_Equip_Bind;

	// ��������װ�����ɵ㻯��ֽ�
	if(pEquip->IsLock() == TRUE)
		return E_Compose_Equip_Lock;

	// ��ʱ�����Ƶ�װ�����ɵ㻯��ֽ�
	if(pEquip->pProtoType->dwTimeLimit != INVALID_VALUE)
		return E_Compose_Equip_Time_Limit;

	// δ������װ�����ɵ㻯��ֽ�
	if(pEquip->equipSpec.byQuality == (BYTE)EIQ_Null)
		return E_Compose_Equip_Not_identify;

	// ��Ǯ����
	GetCurMgr().DecBagSilver(pDeComposeProto->dw_money_consume, elcid_compose_decompose);

	// ���Ӻϳɼ���������(ֻ�м��ܺϳɲŻ����Ӽ��������ȣ�
	Skill* pSkill = GetSkill(dwSkillID);

	INT nSkillLevel = 1;

	if(VALID_POINT(pSkill) && INVALID_VALUE == dwNPCID && INVALID_VALUE == n64ItemID)
	{
		//DWORD	dwSkillExp = (DWORD)((FLOAT)pDeComposeProto->dw_master_incre * (1.0f + (FLOAT)GetAttValue(ERA_Savvy) / 100.0f));
		//ChangeSkillExp(pSkill, dwSkillExp);

		nSkillLevel = pSkill->get_level();
	}

	

	//��ȡ�ֽ���Ʒ����
	INT nOutStuff = 0;
	if (nSkillLevel >=1 && nSkillLevel <=3)
	{
		nOutStuff = nSkillLevel + 2;
		nStuffNum1 = nOutStuff;
	}

	// �����������
	GetItemMgr().DelFromBag(n64Item, (DWORD)elcid_compose_decompose, (INT16)pEquip->n16Num);
	
	// 7������
	INT nSevenNum = 0; 
	// ����������
	INT nFileNum = 0;
	for(INT i = 0; i < nOutStuff; ++i)
	{
		if (pDeComposeProto->n_out_stuff_num == 0 )
			break;
		
		
		//! ���������
		int index = -3;

		INT32 nRate = 0;
		INT32 nRandPct = get_tool()->tool_rand() % 10000;
		for(INT32 j = 0; j < pDeComposeProto->n_out_stuff_num; j++)
		{
			nRate += pDeComposeProto->output_stuff[j].nRate;
			if(nRandPct < nRate)
			{
				index = j;
				break;
			}
		}
		
		//! ��û����,�����������"7"
		if (index == -3)
		{
			index = get_tool()->tool_rand() % 2 - 2;
		}
		// �Ӳ�ֲ��������һ��,����������7
		//int index = get_tool()->tool_rand() % (pDeComposeProto->n_out_stuff_num + 2);
		// ��ȥ2 ���-2 ~ n -2Ϊ 7 -1 Ϊ����

		//index = index - 2;
		nOutStuffIndex[i] = index;
		if (index == -2)
		{
			nSevenNum++;
			continue;
		}
		if (index == -1)
		{
			nFileNum++;
			continue;
		}


		INT nStuffNum = get_tool()->tool_rand() % (pDeComposeProto->output_stuff[index].nSucMaxVal + 1);

		if(nStuffNum < pDeComposeProto->output_stuff[index].nSucMinVal)
			nStuffNum = pDeComposeProto->output_stuff[index].nSucMinVal;

		tagItem *pItem = CreateItem(EICM_Product,  m_dwID, 
			pDeComposeProto->output_stuff[index].dwStuffTypeID, (INT16)nStuffNum, m_dwID);

		if( VALID_POINT(pItem) )
		{
			GetItemMgr().Add2Bag(pItem, elcid_compose_produce, TRUE);
		}
			
	}
	//���ȫ��'7'
	if (nSevenNum == nOutStuff && nOutStuff != 0)
	{

		INT nStuffNum = get_tool()->tool_rand() % (pDeComposeProto->out_per_stuff.nSucMaxVal + 1);

		if(nStuffNum < pDeComposeProto->out_per_stuff.nSucMinVal)
			nStuffNum = pDeComposeProto->out_per_stuff.nSucMinVal;

		tagItem *pItem = CreateItem(EICM_Product,  m_dwID, 
			pDeComposeProto->out_per_stuff.dwStuffTypeID, (INT16)nStuffNum, m_dwID);

		if( VALID_POINT(pItem) )
		{
			GetItemMgr().Add2Bag(pItem, elcid_compose_produce, TRUE);
			HearSayHelper::SendMessage(EHST_EQUIPCOMPOSE, GetID(), pItem->dw_data_id,  nStuffNum);

			GetAchievementMgr().UpdateAchievementCriteria(ete_decomposite_three_seven,1);
		}
	}
	if (nFileNum == nOutStuff && nOutStuff != 0)
	{
		GetAchievementMgr().UpdateAchievementCriteria(eta_decomposite_feil,1);
	}

	if( VALID_POINT(m_pScript))
		m_pScript->OnDeComposeItem( this, dwEquipTypeID );

	
	GetAchievementMgr().UpdateAchievementCriteria(eta_decomposite_equip,1);

	return E_Compose_Consolidate_Success;
	

}

//---------------------------------------------------------------------------------
// ��������ϳ�ʱ������
//---------------------------------------------------------------------------------
DWORD Role::CheckProduceLimit(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, 
								const s_produce_proto_ser* &pProduceProto, BOOL bBind, BYTE byQualityType, BYTE byQualityNum)
{
	Skill* pSkill = GetSkill(NPC_PRODUCE_SKILL);

	if(!VALID_POINT(pSkill))
		return E_Compose_Skill_Not_Exist;

	//// ��⼼�������Ƿ�ƥ��
	if(pSkill->GetTypeEx2() != ESSTE2_ProduceEquip)
		return E_Compose_FormulaNotMatch;

	// ��Ʒ�ϳɷ�ʽ
	if( pProduceProto->e_form_from == EFormulaFrom_Item || pProduceProto->e_form_from == EFormulaFrom_SkillorItem
		|| pProduceProto->e_form_from == EFormulaFrom_NPCorItem && INVALID_VALUE != n64ItemID)
	{
		// ��Ʒ�Ƿ��ڱ�����
		tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
		if (!VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EIST_Produce_Picture)
		{
			return E_Compose_Item_NotFind;
		}
		// ͼֽ���䷽ID����
		if (pItem->pProtoType->nSpecFuncVal1 != dwFormulaID)
		{
			return E_Compose_FormulaID_NotExit;
		}
	}
	// NPC�ϳɷ�ʽ
	else if(( pProduceProto->e_form_from == EFormulaFrom_NPC || pProduceProto->e_form_from == EFormulaFrom_SkillorNPC
		|| pProduceProto->e_form_from == EFormulaFrom_NPCorItem) && INVALID_VALUE != dwNPCID)
	{
		// �ô��챦��
		tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
		DWORD dwMoney = pProduceProto->dw_money_consume;
		if (VALID_POINT(pItem))
		{
			if (pItem->pProtoType->eSpecFunc != EIST_ProduceBox || 
				pItem->pProtoType->nSpecFuncVal2 != 0)
			{
				return E_Compose_Item_NotFind;
			}
			if (pItem->pProtoType->nSpecFuncVal1 > pSkill->get_level())
			{
				return E_Compose_Skill_Not_Same;
			}
			dwMoney = dwMoney * ComposeHelper::GeProduceBoxMoneyParam();
		}
		else // ���ô��챦��
		{
			Creature* pNPC = get_map()->find_creature(dwNPCID);
			if( !VALID_POINT(pNPC)) 
				return E_Compose_NPC_Not_Exist;

			if( FALSE == pNPC->CheckNPCTalkDistance(this) )
				return E_Compose_NPC_Distance;

			if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Compose) )
				return E_Compose_NPCCanNotCompose;
		}


		// �����ҽ�Ǯ�Ƿ��㹻
		if(GetCurMgr().GetBagSilver() < dwMoney)
			return E_Compose_NotEnough_Money;

		// ��Ҫ���NPC�Ƿ���кϳ��䷽
	}
	else
	{
		return E_Compose_Type_Not_Exist;
	}

	// ����ɫ��������				
	ItemMgr& itemMgr = GetItemMgr();	
	if (itemMgr.GetBagFreeSize() <= 0)
		return E_Compose_Bag_Full;

	// �������Ƿ��㹻
	for (int i = 0; i < MAX_PRODUCE_STUFF_QUANTITY; i++)
	{
		DWORD dwItemTypeID = pProduceProto->produce_stuff[i].dwStuffID;
		if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
			continue;

		DWORD dwItemNum = pProduceProto->produce_stuff[i].dwStuffNum;
		if (GetItemMgr().GetBag().GetSameItemCount(dwItemTypeID) < dwItemNum)
			return E_Compose_Stuff_Not_Enough;
	}

	const tagEquipProto* pEquipProto = AttRes::GetInstance()->GetEquipProto(pProduceProto->dw_pro_item_data_id);

	// ���ļ��������Ƿ��㹻
	if (byQualityType != -1)
	{
		DWORD dwQualityItem = ItemHelper::GetProduceQualityID(pEquipProto->byMinUseLevel, byQualityType);
		INT nMinNum = min(pProduceProto->nQualiyNum[byQualityType], byQualityNum);
		INT32 nNum = GetItemMgr().GetBag().GetSameItemCount(dwQualityItem);
		if (nNum < nMinNum)
			return E_Compose_UpQuality_Not_enough;
	}

	return E_Success;
}

//��������
DWORD Role::CheckProduceAlchemy(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, const s_produce_proto_ser* &pProduceProto, BOOL bBind)
{

	//Skill* pSkill = GetSkill(NPC_ALCHEMY_SKILL);

	//if(!VALID_POINT(pSkill))
	//	return E_Compose_Skill_Not_Exist;

	////// ��⼼�������Ƿ�ƥ��
	//if(pSkill->GetTypeEx2() != ESSTE2_DanTraining)
	//	return E_Compose_FormulaNotMatch;

	//INT32 vipLevel = GET_VIP_EXTVAL(GetTotalRecharge(), VIP_LEVEL, int);
	//float priceoff = GET_VIP_EXTVAL(GetTotalRecharge(), MAKE_EQUIP_PRICEOFF, float);

	// NPC�ϳɷ�ʽ
	//if(( pProduceProto->e_form_from == EFormulaFrom_NPC || pProduceProto->e_form_from == EFormulaFrom_SkillorNPC
	//	|| pProduceProto->e_form_from == EFormulaFrom_NPCorItem) )
	//{

		//DWORD dwMoney = pProduceProto->dw_money_consume;
		//if(!VALID_POINT(dwNPCID) && !VALID_POINT(n64ItemID)){
		//	if (vipLevel < VIP_MAKE_EQUIP_NONPC_LEVEL_MIN)
		//		return 	E_Compose_OutOf_Vip_Level;
		//} 
		//else 
		//{
			// �ô��챦��
			//tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
			//if (VALID_POINT(pItem))
			//{
			//	if (pItem->pProtoType->eSpecFunc != EIST_ProduceBox || 
			//		pItem->pProtoType->nSpecFuncVal2 != 1)
			//	{
			//		return E_Compose_Item_NotFind;
			//	}
			//	if (pItem->pProtoType->nSpecFuncVal1 > pSkill->get_level())
			//	{
			//		return E_Compose_Skill_Not_Same;
			//	}
			//	//dwMoney = dwMoney * ComposeHelper::GeProduceBoxMoneyParam();
			//}
			//else // ���ô��챦��
			//{
				Creature* pNPC = get_map()->find_creature(dwNPCID);
				if( !VALID_POINT(pNPC)) 
					return E_Compose_NPC_Not_Exist;

				if( FALSE == pNPC->CheckNPCTalkDistance(this) )
					return E_Compose_NPC_Distance;

				if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Compose) )
					return E_Compose_NPCCanNotCompose;
			//}
		//}


		//// �����ҽ�Ǯ�Ƿ��㹻
		//if(GetCurMgr().GetBagAllSilver() < dwMoney)
		//	return E_Compose_NotEnough_Money;

		// ��Ҫ���NPC�Ƿ���кϳ��䷽
	//}
	//else
	//{
	//	return E_Compose_Type_Not_Exist;
	//}

	// ����ɫ��������				
	ItemMgr& itemMgr = GetItemMgr();	
	if (itemMgr.GetBagFreeSize() <= 0)
		return E_Compose_Bag_Full;

	// �������Ƿ��㹻
	for (int i = 0; i < MAX_PRODUCE_STUFF_QUANTITY; i++)
	{
		DWORD dwItemTypeID = pProduceProto->produce_stuff[i].dwStuffID;
		if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
			continue;

		DWORD dwItemNum = pProduceProto->produce_stuff[i].dwStuffNum;
		if (GetItemMgr().GetBag().GetSameItemCount(dwItemTypeID) < dwItemNum)
			return E_Compose_Stuff_Not_Enough;
	}

	return E_Success;
}
//---------------------------------------------------------------------------------
// ��������ϳ�ʱ������
//---------------------------------------------------------------------------------
DWORD Role::CheckDeComposeLimit(DWORD dwNPCID, DWORD dwSkillID, INT64 n64ItemID, DWORD dwFormulaID, 
						 const s_decompose_proto_ser* &pDeComposeProto, INT64 n64Item, tagEquip *pEquip)
{
	// ���ܺϳɷ�ʽ
	if(( pDeComposeProto->e_form_from == EFormulaFrom_Skill || pDeComposeProto->e_form_from == EFormulaFrom_SkillorItem
		|| pDeComposeProto->e_form_from == EFormulaFrom_SkillorNPC) && INVALID_VALUE != dwSkillID)
	{
		Skill* pSkill = GetSkill(dwSkillID);

		if(!VALID_POINT(pSkill))
			return E_Compose_Skill_Not_Exist;

	//	// ��⼼�������Ƿ�ƥ��
		if(pSkill->GetTypeEx2() != ESSTE2_PointUp)
			return E_Compose_FormulaNotMatch;

	//	// ��⼼�ܵȼ����䷽�ȼ��Ƿ�ƥ��
		if(pSkill->get_level() < pDeComposeProto->n_form_level)
			return E_Compose_FormulaNotMatch;		
	}
	// ��Ʒ�ϳɷ�ʽ
	if(( pDeComposeProto->e_form_from == EFormulaFrom_NPC || pDeComposeProto->e_form_from == EFormulaFrom_SkillorNPC
			|| pDeComposeProto->e_form_from == EFormulaFrom_NPCorItem) && INVALID_VALUE != n64ItemID )
	{
		// ��Ʒ�Ƿ��ڱ�����

		// �Ƿ�Ϊ�ϳ�����Ʒ
	}
	// NPC�ϳɷ�ʽ
	else if( ( pDeComposeProto->e_form_from == EFormulaFrom_Item || pDeComposeProto->e_form_from == EFormulaFrom_SkillorItem
			|| pDeComposeProto->e_form_from == EFormulaFrom_NPCorItem) && INVALID_VALUE != dwNPCID )
	{
		Creature* pNPC = get_map()->find_creature(dwNPCID);
		if( !VALID_POINT(pNPC)) 
			return E_Compose_NPC_Not_Exist;

		if( FALSE == pNPC->CheckNPCTalkDistance(this) )
			return E_Compose_NPC_Distance;

		if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Compose) )
			return E_Compose_NPCCanNotCompose;

		// ��Ҫ���NPC�Ƿ���кϳ��䷽
	}

	// ����ɫ��������				
	ItemMgr& itemMgr = GetItemMgr();	
	if (itemMgr.GetBagFreeSize() <= 0)
		return E_Compose_Bag_Full;

	//// ��һ���ֵ�Ƿ��㹻
	//if(GetAttValue(ERA_Vitality) < (INT)pDeComposeProto->dwVitalityConsume)
	//	return E_Compose_Vitality_Inadequate;

	if(!VALID_POINT(pEquip))
		return E_Compose_Stuff_Not_Enough;

	// �����ҽ�Ǯ�Ƿ��㹻
	//if(GetCurMgr().GetBagSilver() < pDeComposeProto->dw_money_consume)
	//	return E_Compose_NotEnough_Money;

	// ���װ���ȼ��Ƿ�����
	if(((pEquip->pProtoType->byMinUseLevel-1) / 10 + 1)  != pDeComposeProto->by_level)
		return E_Compose_Equip_Level_Inadequate;

	// ���װ��Ʒ���Ƿ�����Ҫ��
	if(pEquip->equipSpec.byQuality != pDeComposeProto->by_quality)
		return E_Compose_Equip_Quality_Inadequate;

	// ������������Ƿ������䷽Ҫ��
	//if(pEquip->pProtoType->eType == EIT_Weapon && pEquip->pProtoType->eTypeEx != pDeComposeProto->eTypeEx)
	//	return E_Compose_Equip_Type_Inadequate;

	// ���װ���Ƿ������䷽Ҫ��
	//if((pEquip->pProtoType->eType == EIT_ClothArmor 
	//	|| pEquip->pProtoType->eType == EIT_Decoration 
	//	|| pEquip->pProtoType->eType == EIT_Armor) 
	//	&& pEquip->pEquipProto->eEquipPos != pDeComposeProto->ePos)

	//	return E_Compose_Equip_Type_Inadequate;

	return E_Success;
}

//---------------------------------------------------------------------------------
// ������������ת��Ϊ��������
//---------------------------------------------------------------------------------
EProduceType Role::Skill2ProduceType(ESkillTypeEx2 eSkillType)					 
{													 				 
	switch(eSkillType)								 
	{												 
	case ESSTE2_Smilt:					return EPCT_Smilt;							 
	case ESSTE2_Artisan:				return EPCT_Artisan;						 
	case ESSTE2_DanTraining:			return EPCT_DanTraining;					 
	case ESSTE2_Smith:					return EPCT_Smith;							 
	case ESSTE2_Casting:				return EPCT_Casting;						 
	case ESSTE2_Dressmaker:				return EPCT_Dressmaker;						 
	case ESSTE2_Aechnics:				return EPCT_Aechnics;
	default:							return EPCT_NULL;
	}												 
}

//---------------------------------------------------------------------------------
// ������Ʒ���Ƿ���Ҫ�������ߣ�
//---------------------------------------------------------------------------------
tagItem* Role::CreateItem(EItemCreateMode eCreateMode, DWORD dwCreateID, DWORD dw_data_id, INT16 n16Num, DWORD dwCreator, BOOL bBind)
{
	tagItemProto *pProto = AttRes::GetInstance()->GetItemProto(dw_data_id);
	if(!VALID_POINT(pProto))
		return (tagItem*)INVALID_VALUE;

	if(pProto->n16MaxLapNum > 1)
		return ItemCreator::Create(EICM_Product,  m_dwID, dw_data_id, n16Num, bBind);
	else
		return ItemCreator::Create(EICM_Product,  m_dwID, dw_data_id, n16Num, bBind, m_dwID);		
}

//---------------------------------------------------------------------------------
// ����IM��Ʒ��Ӱ��
//---------------------------------------------------------------------------------
VOID Role::CalIMEffect(E_consolidate_type_ser eConType, s_ime_effect &IMEffect, INT64 n64IMID, const LPVOID pProto, INT n_num)
{

	// �������Ƿ�������߳ɹ��ʵ�IM
	if( INVALID_VALUE == n64IMID)
		return;

	tagItem *pItemIM = GetItemMgr().GetBagItem(n64IMID);
	
	if(!VALID_POINT(pItemIM))
		return;

	//��������
	if (pItemIM->n16Num < n_num)
		return;

	switch (eConType)
	{
	case ects_inlay:
		{

		}
		break;
	//case ects_brand:
	//	{
	//		if(pItemIM->pProtoType->eSpecFunc == EISF_ProtectSign)
	//		{
	//			IMEffect.e_effect = eime_protectsign;
	//			// ɾ��IM����
	//			GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_null, 1);
	//		}
	//	}
	//	break;
	//case ects_fpsoul:
	//	{

	//	}
	//	break;
	case ects_produce:
		{
			const s_produce_proto_ser*  pProduceProto = (s_produce_proto_ser*)pProto;
			if(pItemIM->pProtoType->eSpecFunc == EISF_ComposeAdvance)
			{
				if(pItemIM->pProtoType->nSpecFuncVal1 == (INT)pProduceProto->e_com_type 
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)ESFCA_AllProduce
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_All)
				{
					IMEffect.e_effect = eime_comadvance;
					IMEffect.dw_param2 = pItemIM->pProtoType->nSpecFuncVal2;

					// ɾ��IM����
					GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_compose_produce, 1);
				}
				break;
			}
			else if (pItemIM->pProtoType->eSpecFunc == EISF_ColorProbability)
			{
				IMEffect.e_effect = eime_color;
				IMEffect.dw_param1 = pItemIM->pProtoType->nSpecFuncVal1;
				IMEffect.dw_param2 = pItemIM->pProtoType->nSpecFuncVal2;

				// ɾ��IM����
				GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_compose_produce, 1);
			}
		}
		break;
	case ects_decompose:
		{

		}
		break;

	case ects_hole:
		{
			if(pItemIM->pProtoType->eSpecFunc == EISF_ProtectSign)
			{
				IMEffect.e_effect = eime_protectsign;
				// ɾ��IM����
				GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_npc_hole, 1);
			}
		}
		break;
	case ects_unbeset:
		{
			if(pItemIM->pProtoType->eSpecFunc == EISF_ProtectSign)
			{
				IMEffect.e_effect = eime_protectsign;
				// ɾ��IM����
				GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_npc_unbeset, 1);
			}
		}
		break;
	case ects_shengxing:
		{
			if(pItemIM->pProtoType->eSpecFunc == EISF_ComposeAdvance)
			{
				/*if(pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_ShengXing 
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_AllConsolidate
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_All)*/
				{
					IMEffect.e_effect = eime_comadvance;
					n_num = n_num > 10 ? 10:n_num;
					//IMEffect.dw_param1 = pItemIM->pProtoType->nSpecFuncVal1;
					//IMEffect.dw_param2 = ItemHelper::GetUpstarsAddPro(n_num);
					// ɾ��IM����
					GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_npc_shengxin, n_num);
				}
			}
		}
		break;
	case ects_fusion:
		{
			if (pItemIM->pProtoType->eSpecFunc == EISF_ComposeAdvance)
			{
				if (pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_Fusion
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_AllConsolidate
					|| pItemIM->pProtoType->nSpecFuncVal1 == (INT)EISFC_All)
				{
					IMEffect.e_effect = eime_comadvance;
					IMEffect.dw_param2 = pItemIM->pProtoType->nSpecFuncVal2;
					GetItemMgr().DelFromBag(n64IMID, (DWORD)elcid_npc_fusion, 1);
				}
			}
		}
		break;
	}
}


//---------------------------------------------------------------------------------
// ����װ������
//---------------------------------------------------------------------------------
VOID Role::CalEquipFlare(tagEquip* pEquip)
{
	//BYTE byFlareVal = 0;
	//BYTE byHoleBNum = 0;
	//// ������Ƕ����
	//for(INT i = 0; i < (INT)pEquip->equipSpec.byHoleNum; ++i)
	//{
	//	byHoleBNum++;
	//	if(pEquip->equipSpec.dwHoleGemID[i] == INVALID_VALUE || pEquip->equipSpec.dwHoleGemID[i] == 0)
	//	{
	//		break;
	//	}
	//}

	//// ����ֵ��=  1�����ƴ��� + 1���Կ̴��� + 2����Ƕ���� + 2����ǰ��ӡ�ȼ� + 3��װ����ǰ�����ȼ�
	////byFlareVal = pEquip->equipSpec.byPosyTimes + pEquip->equipSpec.byEngraveTimes + byHoleBNum * 2 
	////			+ pEquip->equipSpec.byBrandLevel * 2 + pEquip->equipSpec.byLongfuLevel * 3;
	//if (pEquip->equipSpec.byConsolidateLevel == 10)
	//{
	//	byFlareVal = 100;
	//}
	//else if(pEquip->equipSpec.byConsolidateLevelStar >=7)
	//{
	//	byFlareVal = 70;
	//}
	//else if (pEquip->equipSpec.byConsolidateLevelStar >=4)
	//{
	//	byFlareVal = 40;
	//}
	//else
	//{
	//	byFlareVal = 0;
	//}

	//if (pEquip->pEquipProto->eEquipPos != EEP_RightHand)
	//{
	//	if (pEquip->equipSpec.byConsolidateLevelStar == 10)
	//	{
	//		byFlareVal = 100;
	//	}
	//	else
	//	{
	//		byFlareVal = 0;
	//	}
	//}
	//pEquip->SetFlareVal(byFlareVal);
}
//---------------------------------------------------------------------------------
//�����ں�
//---------------------------------------------------------------------------------
DWORD	Role::FusionEquip(INT64 n64Equip1, INT64 n64Equip2, INT64 n64ItemID)
{
	//if (IsInRoleState(ERS_Prictice))
	//	return INVALID_VALUE;

	//// �ҵ����ں�װ��1
	//tagEquip *pEquip1 = (tagEquip*)GetItem(n64Equip1);
	//if(!VALID_POINT(pEquip1))
	//	return E_Fusion_Equip1_Not_Exist;
	//
	//// �ҵ�Ҫ�ںϵ�װ��
	//tagEquip* pEquip2 = (tagEquip*)GetItemMgr().GetBagItem(n64Equip2);
	//if (!VALID_POINT(pEquip2))
	//	return E_Fusion_Equip2_Not_Exist ;
	//	
	////Ҫ�ںϵ���Ʒ��������
	//if (pEquip1->pEquipProto->eEquipPos != EEP_RightHand)
	//	return E_Fusion_NotEquipment1;
	//
	//if (pEquip2->pEquipProto->eEquipPos != EEP_RightHand)
	//	return E_Fusion_NotEquipment2;
	//
	//// �츳���ͱ�����ͬ
	//if (pEquip1->pEquipProto->eTalentType != pEquip2->pEquipProto->eTalentType)
	//	return E_Fusion_Talent_Not_Exist;
	//
	//// ְҵ����
	//if (!(( 1 << m_eClass ) & pEquip2->pEquipProto->dwVocationLimitWear))
	//	return E_Fusion_Class_Not_Exist;

	//// �ȼ�����
	//if(m_nLevel < pEquip2->pEquipProto->byMinUseLevel 
	//	|| m_nLevel > pEquip2->pEquipProto->byMaxUseLevel)
	//{
	//	return E_Fusion_Level_low;
	//}


	//INT32 n32Equip1Level = pEquip1->equipSpec.nLevel;
	//INT32 n32Equip1Exp = pEquip1->equipSpec.nCurLevelExp;

	//if (n32Equip1Level < 2)
	//	return E_Fusion_Equip1Level_low;
	//
	//INT32 n32Equip2Level = pEquip2->equipSpec.nLevel;
	////�ںϺ�ȼ�������

	//pEquip2->equipSpec.nLevel = n32Equip1Level-1;
	//
	////��Ԫ��
	//tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
	//if (VALID_POINT(pItem) && pItem->pProtoType->eSpecFunc == EIST_WeaponFusion)
	//{
	//	pEquip2->equipSpec.nLevel = n32Equip1Level;

	//	GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, pItem->pProtoType->dw_data_id, 1);

	//	GetItemMgr().DelFromBag(n64ItemID, elcid_compose_fusion, 1);
	//}

	//if (pEquip2->equipSpec.nLevel > pEquip2->pEquipProto->byMaxLevel)
	//{
	//	pEquip2->equipSpec.nLevel = pEquip2->pEquipProto->byMaxLevel;
	//	pEquip2->equipSpec.nCurLevelExp = 0;
	//}
	////û������,���������ĵȼ� = ��ǰ�ȼ� +���������ȼ� - 1 ���鰴�ٷֱ�����
	//else
	//{
	//	const tagEquipLevelUpEffect *pEquip1LevelUp = AttRes::GetInstance()->GetEquipLevelUpEffect(n32Equip1Level);
	//	if (!VALID_POINT(pEquip1LevelUp))
	//		return E_Fusion_EquiplevelUp_NotFind;
	//	
	//	const tagEquipLevelUpEffect* pEquip2LevelUp = AttRes::GetInstance()->GetEquipLevelUpEffect(pEquip2->equipSpec.nLevel);
	//	if (!VALID_POINT(pEquip2LevelUp))
	//		return E_Fusion_EquiplevelUp_NotFind;

	//	FLOAT fParam = n32Equip1Exp*1.0f / pEquip1LevelUp->nExpLevelUp;
	//	INT32 n32Equip2Exp = fParam * pEquip2LevelUp->nExpLevelUp;

	//	pEquip2->equipSpec.nCurLevelExp = n32Equip2Exp;
	//}
	//INT16 n16AddTalent = tagEquip::GetAddTalent(n32Equip2Level, pEquip2->equipSpec.nLevel);
	//pEquip2->equipSpec.byMaxTalentPoint += n16AddTalent;

	//GetItemMgr().UpdateEquipSpec(*pEquip2);

	//// ��Ŀ������װ����
	//Equip(n64Equip2, EEP_RightHand);

	////ɾ��������
	//GetItemMgr().DelFromBag(n64Equip1, elcid_compose_fusion, 1);
	//
	//// �ݶ�����10������ֵ
	////ChangeBrotherhood(-10);

	//if( VALID_POINT(m_pScript))
	//	m_pScript->OnWeaponFusion( this );

	//GetAchievementMgr().UpdateAchievementCriteria(ete_fusion_equip, 1);

	return E_Success;

}
//---------------------------------------------------------------------------------
//��������
//---------------------------------------------------------------------------------
DWORD	Role::PurificationEquip(DWORD dwNPCID, INT64 n64ItemID, INT64 n64StuffID)
{
	return E_Success;
}
//---------------------------------------------------------------------------------
//��������
//---------------------------------------------------------------------------------
DWORD	Role::PracticeBegin(INT64 n64Equip, DWORD* dwItem, DWORD* dwItemNum)
{
	//�ҵ�����������
	//tagEquip* pEquip = GetItemMgr().GetEquipBarEquip(n64Equip);

	//����������
	//if (!VALID_POINT(pEquip))
	//	return E_Prictice_NotFind_Equip;
	
	//��������
	//if (!M_is_weapon(pEquip))
	//	return E_Prictice_Not_Weapon;
	
	if (!m_PracticeMgr.IsInCanPracticeState())
		return E_Prictice_Not_State;

	package_list<tagItem*> itemList;

	for (int i = 0; i < MAX_XIULIAN_ITEM_NUMBER; i++)
	{
		if (dwItem[i] != INVALID_VALUE && dwItem[i] != 0)
		{
			const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwItem[i]);
			if (!VALID_POINT(pItemProto))
				return E_Prictice_Speed_item_Error;


			if(pItemProto->byMinUseLevel > get_level() || pItemProto->byMaxUseLevel < get_level())
				return E_Prictice_Speed_item_level_Error;


			INT32 nNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItem[i], dwItemNum[i]);

			if (nNum < dwItemNum[i])
				return E_Prictice_Speed_item_Error;
		}
	}

	m_PracticeMgr.SetPracticeData(n64Equip, itemList);
	m_PracticeMgr.BeginPractice();

	if( VALID_POINT( GetScript( ) ) )
		GetScript( )->OnGetWuhuen( this );

	return E_Success;
}
//---------------------------------------------------------------------------------
//��������
//---------------------------------------------------------------------------------
DWORD Role::PracticeEnd()
{
	if (IsInRoleState(ERS_Prictice))
	{
		m_PracticeMgr.EndPractice(E_Success);
	}
	
	return E_Success;
}
// gm����
DWORD Role::GMPractice(INT16 pos, INT level)
{
	//// �ҵ���ǿ��װ��
	//tagEquip *pEquip = (tagEquip*)GetItemMgr().GetBagItem(pos);
	//if(!VALID_POINT(pEquip))
	//	return E_Consolidate_Equip_Not_Exist;

	////����װ��
	//if(!MIsEquipment(pEquip->dw_data_id))
	//	return E_Consolidate_NotEquipment;
	//
	////��������
	//if (!M_is_weapon(pEquip))
	//	return E_Prictice_Not_Weapon;

	//INT nLevel = level;
	////װ���Ѿ������������pEquipProto->byMaxLevel
	//if(nLevel >= pEquip->pEquipProto->byMaxLevel)
	//{
	//	nLevel = pEquip->pEquipProto->byMaxLevel;
	//}

	//pEquip->LevelChange(nLevel);
	//pEquip->equipSpec.byMaxTalentPoint = 50;

	//GetItemMgr().UpdateEquipSpec(*pEquip);
	//
	//GetAchievementMgr().UpdateAchievementCriteria(ete_xiulian, pEquip->equipSpec.nLevel);

	return E_Success;
}
//---------------------------------------------------------------------------------
// ����ѧ����
//---------------------------------------------------------------------------------
DWORD Role::WeaponLearnSkill(INT64 n64Equip, DWORD dwSkillID)
{
	//DWORD dwSkillTypeID = Skill::CreateTypeID(dwSkillID, 1);

	//tagEquip* pEquip = GetItemMgr().GetEquipBarEquip(n64Equip);
	////����������
	//if (!VALID_POINT(pEquip))
	//	return E_Weapon_LearnSkill_NotFind;
	////��������
	//if (!M_is_weapon(pEquip))
	//	return E_Weapon_LearnSKill_NotWeapon;
	//
	////����ûװ����
	//if (pEquip != GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand))
	//	return E_Weapon_LearnSkill_NotEquip;

	////���ܲ�����
	//const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	//if (!VALID_POINT(pSkillProto))
	//	return E_Weapon_LearnSKill_NotSkill;
	//
	////װ�������иü���
	////if (pEquip->HasSkill(dwSkillID))
	////	return E_Weapon_LearnSkill_Existed;

	////����츳����
	//if( ETT_Null == pSkillProto->eTalentType )
	//	return E_Weapon_LearnSkill_TalentNot;

	//if (pEquip->pEquipProto->eTalentType != pSkillProto->eTalentType)
	//	return E_Weapon_LearnSkill_TalentNot;
	//
	////�����Ǳ�������
	//if (ESUT_Passive != pSkillProto->eUseType)
	//	return E_Weapon_LearnSkill_MustPassive;
	//
	//// �������ȼ�
	//if( m_nLevel < pSkillProto->nNeedRoleLevel )
	//	return E_Weapon_LearnSkill_NeedMoreLevel;

	//// ���ǰ�ü���
	//if( INVALID_VALUE != pSkillProto->dwPreLevelSkillID )
	//{
	//	DWORD dwPreSkillID = Skill::GetIDFromTypeID(pSkillProto->dwPreLevelSkillID);
	//	INT nPreSkillLevel = Skill::GetLevelFromTypeID(pSkillProto->dwPreLevelSkillID);
	//	
	//	BOOL bHasSkill = pEquip->HasSkill(dwPreSkillID);

	//	if (!bHasSkill || pEquip->GetSkillLevel(dwPreSkillID) < nPreSkillLevel)
	//		return E_Weapon_LearnSkill_NoPreSkill;
	//}
	//// �����ѵ��츳�㲻��
	//if ( pEquip->equipSpec.byTalentPoint < pSkillProto->nNeedTalentPoint)
	//	return E_Weapon_LearnSkill_NeedMoreTalentPoint;
	//
	//BYTE byMaxPinnt = pEquip->equipSpec.byMaxTalentPoint;
	//if (pEquip->equipSpec.byAddTalentPoint > 0)
	//{
	//	byMaxPinnt += pEquip->equipSpec.byAddTalentPoint;
	//}
	//// ʣ���츳�㲻��
	//if ( pEquip->equipSpec.byTalentPoint >= byMaxPinnt )
	//	return E_Weapon_LearnSkill_NoTalentPoint;
	//
	//// ������иü���
	//Skill* pSkill = GetSkill(dwSkillID);
	//if( VALID_POINT(pSkill) ) return E_Weapon_LearnSkill_RoleExisted;

	//// ��װ����Ӽ���
	//if (!pEquip->AddSkill(dwSkillTypeID))
	//	return E_Weapon_LearnSkill_FullSkillList;

	//// ������Ͷ�츳��
	//pEquip->equipSpec.byTalentPoint++;

	//GetItemMgr().UpdateEquipSpec(*pEquip);

	//// ����ɫ���ϼ���
	//pSkill = new Skill(dwSkillID, 0, 1, 0, 0, TRUE);
	//AddSkill(pSkill);
	//
	//// ���¼�������
	//if( NeedRecalAtt() )
	//	RecalAtt();

	return E_Success;
}

//---------------------------------------------------------------------------------
// ������������
//---------------------------------------------------------------------------------
DWORD Role::WeaponLevelUpSkill(INT64 n64Equip, DWORD dwSkillID)
{
	
	//tagEquip* pEquip = GetItemMgr().GetEquipBarEquip(n64Equip);
	////����������
	//if (!VALID_POINT(pEquip))
	//	return E_Weapon_LearnSkill_NotFind;
	////��������
	//if (!M_is_weapon(pEquip))
	//	return E_Weapon_LearnSKill_NotWeapon;

	////����ûװ����
	//if (pEquip != GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand))
	//	return E_Weapon_LearnSkill_NotEquip;

	////װ����û�иü���
	////if (!pEquip->HasSkill(dwSkillID))
	////	return E_Weapon_LevelUpSkill_NotExisted;
	//
	//INT nNextLevel = pEquip->GetSkillLevel(dwSkillID) + 1;
	//DWORD dwSkillTypeID = Skill::CreateTypeID(dwSkillID, nNextLevel);

	////���ܲ�����
	//const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
	//if (!VALID_POINT(pSkillProto))
	//	return E_Weapon_LearnSKill_NotSkill;

	////����츳����
	//if( ETT_Null == pSkillProto->eTalentType )
	//	return E_Weapon_LearnSkill_TalentNot;

	//if (pEquip->pEquipProto->eTalentType != pSkillProto->eTalentType)
	//	return E_Weapon_LearnSkill_TalentNot;

	////�����Ǳ�������
	//if (ESUT_Passive != pSkillProto->eUseType)
	//	return E_Weapon_LearnSkill_MustPassive;
	//
	//// �������ȼ�
	//if( m_nLevel < pSkillProto->nNeedRoleLevel )
	//	return E_Weapon_LearnSkill_NeedMoreLevel;

	//// ���ǰ�ü���
	//if( INVALID_VALUE != pSkillProto->dwPreLevelSkillID )
	//{
	//	DWORD dwPreSkillID = Skill::GetIDFromTypeID(pSkillProto->dwPreLevelSkillID);
	//	INT nPreSkillLevel = Skill::GetLevelFromTypeID(pSkillProto->dwPreLevelSkillID);

	//	BOOL bHasSkill = pEquip->HasSkill(dwPreSkillID);

	//	if (!bHasSkill || pEquip->GetSkillLevel(dwPreSkillID) < nPreSkillLevel)
	//		return E_Weapon_LearnSkill_NoPreSkill;
	//}
	//// �����ѵ��츳�㲻��
	//if ( pEquip->equipSpec.byTalentPoint < pSkillProto->nNeedTalentPoint)
	//	return E_Weapon_LearnSkill_NeedMoreTalentPoint;


	//BYTE byMaxPinnt = pEquip->equipSpec.byMaxTalentPoint;
	//if (pEquip->equipSpec.byAddTalentPoint > 0)
	//{
	//	byMaxPinnt += pEquip->equipSpec.byAddTalentPoint;
	//}

	//// ʣ���츳�㲻��
	//if ( byMaxPinnt - pEquip->equipSpec.byTalentPoint <=0 )
	//	return E_Weapon_LearnSkill_NoTalentPoint;

	//// ���û�иü���
	//Skill* pSkill = GetSkill(dwSkillID);
	//if( !VALID_POINT(pSkill) ) return E_Weapon_LevelUpSkill_RoleNoExisted;

	//// ��װ����������
	//if (!pEquip->LevelUpSkill(dwSkillID))
	//	return E_Weapon_LearnSkill_FullSkillList;

	//// ������Ͷ�츳��
	//pEquip->equipSpec.byTalentPoint++;

	//GetItemMgr().UpdateEquipSpec(*pEquip);


	//ChangeSkillLevel(pSkill, ESLC_Learn);

	//// ���¼�������
	//if( NeedRecalAtt() )
	//	RecalAtt();

	return E_Success;
}

//---------------------------------------------------------------------------------
// ����ϴ���ܵ�
//---------------------------------------------------------------------------------
DWORD Role::WeaponClearTalent(INT64 n64ItemID)
{
	//tagEquip* pEquip = (tagEquip*)GetItem(n64Equip);
	//tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	////����������
	//if (!VALID_POINT(pEquip))
	//	return E_Weapon_ClearTalent_NoEquip;

	////��������
	//if (!M_is_weapon(pEquip))
	//	return E_Weapon_ClearTalent_NotWeapon;
	//
	////����Ҫϴ��
	//if (pEquip->equipSpec.byTalentPoint == 0)
	//	return E_Weapon_Clear_Not_clear;

	//// �ж���Ʒ�����ǲ���ϴ�����
	//tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
	//if( !VALID_POINT(pItem) || pItem->pProtoType->eSpecFunc != EISF_RemoveTalentPt )
	//	return E_ClearTalent_ItemNotValid;

	//// �ж�ͨ����ɾ����Ʒ
	//GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_clear_talent);
	//
	//// ɾ����ɫ���ϵļ���
	//for (int i = 0; i < MAX_EQUIP_SKILL_NUM; i++)
	//{
	//	if (pEquip->equipSpec.dwSkillList[i] != 0)
	//	{
	//		RemoveSkill(pEquip->equipSpec.dwSkillList[i]/100);
	//		pEquip->equipSpec.dwSkillList[i] = 0;
	//	}
	//}

	////pEquip->ClearSkill();
	//// �����Ͷ�츳��
	//pEquip->equipSpec.byTalentPoint = 0;
	//pEquip->equipSpec.byAddTalentPoint = 0;

	//
	//if (pEquip->eConType == EICT_Equip)
	//{
	//	// ��ɾ��ԭ����
	//	ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[7], 1, -1);
	//	RecalAtt(TRUE);
	//}

	//// �������������
	//if (pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt != ERA_Null)
	//{
	//	pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
	//	pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;
	//}
	//
	//GetItemMgr().UpdateEquipSpec(*pEquip);
	//RecalAtt(TRUE);
	//
	//GetAchievementMgr().UpdateAchievementCriteria(eta_equip_clear_talent, 1);
	return E_Success;
}

// ������������
DWORD Role::SetLeaveParcitice(BYTE	byTimeType, BYTE	byMulType)
{
	if(is_leave_pricitice())
		return E_Success;

	if(get_level() < LEAVE_PRICTICE_LEVEL)
		return E_Role_LeavePricitice_level_limit;

	if(byTimeType < 1 || byTimeType > 4)
		return E_Role_LeavePricitice_TimeType_Not;

	if(byMulType < 1 || byMulType > 5)
		return E_Role_LeavePricitice_MulType_Not;

	if(byMulType == 4 && get_level() < 40)
		return E_Role_LeavePricitice_level_limit;

	if(byMulType == 5 && get_level() < 60)
		return E_Role_LeavePricitice_level_limit;

	INT nLove = m_PracticeMgr.GetLeavePraciticeLoveFromType(byMulType);

	if(nLove > GetAttValue(ERA_Love))
		return E_Role_LeavePricitice_love_limit;

	if(!IsInRoleState(ERS_Prictice))
		return E_Role_No_Prictice_State;

	if(!IsInRoleStateAny(ERS_RealSafeArea | ERS_Carry))
		return E_Role_LeavePrictice_Area_Limit;

	if(HasPKValue())
		return E_Role_LeavePricitice_RedName_limit;

	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return INVALID_VALUE;

	if(pMap->get_map_type() != EMT_Normal)
		return E_Role_LeavePrictice_Area_Limit;

	set_leave_prictice(TRUE);
	m_PracticeMgr.SetLeavePracticeTime(byTimeType);
	m_PracticeMgr.SetLeavePraciticeMul(byMulType);

	ModAttValue(ERA_Love, -nLove, FALSE);
	SaveLove2DB();
	m_PracticeMgr.SendLeavePraciticeLog();
	m_PracticeMgr.SetAddLoveTotal(nLove);

	g_pvp_mgr.role_offline(this);
	
	// �뿪����
	g_groupMgr.AddEvent(GetID(), EVT_LeaveTeam, 0, NULL);
	
	// ��������ʧ��buff�����ͻ���
	for(INT n = 0; n < MAX_BUFF_NUM; ++n)
	{
		if( !m_Buff[n].IsValid() ) continue;
		
		if (m_Buff[n].Interrupt(EBIF_OffLine))
		{
			RemoveBuff(&m_Buff[n], FALSE);
		}
	}
	return E_Success;
}

//------------------------------------------------------------------------------
// װ����,���
//------------------------------------------------------------------------------
DWORD Role::EquipBind(INT64 n64SerialEquip, INT64 n64SerialItem, DWORD dwNPCID, EBindStatus& eBindType)
{

	// �õ��߰�
	if (dwNPCID == 0)
	{
		// ����װ��
		if (!MIsEquipment(n64SerialEquip))
		{
			return E_Equip_Bind_NotFind;
		}

		tagEquip* pEquip = (tagEquip*)GetItem(n64SerialEquip);
		// װ��������
		if (!VALID_POINT(pEquip))
		{
			return E_Equip_Bind_NotFind;
		}
		
		if (MIsFashion(pEquip->pEquipProto->eEquipPos))
		{
			return E_Equip_Bind_IS_Fashion;
		}

		tagItem* pItem = GetItemMgr().GetBagItem(n64SerialItem);
		if (!VALID_POINT(pItem)) 
			return E_Equip_Bind_ItemNotFind;
	
		if (pItem->pProtoType->eSpecFunc != EIST_EquipBind) 
			return E_Equip_Bind_ItemTypeError;

		// ɾ����ʱ��
		if (pEquip->dwBindTime != 0)
		{
			if (pEquip->eConType == EICT_Bag)
			{
			GetItemMgr().GetBag().GetTimeLimitMgr().RemoveFromTimeLeftList(pEquip->n64_serial);
			}
			else if (pEquip->eConType == EICT_Equip)
			{
			GetItemMgr().GetEquipBar().GetTimeLimitMgr().RemoveFromTimeLeftList(pEquip->n64_serial);
			}
			pEquip->SetBindTime(0);
		}

		// ��װ��
		pEquip->SetBind(EBS_Bind);
		eBindType = EBS_Bind;
		NET_SIS_bind_change send;
		send.n64EquipSerial = n64SerialEquip;
		send.byBind = pEquip->byBind;
		send.time = 0;
		send.dwConType = pEquip->eConType;
		SendMessage(&send, send.dw_size);

		if (pEquip->eConType == EICT_Equip)
		{
			// ��ɾ��ԭ����
			ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
		}

		// ��������һ���������
		//ItemCreator::CreateEquipBindAtt(pEquip, pItem->pProtoType->nSpecFuncVal2/10000);
		{
			// �õ�ָ��Ʒ��װ�����Բ���
			const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
			if(!VALID_POINT(pEquipQltyEffect))
			{
				ASSERT(VALID_POINT(pEquipQltyEffect));
				
			}

			const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
			if(!VALID_POINT(pEquipLevelPctProto))
			{
				ASSERT(VALID_POINT(pEquipLevelPctProto));
				
			}
			
			if (VALID_POINT(pEquipQltyEffect) && VALID_POINT(pEquipLevelPctProto))
			{
				FLOAT fPct = pEquipLevelPctProto->fLevelPct /** pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos]*/;
				FLOAT fParam = pItem->pProtoType->nSpecFuncVal2/10000.0;
				pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = (ERoleAttribute)pItem->pProtoType->nSpecFuncVal1;
				EquipAddAtt eProtoRoleAtt = ItemCreator::ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt);
				pEquip->equipSpec.EquipAttitionalAtt[5].nValue = (INT32)(pEquipQltyEffect->EquipAddAtt[eProtoRoleAtt]*fPct*fParam);
		
			}
		}

		
		//pEquip->equipSpec.EquipAttitionalAtt[5].nValue = pItem->pProtoType->nSpecFuncVal2;

		if (pEquip->eConType == EICT_Equip)
		{
			ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, 1);
			RecalAtt(TRUE);
			//ProcEquipEffectAvatar(pEquip, pEquip->pEquipProto->eEquipPos);
		}


		GetItemMgr().UpdateEquipSpec(*pEquip);
		// �ж�ͨ����ɾ����Ʒ
		GetItemMgr().ItemUsedFromBag(n64SerialItem, (INT16)1, (DWORD)elcid_compose_engrave);


		GetAchievementMgr().UpdateAchievementCriteria(eta_bind_equip, 1);
	}
	// NPC��
	else if (n64SerialItem == INVALID_VALUE)
	{
		
		Creature* pNPC = get_map()->find_creature(dwNPCID);
		if( !VALID_POINT(pNPC)) 
			return E_Equip_Bind_NPC_Error;

		// ����������װ��
		if(n64SerialEquip == INVALID_VALUE)
		{
			for (int i = 0; i < EEP_MaxEquip; i++)
			{
				tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)i);
				if (!VALID_POINT(pEquip)) continue;

				//��ʱ�󶨵��޷�npc��
				if (pEquip->byBind == EBS_Bind)
					continue;
				
				// ��ʱ�����Ƶ�װ��
				if(pEquip->pProtoType->dwTimeLimit != INVALID_VALUE)
					continue;

				if (MIsFashion(pEquip->pEquipProto->eEquipPos))
					continue;

				// ��װ��
				pEquip->SetBind(EBS_Bind);
				eBindType = EBS_Bind;

				pEquip->SetBindTime(g_world.GetWorldTime());

				GetItemMgr().GetEquipBar().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
				
				NET_SIS_bind_change send;
				send.n64EquipSerial = pEquip->n64_serial;
				send.byBind = pEquip->byBind;
				send.time = pEquip->dwBindTime;
				send.dwConType = pEquip->eConType;
				SendMessage(&send, send.dw_size);
			
				if (pEquip->eConType == EICT_Equip)
				{
					// ��ɾ��ԭ����
					ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
				}

				// ��������һ���������
				//ItemCreator::CreateEquipBindAtt(pEquip, 0.5);
				
				// ��������
				// �õ�ָ��Ʒ��װ�����Բ���
				{
					const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
					if(!VALID_POINT(pEquipQltyEffect))
					{
						ASSERT(VALID_POINT(pEquipQltyEffect));
						return E_Compose_Formula_Not_Exist;
					}

					const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
					if(!VALID_POINT(pEquipLevelPctProto))
					{
						ASSERT(VALID_POINT(pEquipLevelPctProto));
						return E_Compose_Formula_Not_Exist;
					}
					INT16 n16Rand = 0;
					FLOAT fPct = pEquipLevelPctProto->fLevelPct /** pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos]*/;
					
					pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = ERA_Physique;
					EquipAddAtt eProtoRoleAtt = ItemCreator::ERA2EAA(pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt);
					pEquip->equipSpec.EquipAttitionalAtt[5].nValue = (INT32)(pEquipQltyEffect->EquipAddAtt[eProtoRoleAtt]*fPct*0.5f + 0.5f);

					if ( pEquip->equipSpec.EquipAttitionalAtt[5].nValue <= 0 )
					{
						pEquip->equipSpec.EquipAttitionalAtt[5].eRoleAtt = ERA_Null;
					}
				}

				if (pEquip->eConType == EICT_Equip)
				{
					ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, 1);
					RecalAtt(TRUE);
				}

				GetItemMgr().UpdateEquipSpec(*pEquip);
				
				

			}
			GetAchievementMgr().UpdateAchievementCriteria(eta_unbind_equip_npc_all, 1);
		}
		// �󶨵���װ��
		else
		{
			// ����װ��
			if (!MIsEquipment(n64SerialEquip))
			{
				return E_Equip_Bind_NotFind;
			}

			tagEquip* pEquip = (tagEquip*)GetItem(n64SerialEquip);
			// װ��������
			if (!VALID_POINT(pEquip))
			{
				return E_Equip_Bind_NotFind;
			}
			
			if (MIsFashion(pEquip->pEquipProto->eEquipPos))
			{
				return E_Equip_Bind_IS_Fashion;
			}

			//��ʱ�󶨵��޷�npc��
			if (pEquip->byBind == EBS_Bind)
				return E_Equip_Bind_Not_bind;
			
			// ��ʱ�����Ƶ�װ��
			if(pEquip->pProtoType->dwTimeLimit != INVALID_VALUE)
				return E_Equip_Bind_Not_bind;


			// ��װ��
			pEquip->SetBind(EBS_Bind);
			eBindType = EBS_Bind;
			pEquip->SetBindTime(g_world.GetWorldTime());
			
			
			if (pEquip->eConType == EICT_Bag)
			{
				GetItemMgr().GetBag().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
			}
			else if (pEquip->eConType == EICT_Equip)
			{
				GetItemMgr().GetEquipBar().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
			}

			NET_SIS_bind_change send;
			send.n64EquipSerial = n64SerialEquip;
			send.byBind = pEquip->byBind;
			send.time = pEquip->dwBindTime;
			send.dwConType = pEquip->eConType;
			SendMessage(&send, send.dw_size);

			if (pEquip->eConType == EICT_Equip)
			{
				// ��ɾ��ԭ����
				ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
			}
			// ��������һ���������
			ItemCreator::CreateEquipBindAtt(pEquip, 0.5);
			
			if (pEquip->eConType == EICT_Equip)
			{
				ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, 1);
				RecalAtt(TRUE);
			}

			GetItemMgr().UpdateEquipSpec(*pEquip);

			GetAchievementMgr().UpdateAchievementCriteria(eta_unbind_equip_npc, 1);
		}

	}

	if(VALID_POINT(GetScript()))
		GetScript()->OnBindEquip(this);

	return E_Success;
}

DWORD Role::EquipUnBind(INT64 n64SerialEquip, INT64 n64SerialItem)
{
	// ����װ��
	if (!MIsEquipment(n64SerialEquip))
	{
		return E_Equip_Bind_NotFind;
	}

	tagEquip* pEquip = (tagEquip*)GetItem(n64SerialEquip);
	// װ��������
	if (!VALID_POINT(pEquip))
	{
		return E_Equip_Bind_NotFind;
	}
	
	tagItem* pItem = GetItemMgr().GetBagItem(n64SerialItem);
	if (!VALID_POINT(pItem)) 
		return E_Equip_Bind_ItemNotFind;

	if (pItem->pProtoType->eSpecFunc != EIST_EquipUnBind) 
		return E_Equip_Bind_ItemTypeError;
	
	// װ�����
	if (pItem->pProtoType->nSpecFuncVal1 == 0)
	{
		if (MEquipIsRide(pEquip->pEquipProto))
		{
			return E_Equip_Bind_ItemTypeError;
		}

		if (pEquip->byBind == EBS_Bind && pEquip->dwBindTime != 0)
		{
			return E_Equip_UnBind_Not_UnBind;
		}

		// ϵͳ�󶨲����Խ��
		if (pEquip->byBind == EBS_Bind )
		{
			return E_Equip_UnBind_SystemBind;
		}

		//if (pEquip->equipSpec.nCurLevelExp > 0 || pEquip->equipSpec.nLevel > 1)
		//{
		//	return E_Equip_UnBind_IS_Level;
		//}

		pEquip->SetBindTime(g_world.GetWorldTime());


		if (pEquip->eConType == EICT_Bag)
		{
			GetItemMgr().GetBag().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
		}
		else if (pEquip->eConType == EICT_Equip)
		{
			GetItemMgr().GetEquipBar().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
		}
	}

	// ������
	else if(pItem->pProtoType->nSpecFuncVal1 == 1)
	{
		if (!MEquipIsRide(pEquip->pEquipProto))
		{
			return E_Equip_Bind_ItemTypeError;
		}
		
		if (pEquip->byBind == EBS_Unbind || pEquip->dwBindTime != 0)
		{
			return E_Equip_UnBind_Not_UnBind;
		}

		if (pEquip->byBind != EBS_Bind)
		{
			return E_Equip_UnBind_Not_UnBind;
		}

		pEquip->SetBindTime(g_world.GetWorldTime());


		if (pEquip->eConType == EICT_Bag)
		{
			GetItemMgr().GetBag().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
		}
		else if (pEquip->eConType == EICT_Equip)
		{
			GetItemMgr().GetEquipBar().GetTimeLimitMgr().Add2TimeLeftList(pEquip->n64_serial, NPC_BIND_TIME, pEquip->dwBindTime, 1);
		}
	}

	// ɾ����ʱ��
	/*if (pEquip->byBind == EBS_NPC_Bind)
	{
		if (pEquip->eConType == EICT_Bag)
		{
			GetItemMgr().GetBag().GetTimeLimitMgr().RemoveFromTimeLeftList(pEquip->n64_serial);
		}
		else if (pEquip->eConType == EICT_Equip)
		{
			GetItemMgr().GetEquipBar().GetTimeLimitMgr().RemoveFromTimeLeftList(pEquip->n64_serial);
		}
	}*/
	
	// �����װ��
	//pEquip->SetBind(pEquip->bCreateBind?EBS_SYSTEM_Bind:EBS_Unbind);
	
	NET_SIS_bind_change send;
	send.n64EquipSerial = n64SerialEquip;
	send.byBind = pEquip->byBind;
	send.time = pEquip->dwBindTime;
	send.dwConType = pEquip->eConType;
	SendMessage(&send, send.dw_size);

	//if (pEquip->eConType == EICT_Equip)
	//{
	//	ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[5], 1, -1);
	//	RecalAtt(TRUE);
	//}


	// ���������
	//ItemCreator::RemoveEquipBindAtt(pEquip);


	GetItemMgr().UpdateEquipSpec(*pEquip);

	// �ж�ͨ����ɾ����Ʒ
	GetItemMgr().ItemUsedFromBag(n64SerialItem, (INT16)1, (DWORD)elcid_compose_engrave);


	GetAchievementMgr().UpdateAchievementCriteria(eta_unbind_equip, 1);
	return E_Success;
}

DWORD Role::EquipReAtt(DWORD dwNPCID, INT64 n64SerialEquip, INT64 n64ItemID, BYTE byIndex, BYTE byType)
{

	Creature* pNPC = get_map()->find_creature(dwNPCID);
	if( !VALID_POINT(pNPC)) 
		return E_Compose_NPC_Not_Exist;

	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
		return E_Compose_NPC_Distance;

	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_ReaAtt) )
		return E_Compose_NPCCanNotCompose;


	// ����װ��
	if (!MIsEquipment(n64SerialEquip))
	{
		return E_Equip_Reatt_NotFind;
	}

	tagEquip* pEquip = (tagEquip*)GetItem(n64SerialEquip);
	// װ��������
	if (!VALID_POINT(pEquip))
	{
		return E_Equip_Reatt_NotFind;
	}
	
	if (pEquip->pEquipProto->eEquipPos == EEP_Shipin1)
	{
		return INVALID_VALUE;
	}

	// ��������װ
	if (pEquip->equipSpec.byQuality < EIQ_Quality3)
	{
		return E_Equip_Reatt_Not_Quality3;
	}
	
	if (byIndex < 0 || byIndex > 2)
	{
		return E_Equip_Reatt_Index_Not;
	}
	
	//switch(pNPC->GetProto()->uFunctionID.dwCommonID)
	//{
	//case 0:
	//	if (pEquip->pEquipProto->byMinUseLevel < 45 || pEquip->pEquipProto->byMinUseLevel > 50)
	//	{
	//		return E_Equip_Reatt_level_not;
	//	}
	//	break;
	//case 1:
	//	if (pEquip->pEquipProto->byMinUseLevel < 51 || pEquip->pEquipProto->byMinUseLevel > 55)
	//	{
	//		return E_Equip_Reatt_level_not;
	//	}
	//	break;
	//case 2:
	//	if (pEquip->pEquipProto->byMinUseLevel < 56 || pEquip->pEquipProto->byMinUseLevel > 60)
	//	{
	//		return E_Equip_Reatt_level_not;
	//	}
	//	break;
	//case 3:
	//	if (pEquip->pEquipProto->byMinUseLevel < 61 || pEquip->pEquipProto->byMinUseLevel > 65)
	//	{
	//		return E_Equip_Reatt_level_not;
	//	}
	//	break;
	//case 4:
	//	if (pEquip->pEquipProto->byMinUseLevel < 66 || pEquip->pEquipProto->byMinUseLevel > 70)
	//	{
	//		return E_Equip_Reatt_level_not;
	//	}
	//	break;
	//default:
	//	return E_Equip_Reatt_level_not;
	//	break;
	//}

	if (pEquip->pEquipProto->byMinUseLevel < 45)
	{
		return E_Equip_Reatt_level_not;
	}

	if (pEquip->eCreateMode == EICM_Product_picture)
	{
		return E_Equip_Reatt_is_Picture_Mod;
	}

	if (pEquip->equipSpec.EquipAttitionalAtt[byIndex].eRoleAtt == ERA_Null)
	{
		return E_Equip_Reatt_Index_Not;	
	}
	
	INT nMinStar = ItemHelper::GetEquipReattStar(pEquip->pEquipProto->byMinUseLevel);
	if (pEquip->equipSpec.byConsolidateLevel < nMinStar)
		return E_Equip_Reatt_Star_not;


	switch(byType)
	{
	case 0 : 
		{
			INT nYuanbao = ItemHelper::GetEquipReattYuanbao(pEquip->pEquipProto->byMinUseLevel);
			// Ԫ������
			if(GetCurMgr().GetBaiBaoYuanBao() < nYuanbao)
				return E_Equip_Reatt_NotEnough_Money;

			ItemCreator::ReattEquip(pEquip, byIndex);
			
			GetItemMgr().UpdateEquipSpec(*pEquip);

			GetCurMgr().DecBaiBaoYuanBao(nYuanbao, elcid_compose_reatt);

			GetAchievementMgr().UpdateAchievementCriteria(eta_equip_reatt, 0, 1);
		}
		break;
	case 1:
		{
			INT nYuanbao = ItemHelper::GetEquipReattYuanbao(pEquip->pEquipProto->byMinUseLevel) / 2;
			// Ԫ������
			if(GetCurMgr().GetBaiBaoYuanBao() < nYuanbao)
				return E_Equip_Reatt_NotEnough_Money;
			
			tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
			if (!VALID_POINT(pItem))
			{
				return E_Equip_Reatt_item_Not_find;
			}
			
			if (pItem->pProtoType->eSpecFunc != EIST_ReAtt_Equip)
			{
				return E_Equip_Reatt_item_Type_Not;
			}
			if (pItem->pProtoType->nSpecFuncVal1 > pEquip->pEquipProto->byMinUseLevel ||
				pItem->pProtoType->nSpecFuncVal2 < pEquip->pEquipProto->byMinUseLevel)
			{
				return E_Equip_Reatt_item_Level_Not;
			}

			ItemCreator::ReattEquip(pEquip, byIndex);

			GetItemMgr().UpdateEquipSpec(*pEquip);

			GetCurMgr().DecBaiBaoYuanBao(nYuanbao, elcid_compose_reatt);
			
			GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_compose_reatt);

			GetAchievementMgr().UpdateAchievementCriteria(eta_equip_reatt, 1, 1);	

		}
		break;
	case 2:
		{
			tagItem* pItem = GetItemMgr().GetBagItem(n64ItemID);
			if (!VALID_POINT(pItem))
			{
				return E_Equip_Reatt_item_Not_find;
			}

			if (pItem->pProtoType->eSpecFunc != EIST_ReAtt_Equip)
			{
				return E_Equip_Reatt_item_Type_Not;
			}
			if (pItem->pProtoType->nSpecFuncVal1 > pEquip->pEquipProto->byMinUseLevel ||
				pItem->pProtoType->nSpecFuncVal2 < pEquip->pEquipProto->byMinUseLevel)
			{
				return E_Equip_Reatt_item_Level_Not;
			}
			
			ItemCreator::ReattEquip(pEquip);

			GetItemMgr().UpdateEquipSpec(*pEquip);
			GetItemMgr().ItemUsedFromBag(n64ItemID, (INT16)1, (DWORD)elcid_compose_reatt);

			GetAchievementMgr().UpdateAchievementCriteria(eta_equip_reatt, 2, 1);

		}
		break;
		
	}

	
	GetAchievementMgr().UpdateAchievementCriteria(ete_reatt_equip, 1);

	return E_Success;
}



DWORD	Role::EquipKaiguang(DWORD dwNPCID, INT64 n64SerialEquip)
{
	// ����װ��
	//if (!MIsEquipment(n64SerialEquip))
	//{
	//	return E_Role_Weapon_Kaiguang_Not_Find;
	//}

	//tagEquip* pEquip = (tagEquip*)GetItem(n64SerialEquip);
	//// װ��������
	//if (!VALID_POINT(pEquip))
	//{
	//	return E_Role_Weapon_Kaiguang_Not_Find;
	//}
	//
	////��������
	//if (!M_is_weapon(pEquip))
	//	return E_Role_Weapon_Kaiguang_Not_Find;
	//
	//// ������60��10��
	//if ( pEquip->pEquipProto->byMinUseLevel < 60 || pEquip->equipSpec.byConsolidateLevel < 10)
	//	return E_Role_Weapon_Kaiguang_Not_Con;
	//
	//INT64 nYuanbao = 0;


	//INT32 nRand = 0;
	//// ��ɫ�ȼ�����
	////if (pEquip->equipSpec.byAddTalentPoint == 0)
	////{
	////	if( get_level() < 60 || pEquip->equipSpec.nLevel < 60 || getGodLevel() < 15)
	////		return E_Role_Weapon_Kaiguang_Not_Role_level;

	////	nYuanbao = 200000;
	////	nRand = get_tool()->rand_in_range(1, 15);
	////}	
	////	

	////if (pEquip->equipSpec.byAddTalentPoint == 1)
	////{
	////	if( get_level() < 65 || pEquip->equipSpec.nLevel < 65 || getGodLevel() < 17)
	////		return E_Role_Weapon_Kaiguang_Not_Role_level;

	////	nYuanbao = 1000000;
	////	nRand = get_tool()->rand_in_range(5, 30);
	////}
	//	
	//
	//// �����ٿ�����
	//if (pEquip->equipSpec.byAddTalentPoint >=2)
	//	return E_Role_Weapon_Kaiguang_Not_goon;
	//
	//// Ԫ������
	//if(GetCurMgr().GetBagSilver() < nYuanbao)
	//	return E_Role_Weapon_Kaiguang_NotEnough_Money;

	//pEquip->equipSpec.byAddTalentPoint++;
	//
	//// �����������
	//pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Physique;
	//pEquip->equipSpec.EquipAttitionalAtt[7].nValue += nRand;

	//GetCurMgr().DecBagSilverEx(nYuanbao, elcid_compose_reatt);

	//GetItemMgr().UpdateEquipSpec(*pEquip);
	
	//if (pEquip->equipSpec.byAddTalentPoint == 1)
	//{
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_kaiguang, 0, 1);
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_kaiguang_get_nai, 0, nRand);
	//}	


	//if (pEquip->equipSpec.byAddTalentPoint == 2)
	//{
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_kaiguang, 1, 1);
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_kaiguang_get_nai, 1, nRand);
	//}


	return E_Success;
}

// װ��ϴ��
DWORD	Role::EquipXili(DWORD dwNPCID, INT64 n64SerialEquip, DWORD dwType)
{
	//// ����װ��
	//if (!MIsEquipment(n64SerialEquip))
	//{
	//	return INVALID_VALUE;
	//}

	//tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	//// װ��������
	//if (!VALID_POINT(pEquip) || !pEquip->IsBind())
	//{
	//	return INVALID_VALUE;
	//}
	//
	//if (pEquip->pEquipProto->eEquipPos == EEP_Shipin1)
	//{
	//	return INVALID_VALUE;
	//}

	//if (pEquip->equipSpec.byQuality < EIQ_Quality1)
	//{
	//	return E_Role_Equip_xili_quality_not;
	//}
	//
	//if (pEquip->pEquipProto->byMinUseLevel < 35)
	//{
	//	return E_Role_Equip_xili_level_not;
	//}
	//
	//if (get_level() < 35)
	//{
	//	return E_Role_Equip_xili_role_level_not;
	//}

	//if (pEquip->eCreateMode == EICM_Product_picture)
	//{
	//	return E_Role_Equip_xili_product_picture;
	//}

	//INT nPerDayFreeXili = GET_VIP_EXTVAL(GetTotalRecharge(), XILI_ADD, INT);
	//INT32 nExplois = ItemHelper::GetXiliExplios(pEquip->pEquipProto->byMinUseLevel);
	//

	//if (dwType == 0)
	//{
	//	// ս������
	//	if(nPerDayFreeXili <= GetDayClearData(ERDCT_Xili))
	//	{
	//		if(GetCurMgr().GetExploits() < nExplois)
	//			return E_Role_Equip_xili_Not_enough_cur;
	//	}
	//} else {

	//	if(GetDayClearData(ERDCT_Xili_FREE_LIMIT) >= EQUIP_XILI_FREE &&
	//		GetDayClearData(ERDCT_Xili_LIMIT_SOMETHING)<=0 ){
	//			return E_Role_Equip_xili_OutOfTimes;
	//	}

	//	if (dwType == 1)
	//	{
	//		// Ԫ������
	//		if(GetCurMgr().GetBaiBaoYuanBao() < 2)
	//			return E_Role_Equip_xili_Not_enough_yuanbao;
	//	}
	//	else
	//	{
	//		// Ԫ������
	//		if(GetCurMgr().GetBaiBaoYuanBao() < 10)
	//			return E_Role_Equip_xili_Not_enough_yuanbao;
	//	}
	//}



	//const tagEquipQltyEffect *pEquipQltyEffect = AttRes::GetInstance()->GetEquipQltyEffect(pEquip->equipSpec.byQuality);
	//if(!VALID_POINT(pEquipQltyEffect))
	//{
	//	return INVALID_VALUE;
	//}

	//const tagEquipLevelPctProto* pEquipLevelPctProto = AttRes::GetInstance()->GetEquipLevelPct((INT16)pEquip->pEquipProto->byLevel);
	//if(!VALID_POINT(pEquipLevelPctProto))
	//{
	//	return INVALID_VALUE;
	//}

	//FLOAT fPct = pEquipLevelPctProto->fLevelPct * pEquipLevelPctProto->fPosPct[pEquip->pEquipProto->eEquipPos];

	//// ϴ���������ֵ
	//INT32 nXiliAtt = pEquipQltyEffect->nXiliAtt * fPct;

	//
	//nCurEquip = n64SerialEquip;
	//
	//// ���Ƿ����������Ѵﵽ���
	//BOOL bAllFull = TRUE;
	//for (int i = 0; i < MAX_RAND_ATT; i++)
	//{
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_Null)
	//		continue;

	//	INT nMax = nXiliAtt;
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExDefense)
	//	{
	//		nMax *= 4;
	//	}
	//	else if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExAttack)
	//	{
	//		nMax *= 10;
	//	}
	//	else if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt > ERA_Agility)
	//	{
	//		nMax *= 2;
	//	}

	//	if (pEquip->equipSpec.EquipAttitional[i] < nMax)
	//	{
	//		bAllFull = FALSE;
	//		break;
	//	}
	//}
	//
	//if (bAllFull)
	//{
	//	return E_Role_Equip_xili_All_full;
	//}

	//for (int i = 0; i < MAX_RAND_ATT; i++)
	//{
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_Null)
	//		continue;
	//	
	//	INT nMax = nXiliAtt;
	//	INT nCurAtt = pEquip->equipSpec.EquipAttitional[i];
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExDefense)
	//	{
	//		nMax *= 4;
	//		nCurAtt /= 4;
	//	}
	//	else if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExAttack)
	//	{
	//		nMax *= 10;
	//		nCurAtt /= 10;
	//	}
	//	else if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt > ERA_Agility)
	//	{
	//		nMax *= 2;
	//		nCurAtt /= 2;
	//	}
	//	
	//	// ������α仯
	//	DWORD dwChange = 0;
	//	INT nRand = get_tool()->tool_rand()%100;
	//	
	//	INT nPct = 0;
	//	INT ntemp = nCurAtt * 1.0 / nMax * 100;
	//	if (ntemp < 25)
	//	{
	//		nPct = 60;
	//	}
	//	else if(ntemp < 50)
	//	{
	//		nPct = 45;
	//	}
	//	else if(ntemp < 75)
	//	{
	//		nPct = 30;
	//	}
	//	else 
	//	{
	//		nPct = 15;
	//	}


	//	if (dwType ==2) //��10Ԫ������10%����
	//	{
	//		nPct+=10;
	//	}
	//	if ( nRand < nPct)//����
	//	{
	//		dwChange = 1;
	//	}
	//	else if (nRand < 80)//����
	//	{
	//		dwChange = 2;
	//	}
	//	
	//	if (pEquip->equipSpec.EquipAttitional[i] >= nMax && dwChange == 1)
	//		continue;

	//	INT32 nPoint = 0;
	//	// һ����������һ�������
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt <= ERA_Agility)
	//	{
	//		INT nTemp = get_tool()->tool_rand()%100;
	//		if (nTemp < 70)
	//		{
	//			nPoint = 1;
	//		}
	//		else
	//		{
	//			nPoint = 2;
	//		}
	//	}
	//	else if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExDefense)
	//	{
	//		INT nTemp = get_tool()->tool_rand()%100;
	//		if (nTemp < 20)
	//		{
	//			nPoint = 1;
	//		}
	//		else if (nTemp < 40)
	//		{
	//			nPoint = 2;
	//		}
	//		else if (nTemp < 60)
	//		{
	//			nPoint = 3;
	//		}
	//		else if (nTemp < 80)
	//		{
	//			nPoint = 4;
	//		}
	//		else 
	//		{
	//			nPoint = 5;
	//		}
	//	}
	//	else
	//	{
	//		INT nTemp = get_tool()->tool_rand()%100;
	//		if (nTemp < 50)
	//		{
	//			nPoint = 1;
	//		}
	//		else if (nTemp < 80)
	//		{
	//			nPoint = 2;
	//		}
	//		else 
	//		{
	//			nPoint = 3;
	//		}
	//	}
	//	
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt == ERA_ExAttack)
	//	{
	//		nPoint *= 10;
	//	}

	//	if (dwChange== 2)
	//		nPoint = -nPoint;
	//	
	//	if (dwChange == 0)
	//		nPoint = 0;

	//	if (pEquip->equipSpec.EquipAttitional[i] + nPoint > nMax)
	//	{
	//		nPoint = nMax - pEquip->equipSpec.EquipAttitional[i];
	//	}
	//	if (pEquip->equipSpec.EquipAttitional[i] + pEquip->equipSpec.EquipAttitionalAtt[i].nValue <= 0 &&nPoint < 0)
	//	{
	//		nPoint = 0;
	//	}

	//	EquipAttitional[i] = nPoint;
	//}

	//if (dwType == 0)
	//{
	//	if(nPerDayFreeXili <= GetDayClearData(ERDCT_Xili)){
	//		GetCurMgr().DecExploits(nExplois, elcid_compose_xili);
	//	} else {
	//		ModRoleDayClearDate(ERDCT_Xili);
	//	}
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_xili, 1, 1);
	//} else {

	//	if(GetDayClearData(ERDCT_Xili_FREE_LIMIT) < EQUIP_XILI_FREE){
	//		ModRoleDayClearDate(ERDCT_Xili_FREE_LIMIT);
	//	} else if(GetDayClearData(ERDCT_Xili_LIMIT_SOMETHING)){
	//		ModRoleDayClearDate(ERDCT_Xili_LIMIT_SOMETHING, -1);
	//	}

	//	if (dwType == 1)
	//	{
	//		GetCurMgr().DecBaiBaoYuanBao(10, elcid_compose_xili);
	//		GetAchievementMgr().UpdateAchievementCriteria(eta_equip_xili, 2, 1);
	//	}
	//	else
	//	{
	//		GetCurMgr().DecBaiBaoYuanBao(50, elcid_compose_xili);
	//		GetAchievementMgr().UpdateAchievementCriteria(eta_equip_xili, 3, 1);
	//	}
	//}



	//if(VALID_POINT(GetScript()))
	//{
	//	GetScript()->OnEquipXiLi(this);
	//}
	//
	//GetAchievementMgr().UpdateAchievementCriteria(eta_equip_xili, 0, 1);	

	return E_Success;
}

// ϴ�������滻
DWORD	Role::EquipXiliChange(INT64 n64SerialEquip)
{
	// ����װ��
	if (!MIsEquipment(nCurEquip))
	{
		return INVALID_VALUE;
	}

	tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(nCurEquip);
	// װ��������
	if (!VALID_POINT(pEquip))
	{
		return INVALID_VALUE;
	}

	// �������滻���Ե�װ��
	if (nCurEquip != n64SerialEquip)
	{
		return INVALID_VALUE;
	}
	for (int i = 0; i < MAX_RAND_ATT; i++)
	{
		pEquip->equipSpec.EquipAttitional[i] += EquipAttitional[i];
	}

	GetItemMgr().UpdateEquipSpec(*pEquip);
	
	ZeroMemory(&EquipAttitional, sizeof(EquipAttitional));
	return E_Success;
}

DWORD	Role::EquipGetWuhuen(INT64 n64SerialEquip, DWORD dwItemID, int nNum)
{
	// ����װ��
	//if (!MIsEquipment(n64SerialEquip))
	//{
	//	return INVALID_VALUE;
	//}

	//tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	//// װ��������
	//if (!VALID_POINT(pEquip))
	//{
	//	return INVALID_VALUE;
	//}
	//
	//// �������� ��Ʒ
	//if (pEquip->pEquipProto->eEquipPos != EEP_RightHand &&
	//	pEquip->pEquipProto->eEquipPos != EEP_Shipin1)
	//{
	//	return INVALID_VALUE;
	//}

	//if (pEquip->equipSpec.nLevel <=1 && pEquip->equipSpec.nCurLevelExp == 0)
	//{
	//	return INVALID_VALUE;
	//}
	//
	//int yuanbao  = 10;
	//BOOL need_yuanbao = FALSE;
	//if (GetDayClearData(4) >= 3)
	//{
	//	if(GetCurMgr().GetBaiBaoYuanBao() < yuanbao )
	//		return E_Role_Equip_Wuhuen_Outof_yuanbao;
	//	need_yuanbao = TRUE;
	//}

	//INT32 nTotleExp = 0;
	//for (int i = 1; i < pEquip->equipSpec.nLevel; i++)
	//{
	//	const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(pEquip->equipSpec.nLevel-i);
	//	if (VALID_POINT(pEffect))
	//	{
	//		if (pEquip->pEquipProto->eEquipPos == EEP_Shipin1)
	//		{
	//			nTotleExp += pEffect->nExpLevelUpShipin;	
	//		}
	//		else
	//		{
	//			nTotleExp += pEffect->nExpLevelUp;
	//		}
	//		
	//	}
	//	nTotleExp += pEquip->equipSpec.nCurLevelExp;
	//}
	//
	//package_list<tagItem*> itemList;


	//double fAddPro = 0.0f;
	//if (dwItemID != INVALID_VALUE)
	//{
	//	const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwItemID);
	//	if (!VALID_POINT(pItemProto))
	//		return INVALID_VALUE;

	//	if (pItemProto->eSpecFunc != EIST_Equip_Get_Wuhuen)
	//		return INVALID_VALUE;
	//	
	//	if (nNum > 5)
	//		return INVALID_VALUE;

	//	INT32 nRealNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItemID, nNum);


	//	fAddPro = pItemProto->nSpecFuncVal1 / 10000.0f;
	//	
	//	GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, dwItemID, min(nRealNum, nNum));

	//	GetItemMgr().DelBagSameItem(itemList, min(nRealNum, nNum), elcid_compose_tiqu);
	//}
	//
	//
	//
	//INT32 nAddWuhuen = nTotleExp * (0.9 + fAddPro * nNum);

	//if (nAddWuhuen >= nTotleExp)
	//{
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_getwuhuen_yibai, 1);
	//}
	//
	//GetAchievementMgr().UpdateAchievementCriteria(ete_fusion_equip, 1);

	//ChangeWuhuen(nAddWuhuen);
	//SaveWuhuen2DB();

	//if(need_yuanbao){
	//	GetCurMgr().DecBaiBaoYuanBao(yuanbao, elcid_compose_tiqu);
	//}
	//
	//ModRoleDayClearDate(4);

	//// �����츳��������
	//pEquip->equipSpec.nLevel = 1;
	//pEquip->equipSpec.nCurLevelExp = 0;
	//pEquip->equipSpec.byTalentPoint = 0;
	//pEquip->equipSpec.byMaxTalentPoint = 0;
	//pEquip->equipSpec.byAddTalentPoint = 0;
	//pEquip->ClearSkill();
	//
	//pEquip->equipSpec.EquipAttitionalAtt[7].eRoleAtt = ERA_Null;
	//pEquip->equipSpec.EquipAttitionalAtt[7].nValue = 0;

	//if (pEquip->pEquipProto->eEquipPos == EEP_Shipin1)
	//{
	//	memset(pEquip->equipSpec.EquipAttitional, 0, sizeof(pEquip->equipSpec.EquipAttitional));
	//}
	//GetItemMgr().UpdateEquipSpec(*pEquip);	
	//
	//if( VALID_POINT(m_pScript))
	//	m_pScript->OnWeaponFusion( this );

	return E_Success;
}

// ��������
DWORD	Role::EquipRonglian(INT64 n64SerialEquip)
{
	// ����װ��
	//if (!MIsEquipment(n64SerialEquip))
	//{
	//	return INVALID_VALUE;
	//}

	//tagEquip* pEquip = (tagEquip*)GetItemMgr().GetEquipBarEquip(n64SerialEquip);
	//// װ��������
	//if (!VALID_POINT(pEquip))
	//{
	//	return INVALID_VALUE;
	//}
	//
	//// ��������
	//if (pEquip->pEquipProto->eEquipPos != EEP_RightHand)
	//{
	//	return INVALID_VALUE;
	//}
	//if (pEquip->equipSpec.nLevel >= pEquip->pEquipProto->byMaxLevel ||
	//	pEquip->equipSpec.nLevel >= get_level())
	//{
	//	return E_Role_Equip_ronglian_level_max;
	//}

	//const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(pEquip->equipSpec.nLevel);
	//if (!VALID_POINT(pEffect))
	//{
	//	return INVALID_VALUE;
	//}
	//
	//INT nAddExp = pEffect->nExpLevelUp - pEquip->equipSpec.nCurLevelExp;

	//if (GetAttValue(ERA_Wuhuen) < nAddExp)
	//	return E_Role_Equip_ronglian_not_wuhuen;
	//
	//ResetWeaponDmg(*pEquip, FALSE);
	//BOOL bLevelUp = pEquip->ExpChange(nAddExp);
	//ResetWeaponDmg(*pEquip, TRUE);
	//GetItemMgr().UpdateEquipSpec(*pEquip);	
	//
	//ChangeWuhuen(-nAddExp);
	//SaveWuhuen2DB();
	//
	//RecalAtt();
	//
	//if (bLevelUp)
	//{
	//	m_achievementMgr.UpdateAchievementCriteria(ete_xiulian, pEquip->equipSpec.nLevel);
	//}

	//if( VALID_POINT( GetScript( ) ) && bLevelUp)
	//{
	//	GetScript( )->OnWeaponLevelUp( this, pEquip->equipSpec.nLevel );
	//	
	//}
	return E_Success;
}

// ��Ʒ��ħ
DWORD	Role::Equipfumo(INT64 n64SerialEquip, DWORD dwItemID, int nNum)
{
	// ����װ��
	//if (!MIsEquipment(n64SerialEquip))
	//{
	//	return INVALID_VALUE;
	//}

	//tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	//// װ��������
	//if (!VALID_POINT(pEquip))
	//{
	//	return INVALID_VALUE;
	//}
	//
	//// ������Ʒ
	//if (pEquip->pEquipProto->eEquipPos != EEP_Shipin1)
	//{
	//	return INVALID_VALUE;
	//}

	//if (pEquip->equipSpec.nLevel >= pEquip->pEquipProto->byMaxLevel ||
	//	pEquip->equipSpec.nLevel >= get_level())
	//{
	//	return E_Role_Equip_ronglian_level_max;
	//}

	//const tagEquipLevelUpEffect* pEffect = AttRes::GetInstance()->GetEquipLevelUpEffect(pEquip->equipSpec.nLevel);
	//if (!VALID_POINT(pEffect))
	//{
	//	return INVALID_VALUE;
	//}

	//INT nAddExp = pEffect->nExpLevelUpShipin - pEquip->equipSpec.nCurLevelExp;

	//if (GetAttValue(ERA_Wuhuen) < nAddExp)
	//	return E_Role_Equip_ronglian_not_wuhuen;
	//

	//// �ж��õ�����������Ʒ
	//package_list<tagItem*> itemList;
	//
	//if (dwItemID != INVALID_VALUE)
	//{
	//	const tagItemProto* pItemProto = AttRes::GetInstance()->GetItemProto(dwItemID);
	//	if (!VALID_POINT(pItemProto))
	//		return INVALID_VALUE;

	//	if (pItemProto->eSpecFunc != EIST_Equip_fumo)
	//		return INVALID_VALUE;

	//	if (nNum > 10)
	//		return INVALID_VALUE;

	//	INT32 nRealNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItemID, nNum);
	//	
	//	GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, dwItemID, min(nRealNum, nNum));


	//	GetItemMgr().DelBagSameItem(itemList, min(nRealNum, nNum), elcid_compose_tiqu);
	//}



	//pEquip->ExpChange(nAddExp);
	//
	////GetAchievementMgr().UpdateAchievementCriteria(eta_shipin_fumo_level, pEquip->equipSpec.nLevel);

	//INT nAddAttValue = 1;
	//// �����ˣ�����������
	//BOOL bCrit = FALSE;
	//INT nRand = get_tool()->tool_rand() % 100;
	//if (nRand <= 5 + 5 * nNum)
	//{
	//	bCrit = TRUE;
	//	GetAchievementMgr().UpdateAchievementCriteria(eta_shipin_fumo_crit, 1);
	//}

	//for (int i = 0; i < MAX_RAND_ATT; i++)
	//{
	//	if (pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt != ERA_Null)
	//	{
	//		INT nAddAttValue = ItemHelper::getShipinFumoAttAdd(pEquip->equipSpec.EquipAttitionalAtt[i].eRoleAtt);
	//		if (bCrit)
	//		{
	//			INT nMin = 0;
	//			INT nMax = 0;
	//			ItemHelper::getShipinFumoAttCritAdd(pEquip->equipSpec.byQuality, nMin, nMax);
	//			nAddAttValue += get_tool()->rand_in_range(nMin, nMax);
	//		}
	//		pEquip->equipSpec.EquipAttitional[i] += nAddAttValue;
	//	}
	//}
	//
	//GetItemMgr().UpdateEquipSpec(*pEquip);	

	//ChangeWuhuen(-nAddExp);
	//SaveWuhuen2DB();
	//
	//GetAchievementMgr().UpdateAchievementCriteria(eta_shipin_fumo, 1);

	return E_Success;
}

// ����
DWORD Role::IdentifyEquip(INT64 n64SerialReel, INT64 n64SerialEquip)
{

	// ��������
	tagItem *pReel = GetItemMgr().GetBagItem(n64SerialReel);
	if(!VALID_POINT(pReel))
	{
		return E_Item_NotFound;
	}

	if(pReel->pProtoType->eSpecFunc != EISF_IdetifyEquip)
	{
		return INVALID_VALUE;
	}
	

	// ���װ��
	tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	// װ��������
	if(!VALID_POINT(pEquip))
	{
		return E_Item_NotFound;
	}
	
	if (!MIsEquipment(pEquip->pEquipProto->dw_data_id))
	{
		return INVALID_VALUE;
	}
	
	// ʱװ��򲻿��Լ���
	if (pEquip->pEquipProto->eEquipPos == EEP_Fashion || 
		pEquip->pEquipProto->eEquipPos == EEP_Body1 ||
		pEquip->pEquipProto->eEquipPos == EEP_Ride)
		return E_Compose_Equip_Not_identify;//gx modify �����˴�����


	//if ((pEquip->equipSpec.byQuality > EIQ_Quality3 && pReel->pProtoType->nSpecFuncVal1 != 1))
	//{
	//	return INVALID_VALUE;
	//}
	
	//if (pEquip->pEquipProto->byMinUseLevel < 30)
	//	return INVALID_VALUE;

	
	EItemQuality byNewQuality = EIQ_Null;
	if (pReel->pProtoType->nSpecFuncVal1 == 1)
	{
		if (pEquip->equipSpec.byQuality <= EIQ_Quality0)
			return INVALID_VALUE;

		byNewQuality = (EItemQuality)pEquip->equipSpec.byQuality; 
	}
	else if(pReel->pProtoType->nSpecFuncVal1 == 2)
	{
		byNewQuality = EIQ_Quality4;
	}
	else if(pReel->pProtoType->nSpecFuncVal1 == 3)
	{
		byNewQuality = EIQ_Quality5;
	}

	ItemCreator::IdentifyEquip(pEquip, byNewQuality);


	GetItemMgr().UpdateEquipSpec(*pEquip);
	//INT nRand = get_tool()->tool_rand() % 10000;
	//EItemQuality eQuality = EIQ_Null;
	//DWORD dwItemID = 0;
	//if (nRand < pReel->pProtoType->nSpecFuncVal1 )
	//{
	//	eQuality = EIQ_Quality2;
	//	dwItemID = pEquip->pProtoType->nSpecFuncVal1;
	//}
	//else
	//{
	//	eQuality = EIQ_Quality3;
	//	dwItemID = pEquip->pProtoType->nSpecFuncVal2;
	//}

	//tagItem *pItem = ItemCreator::Create(EICM_Loot,  m_dwID, dwItemID, 1, true, m_dwID);

	//if(VALID_POINT(pItem) )
	//{
	//	// ���ɼ���������
	//	ItemCreator::IdentifyEquip((tagEquip*)pItem, eQuality);
	//	GetItemMgr().Add2Bag(pItem, elcid_equip_identify, TRUE);
	//}

	
	GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, pReel->dw_data_id, 1);

	// ʹ��һ��
	GetItemMgr().ItemUsedFromBag(n64SerialReel, 1, elcid_equip_identify);
	//GetItemMgr().DelFromBag(n64SerialEquip, elcid_equip_identify, 1);
	
	//gx add 2013.7.20 ��������ʷʫ����˵����ȫ���㲥
	if (EIQ_Quality4 ==byNewQuality && pReel->dw_data_id != 1400020)//����ϴ����
	{
		HearSayHelper::SendMessage(EHST_EQUIPUPGRADE,pEquip->dwOwnerID,pEquip->dw_data_id,0);
	}
	else if (EIQ_Quality5 == byNewQuality && pReel->dw_data_id != 1400020)//����ϴ����
	{
		HearSayHelper::SendMessage(EHST_EQUIPUPGRADE,pEquip->dwOwnerID,pEquip->dw_data_id,1);
	}
	else
	{
		//do nothing
	}
	return E_Success;
}

DWORD Role::PetXiulianSizeChange(DWORD dwNPCID)
{
	Creature* pNPC = get_map()->find_creature(dwNPCID);
	if( !VALID_POINT(pNPC)) 
		return E_Compose_NPC_Not_Exist;

	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
		return E_Compose_NPC_Distance;

	if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Pet_Xiulian) )
		return E_Compose_NPCCanNotCompose;
	
	if (n16_pet_xiulian_size >= ROLE_PET_XIULIAN_SIZE)
		return E_Role_change_petxiulian_max;
	
	INT64 nMonery = RoleHelper::GetPetXiulianSiver(n16_pet_xiulian_size+1);
	if (GetCurMgr().GetBagSilver() < nMonery)
		return E_Role_change_petxiulian_not_monery;

	add_pet_xiulian_size(1);

	GetCurMgr().DecBagSilver(nMonery, elcid_pet_xiulian_size_change);
	return E_Success;
}

DWORD	Role::EquipChange(INT64 n64SerialEquip, DWORD n64Item1, DWORD n64Item2, DWORD n64Item3, INT64& n64NewEquip)
{
	// ���װ��
	tagEquip* pEquip = (tagEquip*)GetItemMgr().GetBagItem(n64SerialEquip);
	// װ��������
	if(!VALID_POINT(pEquip))
	{
		return INVALID_VALUE;
	}
	const tagEquipChange* pProto = AttRes::GetInstance()->GetEquipChangeProto(pEquip->pEquipProto->dw_data_id);
	if (!VALID_POINT(pProto))
	{
		return INVALID_VALUE;
	}

	const tagEquipProto* pEquipProto = AttRes::GetInstance()->GetEquipProto(pProto->dwTargetID);
	if (!VALID_POINT(pEquipProto))
		return INVALID_VALUE;
	
	// Ŀ��װ���ȼ�����
	if (pEquipProto->byMinUseLevel > get_level())
	{
		return INVALID_VALUE;
	}

	// �ж���ͨ�����Ƿ�����

	// ����ɫ��������				
	ItemMgr& itemMgr = GetItemMgr();	
	if (itemMgr.GetBagFreeSize() <= 0)
		return E_Compose_Bag_Full;

	// �������Ƿ��㹻
	//for (int i = 0; i < MAX_PRODUCE_STUFF_QUANTITY; i++)
	//{
	//	DWORD dwItemTypeID = pProto->sSutff[i].dwStuffID;
	//	if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
	//		continue;

	//	DWORD dwItemNum = pProto->sSutff[i].dwStuffNum;
	//	if (GetItemMgr().GetBag().GetSameItemCount(dwItemTypeID) < dwItemNum)
	//		return E_Compose_Stuff_Not_Enough;
	//}


	//package_list<tagItem*> itemList;

	//// ���Ĳ���
	//for (int i = 0; i < MAX_EQUIP_CHANGE_STUFF_QUANTITY; i++)
	//{
	//	DWORD dwItemTypeID = pProto->sSutff[i].dwStuffID;
	//	if (dwItemTypeID == INVALID_VALUE || dwItemTypeID == 0)
	//		continue;

	//	DWORD dwItemNum = pProto->sSutff[i].dwStuffNum;
	//	itemList.clear();
	//	INT32 nNum = GetItemMgr().GetBag().GetSameItemList(itemList, dwItemTypeID, dwItemNum);

	//	GetItemMgr().DelBagSameItem(itemList, dwItemNum, elcid_compose_produce);
	//}

	if (!MIsEquipment(pProto->dwTargetID))
	{
		return INVALID_VALUE;
	}

	package_list<tagItem*> listItem1;
	// �ж�һ����Ʒ
	const tagItemProto* pItemProto1 = AttRes::GetInstance()->GetItemProto(pProto->sSutff[0].dwStuffID);
	if (VALID_POINT(pItemProto1))
	{
		if (n64Item1 != pProto->sSutff[0].dwStuffID)
			return INVALID_VALUE;
		
		// ��Ʒ����
		int nNum = GetItemMgr().GetBagSameItemList(listItem1, n64Item1, pProto->sSutff[0].dwStuffNum);
		if (nNum < pProto->sSutff[0].dwStuffNum)
			return INVALID_VALUE;
		
	}

	// �ж϶�����Ʒ
	package_list<tagItem*> listItem2;
	const tagItemProto* pItemProto2 = AttRes::GetInstance()->GetItemProto(pProto->sSutff[1].dwStuffID);
	if (VALID_POINT(pItemProto2))
	{
		if (n64Item2 != pProto->sSutff[1].dwStuffID)
			return INVALID_VALUE;

		// ��Ʒ����
		int nNum = GetItemMgr().GetBagSameItemList(listItem2, n64Item2, pProto->sSutff[1].dwStuffNum);
		if (nNum < pProto->sSutff[1].dwStuffNum)
			return INVALID_VALUE;


	}
	
	// �ж���������
	package_list<tagItem*> listItem3;
	BOOL bUseSpeItem = FALSE;
	const tagItemProto* pItemProto3 = AttRes::GetInstance()->GetItemProto(pProto->sSutff[2].dwStuffID);
	if (VALID_POINT(pItemProto3) && n64Item3 == pProto->sSutff[2].dwStuffID)
	{
		int nNum = GetItemMgr().GetBagSameItemList(listItem3, n64Item3, pProto->sSutff[2].dwStuffNum);
		if (nNum >= pProto->sSutff[2].dwStuffNum)
		{
			bUseSpeItem = true;
		}
	}


	BYTE nQlty = max((int)pEquip->equipSpec.byQuality - 1, 0);
	if (bUseSpeItem)
	{
		nQlty = pEquip->equipSpec.byQuality;
	}


	tagItem* pNewItem = ItemCreator::Create(EICM_DAMO, GetID(), pProto->dwTargetID);
	if(!VALID_POINT(pNewItem))
	{
		return INVALID_VALUE;
	}

	if(nQlty == INVALID_VALUE)
	{
		// ������	
	}
	else
	{
		ItemCreator::IdentifyEquip(((tagEquip*)pNewItem), (EItemQuality)nQlty);
	}
	
	// �ж���װ��ǿ���ȼ�
	if (bUseSpeItem)
	{
		((tagEquip*)pNewItem)->equipSpec.byConsolidateLevel = max((int)pEquip->equipSpec.byConsolidateLevel, 0);
	}
	else
	{
		((tagEquip*)pNewItem)->equipSpec.byConsolidateLevel = max((int)pEquip->equipSpec.byConsolidateLevel - 3, 0);
	}
	
	// ��װ���̳���װ���ı�ʯ
	((tagEquip*)pNewItem)->equipSpec.byHoleNum = pEquip->equipSpec.byHoleNum;

	memcpy(((tagEquip*)pNewItem)->equipSpec.dwHoleGemID, pEquip->equipSpec.dwHoleGemID, sizeof(pEquip->equipSpec.dwHoleGemID));

	GetItemMgr().UpdateEquipSpec((*(tagEquip*)pNewItem));
	
	// �����װ��
	GetItemMgr().Add2Bag(pNewItem, elcid_npc_fusion, TRUE);

	n64NewEquip = pNewItem->n64_serial;

	// ɾ������
	GetItemMgr().DelBagSameItem(listItem1,  pProto->sSutff[0].dwStuffNum, (DWORD)elcid_compose_tiqu);
	GetItemMgr().DelBagSameItem(listItem2,  pProto->sSutff[1].dwStuffNum, (DWORD)elcid_compose_tiqu);

	if (bUseSpeItem)
	{
		GetItemMgr().DelBagSameItem(listItem3,  pProto->sSutff[2].dwStuffNum, (DWORD)elcid_compose_tiqu);
	}

	// ɾ����װ��
	GetItemMgr().DelFromBag(n64SerialEquip, (DWORD)elcid_compose_tiqu, 1);

	
	return E_Success;
}


DWORD	Role::EquipUseLuckYou(INT64 n64Item,CHAR &cCurLuck,CHAR &cChangeLuck)
{
	tagItem* pItem = GetItemMgr().GetBagItem(n64Item);
	if (!VALID_POINT(pItem))
		return INVALID_VALUE;

	tagEquip* pWeapon = (tagEquip*)GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
	if (!VALID_POINT(pWeapon))
		return INVALID_VALUE;

	// ����������������Ʒ
	if (pItem->pProtoType->eSpecFunc != EISF_Luck)
		return INVALID_VALUE;
	
	if (pWeapon->equipSpec.byLuck >= 7)
		return INVALID_VALUE;

	BYTE byChange = 0;

	if (pItem->pProtoType->nSpecFuncVal1 == 0 && pWeapon->equipSpec.byLuck >= 0)
	{
		int nParam[7][3] = {
			{100, 0, 0},
			{50, 85, 15},
			{25, 61, 39},
			{10, 47, 53},
			{5,  43, 57},
			{3,  42, 58},
			{1,  41, 59}
		};


		int nRand = get_tool()->tool_rand() % 100;

		if (nRand < nParam[pWeapon->equipSpec.byLuck][0])
		{
			byChange = 1;
		}
		else if(nRand < nParam[pWeapon->equipSpec.byLuck][1])
		{
			byChange = 0;
		}
		else
		{
			byChange = -1;
		}

	}
	else
	{
		byChange = 1;
	}

	EquipEffect(pWeapon, FALSE);
	
	pWeapon->equipSpec.byLuck += byChange;

	EquipEffect(pWeapon, TRUE);

	//SetRating();
	//SendTeamEquipInfo();

	GetItemMgr().UpdateEquipSpec(*pWeapon);

	GetItemMgr().DelFromBag(n64Item, (DWORD)elcid_compose_posy, 1);
	
	//��¼�仯��Ϣ�����ͻ�����ʾ�� gx add 2013.8.26
	cCurLuck = pWeapon->equipSpec.byLuck;
	cChangeLuck = byChange;

	return E_Success;
}