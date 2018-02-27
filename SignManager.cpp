#include "StdAfx.h"
#include "SignManager.h"
#include "role_mgr.h"

SignManager g_sign_mgr;

SignManager::SignManager(void)
{
	Reset();
}

SignManager::~SignManager(void)
{
}

VOID SignManager::Reset()
{
	bOpen = FALSE;
	dw_act_id = INVALID_VALUE;
	ZeroMemory(&dw_begin_time, sizeof(dw_begin_time));
}

VOID SignManager::Init(const s_act_info* p_act_info)
{
	if(!VALID_POINT(p_act_info))
		return;

	if(bOpen)
		return;

	dw_act_id = p_act_info->dw_id;

	dw_begin_time.year = p_act_info->act_time.star_time.year;
	dw_begin_time.month = p_act_info->act_time.star_time.month;
	dw_begin_time.day = p_act_info->act_time.star_time.day;

	bOpen = TRUE;
}

VOID SignManager::ResetRoleSignData()
{
	g_roleMgr.reset_sign_data();
}
