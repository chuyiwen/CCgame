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
*	@file		role_container.cpp
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		����װ���������ҡ��ֿ�ȴ�����
*/

#include "StdAfx.h"
#include "role.h"
#include "creature.h"
#include "player_session.h"
#include "../../common/WorldDefine/item_protocol.h"
#include "../../common/WorldDefine/base_define.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/formula_define.h"
#include "map.h"
#include "pet_heti.h"

//****************************** ��װ ******************************************

//--------------------------------------------------------------------------------------------
// ����Ƿ��ܹ�����
//--------------------------------------------------------------------------------------------
DWORD Role::CanEquip(tagEquip *pEquip, EEquipPos ePosDst)
{
	if(IsInRoleState(ERS_Hang))
		return E_Hang_NoCan_ChangeEquip;

	if (M_is_weapon(pEquip) && IsInRoleState(ERS_Prictice))
		return E_Equip_Prictice;

	// �ж�װ���Ƿ����
	/*if(!pEquip->bIdentified)*/
	if(!M_is_identified(pEquip))
	{
		return E_Equip_NotIdentify;
	}
	
	const tagEquipProto *pEquipProto = pEquip->pEquipProto;

	// �ж�ְҵ����
	//if (!(((m_eClassEx << 9) + m_eClass ) & pEquipProto->dwVocationLimitWear))
	//	return E_Equip_VocationLimit;
	if (!(( 1 << m_eClass ) & pEquipProto->dwVocationLimitWear))
		return E_Item_ClassLimit;
	
	// pkֵ�޶�
	if (pEquipProto->dwPkValueLimit != INVALID_VALUE &&
		pEquipProto->dwPkValueLimit < GetPKValue())
	{
		return E_Item_Pk_Value_Limit;
	}

	// �ж�����ȼ��Ƿ����
	if(m_nLevel < pEquipProto->byMinUseLevel 
		|| m_nLevel > pEquipProto->byMaxUseLevel)
	{
		return E_Item_LevelLimit;
	}

	// �жϽ�ɫ�Ա�
	if(pEquipProto->eSexLimit != ESL_Null 
		&& pEquipProto->eSexLimit != m_Avatar.bySex)
	{
		return E_Item_SexLimit;
	}

	//FLOAT fFactor = 1 + pEquip->equipSpec.n16AttALimModPct / 10000.0f;

	// �ж��������� -- ԭʼһ�����Լ�ֻ������������еĳ�ʼ���ԣ������Զ��ӵ���ֶ��ӵ㲿��
	//for(INT32 i=0; i<X_ERA_ATTA_NUM; ++i)
	//{
	//	if(GetBaseAttValue(i) < (INT)(pEquipProto->n16AttALimit[i] * fFactor + pEquip->equipSpec.n16AttALimMod))
	//	{
	//		return E_Item_AttA_Limit;
	//	}
	//}

	// �ж�ְҵ //?? ��ʱע��
	//if(!(pEquipProto->byClassLimit[m_eClass] & m_eClassEx))
	//{
	//	return E_Item_ClassLimit;
	//}

	// �ж���������
	//if(pEquipProto->eClanRepute >= ERT_BEGIN && pEquipProto->eClanRepute < ERT_END)
	//{
	//	ECLanType eClanType = MTRANS_ERT2ECLT(pEquipProto->eClanRepute);
	//	if(GetClanData().RepGetVal(eClanType) < pEquipProto->nClanReputeVal)
	//	{
	//		return E_Item_ClanRepLimit;
	//	}
	//}

	//// ������������ֵ
	//if( 0 )
	//{
	//	return E_Item_OtherClanRepLimit;
	//}

	//// ��������λ���Ƿ��ظ�
	//if(MIsEquipment(pEquip->dw_data_id))
	//{
	//	tagEquip *pOtherWeapon = GetEquipBar().GetItem((INT16)(EEP_RightHand == ePosDst ? EEP_LeftHand : EEP_RightHand));
	//	if(!VALID_POINT(pOtherWeapon))
	//	{
	//		return E_Success;
	//	}

	//	MTRANS_POINTER(pEquipProto, pEquip->pProtoType, tagEquipProto);
	//	MTRANS_POINTER(pOtherProto, pOtherWeapon->pProtoType, tagEquipProto);

	//	if((EWP_WaistBack == pEquipProto->eWeaponPos || EWP_Back == pEquip->pEquipProto->eWeaponPos)
	//		&& pEquip->pEquipProto->eWeaponPos == pOtherProto->eWeaponPos)
	//	{
	//		return E_Equip_WeaponPos_Overlap;
	//	}
	//}

	return E_Success;
}

