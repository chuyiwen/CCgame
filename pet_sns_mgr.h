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
*	@brief		宠物sns管理类
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
	//销毁
	VOID	destory();

	//添加一条sns信息
	BOOL	insertSNSinfo(tagPetSNSInfo* pInfo);
	
	BOOL	deleteSNSinfo(const DWORD dw_pet_id);

	//是否有派遣
	BOOL	IsPetPaiQian(DWORD dw_pet_id);

	tagPetSNSInfo* getPetSNSinfo(DWORD dw_pet_id);
	
	BOOL	IsFriendHasPaiqianPet(DWORD dw_master_id, DWORD dw_friend_id);
	//取得某个玩家派出的宠物信息
	VOID	getPetSNSinfoByMasterID(std::vector<tagPetSNSInfo*>& vec, DWORD masterID);
	
	//取得某人玩家收到的宠物sns信息
	VOID	getPetSNSinfoByFriendID(std::vector<tagPetSNSInfo*>& vec, DWORD friendID);
private:
	
	PetSNSInfoMap		pet_sns_info;


	Mutex mutex_;
};

extern Pet_Sns_mgr g_petSnsMgr;


#endif