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

#pragma once

#include "../../common/WorldDefine/pet_define.h"
#include "../common/ServerDefine/pet_server_define.h"
#include "../../common/WorldDefine/container_define.h"
#include "state_mgr.h"

struct	s_db_pet_soul_create;
struct	tagPetProto;
struct	s_db_pet_att;
class	PetSoul;

//-------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------
class PetAtt
{
	typedef	State<BYTE, EPetState>		PetState;
public:
	//---------------------------------------------------------------------
	// �����ʼ����洢
	//---------------------------------------------------------------------
	PetAtt(PetSoul* pSoul)
		:m_pSoul(pSoul){}
	static VOID InitCreateAtt(s_db_pet_soul_create* pCreate, DWORD dwPetTypeID, DWORD dwOwnerID, LPCTSTR szName, INT nQuality);
	BOOL		InitAtt(const s_db_pet_att* pAtt);
	BOOL		SaveToDB(s_db_pet_att* pAtt);

	//---------------------------------------------------------------------
	// �������ԵĲ���
	//---------------------------------------------------------------------
	VOID		ModAttVal(INT nPetAtt, INT nMod, BOOL bSend = TRUE);
	VOID		SetAttVal(INT nPetAtt, INT nValue, BOOL bSend = TRUE);
	INT			GetAttVal(INT nPetAtt);
	VOID		ExpChange(INT nExpMod, BOOL bSend, BOOL bOverFlow = TRUE);	
	VOID		ChangeLevel(INT nLevel, BOOL bSend);
	BOOL		UpStep(BOOL bSend = TRUE);
	VOID		SetLocked(BOOL bSet);
	BOOL		IsLocked()				const { return m_bLocked;	}
	VOID		ModRenameCount(INT nCount) { m_nRenameCount += nCount; SyncToClientAttChg(ECSPA_nRenameCount, m_nRenameCount);}
	INT			GetRenameCount() { return m_nRenameCount; }

	VOID		SetTimeType(DWORD dwType) { m_dwTimeType = dwType; SyncToClientAttChg(ECSPA_time_type, m_dwTimeType);}
	DWORD		GetTimeType() { return m_dwTimeType; }
	VOID		SetModeType(DWORD dwType) { m_dwModeType = dwType; SyncToClientAttChg(ECSPA_mode_type, m_dwModeType);}
	DWORD		GetModeType() { return m_dwModeType; }
	
	VOID		SetXiulianTime(DWORD dwTime) { m_dwXiulianTime = dwTime; SyncToClientAttChg(ECSPA_xiulian_time, m_dwXiulianTime);}
	tagDWORDTime		GetXiulianTime() { return (tagDWORDTime)m_dwXiulianTime; }

	//---------------------------------------------------------------------
	// һЩget
	//---------------------------------------------------------------------
	const tagPetProto* GetProto()		const {	return m_pProto;						}
	DWORD		GetID()					const { return m_dwPetID;						}
 	INT			GetProtoID()			const {	return m_dwProtoID;						}
	VOID		GetName(LPTSTR tszName)	const;
	VOID		SetName(LPCTSTR tszName) { _tcsncpy(m_tszName, tszName, X_SHORT_NAME); ModRenameCount(1);}
	INT			GetGrade()				const {	return m_nGrade;						}
	INT			GetStep()				const {	return m_nStep;							}
	INT			GetVLevel()				const {	INT nVLevel = 0;	TransStepGrade2VLevel(m_nStep, m_nGrade, nVLevel);	return nVLevel;	}
	INT			GetCurExp()				const {	return m_nExpCur;						}
	INT			GetExpLvlUp()			const {	return CalLvlUpExp(m_nStep, m_nGrade);	}
	INT64		CalPourMoney();
	BYTE		GetStateFlag()			const;
	
	BOOL		IsPetEatFood(EItemTypeReserved etr);
	//---------------------------------------------------------------------
	// ����״̬
	//---------------------------------------------------------------------
	BYTE		GetState()						const { return m_PetState.GetState();				}
	VOID		SetPetState(EPetState eState, BOOL bSync = TRUE);
	VOID		UnSetPetState(EPetState eState, BOOL bSync = TRUE);
	BOOL		IsPetInState(EPetState eState)	const {	return m_PetState.IsInState(eState);		}
	BOOL		IsPetInStateAny(BYTE byState)	const {	return m_PetState.IsInStateAny(byState);	}
	BOOL		IsPetInStateAll(BYTE byState)	const {	return m_PetState.IsInStateAll(byState);	}

private:
	//---------------------------------------------------------------------
	// ʵ��
	//---------------------------------------------------------------------
	INT			EPA2ECSPA(INT nEpa);
//	static INT	CalAptitude(INT nQuality, const tagPetProto* pPetProto);
//	static INT	CalMountSpeedbBase(INT nQuality, INT nStep);
	static INT	CalSpiritMax(INT quality);
	static INT	CalLvlUpExp(INT nStep, INT nGrade);
	static INT	CalTalentCountMax(INT nQuality, INT nStep, INT nGrade)	{	return (1 + nQuality) * 3 + nStep;				}
	static INT	CalPotential(INT nAptitude)								{	return nAptitude * 10;						}

	INT			GetAttDef(INT nPetAtt);
	VOID		OnAttChg(INT nPetAtt, INT nSrcVal, BOOL bSend = TRUE);

	VOID		SyncToClientAttChg( INT nCSPetAtt, INT nCurVal );
	VOID		SyncAllLvlUpChangeAtt();
private:
	//---------------------------------------------------------------------
	// ��Ա����
	//---------------------------------------------------------------------
	INT			m_nExpCur;			//��ǰ����	
	INT			m_nStep;			//��
	INT			m_nGrade;			//��
	INT			m_nRenameCount;		//��������
	PetState	m_PetState;			//״̬
	BOOL		m_bLocked;			//����
	
	DWORD		m_dwXiulianTime;	//������ʼʱ��
	DWORD		m_dwTimeType;		//����ʱ������
	DWORD		m_dwModeType;		//����ģʽ����

	TCHAR		m_tszName[X_SHORT_NAME];
	DWORD		m_dwPetID;
	DWORD		m_dwProtoID;

	INT			m_nAtt[EPA_NUM];

	const tagPetProto* m_pProto;
	PetSoul*	m_pSoul;
};
