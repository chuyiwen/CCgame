/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//ս��������

#pragma once

class Unit;

// ���������ͼ��ܽṹ
struct tagPilotUnit
{
	DWORD dwMapID;		//����mapid
	DWORD dwInstanceID;	//���︱��id
	DWORD dwCretureID;	//��Ӧ����id
	FLOAT fOPRadius;	//���÷�Χ
	DWORD dwOverTime;	//ʣ��ʱ��
	DWORD dwSkillID;	//��Ӧ����id
};

class CombatHandler
{
	friend class Role;
public:
	enum ETargetEffectFlag
	{
		ETEF_Hited		=	0x0001,		// ����
		ETEF_Block		=	0x0002,		// ��
		ETEF_Crited		=	0x0004,		// ����
	};

	enum EInterruptType
	{
		EIT_Null		=	-1,			// ��
		EIT_Move		=	0,			// �ƶ����
		EIT_Skill		=	1,			// ����
	};
	
	//����ʹ�õĸ��׶�
	enum EUseSkillState
	{
		EUSS_NULL		=	-1,
		EUSS_Preparing	=	0,			// ����
		EUSS_Operating	=	1,			// �ͷ���
		EUSS_Piloting	=	2,			// ������
	};
public:
	//----------------------------------------------------------------------------
	// Constructor
	//----------------------------------------------------------------------------
	CombatHandler();

	//----------------------------------------------------------------------------
	// ��ʼ������ʼ�����ºͽ���
	//----------------------------------------------------------------------------
	VOID	Init(Unit* pOwner);
	INT		UseSkill(DWORD dwSkillID, DWORD dwTargetUnitID, DWORD dwSerial ,Vector3 vDesPos = Vector3(0,0,0));
	INT		UseItem(INT64 n64ItemID, DWORD dwTargetUnitID, DWORD dwSerial, DWORD &dw_data_id, bool& bImmediate);
	DWORD	UseRide();//Ares
	VOID	Update();
	VOID	End();

	DWORD CheckUseRide(tagEquip* pRide);

	VOID	SetCombatStateCoolDown() { m_dwCombatStateCoolDown = 6000; }

	//----------------------------------------------------------------------------
	// ȡ������ʹ��
	//----------------------------------------------------------------------------
	VOID	CancelSkillUse(DWORD dwSkillID);

	//----------------------------------------------------------------------------
	// ȡ����Ʒʹ��
	//----------------------------------------------------------------------------
	VOID	CancelItemUse(INT64 n64ItemSerial);

	//----------------------------------------------------------------------------
	// ȡ�����
	//----------------------------------------------------------------------------
	VOID InterruptRide();

	//----------------------------------------------------------------------------
	// �������
	//----------------------------------------------------------------------------
	BOOL	InterruptPrepare(EInterruptType eType, BOOL bOrdinary, BOOL bForce=FALSE);

	//----------------------------------------------------------------------------
	// ����ͷ�
	//----------------------------------------------------------------------------
	BOOL	InterruptOperate(EInterruptType eType, DWORD dwMisc, BOOL bForce=FALSE);

	//----------------------------------------------------------------------------
	// ����mod
	//----------------------------------------------------------------------------
	VOID	ModSkillPrepareModPct(INT nModPct);
	VOID	ModTargetArmorLeftPct(INT nModPct);

	//----------------------------------------------------------------------------
	// ����Get
	//----------------------------------------------------------------------------
	BOOL	IsValid()					{ return IsUseSkill() || IsUseItem() || IsRide(); }
	BOOL	IsUseSkill()				{ return VALID_POINT(m_dwSkillID); }
	BOOL	IsUseItem()					{ return VALID_POINT(m_n64ItemID); }
	BOOL	IsRide()					{ return m_bRide;}
	BOOL	IsPreparing()				{ return IsSkillPreparing() || IsItemPreparing() || IsRidePreparing(); }
	BOOL	IsOperating()				{ return IsSkillOperating() || IsItemOperating() || IsRideOperating(); }
	BOOL	IsPiloting()				{return	 IsSkillPiloting();}
	INT		GetSkillPrepareCountDown()	{ return m_nSkillPrepareCountDown; }
	INT		GetItemPrepareCountDown()	{ return m_nItemPrepareCountDown; }
	INT		GetRidePrepareCountDown()	{ return m_nRidePrepareCountDown; }
	DWORD	GetTargetUnitID()			{ return m_dwTargetUnitID; }
	DWORD	GetSkillID()				{ return m_dwSkillID; }
	FLOAT	GetTargetArmor(Unit* target, DWORD dwArmorType);

