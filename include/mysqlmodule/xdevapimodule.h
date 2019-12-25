#ifndef __MYSQLMODULE_H__
#define __MYSQLMODULE_H__

#include "precomp.h"

class CMYSQL : public CHttpModule
{
public:
    CMYSQL(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);
};
#endif