#include "precomp.h"
#include "sqlite3.h"

inline HRESULT httpSendBack(IN IHttpContext *pHttpContext, std::string result)try {
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
	// Insert the data chunk into the response.
	hr = pHttpContext->GetResponse()->WriteEntityChunks(
		&dataChunk, 1, FALSE, TRUE, &cbSent);
	if (FAILED(hr))
	{
		// Set the HTTP status.
		pHttpContext->GetResponse()->SetStatus(500, "Server Error", 0, hr);
	}
	pHttpContext->GetResponse()->Flush(FALSE, TRUE, &cbSent);
	return hr;
}
catch (std::exception &e) {
	// Retrieve a pointer to the response.
	IHttpResponse *pHttpResponse = pHttpContext->GetResponse();
	HRESULT hr = E_UNEXPECTED;
	pHttpResponse->SetStatus(500, e.what(), 0, hr);

	// End additional processing.
	return RQ_NOTIFICATION_FINISH_REQUEST;
}

REQUEST_NOTIFICATION_STATUS CTVNet::OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider)
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

REQUEST_NOTIFICATION_STATUS CTVNet::OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider) try{
	UNREFERENCED_PARAMETER(pProvider);
	extern IHttpServer *g_HttpServer;
	HRESULT hr;

	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
	IHttpResponse* pHttpResponse = pHttpContext->GetResponse();
	auto hhc = pHttpRequest->GetHeader(HTTP_HEADER_ID::HttpHeaderUpgrade);
	if (hhc != NULL && std::strcmp(hhc, "websocket") == 0) {
		Svandex::WebSocket wsinstance(g_HttpServer, pHttpContext, TVNetMain);
		wsinstance.StateMachine();
		//m_websocket_cont.set_value(TRUE);

		IHttpContext3 *pHttpContext3;
		hr = HttpGetExtendedInterface(g_HttpServer, pHttpContext, &pHttpContext3);
		IWebSocketContext* pWebSocket = (IWebSocketContext*)pHttpContext3->GetNamedContextContainer()->GetNamedContext(IIS_WEBSOCKET);
		pWebSocket->CloseTcpConnection();
		/*
		Since websocket is upgrade from http, normaly we wont use this http request again, which
		dosn't contain much infomation from it, merely with weboscket upgrade info. So we return
		RQ_NOTIFICATION_FINISH_REQUEST to end this connection.
		*/
		return RQ_NOTIFICATION_FINISH_REQUEST;
	}
	else {
		/*
		HTTP protocol
		URL is `/data` to get data from server
		*/
		// get url

		std::map<std::string, int> urls;
		urls["/login"] = TV_LOGIN;
		urls["/register"] = TV_REGISTER;
		urls["/data"] = TV_DATA;

		//Read Request entity to bufHttpRequest
		DWORD cbReceived = 0;
		std::vector<char> bufHttpRequest;
		bufHttpRequest.resize(SVANDEX_BUF_SIZE);
		hr = pHttpRequest->ReadEntityBody(bufHttpRequest.data(), bufHttpRequest.capacity(), FALSE, &cbReceived);
		//TODO: Read only once, but it is not enough

		PCSTR vForwardURL;
		DWORD vPcchValueLength;
		hr = pHttpContext->GetServerVariable("HTTP_URL", &vForwardURL, &vPcchValueLength);

		if (urls.find(vForwardURL) != urls.end()) {
			switch (urls[vForwardURL]) {
			case TV_LOGIN: {//login
				mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/labwireless?ssl-mode=disabled");
				rapidjson::Document requestJson;
				if (requestJson.Parse(bufHttpRequest.data()).HasParseError()) {
					httpSendBack(pHttpContext, Svandex::json::ErrMess("-1"));
				}
				else {
					if (requestJson.HasMember("id") && requestJson.HasMember("password")) {
						std::wstringstream mysql_db_sel;
						mysql_db_sel << L"use labwireless";
						mysql_ss.sql(mysql_db_sel.str()).execute();

						mysql_db_sel.clear();
						mysql_db_sel.str(L"");

						mysql_db_sel << L"select `角色`,`工号`,`密码`,`联系方式`,`激活`,`会话标识`,date_format(`最近更新`,'%Y-%m-%d %T') from `0用户信息` where `工号`=\"" << requestJson["id"].GetString() << "\"";
						auto rsets = mysql_ss.sql(mysql_db_sel.str()).execute();
						if (rsets.count() == 1) {
							//if (rsets.hasData()) {//find user
							mysqlx::Row r = rsets.fetchOne();

							//determine type
							if (r[2].getType() != mysqlx::Value::Type::STRING) {
								httpSendBack(pHttpContext, Svandex::json::ErrMess("-1"));
								mysql_ss.close();
								break;
							}
							if (std::string(r[2]) == requestJson["password"].GetString()) {
								//login successfully

								/*
								1. determine if there exists session
								2. if not, then add session to database
								*/

								//session exists, 10min limits
								//sesssionID
								std::istringstream s_ssId(std::string(r[5]).c_str());
								if (s_ssId.str() == "expired") {//add uuid as session id
									auto uuid = Svandex::tools::GetUUID();
									if (uuid == "") {
										httpSendBack(pHttpContext, "{\"sessionId\":-4}");
									}
									else {
										mysql_db_sel.clear();
										mysql_db_sel.str(L"");
										mysql_db_sel << L"update `0用户信息` set `会话标识`=\""
											<< uuid.c_str()
											<< L"\",`最近更新`=\"" << Svandex::tools::GetCurrentTimeFT().c_str()
											<< L"\" where `工号`=\"" << requestJson["id"].GetString() << "\"";
										mysql_ss.sql(mysql_db_sel.str()).execute();
										httpSendBack(pHttpContext, "{\"sessionId\":\"" + uuid + "\",\"roleId\":\"" +
											std::string(r[0]) + "\"}");
									}
								}
								else {
									//last modified time
									std::istringstream s_lmtime(std::string(r[6]).c_str());
									std::tm t = {};
									s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
									auto retval = std::difftime(std::time(nullptr), std::mktime(&t));
									if (retval > SVANDEX_SESSION_EXPIRED) {//>10min, update last modified time
										mysql_db_sel.clear();
										mysql_db_sel.str(L"");
										mysql_db_sel << L"update `0用户信息` set `最近更新`=\""
											<< Svandex::tools::GetCurrentTimeFT().c_str()
											<< L"\" where `工号`=\"" << requestJson["id"].GetString() << "\"";
										mysql_ss.sql(mysql_db_sel.str()).execute();
									}
									httpSendBack(pHttpContext, "{\"sessionId\":\"" + std::string(r[5]) + "\"}");
								}
							}
							else {//password error
								httpSendBack(pHttpContext, "{\"sessionId\":-1}");
							}
						}
						else {//no registration
							httpSendBack(pHttpContext, "{\"sessionId\":-2}");
						}
					}
					else {//json format parse error
						httpSendBack(pHttpContext, Svandex::json::ErrMess("-2"));
					}
				}
				mysql_ss.close();
				break;
			}
			case TV_REGISTER: {//register new users
				mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/labwireless?ssl-mode=disabled");
				rapidjson::Document requestJson;
				if (requestJson.Parse(bufHttpRequest.data()).HasParseError()) {
					httpSendBack(pHttpContext, Svandex::json::ErrMess("-1"));
				}
				else {
					if (requestJson.HasMember("id") && requestJson.HasMember("password")
						&& requestJson.HasMember("role") && requestJson.HasMember("contact")
						&& requestJson.HasMember("name")) {
						//database selection
						std::wstringstream mysql_db_sel;

						/*
						user infomation table should not be in specified test database
						*/
						mysql_db_sel << L"use labwireless";
						mysql_ss.sql(mysql_db_sel.str()).execute();

						mysql_db_sel.clear();
						mysql_db_sel.str(L"");

						mysql_db_sel << L"select * from `0用户信息` where `工号`=\"" << requestJson["id"].GetString() << "\"";
						auto rsets = mysql_ss.sql(mysql_db_sel.str()).execute();
						if (rsets.count() == 1) {//find user
						//if (rsets.hasData()) {//find user
							httpSendBack(pHttpContext, "{\"registration\":-1}");
						}
						else {//registration
							mysql_db_sel.clear();
							mysql_db_sel.str(L"");
							if (std::strcmp(requestJson["password"].GetString(), "") == 0) {
								httpSendBack(pHttpContext, "{\"registration\":-2}");
							}
							else {
								mysql_db_sel << L"insert into `0用户信息` values(\'"
									<< winrt::to_hstring(requestJson["role"].GetString()).c_str() << "\',\'"
									<< requestJson["id"].GetString() << "\',\'"
									<< requestJson["password"].GetString() << "\',\'"
									<< requestJson["contact"].GetString() << "\',"
									<< "0,\'expired\',\'"
									<< Svandex::tools::GetCurrentTimeFT().c_str() << "\',\'"
									<< winrt::to_hstring(requestJson["name"].GetString()).c_str()
									<< "\')";
								mysql_ss.sql(mysql_db_sel.str()).execute();
								httpSendBack(pHttpContext, "{\"registration\":0}");
							}
						}
					}
					else {
						httpSendBack(pHttpContext, Svandex::json::ErrMess("-2"));
					}
				}
				mysql_ss.close();
				break;
			}
			case TV_DATA: {//data
				mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/labwireless?ssl-mode=disabled");
				rapidjson::Document requestJson;
				if (!requestJson.Parse(bufHttpRequest.data()).HasParseError() && requestJson.HasMember("sessionId")) {
					std::wstringstream mysql_db_sel;
					mysql_db_sel << L"select date_format(`最近更新`,'%Y-%m-%d %T') from `0用户信息` where `会话标识`=\""
						<< requestJson["sessionId"].GetString() << "\"";
					auto rsets = mysql_ss.sql(mysql_db_sel.str()).execute();
					if (rsets.hasData()) {
						mysqlx::Row r = rsets.fetchOne();
						//last modified time
						std::istringstream s_lmtime(std::string(r[0]).c_str());
						std::tm t = {};
						s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
						if (std::difftime(std::time(nullptr), std::mktime(&t)) > SVANDEX_SESSION_EXPIRED) {
							httpSendBack(pHttpContext, "{\"sessionId\":-1}");
							mysql_ss.close();
							break;
						}
						auto result = TV::main(bufHttpRequest);
						httpSendBack(pHttpContext, result);
					}
					else {//no session exist
						httpSendBack(pHttpContext, "{\"sessionId\":-2}");
					}
				}
				else {
					httpSendBack(pHttpContext, Svandex::json::ErrMess("-1"));
				}
				mysql_ss.close();
				break;
			}
			default:
				break;
			}// switch 
		}

		//NO ACTION
		return RQ_NOTIFICATION_CONTINUE;
	}
}
catch (std::exception &e) {
	httpSendBack(pHttpContext, Svandex::json::ErrMess(e.what(), SVANDEX_STL));
	return RQ_NOTIFICATION_CONTINUE;
}

