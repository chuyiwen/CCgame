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
*	@file		guild_position.h
*	@author		lc
*	@date		2010/10/14	initial
*	@version	0.0.1.0
*	@brief		���ְλ����
*/

#include "StdAfx.h"

#include "guild_position.h"
#include "role_mgr.h"
#include "../common/ServerDefine/role_data_server_define.h"
//-------------------------------------------------------------------------------
// ����&����
//-------------------------------------------------------------------------------
guild_position::guild_position()
{
	//n8_current_bangzhu_num	 = 0;
	//n8_current_fubangzhu_num	 = 0;
	
	
	memset(&n8_current_num, 0, sizeof(n8_current_num));
	n16_current_member_num	 = 0;
	//n16_max_member_num	 = 0;
}

guild_position::~guild_position()
{
}


VOID guild_position::init(BYTE by_guild_level_)
{
	//n16_max_member_num	= MGuildMaxMember(by_guild_level_);
}

//-------------------------------------------------------------------------------
//! �жϰ��ְλ�Ƿ�����
//-------------------------------------------------------------------------------
BOOL guild_position::is_position_full(EGuildMemberPos e_guild_position_)
{
	ASSERT(::IsGuildPosValid(e_guild_position_));
	
	return is_full(e_guild_position_);
}

//-------------------------------------------------------------------------------
//! ��ʼ�����ְλȨ��
//-------------------------------------------------------------------------------
VOID  guild_position::init_position_power(s_guild *p_att_)
{
	int len = sizeof(p_att_->dwPosPower);
	memset(p_att_->dwPosPower,0,sizeof(p_att_->dwPosPower));

	tagGuildPower power;
	memset(&power,0,sizeof(tagGuildPower));

	
	//����Ȩ��
	power.bDismissGuild       = 1;

	p_att_->dwPosPower[EGMP_BangZhu]= *(DWORD*)&power;

	//������Ȩ��
	memset(&power,0,sizeof(tagGuildPower));

	p_att_->dwPosPower[EGMP_FuBangZhu]= *(DWORD*)&power;

	//��ͨ����Ȩ��
	memset(&power,0,sizeof(tagGuildPower));

	power.bLeaveGuild         = 1;
	power.bWar				  = 1;
	p_att_->dwPosPower[EGMP_BangZhong]= *(DWORD*)&power;



}

//-------------------------------------------------------------------------------
//! ��ʼ�����ְλ����
//-------------------------------------------------------------------------------
VOID  guild_position::init_position_name(s_guild *p_att_)
{
	memset(p_att_->szPosName,0,sizeof(p_att_->szPosName));
	TCHAR *pos = &p_att_->szPosName[EGMP_BangZhong][0];
	wsprintf(pos,_T("������Ա"));
	pos = &p_att_->szPosName[EGMP_BangZhu][0];
	wsprintf(pos,_T("�᳤"));
	pos = &p_att_->szPosName[EGMP_FuBangZhu][0];
	wsprintf(pos,_T("���᳤"));
	pos = &p_att_->szPosName[EGMP_OFFICER_1][0];
	wsprintf(pos,_T("��᳤��"));
	pos = &p_att_->szPosName[EGMP_OFFICER_2][0];
	wsprintf(pos,_T("����Ա"));
	pos = &p_att_->szPosName[EGMP_OFFICER_3][0];
	wsprintf(pos,_T("�߼���Ӣ"));
	pos = &p_att_->szPosName[EGMP_OFFICER_4][0];
	wsprintf(pos,_T("�м���Ӣ"));
	pos = &p_att_->szPosName[EGMP_OFFICER_5][0];
	wsprintf(pos,_T("������Ӣ"));
	pos = &p_att_->szPosName[EGMP_OFFICER_6][0];
	wsprintf(pos,_T("�߼���Ա"));
	pos = &p_att_->szPosName[EGMP_OFFICER_7][0];
	wsprintf(pos,_T("�м���Ա"));
}

//-------------------------------------------------------------------------------
//! ��Ӱ���Ա
//-------------------------------------------------------------------------------
DWORD guild_position::add_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_)
{
	
	DWORD dw_error_code = add(e_guild_position_);

	//switch(e_guild_position_)
	//{
	//case EGMP_BangZhu:
	//	dw_error_code = add_putong(dw_role_id_);
	//	break;
	//case EGMP_FuBangZhu:
	//	dw_error_code = add_fubangzhu(dw_role_id_);
	//	break;
	//default:	
	//	dw_error_code = add_putong(dw_role_id_);
	//	break;
	//}

	if(E_Success == dw_error_code)
	{
		++n16_current_member_num;
	}

	return dw_error_code;
}

//DWORD guild_position::add_bangzhu(DWORD dw_role_id_)
//{
//	if(is_bangzhu_full())
//		return TRUE;
//	++n8_current_bangzhu_num;
//	return E_Success;
//}
//
//DWORD guild_position::add_fubangzhu(DWORD dw_role_id_)
//{
//	if(is_fubangzhu_full())
//		return TRUE;
//	++n8_current_fubangzhu_num;
//	return E_Success;
//}

//DWORD guild_position::add_putong(DWORD dw_role_id_)
//{
//	//ASSERT(!is_member_full());
//	return E_Success;
//}

//-------------------------------------------------------------------------------
//! ɾ������Ա
//-------------------------------------------------------------------------------
VOID guild_position::remove_member(DWORD dw_role_id_, EGuildMemberPos e_guild_position_)
{
	ASSERT(n16_current_member_num >= 1);

	BOOL  bRunDefault = FALSE;
	
	remove(e_guild_position_);
	//switch(e_guild_position_)
	//{
	//case EGMP_BangZhong:
	//	remove_putong(dw_role_id_);
	//	break;
	//case EGMP_FuBangZhu:
	//	remove_fubangzhu(dw_role_id_);
	//	break;
	//case EGMP_BangZhu:
	//	remove_bangzhu(dw_role_id_);
	//	break;
	//default:	
	//	remove_putong(dw_role_id_);
	//	break;
	//}

	--n16_current_member_num;
}

//VOID guild_position::remove_bangzhu(DWORD dw_role_id_)
//{
//	--n8_current_bangzhu_num;
//	ASSERT(n8_current_bangzhu_num >= 0);
//}
//
//VOID guild_position::remove_fubangzhu(DWORD dw_role_id_)
//{
//	--n8_current_fubangzhu_num;
//	ASSERT(n8_current_fubangzhu_num >= 0);
//}
//
//VOID guild_position::remove_putong(DWORD dw_role_id_)
//{
//	ASSERT(n16_current_member_num >= 1);
//}

BOOL guild_position::is_full(EGuildMemberPos pos) const
{
	if (pos == EGMP_BangZhong)
		return false;
		
	return n8_current_num[pos] >= 1;

}

DWORD guild_position::add(EGuildMemberPos pos)
{
	if(is_full(pos))
		return TRUE;

	if (pos != EGMP_BangZhong)
	{
		n8_current_num[pos]++;
	}

	return E_Success;
}

VOID guild_position::remove(EGuildMemberPos pos)
{
	if (pos != EGMP_BangZhong)
	{
		--n8_current_num[pos];
	}

	ASSERT(n8_current_num[pos] >= 0);
}