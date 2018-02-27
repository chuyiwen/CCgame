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
 *	@brief		����̬����
*/

#ifndef __QUEST_MGR_H__

#define __QUEST_MGR_H__

struct tagQuestProto;
class Creature;

struct circle_quest_elem
{
	UINT16 min_level;
	UINT16 max_level;
	UINT16 rand_number;
	package_map<UINT16, UINT16> quests;
};

struct tagTeamShareCirle
{
	INT16 serial;
	INT16 rand_number;
	std::vector<UINT16> ids;
};

struct tagTeamShareConfig
{
	UINT16 teamavgLevelMin;
	UINT16 teamavgLevelMax;
	std::vector<tagTeamShareCirle*> circls;
	~tagTeamShareConfig()
	{
		size_t size = circls.size();
		for(size_t n = 0; n < size; n++)
			SAFE_DELETE(circls[n]);
	}

};

class QuestMgr
{
public:
	typedef package_map<UINT16, tagQuestProto*>			QUESTPROTO;
	typedef package_map<DWORD, package_list<UINT16>*>	NPCREFQUEST;
	typedef package_map<UINT16, package_list<DWORD>*>	QUESTREFNPC;
	typedef map<string, string>				XMLNODEMAP;
	typedef std::vector<circle_quest_elem*>	CIRCLEQUESTVECTOR;
	typedef CIRCLEQUESTVECTOR::iterator		CIRCLEQUESTITER;

	typedef std::vector<tagTeamShareConfig*>	TEAMSHAREVECTOR;
	typedef TEAMSHAREVECTOR::iterator		TEAMSHARETITER;

public:
	// ��ʼ��
	BOOL init();

	// ����
	VOID destroy();
public:
	// �Ƿ��ܴӴ�NPC��ȡ������
	BOOL can_accept_from_npc(Creature* p_creature, UINT16 u16_quest_id);

	// �Ƿ��ܴӴ�NPC��ɸ�����
	BOOL can_complete_from_npc(Creature* p_creature, UINT16 u16_quest_id);
public:
	// �������Ƿ���Ҫ��������NPC
	BOOL is_need_complete_npc(UINT16 u16_quest_id);

	// �������Ƿ���Ҫ��ȡ�����NPC
	BOOL is_need_accept_npc(UINT16 u16_quest_id);
public:
	// ��̬����
	const tagQuestProto* get_protocol(UINT16 u16_quest_id) { return quest_protos_.find(u16_quest_id); }

	// ���ݵȼ�ȡ��ѭ����������б�����������������
	package_map<UINT16,UINT16>* get_circle_quest( int n_level, UINT16& u16_rand );
	const std::vector<tagTeamShareCirle*>* get_team_share_circle_quest( int n_level);//mwh 2011-09-06
public:
	// ��ָ��Ŀ¼��ȡ����ṹ
	BOOL read_all_quest(LPCTSTR sz_path);
private:
	// ��ȡ����̬����
	DWORD	read_one_quest_from_file(tagQuestProto* p_proto, LPCTSTR sz_file);
	BOOL	read_npc_ref_accept_quest(LPCTSTR sz_file);
	BOOL	read_npc_ref_complete_quest(LPCTSTR sz_file);
	BOOL	read_xml_node(XmlElement* p_node, XMLNODEMAP& node_map);
	BOOL	read_cicle_quest_from_file(LPCTSTR szFileName);
	BOOL	read_team_share_cicle_quest_from_file(LPCTSTR szFileName);//mwh 2011-09-06
	VOID	read_one_circle_quest_from_file(XmlElement* pNode, CIRCLEQUESTVECTOR& v);//
	VOID	read_one_Circle_from_file(XmlElement* pNode, tagTeamShareConfig* pCfg);
	VOID	read_one_team_share_circle_quest_from_file(XmlElement* pNode, tagTeamShareCirle* pCircle);//
private:
	BOOL	set_protocol(tagQuestProto* p_proto, XMLNODEMAP& node_map);
	VOID	set_DWORD( const string& str, DWORD& dw,XMLNODEMAP& node_map);
	VOID	set_DWORD_flag(const string& str, DWORD& dw, INT32 n_mask, XMLNODEMAP& node_map);
	VOID	set_uint16(const string& str, UINT16& un16,XMLNODEMAP& node_map);
	VOID	set_byte(const string& str, BYTE& by,XMLNODEMAP& node_map);
	VOID	set_int16(const string& str, INT16& n16,XMLNODEMAP& node_map);
	VOID	set_int32(const string& str, INT32& n32,XMLNODEMAP& node_map);
	VOID	set_BOOL(const string& str, BOOL& b,XMLNODEMAP& node_map);
	VOID	set_float(const string& str, FLOAT& f,XMLNODEMAP& node_map);	

private:
	// ��������̬����
	QUESTPROTO quest_protos_;	

	// NPC -> �ɽ������б�
	NPCREFQUEST	npc_ref_accept_quest_;

	// NPC -> ����������б�
	NPCREFQUEST	npc_ref_complete_quest_;

	// ���� -> ���������NPC
	QUESTREFNPC	quest_ref_complete_npc_;

	// ���� -> �ɽ�ȡ����NPC
	QUESTREFNPC	quest_ref_accept_npc_;

	// ���������
	CIRCLEQUESTVECTOR circle_quests_;
	TEAMSHAREVECTOR	  team_share_circle_;//mwh 2011-09-06

	// ���������ļ�����Ŀ¼
	tstring path_;

	// �����б��ļ�
	tstring	files_;

	// ����������б��ļ�
	tstring	file_circle_quest_;
	tstring	file_team_share_circle_quest_;//mwh 2011-09-06

	// ����->��ȡNPC�����ļ�
	tstring	file_quest_ref_accept_npc_;	

	// ����->���NPC�����ļ�
	tstring	file_quest_ref_complete_npc_;	
};

extern QuestMgr g_questMgr;

#endif //__QUEST_MGR_H__