//--------------------------------------------------------------------------------------------
// ����
//--------------------------------------------------------------------------------------------
DWORD Role::Equip(INT64 n64_serial, EEquipPos ePosDst)
{
	// ��ô���װ��
	tagItem *pItem = GetItemMgr().GetBagItem(n64_serial);
	if(!VALID_POINT(pItem))
	{
		return E_Item_NotFound;
	}

	// �ж���װ����Ʒ�Ƿ�Ϊװ��
	if(!MIsEquipment(pItem->dw_data_id))
	{
		return E_Item_NotEquipment;
	}

	tagEquip *pEquip = (tagEquip *)pItem;

	// ����Ƿ����װ������
	DWORD dw_error_code = CanEquip(pEquip, ePosDst);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	// ��õ�ǰװ��λ�õ�װ��
	tagEquip *pEquipOld = GetItemMgr().GetEquipBarEquip((INT16)ePosDst);

	// ��װ
	dw_error_code = GetItemMgr().Equip(n64_serial, ePosDst);
	if(E_Success == dw_error_code)
	{
		ProcEquipEffect(pEquip, pEquipOld, (INT16)ePosDst);

		g_ScriptMgr.GetRoleScript()->OnEquip(this, (int)ePosDst, pItem->dw_data_id);
	}

	return dw_error_code;
}

//--------------------------------------------------------------------------------------------
// ���� -- Ŀ��λ��ӦΪ�գ����տͻ���Ӧ������װ����Ϣ
//--------------------------------------------------------------------------------------------
DWORD Role::Unequip(INT64 n64_serial, INT16 n16IndexDst)
{
	// ���Ŀ��λ���Ƿ�Ϊ�գ����վ�ֱ�ӷ���
	if(n16IndexDst != INVALID_VALUE && !GetItemMgr().IsBagOneSpaceFree(n16IndexDst))
	{
		return INVALID_VALUE;
	}

	// ��ô���װ��
	tagEquip *pEquipOld = GetItemMgr().GetEquipBarEquip(n64_serial);
	if(!VALID_POINT(pEquipOld))
	{
		return E_Item_NotFound;
	}

	INT16 n16IndexOld = pEquipOld->n16Index;

	// ��װ
	if(INVALID_VALUE == n16IndexDst)
	{
		n16IndexDst = GetItemMgr().GetBagOneFreeSpace();
		if(INVALID_VALUE == n16IndexDst)
		{
			return E_Bag_NotEnoughSpace;
		}
	}

	if(IsInRoleState(ERS_Hang))
		return E_Hang_NoCan_ChangeEquip;

	DWORD dw_error_code = GetItemMgr().Unequip(n64_serial, n16IndexDst);
	if(dw_error_code != E_Success)
	{
		return dw_error_code;
	}

	ProcEquipEffect(NULL, pEquipOld, n16IndexOld);
	//! mwh 2011-09-11 װ��λ�ô���
	g_ScriptMgr.GetRoleScript()->OnUnEquip(this, (int)n16IndexOld/*pEquipOld->pEquipProto->eEquipPos*/, pEquipOld->pEquipProto->dw_data_id);


	return dw_error_code;
}


//--------------------------------------------------------------------------------------------
// ����ָλ��
//--------------------------------------------------------------------------------------------
DWORD Role::MoveRing(INT64 n64SerialSrc, INT16 n16PosDst)
{
	tagEquip *pRing1 = GetItemMgr().GetEquipBarEquip(n64SerialSrc);
	tagEquip *pRing2 = GetItemMgr().GetEquipBarEquip(n16PosDst);

	INT16 n16OldPos1 = EEP_Null;
	//INT16 n16OldPos2 = EEP_Null;

	if(VALID_POINT(pRing1))
	{
		n16OldPos1 = pRing1->n16Index;
	}

	//if(VALID_POINT(pRing2))
	//{
	//	n16OldPos2 = pRing2->n16Index;
	//}

	DWORD dw_error_code = GetItemMgr().MoveRing(n64SerialSrc, n16PosDst);
	if(E_Success != dw_error_code)
	{
		return dw_error_code;
	}


	// ��װ��λ����Ϣ���͸��ͻ���
	ProcEquipEffectPos(pRing1, pRing2, EICT_Equip, EICT_Equip);

	return E_Success;
}

//--------------------------------------------------------------------------------------------
// ���㻻װӰ��, �����͵��ͻ���
//--------------------------------------------------------------------------------------------
VOID Role::ProcEquipEffect(tagEquip *pNewEquip, tagEquip *pOldEquip, INT16 n16IndexOld, BOOL bDiscard/* = FALSE*/)
{
	// ��װ��λ����Ϣ���͸��ͻ���
	if(!bDiscard)
	{
		ProcEquipEffectPos(pNewEquip, pOldEquip, EICT_Equip, EICT_Bag);
	}

	// ����װ��Ӱ���������� -- ע��Ҫ�����ٴ�
	if(VALID_POINT(pOldEquip) && pOldEquip->GetEquipNewness() > 0)
	{
		ProcEquipEffectAtt(pOldEquip, FALSE, n16IndexOld);
		m_Suit.Remove(pOldEquip, n16IndexOld);
	}

	if(VALID_POINT(pNewEquip) && pNewEquip->GetEquipNewness() > 0)
	{
		ProcEquipEffectAtt(pNewEquip, TRUE, n16IndexOld);
		m_Suit.Add(pNewEquip, n16IndexOld);
	}
	
	//if (VALID_POINT(pNewEquip))
	//{
	//	// ���������,ʱװ��Ҫ��
	//	if ((MEquipIsRide(pNewEquip->pProtoType) || M_is_weapon(pNewEquip) || MIsFashion(pNewEquip->pEquipProto->eEquipPos) ) && !pNewEquip->IsBind())
	//	{
	//		pNewEquip->SetBind(EBS_Equip_Bind);
	//		NET_SIS_bind_change send;
	//		send.n64EquipSerial = pNewEquip->n64_serial;
	//		send.byBind = pNewEquip->byBind;
	//		send.dwConType = pNewEquip->eConType;
	//		SendMessage(&send, send.dw_size);
	//	}
	//}
	// ���¼�����Ӱ������ﵱǰ״̬
	RecalAtt();
	// �����������
	//SendAttChange(ERA_MaxHP);	
	// ����۷�����Χ���
	ProcEquipEffectAvatar(pNewEquip, n16IndexOld);

	// �������װ����Ϣ
	//SetRating();
	//SendTeamEquipInfo();
}

