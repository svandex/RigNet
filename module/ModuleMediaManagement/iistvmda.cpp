#include "config.h"

REQUEST_NOTIFICATION_STATUS CIISTVMedia::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    //_response json object initialization
    rapidjson::Document _response;
    _response.SetObject();
    rapidjson::Document::AllocatorType &_dallocator = _response.GetAllocator();

    std::vector<char> _request;
    TV::Utility::ReadEntity(pHttpContext,_request);

    //IIS may truncate characters, check the last 2 character

    TV::Utility::MultiDataParser mdp(_request, "");
    if(_response.Parse(mdp.metadata().c_str()).HasParseError()){
        _response.AddMember("error", TV::ERROR_JSON_CREAT, _dallocator);
    }else{
        for(size_t index=0;index<_response["metadata"].GetArray().Size();index++){
            if (_response["metadata"][(rapidjson::SizeType)index].HasMember("storable"))
            {
                //store element into file folder
                if(mdp.store(index)){
                    _response.AddMember("success",TV::SUCCESS_ACTION,_dallocator);
                }else{
                    _response.AddMember("error",TV::ERROR_UPLOAD,_dallocator);
                    pHttpContext->GetResponse()->SetStatus(500, "Storing element failed!");
                }
            }
        }
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