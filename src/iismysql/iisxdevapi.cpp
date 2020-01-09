#include "precomp.h"
#include "mysqlx/xdevapi.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Data.Json.h"

using namespace winrt::Windows::Data::Json;

REQUEST_NOTIFICATION_STATUS CMYSQL::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider) try
{
    winrt::init_apartment();
    //parse http body to a json object
    JsonObject request;
    TV::Utility ut;
    std::vector<char> buf;
    ut.ReadEntity<char>(pHttpContext, buf);
    //request.Parse(winrt::to_hstring(buf.data()));
    buf.shrink_to_fit();
    const char *requestcstr = buf.data();
    if (!JsonObject::TryParse(winrt::to_hstring(requestcstr), request))
    {
        pHttpContext->GetResponse()->SetStatus(400, "NOT JSON");
        return RQ_NOTIFICATION_CONTINUE;
    }

    //    assert(request.HasKey(L"user") & request.HasKey(L"password") & request.HasKey(L"ip") & request.HasKey(L"port"));

    //mysql execution
    auto mysqlsessionstr = std::wstring(L"mysqlx://") + request.GetNamedString(L"user").c_str() + L":" + request.GetNamedString(L"password").c_str() + L"@" + request.GetNamedString(L"ip").c_str() + L":" + request.GetNamedString(L"port").c_str() + L"/" + request.GetNamedString(L"database") + L"?ssl-mode=disabled";

    //mysqlx::Session mysqlsession(winrt::to_string(mysqlsessionstr).c_str());
    mysqlx::abi2::r0::SqlResult rsets;
    try
    {
        auto statement=request.GetNamedString(L"statement").c_str();
        //mysqlx::Session mysqlsession(winrt::to_string(mysqlsessionstr).c_str());
        //mysqlx::Session mysqlsession(u"mysqlx://root:hust123@localhost:33060/tvdemo?ssl-mode=disabled");
        //mysqlx::Session mysqlsession("mysqlx://localhost:33060/tvdemo?user=saic&password=saic?123&ssl-mode=disabled");
        mysqlx::Session mysqlsession(mysqlx::SessionOption::HOST, "localhost", mysqlx::SessionOption::PORT, 33060, mysqlx::SessionOption::USER, "saic", mysqlx::SessionOption::PWD, "hust123", mysqlx::SessionOption::DB, "tvdemo", mysqlx::SessionOption::AUTH, mysqlx::AuthMethod::MYSQL41, mysqlx::SessionOption::SSL_MODE, mysqlx::SSLMode::DISABLED);

        rsets = mysqlsession.sql(statement).execute();
    }
    catch (const mysqlx::Error &e)
    {
        pHttpContext->GetResponse()->SetStatus(400, e.what());
        return RQ_NOTIFICATION_FINISH_REQUEST;
    }
    //store mysql results to a json object
    JsonObject response;

    if (rsets.hasData())
    {
        mysqlx::Row r;
        int index = 0;
        while ((r = rsets.fetchOne()))
        {
            JsonArray element;
            for(u_long tmp=0;tmp<r.colCount();tmp++){
               switch (r[tmp].getType())
               {
               case mysqlx::Value::Type::INT64:
                   element.Append(JsonValue::CreateNumberValue(r[tmp]));
                   break;
               case mysqlx::Value::Type::UINT64:
                   element.Append(JsonValue::CreateNumberValue(r[tmp]));
                   break;
               case mysqlx::Value::Type::DOUBLE:
                   element.Append(JsonValue::CreateNumberValue(r[tmp]));
                   break;
               case mysqlx::Value::Type::BOOL:
                   element.Append(JsonValue::CreateBooleanValue((bool)r[tmp]));
                   break;
               case mysqlx::Value::Type::STRING:
                   element.Append(JsonValue::CreateStringValue(winrt::to_hstring(std::string(r[tmp]))));
                   break;
               case mysqlx::Value::Type::VNULL:
                   element.Append(JsonValue::CreateNullValue());
                   break;
               default:
                   element.Append(JsonValue::CreateNullValue());
                   break;
               } 
            }
            response.Insert(winrt::to_hstring(index), element);
            index++;
        }
    }else{
        response.Insert(L"error", JsonValue::CreateStringValue(L"empty results."));
    }

    PCSTR pszBuf = winrt::to_string(response.ToString()).c_str();

    //modify http response
    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType=HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer=(PVOID)pszBuf;
    dataChunk.FromMemory.BufferLength = (USHORT)strlen(pszBuf);

    pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk, 1, FALSE, FALSE, NULL);

    return RQ_NOTIFICATION_CONTINUE;
}
catch (winrt::hresult_error const &ex)
{
    pHttpContext->GetResponse()->SetStatus(400, winrt::to_string(ex.message()).c_str());
    return RQ_NOTIFICATION_FINISH_REQUEST;
}