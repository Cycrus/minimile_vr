#include "treadmill_capture.h"

#include <limits>
#include <cmath>

std::string TreadmillCapture::FindSerialPort()
{

}

int TreadmillCapture::OpenDevice(std::string com_port, DWORD baud_rate)
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);

	this->com_port_ = com_port;
    std::wstring com_port_wstr = this->StrToWstr(com_port);
	this->serial_handle_ = CreateFile(com_port_wstr.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // Config serial port parameter
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);
    GetCommState(this->serial_handle_, &serialParams);
    serialParams.BaudRate = baud_rate;
    serialParams.ByteSize = 8;
    serialParams.StopBits = 1;
    serialParams.Parity = 0;
    SetCommState(this->serial_handle_, &serialParams);

    // Config timeout parameters
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 100;
    timeout.ReadTotalTimeoutConstant = 100;
    timeout.ReadTotalTimeoutMultiplier = 100;
    timeout.WriteTotalTimeoutConstant = 100;
    timeout.WriteTotalTimeoutMultiplier = 20;
    SetCommTimeouts(this->serial_handle_, &timeout);
}

float TreadmillCapture::ReadValue()
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);

    ClearCommError(this->serial_handle_, &this->errors_, &this->status_);
    std::string line;

    char ch;
    DWORD bytesRead = 0;

    while (this->active_) {
        if (!ReadFile(this->serial_handle_, &ch, 1, &bytesRead, NULL) || bytesRead == 0) {
            break;
        }

        if (ch == '\n') {
            break;
        }
    }

    if (!line.empty()) {
        try {
            return std::stof(line);
        }
        catch (const std::exception&) {
            return std::numeric_limits<float>::quiet_NaN();
        }
    }

    return std::numeric_limits<float>::quiet_NaN();
}

void TreadmillCapture::UpdateValueLoop()
{
    while (this->active_)
    {
        std::lock_guard<std::mutex> lock(this->value_lock_);
        this->treadmill_value_ = this->ReadValue();
    }
}

void TreadmillCapture::StartUpdateLoop()
{
    this->active_ = true;
    update_loop_thread_ = std::thread(&TreadmillCapture::UpdateValueLoop, this);
}

void TreadmillCapture::StopUpdateLoop()
{
    this->active_ = false;
    update_loop_thread_.join();
}

int TreadmillCapture::CloseDevice()
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);
    CloseHandle(this->serial_handle_);
}

std::wstring TreadmillCapture::StrToWstr(std::string raw_str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, raw_str.c_str(), -1, nullptr, 0);
    if (size_needed == 0) {
        return L"";
    }

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, raw_str.c_str(), -1, &wstr[0], size_needed);

    // Remove null terminator
    wstr.resize(size_needed - 1);
    return wstr;
}
