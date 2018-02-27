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
 *	@brief		活动管理器
*/

#include "StdAfx.h"

#include "../../common/WorldDefine/activity_protocol.h"

#include "activity_mgr.h"
#include "role_mgr.h"
#include "script_mgr.h"
#include "world.h"
#include "db_session.h"
#include "hearSay_helper.h"
#include "SignManager.h"
#include "guild_manager.h"
#include "BattleGround.h"

activity_mgr* activity_mgr::m_pInstance = NULL; 

// 活动数据
activity_fix::activity_fix():ScriptData()
{

}

activity_fix::~activity_fix()
{

}



VOID activity_fix::init(const s_act_info *p_act_info)
{
	p_act_info_ = p_act_info;
	b_start_ = FALSE;
	dw_tick_   = 0;
	time_events_.clear();

	dw_minute_update_count = 0;

	// 取得脚本
	p_script_ = g_ScriptMgr.GetActScript(p_act_info->dw_id);

// 	if(p_act_info->act_time.by_interval != 0)
// 	{
// 		tagDWORDTime	broad_time = p_act_info->act_time.star_time;
// 		broad_time.hour = p_act_info->act_time.by_start_broad_h;
// 		broad_time.min  = p_act_info->act_time.by_start_broad_m;
// 		
// 		// 计算离开始就还有多少interval
// 		DWORD dwTimeLeft = CalcTimeDiff( p_act_info->act_time.star_time, broad_time ) / 60;
// 		INT	  nBroadNum = dwTimeLeft % p_act_info->act_time.by_interval;
// 
// 		for(INT i = 0; i < nBroadNum; ++i)
// 		{
// 			broad_time = IncreaseTime(broad_time, i * p_act_info->act_time.by_interval);
// 			broad_times_.push_back(broad_time);
// 		}
// 	}

	if(VALID_POINT(p_script_))
	{
		p_script_->OnInit(p_act_info->dw_id);
	}
}

// 初始化脚本数据
VOID activity_fix::init_script_data(s_activity_data* p_script_data)
{
	get_fast_code()->memory_copy(m_ScriptData.dwData, p_script_data->data.dwData, sizeof(DWORD)*ESD_Activity);
}

// 活动是否可以开始
BOOL activity_fix::can_start(tagDWORDTime cur_time)
{
	// 已经开始
	if( is_start()) return FALSE;

	cur_time.sec = 0;

	int leftToStart =  CalcTimeDiff(p_act_info_->act_time.star_time, cur_time);
	INT leftToEnd = CalcTimeDiff(p_act_info_->act_time.end_time, cur_time);
	if(leftToStart > 0 || leftToEnd <= 0) return FALSE;

	switch(p_act_info_->e_act_mode)
	{
	case EAM_PERSIST:
		break;
	case EAM_WEEK:
		{
			INT WeekDay = (INT)WhichWeekday(cur_time); 
			WeekDay = 1 << WeekDay ;
			if(!(p_act_info_->act_time.nWeek & WeekDay))
				return FALSE;

			tagDWORDTime perDayTimeStart = cur_time, perDayTimeEnd = cur_time;
			perDayTimeStart.hour = p_act_info_->by_start_h;
			perDayTimeStart.min = p_act_info_->by_start_m;

			perDayTimeEnd.hour = p_act_info_->by_end_h;
			perDayTimeEnd.min = p_act_info_->by_end_m;
	
			int leftToTodayStart = CalcTimeDiff(perDayTimeStart, cur_time);
			int leftToTodayEnd = CalcTimeDiff(perDayTimeEnd, cur_time);
			if(leftToTodayStart > 0 || leftToTodayEnd <= 0) return FALSE;
		}
		break;
	}

	return TRUE;
}

