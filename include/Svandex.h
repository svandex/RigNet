#pragma once

#include <string>
#include <typeinfo>
#include <fstream>

#include "Windows.h"
#include "pathcch.h"

#include "httpserv.h"

namespace svandex{
    namespace tools{
    //get current path of executable file
    std::string GetCurrentPath();
    }
}