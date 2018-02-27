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
 *	@file		role_ride
 *	@author		mawenhong
 *	@date		2010/11/16	initial
 *	@version	0.0.1.0
 *	@brief		坐骑相关
*/

#include "StdAfx.h"

#include "player_session.h"

#include "../../common/WorldDefine/RoleDefine.h"
#include "../../common/WorldDefine/role_att_protocol.h"
#include "../common/ServerDefine/log_server_define.h"
#include "../../common/WorldDefine/ride_protocol.h"
#include "map.h"
#include "att_res.h"
#include "skill.h"
#include "buff.h"
#include "role.h"
#include "creature_ai.h"
#include "role_mgr.h"
#include "pet_server_define.h"
#include "hearSay_helper.h"
//--------------------------------------------------------------------
// 坐骑强化
//--------------------------------------------------------------------
DWORD Role::upgrade_ride(DWORD dw_npc, INT64 ride_serial,  BYTE number, BOOL useBind)
{
	BYTE old_solidate	= 0;
	DWORD dw_probility	= 0; 
	Creature *p_npc = NULL;
	tagEquip *p_ride = NULL;
	tagItem	*p_ride_item = NULL;
	const tagItemProto *p_proto = NULL;
	const tagItem *p_soulstone = NULL, *p_godstone = NULL;
	
	//Npc功能判断
	p_npc = get_map( )->find_creature( dw_npc );
	if( !VALID_POINT( p_npc ) || !p_npc->IsFunctionNPC( EFNPCT_Ride ))
		return E_Ride_Upgrade_Failed_Error_Npc;
	
	//坐骑判断
	p_ride_item = GetItemMgr( ).GetBag( ).GetItem( ride_serial );
	if( !VALID_POINT( p_ride_item )) return E_Ride_Upgrade_Failed_Not_Exist;

	p_proto = p_ride_item->pProtoType;
	if( !VALID_POINT(p_proto )||!MEquipIsRide(p_proto) )
		return E_Ride_Upgrade_Failed_Not_Exist;

	//强化等级判断
	p_ride = (tagEquip*)p_ride_item;
	if( p_ride->equipSpec.byConsolidateLevel >= MAX_RIDE_CONSOLIDATE_LEVEL )
		return E_Ride_Upgrade_Failed_MaxLevel;

	//骑乘状态判断
	//if( p_ride->equipSpec.eRideState == ERES_Riding )
	//	return E_Ride_Upgrade_Failed_Riding;

	//扣钱判断
	if( GetCurMgr( ).GetBagSilver(  ) < CONSOLIDATE_COST )
		return E_Ride_Upgrade_Failed_Out_of_Money;

	//灵魂石判断
	DWORD dwSoulStoneType = ItemHelper::GetRideUpgradeSoulStoneTypeID( p_ride->equipSpec.byConsolidateLevel);
	DWORD dwGodStoneType = ItemHelper::GetRideUpgradeGodStoneTypeID();

	package_list<tagItem*> SoulStones, GodStones;
	GetItemMgr().GetBag().GetSameItemList(SoulStones, dwSoulStoneType, 1);
	p_soulstone = SoulStones.pop_front();
	if(!VALID_POINT(p_soulstone))
		return E_Ride_Upgrade_Failed_No_SoulStone;


	p_proto = p_soulstone->pProtoType;
	if( !VALID_POINT(p_proto) || p_proto->eSpecFunc != EIST_SoulStone)
		return E_Ride_Upgrade_Failed_No_SoulStone;

	//如果使用了神石
	if( number )
	{
		INT nRetNumber = GetItemMgr().GetBag().GetSameItemList(GodStones, dwGodStoneType, number);
		if(!nRetNumber || nRetNumber < number)
			return E_Ride_Upgrade_Failed_No_GodStone;

		p_godstone = GodStones.front();
		p_proto = p_godstone->pProtoType;
		if( !VALID_POINT(p_proto) || p_proto->eSpecFunc != EIST_GodStone)
			return E_Ride_Upgrade_Failed_No_GodStone;
		
		//提升成功率
		dw_probility = RideGodStoneProbCalc(min(MAX_GODSTONE_NUM_MAX,number));

		GetAchievementMgr().UpdateAchievementCriteria(ete_use_item, dwGodStoneType, min(MAX_GODSTONE_NUM_MAX,number));
	}

	//删除道具、扣钱
	GetItemMgr( ).DelFromBag( p_soulstone->n64_serial, elcid_upgrade_ride, 1);
	if( number ) GetItemMgr().DelBagSameItem(GodStones, number, elcid_upgrade_ride);
	GetCurMgr( ).DecBagSilver( CONSOLIDATE_COST, elcid_upgrade_ride );

	//开始强化
	old_solidate = p_ride->equipSpec.byConsolidateLevel;
	dw_probility += g_RideUpg[old_solidate].dwProbability;
	dw_probility += GET_VIP_EXTVAL(GetTotalRecharge(), CONSOLIDATE_RIDE_PROB, INT);

	BOOL bSuccess = get_tool()->probability( dw_probility / 100 );
	if(bSuccess) p_ride->equipSpec.byConsolidateLevel += 1;
	else p_ride->equipSpec.byConsolidateLevel = g_RideUpg[old_solidate].byFailedTo;

	if( bSuccess && VALID_POINT(m_pScript) )
		m_pScript->OnRideConsolidate( this, p_ride->equipSpec.byConsolidateLevel );

	if(!p_ride->IsBind() && useBind)
	{
		p_ride->SetBind(EBS_Bind);

		NET_SIS_bind_change send;
		send.n64EquipSerial = p_ride->n64_serial;
		send.byBind = p_ride->byBind;
		send.dwConType = p_ride->eConType;
		SendMessage(&send, send.dw_size);
	}
	
	cal_ride_speed( p_ride );
	GetItemMgr( ).UpdateEquipSpec(*p_ride);

	// 强化时发送传闻消息
	BOOL bSendHearChat = old_solidate > 6 ? TRUE : (bSuccess && old_solidate == 6);
	if(bSendHearChat)
	{
		HearSayHelper::SendMessage(EHST_RIDEUPGRADE,
			this->GetID(), p_ride->dw_data_id,  old_solidate + 1,   bSuccess, INVALID_VALUE, p_ride);
	}
	
	if (bSuccess)
	{
		GetAchievementMgr().UpdateAchievementCriteria(ete_strengthen_ride_success, p_ride->equipSpec.byConsolidateLevel);
	}
	else
	{
		GetAchievementMgr().UpdateAchievementCriteria(ete_strengthen_ride_fail, old_solidate, 1);
	}
	
	GetAchievementMgr().UpdateAchievementCriteria(ete_strengthen_ride, 1);

	return bSuccess ? E_Success : E_Ride_Upgrade_Failed_Failed;
}
//--------------------------------------------------------------------
// 上装备
//--------------------------------------------------------------------
DWORD Role::ride_inlay(INT64 ride_serial, INT64* ride_equip, INT equip_number)
{
	INT	n_index = INVALID_VALUE;
	tagEquip *p_ride = NULL;
	tagItem	*p_ride_item = NULL;
	const tagItem *p_ride_equip = NULL;
	const tagItemProto *p_proto = NULL;

	//坐骑装备判断
	p_ride_item = GetItemMgr( ).GetBag( ).GetItem( ride_serial );
	if( !VALID_POINT( p_ride_item )) return E_Ride_InLay_Failed_Not_Exist;

	p_proto = p_ride_item->pProtoType;
	if( !VALID_POINT(p_proto )||!MEquipIsRide(p_proto))
		return E_Ride_InLay_Failed_Not_Exist;

	//骑乘状态
	p_ride = (tagEquip*)p_ride_item;
	//if( p_ride->equipSpec.eRideState == ERES_Riding)
	//	return E_Ride_InLay_Failed_Riding;

	// 整体验证
	for(INT n = 0; n < equip_number; ++n)
	{
		if(VALID_POINT(ride_equip[n]))
		{
			p_ride_equip = GetItemMgr( ).GetBag( ).GetItem( ride_equip[n] );
			if( !VALID_POINT(p_ride_equip) )
				return E_Ride_InLay_Failed_No_Equip;

			p_proto = p_ride_equip->pProtoType;
			if( !VALID_POINT(p_proto ) || p_proto->eSpecFunc != EIST_RideEquip )
				return E_Ride_InLay_Failed_No_Equip;	
		}
	}

	// 整体上装备
	ASSERT(equip_number <= MAX_RIDEHOLE_NUM);
	for(INT n = 0; n < equip_number; ++n)
	{//上装备
		if(VALID_POINT(ride_equip[n]))
		{
			p_ride_equip = GetItemMgr( ).GetBag( ).GetItem( ride_equip[n] );
			if(VALID_POINT(p_ride_equip))
			{
				//p_ride->equipSpec.byHoleGemNess[n] = p_ride_equip->pProtoType->nGemNess;
				p_ride->equipSpec.dwHoleGemID[ n ] = p_ride_equip->dw_data_id;
				GetItemMgr( ).ItemUsedFromBag( ride_equip[n], 1, elcid_riad_inlay);

				GetAchievementMgr().UpdateAchievementCriteria(ete_xiangqian_ride_item, p_ride_equip->dw_data_id, 1);
			}
		}
	}
	
	INT nHoleGemNum = 0;
	for(INT n = 0; n < MAX_EQUIPHOLE_NUM; ++n)
	{//上装备
		
		if (p_ride->equipSpec.dwHoleGemID[ n ] != 0 && p_ride->equipSpec.dwHoleGemID[ n ] != -1)
		{
			nHoleGemNum++;
		}
	}
	
	GetAchievementMgr().UpdateAchievementCriteria(ete_xiangqian_ride, nHoleGemNum);
	

	// 算属性
	cal_ride_speed( p_ride );
	GetItemMgr( ).UpdateEquipSpec(*p_ride);

	return E_Success;
}
//--------------------------------------------------------------------
// 移除镶嵌
//--------------------------------------------------------------------
DWORD Role::remove_ride_inlay(DWORD dw_npc, INT64 ride_serial, INT64 crush_stone, BYTE index )
{
/*
	Creature *p_npc = NULL;
	tagEquip *p_ride = NULL;
	tagItem *p_ride_item = NULL;
	const tagItemProto *p_proto = NULL;
	const tagItem *p_crushstone = NULL;

	//Npc功能判断
	p_npc = get_map( )->find_creature( dw_npc );
	if( !VALID_POINT( p_npc ) || !p_npc->IsFunctionNPC( EFNPCT_Ride ))
		return E_Ride_RemoveInLay_Failed_Error_Npc;

	//坐骑装备判断
	p_ride_item = GetItemMgr( ).GetBag( ).GetItem( ride_serial );
	if( !VALID_POINT( p_ride_item ))
		return E_Ride_RemoveInLay_Failed_Not_Exist;

	p_proto = p_ride_item->pProtoType;
	if( !VALID_POINT(p_proto )||!MEquipIsRide(p_proto))
		return E_Ride_RemoveInLay_Failed_Not_Exist;

	//骑乘状态
	p_ride = (tagEquip*)p_ride_item;
	if( p_ride->equipSpec.eRideState == ERES_Riding)
		return E_Ride_RemoveInLay_Failed_Riding;

	//磨铁石
	p_crushstone = GetItemMgr( ).GetBag( ).GetItem( crush_stone );
	if( !VALID_POINT(p_crushstone ) )
		return E_Ride_RemoveInLay_Failed_No_CrushStone;

	p_proto = p_crushstone->pProtoType;
	if( !VALID_POINT(p_proto ) || p_proto->eSpecFunc != EIST_CrushStone )
		return E_Ride_RemoveInLay_Failed_No_CrushStone;

	//所需金币
	INT32 n_cost = 0;
	if( GetCurMgr( ).GetBagSilver(  ) < n_cost )
		return E_Ride_RemoveInLay_Failed_Out_Of_Money;

	//镶嵌物判断
	if( index >= MAX_RIDEHOLE_NUM )
		return E_Ride_RemoveInLay_Failed_No_InLay;

	if( !VALID_POINT( p_ride->equipSpec.dwHoleGemID[index] ) )
		return E_Ride_RemoveInLay_Failed_No_InLay;

	//删除道具、扣钱
	GetItemMgr( ).DelFromBag( crush_stone, elcid_remove_ride_inlay, 1);
	GetCurMgr( ).DecBagSilver( n_cost, elcid_remove_ride_inlay);

	//清除数据
	p_ride->equipSpec.dwHoleGemID[ index ] = 0;
	for( BYTE n = index + 1; n < MAX_RIDEHOLE_NUM; ++n)
	{
		if( p_ride->equipSpec.dwHoleGemID[n] )
		{
			p_ride->equipSpec.dwHoleGemID[index] = p_ride->equipSpec.dwHoleGemID[n];
			++index;
		}
	}
	p_ride->equipSpec.dwHoleGemID[ index ] = 0;

	cal_ride_speed( p_ride );
	GetItemMgr( ).UpdateEquipSpec(*p_ride);
*/
	return E_Success;
}
//--------------------------------------------------------------------
// 开始骑乘
//--------------------------------------------------------------------
VOID Role::StartRideSpell( )
{

	NET_SIS_begin_ride send;
	send.dwError = GetCombatHandler().UseRide();
	send.dwRoleID = this->GetID();
	//send.nSpellTime = GetCombatHandler().GetRidePrepareCountDown();
	//if(send.dwError != E_Success)
	//{
	//	send.nSpellTime = 0;
	//}else{
	//	tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	//	if(VALID_POINT(pEquip)) send.dwTypeID = pEquip->dw_data_id;
	//}
	if(VALID_POINT(get_map()))
		get_map()->send_big_visible_tile_message(this, &send, send.dw_size);
}
INT Role::begin_ride_op()
{
	tagEquip *pRide = NULL;
	const tagPetProto *p_pet_proto = NULL;

	pRide = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pRide) || !VALID_POINT(pRide->pProtoType) )
	{
		return E_Ride_BeginRide_Failed_Not_Exist;
	}

	p_pet_proto = AttRes::GetInstance()->GetPetProto(((tagEquipProto*)pRide->pProtoType)->dwPetID );
	if( !VALID_POINT(p_pet_proto ))
		return E_Ride_BeginRide_Failed_Not_Exist;

	//骑乘状态
	//if( pRide->equipSpec.eRideState == ERES_Riding || ride_limit( pRide ) )
	//	return E_Ride_BeginRide_Failed_Riding;
	//else pRide->equipSpec.eRideState = ERES_Riding;


	//设置骑乘后的速度
	set_mount_ex( TRUE, 10000, p_pet_proto );
	set_ride_data( pRide->n64_serial, p_pet_proto->dw_data_id, pRide->equipSpec.byConsolidateLevel, 10000);
	//m_AttackStopMountProb = AttRes::GetInstance()->GetVariableLen().attack_stop_mount_prob;
	//m_AttackStopMountProtect = FALSE;

	//{//周围玩家同步
	//	NET_SIS_new_mount send;
	//	send.dw_role_id = this->GetID( );
	//	send.bySolidateLevel = pRide->equipSpec.byConsolidateLevel;
	//	send.dwPetTypeID = p_pet_proto->dw_data_id;
	//	this->get_map( )->send_big_visible_tile_message( this, &send, send.dw_size );
	//}

	//ride_buff_op(pRide, true);

	return E_Success;
}
void Role::ride_buff_op(tagEquip* p_ride, bool add)
{
	for(INT n = 0; n < MAX_EQUIPHOLE_NUM; ++n)
	{
		if(!VALID_POINT(p_ride->equipSpec.dwHoleGemID[ n ]))
			continue;

		const tagItemProto* p_proto = AttRes::GetInstance()->GetItemProto(p_ride->equipSpec.dwHoleGemID[ n ]);
		if(!VALID_POINT(p_proto)) continue;

		const tagBuffProto* p_buff = AttRes::GetInstance()->GetBuffProto(p_proto->nSpecFuncVal2);
		if(!VALID_POINT(p_buff)) continue;

		if(add) TryAddBuff(this, p_buff, NULL, NULL, NULL);
		else RemoveBuff( Buff::GetIDFromTypeID(p_buff->dwID), TRUE);
	}
}
//--------------------------------------------------------------------
// 取消骑乘
//--------------------------------------------------------------------
DWORD Role::cancel_ride()
{
	tagEquip* pRide = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pRide) || !VALID_POINT(pRide->pProtoType))
		return E_Ride_CancelRide_Failed_Not_Riding;

	const tagPetProto* pPetProto = AttRes::GetInstance()->GetPetProto(((tagEquipProto*)pRide->pProtoType)->dwPetID );
	if( !VALID_POINT(pPetProto )) return E_Ride_BeginRide_Failed_Not_Exist;

	//if( pRide->equipSpec.eRideState != ERES_Riding)
	//	return E_Ride_BeginRide_Failed_Riding;
	//else pRide->equipSpec.eRideState = ERES_Idle;

	//设置取消骑乘后的速度
