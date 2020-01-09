#include "precomp.h"

REQUEST_NOTIFICATION_STATUS CIISTVManagement::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    //http body
    std::vector<char> _body;
    TV::Utility::ReadEntity(pHttpContext, _body);

    //json
    rapidjson::Document _request, _response;
    _request.SetObject();
    _response.SetObject();
    rapidjson::StringBuffer _s;
    rapidjson::Writer<rapidjson::StringBuffer> _w(_s);
    rapidjson::Document::AllocatorType &_dallocator = _response.GetAllocator();

    //parse request http body
    if (_request.Parse(_body.data()).HasParseError())
    {
        _response.AddMember("error", TV::ERROR_JSON_PARSE, _dallocator);
        pHttpContext->GetResponse()->SetStatus(400, "parsing request body to json failed.");
    }
    else
    {
        //get url
        auto vForwardURL = TV::Utility::GetServerVariable("HTTP_URL", pHttpContext);

        //different action according to url
        if (vForwardURL.compare("/register") == 0)
        { //register
        if(_request.HasMember("id")&&_request.HasMember("password")&&_request.HasMember("role")&&_request.HasMember("contact")&&_request.HasMember("name")){
            try{
                auto dbpath = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME)[0] + std::string("\\db\\management.db");
                SQLite::Database db(dbpath.c_str(), SQLite::OPEN_READWRITE);
                std::stringstream querystream;
                querystream << "select * from `0用户信息` where `工号`=\'" << _request["id"].GetString() << "\'";
                SQLite::Statement query(db, querystream.str());
                query.executeStep();
                if (query.hasRow())
                {
                    _response.AddMember("error", TV::ERROR_REGISTER_EXIST, _dallocator);
                    pHttpContext->GetResponse()->SetStatus(400, "User has existed.");
                }else{
                    std::stringstream querystream2;
                    querystream2<<"insert into `0用户信息` values(\'"
					<< _request["role"].GetString() << "\',\'"
					<< _request["id"].GetString() << "\',\'"
					<< _request["password"].GetString() << "\',\'"
					<< _request["contact"].GetString() << "\',"
					<< "0,\'expired\',\'"
					<< Svandex::tools::GetCurrentTimeFT().c_str() << "\',\'"
					<< _request["name"].GetString()
					<< "\')";

                    SQLite::Statement query2(db, querystream2.str());
                    query2.exec();
                    _response.AddMember("success", TV::SUCCESS_ACTION, _dallocator);
                }
            }catch(std::exception &e){
                _response.AddMember("error", TV::ERROR_SQLITE_EXEC, _dallocator);
                pHttpContext->GetResponse()->SetStatus(500, e.what());
            }
        }
        else
        {
            _response.AddMember("error", TV::ERROR_JSON_CREAT_SEND, _dallocator);
            pHttpContext->GetResponse()->SetStatus(400, "json memeber incorrect.");
        }
        }
        if (vForwardURL.compare("/login") == 0)
        { //login
            if (_request.HasMember("id") && _request.HasMember("password"))
            {
                try{
                    auto dbpath = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME)[0] + std::string("\\db\\management.db");
                    SQLite::Database db(dbpath.c_str(), SQLite::OPEN_READWRITE);

                    std::stringstream querystream;
                    querystream << "select * from `0用户信息` where `工号`=\'" << _request["id"].GetString() << "\'";
                    SQLite::Statement query(db, querystream.str());

                    query.executeStep();
                    if (query.hasRow())
                    {
                        //comapre password
                        if (query.getColumn(2).getString().compare(_request["password"].GetString()) == 0)
                        { //login successfully
                            std::stringstream updatestream;
                            if (query.getColumn(5).getString().compare("expired") == 0)
                            { //first login
                                auto uuid = Svandex::tools::GetUUID();
                                updatestream << "update `0用户信息` set `会话标识`=\"" << uuid.c_str() << "\",`最近更新`=\'" << Svandex::tools::GetCurrentTimeFT().c_str() << "\' where `工号`=\'" << _request["id"].GetString() << "\'";
                                SQLite::Statement query2(db, updatestream.str());
                                query2.exec();

                                //send back success infomation
                                _response.AddMember("sessionId", rapidjson::Value(uuid.c_str(), _dallocator), _dallocator);
                                _response.AddMember("roleId", rapidjson::Value(query.getColumn(0).getString().c_str(), _dallocator), _dallocator);
                            }
                            else
                            { //last modified time
                                std::istringstream s_lmtime(query.getColumn(6).getText());
                                std::tm t = {};
                                s_lmtime >> std::get_time(&t, "%Y-%m-%d %T");
                                auto retval = std::difftime(std::time(nullptr), std::mktime(&t));

                                if (retval > SVANDEX_SESSION_EXPIRED)
                                {
                                    std::stringstream querystream3;
                                    querystream3 << "update `0用户信息` set `最近更新`=\'" << Svandex::tools::GetCurrentTimeFT().c_str() << "\' where `工号`=\'" << _request["id"].GetString() << "\'";
                                    SQLite::Statement query3(db, querystream3.str());
                                    query3.exec();
                                }

                                //send back success infomation
                                _response.AddMember("sessionId", rapidjson::Value(query.getColumn(5).getText(), _dallocator), _dallocator);
                                _response.AddMember("roleId", rapidjson::Value(query.getColumn(0).getText(), _dallocator), _dallocator);
                            }
                        }
                        else
                        { //login failed
                            _response.AddMember("error", TV::ERROR_LOGIN_PWD, _dallocator);
                            pHttpContext->GetResponse()->SetStatus(400, "password is incorrect");
                        }
                    }
                    else
                    {
                        throw std::exception("no user found.");
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
                _response.AddMember("error", TV::ERROR_JSON_CREAT_SEND, _dallocator);
                pHttpContext->GetResponse()->SetStatus(400, "json memeber incorrect.");
            }
        }
        if (vForwardURL.compare("/exist") == 0)
        { //exist
            if (_request.HasMember("id"))
            {
                try
                {
                    auto dbpath = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME)[0] + std::string("\\db\\management.db");
                    SQLite::Database db(dbpath.c_str(), SQLite::OPEN_READWRITE);

                    //query string
                    std::stringstream querystream;
                    querystream << "select * from `0用户信息` where `工号`=\'" << _request["id"].GetString() << "\'";
                    SQLite::Statement query(db, querystream.str());
                    query.executeStep();
                    if (query.hasRow())
                    {
                        _response.AddMember("success", TV::SUCCESS_ACTION, _dallocator);
                    }
                    else
                    {
                        _response.AddMember("error", TV::ERROR_SQLITE_EMPTY, _dallocator);
                        pHttpContext->GetResponse()->SetStatus(400, "No user exist.");
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
                _response.AddMember("error", TV::ERROR_JSON_CREAT_SEND, _dallocator);
                pHttpContext->GetResponse()->SetStatus(400, "json memeber incorrect.");
            }
        }
    }

    _response.Accept(_w);
    PCSTR pszBuf = _s.GetString();

    //modify http response
    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer = (PVOID)pszBuf;
    dataChunk.FromMemory.BufferLength = (USHORT)strlen(pszBuf);

    pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk, 1, FALSE, FALSE, NULL);

    return RQ_NOTIFICATION_CONTINUE;
}