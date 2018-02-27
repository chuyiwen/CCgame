/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//套装

#pragma once

struct tagEquip;

class Role;
//--------------------------------------------------------------------------------------------

/******************************* 类定义 **********************************/
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
	// 判断该件套装部件是否需要统计
	//--------------------------------------------------------------------------------------------
	BOOL IsNeedCount(const tagEquip *pEquip, const tagEquipProto *pEquipProto, 
					INT nSuitIndex, INT16 n16IndexOther);

	//--------------------------------------------------------------------------------------------
	// 判断是否是套装部件
	//--------------------------------------------------------------------------------------------
	BOOL IsSuitPart(const tagEquip *pEquip, const tagEquipProto *pEquipProto, INT nSuitIndex, INT16 n16OldIndex);

	//--------------------------------------------------------------------------------------------
	// 如果是武器或者戒指，得到另一个件的位置
	//--------------------------------------------------------------------------------------------
	INT16 GetOtherEquipPos(const tagEquip *pEquip, INT16 n16OldIndex);

private:
	Role				*m_pRole;
	package_map<DWORD, INT>	m_mapSuitNum;	// <dwSuitID, nEquipNum>
};


/******************************* 内联实现 **********************************/

//--------------------------------------------------------------------------------------------
// 套装套数
//--------------------------------------------------------------------------------------------
inline INT Suit::GetSuitNum()
{
	return m_mapSuitNum.size();
}

//--------------------------------------------------------------------------------------------
// 某套套装已装备件数
//--------------------------------------------------------------------------------------------
inline INT Suit::GetSuitEquipNum(DWORD dwSuitID)
{
	return m_mapSuitNum.find(dwSuitID);
}

//--------------------------------------------------------------------------------------------
// 判断是否是套装部件
//--------------------------------------------------------------------------------------------
inline BOOL Suit::IsSuitPart(const tagEquip *pEquip, const tagEquipProto *pEquipProto, 
							 INT nSuitIndex, INT16 n16OldIndex)
{
	// 判断最低品级
	if(pEquipProto->bySuitMinQlty[nSuitIndex] > pEquip->equipSpec.byQuality)
	{
		return FALSE;
	}

	// 如果是武器或者戒指，还需要判断另一个装备位置装备
	INT16 n16IndexOther = GetOtherEquipPos(pEquip, n16OldIndex);
	if(n16IndexOther != INVALID_VALUE
		&& !IsNeedCount(pEquip, pEquipProto, nSuitIndex, n16IndexOther))
	{
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
// 如果是武器或者戒指，得到另一个件的位置
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