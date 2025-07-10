#include "device_provider.h"

#include "driverlog.h"


vr::EVRInitError MyDeviceProvider::Init( vr::IVRDriverContext *pDriverContext )
{
	VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );

	// Let's add our controllers to the system.
	treadmill_device_ = std::make_unique< TreadmillDeviceDriver >(vr::TrackedControllerRole_Treadmill);

	// Now we need to tell vrserver about our controllers.
	if ( !vr::VRServerDriverHost()->TrackedDeviceAdded(treadmill_device_->MyGetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, treadmill_device_.get() ) )
	{
		DriverLog( "Failed to create treadmill device!" );
		return vr::VRInitError_Driver_Unknown;
	}

	return vr::VRInitError_None;
}

const char *const *MyDeviceProvider::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

// *depracated*
bool MyDeviceProvider::ShouldBlockStandbyMode()
{
	return false;
}

// *main driver loop*
void MyDeviceProvider::RunFrame()
{
	// call our devices to run a frame
	if (this->treadmill_device_ != nullptr)
	{
		this->treadmill_device_->RunTreadmillFrame();
	}

	//Now, process events that were submitted for this frame.
	vr::VREvent_t vrevent{};
	while ( vr::VRServerDriverHost()->PollNextEvent( &vrevent, sizeof( vr::VREvent_t ) ) )
	{
		if (this->treadmill_device_ != nullptr)
		{
			this->treadmill_device_->ProcessTreadmillEvent(vrevent);
		}
	}
}

void MyDeviceProvider::EnterStandby()
{
}

void MyDeviceProvider::LeaveStandby()
{
}

void MyDeviceProvider::Cleanup()
{
	this->treadmill_device_ = nullptr;
}