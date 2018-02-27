
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//客户端和服务器端间消息处理 -- 百宝袋相关

#include "StdAfx.h"

#include "../common/ServerDefine/role_data_server_define.h"
#include "../../common/WorldDefine/protocol_common_errorcode.h"
#include "player_session.h"
#include "db_session.h"

DWORD PlayerSession::HandleInitBaiBaoRecord(tag_net_message* pCmd)
{
	if( !VALID_POINT(m_pRole) ) return INVALID_VALUE;

	NET_DB2C_load_baobao_log sendDB;
	sendDB.dw_account_id = m_dwAccountID;
	g_dbSession.Send(&sendDB, sendDB.dw_size);

	return E_Success;
}