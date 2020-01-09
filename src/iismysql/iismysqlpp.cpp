#include "config.h"
#include "mysql++.h"
#include "intrin.h"

REQUEST_NOTIFICATION_STATUS CMYSQL::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    rapidjson::Document request;
    std::vector<char> buf;
    TV::Utility::ReadEntity<char>(pHttpContext, buf);
    //request.Parse(winrt::to_hstring(buf.data()));
    buf.shrink_to_fit();
    const char *requestcstr = buf.data();
    if (request.Parse(requestcstr).HasParseError())
    {
        pHttpContext->GetResponse()->SetStatus(400, "NOT JSON");
        return RQ_NOTIFICATION_CONTINUE;
    }

    //    assert(request.HasKey(L"user") & request.HasKey(L"password") & request.HasKey(L"ip") & request.HasKey(L"port"));

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType &dallocator = document.GetAllocator();

    //connection to mysql database
    try
    {
        mysqlpp::Connection conn;
        //if (conn.connect("mysql_cpp_data", "localhost", "saic", "hust123", 3306))
        if (conn.connect(request["database"].GetString(), request["ip"].GetString(), request["user"].GetString(), request["password"].GetString()))
        {
            auto stm = request["statement"].GetString();
            mysqlpp::Query query = conn.query(stm);
            mysqlpp::UseQueryResult res = query.use();
            if (res)
            {
                /*
                kArrayType Value to store each object produced by each row
                */
                rapidjson::Value queryResult(rapidjson::kArrayType);

                /*loop for each row to create a JSON object*/
                while (mysqlpp::Row one_row = res.fetch_row())
                {
                    rapidjson::Value one_row_json(rapidjson::kObjectType);
                    rapidjson::Value blobdata(rapidjson::kArrayType);

                    //loop each column
                    for (size_t icol = 0; icol < res.field_names()->size(); icol++)
                    //for (size_t icol = 0; icol < 1; icol++)
                    {
                        //move resouces to document
                        const char* colname=res.field_name((int)icol).data();
                        //one_row_json.AddMember(rapidjson::Value(colname, dallocator).Move(), rapidjson::Value(one_row.at(icol).data(), dallocator).Move(), dallocator);
                        one_row_json.AddMember(rapidjson::Value(std::to_string(icol).c_str(), dallocator).Move(), rapidjson::Value(one_row.at(icol).data(), dallocator).Move(), dallocator);
                    }

                    //process each blob
                    /*
                    const char *database = one_row.at(1).data();
                    unsigned __int64 *data = (unsigned __int64 *)(database + 4);
                    //array
                    for (size_t index = 0; index < 1000; index++)
                    {
                        //auto conv_befor = *(data + index);
                        //PLC has different endian with Intel Computer
                        auto conv_after = (_byteswap_uint64(*(data + index)));
                        //unsigned __int64 *p1 = &conv_befor;
                        unsigned __int64 *p2 = &conv_after;

                        //auto element = *(double*)p1;
                        double element = *(double*)p2;

                        //blobdata.PushBack(rapidjson::Value(*(data + index)).Move(), dallocator);
                        blobdata.PushBack(rapidjson::Value(element).Move(), dallocator);
                    }

                    one_row_json.AddMember("blob_data", blobdata.Move(), dallocator);
                    */
                    queryResult.PushBack(one_row_json.Move(), dallocator);
                }

                /*
                Add queryResult to document
                */
                document.AddMember("query_result", queryResult.Move(), dallocator);
            }
            else
            {
                document.AddMember("error_message", rapidjson::Value(query.error(), dallocator), dallocator);
            }
        }
        else
        {
            document.AddMember("error_message", "DB connection failed", dallocator);
        }
    }
    catch (const mysqlpp::Exception &e)
    {
        pHttpContext->GetResponse()->SetStatus(400, e.what());
        return RQ_NOTIFICATION_CONTINUE;
    }
    catch (const std::exception &e)
    {
        pHttpContext->GetResponse()->SetStatus(400, e.what());
        return RQ_NOTIFICATION_CONTINUE;
    }

   // document.AddMember("library_value", rapidjson::Value(mysqlpp::get_library_version()).Move(), dallocator);

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    document.Accept(writer);

    PCSTR pszBuf = s.GetString();

    //modify http response
    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer = (PVOID)pszBuf;
    dataChunk.FromMemory.BufferLength = (USHORT)strlen(pszBuf);

    pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk, 1, FALSE, FALSE, NULL);

    return RQ_NOTIFICATION_CONTINUE;
}