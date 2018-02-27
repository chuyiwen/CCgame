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
*	@file		quest_mgr
*	@author		mwh
*	@date		2011/10/22	initial
*	@version	0.0.1.0
*	@brief		任务静态管理
*/

#include "stdafx.h"

#include "../../common/WorldDefine/QuestDef.h"

#include "quest_mgr.h"
#include "script_mgr.h"
#include "unit.h"
#include "creature.h"
#include "world.h"

QuestMgr g_questMgr;

// 初始化
BOOL QuestMgr::init()
{
	path_ =	World::p_var->get_string(_T("path"), _T("quest"));
	files_ = World::p_var->get_string(_T("list"), _T("quest"));
	file_circle_quest_ = World::p_var->get_string(_T("loop_random_quest"), _T("quest"));
	file_team_share_circle_quest_ = World::p_var->get_string(_T("team_share_quest"), _T("quest"));
	file_quest_ref_accept_npc_ = World::p_var->get_string(_T("acceptQuest_NPC_relation"), _T("quest"));
	file_quest_ref_complete_npc_ = World::p_var->get_string(_T("completeQuest_NPC_relation"), _T("quest"));

	// 读取失败返回
	if(path_.empty() || !read_all_quest(files_.c_str()))return FALSE;
	//if(!file_circle_quest_.empty()) read_cicle_quest_from_file(file_circle_quest_.c_str());
	//if(!file_team_share_circle_quest_.empty()) read_team_share_cicle_quest_from_file(file_team_share_circle_quest_.c_str());
	
	// 任务->接取NPC关联文件
//	if( !file_quest_ref_accept_npc_.empty() ) 
//		read_npc_ref_accept_quest(file_quest_ref_accept_npc_.c_str());

	// 任务->完成NPC关联文件
//	if( !file_quest_ref_complete_npc_.empty() )
//		read_npc_ref_complete_quest(file_quest_ref_complete_npc_.c_str());

	return TRUE;
}
//	把所有的循环随机任务按分类读取
BOOL QuestMgr::read_cicle_quest_from_file( LPCTSTR szFileName )
{
	if( !VALID_POINT( szFileName ) )
	{
		return FALSE;
	}
	XmlDocument doc;
	if( false == doc.LoadFile(NULL,szFileName) ) 
		return FALSE;

	XmlHandle hDoc( &doc );

	// 是否是任务关系文件
	XmlHandle eleHandle = hDoc.FirstChildElement("loop_random_quest");
	XmlElement *pElem = eleHandle.Element();
	for(; pElem != NULL; pElem = pElem->NextSiblingElement( ) )
	{
		read_one_circle_quest_from_file( pElem, circle_quests_);
	}

	return TRUE;
}

VOID QuestMgr::read_one_circle_quest_from_file( XmlElement* pNode, CIRCLEQUESTVECTOR& v)
{
	if( pNode  == NULL )
		return ;

	const char* pMinLevel = pNode->Attribute("min_level");
	const char* pMaxLevel = pNode->Attribute("max_level");
	const char* pRandNum  = pNode->Attribute("rand_number");
	if( !pMinLevel || !pMaxLevel || !pRandNum )
		return ;

	XmlElement* pSonElem = pNode->FirstChildElement( );
	if( pSonElem == NULL )
		return ;

	circle_quest_elem* p_circle_quest_item = new circle_quest_elem;
	p_circle_quest_item->min_level = (UINT16)atol( pMinLevel );
	p_circle_quest_item->max_level = (UINT16)atol( pMaxLevel );
	p_circle_quest_item->rand_number = (UINT16)atol( pRandNum );
	for( ; pSonElem != NULL; pSonElem = pSonElem->NextSiblingElement( ) )
	{
		const char* pQuestID = pSonElem->Attribute("quest_id");
		if( pQuestID != NULL )
		{
			register UINT16  quest_id = (UINT16)atol( pQuestID );
			if( quest_id )
				p_circle_quest_item->quests.add( quest_id, quest_id );
		}
	}

	v.push_back( p_circle_quest_item );
}

//mwh 2011-09-06
BOOL QuestMgr::read_team_share_cicle_quest_from_file( LPCTSTR szFileName )
{
	if( !VALID_POINT( szFileName ) )
	{
		return FALSE;
	}
	XmlDocument doc;
	if( false == doc.LoadFile(NULL,szFileName) ) 
		return FALSE;

	XmlHandle hDoc( &doc );

	// 是否是任务关系文件
	XmlHandle eleHandle = hDoc.FirstChildElement();
	XmlElement *pElem = eleHandle.Element();
	for(; pElem != NULL; pElem = pElem->NextSiblingElement( ) )
	{
		tagTeamShareConfig* pTeamShareConfig = new tagTeamShareConfig;
		read_one_Circle_from_file(pElem, pTeamShareConfig);
		team_share_circle_.push_back(pTeamShareConfig);
	}

	return TRUE;
}

