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
#include "BattleGround.h"

// 战场管理
class BattleGroundMgr
{

public:
	// 创建新战场
	BattleGround* CreateBattleGround();
	// 获取一个人数没满的战场
	BattleGround* GetIdleBattleGround();

private:
	
};