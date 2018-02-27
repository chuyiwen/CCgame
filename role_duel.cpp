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
 *	@file		role_duel
 *	@author		mwh
 *	@date		2011/05/17	initial
 *	@version	0.0.1.0
 *	@brief		切磋系统
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/duel_protocol.h"
#include "map.h"
#include "player_session.h"
#include "creature.h"
#include "role.h"
#include "role_mgr.h"
#include "map_instance_pvp.h"
#include "map_instance_1v1.h"
#include "map_instance_pvp_biwu.h"

#define MAP_ROLE_GET(Var,RoleID)\
	Role* Var = 0; Map* pMap = this->get_map();\
	if(VALID_POINT(pMap)) Var=pMap->find_role((RoleID));

#define MAP_ROLE_GET_RETURN(Var,RoleID, Ret)\
	Map* pMap = this->get_map();\
	if(!VALID_POINT(pMap)) return Ret;\
	Role* Var = pMap->find_role((RoleID));\
	if(!VALID_POINT(Var)) return Ret;

VOID Role::UpdateDuel()
{
	if(!IsDuel()) return;
	else if(IsDuelPrepare()) UpdatePrepare();
	else if(IsDuelOperate()) UpdateOperate();
	else EndDuel();
}

VOID Role::UpdatePrepare()
{
	if(!IsDuel()) return;
	if(!IsDuelPrepare()) return;

	MAP_ROLE_GET(pTarget, GetDuelTarget());
	if(!VALID_POINT(pTarget))
	{
		NET_SIS_AskForDuel send;
		send.dwTarget = GetDuelTarget();
		send.dwError = ETarget_NotExistOrOffline;
		this->SendMessage(&send, send.dw_size);
		this->EndDuel(); return;
	}
	
	if((--mDuelTickDown == 0) )
	{
		NET_SIS_AskForDuel send;
		send.dwTarget = GetID();
		send.dwError = EAskForDuel_TargetRefuse;
		pTarget->SendMessage(&send, send.dw_size);
		this->EndDuel(); pTarget->EndDuel();
		return;
	}
}

VOID Role::UpdateOperate()
{
	if(!IsDuel()) return;
	if(!IsDuelOperate()) return;
	
	// 目标丢失
	MAP_ROLE_GET(pTarget,GetDuelTarget());
	if(!VALID_POINT(pTarget))
	{
		NET_SIS_DuelFinish send;
		send.dwError = E_Success;
		send.dwTargetID = GetDuelTarget();
		SendMessage(&send, send.dw_size);
		DealDuelBanner(FALSE);
		DuelBroadcast(GetID(), GetDuelTarget());
		this->EndDuel();  return ;
	}

	// 距离检测
	if(!IsInDuelDistance())
	{
		NET_SIS_DuelFinish send;
		send.dwError = E_Success;
		send.dwTargetID = GetDuelTarget();
		SendMessage(&send, send.dw_size);

		send.dwTargetID = GetID();
		pTarget->SendMessage(&send, send.dw_size);

		DealDuelBanner(FALSE);
		DuelBroadcast(GetDuelTarget(), GetID());
		
		this->EndDuel(); pTarget->EndDuel();  
		return ;
	}
}

VOID Role::EndDuel()
{
	InitDuel();
}

VOID Role::DuelBroadcast(DWORD dwWinner, DWORD dwLoser)
{
	NET_SIS_DuelResultBroad send;
	send.dwWinner = dwWinner;
	send.dwLoser = dwLoser;
	Map* pMap = this->get_map();
	if(VALID_POINT(pMap))
		pMap->send_big_visible_tile_message(this, &send, send.dw_size);

	Role* p_win_role = g_roleMgr.get_role(dwWinner);
	Role* p_lost_role = g_roleMgr.get_role(dwLoser);

	if(VALID_POINT(p_win_role))
	{
		p_win_role->GetAchievementMgr().UpdateAchievementCriteria(eta_duel_win,1);
		if(VALID_POINT(p_lost_role))
		{
			p_win_role->GetAchievementMgr().UpdateAchievementCriteria(ete_duel_class, p_lost_role->GetClass(), 1);
			p_win_role->GetAchievementMgr().UpdateAchievementCriteria(ete_duel_level, p_lost_role->get_level() - p_win_role->get_level(), 1);
		}
	}
	if(VALID_POINT(p_lost_role))
	{
		p_lost_role->GetAchievementMgr().UpdateAchievementCriteria(eta_duel_lost,1);
	}
}

