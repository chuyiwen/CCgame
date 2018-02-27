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

#include "stdafx.h"

#include "unit.h"
#include "buff.h"
#include "buff_effect.h"

//----------------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------------
Buff::Buff()
: m_pOwner(NULL), m_nIndex(INVALID_VALUE), m_dwSrcUnitID(INVALID_VALUE), m_dwSrcSkillID(INVALID_VALUE), m_n64ItemID(INVALID_VALUE),
  m_dwItemTypeID(INVALID_VALUE), m_dwID(INVALID_VALUE), m_nLevel(0), m_nPersistTick(0), m_nCurTick(0),
  m_nWrapTimes(0), m_pProto(NULL), m_pMod(NULL), m_eState(EBS_Idle),m_dwAbsorbDmg(0)
{

}

//----------------------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------------------
Buff::~Buff()
{
	SAFE_DELETE(m_pMod);
}

//----------------------------------------------------------------------------------------------------
// ��ʼ������������Ϸ����ʱ���Buff
//----------------------------------------------------------------------------------------------------
VOID Buff::Init(const tagBuffProto* pBuffProto, Unit* pTarget, Unit* pSrc, DWORD dwSrcSkillID, const tagItem* pItem, INT nIndex, package_list<DWORD>* listModifier)
{
	ASSERT( VALID_POINT(pBuffProto) );
	ASSERT( VALID_POINT(pTarget) );

	BeginInit();				// ����Ŀǰ���ڳ�ʼ��

	// ���þ�̬����
	m_pProto = pBuffProto;

	// ���Ҵ�����
	m_pTrigger = AttRes::GetInstance()->GetTriggerProto(pBuffProto->dwOPTrigger);

	// ������������
	m_pOwner = pTarget;
	m_nIndex = nIndex;

	m_dwSrcUnitID = (VALID_POINT(pSrc) ? pSrc->GetID() : INVALID_VALUE);
	m_dwSrcSkillID = dwSrcSkillID;

	if( VALID_POINT(pItem) )
	{
		m_n64ItemID = pItem->n64_serial;
		m_dwItemTypeID = pItem->dw_data_id;
	}
	else
	{
		m_n64ItemID = INVALID_VALUE;
		m_dwItemTypeID = INVALID_VALUE;
	}

	m_dwID = GetIDFromTypeID(pBuffProto->dwID);
	m_nLevel = GetLevelFromTypeID(pBuffProto->dwID);

	m_nPersistTick = 0;
	m_nCurTick = 0;
	m_nWrapTimes = 1;

	// ����BuffӰ��
	if( VALID_POINT(listModifier) && FALSE == listModifier->empty() )
	{
		if( VALID_POINT(m_pMod) )	m_pMod->Clear();
		else					m_pMod = new tagBuffMod;

		// Buffʩ��������������Ӱ���buff�ļ���
		package_list<DWORD>::list_iter it = listModifier->begin();
		DWORD dwSkillTypeID = INVALID_VALUE;

		while( listModifier->find_next(it, dwSkillTypeID) )
		{
			const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(dwSkillTypeID);
			if( !VALID_POINT(pSkillProto) ) continue;

			m_pMod->SetMod(pSkillProto);
		}
	}

	// ����Buff�ĳ�ʼ˲ʱЧ��
	if( !VALID_POINT(m_pTrigger) )
	{
		BuffEffect::CalBuffInstantEffect(m_pOwner, pSrc, EBEM_Instant, m_pProto, m_pMod);
	}

	// ����Buf�ĳ�����Ч��
	BuffEffect::CalBuffPersistEffect(this, m_pOwner, pSrc, m_pProto, m_pMod);
	
	// ���ó�ʼ���ű�
	const BuffScript* pScript = g_ScriptMgr.GetBuffScript(m_dwID);
	if (VALID_POINT(pScript) && VALID_POINT(pSrc))
	{
		pScript->OnInit(pSrc->get_map(), m_dwID, m_nLevel, m_pOwner->GetID(), pSrc->GetID());
	}

	EndInit();				// ��ʼ������
}

