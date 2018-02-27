#ifndef AUTO_PAIMAI
#define AUTO_PAIMAI

#include "../common/ServerDefine/paimai_server_define.h"

typedef package_map<DWORD, tagAutoPaimai*> MAP_AUTO_PAIMAI;

class auto_paimai
{
public:
	auto_paimai(void);
	~auto_paimai(void);

	VOID Update();

	VOID load_all_auto_paimai(NET_DB2S_load_auto_paimai* p_recv);

	VOID check_is_paimai();

	VOID check_is_paimai(DWORD dw_auto_paimai_id);

	VOID set_is_paimai(DWORD dw_auto_paimai_id, BOOL b_have);

	VOID set_init_ok(BOOL b_ok) { b_init_ok = b_ok; };

	VOID create_auto_paimai(tagAutoPaimai* p);

	VOID update_auto_paimai_to_db(tagAutoPaimai* p);

	VOID reset_auto_paimai_inventory();

	VOID reload_auto_paimai();
private:

	BOOL b_init_ok;

	MAP_AUTO_PAIMAI map_auto_paimai;

	DWORD dw_update_time;
};

extern auto_paimai g_auto_paimai;

#endif
