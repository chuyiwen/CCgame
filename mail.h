#pragma once
#include "mail_item.h"
#include "../../common/WorldDefine/mail_define.h"
#include "../common/ServerDefine/mail_server_define.h"
#include "../common/ServerDefine/mail_server_define.h"

class mail
{
public:
	mail(void);
	~mail(void);

public:

	const tagMail& GetMailAtt() const;

	VOID InitMail(s_load_mail* pLoadMail);

	VOID InitContent2DB();

	VOID InitItem2DB();

	VOID InitContent(tag_mail_content* p_mail_content);

	VOID InitItem(tagItem* pItem);

	VOID CreateMail(DWORD dwSerial, tstring& stName, tstring& stContent, tagMailBase& stMailBase, DWORD* dwItemTypeID = NULL, INT16* n16Qlty = NULL, INT* n_num = NULL, INT nItemNum = 0, BOOL bBind = TRUE);

	VOID CreateMail(DWORD dwSerial, tstring& stName, tstring& stContent, tagMailBase& stMailBase, tagItem* pItem[], INT nItemNum=1);

	VOID Update();

	VOID add_item(DWORD dw_role_id, INT64& n64ItemSerial);

	VOID add_item(tagItem* pItem);

	VOID save_update_item_to_db();

	VOID SaveMail2DB();

	VOID SaveUpdateMail2DB();

	VOID DelMail2DB();

	VOID SetSendMailTime();

	VOID GetSendItemNum(INT& nItemNum);

	VOID DeleteItemID(INT64 n64_id);

	VOID CheckSendMail();

	VOID CheckWithdrawalMail();

	VOID CheckDelMail();

	VOID DelMail();

	VOID SendMail();

	BOOL IsDel() { return bDel; }

	BOOL IsReturn() { return bWithdrawal; }

	VOID SetReturn(BOOL bReturn) { bWithdrawal = bReturn; }

	tagItem* GetSpaceItem(INT16 n16Space);

	VOID SendMailContent(Role* pRole);

	VOID SendMailItem(Role* pRole);

	DWORD AcceptAccessoryMail(Role* pRole);

	VOID ChangeCmd(DWORD& dw_cmd_id);

	VOID  CleanAccessory();

	VOID ReadMail();

	DWORD DeleteMail();

	DWORD ReturnMail();

	mail_item& GetMailItem() { return m_MailItem; }
	
	DWORD GetSolve() { return m_Att.dwSolve; }
private:

	tagMail m_Att;

	mail_item m_MailItem;

	tagDWORDTime dwSendMailLastTime;		// 发送邮件最后更新时间

	tagDWORDTime dwWithdrawalLastTime;		// 退件最后更新时间

	tagDWORDTime dwDelMailLastTime;			// 删除邮件最后更新时间

	BOOL	bDel;

	BOOL	bWithdrawal;

	DWORD	dw_update_time;
};
