#include "precomp.h"
// Create the module class.
class CRigNet : public CHttpModule
{
public:
	REQUEST_NOTIFICATION_STATUS OnSendResponse(IN IHttpContext *pHttpContext, IN ISendResponseProvider *pProvider);
	REQUEST_NOTIFICATION_STATUS OnAuthenticateRequest(IN IHttpContext *pHttpContext, IN IAuthenticationProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnPostAuthenticateRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnPostAuthorizeRequest(IN IHttpContext *pHttpContext, IN IHttpEventProvider* pProvider);
	REQUEST_NOTIFICATION_STATUS OnAsyncCompletion(IN IHttpContext* pHttpContext, IN DWORD dwNotification, IN BOOL fPostNotification, IN IHttpEventProvider* pProvider, IN IHttpCompletionInfo* pCompletionInfo);

	std::promise<BOOL> m_websocket_cont;
};

HRESULT RigNetMain(std::vector<char> &WebSocketReadLine, std::vector<char> &WebSocketWritLine);

namespace RigNet {
/*
This class is used to read settings from setting.json file
*/
	class Setting
	{
	public:
		static RigNet::Setting *instance()
		{
			if (!m_instance)
			{
				m_instance = new RigNet::Setting();
			}
			return m_instance;
		}

		bool LoadSetting();

		class deletePTR
		{
		public:
			~deletePTR()
			{
				if (RigNet::Setting::m_instance)
				{
					delete RigNet::Setting::m_instance;
				}
			}
		};

	private:
		Setting() {};
		Setting(const RigNet::Setting &);

	public:
		std::string filepath = "setting.json";
		rapidjson::Document jsonobj;

		//ip addr
		std::string addr_plc;
		std::string addr_nic;
		std::string addr_mysql;

	private:
		static RigNet::Setting *m_instance;
		static deletePTR del;
	};

	/*
	Via WebSocket
	*/
		std::string main(std::vector<char> &msg);
		std::string mysql(const rapidjson::Document &&msg);
}