//----------------------------------------------------------------------------------------------------
// ��ʼ�����������������ʱ���buff
//----------------------------------------------------------------------------------------------------
VOID Buff::Init(Unit* pTarget, const s_buff_save* pBuffSave, INT nIndex)
{
	if( !VALID_POINT(pTarget) || !VALID_POINT(pBuffSave) )
		return;

	BeginInit();

	m_pProto = AttRes::GetInstance()->GetBuffProto(Buff::GetTypeIDFromIDAndLevel(pBuffSave->dw_buff_id_, pBuffSave->n_level_));
	if( !VALID_POINT(m_pProto) )
	{
		print_message(_T("Can not find the buff proto when loading the buff: id=%u, roleid=%u\r\n"), pBuffSave->dw_buff_id_, pTarget->GetID());
		m_dwID = INVALID_VALUE;
		EndInit();
		return;
	}

	m_pTrigger = AttRes::GetInstance()->GetTriggerProto(m_pProto->dwOPTrigger);

	m_dwID = pBuffSave->dw_buff_id_;
	m_nLevel = pBuffSave->n_level_;

	m_pOwner = pTarget;
	m_nIndex = nIndex;

	m_dwSrcUnitID = pBuffSave->dw_src_unit_id_;
	m_dwSrcSkillID = pBuffSave->dw_src_skill_id_;
	m_n64ItemID = pBuffSave->n_serial_;
	m_dwItemTypeID = pBuffSave->dw_item_type_id_;

	m_nPersistTick = pBuffSave->n_persist_tick_;
	m_nCurTick = 0;

	m_nWrapTimes = pBuffSave->n_cur_lap_times_;

	// ����BuffӰ��
	if( pBuffSave->n_modifier_num_ > 0 )
	{
		if( VALID_POINT(m_pMod) )	m_pMod->Clear();
		else					m_pMod = new tagBuffMod;

		DWORD* pdwSkillTypeID = (DWORD*)pBuffSave->by_data_;

		for(INT n = 0; n < pBuffSave->n_modifier_num_; ++n)
		{
			const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(pdwSkillTypeID[n]);
			if( !VALID_POINT(pSkillProto) ) continue;

			m_pMod->SetMod(pSkillProto);
		}
	}

	// ����Buff�ĳ�����Ч�����������ô�õ�Buffʩ���ߵ�ָ�룿����
	BuffEffect::CalBuffPersistEffect(this, m_pOwner, NULL, m_pProto, m_pMod, m_nWrapTimes);

	EndInit();
}

//----------------------------------------------------------------------------------------------------
// ��ʼ��tagBuffSave�ṹ
//----------------------------------------------------------------------------------------------------
VOID Buff::InitBuffSave(OUT s_buff_save *pBuffSave, OUT INT32 &nSize)
{
	pBuffSave->dw_buff_id_				= m_dwID;
	pBuffSave->n_persist_tick_			= m_nPersistTick;;
	pBuffSave->n_level_				= m_nLevel;

	pBuffSave->n_cur_lap_times_		= m_nWrapTimes;

	pBuffSave->dw_src_unit_id_			= m_dwSrcUnitID;
	pBuffSave->dw_src_skill_id_			= m_dwSrcSkillID;
	pBuffSave->n_serial_			= m_n64ItemID;
	pBuffSave->dw_item_type_id_			= m_dwItemTypeID;

	// Buff�ļ���Ӱ��
	M_trans_pointer(pModifier, pBuffSave->by_data_, DWORD);
	INT n = 0;

	if( VALID_POINT(m_pMod) && FALSE == m_pMod->listModifier.empty() )
	{	
		package_list<DWORD>::list_iter it = m_pMod->listModifier.begin();
		DWORD dwSkillTypeID = INVALID_VALUE;
		while( m_pMod->listModifier.find_next(it, dwSkillTypeID) )
		{
			pModifier[n] = dwSkillTypeID;
			++n;
		}
	}
	pBuffSave->n_modifier_num_ = n;

	nSize = (INT32)((size_t)&pModifier[n] - (size_t)pBuffSave);
}

