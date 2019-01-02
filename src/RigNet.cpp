#include "RigNet.h"

void on_message(server *s, websocketpp::connection_hdl hdl, server::message_ptr msg) try
{
    static std::map<std::string, std::function<std::string(server *, const rapidjson::Document &&)>> rig_dispatchlist_map;

    //dispatch initialization
    rig_dispatchlist_map["mysql"] = rignet_mysql;
    rig_dispatchlist_map["plc"] = rignet_plc;
    rig_dispatchlist_map["nicard"] = rignet_nicard;

    rapidjson::Document json_msg;
    if (json_msg.Parse(msg->get_payload().c_str()).HasParseError())
    {
        s->send(hdl, "{\"rapidjson\":\"parse error\"}", websocketpp::frame::opcode::TEXT);
        return;
    }

    if (json_msg.HasMember("comtype"))
    {
        if (json_msg["comtype"].IsString())
        {
            auto f = rig_dispatchlist_map[json_msg["comtype"].GetString()];
            auto result = f(s, std::move(json_msg));
            s->send(hdl, result, websocketpp::frame::opcode::TEXT);
            return;
        }
        else
        {
            s->send(hdl, "{\"rapidjson\":\"command type member should be string.\"}", websocketpp::frame::opcode::TEXT);
            return;
        }
    }
    else
    {
        s->send(hdl, "{\"rapidjson\":\"json has no member named comtype\"}", websocketpp::frame::opcode::TEXT);
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
    tv::Setting *tvs = tv::Setting::INSTANCE();

    mysqlx::Session mysql_ss(
            tvs->jsonobj["mysql"]["ip"].GetString(),
            tvs->jsonobj["mysql"]["port"].GetUint(),
            tvs->jsonobj["mysql"]["user"].GetString(),
            tvs->jsonobj["mysql"]["pwd"].GetString(),
            json_msg["sql"]["database"].GetString()
            );

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
                    case mysqlx::Value::Type::RAW : {
                                                        auto vele = r[tmp].getRawBytes();
                                                        time_t vtime=*vele.begin();
                                                        char buf[256];
                                                        //ctime_s(buf,256,&vtime);
                                                        tm v_tm;
                                                        localtime_s(&v_tm,&vtime);                             
                                                        asctime_s(buf,256,&v_tm);                             
                                                        writer.String(buf);
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

std::string rignet_plc(server *s, const rapidjson::Document &&json_msg)
{
    return std::string("{\"rapidjson\":\"success\"}");
}
std::string rignet_nicard(server *s, const rapidjson::Document &&json_msg)
{
    return std::string("{\"rapidjson\":\"success\"}");
}
