
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/

#pragma once

#include "../../common/WorldDefine/GMDefine.h"


//-----------------------------------------------------------------------------
// GM˫������
//-----------------------------------------------------------------------------
class GMDoublePolicy
{
public:
	//-----------------------------------------------------------------------------
	// ����������
	//-----------------------------------------------------------------------------
	GMDoublePolicy();
	virtual ~GMDoublePolicy(){}

public:
	//-----------------------------------------------------------------------------
	// ��ȡ���ʣ���ͼ�̵߳��ã�
	//-----------------------------------------------------------------------------
	virtual FLOAT	GetExpRate() const 							
	{	
		return m_bDouble[EGMDT_EXP] ? m_fCurDouble[EGMDT_EXP] : 1.0f;
	}
	virtual FLOAT	GetLootRate(BOOL bNormalMap) const	
	{
		return m_bDouble[EGMDT_ITEM] ? (bNormalMap ? m_fCurDouble[EGMDT_ITEM] : 1.0f) : 1.0f;	
	}

	virtual FLOAT	GetPracticeRate() const 							
	{	
		return m_bDouble[EGMDT_Practice] ? m_fCurDouble[EGMDT_Practice] : 1.0f;
	}

	virtual FLOAT GetDancingRate( ) const
	{
			return m_bDouble[EGMDT_Dacning] ? m_fCurDouble[EGMDT_Dacning] : 1.0f;
	}

public:
	//-----------------------------------------------------------------------------
	// ����˫�������̣߳�
	//-----------------------------------------------------------------------------
	virtual VOID	SetRate(EGMDoubleType eDoubleType, DWORD dwRate, DWORD dwStart, DWORD dwPersistSeconds );				

	//-----------------------------------------------------------------------------
	// ˫������
	//-----------------------------------------------------------------------------
	virtual VOID	DoubleSwitch(EGMDoubleType eType, BOOL bFlag) { m_bDouble[eType] = bFlag; }		

	//-----------------------------------------------------------------------------
	// ���£����̣߳�
	//-----------------------------------------------------------------------------
	VOID	Update();

private:
	FLOAT	m_fCurDouble[EGMDT_NUMBER];		// ˫������ 0-5
	BOOL	m_bDouble[EGMDT_NUMBER];			// ˫������
	DWORD	m_dwEndTime[EGMDT_NUMBER];		// ˫�����ʵĽ�ֹʱ��
};

//-----------------------------------------------------------------------------
// GM����
//-----------------------------------------------------------------------------
class GMPolicy
{
public:
	//-----------------------------------------------------------------------------
	// ����������
	//-----------------------------------------------------------------------------
	GMPolicy()
	{
		m_pDoublePolicy = new GMDoublePolicy;
	}
	virtual ~GMPolicy()
	{
		SAFE_DELETE(m_pDoublePolicy);
	}

public:
	//-----------------------------------------------------------------------------
	// ��ȡ���ʣ���ͼ�̵߳��ã�
	//-----------------------------------------------------------------------------
	FLOAT	GetExpRate() const 							
	{	
		return m_pDoublePolicy->GetExpRate();
	}
	FLOAT	GetLootRate(BOOL bNormalMap = TRUE) const	
	{
		return m_pDoublePolicy->GetLootRate(bNormalMap);
	}

	FLOAT GetPracticeRate() const 
	{
		return m_pDoublePolicy->GetPracticeRate( );
	}

	FLOAT GetDancingRate( ) const
	{
		return m_pDoublePolicy->GetDancingRate( );
	}

	//-----------------------------------------------------------------------------
	// ����˫�������̣߳�
	//-----------------------------------------------------------------------------
	VOID	SetRate(EGMDoubleType eDoubleType, DWORD dwRate, DWORD dwStart, DWORD dwPersistSeconds )
	{
		m_pDoublePolicy->SetRate(eDoubleType, dwRate, dwStart, dwPersistSeconds);
	}

	//-----------------------------------------------------------------------------
	// ˫������
	//-----------------------------------------------------------------------------
	VOID	DoubleSwitch(EGMDoubleType eType, BOOL bFlag) { m_pDoublePolicy->DoubleSwitch(eType, bFlag); }

public:
	//-----------------------------------------------------------------------------
	// ���£����̣߳�
	//-----------------------------------------------------------------------------
	VOID Update()
	{
		m_pDoublePolicy->Update();
	}

private:
	GMDoublePolicy*		m_pDoublePolicy;
};

extern GMPolicy g_GMPolicy;