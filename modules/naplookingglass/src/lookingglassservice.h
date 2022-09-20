/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// Nap includes
#include <nap/service.h>

// External includes
#include <unordered_map>

namespace nap
{
	// Forward declarations
	class LookingGlassService;

	//////////////////////////////////////////////////////////////////////////
	// Service Configuration
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Looking Glass service configuration
	 */
	class NAPAPI LookingGlassServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:

		/**
		 * The HoloPlay Service license type
		 */
		enum EHoloPlayLicenseType
		{
			Commercial,
			NonCommercial,
		};

		std::string					mAppName = "NAPAPP";									///< Property: 'ApplicationName' Client application name.
		EHoloPlayLicenseType		mLicenseType = EHoloPlayLicenseType::NonCommercial;		///< Property: 'LicenseType' The license type.
		bool						mLogState = false;										///< Property: 'LogState' Logs the internal device state variable on initialization.
		virtual rtti::TypeInfo		getServiceType() const override { return RTTI_OF(LookingGlassService); }
	};


	//////////////////////////////////////////////////////////////////////////
	// Looking Glass Service
	//////////////////////////////////////////////////////////////////////////

    /**
     * LookingGlassService
	 *
	 * Handles general communication with the HoloPlay Service. Registers the current application, communicates
	 * initialization and shutdown. The nap::LookingGlassService keeps a registry of connected validated
	 * looking glass devices.
     */
    class NAPAPI LookingGlassService : public nap::Service
    {
		RTTI_ENABLE(nap::Service)
		friend class LookingGlassDevice;
    public:
		// Constructor
        LookingGlassService(ServiceConfiguration* configuration);
        
        /**
         * Initializes the looking glass service
         */
        virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Shutdown the service
		 */
		virtual void shutdown() override;

		/**
		 * Returns a device index of the specified device identified by its serial number, otherwise -1.
		 * Called by nap::LookingGlassDevice to validate itself.
		 * 
		 * @return device index of the specified device identified by its serial number, otherwise -1
		 */
		int findDeviceIndex(std::string deviceSerial) const;

		bool mAllowFailure = true; ///< Property: 'AllowFailure' Set to true if the app can init when failing to initialize the service. Failure usually indicates that the holoplay service app is not running.
        
    protected:
        virtual void registerObjectCreators(rtti::Factory& factory) override final;

	private:
		// Device IDs are persistent throughout the connection lifetime of a device. 
		std::unordered_map<std::string, int> mConnectedDevices;
    };
}
