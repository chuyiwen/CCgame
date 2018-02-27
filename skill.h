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
*	@file		skill.h
*	@author		lc
*	@date		2010/09/08	initial
*	@version	0.0.1.0
*	@brief		����
*/
#pragma once

#include "skill_buff.h"
#include "skill_trigger.h"

//-----------------------------------------------------------------------------------------
// ���ܵȼ��ı䷽ʽ
//-----------------------------------------------------------------------------------------
enum ESkillLevelChange
{
	ESLC_Learn = 0,		// Ͷ��
	ESLC_Self = 1,		// �̶�
	ESLC_Temp = 2,		// ��ʱ
};

//-----------------------------------------------------------------------------------------
// �������ܶԸü��ܵ�Ӱ��
//-----------------------------------------------------------------------------------------
struct tagSkillMod
{
	// ��������Ӱ��
	FLOAT			fOPDistMod;								// ���þ���ӳ�
	FLOAT			fOPRadiusMod;							// ���÷�Χ�ӳ�
	INT				nPrepareTimeMod;						// ����ʱ��ӳɣ����룩
	INT				nCoolDownMod;							// ��ȴʱ��ӳ�
	INT				nEnmityMod;								// ���ܳ�޼ӳ�
	INT				nEnmityModPct;							// ���ܳ�ްٷֱȼӳ�
	FLOAT			fEnmityParamMod;						// ���ϵ���ӳ�
	INT				nHitMod;								// ���������ʼӳ�
	INT				nHitModPct;								// ���������ʰٷֱȼӳ�
	INT				nCritMod;								// ���������ʼӳ�
	INT				nCritModPct;							// ���������ʰٷֱȼӳ�

	// �˺�
	INT				nChannelDmgMod[MAX_CHANNEL_TIMES];		// �ܵ��˺�ÿһ�ε��˺����ӳ�
	INT				nChannelDmgModPct[MAX_CHANNEL_TIMES];	// �ܵ��˺�ÿһ�ε��˺����İٷֱȼӳ�

	// ״̬
	DWORD			dwBuffTypeID[MAX_BUFF_PER_SKILL];		// ��������buff
	DWORD			dwTriggerID[MAX_BUFF_PER_SKILL];		// ����������buff������

	// ����
	INT				nSkillCostMod[ESCT_End];				// �������ļӳ�
	INT				nSkillCostModPct[ESCT_End];				// �������İٷֱȼӳ�

	// ��������Ӱ��
	mutable package_map<ERoleAttribute, INT>	mapRoleAttMod;		// �������ܶԸü�����ɵ���������Ӱ��ļӳ�
	mutable package_map<ERoleAttribute, INT>	mapRoleAttModPct;	// �������ܶԸü�����ɵ���������Ӱ��İٷֱȼӳ�

	tagSkillMod()
	{
		fOPDistMod = 0.0f;
		fOPRadiusMod = 0.0f;
		nPrepareTimeMod = 0;
		nCoolDownMod = 0;
		nEnmityMod = 0;
		nEnmityModPct = 0;
		nHitMod = 0;
		nHitModPct = 0;
		nCritMod = 0;
		nCritModPct = 0;
		fEnmityParamMod = 0.0f;

		memset(dwBuffTypeID, 0xFF, sizeof(dwBuffTypeID));
		memset(dwTriggerID, 0xFF, sizeof(dwTriggerID));
		ZeroMemory(nChannelDmgMod, sizeof(nChannelDmgMod));
		ZeroMemory(nChannelDmgModPct, sizeof(nChannelDmgModPct));
		ZeroMemory(nSkillCostMod, sizeof(nSkillCostMod));
		ZeroMemory(nSkillCostModPct, sizeof(nSkillCostModPct));
	}
};

class SkillScript;