	INT		GetPilotTimeCD()			{return m_nPersistSkillTimeCD;}
	//----------------------------------------------------------------------------
	// ����ʹ���ж�
	//----------------------------------------------------------------------------
	INT		CanCastSkill(Skill* pSkill, DWORD dwTargetUnitID ,const Vector3&);

	INT		CheckSkillAbility(Skill* pSkill);							// ���Լ�������
	INT		CheckOwnerLimitSkill();											// ���Լ�������������
	INT		CheckSkillLimit(Skill* pSkill);								// ���Լ��ܱ�������
	INT		CheckTargetLimit(Skill* pSkill, DWORD dwTargetUnitID,const Vector3&);		// ����Ŀ������
	INT		CheckCostLimit(Skill* pSkill);								// ������������
	INT		CheckVocationLimit(Skill* pSkill);							// ����ְҵ����
	INT		CheckTargetLogicLimit(Skill* pSkill, Unit* pTarget);		// ��⼼�ܺ�Ŀ���
	INT		CheckMapLimit(Skill* pSkill);								// ����ͼ����
	VOID	CheckInCombat(Unit* pTarget);								// ս��״̬�ж�

	BOOL	CheckSkillConflict(Skill* pSkill);							// ��鼼��ʹ�õĳ�ͻ

	//----------------------------------------------------------------------------
	// ���㼼��Ч��
	//----------------------------------------------------------------------------
	DWORD	CalculateSkillEffect(Skill* pSkill, Unit* pTarget);
	VOID	CalSkillTargetList(Vector3 vPosDes = Vector3(0,0,0), DWORD dwMaxNumber = INVALID_VALUE);
	BOOL	CalculateHit(Skill* pSkill, Unit* pTarget);
	BOOL	CalculateBlock(Skill* pSkill, Unit* pTarget);
	BOOL	CalculateCritRate(Skill* pSkill, Unit* pTarget);
	FLOAT	CalculateCritAmount(Skill* pSkill, Unit* pTarget);	
	VOID	CalculateDmg(Skill* pSkill, Unit* pTarget);
	VOID	CalculateCost(Skill* pSkill);
	
	VOID	CalSkillTargetList(package_list<DWORD>& targetList, const tagPilotUnit* pPilotUnit, Vector3 vPosDes = Vector3(0,0,0));
	VOID	CalculateDmgNoSpe(Skill* pSkill, Unit* pTarget);
	VOID	ClearPilotList();
	//----------------------------------------------------------------------------
	// ��Ʒʹ���ж�
	//----------------------------------------------------------------------------
	INT		can_use_item(tagItem* pItem);
	INT		CheckItemAbility(tagItem* pItem);							// �����Ʒ����
	INT		CheckOwnerLimitItem();										// �������
	INT		CheckRoleProtoLimit(tagItem* pItem);						// ���������������
	INT		CheckRoleStateLimit(tagItem* pItem);						// �������״̬����
	INT		CheckRoleVocationLimit(tagItem* pItem);						// �������ְҵ����
	INT		CheckMapLimit(tagItem* pItem);								// ����ͼ����
	BOOL	CheckItemConflict(tagItem* pItem);

	//-----------------------------------------------------------------------------
	// ������ƷЧ��
	//-----------------------------------------------------------------------------
	VOID	CalUseItemTargetList();
	
	//-----------------------------------------------------------------------------
	// ��������Ч��
	//-----------------------------------------------------------------------------
	//VOID	OnHit(Skill* pSkill, INT32 nlevelSub, BOOL bCrit = FALSE);//����ʱ
	//VOID	OnDmg(Unit* pRole, INT dmg, INT32 nlevelSub, BOOL bCrit = FALSE);//�ܵ��˺�ʱ

	//-----------------------------------------------------------------------------
	// �����������仯�߼���غ���
	//-----------------------------------------------------------------------------
	//�õ���������
	//DWORD	GetChargeType();
private:
	//-----------------------------------------------------------------------------
	// ����Get
	//-----------------------------------------------------------------------------
	bool	IsSkillPreparing()			{ return m_eUseSkillState == EUSS_Preparing; }
	bool	IsSkillOperating()			{ return m_eUseSkillState == EUSS_Operating; }
	bool	IsSkillPiloting()			{ return m_eUseSkillState == EUSS_Piloting;}	//add by guohui
	bool	IsItemPreparing()			{ return m_bItemPreparing; }
	bool	IsItemOperating()			{ return m_bItemOperating; }
	bool	IsRidePreparing()			{ return m_bRidePreparing; }
	bool	IsRideOperating()			{ return m_bRideOperating; }

