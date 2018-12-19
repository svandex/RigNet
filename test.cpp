#include <iostream>
#include "RigNet.h"

int main(void)
{
    tv::Setting *tvs = tv::Setting::INSTANCE();
    tvs->filepath = "/home/svandex/Documents/qiu/code/RigNet/data/setting.json";
    //Load Setting from setting.json in project root path
    if (!tvs->LoadSetting())
    {
        std::cout << "Load Setting.json failed!" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        server ts;
        //ts.set_open_handshake_timeout(1000000);
        //ts.set_max_http_body_size(64000000);
        //ts.set_http_handler(bind(&on_http, &ts, ::_1));
        ts.set_message_handler(bind(&on_message, &ts, ::_1, ::_2));

        //preparation
        ts.init_asio();
        ts.listen(websocketpp::lib::asio::ip::tcp::v4(), 9002);
        ts.start_accept();

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
