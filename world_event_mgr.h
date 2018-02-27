/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//游戏世界事件触发

#pragma once

class WorldScript;

class CWorldEventMgr
{
public:
	CWorldEventMgr() {}
	~CWorldEventMgr() { m_pScript = NULL; }

	BOOL	Init();
	VOID	OnClock(BYTE byHour);

	VOID	OnAdventure();

private:
	const WorldScript*	m_pScript; 
};