//----------------------------------------------------------------------------------------------------
// ���£��������True�����ʾ��BuffҪɾ����
//----------------------------------------------------------------------------------------------------
BOOL Buff::Update()
{
	if( !IsValid() ) return FALSE;

	BeginUpdate();

	// ����Ǽ�����õ�buff������ʱ���Ƿ���
	if( IsInterOP() )
	{
		if( ++m_nCurTick >= m_pProto->nInterOPTick )
		{
			// ����������Ч��
			Unit* pSrc = NULL;
			if( VALID_POINT(m_pOwner->get_map()) )
			{
				pSrc = m_pOwner->get_map()->find_unit(m_dwSrcUnitID);
			}
			BuffEffect::CalBuffInstantEffect(m_pOwner, pSrc, EBEM_Inter, m_pProto, m_pMod, m_nWrapTimes);

			// ����ʱ�ű�����
			const BuffScript* pScript = g_ScriptMgr.GetBuffScript(m_dwID);
			if (VALID_POINT(pScript) && VALID_POINT(pSrc))
			{
				pScript->OnTrigger(pSrc->get_map(), m_dwID, m_nLevel, m_pOwner->GetID(), pSrc->GetID());
			}
			m_nCurTick = 0;
		}
	}

	// ���buff�ĳ���ʱ��
	if( !IsPermanent() )
	{
		if( ++m_nPersistTick >= GetMaxPersistTick() )
		{
			// ����ʱ�䵽��
			EndUpdate();
			return TRUE;
		}
	}

	//���¹⻷
	if (m_pProto->eEffect[EBEM_Persist] == EBET_Ring)
	{
		Unit* pSrc = NULL;
		if( VALID_POINT(m_pOwner->get_map()) )
		{
			pSrc = m_pOwner->get_map()->find_unit(m_dwSrcUnitID);
		}

		EBuffEffectType eEffect = m_pProto->eEffect[EBEM_Persist];

		if( EBET_Null != eEffect )
		{
			DWORD dwEffectMisc1 = m_pProto->dwEffectMisc1[EBEM_Persist];
			DWORD dwEffectMisc2 = m_pProto->dwEffectMisc2[EBEM_Persist];

			BOOL bHaveMod = FALSE;
			if( VALID_POINT(m_pMod) && m_pMod->IsValid() && EBEM_Persist == m_pMod->eModBuffEffectMode )
			{
				bHaveMod = TRUE;
			}

			if( bHaveMod )
			{
				dwEffectMisc1 += m_pMod->nEffectMisc1Mod;
				dwEffectMisc2 += m_pMod->nEffectMisc2Mod;
			}

			BuffEffect::CalBuffEffect(this, m_pOwner, pSrc, eEffect, dwEffectMisc1, dwEffectMisc2, TRUE, m_pProto);
		}
	}
	EndUpdate();

	return FALSE;
}

//----------------------------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------------------------
VOID Buff::Destroy(BOOL bSelf)
{
	BeginDestroy();

	Unit* pSrc = NULL;
	if( VALID_POINT(m_pOwner->get_map()) )
	{
		pSrc = m_pOwner->get_map()->find_unit(m_dwSrcUnitID);
	}
	// ����Buff�Ľ���ʱЧ����ֻ����ʱ�䵽���������ʱ�ż���buff����ʱЧ��
	if( bSelf )
	{
		BuffEffect::CalBuffInstantEffect(m_pOwner, pSrc, EBEM_Finish, m_pProto, m_pMod, m_nWrapTimes);
	}

	// ȡ��Buff�ĳ�����Ч��
	BuffEffect::CalBuffPersistEffect(this, m_pOwner, pSrc, m_pProto, m_pMod, m_nWrapTimes, FALSE);
	
	// ����ʱ�ű�����
	const BuffScript* pScript = g_ScriptMgr.GetBuffScript(m_dwID);
	if (VALID_POINT(pScript) && VALID_POINT(pSrc) )
	{
		pScript->OnDestory(pSrc->get_map(), m_dwID, m_nLevel, m_pOwner->GetID(), pSrc->GetID());
	}


	// ���Buff
	m_dwID = INVALID_VALUE;
	m_pOwner = NULL;
	m_pProto = NULL;

	// ���Mod��ֵ�����ﲻ�ͷŵ������ǽ�������գ��Ա��´���ʹ�ø�Buffʱ�����ٷ����ڴ�
	if( VALID_POINT(m_pMod) )
	{
		m_pMod->Clear();
	}

	EndDestroy();
}

//----------------------------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------------------------
VOID Buff::Wrap()
{
	BeginUpdate();

	Unit* pSrc = NULL;
	if( VALID_POINT(m_pOwner->get_map()) )
	{
		pSrc = m_pOwner->get_map()->find_unit(m_dwSrcUnitID);
	}

	++m_nWrapTimes;
	m_nPersistTick = 0;

	// ��������һ��Buff��һ��˲��Ч��
	BuffEffect::CalBuffInstantEffect(m_pOwner, pSrc, EBEM_Instant, m_pProto, m_pMod);

	// �鿴�Ƿ���Ҫ���ۼ�һ�γ�����Ч��
	if( m_nWrapTimes > GetMaxWrapTimes() )
	{
		m_nWrapTimes = GetMaxWrapTimes();
	}
	else
	{
		// ��û�г��������Ӵ��������ټ���һ��Buff����Ч��
		m_pOwner->WrapBuffPersistEffect(this, pSrc, m_pProto, m_pMod);
	}

	EndUpdate();
}

