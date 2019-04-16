#include "precomp.h"
// Create the module class.
class CRigNet : public CHttpModule
{
public:
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	//REQUEST_NOTIFICATION_STATUS OnEndRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	//REQUEST_NOTIFICATION_STATUS OnBeginRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	//REQUEST_NOTIFICATION_STATUS OnReleaseRequestState(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnPostEndRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnEndRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
};