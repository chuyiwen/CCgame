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
*	@brief		宠物实体
*/

#pragma once

#include "creature.h"

class PetSoul;
class PetTracker;
class Role;

//----------------------------------------------------------------------------------------------------
// 宠物实体
//----------------------------------------------------------------------------------------------------
class Pet : public Creature
{
public:
	//----------------------------------------------------------------------------------------------------
	// 与PetSoul整合部分
	//----------------------------------------------------------------------------------------------------
	BOOL			IntegrateSoul(PetSoul* pSoul);
	VOID			DetachSoul();

	//----------------------------------------------------------------------------------------------------
	// 继承自Creature部分
	//----------------------------------------------------------------------------------------------------
	virtual VOID	Update();
	virtual VOID	OnAttChange(INT nIndex);
	virtual DWORD	GetTypeID();

	//----------------------------------------------------------------------------------------------------
	// 创建与删除
	//----------------------------------------------------------------------------------------------------
	static Pet*		Create(DWORD dwPetID, PetSoul* pSoul);
	static VOID		Delete(Pet* pToDel);
	
	//----------------------------------------------------------------------------------------------------
	// 对外接口
	//----------------------------------------------------------------------------------------------------
	BYTE			GetPetState();
	Role*			GetMaster();
	PetSoul*		GetSoul() const { return m_pSoul; }
	
	VOID			ResetHappyTimeCount() { nHappyTime = HAPPY_RESUME_COUNT; nSadTime = SAD_RESUME_COUNT; }

	VOID			SetDel() { bDel = TRUE; }
	BOOL			IsDel() { return bDel; }
private:
	//----------------------------------------------------------------------------------------------------
	// 构造与析构
	//----------------------------------------------------------------------------------------------------
	Pet(DWORD dwID, DWORD dwMapID,  DWORD dwMasterID, Vector3& vPos, Vector3& vFace, PetSoul* pSoul);
	~Pet();
	BOOL			Init(Role* pMaster );

private:
	PetSoul*		m_pSoul;			// 宠物灵魂
	PetTracker*		m_pTracker;			// 跟踪器
	EPetModelType	m_ModelType;		// 宠物模型

	INT				nChangeExpTime;		// 经验更新计时
	INT				nHappyTime;			// 心情更新计时
	INT				nSadTime;			// 伤心计时
	BOOL			bDel;
};
