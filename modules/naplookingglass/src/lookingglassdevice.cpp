/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "lookingglassdevice.h"

// Local includes
#include "lookingglassservice.h"

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Third-party includes
#include <HoloPlayCore.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LookingGlassDevice)
	RTTI_CONSTRUCTOR(nap::LookingGlassService&)
	RTTI_PROPERTY("Serial",				&nap::LookingGlassDevice::mSerial,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("QuiltSettings",		&nap::LookingGlassDevice::mQuiltSettingsResource,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableCustomQuilt",	&nap::LookingGlassDevice::mEnableCustomQuiltResource,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RenderWindow",		&nap::LookingGlassDevice::mRenderWindow,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ForceInit",			&nap::LookingGlassDevice::mForceInit,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	const std::string sViewConeQuery		= "/calibration/viewCone/value";
    const std::string sQuiltColumnsQuery	= "/defaultQuilt/tileX";
    const std::string sQuiltRowsQuery		= "/defaultQuilt/tileY";
    const std::string sQuiltWidthQuery	    = "/defaultQuilt/quiltX";
    const std::string sQuiltHeightQuery	    = "/defaultQuilt/quiltY";


	//////////////////////////////////////////////////////////////////////////
	// LookingGlassDevice
	//////////////////////////////////////////////////////////////////////////

	LookingGlassDevice::LookingGlassDevice(LookingGlassService& service) :
		mService(&service) { }


	bool LookingGlassDevice::init(utility::ErrorState& errorState)
	{
		// Set the device index
		mDeviceIndex = mService->findDeviceIndex(mSerial);

		// Ensure the device is valid, but skip the check if 'ForceInit' is enabled.
		if (!mForceInit)
		{
			if (!errorState.check(isValid(), "Failed to find device with serial '%s'", mSerial.c_str()))
				return false;
		}

		// Log a warning if 'ForceInit' is enabled while no device was found.
		else if (!isValid())
		{
			nap::Logger::warn("Failed to find device with serial '%s'. Continuing anyway as 'ForceInit' is enabled. This mode is strictly for development purposes only!", mSerial.c_str());
		}

		// Acquire quilt settings from device
		mOptimalQuiltSettings.mWidth	= hpc_GetDevicePropertyInt(mDeviceIndex, sQuiltWidthQuery.c_str());
		mOptimalQuiltSettings.mHeight	= hpc_GetDevicePropertyInt(mDeviceIndex, sQuiltHeightQuery.c_str());
		mOptimalQuiltSettings.mColumns	= hpc_GetDevicePropertyInt(mDeviceIndex, sQuiltColumnsQuery.c_str());
		mOptimalQuiltSettings.mRows		= hpc_GetDevicePropertyInt(mDeviceIndex, sQuiltRowsQuery.c_str());

		// Copy custom quilt settings from public property
		mCustomQuiltSettings = mQuiltSettingsResource;

		// Ensure the custom quilt flag is not modified at runtime
		mEnableCustomQuilt = isValid() ? mEnableCustomQuiltResource : true;

		// Determine whether to use a custom or recommended quilt
		const QuiltSettings& settings = getQuiltSettings();

		// Verify quilt settings
		if (!errorState.check(settings.mColumns > 0 && settings.mRows > 0, "Invalid rows/columns in property 'QuiltSettings'"))
			return false;

		if (!errorState.check(settings.mWidth > 0 && settings.mHeight > 0, "Invalid width/height in property 'QuiltSettings'"))
			return false;

		if (isValid())
		{
			// Acquire calibration settings from device
			mCalibration.pitch = hpc_GetDevicePropertyPitch(mDeviceIndex);
			mCalibration.tilt = hpc_GetDevicePropertyTilt(mDeviceIndex);
			mCalibration.center = hpc_GetDevicePropertyCenter(mDeviceIndex);
			mCalibration.invView = hpc_GetDevicePropertyInvView(mDeviceIndex);
			mCalibration.subp = hpc_GetDevicePropertySubp(mDeviceIndex);
			mCalibration.displayAspect = hpc_GetDevicePropertyDisplayAspect(mDeviceIndex);
			mCalibration.ri = hpc_GetDevicePropertyRi(mDeviceIndex);
			mCalibration.bi = hpc_GetDevicePropertyBi(mDeviceIndex);

			// Store window position and screen resolution
			mWindowPosition = { hpc_GetDevicePropertyWinX(mDeviceIndex), hpc_GetDevicePropertyWinY(mDeviceIndex) };
			mScreenResolution = { hpc_GetDevicePropertyScreenW(mDeviceIndex), hpc_GetDevicePropertyScreenH(mDeviceIndex) };

			// Store view cone
			mViewCone = hpc_GetDevicePropertyFloat(mDeviceIndex, sViewConeQuery.c_str());
		}

		// Position and resize the render window appropriately
		if (mRenderWindow != nullptr)
		{
			if (isValid())
			{
				glm::ivec2 position = glm::ivec2(getPosition());
				glm::ivec2 size = glm::ivec2(getResolution());
				mRenderWindow->setPosition(position);
				mRenderWindow->setSize(size);
			}
			else
			{
				// Hide if the device is invalid
				mRenderWindow->hide();
			}
		}
		return true;
	}
}