BOOL Role::IsInDuelDistance()
{
	FLOAT fRealtSq = Vec3DistSq(GetCurPos(), mBannerPos);
	return fRealtSq < DUELMAXDISTANCESQ;
}
INT Role::CheckDuelLimitSelf(Role* pTarget,  BOOL bSecond)
{
	if(HasPKValue())
	{
		return EAskForDuel_RedNameYourSelf;
	}

	if(IsInRoleStateAny(ERS_StallSet | ERS_Stall) || GetPKState() != ERolePK_Peace) 
	{
		return EAskForDuel_StateLimitYourSelf;
	}

	if(!this->IsInDistance(*pTarget, ASKDISTANCEMAX))
	{
		return EAskForDuel_OutOfDistance;
	}

	if(IsDuel() && !bSecond)
	{
		if(GetDuelTarget() == pTarget->GetID())
		{
			return EAskForDuel_AlreadyAskFor;
		}
		return EAskForDuel_AlreadyInDuel;
	}

	return E_Success;
}
INT Role::CheckDuelLimitTarget(Role* pTarget, BOOL bSecond)
{
	// 再目标
	if(pTarget->HasPKValue())
	{
		return EAskForDuel_RedNameTarget;
	}
	
	if(pTarget->IsInRoleStateAny(ERS_StallSet | ERS_Stall) || pTarget->GetPKState() != ERolePK_Peace)
	{
		return EAskForDuel_StateLimitTarget;
	}
	
	if(pTarget->IsDuel() && !bSecond)
	{
		return EAskForDuel_TargetAlreadyInDuel;
	}

	return E_Success;
}

INT Role::AskForDuel(DWORD dwTarget)
{
	MAP_ROLE_GET_RETURN(pTarget, dwTarget, ETarget_NotExistOrOffline);

	INT tRet = CheckDuelLimitSelf(pTarget);
	if(tRet != E_Success) return tRet;

	tRet = CheckDuelLimitTarget(pTarget);
	if(tRet != E_Success) return tRet;

	this->SetDuelTarget(dwTarget);
	this->SetDuel(TRUE);
	this->SetDuelPrepare(TRUE);
	this->SetDuelTarget(dwTarget);
	this->SetDuelTickDown(ASKTIMEOUTTIMETICK+10);

	NET_SIS_AskForDuelTransfer send;
	send.dwTarget = this->GetID();
	pTarget->SendMessage(&send, send.dw_size);
	pTarget->SetDuel(TRUE);
	pTarget->SetDuelPrepare(TRUE);
	pTarget->SetDuelTarget(GetID());
	pTarget->SetDuelTickDown(ASKTIMEOUTTIMETICK);

	return E_Success;
}

