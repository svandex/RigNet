#include "visa.h"
#include "RigNet.h"

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