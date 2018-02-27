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
*	@file		map.h
*	@author		lc
*	@date		2011/03/14	initial
*	@version	0.0.1.0
*	@brief		��ͼ
*/


#ifndef MAP
#define MAP

#include "../../common/WorldDefine/time.h"
#include "../../common/WorldDefine/map_protocol.h"
#include "../../common/WorldDefine/combat_protocol.h"
#include "../../common/WorldDefine/ItemDefine.h"
#include "../../common/WorldDefine/group_define.h"
#include "../../common/WorldDefine/WeatherProtocol.h"
#include "group_mgr.h"
#include "script_mgr.h"

class PlayerSession;
class Unit;
class Role;
class Creature;
class Pet;
class Map;
class Shop;
class MapScript;
class NPCTeamMgr;
class guild_chamber;
class guild;
class c_sparse_graph;


struct tagItem;
struct tagEquip;
struct pathNode;
enum   ECreatureSpawnedType;


//-------------------------------------------------------------------------------
//! ��ɫվ��λ�õ������С�߶�
//-------------------------------------------------------------------------------
const FLOAT GET_MAX_HEIGHT_Y = 10000000.0f;
const FLOAT GET_MIN_HEIGHT_Y = -10000000.0f;

const INT GROUND_ITEM_PICK_UP_TIME = 10 * TICK_PER_SECOND; 
const INT GROUND_ITEM_DISAPPEAR_TIME = 40 * TICK_PER_SECOND; 

//--------------------------------------------------------------------------------
//! ��������
//--------------------------------------------------------------------------------
enum E_map_instance_grade
{
	EMIG_NOLIMIT	=	0,				
	EMIG_512		=	1,				
	EMIG_1024		=	2,				
	EMIG_2048		=	3,			
	EMIG_3072		=	4,				

	EMIG_END		=	5,
};

const INT INSTANCE_POWER[EMIG_END] = {0, 1, 3, 6, 9};	//! ����Ȩ��

//--------------------------------------------------------------------------------
//�� ��ͼ����������
//--------------------------------------------------------------------------------
struct tag_way_point_info
{
	Vector3		v_pos;
	Vector3		v_range;
	DWORD		dw_time;			// ������ͣ��ʱ�� 
	FLOAT		f_yaw;				// ����, ��λ�Ƕ�.

public:
	tag_way_point_info(DWORD)
	{
		v_pos.x = v_pos.y = v_pos.z = -1.0;dw_time = 0;
	}
	tag_way_point_info()
	{

	}
};

struct tag_map_way_point_info_list
{
	DWORD							dw_id;					// ������ID
	mutable package_list<tag_way_point_info>	list;		// �������б�

	tag_map_way_point_info_list() { dw_id = INVALID_VALUE; }
};


//--------------------------------------------------------------------------------
//! ��ͼ����������
//--------------------------------------------------------------------------------
struct tag_map_trigger_info
{
	DWORD 				dw_att_id;					//! ��ͼ�༭��id
	EMapTrigger			e_type;						//����ͼ����
	Vector3				boxPos;						//! ��Χ��
	Vector3				boxSize;					//����Χ��С
	DWORD				dw_map_id;					//����������Ŀ���ͼID
	DWORD				dw_way_point_id;			//����������Ŀ���ͼ������ID
	DWORD				dw_param;					//������
	DWORD				dw_quest_id;				//������ı��

	tag_map_trigger_info()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMT_Null;
		dw_map_id = INVALID_VALUE;
		dw_way_point_id = INVALID_VALUE;
		dw_param = INVALID_VALUE;
		dw_quest_id = INVALID_VALUE;
	}
	~tag_map_trigger_info()
	{
	}
};
//--------------------------------------------------------------------------------
//! ��ͼ����������
// gx add 2013.4.17
//--------------------------------------------------------------------------------
struct tag_map_trigger_info_extra
{
	DWORD 				dw_att_id;					//! ��ͼ�༭��id
	EMapTrigger			e_type;						//����ͼ����
	FLOAT				f_height;					//���߶�
	DWORD				dw_map_id;					//����������Ŀ���ͼID
	DWORD				dw_way_point_id;			//����������Ŀ���ͼ������ID
	DWORD				dw_param;					//������
	DWORD				dw_quest_id;				//������ı��

	tag_map_trigger_info_extra()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMT_Null;
		f_height = 0.0f;
		dw_map_id = INVALID_VALUE;
		dw_way_point_id = INVALID_VALUE;
		dw_param = INVALID_VALUE;
		dw_quest_id = INVALID_VALUE;
	}
	~tag_map_trigger_info_extra()
	{
	}
};
//end 
//--------------------------------------------------------------------------------
//! ��ͼ��������
//--------------------------------------------------------------------------------
struct tag_map_area_info
{
	DWORD 				dw_att_id;			//! ��ͼ�༭��id
	EMapArea			e_type;				//! ��������
	//HRGN				h_polygon;			//! ����ξ��
	//AABBox				box;				//! �����Χ��
	//FLOAT				f_height;			//! �߶�
	Vector3				boxPos;						//! ��Χ��
	Vector3				boxSize;					//����Χ��С

	tag_map_area_info()
	{
		dw_att_id = INVALID_VALUE;
		e_type = EMA_Null;
		//f_height = 0.0f;
	}
	~tag_map_area_info()
	{
		//DeleteObject(h_polygon);
	}
};

//--------------------------------------------------------------------------------
//! ��ͼ���NPC
//--------------------------------------------------------------------------------
struct tagMapCreatureInfo
{
	DWORD 					dw_att_id;			//! ��ͼ�༭��id
	DWORD 					dw_type_id;			//! ����id
	Vector3					v_pos;				//������
	FLOAT 					f_yaw;				//! ����
	tag_map_way_point_info_list*	list_patrol;			//! Ѳ�ߵ������б�
	BOOL					b_collide;			//! �Ƿ���ײ

	tagMapCreatureInfo() { ZeroMemory(this, sizeof(*this)); }
};
//--------------------------------------------------------------------------------
//! ��ͼˢ�ֵ���Ϣ
//- gx add 2013.4.16
//--------------------------------------------------------------------------------
struct tagMapMonsterInfo
{
	DWORD 					dw_type_id;			//! ˢ�ֵ�id
	Vector3					v_pos;				//������
	FLOAT 					f_yaw;				//! ����
	tagMapMonsterInfo() { ZeroMemory(this, sizeof(*this)); }
};
//gx end

//--------------------------------------------------------------------------------
//! ��ͼˢ�ֵ�����
//--------------------------------------------------------------------------------
struct tag_map_spawn_point_info
{
	DWORD				dw_att_id;				//! ��ͼ�༭��id
	DWORD				dw_spawn_point_id;		//! ˢ�ֵ�id
	DWORD				dw_group_id;			//! ������id
	INT					n_level_increase;		//! ����ˢ�ֵȼ�����
	BOOL				b_collide;				//���Ƿ���ײ
	Vector3				v_pos;					//! ����
	FLOAT				f_angle;				//! ����
	//BOOL				b_hasUnit;				//! �Ƿ�ˢ�˹�
	tag_map_way_point_info_list*	list_patrol;//! Ѳ�ߵ������б�

	tag_map_spawn_point_info() { ZeroMemory(this, sizeof(*this)); }
};

