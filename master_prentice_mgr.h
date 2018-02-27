/*******************************************************************************

	Copyright 2010 by tiankong Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	tiankong Interactive  Co., Ltd.

*******************************************************************************/
#ifndef __MASTER_PRENTICE_MGR_H__
#define __MASTER_PRENTICE_MGR_H__

/**
*	@file		master_prentice_mgr
*	@date		2010/12/03	initial
*	@brief		师徒 需要注意多线程同步问题。
*/

#include "event_mgr.h"

struct s_master_placard;

class master_prentice_mgr : public EventMgr<master_prentice_mgr>
{
	typedef EventMgr<master_prentice_mgr> Super;
	typedef package_safe_list<DWORD> SaveList;
	typedef package_map<DWORD,s_master_placard*> MasterPlacardMap;
	typedef MasterPlacardMap::map_iter MasterPlacardMapIterator;
	typedef package_list<DWORD> MasterRecruit;
	typedef MasterRecruit::list_iter MasterRecruitIterator;

	SaveList		 save_list_;
	MasterRecruit master_recruit_;
	MasterPlacardMap master_placards_;	
public:
	//--------------------------------------------------------------------
	// 主线程调用
	//--------------------------------------------------------------------
	VOID init( );//初始化
	VOID init_placard( const VOID* p_data );//初始化师徒榜数据
	VOID init_recruit( const VOID* p_data, INT iNumber);
	VOID update( );//更新
	VOID destroy( ){};//销毁
	VOID force_save_all( );//关服时强制存数据
public:
	//--------------------------------------------------------------------
	// 在地图线程之上
	//--------------------------------------------------------------------
	VOID make_master( DWORD dw_sender, VOID* p_message); //拜师
	VOID make_master_ex( DWORD dw_sender, VOID* p_message);//师傅回应

	VOID make_prentice( DWORD dw_sender, VOID* p_message);//收徒
	VOID make_prentice_ex( DWORD dw_sender, VOID*  p_message );//徒弟回应

	VOID master_prentice_break( DWORD dw_sender, VOID* p_message );//师傅解除与徒弟的关系
	VOID show_in_master_placard( DWORD dw_sender, VOID* p_message );//徒弟解除与师傅的关系

	VOID get_master_placard(DWORD dw_sender, VOID* p_message );//取得师徒榜数据
	VOID get_master_prentices( DWORD dw_sender, VOID* p_message );//取得师门数据

	VOID role_deleted( DWORD dw_sender, VOID* p_message );//删除角色时

	VOID call_in_master( DWORD dw_sender, VOID* p_message );//徒弟召请师傅
	VOID prentice_call_in(DWORD dw_sender, VOID* p_message );//师傅回应

	VOID master_teach_prentice(DWORD dw_sender,VOID* p_message);//师傅给徒弟传功 gx add 2013.12.06

	VOID join_master_recruit(Role* p_role);
	VOID leave_master_recruit(Role* p_role);
	VOID query_page_master_recruit(Role* p_role, INT n_cur_page);
	VOID say_goodbye_to_master(Role* p_role, BYTE byAck);

	//师徒上下线的互相提醒 gx add 2013.12.13
	VOID send_login_to_fellow(Role *p_role);
	VOID send_logout_to_fellow(Role *p_role);
	//end
public:
	//--------------------------------------------------------------------
	// 只在脚本中使用
	//--------------------------------------------------------------------
	BOOL has_prentice( DWORD dw_role_id );
	BYTE get_prentice_num(DWORD dw_role_id);
	BOOL find_role_in_recruit(DWORD dwRoleID);
	int fill_prentice_list(DWORD dw_role_id, DWORD	(&dw_prenices)[5]);
private:
	//--------------------------------------------------------------------
	// 在主线程调用的借口内部调用
	//--------------------------------------------------------------------
	VOID save_to_db( );//存储数据到数据库
	VOID register_event( );//注册所有事件
protected:
	//--------------------------------------------------------------------
	// 需通过AddEvet来处理
	//--------------------------------------------------------------------
	VOID syn_role_levelup( DWORD dw_sender, VOID* p_message);//玩家升级：出师、加入师徒榜
private:
	//--------------------------------------------------------------------
	// 内部辅助函数
	//--------------------------------------------------------------------
	s_master_placard* get_master_placard( DWORD dw_role_id ){return master_placards_.find( dw_role_id );};
	BOOL add_new_prentice( DWORD dw_master, DWORD dw_prentice);
	VOID prentice_graduate( s_master_placard* p_master_placard, DWORD dw_prentice );
	VOID send_for_each( const s_master_placard* p_master_placard, LPVOID p_message, DWORD dw_size );
	VOID send_members_to_one( Role* p_dest, const s_master_placard* p_master_placard, DWORD dw_except = 0 );
	VOID send_members_to_one_ex( Role* p_dest, const s_master_placard* p_master_placard, DWORD dw_except = 0 );
	VOID prentice_breakwith_master( DWORD dw_prentice );
	VOID master_fire_prentice( DWORD dw_master, DWORD dw_prentice);
	VOID delete_one_master_placard( DWORD dw_master );
	VOID remove_role_in_recruit(DWORD dwRoleID, BOOL bSndDB = FALSE);
	VOID say_goodbye_to_master(Role* p_role, bool b = true);
};

extern master_prentice_mgr g_MasterPrenticeMgr;

#endif //__MASTER_PRENTICE_MGR_H__