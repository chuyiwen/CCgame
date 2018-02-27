/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//宠物技能

#pragma once

struct	s_db_pet_skill;
struct	tagPetSkillProto;
struct	tagPetSkillMsgInfo;
struct	tagPetSkillCmdParam;

class PetSoul;
class Role;

// 技能收获时间
const INT	COUNT_DOWN_GAIN		= 5 * 60 * TICK_PER_SECOND;

// 技能使用时间
const INT	COUNT_DOWN_USING	= 2 * 60 * 60 * TICK_PER_SECOND;



//----------------------------------------------------------------------------------------------------
// 宠物技能
//----------------------------------------------------------------------------------------------------
class PetSkill
{
protected:
	PetSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1 = INVALID_VALUE, INT nPara2 = INVALID_VALUE);
	virtual	~PetSkill(){}	

public:
	const tagPetSkillProto* GetProto() const { return m_pProto; }

	//----------------------------------------------------------------------------------------------------
	// 创建与删除
	//----------------------------------------------------------------------------------------------------
	static PetSkill*	CreatePetSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1 = INVALID_VALUE, INT nPara2 = INVALID_VALUE);
	static PetSkill*	CreateDBPetSkill(s_db_pet_skill* pDBPetSkill, PetSoul* pSoul);
	static VOID			DeletePetSkill(PetSkill* p2Delete)	{	SAFE_DELETE(p2Delete);	}
	BOOL				SaveToDB( s_db_pet_skill* pData);

	//----------------------------------------------------------------------------------------------------
	// 填写技能信息
	//----------------------------------------------------------------------------------------------------
	VOID				FillClientInfo(tagPetSkillMsgInfo* pInfo);
	DWORD				GetSkillTypeID() const;
	BYTE				GetCastCondition() const;

	//----------------------------------------------------------------------------------------------------
	// 更新
	//----------------------------------------------------------------------------------------------------
	virtual VOID		Update();

	//----------------------------------------------------------------------------------------------------
	// 冷却
	//----------------------------------------------------------------------------------------------------
	BOOL				IsCoolDowning()		{		return VALID_VALUE(m_nCoolDownTick);				}
	VOID SetCoolDowning(INT nTickAdd = 0);
	VOID				CoolDowning()		{		if (IsCoolDowning())	--m_nCoolDownTick;		}

	BOOL				IsWorkCounting()	{		return VALID_VALUE(m_nWorkCountTick);				}
	VOID SetWorkCounting(INT nTickAdd = 0);
	VOID				WorkCounting()		{		if (IsWorkCounting())	--m_nWorkCountTick;		}

	//----------------------------------------------------------------------------------------------------
	// 其它
	//----------------------------------------------------------------------------------------------------
	BOOL				CanSetWorking( BOOL bWorking );
	BOOL				SetWorking(BOOL bWorking);

public:
	virtual BOOL		Active(Role* pTarget, INT nAddLevel = 0) { return FALSE; }
	virtual BOOL		DeActive(Role* pTarget, BOOL bSendMsg = TRUE) { return FALSE; }

protected:
	PetSoul*			GetSoul()	const			{		return m_pSoul;				}
	Role*				GetMaster() const;

	INT					m_nCoolDownTick;
	INT					m_nWorkCountTick;

private:
	const tagPetSkillProto* m_pProto;
	PetSoul*			m_pSoul;
};

//----------------------------------------------------------------------------------------------------
// 被动接口
//----------------------------------------------------------------------------------------------------
class PassiveSkill : public PetSkill
{
public:
	PassiveSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1 = INVALID_VALUE, INT nPara2 = INVALID_VALUE)
		:PetSkill(dwSkillTypeID, pSoul, nPara1, nPara2){}
//public:
//	BOOL		Active() {}
//	BOOL		DeActive() {}
};

//----------------------------------------------------------------------------------------------------
// 主动接口
//----------------------------------------------------------------------------------------------------
class ActiveSkill : public PetSkill
{
public:
	ActiveSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1 = INVALID_VALUE, INT nPara2 = INVALID_VALUE)
		:PetSkill(dwSkillTypeID, pSoul, nPara1, nPara2){}
