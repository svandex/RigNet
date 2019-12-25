#pragma once

#include <string>
#include <typeinfo>
#include <fstream>

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

#include "mysqlx/xdevapi.h"

#include "pathcch.h"

#include "Svandex.h"

#define GLOG_NO_ABBREVIATED_SEVERITIE
#include "glog/logging.h"

namespace tv
{
	class Setting
	{
	public:
		static tv::Setting *instance()
		{
			if (!m_instance)
			{
				m_instance = new tv::Setting();
			}
			return m_instance;
		}

		bool LoadSetting();

		class deletePTR
		{
		public:
			~deletePTR()
			{
				if (tv::Setting::m_instance)
				{
					delete tv::Setting::m_instance;
				}
			}
		};

	private:
		Setting() {};
		Setting(const tv::Setting &);

	public:
		std::string filepath = "setting.json";
		rapidjson::Document jsonobj;

		//ip addr
		std::string addr_plc;
		std::string addr_nic;
		std::string addr_mysql;

	private:
		static tv::Setting *m_instance;
		static deletePTR del;
	};

	/*
	Delegate to member function of a Class
	*/
	template <typename T, typename R, typename... Args>
	class RigFunctor
	{
	public:
		RigFunctor(T *t, R(T::*f)(Args...)) : m_t(t), m_f(f) {}

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
		RigFunctor_s(R(*f)(Args...)) : m_f(f)
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
	RigFunctor<T, R, Args...> MakeRigFunctor(T *t, R(T::*f)(Args...))
	{
		return RigFunctor<T, R, Args...>(t, f);
	}

	template <typename R, typename... Args>
	RigFunctor_s<R, Args...> MakeRigFunctor_s(R(*f)(Args...))
	{
		return RigFunctor_s<R, Args...>(f);
	}

} // namespace tv

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

void on_message(server *s, websocketpp::connection_hdl hdl, server::message_ptr msg);
void on_http(server *s, websocketpp::connection_hdl hdl);

//dispatch list
std::string rignet_mysql(server *s, const rapidjson::Document &&json_msg);
std::string rignet_plc(server *s, const rapidjson::Document &&json_msg);
std::string rignet_nicard(server *s, const rapidjson::Document &&json_msg);