//--------------------------------------------------------------------------------------------
// ���㻻װӰ��, ��װ��λ����Ϣ���͸��ͻ���
//--------------------------------------------------------------------------------------------
VOID Role::ProcEquipEffectPos(tagEquip *pNewEquip, tagEquip *pOldEquip, EItemConType eConTypeNewDst, EItemConType eConTypeNewSrc)
{
	NET_SIS_item_position_change_extend sendPos;
	sendPos.eConTypeSrc1 = eConTypeNewSrc;
	sendPos.eConTypeSrc2 = eConTypeNewDst;
	sendPos.eConTypeDst1 = eConTypeNewDst;
	sendPos.eConTypeDst2 = eConTypeNewSrc;
	sendPos.n64Serial1 = INVALID_VALUE;
	sendPos.n64Serial2 = INVALID_VALUE;
	sendPos.n16Num1 = 1;
	sendPos.n16Num2 = 1;
	sendPos.n16PosDst1 = INVALID_VALUE;
	sendPos.n16PosDst2 = INVALID_VALUE;

	if(VALID_POINT(pNewEquip))
	{
		sendPos.n64Serial1 = pNewEquip->n64_serial;
		sendPos.n16PosDst1 = pNewEquip->n16Index;
	}

	if(VALID_POINT(pOldEquip))
	{
		sendPos.n64Serial2 = pOldEquip->n64_serial;
		sendPos.n16PosDst2 = pOldEquip->n16Index;
	}

	SendMessage(&sendPos, sendPos.dw_size);
}

//���������ϵļ���
VOID Role::ProcWeaponSkill(tagEquip* pEquip, bool bEquip)
{
	/*ASSERT(VALID_POINT(pEquip));

	if (bEquip)
	{
		for (int i = 0; i < MAX_EQUIP_SKILL_NUM; i++)
		{
			DWORD dwSkillTypeID = pEquip->equipSpec.dwSkillList[i];
			if (dwSkillTypeID != 0)
			{		
				Skill* pSkill = new Skill(Skill::GetIDFromTypeID(dwSkillTypeID), 0, Skill::GetLevelFromTypeID(dwSkillTypeID), 0, 0, TRUE);
				AddSkill(pSkill);
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_EQUIP_SKILL_NUM; i++)
		{
			DWORD dwSkillTypeID = pEquip->equipSpec.dwSkillList[i];
			if (dwSkillTypeID != 0)
			{		
				RemoveSkill(Skill::GetIDFromTypeID(dwSkillTypeID));
			}
		}
	}*/
}

