#pragma once

#include <memory>

#include "controller_device_driver.h"
#include "openvr_driver.h"

/**
 * The wrapper for the driver devices. Exposes the individual driver server objects to the OpenVR framework.
 */
class MyDeviceProvider : public vr::IServerTrackedDeviceProvider
{
public:
	/**
	 * Initializes the treadmill driver.
	 */
	vr::EVRInitError Init( vr::IVRDriverContext *pDriverContext ) override;

	/**
	 * Default method.
	 */
	const char *const *GetInterfaceVersions() override;

	/**
	 * Main loop method. Calls the device driver RunFrame method.
	 */
	void RunFrame() override;

	/**
	 * Unused.
	 */
	bool ShouldBlockStandbyMode() override;

	/**
	 * Unused.
	 */
	void EnterStandby() override;

	/**
	 * Unused.
	 */
	void LeaveStandby() override;

	/**
	 * Deactivates the device driver.
	 */
	void Cleanup() override;

private:
	std::unique_ptr<TreadmillDeviceDriver> treadmill_device_;
};