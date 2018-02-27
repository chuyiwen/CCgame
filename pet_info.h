//-----------------------------------------------------------------------------
//!\file pet_info.h
//!\author xlguo
//!
//!\date 2009-04-03
//! last 
//!
//!\brief ��������
//!
//!	Copyright (c) 2004 TENGWU Entertainment All rights reserved.
//-----------------------------------------------------------------------------
#pragma once
/*
//----------------------------------------------------------------------------
// ���＼��״̬
//----------------------------------------------------------------------------
enum EPetSkillStat
{
	EPSS_Using		= 0,
	EPSS_CanUse		= 1,
	EPSS_CanNotUse	= 2,
	EPSS_Passive	= 3,
};
const INT	MAX_SKILL_MOD_ATT	= 3;
//----------------------------------------------------------------------------
// ���＼��ԭ��
//----------------------------------------------------------------------------
struct tagPetSkillProto
{
	UINT16	u16PetSkillID;
	INT		AttIndexs[MAX_SKILL_MOD_ATT];
	INT		AttMods[MAX_SKILL_MOD_ATT];
	DWORD	dwScriptID;
};

//----------------------------------------------------------------------------
// ���＼��
//----------------------------------------------------------------------------
class PetSkill
{
public:
	BOOL	SetOwner(Pet* pPet);

	EPetSkillStat	GetStatus();
	BOOL	CanCast();
	VOID	Cast();
	BOOL	Skill();

private:
	tagPetSkillProto*	pProto;
	PetSoul*			pPetSoul;
	DWORD				dwItemID;
};

//! ����������ֵ����ö��
enum EPetAttribute
{   
	EPA_Begin		= 0,
	EPA_Quality		= EPA_A_BEGIN,	//����	
	EPA_Iq			,	//����

	EPA_A_BEGIN		= EPA_Begin,
	EPA_Speed		,	//�ٶ�(1-999)
	EPA_Endurance	,	//����(1-999)
	EPA_Spirit		,	//����(1-999)
	EPA_A_END		,
	EPA_A_NUM		= EPA_A_END - EPA_A_BEGIN,

	EPA_B_BEGIN		= EPA_A_END,
	EPA_MaxSkillNum	,	//����ѧ������
	EPA_CurSkillNum	,	//��ǰ��ѧ������
	EPA_MaxBagCap	,	//��󱳰���������
	EPA_CurBagCap	,	//��ǰ������������
	EPA_MaxMagic	,	//�������,����ʩ�ż�������
	EPA_CurMagic	,	//��ǰ����,����ʩ�ż�������
	EPA_RidingSpeed	,	//����ٶ�
	EPA_B_END		,
	EPA_B_NUM		= EPA_B_END - EPA_B_BEGIN,

	EPA_S_BEGIN		= EPA_B_END,
	EPA_Loyal		,	//�ҳ϶�
	EPA_Hungry		,	//������
	EPA_S_END		,
	EPA_S_NUM		= EPA_S_END - EPA_S_BEGIN,

	EPA_End			= EPA_S_END,
	EPA_NUM			= EPA_End - EPA_Begin,
}

#define MTransEPAttA2Index(EPAAttA)		((EPAAttA) - EPA_A_BEGIN)
#define MTransIndex2EPAttA(Index)		((Index) + EPA_A_BEGIN)
*/

//----------------------------------------------------------------------------
// ������꣬������еĳ���
//----------------------------------------------------------------------------
// class PetSoul
// {
// 	// ��������
// private:
// 	Pet*	m_pEntity;		// ����ʵ��
// 	Role*	m_pMasterRole;	// ����
// 
// 	DWORD			m_dwNameID;	//��������id
// 	INT				m_nLevel;	//��ǰ�ȼ�(0-200)
// 	INT				m_nExp;		//��ǰ����2^37
// 	EPetGrowthStage	m_eStage;	//�����׶�
// 	EPetState		m_eState;	//����״̬
// 
// 	INT		m_nPotentialPt;	//Ǳ�ܵ�
// 	INT		m_PotentialPts[EPA_A_NUM];
// 
// 	INT		m_nAttBase[EPA_NUM];	// ����������Ͷ�㣩
// 	INT		m_nAttAddOn[EPA_NUM];	// װ���ӳ�
// 	INT		m_nAttCur[EPA_NUM];		// 
// 
// public:
// 	VOID	ModAttValue(INT32 nIndex)
// 	{
// 		// ��Ϊ�ٻ�״̬���³���ʵ��
// 		if (!P_VALID(m_pEntity)) return;
// 	}
// 
// public:
// 	// ѧϰ����
// 	BOOL	LearnSkill(DWORD dwSkillID, INT64 n64ItemID);
// 	// ���ü���
// 	BOOL	SetSkill();
// 
// 	// װ��
// 	BOOL	MountEquip(PetEquip* pPetEquip);
// 	BOOL	UnLoadEquip(UINT16 u16PetEquipID);
// 
// public:
// 	// �������м��ܸ�Ҫ��Ϣ
// 	VOID	SendAllPetSkillBrief();
// 	// �������м�����ϸ��Ϣ
// 	VOID	SendAllPetSkillDetail();
// 	// ��������װ����Ҫ��Ϣ
// 	VOID	SendAllPetEquipBrief();
// 	// ��������װ����ϸ��Ϣ
// 	VOID	SendAllPetEquipDetail();
// 
// public:
// 	// ��ʼ����������
// 	BOOL	Init(PVOID pData);
// 
// 	// ����������£��ҳ϶ȣ������ȣ����ܵȵȣ�
// 	VOID	Update()
// 	{
// 		// �������м���
// 		// �ٻ�״̬���¸�������
// 	}
// };