VOID Role::ProcEquipSkill(const tagEquipProto* pEquipProto, bool bEquip)
{
	if (bEquip)
	{
		for (int i = 0; i < EQUIP_SKILL_NUMBER; i++)
		{
			DWORD dwSkillTypeID = pEquipProto->dwSkillID[i];
			if (dwSkillTypeID != 0)
			{		
				Skill* pSkill = new Skill(Skill::GetIDFromTypeID(dwSkillTypeID), 0, Skill::GetLevelFromTypeID(dwSkillTypeID), 0, 0, TRUE);
				AddSkill(pSkill);
			}
		}
	}
	else
	{
		for (int i = 0; i < EQUIP_SKILL_NUMBER; i++)
		{
			DWORD dwSkillTypeID = pEquipProto->dwSkillID[i];
			if (dwSkillTypeID != 0)
			{		
				RemoveSkill(Skill::GetIDFromTypeID(dwSkillTypeID));
			}
		}
	}
}
//--------------------------------------------------------------------------------------------
// ���㻻װӰ����������
//--------------------------------------------------------------------------------------------
VOID Role::ProcEquipEffectAtt(tagEquip *pEquip, bool bEquip, const INT16 n16Index)
{
	ASSERT(VALID_POINT(pEquip));
	
	INT32 nFactor = 1;
	pFun_RegTriggerEquipSet pRegTriggerEquipSet = &Role::RegisterTriggerEquipSet;
	if(!bEquip)
	{
		nFactor = -1;
		pRegTriggerEquipSet = &Role::UnRegisterTriggerEquipSet;
	}

	const tagEquipProto *pEquipProto = pEquip->pEquipProto;

	// ����
	if(M_is_weapon(pEquip))
	{
		// ������������������(�Ѱ��Կ�Ӱ���������)
		//ResetWeaponDmg(*pEquip, bEquip);
		//���������ϵļ���
		ProcWeaponSkill(pEquip, bEquip);

		// ɾ��������buff
		if (IsHaveBuff(HETI_BUFF))
		{
			PetHeti::cancelHeti(this);
			RemoveBuff(HETI_BUFF, TRUE);
		}
	}
	ProcEquipSkill(pEquip->pEquipProto, bEquip);

	//else
	//{
		// ���߻���
	//	ModBaseAttValue(ERA_ArmorEx, pEquip->GetArmor() * nFactor);
		// �Կ�
		//ChangeRoleAtt(pEquip->equipSpec.nEngraveAtt, MAX_ROLEATT_ENGRAVE_EFFECT, ERA_EngraveAtt_Start, nFactor);
	//}


	//�˺�
	//ModBaseAttValue(ERA_ExAttackMin, pEquip->GetMinDmg() * nFactor);
	//ModBaseAttValue(ERA_ExAttackMax, pEquip->GetMaxDmg() * nFactor);
	
	//ModBaseAttValue(ERA_ArmorEx, pEquip->GetArmor() * nFactor);

	// ��Ʒ���޹ص����Լӳ�
	//ChangeRoleAtt(pEquipProto->BaseEffect, MAX_ROLEATT_BASE_EFFECT, nFactor);
	
	// ����
	//ChangeRoleAtt(pEquip->equipSpec.PosyEffect, MAX_ROLEATT_POSY_EFFECT, nFactor);

	// һ������
	//ChangeRoleAtt(pEquip->equipSpec.nRoleAttEffect, X_ERA_ATTA_NUM, ERA_AttA_Start, nFactor);
	
	// ��������
	tagRoleAttEffect eAttTemp[MAX_BASE_ATT];	
	memcpy(eAttTemp, pEquip->equipSpec.EquipAttitionalAtt, sizeof(tagRoleAttEffect) * MAX_BASE_ATT);
	for (int i = 0; i < MAX_BASE_ATT; i++)
	{
		//eAttTemp[i].nValue += eAttTemp[i].nValue * 0.1 * pEquip->equipSpec.byConsolidateLevel;
		eAttTemp[i].nValue += ItemHelper::getConsolidateAtt(eAttTemp[i].nValue, pEquip->equipSpec.byConsolidateLevel);
	}
	ChangeRoleAtt(eAttTemp, MAX_BASE_ATT, nFactor);

	ChangeRoleAtt(&pEquip->equipSpec.EquipAttitionalAtt[MAX_BASE_ATT], MAX_RAND_ATT, nFactor);


	// ��Ƕ
	ChangeRoleAtt(pEquip->equipSpec.dwHoleGemID, pEquip->equipSpec.byHoleNum, nFactor);

	// ����
	ModAttModValue(ERA_Luck, pEquip->equipSpec.byLuck * nFactor);

	SetRating(pEquip->equipSpec.nRating* nFactor);
	// �������� -- ����ֱ��Ӱ�컻װ

	// װ��Buff��
	(this->*pRegTriggerEquipSet)(pEquipProto->dwTriggerID0, pEquipProto->dwBuffID0, n16Index);
	(this->*pRegTriggerEquipSet)(pEquipProto->dwTriggerID1, pEquipProto->dwBuffID1, n16Index);
	(this->*pRegTriggerEquipSet)(pEquipProto->dwTriggerID2, pEquipProto->dwBuffID2, n16Index);

	// ʱװ����
// 	if(pEquip->equipSpec.n16Appearance > 0)
// 	{
// 		ModAttModValue(ERA_Appearance, pEquip->equipSpec.n16Appearance * nFactor);
// 	}

// 	if(pEquip->equipSpec.byRein != 0)
// 	{
// 		ModAttModValue(ERA_Rein, pEquip->equipSpec.byRein * nFactor);
// 	}

// 	if(pEquip->equipSpec.bySavvy != 0)
// 	{
// 		ModAttModValue(ERA_Savvy, pEquip->equipSpec.bySavvy * nFactor);
// 	}

// 	if(pEquip->equipSpec.byFortune != 0)
// 	{
// 		ModAttModValue(ERA_Fortune, pEquip->equipSpec.byFortune * nFactor);
// 	}
	

	// �������Թ����Ϳ���
	//if (pEquip->equipSpec.byConsolidateLevel >= 7)
	//{
	//	if (MIsAttackEquip(pEquip->pEquipProto->eEquipPos))
	//	{
	//		ModAttModValue(ERA_InAttackMin, 50*(pEquip->equipSpec.byConsolidateLevel - 6) * nFactor);
	//	}
	//	else if(MIsDefEquip(pEquip->pEquipProto->eEquipPos))
	//	{
	//		ERoleAttribute att1;
	//		ERoleAttribute att2;
	//		RoleHelper::Class2EAR(GetClass(), att1, att2);
	//		ModAttModValue(att1, (50+25*(pEquip->equipSpec.byConsolidateLevel - 7)) * nFactor);
	//		ModAttModValue(att2, (50+25*(pEquip->equipSpec.byConsolidateLevel - 7)) * nFactor);
	//	}
	//	
	//}

}

