#ifndef __IISSQLITE_H__
#define __IISSQLITE_H__

#include "config.h"

class CIISSQLite : public CHttpModule
{
public:
    CIISSQLite(){};
    bool execution(const char *, const char *, std::string &);

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);
};

#endif