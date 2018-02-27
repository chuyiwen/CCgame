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
*	@brief		ʦͽ ��Ҫע����߳�ͬ�����⡣
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
	// ���̵߳���
	//--------------------------------------------------------------------
	VOID init( );//��ʼ��
	VOID init_placard( const VOID* p_data );//��ʼ��ʦͽ������
	VOID init_recruit( const VOID* p_data, INT iNumber);
	VOID update( );//����
	VOID destroy( ){};//����
	VOID force_save_all( );//�ط�ʱǿ�ƴ�����
public:
	//--------------------------------------------------------------------
	// �ڵ�ͼ�߳�֮��
	//--------------------------------------------------------------------
	VOID make_master( DWORD dw_sender, VOID* p_message); //��ʦ
	VOID make_master_ex( DWORD dw_sender, VOID* p_message);//ʦ����Ӧ

	VOID make_prentice( DWORD dw_sender, VOID* p_message);//��ͽ
	VOID make_prentice_ex( DWORD dw_sender, VOID*  p_message );//ͽ�ܻ�Ӧ

	VOID master_prentice_break( DWORD dw_sender, VOID* p_message );//ʦ�������ͽ�ܵĹ�ϵ
	VOID show_in_master_placard( DWORD dw_sender, VOID* p_message );//ͽ�ܽ����ʦ���Ĺ�ϵ

	VOID get_master_placard(DWORD dw_sender, VOID* p_message );//ȡ��ʦͽ������
	VOID get_master_prentices( DWORD dw_sender, VOID* p_message );//ȡ��ʦ������

	VOID role_deleted( DWORD dw_sender, VOID* p_message );//ɾ����ɫʱ

	VOID call_in_master( DWORD dw_sender, VOID* p_message );//ͽ������ʦ��
	VOID prentice_call_in(DWORD dw_sender, VOID* p_message );//ʦ����Ӧ

	VOID master_teach_prentice(DWORD dw_sender,VOID* p_message);//ʦ����ͽ�ܴ��� gx add 2013.12.06

	VOID join_master_recruit(Role* p_role);
	VOID leave_master_recruit(Role* p_role);
	VOID query_page_master_recruit(Role* p_role, INT n_cur_page);
	VOID say_goodbye_to_master(Role* p_role, BYTE byAck);

	//ʦͽ�����ߵĻ������� gx add 2013.12.13
	VOID send_login_to_fellow(Role *p_role);
	VOID send_logout_to_fellow(Role *p_role);
	//end
public:
	//--------------------------------------------------------------------
	// ֻ�ڽű���ʹ��
	//--------------------------------------------------------------------
	BOOL has_prentice( DWORD dw_role_id );
	BYTE get_prentice_num(DWORD dw_role_id);
	BOOL find_role_in_recruit(DWORD dwRoleID);
	int fill_prentice_list(DWORD dw_role_id, DWORD	(&dw_prenices)[5]);
private:
	//--------------------------------------------------------------------
	// �����̵߳��õĽ���ڲ�����
	//--------------------------------------------------------------------
	VOID save_to_db( );//�洢���ݵ����ݿ�
	VOID register_event( );//ע�������¼�
protected:
	//--------------------------------------------------------------------
	// ��ͨ��AddEvet������
	//--------------------------------------------------------------------
	VOID syn_role_levelup( DWORD dw_sender, VOID* p_message);//�����������ʦ������ʦͽ��
private:
	//--------------------------------------------------------------------
	// �ڲ���������
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