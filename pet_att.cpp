/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��������

#include "StdAfx.h"
#include "pet_att.h"
#include "../../common/WorldDefine/pet_protocol.h"
#include "../common/ServerDefine/role_data_server_define.h"
#include "role.h"
#include "world.h"
#include "att_res.h"
#include "pet_server_define.h"
#include "pet_soul.h"

#include "att_res.h"

//-------------------------------------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------------------------------------
// const INT MAX_QUALITY_SCALE	= 3;
// struct 
// {
// 	INT nMin[MAX_QUALITY_SCALE];
// 	INT nMax[MAX_QUALITY_SCALE];
// 	INT nProb[MAX_QUALITY_SCALE];
// } QualityScales[EIQ_End] = 
// {
// 	{		{1,	11,	21},	{10,	20,	30},	{45,	45,	10}	},
// 	{		{21,31,	41},	{30,	40,	50},	{45,	45,	10}	},
// 	{		{41,51,	61},	{50,	60,	70},	{45,	45,	10}	},
// 	{		{61,71,	81},	{70,	80,	90},	{45,	45,	10}	},
// 	{		{90,0,	0},		{100,	0,	0},		{100,	0,	0}	},
// };

//INT PetAtt::CalAptitude( INT nQuality, const tagPetProto* pPetProto)
//{
//	if (nQuality <= EIQ_Null || nQuality >= EIQ_End)
//		return 1;

//	return get_tool()->rand_in_range(pPetProto->nAptitudeMin[nQuality], pPetProto->nAptitudeMax[nQuality]);
// 	for (INT i=0; i<MAX_QUALITY_SCALE * 10; ++i)
// 	{
// 		INT nIndex = i % MAX_QUALITY_SCALE;
// 		if (IUTIL->probability(QualityScales[nQuality].nProb[nIndex]))
// 		{
// 			return IUTIL->rand_in_range(QualityScales[nQuality].nMin[nIndex], QualityScales[nQuality].nMax[nIndex]);
// 		}
// 	}
	
//	return QualityScales[nQuality].nMin[0];
//}

////-------------------------------------------------------------------------------------------------------
//// ����ٶ�
////-------------------------------------------------------------------------------------------------------
//INT MountSpeedStepAdd[NUM_PET_STEP] =
//{
//	0,		// 0��0%
//	100,	// 1��1%
//	200,	// 2��2%
//	400,	// 3��4%
//	600,	// 4��6%
//	900,	// 5��9%
//	1200,	// 6��12%
//	1600,	// 7��16%
//	2000,	// 8��20%
//	2500	// 9��25%
//};
//INT MountSpeedQualityBase[EIQ_End] =
//{
//	3000, 4000, 5000, 6000, 7000
//};
//
//INT PetAtt::CalMountSpeedbBase(INT nQuality, INT nStep)
//{
//	return MountSpeedStepAdd[nStep] + MountSpeedQualityBase[nQuality];
//}


//-------------------------------------------------------------------------------------------------------
// ������������
//-------------------------------------------------------------------------------------------------------
INT PetAtt::CalLvlUpExp(INT nStep, INT nGrade)
{
	INT nVLevel = 0;
	TransStepGrade2VLevel(nStep, nGrade, nVLevel);
	const tagPetLvlUpProto* pLvlUpProto = AttRes::GetInstance()->GetPetLvlUpProto(nVLevel);
	if (!VALID_POINT(pLvlUpProto))
	{
		return 0;
	}

	return pLvlUpProto->nExpLvlUpNeed;
}