// ˢ�ֵ����б�
struct tag_map_spawn_point_list
{
	DWORD							dw_id;						//��id
	mutable package_map<DWORD,	tag_map_spawn_point_info*> list;//ˢ�ֵ�
};


//--------------------------------------------------------------------------------
//! ��ͼ������ 
//--------------------------------------------------------------------------------
struct tag_map_door_info
{
	DWORD 					dw_att_id;			//! ��ͼ�༭��id
	DWORD 					dw_type_id;			//! ������id
	Vector3					v_pos;				//! ����
	FLOAT 					f_yaw;				//! ����
	BOOL                    b_init_open_state;     //! �ų�ʼ״̬��TRUE�򿪣�FALSE�ر�		

	tag_map_door_info() { ZeroMemory(this, sizeof(*this)); }
};


//--------------------------------------------------------------------------------
//! ��ͼ��̬����
//--------------------------------------------------------------------------------
struct tag_map_info
{
	typedef package_map<DWORD, tag_map_way_point_info_list*>	MAP_WAY_POINT_LIST;		// ����������
	typedef package_map<DWORD, tag_map_trigger_info*>			MAP_TRIGGER;			// ����������
	typedef package_map<DWORD, tag_map_area_info*>				MAP_AREA;				// ��������
	typedef package_map<DWORD, tagMapCreatureInfo*>				MAP_CREATURE_INFO;		// ��������
	typedef package_map<DWORD, tag_map_spawn_point_info*>		MAP_SPAWN_POINT;		// ����ˢ�ֵ�
	typedef package_map<DWORD, tagMapTriggerEffect*>			MAP_TRIGGER_EFFECT;		// ������Ч
	typedef package_map<DWORD, tag_map_door_info*>					MAP_DOOR_INFO;		    // ������
	typedef package_map<DWORD, tag_map_spawn_point_list*>		MAP_SPAWN_POINT_LIST;	// ����ˢ�ֵ���
	//gx add 2013.4.16
	typedef package_map<DWORD, tagMapMonsterInfo*>				MAP_MONSTER_INFO;		// �������� ���ڼ�¼ˢ�ֵ�
	//end
	//gx add 2013.4.17
	typedef package_map<DWORD, tag_map_trigger_info_extra*>		MAP_TRIGGER_EXTRA;			// ����������
	//end
	DWORD					dw_id;							// ��ͼID
	TCHAR					sz_map_name[X_SHORT_NAME];	// ��ͼ�ļ�����

	EMapType				e_type;							// ��ͼ����
	ENormalMapType			e_normal_map_type;				// ��ͨ��ͼ������
	E_map_instance_grade	e_instance_grade;					// ��������
	
	INT						n_width;							// ����
	INT						n_height;						// ���
	INT						n_visit_distance;						// ���Ӿ���
	DWORD					dw_weather_id;					// ��Ӧ����������ID
	Vector3					v_reborn_positon;						// �����
	DWORD					dw_out_map_id;						// �ӳ����д��ͳ��ĵ�ͼID
	Vector3					v_out_map_position;						// �ӳ����д��ͳ��ĵ�ͼ����
	INT						n_act_id;							// �ID 
	BOOL					b_raid;		
	BOOL					b_AddCooldownTime;
	INT						n_weather_change_tick;


	mutable std::vector<DWORD>		vec_weahter_ids;		// ��������
	mutable MAP_WAY_POINT_LIST		map_way_point_list;		// ����������
	mutable MAP_TRIGGER				map_trigger;			// ����������
	//gx add 2013.4.17
	mutable MAP_TRIGGER_EXTRA		map_trigger_extra;		//����������
	//end
	mutable MAP_AREA				map_safe_area;			// ��ȫ��
	mutable MAP_AREA				map_pvp_area;			// PVP��
	mutable MAP_AREA				map_stall_area;			// ��̯��
	mutable MAP_AREA				map_prison_area;		// ������
	mutable MAP_AREA				map_script_area;		// �ű�����
	mutable MAP_AREA				map_common_area;		// ��ͨ��
	mutable MAP_AREA				map_real_safe_area;		// ���԰�ȫ��
	mutable MAP_AREA				map_guild_battle;		// ���ս��
	mutable MAP_AREA				map_no_punish_area;		// �޳ͷ���
	mutable MAP_AREA				map_hang_area;			// �һ���
	mutable MAP_AREA				map_KongFu_area;		// ������
	mutable MAP_AREA				map_Comprehend_area;	// ������
	mutable MAP_AREA				map_Dancing_area;		// ������
	mutable MAP_CREATURE_INFO		map_creature_info;		// ��������
	mutable MAP_MONSTER_INFO		map_monster_info;		// �������� gx add 2013.4.16
	mutable MAP_SPAWN_POINT			map_spawn_point;		// ����ˢ�ֵ�
	mutable MAP_TRIGGER_EFFECT		map_trigger_effect;		// ������Ч
	mutable MAP_DOOR_INFO		    map_door_info;			// ������
	mutable MAP_SPAWN_POINT_LIST	map_spawn_point_list;	// ����ˢ�ֵ���
	mutable std::vector<int>		map_block_data;			// ��ײ��Ϣ


	DWORD GetMemorySize()
	{
		DWORD dwMemory = 0;
		dwMemory += sizeof(dw_id);
		dwMemory += sizeof(sz_map_name);
		dwMemory += sizeof(e_type);
		dwMemory += sizeof(e_normal_map_type);
		dwMemory += sizeof(e_instance_grade);
		dwMemory += sizeof(n_width);
		dwMemory += sizeof(n_height);
		dwMemory += sizeof(n_visit_distance);
		dwMemory += sizeof(dw_weather_id);
		dwMemory += sizeof(v_reborn_positon);
		dwMemory += sizeof(dw_out_map_id);
		dwMemory += sizeof(v_out_map_position);
		dwMemory += sizeof(n_act_id);

		dwMemory += (map_way_point_list.size() * sizeof(tag_map_way_point_info_list));
		dwMemory += (map_trigger.size() * sizeof(tag_map_trigger_info));
		dwMemory += (map_trigger_extra.size() * sizeof(tag_map_trigger_info_extra));//gx add 2013.4.17

		dwMemory += (map_safe_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_pvp_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_stall_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_prison_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_script_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_common_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_real_safe_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_guild_battle.size() * sizeof(tag_map_area_info));
		dwMemory += (map_no_punish_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_hang_area.size() * sizeof(tag_map_area_info));
		dwMemory += (map_creature_info.size() * sizeof(tagMapCreatureInfo));//gx modify
		dwMemory += (map_monster_info.size() * sizeof(tagMapMonsterInfo));//gx add 2013.4.16
		dwMemory += (map_spawn_point.size() * sizeof(tag_map_spawn_point_info));
		dwMemory += (map_trigger_effect.size() * sizeof(tagMapTriggerEffect));
		dwMemory += (map_door_info.size() * sizeof(tag_map_door_info));
		return dwMemory;
	}

};

//------------------------------------------------------------------------
//! ��ͼ����id������
//------------------------------------------------------------------------
class ground_item_id_builder
{
public:
	ground_item_id_builder():n64_last_valid(1){}
	INT64 builder_valid_id()
	{
		return n64_last_valid++;
	}
private:
	INT64	n64_last_valid;
};

