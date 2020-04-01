
#include "ModuleStaffManagement.h"

// Create the module's class factory.
class CTVNetFactory : public IHttpModuleFactory
{
public:
    HRESULT
    GetHttpModule(
        OUT CHttpModule **ppModule,
        IN IModuleAllocator *pAllocator)
    {
        UNREFERENCED_PARAMETER(pAllocator);

        // Create a new instance.
        CIISTVManagement *pModule = new CIISTVManagement;

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
        new CTVNetFactory,
        RQ_EXECUTE_REQUEST_HANDLER | RQ_PRE_EXECUTE_REQUEST_HANDLER, // | RQ_SEND_RESPONSE,
        0);
}