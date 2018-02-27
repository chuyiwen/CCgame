/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物实体

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
// 宠物实体
// 说明：
//		技能id		dwPetSkillID;
//		技能lvl		dwPetSkillLvl;
//		技能typeid	dwPetSkillTypeID;
//		dwPetSkillTypeID = dwPetSkillID * 100 + dwPetSkillLvl;
//----------------------------------------------------------------------------------------------------
class PetSoul
{
	friend class PetAtt;
	friend class PetPocket;
public:
	typedef package_map<DWORD, PetSkill*>		PetSkillMap;	
	//----------------------------------------------------------------------------------------------------
	// 创建与删除
	//----------------------------------------------------------------------------------------------------
	static VOID		CreateDBSoul( DWORD dw_data_id,LPCTSTR tszName,Role* pMaster, INT nQualiry);
	static PetSoul* CreateSoulByDBData(const BYTE* &pData, BOOL bCreate);
	static VOID		DeleteSoul(PetSoul* pSoul, BOOL bFromDB = FALSE);

	//----------------------------------------------------------------------------------------------------
	// 更新
	//----------------------------------------------------------------------------------------------------
	VOID			Update();

	//----------------------------------------------------------------------------------------------------
	// 躯体与地图
	//----------------------------------------------------------------------------------------------------
	DWORD			BodyEnterMap();

	BOOL			IsPetInMap();
	DWORD			BodyLeaveMap(BOOL bSendMsg = TRUE);

	//----------------------------------------------------------------------------------------------------
	// 获取
	//----------------------------------------------------------------------------------------------------
	Role*			GetMaster()		const	{return m_pMaster;	}
	Pet*			GetBody()		const	{return m_pBody;	}
	VOID			SetBody(Pet* pBody)		{m_pBody = pBody;	}
	DWORD			GetID()			const;
	DWORD			GetProtoID()	const;
	PetAtt&			GetPetAtt()				{ return m_PetAtt;	}
	const tagPetProto* GetProto();

	//----------------------------------------------------------------------------------------------------
	// 触发宠物技能buff
	//----------------------------------------------------------------------------------------------------
	VOID OnPetSkillBuffTrigger(ETriggerEventType eEvent, EPassiveSkillAndEquipTrigger eTriggerType);

	//----------------------------------------------------------------------------------------------------
	// 锁定
	//----------------------------------------------------------------------------------------------------
	BOOL			IsLocked()		const	{ return m_PetAtt.IsLocked();	}
	VOID			SetLocked(BOOL bSet);

	VOID			FillClientPetAtt(tagPetInitAttr* pInitAtt);
	BOOL			SaveToDB(IN LPVOID pData, OUT LPVOID &pOutPointer);

	//----------------------------------------------------------------------------------------------------
	// 整合与分离
	//----------------------------------------------------------------------------------------------------
	BOOL			IntegrateInPet(Pet* pPet);
	VOID			DetachFromPet();
	BOOL			IntegrateInRole(Role* pRole);
	VOID			DetachFromRole();

	//----------------------------------------------------------------------------------------------------
	//	宠物状态相关
	//----------------------------------------------------------------------------------------------------
	BOOL			IsInState(EPetState eState);

	//----------------------------------------------------------------------------------------------------
	// 召唤相关
	//----------------------------------------------------------------------------------------------------
	BOOL			IsCalled();
	BOOL			CanSetCalled();
	BOOL			SetCalled(BOOL bSet, BOOL bSync = TRUE);

	//----------------------------------------------------------------------------------------------------
	// 合体相关
	//----------------------------------------------------------------------------------------------------
	BOOL			IsRonghe();
	VOID			SetHeti(BOOL bSet, Role* pRole, INT nAddSkillLevel = 0, BOOL bSync = TRUE);
	Role*			GetHeti() { return m_pHeti;}
	//----------------------------------------------------------------------------------------------------
	// 工作相关
	//----------------------------------------------------------------------------------------------------
	BOOL			IsWorking();
	BOOL			CanSetWroking();
	BOOL			SetWorking(BOOL bSet, DWORD dwFrinedID = INVALID_VALUE, DWORD dwTime = INVALID_VALUE);
	BOOL			GetWorkingSkillTypeID()const {return m_dwWorkingSkillTypeID;}
	

	BOOL			CanSetDead();
	BOOL			CanSetXiulian();
	//----------------------------------------------------------------------------------------------------
	//	宠物技能相关
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
	//	宠物装备相关
	//----------------------------------------------------------------------------------------------------
	DWORD			Equip(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);
	DWORD			UnEquip(INT64 n64ItemID, INT16 n16DstPos, BOOL bSend = FALSE);
	DWORD			EquipSwapPos(INT64 n64ItemID, INT8 n8DstPos, BOOL bSend = FALSE);
	VOID			GetEquipInfo(INT64 n64ItemID, tagPetEquipMsgInfo* pMsgInfo);
	BOOL			HasEquip();

	//----------------------------------------------------------------------------------------------------
	// 经验与升级
	//----------------------------------------------------------------------------------------------------
	DWORD			PourExp(INT64 n64ItemID);
	VOID			OnLevelUp();
	DWORD			Enhance(INT nAptitudeAdd);
	DWORD			UpStep(DWORD dwPetSkillTypeID);

	//----------------------------------------------------------------------------------------------------
	// 喂食
	//----------------------------------------------------------------------------------------------------
	DWORD			Feed(INT nSpritMod);

	//----------------------------------------------------------------------------------------------------
	// 随机技能
	//----------------------------------------------------------------------------------------------------
	BOOL			InitRandomSkill();
	VOID			RandNomalSkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandBuffSkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandSpecialtySkill(INT nSkillLevel = INVALID_VALUE);
	VOID			RandActionSkill(INT nSkillLevel = INVALID_VALUE);
	//----------------------------------------------------------------------------------------------------
	// 激活某种类型的技能
	//----------------------------------------------------------------------------------------------------
	VOID			SetActiveSkill(EPetskillType ePetSkillType, bool bActive);


	//DWORD			GetPaiqianFriendID() { return m_dwFriendID; }
private:
	//----------------------------------------------------------------------------------------------------
	// 实现
	//----------------------------------------------------------------------------------------------------
	BOOL			SetMaster(Role* pRole)	{	m_pMaster = pRole;	return TRUE;}

	//----------------------------------------------------------------------------------------------------
	// 构造与析构
	//----------------------------------------------------------------------------------------------------
	PetSoul();
	virtual	~PetSoul();

	//----------------------------------------------------------------------------------------------------
	// 各种初始化
	//----------------------------------------------------------------------------------------------------
	BOOL			Init(const s_db_pet* &pSoulData, BOOL bCreate);
	VOID			InitPetEquip(IN PVOID pData, OUT PVOID& pDataOut, IN INT n_num);
	VOID			InitPetSkill(IN PVOID pData, OUT PVOID& pDataOut, IN INT n_num);
	
private:
	Role*			m_pMaster;				// 主人
	Pet*			m_pBody;				// 躯体
	Role*			m_pHeti;				// 合体对象

	PetAtt			m_PetAtt;				// 宠物属性
	PetSkillMap		m_mapPetSkill;			// 宠物技能
	DWORD			m_dwWorkingSkillTypeID;	// 当前工作技能
	PetEquipBar		m_EquipBar;				// 宠物装备栏

	INT				nCount;					// 生命流失计数

	//DWORD			m_dwFriendID;			// 派遣好友id
	//DWORD			m_dwBeginTime;			// 派遣开始时间
};


