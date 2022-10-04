/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "lookingglassservice.h"

#include "lookingglassdevice.h"

#include <nap/logger.h>
#include <utility/stringutils.h>
#include <rtti/objectptr.h>
#include <rtti/factory.h>

 // Third-party includes
#include <HoloPlayCore.h>

RTTI_BEGIN_ENUM(nap::LookingGlassServiceConfiguration::EHoloPlayLicenseType)
	RTTI_ENUM_VALUE(nap::LookingGlassServiceConfiguration::EHoloPlayLicenseType::Commercial, "Commercial"),
	RTTI_ENUM_VALUE(nap::LookingGlassServiceConfiguration::EHoloPlayLicenseType::NonCommercial, "NonCommercial")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::LookingGlassServiceConfiguration)
	RTTI_PROPERTY("ApplicationName", &nap::LookingGlassServiceConfiguration::mAppName, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LicenseType", &nap::LookingGlassServiceConfiguration::mLicenseType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Log State", &nap::LookingGlassServiceConfiguration::mLogState, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LookingGlassService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	// Address used to access the 'state' property of a device
	constexpr char* sStateQuery = "/state";

	// Value that indicates a device's calibration is valid
	constexpr char* sStateOk = "ok";

	// HoloPlay Core SDK functions return a nonzero value if a preallocated buffer is of insufficient size.
	constexpr int sHoloPlayCoreSuccess = 0;

	static std::string getClientErrorMessage(hpc_client_error errorCode)
	{
		switch (errorCode)
		{
		case hpc_client_error::hpc_CLIERR_NOSERVICE:
			return "HoloPlay Service not installed or not running.";
		case hpc_client_error::hpc_CLIERR_VERSIONERR:
			return "HoloPlay Service/HoloPlay Core version mismatch.";
		case hpc_client_error::hpc_CLIERR_SERIALIZEERR:
			return  "Something wrong with the serialization of message data being sent to HoloPlay Service.";
		case hpc_client_error::hpc_CLIERR_DESERIALIZEERR:
			return  "Something wrong with the serialization of message data being received from HoloPlay Service.";
		case hpc_client_error::hpc_CLIERR_MSGTOOBIG:
			return "Message sent was too large and was rejected.";
		case hpc_client_error::hpc_CLIERR_SENDTIMEOUT:
			return  "Interprocess pipe send timeout. HoloPlay Service was detected but did not consume message.";
		case hpc_client_error::hpc_CLIERR_RECVTIMEOUT:
			return "Interprocess pipe receive timeout. HoloPlay Service received message but did not respond.";
		case hpc_client_error::hpc_CLIERR_PIPEERROR:
			return  "Interprocess pipe broken. HoloPlay Service may not be running.";
		case hpc_client_error::hpc_CLIERR_APPNOTINITIALIZED:
			return "hpc_RefreshState called before hpc_InitializeApp.";
		default:
			return "Unknown error";
		}
	};


	//////////////////////////////////////////////////////////////////////////

	LookingGlassService::LookingGlassService(ServiceConfiguration* configuration) :
		Service(configuration) {}


	void LookingGlassService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<LookingGlassDeviceObjectCreator>(*this));
	}


	bool LookingGlassService::init(nap::utility::ErrorState& errorState)
	{
		// Acquire service config
		LookingGlassServiceConfiguration* config = getConfiguration<LookingGlassServiceConfiguration>();

		// Allocate char buffer
		std::string char_buffer(256, '\0');
		hpc_GetHoloPlayCoreVersion((char*)char_buffer.data(), char_buffer.size());
		nap::Logger::info("HoloPlay Core v%s", char_buffer.c_str());

		// Get the license type
		hpc_license_type license_type = config->mLicenseType == LookingGlassServiceConfiguration::EHoloPlayLicenseType::Commercial ?
			hpc_license_type::hpc_LICENSE_COMMERCIAL :
			hpc_license_type::hpc_LICENSE_NONCOMMERCIAL;

		// The call to hpc_InitializeApp is required to complete successfully for an app to communicate with the Holoplay service
		hpc_client_error error_code = hpc_InitializeApp(config->mAppName.c_str(), license_type);
		if (error_code != hpc_client_error::hpc_CLIERR_NOERROR)
		{
            // Must tear down the message pipe
            hpc_TeardownMessagePipe();

            std::string errorMessage = "Failed to initialize LookingGlass service: " + getClientErrorMessage(error_code);
            if (mAllowFailure)
            {
                nap::Logger::warn(errorMessage);
                return true;
            }
            else {
                errorState.fail(errorMessage.c_str());
                return false;
            }
		}

		char_buffer.assign(char_buffer.size(), '\0');
		hpc_GetHoloPlayServiceVersion((char*)char_buffer.data(), char_buffer.size());
		nap::Logger::info("HoloPlay Service v%s", char_buffer.c_str());

		// Check number of devices connected with valid calibrations
		int device_count = hpc_GetNumDevices();
		nap::Logger::info("%d device%s connected", device_count, device_count == 1 ? "" : "s");
		nap::Logger::info(device_count > 0 ? "Initialize devices..." : "Initialization without device...");

		for (int i = 0; i < device_count; i++)
		{
			// Information to log
			nap::Logger::info("Info for Looking Glass device at display %d:", i);

			// We can check the 'state' property to detect whether the USB cable is connected
			// We require this in order to fetch the serial number and window coordinates
			hpc_GetDevicePropertyString(i, sStateQuery, (char*)
			char_buffer.data(), char_buffer.size());

			// Ensure the device has a USB connection
			if (!errorState.check(utility::stringFormat("%s", char_buffer.c_str()) == sStateOk, "Device '%d' not calibrated. Reports status '%s'. Power on the device and (re-)connect the USB cable.", i, char_buffer.c_str()))
				return false;

			// Add to connected devices
			hpc_GetDeviceSerial(i, (char*)char_buffer.data(), char_buffer.size());
			mConnectedDevices.insert({utility::stringFormat("%s", char_buffer.c_str()), i});
			nap::Logger::info("Serial: %s", char_buffer.c_str());
			
			hpc_GetDeviceType(i, (char*)char_buffer.data(), char_buffer.size());
			nap::Logger::info("Device type: %s", char_buffer.c_str());
			nap::Logger::info("Window Coordinates: (%d, %d)", hpc_GetDevicePropertyWinX(i), hpc_GetDevicePropertyWinY(i));
			nap::Logger::info("Screen Resolution: (%d, %d)", hpc_GetDevicePropertyScreenW(i), hpc_GetDevicePropertyScreenH(i));
		}

		if (config->mLogState)
		{
			size_t buffer_size = 2048;
			std::string state_buffer(buffer_size, '\0');

			// hpc functions return a nonzero value if the buffer size is insufficient
			while (hpc_GetStateAsJSON((char*)state_buffer.data(), state_buffer.size()) != sHoloPlayCoreSuccess)
			{
				buffer_size *= 2;
				state_buffer = std::string(buffer_size, '\0');
			}

			nap::Logger::info("State: %s", state_buffer.c_str());
		}

		return true;
	}


	int LookingGlassService::findDeviceIndex(std::string deviceSerial) const
	{
		const auto it = mConnectedDevices.find(deviceSerial);

		if (it != mConnectedDevices.end())
			return it->second;

		return -1;
	}


	void LookingGlassService::shutdown()
	{
		hpc_CloseApp();
	}
}
