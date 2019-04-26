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
#include "httptrace.h"
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
	class WebSocket :public IHttpStoredContext {
		/*
		Singleton class, used to implement WebSocket Protocol


		*/
		typedef std::function<HRESULT(std::vector<char>&, std::vector<char>&)> WebSocketFunctor;
	public:
		//singleton
		static WebSocket* getInstance(IHttpServer *is, IHttpContext *ic, WebSocketFunctor wsf);

		//deleter
		void CleanupStoredContext(){
			piwc = nullptr;
			delete m_pself;
			m_pself = nullptr;
		}

		//pointer to WebSocket Context
		IWebSocketContext* pWebSocketContext() {
			return piwc;
		}

		//state machine
		HRESULT StateMachine();

	public:
		//buffer for one operation
		std::vector<char> m_read_once;
		std::vector<char> m_writ_once;
		std::vector<char> m_buf;

		//number of bytes in each read operation
		DWORD m_read_bytes;
		//functor to operate after websocket read has finished
		WebSocketFunctor m_opreation_functor;
		//http context
		IHttpContext* m_HttpContext;
		//promise
		std::promise<BOOL> m_promise;
		//condition variable
		std::condition_variable m_cv;
		//public mutex
		std::mutex m_pub_mutex;
		//read times
		DWORD m_num = 0;
		//flags
		BOOL m_readmore=FALSE;
		BOOL m_sm_cont=FALSE;
		BOOL m_close = FALSE;
	private:
		WebSocket(IHttpServer *is, IHttpContext *ic, WebSocketFunctor wsf);
	private:
		//singleton
		static WebSocket* m_pself;
		//mutex
		static std::mutex m_mutex;
		//IHttpServer will handle cleaness of IHttpStoredContext
		static IWebSocketContext *piwc;


		//IHttp- variable
		IHttpServer* m_HttpServer;

		//read flags
		BOOL m_fisutf8;
		BOOL m_fconnectionclose;
		BOOL m_fCompletionExpected;
		BOOL m_ffinalfragment;
    };

	namespace functor {
		void WINAPI ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
		void WINAPI WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);

		//Supplementaly
		void WINAPI fNULL(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
	}

#endif
}