#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

/**
 * The main class responsible for connecting to the treadmill load cell
 * hardware. Opens a background tread reading its data and publishes it
 * on a standardized interface method.
 */
class TreadmillCapture
{
public:
    /**
     * We handle all setup and destruction in dedicated methods.
     * We do not rely on object lifetime here to have more possibilities
     * in the use of the driver.
     */
    TreadmillCapture() = default;
    ~TreadmillCapture() = default;

    /**
     * Sets up the serial connection by actively seraching for the correct device
     * and starts the whole background capture thread.
     */
    void StartBackgroundCapture();
    
    /**
     * Stops and joins the background capture thread.
     */
    void StopBackgroundCapture();

    /**
     * Returns the last read treadmill value. Only thing it does is holding a very
     * brief lock, so that it keeps to be as cheap as possible.
     */
    float GetTreadmillValue();
    
    /**
     * Returns true if the background thread is currently active.
     */
    bool isActive();

    /**
     * Returns true if the serial connection is established. Correctly represents the connection
     * state in every moment with a latency of max 1-2 seconds.
     */
    bool isConnected();

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

    /**
     * Returns the com port id string of the first connected USB serial device
     * which contains the given substring in its device name.
     * The names of all currently connected devices can be listed on Windows with
     * the powershell command:
     *      Get-CimInstance Win32_SerialPort | Select-Object Name, DeviceID, Description
     */
    std::wstring FindSerialPort(std::wstring device_substring);

    /**
     * Extracts the port id of a full com port name string. Used in FindSerialPort.
     */
    static std::wstring ExtractSerialPortFromName(std::wstring serial_name);

    /**
     * Opens a serial connection to the given com port with the given baud rate.
     */
    int OpenDevice(std::wstring com_port, DWORD baud_rate);

    /**
     * Starts the update loop thread.
     */
    int StartUpdateLoop();

    /**
     * Stops and joins the update loop thread.
     */
    int StopUpdateLoop();

    /**
     * Closes the serial connection.
     */
    int CloseDevice();

    /**
     * Actively reads the serial connection to return the next sent value if valid.
     */
    float ReadValue();

    /**
     * A loop running as long as active_ is true. It periodically reads the serial port
     * and updates the treadmill value which can then be read with the GetTreadmillValue()
     * method.
     */
    void UpdateValueLoop();
};
