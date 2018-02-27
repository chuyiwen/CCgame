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
*	@file		pet.h
*	@author		lc
*	@date		2010/12/17	initial
*	@version	0.0.1.0
*	@brief		����ʵ��
*/

#pragma once

#include "creature.h"

class PetSoul;
class PetTracker;
class Role;

//----------------------------------------------------------------------------------------------------
// ����ʵ��
//----------------------------------------------------------------------------------------------------
class Pet : public Creature
{
public:
	//----------------------------------------------------------------------------------------------------
	// ��PetSoul���ϲ���
	//----------------------------------------------------------------------------------------------------
	BOOL			IntegrateSoul(PetSoul* pSoul);
	VOID			DetachSoul();

	//----------------------------------------------------------------------------------------------------
	// �̳���Creature����
	//----------------------------------------------------------------------------------------------------
	virtual VOID	Update();
	virtual VOID	OnAttChange(INT nIndex);
	virtual DWORD	GetTypeID();

	//----------------------------------------------------------------------------------------------------
	// ������ɾ��
	//----------------------------------------------------------------------------------------------------
	static Pet*		Create(DWORD dwPetID, PetSoul* pSoul);
	static VOID		Delete(Pet* pToDel);
	
	//----------------------------------------------------------------------------------------------------
	// ����ӿ�
	//----------------------------------------------------------------------------------------------------
	BYTE			GetPetState();
	Role*			GetMaster();
	PetSoul*		GetSoul() const { return m_pSoul; }
	
	VOID			ResetHappyTimeCount() { nHappyTime = HAPPY_RESUME_COUNT; nSadTime = SAD_RESUME_COUNT; }

	VOID			SetDel() { bDel = TRUE; }
	BOOL			IsDel() { return bDel; }
private:
	//----------------------------------------------------------------------------------------------------
	// ����������
	//----------------------------------------------------------------------------------------------------
	Pet(DWORD dwID, DWORD dwMapID,  DWORD dwMasterID, Vector3& vPos, Vector3& vFace, PetSoul* pSoul);
	~Pet();
	BOOL			Init(Role* pMaster );

private:
	PetSoul*		m_pSoul;			// �������
	PetTracker*		m_pTracker;			// ������
	EPetModelType	m_ModelType;		// ����ģ��

	INT				nChangeExpTime;		// ������¼�ʱ
	INT				nHappyTime;			// ������¼�ʱ
	INT				nSadTime;			// ���ļ�ʱ
	BOOL			bDel;
};