//-------------------------------------------------------------------------
// ��ʼ�����ݿⴴ��������Ҫ������
//-------------------------------------------------------------------------
VOID PetAtt::InitCreateAtt( s_db_pet_soul_create* pCreate, DWORD dwPetTypeID, DWORD dwOwnerID, LPCTSTR szName, INT nQuality )
{
	M_trans_else_ret(pProto, AttRes::GetInstance()->GetPetProto(dwPetTypeID), const tagPetProto, );

	pCreate->dw_proto_id			= dwPetTypeID;
	pCreate->dw_master_id			= dwOwnerID;
	_tcsncpy(pCreate->sz_name_, szName, X_SHORT_NAME);
	
	pCreate->n_quality_			= nQuality;
	pCreate->n_aptitude_		= 0;//CalAptitude(nQuality, pProto);
	pCreate->n_spirte_			= CalSpiritMax(pCreate->n_quality_);
	pCreate->n_potential_		= CalPotential(pCreate->n_aptitude_);//���ʡ�10
}

//-------------------------------------------------------------------------
// ��ʼ��
//-------------------------------------------------------------------------
BOOL PetAtt::InitAtt( const s_db_pet_att* pAtt)
{
	m_pProto					= AttRes::GetInstance()->GetPetProto(pAtt->dw_proto_id_);
	if (!VALID_POINT(m_pProto) || !VALID_POINT(pAtt))
	{
		ASSERT(0);
		return FALSE;
	}

	//---------------------------------��ʼ�����ݿ�����-----------------------------
	_tcsncpy(m_tszName, pAtt->sz_name_, X_SHORT_NAME);
	m_PetState.InitState(pAtt->by_pet_state_ & 0xee);				//״̬
	m_dwProtoID					= pAtt->dw_proto_id_;
	m_nExpCur					= pAtt->n_exp_cur_;		//��ǰ����	
	m_nStep						= pAtt->n_step_;		//��
	m_nGrade					= pAtt->n_grade_;		//��
	m_dwPetID					= pAtt->dw_pet_id_;		//����id
	m_bLocked					= pAtt->b_locked_;		//����
	m_nRenameCount				= pAtt->n_rename_count_;	//��������
	m_dwTimeType				= pAtt->dw_time_type;
	m_dwModeType				= pAtt->dw_mode_type;
	m_dwXiulianTime				= pAtt->dw_xiulian_time;

	m_nAtt[epa_quality]			= pAtt->n_quality_;		//��ȡƷ��
	m_nAtt[epa_aptitude]		= pAtt->n_aptitude_;		//��ȡ����
	m_nAtt[epa_spirit]			= pAtt->n_spirit_;		//��ȡ��ǰ����
	m_nAtt[epa_potential]		= pAtt->n_potential_;		//��ȡ��ǰǱ��
	m_nAtt[epa_talent_count]	= pAtt->n_talent_count_;	//��ȡ�츳����
	m_nAtt[epa_wuxing_energy]	= pAtt->n_wuxing_energy_;	//��ȡ������
	m_nAtt[epa_happy_value]		= pAtt->n_happy_value_;
	m_nAtt[epa_color]			= pAtt->n_color;
	m_nAtt[epa_strength]		= pAtt->n_strength;
	m_nAtt[epa_agility]			= pAtt->n_agility;
	m_nAtt[epa_innerForce]		= pAtt->n_innerForce;
	m_nAtt[epa_att_point]		= pAtt->n_att_point;


	//---------------------------------��ʼ��Ĭ��ֵ---------------------------------
	m_nAtt[epa_spirit_max]		= 0;					//�������ֵ
	m_nAtt[epa_talent_count_max]	= 0;					//��ȡ�츳����
	m_nAtt[epa_mount_num]		= 0;					//�����Ŀ
	m_nAtt[epa_mount_num_max]		= 0;					//�����Ŀ���ֵ

	m_nAtt[epa_bag_grid]			= 0;					//���Ҹ���	Ĭ��4��		װ���ӳ�2~16
	m_nAtt[epa_delivery_consume]	= 0;					//��ݺ�ʱ	Ĭ��0��		װ���ӳ�-��~0��ms����Ӧ���ܱ�����ʱ�䡱
	m_nAtt[epa_sell_consume]		= 0;					//������ʱ	Ĭ��0��		װ���ӳ�-��~0��ms����Ӧ���ܱ�����ʱ�䡱
	m_nAtt[epa_storage_consume]	= 0;					//��ź�ʱ	Ĭ��0��		װ���ӳ�-��~0��ms����Ӧ���ܱ�����ʱ�䡱
	m_nAtt[epa_gather_consume]	= 0;					//�ɼ���ʱ	Ĭ��0��		װ���ӳ�-��~0��ms����Ӧ���ܱ�����ʱ�䡱
	m_nAtt[epa_train_resume]		= 0;					//ѵ���ָ�	Ĭ��0��		װ���ӳ�-��~0��ms����Ӧ���ܱ��ָ�ʱ�䡱
	m_nAtt[epa_spirit_rate]		= 0;					//��������	Ĭ��100%��	װ���ӳ�-100%~0%����Ӧ���ܱ�energy_cost
	m_nAtt[epa_exp_rate]			= 0;					//����ӳ�	Ĭ��100%��	װ���ӳ�0%~400%
	m_nAtt[epa_pick_up_resume]	= 0;					//ʰȡ�ָ�	Ĭ��0��		װ���ӳ�-20~-1
	m_nAtt[epa_medicine_saving]	= 0;					//��ҩ��ʡ	Ĭ��0%��	װ���ӳ�1%~5%
	m_nAtt[epa_strength_effect]	= 0;					//ǿ��Ч��	Ĭ��100%��	װ���ӳ�0%~400%
	m_nAtt[epa_wuxing_consume]	= 0;					//����������	Ĭ��100%��	װ���ӳ�-50%~0%����Ӧ���ܱ����������ġ�

	m_nAtt[epa_mount_speed]		= 0;					//����ٶ�	Ĭ��Ʒ�ʺͽ׼��㣬װ���ӳ�0%~100%
	
	return TRUE;
}