enum E_ground_item_update
{
	EGIU_null				= -1,
	EGIU_remove				= 0,	// �Ƴ�
	EGIU_synchronize		= 1,	// ͬ��
};

//------------------------------------------------------------------------
//! ������Ʒ��������
//------------------------------------------------------------------------
struct tagSiceData
{
	BYTE byNeedSice;
	BYTE byNumber;
	DWORD dw_role_id[MAX_TEAM_NUM];
	BYTE byGreedSice[MAX_TEAM_NUM];
	BYTE byRequireSice[MAX_TEAM_NUM];
public:	
	tagSiceData( )
	{ 
		ZeroMemory(this,sizeof(*this));
	}

	VOID SetSiceTotal( BYTE byTotal ){ byNeedSice = byTotal;}
	VOID AddGreedSice( DWORD dwRole, BYTE byScore )
	{
		INT index = GetIndex( dwRole );
		if( index < 0 && byNumber >= MAX_TEAM_NUM) return; 
		if( index < 0 ) index = byNumber++;
		dw_role_id[index] = dwRole;
		byGreedSice[index] =  byScore;
	}

	VOID AddRequireSice( DWORD dwRole, BYTE byScore )
	{
		INT index = GetIndex( dwRole );
		if( index < 0 && byNumber >= MAX_TEAM_NUM) return; 
		if( index < 0 ) index = byNumber++;
		dw_role_id[index] = dwRole;
		byRequireSice[index] =  byScore;
	}


	INT GetIndex( DWORD dwRole )
	{
		INT index = -1;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] == dwRole )
			{
				index = n; break;
			}
		}
		return index;
	}

	DWORD GetGreedMax( ) 
	{
		INT index = -1; BYTE byScore = 0;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] && byGreedSice[n] > byScore ) 
			{
				byScore = byGreedSice[n];
				index = n;
			}
		}

		return index >= 0 ? dw_role_id[index] : INVALID_VALUE;	
	}

	DWORD GetRequireMax( BYTE& byMaxRequireSice) 
	{
		INT index = -1; BYTE byScore = 0;
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] && byRequireSice[n] > byScore )
			{
				byScore = byRequireSice[n];
				index = n;
			}
		}
		byMaxRequireSice = byScore;
		return index >= 0 ? dw_role_id[index] : INVALID_VALUE;	
	}

	BYTE GetScore( DWORD dwRole, BYTE& byType) const
	{
		for( DWORD n = 0; n < byNumber; ++n)
		{
			if( dw_role_id[n] == dwRole )
			{
				if( byRequireSice[n] )
				{
					byType = EPSM_Require;
					return  byRequireSice[n];
				}
				if( byGreedSice[n] )
				{
					byType = EPSM_Greed;
					return  byGreedSice[n];
				}
			}
		}
		byType = EPSM_Null;
		return 0;
	}


	BOOL IsSiceFinishi( ) const {return byNumber == byNeedSice;}

	VOID SetForceSiceFinishi( ){ byNeedSice = byNumber; }
};

//------------------------------------------------------------------------
// ���������ϵ���Ʒ�ṹ
//------------------------------------------------------------------------
struct tag_ground_item
{
	INT64					n64_id;				// ����ID
	DWORD					dw_type_id;			// ����ID
	INT						n_num;				// �������Ǯ

	tagItem*				p_item;				// ��Ʒָ��
	DWORD					dw_put_down_unit_id;	// ������Ʒ��UnitID
	DWORD					dw_owner_id;			// ������ɫ
	DWORD					dw_team_id;			// ��������
	DWORD					dw_group_id;			// �����Ŷ�ID
	INT						n_tick_count_down;		// ������Ʒ��tick����ʱ
	Vector3					v_pos;				// ��Ʒ����

	EPickMode				e_pick_mode;
	EGTAssignState			e_assign_state;
	INT						n_assign_tick_down;
	tagSiceData				st_sice_data;
	DWORD			dw_boss_id;



	tag_ground_item(	INT64 n64_id_, DWORD dw_type_id_, INT n_num_, tagItem* p_item_, 
					Vector3 v_pos_, DWORD dw_owner_id_, DWORD dw_team_id_, DWORD dw_group_id_, DWORD dw_put_down_unit_id_,
					EPickMode e_pick_mode_ = EPUM_Free, EGTAssignState e_assign_state_ = EGTAS_Null, INT n_tick_down_ = 0, DWORD dw_boss_id_ = 0)
	{
		n64_id			=	n64_id_;
		dw_type_id		=	dw_type_id_;
		n_num			=	n_num_;

		p_item			=	p_item_;
		v_pos			=	v_pos_;
		dw_owner_id		=	dw_owner_id_;
		dw_team_id		=	dw_team_id_;
		dw_group_id		=	dw_group_id_;
		dw_put_down_unit_id	=	dw_put_down_unit_id_;
		n_tick_count_down	=	0;

		e_pick_mode = e_pick_mode_;
		e_assign_state = e_assign_state_;
		n_assign_tick_down = n_tick_down_;

		dw_boss_id = dw_boss_id_;
	}


	// ������£�����TRUEʱɾ������
	BOOL update()
	{
		E_ground_item_update e_update = EGIU_null;
		// ����ʱ��
		++n_tick_count_down;

		if( n_assign_tick_down > 0 && e_assign_state != EGTAS_WaitForUmpirage )
		{//��Ҫ����
			--n_assign_tick_down;
			if( !n_assign_tick_down && e_assign_state != EGTAS_AssignFinish && e_assign_state != EGTAS_Null)
			{//�����˻�û������
				if( e_pick_mode == EPUM_Leader )
				{ //�ӳ�û�з���,�������ʰȡ
					e_pick_mode = EPUM_Free; 
					e_assign_state = EGTAS_Null;
					dw_team_id	= INVALID_VALUE;
					dw_owner_id	= INVALID_VALUE;
				}

				else if( e_pick_mode == EPUM_Sice )
				{ 
					e_assign_state =  EGTAS_AssignFinish; 
					st_sice_data.SetForceSiceFinishi( ); 

					tagAssignFinish send;
					send.dwTeamID = dw_team_id;
					send.n64GroundID = n64_id;
					g_groupMgr.AddEvent(0, EVT_TeamSiceFinish, sizeof(tagAssignFinish), &send);
				}
			}
		}

		// 0.5��������ʰȡ
		if (GROUND_ITEM_PICK_UP_TIME == n_tick_count_down /*&& !VALID_VALUE(dw_team_id)*/)
		{
			/*e_update =	VALID_VALUE(dw_owner_id) ? EGIU_null : EGIU_synchronize;
			dw_owner_id	= INVALID_VALUE;
			dw_team_id	= INVALID_VALUE;*/
	
			dw_team_id	= INVALID_VALUE;
			dw_owner_id	= INVALID_VALUE;
			e_update = EGIU_synchronize;
		}
		// 1������ʧ
		else if (n_tick_count_down >= GROUND_ITEM_DISAPPEAR_TIME)
		{
			e_update = EGIU_remove;
		}
		return e_update;
	}

	// ����
	VOID destroy_item()
	{
		if (VALID_POINT(p_item))
		{
			::Destroy(p_item);
		}
	}

