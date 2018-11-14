#include "NICard.h"

int NICard::m_init_num = 0;
ViSession *NICard::defaultRM = new ViSession();

NICard::NICard(const char *remAddr)
{
    if (NICard::m_init_num == 0)
    {
        run(tv::MakeRigFunctor_s(viOpenDefaultRM), NICard::defaultRM);
    }

    /*find resources*/
    std::cout << std::endl
              << "...Start finding..." << std::endl
              << std::endl;
    ViFindList flist;
    ViUInt32 numInstrs;
    ViChar desc[VI_FIND_BUFLEN];
    try
    {
        run(tv::MakeRigFunctor_s(viFindRsrc), *NICard::defaultRM, "?*", &flist, &numInstrs, desc);
    }
    catch (std::exception &e)
    {
        std::cout << "No resource found" << std::endl;
        std::cout << desc << std::endl;
    }
    std::cout << "found " << numInstrs << " devices." << std::endl;
    if (numInstrs > 0)
    {
        std::cout << desc << std::endl;
        while (numInstrs--)
        {
            viFindNext(flist, desc);
            std::cout << desc << std::endl;
        }
    }
    std::cout << "...End finding..." << std::endl
              << std::endl;
    viClose(flist);

    /*
    std::string res = "TCPIP0::" + std::string(remAddr) + "::SOCKET";
    //std::string res = "ASRL0::" + std::string(remAddr) + "::INSTR";
    //std::string res = "TCPIP0::" + std::string(remAddr) + "::INSTR";
    std::cout << res << std::endl;

    run(tv::MakeRigFunctor_s(viOpen), *NICard::defaultRM, res.c_str(), VI_NULL, VI_NULL, &m_instr);
    */

    run(tv::MakeRigFunctor_s(viOpen), *NICard::defaultRM, "USB0::0x3923::0x7269::NI-VISA-1002::RAW", VI_NULL, VI_NULL, &m_instr);
    NICard::m_init_num += 1;
}

void NICard_s::config_default()
{
    /*sampling configuration*/
    run(tv::MakeRigFunctor_s(DAQmxCfgSampClkTiming), m_tk,"", m_frq, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_frq_num);

    /*sampling function*/
    run(tv::MakeRigFunctor_s(DAQmxRegisterEveryNSamplesEvent), m_tk, DAQmx_Val_Acquired_Into_Buffer, m_frq_num, 0, EveryNCallback, nullptr);
    run(tv::MakeRigFunctor_s(DAQmxRegisterDoneEvent), m_tk, 0, DoneCallback, nullptr);
}

void NICard_s::start()
{
    run(tv::MakeRigFunctor_s(DAQmxStartTask), m_tk);
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    /* sampling*/
    return DAQmxSuccess;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
    return DAQmxSuccess;
}