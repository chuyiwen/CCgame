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
#include "../common/ServerDefine/gm_tool_define.h"
enum EDoubleAct
{
	EDA_Null		= -1,
	EDA_ExpStart	= 0,
	EDA_ExpStop		= 1,
	EDA_LootStart	= 10,
	EDA_LootStop	= 11,
};

inline EDoubleAct TransEDT2EDA(E_double_type	eEdt, BOOL bStart)
{
	if(eEdt == edt_exp)
	{
		if(bStart)	return EDA_ExpStart;
		else		return EDA_ExpStop;
	}
	else if (eEdt == edt_item)
	{
		if(bStart)	return EDA_LootStart;
		else		return EDA_LootStop;
	}
	return EDA_Null;
}

inline E_double_type TransEDA2EDT(EDoubleAct eEda)
{
	if (eEda == EDA_ExpStart ||	eEda == EDA_ExpStop)
	{
		return edt_exp;
	}
	else if (eEda == EDA_LootStart || eEda == EDA_LootStop)
	{
		return edt_item;
	}
	return edt_null;
}

inline BOOL TransEDA2BStart(EDoubleAct eEda)
{
	if (eEda == EDA_ExpStart ||	eEda == EDA_LootStart)
	{
		return TRUE;
	}
	else// if (eEda == EDA_ExpStop || eEda == EDA_LootStop)
	{
		return FALSE;
	}
}