	// �Ƿ���Ч
	BOOL is_valid()
	{
		if (dw_type_id == TYPE_ID_MONEY && !VALID_POINT(p_item))
		{
			return TRUE;
		}
		else if (dw_type_id != TYPE_ID_MONEY && VALID_POINT(p_item))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	BOOL is_notice( ) const
	{
		if(e_pick_mode == EPUM_Free /*||  e_pick_mode == EPUM_Order */)
			return FALSE;
		
		return e_assign_state == EGTAS_WaitForUmpirage;
	}

	BOOL is_wait_for_assign( ) const 
	{
		if( e_assign_state == EGTAS_WaitForSice ) return TRUE;
		if( e_assign_state == EGTAS_WaitForLeaderAssign ) return TRUE;
		return FALSE;
	}

	VOID finist_umpirage( )
	{
		if( e_pick_mode == EPUM_Leader )
			e_assign_state = EGTAS_WaitForLeaderAssign;

		if( e_pick_mode == EPUM_Sice )
			e_assign_state = EGTAS_WaitForSice;
	}

	BYTE get_score( DWORD dw_role_id_, BYTE& by_type_ ) const
	{
		return get_sice_data( ).GetScore( dw_role_id_, by_type_ );
	}

	DWORD get_boss_id() const {return dw_boss_id;}
	const tagSiceData& get_sice_data( ) const { return st_sice_data;}
	tagSiceData& get_sice_data( ) { return st_sice_data;}
	//VOID SetDelNextTick( ){ nTickCountDown = GROUND_ITEM_DISAPPEAR_TIME; }
};

//----------------------------------------------------------------------------------
// ���ӵ�ש
//----------------------------------------------------------------------------------

// �����м���ϰ˷���
enum E_unit_direction
{
	EUD_center = 0,
	EUD_top = 1,
	EUD_bottom = 2,
	EUD_left = 3,
	EUD_right = 4,
	EUD_left_top = 5,
	EUD_left_bottom = 6,
	EUD_right_top = 7,
	EUD_right_bottom = 8,
	EUD_end = 9,
};

struct tagVisTile
{
	package_map<DWORD, Role*>				map_role;
	package_map<DWORD, Role*>				map_leave_role;
	package_map<DWORD, Creature*>			map_creature;
	package_map<INT64, tag_ground_item*>	map_ground_item;
	package_map<DWORD, Unit*>				map_invisible_unit;
	INT										n_max_num;

	tagVisTile()
	{
		n_max_num = 100;
	}
};

//---------------------------------------------------------------------------------
// ����ID������
//---------------------------------------------------------------------------------
class creature_id_builder
{
public:
	VOID init(const tag_map_info* p_info_)
	{
		// ���ǵ���Ѩˢ���Լ�ʵʱˢ���Ĺ�����������֮��������100��ID
		//INT nCreatureNum = p_info_->map_creature_info.size() + p_info_->map_spawn_point.size() + 100;
		INT nCreatureNum = p_info_->map_monster_info.size() + p_info_->map_spawn_point.size() + 100;
		DWORD dw_min_id = (DWORD)MIN_CREATURE_ID;
		for(INT n = 0; n < nCreatureNum; n++)
		{
			list_free_creature_id.push_back(dw_min_id+(DWORD)n);
		}
		dw_max_creature_id = dw_min_id + nCreatureNum;
	}

	DWORD get_new_creature_id()
	{
		DWORD dw_creature_id = list_free_creature_id.pop_front();

		if( !VALID_POINT(dw_creature_id) )
		{
			dw_creature_id = dw_max_creature_id;
			// list���Ѿ�û�ж����id�ˣ���������100��
			for(INT n = 1; n < 100; n++)
			{
				dw_max_creature_id ++;
				//DWORD dwNewID = dw_max_creature_id+(DWORD)n;
				if (dw_max_creature_id > MAX_CREATURE_ID)
				{
					dw_max_creature_id = MID_CREATURE_ID;
				}
				list_free_creature_id.push_back(dw_max_creature_id);
			}
			
			dw_max_creature_id ++;
		}

		return dw_creature_id;
	}

	DWORD return_creature_id(DWORD dw_creature_id_)
	{
		ASSERT( IS_CREATURE(dw_creature_id_) );
		list_free_creature_id.push_front(dw_creature_id_);
	}

private:
	package_list<DWORD>		list_free_creature_id;
	DWORD					dw_max_creature_id;
};

//---------------------------------------------------------------------------------------
// ��ͼ��
//---------------------------------------------------------------------------------------
class Map : public ScriptData<ESD_Map>
{
public:
	typedef	package_map<DWORD, PlayerSession*>		SESSION_MAP;
	typedef package_map<DWORD, Role*>				ROLE_MAP;
	typedef package_map<DWORD, Creature*>			CREATURE_MAP;
	typedef package_map<INT64, tag_ground_item*>	GROUND_ITEM_MAP;
	typedef package_map<DWORD, Shop*>				SHOP_MAP;
	typedef package_map<DWORD, guild_chamber*>		CHAMBER_MAP;

public:
	Map();
	virtual ~Map();

	//-------------------------------------------------------------------------------------
	// ���������º�����
	//-------------------------------------------------------------------------------------
	BOOL				init(const tag_map_info* pInfo);
	VOID				update();
	VOID				destroy();
	VOID				init_creature_script();

	VOID				update_weather();
	
	//bool				initSparseGraph(const tag_map_info* pInfo);
	//-------------------------------------------------------------------------------------
	// ����Get
	//-------------------------------------------------------------------------------------
	DWORD				get_map_id()			{ return dw_id; }
	DWORD				get_instance_id()		{ return dw_instance_id; }
	EMapType			get_map_type()		{ return p_map_info->e_type; }
	ENormalMapType		get_normal_map_type()	{ return p_map_info->e_normal_map_type; }
	const tag_map_info*	get_map_info()		{ return p_map_info; }
	INT					get_role_num()		{ return map_role.size(); }
	INT					get_leave_role_num() { return map_leave_role.size(); }
	NPCTeamMgr*			get_team_manager()		{ return p_npc_team_mgr; }
	ROLE_MAP&			get_role_map()		{ return map_role; }
	ROLE_MAP&			get_leave_role_map() { return map_leave_role; }
	DWORD				get_update_time()	{ return dw_update_time; }
	//c_sparse_graph*		get_path_finder()	{ return m_pSparseGraph; }

	//------------------------------------------------------------------------------------------
	// ˫��
	//------------------------------------------------------------------------------------------
	FLOAT				get_exp_rate() ;
	FLOAT				get_drop_rate();
	FLOAT				get_practice_rate();