// 是否广播 活动即将开始
BOOL activity_fix::is_start_broad(tagDWORDTime cur_time)
{
	// 已经开始
	if( is_start() || p_act_info_->act_time.by_interval == 0)
		return FALSE;

	cur_time.sec = 0;

	INT leftToStart =  CalcTimeDiff(p_act_info_->act_time.star_time, cur_time);
	INT leftToEnd = CalcTimeDiff(p_act_info_->act_time.end_time, cur_time);
	if(leftToStart > 0 || leftToEnd <= 0) return FALSE;


	switch(p_act_info_->e_act_mode)
	{
		case EAM_PERSIST:
			return FALSE;
			break;
		case EAM_WEEK:
			{
				INT WeekDay = (INT)WhichWeekday(cur_time); 
				WeekDay = 1 << WeekDay ;

				if(!(p_act_info_->act_time.nWeek & WeekDay))
					return FALSE;

				tagDWORDTime perDayTime = cur_time;
				perDayTime.hour = p_act_info_->by_start_h;
				perDayTime.min = p_act_info_->by_start_m;
				int leftToTodayStart = CalcTimeDiff(perDayTime, cur_time);
				if(leftToTodayStart <= 0) return FALSE;


				tagDWORDTime	broad_time = cur_time;
				broad_time.hour = p_act_info_->act_time.by_start_broad_h;
				broad_time.min  = p_act_info_->act_time.by_start_broad_m;
				INT leftElapse = (INT)(CalcTimeDiff(cur_time, broad_time) / 60);
				if((leftElapse || cur_time == broad_time )&& (leftElapse % p_act_info_->act_time.by_interval) == 0)
					return TRUE;
			}
			break;
	}

	return FALSE;
}

// 活动是否结束
BOOL activity_fix::is_finish(tagDWORDTime cur_time)
{
	//没开始
	if( !is_start() ) return FALSE;

	int left =  CalcTimeDiff(p_act_info_->act_time.end_time, cur_time);
	if( left <= 0 ) return TRUE;

	switch(p_act_info_->e_act_mode)
	{
	case EAM_PERSIST:
		return FALSE;
		break;
	case EAM_WEEK:
		{
			tagDWORDTime	finish_time = cur_time;
			finish_time.hour = p_act_info_->by_end_h;
			finish_time.min  = p_act_info_->by_end_m;
			INT leftToFinish = (INT)CalcTimeDiff(finish_time, cur_time);
			if(leftToFinish <= 0) return TRUE;
		}
		break;
	}

	return FALSE;
}

