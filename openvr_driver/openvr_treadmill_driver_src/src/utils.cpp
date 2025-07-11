#include "utils.h"

std::wstring StrToWstr(std::string raw_str)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, raw_str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, raw_str.c_str(), -1, &wstr[0], size);
    return wstr;
}

std::string WstrToStr(std::wstring raw_wstr)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, raw_wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, raw_wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str;
}