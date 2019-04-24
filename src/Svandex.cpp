#include "Svandex.h"

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

Svandex::WebSocket::WebSocket(IHttpServer *is, IHttpContext *ic)
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
	}


	//variable member initialization
	m_read_bytes = 10;

	//flags
	m_fisutf8 = TRUE;
	m_ffinalfragment = FALSE;
	m_fconnectionclose = FALSE;
}

void Svandex::WebSocket::init_read_flags() {
	m_fisutf8 = TRUE;
	m_fconnectionclose = FALSE;
}

HRESULT Svandex::WebSocket::run()try {
	HRESULT hr = S_OK;
	BOOL fReadSuccess = TRUE;
	DWORD read_bytes = 0;

	//WebSocket member initialization
	m_buf.reserve(m_read_bytes);
	m_fCompletionExpected = FALSE;
	//LOOP
	while (TRUE) {
		//read operation
		hr = piwc->ReadFragment(m_buf.data(), &read_bytes, TRUE, &m_fisutf8, &m_ffinalfragment, &m_fconnectionclose, Svandex::functor::ReadAsyncCompletion, this, &m_fCompletionExpected);

		if (m_fconnectionclose) {
			piwc->CloseTcpConnection();
			break;
		}

		if (m_ffinalfragment) {
			if (m_fconnectionclose) {
				piwc->CloseTcpConnection();
				break;
			}
			//After read operation finished, do your own work here
			m_writ_once.push_back('h');
			m_writ_once.push_back('e');
			m_writ_once.push_back('l');
			m_writ_once.push_back('l');
			m_writ_once.push_back('o');

			//write operation
			DWORD writ_bytes = m_writ_once.size();
			hr = piwc->WriteFragment(m_writ_once.data(), &writ_bytes, TRUE, TRUE, TRUE, Svandex::functor::WritAsyncCompletion, this, &m_fCompletionExpected);

			//final stage
			m_read_once.clear();
			m_writ_once.clear();
			m_buf.clear();
			m_ffinalfragment = FALSE;
		}
	}
	return hr;
}
catch (std::exception &e) {
	auto ewhat = e.what();
	HRESULT hr;
	hr = HRESULT_FROM_WIN32(ERROR_OPERATION_IN_PROGRESS);
	return hr;
}

void WINAPI Svandex::functor::ReadAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
	if (!FAILED(hr) && cbio > 0) {
		if (pws->m_read_once.capacity() - pws->m_read_once.size() < cbio) {
			pws->m_read_once.reserve(pws->m_read_once.capacity() + pws->m_read_bytes);
		}
		//merge m_buf to m_read_once
		pws->m_read_once.insert(pws->m_read_once.end(), pws->m_buf.begin(), pws->m_buf.end());
	}
}

void WINAPI Svandex::functor::WritAsyncCompletion(HRESULT hr, PVOID completionContext, DWORD cbio, BOOL fUTF8Encoded, BOOL fFinalFragment, BOOL fClose) {
	Svandex::WebSocket* pws = (Svandex::WebSocket*)completionContext;
}