/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/



#ifndef _TITLE_H_
#define _TITLE_H_

#include "../WorldDefine/achievement_define.h"

//-----------------------------------------------------------------------------
// 称号数目
//-----------------------------------------------------------------------------
const UINT16 MAX_TITLE_NUM			= 66;

//-----------------------------------------------------------------------------
// 称号效果数量
//-----------------------------------------------------------------------------
const INT	MAX_TITLE_SPECNUM		= 5;

//-----------------------------------------------------------------------------
// 称号ID是否有效
//-----------------------------------------------------------------------------
#define TITLEID_VALID( id ) ((id) >= 0 && (id) < MAX_TITLE_NUM)


////-----------------------------------------------------------------------------
//// 称号条件基类
////-----------------------------------------------------------------------------
//class condition
//{
//public:
//	condition(UINT16 n_title_id, DWORD dw_param1, DWORD dw_param2, BOOL b_show)
//		:n16_title_id_(n_title_id), dw_param1_(dw_param1), dw_param2_(dw_param2),b_show_(b_show){}
//	bool is_count_cond() const;
//	e_check_result check(DWORD dw_arg1, DWORD dw_arg2) {	return do_check(dw_arg1, dw_arg2);	}
//	void set_para2(DWORD dw_para2)	{	dw_param2_ = dw_para2; }
//	DWORD get_para2() const {return dw_param2_;}
//	BOOL  is_show() { return b_show_; }
//protected:
//	//	Role* GetRole() const {return m_pRole;}
//private:
//	virtual e_check_result do_check(DWORD dwArg1, DWORD dwArg2) = 0;
//
//protected:
//	DWORD	dw_param1_;
//	DWORD	dw_param2_;
//private:
//	UINT16	n16_title_id_;
//	BOOL	b_show_;
//	//	Role*	m_pRole;
//};
//
//
////-----------------------------------------------------------------------------
//// 计数条件
////-----------------------------------------------------------------------------
//class count_condition: public condition
//{
//public:
//	count_condition(INT16 n_title_id, DWORD dw_param1, DWORD dw_param2, BOOL b_show)
//		:condition(n_title_id, dw_param1, dw_param2, b_show){}
//
//private:
//	virtual e_check_result do_check(DWORD dwSubType, DWORD dwArg2)
//	{
//		if ( VALID_VALUE(dw_param1_) && dwSubType != dw_param1_) return ecr_noafect; 
//		if ((--dw_param2_) <= 0)
//		{
//			return ecr_active;
//		}
//		else
//		{
//			return ecr_count_down;
//		}
//	}
//};
//
////-----------------------------------------------------------------------------
//// 阈值条件
////-----------------------------------------------------------------------------
//class ValueCondition: public condition
//{
//public:
//	ValueCondition(UINT16 u16TitleID, DWORD dwPara1, DWORD dwPara2, BOOL IsShow)
//		:condition(u16TitleID, dwPara1, dwPara2, IsShow){}
//private:
//	virtual e_check_result do_check(DWORD dwSubType, DWORD dwValue)
//	{
//		if ( VALID_VALUE(dw_param1_) && dwSubType != dw_param1_) return ecr_noafect; 
//		if (dwValue >= dw_param2_)
//		{
//			return ecr_active;
//		}
//		else
//		{
//			return ecr_noafect; 
//		}
//	}
//};
//
////-----------------------------------------------------------------------------
//// 复杂检测条件
////-----------------------------------------------------------------------------
//class CheckCondition:public condition
//{
//public:
//	CheckCondition(UINT16 u16TitleID, DWORD dwPara1, DWORD dwPara2, BOOL IsShow)
//		:condition(u16TitleID, dwPara1, dwPara2, IsShow){}
//private:
//	virtual e_check_result do_check(DWORD dwArg1, DWORD dwArg2)
//	{
//		//		Role* pRole = GetRole();
//
//		return ecr_noafect;//pRole->CheckCondition();
//	}
//};

//-----------------------------------------------------------------------------
// 称号效果
//-----------------------------------------------------------------------------
struct tagTitleSpecEffect
{
	INT					nValue;			// 值
	DWORD				dwItemTypeID;	// 物品ID
};

//-----------------------------------------------------------------------------
// 称号原型
//-----------------------------------------------------------------------------
struct tagTitleProto
{
// 	enum
// 	{
// 		EVENTSNUM	= 1,
// 	};

	UINT16				m_u16ID;
	DWORD				m_dwBuffID;

	//e_condition_type 	m_CondType;
	//DWORD				m_dwPara1;
	//DWORD				m_dwPara2;

	BOOL				bShow;

	//tagTitleSpecEffect  st_TitleSpecEffect[MAX_TITLE_SPECNUM];

	//e_achievement_event	m_Events[EVENTSNUM];

	//// 生成条件
	//condition* MakeCond(DWORD dwPara1, DWORD dwPara2, BOOL IsShow) const 
	//{
	//	dwPara1 = VALID_VALUE(dwPara1) ? dwPara1 : m_dwPara1;
	//	dwPara2 = VALID_VALUE(dwPara2) ? dwPara2 : m_dwPara2;
	//	switch (m_CondType)
	//	{
	//	case ect_count:
	//		return new count_condition(m_u16ID, dwPara1, dwPara2, IsShow);
	//		break;
	//	case ect_value:
	//		return new ValueCondition(m_u16ID, dwPara1, dwPara2, IsShow);
	//		break;
	//	case ect_check:
	//		return new CheckCondition(m_u16ID, dwPara1, dwPara2, IsShow);
	//		break;
	//	default:
	//		ASSERT(0);
	//		return NULL;
	//		break;
	//	}
	//}
};


#endif