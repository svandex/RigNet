#include "precomp.h"

REQUEST_NOTIFICATION_STATUS CRigNet::OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider)
try {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;

	// Retrieve a pointer to the request
	IHttpRequest *pHttpRequest = pHttpContext->GetRequest();
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	// Create an HRESULT to receive return values from methods.
	HRESULT hr = S_OK;

	if (pHttpRequest != NULL && pHttpResponse != NULL)
	{
		//Accept Key
		if (auto value = pHttpResponse->GetHeader("Sec-WebSocket-Accept")) {
		}

		// Test for an error.
		if (FAILED(hr))
		{
			// Set the HTTP status.
			pHttpResponse->SetStatus(500, "Server Error", 0, hr);

			// End additional processing.
			return RQ_NOTIFICATION_CONTINUE;
		}
	}
	else
	{
		// End additional processing.
		return RQ_NOTIFICATION_FINISH_REQUEST;
	}
	// Test for an error.
	/*
	if (pHttpResponse != NULL)
	{
		// Clear the existing response.
		pHttpResponse->Clear();
		// Set the MIME type to plain text.
		pHttpResponse->SetHeader(
			HttpHeaderContentType, "text/plain",
			(USHORT)strlen("text/plain"), TRUE);

		// Create a string with the response.
		PCSTR pszBuffer = "Hello World!";
		// Create a data chunk.
		HTTP_DATA_CHUNK dataChunk;
		// Set the chunk to a chunk in memory.
		dataChunk.DataChunkType = HttpDataChunkFromMemory;
		// Buffer for bytes written of data chunk.
		DWORD cbSent;

		// Set the chunk to the buffer.
		dataChunk.FromMemory.pBuffer =
			(PVOID)pszBuffer;
		// Set the chunk size to the buffer size.
		dataChunk.FromMemory.BufferLength =
			(USHORT)strlen(pszBuffer);
		// Insert the data chunk into the response.
		hr = pHttpResponse->WriteEntityChunks(
			&dataChunk, 1, FALSE, TRUE, &cbSent);

		// Test for an error.
		if (FAILED(hr))
		{
			// Set the HTTP status.
			pHttpResponse->SetStatus(500, "Server Error", 0, hr);
		}

		// End additional processing.
		return RQ_NOTIFICATION_FINISH_REQUEST;
	}
*/

// Return processing to the pipeline.
	return RQ_NOTIFICATION_CONTINUE;
}
catch (std::exception &e) {
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	HRESULT hr = E_UNEXPECTED;
	pHttpResponse->SetStatus(500, e.what(), 0, hr);

	// End additional processing.
	return RQ_NOTIFICATION_FINISH_REQUEST;
}

REQUEST_NOTIFICATION_STATUS CRigNet::OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo) {
	extern IHttpServer *g_HttpServer;
	Svandex::WebSocket* ws = Svandex::WebSocket::getInstance(g_HttpServer, pHttpContext, RigNetMain);
	ws->m_promise.get_future().wait();
	ws->pWebSocketContext()->CloseTcpConnection();
	return RQ_NOTIFICATION_CONTINUE;
}

REQUEST_NOTIFICATION_STATUS CRigNet::OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;

	Svandex::WebSocket* ws = Svandex::WebSocket::getInstance(g_HttpServer, pHttpContext, RigNetMain);
	ws->readonce();
	return RQ_NOTIFICATION_PENDING;
}

HRESULT RigNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine) {
	WebSocketWritLine.push_back('L');
	WebSocketWritLine.reserve(WebSocketWritLine.size() + WebSocketReadLine.size());
	WebSocketWritLine.insert(WebSocketWritLine.end(), WebSocketReadLine.begin(), WebSocketReadLine.end());
	return S_OK;
}