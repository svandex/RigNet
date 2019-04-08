#include "Svandex.h"

std::string svandex::tools::GetCurrentPath()
{
    TCHAR dest[MAX_PATH];
    if (!dest)
        return std::string();

    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL)
    {
        DWORD length = GetModuleFileName(hModule, dest, MAX_PATH);
    }
    else
    {
        return std::string();
    }

#if (NTDDI_VERSION >= NTDDI_WIN8)
    PathCchRemoveFileSpec(dest, MAX_PATH);
#else
    PathRemoveFileSpec(dest);
#endif
    size_t len = wcslen(dest) + 1;
    size_t converted = 0;
    char *destStr = (char *)malloc(len * sizeof(char));
    wcstombs_s(&converted, destStr, len, dest, _TRUNCATE);
    auto returnStr=std::string(destStr);
    free(destStr);
    return returnStr;
}