VOID QuestMgr::read_one_Circle_from_file(XmlElement* pNode, tagTeamShareConfig* pCfg)
{
	if( pNode  == NULL )
		return ;

	const char* pMinLevel = pNode->Attribute("avg_leve_min");
	const char* pMaxLevel = pNode->Attribute("avg_leve_max");
	if( !pMinLevel || !pMaxLevel)
		return ;

	XmlElement* pSonElem = pNode->FirstChildElement( );
	if( pSonElem == NULL )
		return ;

	pCfg->teamavgLevelMin = (UINT16)atol( pMinLevel );
	pCfg->teamavgLevelMax = (UINT16)atol( pMaxLevel );

	//for(; pSonElem != NULL; pSonElem = pSonElem->NextSiblingElement( ) )
	//{
	//	tagTeamShareCirle *pShareCircle = new tagTeamShareCirle;
	//	read_one_team_share_circle_quest_from_file(pSonElem, pShareCircle);
	//	pCfg->circls.push_back(pShareCircle);
	//}
}

VOID QuestMgr::read_one_team_share_circle_quest_from_file( XmlElement* pNode, tagTeamShareCirle* pCircle)
{
	if( pNode  == NULL )
		return ;

	const char* pSerial = pNode->Attribute("circl_serial");
	const char* pRandnumber = pNode->Attribute("rand_number");
	if( !pSerial || !pRandnumber)
		return ;

	XmlElement* pSonElem = pNode->FirstChildElement( );
	if( pSonElem == NULL )
		return ;

	pCircle->serial = (INT16)atol( pSerial );
	pCircle->rand_number = (UINT16)atol( pRandnumber );

	for( ; pSonElem != NULL; pSonElem = pSonElem->NextSiblingElement( ) )
	{
		const char* pQuestID = pSonElem->Attribute("quest_id");
		if( pQuestID != NULL )
		{
			register UINT16  quest_id = (UINT16)atol( pQuestID );
			if( quest_id )
				pCircle->ids.push_back(quest_id);
		}
	}
}

// 根据等级取得循环随机任务列表，并返回最大随机个数
package_map<UINT16,UINT16>* QuestMgr::get_circle_quest( int n_level, UINT16& u16_rand )  
{
	u16_rand = 0;

	for( size_t n = 0; n < circle_quests_.size( ); ++n )
	{
		circle_quest_elem* elem = circle_quests_[n];
		if( elem->min_level <= n_level && elem->max_level >= n_level )
		{
			u16_rand = elem->rand_number;
			return &elem->quests;
		}
	}
	return NULL;
}

const std::vector<tagTeamShareCirle*>* QuestMgr::get_team_share_circle_quest( int n_level )  
{
	for( size_t n = 0; n < team_share_circle_.size( ); ++n )
	{
		tagTeamShareConfig* elem = team_share_circle_[n];
		if( elem->teamavgLevelMin <= n_level && elem->teamavgLevelMax >= n_level )
		{
			return &elem->circls;
		}
	}
	return NULL;
}

// 销毁
VOID QuestMgr::destroy()
{
	// 静态数据
	tagQuestProto* p_proto = NULL;
	QUESTPROTO::map_iter it = quest_protos_.begin();
	while( quest_protos_.find_next(it, p_proto) ) SAFE_DELETE(p_proto);
	quest_protos_.clear();

	// NPC -> 可接任务列表
	package_list<UINT16>* p_list = NULL;
	NPCREFQUEST::map_iter it2 = npc_ref_accept_quest_.begin();
	while( npc_ref_accept_quest_.find_next(it2, p_list) ) SAFE_DELETE(p_list);
	npc_ref_accept_quest_.clear();

	// NPC -> 可完成任务列表
	it2 = npc_ref_complete_quest_.begin();
	while( npc_ref_complete_quest_.find_next(it2, p_list) ) SAFE_DELETE(p_list);
	npc_ref_complete_quest_.clear();

	// 任务 -> 可接取任务NPC
	package_list<DWORD>* p_list2 = NULL;
	QUESTREFNPC::map_iter it3 = quest_ref_accept_npc_.begin();
	while( quest_ref_accept_npc_.find_next(it3, p_list2) ) SAFE_DELETE(p_list2);
	quest_ref_accept_npc_.clear();

	// 任务 -> 可完成任务NPC
	it3 = quest_ref_complete_npc_.begin();
	while( quest_ref_complete_npc_.find_next(it3, p_list2) ) SAFE_DELETE(p_list2);
	quest_ref_complete_npc_.clear();

	// 环随机任务
	for( size_t n=0; n < circle_quests_.size( ); ++n) SAFE_DELETE(circle_quests_[n]);
	circle_quests_.clear( );

	for( size_t n=0; n < team_share_circle_.size( ); ++n) SAFE_DELETE(team_share_circle_[n]);
	team_share_circle_.clear( );
}