#ifdef USE_RIDE_EX
	OnInterruptBuffEvent(EBIF_UnMount);
#endif
	set_mount_ex( FALSE, get_ride_speed(), pPetProto);
	set_ride_data(0,0,0,0);
	ride_buff_op(pRide, false);
	m_AttackStopMountProb = AttRes::GetInstance()->GetVariableLen().attack_stop_mount_prob;
	m_AttackStopMountProtect = FALSE;

	//! 计算镶嵌物时限
	BOOL bNeedCalcAtt = FALSE;
	for(int n = 0; n < MAX_RIDEHOLE_NUM; ++n)
	{
		DWORD dwGemID = pRide->equipSpec.dwHoleGemID[n];
		//if(VALID_POINT(dwGemID) && VALID_POINT(pRide->equipSpec.byHoleGemNess[n]))
		//{
			/*--pRide->equipSpec.byHoleGemNess[n];
			if(pRide->equipSpec.byHoleGemNess[n] == 0)
			{
				pRide->equipSpec.dwHoleGemID[n] = 0;
				pRide->equipSpec.byHoleGemNess[n] = 0;
			}*/
		//	bNeedCalcAtt = TRUE;
		//}
	}

	if(bNeedCalcAtt)
	{
		cal_ride_speed( pRide );
		GetItemMgr( ).UpdateEquipSpec(*pRide);
	}

	return E_Success;
}

