#include "NICard_s.h"

void NICard_s::config_default()
{
    /*sampling configuration*/
    run(tv::MakeRigFunctor_s(DAQmxBaseCfgSampClkTiming), m_tk, "", m_frq, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_frq_num);
    run(tv::MakeRigFunctor_s(DAQmxBaseCfgInputBuffer), m_tk, 200000);

#ifdef __nidaqmx_h__
    /*sampling function*/
    run(tv::MakeRigFunctor_s(DAQmxRegisterEveryNSamplesEvent), m_tk, DAQmx_Val_Acquired_Into_Buffer, m_frq_num, 0, EveryNCallback, this);
    run(tv::MakeRigFunctor_s(DAQmxRegisterDoneEvent), m_tk, 0, DoneCallback, nullptr);
#endif
}

void NICard_s::start()
{
    run(tv::MakeRigFunctor_s(DAQmxBaseStartTask), m_tk);

    //store data
    time_t startTime = time(nullptr);
    while (time(nullptr) < startTime + 10)
    {
        int32 readpoints;
        run(tv::MakeRigFunctor_s(DAQmxBaseReadAnalogF64), m_tk, m_frq_num, 10.0, DAQmx_Val_GroupByChannel, *m_data_buf.get(), m_frq_num * m_chan_num, &readpoints, nullptr);
        for (int32 i = 0; i < 10; i++)
        {
            printf("data0[%ld] = %f\tdata1[%ld] = %f\n", i, (*m_data_buf.get())[2 * i], i, (*m_data_buf.get())[2 * i + 1]);
        }
    }

    printf("\ndata aqusition end.\n");
}

#ifdef __nidaqmx_h__
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    /* sampling*/
    auto nic = reinterpret_cast<NICard_s *>(callbackData);
    int32 read;
    nic->run(tv::MakeRigFunctor_s(DAQmxReadAnalogF64), nic->m_tk, nic->m_frq_num, 10.0, DAQmx_Val_GroupByChannel, nic->m_data_buf, nic->m_frq_num * nic->m_chan_num, &read, nullptr);
    return DAQmxSuccess;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
    return DAQmxSuccess;
}
#endif