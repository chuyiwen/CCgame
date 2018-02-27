/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//��������

#pragma once

class PetSoul;
class Pet;
class Role;
struct s_db_pet;

//----------------------------------------------------------------------------------------------------
// ��������
//----------------------------------------------------------------------------------------------------
class PetPocket
{
	typedef	package_map<DWORD, PetSoul*>	SoulTMap;
	typedef	package_map<DWORD, Pet*>		PetTMap;
	typedef package_list<PetSoul*>			SoulTList;

public:
	//----------------------------------------------------------------------------------------------------
	// ���������
	//----------------------------------------------------------------------------------------------------
	~PetPocket(){	Destroy();	}

	//----------------------------------------------------------------------------------------------------
	// �����ӿ�
	//----------------------------------------------------------------------------------------------------
	BOOL		Init(const BYTE* &pData, INT n_num, Role* pRole, INT16 nSize);
	BOOL		SaveToDB(IN LPVOID pData, OUT LPVOID &pOutPointer, OUT INT32 &n_num);
	VOID		Destroy();
	VOID		Update();

public:
	//----------------------------------------------------------------------------------------------------
	// ���߷���
	//----------------------------------------------------------------------------------------------------
	PetSoul*	GetAway(DWORD dwPetID, BOOL bSync = FALSE);
	DWORD		PutIn(PetSoul* pSoul, BOOL bSend = TRUE);

	//----------------------------------------------------------------------------------------------------
	// ���
	//----------------------------------------------------------------------------------------------------
	VOID		GetAllPetID(DWORD* dwPetIDbuf, INT& num);
	PetSoul*	GetPetSoul(DWORD dwPetID);
	PetSoul*	GetCalledPetSoul();
	INT16		GetSize() { return m_nSize; }
	//----------------------------------------------------------------------------------------------------
	// ���ɳ���
	//----------------------------------------------------------------------------------------------------
	DWORD		HatchEgg(INT64 n64ItemID, LPCTSTR tszName);
	
	// ���������
	DWORD		SpePetItem(INT64 n64ItemID);
	DWORD		ChangeSize();
	//----------------------------------------------------------------------------------------------------
	// ��������
	//----------------------------------------------------------------------------------------------------
	DWORD		LockPet(DWORD dwPetID, INT64 n64ItemID);
	DWORD		UnLockPet(DWORD dwPetID, INT64 n64ItemID);

	//----------------------------------------------------------------------------------------------------
	// ��ǿ����
	//----------------------------------------------------------------------------------------------------
	DWORD		CalledSoulEnhance(INT64 n64ItemID);
	DWORD		CalledSoulUpStep(INT64 n64ItemID, DWORD &dwSkillID, INT &nDstStep);
	DWORD		ReName(DWORD dwPetID, LPCTSTR tszName);
	//----------------------------------------------------------------------------------------------------
	// �ٻ���Ϣ
	//----------------------------------------------------------------------------------------------------
	DWORD		CallPet(DWORD dwPetID);
	DWORD		RestPet(DWORD dwPetID, BOOL bDestory = FALSE);
	DWORD		RestAPet();
	
	// ���г���ȡ������״̬
	VOID		RestAllPetHeti();

	//----------------------------------------------------------------------------------------------------
	// �ٻ��ĳ���ιʳ
	//----------------------------------------------------------------------------------------------------
	DWORD		CalledPetFeed(INT64 n64ItemID, DWORD dwFoodType);
	
	DWORD		InitRandomSkill(DWORD dwPetID);

	//�������
	DWORD		RebornPet(DWORD dwPetID, INT64 dw64ItemID);

	//Ⱦɫ
	DWORD		PetColor(DWORD dwPetID, INT nColor );

	//�ɱ�
	DWORD		PetChange(DWORD dwPetID, INT64 dw64ItemID);

	//������
	DWORD		PetAddPoint(DWORD dwPetID, BYTE byType, INT nValue);

	//�����ں�	
	DWORD		FusionPet(DWORD dwPetID1, DWORD dwPetID2, INT64 dw64ItemID1);
	
	//�������
	DWORD		petXiulian(DWORD dwPetID, DWORD dwNpcID, DWORD dwTimeType, DWORD dwModeType);

	//��������ջ�
	DWORD		petXiulianRetrun(DWORD dwPetID);

	INT			GetXiulianNumber();

	INT			GetMaxPetLevel();
public:
	//----------------------------------------------------------------------------------------------------
	// �ٻ�������������
	//----------------------------------------------------------------------------------------------------
	VOID		CalledPetEnterPocket();
	VOID		CalledPetLeavePocket();

	//----------------------------------------------------------------------------------------------------
	// ���ｻ�����
	//----------------------------------------------------------------------------------------------------
	BOOL		CheckExistInPocket(DWORD *dwPetIDs, INT n_num );
	DWORD		CanExchange(DWORD dwPetID);
	BOOL		GetFreeSize();
	VOID		TakeFromPocket(PetSoul* *pSouls, INT nSizeSouls, DWORD* dwPetIDs, INT nNumPetID);
	VOID		PutInPocket(PetSoul* *pSouls, INT nSizeSouls);

private:
	Role*			m_pMaster;			// ����
	SoulTMap		m_mapAllSoul;		// ����PetSoul
	INT				m_nCalledPets;		// �Ѿ��ٻ��ĳ�������
	DWORD			m_dwPetIDForUpStep;	// ���׵ĳ���id
	
	INT16			m_nSize;
public:
	//----------------------------------------------------------------------------------------------------
	// ����
	//----------------------------------------------------------------------------------------------------
	BOOL		DynamicTest(INT nTestNo, INT nArg1, INT nArg2);

};


