#ifndef _SIMENSPLC_H_
#define _SIMENSPLC_H_
#endif

#include "snap7.h"
#include "RigNet.h"

class SimensPLC
{
public:
  SimensPLC(const char *remAddr, int rack, int slot);

  inline void ShowErrText(int status)
  {
    if (status != 0)
    {
      //      std::cout << "Err! " << CliErrorText(client->LastError()) << std::endl;
      throw std::logic_error(CliErrorText(client->LastError()));
    }
  }

  template <typename... rfArgs, typename... Args>
  void run(TS7Client *t, tv::RigFunctor<TS7Client, int, rfArgs...> rf, Args... args)
  {
    int status = rf(std::forward<Args>(args)...);
    ShowErrText(status);
  }

  template <typename T, typename... rfArgs, typename... Args>
  void run(tv::RigFunctor<T, int, rfArgs...> rf, Args... args)
  {
    int status = rf(std::forward<Args>(args)...);
    ShowErrText(status);
  }

  ~SimensPLC()
  {
    client->Disconnect();
    delete client;
  }

public:
  TS7Client *client;
  TS7OrderCode oc;

private:
};
