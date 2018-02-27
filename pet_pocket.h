/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物容器

#pragma once

class PetSoul;
class Pet;
class Role;
struct s_db_pet;

//----------------------------------------------------------------------------------------------------
// 宠物容器
//----------------------------------------------------------------------------------------------------
class PetPocket
{
	typedef	package_map<DWORD, PetSoul*>	SoulTMap;
	typedef	package_map<DWORD, Pet*>		PetTMap;
	typedef package_list<PetSoul*>			SoulTList;

public:
	//----------------------------------------------------------------------------------------------------
	// 构造和析构
	//----------------------------------------------------------------------------------------------------
	~PetPocket(){	Destroy();	}

	//----------------------------------------------------------------------------------------------------
	// 公共接口
	//----------------------------------------------------------------------------------------------------
	BOOL		Init(const BYTE* &pData, INT n_num, Role* pRole, INT16 nSize);
	BOOL		SaveToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID		Destroy();
	VOID		Update();

public:
	//----------------------------------------------------------------------------------------------------
	// 拿走放入
	//----------------------------------------------------------------------------------------------------
	PetSoul*	GetAway(DWORD dwPetID, BOOL bSync = FALSE);
	DWORD		PutIn(PetSoul* pSoul, BOOL bSend = TRUE);

	//----------------------------------------------------------------------------------------------------
	// 获得
	//----------------------------------------------------------------------------------------------------
	VOID		GetAllPetID(DWORD* dwPetIDbuf, INT& num);
	PetSoul*	GetPetSoul(DWORD dwPetID);
	PetSoul*	GetCalledPetSoul();
	INT16		GetSize() { return m_nSize; }
	//----------------------------------------------------------------------------------------------------
	// 生成宠物
	//----------------------------------------------------------------------------------------------------
	DWORD		HatchEgg(INT64 n64ItemID, LPCTSTR tszName);
	
	// 扩充宠物栏
	DWORD		SpePetItem(INT64 n64ItemID);
	DWORD		ChangeSize();
	//----------------------------------------------------------------------------------------------------
	// 锁定解锁
	//----------------------------------------------------------------------------------------------------
	DWORD		LockPet(DWORD dwPetID, INT64 n64ItemID);
	DWORD		UnLockPet(DWORD dwPetID, INT64 n64ItemID);

	//----------------------------------------------------------------------------------------------------
	// 增强宠物
	//----------------------------------------------------------------------------------------------------
	DWORD		CalledSoulEnhance(INT64 n64ItemID);
	DWORD		CalledSoulUpStep(INT64 n64ItemID, DWORD &dwSkillID, INT &nDstStep);
	DWORD		ReName(DWORD dwPetID, LPCTSTR tszName);
	//----------------------------------------------------------------------------------------------------
	// 召唤休息
	//----------------------------------------------------------------------------------------------------
	DWORD		CallPet(DWORD dwPetID);
	DWORD		RestPet(DWORD dwPetID, BOOL bDestory = FALSE);
	DWORD		RestAPet();
	
	// 所有宠物取消合体状态
	VOID		RestAllPetHeti();

	//----------------------------------------------------------------------------------------------------
	// 召唤的宠物喂食
	//----------------------------------------------------------------------------------------------------
	DWORD		CalledPetFeed(INT64 n64ItemID, DWORD dwFoodType);
	
	DWORD		InitRandomSkill(DWORD dwPetID);

	//复活宠物
	DWORD		RebornPet(DWORD dwPetID, INT64 dw64ItemID);

	//染色
	DWORD		PetColor(DWORD dwPetID, INT nColor );

	//蜕变
	DWORD		PetChange(DWORD dwPetID, INT64 dw64ItemID);

	//点属性
	DWORD		PetAddPoint(DWORD dwPetID, BYTE byType, INT nValue);

	//宠物融合	
	DWORD		FusionPet(DWORD dwPetID1, DWORD dwPetID2, INT64 dw64ItemID1);
	
	//宠物寄养
	DWORD		petXiulian(DWORD dwPetID, DWORD dwNpcID, DWORD dwTimeType, DWORD dwModeType);

	//宠物寄养收回
	DWORD		petXiulianRetrun(DWORD dwPetID);

	INT			GetXiulianNumber();

	INT			GetMaxPetLevel();
public:
	//----------------------------------------------------------------------------------------------------
	// 召唤宠物进出宠物带
	//----------------------------------------------------------------------------------------------------
	VOID		CalledPetEnterPocket();
	VOID		CalledPetLeavePocket();

	//----------------------------------------------------------------------------------------------------
	// 宠物交易相关
	//----------------------------------------------------------------------------------------------------
	BOOL		CheckExistInPocket(DWORD *dwPetIDs, INT n_num );
	DWORD		CanExchange(DWORD dwPetID);
	BOOL		GetFreeSize();
	VOID		TakeFromPocket(PetSoul* *pSouls, INT nSizeSouls, DWORD* dwPetIDs, INT nNumPetID);
	VOID		PutInPocket(PetSoul* *pSouls, INT nSizeSouls);

private:
	Role*			m_pMaster;			// 主人
	SoulTMap		m_mapAllSoul;		// 所有PetSoul
	INT				m_nCalledPets;		// 已经召唤的宠物数量
	DWORD			m_dwPetIDForUpStep;	// 升阶的宠物id
	
	INT16			m_nSize;
public:
	//----------------------------------------------------------------------------------------------------
	// 测试
	//----------------------------------------------------------------------------------------------------
	BOOL		DynamicTest(INT nTestNo, INT nArg1, INT nArg2);

};


