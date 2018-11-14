#pragma once

#include "RigNet.h"

#define NIVISA_USB

#include "visa.h"
//#include "IviVisaType.h"

#include "NIDAQmx.h"

#define MAX_CNT 200

class NICard
{
public:
  NICard() = delete;
  NICard(const char *remAddr);

  template <typename... rfArgs, typename... Args>
  void run(tv::RigFunctor_s<ViStatus, rfArgs...> rf, Args... args)
  {
    m_status = rf(std::forward<Args>(args)...);
    if (m_status < VI_SUCCESS && !(m_status >= VI_SUCCESS))
    {
      ViChar errdesc[MAX_CNT] = {'\0'};
      viStatusDesc(m_instr, m_status, errdesc);
      throw std::logic_error(errdesc);
    }
  }

  ~NICard()
  {
    viClose(m_instr);
    NICard::m_init_num -= 1;
    if (NICard::m_init_num == 0)
    {
      viClose(*NICard::defaultRM);
    }
  }

public:
  ViSession m_instr; //id
  ViStatus m_status;

  static int m_init_num;
  static ViSession *defaultRM; //only one exists

  ViUInt32 m_ret_cnt;
  ViByte m_buffer[MAX_CNT] = {'\0'};
};

/*NICard class that implemented by using nidaqmx*/
class NICard_s
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

    /*create task*/
    run(tv::MakeRigFunctor_s(DAQmxCreateTask), m_tk_name, &m_tk);

    auto channel_number = daqmx_obj["channels"].GetArray().Size();

    /*data buffer*/
    m_data_buf = new float64(m_frq_num * channel_number);
  }

  ~NICard_s()
  {
    delete m_data_buf;
    DAQmxStopTask(m_tk);
    DAQmxClearTask(m_tk);
  }

  template <typename... rfArgs, typename... Args>
  void run(tv::RigFunctor_s<int32, rfArgs...> rf, Args... args)
  {
    m_error = rf(std::forward<Args>(args)...);
    if (DAQmxFailed(m_error))
    {
      DAQmxGetExtendedErrorInfo(m_errbuf, 2048);
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

private:
private:
  TaskHandle m_tk;              //task handle
  const char *m_tk_name;        //task name
  std::string m_dev_name;       //device name
  int32 m_error = 0;            //error code
  char m_errbuf[2048] = {'\0'}; //error buffer
  float64 m_frq;                //sample frequency
  uint32_t m_frq_num;           //sample number
  float64 *m_data_buf;          //data buffer
};

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);