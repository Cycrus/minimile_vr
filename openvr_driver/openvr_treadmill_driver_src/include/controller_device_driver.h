#pragma once

#include <array>
#include <string>

#include "openvr_driver.h"
#include <atomic>
#include <thread>

enum TreadmillComponents
{
	a_touch,
	a_click,

	trigger_value,
	trigger_click,

	haptic,

	MAX
};

class TreadmillDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	TreadmillDeviceDriver( vr::ETrackedControllerRole role );

	vr::EVRInitError Activate( uint32_t unObjectId ) override;

	void EnterStandby() override;

	void *GetComponent( const char *pchComponentNameAndVersion ) override;

	void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	vr::DriverPose_t GetPose() override;

	void Deactivate() override;

	// ----- Functions we declare ourselves below -----

	const std::string &MyGetSerialNumber();

	void RunTreadmillFrame();
	void ProcessTreadmillEvent( const vr::VREvent_t &vrevent );
	
private:
	std::atomic< vr::TrackedDeviceIndex_t > my_controller_index_;

	vr::ETrackedControllerRole treadmill_role_;

	std::string model_number_;
	std::string serial_number_;

	std::array< vr::VRInputComponentHandle_t, TreadmillComponents::MAX > input_handles_;

	std::atomic< bool > is_active_;
};