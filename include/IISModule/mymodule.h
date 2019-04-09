#ifndef __MYMODULE_H__
#define __MYMODULE_H__

#include "precomp.h"

//the module implementation

class CMyHttpModule : public CHttpModule
{
public:
	//memeber function as event handler
	REQUEST_NOTIFICATION_STATUS OnAcquireRequestState(
		IN IHttpContext4 *pHttpContext,
		IN OUT IHttpEventProvider *pProvider);
};
#endif