REQUEST_NOTIFICATION_STATUS CTVNet::OnPostAuthenticateRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	return RQ_NOTIFICATION_CONTINUE;
}

REQUEST_NOTIFICATION_STATUS CTVNet::OnAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	UNREFERENCED_PARAMETER(pProvider);
	extern IHttpServer *g_HttpServer;
	/*
	Login Determination

	1. get cache ,if not it means user has not login and then send back login file to it
	2. get cache , if exist it needs to determine whether the client has access to the file

	*/
	IHttpRequest* pHttpRequest = pHttpContext->GetRequest();
	// get url
	PCSTR v_forwardURL;
	DWORD v_pcchValueLength;
	HRESULT hr = pHttpContext->GetServerVariable("HTTP_URL", &v_forwardURL, &v_pcchValueLength);
	//
	if (std::string(v_forwardURL).find("/loginGateWay") != std::string::npos) {
		/*
		localStorage to store permanent data
		sessionStorage to store session related data
		*/
	}
	else {
		/*
		first check if the connection has a sessionid
		*/
	}

	return RQ_NOTIFICATION_CONTINUE;

}

REQUEST_NOTIFICATION_STATUS CTVNet::OnPostAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider) {
	return RQ_NOTIFICATION_CONTINUE;
}

HRESULT TVNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine) {
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
		return Svandex::json::ErrMess("request body is empty", "HTTP");
	}
	static std::map<std::string, std::function<std::string(const rapidjson::Document &&msg)>> rig_dispatchlist_map;

	//dispatch initialization
	rig_dispatchlist_map["mysql"] = TV::mysql;
	rig_dispatchlist_map["sqlite"] = TV::sqlite;
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

	if (json_msg.HasMember("comtype"))
	{
		if (json_msg["comtype"].IsString())
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
		rapidjson::Writer<rapidjson::StringBuffer> rj(sb);

		/*start object*/
		rj.StartObject();
		int l_index = 0;
		while ((r = rsets.fetchOne()))
		{
			rj.Key(std::to_string(l_index).c_str());
			rj.StartArray();
			for (size_t tmp = 0; tmp < r.colCount(); tmp++)
			{
				switch (r[tmp].getType()) {
				case mysqlx::Value::Type::UINT64: rj.Uint64((uint64_t)r[tmp]); break;
				case mysqlx::Value::Type::INT64: rj.Int64((int64_t)r[tmp]); break;
				case mysqlx::Value::Type::STRING: rj.String(std::string(r[tmp]).c_str()); break;
				case mysqlx::Value::Type::DOUBLE: rj.Double((double)r[tmp]); break;
				case mysqlx::Value::Type::BOOL: rj.Bool((bool)r[tmp]); break;
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
					rj.String(buf);
					break;
*/
					rj.String("rawbytes");
					break;
				}
				default:
					rj.String("null");
					break;

				}
			}
			rj.EndArray();

			l_index++;
		}
		//websocket process id return to client
		/*
		rj.Key("wspid");
		rj.String(json_msg["wspid"].GetString());
		 */
		rj.EndObject();
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

