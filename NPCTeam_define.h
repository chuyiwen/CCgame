
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//NPC队伍相关定义

#pragma once


#pragma pack(push, 1)

const INT TEAM_WIDTH = 5;
const INT TEAM_HEIGH = 5;


// 怪物小队阵形
enum ENPCTeamOrder
{
	NTO_NO1  =  1,
	NTO_NO2, 
	NTO_NO3,
	NTO_NO4, 
	NTO_NO5, 
	NTO_NO6, 
	NTO_NO7, 
	NTO_NO8, 
	NTO_NO9, 
	NTO_NO10, 
	NTO_NO11, 
	NTO_NO12, 
	NTO_NO13, 
	NTO_NO14, 
	NTO_NO15, 
	NTO_NO16, 
	NTO_NO17, 
	NTO_NO18, 
	NTO_NO19, 
	NTO_NO20, 
	NTO_NO21, 
	NTO_NO22, 
	NTO_NO23, 
	NTO_NO24, 
	NTO_NO25, 
	NTO_NO26, 
	NTO_NO27, 
	NTO_NO28, 
	NTO_NO29, 
	NTO_NO30, 
	NTO_NO31, 
	NTO_NO32, 
	NTO_END,
};

// 怪物小队事件
enum ENPCTeamEvent
{
	NTE_NULL  =  0,
	NTE_SynEnmity,
};

// 怪物小队位置索引
struct tagNPCTeamOrder
{
	std::vector<POINT>		NPCOrder;
};

#pragma pack(pop)