//------------------------------------------------------------------------------------------------------
// ���
//------------------------------------------------------------------------------------------------------
BOOL Buff::Interrupt(EBuffInterruptFlag eFlag, INT nMisc)
{
	if( !IsValid() ) return FALSE;

	// ���buff������flag�еĴ�Ϸ�ʽ������ʧ��
	if( !(eFlag & m_pProto->dwInterruptFlag) ) return FALSE;

	// ����ĳЩ������жϣ�Ҫ�����ж�
	switch(eFlag)
	{
		// ��������
	case EBIF_HPLower:
		{
			INT nHPLimit = m_pProto->nHPInterruptLimit;
			if( nHPLimit < 100000 )
			{
				return m_pOwner->GetAttValue(ERA_HP) < nHPLimit;
			}
			else if( m_pOwner->GetAttValue(ERA_MaxHP) > 0 )
			{
				return m_pOwner->GetAttValue(ERA_HP) * 10000 / m_pOwner->GetAttValue(ERA_MaxHP) < (nHPLimit - 100000);
			}
			else
			{
				return FALSE;
			}
		}
		break;

		// ��������
	case EBIF_MPLower:
		{
			INT nMPLimit = m_pProto->nMPInterruptLimit;
			if( nMPLimit < 100000 )
			{
				return m_pOwner->GetAttValue(ERA_MP) < nMPLimit;
			}
			else if( m_pOwner->GetAttValue(ERA_MaxMP) > 0 )
			{
				return m_pOwner->GetAttValue(ERA_MP) * 10000 / m_pOwner->GetAttValue(ERA_MaxMP) < (nMPLimit - 100000);
			}
			else
			{
				return FALSE;
			}
		}
		break;

		// ���ĵ���
	case EBIF_RageLower:
		{
			INT nRageLimit = m_pProto->nRageInterruptLimit;
			return m_pOwner->GetAttValue(ERA_Love) <= nRageLimit;
		}
		break;

		// �־�������
	//case EBIF_EnduranceLower:
	//	{
	//		INT nEnduranceLimit = m_pProto->nEnduranceInterruptLimit;
	//		if( nEnduranceLimit < 100000 )
	//		{
	//			return m_pOwner->GetAttValue(ERA_Endurance) < nEnduranceLimit;
	//		}
	//		else if( m_pOwner->GetAttValue(ERA_MaxEndurance) > 0 )
	//		{
	//			return m_pOwner->GetAttValue(ERA_Endurance) * 10000 / m_pOwner->GetAttValue(ERA_MaxEndurance) < (nEnduranceLimit - 100000);
	//		}
	//		else
	//		{
	//			return FALSE;
	//		}
	//	}
	//	break;

		// ��������
	//case EBIF_VitalityLower:
	//	{
	//		INT nVitalityLimit = m_pProto->nVitalityInterruptLimit;

	//		if( nVitalityLimit < 100000 )
	//		{
	//			return m_pOwner->GetAttValue(ERA_Vitality) < nVitalityLimit;
	//		}
	//		else if( m_pOwner->GetAttValue(ERA_MaxVitality) > 0 )
	//		{
	//			return m_pOwner->GetAttValue(ERA_Vitality) * 10000 / m_pOwner->GetAttValue(ERA_MaxVitality) < (nVitalityLimit - 100000);
	//		}
	//		{
	//			return FALSE;
	//		}
	//	}
	//	break;

		// ������
	case EBIF_BeAttacked:
		{
			INT nProp = m_pProto->nAttackInterruptRate + ( VALID_POINT(m_pMod) ? m_pMod->nAttackInterruptRateMod : 0 );

			if( get_tool()->tool_rand() % 10000 > nProp ) return FALSE;
		}
		break;

	default:
		break;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------------------------------------------
// ����
//-----------------------------------------------------------------------------------------------------------------
BOOL Buff::OnTrigger(Unit* pTarget, ETriggerEventType eEvent, DWORD dwEventMisc1/* =INVALID_VALUE */, DWORD dwEventMisc2/* =INVALID_VALUE */)
{
	if( !IsValid() ) return FALSE;

	if( m_eState != EBS_Idle ) return FALSE;

	// ��ⴥ������������
	if( !VALID_POINT(m_pTrigger) || m_pTrigger->eEventType != eEvent ) return FALSE;

	// ����Trigger�Ƿ�����
	if( !m_pOwner->TestTrigger(pTarget, m_pTrigger, dwEventMisc1, dwEventMisc2) ) return FALSE;

	// �������ڸ���
	BeginUpdate();

	// ����Buff˲ʱЧ��
	Unit* pSrc = NULL;
	if( VALID_POINT(m_pOwner->get_map()) )
	{
		pSrc = m_pOwner->get_map()->find_unit(m_dwSrcUnitID);
	}
	BuffEffect::CalBuffInstantEffect(m_pOwner, pSrc, EBEM_Instant, m_pProto, m_pMod, m_nWrapTimes, pTarget);

	// ����ֹͣ����
	EndUpdate();

	return TRUE;
}