	//-----------------------------------------------------------------------------
	// Mod�ײ���ú���
	//-----------------------------------------------------------------------------
	VOID	ModPct(IN OUT FLOAT &fDstPct, IN INT nModPct);

	//-----------------------------------------------------------------------------
	// ���²���
	//-----------------------------------------------------------------------------
	VOID	UpdateSkillPrepare();
	VOID	UpdateSkillOperate();
	VOID	UpdateItemPrepare();
	VOID	UpdateItemOperate();
	VOID	UpdateRidePrepare();
	VOID	UpdateRideOperate();
	VOID	UpdateSkillPiloting();
	VOID	UpdatePrepareUnit();
	//-----------------------------------------------------------------------------
	// ��������
	//-----------------------------------------------------------------------------
	VOID	EndUseSkill();
	VOID	EndUseItem();
	VOID	EndRide();

	//----------------------------------------------------------------------------
	// �˺���ʽ��ϵ������
	//----------------------------------------------------------------------------
	FLOAT	CalBaseDmg(Skill* pSkill, Unit* pTarget);
	FLOAT	CalAttackDefenceCoef(Skill* pSkill, Unit* pTarget);
	//FLOAT	CalMoraleCoef(Unit* pTarget);
	FLOAT	CalDerateCoef(Skill* pSkill, Unit* pTarget);
	//FLOAT	CalInjuryCoef();
	FLOAT	CalLevelCoef(Skill* pSkill, Unit* pTarget);

private:
	Unit*			m_pOwner;					// ������

	DWORD			m_dwSkillID;				// �����ļ���ID
	INT64			m_n64ItemID;				// ʹ����Ʒ64λID
	DWORD			m_dwTargetUnitID;			// ���ܷ�������Ŀ��ID
	DWORD			m_dwTargetEffectFlag;		// ��Ŀ�������Ч��
	DWORD			m_dwSkillSerial;			// ���ܹ������к�
	DWORD			m_dwItemSerial;				// ��Ʒʹ�����к�
	DWORD			m_dwTargetUnitIDItem;		// ʹ����Ʒ��Ŀ��ID
	BOOL			m_bRide;					// ʹ������
	
	EUseSkillState	m_eUseSkillState;			// ��ǰ���ܽ׶�
	//bool			m_bSkillPreparing;			// �����Ƿ�������
	//bool			m_bSkillOperating;			// �����Ƿ��ڷ���
	//bool			m_bSkillPiloting;			// ��������������

	bool			m_bItemOperating;			// ��Ʒ�Ƿ��ڷ���
	bool			m_bItemPreparing;			// ��Ʒ��������

	bool			m_bRidePreparing;			// ��������
	bool			m_bRideOperating;			// ���﷢��

	bool			m_bTrigger;					// �Ƿ���㴥�����

	INT				m_nRidePrepareCountDown;
	INT				m_nSkillPrepareCountDown;	// �������ֵ���ʱ�����룩
	INT				m_nItemPrepareCountDown;	// ��Ʒ���ֵ���ʱ�����룩

	FLOAT			m_fSkillPrepareModPct;		// ��������ʱ��Ӱ��ٷֱ�
	FLOAT			m_fTargetArmorLeftPct;		// Ŀ�껤��������ʣ��ٷֱȣ�1.0f - �����ٷֱȣ�

	INT				m_nSkillOperateTime;		// ���ܲ�����ʱ�䣬���ڼ�������˺������룩
	INT				m_nSkillCurDmgIndex;		// ��ǰҪ����ڼ����˺�

	INT				m_nPersistSkillTimeCD;		//��������ʱ�䵹��ʱ
	INT				m_nPersistSkillTime;		//��������ʱ��
	INT				m_nPersistSkillCnt;			//�������ܵ�ǰ����
	
	//bool			m_bDropMP;					// ���������Ƿ����
	Vector3			m_vPersistSkillPos;			//��������Ŀ��� ��������Ŀ��id�����ģ�

	INT				m_dwPublicCoolDown;				// ���ܹ�����ȴʱ��

