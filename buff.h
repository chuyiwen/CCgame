/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//״̬

#pragma once

class Unit;

//--------------------------------------------------------------------------------
// ��ӵ����ϵ�buff
//--------------------------------------------------------------------------------
class Buff
{
public:
	enum EBuffState				// Buff״̬
	{
		EBS_Idle		=	0,	// ����
		EBS_Initing		=	1,	// ��ʼ��
		EBS_Updating	=	2,	// ����
		EBS_Destroying	=	3,	// ɾ��
	};

public:
	Buff();
	~Buff();

	//----------------------------------------------------------------------------
	// ��ʼ�������º����ٺͱ���
	//----------------------------------------------------------------------------
	VOID			Init(const tagBuffProto* pBuffProto, Unit* pTarget, Unit* pSrc, DWORD dwSrcSkillID, 
						const tagItem* pItem, INT nIndex, package_list<DWORD>* listModifier=NULL);
	VOID			Init(Unit* pTarget, const s_buff_save* pBuffSave, INT nIndex);
	VOID			InitBuffSave(OUT s_buff_save *pBuffSave, OUT INT32 &nSize);
	BOOL			Update();
	VOID			Destroy(BOOL bSelf);

	//----------------------------------------------------------------------------
	// ���Ӻʹ�Ϻʹ���
	//----------------------------------------------------------------------------
	VOID			Wrap();
	BOOL			Interrupt(EBuffInterruptFlag eFlag, INT nMisc=INVALID_VALUE);
	BOOL			OnTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1=INVALID_VALUE, DWORD dwEventMisc2=INVALID_VALUE);

	//----------------------------------------------------------------------------
	// ����Get
	//----------------------------------------------------------------------------
	BOOL			IsValid()				{ return VALID_POINT(m_dwID); }
	DWORD			GetID()					{ return m_dwID; }
	DWORD			GetTypeID()				{ return m_pProto->dwID; }
	INT				get_level()				{ return m_nLevel; }
	INT				GetGroupLevel()			{ return m_pProto->nLevel; }
	INT				GetIndex()				{ return m_nIndex; }
	DWORD			GetSrcUnitID()			{ return m_dwSrcUnitID; }
	DWORD			GetSrcSkillID()			{ return m_dwSrcSkillID; }
	DWORD			GetGroupFlag()			{ return m_pProto->dwGroupFlag; }
	DWORD			GetBuffInterruptID()	{ return m_pProto->dwBuffInterruptID; }
	INT				GetWrapTimes()			{ return m_nWrapTimes; }
	EBuffResistType	GetResistType()			{ return m_pProto->eResistType; }
	EBuffEffectType	GetEffectType(EBuffEffectMode eMod)			{ return m_pProto->eEffect[eMod]; }
	EBuffState		GetState()				{ return m_eState; }

	INT				GetPersistTimeLeft()	{ return ( !IsPermanent() ? ((GetMaxPersistTick() - m_nPersistTick) * TICK_TIME) : INVALID_VALUE); }
	INT				GetPersistTime()		{ return ( !IsPermanent() ? GetMaxPersistTick() * TICK_TIME : INVALID_VALUE); }
	
	BOOL			IsBenifit()				
	{ 
		if (!VALID_POINT(m_pProto))
			return FALSE;

		return m_pProto->bBenifit;
	}

	const tagBuffProto* GetProto() { return m_pProto;}
	//�˺��������
	DWORD GetAbsorbDmg() const { return m_dwAbsorbDmg; }
	void SetAbsorbDmg(DWORD val) { m_dwAbsorbDmg = val; }
	//-----------------------------------------------------------------------------
	// һЩ����ľ�̬����
	//-----------------------------------------------------------------------------
	static DWORD	GetIDFromTypeID(DWORD dw_data_id)						{ return dw_data_id / 100; }
	static INT		GetLevelFromTypeID(DWORD dw_data_id)					{ return dw_data_id % 100; }
	static DWORD	GetTypeIDFromIDAndLevel(DWORD dwID, INT nLevel)		{ return dwID * 100 + nLevel; }

