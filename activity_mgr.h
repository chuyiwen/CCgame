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
 *	@file		activity_mgr
 *	@author		mwh
 *	@date		2011/03/10	initial
 *	@version	0.0.1.0
 *	@brief		�������
*/

#ifndef __ACTIVITY_MGR_H__

#define __ACTIVITY_MGR_H__

#include "../../common/WorldDefine/activity_define.h"
#include "../common/ServerDefine/activity_server_define.h"

class ActScript;

// �����
class activity_fix : public ScriptData<ESD_Activity>
{
public:
	activity_fix();
	~activity_fix();
public:
	VOID init(const s_act_info *p_act_info);
	VOID init_script_data(s_activity_data* p_script_data);
	BOOL can_start(tagDWORDTime cur_time);
	BOOL is_start_broad(tagDWORDTime cur_time);
	BOOL is_end_broad(tagDWORDTime cur_time);
	BOOL is_finish(tagDWORDTime cur_time);
	BOOL is_start() { return b_start_; }
	VOID start();
	VOID end();	
	VOID update();
	VOID on_minute(tagDWORDTime cur_time);
public:
	VOID add_event_time(DWORD dw_time);
	VOID save_to_db();
public:
	DWORD get_activity_id() const { return p_act_info_->dw_id; }
	const ActScript* get_script() const { return p_script_; }
	const s_act_info* get_info() const { return p_act_info_; }
	DWORD get_minute_update_count() { return dw_minute_update_count; }
private:
	VOID broad_state(DWORD activity_id, INT n_state);
private:
	// ��ʼ��־
	BOOL b_start_;
	
	// ������ܼ�tick
	DWORD dw_tick_;	

	// �ÿ���Ӹ��´�������
	DWORD dw_minute_update_count;	

	// ���ʱ�䴥���¼�
	std::set<DWORD>	time_events_;

	// ��㲥ʱ��
	//std::vector<tagDWORDTime> broad_times_;

	// ��ű�
	const ActScript* p_script_;
	// ���̬����
	const s_act_info* p_act_info_;
};


// �������
class activity_mgr
{
public:
	activity_mgr(){}
	~activity_mgr();

	BOOL start();
	VOID update();
	VOID on_minute(DWORD cur_time);
	VOID destroy();

	BOOL any_activity_start();
	VOID save_all_to_db();

	static activity_mgr* GetInstance();
	static VOID	   Destory();

public:
	activity_fix* get_activity(DWORD activity_id)	{ return activities_.find(activity_id); }
private:
	BOOL read_activity_file();

private:
	file_container* p_var_;
	package_map<DWORD,activity_fix*> activities_;
	package_map<DWORD,s_act_info*> activity_infos_;

	static activity_mgr*	m_pInstance; 
};

//extern activity_mgr	g_activityMgr;

#endif //__ACTIVITY_MGR_H__