//--------------------------------------------------------------------------------------------
// ������
//--------------------------------------------------------------------------------------------
class Skill
{
public:
	//----------------------------------------------------------------------------------------
	// Constructor
	//----------------------------------------------------------------------------------------
	Skill();
	Skill(DWORD dwID, INT nSelfLevel, INT nLearnLevel, INT nCoolDown, INT nProficiency, BOOL bFromeEquip = FALSE);
	~Skill()	{ SAFE_DELETE(m_pMod); }

	//----------------------------------------------------------------------------------------
	// һЩͨ�ú���
	//----------------------------------------------------------------------------------------
	static DWORD			GetIDFromTypeID(DWORD dw_data_id)		{ return dw_data_id / 100; }
	static INT				GetLevelFromTypeID(DWORD dw_data_id)	{ return dw_data_id % 100; }
	static DWORD			CreateTypeID(DWORD dwID, INT nLevel){ return dwID * 100 + nLevel; }

	//----------------------------------------------------------------------------------------
	// ��ʼ�������£��������Ϣ
	//----------------------------------------------------------------------------------------
	BOOL					Init(DWORD dwID, INT nSelfLevel, INT nLearnLevel, INT nCoolDown, INT nProficiency, BOOL bFromOther);
	VOID					Update();
	VOID					InitSkillSave(OUT s_skill_save* pSkillSave);
	VOID					GenerateMsgInfo(tagSkillMsgInfo* const pMsgInfo);

	//----------------------------------------------------------------------------------------
	// ����Get
	//----------------------------------------------------------------------------------------
	DWORD					GetID()					{ return m_dwID; }
	DWORD					GetTypeID()				{ return m_pProto->dwID; }
	INT						get_level()				{ return m_nLevel; }
	INT						GetLearnLevel()			{ return m_nLearnLevel; }
	INT						GetSelfLevel()			{ return m_nSelfLevel; }
	INT						GetTempAddLevel()		{ return m_nTempAddLevel; }
	INT						GetProficiency()		{ return m_nProficiency; }
	INT						GetMaxLevel()			{ return m_pProto->nMaxLevel; }
	INT						GetMaxLearnLevel()		{ return m_pProto->nMaxLearnLevel; }
	INT						GetCoolDownCountDown()	{ return m_nCoolDownCountDown; }
	ESkillType				GetType()				{ return m_pProto->eType; }
	ESkillTypeEx			GetTypeEx()				{ return (ESkillTypeEx)m_pProto->nType2; }
	ESkillTypeEx2			GetTypeEx2()			{ return (ESkillTypeEx2)m_pProto->nType3; }
	ESkillUseType			GetUseType()			{ return m_pProto->eUseType; }
	ESkillDmgType			GetDmgType()			{ return m_pProto->eDmgType; }
	ESkillDistType			GetDistType()			{ return m_pProto->eDistType; }
	ESkillTargetType		GetTargetType()			{ return m_pProto->eTargetType; }
	//DWORD					GetTargetSkillID()		{ return m_pProto->dwTargetSkillID; }
	DWORD					GetTargetBuffID()		{ return m_pProto->dwTargetBuffID; }
	DWORD					GetTargetTriggerID()	{ return m_pProto->dwTargetTriggerID; }
	ESkillOPType			GetOPType()				{ return m_pProto->eOPType; }
	INT						GetDmgTimes()			{ return m_pProto->nDmgTimes; }
	ETalentType				GetTalentType()			{ return m_pProto->eTalentType; }
	INT						GetLevelUpExp()			{ return m_pProto->nLevelUpExp; }
	INT						GetLevelUpType()		{ return m_pProto->eLevelUpType; }
	BOOL					GetHasPet()				{ return m_pProto->bHasPet; }
	INT						GetHitNumber()			{ return m_pProto->nHitNumber; }

