#define _WINSOCKAPI_
#include <httpserv.h>

class MyGlobalModule : public CGlobalModule
{
public:

    GLOBAL_NOTIFICATION_STATUS
    OnGlobalPreBeginRequest(IN IPreBeginRequestProvider * pProvider)
    {
        HRESULT hr = S_OK;
        IHttpContext* pHttpContext = pProvider->GetHttpContext();
        IHttpResponse * pHttpResponse = pHttpContext->GetResponse();

        if (pHttpResponse != NULL)
        {
            HTTP_DATA_CHUNK dataChunk1;
            HTTP_DATA_CHUNK dataChunk2;
            
            pHttpResponse->Clear();

            int BUFFERLENGTH = 256;

            //char szBuffer[BUFFERLENGTH];
            //char szBuffer2[BUFFERLENGTH];   

            char* szBuffer = (char *) pHttpContext->AllocateRequestMemory(BUFFERLENGTH);
            char* szBuffer2 = (char *) pHttpContext->AllocateRequestMemory(BUFFERLENGTH);

            dataChunk1.DataChunkType = HttpDataChunkFromMemory;
            strcpy_s(szBuffer, 255, "Hello world!!!\r\n");

            dataChunk1.FromMemory.pBuffer = (PVOID) szBuffer;
            dataChunk1.FromMemory.BufferLength = (ULONG) strlen(szBuffer);
            hr = pHttpResponse->WriteEntityChunkByReference( &dataChunk1, -1);

            if (FAILED(hr))
            {
                pProvider->SetErrorStatus( hr );                
                return GL_NOTIFICATION_HANDLED;
            }

            dataChunk2.DataChunkType = HttpDataChunkFromMemory;
            wchar_t wstrTest1[] = L"안녕하세요"; 
            int encodedStrLen = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wstrTest1, -1, szBuffer2, BUFFERLENGTH, NULL, NULL);

            dataChunk2.FromMemory.pBuffer = (PVOID) szBuffer2;
            dataChunk2.FromMemory.BufferLength = encodedStrLen;
            hr = pHttpResponse->WriteEntityChunkByReference( &dataChunk2, -1);

            if (FAILED(hr))
            {
                pProvider->SetErrorStatus( hr );                
                return GL_NOTIFICATION_HANDLED;
            }
            return GL_NOTIFICATION_HANDLED;
        }

        return GL_NOTIFICATION_CONTINUE;
    }

    VOID Terminate()
    {
        delete this;
    }

    MyGlobalModule()
    {
    }

    ~MyGlobalModule()
    {
    }
};

HRESULT
__stdcall
RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo * pModuleInfo,
    IHttpServer * pGlobalInfo
)
{
    UNREFERENCED_PARAMETER( dwServerVersion );
    UNREFERENCED_PARAMETER( pGlobalInfo );

    MyGlobalModule * pGlobalModule = new MyGlobalModule;

    if (NULL == pGlobalModule)
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    return pModuleInfo->SetGlobalNotifications(
        pGlobalModule, GL_PRE_BEGIN_REQUEST );
}