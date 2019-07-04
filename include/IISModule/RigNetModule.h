#include "precomp.h"
// Create the module class.
class CTVNet : public CHttpModule
{
public:
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnPostAuthenticateRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnPostAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);
	REQUEST_NOTIFICATION_STATUS OnReadEntity(IN IHttpContext* pHttpContext, IN IReadEntityProvider* pProvider);

	std::promise<BOOL> m_websocket_cont;
};

HRESULT TVNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);

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
		std::string main(std::vector<char> &msg);
		std::string mysql(const rapidjson::Document &&msg);
		std::string sqlite(const rapidjson::Document &&msg);
}
