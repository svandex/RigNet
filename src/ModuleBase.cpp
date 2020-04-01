#include "ModuleBase.h"

std::string ModuleBase::SeverVariable(const char *vname)
{
    PCSTR _pcstr;
    DWORD _dword;
    if (_pHttpContext != nullptr && _pHttpContext->GetServerVariable(vname, &_pcstr, &_dword) == S_OK)
    {
        return std::string(_pcstr);
    }
    else
    {
        return std::string();
    }
}

void ModuleBase::WriteStringEntityChunk(const char *pszBuf)
{
    HTTP_DATA_CHUNK dataChunk;
    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    if (!pszBuf)
    {
        dataChunk.FromMemory.pBuffer = (PVOID) "ModuleBase::WriteStringEntityChunk get Null pointer.";
        dataChunk.FromMemory.BufferLength = (USHORT)strlen("ModuleBase::WriteStringEntityChunk get Null pointer.");
    }
    else
    {
        dataChunk.FromMemory.pBuffer = (PVOID)pszBuf;
        dataChunk.FromMemory.BufferLength = (USHORT)strlen(pszBuf);
    }

    _pHttpContext->GetResponse()->WriteEntityChunks(&dataChunk, 1, FALSE, FALSE, NULL);
}

void ModuleBase::WriteStringEntityChunk(void)
{
    WriteStringEntityChunk(_return_string.GetString());
}

void ModuleBase::AddStringKeyValueToRet(const char *key, const char *value)
{
    rapidjson::Document::AllocatorType &_dallocator = _default_document.GetAllocator();
    _default_document.AddMember(rapidjson::Value(key, _dallocator), rapidjson::Value(value, _dallocator), _dallocator);
}

void ModuleBase::AddExceptionMessageToRet(std::exception &e)
{
    AddStringKeyValueToRet("exception", e.what());
}

bool ModuleBase::Initialization()
{
    //数据没有全部存储在_pointer_to_entityChunks中
    auto _remaining_count = _pHttpContext->GetRequest()->GetRemainingEntityBytes();

    //请求数据大小对应的动态内存，该内存会被IIS自动删除
    _pointer_to_entity = _pHttpContext->AllocateRequestMemory(_remaining_count);

    DWORD _bytes_read = 0;
    DWORD _total_bytes_read = 0;

    uint8_t _error_times = 0;
    while (_pHttpContext->GetRequest()->GetRemainingEntityBytes() != 0)
    {
        HRESULT hr = _pHttpContext->GetRequest()->ReadEntityBody((char *)_pointer_to_entity + _total_bytes_read, (DWORD)(_remaining_count - _total_bytes_read), false, &_bytes_read);

        if (hr == S_OK && _bytes_read > 0)
        {
            _total_bytes_read += _bytes_read;
        }
        else
        {
            _error_times++;
            if (_error_times > 3)
            {
                break;
            }
        }

        if (hr == ERROR_HANDLE_EOF)
        {
            break;
        }
    }

    _size_of_entity = _total_bytes_read;

    if (_total_bytes_read != _remaining_count || _error_times > 3)
    {
        return false;
    }else{
        return true;
    }
}