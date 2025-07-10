#include "controller_device_driver.h"

#include "driverlog.h"
#include "vrmath.h"

// Let's create some variables for strings used in getting settings.
// "<my_driver>_<section>"
static const char* my_controller_main_settings_section = "driver_CustomTreadmill";

// These are the keys we want to retrieve the values for in the settings
static const char *my_controller_settings_key_model_number = "mycontroller_model_number";
static const char *my_controller_settings_key_serial_number = "mycontroller_serial_number";


TreadmillDeviceDriver::TreadmillDeviceDriver( vr::ETrackedControllerRole role )
{
	is_active_ = false;
	treadmill_role_ = role;

	// We have our model number and serial number stored in SteamVR settings. We need to get them and do so here.
	char model_number[ 1024 ];
	vr::VRSettings()->GetString( my_controller_main_settings_section, my_controller_settings_key_model_number, model_number, sizeof( model_number ) );
	model_number_ = model_number;
	serial_number_ = model_number;

	DriverLog( "My Controller Model Number: %s", model_number_.c_str() );
	DriverLog( "My Controller Serial Number: %s", serial_number_.c_str() );
}

vr::EVRInitError TreadmillDeviceDriver::Activate( uint32_t unObjectId )
{
	is_active_ = true;
	my_controller_index_ = unObjectId;

	// Properties are stored in containers, usually one container per device index.
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(my_controller_index_);

	// First, let's set the model number.
	vr::VRProperties()->SetStringProperty(container, vr::Prop_ModelNumber_String, model_number_.c_str());

	// Let's tell SteamVR our role which we received from the constructor earlier.
	vr::VRProperties()->SetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, treadmill_role_);


	// Now let's set up our inputs
	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String, "{CustomTreadmill}/input/mycontroller_profile.json");

	// Let's set up our "A" button. We've defined it to have a touch and a click component.
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/a/touch", &input_handles_[TreadmillComponents::a_touch]);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/a/click", &input_handles_[TreadmillComponents::a_click]);

	// Let's set up our trigger. We've defined it to have a value and click component.
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trigger/value", &input_handles_[TreadmillComponents::trigger_value], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateBooleanComponent(container, "/input/trigger/click", &input_handles_[TreadmillComponents::trigger_click]);

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
	// Let's retrieve the Hmd pose to base our controller pose off.

	// First, initialize the struct that we'll be submitting to the runtime to tell it we've updated our pose.
	vr::DriverPose_t pose = { 0 };

	// These need to be set to be valid quaternions. The device won't appear otherwise.
	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;

	vr::TrackedDevicePose_t hmd_pose{};

	// GetRawTrackedDevicePoses expects an array.
	// We only want the hmd pose, which is at index 0 of the array so we can just pass the struct in directly, instead of in an array
	vr::VRServerDriverHost()->GetRawTrackedDevicePoses( 0.f, &hmd_pose, 1 );

	// Get the position of the hmd from the 3x4 matrix GetRawTrackedDevicePoses returns
	const vr::HmdVector3_t hmd_position = HmdVector3_From34Matrix( hmd_pose.mDeviceToAbsoluteTracking );
	// Get the orientation of the hmd from the 3x4 matrix GetRawTrackedDevicePoses returns
	const vr::HmdQuaternion_t hmd_orientation = HmdQuaternion_FromMatrix( hmd_pose.mDeviceToAbsoluteTracking );

	// pitch the controller 90 degrees so the face of the controller is facing towards us
	const vr::HmdQuaternion_t offset_orientation = HmdQuaternion_FromEulerAngles( 0.f, DEG_TO_RAD(90.f), 0.f );

	// Set the pose orientation to the hmd orientation with the offset applied.
	pose.qRotation = hmd_orientation * offset_orientation;

	const vr::HmdVector3_t offset_position = {
		treadmill_role_ == vr::TrackedControllerRole_LeftHand ? -0.15f : 0.15f, // translate the controller left/right 0.15m depending on its role
		0.1f,																		// shift it up a little to make it more in view
		-0.5f,																		// put each controller 0.5m forward in front of the hmd so we can see it.
	};

	// Rotate our offset by the hmd quaternion (so the controllers are always facing towards us), and add then add the position of the hmd to put it into position.
	const vr::HmdVector3_t position = hmd_position + ( offset_position * hmd_orientation );

	// copy our position to our pose
	pose.vecPosition[ 0 ] = position.v[ 0 ];
	pose.vecPosition[ 1 ] = position.v[ 1 ];
	pose.vecPosition[ 2 ] = position.v[ 2 ];

	// The pose we provided is valid.
	// This should be set is
	pose.poseIsValid = true;

	// Our device is always connected.
	// In reality with physical devices, when they get disconnected,
	// set this to false and icons in SteamVR will be updated to show the device is disconnected
	pose.deviceIsConnected = true;

	// The state of our tracking. For our virtual device, it's always going to be ok,
	// but this can get set differently to inform the runtime about the state of the device's tracking
	// and update the icons to inform the user accordingly.
	pose.result = vr::TrackingResult_Running_OK;

	return pose;
}

void TreadmillDeviceDriver::EnterStandby()
{
}

void TreadmillDeviceDriver::Deactivate()
{
	// unassign our controller index (we don't want to be calling vrserver anymore after Deactivate() has been called
	my_controller_index_ = vr::k_unTrackedDeviceIndexInvalid;
}

void TreadmillDeviceDriver::RunTreadmillFrame()
{
	// Update our inputs here. For actual inputs coming from hardware, these will probably be read in a separate thread.
	vr::VRDriverInput()->UpdateBooleanComponent( input_handles_[TreadmillComponents::a_click], false, 0 );
	vr::VRDriverInput()->UpdateBooleanComponent( input_handles_[TreadmillComponents::a_touch], false, 0 );

	vr::VRDriverInput()->UpdateBooleanComponent( input_handles_[TreadmillComponents::trigger_click], false, 0 );
	vr::VRDriverInput()->UpdateScalarComponent( input_handles_[TreadmillComponents::trigger_value], 0.f, 0 );
}

void TreadmillDeviceDriver::ProcessTreadmillEvent( const vr::VREvent_t &vrevent )
{
	switch ( vrevent.eventType )
	{
		// Listen for haptic events
		case vr::VREvent_Input_HapticVibration:
		{
			if ( vrevent.data.hapticVibration.componentHandle == input_handles_[TreadmillComponents::haptic] )
			{
				float duration = vrevent.data.hapticVibration.fDurationSeconds;
				float frequency = vrevent.data.hapticVibration.fFrequency;
				float amplitude = vrevent.data.hapticVibration.fAmplitude;

				DriverLog( "Haptic event triggered for %s hand. Duration: %.2f, Frequency: %.2f, Amplitude: %.2f", treadmill_role_ == vr::TrackedControllerRole_LeftHand ? "left" : "right",
					duration, frequency, amplitude );
			}
			break;
		}
		default:
			break;
	}
}

const std::string & TreadmillDeviceDriver::MyGetSerialNumber()
{
	return serial_number_;
}