//VOID Role::cancel_ride_ex( )
//{
//
//	if (IsInState(ERS_Mount))
//	{
//		NET_SIS_cancel_ride sisCancelRide;
//		sisCancelRide.dwError	= GetRaidMgr().CancelRaid( );
//		SendMessage(&sisCancelRide, sisCancelRide.dw_size );
//	}
//
//}
VOID Role::InterruptRide()
{
	GetCombatHandler().InterruptRide();
}
//--------------------------------------------------------------------
// 设置骑乘
//--------------------------------------------------------------------
VOID Role::set_mount( BOOL b_set, INT n_speed, const tagPetProto* p_proto )
{
	if (b_set)
	{
		// 设置人物状态
		SetRoleState(ERS_Mount);

		// 设置碰撞盒
		SetSize(p_proto->vSize);

		// 设置骑乘速度
		ModAttModValue(ERA_Speed_Mount, n_speed);
	}
	else
	{
		// 设置人物状态
		UnsetRoleState(ERS_Mount);

		// 设置碰撞盒
		SetAttRecalFlag(ERA_Shape);

		// 设置骑乘速度
		ModAttModValue(ERA_Speed_Mount, -n_speed);
	}

	RecalAtt();	
}
VOID Role::set_mount_ex( BOOL b_set, INT n_speed, const tagPetProto* p_proto )
{
	//增加role裸体速度的百分比
	//INT n_speed_delta = (INT)( GetBaseAttValue(ERA_Speed_XZ) * ( n_speed / 10000.0f ) );
	if (b_set)  
	{
		// 设置人物状态
		SetRoleState(ERS_Mount);

		// 设置碰撞盒 
		//SetSize(p_proto->vSize);

		// 设置骑乘速度
		//ModAttModValue(ERA_Speed_Mount, n_speed_delta );
	}
	else
	{
		// 设置人物状态
		UnsetRoleState(ERS_Mount);

		// 设置碰撞盒
		SetAttRecalFlag(ERA_Shape);

		// 设置骑乘速度
		//ModAttModValue(ERA_Speed_Mount, -n_speed_delta );
	}

	//RecalAtt();	
}
//--------------------------------------------------------------------
// 设置/获取骑乘数据
//--------------------------------------------------------------------
VOID Role::set_ride_data( INT64 n64_serial, DWORD dw_data_id, BYTE level,INT speed)
{
	ride_data_.n64_serial = n64_serial;
	ride_data_.dwRideTypeID = dw_data_id;
	ride_data_.bySolidateLevel = level;
	ride_data_.nSpeed = speed;
}
//--------------------------------------------------------------------
// 更新
//--------------------------------------------------------------------
VOID Role::update_ride( )
{
	if( VALID_POINT(get_ride_serial()))
	{
		if(!VALID_POINT(GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride)))
		{
			// 设置人物状态
			UnsetRoleState(ERS_Mount);

			// 设置碰撞盒
			SetAttRecalFlag(ERA_Shape);

			SetAttModValue(ERA_Speed_Mount, 0);

			NET_SIS_cancel_ride sisCancelRide;
			sisCancelRide.dwRoleID = GetID();
			sisCancelRide.dwError		= E_Success;
			SendMessage(&sisCancelRide, sisCancelRide.dw_size );

			set_ride_data(0,0,0,0);
		}
		else
		{
			if( IsInRoleState( ERS_Mount ))
			{
		
			}
			else this->StopMount( );
		}
	}
}
//--------------------------------------------------------------------
// 计算坐骑速度
//--------------------------------------------------------------------
void Role::cal_ride_speed( tagEquip* p_equip )
{
	//if( !VALID_POINT( p_equip )) return;

	//p_equip->equipSpec.dwSpeed = 0;
	//if( VALID_POINT( p_equip->pEquipProto))
	//{
	//	const tagPetProto* pPetProto = AttRes::GetInstance()->GetPetProto( p_equip->pEquipProto->dwPetID );
	//	p_equip->equipSpec.dwSpeed = VALID_POINT( pPetProto ) ? pPetProto->nMountSpeed : 0;
	//}

	//ASSERT( p_equip->equipSpec.byConsolidateLevel <= MAX_RIDE_CONSOLIDATE_LEVEL );
	//if(p_equip->equipSpec.byConsolidateLevel > 0)//强化提升
	//	p_equip->equipSpec.dwSpeed += g_RideUpg[p_equip->equipSpec.byConsolidateLevel - 1].dwIncSpeed;

	//for( INT32 n = 0; n < MAX_RIDEHOLE_NUM; ++n )
	//{//镶嵌的装备提升
	//	DWORD dwGemID = p_equip->equipSpec.dwHoleGemID[ n ];
	//	if( VALID_POINT( dwGemID ) )
	//	{
	//		const tagItemProto* pProto = AttRes::GetInstance()->GetItemProto(dwGemID);
	//		if( VALID_POINT( pProto ) && pProto->eSpecFunc == EIST_RideEquip )
	//			p_equip->equipSpec.dwSpeed += pProto->nSpecFuncVal1;
	//		else 
	//			print_message(_T("[ERROR_CLUE_ON]镶嵌的骑乘装备数据不存在:%d\n\n"),dwGemID );
	//	};
	//}
}
BOOL Role::ride_limit(tagEquip* pRide)
{
	//if(!VALID_POINT(pRide))
	//{
	//	pRide = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	//}

	//if( !VALID_POINT(pRide) ||
	//	!VALID_POINT(pRide->pProtoType) ||
	//	!MEquipIsRide(pRide->pProtoType) )
	//	return TRUE;

	Map* p_map = get_map( );
	if( !VALID_POINT(p_map) ) return TRUE;
	if( !VALID_POINT(p_map->get_map_info( )) )return TRUE;

	if (!p_map->get_map_info()->b_raid)
	{
		return TRUE;
	}
	//if( p_map->get_map_info( )->e_type != EMT_Normal )
	//{
	//	const tagInstance* p_instance = AttRes::GetInstance()->get_instance_proto(p_map->get_map_info( )->dw_id);

	//	if( VALID_POINT(p_instance))
	//	{
	//		if( pRide->equipSpec.byConsolidateLevel < p_instance->byConsolidateLevelMin)  return TRUE;
	//	}
	//	else  return TRUE;
	//}
	
	return FALSE;
}

