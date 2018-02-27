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
*	@brief		帮会职位管理
*/

#include "StdAfx.h"

#include "guild_position.h"
#include "role_mgr.h"
#include "../common/ServerDefine/role_data_server_define.h"
//-------------------------------------------------------------------------------
// 构造&析构
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
//! 判断帮会职位是否已满
//-------------------------------------------------------------------------------
BOOL guild_position::is_position_full(EGuildMemberPos e_guild_position_)
{
	ASSERT(::IsGuildPosValid(e_guild_position_));
	
	return is_full(e_guild_position_);
}

//-------------------------------------------------------------------------------
//! 初始化帮会职位权限
//-------------------------------------------------------------------------------
VOID  guild_position::init_position_power(s_guild *p_att_)
{
	int len = sizeof(p_att_->dwPosPower);
	memset(p_att_->dwPosPower,0,sizeof(p_att_->dwPosPower));

	tagGuildPower power;
	memset(&power,0,sizeof(tagGuildPower));

	
	//帮主权限
	power.bDismissGuild       = 1;

	p_att_->dwPosPower[EGMP_BangZhu]= *(DWORD*)&power;

	//副帮主权限
	memset(&power,0,sizeof(tagGuildPower));

	p_att_->dwPosPower[EGMP_FuBangZhu]= *(DWORD*)&power;

	//普通帮众权限
	memset(&power,0,sizeof(tagGuildPower));

	power.bLeaveGuild         = 1;
	power.bWar				  = 1;
	p_att_->dwPosPower[EGMP_BangZhong]= *(DWORD*)&power;



}

//-------------------------------------------------------------------------------
//! 初始化帮会职位名称
//-------------------------------------------------------------------------------
VOID  guild_position::init_position_name(s_guild *p_att_)
{
	memset(p_att_->szPosName,0,sizeof(p_att_->szPosName));
	TCHAR *pos = &p_att_->szPosName[EGMP_BangZhong][0];
	wsprintf(pos,_T("初级会员"));
	pos = &p_att_->szPosName[EGMP_BangZhu][0];
	wsprintf(pos,_T("会长"));
	pos = &p_att_->szPosName[EGMP_FuBangZhu][0];
	wsprintf(pos,_T("副会长"));
	pos = &p_att_->szPosName[EGMP_OFFICER_1][0];
	wsprintf(pos,_T("议会长老"));
	pos = &p_att_->szPosName[EGMP_OFFICER_2][0];
	wsprintf(pos,_T("议会官员"));
	pos = &p_att_->szPosName[EGMP_OFFICER_3][0];
	wsprintf(pos,_T("高级精英"));
	pos = &p_att_->szPosName[EGMP_OFFICER_4][0];
	wsprintf(pos,_T("中级精英"));
	pos = &p_att_->szPosName[EGMP_OFFICER_5][0];
	wsprintf(pos,_T("初级精英"));
	pos = &p_att_->szPosName[EGMP_OFFICER_6][0];
	wsprintf(pos,_T("高级会员"));
	pos = &p_att_->szPosName[EGMP_OFFICER_7][0];
	wsprintf(pos,_T("中级会员"));
}

//-------------------------------------------------------------------------------
//! 添加帮会成员
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
//! 删除帮会成员
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