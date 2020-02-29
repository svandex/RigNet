#include "config.h"

REQUEST_NOTIFICATION_STATUS CIISSQLite::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    //_response json object initialization
    rapidjson::Document _response, _request;
    _response.SetObject();
    _request.SetObject();
    rapidjson::Document::AllocatorType &_dallocator = _response.GetAllocator();

    std::vector<char> _requestV;
    TV::Utility::ReadEntity(pHttpContext, _requestV);

    std::string result;
    //sqlite execution
    if (execution("management", "select * from `0用户信息`", result))
    {
    }

    //sqlite processing
    if (_request.Parse(_requestV.data()).HasParseError())
    {
        pHttpContext->GetResponse()->SetStatus(400, "request data prasing error.");
        _response.AddMember("error", TV::ERROR_JSON_CREAT, _dallocator);
    }
    else if (_response.HasMember("sessionId"))
    {
        try
        {
            auto dbpath = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME)[0] + std::string("\\db\\management.db");
            SQLite::Database db(dbpath.c_str(), SQLite::OPEN_READWRITE);

            std::stringstream querystream;
            querystream << "select * from `0用户信息` where `会话标识`=\'" << _request["sessionId"].GetString() << "\'";
            SQLite::Statement query(db, querystream.str());

            query.executeStep();
            if (!query.hasRow())
            {
                _response.AddMember("error", TV::ERROR_SESSION_EXPIRED, _dallocator);
                pHttpContext->GetResponse()->SetStatus(400, "relogin.");
            }
            else
            {
                std::istringstream s_lmtime(query.getColumn("最近更新").getText());
                std::tm t = {};
                s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
                auto retval = std::difftime(std::time(nullptr), std::mktime(&t));

                if (retval > SVANDEX_SESSION_EXPIRED)
                {
                    _response.AddMember("error", TV::ERROR_SESSION_EXPIRED, _dallocator);
                    pHttpContext->GetResponse()->SetStatus(400, "session expired, relogin.");
                }
                else
                {
                    std::string _result;
                    //sqlite execution
                    if (!execution(_request["database"].GetString(), _request["statement"].GetString(), _result) || _response.Parse(_result.c_str()).HasParseError())
                    {
                        _response.AddMember("error", TV::ERROR_SQLITE_EXEC, _dallocator);
                        pHttpContext->GetResponse()->SetStatus(500, "sqlite execution error");
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            _response.AddMember("error", TV::ERROR_SQLITE_EXEC, _dallocator);
            pHttpContext->GetResponse()->SetStatus(500, e.what());
        }
    }
    else
    {
        pHttpContext->GetResponse()->SetStatus(400, "json has incorrect key.");
        _response.AddMember("error", TV::ERROR_JSON_KEY, _dallocator);
    }

    //json object to string
    rapidjson::StringBuffer _s;
    rapidjson::Writer<rapidjson::StringBuffer> _w(_s);
    _response.Accept(_w);

    //get json string
    PCSTR pszBuf = _s.GetString();

    //send http chunk
    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer = (PVOID)pszBuf;
    dataChunk.FromMemory.BufferLength = (USHORT)strlen(pszBuf);

    pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk, 1, FALSE, FALSE, NULL);

    return RQ_NOTIFICATION_CONTINUE;
}

bool CIISSQLite::execution(const char *dbname, const char *statement, std::string &returnValue)
{
    rapidjson::Document _return;
    _return.SetObject();
    rapidjson::Document::AllocatorType &_dallocator = _return.GetAllocator();
    rapidjson::StringBuffer _s;
    rapidjson::Writer<rapidjson::StringBuffer> _w(_s);

    try
    {
        auto dbpath = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME)[0] + std::string("\\db\\") + dbname + ".db";
        SQLite::Database db(dbpath.c_str(), SQLite::OPEN_READWRITE);
        SQLite::Statement query(db, statement);

        size_t _step = 0;
        while (query.executeStep())
        {
            rapidjson::Value one_row(rapidjson::kArrayType);
            one_row.SetArray();
#define TV_SQLITE_LEGACY_API
#ifdef TV_SQLITE_LEGACY_API
            for (int index = 0; index < query.getColumnCount(); index++)
            {
                switch (query.getColumn(index).getType())
                {
                case SQLITE_TEXT:
                {
                    one_row.PushBack(rapidjson::Value(query.getColumn(index).getString().c_str(), _dallocator).Move(), _dallocator);
                    break;
                }
                case SQLITE_INTEGER:
                {
                    one_row.PushBack(rapidjson::Value(std::to_string(query.getColumn(index).getInt64()).c_str(), _dallocator).Move(), _dallocator);
                    break;
                }
                case SQLITE_FLOAT:
                {
                    one_row.PushBack(rapidjson::Value(std::to_string(query.getColumn(index).getDouble()).c_str(), _dallocator).Move(), _dallocator);
                    break;
                }
                default:
                {
                    one_row.PushBack(rapidjson::Value(""), _dallocator);
                }
                break;
                }
            }
            _return.AddMember(rapidjson::Value(std::to_string(_step).c_str(), _dallocator), one_row.Move(), _dallocator);
            _step++;
#else
            for (int index = 0; index < query.getColumnCount(); index++)
            {
                rapidjson::Value one_element(rapidjson::kObjectType);
                switch (query.getColumn(index).getType())
                {
                case SQLITE_TEXT /* constant-expression */:
                    /* code */
                    one_element.AddMember(rapidjson::Value(query.getColumnName(index), _dallocator), rapidjson::Value(query.getColumn(index).getText(), _dallocator), _dallocator);
                    break;
                case SQLITE_INTEGER:
                    one_element.AddMember(rapidjson::Value(query.getColumnName(index), _dallocator), rapidjson::Value(query.getColumn(index).getInt64()).Move(), _dallocator);
                    break;
                case SQLITE_FLOAT:
                    one_element.AddMember(rapidjson::Value(query.getColumnName(index), _dallocator), rapidjson::Value(query.getColumn(index).getDouble()).Move(), _dallocator);
                    break;
                default:
                    one_element.AddMember(rapidjson::Value(query.getColumnName(index), _dallocator), rapidjson::Value(""), _dallocator);
                    break;
                }
                one_row.PushBack(one_element.Move(), _dallocator);
            }
            _return.AddMember("result", one_row.Move(), _dallocator);
#endif
        }
        _return.Accept(_w);
        returnValue.assign(_s.GetString());
        return true;
    }
    catch (std::exception &e)
    {
        returnValue.assign(e.what());
        return false;
    }
}