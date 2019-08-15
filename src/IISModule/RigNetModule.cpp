#include "precomp.h"

inline HRESULT httpSendBack(IN IHttpContext *pHttpContext, std::string result)try {
	rapidjson::Document resultjson;
	if (resultjson.Parse(result.c_str()).HasParseError()) {
		// Set the HTTP status.
		pHttpContext->GetResponse()->SetStatus(500, "Server Error", 0, E_UNEXPECTED);
		result = Svandex::json::message(std::to_string(TV::ERROR_JSON_CREAT));
	}


	HRESULT hr = S_OK;
	//PCSTR pszBuf = "hello";
	PCSTR pszBuf = result.c_str();
	// Create a data chunk.
	HTTP_DATA_CHUNK dataChunk;
	// Set the chunk to a chunk in memory.
	dataChunk.DataChunkType = HttpDataChunkFromMemory;
	// Buffer for bytes written of data chunk.
	DWORD cbSent;

	// Set the chunk to the buffer.
	dataChunk.FromMemory.pBuffer =
		(PVOID)pszBuf;
	// Set the chunk size to the buffer size.
	dataChunk.FromMemory.BufferLength =
		(USHORT)strlen(pszBuf);
	BOOL completed;
	// Insert the data chunk into the response.
	hr = pHttpContext->GetResponse()->WriteEntityChunks(
		&dataChunk, 1, FALSE, TRUE, &cbSent, &completed);
	if (FAILED(hr)||resultjson.HasMember("error"))
	{
		// Set the HTTP status.
		pHttpContext->GetResponse()->SetStatus(500, "Server Error", 0, hr);
	}
	pHttpContext->GetResponse()->Flush(FALSE, TRUE, &cbSent, &completed);
	return hr;
}
catch (std::exception &e) {
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	HRESULT hr = E_UNEXPECTED;
	pHttpResponse->SetStatus(500, e.what(), 0, hr);
	return hr;
}

REQUEST_NOTIFICATION_STATUS CTVNet::OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider)
try {
	UNREFERENCED_PARAMETER(pProvider);

	extern IHttpServer *g_HttpServer;

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

REQUEST_NOTIFICATION_STATUS CTVNet::OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo) {
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

REQUEST_NOTIFICATION_STATUS CTVNet::OnExecuteRequestHandler(IN IHttpContext* pHttpContext, IN IHttpEventProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);
	extern IHttpServer *g_HttpServer;
	HRESULT hr;

	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
	IHttpResponse* pHttpResponse = pHttpContext->GetResponse();

	USHORT headerValue;
	auto hhc = pHttpRequest->GetHeader(HTTP_HEADER_ID::HttpHeaderUpgrade, &headerValue);
	if (hhc != NULL && std::strcmp(hhc, "websocket") == 0) {
		/*
		WebSocket protocol
		*/
		Svandex::WebSocket wsinstance(g_HttpServer, pHttpContext, TV::TVNetMain);
		wsinstance.StateMachine();
		//m_websocket_cont.set_value(TRUE);

		/*
		IHttpContext3 *pHttpContext3;
		hr = HttpGetExtendedInterface(g_HttpServer, pHttpContext, &pHttpContext3);
		IWebSocketContext* pWebSocket = (IWebSocketContext*)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
		pWebSocket->CloseTcpConnection();
		*/
		return RQ_NOTIFICATION_CONTINUE;
	}
	else {
		/*
		HTTP protocol
		*/

		// get url
		std::map<std::string, int> urls;
		urls["/login"] = TV_LOGIN;
		urls["/register"] = TV_REGISTER;
		urls["/sqlitedata"] = TV_SQLITE_DATA;
		urls["/exist"] = TV_EXIST;


		PCSTR vForwardURL;
		DWORD vPcchValueLength;
		hr = pHttpContext->GetServerVariable("HTTP_URL", &vForwardURL, &vPcchValueLength);

		if (urls.find(vForwardURL) != urls.end()) {
			if (sqlite3_threadsafe() != 0) {
				sqlite3_config(SQLITE_CONFIG_SERIALIZED);
			}
			std::shared_ptr<TV::CTVHttp> ctvhttp;

			try {
				switch (urls[vForwardURL]) {
				case TV_LOGIN: {
					ctvhttp = std::make_shared<TV::CTVHttpLogin>(pHttpContext);
				}//TV_LOGIN
				case TV_SQLITE_DATA: {
					ctvhttp = std::make_shared<TV::CTVHttpData>(pHttpContext);
				}//TV_SQLITE_DATA
				case TV_REGISTER: {
					ctvhttp = std::make_shared<TV::CTVHttpRegister>(pHttpContext);
				}//TV_REGISTER
				case TV_EXIST: {
					ctvhttp = std::make_shared<TV::CTVHttpExist>(pHttpContext);
				}//TV_EXIST
				}
			}
			catch (std::exception& e) {
				return RQ_NOTIFICATION_CONTINUE;
			}

			ctvhttp->process();
		}
		return RQ_NOTIFICATION_CONTINUE;
	}
}