	INT				m_dwCombatStateCoolDown;		// ս��״̬��ȴʱ��
	
	INT				m_dwChixuCoolDown;				// ��ǽ�༼�ܼ��

	BOOL			m_bCD;

	package_list<DWORD>	m_listTargetID;				// ����Ŀ���б�
	package_list<DWORD>	m_listHitedTarget;			// �������е�Ŀ��
	package_list<DWORD>	m_listDodgedTarget;			// �������ܵ�Ŀ��
	package_list<DWORD>	m_listBlockedTarget;		// ���ܸ񵲵�Ŀ��
	package_list<DWORD>	m_listCritedTarget;			// ���ܱ�����Ŀ��

	
	std::map<DWORD, tagPilotUnit*>	m_listPilotUnit;	// �����͵��＼��
};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline CombatHandler::CombatHandler()
: m_pOwner(NULL), m_dwSkillID(INVALID_VALUE), m_n64ItemID(INVALID_VALUE), m_dwTargetUnitID(INVALID_VALUE),
m_dwTargetEffectFlag(0), m_dwSkillSerial(0), m_dwItemSerial(0), m_eUseSkillState(EUSS_NULL),/*m_bSkillPreparing(false), */m_bItemPreparing(false),
/*m_bSkillOperating(false),*/ m_bItemOperating(false), m_nSkillPrepareCountDown(0), m_nItemPrepareCountDown(0),
m_fSkillPrepareModPct(1.0f), m_fTargetArmorLeftPct(1.0f), m_nSkillOperateTime(0), m_nSkillCurDmgIndex(0)/*,m_bSkillPiloting(false)*/
,m_nPersistSkillTime(0),m_nPersistSkillCnt(0),m_bTrigger(FALSE),m_dwPublicCoolDown(0),m_dwCombatStateCoolDown(0)//,m_bDropMP(true)
,m_bRide(FALSE),m_bRidePreparing(FALSE),m_nRidePrepareCountDown(0),m_bCD(FALSE),m_dwChixuCoolDown(0)
{
}

//-----------------------------------------------------------------------------
// ��ǰ���ܳ�ʼ��
//-----------------------------------------------------------------------------
inline VOID CombatHandler::Init(Unit* pOwner)
{
	m_pOwner	=	pOwner;
}


//-------------------------------------------------------------------------------------------
// ����ʹ����Ʒ
//-------------------------------------------------------------------------------------------
inline VOID CombatHandler::EndUseItem()
{
	m_n64ItemID					=	INVALID_VALUE;
	m_dwItemSerial				=	0;
	m_bItemPreparing			=	false;
	m_bItemOperating			=	false;
	m_nItemPrepareCountDown		=	0;
}
//-------------------------------------------------------------------------------------------
// ����ʹ������
//-------------------------------------------------------------------------------------------
inline VOID CombatHandler::EndRide()
{
	m_bRide = FALSE;
	m_bRidePreparing = false;
	m_bRideOperating = false;
	m_nRidePrepareCountDown = 0;
}

//--------------------------------------------------------------------------------------------
// ����ս��ϵͳ���ж���
//--------------------------------------------------------------------------------------------
inline VOID CombatHandler::End()
{
	EndUseSkill();
	EndUseItem();
	EndRide();
}

//-----------------------------------------------------------------------------
// Mod�ײ���ú���
//-----------------------------------------------------------------------------
inline VOID CombatHandler::ModPct(IN OUT FLOAT &fDstPct, IN INT nModPct)
{
	fDstPct += (FLOAT)nModPct / 10000.0f;

	if(fDstPct < 0.0f)
	{
		// ���ٷֱȱ�Ϊ��ֵʱ���޷����з������
		ASSERT(fDstPct >= 0.0f);
		fDstPct = 0.0f;
	}
}

//----------------------------------------------------------------------------
// ��������ʱ��Ӱ��ٷֱ�
//----------------------------------------------------------------------------
inline VOID CombatHandler::ModSkillPrepareModPct(INT nModPct)
{
	ModPct(m_fSkillPrepareModPct, nModPct);
}

//----------------------------------------------------------------------------
// ���ö�Ŀ�껤��Ӱ��ٷֱ�
//----------------------------------------------------------------------------
inline VOID CombatHandler::ModTargetArmorLeftPct(INT nModPct)
{
	ModPct(m_fTargetArmorLeftPct, nModPct);
}
