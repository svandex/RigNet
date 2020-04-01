#ifndef __IISTVMGT_H__
#define __IISTVMGT_H__

#include "HeaderPrecompilation.h"

class CIISTVManagement : public CHttpModule, public ModuleBase
{
public:
    CIISTVManagement(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);

    REQUEST_NOTIFICATION_STATUS OnPreExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *)
    {
        _pHttpContext = pHttpContext;
        return RQ_NOTIFICATION_CONTINUE;
    }
};

#endif