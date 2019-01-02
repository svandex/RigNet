#include <iostream>
#include "RigNet.h"

int main(int argc, char *argv[])
{
    tv::Setting *tvs = tv::Setting::INSTANCE();
    if(argc!=2){
        std::cout<<"require setting.json path\n";
        return 0;
    }
    tvs->filepath = argv[1];
    //tvs->filepath = "C:/Users/saictv/Repositories/RigNet/data/setting.json";
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

        log.open("output.log");
        ts.get_alog().set_ostream(&log);
        ts.get_elog().set_ostream(&log);

        //preparation
        ts.init_asio();
        ts.listen(websocketpp::lib::asio::ip::tcp::v4(), 9001);
        ts.start_accept();

        char host_name[255];
        gethostname(host_name,sizeof(host_name));
        std::cout<<"Please Connect to "<<host_name<<std::endl;

        //tvrig_server.run();
        std::thread t(&server::run, &ts);
        char x;
        std::cin >> x;
        ts.stop_listening();
        t.join();
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
