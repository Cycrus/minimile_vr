#include "controller_device_driver.h"

#include "driverlog.h"
#include "utils.h"

// Let's create some variables for strings used in getting settings.
// "<my_driver>_<section>"
static const char* treadmill_main_settings_section = "driver_CustomTreadmill";

// These are the keys we want to retrieve the values for in the settings
static const char *treadmill_settings_key_model_number = "mycontroller_model_number";


TreadmillDeviceDriver::TreadmillDeviceDriver( vr::ETrackedControllerRole role )
{
	is_active_ = false;
	treadmill_role_ = role;

	// We have our model number and serial number stored in SteamVR settings. We need to get them and do so here.
	char model_number[ 1024 ];
	vr::VRSettings()->GetString(treadmill_main_settings_section, treadmill_settings_key_model_number, model_number, sizeof( model_number ) );
	serial_number_ = model_number;

	DriverLog( "Treadmill Serial Number: %s", serial_number_.c_str() );
}

vr::EVRInitError TreadmillDeviceDriver::Activate( uint32_t unObjectId )
{
	is_active_ = true;
	controller_index_ = unObjectId;

	this->treadmill_device_.StartBackgroundCapture();

	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(controller_index_);

	vr::VRProperties()->SetStringProperty(container, vr::Prop_ModelNumber_String, serial_number_.c_str());
	vr::VRProperties()->SetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, treadmill_role_);
	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String, "{CustomTreadmill}/input/mycontroller_profile.json");

	// Let's set up our trigger. We've defined it to have a value and click component.
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trigger/value", &input_handles_[TreadmillComponents::TRIGGER_VALUE], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trackpad/y", &input_handles_[TreadmillComponents::TRACKPAD_Y], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/y", &input_handles_[TreadmillComponents::JOYSTICK_Y], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trackpad/x", &input_handles_[TreadmillComponents::TRACKPAD_X], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/x", &input_handles_[TreadmillComponents::JOYSTICK_X], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);


	return vr::VRInitError_None;
}

void * TreadmillDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
	return nullptr;
}

void TreadmillDeviceDriver::DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
	if ( unResponseBufferSize >= 1 )
		pchResponseBuffer[ 0 ] = 0;
}

vr::DriverPose_t TreadmillDeviceDriver::GetPose()
{
	vr::DriverPose_t pose = { 0 }; //This example doesn't use Pose, so this method is just returning a default Pose.
	pose.poseIsValid = false;
	pose.result = vr::TrackingResult_Calibrating_OutOfRange;
	pose.deviceIsConnected = true;

	vr::HmdQuaternion_t quat;
	quat.w = 1;
	quat.x = 0;
	quat.y = 0;
	quat.z = 0;

	pose.qWorldFromDriverRotation = quat;
	pose.qDriverFromHeadRotation = quat;

	return pose;
}

void TreadmillDeviceDriver::EnterStandby()
{
}

void TreadmillDeviceDriver::Deactivate()
{ 
	// unassign our controller index (we don't want to be calling vrserver anymore after Deactivate() has been called
	controller_index_ = vr::k_unTrackedDeviceIndexInvalid;

	this->treadmill_device_.StopBackgroundCapture();
}

void TreadmillDeviceDriver::RunTreadmillFrame()
{
	float treadmill_value = this->treadmill_device_.GetTreadmillValue();

	vr::VRDriverInput()->UpdateScalarComponent(input_handles_[TreadmillComponents::TRIGGER_VALUE], treadmill_value, 0);
	vr::VRDriverInput()->UpdateScalarComponent(input_handles_[TreadmillComponents::TRACKPAD_Y], treadmill_value, 0);
	vr::VRDriverInput()->UpdateScalarComponent(input_handles_[TreadmillComponents::JOYSTICK_Y], treadmill_value, 0);
	vr::VRDriverInput()->UpdateScalarComponent(input_handles_[TreadmillComponents::TRACKPAD_X], 0.0f, 0);
	vr::VRDriverInput()->UpdateScalarComponent(input_handles_[TreadmillComponents::JOYSTICK_X], 0.0f, 0);
}

void TreadmillDeviceDriver::ProcessTreadmillEvent( const vr::VREvent_t &vrevent )
{
}

const std::string & TreadmillDeviceDriver::GetSerialNumber()
{
	return serial_number_;
}
