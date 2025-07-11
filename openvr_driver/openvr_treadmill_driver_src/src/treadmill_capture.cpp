#include "treadmill_capture.h"

#include <limits>
#include <cmath>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <regex>

#include "driverlog.h"
#include "utils.h"

#pragma comment(lib, "setupapi.lib")

void TreadmillCapture::StartBackgroundCapture()
{
    this->StartUpdateLoop();
}

void TreadmillCapture::StopBackgroundCapture()
{
    this->StopUpdateLoop();
    this->CloseDevice();
}

std::wstring TreadmillCapture::FindSerialPort(std::wstring device_substring)
{
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return L"";
    }

    SP_DEVINFO_DATA devInfoData = {};
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    std::wstring found_port = L"";

    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfoData); ++i) {
        TCHAR buffer[256];
        DWORD buffersize = 0;
        
        // Get the friendly name
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &devInfoData, SPDRP_FRIENDLYNAME,
            nullptr, (PBYTE)buffer, sizeof(buffer), &buffersize)) {

            // Optional: detect Arduino by matching known patterns
            std::wstring name = std::wstring(buffer);
            if (name.find(device_substring) != std::wstring::npos) {
                found_port = TreadmillCapture::ExtractSerialPortFromName(name);
                break;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return found_port;
}

std::wstring TreadmillCapture::ExtractSerialPortFromName(std::wstring serial_name)
{
    std::wregex pattern(L"\\((COM\\d+)\\)");
    std::wsmatch match;

    if (std::regex_search(serial_name, match, pattern) && match.size() > 1)
    {
        std::wstring comport = match[1].str();
        return comport;
    }
    else
    {
        return L"";
    }
}

int TreadmillCapture::OpenDevice(std::wstring com_port, DWORD baud_rate)
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);

	this->com_port_ = com_port;
	this->serial_handle_ = CreateFile(com_port.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

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

    this->is_connected_ = true;
    DriverLog("Connected to serial port");

    return 0;
}

float TreadmillCapture::ReadValue()
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);

    ClearCommError(this->serial_handle_, &this->errors_, &this->status_);

    char ch;
    DWORD bytesRead = 0;
    int chars_read = 0;
    bool error = false;

    while (this->active_) {
        if (!ReadFile(this->serial_handle_, &ch, 1, &bytesRead, NULL) || bytesRead == 0)
        {
            this->buffer_[0] = '\0';
            error = true;
            break;
        }

        if (chars_read > 128)
        {
            this->buffer_[0] = '\0';
            error = true;
            break;
        }

        if (ch == 13)
        {
            this->buffer_[chars_read] = '\0';
            break;
        }
        else if (ch != 10)
        {
            this->buffer_[chars_read] = ch;
            chars_read++;
        }
    }

    if (!error)
    {
        try
        {
            return (float)std::atof(this->buffer_);
        }
        catch (const std::exception&)
        {
            DriverLog("Cannot cast buffer to float");
            return std::numeric_limits<float>::quiet_NaN();
        }
    }

    return std::numeric_limits<float>::quiet_NaN();
}

void TreadmillCapture::UpdateValueLoop()
{
    while (this->active_)
    {
        float tmp_value = this->ReadValue();
        bool error = isnan(tmp_value);
        if (error)
            tmp_value = 0.0;
        this->value_lock_.lock();
        this->treadmill_value_ = tmp_value;
        this->value_lock_.unlock();

        if (error)
        {
            this->CloseDevice();
            std::wstring device = this->FindSerialPort(L"Arduino");
            DriverLog("Found Device: (below)");
            DriverLog(WstrToStr(device).c_str());
            this->OpenDevice(device, 9600);
            Sleep(2000);
        }
    }
}

int TreadmillCapture::StartUpdateLoop()
{
    this->active_ = true;
    this->update_loop_thread_ = std::thread(&TreadmillCapture::UpdateValueLoop, this);
    return 0;
}

int TreadmillCapture::StopUpdateLoop()
{
    this->active_ = false;
    if (this->update_loop_thread_.joinable())
    {
        this->update_loop_thread_.join();
    }
    return 0;
}

int TreadmillCapture::CloseDevice()
{
    std::lock_guard<std::mutex> lock(this->serial_lock_);
    CloseHandle(this->serial_handle_);
    this->is_connected_ = false;
    return 0;
}

float TreadmillCapture::GetTreadmillValue()
{
    std::lock_guard<std::mutex> lock(this->value_lock_);
    return this->treadmill_value_;
}