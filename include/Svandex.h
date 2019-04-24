/*
Common Tools Header file, Svandex.h

Author: Svandex
Mail: svandex@icloud.com

macro:

SAE_WEBSOCKET				websocket support in IIS

*/

#define SAE_WEBSOCKET


#pragma once

#include <string>

#include "Windows.h"
#include "pathcch.h"

#ifdef SAE_WEBSOCKET
#include "httpserv.h"
#include "iiswebsocket.h"
#endif

#include <vector>
#include <future>
#include <mutex>
#include <functional>
#include <chrono>

namespace Svandex{
    namespace tools{
    //get current path of executable file
    std::string GetCurrentPath();
    std::string GetEnvVariable(const char* pEnvName);
    }

#ifdef SAE_WEBSOCKET
	class WebSocket{
		typedef std::function<HRESULT(std::vector<char>&, std::vector<char>&)> WebSocketFunctor;
	public:
		WebSocket(IHttpServer *is, IHttpContext *ic, WebSocketFunctor wsf);
		//ReadAsync and WriteAsync share the same CompletionContext
		HRESULT run();
		IWebSocketContext* pWebSocketContext() {
			return piwc;
		}
		~WebSocket() {
			piwc = nullptr;
		}

		//WebSocket Read Once
		HRESULT readonce();

	public:
		//buffer for one operation
		std::vector<char> m_read_once;
		std::vector<char> m_writ_once;
		std::vector<char> m_buf;

		//number of bytes in each read operation
		DWORD m_read_bytes;
		//functor to operate after websocket read has finished
		WebSocketFunctor m_opreation_functor;
		//finish
		BOOL m_ffinish;

	private:
		//IHttpServer will handle cleaness of IHttpStoredContext
		IWebSocketContext *piwc = nullptr;


		//IHttp- variable
		IHttpServer* m_HttpServer;
		IHttpContext* m_HttpContext;

		//read flags
		BOOL m_fisutf8;
		BOOL m_fconnectionclose;

		//whether async functor has been completed
		BOOL m_fCompletionExpected;
		BOOL m_ffinalfragment;
    };

	namespace functor {
		void WINAPI ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
		void WINAPI WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
	}

#endif
}