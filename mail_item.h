#pragma once
#include "container_template.h"
#include "../../common/WorldDefine/ItemDefine.h"


typedef Container<tagItem> MailItemContainer;

class mail_item : public MailItemContainer
{
public:
	mail_item(void);
	~mail_item(void);
};
