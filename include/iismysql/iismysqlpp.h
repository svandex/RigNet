#ifndef __IISMYSQLPP_H__
#define __IISMYSQLPP_H__

#include "config.h"

class CMYSQL : public CHttpModule
{
public:
    CMYSQL(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);
};
#endif