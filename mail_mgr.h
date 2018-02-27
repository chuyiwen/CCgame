#pragma once
#include "mutex.h"
#include "mail.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/mail_server_define.h"

typedef package_map<DWORD, mail*> MAP_MAIL;
typedef package_map<DWORD, package_map<DWORD, mail*>*> MAP_NEWMAIL;

class mail_mgr
{
public:
	mail_mgr(void);
	~mail_mgr(void);

	VOID Destroy();

	VOID InitDBLoadMail(NET_DB2S_load_all_mail* pLoadAllMail);

	VOID InitDBLoadMailContent(NET_DB2S_load_mail_content* pLeadMailContent);

	VOID InitDBLoadMailItem(LPVOID pData, INT& n_num);

	VOID InitDBMaxMailSerial(DWORD& dwSerialNum);

	VOID CreateMailSerialNum(DWORD& dwSerialNum);

	DWORD CreateMail(DWORD dwNPCID, Role* pRole, tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent);

	DWORD CreateMail(tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent, DWORD* dwItemTypeID = NULL, INT16* n16Qlty = NULL, INT* n_num = NULL, INT nItemNum = 0, BOOL bBind = TRUE);

	DWORD CreateMail(tagMailBase& stMailBase, LPCTSTR szName, LPCTSTR szContent, tagItem* pItem[], INT nItemNum=1);

	VOID  GetMailInfo(Role* pRole);

	VOID GetOneMailInfo(Role* pRole, DWORD dwMailID);

	DWORD  GetMailContent(DWORD dwMailID, Role* pRole);

	DWORD GetMailItem(DWORD dwMailID, Role*pRole);

	VOID GetRoleMailNum(INT32& n32Num, BOOL& bRead, DWORD dw_role_id);

	//MAP_MAIL& GetMailMap() { return m_mapMail; };

	mail* GetMail(DWORD dwMailSerial);

	mail* GetMail(DWORD dw_role_id, DWORD dwMailSerial);

	VOID Update();

	VOID DestoryAllMail();

	VOID ClearRoleSendMainNum();

	VOID AddRoleMail(DWORD dw_role_id, mail* pMail);

	BOOL IsNoReadMail(Role* pRole);

	VOID SetInitOK(BOOL b) { m_bInitOK = b; }

private:

	//MAP_MAIL m_mapMail;
	DWORD   m_dwMaxSerialNum;		// ÓÊ¼þ±àºÅ
	Mutex	m_Mutex;
	MAP_NEWMAIL m_mapNewMail;

	BOOL	m_bInitOK;
};

extern mail_mgr g_mailmgr;
