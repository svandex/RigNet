/*

Rig Lab Workbench Network

Creator: Qiu Juncheng
Date: 2018-10-30

*/

#include <iostream>
#include "RigNet.h"
#include "Svandex.h"

#ifdef NI_ENABLE
#include "NICard_s.h"
#endif

int main(int argc, char *argv[])
{
    tv::Setting *tvs = tv::Setting::instance();
	tvs->filepath = svandex::tools::GetCurrentPath() + "/setting.json";
    //Load Setting from setting.json in project root path
    if (!tvs->LoadSetting())
    {
        std::cout << "Load Setting.json failed!" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        server ts;
        std::ofstream log;

        ts.set_message_handler(bind(&on_message, &ts, ::_1, ::_2));

        //logging
        ts.set_access_channels(websocketpp::log::alevel::connect);
        ts.set_access_channels(websocketpp::log::alevel::disconnect);
        ts.set_access_channels(websocketpp::log::alevel::app);

        log.open((svandex::tools::GetCurrentPath()+"output.log").c_str());
        ts.get_alog().set_ostream(&log);
        ts.get_elog().set_ostream(&log);

        //preparation
        ts.init_asio();
        ts.listen(websocketpp::lib::asio::ip::tcp::v4(), 9002);
        ts.start_accept();

        /*
        char host_name[255];
        gethostname(host_name,sizeof(host_name));
        std::cout<<"Please Connect to "<<host_name<<std::endl;
        */

        //tvrig_server.run();
		ts.run();
    }
    catch (websocketpp::exception &e)
    {
        std::wcout << "websocketpp: " << e.what() << std::endl;
    }
    catch (std::exception &e)
    {
        std::wcout << "std: " << e.what() << std::endl;
    }
}
