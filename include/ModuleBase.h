#ifndef __MODULEBASE_H__
#define __MODULEBASE_H__

#include "HeaderPrecompilation.h"

class ModuleBase{
public:
    ModuleBase(){
        _default_document.SetObject();
        _default_writer.Reset(_return_string);
    }
    ModuleBase(const ModuleBase &) = delete;

protected:
    //获得比如HTTP_URL、CONTENT_LENGTH类参数
    std::string SeverVariable(const char *vname);

    //通常只会返回一个JSON字符串
    void WriteStringEntityChunk(void);//默认返回_return_string
    void WriteStringEntityChunk(const char *);

    //当模块内的执行代码无法创建返回的字符串时，使用内部自带的Document对象创建返回字符串
    void AddExceptionMessageToRet(std::exception &);

    //增加额外的字符串信息
    void AddStringKeyValueToRet(const char *, const char *);

    //确定能够写入到_default_writer的Document对象
    void EndReturnValueConstruction(rapidjson::Document &&_return_document){
        _return_document.Accept(_default_writer);
    }

    //请求中Transfer-Encoding是chunk时，需要循环读取所有正文内容
    bool Initialization();

protected:
    IHttpContext *_pHttpContext = nullptr;

    rapidjson::Document _default_document;
    rapidjson::StringBuffer _return_string;
    rapidjson::Writer<rapidjson::StringBuffer> _default_writer;

    void *_pointer_to_entity = nullptr;
    unsigned long _size_of_entity = 0;
};
#endif