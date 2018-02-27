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
*	@file		pet.cpp
*	@author		lc
*	@date		2010/12/17	initial
*	@version	0.0.1.0
*	@brief		����ʵ��
*/

#include "StdAfx.h"
#include "pet.h"

#include "pet_soul.h"
#include "pet_tracker.h"
#include "role.h"
#include "pet_server_define.h"
#include "pet_pocket.h"
//----------------------------------------------------------------------------------------------------
// �����dumyԭ��
//----------------------------------------------------------------------------------------------------
//const DWORD TYPE_ID_PET = 9900001;

//----------------------------------------------------------------------------------------------------
// ��������ʵ��
//----------------------------------------------------------------------------------------------------
Pet* Pet::Create( DWORD dwPetID, PetSoul* pSoul )
{
	M_trans_else_ret(pMaster, pSoul->GetMaster(), Role, NULL);
	Pet* pNew = new Pet(dwPetID, pMaster->GetMapID(), pMaster->GetID(), pMaster->GetMoveData().m_vPos, pMaster->GetMoveData().m_vFace, pSoul);
	pNew->Init(pMaster);
	return pNew;
}

//----------------------------------------------------------------------------------------------------
// ɾ������ʵ��
//----------------------------------------------------------------------------------------------------
VOID Pet::Delete(Pet* pToDel)
{
	SAFE_DELETE(pToDel);
}

//----------------------------------------------------------------------------------------------------
// ���캯��
//----------------------------------------------------------------------------------------------------
Pet::Pet( DWORD dwID, DWORD dwMapID, DWORD dwMasterID, Vector3& vPos, Vector3& vFace, PetSoul* pSoul ) 
:Creature(dwID, dwMapID, vPos, vFace, ECST_ByRole, dwMasterID, INVALID_VALUE, FALSE),m_pSoul(pSoul),
nChangeExpTime(CHANGEEXP_RESUME_COUNT),
nHappyTime(HAPPY_RESUME_COUNT),nSadTime(SAD_RESUME_COUNT)
{
	bDel = FALSE;
	m_pTracker = new PetTracker(this);
	m_pTracker->SetTarget(pSoul->GetMaster());
	m_pTracker->SetTargetID(pSoul->GetMaster()->GetID());
}

//----------------------------------------------------------------------------------------------------
// ��������
//----------------------------------------------------------------------------------------------------
Pet::~Pet()
{
	SAFE_DELETE(m_pTracker);
}

//----------------------------------------------------------------------------------------------------
// ���������
//----------------------------------------------------------------------------------------------------
BOOL Pet::IntegrateSoul(PetSoul* pSoul)
{
	// precondition
	if (!VALID_POINT(pSoul) ||
		GetID() != pSoul->GetID())
	{
		return FALSE;
	}

	//Init PetSoul of this Object
	m_pSoul = pSoul;

	//IntegrateInPet PetSoul with this Pet
	if (!m_pSoul->IntegrateInPet(this))
	{
		return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------------------------------
// ��ʵ�����
//----------------------------------------------------------------------------------------------------
VOID Pet::DetachSoul()
{
	//DetachFromPet PetSoul
	m_pSoul->DetachFromPet();

	//Reset PetSoul to NULL
	m_pSoul = NULL;

}

//----------------------------------------------------------------------------------------------------
// ����
//----------------------------------------------------------------------------------------------------
VOID Pet::Update()
{

	UpdateBuff();
	m_pTracker->Update();
	
	INT nHappyValue = m_pSoul->GetPetAtt().GetAttVal(epa_happy_value);
	
	// ������ʱ,������
	if ( nHappyValue > 100)
	{
		// �����ٻ�ʱ����������
		if (--nChangeExpTime <= 0)
		{
			INT nExpChange = CHANGEEXP_VALUE;
			//����ʱ���ܶ��������
			if (nHappyValue > 200)
			{
				nExpChange = nExpChange * 2;
			}

			m_pSoul->GetPetAtt().ExpChange(nExpChange, TRUE, FALSE);
			nChangeExpTime = CHANGEEXP_RESUME_COUNT;
		}
	}
	if (nHappyValue > 0)
	{	// �������
		if (--nHappyTime <= 0)
		{	
			nHappyValue--;
			m_pSoul->GetPetAtt().ModAttVal(epa_happy_value, -1);
			nHappyTime = HAPPY_RESUME_COUNT;
			//�������Զ��ջ�
			if (nHappyValue == 0)
			{
				GetMaster()->GetPetPocket()->RestPet(m_pSoul->GetID());
				return;
			}
		}
	}

	if (nHappyValue == 0)
	{
		if (--nSadTime <= 0)
		{
			nSadTime = SAD_RESUME_COUNT;
			GetMaster()->GetPetPocket()->RestPet(m_pSoul->GetID());
			return;
		}
	}

	// ����Ϊ0���ջ�
	//if (m_pSoul->GetPetAtt().GetAttVal(epa_spirit) <=0)
	//{
	//	GetMaster()->GetPetPocket()->RestPet(m_pSoul->GetID());
	//}
}

//----------------------------------------------------------------------------------------------------
// ���Ըı�
//----------------------------------------------------------------------------------------------------
VOID Pet::OnAttChange(INT nIndex)
{
	switch(nIndex)
	{
	case ERA_Speed_XZ:
		m_fXZSpeed = X_DEF_XZ_SPEED * (m_nAtt[ERA_Speed_XZ] / 10000.0f);
		m_MoveData.StopMove();
		break;
	}

}

//----------------------------------------------------------------------------------------------------
// ��ʼ��
//----------------------------------------------------------------------------------------------------
BOOL Pet::Init(Role* pMaster)
{
	ASSERT(VALID_POINT(GetSoul()));

	const tagCreatureProto* pProto = AttRes::GetInstance()->GetCreatureProto(GetSoul()->GetPetAtt().GetProtoID());
	ASSERT(VALID_POINT(pProto));

	Creature::Init(pProto, NULL);

	
	SetSize(GetSoul()->GetProto()->vSize * GetSoul()->GetProto()->fScale);
	m_fXZSpeed = pMaster->GetXZSpeed();
	return TRUE;
}

//----------------------------------------------------------------------------------------------------
// ��ȡtypeid
//----------------------------------------------------------------------------------------------------
DWORD Pet::GetTypeID()
{
	return m_pSoul->GetProtoID();
}

//----------------------------------------------------------------------------------------------------
// ��ø���ģ��
//----------------------------------------------------------------------------------------------------
BYTE Pet::GetPetState()
{
	return GetSoul()->GetPetAtt().GetState();
}

//----------------------------------------------------------------------------------------------------
// �������
//----------------------------------------------------------------------------------------------------
Role* Pet::GetMaster()
{
	return GetSoul()->GetMaster();
}