	//--------------------------------------------------------------------------------------
	// ���ӵ�ש���
	//--------------------------------------------------------------------------------------
	VOID				get_visible_tile(INT n_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile(Vector3& v_pos_, FLOAT f_range_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_add(INT n_ole_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_[EUD_end]);
	VOID				get_visible_tile_add_and_dec(INT n_old_visible_index_, INT n_new_visible_index_, tagVisTile* p_visible_tile_add_[EUD_end], tagVisTile* p_visible_tile_dec_[EUD_end]);

	//--------------------------------------------------------------------------------------
	// ��ң�����͵�����Ʒ����Ӻ�ɾ��
	//--------------------------------------------------------------------------------------
	virtual VOID		add_role(Role* p_role_, Map* pSrcMap = NULL);
	virtual INT			can_enter(Role *p_role_)			{ return TRUE; }
	virtual BOOL		changePkValue()	{ return TRUE; }
	virtual DWORD		get_param() { return INVALID_VALUE; }
	virtual DWORD		FriendEnemy(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_);
	VOID				role_logout(DWORD dw_id_);
	VOID				role_leave_logout(DWORD	dw_id_);
	VOID				role_leave_map_practice(DWORD dw_id_);
	virtual	VOID		role_leave_map(DWORD dw_id_, BOOL b_leave_online = FALSE);
	VOID				add_creature_on_load(Creature* p_creature_);
	VOID				add_creature(Creature* p_creature_);
	VOID				remove_creature(Creature* p_creature_);
	VOID				remove_npc( Creature* p_creature_ );
	VOID				remove_pet(Pet* p_pet_, BOOL bSendMsg = TRUE);
	VOID				remove_flow_creature(Creature* p_creature);
	VOID				add_ground_item(tag_ground_item *p_ground_item_);
	VOID				remove_ground_item(tag_ground_item* p_ground_item_);
	virtual VOID 		spawn_creature_replace(Creature* p_dead_creature_);
	VOID				spawn_list_creature_replace(Creature* p_dead_creature_);
	Creature*			create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, DWORD dw_spawner_id_=INVALID_VALUE, BOOL b_collide_=FALSE, LPCTSTR p_patrol_list_name_=NULL);
	Creature*			create_creature(DWORD dw_data_id_, Vector3& v_pos_, Vector3& v_face_to_, ECreatureSpawnedType e_spawned_type_, 
										DWORD dw_guild_id_ = INVALID_VALUE, DWORD dw_spawner_id_=INVALID_VALUE, BOOL b_collide_=FALSE, 
										TCHAR* p_patrol_list_name_=NULL);
	VOID				delete_creature(DWORD dw_id_);
	VOID				send_npc_loading(Role* p_role_);
	VOID				add_pet_delete_map(Creature* pPet);
	//---------------------------------------------------------------------------------------
	// ��Ϸ����Һ�����Ĳ�ѯ
	//---------------------------------------------------------------------------------------
	Role*				find_role(DWORD dw_id_)			{ return map_role.find(dw_id_); }		// һ���ڵ�ͼ�߳�֮��ʹ��
	Role*				find_leave_role(DWORD dw_id_)	{ return map_leave_role.find(dw_id_); }
	Creature*			find_creature(DWORD dw_id_)		{ return map_creature.find(dw_id_); }	// һ���ڵ�ͼ�߳�֮��ʹ��
	Pet*				find_pet(DWORD dw_id_);				// һ���ڵ�ͼ�߳�֮��ʹ��;	
	Unit*				find_unit(DWORD dw_id_);
	tag_ground_item*	get_ground_item(INT64 n64_serial)	{ return map_ground_item.find(n64_serial); }
	Shop*				get_shop(DWORD dw_npc_id_)			{ return map_shop.find(dw_npc_id_); }
	guild_chamber*		get_chamber(DWORD dw_npc_id_)			{ return map_chamber.find(dw_npc_id_); }

	//--------------------------------------------------------------------------------------
	// ���ڵ�ͼ�Ź���ĸ���ͬ������
	//--------------------------------------------------------------------------------------
	VOID				synchronize_movement_to_big_visible_tile(Unit* p_self_);
	VOID				synchronize_remove_unit_to_big_visible_tile(Unit* p_self_);
	VOID				synchronize_change_visible_tile(Unit* p_self_, INT n_old_visible_index_, INT n_new_visible_index_);
	VOID				synchronize_role_leave_map(Role* p_self_, BOOL b_leave_online = FALSE);
	VOID				send_big_visible_tile_message(Unit* p_self_, const LPVOID p_message_, const DWORD dw_size_);
	VOID				send_big_visible_tile_message(tagVisTile *p_visible_tile_[EUD_end], const LPVOID p_message_, const DWORD dw_size_);
	VOID				send_map_message(const LPVOID p_message_, const DWORD dw_size_);
	VOID				synchronize_remove_units(Unit* p_self_, tagVisTile* p_visible_tile_dec_[EUD_end]);
	VOID				synchronize_add_units(Unit* p_self_, tagVisTile* p_visible_tile_add_[EUD_end]);
	VOID				synchronize_add_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);
	VOID				synchronize_ground_item_state(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);
	VOID				synchronize_remove_ground_item(tag_ground_item *p_ground_item_, tagVisTile *p_visible_tile_add_[EUD_end]);

	//--------------------------------------------------------------------------------------
	// ��ͼ��Ϣ��ͬ��
	//--------------------------------------------------------------------------------------
	VOID				send_scene_effect(Role* p_self_);
	VOID				play_scene_effect(ESceneEffectType e_type_, DWORD dw_obj_id_, Vector3 v_pos_, DWORD dw_effect_id_);
	VOID				stop_scene_effect(DWORD dw_obj_id_);

	//----------------------------------------------------------------------------------------
	// ��ͼ�������
	//----------------------------------------------------------------------------------------
	INT64				check_area(Role* p_role_);							// ����������ڵĸ������򣨷��������־λ��

	//----------------------------------------------------------------------------------------
	// �����⼰����
	//----------------------------------------------------------------------------------------
	BOOL				is_position_valid(const Vector3& v_vector_);
	VOID				fix_position(Vector3& v_vector_);

	//----------------------------------------------------------------------------------------
	// ��ͼ�������������
	//----------------------------------------------------------------------------------------
	BOOL				is_in_trigger(Role* p_role_, DWORD dw_map_trigger_id_);	// �Ƿ��ڴ�������
	DWORD				get_trigger_quest_serial(DWORD dw_map_trigger_id_);	// �õ�ĳ��trigger��Ӧ���������к�
	
	//----------------------------------------------------------------------------------------
	// ������ص�һЩ����
	//----------------------------------------------------------------------------------------
	BOOL				can_putdown(Unit* p_unit, INT n_index_, Vector3& v_position_);
	VOID				putdown_item(Creature* p_creature_, tagItem* p_loot_item_, DWORD dw_owner_id_, DWORD dw_team_id_, 
									 Vector3& v_position_, EPickMode e_pick_mode_ = EPUM_Free, EGTAssignState e_assign_sate_ = EGTAS_Null, INT n_tick_down_ = 0);
	VOID				putdown_money(Creature* p_creature_, INT n_money_, DWORD dwTeamID, DWORD dwOwnerID, Vector3 v_positon_);
	BOOL				get_can_go_position_from_index(INT32 n_index_, INT32 n_index_x_, INT32 n_index_z_, INT32 n_, Vector3& v_position_);
	INT64				get_ground_item_id()		{ return builder_ground_item_id.builder_valid_id(); }

	//----------------------------------------------------------------------------------------
	// ��ͼս��ϵͳ������أ���ҪӦ���ڸ���
	//----------------------------------------------------------------------------------------
	BOOL				can_use_item(DWORD dw_data_id, INT64 n64_Item_Serial);
	BOOL				can_use_skill(DWORD dw_item_id_);
	VOID				onclock(INT nClock);

	//----------------------------------------------------------------------------------------
	// ���ҹ�ϵ�ж�
	//----------------------------------------------------------------------------------------
	DWORD				get_friend_enemy_flag(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_);

