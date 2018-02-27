/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物属性

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
// 生成资质
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
//// 骑乘速度
////-------------------------------------------------------------------------------------------------------
//INT MountSpeedStepAdd[NUM_PET_STEP] =
//{
//	0,		// 0阶0%
//	100,	// 1阶1%
//	200,	// 2阶2%
//	400,	// 3阶4%
//	600,	// 4阶6%
//	900,	// 5阶9%
//	1200,	// 6阶12%
//	1600,	// 7阶16%
//	2000,	// 8阶20%
//	2500	// 9阶25%
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
// 计算升级经验
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
// 初始化数据库创建宠物需要的数据
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
	pCreate->n_potential_		= CalPotential(pCreate->n_aptitude_);//资质×10
}

//-------------------------------------------------------------------------
// 初始化
//-------------------------------------------------------------------------
BOOL PetAtt::InitAtt( const s_db_pet_att* pAtt)
{
	m_pProto					= AttRes::GetInstance()->GetPetProto(pAtt->dw_proto_id_);
	if (!VALID_POINT(m_pProto) || !VALID_POINT(pAtt))
	{
		ASSERT(0);
		return FALSE;
	}

	//---------------------------------初始化数据库属性-----------------------------
	_tcsncpy(m_tszName, pAtt->sz_name_, X_SHORT_NAME);
	m_PetState.InitState(pAtt->by_pet_state_ & 0xee);				//状态
	m_dwProtoID					= pAtt->dw_proto_id_;
	m_nExpCur					= pAtt->n_exp_cur_;		//当前经验	
	m_nStep						= pAtt->n_step_;		//阶
	m_nGrade					= pAtt->n_grade_;		//等
	m_dwPetID					= pAtt->dw_pet_id_;		//宠物id
	m_bLocked					= pAtt->b_locked_;		//锁定
	m_nRenameCount				= pAtt->n_rename_count_;	//改名次数
	m_dwTimeType				= pAtt->dw_time_type;
	m_dwModeType				= pAtt->dw_mode_type;
	m_dwXiulianTime				= pAtt->dw_xiulian_time;

	m_nAtt[epa_quality]			= pAtt->n_quality_;		//读取品质
	m_nAtt[epa_aptitude]		= pAtt->n_aptitude_;		//读取资质
	m_nAtt[epa_spirit]			= pAtt->n_spirit_;		//读取当前灵力
	m_nAtt[epa_potential]		= pAtt->n_potential_;		//读取当前潜能
	m_nAtt[epa_talent_count]	= pAtt->n_talent_count_;	//读取天赋计数
	m_nAtt[epa_wuxing_energy]	= pAtt->n_wuxing_energy_;	//读取五行力
	m_nAtt[epa_happy_value]		= pAtt->n_happy_value_;
	m_nAtt[epa_color]			= pAtt->n_color;
	m_nAtt[epa_strength]		= pAtt->n_strength;
	m_nAtt[epa_agility]			= pAtt->n_agility;
	m_nAtt[epa_innerForce]		= pAtt->n_innerForce;
	m_nAtt[epa_att_point]		= pAtt->n_att_point;


	//---------------------------------初始化默认值---------------------------------
	m_nAtt[epa_spirit_max]		= 0;					//灵力最大值
	m_nAtt[epa_talent_count_max]	= 0;					//读取天赋计数
	m_nAtt[epa_mount_num]		= 0;					//骑乘数目
	m_nAtt[epa_mount_num_max]		= 0;					//骑乘数目最大值

	m_nAtt[epa_bag_grid]			= 0;					//行囊格数	默认4，		装备加成2~16
	m_nAtt[epa_delivery_consume]	= 0;					//快递耗时	默认0，		装备加成-？~0：ms，对应技能表“工作时间”
	m_nAtt[epa_sell_consume]		= 0;					//贩卖耗时	默认0，		装备加成-？~0：ms，对应技能表“工作时间”
	m_nAtt[epa_storage_consume]	= 0;					//存放耗时	默认0，		装备加成-？~0：ms，对应技能表“工作时间”
	m_nAtt[epa_gather_consume]	= 0;					//采集耗时	默认0，		装备加成-？~0：ms，对应技能表“工作时间”
	m_nAtt[epa_train_resume]		= 0;					//训练恢复	默认0，		装备加成-？~0：ms，对应技能表“恢复时间”
	m_nAtt[epa_spirit_rate]		= 0;					//灵力消耗	默认100%，	装备加成-100%~0%，对应技能表energy_cost
	m_nAtt[epa_exp_rate]			= 0;					//经验加成	默认100%，	装备加成0%~400%
	m_nAtt[epa_pick_up_resume]	= 0;					//拾取恢复	默认0，		装备加成-20~-1
	m_nAtt[epa_medicine_saving]	= 0;					//吃药节省	默认0%，	装备加成1%~5%
	m_nAtt[epa_strength_effect]	= 0;					//强身效果	默认100%，	装备加成0%~400%
	m_nAtt[epa_wuxing_consume]	= 0;					//五行力消耗	默认100%，	装备加成-50%~0%，对应技能表“五行力消耗”

	m_nAtt[epa_mount_speed]		= 0;					//骑乘速度	默认品质和阶计算，装备加成0%~100%
	
	return TRUE;
}

