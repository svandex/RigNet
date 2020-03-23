#ifndef __IISTVMDA_H__
#define __IISTVMDA_H__

#include "config.h"


class CIISTVMedia : public CHttpModule
{
public:
    CIISTVMedia(){};

    REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *, IN IHttpEventProvider *);
};

#endif