	//------------------------------------------------------------------------------------------
	// ��ͼ�������
	//------------------------------------------------------------------------------------------
	tag_map_area_info*		is_in_area(tag_map_info::MAP_AREA& map_area_, float x, float z);

	//----------------------------------------------------------------------------------------
	// һЩ��������
	//----------------------------------------------------------------------------------------
	VOID				send_goto_new_map_to_player(Role* p_self_);
	BOOL				in_same_big_visible_tile(Unit* p_self_, Unit* p_remote_);
	INT					world_position_to_visible_index(const Vector3& v_pos_);
	BOOL				if_can_go(FLOAT f_x_, FLOAT f_z_);
	BOOL				if_can_direct_go(FLOAT f_srcx_, FLOAT f_src_y_, FLOAT f_dest_x_, FLOAT f_dest_z_, pathNode& n_near_pos_);

	template<typename Operation>
	VOID				foreach_unit_in_big_visible_tile(Unit* p_unit_, Operation oper_);

	template<typename RoleOperation>
	VOID				for_each_role_in_map( RoleOperation oper_);

	template<typename Operation, typename Predicate>
	VOID				foreach_unit_in_map(Operation oper_, Predicate pred_);
	
	BOOL				is_point_has_uint(DWORD id);

	BOOL				RandSpwanPoint(tag_map_spawn_point_list* spawnList, tag_map_spawn_point_info*& pMapSpawnInfo);
	
	//BOOL				is_have_unit(const Vector3& point);
	//VOID				set_have_unit(const Vector3& point, bool bset);
	//-----------------------------------------------------------------------------------------
	// ��ͼ�¼�
	//-----------------------------------------------------------------------------------------
	virtual	VOID		on_role_die(Role* p_role_, Unit* p_killer_);
	virtual VOID		on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
								  INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_);
	virtual VOID		on_creature_die(Creature* p_creature_, Unit* p_killer_);
	VOID				on_creature_disappear(Creature* p_creature_);
	VOID				on_enter_trigger(Role* p_role_, tag_map_trigger_info* p_info_);
	VOID				on_enter_area(Role* p_role_, tag_map_area_info* p_info_);
	INT		            can_invite_join_team();
	INT 				can_leave_team();
	INT					can_change_leader();
	INT					can_kick_member();
	BOOL				can_set_safe_guard();

	//------------------------------------------------------------------------------------------
	// Ǳ��
	//------------------------------------------------------------------------------------------
	VOID				lurk(Unit *p_unit_);
	VOID				unlurk(Unit *p_unit_);
	VOID				update_lurk_to_big_visible_tile_role(Unit *p_unit_);
	VOID				update_big_visible_tile_lurk_unit_to_role(Role* p_role_);

	Role*				stop_leave_prictice(DWORD	dw_id_);
	VOID				remove_leave_pricitce_visible_tile(Unit* p_unit_);


	void SetWeather(DWORD id, BOOL bSnd = TRUE)
	{
		if(id != dw_weather_id_)
		{
			dw_weather_id_ = id;

			if(bSnd){
				NET_SIS_change_Weather send;
				send.dwMapID = get_map_id();
				send.dwInstanceID = get_instance_id( );
				send.dwWeather = id;
				send_map_message(&send, send.dw_size);
			}
		}
	}
	DWORD GetWeather( ) const{return dw_weather_id_; }

	BOOL IsHaveUnit(const Vector3 piont);

	DWORD getNewCreatureID() { return builder_creature_id.get_new_creature_id(); }

	// ��ȡ��ͼ�ڽ�ɫ���id,�в�ͬ�ķ���inval
	DWORD getAllRoleSameGuildID();
protected:
	//-----------------------------------------------------------------------------------------
	// ��ʼ��
	//-----------------------------------------------------------------------------------------
	BOOL				init_logical_info(DWORD dw_guild_id_ = INVALID_VALUE);	// ���ݵ�ͼ������Ϣ��ʼ�������߼���Ϣ
	BOOL				init_all_map_creature(DWORD dw_guild_id_ = INVALID_VALUE);					// ���ɵ�ͼ��ĳ�ʼ����
	virtual BOOL		init_all_fixed_creature(DWORD dw_guild_id_ = INVALID_VALUE);					// ���ɵ�ͼ������аڷŹ���
	virtual BOOL		init_all_fixed_creature_ex(DWORD dw_guild_id_ = INVALID_VALUE);//gx add 2013.4.16
	virtual BOOL		init_all_spawn_point_creature(DWORD dw_guild_id_ = INVALID_VALUE);			// ��������ˢ�µ����
	virtual BOOL		init_all_spawn_list_creature(DWORD dw_guild_id_ = INVALID_VALUE);	// ��������ˢ�ֵ������
	VOID				init_nest_creature(Creature* p_creature_);	// ��ʼ����Ѩ����
	VOID				init_team_creature(Creature* p_creature_);  // ��ʼ��С�ӹ���
	VOID				init_team_creature(Creature* p_creature_, guild* p_guild_);	//��ʼ��С�ӹ���
	BOOL				add_shop(DWORD dw_npc_id_, DWORD dw_shop_id_);	// ��ʼ���̵�
	BOOL				add_chamber(DWORD dw_npc_id_, DWORD dw_chamber_id_);	// ��ʼ���̻�

	//------------------------------------------------------------------------------------------
	// ��ͼ�¼�
	//------------------------------------------------------------------------------------------
	VOID				on_init();								// ��ʼ��ʱ

	//------------------------------------------------------------------------------------------
	// ����
	//------------------------------------------------------------------------------------------
	VOID				update_session();			// ����õ�ͼ���������ҵ�������Ϣ
	VOID				update_all_objects();			// ���¸õ�ͼ�����ж���
	VOID				update_all_shops();			// ���¸õ�ͼ�����ж���
	VOID				update_all_chamber();			// ���¸õ�ͼ�����ж���
	VOID				update_num();

	//------------------------------------------------------------------------------------------
	// �Ź������
	//------------------------------------------------------------------------------------------
	VOID				add_to_visible_tile(Unit* p_unit_, INT n_visible_index_);
	VOID				remove_from_visible_tile(Unit* p_unit_, BOOL b_leave_online = FALSE);
	VOID				add_ground_item_to_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_);
	VOID				remove_ground_item_from_visible_tile(tag_ground_item* p_ground_item_, INT n_visible_index_);

	//------------------------------------------------------------------------------------------
	// ����ͬ��
	//------------------------------------------------------------------------------------------
	VOID				syn_big_visible_tile_invisible_unit_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end]);
	VOID				syn_big_visible_tile_visible_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTile[EUD_end]);

	VOID				syn_invisible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
													const LPVOID p_message, const DWORD dw_size);
	VOID				syn_visible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
													const LPVOID p_message, const DWORD dw_size);

	VOID				syn_remove_big_visible_tile_unit_and_ground_item_to_role(Role* pRole, tagVisTile *pVisTileDec[EUD_end]);

	//------------------------------------------------------------------------------------------
	// ��������
	//------------------------------------------------------------------------------------------
	DWORD				calculate_movement_message(Unit* pSelf, LPBYTE p_message, DWORD dw_size);


