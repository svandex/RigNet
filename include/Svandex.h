#pragma once

#include <string>

#include "Windows.h"
#include "pathcch.h"

#include <vector>

namespace svandex{
    namespace tools{
    //get current path of executable file
    std::string GetCurrentPath();
    std::string GetEnvVariable(const char* pEnvName);
    }
}