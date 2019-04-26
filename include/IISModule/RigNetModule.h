#include "precomp.h"
// Create the module class.
class CRigNet : public CHttpModule
{
public:
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);

	std::promise<BOOL> m_websocket_cont;
};

HRESULT RigNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);