	INT						GetDmg(INT nIndex);
	INT						GetDmgMod(INT nIndex);
	INT						GetDmgModPct(INT nIndex);
	INT						GetDmgTime(INT nIndex);
	FLOAT					GetOPDist();
	FLOAT					GetOPRadius();
	INT						GetPrepareTime();
	INT						GetCoolDown();
	FLOAT					GetHit();
	FLOAT					GetCrit();
	INT						GetCost(ESkillCostType eCostType);
	VOID					GetCostItem(DWORD& dwItemID, INT& n_num);
	DWORD					GetBuffTypeID(INT nIndex);
	DWORD					GetTriggerID(INT nIndex);
	INT						GetEnmity();
	FLOAT					GetEnmityParam();
	const SkillScript*		GetSkillScript()		{ return m_pScript; }

	const tagSkillProto*	GetProto()				{ return m_pProto; }
	const tagSkillMod*		GetMod()				{ return m_pMod; }
	const Unit*				GetOwner()				{ return m_pOwner; }
	
	//add by guohui
	INT						GetPilotTime()			{return m_pProto->nPilotTime;}
	INT						GetPilotNum()			{return m_pProto->nPilotNum;}

	BOOL					IsAffected()			{ return VALID_POINT(m_pMod); }
	BOOL					IsActive()				{ return ESUT_Active == GetUseType(); }
	BOOL					IsPassive()				{ return ESUT_Passive == GetUseType(); }
	BOOL					IsOrdAttackSkill()		{ return ESDGT_Ordinary == GetDmgType(); }
	//BOOL					IsExAttackSkill()		{ ESkillDmgType eType = GetDmgType(); return ( ESDGT_Bleeding == eType || ESDGT_Brunt == eType || ESDGT_Bang == eType || ESDGT_Ordinary == eType ); }
	//BOOL					IsInAttackSkill()		{ ESkillDmgType eType = GetDmgType(); return ( ESDGT_Poison == eType || ESDGT_Thinker == eType || ESDGT_Injury == eType ); }
	//BOOL					IsStuntSkill()			{ return ESDGT_Stunt == GetDmgType(); }
	BOOL					IsMelee()				{ return ESDT_Melee == GetDistType(); }
	BOOL					IsRanged()				{ return ESDT_Ranged == GetDistType(); }
	BOOL					IsInner()				{ return ESDT_Inner == GetDistType(); }
	BOOL					IsFriendly()			{ return m_pProto->bFriendly; }
	BOOL					IsHostile()				{ return m_pProto->bHostile; }
	BOOL					IsInDependent()			{ return m_pProto->bIndependent; }
	BOOL					IsMoveCancel()			{ return m_pProto->bMoveCancel; }
	BOOL					IsCanModAtt();

	//--------------------------------------------------------------------------------
	// ����Set
	//--------------------------------------------------------------------------------
	VOID					SetOwner(Unit* pOwner)		{ m_pOwner = pOwner; }
	VOID					AddProficiency(INT nValue)	{ m_nProficiency += nValue; }
	VOID					SetProficiency(INT nValue)	{ m_nProficiency = nValue; }

	//--------------------------------------------------------------------------------
	// ��������ӳ�
	//--------------------------------------------------------------------------------
	BOOL					SetMod(const tagSkillProto* pProto);
	BOOL					UnSetMod(const tagSkillProto* pProto);

	//--------------------------------------------------------------------------------
	// �������Զ���������Ӱ��
	//--------------------------------------------------------------------------------
	VOID					SetOwnerMod();
	VOID					UnsetOwnerMod();

	//---------------------------------------------------------------------------------
	// ��������������
	//---------------------------------------------------------------------------------
	VOID					IncLevel(ESkillLevelChange eType, INT nInc=1);
	VOID					DecLevel(ESkillLevelChange eType, INT nDec=1);

	//--------------------------------------------------------------------------------
	// ��ȴ
	//--------------------------------------------------------------------------------
	VOID					StartCoolDown()		{ m_nCoolDownCountDown += GetCoolDown(); }
	VOID					ClearCoolDown()		{ m_nCoolDownCountDown = 0; }
	BOOL					CountDownCoolDown();	
	