//-------------------------------------------------------------------------
// �޸ľ���
//-------------------------------------------------------------------------
VOID PetAtt::ExpChange( INT nExpMod, BOOL bSend, BOOL bOverFlow )
{
	if (0 == nExpMod)
		return;
	nExpMod *= (GetAttVal(epa_exp_rate) / 10000.0f);

	INT		nLvlUpExp = 0;
	INT		nVLevel = 0;
	BOOL	bLvlUp	= FALSE;

	// ���پ���
	if (nExpMod < 0)
	{
		m_nExpCur += nExpMod;
		m_nExpCur = m_nExpCur < 0 ? 0 : m_nExpCur;
	}
	// ���Ӿ���
	else if (nExpMod > 0)
	{
		nLvlUpExp = GetExpLvlUp();
		m_nExpCur += nExpMod;

		while (m_nExpCur >= nLvlUpExp && nLvlUpExp != 0)
		{
			// ���ﵽ��ǰ����� �� ��ע������
			if (nLvlUpExp <= m_nExpCur && MAX_PET_GRADE == m_nGrade)
			{
				m_nExpCur = nLvlUpExp;
				break;
			}
			// �����µȼ�
			TransStepGrade2VLevel(m_nStep, m_nGrade, nVLevel);

			// �����ɱ��������
			if (nVLevel >= MIN_CHANGE_LEVEL)
			{
				if (!bOverFlow)
				{
					m_nExpCur -= nExpMod;
				}
				
				break;
			}

			m_nExpCur		-= nLvlUpExp;
			nVLevel		+= 1;
			TransVLevel2StepGrade(nVLevel, m_nStep, m_nGrade);

			m_pSoul->OnLevelUp();

			SyncAllLvlUpChangeAtt();

			nLvlUpExp	= GetExpLvlUp();
			bLvlUp		= TRUE;
		}
	}

	if (bSend)
	{
		// ͬ����ǰ����
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);
		if (bLvlUp)
		{
			// ͬ���������龭��
			SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

			// ͬ������ȼ�
			SyncToClientAttChg(ECSPA_nLevel, nVLevel);
		}
	}
}
VOID PetAtt::ChangeLevel(INT nLevel, BOOL bSend)
{
	INT nCurLevel = 0;
	TransStepGrade2VLevel(m_nStep, m_nGrade, nCurLevel);

	if (nLevel == nCurLevel) return;

	TransVLevel2StepGrade(nLevel, m_nStep, m_nGrade);
	m_nExpCur = 0;

	SyncAllLvlUpChangeAtt();
	INT nLvlUpExp	= GetExpLvlUp();

	if (bSend)
	{
		// ͬ����ǰ����
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);

		// ͬ���������龭��
		SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

		// ͬ������ȼ�
		SyncToClientAttChg(ECSPA_nLevel, nLevel);

	}

}
//-------------------------------------------------------------------------
// ��ȡ����ֵ
//-------------------------------------------------------------------------
INT PetAtt::GetAttVal( INT nPetAtt )
{
	return m_nAtt[nPetAtt] + GetAttDef(nPetAtt);
}