VOID Role::DuelResponse(DWORD dwTarget, BYTE byAck)
{
	MAP_ROLE_GET(pTarget, dwTarget);

	if(!IsDuelPrepare()) return;

	if(!VALID_POINT(pTarget))
	{
		NET_SIS_AskForDuelResponse  send;
		send.dwTarget = dwTarget;
		send.dwError = ETarget_NotExistOrOffline;
		this->SendMessage(&send, send.dw_size);
		this->EndDuel(); return;
	}

	if(!IsDuel())
	{
		NET_SIS_AskForDuelResponse send;
		send.dwTarget = dwTarget;
		send.dwError = EDuel_Breakout;
		this->SendMessage(&send, send.dw_size);
		pTarget->EndDuel(); this->EndDuel();
		return;
	}

	if(!byAck)
	{
		NET_SIS_AskForDuel send;
		send.dwTarget = GetID();
		send.dwError = EAskForDuel_TargetRefuse;
		pTarget->SendMessage(&send, send.dw_size);
		EndDuel(); pTarget->EndDuel();
		return ;
	}
	
	// 二次判断
	INT tRet = CheckDuelLimitTarget(pTarget, TRUE);
	if(tRet != E_Success)
	{
		{
			NET_SIS_AskForDuelResponse send;
			send.dwError = tRet;
			send.dwTarget = dwTarget;
			SendMessage(&send, send.dw_size);
		}

		{
			NET_SIS_AskForDuel send;
			send.dwTarget = GetID();
			send.dwError = EAskForDuel_StateLimitYourSelf;
			pTarget->SendMessage(&send, send.dw_size);
		}

		EndDuel(); pTarget->EndDuel();
		return;
	}

	tRet = CheckDuelLimitSelf(pTarget, TRUE);
	if(tRet != E_Success)
	{
		{
			NET_SIS_AskForDuelResponse send;
			send.dwError = tRet;
			send.dwTarget = dwTarget;
			SendMessage(&send, send.dw_size);
		}
		{
			NET_SIS_AskForDuel send;
			send.dwError = tRet;
			send.dwTarget = GetID();
			if(tRet != EAskForDuel_OutOfDistance) 
				send.dwError = EAskForDuel_StateLimitTarget;
			pTarget->SendMessage(&send, send.dw_size);
		}

		EndDuel(); pTarget->EndDuel();
		return;
	}

	Vector3 _thisPos = GetCurPos();
	Vector3 _TargetPos = pTarget->GetCurPos();
	mBannerPos.x = (_thisPos.x + _TargetPos.x)/2;
	mBannerPos.z = (_thisPos.z + _TargetPos.z)/2;
	DealDuelBanner(TRUE); 

	NET_SIS_DuelStart send;
	send.dwTargetID = GetID();
	send.bannerPos = mBannerPos;
	pTarget->SendMessage(&send, send.dw_size);
	pTarget->SetBannerPos(mBannerPos);
	pTarget->SetDueBannerID(mDueBannerID);
	pTarget->SetDuelPrepare(FALSE);
	pTarget->SetDuelOperate(TRUE);

	send.dwTargetID = pTarget->GetID();
	this->SendMessage(&send, send.dw_size);
	this->SetDuelPrepare(FALSE);
	this->SetDuelOperate(TRUE);
}

// pvp死亡
BOOL Role::PVPDead(Unit* pKiller)
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return FALSE;

	if(pMap->get_map_info()->e_type != EMT_PVP)
		return FALSE;

	map_instance_pvp* p_instance_pvp = (map_instance_pvp*)pMap;
	if(!VALID_POINT(p_instance_pvp))
		return FALSE;

	this->RemoveAllBuff(FALSE); // 清除所有DBUFF
	this->WhenDeadDealDuelHPMP(); 
	GetCombatHandler().EndUseSkill( );

	p_instance_pvp->on_leave_instance(GetID());

	if(VALID_POINT(pKiller) && pKiller->IsRole())
	{
		NET_SIS_pvp_result send;
		send.b_win = FALSE;
		g_roleMgr.get_role_name(pKiller->GetID(), send.sz_kill_name);
		SendMessage(&send, send.dw_size);

		send.b_win = TRUE;
		g_roleMgr.get_role_name(GetID(), send.sz_kill_name);
		((Role*)pKiller)->SendMessage(&send, send.dw_size);
	}

	return TRUE;
}
// pvp比武死亡
BOOL Role::PVPBiWuDead(Unit* pKiller)
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return FALSE;

	if(pMap->get_map_info()->e_type != EMT_PVP_BIWU)
		return FALSE;

	map_instance_pvp_biwu* p_instance_pvp_biwu = (map_instance_pvp_biwu*)pMap;
	if(!VALID_POINT(p_instance_pvp_biwu))
		return FALSE;

	this->RemoveAllBuff(FALSE); // 清除所有DBUFF
	this->WhenDeadDealDuelHPMP(); 
	GetCombatHandler().EndUseSkill( );

	p_instance_pvp_biwu->on_leave_instance(GetID());

	pMap->on_role_die(this, pKiller);

	//if(VALID_POINT(pKiller) && pKiller->IsRole())
	//{
	//	NET_SIS_pvp_biwu_result send;
	//	send.b_win = FALSE;
	//	g_roleMgr.get_role_name(pKiller->GetID(), send.sz_kill_name);
	//	SendMessage(&send, send.dw_size);

	//	send.b_win = TRUE;
	//	g_roleMgr.get_role_name(GetID(), send.sz_kill_name);
	//	((Role*)pKiller)->SendMessage(&send, send.dw_size);
	//}

	return TRUE;
}