	BOOL					IsFromeEquip() { return m_bFromOther; }
private:
	//--------------------------------------------------------------------------
	// ����ĳЩ����buff
	//--------------------------------------------------------------------------
	VOID SetSkillBuff(const tagSkillProto* pProto);

	//--------------------------------------------------------------------------
	// ����ĳЩ����buff
	//--------------------------------------------------------------------------
	VOID UnSetSkillBuff(const tagSkillProto* pProto);

	//--------------------------------------------------------------------------
	// ע�Ἴ�ܱ���Ĵ���������
	//--------------------------------------------------------------------------
	VOID RegisterTriggerEvent();

	//--------------------------------------------------------------------------
	// ��ע�Ἴ�ܱ���Ĵ���������
	//--------------------------------------------------------------------------
	VOID UnRegisterTriggerEvent();


private:
	DWORD					m_dwID;						// ����ID����ID��
	INT						m_nLevel;					// ���ܵ�ǰ�ȼ������õȼ�+Ͷ�ŵȼ�+��ʱ�ȼ���

	INT						m_nLearnLevel;				// ���ܵ�ǰͶ�ŵȼ�
	INT						m_nSelfLevel;				// �������õȼ�
	INT						m_nTempAddLevel;			// ��ʱ���ϵĵȼ�						
	INT						m_nProficiency;				// ����������

	INT						m_nCoolDownCountDown;		// ������ȴ
	BOOL					m_bFromOther;				// �Ƿ��Ǵӱ��;�����

	const SkillScript*		m_pScript;					// ���ܽű�
	const tagSkillProto*	m_pProto;					// ���ܾ�̬����
	mutable tagSkillMod*	m_pMod;						// ��������Ӱ�죨���û���κμ���Ӱ��������ֵΪNULL��
	Unit*					m_pOwner;					// ����������
};

//---------------------------------------------------------------------------------
// ���캯��
//---------------------------------------------------------------------------------
inline Skill::Skill() 
:	m_dwID(INVALID_VALUE), m_nLevel(INVALID_VALUE), m_nLearnLevel(INVALID_VALUE), 
	m_nSelfLevel(INVALID_VALUE), m_nTempAddLevel(INVALID_VALUE), m_nCoolDownCountDown(INVALID_VALUE),
	m_pProto(NULL), m_pMod(NULL), m_pOwner(NULL),m_nProficiency(INVALID_VALUE), m_pScript(NULL)
{
}

inline Skill::Skill(DWORD dwID, INT nSelfLevel, INT nLearnLevel, INT nCoolDown, INT nProficiency, BOOL bFromeEquip)
:	m_dwID(INVALID_VALUE), m_nLevel(INVALID_VALUE), m_nLearnLevel(INVALID_VALUE), 
	m_nSelfLevel(INVALID_VALUE), m_nTempAddLevel(INVALID_VALUE), m_nCoolDownCountDown(INVALID_VALUE),
	m_pProto(NULL), m_pMod(NULL), m_pOwner(NULL),m_nProficiency(INVALID_VALUE), m_pScript(NULL),
	m_bFromOther(bFromeEquip)
{
	Init(dwID, nSelfLevel, nLearnLevel, nCoolDown, nProficiency,bFromeEquip);
}

