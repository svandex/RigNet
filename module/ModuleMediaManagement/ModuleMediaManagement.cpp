#include "ModuleMediaManagement.h"

REQUEST_NOTIFICATION_STATUS CIISTVMedia::OnExecuteRequestHandler(IN IHttpContext *pHttpContext, IN IHttpEventProvider *pProvider)
{
    rapidjson::Document::AllocatorType &_dallocator = _default_document.GetAllocator();

    if (std::strcmp(pHttpContext->GetRequest()->GetHttpMethod(), "POST") != 0)
    {
        _default_document.AddMember("http_method", "false", _dallocator);
    }

    if (Initialization())
    {
        _default_document.AddMember("http_chunk_read_data", "true", _dallocator);
        /*
        g_mime_init();

        GMimeMessage *message;
        GMimeStream *stream;
        GMimeParser *parser;

        stream = g_mime_stream_mem_new_with_buffer((const char *)_pointer_to_entity, _size_of_entity);
        parser = g_mime_parser_new_with_stream(stream);

        g_object_unref(stream);

        message = g_mime_parser_construct_message(parser, NULL);

        g_object_unref(parser);
        g_mime_shutdown();
        */
    }
    else
    {
        _default_document.AddMember("http_chunk_read_processing", "failed", _dallocator);
    }

    //返回值字符串
    EndReturnValueConstruction(std::move(_default_document));
    WriteStringEntityChunk();

    return RQ_NOTIFICATION_CONTINUE;
}
