/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//����ʵ��

#pragma once

#include "pet_att.h"
#include "pet_equip.h"
#include "../../common/WorldDefine/pet_skill_protocol.h"

class Pet;
class Role;
class PetTracker;
class PetSkill;

struct tagItem;
struct s_db_pet;
struct tagPetSkillCmdParam;
enum EPassiveSkillAndEquipTrigger;
enum ETriggerEventType;

//----------------------------------------------------------------------------------------------------
// ����ʵ��
// ˵����
//		����id		dwPetSkillID;
//		����lvl		dwPetSkillLvl;
//		����typeid	dwPetSkillTypeID;
//		dwPetSkillTypeID = dwPetSkillID * 100 + dwPetSkillLvl;
//----------------------------------------------------------------------------------------------------
class PetSoul
{
	friend class PetAtt;
	friend class PetPocket;
public:
	typedef package_map<DWORD, PetSkill*>		PetSkillMap;	
	//----------------------------------------------------------------------------------------------------
	// ������ɾ��
	//----------------------------------------------------------------------------------------------------
	static VOID		CreateDBSoul( DWORD dw_data_id,LPCTSTR tszName,Role* pMaster, INT nQualiry);
	static PetSoul* CreateSoulByDBData(const BYTE* &pData, BOOL bCreate);
	static VOID		DeleteSoul(PetSoul* pSoul, BOOL bFromDB = FALSE);

	//----------------------------------------------------------------------------------------------------
	// ����
	//----------------------------------------------------------------------------------------------------
	VOID			Update();

	//----------------------------------------------------------------------------------------------------
	// �������ͼ
	//----------------------------------------------------------------------------------------------------
	DWORD			BodyEnterMap();

	BOOL			IsPetInMap();
	DWORD			BodyLeaveMap(BOOL bSendMsg = TRUE);

	//----------------------------------------------------------------------------------------------------
	// ��ȡ
	//----------------------------------------------------------------------------------------------------
	Role*			GetMaster()		const	{return m_pMaster;	}
	Pet*			GetBody()		const	{return m_pBody;	}
	VOID			SetBody(Pet* pBody)		{m_pBody = pBody;	}
	DWORD			GetID()			const;
	DWORD			GetProtoID()	const;
	PetAtt&			GetPetAtt()				{ return m_PetAtt;	}
	const tagPetProto* GetProto();

	//----------------------------------------------------------------------------------------------------
	// �������＼��buff
	//----------------------------------------------------------------------------------------------------
	VOID OnPetSkillBuffTrigger(ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType);

	//----------------------------------------------------------------------------------------------------
	// ����
	//----------------------------------------------------------------------------------------------------
	BOOL			IsLocked()		const	{ return m_PetAtt.IsLocked();	}
	VOID			SetLocked(BOOL bSet);

	VOID			FillClientPetAtt(tagPetInitAttr* pInitAtt);
	BOOL			SaveToDB(IN LPVOID pData, OUT LPVOID &pOutPointer);

	//----------------------------------------------------------------------------------------------------
	// ���������
	//----------------------------------------------------------------------------------------------------
	BOOL			IntegrateInPet(Pet* pPet);
	VOID			DetachFromPet();
	BOOL			IntegrateInRole(Role* pRole);
	VOID			DetachFromRole();

	//----------------------------------------------------------------------------------------------------
	//	����״̬���
	//----------------------------------------------------------------------------------------------------
	BOOL			IsInState(EPetState eState);

	//----------------------------------------------------------------------------------------------------
	// �ٻ����
	//----------------------------------------------------------------------------------------------------
	BOOL			IsCalled();
	BOOL			CanSetCalled();
	BOOL			SetCalled(BOOL bSet, BOOL bSync = TRUE);

	//----------------------------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------------------------
	BOOL			IsRonghe();
	VOID			SetHeti(BOOL bSet, Role* pRole, INT nAddSkillLevel = 0, BOOL bSync = TRUE);
	Role*			GetHeti() { return m_pHeti;}
	//----------------------------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------------------------
	BOOL			IsWorking();
	BOOL			CanSetWroking();
	BOOL			SetWorking(BOOL bSet, DWORD dwFrinedID = INVALID_VALUE, DWORD dwTime = INVALID_VALUE);
	BOOL			GetWorkingSkillTypeID()const {return m_dwWorkingSkillTypeID;}
	

