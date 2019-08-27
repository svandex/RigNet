#include "precomp.h"

#define TV_PROJECT_NAME "TVNET"
// Create the module class.

namespace TV {
	// error code table

	/*client error*/
	constexpr int32_t ERROR_ACTION = 4001;/*action failed*/
	constexpr int32_t ERROR_JSON_PARSE = 4002;/*http request JSON parsing error*/
	constexpr int32_t ERROR_JSON_KEY = 4003;/*key spell error in json format*/
	constexpr int32_t ERROR_SQLITE_EXEC = 4004;/*sqlite engine exeution error*/
	constexpr int32_t ERROR_SQLITE_EMPTY = 4005;/*sqlite execution return empty*/
	constexpr int32_t ERROR_REGISTER_EXIST = 4006;/*User has been registered*/
	constexpr int32_t ERROR_LOGIN_PWD = 4007;/*password error when logging in*/
	constexpr int32_t ERROR_HTTP_CTYPE = 4008;/*content type not suppored*/
	constexpr int32_t ERROR_NO_ENV = 4009;/*TV_PROJECT_NAME env variable not setup*/
	constexpr int32_t ERROR_SQLITE_OPEN= 4010;/*TV_PROJECT_NAME env variable not setup*/
	constexpr int32_t ERROR_JPEG_FORMAT = 4011;/*uploaded file format is incorrect*/

	constexpr int32_t ERROR_SESSION_EXPIRED = 4444;/*action time expired*/

	/*server error*/
	constexpr int32_t ERROR_JSON_CREAT = 5001;/*JSON construction error within server*/
	constexpr int32_t ERROR_UUID_CREAT = 5002;/*uuid error in server*/
	constexpr int32_t ERROR_URL_NOEXIST = 5003;/*URL doesn't exist*/
	constexpr int32_t ERROR_JSON_CREAT_SEND = 5004;/*JSON creation error when sending payload*/

	/*debug*/
	constexpr int32_t ERROR_DEBUG = 9999;

	// success code table
	constexpr int32_t SUCCESS_ACTION = 2001;

	//url code
	constexpr int32_t URL_LOGIN = 1;
	constexpr int32_t URL_REGISTER = 2;
	constexpr int32_t URL_SQLITE_DATA = 3;
	constexpr int32_t URL_EXIST = 4;
	constexpr int32_t URL_UPLOAD = 5;

	class Utility {
	public:
		std::string tvGetServerVariable(const char* vName,IHttpContext* pHttpContext) {
			if (pHttpContext->GetServerVariable(vName, &m_tempcsTR, &m_tempDWORD) == S_OK) {
				return std::string(m_tempcsTR);
			}
			else {
				return "";
			}
		};

		template<typename T>
		void tvReadEntity(IHttpContext* pHttpContext, std::vector<T>& buf);
	private:
		PCSTR m_tempcsTR;
		DWORD m_tempDWORD;
	};

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

	/* TVNetMain, as functor for initialization of websocket */
	HRESULT TVNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);

	/* Following Three functions used for responding comtype in each request json struct */
	std::string main(std::vector<char> &msg);
#ifdef USE_MYSQL
	std::string mysql(const rapidjson::Document &&msg);
#endif
	std::string sqlite(const rapidjson::Document &&msg);

	/* Inner implementaton of sqlite query */
	namespace SQLITE {
		std::string general(const char* db, const char* stm);
	}

	bool isJPEG(std::string savepath);

	class CTVHttp {
	public:
		CTVHttp(IHttpContext* pHttpContext);
		virtual void process() = 0;
	protected:
		rapidjson::Document m_RequestJSON;
		std::vector<char> m_HttpRequestJSONBuffer;
		IHttpContext* m_pHttpContext;
		rapidjson::Document m_ResultJSON;
		std::wstringstream m_dbstm;
	};

	class CTVHttpLogin :public TV::CTVHttp {
	public:
		CTVHttpLogin(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	};

	class CTVHttpExist :public TV::CTVHttp {
	public:
		CTVHttpExist(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	};

	class CTVHttpRegister :public TV::CTVHttp {
	public:
		CTVHttpRegister(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	};

	class CTVHttpData :public TV::CTVHttp {
	public:
		CTVHttpData(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	};

	class CTVHttpUpload :public TV::CTVHttp {
	public:
		CTVHttpUpload(IHttpContext* pHttpContext) :TV::CTVHttp(pHttpContext){

		}
		void process();
	};
}

class CTVNet : public CHttpModule
{
public:
	CTVNet() {
		urls["/login"] = TV::URL_LOGIN;
		urls["/register"] = TV::URL_REGISTER;
		urls["/sqlitedata"] = TV::URL_SQLITE_DATA;
		urls["/exist"] = TV::URL_EXIST;
		urls["/upload"] = TV::URL_UPLOAD;
	}
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);
	REQUEST_NOTIFICATION_STATUS OnReadEntity(IN IHttpContext* pHttpContext, IN IReadEntityProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext* pHttpContext, IN IHttpEventProvider* pProvider);

	std::promise<BOOL> m_websocket_cont;
	std::map<std::string, int> urls;
};

