
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
// GM双倍策略
//-----------------------------------------------------------------------------
class GMDoublePolicy
{
public:
	//-----------------------------------------------------------------------------
	// 构造与析构
	//-----------------------------------------------------------------------------
	GMDoublePolicy();
	virtual ~GMDoublePolicy(){}

public:
	//-----------------------------------------------------------------------------
	// 获取倍率（地图线程调用）
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
	// 设置双倍（主线程）
	//-----------------------------------------------------------------------------
	virtual VOID	SetRate(EGMDoubleType eDoubleType, DWORD dwRate, DWORD dwStart, DWORD dwPersistSeconds );				

	//-----------------------------------------------------------------------------
	// 双倍开关
	//-----------------------------------------------------------------------------
	virtual VOID	DoubleSwitch(EGMDoubleType eType, BOOL bFlag) { m_bDouble[eType] = bFlag; }		

	//-----------------------------------------------------------------------------
	// 更新（主线程）
	//-----------------------------------------------------------------------------
	VOID	Update();

private:
	FLOAT	m_fCurDouble[EGMDT_NUMBER];		// 双倍倍率 0-5
	BOOL	m_bDouble[EGMDT_NUMBER];			// 双倍开启
	DWORD	m_dwEndTime[EGMDT_NUMBER];		// 双倍倍率的截止时间
};

//-----------------------------------------------------------------------------
// GM策略
//-----------------------------------------------------------------------------
class GMPolicy
{
public:
	//-----------------------------------------------------------------------------
	// 构造与析构
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
	// 获取倍率（地图线程调用）
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
	// 设置双倍（主线程）
	//-----------------------------------------------------------------------------
	VOID	SetRate(EGMDoubleType eDoubleType, DWORD dwRate, DWORD dwStart, DWORD dwPersistSeconds )
	{
		m_pDoublePolicy->SetRate(eDoubleType, dwRate, dwStart, dwPersistSeconds);
	}

	//-----------------------------------------------------------------------------
	// 双倍开关
	//-----------------------------------------------------------------------------
	VOID	DoubleSwitch(EGMDoubleType eType, BOOL bFlag) { m_pDoublePolicy->DoubleSwitch(eType, bFlag); }

public:
	//-----------------------------------------------------------------------------
	// 更新（主线程）
	//-----------------------------------------------------------------------------
	VOID Update()
	{
		m_pDoublePolicy->Update();
	}

private:
	GMDoublePolicy*		m_pDoublePolicy;
};

extern GMPolicy g_GMPolicy;