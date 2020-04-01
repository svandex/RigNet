#include "ModuleStaffManagement.h"

REQUEST_NOTIFICATION_STATUS CIISTVManagement::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    std::map<std::string, int> _url_map;
    _url_map["/login"] = 1;
    _url_map["/regst"] = 2;
    _url_map["/exist"] = 3;

    HTTP_REQUEST *_RawHttpRequest = pHttpContext->GetRequest()->GetRawHttpRequest();
    const char *_json_string = (const char *)(_RawHttpRequest->pEntityChunks->FromMemory.pBuffer);

    // 除非是上传文件等，默认情况下Chunk的数量都是1
    if (_RawHttpRequest->EntityChunkCount == 1 || !_default_document.Parse(_json_string).HasParseError())
    {
        try
        {
            switch (_url_map[SeverVariable("HTTP_URL")])
            {
            case 1: //登录
            {
                TestValidation::LoginUser _LoginUser(_json_string);
                _default_document = std::move(_LoginUser.Login().Ret());
                break;
            }
            case 2: //注册
            {
                TestValidation::RegisterUser _RegisterUser(_json_string);
                _default_document = std::move(_RegisterUser.Register().Ret());
                break;
            }
            case 3: //存在
            {
                TestValidation::BaseUser _BaseUser(_json_string);
                if (_BaseUser.Existence())
                {
                    AddStringKeyValueToRet("result", "true");
                }
                else
                {
                    AddStringKeyValueToRet("result", "false");
                }
                break;
            }
            default: //URL错误
            {
                AddStringKeyValueToRet("message", "incorrect url.");
            }
            }
        }
        catch (std::exception &e)
        {
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