//-------------------------------------------------------------------------
// ��������ֵ
//-------------------------------------------------------------------------
VOID PetAtt::SetAttVal(INT nPetAtt, INT nValue, BOOL bSend /* = TRUE */)
{
	INT nSrcVal = m_nAtt[nPetAtt];
	m_nAtt[nPetAtt] = nValue;
	OnAttChg(nPetAtt, nSrcVal, bSend);
}

//-------------------------------------------------------------------------
// �ӳ����Ե�Ĭ��ֵ
//-------------------------------------------------------------------------
INT PetAtt::GetAttDef( INT nPetAtt )
{
	INT nDefVal = 0;
	switch(nPetAtt)
	{
	case epa_talent_count_max:
		nDefVal = CalTalentCountMax(m_nAtt[epa_quality], m_nStep, m_nGrade);
		break;
	case epa_spirit_max:
		nDefVal = CalSpiritMax(m_nAtt[epa_quality]);
		break;
	case epa_bag_grid:
		nDefVal = 4;
		break;
	case epa_spirit_rate:
	case epa_exp_rate:
	case epa_strength_effect:
	case epa_wuxing_consume:
		nDefVal = 10000;
		break;
	case epa_mount_speed:
		nDefVal = GetProto()->nMountSpeed;
		break;
	case epa_mount_num_max:
		nDefVal = GetProto()->nMountable;
		break;
	case epa_happy_value:
		nDefVal = 0;
	default:
		nDefVal = 0;
		break;
	}

	return nDefVal;
}

//-------------------------------------------------------------------------
// ����ֵ�仯����
//-------------------------------------------------------------------------
VOID PetAtt::OnAttChg( INT nPetAtt, INT nSrcVal, BOOL bSend /*= TRUE*/ )
{
	// ����֪ͨ
	// ECSPA_nLevel
	// ECSPA_nExpCurrent
	// ECSPA_nExpLevelUp
	// ECSPA_PetState	

	switch(nPetAtt)
	{
	case epa_spirit:
		if (m_nAtt[epa_spirit] > GetAttVal(epa_spirit_max))
		{
			m_nAtt[epa_spirit] = GetAttVal(epa_spirit_max);
		}
		else if (m_nAtt[epa_spirit] < 0)
		{
			m_nAtt[epa_spirit] = 0;
		}
		break;
	case epa_talent_count:
		if (m_nAtt[epa_talent_count] > GetAttVal(epa_talent_count_max))
		{
			m_nAtt[epa_talent_count] = GetAttVal(epa_talent_count_max);
		}
		else if (m_nAtt[epa_talent_count] < 0)
		{
			m_nAtt[epa_talent_count] = 0;
		}
		break;
	case epa_mount_num:
		if (m_nAtt[epa_mount_num] > GetAttVal(epa_mount_num_max))
		{
			m_nAtt[epa_mount_num] = GetAttVal(epa_mount_num_max);
		}
		else if (m_nAtt[epa_mount_num] < 0)
		{
			m_nAtt[epa_mount_num] = 0;
		}
		break;
	case epa_aptitude:
		//m_nAtt[EPA_Spirit]		= CalSpiritMax(m_nAtt[EPA_Aptitude], m_nStep, m_nGrade);
		//m_nAtt[EPA_Potential]	= CalPotential(m_nAtt[EPA_Aptitude]);
		//if(m_nAtt[epa_aptitude] > GetProto()->nAptitudeMax[GetAttVal(epa_quality)])
		//{
		//	m_nAtt[epa_aptitude] = GetProto()->nAptitudeMax[GetAttVal(epa_quality)];
		//}
		//else if (m_nAtt[epa_aptitude] < 0)
		//{
			m_nAtt[epa_aptitude] = 0;
		//}
		break;
 	case epa_mount_speed:
// 		{
// 			Role* pMaster = m_pSoul->GetMaster();
// 			if (VALID_POINT(pMaster) && m_pSoul->IsMounted())
// 			{
// 				pMaster->ModMountSpeed(m_nAtt[epa_mount_speed] - nSrcVal);
// 			}
// 		}
 		break;
	case epa_happy_value:
		{
			if (m_nAtt[epa_happy_value] > MAX_HAPPY_VALUE)
			{
				m_nAtt[epa_happy_value] = MAX_HAPPY_VALUE;
			}
			else if (m_nAtt[epa_happy_value] < 0)
			{
				m_nAtt[epa_happy_value] = 0;
			}
		}
		break;

	}

	if (bSend)
	{
		INT nSCPet = EPA2ECSPA(nPetAtt);	
		if (nSCPet != ECSPA_Null)
		{
			SyncToClientAttChg(nSCPet, GetAttVal(nPetAtt));
		}
	}
}

