#include "precomp.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Data.Json.h"

// Create the module class.

using namespace winrt::Windows::Data::Json;

namespace TV
{

/*
This class is used to read settings from setting.json file
*/
class Setting
{
public:
	static TV::Setting *instance()
	{
		if (!m_instance)
		{
			m_instance = new TV::Setting();
		}
		return m_instance;
	}

	bool LoadSetting();

	class deletePTR
	{
	public:
		~deletePTR()
		{
			if (TV::Setting::m_instance)
			{
				delete TV::Setting::m_instance;
			}
		}
	};

private:
	Setting(){};
	Setting(const TV::Setting &);

public:
	std::string filepath = "setting.json";
	rapidjson::Document jsonobj;

	//ip addr
	std::string addr_plc;
	std::string addr_nic;
	std::string addr_mysql;

private:
	static TV::Setting *m_instance;
	static deletePTR del;
};

/*
This class is used to parse multidata
*/
class MultiDataParser
{
public:
	MultiDataParser() = delete;
	MultiDataParser(std::vector<char> &, std::string);

	//extract parameter, if there exist images then store them into file directory and write to database
	JsonObject execute();

	//if construtor succeed
	bool m_isConstruted = false;

private:
	//http body
	std::vector<char> m_body;
	//boundary string
	std::string m_boundary;

	//positions for boundaries
	std::vector<uint64_t> m_boundaries;
	//positions vector for \r\n
	std::vector<uint64_t> m_rnPositions;

	//positions vector for element which is after \r\n\r\n
	std::vector<uint64_t> m_elePositions;
	std::vector<uint64_t> m_elePositionEnds;

	std::string storeImage(uint32_t);
};

/*
	Via WebSocket
	*/

/* TVNetMain, as functor for initialization of websocket */
HRESULT TVNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);

/* Following Three functions used for responding comtype in each request json struct */
std::string main(std::vector<char> &msg);
#ifdef USE_MYSQL
std::string mysql(const rapidjson::Document &&msg);
#endif
std::string sqlite(const rapidjson::Document &&msg);

/* Inner implementaton of sqlite query */
namespace SQLITE
{
std::string general(const char *db, const char *stm);
}

bool isJPEG(std::string savepath);

class CTVHttp
{
public:
	CTVHttp(IHttpContext *pHttpContext);
	virtual void process() = 0;

protected:
	rapidjson::Document m_RequestJSON;
	std::vector<char> m_HttpRequestJSONBuffer;
	IHttpContext *m_pHttpContext;
	rapidjson::Document m_ResultJSON;
	std::wstringstream m_dbstm;
};

class CTVHttpLogin : public TV::CTVHttp
{
public:
	CTVHttpLogin(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};

class CTVHttpExist : public TV::CTVHttp
{
public:
	CTVHttpExist(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};

class CTVHttpRegister : public TV::CTVHttp
{
public:
	CTVHttpRegister(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};

class CTVHttpDataSQLite : public TV::CTVHttp
{
public:
	CTVHttpDataSQLite(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};

class CTVHttpDataMYSQL : public TV::CTVHttp
{
public:
	CTVHttpDataMYSQL(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};

class CTVHttpUpload : public TV::CTVHttp
{
public:
	CTVHttpUpload(IHttpContext *pHttpContext) : TV::CTVHttp(pHttpContext)
	{
	}
	void process();
};
} // namespace TV

class CTVNet : public CHttpModule
{
public:
	CTVNet()
	{
		m_urls["/login"] = TV::URL_LOGIN;
		m_urls["/register"] = TV::URL_REGISTER;
		m_urls["/sqlitedata"] = TV::URL_SQLITE_DATA;
		m_urls["/exist"] = TV::URL_EXIST;
		m_urls["/upload"] = TV::URL_UPLOAD;
		m_urls["/mysqldata"] = TV::URL_MYSQL_DATA;
	}
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext *pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider *pProvider, IN IHttpCompletionInfo *pCompletionInfo);
	REQUEST_NOTIFICATION_STATUS OnReadEntity(IN IHttpContext *pHttpContext, IN IReadEntityProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider);

	std::promise<BOOL> m_websocket_cont;
	std::map<std::string, int> m_urls;
	JsonObject m_json_response;
};
