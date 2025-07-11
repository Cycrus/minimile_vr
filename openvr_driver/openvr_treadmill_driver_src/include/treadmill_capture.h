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

    void StartBackgroundCapture();
    void StopBackgroundCapture();

    float GetTreadmillValue();

private:
    HANDLE serial_handle_ = INVALID_HANDLE_VALUE;
    std::wstring com_port_ = L"";
    DWORD errors_;
    COMSTAT status_;

    std::thread update_loop_thread_;
    std::mutex serial_lock_;
    std::mutex value_lock_;

    bool active_ = false;
    bool is_connected_ = false;
    char buffer_[256] = { 0 };
    float treadmill_value_ = 0.0f;

    std::wstring FindSerialPort(std::wstring device_substring);
    static std::wstring ExtractSerialPortFromName(std::wstring serial_name);
    int OpenDevice(std::wstring com_port, DWORD baud_rate);
    int StartUpdateLoop();
    int StopUpdateLoop();
    int CloseDevice();
    float ReadValue();
    void UpdateValueLoop();
};