//--------------------------------------------------------------------------------------------
// ���㻻װӰ���������, �����͵��ͻ���
//--------------------------------------------------------------------------------------------
VOID Role::ProcEquipEffectAvatar(tagEquip *pNewEquip, INT16 n16IndexOld)
{
	Map *pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(pMap);
		return;
	}

	// ����װ����ʾ
	INT32 nPos = ResetOneEquipDisplay(pNewEquip, n16IndexOld);
	if(!VALID_VALUE(nPos))
	{
		return;
	}

	NET_SIS_avatar_equip_change send;
	send.dw_role_id		= GetID();
	send.EquipDisplay	= GetAvatarEquip().AvatarEquip[nPos];
	send.EquipDisplay.byDisplayPos = nPos;

	pMap->send_big_visible_tile_message(this, &send, send.dw_size);
}

//--------------------------------------------------------------------------------------------
// ����װ����ʾ����������ʾ��λö��ֵ��INVALID_VALUE��ʾ��װ��������ʾ
//--------------------------------------------------------------------------------------------
INT32 Role::ResetOneEquipDisplay(tagEquip* pEquip, INT16 n16Index)
{
	INT32 nPos = GetEquipDisplayPos(n16Index);
	if(!VALID_VALUE(nPos))
	{
		return INVALID_VALUE;
	}

	// �ж���ʱװ������װ��
	bool bFashion = true;
	if(n16Index != EEP_Fashion)
	{
		bFashion = FALSE;
	}

	if(!VALID_POINT(pEquip))
	{		
		SetEquipDisplay(bFashion, nPos, INVALID_VALUE, X_DEFAULT_FLARE_VAL, EC_Null);
	}
	else
	{
		// �������Ч�� --
		SetEquipDisplay(bFashion, nPos, pEquip->dw_data_id, 
						0, 0);
		
		BYTE byEquipEffect = 0;
		if (pEquip->equipSpec.byConsolidateLevel >= 7)
		{
			byEquipEffect = 2;
		}
		else if (pEquip->equipSpec.byConsolidateLevel >= 4)
		{
			byEquipEffect = 1;
		}
			
		if (pEquip->pEquipProto->eEquipPos == EEP_RightHand)
		{
			byEquipEffect = 2;
		}
		SetEquipEffect(nPos, byEquipEffect);
	}

	// �жϸ�����Ƿ���Ҫ��ʾ
	//if(GetDisplaySet().bFashionDisplay != bFashion && !M_is_wapon_by_display_pos(nPos))
	//{
	//	nPos = INVALID_VALUE;
	//}

	return nPos;
}

//--------------------------------------------------------------------------------------------
// �ж�װ����λ����װ���Ƿ���Ҫ��ʾ
//--------------------------------------------------------------------------------------------
INT32 Role::GetEquipDisplayPos(INT16 n16EquipPos)
{
	// ��Ʒ����Ҫ��ʾ
	if(EEP_Finger1 == n16EquipPos || EEP_Finger2 == n16EquipPos
		|| EEP_Waist == n16EquipPos || EEP_Neck == n16EquipPos)
	{
		return INVALID_VALUE;
	}

	// ת��Ϊ��ʾλ��
	switch(n16EquipPos)
	{
	// ����
	case EEP_RightHand:
		return EAE_RWeapon;
		break;

	// ʱװ
	case EEP_Fashion:
		return EAE_Upper;

	// ����
	case EEP_Body:
		return EAE_Upper;
		break;
	case EEP_Body1:
		return EAE_Back;
		break;

	}

	return INVALID_VALUE;
}

//--------------------------------------------------------------------------------------------
// ����ĥ��������˺�Ӱ��.����ֵΪ�������˺�Ӱ��İٷֱ�ֵ.
//--------------------------------------------------------------------------------------------
FLOAT Role::CalAbrasionEffect(const tagEquip &Equip)
{
	INT32 nCurNewness = Equip.pEquipProto->n16Newness 
							- Equip.nUseTimes / ABRASION2USETIMES;
	nCurNewness = max(nCurNewness, 0);

	/*	AΪ������ǰո�¶ȣ�BΪ����ԭʼ�˺�
		A=200-999	�����˺�����꣩�ӳ� = 8% * B
		A=121-199	�����˺�����꣩�ӳ� =��A-120��* 0.1% * B
		A=80-120	�����˺�����꣩�ӳ� = 0
		A=50-79		�����˺�����꣩�ӳ� = -5% * B
		A=20-49		�����˺�����꣩�ӳ� = -15% * B
		A=5-19		�����˺�����꣩�ӳ� = -25% * B
		A=0-4		�����˺�����꣩�ӳ� = -50% * B	*/

	if(nCurNewness >= 200)
	{
		return 0.08f;
	}

	if(nCurNewness >= 121)
	{
		return (nCurNewness - 120) * 0.001f;
	}

	if(nCurNewness >= 80)
	{
		return 0.0f;
	}

	if(nCurNewness >= 50)
	{
		return -0.05f;
	}

	if(nCurNewness >= 20)
	{
		return -0.15f;
	}

	if(nCurNewness >= 5)
	{
		return -0.25f;
	}

	return -0.5f;
}