private:
	//-----------------------------------------------------------------------------
	// ״̬�л�
	//-----------------------------------------------------------------------------
	VOID			BeginInit()			{ m_eState = EBS_Initing; }
	VOID			EndInit()			{ m_eState = EBS_Idle; }
	VOID			BeginUpdate()		{ m_eState = EBS_Updating; }
	VOID			EndUpdate()			{ m_eState = EBS_Idle; }
	VOID			BeginDestroy()		{ m_eState = EBS_Destroying; }
	VOID			EndDestroy()		{ m_eState = EBS_Idle; }

	//----------------------------------------------------------------------------
	// һЩGet����
	//----------------------------------------------------------------------------
	INT				GetMaxPersistTick();
	INT				GetMaxWrapTimes();
	INT				GetAttackInterrupt();

	//----------------------------------------------------------------------------
	// ������Buff�ͼ��������buff�ж�
	//----------------------------------------------------------------------------
	INT				IsPermanent();
	INT				IsInterOP();

	//----------------------------------------------------------------------------
	// ��������ID
	//----------------------------------------------------------------------------
	DWORD			CreateTypeID(DWORD dwID, INT nLevel)	{ return dwID * 100 + nLevel; }
	


private:
	EBuffState						m_eState;						// ��ǰ״̬
	DWORD							m_dwID;							// ID
	INT								m_nLevel;						// �ȼ�
	INT								m_nIndex;						// ��Ӧ��Buff�е�����
	DWORD							m_dwSrcUnitID;					// ԴUnit��ID
	DWORD							m_dwSrcSkillID;					// ���ĸ����ܲ�������ID��
	INT64							m_n64ItemID;					// ���ĸ���Ʒ��װ������
	DWORD							m_dwItemTypeID;					// ��Ʒ��װ��������ID
	DWORD							m_dwAbsorbDmg;					// �˺�������

	INT								m_nPersistTick;					// ��ǰ�ĳ���ʱ��
	INT								m_nCurTick;						// ��ǰ����ʱ�� 
	INT								m_nWrapTimes;					// ��ǰ���Ӵ���

	Unit*							m_pOwner;						// ��˭����
	const tagBuffProto*				m_pProto;						// ��̬����
	const tagTriggerProto*			m_pTrigger;						// Ч��������
	tagBuffMod*						m_pMod;							// ���Լӳ�

};

//-------------------------------------------------------------------------------------
// �õ�Buff��������ʱ��
//-------------------------------------------------------------------------------------
inline INT Buff::GetMaxPersistTick()
{
	INT nPersistTick = m_pProto->nPersistTick;
	if( VALID_POINT(nPersistTick) && VALID_POINT(m_pMod) )
	{
		nPersistTick = nPersistTick + m_pMod->nPersistTickMod;
	}
	return nPersistTick;
}

//-------------------------------------------------------------------------------------
// �õ�Buff�������Ӵ���
//-------------------------------------------------------------------------------------
inline INT Buff::GetMaxWrapTimes()
{
	return m_pProto->nWarpTimes + ( VALID_POINT(m_pMod) ? m_pMod->nWarpTimesMod : 0 );
}

//--------------------------------------------------------------------------------------
// �õ�Buff�Ĺ�������ϼ���
//--------------------------------------------------------------------------------------
inline INT Buff::GetAttackInterrupt()
{
	return m_pProto->nAttackInterruptRate + (VALID_POINT(m_pMod) ? m_pMod->nAttackInterruptRateMod : 0);
}

//--------------------------------------------------------------------------------------
// Buff�Ƿ�Ϊ������Buff
//--------------------------------------------------------------------------------------
inline INT Buff::IsPermanent()
{
	return !VALID_POINT(m_pProto->nPersistTick);
}

//--------------------------------------------------------------------------------------
// Buff�Ƿ�Ϊ���������Buff
//--------------------------------------------------------------------------------------
inline INT Buff::IsInterOP()
{
	return m_pProto->nInterOPTick > 0;
}