/*
sqlite implementation
 */

 /*
 callback struct
 */
struct cPara {
	void * data;
	int ki;
} cp;

static int sqlite_callback(void *data, int argc, char **argv, char **azColName) {
	auto pData = (cPara*)data;
	auto rj = (rapidjson::Writer<rapidjson::StringBuffer>*)(pData->data);
	rj->Key(std::to_string(pData->ki).c_str());
	pData->ki += 1;
	rj->StartArray();
	for (size_t i = 0; i < argc; i++)
	{
		rj->String(argv[i]);
	}
	rj->EndArray();
	return 0;
}

std::string TV::sqlite(const rapidjson::Document &&msg) try
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> rj(sb);

	/*start object*/
	rj.StartObject();
	/*sqlite */
	sqlite3* db;
	const char* sql;
	int rc, keyIndex;
	char *errMsg = 0;

	keyIndex = 0;

	auto dbPath = std::string("C:\\Users\\saictv\\Desktop\\Sites\\db\\") + msg["sql"]["database"].GetString() + ".db";

	rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc)
	{
		return Svandex::json::ErrMess("Failed To Open Sqlite Database");
	}

	cp.data = &rj;
	cp.ki = keyIndex;


	sql = msg["sql"]["statement"].GetString();
	rc = sqlite3_exec(db, sql, sqlite_callback, &cp, &errMsg);

	/*end object */
	rj.EndObject();
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
catch (std::exception &e)
{
	return Svandex::json::ErrMess(e.what());
}

REQUEST_NOTIFICATION_STATUS CTVNet::OnReadEntity(IN IHttpContext *pHttpContext, IN IReadEntityProvider *pProvider)
{

	return RQ_NOTIFICATION_CONTINUE;
}
