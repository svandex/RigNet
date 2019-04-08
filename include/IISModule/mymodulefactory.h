#ifndef __MYMODULEFACTORY_H__
#define __MYMODULEFACTORY_H__

#include "precomp.h"

//factory class for CMyHttpModule

class CMyHttpModuleFactory : public IHttpModuleFactory
{
  public:
	virtual HRESULT GetHttpModule(
		OUT CHttpModule **ppModule,
		IN IModuleAllocator *)
	{
		HRESULT hr = S_OK;
		CMyHttpModule *pModule = nullptr;
		if (ppModule == nullptr)
		{
			return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		}
		pModule = new CMyHttpModule();
		if (pModule == nullptr)
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
		}

		*ppModule = pModule;
		pModule = nullptr;

		return hr;
	}

	virtual void Terminate()
	{
		delete this;
	}
};
#endif