/*
TVNET main entry

Main control for websocket
*/
HRESULT TV::TVNetMain(std::vector<char>& WebSocketReadLine, std::vector<char>& WebSocketWritLine) {
	/*
WebSocketWritLine.insert(WebSocketWritLine.end(), WebSocketReadLine.begin(), WebSocketReadLine.end());
*/
	const char* result = TV::main(WebSocketReadLine).data();
	WebSocketWritLine.clear();
	WebSocketWritLine.reserve(WebSocketWritLine.size() + std::strlen(result));
	WebSocketWritLine.insert(WebSocketWritLine.end(), result, result + std::strlen(result));
	return S_OK;
}

std::string TV::main(std::vector<char>& _websocket_in) try {
	_websocket_in.shrink_to_fit();
	if (_websocket_in.size() == 0) {
		return Svandex::json::message("request body is empty", "HTTP");
	}
	static std::map<std::string, std::function<std::string(const rapidjson::Document && msg)>> rig_dispatchlist_map;

	//dispatch initialization
	//rig_dispatchlist_map["mysql"] = TV::mysql;
	rig_dispatchlist_map["sqlite"] = TV::sqlite;
	/*
	rig_dispatchlist_map["plc"] = rignet_plc;
	rig_dispatchlist_map["nicard"] = rignet_nicard;
	*/

	rapidjson::Document json_msg;
	//    std::cout<<msg->get_payload().c_str()<<std::endl;
	auto temp = _websocket_in.data();
	auto string = std::string(temp);
	if (json_msg.Parse(_websocket_in.data()).HasParseError())
	{
		return Svandex::json::message("not a json object");
	}

	if (json_msg.HasMember("comtype"))
	{
		if (json_msg["comtype"].IsString())
		{
			auto f = rig_dispatchlist_map[json_msg["comtype"].GetString()];
			return f(std::move(json_msg));
		}
		else
		{
			return Svandex::json::message("command type member should exist.");
		}
	}
	else
	{
		return Svandex::json::message("json has no member named comtype.");
	}
}
catch (std::exception &e)
{
	return Svandex::json::message(e.what(), SVANDEX_STL);
}

