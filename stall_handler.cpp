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
 *	@file		stall_handler
 *	@date		2010/12/20	initial
 *	@brief		摆摊消息处理
*/

#include "StdAfx.h"
#include "../../common/WorldDefine/stall_protocol.h"
#include "player_session.h"
#include "role.h"
#include "vip_stall.h"

//-----------------------------------------------------------------------------
// 准备摆摊
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallStart(tag_net_message* pCmd)
{
	if(VALID_POINT(m_pRole) && GetFatigueGuarder().GetEarnRate() < 10000)
	{
		GetRole()->SendFatigueGuardInfo(E_FatigueLimit_Stall);
		return 0;
	}

	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_start);
	NET_SIC_stall_start * p_receive = ( NET_SIC_stall_start * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	//gx modify 2013.6.25
	/*if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}*/

	NET_SIS_stall_start send;
	send.dw_error_code = pRole->StartStall();
	send.byMaxIndex = pRole->GetStallMaxIdx( );
	send.byStallLevel = pRole->GetStallModeLevel( );
	SendMessage(&send, send.dw_size);

	if (E_Success == send.dw_error_code)//可以摆摊后，通知下马
	{
		pRole->StopMount();
	}
	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 商品上架
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallSetGoods(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_set_goods);
	NET_SIC_stall_set_goods * p_receive = ( NET_SIC_stall_set_goods * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_stall_set_goods send;
	send.dw_error_code = pRole->SetStallGoods(p_receive->n64_serial, p_receive->n64UnitPrice, p_receive->byIndex);
	SendMessage(&send, send.dw_size);

	// 是否需要向周围玩家广播
	if(pRole->IsSetGoodsFinish() && E_Success == send.dw_error_code && VALID_POINT(pRole->get_map()))
	{
		NET_SIS_stall_set_refresh send;
		send.dwStallRoleID = pRole->GetID();
		send.byIndex = p_receive->byIndex;

		if( VALID_POINT(pRole->get_map()) )
		{
			pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
		}
	}

	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 商品下架
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallUnsetGoods(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_unset_goods);
	NET_SIC_stall_unset_goods * p_receive = ( NET_SIC_stall_unset_goods * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	NET_SIS_stall_unset_goods send;
	send.dw_error_code = pRole->UnsetStallGoods(p_receive->byIndex);
	SendMessage(&send, send.dw_size);

	// 额外处理
	if(pRole->IsSetGoodsFinish() && E_Success == send.dw_error_code)
	{
		// 检查摊位上是否还有商品
		if(pRole->IsNoGoodsInStall())
		{
			// 没有，则收摊
			pRole->SendCloseStall();
		}
		else if(VALID_POINT(pRole->get_map()))
		{
			// 有，则向周围玩家广播
			NET_SIS_stall_unset_refresh send;
			send.dwStallRoleID = pRole->GetID();
			send.byIndex = p_receive->byIndex;

			if( VALID_POINT(pRole->get_map()) )
			{
				pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
			}
		}
	}
	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 设置标题
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallSetTitle(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_set_title);
	NET_SIC_stall_set_title * p_receive = ( NET_SIC_stall_set_title * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//GET_MESSAGE_STRING(strTitle, p_receive->szData, p_receive->dw_size, NET_SIC_stall_set_title, szData);
	INT32 tmpStrSz = ( p_receive->dw_size - FIELD_OFFSET ( NET_SIC_stall_set_title , szData ) ) / sizeof ( TCHAR ) ;   
	if ( tmpStrSz < 0 ) tmpStrSz = 0 ;   
	tstring strTitle ( p_receive->szData , tmpStrSz ) ;  

	NET_SIS_stall_set_title send;
	send.dw_error_code = pRole->SetStallTitle(strTitle.c_str());
	SendMessage(&send, send.dw_size);


	return send.dw_error_code;
}
//-----------------------------------------------------------------------------
// 设置广告
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallSetAdText(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_set_advertisement);
	NET_SIC_stall_set_advertisement * p_receive = ( NET_SIC_stall_set_advertisement * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	//GET_MESSAGE_STRING(strAd, p_receive->szData, p_receive->dw_size, NET_SIC_stall_set_advertisement, szData);
	INT32 tmpStrSz = ( p_receive->dw_size - FIELD_OFFSET ( NET_SIC_stall_set_advertisement , szData ) ) / sizeof ( TCHAR ) ;  
	if ( tmpStrSz < 0 ) tmpStrSz = 0 ;   
	tstring strAd ( p_receive->szData , tmpStrSz ) ;  

	NET_SIS_stall_set_advertisement send;
	send.dw_error_code = pRole->SetStallAd(strAd.c_str());
	SendMessage(&send, send.dw_size);


	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 通知周围玩家，自己开始摆摊
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallSetFinish(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_set_finish);
	NET_SIC_stall_set_finish * p_receive = ( NET_SIC_stall_set_finish * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!VALID_POINT(pRole->get_map()))
	{
		return INVALID_VALUE;
	}

	NET_SIS_stall_set_finish send;
	send.dw_error_code	= pRole->SetStallFinish();
	send.dw_role_id		= pRole->GetID();
	send.byStallLevel	= pRole->GetStallModeLevel();

	if( VALID_POINT(pRole->get_map()) )
	{
		pRole->get_map()->send_big_visible_tile_message(pRole, &send, send.dw_size);
	}

	if( E_Success == send.dw_error_code )
	{
		pRole->GetMoveData().StopMoveForce();
	}
	return send.dw_error_code;
}

//-----------------------------------------------------------------------------
// 收摊
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallClose(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_close);
	NET_SIC_stall_close * p_receive = ( NET_SIC_stall_close * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	if(!VALID_POINT(pRole->get_map()))
	{
		return INVALID_VALUE;
	}

	return pRole->SendCloseStall();
}

//-----------------------------------------------------------------------------
// 获得摊位标题
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallGetTitle(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_get_title);
	NET_SIC_stall_get_title * p_receive = ( NET_SIC_stall_get_title * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	for(INT n = 0;  n < p_receive->nNumber; ++n)
	{
		// 找到摊主
		Role *pStallRole = pMap->find_role(p_receive->dwStallRoleID[n]);
		if(!VALID_POINT(pStallRole))  continue;

		// mwh2011-11-10注意此处: 必须使用临时变量，因为下面会动态计算包大小
		NET_SIS_stall_get_title send;
		send.dwStallRoleID = p_receive->dwStallRoleID[n];
		send.dw_error_code = pStallRole->GetStallTitle(send.szTitle);
		send.dw_size -=  ((STALL_MAX_TITLE_NUM - 1) * sizeof(TCHAR));
		if(E_Success == send.dw_error_code)
			send.dw_size += _tcslen(send.szTitle) * sizeof(TCHAR);

		SendMessage(&send, send.dw_size);
	}

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得摊位广告
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallGetAd(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_get_advertisement);
	NET_SIC_stall_get_advertisement * p_receive = ( NET_SIC_stall_get_advertisement * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 找到摊主
	Role *pStallRole = pMap->find_role(p_receive->dwStallRoleID);
	if(!VALID_POINT(pStallRole))
	{
		return INVALID_VALUE;
	}

	INT nChNumTitle = 0;
	
	NET_SIS_stall_get_advertisement send;
	send.dwStallRoleID = p_receive->dwStallRoleID;
	send.dw_error_code = pStallRole->GetStallAd(send.szAd);

	// 重新计算消息大小
	if(E_Success == send.dw_error_code)
	{
		send.dw_size -= ((STALL_AD_CHAR_MAX - _tcslen(send.szAd) - 1) * sizeof(TCHAR));
	}
	else
	{
		send.dw_size -= ((STALL_AD_CHAR_MAX - 1) * sizeof(TCHAR));
	}

	SendMessage(&send, send.dw_size);

	return E_Success;
}

//-----------------------------------------------------------------------------
// 获得摊位上所有物品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallGet(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_get);
	NET_SIC_stall_get * p_receive = ( NET_SIC_stall_get * ) pCmd ;  	
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 找到摊主
	Role *pStallRole = pMap->find_role(p_receive->dwStallRoleID);
	if(!VALID_POINT(pStallRole))
	{
		return INVALID_VALUE;
	}

	// 判断距离
	if(!pRole->IsInDistance(*pStallRole, MAX_EXCHANGE_DISTANCE))
	{
		return INVALID_VALUE;
	}

	// 为返回消息分配空间
	INT	nSzMsg = pStallRole->CalStallGoodsMemUsed();
	if(0 == nSzMsg)
	{
		// 摊位上没有商品
		return INVALID_VALUE;
	}

	nSzMsg += (sizeof(NET_SIS_stall_get) - 1);
	CREATE_MSG(pSend, nSzMsg, NET_SIS_stall_get);

	DWORD dw_error_code = pStallRole->GetStallGoods(pSend->byData, pSend->byNum, nSzMsg);
	pSend->dw_error_code		= dw_error_code;
	pSend->dwStallRoleID	= p_receive->dwStallRoleID;
	
	// 重新计算大小
	pSend->dw_size = nSzMsg + (sizeof(NET_SIS_stall_get) - 1);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 购买摊位上的商品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallBuy(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_buy);
	NET_SIC_stall_buy * p_receive = ( NET_SIC_stall_buy * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}
	//gx modify 2013.6.25
	/*if(!pRole->get_check_safe_code())
	{
		if(GetBagPsd() != p_receive->dw_safe_code)
		{

			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = FALSE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			return INVALID_VALUE;
		}

		else 
		{
			NET_SIS_code_check_ok send_check;
			send_check.bSuccess = TRUE;
			pRole->SendMessage(&send_check, send_check.dw_size);

			pRole->set_check_safe_code();
		}
	}*/

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 找到摊主
	Role *pStallRole = pMap->find_role(p_receive->dwStallRoleID);
	if(!VALID_POINT(pStallRole))
	{
		return INVALID_VALUE;
	}

	// 判断距离
	if(!pRole->IsInDistance(*pStallRole, MAX_EXCHANGE_DISTANCE))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	INT16 n16RemainNum = 0;
	DWORD dw_error_code = pStallRole->BuyStallGoods(pRole, 
							p_receive->n64_serial, p_receive->n64UnitPrice, p_receive->n16Num, p_receive->byIndex, n16RemainNum);
	
	if(dw_error_code != E_Success)
	{
		NET_SIS_stall_buy send;
		send.dw_error_code = dw_error_code;
		SendMessage(&send, send.dw_size);
	}
	else
	{
		// 检查摊位上是否还有商品
		if(pStallRole->IsNoGoodsInStall())
		{
			// 没有，则收摊
			pStallRole->SendCloseStall();
		}
		else
		{// 有，则向周围广播更新后的商品个数
			NET_SIS_stall_buy_refresh send;
			send.dwStallRoleID = p_receive->dwStallRoleID;
			send.byIndex = p_receive->byIndex;
			send.n16Num = n16RemainNum;
			pMap->send_big_visible_tile_message(pStallRole, &send, send.dw_size);
		}

		//成就
		pStallRole->GetAchievementMgr().UpdateAchievementCriteria(eta_baitan_sell, 1);
	}

	return dw_error_code;
}

//-----------------------------------------------------------------------------
// 获取摊位上指定位置的商品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleRoleStallGetSpec(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_stall_get_special);
	NET_SIC_stall_get_special * p_receive = ( NET_SIC_stall_get_special * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	// 找到摊主
	Role *pStallRole = pMap->find_role(p_receive->dwStallRoleID);
	if(!VALID_POINT(pStallRole))
	{
		return INVALID_VALUE;
	}

	// 判断距离
	if(!pRole->IsInDistance(*pStallRole, MAX_EXCHANGE_DISTANCE))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	// 是否收摊
	if(!pStallRole->IsInRoleState(ERS_Stall))
	{
		return INVALID_VALUE;
	}

	// 为返回消息分配空间
	INT	nSzMsg = (sizeof(NET_SIS_stall_get_special) - 1) + (sizeof(tagMsgStallGoods) - 1 + SIZE_EQUIP);
	CREATE_MSG(pSend, nSzMsg, NET_SIS_stall_get_special);

	DWORD dw_error_code = pStallRole->GetStallSpecGoods(p_receive->byIndex, pSend->byData, nSzMsg);
	pSend->dw_error_code		= dw_error_code;
	pSend->dwStallRoleID	= p_receive->dwStallRoleID;

	// 重新计算大小
	pSend->dw_size = nSzMsg + (sizeof(NET_SIS_stall_get_special) - 1);

	SendMessage(pSend, pSend->dw_size);

	MDEL_MSG(pSend);

	return dw_error_code;
}
//--------------------------------------------------------------------
// 聊天
//--------------------------------------------------------------------
DWORD PlayerSession::HandleStallChat( tag_net_message* pCmd )
{
	//GET_MESSAGE(p, pCmd, NET_SIC_stall_chat );
	NET_SIC_stall_chat * p = ( NET_SIC_stall_chat * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	Role* pStallRole = pMap->find_role( p->dwStallRoleID );
	if( !VALID_POINT(pStallRole ) )
		return INVALID_VALUE;

	// 判断距离
	if(!pRole->IsInDistance(*pStallRole, MAX_EXCHANGE_DISTANCE))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	// 是否收摊
	if(!pStallRole->IsInRoleState(ERS_Stall))
	{
		return INVALID_VALUE;
	}

	p->cMessage[STALL_MESSAGE_CHAR_MAX-1] = (TCHAR)0;
	
	NET_SIS_stall_chat send;
	size_t nLen =  _tcslen(p->cMessage);
	if( nLen > 0 ) get_fast_code()->memory_copy( send.cMessage, p->cMessage, nLen * sizeof(TCHAR) );
	send.cMessage[nLen] = (TCHAR)0;
	send.dwSender = pRole->GetID( );
	send.dwStallRoleID = p->dwStallRoleID;
	send.dw_size -= (STALL_MESSAGE_CHAR_MAX - 1 - nLen) * sizeof(TCHAR);
	pMap->send_big_visible_tile_message( pStallRole, &send, send.dw_size );
	pStallRole->SaveStallChat( send.dwSender, send.cMessage, nLen );
	return 0;
}
//--------------------------------------------------------------------
// 历史聊天记录
//--------------------------------------------------------------------
DWORD PlayerSession::HandleStallHistoryChat( tag_net_message* pCmd )
{
	//GET_MESSAGE(p, pCmd, NET_SIC_stall_history_chat );
	NET_SIC_stall_history_chat * p = ( NET_SIC_stall_history_chat * ) pCmd ;  
	// 获取人物
	Role* pRole = GetRole();
	if(!VALID_POINT(pRole))
	{
		return INVALID_VALUE;
	}

	// 获得map
	Map *pMap = pRole->get_map();
	if(!VALID_POINT(pMap))
	{
		return INVALID_VALUE;
	}

	Role* pStallRole = pMap->find_role( p->dwStallRoleID );
	if( !VALID_POINT(pStallRole ) )
		return INVALID_VALUE;

	// 判断距离
	if(!pRole->IsInDistance(*pStallRole, MAX_EXCHANGE_DISTANCE))
	{
		ASSERT(0);
		return INVALID_VALUE;
	}

	// 是否收摊
	if(!pStallRole->IsInRoleState(ERS_Stall))
	{
		return INVALID_VALUE;
	}

	pStallRole->GetStallHistoryChat( pRole );

	return E_Success;
}
//-----------------------------------------------------------------------------
// 获取所有VIP摊位信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleGetAllVIPStallInfo(tag_net_message* pCmd)
{
	return 0;
}

//-----------------------------------------------------------------------------
// 更新VIP摊位信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleUpdateVIPStallInfo(tag_net_message* pCmd)
{
	return 0;
}

//-----------------------------------------------------------------------------
// 申请VIP摊位
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleApplyVIPStall(tag_net_message* pCmd)
{
	return 0;
}

//-----------------------------------------------------------------------------
// 获取某一个VIP摊位上的物品信息
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleSpecVIPStallGet(tag_net_message* pCmd)
{
	return 0;
}

//-----------------------------------------------------------------------------
// 购买VIP摊位上的商品
//-----------------------------------------------------------------------------
DWORD PlayerSession::HandleBuyVIPStallGoods(tag_net_message* pCmd)
{
	return 0;
}