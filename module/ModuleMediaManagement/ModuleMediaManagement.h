#ifndef __MODULEMEDIAMANAGEMENT_H__
#define __MODULEMEDIAMANAGEMENT_H__

#include "HeaderPrecompilation.h"
#include "httpcach.h"

class CIISTVMedia : public CHttpModule, public ModuleBase
{
public:
    CIISTVMedia(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);

    REQUEST_NOTIFICATION_STATUS OnPreExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *)
    {
        _pHttpContext = pHttpContext;
        return RQ_NOTIFICATION_CONTINUE;
    }
};

#endif