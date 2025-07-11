#pragma once

#include <Windows.h>
#include <iostream>
#include <string>

/**
 * A helper function to cast a string into a wide string.
 */
extern std::wstring StrToWstr(std::string raw_str);

/**
 * A helper function to cast a wide string into a string.
 */
extern std::string WstrToStr(std::wstring raw_wstr);