//--------------------------------------------------------------------------------------------
// ���������˺� -- ��װʱ����.
//--------------------------------------------------------------------------------------------
VOID Role::ResetWeaponDmg(const tagEquip &Equip, BOOL bEquip)
{
	if(bEquip)	// ����
	{
		//FLOAT fDmgPct = 1.0f + CalAbrasionEffect(Equip);
		
		//INT nMinDmg = Equip.equipSpec.n16MinDmg + Equip.equipSpec.nLevel - 1;
		//INT nMaxDmg = Equip.equipSpec.n16MaxDmg + Equip.equipSpec.nLevel - 1;

		//nMinDmg = CalConsolidate(nMinDmg, Equip.equipSpec.byConsolidateLevel, CONSOLIDATE_PARAM);
		//nMaxDmg = CalConsolidate(nMaxDmg, Equip.equipSpec.byConsolidateLevel, CONSOLIDATE_PARAM);
		//INT nMinDmgIn = CalConsolidate(Equip.equipSpec.n16MinDmgIn, Equip.equipSpec.byConsolidateLevel, CONSOLIDATE_PARAM);
		//INT nMaxDmgIn = CalConsolidate(Equip.equipSpec.n16MaxDmgIn, Equip.equipSpec.byConsolidateLevel, CONSOLIDATE_PARAM);

		//nMinDmg = CalConsolidate(nMinDmg, Equip.equipSpec.nLevel, WEAPON_LEVEL_PARAM);
		//nMaxDmg = CalConsolidate(nMaxDmg, Equip.equipSpec.nLevel, WEAPON_LEVEL_PARAM);

		//ModBaseAttValue(ERA_ExAttackMin, Equip.GetMinDmg());
		//ModBaseAttValue(ERA_ExAttackMax, Equip.GetMaxDmg());
		//SetBaseAttValue(ERA_InAttackMin, (INT)(nMinDmgIn * fDmgPct));
		//SetBaseAttValue(ERA_InAttackMax, (INT)(nMaxDmgIn * fDmgPct));

		// �Կ�
		//ChangeRoleAtt(Equip.equipSpec.nEngraveAtt, MAX_ROLEATT_ENGRAVE_EFFECT, ERA_EngraveAtt_Start, 1);
	}
	else	// ����
	{
		//ModBaseAttValue(ERA_ExAttackMin, -Equip.GetMinDmg());
		//ModBaseAttValue(ERA_ExAttackMax, -Equip.GetMaxDmg());
		//SetBaseAttValue(ERA_InAttackMin, 0);
		//SetBaseAttValue(ERA_InAttackMax, 0);

		// �Կ�
		//ChangeRoleAtt(Equip.equipSpec.nEngraveAtt, MAX_ROLEATT_ENGRAVE_EFFECT, ERA_EngraveAtt_Start, -1);
	}
}
//����ǿ�����ǵȼ�Ӱ��
INT Role::CalConsolidate(INT16 nBaseValue, BYTE byConLevel, float fParam)
{
	if (nBaseValue == 0)
		return 0;

	float result = nBaseValue;

	for (BYTE i = 0; i < byConLevel; ++i)
	{
		result = result + (result * fParam);
	}
	
	result += byConLevel;

	return (int)result;
}

tagEquip*	Role::GetWeapon()
{
	return GetItemMgr().GetEquipBarEquip((INT16)EEP_RightHand);
}
//--------------------------------------------------------------------------------------------
// ���������˺� -- ո�¶ȱ仯ʱ����.
//--------------------------------------------------------------------------------------------
VOID Role::ResetWeaponDmg(tagEquip &Equip)
{
	// ����ո�¶ȱ仯֮ǰ�������˺�Ӱ��
	--Equip.nUseTimes;
	FLOAT fDmgPctPre = CalAbrasionEffect(Equip);

	// ��ǰո�¶ȶ������˺�Ӱ��
	++Equip.nUseTimes;
	FLOAT fDmgPct = CalAbrasionEffect(Equip);

	// Ӱ�첻ͬ������������
	if(fDmgPctPre != fDmgPct)
	{
		fDmgPct += 1.0f;

		//SetBaseAttValue(ERA_ExAttackMin, (INT)(Equip.equipSpec.n16MinDmg * fDmgPct));
		//SetBaseAttValue(ERA_ExAttackMax, (INT)(Equip.equipSpec.n16MaxDmg * fDmgPct));
		//SetBaseAttValue(ERA_InAttackMin, (INT)(Equip.equipSpec.n16MinDmgIn * fDmgPct));
		//SetBaseAttValue(ERA_InAttackMax, (INT)(Equip.equipSpec.n16MaxDmgIn * fDmgPct));
	}
}