protected:
	

	DWORD					dw_id;					// ��ͼID
	DWORD					dw_instance_id;			// ����ID����ͨ��ͼ��IDΪINVALID_VALUE��

	//-------------------------------------------------------------------------------------------
	// ��������
	//-------------------------------------------------------------------------------------------
	SESSION_MAP					map_session;				// ��ͼ�����session�б�
	ROLE_MAP					map_role;					// ��ͼ������������
	ROLE_MAP					map_leave_role;				// ��ͼ������йһ����
	CREATURE_MAP				map_creature;				// ��ͼ�������������
	CREATURE_MAP				map_respawn_creature;		// ������ȴ�ˢ�µ�����
	CREATURE_MAP				map_door;					// ��ͼ�ϵ��Ŷ����б�
	CREATURE_MAP				map_npc;					// �ӵ�ͼ���Ƴ��ı����͵�NPC
	GROUND_ITEM_MAP				map_ground_item;			// ���������Ʒ
	SHOP_MAP					map_shop;					// ��ͼ����������̵�<dwNpcID, pShop>
	CHAMBER_MAP					map_chamber;				// ���������̻�
	CREATURE_MAP				map_pet_delete;				// ��ͼ���������Ҫ�ͷ��ڴ�ĳ���

	//-------------------------------------------------------------------------------------------
	// �Ź���
	//-------------------------------------------------------------------------------------------
	INT						n_visible_tile_array_width;	// �����ӵ�ש���
	INT						n_visible_tile_array_height;	// �����ӵ�ש����
	tagVisTile*				p_visible_tile;				// ���ӵ�ש����̬���ɵ�����

	//-------------------------------------------------------------------------------------------
	// ��ͼ����
	//-------------------------------------------------------------------------------------------
	const tag_map_info*		p_map_info;				// ��ͼ������Ϣ���ⲿ��̬���ԣ�����ɾ����
	const MapScript*		p_script;				// ��ͼ�ű�
	typedef std::map<DWORD, BOOL> 	MAP_POINT_HAS_LIST;			// ˢ�ֵ��Ƿ�ˢ�˹�
	MAP_POINT_HAS_LIST		map_point_has_list;
	//c_sparse_graph*				m_pSparseGraph;		// Ѱ·
	//MAP_POINT_HAS_LIST		map_point_has_unit;		// ĳ�����Ƿ�������

	//-------------------------------------------------------------------------------------------
	// �Ѵ���������Ч�б�
	//-------------------------------------------------------------------------------------------
	package_list<DWORD>			list_scene_effect;		// �Ѵ���������ЧObjID�б�
	Mutex					h_mutex;				// ��Ч�б���

	//-------------------------------------------------------------------------------------------
	// ����ID������
	//-------------------------------------------------------------------------------------------
	creature_id_builder			builder_creature_id;		// ����ID������

	//-------------------------------------------------------------------------------------------
	// ����ID������
	//-------------------------------------------------------------------------------------------
	ground_item_id_builder			builder_ground_item_id;		// ����ID������

	//-------------------------------------------------------------------------------------------
	// ����С�ӹ�����
	//-------------------------------------------------------------------------------------------
	NPCTeamMgr*				p_npc_team_mgr;

	DWORD					dw_update_num_time;

	package_list<tagNPCLoading*>		list_NPCLoading;

	DWORD					dw_update_time;
	DWORD				dw_weather_id_;
	INT				n_chang_weahter_tick_;

};

//-----------------------------------------------------------------------------------------------
// ��������
//-----------------------------------------------------------------------------------------------
class map_instance : public Map
{
public:
	map_instance() : b_delete(FALSE), b_end(FALSE),b_complete(FALSE) {}
	virtual ~map_instance() {}

	virtual BOOL	init(const tag_map_info* pInfo, DWORD dwInstanceID, Role* pCreator=NULL, DWORD dwMisc=INVALID_VALUE) = 0;
	virtual VOID	update() = 0;
	virtual VOID	destroy() = 0;

	virtual VOID	add_role(Role* pRole, Map* pSrcMap) = 0;
	virtual	VOID	role_leave_map(DWORD dwID, BOOL b_leave_online = FALSE) = 0;
	virtual INT		can_enter(Role *pRole, DWORD dwParam = 0)		{ if( is_delete() || is_complete() ) return E_Instance_End_Delete; else return E_Success; }
	virtual BOOL	can_destroy()				{ return is_delete() && map_role.empty(); }

	virtual VOID	on_destroy() = 0;

	//---------------------------------------------------------------------------------------------
	// ����������ɾ����־λ
	//---------------------------------------------------------------------------------------------
	VOID			set_delete()					{ if( is_delete() ) return; Interlocked_Exchange((LPLONG)&b_delete, TRUE); on_delete(); }
	VOID			set_end()					{ if( is_end() ) return; Interlocked_Exchange((LPLONG)&b_end, TRUE); }
	VOID			set_complete()				{ if(is_complete()) return; Interlocked_Exchange((LPLONG)&b_complete, TRUE); }
	BOOL			is_end()						{ return b_end; }
	BOOL			is_delete()					{ return b_delete; }
	BOOL			is_complete()				{ return b_complete; }

protected:
	virtual VOID	on_delete() = 0;						// ��������ɾ����־ʱ�Ĵ���ע�⣬�������ڸ�������ʱ����
	virtual BOOL	init_all_spawn_point_creature(DWORD dwGuildID = INVALID_VALUE) = 0;	// ��������ˢ�µ����

protected:
	volatile BOOL			b_delete;					// ����ɾ����־λ
	volatile BOOL			b_end;						// ����������־λ
	volatile BOOL			b_complete;				// ���������Ƿ����
};

//------------------------------------------------------------------------------------------------
// ĳ�����Ƿ��Ƿ��ǿ����ߵ�
//------------------------------------------------------------------------------------------------
inline BOOL Map::if_can_go(FLOAT f_x_, FLOAT f_z_)
{
	// �ļ�����Ǵ����Ͻ�Ϊԭ�㣬ת�����½�Ϊԭ��
	int nTitleIndex = (int)(f_x_/TILE_SCALE) + (p_map_info->n_height - (int)(f_z_/TILE_SCALE) - 1) * p_map_info->n_width;
	if (nTitleIndex >= p_map_info->map_block_data.size())
	{
		return false;
	}
	return (p_map_info->map_block_data[nTitleIndex] == 0);
	//return get_nav_map()->GetNPCNavMap()->IfCanGo(f_x_, f_z_);
}


//------------------------------------------------------------------------------------------------------------------
// �������
//------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_role_die(Role* p_role_, Unit* p_killer_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnRoleDie(p_role_, p_killer_, this);
	}
}

//------------------------------------------------------------------------------------------------------------------
// ����Ƿ���Կ���PK����
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_set_safe_guard()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_set_safe_guard(this);
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------------
// ����Ƿ����ʹ����Ʒ
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_use_item(DWORD dw_data_id, INT64 n64_Item_Serial)
{
	if( VALID_POINT(p_script) ) 
	{
		return p_script->can_use_item(this, dw_data_id, n64_Item_Serial);
	}

	return TRUE;
}

inline VOID Map::onclock(INT nClock)
{
	if(VALID_POINT(p_script))
	{
		p_script->onclock(this, nClock);
	}
}

//------------------------------------------------------------------------------------------------------------------
// ����Ƿ����ʹ�ü���
//------------------------------------------------------------------------------------------------------------------
inline BOOL Map::can_use_skill(DWORD dw_data_id)
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_use_skill(this, dw_data_id);
	}

	return TRUE;
}

