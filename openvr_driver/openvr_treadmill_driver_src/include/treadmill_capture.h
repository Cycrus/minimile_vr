#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

class TreadmillCapture
{
public:
    TreadmillCapture() = default;
    ~TreadmillCapture() = default;

    std::string FindSerialPort();
    int OpenDevice(std::string com_port, DWORD baud_rate);
    void StartUpdateLoop();
    void StopUpdateLoop();
    int CloseDevice();

private:
    HANDLE serial_handle_;
    std::string com_port_;
    DWORD errors_;
    COMSTAT status_;
    char buffer_[256] = { 0 };
    std::mutex serial_lock_;
    std::mutex value_lock_;
    float treadmill_value_ = 0.0f;
    std::thread update_loop_thread_;
    bool active_ = false;

    std::wstring StrToWstr(std::string raw_str);
    float ReadValue();
    void UpdateValueLoop();
};
