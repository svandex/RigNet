/*

Rig Lab Workbench Network

Creator: Qiu Juncheng
Date: 2018-10-30

*/

#include <iostream>

#include "NICard_s.h"
#include "SimensPLC.h"

/*
TWo Universal buffer that hold data, Two threads that writes to buffer and writes to mysql database

Cannot write to database directly becasue database response requires time

1. NI BUffer
2. PLC Buffer
*/

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

    /* 
    SimensPLC plc(tvs.addr_plc, 0, 1);
    if (plc.client->Connected())
    {
        plc.run(plc.client, tv::MakeRigFunctor(plc.client, &TS7Client::GetOrderCode), &(plc.oc));
        std::cout << plc.oc.Code << std::endl;
    }else{
        std::cout<<"Connection Failed!"<<std::endl;
    }
    */

    try
    {
        NICard_s nic(*tvs);
        nic.config_default();
        nic.start();

        return 0;
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
        ts.stop();
        t.join();
    }
    catch (websocketpp::exception &e)
    {
        std::wcout << "websocketpp: " << e.what() << std::endl;
    }
    catch (std::exception &e)
    {
        std::wcout << e.what() << std::endl;
    }
}