/*
Common Tools Header file, Svandex.h

Author: Svandex
Mail: svandexcn@icloud.com

Common Tools Header file, only containing platform headers and STL headers.

This file won't contain third party dependent file, and only contains what 
I commonly use in daily work.

It only support Windows right now ,Linux support is on the way.

macro:

SAE_WEBSOCKET				websocket support in IIS
SAE_JSON					json support
SAE_SQL						sql support

*/

#define SAE_WEBSOCKET
#define SAE_JSON
#define SAE_SQL


#pragma once

#include "Windows.h"
#include "pathcch.h"

#ifdef SAE_WEBSOCKET
#include "httpserv.h"
#include "httptrace.h"
#include "iiswebsocket.h"
#endif

//Windows
#include <string>
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

	/*
	Use IHttpStoredContext to manage variables which lifetime is related to IHttpContext
	This method could replace singleton mode and is thread-safe.
	*/
#define SVANDEX_STOREDCONTEXT L"Svandex_Implemented_StoredContext"
#define SVANDEX_BUF_SIZE 256

	/*
	WebSocket Class which holds variables related to IWebSocketContext function arguments,
	This class would be cleaned up when the request is end, then IIS would call `CleanupStoredContext`
	instead of WebSocket Class's own destructor to desctruct related variables.
	*/
	class WebSocket :public IHttpStoredContext {
		/*
		Singleton class, used to implement WebSocket Protocol


		*/
	public:
		typedef std::function<HRESULT(std::vector<char>&, std::vector<char>&)> WebSocketFunctor;

		//Constructor
		WebSocket() = delete;
		WebSocket(IHttpServer *is, IHttpContext *ic, WebSocketFunctor wsf, DWORD bufsize = SVANDEX_BUF_SIZE);

		//move operator
		WebSocket& operator=(WebSocket&&);

		//state machine
		HRESULT StateMachine();

		//deleter
		void CleanupStoredContext(){
			m_HttpContext = nullptr;
			m_HttpServer = nullptr;
			m_opreation_functor = nullptr;
			delete this;
		}

	public:
		//buffer for one operation
		std::vector<char> m_read_once;
		std::vector<char> m_writ_once;
		std::vector<char> m_buf;

		//number of bytes in each read operation
		DWORD m_buf_size;
		//functor to operate after websocket read has finished
		WebSocketFunctor m_opreation_functor;

		/*
		IIS variable
		*/
		//IHttp- variable
		IHttpServer* m_HttpServer;
		//http context
		IHttpContext* m_HttpContext;

		/*
		WebSocket State Machine Thread Management
		*/
		std::condition_variable m_cv;
		std::mutex m_pub_mutex;

		//contiune state machine
		BOOL m_sm_cont = FALSE;
		//exit state machine
		BOOL m_close = FALSE;

	private:
		void reset_arguments() {
			/*
			IWebSocketContext readfragment argument
			*/
			BOOL m_fisutf8 = TRUE;
			BOOL m_fconnectionclose = FALSE;
			BOOL m_fCompletionExpected = FALSE;
			BOOL m_ffinalfragment = FALSE;
		};

	private:
		/*
		IWebSocketContext readfragment argument
		*/
		BOOL m_fisutf8;
		BOOL m_fconnectionclose;
		BOOL m_fCompletionExpected;
		BOOL m_ffinalfragment;

	};

	namespace functor {
		//State Machine Async Function
		void WINAPI ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
		void WINAPI WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);

		//Supplementaly
		void WINAPI fNULL(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose);
	}

#endif

#ifdef SAE_JSON
#define SVANDEX_RAPIDJSON "RAPIDJSON"
	namespace json {
		std::string ErrMess(const char* _Mess, const char* _Type = SVANDEX_RAPIDJSON) {
			return (std::string("{\"") + _Type + "\":\"" + _Mess + "\"}");
		}
	}
#endif

#ifdef SAE_SQL
#endif
}