//-------------------------------------------------------------------------
// �޸�����ֵ
//-------------------------------------------------------------------------
VOID PetAtt::ModAttVal( INT nPetAtt, INT nMod, BOOL bSend /*= TRUE*/ )
{
	INT nSrcVal = m_nAtt[nPetAtt];
	m_nAtt[nPetAtt] += nMod;
	OnAttChg(nPetAtt, nSrcVal, bSend);
}

//-------------------------------------------------------------------------
// ��ȡ����
//-------------------------------------------------------------------------
VOID PetAtt::GetName( LPTSTR tszName ) const
{
	_tcsncpy(tszName, m_tszName, X_SHORT_NAME);

}

//-------------------------------------------------------------------------
// �����ݿ�
//-------------------------------------------------------------------------
BOOL PetAtt::SaveToDB( s_db_pet_att* pAtt )
{
	if (!VALID_POINT(m_pSoul) || !VALID_POINT(m_pProto))
	{
		ASSERT(0);
		return FALSE;
	}

	DWORD dwMasterID		= VALID_POINT(m_pSoul->GetMaster()) ? m_pSoul->GetMaster()->GetID() : INVALID_VALUE;

	GetName(pAtt->sz_name_);
	
	pAtt->dw_pet_id_			= m_dwPetID;						// ����ID
	pAtt->dw_proto_id_			= m_pProto->dw_data_id;				// ԭ��ID
	pAtt->dw_master_id_		= dwMasterID;						// ����ID
	pAtt->by_pet_state_		= m_PetState.GetState();			// ����״̬	

	pAtt->n_quality_			= m_nAtt[epa_quality];				// Ʒ��
	pAtt->n_aptitude_			= m_nAtt[epa_aptitude];				// ����
	pAtt->n_potential_		= m_nAtt[epa_potential];			// ��ǰǱ��
	pAtt->n_spirit_			= m_nAtt[epa_spirit];				// ��ǰ����
	pAtt->n_wuxing_energy_		= m_nAtt[epa_wuxing_energy];			// ������
	pAtt->n_talent_count_		= m_nAtt[epa_talent_count];			// �츳����
	

	pAtt->n_exp_cur_			= m_nExpCur;						// ��ǰ����
	pAtt->n_step_				= m_nStep;							// ��
	pAtt->n_grade_			= m_nGrade;							// ��
	pAtt->b_locked_			= m_bLocked;						// ����
	pAtt->n_rename_count_		= m_nRenameCount;					// ��������
	pAtt->n_happy_value_		= m_nAtt[epa_happy_value];			// ���Ķ�
	pAtt->n_color				= m_nAtt[epa_color];				// ��ɫ
	pAtt->n_strength			= m_nAtt[epa_strength];		
	pAtt->n_agility				= m_nAtt[epa_agility];		
	pAtt->n_innerForce			= m_nAtt[epa_innerForce];
	pAtt->n_att_point			= m_nAtt[epa_att_point];
	pAtt->dw_time_type			= m_dwTimeType;
	pAtt->dw_mode_type			= m_dwModeType;
	pAtt->dw_xiulian_time		= m_dwXiulianTime;

	return TRUE;
}

