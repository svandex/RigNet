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

bool tv::Setting::LoadSetting()
{

    //load setting from json file
    return true;
}

std::string rignet_mysql(server *s, const rapidjson::Document &&json_msg) try
{
    mysqlx::Session mysql_ss("localhost", 33060, "svandex", "y1ban@Hust");
    mysql_ss.sql("use funtestdemo;").execute();

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
        while (r = rsets.fetchOne())
        {
            writer.Key(std::to_string(indx).c_str());
            writer.StartArray();
            for (unsigned tmp = 0; tmp < r.colCount(); tmp++)
            {
                if (r[tmp].getType() == mysqlx::Value::Type::INT64)
                {
                    writer.Int(int(r[tmp]));
                }
                if (r[tmp].getType() == mysqlx::Value::Type::STRING){
                    writer.String(std::string(r[tmp]).c_str());
                }
                else
                {
                    writer.String("null");
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
}

std::string rignet_plc(server *s, const rapidjson::Document &&json_msg)
{
    return std::string("{\"rapidjson\":\"success\"}");
}
std::string rignet_nicard(server *s, const rapidjson::Document &&json_msg)
{
    return std::string("{\"rapidjson\":\"success\"}");
}