//--------------------------------------------------------------------
// 装备坐骑
//--------------------------------------------------------------------
DWORD Role::EquipRide(INT64 n64Ride)
{
	// 获得待穿装备
	tagItem *pItem = GetItemMgr().GetBagItem(n64Ride);
	if(!VALID_POINT(pItem))
	{
		return E_Item_NotFound;
	}

	// 判断欲装备物品是否为装备
	if(!MIsEquipment(pItem->dw_data_id))
	{
		return E_Item_NotEquipment;
	}

	if(!VALID_POINT(pItem->pProtoType))
	{
		return E_Item_NotFound;
	}

	// 判断阶数是否足够
	if (pItem->pProtoType->nSpecFuncVal1 > GetRaidMgr().GetStep())
	{
		return E_Item_CanNotAdd;
	}
	// 先下来
	StopMount();

	tagEquip* pOldEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	tagEquip* pNewEquip = (tagEquip*)pItem;
	DWORD dwRet = GetItemMgr().Equip(n64Ride,EEP_Ride);
	if(dwRet == E_Success)
	{
		//ProcEquipEffectPos(pNewEquip, pOldEquip, EICT_Equip, EICT_Bag);
		//if(!pNewEquip->IsBind())
		//{
		//	pNewEquip->SetBind(EBS_Bind);
		//	NET_SIS_bind_change send;
		//	send.n64EquipSerial = pNewEquip->n64_serial;
		//	send.byBind = pNewEquip->byBind;
		//	send.dwConType = pNewEquip->eConType;
		//	SendMessage(&send, send.dw_size);
		//}
		ProcEquipEffect(pNewEquip, pOldEquip, (INT16)EEP_Ride);
	}

	return dwRet;
}