//--------------------------------------------------------------------------------------------
// ����װ������Ӱ��,���޸Ķ�Ӧ�����Լӳ�ֵ.
//--------------------------------------------------------------------------------------------
VOID Role::ChangeRoleAtt(const tagRoleAttEffect Effect[], INT32 nArraySz, INT32 nFactor)
{
	for(INT32 i=0; i<nArraySz; ++i)
	{
		if(Effect[i].eRoleAtt <= ERA_Null || Effect[i].eRoleAtt >= ERA_End)
		{
			//ASSERT(Effect[i].eRoleAtt == ERA_Null);
			continue;
		}
		
		if(M_is_value_pct(Effect[i].nValue))		// �ٷ���
		{
			ModAttModValuePct(Effect[i].eRoleAtt, M_value_pct_trans(Effect[i].nValue) * nFactor);
		}
		else	// ��ֵ
		{
			ModAttModValue(Effect[i].eRoleAtt, Effect[i].nValue * nFactor);
		}
	}
}

//--------------------------------------------------------------------------------------------
// ����װ������Ӱ��,���޸Ķ�Ӧ�����Լӳ�ֵ.
//--------------------------------------------------------------------------------------------
VOID Role::ChangeRoleAtt(const INT32 nValue[], INT32 nArraySz, INT32 nAttStart, INT32 nFactor)
{
	ASSERT(nAttStart > ERA_Null && nAttStart + nArraySz < ERA_End);
	
	for(INT32 i=0; i<nArraySz; ++i)
	{
		if(M_is_value_pct(nValue[i]))		// �ٷ���
		{
			ModAttModValuePct(nAttStart + i, M_value_pct_trans(nValue[i]) * nFactor);
		}
		else	// ��ֵ
		{
			ModAttModValue(nAttStart + i, nValue[i] * nFactor);
		}
	}
}

//--------------------------------------------------------------------------------------------
// ����װ������Ӱ��,���޸Ķ�Ӧ�����Լӳ�ֵ. -- ��ʯӰ��
//--------------------------------------------------------------------------------------------
VOID Role::ChangeRoleAtt(const DWORD dwValue[], INT32 nArraySz, INT32 nFactor)
{
	ASSERT(nArraySz <= MAX_EQUIPHOLE_NUM);

	for(INT32 i=0; i<nArraySz; ++i)
	{
		if(0 == dwValue[i] || ERA_Null == dwValue[i])
		{
			continue;
		}

		const tagConsolidateItem *pGemProto = AttRes::GetInstance()->GetConsolidateProto(dwValue[i]);
		if(!VALID_POINT(pGemProto))
		{
			ASSERT(VALID_POINT(pGemProto));
			continue;
		}

		ChangeRoleAtt((tagRoleAttEffect*)pGemProto->tagRoleAtt, MAX_CONSOLIDATE_ROLEATT, nFactor);
	}
}

//--------------------------------------------------------------------------------------------
// װ����buff����
//--------------------------------------------------------------------------------------------
VOID Role::ProcEquipBuffTrigger(DWORD dwBuffID, BOOL bEquip)
{
	INT nTmp;
	ERoleAttribute	eTmp;
	INT nFactor = (bEquip) ? 1 : -1;
	
	const tagBuffProto *pBuffProto = AttRes::GetInstance()->GetBuffProto(dwBuffID);
	if( !VALID_POINT(pBuffProto) )
	{
		m_att_res_caution(_T("buff proto"), _T("BuffID"), dwBuffID);
		return;
	}
	if (bEquip)
	{
		TryAddBuff(this, pBuffProto, NULL, NULL, NULL);
	}
	else
	{
		RemoveBuff(dwBuffID/100, TRUE);
	}
	

	//if( pBuffProto->mapRoleAttMod.size() > 0 )
	//{
	//	package_map<ERoleAttribute, INT>::map_iter iter = pBuffProto->mapRoleAttMod.begin();
	//	while( pBuffProto->mapRoleAttMod.find_next(iter, eTmp, nTmp) )
	//	{
	//		ModAttModValue(eTmp, nTmp * nFactor);
	//	}
	//}

	//if( pBuffProto->mapRoleAttModPct.size() > 0 )
	//{
	//	package_map<ERoleAttribute, INT>::map_iter iter = pBuffProto->mapRoleAttModPct.begin();
	//	while( pBuffProto->mapRoleAttModPct.find_next(iter, eTmp, nTmp) )
	//	{
	//		ModAttModValuePct(eTmp, nTmp * nFactor);
	//	}
	//}
}
// ����,ȡ��ĳ��װ��������
VOID Role::EquipEffect(tagEquip* pEquip, BOOL bEquip)
{
	if (bEquip)
	{
		ProcEquipEffectAtt(pEquip, TRUE, pEquip->n16Index);
		m_Suit.Add(pEquip, pEquip->n16Index);
	}
	else
	{
		ProcEquipEffectAtt(pEquip, FALSE, pEquip->n16Index);
		m_Suit.Remove(pEquip, pEquip->n16Index);
	}

	RecalAtt();

}
// ��ɫ�ɼ����ܼӳ�
INT Role::CalGatherRate( Creature* pCreature )
{
	Skill* pGatherSkill = NULL;
	if ( pCreature->IsNatuRes() )
		pGatherSkill = GetSkill(GATHER_SKILL_MINING);
	else if ( pCreature->IsManRes() )
		pGatherSkill = GetSkill(GATHER_SKILL_HARVEST);
	else if ( pCreature->IsGuildNatuRes() )
		pGatherSkill = GetSkill(GATHER_SKILL_GMINING);
	else if ( pCreature->IsGuildManRes() )
		pGatherSkill = GetSkill(GATHER_SKILL_GHARVEST);
	if ( !VALID_POINT(pGatherSkill) )
		return 0;

	INT nSkillLvl = pGatherSkill->get_level();

	if (nSkillLvl <= 0)
		return 0;

	// �õ��ջ��ɫ��Ե 	
	FLOAT fFortune = (FLOAT)GetAttValue(ERA_Fortune);

	// ��������ʼӳ�	�������ʼӳ�=5%�����ջ��ܵȼ�-1��/10+��ɫ��Ե/1000
	INT nAddRatPct = INT((5.0f * FLOAT(nSkillLvl - 1) + fFortune) * 10.0f);

	return nAddRatPct;
};

