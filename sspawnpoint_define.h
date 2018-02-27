
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//服务器用非副本刷怪点原型


// 每个刷怪点怪物最多种类
const INT MAX_CREATURE_PER_SSPAWNPOINT	= 50;

//--------------------------------------------------------------------------------
// 服务器端刷怪点结构（普通地图）
//--------------------------------------------------------------------------------
struct tagSSpawnPointProto
{
	DWORD				dwSpawnPointID;								// 刷怪点id
	DWORD				dwTypeIDs[MAX_CREATURE_PER_SSPAWNPOINT];	// 怪物typeid

	tagSSpawnPointProto() { ZeroMemory(this, sizeof(*this)); }
};