// 是否能从此NPC接取该任务
BOOL QuestMgr::can_accept_from_npc(Creature* p_creature, UINT16 u16_quest_id)
{
	if( !VALID_POINT(p_creature) ) return FALSE;

	package_list<UINT16>* p_list = npc_ref_accept_quest_.find(p_creature->GetTypeID());

	if( !VALID_POINT(p_list) || !p_list->is_exist(u16_quest_id) )return FALSE;

	return TRUE;
}

// 是否能从此NPC完成该任务
BOOL QuestMgr::can_complete_from_npc(Creature* p_creature, UINT16 u16_quest_id)
{
	if( !VALID_POINT(p_creature) ) return FALSE;

	package_list<UINT16>* p_list = npc_ref_complete_quest_.find(p_creature->GetTypeID());

	if( !VALID_POINT(p_list) || !p_list->is_exist(u16_quest_id) )return FALSE;

	return TRUE;
}

// 此任务是否需要完成任务的NPC
BOOL QuestMgr::is_need_complete_npc(UINT16 u16_quest_id)
{
	return quest_ref_complete_npc_.is_exist(u16_quest_id);
}

// 此任务是否需要接取任务的NPC
BOOL QuestMgr::is_need_accept_npc(UINT16 u16_quest_id)
{
	return quest_ref_accept_npc_.is_exist(u16_quest_id);
}

// 从指定目录读取任务结构
BOOL QuestMgr::read_all_quest(LPCTSTR sz_path)
{
	list<tstring> list_files;
	file_container* p_var = new file_container;
	p_var->load(g_world.get_virtual_filesys(), sz_path, "quest_id", &list_files);

	// 读取所有任务文件
	list<tstring>::iterator it = list_files.begin();
	for(; it != list_files.end(); ++it)
	{
		tagQuestProto* p_proto = new tagQuestProto;
		ZeroMemory(p_proto, sizeof(tagQuestProto));

		tstring name = path_;  name += _T("\\") + (*it) + _T(".xml");
		DWORD dw_error = read_one_quest_from_file(p_proto, name.c_str());
		if( 1 == dw_error )
		{
			print_message(_T("read quest file error [file: %s]\r\n"), name.c_str());
			SAFE_DELETE(p_proto);
		}
		else if( 2 == dw_error )
		{
			SAFE_DELETE(p_proto);
			continue;
		}
		else 
		{
			quest_protos_.add(p_proto->id, p_proto);
		}
	}

	SAFE_DELETE(p_var);

	return TRUE;
}

// 读取任务静态数据
DWORD QuestMgr::read_one_quest_from_file(tagQuestProto* p_proto, LPCTSTR sz_file)
{
	if ( !VALID_POINT(p_proto) || !VALID_POINT(sz_file) ) return 1;

	// 加载XML
	XmlDocument doc;
	if( !doc.LoadFile(g_world.get_virtual_filesys(), sz_file) ) return 1;

	XmlHandle h_doc(&doc);
	XmlHandle h_root(0);

	// 是否是任务文件
	XmlElement *p_elem = h_doc.FirstChildElement("quest").Element();
	if( !VALID_POINT(p_elem) ) return 2;

	h_root = XmlHandle( p_elem );

	XmlElement* p_rew				= h_root.FirstChild("reward").FirstChild().Element();
	XmlElement* p_property			= h_root.FirstChild("property").FirstChild().Element();
	XmlElement* p_accept_condition	= h_root.FirstChild("accept_conditions").FirstChild().Element();
	XmlElement* p_complete_condition= h_root.FirstChild("complete_conditions").FirstChild().Element();

	XMLNODEMAP node_map;
	read_xml_node(p_property, node_map);
	read_xml_node(p_accept_condition, node_map);
	read_xml_node(p_complete_condition, node_map);
	read_xml_node(p_rew, node_map);

	set_protocol(p_proto, node_map);
	return 0;	
}

