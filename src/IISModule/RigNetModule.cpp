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
	/*
	Determine whether module has enabled websocket module,
	other module may also call OnAsyncCompletion
	*/
	extern IHttpServer *g_HttpServer;
	IHttpContext3 *pHttpContext3;
	HRESULT hr = HttpGetExtendedInterface(g_HttpServer, pHttpContext, &pHttpContext3);
	IWebSocketContext* pWebSocket = (IWebSocketContext*)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
	if (pWebSocket) {
		/*
		m_websocket_cont.get_future().wait();
		pWebSocket->CloseTcpConnection();
		*/
		return RQ_NOTIFICATION_PENDING;
	}
	else {
		return RQ_NOTIFICATION_CONTINUE;
	}
}

REQUEST_NOTIFICATION_STATUS CRigNet::OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);
	extern IHttpServer *g_HttpServer;

	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
	if (pHttpRequest->GetHeader(HTTP_HEADER_ID::HttpHeaderUpgrade) != NULL && pHttpContext->GetConnection()->IsConnected()) {
		/*
		IHttpContext3 *pHttpContext3;
		hr = HttpGetExtendedInterface(g_HttpServer,pHttpContext, &pHttpContext3);
		pHttpContext3->GetNamedContextContainer()->SetNamedContext(&wsinstance, SVANDEX_STOREDCONTEXT);
		*/
		Svandex::WebSocket wsinstance(g_HttpServer, pHttpContext, RigNetMain);
		wsinstance.StateMachine();
		//m_websocket_cont.set_value(TRUE);

		/*
		Since websocket is upgrade from http, normaly we wont use this http request again, which 
		dosn't contain much infomation from it, merely with weboscket upgrade info. So we return
		RQ_NOTIFICATION_FINISH_REQUEST to end this connection.
		*/
		return RQ_NOTIFICATION_FINISH_REQUEST;
	}
	else {
		return RQ_NOTIFICATION_CONTINUE;
	}
}

REQUEST_NOTIFICATION_STATUS CRigNet::OnPostAuthenticateRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	return RQ_NOTIFICATION_CONTINUE;
}
REQUEST_NOTIFICATION_STATUS CRigNet::OnAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);
	extern IHttpServer *g_HttpServer;
	/*
	Login Determination

	1. get cache ,if not it means user has not login and then send back login file to it
	2. get cache , if exist it needs to determine whether the client has access to the file

	*/
	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
	PCWSTR v_forwardURL = pHttpRequest->GetForwardedUrl();

	return RQ_NOTIFICATION_CONTINUE;

}

REQUEST_NOTIFICATION_STATUS CRigNet::OnPostAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	return RQ_NOTIFICATION_CONTINUE;
}

HRESULT RigNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine) {
	/*
	WebSocketWritLine.insert(WebSocketWritLine.end(), WebSocketReadLine.begin(), WebSocketReadLine.end());
	*/
	const char* result = RigNet::main(WebSocketReadLine).data();
	WebSocketWritLine.clear();
	WebSocketWritLine.reserve(WebSocketWritLine.size() + std::strlen(result));
	WebSocketWritLine.insert(WebSocketWritLine.end(), result, result + std::strlen(result));
	return S_OK;
}