#ifdef USE_MYSQL
std::string TV::mysql(const rapidjson::Document &&json_msg)try {
	/*
	TV::Setting *tvs = TV::Setting::instance();

	auto mysql_login = std::string("mysqlx://") + tvs->jsonobj["mysql"]["user"].GetString() + std::string(":") + tvs->jsonobj["mysql"]["pwd"].GetString() + std::string("@") + tvs->jsonobj["mysql"]["ip"].GetString() + std::string(":") + std::to_string(tvs->jsonobj["mysql"]["port"].GetInt()) + std::string("/eslam?ssl-mode=disabled");
	mysqlx::Session mysql_ss(mysql_login.c_str());
	*/

	mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/eslam?ssl-mode=disabled");
	//database selection
	std::stringstream mysql_db_sel;
	mysql_db_sel << "use " << json_msg["sql"]["database"].GetString();
	mysql_ss.sql(mysql_db_sel.str()).execute();

	auto mysql_stm = winrt::to_hstring(json_msg["sql"]["statement"].GetString()).c_str();

	//TODO: unicode char conversion

	/*see if semicolon is at the end*/
	auto rsets = mysql_ss.sql(mysql_stm).execute();

	if (rsets.hasData())
	{
		mysqlx::Row r;
		rapidjson::StringBuffer sb;
		rapidjson::Writer<rapidjson::StringBuffer> responseWriter(sb);

		/*start object*/
		responseWriter.StartObject();
		int l_index = 0;
		while ((r = rsets.fetchOne()))
		{
			responseWriter.Key(std::to_string(l_index).c_str());
			responseWriter.StartArray();
			for (size_t tmp = 0; tmp < r.colCount(); tmp++)
			{
				switch (r[tmp].getType()) {
				case mysqlx::Value::Type::UINT64: responseWriter.Uint64((uint64_t)r[tmp]); break;
				case mysqlx::Value::Type::INT64: responseWriter.Int64((int64_t)r[tmp]); break;
				case mysqlx::Value::Type::STRING: responseWriter.String(std::string(r[tmp]).c_str()); break;
				case mysqlx::Value::Type::DOUBLE: responseWriter.Double((double)r[tmp]); break;
				case mysqlx::Value::Type::BOOL: responseWriter.Bool((bool)r[tmp]); break;
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
					responseWriter.String(buf);
					break;
*/
					responseWriter.String("rawbytes");
					break;
				}
				default:
					responseWriter.String("null");
					break;

				}
			}
			responseWriter.EndArray();

			l_index++;
		}
		//websocket process id return to client
		/*
		responseWriter.Key("wspid");
		responseWriter.String(json_msg["wspid"].GetString());
		 */
		responseWriter.EndObject();
		/*end object*/

		return std::string(sb.GetString());
	}
	else
	{
		return Svandex::json::ErrMess("mysql result sets empty");
	}

	//close mysql session
	mysql_ss.close();
}
catch (std::exception &e)
{
	return Svandex::json::ErrMess(e.what());
}
#endif

/*
sqlite implementation

class used to support sending parameters 
 */
class cPara {
public:
	void * data;
	int ki;
};

/*
sqlite callback function
 */
static int sqlite_callback(void *data, int argc, char **argv, char **azColName) {
	auto pData = (cPara*)data;
	auto responseWriter = (rapidjson::Writer<rapidjson::StringBuffer>*)(pData->data);
	responseWriter->Key(std::to_string(pData->ki).c_str());
	pData->ki += 1;
	responseWriter->StartArray();

	if (argc == 0) {
		return 0;
	}

	for (size_t i = 0; i < argc; i++)
	{
		responseWriter->String(argv[i]);
	}
	responseWriter->EndArray();
	return 0;
}

std::string TV::sqlite(const rapidjson::Document &&msg) try
{
	return TV::SQLITE::general(msg["database"].GetString(), msg["statement"].GetString());
}
catch (std::exception &e)
{
	return Svandex::json::message(e.what());
}

std::string TV::SQLITE::general(const char* dbname, const char* stm) {
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> responseWriter(sb);

	/*start object*/
	responseWriter.StartObject();
	/*sqlite */
	sqlite3* db;
	int rc;
	char *errMsg = 0;
	cPara cp;

	auto dbPath = std::string(DB_DIR) + dbname + ".db";

	rc = sqlite3_open(dbPath.c_str(), &db);
	//rc = sqlite3_open_v2(dbPath.c_str(), &db, SQLITE_OPEN_NOMUTEX, NULL);
	if (rc)
	{
		return Svandex::json::message("Failed To Open Sqlite Database");
	}

	cp.data = &responseWriter;
	cp.ki = 0;

	rc = sqlite3_exec(db, stm, sqlite_callback, &cp, &errMsg);

	/*end object */
	responseWriter.EndObject();
	std::string rs;

	if (rc != SQLITE_OK)
	{
		rs = errMsg;
		sqlite3_free(errMsg);
	}
	else
	{
		rs = sb.GetString();
	}
	sqlite3_close(db);
	return rs;

}

REQUEST_NOTIFICATION_STATUS CTVNet::OnReadEntity(IN IHttpContext *pHttpContext, IN IReadEntityProvider *pProvider)
{

	return RQ_NOTIFICATION_CONTINUE;
}