//-------------------------------------------------------------------------
// ö��ӳ��
//-------------------------------------------------------------------------
INT PetAtt::EPA2ECSPA( INT nEpa )
{
	switch(nEpa)
	{
	case epa_quality:
		return ECSPA_nQuality;
	case epa_aptitude:
		return ECSPA_nAptitude;
	case epa_spirit_max:
		return ECSPA_nSpiritMax;
	case epa_spirit:
		return ECSPA_nSpirit;
	case epa_potential:
		return ECSPA_nPotential;
	case epa_wuxing_energy:
		return ECSPA_nWuXingEnergy;
	case epa_mount_speed:
		return ECSPA_nMountSpeed;
	case epa_bag_grid:
		return ECSPA_BagGrid;
	case epa_mount_num:
		return ECSPA_nMountNum;
	case epa_mount_num_max:
		return ECSPA_nMountNumMax;
	case epa_happy_value:
		return ECSPA_nHappyValue;
	case epa_color:
		return ECSPA_nColor;
	case epa_strength:
		return ECSPA_nstrength;
	case epa_agility:
		return ECSPA_nagility;
	case epa_innerForce:
		return ECSPA_ninnerForce;
	case epa_att_point:
		return ECSPA_natt_point;
	default:
		return ECSPA_Null;
	}
}

INT	PetAtt::CalSpiritMax(INT quality)
{
	float fSpritMax = 25000;
	for (int i = 0; i < quality; i++)
	{
		fSpritMax = fSpritMax*1.2f;
	}

	return (INT)fSpritMax;

}
//-------------------------------------------------------------------------
// ͬ�����ͻ���
//-------------------------------------------------------------------------
void PetAtt::SyncToClientAttChg( INT nCSPetAtt, INT nCurVal )
{
	if (!VALID_POINT(m_pSoul->GetMaster()))
		return;

	NET_SIS_pet_att_change send;
	send.dwPetID		= m_dwPetID;
	send.u32NewValue	= nCurVal;
	send.u8AttrType		= nCSPetAtt;
	m_pSoul->GetMaster()->SendMessage(&send, send.dw_size);
}

//-------------------------------------------------------------------------
// ���ó���״̬
//-------------------------------------------------------------------------
VOID PetAtt::SetPetState( EPetState eState, BOOL bSync )
{
	if (!m_PetState.IsInState(eState))
	{
		m_PetState.SetState(eState);
		if (bSync)
		{
			SyncToClientAttChg(ECSPA_PetState, m_PetState.GetState());
		}		
	}
}

//-------------------------------------------------------------------------
// ȡ����������
//-------------------------------------------------------------------------
VOID PetAtt::UnSetPetState( EPetState eState, BOOL bSync )
{
	if (m_PetState.IsInState(eState))
	{
		m_PetState.UnsetState(eState);
		if (bSync)
		{
			SyncToClientAttChg(ECSPA_PetState, m_PetState.GetState());
		}		
	}
}

