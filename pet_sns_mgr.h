/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
/**
*	@file		pet_sns_mgr.h
*	@author		lc
*	@date		2011/6/30	initial
*	@version	0.0.1.0
*	@brief		����sns������
*/

#ifndef _PET_SNS_MGR_H_
#define _PET_SNS_MGR_H_

#include "mutex.h"
#include "../../common/WorldDefine/pet_define.h"

class Pet_Sns_mgr
{
public:
	typedef package_map<DWORD, tagPetSNSInfo*> PetSNSInfoMap;
public:
	Pet_Sns_mgr() { pet_sns_info.clear(); }
	~Pet_Sns_mgr() {};
	
	VOID	initList(tagPetSNSInfo* pInfo, int nNum);
	//����
	VOID	destory();

	//���һ��sns��Ϣ
	BOOL	insertSNSinfo(tagPetSNSInfo* pInfo);
	
	BOOL	deleteSNSinfo(const DWORD dw_pet_id);

	//�Ƿ�����ǲ
	BOOL	IsPetPaiQian(DWORD dw_pet_id);

	tagPetSNSInfo* getPetSNSinfo(DWORD dw_pet_id);
	
	BOOL	IsFriendHasPaiqianPet(DWORD dw_master_id, DWORD dw_friend_id);
	//ȡ��ĳ������ɳ��ĳ�����Ϣ
	VOID	getPetSNSinfoByMasterID(std::vector<tagPetSNSInfo*>& vec, DWORD masterID);
	
	//ȡ��ĳ������յ��ĳ���sns��Ϣ
	VOID	getPetSNSinfoByFriendID(std::vector<tagPetSNSInfo*>& vec, DWORD friendID);
private:
	
	PetSNSInfoMap		pet_sns_info;


	Mutex mutex_;
};

extern Pet_Sns_mgr g_petSnsMgr;


#endif