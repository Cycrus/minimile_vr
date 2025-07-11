#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

extern std::wstring StrToWstr(std::string raw_str);

extern std::string WstrToStr(std::wstring raw_wstr);