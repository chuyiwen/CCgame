#include "stdafx.h"
#include "../../common/WorldDefine/Raid_define.h"
#include "../../Common/WorldDefine/ride_protocol.h"
#include "RaidMgr.h"
#include "role.h"
#include "hearSay_helper.h"


RaidMgr::RaidMgr()
{

}

RaidMgr::~RaidMgr()
{

}

BOOL RaidMgr::init(Role* pRole, INT nStep, INT nGrade, INT nexp)
{
	m_pOwner = pRole;

	m_nStep = nStep;
	m_nGrade = nGrade;
	m_nExpCur = nexp;
	return TRUE;
}

VOID RaidMgr::SaveToDB()
{
	if (!VALID_POINT(m_pOwner))
		return;

	NET_DB2C_update_mounts send;
	send.dw_role_id = m_pOwner->GetID();
	send.sMounts.nExp = m_nExpCur;
	send.sMounts.nStep = m_nStep;
	send.sMounts.nGrade = m_nGrade;

	g_dbSession.Send(&send, send.dw_size);
}

// 获取坐骑模型
DWORD RaidMgr::getRaidMode()
{

	tagEquip* pRaid = m_pOwner->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if (!VALID_POINT(pRaid))
	{
		INT nLevel = 0;
		StepGrade2VLevel(m_nStep, m_nGrade, nLevel);
		const tagRaidProto* pProto = AttRes::GetInstance()->GetRaidProto(nLevel);
		if (VALID_POINT(pProto))
		{
			return pProto->dwModleID;
		}
		return INVALID_VALUE;
	}
		
	return pRaid->pEquipProto->nSpecFuncVal2;
}

INT RaidMgr::getLevel()
{
	INT nLevel = 0;
	StepGrade2VLevel(m_nStep, m_nGrade, nLevel);

	return nLevel;
}

VOID RaidMgr::ExpChange( INT nExpMod, BOOL bSend, BOOL bOverFlow /*= TRUE*/ )
{
	if (0 == nExpMod)
		return;

	INT		nLvlUpExp = 0;
	INT		nVLevel = 0;
	BOOL	bLvlUp	= FALSE;

	int nOldStep = m_nStep;//得到升级前的阶 
	// 减少经验
	if (nExpMod < 0)
	{
		m_nExpCur += nExpMod;
		m_nExpCur = m_nExpCur < 0 ? 0 : m_nExpCur;
	}
	// 增加经验
	else if (nExpMod > 0)
	{
		nLvlUpExp = GetExpLvlUp();
		m_nExpCur += nExpMod;

		// 计算新等级
		StepGrade2VLevel(m_nStep, m_nGrade, nVLevel);
		while (m_nExpCur >= nLvlUpExp && nLvlUpExp != 0 && nVLevel < MAX_RAID_VLEVEL)
		{
			//// 若达到当前等最顶级 且 灌注经验满
			//if (nLvlUpExp <= m_nExpCur )
			//{
			//	m_nExpCur = nLvlUpExp;
			//}

			AcitveAtt(false);
			
			m_nExpCur		-= nLvlUpExp;
			nVLevel		+= 1;
			VLevel2StepGrade(nVLevel, m_nStep, m_nGrade);

			AcitveAtt(true);

			m_pOwner->RecalAtt(TRUE);

			//SyncAllLvlUpChangeAtt();

			nLvlUpExp	= GetExpLvlUp();
			bLvlUp		= TRUE;
		}

		if (m_nExpCur > nLvlUpExp)
		{
			m_nExpCur = nLvlUpExp;
		}
	}

	if (bSend)
	{
		sendRaidData();
	}

	SaveToDB();

	if (m_nStep > nOldStep)//若升阶了
	{
		//升到特定的阶给全服公告
		if (3 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),3);
		}
		else if (4 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),4);
		}
		else if (5 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),5);
		}
		else if (6 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),6);
		}
		else if (7 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),7);
		}
		else if (8 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),8);
		}
		else if (9 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),9);
		}
		else if (10 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),10);
		}
		else if (11 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),11);
		}
		else if (12 == m_nStep)
		{
			HearSayHelper::SendMessage(EHST_RIDEUPGRADE,m_pOwner->GetID(),12);
		}
		else 
		{
			//do nothing
		}
	}
}

VOID RaidMgr::sendRaidData()
{
	NET_SIS_get_raid_att send;
	send.nLevel		= 0;
	StepGrade2VLevel(m_nStep, m_nGrade, send.nLevel);
	send.nCurExp = m_nExpCur;
	send.dwType = getRaidMode();
	m_pOwner->SendMessage(&send, send.dw_size);
}	

INT RaidMgr::GetExpLvlUp()
{

	INT nVLevel = 0;
	StepGrade2VLevel(m_nStep, m_nGrade, nVLevel);
	const tagRaidProto* pProto = AttRes::GetInstance()->GetRaidProto(nVLevel);
	return pProto->dwNeedExp;
}

VOID RaidMgr::AcitveAtt(BOOL bActive)
{
	INT32 nFactor = bActive ? 1 : -1;
	INT nVLevel = 0;
	StepGrade2VLevel(m_nStep, m_nGrade, nVLevel);
	const tagRaidProto* pProto = AttRes::GetInstance()->GetRaidProto(nVLevel);
	for (int i = 0; i < MAX_RAID_ATT_NUM; i++)
	{
		if (pProto->sRoleAtt[i].eRoleAtt != ERA_Null &&
			pProto->sRoleAtt[i].nValue != 0)
		{
			m_pOwner->ModAttModValue(pProto->sRoleAtt[i].eRoleAtt, pProto->sRoleAtt[i].nValue * nFactor);
			
		}
		
	}
	m_pOwner->SetRating(pProto->nRating * nFactor);
}

