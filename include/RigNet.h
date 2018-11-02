#pragma once

#ifndef ___RIGNET_H___
#define ___RIGNET_H___
#endif

#include <string>
#include <typeinfo>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace tv
{
class Setting
{
public:
  Setting(){};
  Setting(const char *fp) : filepath(fp) {}
  Setting(const char *addrplc, const char *addrnic) : addr_plc(addrplc), addr_nic(addrnic) {}
  bool LoadSetting();

public:
  std::string filepath = "setting.json";
  int NICardNumber = 0;
  int SimensPLCNumber = 0;
  std::string addr_plc;
  std::string addr_nic;
};

/*
Delegate to member function of a Class
*/
template <typename T, typename R, typename... Args>
class RigFunctor
{
public:
  RigFunctor(T *t, R (T::*f)(Args...)) : m_t(t), m_f(f) {}

  R operator()(Args &&... args)
  {
    return (m_t->*m_f)(std::forward<Args>(args)...);
  }

private:
  T *m_t;    //When member function called, A object is needed
  R(T::*m_f) //member function caller
  (Args...);
};

/*
Delegate to non-member function
*/

template <typename R, typename... Args>
class RigFunctor_s
{
public:
  RigFunctor_s(R (*f)(Args...)) : m_f(f)
  {
  }
  R operator()(Args &&... args)
  {
    return (*m_f)(std::forward<Args>(args)...);
  }

public:
  //functor literal name

private:
  R(*m_f)
  (Args...);
};

template <typename T, typename R, typename... Args>
RigFunctor<T, R, Args...> MakeRigFunctor(T *t, R (T::*f)(Args...))
{
  return RigFunctor<T, R, Args...>(t, f);
}

template <typename R, typename... Args>
RigFunctor_s<R, Args...> MakeRigFunctor_s(R (*f)(Args...))
{
  return RigFunctor_s<R, Args...>(f);
}

} // namespace tv

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

void on_message(server *s, websocketpp::connection_hdl hdl, server::message_ptr msg);