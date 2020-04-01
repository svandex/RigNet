#include "ModuleSQLite.h"

REQUEST_NOTIFICATION_STATUS CIISSQLite::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    HTTP_REQUEST *_RawHttpRequest = pHttpContext->GetRequest()->GetRawHttpRequest();
    const char *_json_string = (const char *)(_RawHttpRequest->pEntityChunks->FromMemory.pBuffer);

    if (_RawHttpRequest->EntityChunkCount == 1 || !_default_document.Parse(_json_string).HasParseError()){
        try{
            TestValidation::DataUser _DataUser(_json_string);
            _default_document = std::move(_DataUser.Data().Ret());
        }catch(std::exception&e){
            AddExceptionMessageToRet(e);
        }
    }
    else
    {
        AddStringKeyValueToRet("message", "chunk number exceeds one./n json parsing failed.");
    }

    //返回值字符串
    EndReturnValueConstruction(std::move(_default_document));
    WriteStringEntityChunk();

    return RQ_NOTIFICATION_CONTINUE;
}