TV::CTVHttp::CTVHttp(IHttpContext* pHttpContext) {
	m_pHttpContext = pHttpContext;

	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();

	DWORD cbReceived = 0;
	BOOL completed;
	m_BufHttpRequest.resize(SVANDEX_BUF_SIZE);
	pHttpRequest->ReadEntityBody(m_BufHttpRequest.data(), (DWORD)m_BufHttpRequest.capacity(), FALSE, &cbReceived, &completed);

	if (m_RequestJSON.Parse(m_BufHttpRequest.data()).HasParseError()) {
		httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_JSON_PARSE)));
		throw std::exception("json parsing error.");
	}
}

void TV::CTVHttpLogin::process() {
	if (m_RequestJSON.HasMember("id") && m_RequestJSON.HasMember("password")) {
		//m_dbstm << L"select `角色`,`密码`,`会话标识`,date_format(`最近更新`,'%Y-%m-%d %T') from `0用户信息` where `工号`=\"" << m_RequestJSON["id"].GetString() << "\"";
		m_dbstm << L"select `角色`,`密码`,`会话标识`,`最近更新` from `0用户信息` where `工号`=\"" << m_RequestJSON["id"].GetString() << "\"";
		rapidjson::Document rcDOC;
		auto result = TV::SQLITE::general("labwireless", winrt::to_string(m_dbstm.str()).c_str()).c_str();
		if (rcDOC.Parse(result).HasParseError() || !rcDOC.HasMember("0")) {
			httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_SQLITE_EXEC)));
			return;
		}

		/**/
		if (std::strcmp(rcDOC["0"][1].GetString(), m_RequestJSON["password"].GetString()) == 0) {
			//login successfully
			std::istringstream s_ssId(rcDOC["0"][2].GetString());
			if (s_ssId.str() == "expired") {//add uuid as session id
				auto uuid = Svandex::tools::GetUUID();
				if (uuid == "") {
					httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_UUID_CREAT)));
				}
				else {
					m_dbstm.clear();
					m_dbstm.str(L"");
					m_dbstm << L"update `0用户信息` set `会话标识`=\""
						<< uuid.c_str()
						<< L"\",`最近更新`=\"" << Svandex::tools::GetCurrentTimeFT().c_str()
						<< L"\" where `工号`=\"" << m_RequestJSON["id"].GetString() << "\"";
					TV::SQLITE::general("labwireless", winrt::to_string(m_dbstm.str()).c_str());

					httpSendBack(m_pHttpContext, "{\"sessionId\":\"" + uuid + "\",\"roleId\":\"" +
						std::string(rcDOC["0"][0].GetString()) + "\"}");
				}
			}
			else {
				//last modified time
				std::istringstream s_lmtime(rcDOC["0"][3].GetString());
				std::tm t = {};
				s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
				auto retval = std::difftime(std::time(nullptr), std::mktime(&t));
				if (retval > SVANDEX_SESSION_EXPIRED) {//>10min, update last modified time
					m_dbstm.clear();
					m_dbstm.str(L"");
					m_dbstm << L"update `0用户信息` set `最近更新`=\""
						<< Svandex::tools::GetCurrentTimeFT().c_str()
						<< L"\" where `工号`=\"" << m_RequestJSON["id"].GetString() << "\"";
					TV::SQLITE::general("labwireless", winrt::to_string(m_dbstm.str()).c_str());
				}
				httpSendBack(m_pHttpContext, "{\"sessionId\":\"" + std::string(rcDOC["0"][2].GetString()) + "\",\"roleId\":\"" + std::string(rcDOC["0"][0].GetString()) + "\",\"userId\":\"" + m_RequestJSON["id"].GetString() + "\"}");
			}
		}
		else {
			httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_LOGIN_PWD)));
		}
	}
	else {
		httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_LOGIN_NOMEMBER)));
	}
}

void TV::CTVHttpExist::process() {
	if (m_RequestJSON.HasMember("userId")) {
		std::wstringstream db_stm;
		db_stm << L"select * from `0用户信息` where `工号`=\"" << m_RequestJSON["userId"].GetString() << "\"";
		auto rcStr = TV::SQLITE::general("labwireless", winrt::to_string(db_stm.str()).c_str());

		rapidjson::Document rcDoc;
		if (rcDoc.Parse(rcStr.c_str()).HasMember("0")) {
			httpSendBack(m_pHttpContext, Svandex::json::message("1", "result"));
		}
		else {
			httpSendBack(m_pHttpContext, Svandex::json::message("0", "result"));
		}
	}

}