// 1v1竞技场死亡
BOOL Role::Dead1v1(Unit* pKiller)
{
	Map* pMap = get_map();
	if(!VALID_POINT(pMap))
		return FALSE;

	if(pMap->get_map_info()->e_type != EMT_1v1)
		return FALSE;

	map_instance_1v1* p_instance_1v1 = (map_instance_1v1*)pMap;
	if(!VALID_POINT(p_instance_1v1))
		return FALSE;

	this->RemoveAllBuff(FALSE); // 清除所有DBUFF
	this->WhenDeadDealDuelHPMP(); 
	GetCombatHandler().EndUseSkill( );

	p_instance_1v1->on_leave_instance(GetID());

	return TRUE;
}

BOOL Role::WhenDeadDuelDeal(Unit* pKiller)
{
	if(!IsDuel()) return FALSE;
	if(!VALID_POINT(pKiller) ||
		!pKiller->IsRole() || 
		pKiller->GetID() != GetDuelTarget())
	{
		NET_SIS_DuelFinish send;
		send.dwError = EDuel_Breakout;
		send.dwTargetID = GetDuelTarget();
		SendMessage(&send, send.dw_size);

		Role* pTarget = g_roleMgr.get_role(GetDuelTarget());
		if(VALID_POINT(pTarget))
		{
			send.dwTargetID = GetID();
			pTarget->SendMessage(&send, send.dw_size);
			pTarget->EndDuel();
		}

		DealDuelBanner(FALSE);
		this->EndDuel(); return FALSE;
	}


	Role* pTarget = static_cast<Role*>(pKiller);
	
	NET_SIS_DuelFinish send;
	send.dwError = E_Success;
	send.dwTargetID = GetDuelTarget();
	this->SendMessage(&send, send.dw_size);
	this->RemoveAllBuff(FALSE); // 清除所有DBUFF
	this->WhenDeadDealDuelHPMP(); 
	GetCombatHandler().EndUseSkill( );

	send.dwTargetID = GetID();
	pTarget->SendMessage(&send, send.dw_size);
	pTarget->RemoveAllBuff(FALSE); // 清除所有DBUFF
	pTarget->WhenDeadDealDuelHPMP();
	pTarget->GetCombatHandler().EndUseSkill( );

	DealDuelBanner(FALSE);
	DuelBroadcast(GetDuelTarget(), GetID());

	this->EndDuel(); pTarget->EndDuel();  

	pTarget->GetAchievementMgr().UpdateAchievementCriteria(eta_duel, 1);
	this->GetAchievementMgr().UpdateAchievementCriteria(eta_duel, 1);

	return TRUE;
}
VOID Role::WhenDeadDealDuelHPMP()
{
	//FLOAT  fAddPer =  FINISHHPRESTOREPER / 10000.0f;
	//ModAttValue(ERA_HP, GetAttValue(ERA_MaxHP) * fAddPer );

	// 药师和刺客回蓝5%
	//EClassType eClass = GetClass();
	//if(eClass == EV_Astrologer || eClass == EV_Blader)
	//	ModAttValue(ERA_MP, GetAttValue(ERA_MaxMP) * fAddPer );
}
VOID Role::DealDuelBanner(BOOL bShow)
{
	Map* pMap = get_map( );
	if(!VALID_POINT(pMap)) return;

	if(!bShow)
	{
		if(VALID_POINT(mDueBannerID))
		{
			Creature* pCreature = pMap->find_creature(mDueBannerID);
			if(VALID_POINT(pCreature) ) pCreature->OnDisappear( );
		}
		mDueBannerID = 0;
	}
	else if(!VALID_POINT(mDueBannerID))
	{
		ASSERT(!VALID_POINT(mDueBannerID));

		Vector3 vPos = mBannerPos;
		Vector3 vFace = this->GetFaceTo();
		Creature* pCreature = pMap->create_creature(DUELBANNERTYPEID, vPos, vFace, INVALID_VALUE, FALSE);
		if(VALID_POINT(pCreature)) mDueBannerID = pCreature->GetID();
	}
}

VOID Role::InitDuel()
{
	mIsDuel = FALSE;
	mIsDuelPrepare = FALSE;
	mIsDuelOperate = FALSE;
	mDuelTickDown = 0;
	mDuelTarget = 0;
	mDueBannerID = 0;
}