std::string RigNet::main(std::vector<char>& _websocket_in) try {
	_websocket_in.shrink_to_fit();
	static std::map<std::string, std::function<std::string(const rapidjson::Document &&msg)>> rig_dispatchlist_map;

	//dispatch initialization
	rig_dispatchlist_map["mysql"] = RigNet::mysql;
	/*
	rig_dispatchlist_map["plc"] = rignet_plc;
	rig_dispatchlist_map["nicard"] = rignet_nicard;
	*/

	rapidjson::Document json_msg;
	//    std::cout<<msg->get_payload().c_str()<<std::endl;
	if (json_msg.Parse(_websocket_in.data()).HasParseError())
	{
		return Svandex::json::ErrMess("not a json object");
	}

	if (json_msg.HasMember("comtype") && json_msg.HasMember("wspid"))
	{
		if (json_msg["comtype"].IsString() && json_msg["wspid"].IsString())
		{
			auto f = rig_dispatchlist_map[json_msg["comtype"].GetString()];
			return f(std::move(json_msg));
		}
		else
		{
			return Svandex::json::ErrMess("command type member or wspid should exist.");
		}
	}
	else
	{
		return Svandex::json::ErrMess("json has no member named comtype or wspid");
	}
}
catch (std::exception &e)
{
	return Svandex::json::ErrMess(e.what(), SVANDEX_STL);
}

std::string RigNet::mysql(const rapidjson::Document &&json_msg)try {
	/*
	RigNet::Setting *tvs = RigNet::Setting::instance();

	auto mysql_login = std::string("mysqlx://") + tvs->jsonobj["mysql"]["user"].GetString() + std::string(":") + tvs->jsonobj["mysql"]["pwd"].GetString() + std::string("@") + tvs->jsonobj["mysql"]["ip"].GetString() + std::string(":") + std::to_string(tvs->jsonobj["mysql"]["port"].GetInt()) + std::string("/eslam?ssl-mode=disabled");
	mysqlx::Session mysql_ss(mysql_login.c_str());
	*/

	mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/eslam?ssl-mode=disabled");
	//database selection
	std::stringstream mysql_db_sel;
	mysql_db_sel << "use " << json_msg["sql"]["database"].GetString();
	mysql_ss.sql(mysql_db_sel.str()).execute();

	auto mysql_stm = json_msg["sql"]["statement"].GetString();

	/*see if semicolon is at the end*/
	auto rsets = mysql_ss.sql(std::string(mysql_stm)).execute();

	if (rsets.hasData())
	{
		mysqlx::Row r;
		rapidjson::StringBuffer sb;
		rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

		/*start object*/
		writer.StartObject();
		int l_index = 0;
		while ((r = rsets.fetchOne()))
		{
			writer.Key(std::to_string(l_index).c_str());
			writer.StartArray();
			for (unsigned tmp = 0; tmp < r.colCount(); tmp++)
			{
				switch (r[tmp].getType()) {
				case mysqlx::Value::Type::UINT64: writer.Uint64((uint64_t)r[tmp]); break;
				case mysqlx::Value::Type::INT64: writer.Int64((int64_t)r[tmp]); break;
				case mysqlx::Value::Type::STRING: writer.String(std::string(r[tmp]).c_str()); break;
				case mysqlx::Value::Type::DOUBLE: writer.Double((double)r[tmp]); break;
				case mysqlx::Value::Type::BOOL: writer.Bool((bool)r[tmp]); break;
				case mysqlx::Value::Type::RAW: {
					/*
					r[tmp].print(std::cout);
					auto vele = r[tmp].getRawBytes();
					time_t vtime = *(uint32_t*)vele.first * 65536;
					char buf[256];
					tm v_tm;
					gmtime_s(&v_tm, &vtime);
					ctime_s(buf, 256, &vtime);
					std::cout << buf << std::endl;
					asctime_s(buf, 256, &v_tm);
					std::cout << buf << std::endl;
					writer.String(buf);
					break;
*/
					writer.String("rawbytes");
					break;
				}
				default:
					writer.String("null");
					break;

				}
			}
			writer.EndArray();

			l_index++;
		}
		//websocket process id return to client
		writer.Key("wspid");
		writer.String(json_msg["wspid"].GetString());
		writer.EndObject();
		/*end object*/

		return std::string(sb.GetString());
	}
	else
	{
		return Svandex::json::ErrMess("mysql result sets empty", SVANDEX_STL);
	}

}
catch (std::exception &e)
{
	return Svandex::json::ErrMess(e.what(), SVANDEX_STL);
}
