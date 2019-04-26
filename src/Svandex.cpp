#include "Svandex.h"
IWebSocketContext* Svandex::WebSocket::piwc = nullptr;
Svandex::WebSocket* Svandex::WebSocket::m_pself = nullptr;
std::mutex Svandex::WebSocket::m_mutex;

std::string Svandex::tools::GetCurrentPath()
{
    TCHAR dest[MAX_PATH];
    if (!dest)
        return std::string();

    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
        DWORD length = GetModuleFileName(hModule, dest, MAX_PATH);
    }
    else
    {
        return std::string();
    }

#if (NTDDI_VERSION >= NTDDI_WIN8)
    PathCchRemoveFileSpec(dest, MAX_PATH);
#else
    PathRemoveFileSpec(dest);
#endif
    size_t len = wcslen(dest) + 1;
    size_t converted = 0;
    char *destStr = (char *)malloc(len * sizeof(char));
    wcstombs_s(&converted, destStr, len, dest, _TRUNCATE);
    auto returnStr=std::string(destStr);
    free(destStr);
    return returnStr;
}

std::string Svandex::tools::GetEnvVariable(const char *pEnvName)
{
    std::vector<std::string> vbuf;
    char* buf[MAX_PATH];
    size_t buf_num;
    _dupenv_s(buf, &buf_num, pEnvName);
    auto return_value = std::string(*buf);
    return std::string(*buf);
}

Svandex::WebSocket* Svandex::WebSocket::getInstance(IHttpServer *is, IHttpContext *ic, Svandex::WebSocket::WebSocketFunctor wsf) {
	std::lock_guard<std::mutex> lk(m_mutex);
	if (!m_pself) {
		m_pself = new WebSocket(is, ic, wsf);
	}
	return m_pself;
}

Svandex::WebSocket::WebSocket(IHttpServer *is, IHttpContext *ic, Svandex::WebSocket::WebSocketFunctor wsf)
	:m_HttpServer(is), m_HttpContext(ic) {
	IHttpContext3 *pHttpContext3;
	HRESULT hr = S_OK;
	if (!piwc) {
		IHttpResponse *pHttpResponse = ic->GetResponse();
		pHttpResponse->ClearHeaders();
		pHttpResponse->SetStatus(101, "Switching Protocols");
		hr = pHttpResponse->Flush(TRUE, TRUE, NULL);
		if (!FAILED(hr)) {//FLUSH Successfully
			hr = HttpGetExtendedInterface(is, ic, &pHttpContext3);
			piwc = (IWebSocketContext*)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
			m_fconnectionclose = TRUE;
			if (!piwc) {
				throw std::exception("Cannot get IWebSocketContext Pointer.");
			}
		}
		//variable member initialization
		m_read_bytes = 10;
		m_opreation_functor = wsf;

		//flags
		m_fisutf8 = TRUE;
		m_ffinalfragment = FALSE;
		m_fconnectionclose = FALSE;
		m_fCompletionExpected = FALSE;

		//buf initialization
		m_buf.reserve(m_read_bytes);
	}
	//TODO: set named context into IHttpContext, easy to manage memory
}

HRESULT Svandex::WebSocket::StateMachine() {
	/*
	Main Thread
	Controlling read cycle, need syncronization from readAsycn and writAsyc thread
	*/
	DWORD read_bytes = m_read_bytes;
	m_buf.resize(m_read_bytes);
	HRESULT hr = piwc->ReadFragment(m_buf.data(), &read_bytes, TRUE, &m_fisutf8, &m_ffinalfragment, &m_fconnectionclose, Svandex::functor::ReadAsyncCompletion, this, &m_fCompletionExpected);
	while (TRUE) {
		std::unique_lock<std::mutex> lk(m_pub_mutex);
		m_cv.wait(lk, [this] {
			return m_sm_cont == TRUE;
			});
		m_sm_cont = FALSE;
		if (m_close) {
			break;
		}
		read_bytes = m_read_bytes;
		m_buf.resize(m_read_bytes);
		HRESULT hr = piwc->ReadFragment(m_buf.data(), &read_bytes, TRUE, &m_fisutf8, &m_ffinalfragment, &m_fconnectionclose, Svandex::functor::ReadAsyncCompletion, this, &m_fCompletionExpected);
	}
	m_promise.set_value(TRUE);
	return hr;
}

void WINAPI Svandex::functor::ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	/*
	Read Async Thread
	Will be closed after return
	*/
	if (completionContext == nullptr) {
		return;
	}
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
	if (FAILED(hr)) {
		pws->m_sm_cont = TRUE;
		pws->m_close = TRUE;
		pws->m_cv.notify_all();
		return;
	}
	else {
		if (fClose) {
			pws->m_sm_cont = TRUE;
			pws->m_close = TRUE;
			pws->m_cv.notify_all();
			return;
		}
		HRESULT hrac;
		if (cbio > 0) {
			pws->m_read_once.reserve(pws->m_read_once.size() + cbio);
			//merge m_buf to m_read_once
			pws->m_read_once.insert(pws->m_read_once.end(), pws->m_buf.begin(), pws->m_buf.begin() + cbio);
		}
		while (!fFinalFragment) {
			DWORD t_writ = 0;
			BOOL t_fce = FALSE;
			hrac = pws->pWebSocketContext()->WriteFragment((PVOID)"noasync", &t_writ, TRUE, TRUE, TRUE, Svandex::functor::fNULL, NULL, &t_fce);

			//read again
			cbio = 10;
			pws->m_buf.resize(pws->m_read_bytes);
			hrac = pws->pWebSocketContext()->ReadFragment(pws->m_buf.data(), &cbio, TRUE, &fUTF8Encoded, &fFinalFragment, &fClose, Svandex::functor::fNULL, NULL, &t_fce);

			//has read
			if (cbio > 0) {
				pws->m_read_once.reserve(pws->m_read_once.size() + cbio);
				//merge m_buf to m_read_once
				pws->m_read_once.insert(pws->m_read_once.end(), pws->m_buf.begin(), pws->m_buf.begin() + cbio);
			}
			if (FAILED(hrac)) {
				pws->m_sm_cont = TRUE;
				pws->m_close = TRUE;
				pws->m_cv.notify_all();
				return;
			}
		}//while

		pws->m_writ_once.clear();
		hrac = pws->m_opreation_functor(pws->m_read_once, pws->m_writ_once);
		DWORD writ_bytes = pws->m_writ_once.size();
		hrac = pws->pWebSocketContext()->WriteFragment(pws->m_writ_once.data(), &writ_bytes, TRUE, TRUE, TRUE, Svandex::functor::WritAsyncCompletion, pws);
		//TODO:notify main thread that read has finished and needs another readonce
	}
}

void WINAPI Svandex::functor::WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
	std::lock_guard<std::mutex> lk(pws->m_pub_mutex);
	if (FAILED(hr)) {
		pws->m_sm_cont = TRUE;
		pws->m_close = TRUE;
		pws->m_cv.notify_all();
		return;
	}
	pws->m_read_once.clear();
	//TODO:notify main thread that write has finished
	pws->m_sm_cont = TRUE;
	pws->m_cv.notify_all();
}

void WINAPI Svandex::functor::fNULL(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
}
