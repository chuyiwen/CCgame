/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宝箱管理

#pragma once

#include "../../common/WorldDefine/TreasureChest_define.h"
#include "role.h"

class TreasureChestMgr
{
private:
	vector< vector<tagChestItem> >	m_vecItems;						// 存放所有宝箱和物品，一个宝箱有16个物品
	
public:
	BOOL						Init();								// 初始化宝箱内物品
	vector<tagChestItem>&		GetChest(INT nIndex);				// 获得一种宝箱的引用
	BOOL						CanOpenChest(DWORD dwChestID, DWORD dwKeyID);	//判断能否开启宝箱
	VOID						Destroy();							// 销毁
	tagChestItem*				GetRandomItem(DWORD dwChestID, ERateType eRate, FLOAT fRand); //开出宝箱中物品
	ERateType					GetRate(Role* pRole);				// 得到当前宝箱掉率
	// 向客户端发送消息
	BOOL						SendMsg2Client(Role* pRole, DWORD dwChestID, const std::string strMsgName, 
											BOOL bOpened, BOOL bDestroy, DWORD dwItemID, INT nItemNum, DWORD dw_error_code);		
	
};

extern TreasureChestMgr g_TreasureChestMgr;