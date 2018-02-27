/*******************************************************************************

Copyright 2010 by Shengshi Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
Shengshi Interactive  Co., Ltd.

*******************************************************************************/



#ifndef _ACHIEVEMENT_H_
#define _ACHIEVEMENT_H_


//-----------------------------------------------------------------------------
// 成就数目
//-----------------------------------------------------------------------------
const UINT16 MAX_TITLE_NUM			= 66;


//-----------------------------------------------------------------------------
// 成就ID是否有效
//-----------------------------------------------------------------------------
#define TITLEID_VALID( id ) ((id) >= 0 && (id) < MAX_TITLE_NUM)

//-----------------------------------------------------------------------------
// 触发事件
//-----------------------------------------------------------------------------
enum E_title_event
{	
	ete_null						=-1,
	ete_begin						=0 ,	

	ete_kill_monster				=0 ,	// 击杀怪物
	ete_kill_npc					=1 ,	// 击杀NPC
	ete_kill_boss					=2 ,	// 击杀Boss
	ete_kill_role					=3 ,	// 击杀角色
	ete_quest_complete				=4 ,	// 完成任务
	ete_quest_failed				=5 ,	// 任务失败
	ete_role_die					=6 ,	// 角色死亡
	ete_role_skilled_by_role		=7 ,	// 角色被其他角色击杀
	ete_composite_equip_success		=8 ,	// 合成装备成功
	ete_composite_item_success		=9 ,	// 合成物品成功
	ete_strengthen_equip_success	=10,	// 强化装备成功
	ete_strengthen_equip_failed		=11,	// 强化装备失败
	ete_strengthen_equip_perfect	=12,	// 强化装备完美
	ete_identify_blue_equip			=13,	// 鉴定出蓝色或蓝色以上品级
	ete_stall						=14,	// 摆摊
	ete_gather						=15,	// 采集、收获
	ete_shout						=16,	// 使用传音
	ete_role_transaction_success	=17,	// 与角色成功交易
	ete_create_faction				=18,	// 建立帮派
	ete_be_castllan					=19,	// 成为城主
	ete_friend_make					=20,	// 加为好友
	ete_marry_join					=21,	// 结婚
	ete_marry_separation			=22,	// 离婚
	ete_use_item					=23,	// 角色使用道具
	ete_use_skill					=24,	// 角色使用技能
	ete_join_famehall				=25,	// 角色进入某个名人堂
	ete_role_level					=26,	// 角色达到级别
	ete_repute_change				=27,	// 声望变化
	ete_friend_value				=28,	// 友好度变化

	ete_max_event_num					=29,	// 事件数目
	ete_end								=ete_max_event_num,
};


//-----------------------------------------------------------------------------
// 称号条件类型
//-----------------------------------------------------------------------------
enum e_condition_type
{
	ect_count						= 0,	// 计数条件
	ect_value						= 1,	// 阈值条件
	ect_check						= 2,	// 特殊检查条件
};

//-----------------------------------------------------------------------------
// 条件检查返回值
//-----------------------------------------------------------------------------
enum e_check_result
{
	ecr_active						= 0,	// 激活
	ecr_count_down					= 1,	// 减少
	ecr_noafect						= 2,	// 无影响
};


//-----------------------------------------------------------------------------
// 称号条件基类
//-----------------------------------------------------------------------------
class condition
{
public:
	condition(UINT16 n_title_id, DWORD dw_param1, DWORD dw_param2, BOOL b_show)
		:n16_title_id_(n_title_id), dw_param1_(dw_param1), dw_param2_(dw_param2),b_show_(b_show){}
	bool is_count_cond() const;
	e_check_result check(DWORD dw_arg1, DWORD dw_arg2) {	return do_check(dw_arg1, dw_arg2);	}
	void set_para2(DWORD dw_para2)	{	dw_param2_ = dw_para2; }
	DWORD get_para2() const {return dw_param2_;}
	BOOL  is_show() { return b_show_; }
protected:
	//	Role* GetRole() const {return m_pRole;}
private:
	virtual e_check_result do_check(DWORD dwArg1, DWORD dwArg2) = 0;

protected:
	DWORD	dw_param1_;
	DWORD	dw_param2_;
private:
	UINT16	n16_title_id_;
	BOOL	b_show_;
	//	Role*	m_pRole;
};


//-----------------------------------------------------------------------------
// 计数条件
//-----------------------------------------------------------------------------
class count_condition: public condition
{
public:
	count_condition(INT16 n_title_id, DWORD dw_param1, DWORD dw_param2, BOOL b_show)
		:condition(n_title_id, dw_param1, dw_param2, b_show){}

private:
	virtual e_check_result do_check(DWORD dwSubType, DWORD dwArg2)
	{
		if ( VALID_VALUE(dw_param1_) && dwSubType != dw_param1_) return ecr_noafect; 
		if ((--dw_param2_) <= 0)
		{
			return ecr_active;
		}
		else
		{
			return ecr_count_down;
		}
	}
};

//-----------------------------------------------------------------------------
// 阈值条件
//-----------------------------------------------------------------------------
class ValueCondition: public condition
{
public:
	ValueCondition(UINT16 u16TitleID, DWORD dwPara1, DWORD dwPara2, BOOL IsShow)
		:condition(u16TitleID, dwPara1, dwPara2, IsShow){}
private:
	virtual e_check_result do_check(DWORD dwSubType, DWORD dwValue)
	{
		if ( VALID_VALUE(dw_param1_) && dwSubType != dw_param1_) return ecr_noafect; 
		if (dwValue >= dw_param2_)
		{
			return ecr_active;
		}
		else
		{
			return ecr_noafect; 
		}
	}
};

//-----------------------------------------------------------------------------
// 复杂检测条件
//-----------------------------------------------------------------------------
class CheckCondition:public condition
{
public:
	CheckCondition(UINT16 u16TitleID, DWORD dwPara1, DWORD dwPara2, BOOL IsShow)
		:condition(u16TitleID, dwPara1, dwPara2, IsShow){}
private:
	virtual e_check_result do_check(DWORD dwArg1, DWORD dwArg2)
	{
		//		Role* pRole = GetRole();

		return ecr_noafect;//pRole->CheckCondition();
	}
};



//-----------------------------------------------------------------------------
// 成就原型
//-----------------------------------------------------------------------------
struct tagAchievementProto
{

	UINT16				m_u16ID;
	DWORD				m_dwBuffID;

	e_condition_type 	m_CondType;
	DWORD				m_dwPara1;
	DWORD				m_dwPara2;

	BOOL				bShow;


	E_title_event		m_Events;
};

// 进行中的成就
struct CriteriaProgress
{
	DWORD date;			// 时间
	DWORD counter;		// 计数
	bool changed;		// 是否改变,记录数据库用
	bool timedCriteriaFailed;
};


// 已完成的成就
struct CompletedAchievementData
{
	DWORD date;			// 时间
	bool changed;		// 是否改变,记录数据库用
};


#endif