// 读取节点属性
BOOL QuestMgr::read_xml_node(XmlElement* p_node, XMLNODEMAP& node_map)
{
	if ( !VALID_POINT(p_node) ) return FALSE;

	XmlAttribute* p_attr = NULL;
	for(; VALID_POINT(p_node); p_node = p_node->NextSiblingElement() )
	{
		p_attr =  p_node->FirstAttribute();

		string first_name;
		if( VALID_POINT(p_attr) ) first_name = p_attr->Name();

		for(; VALID_POINT(p_attr); p_attr = p_attr->Next())
		{
			if( VALID_POINT(p_attr->Name())&& VALID_POINT(p_attr->Value()) )
			{
				if( !VALID_POINT(p_attr->Previous()) )
					node_map.insert(make_pair(first_name, string(p_attr->Value())));
				else
				{
					string tmp = first_name;
					tmp += "-";
					tmp += p_attr->Name();
					node_map.insert( make_pair(tmp, p_attr->Value()));
				}
			}
		}
	}
	return TRUE;
}

// NPC -> 可接任务列表 / 任务 -> 可接取任务NPC
BOOL QuestMgr::read_npc_ref_accept_quest(LPCTSTR sz_file)
{
	// 清空map
	npc_ref_accept_quest_.clear();

	// 读入xml文件
	if( !VALID_POINT(sz_file) ) return FALSE;

	XmlDocument doc;
	if( !doc.LoadFile(g_world.get_virtual_filesys(), sz_file) )  return FALSE;

	XmlHandle h_doc(&doc);
	XmlHandle h_elem = h_doc.FirstChildElement("accept_link").FirstChildElement();
	XmlElement* p_elem = h_elem.Element();
	if ( !VALID_POINT(p_elem) ) return FALSE;

	// 依次读取属性对
	for( ; VALID_POINT(p_elem); p_elem = p_elem->NextSiblingElement() )
	{
		const char* p_npc_id = p_elem->Attribute("npc_model_id");
		const char* p_quest_id = p_elem->Attribute("quest_id");

		if( VALID_POINT(p_npc_id) && VALID_POINT(p_quest_id) )
		{
			DWORD dw_npc_id = atol(p_npc_id);
			UINT16 u16_quest_id = (UINT16)atol(p_quest_id);

			package_list<UINT16>* p_list = npc_ref_accept_quest_.find(dw_npc_id);
			if( !VALID_POINT(p_list) )
			{
				p_list = new package_list<UINT16>;
				npc_ref_accept_quest_.add(dw_npc_id, p_list);
			}

			p_list->push_back(u16_quest_id);

			package_list<DWORD>* p_list2 = quest_ref_accept_npc_.find(u16_quest_id);
			if( !VALID_POINT(p_list2) )
			{
				p_list2 = new package_list<DWORD>;
				quest_ref_accept_npc_.add(u16_quest_id, p_list2);
			}
			p_list2->push_back(dw_npc_id);
		}	
	}

	return TRUE;
}

// // NPC -> 可完成任务列表 / // 任务 -> 可完成任务NPC
BOOL QuestMgr::read_npc_ref_complete_quest(LPCTSTR sz_file)
{
	npc_ref_complete_quest_.clear();

	// 读入xml文件
	if( !VALID_POINT(sz_file) )return FALSE;

	XmlDocument doc;
	if( !doc.LoadFile(g_world.get_virtual_filesys(),sz_file) )  return FALSE;

	XmlHandle h_doc( &doc );
	XmlHandle h_elem = h_doc.FirstChildElement("complete_link").FirstChildElement();
	XmlElement *p_elem = h_elem.Element();
	if( !p_elem ) return FALSE;

	for( ; VALID_POINT(p_elem); p_elem = p_elem->NextSiblingElement() )
	{
		const char* p_npc_id = p_elem->Attribute("npc_model_id");
		const char* p_quest_id = p_elem->Attribute("quest_id");

		if( VALID_POINT(p_npc_id) && VALID_POINT(p_quest_id) )
		{
			DWORD dw_npc_id = atol(p_npc_id);
			UINT16 u16_quest_id = (UINT16)atol(p_quest_id);

			package_list<UINT16>* p_list = npc_ref_complete_quest_.find(dw_npc_id);
			if( !VALID_POINT(p_list) )
			{
				p_list = new package_list<UINT16>;
				npc_ref_complete_quest_.add(dw_npc_id, p_list);
			}
			p_list->push_back(u16_quest_id);

			package_list<DWORD>* p_list2 = quest_ref_complete_npc_.find(u16_quest_id);
			if( !VALID_POINT(p_list2) )
			{
				p_list2 = new package_list<DWORD>;
				quest_ref_complete_npc_.add(u16_quest_id, p_list2);
			}
			p_list2->push_back(dw_npc_id);
		}
	}
	return TRUE;
}