INT64 PetAtt::CalPourMoney()
{
	INT nVLevel = 0;
	TransStepGrade2VLevel(m_nStep, m_nGrade, nVLevel);
	const tagPetLvlUpProto* pLvlUpProto = AttRes::GetInstance()->GetPetLvlUpProto(nVLevel);
	if (!VALID_POINT(pLvlUpProto))
	{
		return 0;
	}

	INT32 nID = m_pSoul->GetMaster()->GetClass()*1000 + m_pSoul->GetMaster()->get_level();
	const s_level_up_effect* pEffect = AttRes::GetInstance()->GetLevelUpEffect(nID);

	return INT64(pEffect->n_exp_level_up_ / 10000.0f * (pLvlUpProto->nMoneyRatePourExpNeed) / 5.0f);
}

BYTE PetAtt::GetStateFlag() const
{
	BYTE byState = 0;
	byState |= ( m_PetState.IsInState(EPS_Called)		? EPSF_Called :		EPSF_UnCalled);
	byState |= ( m_PetState.IsInState(EPS_Working)		? EPSF_Working :	EPSF_UnWorking);
	byState |= ( m_PetState.IsInState(EPS_Preparing)	? EPSF_Preparing :	EPSF_UnPreparing);
	byState |= ( m_PetState.IsInState(EPS_Mounting)		? EPSF_Mounting:	EPSF_UnMounting);
	byState |= ( m_PetState.IsInState(EPS_Ronghe)		? EPSF_Ronghe:		EPSF_UnRonghe);
	byState |= ( m_PetState.IsInState(EPS_Dead)			? EPSF_Dead:		EPSF_UnDead);
	byState |= ( m_PetState.IsInState(EPS_Xiulian)		? EPSF_Xiulian:		EPSF_UnXiulian);
	return byState;
}

BOOL PetAtt::IsPetEatFood(EItemTypeReserved etr)
{
	if (etr < EITR_PETFOOD_Fish || etr > EITR_PETFOOD_Siliao)
		return FALSE;

	switch(etr)
	{
	case EITR_PETFOOD_Siliao:
		return TRUE;

	case EITR_PETFOOD_Fish:
		if (m_pProto->dwFoodLimit & EPF_fish)
			return TRUE;
		break;

	case EITR_PETFOOD_ROU:
		if (m_pProto->dwFoodLimit & FPF_Rou)
			return TRUE;
		break;

	case EITR_PETFOOD_Cai:
		if (m_pProto->dwFoodLimit & FPF_Cai)
			return TRUE;
		break;
	}

	return FALSE;
}

VOID PetAtt::SyncAllLvlUpChangeAtt()
{
	SyncToClientAttChg(EPA2ECSPA(epa_talent_count_max), GetAttVal(epa_talent_count_max));
	SyncToClientAttChg(EPA2ECSPA(epa_mount_speed), GetAttVal(epa_mount_speed));
	SyncToClientAttChg(EPA2ECSPA(epa_spirit_max), GetAttVal(epa_spirit_max));
}

BOOL PetAtt::UpStep( BOOL bSend /*= TRUE*/ )
{
	INT nLvlUpExp = GetExpLvlUp();
	INT nVLevel = 0;
	BOOL bLvlUp = FALSE;
	if (m_nExpCur >= nLvlUpExp && nLvlUpExp != 0 && GetGrade() != MAX_PET_GRADE)
	{
		TransStepGrade2VLevel(m_nStep, m_nGrade, nVLevel);
		nVLevel		+= 1;
		TransVLevel2StepGrade(nVLevel, m_nStep, m_nGrade);
		
		m_nExpCur		-= nLvlUpExp;

		m_pSoul->OnLevelUp();
		SyncAllLvlUpChangeAtt();
		
		
		
		nLvlUpExp	= GetExpLvlUp();
		bLvlUp		= TRUE;
	}

	if (bLvlUp && bSend)
	{
		// ͬ����ǰ����
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);

		// ͬ���������龭��
		SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

		// ͬ������ȼ�
		SyncToClientAttChg(ECSPA_nLevel, nVLevel);
	}

	return bLvlUp;
}

VOID PetAtt::SetLocked( BOOL bSet )
{
	if (bSet == m_bLocked)
	{
		return;
	}
	m_bLocked = bSet;

	SyncToClientAttChg(ECSPA_bLocked, m_bLocked);
}
