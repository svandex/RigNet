#pragma once

#include "RigNet.h"
#include "NIDAQmxBase.h"

/*NICard class that implemented by using nidaqmx*/
class NICard_s final
{
  public:
    NICard_s() = delete;
    NICard_s(const NICard_s &) = delete;
    NICard_s(tv::Setting &s)
    {
        auto daqmx_obj = s.jsonobj["nicard"]["daqmx"].GetObject();
        m_tk_name = daqmx_obj["taskname"].GetString();
        m_dev_name = daqmx_obj["devicename"].GetString();
        m_frq_num = daqmx_obj["samplenumber"].GetInt();
        m_frq = daqmx_obj["samplefrequency"].GetFloat();

        /*add network device*/
        /*
    char devname[256];
    run(tv::MakeRigFunctor_s(DAQmxAddNetworkDevice), daqmx_obj["ip"].GetString(), m_dev_name.c_str(), 1, 10.0, devname, 256);
    std::cout << devname << std::endl;
    */

        /*reset device*/
        run(tv::MakeRigFunctor_s(DAQmxBaseResetDevice), m_dev_name.c_str());
        /*create task*/
        run(tv::MakeRigFunctor_s(DAQmxBaseCreateTask), m_tk_name, &m_tk);

        m_chan_num = daqmx_obj["channels"].GetArray().Size();
        for (uint16_t index = 0; index < m_chan_num; index++)
        {
            auto channel_elm = daqmx_obj["channels"][index].GetObject();
            //auto port_name = m_dev_name + std::string("/") + std::string(channel_elm["port"].GetString());
            run(tv::MakeRigFunctor_s(DAQmxBaseCreateAIVoltageChan), m_tk, (m_dev_name + std::string("/") + std::string(channel_elm["port"].GetString())).c_str(), channel_elm["name"].GetString(), DAQmx_Val_Cfg_Default, channel_elm["min"].GetFloat(), channel_elm["max"].GetFloat(), DAQmx_Val_Volts, nullptr);
        }

        /*data buffer*/
        m_data_buf = std::make_shared<float64 *>(new float64[m_frq_num * m_chan_num]);
    }

    ~NICard_s()
    {
        DAQmxBaseStopTask(m_tk);
        DAQmxBaseClearTask(m_tk);
    }

    template <typename... rfArgs, typename... Args>
    void run(tv::RigFunctor_s<int32, rfArgs...> rf, Args... args)
    {
        m_error = rf(std::forward<Args>(args)...);
        if (DAQmxFailed(m_error))
        {
            DAQmxBaseGetExtendedErrorInfo(m_errbuf, 2048);
            if (m_tk != 0)
            {
                m_tk = 0;
                std::cout << "daqmx: ";
                throw std::logic_error(m_errbuf);
            }
        }
    }

    void config_default();
    void start();

#ifdef __nidaqmx_h__
    friend int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
    friend int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
#endif

  private:
    TaskHandle m_tk;                       //task handle
    const char *m_tk_name;                 //task name
    std::string m_dev_name;                //device name
    int32 m_error = 0;                     //error code
    char m_errbuf[2048] = {'\0'};          //error buffer
    float64 m_frq;                         //sample frequency
    uint32_t m_frq_num;                    //sample number
    std::shared_ptr<float64 *> m_data_buf; //data buffer
    uint32_t m_chan_num;                   //channel number
};

#ifdef __nidaqmx_h__
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
#endif