//------------------------------------------------------------------------------------------------------------------
// ��Ҹ���
//------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_revive(Role* p_role_, ERoleReviveType e_type_, INT &n_revive_hp_, 
						   INT &n_revive_mp_, FLOAT &fx_, FLOAT &fy_, FLOAT &fz_, DWORD &dw_reborn_map_id_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->Revive(p_role_, e_type_, n_revive_hp_, n_revive_mp_, fx_, fy_, fz_, dw_reborn_map_id_);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// ��������
//-------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_creature_die(Creature* p_creature_, Unit* p_killer_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnCreatureDie(p_creature_, p_killer_, this);
	}
}

//-------------------------------------------------------------------------------------------------------------------
// ������ʧ
//-------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_creature_disappear(Creature* p_creature_)
{
	if( VALID_POINT(p_script) ) 
	{
		p_script->OnCreatureDisappear(p_creature_, this);
	}
}

//--------------------------------------------------------------------------------------------------------------------
// ��ҽ��봥����
//--------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_enter_trigger(Role* p_role_, tag_map_trigger_info* p_info_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnEnterTrigger(p_role_, p_info_, this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// ��ҽ�������
//---------------------------------------------------------------------------------------------------------------------
inline VOID Map::on_enter_area(Role* p_role_, tag_map_area_info* p_info_)
{
	if( VALID_POINT(p_script) )
	{
		p_script->OnEnterArea(p_role_, p_info_, this);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// ���ɼ���λͬ������Χ���
//---------------------------------------------------------------------------------------------------------------------
inline VOID Map::syn_visible_unit_to_big_visible_tile_role(Unit* pUnit, tagVisTile *pVisTile[EUD_end], 
											const LPVOID p_message, const DWORD dw_size)
{
	send_big_visible_tile_message(pVisTile, p_message, dw_size);
}

//---------------------------------------------------------------------------------------------------------------------
// �Ƿ������������
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_invite_join_team()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_invite_join_team(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// �Ƿ������뿪����
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_leave_team()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_leave_team(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// �Ƿ������ƽ��ӳ�
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_change_leader()
{
	if( VALID_POINT(p_script) )
	{
		return p_script->can_change_leader(this);
	}

	return 0;
}

//---------------------------------------------------------------------------------------------------------------------
// �Ƿ�����ߵ����
//---------------------------------------------------------------------------------------------------------------------
inline INT Map::can_kick_member()
{
	if( VALID_POINT(p_script) ) 
	{
		return p_script->can_kick_member(this);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------
// ���ҹ�ϵ�ж�
//-----------------------------------------------------------------------------------------------------------------------
inline DWORD Map::get_friend_enemy_flag(Unit* p_src_, Unit* p_target_, BOOL& b_ignore_)
{
	if( !VALID_POINT(p_script) )
	{
		return FriendEnemy(p_src_, p_target_, b_ignore_);;
	}

	return p_script->FriendEnemy(this, p_src_, p_target_, b_ignore_);
}

//-----------------------------------------------------------------------------------------------------------------------
// ����Ƿ�
//-----------------------------------------------------------------------------------------------------------------------
inline BOOL Map::is_position_valid(const Vector3& v_vector_)
{
	// ����Ƿ񳬳��߽�
	if( v_vector_.x < 0.0f || v_vector_.x >= FLOAT(p_map_info->n_width * TILE_SCALE) ||
		v_vector_.z < 0.0f || v_vector_.z >= FLOAT(p_map_info->n_height * TILE_SCALE) )
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------------------------------------------------------
// ��������
//--------------------------------------------------------------------------------------------------------------------------
inline VOID Map::fix_position(Vector3& v_vector_)
{
	// ������Χ
	if( v_vector_.x < 0.0f )				v_vector_.x = 0.0f;
	if( v_vector_.z < 0.0f )				v_vector_.z = 0.0f;
	if( v_vector_.x >= FLOAT(p_map_info->n_width * TILE_SCALE) )		v_vector_.x = FLOAT(p_map_info->n_width * TILE_SCALE) - 1.0f;
	if( v_vector_.z >= FLOAT(p_map_info->n_height * TILE_SCALE) )	v_vector_.z = FLOAT(p_map_info->n_height * TILE_SCALE) - 1.0f;

	v_vector_.y = 0;
}

template<typename Operation>
VOID Map::foreach_unit_in_big_visible_tile(Unit* p_unit_, Operation oper_)
{
	if( !VALID_POINT(p_unit_) ) return;

	tagVisTile* pVisTile[EUD_end] = {0};
	get_visible_tile(p_unit_->GetVisTileIndex(), pVisTile);

	for(INT n = 0; n < EUD_end; n++)
	{
		if( NULL == pVisTile[n] )
			continue;

		// �ҵ�ÿ����ש����
		package_map<DWORD, Role*>& mapRole = pVisTile[n]->map_role;
		mapRole.reset_iterator();
		Role* pRole = NULL;

		while( mapRole.find_next(pRole) )
		{
			if( VALID_POINT(pRole) )
			{
				oper_(pRole);				
			}
		}
	}
}

template<typename Operation, typename Predicate>
VOID Map::foreach_unit_in_map(Operation oper_, Predicate pred_)
{
	// �ҵ�ÿ����ͼ����
	ROLE_MAP::map_iter itrR = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itrR, pRole) )
	{
		if( VALID_POINT(pRole) && pred_(pRole))
		{
			oper_(pRole);				
		}
	}

	// �ҵ�ÿ����ͼ�Ĺ���
	CREATURE_MAP::map_iter itrC = map_creature.begin();
	Creature* pCreature = NULL;

	while( map_creature.find_next(itrC, pCreature) )
	{
		if( VALID_POINT(pCreature) && pred_(pCreature))
		{
			oper_(pCreature);				
		}
	}
}

template<typename RoleOperation>
VOID Map::for_each_role_in_map( RoleOperation oper_)
{
	// �ҵ�ÿ����ͼ����
	ROLE_MAP::map_iter itrR = map_role.begin();
	Role* pRole = NULL;

	while( map_role.find_next(itrR, pRole) )
		if(VALID_POINT(pRole)) oper_(pRole);			
}


// ��ˢ�ֵ��������һ��û�йֵ�ˢ�ֵ�
//class MapSpawnListHelper
//{
//public:
//	static BOOL RandSpwanPoint(Map* pMap, tag_map_spawn_point_list* spawnList, tag_map_spawn_point_info*& pMapSpawnInfo)
//	{
//		if (!VALID_POINT(spawnList))
//			return false;
//		package_map<DWORD,	tag_map_spawn_point_info*> list = spawnList->list;
//		DWORD dwSpawnID = 0;
//		tag_map_spawn_point_info* pInfo = NULL;
//		package_map<DWORD,	tag_map_spawn_point_info*>::map_iter it = list.begin();
//		while(list.find_next(it, dwSpawnID, pInfo))
//		{
//			if (pMap->is_point_has_uint(pInfo->dw_att_id))
//			{
//				list.erase(dwSpawnID);
//			}
//		}
//		if (list.size() <= 0)
//			return false;
//
//		return list.rand_find(dwSpawnID, pMapSpawnInfo);
//	}
//};

#endif
