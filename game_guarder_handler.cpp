
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//反外挂处理过程

#include "stdafx.h"
#include "player_session.h"
#include "game_guarder.h"

#include "../../common/WorldDefine/game_sentinel_protocol.h"

DWORD PlayerSession::HandleGameGuarderMsg(tag_net_message* pCmd)
{
	//GET_MESSAGE(p_receive, pCmd, NET_SIC_game_sentinel);
	NET_SIC_game_sentinel * p_receive = ( NET_SIC_game_sentinel * ) pCmd ; 
	switch (p_receive->chCmd)	
	{
	case 'T':
		g_gameGuarder.Transport(GetSessionID(), p_receive->chData, p_receive->nLen);
		break;
	case 'R':
		//g_gameGuarder.Ret(GetSessionID(), p_receive->chData);
		break;
	default:
		ASSERT(0);
		break;
	}
	return 0;
}
