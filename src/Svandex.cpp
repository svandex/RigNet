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
	}
}

HRESULT Svandex::WebSocket::run()try {
	HRESULT hr = S_OK;

	//WebSocket member initialization
	DWORD read_bytes = m_read_bytes;
	m_buf.resize(m_read_bytes);
	//LOOP
	hr = readonce();
	if (FAILED(hr)) {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
		return hr;
	}
	return hr;
}
catch (std::exception &e) {
	auto ewhat = e.what();
	HRESULT hr;
	hr = HRESULT_FROM_WIN32(ERROR_OPERATION_IN_PROGRESS);
	return hr;
}

HRESULT Svandex::WebSocket::readonce() {
	DWORD read_bytes = m_read_bytes;
	m_buf.resize(m_read_bytes);
	HRESULT hr = piwc->ReadFragment(m_buf.data(), &read_bytes, TRUE, &m_fisutf8, &m_ffinalfragment, &m_fconnectionclose, Svandex::functor::ReadAsyncCompletion, this, &m_fCompletionExpected);
	return hr;
}

void WINAPI Svandex::functor::ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
	pws->m_num += 1;
	std::lock_guard<std::mutex> lk(pws->m_pub_mutex);
	if (FAILED(hr)) {
		pws->pWebSocketContext()->CloseTcpConnection();
		pws->m_HttpContext->IndicateCompletion(RQ_NOTIFICATION_CONTINUE);
		return;
	}
	else {
		if (fClose) {
			pws->pWebSocketContext()->CloseTcpConnection();
			pws->m_HttpContext->IndicateCompletion(RQ_NOTIFICATION_CONTINUE);
			pws->m_promise.set_value(TRUE);
			return;
		}
		HRESULT hrac;
		if (cbio > 0) {
			pws->m_read_once.reserve(pws->m_read_once.size() + pws->m_buf.size());
			//merge m_buf to m_read_once
			pws->m_read_once.insert(pws->m_read_once.end(), pws->m_buf.begin(), pws->m_buf.end());
		}
		if (fFinalFragment) {//write operation
			pws->m_writ_once.clear();
			hrac = pws->m_opreation_functor(pws->m_read_once, pws->m_writ_once);
			DWORD writ_bytes = pws->m_writ_once.size();
			pws->pWebSocketContext()->CancelOutstandingIO();
			hrac = pws->pWebSocketContext()->WriteFragment(pws->m_writ_once.data(), &writ_bytes, TRUE, TRUE, TRUE, Svandex::functor::WritAsyncCompletion, pws);
		}
		else {
			hrac = pws->readonce();
		}
		if (FAILED(hrac)) {
			pws->pWebSocketContext()->CloseTcpConnection();
			pws->m_HttpContext->IndicateCompletion(RQ_NOTIFICATION_CONTINUE);
			return;
		}
	}
}

void WINAPI Svandex::functor::WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
	if (!FAILED(hr)) {
		pws->pWebSocketContext()->SendConnectionClose(FALSE, 1000);
		pws->m_promise.set_value(TRUE);
		return;
	}
	pws->m_read_once.clear();
	pws->readonce();
}