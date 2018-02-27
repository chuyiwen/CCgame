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
// �ɾ���Ŀ
//-----------------------------------------------------------------------------
const UINT16 MAX_TITLE_NUM			= 66;


//-----------------------------------------------------------------------------
// �ɾ�ID�Ƿ���Ч
//-----------------------------------------------------------------------------
#define TITLEID_VALID( id ) ((id) >= 0 && (id) < MAX_TITLE_NUM)

//-----------------------------------------------------------------------------
// �����¼�
//-----------------------------------------------------------------------------
enum E_title_event
{	
	ete_null						=-1,
	ete_begin						=0 ,	

	ete_kill_monster				=0 ,	// ��ɱ����
	ete_kill_npc					=1 ,	// ��ɱNPC
	ete_kill_boss					=2 ,	// ��ɱBoss
	ete_kill_role					=3 ,	// ��ɱ��ɫ
	ete_quest_complete				=4 ,	// �������
	ete_quest_failed				=5 ,	// ����ʧ��
	ete_role_die					=6 ,	// ��ɫ����
	ete_role_skilled_by_role		=7 ,	// ��ɫ��������ɫ��ɱ
	ete_composite_equip_success		=8 ,	// �ϳ�װ���ɹ�
	ete_composite_item_success		=9 ,	// �ϳ���Ʒ�ɹ�
	ete_strengthen_equip_success	=10,	// ǿ��װ���ɹ�
	ete_strengthen_equip_failed		=11,	// ǿ��װ��ʧ��
	ete_strengthen_equip_perfect	=12,	// ǿ��װ������
	ete_identify_blue_equip			=13,	// ��������ɫ����ɫ����Ʒ��
	ete_stall						=14,	// ��̯
	ete_gather						=15,	// �ɼ����ջ�
	ete_shout						=16,	// ʹ�ô���
	ete_role_transaction_success	=17,	// ���ɫ�ɹ�����
	ete_create_faction				=18,	// ��������
	ete_be_castllan					=19,	// ��Ϊ����
	ete_friend_make					=20,	// ��Ϊ����
	ete_marry_join					=21,	// ���
	ete_marry_separation			=22,	// ���
	ete_use_item					=23,	// ��ɫʹ�õ���
	ete_use_skill					=24,	// ��ɫʹ�ü���
	ete_join_famehall				=25,	// ��ɫ����ĳ��������
	ete_role_level					=26,	// ��ɫ�ﵽ����
	ete_repute_change				=27,	// �����仯
	ete_friend_value				=28,	// �Ѻöȱ仯

	ete_max_event_num					=29,	// �¼���Ŀ
	ete_end								=ete_max_event_num,
};


//-----------------------------------------------------------------------------
// �ƺ���������
//-----------------------------------------------------------------------------
enum e_condition_type
{
	ect_count						= 0,	// ��������
	ect_value						= 1,	// ��ֵ����
	ect_check						= 2,	// ����������
};

//-----------------------------------------------------------------------------
// ������鷵��ֵ
//-----------------------------------------------------------------------------
enum e_check_result
{
	ecr_active						= 0,	// ����
	ecr_count_down					= 1,	// ����
	ecr_noafect						= 2,	// ��Ӱ��
};


//-----------------------------------------------------------------------------
// �ƺ���������
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
// ��������
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
// ��ֵ����
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
// ���Ӽ������
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
// �ɾ�ԭ��
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

// �����еĳɾ�
struct CriteriaProgress
{
	DWORD date;			// ʱ��
	DWORD counter;		// ����
	bool changed;		// �Ƿ�ı�,��¼���ݿ���
	bool timedCriteriaFailed;
};


// ����ɵĳɾ�
struct CompletedAchievementData
{
	DWORD date;			// ʱ��
	bool changed;		// �Ƿ�ı�,��¼���ݿ���
};


#endif