#include "NICard_s.h"
NICard_s *NICard_s::m_instance = nullptr;
NICard_s::deletePTR NICard_s::del;

void NICard_s::config()
{
    /*sampling configuration*/
    run(tv::MakeRigFunctor_s(DAQmxCfgSampClkTiming), m_tk, "", m_frq, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_frq_num);
    run(tv::MakeRigFunctor_s(DAQmxCfgInputBuffer), m_tk, 200000);

    /*sampling function*/
    run(tv::MakeRigFunctor_s(DAQmxRegisterEveryNSamplesEvent), m_tk, DAQmx_Val_Acquired_Into_Buffer, m_frq_num, 0, EveryNCallback, this);
    run(tv::MakeRigFunctor_s(DAQmxRegisterDoneEvent), m_tk, 0, DoneCallback, nullptr);
}

void NICard_s::start()
{
    run(tv::MakeRigFunctor_s(DAQmxStartTask), m_tk);
}

void NICard_s::get_data_in(int sec){
    //store data
    time_t startTime = time(nullptr);
    while (time(nullptr) < startTime + sec)
    {
        int32 readpoints;
        run(tv::MakeRigFunctor_s(DAQmxReadAnalogF64), m_tk, m_frq_num, 10.0, DAQmx_Val_GroupByChannel, *m_data_buf.get(), m_frq_num * m_chan_num, &readpoints, nullptr);
        for (int32 i = 0; i < 10; i++)
        {
            printf("data0[%ld] = %f\tdata1[%ld] = %f\n", i, (*m_data_buf.get())[2 * i], i, (*m_data_buf.get())[2 * i + 1]);
        }
    }

    printf("\ndata aqusition end.\n");

}

std::string NICard_s::return_buffer_json()
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> JsonWriter(s);

    JsonWriter.StartObject();
    JsonWriter.Key("nicard");
    JsonWriter.String("success");
    for (uint32_t index = 1; index <= m_chan_num; index++)
    {
        JsonWriter.Key(("channel_" + std::to_string(index)).c_str());
        JsonWriter.StartArray();
        for (uint32_t index_num = 1; index_num <= m_frq_num; index_num++)
        {
            //TO DO
            JsonWriter.Double((*m_data_buf)[m_chan_num*(index_num-1)+index-1]);
        }
        JsonWriter.EndArray();
    }
    JsonWriter.EndObject();

    return s.GetString();
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
    /* sampling*/
    auto nic = reinterpret_cast<NICard_s *>(callbackData);
    int32 read;
    nic->run(tv::MakeRigFunctor_s(DAQmxReadAnalogF64), nic->m_tk, nic->m_frq_num, 10.0, DAQmx_Val_GroupByChannel, *(nic->m_data_buf).get(), nic->m_frq_num * nic->m_chan_num, &read, nullptr);
    return DAQmxSuccess;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
    return DAQmxSuccess;
}
