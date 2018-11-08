/*

Rig Lab Workbench Network

Creator: Qiu Juncheng
Date: 2018-10-30

*/

#include <iostream>

/*
1. Setting Class
2. Initialize websocket server
*/
//#include "RigNet.h"

/*
1. NICard Class
2. SimensPLC Class
*/

#include "NICard.h"
#include "SimensPLC.h"

/*
TWo Universal buffer that hold data, Two threads that writes to buffer and writes to mysql database

Cannot write to database directly becasue database response requires time

1. NI BUffer
2. PLC Buffer
*/

int main(void)
{
    tv::Setting tvs;
    //Load Setting from setting.json in project root path
    tvs.addr_plc = "192.168.0.1";
    tvs.addr_nic = "192.168.0.101::3580";
    tvs.addr_mysql = "localhost";
    if (!tvs.LoadSetting())
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
        NICard nic(tvs.addr_nic.c_str());
        ViChar intfdesc[VI_FIND_BUFLEN];
        nic.run(tv::MakeRigFunctor_s(viGetAttribute), nic.m_instr, VI_ATTR_INTF_INST_NAME, intfdesc);
        std::printf("Interface Description:%s \n", intfdesc);

        nic.run(tv::MakeRigFunctor_s(viGetAttribute), nic.m_instr, VI_ATTR_TCPIP_HOSTNAME, intfdesc);
        std::printf("TCPIP Hostname: %s \n", intfdesc);

        //protocal
        //nic.run(tv::MakeRigFunctor_s(viSetAttribute), nic.m_instr, VI_ATTR_IO_PROT, VI_PROT_4882_STRS);
        ViUInt16 t_prot = 0;
        nic.run(tv::MakeRigFunctor_s(viGetAttribute), nic.m_instr, VI_ATTR_IO_PROT, &t_prot);
        std::cout << "Protocal: " << t_prot << std::endl;

        //Attribute Setting
        nic.run(tv::MakeRigFunctor_s(viSetAttribute), nic.m_instr, VI_ATTR_TMO_VALUE, 2000);
        //nic.run(tv::MakeRigFunctor_s(viSetAttribute), nic.m_instr, VI_ATTR_TERMCHAR, 0x0D);
        nic.run(tv::MakeRigFunctor_s(viSetAttribute), nic.m_instr, VI_ATTR_TERMCHAR_EN, VI_TRUE);
        //nic.run(tv::MakeRigFunctor_s(viSetAttribute), nic.m_instr, VI_ATTR_SEND_END_EN, VI_FALSE);

        //Commands
        nic.run(tv::MakeRigFunctor_s(viClear), nic.m_instr);
        //nic.run(tv::MakeRigFunctor_s(viWrite), nic.m_instr, (ViConstBuf) "READ:WAVFM:CH1\n", 14, &nic.m_ret_cnt);
        nic.run(tv::MakeRigFunctor_s(viWrite), nic.m_instr, (ViConstBuf) "*IDN?\n", 6, &nic.m_ret_cnt);
        std::cout << "Write COmpleted." << std::endl;
        ViByte buf[1024];
        nic.run(tv::MakeRigFunctor_s(viAssertTrigger),nic.m_instr,VI_TRIG_PROT_DEFAULT);
        nic.run(tv::MakeRigFunctor_s(viRead), nic.m_instr, buf, 1024, &nic.m_ret_cnt);
        std::cout << "nicard: " << buf << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;

    //websocket server
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
        ts.stop();
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