void TV::CTVHttpRegister::process() {
	if (m_RequestJSON.HasMember("id") && m_RequestJSON.HasMember("password")
		&& m_RequestJSON.HasMember("role") && m_RequestJSON.HasMember("contact")
		&& m_RequestJSON.HasMember("name")) {
		//database selection
		m_dbstm << L"select * from `0用户信息` where `工号`=\"" << m_RequestJSON["id"].GetString() << "\"";
		auto rcStr = TV::SQLITE::general("labwireless", winrt::to_string(m_dbstm.str()).c_str());

		rapidjson::Document rcDoc;
		if (rcDoc.Parse(rcStr.c_str()).HasMember("0")) {
			httpSendBack(m_pHttpContext, Svandex::json::message(std::to_string(TV::ERROR_REGISTER_EXIST)));
		}
		else {
			m_dbstm.clear();
			m_dbstm.str(L"");
			m_dbstm << L"insert into `0用户信息` values(\'"
				<< winrt::to_hstring(m_RequestJSON["role"].GetString()).c_str() << "\',\'"
				<< m_RequestJSON["id"].GetString() << "\',\'"
				<< m_RequestJSON["password"].GetString() << "\',\'"
				<< m_RequestJSON["contact"].GetString() << "\',"
				<< "0,\'expired\',\'"
				<< Svandex::tools::GetCurrentTimeFT().c_str() << "\',\'"
				<< winrt::to_hstring(m_RequestJSON["name"].GetString()).c_str()
				<< "\')";
			TV::SQLITE::general("labwireless", winrt::to_string(m_dbstm.str()).c_str());
		}
	}
}

void TV::CTVHttpData::process() {
	if (m_RequestJSON.HasMember("sessionId")) {
		std::wstringstream db_stm;
		db_stm << L"select `最近更新` from `0用户信息` where `会话标识`=\"" << m_RequestJSON["sessionId"].GetString() << "\"";

		rapidjson::Document rcDoc;
		auto rcStr = TV::SQLITE::general("labwireless", winrt::to_string(db_stm.str()).c_str());
		if (rcDoc.Parse(rcStr.c_str()).HasParseError()) {
			httpSendBack(m_pHttpContext, Svandex::json::message("sqlite parse failed", "data"));
		}

		if (!rcDoc.HasMember("0")) {
			httpSendBack(m_pHttpContext, Svandex::json::message("sqlite return empty", "data"));
		}
		std::istringstream s_lmtime(rcDoc["0"][0].GetString());
		std::tm t = {};
		s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
		if (std::difftime(std::time(nullptr), std::mktime(&t)) > SVANDEX_SESSION_EXPIRED) {
			httpSendBack(m_pHttpContext, Svandex::json::message("Session Expired.", "data"));
		}
		/*
		add comtype to json
		*/
		m_RequestJSON.GetObjectW().AddMember("comtype", rapidjson::Value("sqlite"), m_RequestJSON.GetAllocator());
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> tmpWriter(buffer);
		m_RequestJSON.Accept(tmpWriter);
		auto m_RequestJSONStr = buffer.GetString();
		memcpy_s(m_BufHttpRequest.data(), m_BufHttpRequest.size(), m_RequestJSONStr, std::strlen(m_RequestJSONStr));

		auto result = TV::main(m_BufHttpRequest);
		httpSendBack(m_pHttpContext, result);
	}
	else if (m_RequestJSON.HasMember("readonly")) {
		/*
		add comtype to json
		*/
		m_RequestJSON.GetObjectW().AddMember("comtype", rapidjson::Value("sqlite"), m_RequestJSON.GetAllocator());
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> tmpWriter(buffer);
		m_RequestJSON.Accept(tmpWriter);
		auto m_RequestJSONStr = buffer.GetString();
		memcpy_s(m_BufHttpRequest.data(), m_BufHttpRequest.size(), m_RequestJSONStr, std::strlen(m_RequestJSONStr));

		auto result = TV::main(m_BufHttpRequest);
		httpSendBack(m_pHttpContext, result);
	}
	else {
		httpSendBack(m_pHttpContext, Svandex::json::message("Parse Error or SessionId key not found", "data"));
	}

}