//--------------------------------------------------------------------
// 卸下坐骑
//--------------------------------------------------------------------
DWORD Role::UnEquipRide( )
{
	tagEquip* pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pEquip))
	{
		return E_Item_NotFound;
	}

	StopMount();

	INT16 n16IndexDst = GetItemMgr().GetBagOneFreeSpace();
	if(INVALID_VALUE == n16IndexDst)
	{
		return E_Bag_NotEnoughSpace;
	}

	DWORD dwRet = GetItemMgr().Unequip(pEquip->n64_serial, n16IndexDst);
	if(dwRet == E_Success)
	{
		ProcEquipEffect(NULL, pEquip, (INT16)EEP_Ride);
		//ProcEquipEffectPos(NULL, pEquip, EICT_Equip, EICT_Bag);
	}
	
	return dwRet;
}

VOID Role::DealMountSkill(BOOL bSet)
{
#ifdef USE_RIDE_EX
	INT32 i;
	tagEquip *pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	if(!VALID_POINT(pEquip) || pEquip->equipSpec.byConsolidateLevel < MAX_RIDE_CONSOLIDATE_LEVEL)
		return;

	tagEquipProto *pEquipProto = (tagEquipProto*)pEquip->pEquipProto;
	if(!VALID_POINT(pEquipProto)) return;

	const tagPetProto* pPetProto = AttRes::GetInstance()->GetPetProto( pEquipProto->dwPetID );
	if(!VALID_POINT(pPetProto)) return;

	for( i = 0; i < MOUNT_SKILL_NUM; i++)
	{
		const tagSkillProto* pSkillProto = AttRes::GetInstance()->GetSkillProto(pPetProto->dwMountSkillID[i]);
		if( !VALID_POINT(pSkillProto) ) break;

		// 生成技能
		DWORD dwSkillID = Skill::GetIDFromTypeID(pSkillProto->dwID);
		INT nLevel = Skill::GetLevelFromTypeID(pSkillProto->dwID);
		if( bSet ) {
			Skill *pSkillEx = GetSkill(dwSkillID);
			if(!VALID_POINT(pSkillEx)){
				Skill* pSkill = new Skill(dwSkillID, nLevel, 0, 0, 0, TRUE);
				if(pSkill) AddSkill(pSkill);
			}
		} else if (!bSet) {
			RemoveSkill(dwSkillID);
		}
	}
#else

#endif
}

BOOL Role::AttackProbStopMount( )
{
#ifdef USE_RIDE_EX
	if(m_AttackStopMountProtect) return FALSE;
	INT nProb = get_tool()->tool_rand( ) % 10000;
	return nProb <= m_AttackStopMountProb;
#else
	return FALSE;
#endif
}

INT Role::GetMountSpellTime( )
{
#ifdef USE_RIDE_EX
	tagEquip *pEquip = GetItemMgr().GetEquipBarEquip((INT16)EEP_Ride);
	const tagItemProto *pItemProto = pEquip->pProtoType;
	if(!VALID_POINT(pItemProto)) return 3000;

	BYTE bySolidateLevel = pEquip->equipSpec.byConsolidateLevel;
	return pItemProto->nSpecFuncVal1 + GetMountSpellTimeDelta(bySolidateLevel);
#else 
	return 3000;
#endif
}