//-------------------------------------------------------------------------
// 修改经验
//-------------------------------------------------------------------------
VOID PetAtt::ExpChange( INT nExpMod, BOOL bSend, BOOL bOverFlow )
{
	if (0 == nExpMod)
		return;
	nExpMod *= (GetAttVal(epa_exp_rate) / 10000.0f);

	INT		nLvlUpExp = 0;
	INT		nVLevel = 0;
	BOOL	bLvlUp	= FALSE;

	// 减少经验
	if (nExpMod < 0)
	{
		m_nExpCur += nExpMod;
		m_nExpCur = m_nExpCur < 0 ? 0 : m_nExpCur;
	}
	// 增加经验
	else if (nExpMod > 0)
	{
		nLvlUpExp = GetExpLvlUp();
		m_nExpCur += nExpMod;

		while (m_nExpCur >= nLvlUpExp && nLvlUpExp != 0)
		{
			// 若达到当前等最顶级 且 灌注经验满
			if (nLvlUpExp <= m_nExpCur && MAX_PET_GRADE == m_nGrade)
			{
				m_nExpCur = nLvlUpExp;
				break;
			}
			// 计算新等级
			TransStepGrade2VLevel(m_nStep, m_nGrade, nVLevel);

			// 必须蜕变才能升级
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
		// 同步当前经验
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);
		if (bLvlUp)
		{
			// 同步升级经验经验
			SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

			// 同步虚拟等级
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
		// 同步当前经验
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);

		// 同步升级经验经验
		SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

		// 同步虚拟等级
		SyncToClientAttChg(ECSPA_nLevel, nLevel);

	}

}
//-------------------------------------------------------------------------
// 获取属性值
//-------------------------------------------------------------------------
INT PetAtt::GetAttVal( INT nPetAtt )
{
	return m_nAtt[nPetAtt] + GetAttDef(nPetAtt);
}

//-------------------------------------------------------------------------
// 设置属性值
//-------------------------------------------------------------------------
VOID PetAtt::SetAttVal(INT nPetAtt, INT nValue, BOOL bSend /* = TRUE */)
{
	INT nSrcVal = m_nAtt[nPetAtt];
	m_nAtt[nPetAtt] = nValue;
	OnAttChg(nPetAtt, nSrcVal, bSend);
}

//-------------------------------------------------------------------------
// 加成属性的默认值
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
// 属性值变化触发
//-------------------------------------------------------------------------
VOID PetAtt::OnAttChg( INT nPetAtt, INT nSrcVal, BOOL bSend /*= TRUE*/ )
{
	// 额外通知
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
// 修改属性值
//-------------------------------------------------------------------------
VOID PetAtt::ModAttVal( INT nPetAtt, INT nMod, BOOL bSend /*= TRUE*/ )
{
	INT nSrcVal = m_nAtt[nPetAtt];
	m_nAtt[nPetAtt] += nMod;
	OnAttChg(nPetAtt, nSrcVal, bSend);
}

//-------------------------------------------------------------------------
// 获取名字
//-------------------------------------------------------------------------
VOID PetAtt::GetName( LPTSTR tszName ) const
{
	_tcsncpy(tszName, m_tszName, X_SHORT_NAME);

}

//-------------------------------------------------------------------------
// 存数据库
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
	
	pAtt->dw_pet_id_			= m_dwPetID;						// 宠物ID
	pAtt->dw_proto_id_			= m_pProto->dw_data_id;				// 原型ID
	pAtt->dw_master_id_		= dwMasterID;						// 主人ID
	pAtt->by_pet_state_		= m_PetState.GetState();			// 灵兽状态	

	pAtt->n_quality_			= m_nAtt[epa_quality];				// 品质
	pAtt->n_aptitude_			= m_nAtt[epa_aptitude];				// 资质
	pAtt->n_potential_		= m_nAtt[epa_potential];			// 当前潜能
	pAtt->n_spirit_			= m_nAtt[epa_spirit];				// 当前灵力
	pAtt->n_wuxing_energy_		= m_nAtt[epa_wuxing_energy];			// 五行力
	pAtt->n_talent_count_		= m_nAtt[epa_talent_count];			// 天赋计数
	

	pAtt->n_exp_cur_			= m_nExpCur;						// 当前经验
	pAtt->n_step_				= m_nStep;							// 阶
	pAtt->n_grade_			= m_nGrade;							// 等
	pAtt->b_locked_			= m_bLocked;						// 锁定
	pAtt->n_rename_count_		= m_nRenameCount;					// 改名次数
	pAtt->n_happy_value_		= m_nAtt[epa_happy_value];			// 开心度
	pAtt->n_color				= m_nAtt[epa_color];				// 颜色
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
// 枚举映射
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
// 同步到客户端
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
// 设置宠物状态
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
// 取消宠物属性
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
		// 同步当前经验
		SyncToClientAttChg(ECSPA_nExpCurrent, m_nExpCur);

		// 同步升级经验经验
		SyncToClientAttChg(ECSPA_nExpLevelUp, nLvlUpExp);

		// 同步虚拟等级
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
