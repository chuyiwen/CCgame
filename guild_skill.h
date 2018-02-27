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
*	@file		guild_skill.h
*	@author		lc
*	@date		2011/02/25	initial
*	@version	0.0.1.0
*	@brief		
*/


#ifndef GUILD_SKILL
#define GUILD_SKILL

class guild;
class Role;
struct tagGuildSkill;
struct s_guild_skill_info;

class guild_skill
{
public:
	guild_skill();
	~guild_skill();

	VOID	init(guild* p_guild_, BOOL b_request_ = FALSE);
	VOID	destroy();

	BOOL	is_init_ok()	{ return b_init; }

public:
	//! ������ɼ�������
	BOOL	load_guild_skill_info(s_guild_skill_info* p_skill_info_, INT n_skill_num_);

	//! ��ȡ���а�Ἴ������
	DWORD	get_guild_skill_info(DWORD& dw_skill_id_, INT16& n16_progress_, INT& n_skill_num_, DWORD* p_level_info_);

	//! ��Ἴ����������
	DWORD	skill_upgrade_on_clock();

	//! �Ͻ���Ʒ������Ἴ��
	DWORD	skill_upgrade_turn_in(Role* p_role_, INT64 n64_serial_, DWORD& dw_skill_id_, INT16& n16_progress_);

	//�� ѧϰ��Ἴ��
	DWORD	learn_guild_skill(Role* p_role_, DWORD dw_skill_id_, INT& n_level_, BOOL& b_learn_error_);

	//! ������Ἴ��
	VOID	create_guild_skill();

	//! ���õ�ǰҪ�����İ�Ἴ��
	DWORD	set_current_upgrade_guild_skill(DWORD dw_role_id_, DWORD dw_skill_id_, INT& nLevel_, INT16& n16_progress_);

	//! ������а�Ἴ������
	VOID	remove_guild_skill_info();

private:
	
	VOID	save_skill_info_to_db(DWORD dw_skill_id_);
	VOID	change_research_skill(DWORD dw_new_skill_id_);
	
private:
	typedef package_map<DWORD, tagGuildSkill*>	MAP_GUILD_SKILL;

	BOOL			b_init;
	guild*			p_guild;

	DWORD			dw_current_skill;
	MAP_GUILD_SKILL	map_guild_skill;
};
#endif