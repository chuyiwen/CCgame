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

#pragma once

struct tagEquip;

class Role;
//--------------------------------------------------------------------------------------------

/******************************* �ඨ�� **********************************/
class Suit
{
public:
	Suit(Role *pRole);
	~Suit();

	VOID Add(const tagEquip *pEquip, INT16 n16EquipPos, BOOL bSend2Client = TRUE);
	VOID Remove(const tagEquip *pEquip, INT16 n16OldIndex);

public:
	INT GetSuitNum();
	INT GetSuitEquipNum(DWORD dwSuitID);
	VOID InitSendInitState(BYTE *pData);
	
	BOOL IsNeedTriggerCount(const tagSuitProto* pSuitProto) { return pSuitProto->nEffectConutPos != INVALID_VALUE; }
	INT GetSuitTriggerCount(const tagSuitProto* pSuitProto);
	VOID ConsumeTriggerCount(const tagSuitProto* pSuitProto);
private:
	//--------------------------------------------------------------------------------------------
	// �жϸü���װ�����Ƿ���Ҫͳ��
	//--------------------------------------------------------------------------------------------
	BOOL IsNeedCount(const tagEquip *pEquip, const tagEquipProto *pEquipProto, 
					INT nSuitIndex, INT16 n16IndexOther);

	//--------------------------------------------------------------------------------------------
	// �ж��Ƿ�����װ����
	//--------------------------------------------------------------------------------------------
	BOOL IsSuitPart(const tagEquip *pEquip, const tagEquipProto *pEquipProto, INT nSuitIndex, INT16 n16OldIndex);

	//--------------------------------------------------------------------------------------------
	// ������������߽�ָ���õ���һ������λ��
	//--------------------------------------------------------------------------------------------
	INT16 GetOtherEquipPos(const tagEquip *pEquip, INT16 n16OldIndex);

private:
	Role				*m_pRole;
	package_map<DWORD, INT>	m_mapSuitNum;	// <dwSuitID, nEquipNum>
};


/******************************* ����ʵ�� **********************************/

//--------------------------------------------------------------------------------------------
// ��װ����
//--------------------------------------------------------------------------------------------
inline INT Suit::GetSuitNum()
{
	return m_mapSuitNum.size();
}

//--------------------------------------------------------------------------------------------
// ĳ����װ��װ������
//--------------------------------------------------------------------------------------------
inline INT Suit::GetSuitEquipNum(DWORD dwSuitID)
{
	return m_mapSuitNum.find(dwSuitID);
}

//--------------------------------------------------------------------------------------------
// �ж��Ƿ�����װ����
//--------------------------------------------------------------------------------------------
inline BOOL Suit::IsSuitPart(const tagEquip *pEquip, const tagEquipProto *pEquipProto, 
							 INT nSuitIndex, INT16 n16OldIndex)
{
	// �ж����Ʒ��
	if(pEquipProto->bySuitMinQlty[nSuitIndex] > pEquip->equipSpec.byQuality)
	{
		return FALSE;
	}

	// ������������߽�ָ������Ҫ�ж���һ��װ��λ��װ��
	INT16 n16IndexOther = GetOtherEquipPos(pEquip, n16OldIndex);
	if(n16IndexOther != INVALID_VALUE
		&& !IsNeedCount(pEquip, pEquipProto, nSuitIndex, n16IndexOther))
	{
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
// ������������߽�ָ���õ���һ������λ��
//--------------------------------------------------------------------------------------------
inline INT16 Suit::GetOtherEquipPos(const tagEquip *pEquip, INT16 n16OldIndex)
{
	if(M_is_ring(pEquip))
	{
		return (EEP_Finger1 == n16OldIndex ? EEP_Finger2 : EEP_Finger1);
	}
	//else if(MIsWeapon(pEquip))
	//{
	//	return (EEP_RightHand == n16OldIndex ? EEP_LeftHand : EEP_RightHand);
	//}

	return INVALID_VALUE;
}