// 骑乘
DWORD	RaidMgr::BeginRaid()
{
	if (!VALID_POINT(m_pOwner))
		return INVALID_VALUE;

	if (m_pOwner->get_level() < MOUNTS_NEED_LEVEL)
		return INVALID_VALUE;

	if (m_pOwner->IsInState(ES_Dizzy) ||m_pOwner->IsInState(ES_Dead))
		return INVALID_VALUE;

	if (m_pOwner->IsInRoleStateAny(ERS_Combat | ERS_Mount | ERS_Stall | ERS_ComPractice))
	{
		return INVALID_VALUE;
	}


	if (m_pOwner->IsInStateCantMove())
		return INVALID_VALUE;

	// 安全区没有装备坐骑时装不能上马
	const tagEquip* pRaid = m_pOwner->GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if (m_pOwner->IsInRoleState(ERS_DancingArea) && !VALID_POINT(pRaid))
	{
		return E_Ride_BeginRide_Failed_Without_Fashion;
	}

	// 地图限制
	BOOL bLimited = ((Role*)m_pOwner)->ride_limit(NULL);
	if(bLimited) return E_Ride_BeginRide_Failed_StateLimit;

	m_pOwner->set_mount_ex( TRUE, 5000, NULL );
	

	return E_Success;
}

DWORD RaidMgr::CancelRaid()
{
	if (!VALID_POINT(m_pOwner))
		return INVALID_VALUE;

	if (!m_pOwner->IsInRoleState(ERS_Mount))
		return INVALID_VALUE;

	//设置取消骑乘后的速度
	m_pOwner->set_mount_ex( FALSE, 5000, NULL);
	
	
	return E_Success;
}

// 培养
DWORD RaidMgr::Tog(BYTE byType,  INT& nCritNum, INT& nGetExp)
{
	if (getLevel() >= MAX_RAID_VLEVEL)
		return INVALID_VALUE;

	if (m_pOwner->get_level() < MOUNTS_NEED_LEVEL)
		return INVALID_VALUE;

	// 每天次数限制
	if (m_pOwner->GetDayClearData(ERDCT_Tog_mounts) >= MAX_DAY_TOG_TIME) 
		return INVALID_VALUE;


	INT64 nMonery = 0;
	INT nLiquan = 0;
	INT nYuanbao = 0;

	INT nPro = get_tool()->tool_rand() % 100;
	INT nExp = 0;
	if (byType == 0 )
	{
		nMonery = 20000;
		if (m_pOwner->GetCurMgr().GetBagSilver() < 20000)
			return INVALID_VALUE;

		nExp = 100 + 25 * m_nStep;
		if (nPro < 25)
		{
			nExp *= 10;
			nCritNum = 1;
		}
	}
	else if( byType == 1)
	{
		if (m_pOwner->GetCurMgr().GetBagYuanBao() >= 10)
		{
			nLiquan = 10;
		}
		else if (m_pOwner->GetCurMgr().GetBaiBaoYuanBao() >= 10)
		{
			nYuanbao = 10;
		}
		else
		{
			return INVALID_VALUE;
		}
		
		nExp = 150 + 25 * m_nStep;
		if (nPro < 45)
		{
			nExp *= 10;
			nCritNum = 1;
		}
	}
	else if (byType == 2)
	{

		if (m_pOwner->GetCurMgr().GetBaiBaoYuanBao() >= 100)
		{
			nYuanbao = 100;
		}
		else
		{
			return INVALID_VALUE;
		}

		nExp = 0;
		for (int i = 0; i < 10; i++)
		{
			nPro = get_tool()->tool_rand() % 100;
			int nTempExp = 200 + 25 * m_nStep;
			if (nPro < 65)
			{
				nCritNum++;
				nTempExp *= 10;
			}
			nExp += nTempExp;
		}
		
	}

	else if (byType == 3)
	{

		if (m_pOwner->GetCurMgr().GetBaiBaoYuanBao() >= 1000)
		{
			nYuanbao = 1000;
		}
		else
		{
			return INVALID_VALUE;
		}

		nExp = 0;
		for (int i = 0; i < 100; i++)
		{
			nPro = get_tool()->tool_rand() % 100;
			int nTempExp = 200 + 25 * m_nStep;
			if (nPro < 75)
			{
				nCritNum++;
				nTempExp *= 10;
			}
			nExp += nTempExp;
		}

	}

	m_pOwner->ModRoleDayClearDate(ERDCT_Tog_mounts);

	
	ExpChange(nExp, TRUE);
	nGetExp = nExp;
	// 消耗元宝金币
	m_pOwner->GetCurMgr().DecBagSilver(nMonery, elcid_role_pour_exp_pet);
	m_pOwner->GetCurMgr().DecBagYuanBao(nLiquan, elcid_role_pour_exp_pet);
	m_pOwner->GetCurMgr().DecBaiBaoYuanBao(nYuanbao, elcid_role_pour_exp_pet);



	return E_Success;
}