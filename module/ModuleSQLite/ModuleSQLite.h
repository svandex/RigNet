#ifndef __MODULESQLITE_H__
#define __MODULESQLITE_H__

#include "HeaderPrecompilation.h"

class CIISSQLite : public CHttpModule, public ModuleBase
{
public:
    CIISSQLite(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);

    REQUEST_NOTIFICATION_STATUS OnPreExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *)
    {
        _pHttpContext = pHttpContext;
        return RQ_NOTIFICATION_CONTINUE;
    }
};

#endif