#ifndef __HEADERPRECOMPILATION_H__
#define __HEADERPRECOMPILATION_H__

//#define WIN32_LEAN_AND_MEAN
#define TV_PROJECT_NAME "TVNET"

/*
GMIME
#if defined(_MSC_VER)
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif
#include <basetsd.h>
#include "gmime/gmime.h"
*/

//STL
#include <vector>
#include <array>
#include <iostream>
#include <regex>
#include <algorithm>
#include <fstream>
#include <locale>
#include <codecvt>


/*
rapidjson include header files

https://github.com/tencent/rapidjson
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#pragma pop_macro("min")
#pragma pop_macro("max")
*/

#include "rapidjson/stringbuffer.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"

/*
mysql xdev api header files

https://github.com/mysql/mysql-connector-cpp
#include "mysqlx/xdevapi.h"
*/


/*
sqlite header file
*/
#include "sqlite3.h"
#include "SQLiteCpp/SQLiteCpp.h"

/*
#include "RigNetModule.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Data.Json.h"
*/
//Project header files
#include "User.h"
#include "Svandex.h"
#include "ProjectUtility.h"
#include "ModuleBase.h"


namespace TV{
// error code table

/*client error*/
constexpr int32_t ERROR_ACTION = 4001;		   /*action failed*/
constexpr int32_t ERROR_JSON_PARSE = 4002;	 /*http request JSON parsing error*/
constexpr int32_t ERROR_JSON_KEY = 4003;	   /*key spell error in json format*/
constexpr int32_t ERROR_SQLITE_EXEC = 4004;	/*sqlite engine exeution error*/
constexpr int32_t ERROR_SQLITE_EMPTY = 4005;   /*sqlite execution return empty*/
constexpr int32_t ERROR_REGISTER_EXIST = 4006; /*User has been registered*/
constexpr int32_t ERROR_LOGIN_PWD = 4007;	  /*password error when logging in*/
constexpr int32_t ERROR_HTTP_CTYPE = 4008;	 /*content type not suppored*/
constexpr int32_t ERROR_NO_ENV = 4009;		   /*TV_PROJECT_NAME env variable not setup*/
constexpr int32_t ERROR_SQLITE_OPEN = 4010;	/*TV_PROJECT_NAME env variable not setup*/
constexpr int32_t ERROR_JPEG_FORMAT = 4011;	/*uploaded file format is incorrect*/

constexpr int32_t ERROR_SESSION_EXPIRED = 4444; /*action time expired*/

/*server error*/
constexpr int32_t ERROR_JSON_CREAT = 5001;		/*JSON construction error within server*/
constexpr int32_t ERROR_UUID_CREAT = 5002;		/*uuid error in server*/
constexpr int32_t ERROR_URL_NOEXIST = 5003;		/*URL doesn't exist*/
constexpr int32_t ERROR_JSON_CREAT_SEND = 5004; /*JSON creation error when sending payload*/
constexpr int32_t ERROR_REGEX = 5005;			/*regex error*/
constexpr int32_t ERROR_UPLOAD = 5006;			/*upload http body format error*/

/*debug*/
constexpr int32_t ERROR_DEBUG = 9999;

// success code table
constexpr int32_t SUCCESS_ACTION = 2001;

//url code
constexpr int32_t URL_LOGIN = 1;
constexpr int32_t URL_REGISTER = 2;
constexpr int32_t URL_SQLITE_DATA = 3;
constexpr int32_t URL_EXIST = 4;
constexpr int32_t URL_UPLOAD = 5;
constexpr int32_t URL_MYSQL_DATA = 6;
} // namespace TV

#endif