public:
	DWORD				HandleCmd(tagPetSkillCmdParam* pCmd);
	virtual DWORD		HandleCmdImpl(tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd) = 0;
};

//----------------------------------------------------------------------------------------------------
// 自身提升接口
//----------------------------------------------------------------------------------------------------
class EnhanceSkill : public PetSkill
{
public:
	EnhanceSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1 = INVALID_VALUE, INT nPara2 = INVALID_VALUE)
		:PetSkill(dwSkillTypeID, pSoul, nPara1, nPara2){}
public:
	virtual BOOL		Open() = 0;
	virtual BOOL		Close() = 0;
};

//----------------------------------------------------------------------------------------------------
// 宠物强化技能
//----------------------------------------------------------------------------------------------------
class PetMountAddSkill:public EnhanceSkill
{
	friend class PetSkill;

	PetMountAddSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2) 
		:EnhanceSkill(dwSkillTypeID, pSoul, nPara1, nPara2),m_bInUsing(FALSE){}

private:
	BOOL				Open();
	BOOL				Close();

private:
	BOOL				m_bInUsing;
};

//----------------------------------------------------------------------------------------------------
// 宠物强身技能
//----------------------------------------------------------------------------------------------------
class PetStrengthSkill:public PassiveSkill
{
	friend class PetSkill;
	
	PetStrengthSkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2) 
		:PassiveSkill(dwSkillTypeID, pSoul, nPara1, nPara2),m_bInUsing(FALSE){}

private:
	BOOL				Active(Role* pTarget, INT nAddLevel = 0);
	BOOL				DeActive(Role* pTarget, BOOL bSendMsg = TRUE);

private:
	BOOL				m_bInUsing;
};

//----------------------------------------------------------------------------------------------------
// 宠物特长
//----------------------------------------------------------------------------------------------------
class PetSpecialtySkill : public PassiveSkill
{
	friend class PetSkill;

	PetSpecialtySkill(DWORD dwSkillTypeID, PetSoul* pSoul, INT nParam1, INT nParam2)
		:PassiveSkill(dwSkillTypeID, pSoul, nParam1, nParam2),m_bInUsing(FALSE){}

private:
	BOOL				Active(Role* pTarget, INT nAddLevel = 0);
	BOOL				DeActive(Role* pTarget, BOOL bSendMsg = TRUE);

private:
	BOOL				m_bInUsing;
};

//----------------------------------------------------------------------------------------------------
// 宠物喂药技能
//----------------------------------------------------------------------------------------------------
class PetMedicineFeedSkill : public ActiveSkill
{
	friend class PetSkill;
	PetMedicineFeedSkill( DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2 )
		:ActiveSkill(dwSkillTypeID, pSoul, nPara1, nPara2){}

private:
	DWORD				HandleCmdImpl(tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd);

};

//----------------------------------------------------------------------------------------------------
// 宠物拾取技能
//----------------------------------------------------------------------------------------------------
class PetPickUpSkill : public ActiveSkill
{
	friend class PetSkill;
	PetPickUpSkill( DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2 )
		:ActiveSkill(dwSkillTypeID, pSoul, nPara1, nPara2)	{}

private:
	DWORD				HandleCmdImpl(tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd);
};

//----------------------------------------------------------------------------------------------------
// 宠物天赋技能
//----------------------------------------------------------------------------------------------------
class PetWuXingSkill : public ActiveSkill
{
	friend class PetSkill;
	PetWuXingSkill( DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2 )
		:ActiveSkill(dwSkillTypeID, pSoul, nPara1, nPara2)	{}

private:
	DWORD				HandleCmdImpl(tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd);
};

//----------------------------------------------------------------------------------------------------
// 宠物状态技能
//----------------------------------------------------------------------------------------------------
class PetBuffSkill : public ActiveSkill
{
	friend class PetSkill;
	PetBuffSkill( DWORD dwSkillTypeID, PetSoul* pSoul, INT nPara1, INT nPara2 )
		:ActiveSkill(dwSkillTypeID, pSoul, nPara1, nPara2)	{}

private:
	DWORD				HandleCmdImpl(tagPetSkillCmdParam* pCmd, const tagPetSkillProto* pSkillProto, INT &nCoolDownAdd, INT &nWorkingAdd);
	BOOL				bInUse;
};

