
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//������ϵͳ��Ϣ�������

#include "stdafx.h"
#include "player_session.h"
DWORD PlayerSession::HandleGetFatigueInfo(tag_net_message* pCmd)
{
	NET_SIC_enthrallment_info* p_receive = (NET_SIC_enthrallment_info*)pCmd;

	GetFatigueGuarder().NotifyClient();

	return 0;
}