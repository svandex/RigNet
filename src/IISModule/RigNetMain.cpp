/*
Rig Net Module In IIS

Author: Juncheng Qiu
Mail: qiujuncheng@saicmotor.com
Git: https://github.com/svandex/rignet
license: Apache

features includeing:

1. websocket protocol support

2. login support

3. mysql xdevapi support


*/
#include "precomp.h"

// Create the module's class factory.
class CRigNetFactory : public IHttpModuleFactory
{
public:
    HRESULT
    GetHttpModule(
        OUT CHttpModule **ppModule,
        IN IModuleAllocator *pAllocator)
    {
        UNREFERENCED_PARAMETER(pAllocator);

        // Create a new instance.
		CRigNet *pModule = new CRigNet;

        // Test for an error.
        if (!pModule)
        {
            // Return an error if the factory cannot create the instance.
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            // Return a pointer to the module.
            *ppModule = pModule;
            pModule = NULL;
            // Return a success status.
            return S_OK;
        }
    }

    void
    Terminate()
    {
        // Remove the class from memory.
        delete this;
    }
};

// Create the module's exported registration function.
HRESULT
__stdcall RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo *pModuleInfo,
    IHttpServer *pGlobalInfo)
{
    UNREFERENCED_PARAMETER(dwServerVersion);
    //UNREFERENCED_PARAMETER(pGlobalInfo);
    extern IHttpServer *g_HttpServer;
    g_HttpServer = pGlobalInfo;

    // Set the request notifications and exit.
    return pModuleInfo->SetRequestNotifications(
		new CRigNetFactory,
		RQ_AUTHENTICATE_REQUEST,// | RQ_SEND_RESPONSE,
        0);
}