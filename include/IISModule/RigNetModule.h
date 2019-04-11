#include "precomp.h"
// Create the module class.
class CRigNet : public CHttpModule
{
public:
    CRigNet()
    {
        google::InitGoogleLogging("RigNetServiceGlog");
        google::SetLogDestination(google::GLOG_INFO, "C:/Users/Public/Documents/glogs/");
    }
    ~CRigNet(){
        google::ShutdownGoogleLogging();
    }
    REQUEST_NOTIFICATION_STATUS
    OnBeginRequest(
        IN IHttpContext *pHttpContext,
        IN IHttpEventProvider *pProvider);
};