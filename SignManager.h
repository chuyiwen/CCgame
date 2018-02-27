#ifndef SIGNMANAGER
#define SIGNMANAGER

#include "../../common/WorldDefine/activity_define.h"

class SignManager
{
public:
	SignManager(void);
	~SignManager(void);
public:
	VOID	Init(const s_act_info* p_act_info);
	VOID	Reset();

	DWORD	get_act_id() { return dw_act_id; }

	tagDWORDTime	get_begin_time() { return dw_begin_time; }

	BOOL	is_open() { return bOpen; }

	VOID	ResetRoleSignData();
private:
	BOOL	bOpen;

	DWORD	dw_act_id;

	tagDWORDTime	dw_begin_time;
};

extern SignManager g_sign_mgr;

#endif