// 初始化任务静态数据
BOOL QuestMgr::set_protocol(tagQuestProto* p_proto, XMLNODEMAP& node_map)
{

	p_proto->quest_flags = 0;
	p_proto->undone_quest_id = 0;
	p_proto->uncomplete_quest_id = 0;

	// property
	set_uint16("quest_id", p_proto->id, node_map);
	set_uint16("pre_quest_id1", p_proto->prev_quest_id, node_map);
	set_uint16("pre_quest_id2", p_proto->prev_quest2_id, node_map );
	set_int32("pre_quest_relation", p_proto->prev_quest_relation, node_map);
	set_uint16("next_quest_id", p_proto->next_quest_id, node_map);
	set_uint16("mutex_quest_id", p_proto->uncomplete_quest_id, node_map);
	set_uint16("undone_quest_id", p_proto->undone_quest_id, node_map);
	set_BOOL("auto_add_quest", p_proto->auto_add_quest, node_map);
	set_BOOL("show_quest_dialog", p_proto->show_quest_dialog, node_map );
	set_BOOL("forbid_delete", p_proto->no_delete, node_map );

	p_proto->accept_flags = 0;
	set_DWORD_flag("player_accept_flag", p_proto->accept_flags, Quest_Accept_Flag_Player, node_map);
	set_DWORD_flag("system_accept_flag", p_proto->accept_flags, Quest_Accept_Flag_System, node_map);
	set_DWORD_flag("item_accept_flag", p_proto->accept_flags, Quest_Accept_Flag_Item, node_map);

	set_DWORD("quest_type", (DWORD&)p_proto->type, node_map);
	set_DWORD("target_mode", (DWORD&)p_proto->target_mode,node_map);
	set_byte("suggested_players", p_proto->suggested_players, node_map);
	set_DWORD("limit_times", p_proto->limit_time, node_map);
	set_byte("accept_times", p_proto->accept_times, node_map);
	set_BOOL("is_periodic", p_proto->period, node_map);
	set_DWORD("periodic_type", (DWORD&)p_proto->period_type, node_map);
	set_DWORD("week", (DWORD&)p_proto->week, node_map);
	set_BOOL("repeatable", p_proto->repeatable, node_map);

	p_proto->loop_rand_quest_flag = FALSE;
	set_BOOL("can_loop", p_proto->loop_rand_quest_flag, node_map);

	p_proto->xuanshang_flag = FALSE;
	set_BOOL("can_xuanshang", p_proto->xuanshang_flag, node_map);

	p_proto->xs_recv_need_money = 0;
	set_uint16("xuanshang_accept_money", p_proto->xs_recv_need_money, node_map);

	p_proto->xs_send_need_money = 0;
	set_uint16("xuanshang_putout_money", p_proto->xs_send_need_money, node_map);

	p_proto->team_rand_share = FALSE;
	set_BOOL("team_rand_share", p_proto->team_rand_share, node_map);


	for(INT i = 0; i < QUEST_ACCEPT_NPC; ++i)
	{
		stringstream ss;
		string str;
		ss << "accpet_npc_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->accept_quest_npc[i], node_map);
	}

	for( INT i = 0; i < QUEST_ACCEPT_NPC; ++i )
	{
		if( !VALID_POINT(p_proto->accept_quest_npc[i]))break;
		DWORD dw_npc = p_proto->accept_quest_npc[i];
		package_list<UINT16>* p_list = npc_ref_accept_quest_.find( dw_npc );
		if( !VALID_POINT(p_list) ) 
		{
			p_list = new package_list<UINT16>;
			npc_ref_accept_quest_.add( dw_npc, p_list );
		}
		p_list->push_back( p_proto->id );

		package_list<DWORD>* p_list2 = quest_ref_accept_npc_.find( p_proto->id );
		if( !VALID_POINT(p_list2) )
		{
			p_list2 = new package_list<DWORD>;
			quest_ref_accept_npc_.add( p_proto->id, p_list2 );
		}
		p_list2->push_back( dw_npc );
	}

	for(INT i = 0; i < QUEST_COMPLETE_NPC; ++i)
	{
		stringstream ss;
		string str;
		ss << "complete_npc_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->complete_quest_npc[i], node_map);
	}

	for( INT i = 0; i < QUEST_COMPLETE_NPC; ++i )
	{
		if( !VALID_POINT(p_proto->complete_quest_npc[i]))break;
		DWORD dw_npc = p_proto->complete_quest_npc[i];
		package_list<UINT16>* p_list = npc_ref_complete_quest_.find( dw_npc );
		if( !VALID_POINT(p_list) ) 
		{
			p_list = new package_list<UINT16>;
			npc_ref_complete_quest_.add( dw_npc, p_list );
		}
		p_list->push_back( p_proto->id );

		package_list<DWORD>* p_list2 = quest_ref_complete_npc_.find( p_proto->id );
		if( !VALID_POINT(p_list2) )
		{
			p_list2 = new package_list<DWORD>;
			quest_ref_complete_npc_.add( p_proto->id, p_list2 );
		}
		p_list2->push_back( dw_npc );
	}

	for( int i = 0; i < QUEST_ITEMS_COUNT ; ++i )
	{
		stringstream ss;
		string str;
		ss << "invest_obj_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->investigate_objects[i], node_map );

		if( p_proto->investigate_objects[i] > 0 )
		{
			p_proto->quest_flags = p_proto->quest_flags | Quest_Flag_Invest;
		}
	}

	set_DWORD("src_item_id", p_proto->src_item, node_map);
	set_int16("src_item_id-value", p_proto->src_item_num, node_map);
	p_proto->src_item_bind = TRUE;
	set_BOOL("src_item_id-bind", p_proto->src_item_bind, node_map);
	set_float("dest_x", p_proto->destination_x, node_map);
	set_float("dest_y", p_proto->destination_y, node_map);
	set_float("dest_z", p_proto->destination_z, node_map);

	//accept-conditions
	set_int32("ac_require_max_level", p_proto->accept_req_max_level, node_map);
	set_int32("ac_require_min_level", p_proto->accept_req_min_level, node_map);

	for(INT i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_reputation" << i + 1 << endl;
		ss >> str;	
		set_int32(str, p_proto->accept_req_rep[i], node_map);
		string tmp = str;
		tmp += "-max";
		set_int32(tmp, p_proto->accept_req_max_rep[i], node_map);
		tmp = str;
		tmp += "-min";
		set_int32(tmp, p_proto->accept_req_min_rep[i], node_map);
	}
	for(INT i = 0; i < QUEST_SKILLS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_skill" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->accept_req_skill[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->accept_req_skill_val[i], node_map);
	}

	for(INT i = 0; i < QUEST_ITEMS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_cant_has_item_id" << i + 1 << endl;
		ss >> str;	
		p_proto->accept_cant_has_item[i] = 0;
		set_DWORD(str, p_proto->accept_cant_has_item[i], node_map);
	}

	set_byte("ac_require_sex", p_proto->sex, node_map);
	set_DWORD("ac_require_class", (DWORD&)p_proto->class_type, node_map);
	set_DWORD("ac_require_map_id", p_proto->accept_req_map, node_map);

	for(INT i = 0; i < QUEST_ITEMS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_item_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->accept_req_item[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int16(tmp, p_proto->accept_req_item_num[i], node_map);
	}
	for(INT i = 0; i < QUEST_ATTS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_att" << i + 1 << endl;
		ss >> str;	
		set_int16(str, p_proto->accept_req_att[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->accept_req_att_val[i], node_map);
	}

	set_BOOL("ac_require_delete_item_flag", p_proto->del_req_item, node_map);

	for(INT i = 0; i < QUEST_TRIGGERS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "ac_require_trigger_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->accept_req_trriger[i], node_map);
	}

	set_BOOL("ac_require_married_flag", p_proto->married, node_map);
	set_byte("ac_require_love_number", p_proto->lover_num, node_map);
	set_int32("ac_require_money_number", p_proto->accept_req_money, node_map );
	set_BOOL("ac_require_delete_money_number", p_proto->accept_del_money, node_map);

	p_proto->bNeedGuild = FALSE;
	set_BOOL("ac_require_need_guild", p_proto->bNeedGuild, node_map);

	p_proto->bBangZhu = FALSE;
	set_BOOL("ac_require_need_bangzhu", p_proto->bBangZhu, node_map);

	p_proto->bNeedMaster = FALSE;
	set_BOOL("ac_require_need_master", p_proto->bNeedMaster, node_map);

	p_proto->bNeedPrentice = FALSE;
	set_BOOL("ac_require_need_prentice", p_proto->bNeedPrentice, node_map);

	p_proto->accept_guild_level_min = 0;
	p_proto->accept_guild_level_max = 0;
	set_uint16("ac_require_guild_level_min", p_proto->accept_guild_level_min, node_map);
	set_uint16("ac_require_guild_level_max", p_proto->accept_guild_level_max, node_map);

	p_proto->accept_vip_level = 0;
	set_uint16("accept_vip_level", p_proto->accept_vip_level, node_map);

	// complete_condition
	for(INT i = 0; i < QUEST_ITEMS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "cc_require_item_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->complete_req_item[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int16(tmp, p_proto->complete_req_item_num[i], node_map);
		if( p_proto->complete_req_item_num[i] > 0 )
		{
			p_proto->quest_flags = p_proto->quest_flags | Quest_Flag_ITEM;
		}
	}
	set_BOOL("cc_require_only_one_item", p_proto->only_one_item, node_map);
	set_BOOL("cc_require_delete_item", p_proto->delete_item, node_map);

	for(INT i = 0; i < QUEST_CREATURES_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "cc_require_creature_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->complete_req_creature[i*3], node_map);
		string tmp = str;
		tmp += "-id2";
		set_DWORD(tmp, p_proto->complete_req_creature[i*3+1], node_map);
		tmp = str;
		tmp += "-id3";
		set_DWORD(tmp, p_proto->complete_req_creature[i*3+2], node_map);
		tmp = str;
		tmp += "-value";
		set_int16(tmp, p_proto->complete_req_creature_num[i], node_map);
		if( p_proto->complete_req_creature_num[i] > 0 )
		{
			p_proto->quest_flags = p_proto->quest_flags | Quest_Flag_KILL;
		}
	}

	set_BOOL("cc_require_crearture_level", p_proto->creature_level, node_map);
	set_BOOL("cc_require_only_one_creature", p_proto->only_one_creature, node_map);

	for(INT i = 0; i < QUEST_NPC_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "cc_require_npc_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->complete_req_npc[i], node_map);
		if( p_proto->complete_req_npc[i] > 0 )
		{
			p_proto->quest_flags = p_proto->quest_flags | Quest_Flag_NPC_TALK;
		}
	}

	set_BOOL("cc_require_only_in_order", p_proto->only_in_order, node_map);
	set_BOOL("cc_require_only_one_npc", p_proto->only_one_npc, node_map);

	for(INT i = 0; i < QUEST_TRIGGERS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "cc_require_trigger_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->complete_req_trigger[i], node_map );
		if( p_proto->complete_req_trigger[i] > 0 )
		{
			p_proto->quest_flags = p_proto->quest_flags | Quest_Flag_Trigger;
		}
	}
	set_BOOL("cc_require_only_one_trigger", p_proto->only_one_trigger, node_map);

	set_int32("cc_require_money", p_proto->complete_req_money, node_map);
	set_BOOL("cc_require_money", p_proto->complete_del_money, node_map);
	set_DWORD("cc_require_map_id", p_proto->complete_req_map, node_map);
	set_float("cc_require_map_x", p_proto->complete_req_map_x, node_map);
	set_float("cc_require_map_y", p_proto->complete_req_map_y, node_map);
	set_float("cc_require_map_z", p_proto->complete_req_map_z, node_map);
	set_float("cc_require_map_radius", p_proto->complete_req_map_radius, node_map);

	set_BOOL("cc_require_map_random", p_proto->coordinate_random, node_map);
	set_BOOL("cc_require_only_one_condition", p_proto->only_one_condition, node_map);

	set_BOOL("cc_require_married_flag", p_proto->complete_req_married, node_map);
	set_byte("cc_require_love_number", p_proto->complete_req_lover_num, node_map);
	set_int32("cc_require_require_level", p_proto->complete_req_level, node_map);

	set_DWORD("cc_require_event_type", (DWORD&)p_proto->event_type, node_map);
	set_DWORD("cc_require_useitemevent_item_id", p_proto->use_item, node_map);
	set_DWORD("cc_require_useskillevent_skill_id", p_proto->use_skill, node_map);

	p_proto->del_vigour = FALSE; p_proto->vigour_val = 0;
	set_BOOL("cc_del_vigour", p_proto->del_vigour, node_map);
	set_int32("cc_vigour_val", p_proto->vigour_val, node_map);

	p_proto->del_banggong = FALSE; p_proto->banggong_val = 0;
	set_BOOL("cc_del_banggong", p_proto->del_banggong, node_map);
	set_int32("cc_banggong_val", p_proto->banggong_val, node_map);

	p_proto->complete_req_bag_password = FALSE;
	set_BOOL("cc_require_bag_password", p_proto->complete_req_bag_password, node_map);
	
	p_proto->complete_need_yuanbao = 0;
	set_int32("cc_need_yuanbao", p_proto->complete_need_yuanbao, node_map);


	// Reward
	set_BOOL("BindMoney",p_proto->bind_money_flag, node_map);
	set_int32("reward_money", p_proto->rew_money, node_map);
	set_int32("reward_xp", p_proto->rew_xp, node_map);
	set_int32("rew_exploit",	p_proto->rew_exploits, node_map);

	p_proto->rew_yuanbao = 0;
	p_proto->rew_prosperity = 0;
	p_proto->rew_contributions = 0;
	p_proto->rew_guild_fund = 0;

	set_int32("rew_prosperity",	p_proto->rew_prosperity, node_map);
	set_int32("rew_guild_fund",	p_proto->rew_guild_fund, node_map);
	set_int32("rew_contributions",	p_proto->rew_contributions, node_map);
	set_int32("rew_yuanbao",	p_proto->rew_yuanbao, node_map);

	for(INT i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_reputation" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_rep[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->rew_rep_val[i], node_map);
	}	

	//贡献
	for( int i = 0; i < QUEST_CONTRIBUTION_COUNT; ++i )
	{
		stringstream ss;
		string str;
		ss << "reward_contribution" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_contribution[i], node_map );
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->rew_contribution_val[i], node_map );
	}

	for(INT i = 0; i < QUEST_REW_ITEM * X_ClASS_TYPE_NUM; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_item_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_item[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int16(tmp, p_proto->rew_item_num[i], node_map);

		tmp = str;
		tmp += "-quality";
		set_int16(tmp, p_proto->rew_item_quality[i], node_map);

		tmp = str;
		tmp += "-role_class";
		set_int16(tmp, (INT16&)p_proto->rew_item_classtype[i], node_map);
		
		//gx add 2013.6.3 区分玩家性别
		tmp = str;
		tmp += "-role_sex";
		p_proto->rew_item_rolesex[i] = (BYTE)QUESTREWARDNOSEX;
		set_byte(tmp, p_proto->rew_item_rolesex[i], node_map);
		//end
		tmp = str;
		tmp += "-bind"; p_proto->rew_item_bind[i] = TRUE;
		set_BOOL(tmp, p_proto->rew_item_bind[i], node_map);
	}

	for(INT i = 0; i < QUEST_REW_ITEM * X_ClASS_TYPE_NUM; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_choice_item_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_choice_item[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int16(tmp, p_proto->rew_choice_item_num[i], node_map);
		
		tmp = str;
		tmp += "-quality";
		set_int16(tmp, p_proto->rew_choice_quality[i], node_map);

		tmp = str;
		tmp += "-role_class";
		set_int16(tmp, (INT16&)p_proto->rew_choice_classtype[i], node_map);

		tmp = str;
		tmp += "-bind"; p_proto->rew_choice_bind[i] = TRUE;
		set_BOOL(tmp, p_proto->rew_choice_bind[i], node_map);
	}

	for(INT i = 0; i < QUEST_ATTS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_att" << i + 1 << endl;
		ss >> str;	
		set_int16(str, p_proto->rew_att[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->rew_att_val[i], node_map);
	}

	for(INT i = 0; i < QUEST_SKILLS_COUNT; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_skill_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_skill[i], node_map);
		string tmp = str;
		tmp += "-value";
		set_int32(tmp, p_proto->rew_skill_val[i], node_map);
	}

	for(INT i = 0; i < QUEST_REW_BUFF; ++i)
	{
		stringstream ss;
		string str;
		ss << "reward_buff_id" << i + 1 << endl;
		ss >> str;	
		set_DWORD(str, p_proto->rew_buff[i], node_map);
	}
	return TRUE;
}

// string -> DWORD
VOID QuestMgr::set_DWORD(const string& str, DWORD& dw, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if( pos != node_map.end() )
	{
		dw = atol(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> DWORD | n_mask
VOID QuestMgr::set_DWORD_flag(const string& str, DWORD& dw, INT32 n_mask, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if( pos != node_map.end() )
	{
		DWORD n = atol(pos->second.c_str());
		if( n != 0 ) dw = dw | n_mask;
		node_map.erase(pos);
	}
}

// string -> UINT16
VOID QuestMgr::set_uint16(const string& str, UINT16& un16, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if( pos != node_map.end() )
	{
		un16 = (UINT16)atoi(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> BYTE
VOID QuestMgr::set_byte(const string& str, BYTE& by, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if ( pos != node_map.end() )
	{
		by = (BYTE)atoi(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> INT16
VOID QuestMgr::set_int16(const string& str, INT16& n16, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if( pos != node_map.end() )
	{
		n16 = (INT16)atoi(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> INT32
VOID QuestMgr::set_int32(const string& str, INT32& n32, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if( pos != node_map.end() )
	{
		n32 = atoi(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> BOOL
VOID QuestMgr::set_BOOL(const string& str, BOOL& b, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if ( pos != node_map.end() )
	{
		b = atol(pos->second.c_str());
		node_map.erase(pos);
	}
}

// string -> Float
void  QuestMgr::set_float(const string& str, FLOAT& f, XMLNODEMAP& node_map)
{
	XMLNODEMAP::iterator pos = node_map.find(str);
	if ( pos != node_map.end() )
	{
		f = (FLOAT)atof(pos->second.c_str());
		node_map.erase(pos);
	}
}