//---------------------------------------------------------------------------------
// ����������mod����Ҫ�ı䣬���ü���ԭ��buffҪ���⴦��һ�£�
//---------------------------------------------------------------------------------
inline VOID Skill::IncLevel(ESkillLevelChange eType, INT nInc)
{
	ASSERT( nInc > 0 );
		
	INT nNewLevel = min(m_pProto->nMaxLevel, m_nLearnLevel + m_nSelfLevel + m_nTempAddLevel + nInc);
	if( nNewLevel == m_nLevel ) return;

	const tagSkillProto* pNewProto = AttRes::GetInstance()->GetSkillProto(CreateTypeID(m_dwID, nNewLevel));
	if (!VALID_POINT(pNewProto))
		return;

	if( ESLC_Learn == eType )		m_nLearnLevel += nInc;
	else if( ESLC_Self == eType )	m_nSelfLevel += nInc;
	else if( ESLC_Temp == eType )	m_nTempAddLevel += nInc;

	//INT nNewLevel = min(m_pProto->nMaxLevel, m_nLearnLevel + m_nSelfLevel + m_nTempAddLevel);
	//if( nNewLevel == m_nLevel ) return;

	//const tagSkillProto* pNewProto = AttRes::GetInstance()->GetSkillProto(CreateTypeID(m_dwID, nNewLevel));
	//ASSERT( VALID_POINT(pNewProto) );

	// ȥ���õȼ���ԭ��buff
	UnSetSkillBuff(m_pProto);

	m_nLevel = nNewLevel;
	m_pProto = pNewProto;

	// �����µȼ���buff
	SetSkillBuff(m_pProto);
}

//-----------------------------------------------------------------------------------------------
// ���ܽ�����mod����Ҫ�ı䣬���ü���ԭ��buffҪ���⴦��һ�£�
//-----------------------------------------------------------------------------------------------
inline VOID Skill::DecLevel(ESkillLevelChange eType, INT nDec)
{
	ASSERT( nDec > 0 );

	if( ESLC_Learn == eType )		{ m_nLearnLevel -= nDec;	ASSERT(m_nLearnLevel >= 0); }
	else if( ESLC_Self == eType )	{ m_nSelfLevel -= nDec;		ASSERT(m_nSelfLevel >= 0); }
	else if( ESLC_Temp == eType )	{ m_nTempAddLevel -= nDec;	ASSERT( m_nTempAddLevel >= 0 ); }

	INT nNewLevel = min(m_pProto->nMaxLevel, m_nLearnLevel + m_nSelfLevel + m_nTempAddLevel);
	if( nNewLevel == m_nLevel ) return;

	const tagSkillProto* pNewProto = AttRes::GetInstance()->GetSkillProto(CreateTypeID(m_dwID, nNewLevel));
	ASSERT( VALID_POINT(pNewProto) );

	// ȥ��ԭ���ȼ���buff
	UnSetSkillBuff(m_pProto);

	m_nLevel = nNewLevel;
	m_pProto = pNewProto;

	// �����µȼ���buff
	SetSkillBuff(m_pProto);
}

