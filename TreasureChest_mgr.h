/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//�������

#pragma once

#include "../../common/WorldDefine/TreasureChest_define.h"
#include "role.h"

class TreasureChestMgr
{
private:
	vector< vector<tagChestItem> >	m_vecItems;						// ������б������Ʒ��һ��������16����Ʒ
	
public:
	BOOL						Init();								// ��ʼ����������Ʒ
	vector<tagChestItem>&		GetChest(INT nIndex);				// ���һ�ֱ��������
	BOOL						CanOpenChest(DWORD dwChestID, DWORD dwKeyID);	//�ж��ܷ�������
	VOID						Destroy();							// ����
	tagChestItem*				GetRandomItem(DWORD dwChestID, ERateType eRate, FLOAT fRand); //������������Ʒ
	ERateType					GetRate(Role* pRole);				// �õ���ǰ�������
	// ��ͻ��˷�����Ϣ
	BOOL						SendMsg2Client(Role* pRole, DWORD dwChestID, const std::string strMsgName, 
											BOOL bOpened, BOOL bDestroy, DWORD dwItemID, INT nItemNum, DWORD dw_error_code);		
	
};

extern TreasureChestMgr g_TreasureChestMgr;