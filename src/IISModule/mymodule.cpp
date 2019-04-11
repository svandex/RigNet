#include "precomp.h"

REQUEST_NOTIFICATION_STATUS CMyHttpModule::OnBeginRequest(
    IN IHttpContext *pHttpContext,
    IN IHttpEventProvider *pProvider)
{
    HRESULT hr = S_OK;
    std::fstream fd("C:/Users/saictv/Desktop/test.txt", std::ios::out | std::ios::app);
    fd << "success" << std::endl;
    if (FAILED(hr))
    {
        return RQ_NOTIFICATION_FINISH_REQUEST;
    }
    else
    {
        return RQ_NOTIFICATION_CONTINUE;
    }
}