// 是否广播活动即将结束
BOOL activity_fix::is_end_broad(tagDWORDTime cur_time)
{
	//没开始
	if( !is_start() ) return FALSE;

	cur_time.sec = 0;

	switch (p_act_info_->e_act_mode)
	{
	case EAM_PERSIST :
		{
			if(cur_time.year == p_act_info_->act_time.end_time.year &&
				cur_time.month == p_act_info_->act_time.end_time.month &&
				cur_time.day == p_act_info_->act_time.end_time.day &&
				cur_time.hour == p_act_info_->act_time.by_end_broad_h && 
				cur_time.min == p_act_info_->act_time.by_end_broad_m ){
					return TRUE;
			}
		}
		break;
	case EAM_WEEK:
		{
			tagDWORDTime broad_time = cur_time;
			broad_time.hour =  p_act_info_->act_time.by_end_broad_h;
			broad_time.min = p_act_info_->act_time.by_end_broad_m;

			if( cur_time.hour == p_act_info_->act_time.by_end_broad_h && 
				cur_time.min == p_act_info_->act_time.by_end_broad_m )
			{
				return TRUE;
			} else if ( cur_time.hour >= p_act_info_->act_time.by_end_broad_h ||
				cur_time.min > p_act_info_->act_time.by_end_broad_m){
				INT leftElapse = (INT)(CalcTimeDiff(cur_time, broad_time) / 60);
				if( p_act_info_->act_time.by_interval && 
					(leftElapse || (cur_time == broad_time)) && !(leftElapse % p_act_info_->act_time.by_interval))
					return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

// 广播活动状态
VOID activity_fix::broad_state(DWORD activity_id, INT n_state)
{
	if( VALID_POINT(p_script_) ) 
	{
		p_script_->BroadActivityState(activity_id, n_state);
	}
}


// 活动开始
VOID activity_fix::start()
{ 
	b_start_ = TRUE;
	dw_minute_update_count = 0;

	if(p_act_info_->b_sign)
	{
		g_sign_mgr.Init(p_act_info_);
	}

	if (VALID_POINT(p_script_))
		p_script_->OnActStart(p_act_info_->dw_id);

	// 特殊判断攻城战开启
	if (p_act_info_->dw_id == 16)
	{
		g_guild_manager.set_begin_SBK(TRUE);
		g_guild_manager.set_door_open(TRUE);
	}
}

// 活动结束
VOID activity_fix::end() 
{
	b_start_ = FALSE; 
	dw_tick_   = 0;
	dw_minute_update_count = 0;

	if(p_act_info_->b_sign)
	{
		g_sign_mgr.Reset();
		g_sign_mgr.ResetRoleSignData();
	}

	if (VALID_POINT(p_script_))
		p_script_->on_act_end(p_act_info_->dw_id);

	// 存储活动脚本数据
	NET_DB2C_save_activity_data	send;
	send.activity.dw_id = p_act_info_->dw_id;
	get_fast_code()->memory_copy(send.activity.data.dwData, 
								 m_ScriptData.dwData, 
								 sizeof(DWORD)*ESD_Activity);
	g_dbSession.Send(&send, send.dw_size);

	// 特殊判断攻城战关闭
	if (p_act_info_->dw_id == 16)
	{
		g_guild_manager.onEndSBKWar();
		//g_guild_manager.set_begin_SBK(FALSE);
		//g_guild_manager.reset_member_ballot();
		//
		//guild* pGuild = g_guild_manager.get_guild(g_guild_manager.get_SBK_guild());
		//if (VALID_POINT(pGuild))
		//{
		//	//发送公告
		//	TCHAR sz_buff[X_LONG_NAME] = _T("");
		//	tstring stname = pGuild->get_guild_att().str_name;
		//	_tcsncpy(sz_buff, stname.c_str(), stname.size());

		//	HearSayHelper::SendMessage(EHST_GUILDFIRSTKILL,
		//		INVALID_VALUE, g_guild_manager.get_SBK_guild(),  1, INVALID_VALUE, INVALID_VALUE, NULL, FALSE, sz_buff, (stname.length() + 1) * sizeof(TCHAR));

		//}


	}

	BattleGround::get_singleton().end(p_act_info_->dw_id);

}

// 时间触发事件 时间列表
VOID activity_fix::add_event_time(DWORD dw_time)
{
	time_events_.insert(dw_time * TICK_PER_SECOND);
}

// 每分钟更新一次
VOID activity_fix::on_minute(tagDWORDTime cur_time)
{
	if( !is_start() )
	{
		// 任务没有开启，检测它是否能够开启
		if( can_start(cur_time) )
		{
			start();
			broad_state(p_act_info_->dw_id, 1);			// 已经开始
			HearSayHelper::SendMessage(EHST_ACTIVITY, -1, p_act_info_->dw_id, 1);

			return;
		}
		else if(is_start_broad(cur_time))		// 开始前的广播
		{
			broad_state(p_act_info_->dw_id, 0);
			return;
		}
	}
	else
	{
		dw_minute_update_count++;

		if(VALID_POINT(p_script_))
		{
			p_script_->OnTimerMin(p_act_info_->dw_id);
		}

		if( is_finish(cur_time) )
		{
			HearSayHelper::SendMessage(EHST_ACTIVITY, -1, p_act_info_->dw_id, 0);
			end();
			broad_state(p_act_info_->dw_id, 3);			// 已经结束
			return;
		}
		else if(is_end_broad(cur_time))		// 结束前的广播
		{
			broad_state(p_act_info_->dw_id, 2);
			// 特殊判断关闭传送点
			if (p_act_info_->dw_id == 16)
			{
				g_guild_manager.set_door_open(FALSE);
			}
			return;
		}
	}
}

// 每个tick刷新
VOID activity_fix::update()
{
	if(!is_start())
		return;

	if(dw_tick_ % 5 == 0)
	{
		if (VALID_POINT(p_script_))
		{
			p_script_->OnTimer(p_act_info_->dw_id, (dw_tick_ * TICK_TIME) / 1000);
		}
	}

	++dw_tick_;
}

// 保存活动到数据库
VOID activity_fix::save_to_db()
{
	// 存储活动脚本数据
	NET_DB2C_save_activity_data	sendDB;
	sendDB.activity.dw_id = p_act_info_->dw_id;
	get_fast_code()->memory_copy(sendDB.activity.data.dwData, m_ScriptData.dwData, sizeof(DWORD)*ESD_Activity);
	g_dbSession.Send(&sendDB, sendDB.dw_size);
}

// 活动管理
activity_mgr::~activity_mgr()
{
	destroy();
}

activity_mgr* activity_mgr::GetInstance()
{
	if(!VALID_POINT(m_pInstance))
	{
		m_pInstance = new activity_mgr;
	}

	return m_pInstance;
}

VOID activity_mgr::Destory()
{
	SAFE_DELETE(m_pInstance);
}



VOID activity_mgr::destroy()
{
	s_act_info	*p_act_info = (s_act_info*)INVALID_VALUE;
	activity_infos_.reset_iterator();
	while (activity_infos_.find_next(p_act_info))
	{
		SAFE_DELETE(p_act_info);
	}
	activity_infos_.clear();

	activity_fix *p_activity = (activity_fix*)INVALID_VALUE;
	activities_.reset_iterator();
	while (activities_.find_next(p_activity))
	{
		SAFE_DELETE(p_activity);
	}
	activities_.clear();
}

// 读取活动文件
BOOL activity_mgr::read_activity_file()
{
	std::list<tstring>				local_fields;
	std::list<tstring>::iterator	iter;

	if(!p_var_->load(g_world.get_virtual_filesys(), 
					_T("data/Config/table/ActivityInfo.xml"), 
					"id", &local_fields))
	{
		print_message(_T("There is a faild error on reading ActivityInfo.xml"));
		return FALSE;
	}

	for(iter = local_fields.begin(); iter != local_fields.end(); ++iter)
	{
		s_act_info* p_act_info = new s_act_info;
		if(!VALID_POINT(p_act_info))
			return FALSE;

		// 读所有活动属性
		p_act_info->dw_id = p_var_->get_dword(_T("id"), iter->c_str());
		p_act_info->e_act_mode = (e_act_mod)p_var_->get_dword(_T("activitymode"), iter->c_str(), 0);
		p_act_info->act_time.nWeek = (e_week_day)p_var_->get_dword(_T("weekday"), iter->c_str(), 0);
		p_act_info->act_time.by_interval = (BYTE)p_var_->get_dword(_T("broadinterval"), iter->c_str(), 0);
		p_act_info->act_time.star_time.year = (BYTE)p_var_->get_dword(_T("startyear"), iter->c_str(), 0);
		p_act_info->act_time.star_time.month = (BYTE)p_var_->get_dword(_T("startmonth"), iter->c_str(), 0);
		p_act_info->act_time.star_time.day = (BYTE)p_var_->get_dword(_T("startday"), iter->c_str(), 0);
		p_act_info->act_time.star_time.hour= (BYTE)p_var_->get_dword(_T("starthour"), iter->c_str(), 0);
		p_act_info->act_time.star_time.min = (BYTE)p_var_->get_dword(_T("startminute"), iter->c_str(), 0);
		p_act_info->act_time.by_start_broad_h = (BYTE)p_var_->get_dword(_T("startbroadhour"), iter->c_str(), 0);
		p_act_info->act_time.by_start_broad_m = (BYTE)p_var_->get_dword(_T("startbroadminute"), iter->c_str(), 0);
		p_act_info->act_time.end_time.year = (BYTE)p_var_->get_dword(_T("endyear"), iter->c_str(), 0);
		p_act_info->act_time.end_time.month = (BYTE)p_var_->get_dword(_T("endmonth"), iter->c_str(), 0);
		p_act_info->act_time.end_time.day = (BYTE)p_var_->get_dword(_T("endday"), iter->c_str(), 0);
		p_act_info->act_time.end_time.hour = (BYTE)p_var_->get_dword(_T("endhour"), iter->c_str(), 0);
		p_act_info->act_time.end_time.min = (BYTE)p_var_->get_dword(_T("endminute"), iter->c_str(), 0);
		p_act_info->act_time.by_end_broad_h = (BYTE)p_var_->get_dword(_T("endbroadhour"), iter->c_str(), 0);
		p_act_info->act_time.by_end_broad_m = (BYTE)p_var_->get_dword(_T("endbroadminute"), iter->c_str(), 0);
		p_act_info->by_start_h = (BYTE)p_var_->get_dword(_T("startdayhour"), iter->c_str(), 0);
		p_act_info->by_start_m = (BYTE)p_var_->get_dword(_T("startdayminute"), iter->c_str(), 0);
		p_act_info->by_end_h = (BYTE)p_var_->get_dword(_T("enddayhour"), iter->c_str(), 0);
		p_act_info->by_end_m = (BYTE)p_var_->get_dword(_T("enddayminute"), iter->c_str(), 0);
		p_act_info->min_level = (BYTE)p_var_->get_dword(_T("MinLevel"), iter->c_str(), 255);
		p_act_info->max_level = (BYTE)p_var_->get_dword(_T("MaxLevel"), iter->c_str(), 255);
		p_act_info->b_sign = (BOOL)p_var_->get_int(_T("sign"), iter->c_str(), 0);

		LPCTSTR sz_map_name = (LPCTSTR)p_var_->get_string(_T("mapid"), iter->c_str(), _T(""));
		p_act_info->dw_map_id = get_tool()->crc32(sz_map_name);
	
		activity_infos_.add(p_act_info->dw_id, p_act_info);
	}

	return TRUE;
}

// 启动
BOOL activity_mgr::start()
{
	p_var_ = new file_container;
	

	// 读取文件
	if(!read_activity_file())
	{
		SAFE_DELETE(p_var_);
		return FALSE;
	} else SAFE_DELETE(p_var_);

	s_act_info	*p_act_info = (s_act_info*)INVALID_VALUE;
	activity_fix	*p_activity = (activity_fix*)INVALID_VALUE;
	tagDWORDTime cur_time = GetCurrentDWORDTime();

	activity_infos_.reset_iterator();
	while (activity_infos_.find_next(p_act_info))
	{
		p_activity = new activity_fix;
		if(!VALID_POINT(p_activity)) return FALSE;

		activities_.add(p_act_info->dw_id, p_activity);
		p_activity->init(p_act_info);
	}

	return TRUE;
}

// 每分钟更新一次
VOID activity_mgr::on_minute(DWORD cur_time)
{
	activity_fix* p_activity = 0;

	activities_.reset_iterator();
	while( activities_.find_next(p_activity) )
	{
		p_activity->on_minute(cur_time);
	}

}

// 每个tick刷新
VOID activity_mgr::update()
{
	activity_fix* pActivity = 0;

	activities_.reset_iterator();
	while( activities_.find_next(pActivity) )
	{
		pActivity->update();
	}
}

// 是否有活动已经开始
BOOL activity_mgr::any_activity_start()
{
	activity_fix* pActivity = 0;

	activities_.reset_iterator();
	while( activities_.find_next(pActivity) )
	{
		if(pActivity->is_start())
			return TRUE;
	}

	return FALSE;
}

// 保存所有活动到数据库
VOID activity_mgr::save_all_to_db()
{
	activity_fix* pActivity = 0;

	activities_.reset_iterator();
	while( activities_.find_next(pActivity) )
	{
		pActivity->save_to_db();
	}
}

//activity_mgr	g_activityMgr;