//---------------------------------------------------------------------------------------------
// ��ȴ����ʱ
//---------------------------------------------------------------------------------------------
inline BOOL Skill::CountDownCoolDown()
{
	m_nCoolDownCountDown -= TICK_TIME;
	if( m_nCoolDownCountDown <= 0 )
	{
		m_nCoolDownCountDown = 0;
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------------------------
// �Ƿ��ǿ���Ӱ���������Եļ���
//---------------------------------------------------------------------------------------------
inline BOOL Skill::IsCanModAtt()
{
	ESkillTargetType eTargetType = m_pProto->eTargetType;
	return IsPassive() && ESTT_Skill != eTargetType && ESTT_Buff != eTargetType && ESTT_Trigger != eTargetType;
}

//---------------------------------------------------------------------------------
// �õ�ĳһ���������˺�ֵ
//---------------------------------------------------------------------------------
inline INT Skill::GetDmg(INT nIndex)
{
	if( nIndex < 0 && nIndex >= MAX_CHANNEL_TIMES ) return 0;

	INT nDmg = m_pProto->nChannelDmg[nIndex];

	//if( VALID_POINT(m_pMod) )
	//{
	//	nDmg = nDmg + m_pMod->nChannelDmgMod[nIndex] + 
	//		INT((FLOAT)nDmg * ((FLOAT)m_pMod->nChannelDmgModPct[nIndex] / 10000.0f));
	//}

	return nDmg;
}

inline INT Skill::GetDmgMod(INT nIndex)
{
	if( nIndex < 0 && nIndex >= MAX_CHANNEL_TIMES ) return 0;
	
	INT nDmg = 0;
	if( VALID_POINT(m_pMod) )
	{
		nDmg = m_pMod->nChannelDmgMod[nIndex];
	}
	return nDmg;
}

inline INT Skill::GetDmgModPct(INT nIndex)
{
	if( nIndex < 0 && nIndex >= MAX_CHANNEL_TIMES ) return 0;

	INT nDmg = 0;
	if( VALID_POINT(m_pMod) )
	{
		nDmg = m_pMod->nChannelDmgModPct[nIndex];
	}
	return nDmg;
}
//---------------------------------------------------------------------------------
// �õ�ĳһ��������Ӧ���˺�ʱ�䣨���룩
//---------------------------------------------------------------------------------
inline INT Skill::GetDmgTime(INT nIndex)
{
	ASSERT( nIndex >=0 && nIndex < MAX_CHANNEL_TIMES );
	return m_pProto->nChannelTime[nIndex];
}

//---------------------------------------------------------------------------------
// �õ���������
//---------------------------------------------------------------------------------
inline FLOAT Skill::GetOPDist()
{
	return m_pProto->fOPDist + ( VALID_POINT(m_pMod) ? m_pMod->fOPDistMod : 0 );
}

//---------------------------------------------------------------------------------
// �õ������뾶
//---------------------------------------------------------------------------------
inline FLOAT Skill::GetOPRadius()
{
	return m_pProto->fOPRadius + ( VALID_POINT(m_pMod) ? m_pMod->fOPRadiusMod : 0 );
}

//----------------------------------------------------------------------------------
// �õ�����ʱ��
//----------------------------------------------------------------------------------
inline INT Skill::GetPrepareTime()
{
	return m_pProto->nPrepareTime + ( VALID_POINT(m_pMod) ? m_pMod->nPrepareTimeMod : 0 );
}

//----------------------------------------------------------------------------------
// �õ���ȴʱ��
//----------------------------------------------------------------------------------
inline INT Skill::GetCoolDown()
{
	return m_pProto->nCoolDown + ( VALID_POINT(m_pMod) ? m_pMod->nCoolDownMod : 0 );
}

//----------------------------------------------------------------------------------
// �õ�����������
//----------------------------------------------------------------------------------
inline FLOAT Skill::GetHit()
{
	INT nHit = m_pProto->nHit;

	if( VALID_POINT(m_pMod) )
	{
		nHit = nHit + m_pMod->nHitMod + INT((FLOAT)nHit * ((FLOAT)m_pMod->nHitModPct / 10000.0f));
	}
	return (FLOAT)nHit / 10000.0f;
}

//----------------------------------------------------------------------------------
// �õ�����������
//----------------------------------------------------------------------------------
inline FLOAT Skill::GetCrit()
{
	INT nCrit = m_pProto->nCrit;

	if( VALID_POINT(m_pMod) )
	{
		nCrit = nCrit + m_pMod->nCritMod + INT((FLOAT)nCrit * ((FLOAT)m_pMod->nCritModPct / 10000.0f));
	}
	return (FLOAT)nCrit / 10000.0f;
}

//----------------------------------------------------------------------------------
// �õ����ܳ��
//----------------------------------------------------------------------------------
inline INT Skill::GetEnmity()
{
	INT nEnmity = m_pProto->nEnmity;

	if(VALID_POINT(m_pMod))
	{
		nEnmity = nEnmity + m_pMod->nEnmityMod + INT((FLOAT)nEnmity * ((FLOAT)m_pMod->nEnmityModPct / 10000.0f));
	}
	return nEnmity;
}

// �õ����ϵ��
inline FLOAT	Skill::GetEnmityParam()
{
	FLOAT fEnmityParam = m_pProto->fEnmityParam;
	if (VALID_POINT(m_pMod))
	{
		fEnmityParam = fEnmityParam + m_pMod->fEnmityParamMod;
	}
	return fEnmityParam;
}
//-------------------------------------------------------------------------------------
// �õ���������
//-------------------------------------------------------------------------------------
inline INT Skill::GetCost(ESkillCostType eCostType)
{
	ASSERT( eCostType >= 0 && eCostType < ESCT_End );

	INT nCost = m_pProto->nSkillCost[eCostType];

	if( VALID_POINT(m_pMod) )
	{
		nCost = nCost + m_pMod->nSkillCostMod[eCostType] + INT((FLOAT)nCost * ((FLOAT)m_pMod->nSkillCostModPct[eCostType] / 10000.0f));
	}

	return nCost;
}

//---------------------------------------------------------------------------------------
// �õ���Ʒ����
//---------------------------------------------------------------------------------------
inline VOID Skill::GetCostItem(DWORD& dwItemID, INT& n_num)
{
	dwItemID = m_pProto->dwCostItemID;
	n_num = m_pProto->nCostItemNum;
}

//---------------------------------------------------------------------------------------
// �õ�ĳ��������Ӧ��BuffID
//---------------------------------------------------------------------------------------
inline DWORD Skill::GetBuffTypeID(INT nIndex)
{
	if( nIndex < 0 || nIndex >= MAX_BUFF_PER_SKILL )
		return INVALID_VALUE;

	if( VALID_POINT(m_pMod) )	return m_pMod->dwBuffTypeID[nIndex];
	else					return m_pProto->dwBuffID[nIndex];
}

//---------------------------------------------------------------------------------------
// �õ�ĳ��������Ӧ��TriggerID
//---------------------------------------------------------------------------------------
inline DWORD Skill::GetTriggerID(INT nIndex)
{
	if( nIndex < 0 || nIndex >= MAX_BUFF_PER_SKILL )
		return INVALID_VALUE;

	if( VALID_POINT(m_pMod) )	return m_pMod->dwTriggerID[nIndex];
	else					return m_pProto->dwTriggerID[nIndex];
}

//---------------------------------------------------------------------------------------
// �������������Buff
//---------------------------------------------------------------------------------------
inline VOID Skill::SetSkillBuff(const tagSkillProto* pProto)
{
	if( !VALID_POINT(m_pMod) ) return;

	UnRegisterTriggerEvent();

	// ���Ը������һ��buff
	for(INT n = 0; n < MAX_BUFF_PER_SKILL; n++)
	{
		if( FALSE == VALID_VALUE(pProto->dwBuffID[n]) )
			continue;

		for(INT m = 0; m < MAX_BUFF_PER_SKILL; m++)
		{
			if( VALID_POINT(m_pMod->dwBuffTypeID[m]) ) continue;
			m_pMod->dwBuffTypeID[m] = pProto->dwBuffID[n];
			m_pMod->dwTriggerID[m] = pProto->dwTriggerID[n];
			break;
		}
	}

	RegisterTriggerEvent();
}

//---------------------------------------------------------------------------------------
// �������������ĳЩBuff
//---------------------------------------------------------------------------------------
inline VOID Skill::UnSetSkillBuff(const tagSkillProto* pProto)
{
	if( !VALID_POINT(m_pMod) ) return;

	UnRegisterTriggerEvent();

	for(INT n = 0; n < MAX_BUFF_PER_SKILL; n++)
	{
		if( FALSE == VALID_VALUE(pProto->dwBuffID[n]) ) continue;

		for(INT m = 0; m < MAX_BUFF_PER_SKILL; m++)
		{
			if( m_pMod->dwBuffTypeID[m] != pProto->dwBuffID[n] )
				continue;

			m_pMod->dwBuffTypeID[m] = INVALID_VALUE;
			m_pMod->dwTriggerID[m] = INVALID_VALUE;
			break;
		}
	}

	RegisterTriggerEvent();
}