	BOOL			CanSetDead();
	BOOL			CanSetXiulian();
	//----------------------------------------------------------------------------------------------------
	//	���＼�����
	//----------------------------------------------------------------------------------------------------
	DWORD			LearnBookSkill(INT64 n64ItemID);
	VOID			LearnNormalSkill(INT nLevel, INT nQuality);
	DWORD			SkillLevelUp(DWORD dwID);

	DWORD			LearnSkill( DWORD dwPetSkillTypeID );
	DWORD			ForgetSkill(DWORD dwPetSkillID, BOOL bFromeClient = FALSE);
	
	DWORD			AddSkill(PetSkill* pSkill);
	DWORD			AddSkillOnLoad(PetSkill* pSkill);

	DWORD			RemoveSkill(PetSkill* pSkill);
	PetSkill*		GetSkill(DWORD dwPetSkillID);
	INT				ExportSpecSkill(INT nPetAtt, std::list<PetSkill*> &listModSkill);
	INT				GetModSkillType(INT nPetAtt);
	DWORD			HandleSkillCmd(DWORD dwPetSkillTypeID, tagPetSkillCmdParam* pCmd);
	VOID			UpdateAllSkill();
	VOID			RandRemoveNoramlSkill();
	DWORD			GetSkillNum(EPetskillType type);
	BOOL			HasSkill(DWORD dwSkillID);
	BOOL			HasActiveSkill();
	DWORD			GetActiveSkill();
	DWORD			GetBuffSkill();
	VOID			RemoveNormalSkill();
	VOID			RemoveBuffSkill();
	VOID			RemoveSpecialtySkill();
	//----------------------------------------------------------------------------------------------------
	//	����װ�����
	//----------------------------------------------------------------------------------------------------
	DWORD			Equip(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);
	DWORD			UnEquip(INT64 n64ItemID, INT16 n16DstPos, BOOL bSend = FALSE);
	DWORD			EquipSwapPos(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);
	VOID			GetEquipInfo(INT64 n64ItemID, tagPetEquipMsgInfo* pMsgInfo);
	BOOL			HasEquip();

	//----------------------------------------------------------------------------------------------------
	// ����������
	//----------------------------------------------------------------------------------------------------
	DWORD			PourExp(INT64 n64ItemID);
	VOID			OnLevelUp();
	DWORD			Enhance(INT nAptitudeAdd);
	DWORD			UpStep(DWORD dwPetSkillTypeID);

	//----------------------------------------------------------------------------------------------------
	// ιʳ
	//----------------------------------------------------------------------------------------------------
	DWORD			Feed(INT nSpritMod);

	//----------------------------------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------------------------------
	BOOL			InitRandomSkill();
	VOID			RandNomalSkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandBuffSkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandSpecialtySkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandActionSkill(INT nSkillLevel = INVALID_VALUE);
	//----------------------------------------------------------------------------------------------------
	// ����ĳ�����͵ļ���
	//----------------------------------------------------------------------------------------------------
	VOID			SetActiveSkill(EPetskillType ePetSkillType, bool bActive);


	//DWORD			GetPaiqianFriendID() { return m_dwFriendID; }
private:
	//----------------------------------------------------------------------------------------------------
	// ʵ��
	//----------------------------------------------------------------------------------------------------
	BOOL			SetMaster(Role* pRole)	{	m_pMaster = pRole;	return TRUE;}

	//----------------------------------------------------------------------------------------------------
	// ����������
	//----------------------------------------------------------------------------------------------------
	PetSoul();
	virtual	~PetSoul();

	//----------------------------------------------------------------------------------------------------
	// ���ֳ�ʼ��
	//----------------------------------------------------------------------------------------------------
	BOOL			Init(const s_db_pet* &pSoulData, BOOL bCreate);
	VOID			InitPetEquip(IN PVOID pData, OUT PVOID& pDataOut, IN INT n_num);
	VOID			InitPetSkill(IN PVOID pData, OUT PVOID& pDataOut, IN INT n_num);
	
private:
	Role*			m_pMaster;				// ����
	Pet*			m_pBody;				// ����
	Role*			m_pHeti;				// �������

	PetAtt			m_PetAtt;				// ��������
	PetSkillMap		m_mapPetSkill;			// ���＼��
	DWORD			m_dwWorkingSkillTypeID;	// ��ǰ��������
	PetEquipBar		m_EquipBar;				// ����װ����

	INT				nCount;					// ������ʧ����

	//DWORD			m_dwFriendID;			// ��ǲ����id
	//DWORD			m_dwBeginTime;			// ��ǲ��ʼʱ��
};


