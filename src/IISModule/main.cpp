#include "precomp.h"

//Global server instance
IHttpServer2 *g_pHttpServer = nullptr;

//Global module context id
PVOID g_pModuleContext = nullptr;

//The RegisterModule entrypoint
HRESULT __stdcall RegisterModule(
	DWORD dwServerVersion,
	IHttpModuleRegistrationInfo2 *pModuleInfo,
	IHttpServer2 *pHttpServer)
{
	HRESULT hr = S_OK;
	if (pModuleInfo == nullptr || pHttpServer == nullptr)
	{
		return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
	}

	//save IHttpServer and the module context id for future use
	g_pModuleContext = pModuleInfo->GetId();
	g_pHttpServer = pHttpServer;

	//create the module factory
	CMyHttpModuleFactory *pFactory = new CMyHttpModuleFactory();
	if (pFactory == nullptr)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	}

	//register for server events
	hr = pModuleInfo->SetRequestNotifications(pFactory, RQ_BEGIN_REQUEST, 0);
	if (FAILED(hr))
	{
		if (pFactory != nullptr)
		{
			delete pFactory;
			pFactory = nullptr;
			return hr;
		}
	}
	return hr;
}