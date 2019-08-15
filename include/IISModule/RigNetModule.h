#include "precomp.h"

// Create the module class.
class CTVNet : public CHttpModule
{
public:
	CTVNet() {
		urls["/login"] = TV::URL_LOGIN;
		urls["/register"] = TV::URL_REGISTER;
		urls["/sqlitedata"] = TV::URL_SQLITE_DATA;
		urls["/exist"] = TV::URL_EXIST;
	}
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);
	REQUEST_NOTIFICATION_STATUS OnReadEntity(IN IHttpContext* pHttpContext, IN IReadEntityProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext* pHttpContext, IN IHttpEventProvider* pProvider);

	std::promise<BOOL> m_websocket_cont;
	std::map<std::string, int> urls;
};

namespace TV {
// error code table
constexpr int32_t ERROR_JSON_PARSE = 1001;/*http request JSON parsing error*/
constexpr int32_t ERROR_SQLITE_EXEC = 1002;/*sqlite engine exeution error*/
constexpr int32_t ERROR_UUID_CREAT = 1003;/*uuid error in server*/
constexpr int32_t ERROR_JSON_CREAT = 1004;/*JSON construction error within server*/
constexpr int32_t ERROR_REGISTER_EXIST = 1005;/*User has been registered*/

constexpr int32_t ERROR_LOGIN_NOMEMBER = 2001;
constexpr int32_t ERROR_LOGIN_PWD = 2002;

//url code
constexpr int32_t URL_LOGIN = 1;
constexpr int32_t URL_REGISTER = 2;
constexpr int32_t URL_SQLITE_DATA = 3;
constexpr int32_t URL_EXIST = 4;


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
		Setting() {};
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
	Via WebSocket
	*/

	/*
	TVNetMain, as functor for initialization of websocket
	*/
	HRESULT TVNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);

	/*
	Following Three functions used for responding comtype in each request json struct
	*/
	std::string main(std::vector<char> &msg);
#ifdef USE_MYSQL
	std::string mysql(const rapidjson::Document &&msg);
#endif
	std::string sqlite(const rapidjson::Document &&msg);

	/*
	Inner implementaton of sqlite query
	*/
	namespace SQLITE {
		std::string general(const char* db, const char* stm);
	}

	class CTVHttp {
	public:
		CTVHttp(IHttpContext* pHttpContext);
		virtual void process() = 0;
	protected:
		rapidjson::Document m_RequestJSON;
		std::vector<char> m_BufHttpRequest;
		IHttpContext* m_pHttpContext;
	};

	class CTVHttpLogin :public TV::CTVHttp {
	public:
		CTVHttpLogin(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	private:
		rapidjson::Document m_ResultJSON;
		std::wstringstream m_dbstm;
	};

	class CTVHttpExist :public TV::CTVHttp {
	public:
		CTVHttpExist(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	private:
		rapidjson::Document m_ResultJSON;
		std::wstringstream m_dbstm;
	};

	class CTVHttpRegister :public TV::CTVHttp {
	public:
		CTVHttpRegister(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	private:
		rapidjson::Document m_ResultJSON;
		std::wstringstream m_dbstm;
	};

	class CTVHttpData :public TV::CTVHttp {
	public:
		CTVHttpData(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	private:
		rapidjson::Document m_ResultJSON;
		std::wstringstream m_dbstm;
	};
}
