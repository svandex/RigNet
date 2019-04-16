#include "precomp.h"
#include "openssl/sha.h"
#include "thirdparty/base64.h"
#include "comdef.h"
#include "websocket.h"

REQUEST_NOTIFICATION_STATUS CRigNet::OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider)
try {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;
	const char* WebSocketMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	uint16_t ws_connect_times = 0;

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
			DLOG(INFO) << "Accept Key: " << value << std::endl;
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
		LOG(INFO) << "Cannot Get Request or Response Context" << std::endl;
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

	LOG(WARNING) << e.what() << std::endl;

	// End additional processing.
	return RQ_NOTIFICATION_FINISH_REQUEST;
}

void WINAPI wsCompleted(HRESULT hrError,
	VOID* pvCompletionContext,
	DWORD cbIO,
	BOOL fUTF8Encoded,
	BOOL fFinalFragment,
	BOOL fClose) {
	LOG(INFO) << "write " << cbIO << std::endl;

}

REQUEST_NOTIFICATION_STATUS CRigNet::OnEndRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;
	const char* WebSocketMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	DWORD pcbSent;
	USHORT sCode;

	// Retrieve a pointer to the request
	IHttpRequest *pHttpRequest = pHttpContext->GetRequest();
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	// Create an HRESULT to receive return values from methods.
	HRESULT hr = S_OK;
	if (pHttpRequest != NULL && pHttpResponse != NULL) {
		//First time WebSocket Connection
		LOG(INFO) << "requet info count" << pHttpRequest->GetRawHttpRequest()->RequestInfoCount;
		if (auto WS_ver = pHttpRequest->GetHeader("Sec-WebSocket-Version")) {
			LOG(INFO) << "---------------New REQUEST------------------" << std::endl;

			//IHttpContext3
		IHttpContext3 *pHttpContext3;
		hr = HttpGetExtendedInterface(g_HttpServer, pHttpContext, &pHttpContext3);

		//IWebSocketContext
		IWebSocketContext *pWebSocketContext = NULL;
		if (pHttpContext3->GetNamedContextContainer()) {
			pWebSocketContext = (IWebSocketContext *)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
			if (NULL != pWebSocketContext) {
				LOG(INFO) << "pWebSocketContext exists" << std::endl;
				pcbSent = 8;
				pWebSocketContext->GetCloseStatus(&sCode);
				LOG(INFO) << "Web SOcket Status Code" << sCode << std::endl;
				hr = pWebSocketContext->WriteFragment((void*)"hello world", &pcbSent, FALSE, FALSE, TRUE, wsCompleted);
				LOG(INFO) << "Has Write " << pcbSent << " Bytes, hr : " << hr << std::endl;
			}
			else {
				pHttpResponse->ClearHeaders();
				pHttpResponse->SetStatus(101, "Switching Protocols");
				/*
				pHttpResponse->SetHeader(HTTP_HEADER_ID::HttpHeaderUpgrade, "websocket", (USHORT)std::strlen("websocket"), TRUE);
				pHttpResponse->SetHeader(HTTP_HEADER_ID::HttpHeaderConnection, "Upgrade", (USHORT)std::strlen("Upgrade"), TRUE);
*/
				//Accept Key
				if (auto value = pHttpResponse->GetHeader("Sec-WebSocket-Accept")) {
					DLOG(INFO) << "Accept Key: " << value << std::endl;
				}
				//flush response
				pHttpResponse->Flush(FALSE, TRUE, &pcbSent);
				//Accept Key
				if (auto value = pHttpResponse->GetHeader("Sec-WebSocket-Accept")) {
					DLOG(INFO) << "Accept Key: " << value << std::endl;
				}
				LOG(INFO) << "Has Send " << pcbSent << " bytes to client" << std::endl;
				pWebSocketContext = (IWebSocketContext *)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
				if (pWebSocketContext == NULL) {
					LOG(INFO) << "still no websocket" << std::endl;
				}
				else {
					pHttpContext3->EnableFullDuplex();
					//buffer
					PBYTE buf = (PBYTE)"helloworld";
					PBYTE bufr = (PBYTE)malloc(MAX_PATH);

					// Create a string with the response.
					PCSTR pszBuffer = "Hello World!";
					// Create a data chunk.
					HTTP_DATA_CHUNK dataChunk;
					// Set the chunk to a chunk in memory.
					dataChunk.DataChunkType = HttpDataChunkFromMemory;
					// Buffer for bytes written of data chunk.
					DWORD cbSent;
					cbSent = 5;

					// Set the chunk to the buffer.
					dataChunk.FromMemory.pBuffer =
						(PVOID)pszBuffer;
					// Set the chunk size to the buffer size.
					dataChunk.FromMemory.BufferLength =
						(USHORT)strlen(pszBuffer);

					hr = g_HttpServer->AddFragmentToCache(&dataChunk, L"HWTEST");
					hr = g_HttpServer->ReadFragmentFromCache(L"HWTEST", bufr, 10, &cbSent);

					//var
					pcbSent = 6;
					BOOL utf8Encoded;
					BOOL finalFragment;
					BOOL ConnectionCLose;
					BOOL cmpl;
					time_t startT = time(NULL);
					while (time(NULL) - startT < 10) {
						pWebSocketContext->GetCloseStatus(&sCode);
						hr = pWebSocketContext->WriteFragment(bufr, &pcbSent, FALSE, FALSE, FALSE, wsCompleted,NULL,&cmpl);
						LOG(INFO) << "IF completed :" << cmpl << std::endl;
						LOG(INFO) << "Has Write " << pcbSent << " Bytes,hr is " << hr << ",write " << buf << std::endl;
					}
					pcbSent = 6;
					hr = pWebSocketContext->ReadFragment(bufr, &pcbSent, false, &utf8Encoded, &finalFragment, &ConnectionCLose);
						LOG(INFO) << "Has Read " << pcbSent << " Bytes,hr is " << hr << std::endl;
					if (pWebSocketContext->SendConnectionClose(FALSE, 1000) == S_OK) {
						LOG(INFO) << "WebSocket Close SUccessfully." << std::endl;
					}
					free(bufr);

					//hr = pWebSocketContext->SendConnectionClose(false, sCode);
				}
			}
		}


			/*
			//Forward URL
			if (auto WS_value = pHttpRequest->GetForwardedUrl()) {
				char* ws_value_char = (char*)malloc((wcslen(WS_value) + 1) * sizeof(char));
				size_t ws_conv;
				wcstombs_s(&ws_conv, ws_value_char, wcslen(WS_value) + 1,WS_value, _TRUNCATE);
				LOG(INFO) << "Forwarded URL: " << ws_value_char << std::endl;
				free(ws_value_char);
			}

			//Set Sec-Websocket-Accept
			auto WS_key = pHttpRequest->GetHeader("Sec-WebSocket-Key");
			auto WS_SHA1_ICHAR = (std::string(WS_key) + std::string(WebSocketMagicString)).c_str();
			LOG(INFO) << "Concated SRTRING: " << WS_SHA1_ICHAR <<", size: "<<std::strlen(WS_SHA1_ICHAR) << std::endl;

			//SHA1
			UCHAR WS_SHA1_UCHAR[SHA_DIGEST_LENGTH];
			SHA1((UCHAR*)WS_SHA1_ICHAR, std::strlen(WS_SHA1_ICHAR), WS_SHA1_UCHAR);

			//BASE64 , char base64_string[28]
			int encoded_data_length = Base64encode_len(SHA_DIGEST_LENGTH);
			char* base64_string = (char*)malloc(encoded_data_length);
			Base64encode(base64_string, (CHAR*)WS_SHA1_UCHAR, SHA_DIGEST_LENGTH);

			//Set Headers
			hr = pHttpResponse->SetHeader("Sec-WebSocket-Accept", base64_string, SHA_DIGEST_LENGTH, TRUE);
			LOG(INFO) << "Sec-WebSocket-Accept: " << base64_string << std::endl;
			free(base64_string);
*/

		}
		google::FlushLogFiles(google::GLOG_INFO);
		return RQ_NOTIFICATION_CONTINUE;
	}
	else
	{
		LOG(INFO) << "Cannot Get Request or Response Context" << std::endl;
		// End additional processing.
		return RQ_NOTIFICATION_CONTINUE;
	}

}

REQUEST_NOTIFICATION_STATUS CRigNet::OnPostEndRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;
	const char* WebSocketMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	// Retrieve a pointer to the request
	IHttpRequest *pHttpRequest = pHttpContext->GetRequest();
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	// Create an HRESULT to receive return values from methods.
	HRESULT hr = S_OK;
	if (pHttpRequest != NULL && pHttpResponse != NULL) {
		//Check Upgrade
		if (auto WS_ver = pHttpRequest->GetHeader("Sec-WebSocket-Version"))//First time WebSocket Connection
		{
			//Accept Key
			if (auto value = pHttpResponse->GetHeader("Sec-WebSocket-Accept")) {
				DLOG(INFO) << "Accept Key: " << value << std::endl;
			}
		}
	}
	return RQ_NOTIFICATION_CONTINUE;
}