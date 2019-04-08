#include "RigNet.h"

#ifdef NI_ENABLE
#include "NICard_s.h"
#endif

void on_http(server *s, websocketpp::connection_hdl hdl) try{//handling HTTP packet

}
catch (websocketpp::exception const &e)
{
    std::cout << "websocketpp error:" << e.what() << std::endl;
}
catch (std::exception &e)
{
    std::wcout << "std: " << e.what() << std::endl;
}

void on_message(server *s, websocketpp::connection_hdl hdl, server::message_ptr msg) try
{
    static std::map<std::string, std::function<std::string(server *, const rapidjson::Document &&)>> rig_dispatchlist_map;

    //dispatch initialization
    rig_dispatchlist_map["mysql"] = rignet_mysql;
    rig_dispatchlist_map["plc"] = rignet_plc;
    rig_dispatchlist_map["nicard"] = rignet_nicard;

    rapidjson::Document json_msg;
//    std::cout<<msg->get_payload().c_str()<<std::endl;
    if (json_msg.Parse(msg->get_payload().c_str()).HasParseError())
    {
        s->send(hdl, "{\"rapidjson\":\"parse error\"}", websocketpp::frame::opcode::TEXT);
        return;
    }

    if (json_msg.HasMember("comtype")&&json_msg.HasMember("wspid"))
    {
        if (json_msg["comtype"].IsString()&&json_msg["wspid"].IsString())
        {
            auto f = rig_dispatchlist_map[json_msg["comtype"].GetString()];
            auto result = f(s, std::move(json_msg));
            s->send(hdl, result, websocketpp::frame::opcode::TEXT);
            return;
        }
        else
        {
            s->send(hdl, "{\"rapidjson\":\"command type member or wspid should be string.\"}", websocketpp::frame::opcode::TEXT);
            return;
        }
    }
    else
    {
        s->send(hdl, "{\"rapidjson\":\"json has no member named comtype or wspid\"}", websocketpp::frame::opcode::TEXT);
        return;
    }
}
catch (websocketpp::exception const &e)
{
    std::cout << "websocketpp error:" << e.what() << std::endl;
}
catch (std::exception &e)
{
    std::wcout << "std: " << e.what() << std::endl;
}

tv::Setting *tv::Setting::m_instance = nullptr;
tv::Setting::deletePTR tv::Setting::del; /*static object to delete m_instance*/

bool tv::Setting::LoadSetting()
{
    std::ifstream ifs(filepath);
    rapidjson::IStreamWrapper isw(ifs);

    if (jsonobj.ParseStream(isw).HasParseError())
    {
        std::cout << rapidjson::GetParseError_En(jsonobj.GetParseError()) << std::endl;
        throw std::logic_error("setting.json parse error, check syntax.");
    }
    else
    {
        //load setting from json file
        addr_mysql = jsonobj["mysql"]["ip"].GetString();
        addr_nic = jsonobj["nicard"]["daqmx"]["ip"].GetString();
        addr_plc = jsonobj["simensplc"]["ip"].GetString();
        return true;
    }
}