// ����ʵ��ֵ���������־���ӳ����Լ��㣩
INT Role::CalRealGainExp( INT nSrcExp, FLOAT fAdd /*= 0*/ )
{
	// ԭ����/��Ǯ/���� * (1+�ӳ�/10000)
	return INT(nSrcExp * (1 + GetAttValue(ERA_Exp_Add_Rate)/10000.0f) + fAdd);
	
}

//------------------------------------------------------------------------------
// װ��ά����ؼ���
//------------------------------------------------------------------------------
DWORD Role::EquipRepair(INT64 n64SerialEquip, DWORD	dwNPCID)
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
	{
		ASSERT(VALID_POINT(pMap));
		return INVALID_VALUE;
	}

	Creature* pNPC = pMap->find_creature(dwNPCID);
	if( !VALID_POINT(pNPC) ) 
		return E_Compose_NPC_Not_Exist;

	if( FALSE == pNPC->CheckNPCTalkDistance(this) )
		return E_Compose_NPC_Distance;


	/*if( FALSE == pNPC->IsFunctionNPC(EFNPCT_Repair) )
	return E_Repair_NPCCanNotRepair;*/
	
	// ������װ��
	if (n64SerialEquip != INVALID_VALUE)
	{
		tagEquip *pEquip = (tagEquip*)GetItem(n64SerialEquip);
		if(!VALID_POINT(pEquip))
			return E_Repair_Equip_Not_Exist;

		if(!MIsEquipment(pEquip->dw_data_id))
			return E_Repair_NotEquipment;

		if (pEquip->GetNewess() == 0)
		{
			return E_Repair_NotRepair;
		}

		INT64 n64RepairPrice = FormulaHelper::GetEquipRepairPrice(pEquip);
		// �����ҽ�Ǯ�Ƿ��㹻
		if(GetCurMgr().GetBagSilver() < n64RepairPrice)
			return E_Repair_NotEnough_Money;

		// ��Ǯ����
		GetCurMgr().DecBagSilver(n64RepairPrice, elcid_npc_repair);

		EquipRepair(pEquip);
		
		
	}
	else//������������װ��
	{
		for (int i = 0; i < EEP_MaxEquip; i++)
		{
			tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)i);
			if (!VALID_POINT(pEquip)) continue;

			if (pEquip->GetNewess() == 0)
				continue;
			
			INT64 n64RepairPrice = FormulaHelper::GetEquipRepairPrice(pEquip);
			// �����ҽ�Ǯ�Ƿ��㹻
			if(GetCurMgr().GetBagSilver() < n64RepairPrice)
				return E_Repair_NotEnough_Money;

			// ��Ǯ����
			GetCurMgr().DecBagSilver(n64RepairPrice, elcid_npc_repair);

			EquipRepair(pEquip);
		}
	}

	GetAchievementMgr().UpdateAchievementCriteria(eta_equip_xiuli, 1);	

	return E_Success;
}

VOID Role::EquipRepair(tagEquip* pEquip)
{
	ASSERT(VALID_POINT(pEquip));
	
	INT16 nCurNewness = pEquip->GetEquipNewness();

	pEquip->ZeroEuqipUseTimes();
	
	// ֮ǰû�;�,�Ҵ�������,�����������
	if (nCurNewness <= 0 && pEquip->eConType == EICT_Equip)
	{
		EquipEffect(pEquip, TRUE);

		//SetRating();
		//SendTeamEquipInfo();

	}

	NET_SIS_newess_change send;
	send.n64EquipSerial = pEquip->n64_serial;
	send.nAttackTimes = pEquip->nUseTimes;
	SendMessage(&send, send.dw_size);

	GetItemMgr().UpdateEquipSpec(*pEquip);
}


BOOL Role::IsFriend(DWORD dwFriendID)
{
	return VALID_POINT(GetFriendPtr(dwFriendID));
}