#pragma once

#include <array>
#include <string>
#include <atomic>
#include <thread>

#include "openvr_driver.h"
#include "treadmill_capture.h"

/**
 * A collection of input identifiers for the input handle array.
 */
enum TreadmillComponents
{
	TRIGGER_VALUE,
	TRACKPAD_Y,
	JOYSTICK_Y,
	TRACKPAD_X,
	JOYSTICK_X,
	MAX
};

/**
 * This class serves as the main treadmill driver container. It contains the treadmill
 * serial connection object and connects its output with the OpenVR input system.
 */
class TreadmillDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	/**
	 * Constructor. Sets the role and serial number of the driver.
	 */
	TreadmillDeviceDriver( vr::ETrackedControllerRole role );

	/**
	 * Overridden. Called when the driver activates. Establishes the serial connection and starts
	 * the background thread receiving data from the load cell. Also defines the input capabilities
	 * of the module.
	 */
	vr::EVRInitError Activate( uint32_t unObjectId ) override;

	/**
	 * Unused.
	 */
	void EnterStandby() override;

	/**
	 * Unused.
	 */
	void *GetComponent( const char *pchComponentNameAndVersion ) override;

	/**
	 * Unused.
	 */
	void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	/**
	 * Unused.
	 */
	vr::DriverPose_t GetPose() override;

	/**
	 * Closes the serial connection to the load cell device and stops thre receiver thread.
	 */
	void Deactivate() override;

	/**
	 * Returns the serial number (unique identifier inside the OpenVR framework.
	 */
	const std::string &GetSerialNumber();

	/**
	 * Main worker method. Maps the read treadmill value to the OpenVR input.
	 */
	void RunTreadmillFrame();

	/**
	 * Unused.
	 */
	void ProcessTreadmillEvent( const vr::VREvent_t &vrevent );
	
private:
	std::atomic< vr::TrackedDeviceIndex_t > controller_index_;
	vr::ETrackedControllerRole treadmill_role_;

	std::string serial_number_;

	std::array< vr::VRInputComponentHandle_t, TreadmillComponents::MAX > input_handles_;

	std::atomic< bool > is_active_;

	TreadmillCapture treadmill_device_;
};