std::string rignet_mysql(server *s, const rapidjson::Document &&json_msg) try
{
    tv::Setting *tvs = tv::Setting::instance();

	auto mysql_login = std::string("mysqlx://") + tvs->jsonobj["mysql"]["user"].GetString() + std::string(":") + tvs->jsonobj["mysql"]["pwd"].GetString() + std::string("@") + tvs->jsonobj["mysql"]["ip"].GetString() + std::string(":") + std::to_string(tvs->jsonobj["mysql"]["port"].GetInt()) + std::string("/eslam?ssl-mode=disabled");

	//mysqlx::Session mysql_ss("mysqlx://saictv:saictv@localhost:33060/eslam?ssl-mode=disabled");
	mysqlx::Session mysql_ss(mysql_login.c_str());
    //database selection
    std::stringstream mysql_db_sel;
    mysql_db_sel << "use " << json_msg["sql"]["database"].GetString() ;
    mysql_ss.sql(mysql_db_sel.str()).execute();

    auto mysql_stm = json_msg["sql"]["statement"].GetString();

    /*see if semicolon is at the end*/
    auto rsets = mysql_ss.sql(std::string(mysql_stm)).execute();

    if (rsets.hasData())
    {
        mysqlx::Row r;
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

        /*start object*/
        writer.StartObject();
        int indx = 0;
        while ((r = rsets.fetchOne()))
        {
            writer.Key(std::to_string(indx).c_str());
            writer.StartArray();
            for (unsigned tmp = 0; tmp < r.colCount(); tmp++)
            {
                switch(r[tmp].getType()){
                    case mysqlx::Value::Type::UINT64 : writer.Uint64((uint64_t)r[tmp]);break;
                    case mysqlx::Value::Type::INT64 : writer.Int64((int64_t)r[tmp]);break;
                    case mysqlx::Value::Type::STRING: writer.String(std::string(r[tmp]).c_str());break;
                    case mysqlx::Value::Type::DOUBLE: writer.Double((double)r[tmp]);break;
                    case mysqlx::Value::Type::BOOL: writer.Bool((bool)r[tmp]);break;
					case mysqlx::Value::Type::RAW: {
						/*
						r[tmp].print(std::cout);
						auto vele = r[tmp].getRawBytes();
						time_t vtime = *(uint32_t*)vele.first * 65536;
						char buf[256];
						tm v_tm;
						gmtime_s(&v_tm, &vtime);
						ctime_s(buf, 256, &vtime);
						std::cout << buf << std::endl;
						asctime_s(buf, 256, &v_tm);
						std::cout << buf << std::endl;
						writer.String(buf);
						break;
*/
						writer.String("rawbytes");
						break;
					}
					default:
						writer.String("null");
						break;

				}
			}
			writer.EndArray();

			indx++;
		}
		//websocket process id return to client
		writer.Key("wspid");
		writer.String(json_msg["wspid"].GetString());
		writer.EndObject();
            /*end object*/

        return std::string(sb.GetString());
    }
    else
    {
        return std::string("{\"rapidjson\":\"mysql result sets empty\"}");
    }
}
catch (mysqlx::Error const &e)
{
    std::cout << "mysql error: " << e.what() << std::endl;
    return std::string("{\"mysql\":\" Joyce-Xu, You encounter an error!\"}");
}
catch (std::exception const &e) {
    std::cout << "std error: " << e.what() << std::endl;
    return std::string("{\"std\":\" Joyce-Xu, You encounter an error!\"}");
}

std::string rignet_plc(server *s, const rapidjson::Document &&json_msg)
{
    return std::string("{\"rapidjson\":\"success\"}");
}

std::string rignet_nicard(server *s, const rapidjson::Document &&json_msg) try
{
#ifdef NI_ENABLE
    tv::Setting *tvs = tv::Setting::INSTANCE();

	if (json_msg.HasMember("savefiledata")) {
		auto sfdata = json_msg["savefiledata"].GetArray();
		time_t nt = time(NULL);
		struct tm ptm;
		localtime_s(&ptm, &nt);
		auto parentdir_num = tvs->filepath.length() - 12;
		auto parentdir = tvs->filepath.substr(0, parentdir_num);
		auto savepath = parentdir + std::to_string(ptm->tm_mon + 1) + std::to_string(ptm->tm_mday) + std::to_string(ptm->tm_hour) + std::to_string(ptm->tm_min) + std::to_string(ptm->tm_sec) + ".txt";

		std::ofstream fs(savepath);
		for (int index = 0; index < (int)sfdata.Size(); index++) {
			fs << sfdata[index].GetDouble() << std::endl;
		}
		return std::string("{\"nicard\":\"saved\"}");
	}
    auto nicard_json = tvs->jsonobj["nicard"]["daqmx"].GetObject();

    //NI Card Initialization
    auto nic = NICard_s::instance(*tvs);
    if (nic)
    {
        return nic->return_buffer_json();
    }else{
        return std::string("{\"raidjson\":\"failed to initialize NI card.\"}");
    }
#else
	return std::string("{\"result\":\"NI_DISABLE\"}");
#endif
}
catch (std::exception const &e)
{
    std::cout << "nicard error: " << e.what() << std::endl;
    return std::string("{\"mysql\":\" Joyce-Xu, You encounter an error!\"}");
}
