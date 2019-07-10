#include "precomp.h"
// Create the module class.
class CTVNet : public CHttpModule
{
public:
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);
	REQUEST_NOTIFICATION_STATUS OnReadEntity(IN IHttpContext* pHttpContext, IN IReadEntityProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnExecuteRequestHandler(IN IHttpContext* pHttpContext, IN IHttpEventProvider* pProvider);

	std::promise<BOOL> m_websocket_cont;
};

namespace TV {
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
}
