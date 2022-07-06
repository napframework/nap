/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

// Nap includes
#include <nap/service.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <renderwindow.h>
#include <mathutils.h>

// Local includes
#include "quiltsettings.h"

namespace nap
{
	// Forward declarations
	class LookingGlassService;


	//////////////////////////////////////////////////////////////////////////
	// Looking Glass Device
	//////////////////////////////////////////////////////////////////////////

	/**
	 * LookingGlassDevice
	 *
	 * A resource that represents a Looking Glass. Used to create nap::QuiltRenderTarget, nap::QuiltCameraComponent,
	 * nap::RenderLightFieldComponent tuned to the specs of a specific device. Requires a valid serial number to be
	 * set in the "Serial" property. The serial number is validated on initialization with a query to
	 * nap::LookingGlassService.
	 */
	class NAPAPI LookingGlassDevice : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * A struct grouping looking glass calibration settings.
		 * Required by nap::LightFieldShader.
		 */
		struct CalibrationSettings
		{
			float		pitch = 0.f;
			float		tilt = 0.f;
			float		center = 0.f;
			int			invView	= 0;
			float		subp = 0.f;
			float		displayAspect = 1.f;
			int			ri = 0;
			int			bi = 0;
		};

		// Constructor
		LookingGlassDevice(LookingGlassService& service);

		/**
		 * Initializes this Looking Glass device. 
		 * 
		 * Addresses nap::LookingGlassService to check if a device with a matching serial
		 * number is currently connected to the current machine. Initialization fails if no
		 * Looking Glass is connected with a matching serial number.
		 *
		 * @param errorState contains information about the error in case initialization fails.
		 * @return true if the service initialized successfully, otherwise false
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the window position of the current looking glass device, as reported by the OS.
		 */
		glm::ivec2 getPosition() const									{ return mWindowPosition; }

		/**
		 * @return the screen resolution of the current looking glass device.
		 */
		glm::uvec2 getResolution() const								{ return mScreenResolution; }

		/**
		 * @return the screen aspect ratio of the current looking glass device.
		 */
		float getAspectRatio() const									{ return mScreenResolution.x / static_cast<float>(mScreenResolution.y); }

		/**
		 * @return the calibration settings of the current looking glass device.
		 */
		const CalibrationSettings& getCalibrationSettings() const		{ return mCalibration; }

		/**
		 * @return the recommended quilt settings of the current looking glass device.
		 */
		const QuiltSettings& getQuiltSettings()	const					{ return mEnableCustomQuilt ? mCustomQuiltSettings : mOptimalQuiltSettings; }

		/**
		 * @return the device index.
		 */
		int getIndex() const											{ return mDeviceIndex; }

		/**
		 * @return view cone size in degrees.
		 */
		float getViewCone() const										{ return mViewCone; }

		/**
		 * @return if the device is valid.
		 */
		bool isValid() const											{ return mDeviceIndex >= 0; }

		std::string					mSerial;							///< Property: 'Serial' the Looking Glass device serial number.
		QuiltSettings				mQuiltSettingsResource;				///< Property: 'QuiltSettings' optional custom quilt settings.
		ResourcePtr<RenderWindow>	mRenderWindow;						///< Property: 'RenderWindow' a render window to setup for rendering.
		bool						mEnableCustomQuiltResource = false;	///< Property: 'EnableCustomQuilt' whether to use custom quilt settings.
		bool						mForceInit = false;					///< Property: 'ForceInit' forces initialization, allows you to initialize the current virtual device without the looking glass (debug only).

	private:
		LookingGlassService*		mService = nullptr;					//< Reference to the looking glass service
			
		glm::ivec2					mWindowPosition = {0, 0};			//< The window position of the current looking glass device
		glm::uvec2					mScreenResolution = {3850, 2160};	//< The screen resolution of the current looking glass device

		CalibrationSettings			mCalibration;						//< Device calibration settings
		QuiltSettings				mOptimalQuiltSettings;				//< Optimal quilt settings as queried from the device
		QuiltSettings				mCustomQuiltSettings;				//< Custom quilt settings

		bool						mEnableCustomQuilt = false;			//< Whether to use custom quilt settings
		float						mViewCone = 40.0f;					//< View cone size in degrees
		int							mDeviceIndex = -1;					//< Assigned by lookingglasservice on creation	
	};

	// Object creator used for constructing the looking glass device
	using LookingGlassDeviceObjectCreator = rtti::ObjectCreator<LookingGlassDevice, LookingGlassService>;
}
