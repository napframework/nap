/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "sdlinputservice.h"

// External includes
#include <SDL_joystick.h>
#include <inputservice.h>
#include <SDL.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <memory.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SDLInputService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SDLInputService::SDLInputService(ServiceConfiguration* configuration) : Service(configuration)
	{ }

	void SDLInputService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(nap::InputService));
	}


	bool SDLInputService::init(utility::ErrorState& error)
	{
		// Initialize game controller layer from SDL
		SDL_Init(SDL_INIT_GAMECONTROLLER);
		SDL_JoystickEventState(SDL_ENABLE);

		// Listen for controller connect / disconnect signals
		mInputService = getCore().getService<nap::InputService>();
		assert(mInputService != nullptr);
		mInputService->controllerConnectionChanged.connect(mConnectionChanged);

		// All done
		return true;
	}


	void SDLInputService::shutdown()
	{
		mSystemControllers.clear();
		mInputService = nullptr;
	}


	bool SDLInputService::isConnected(int deviceID) const
	{
		for (auto& controller : mSystemControllers)
		{
			if (controller.second->mDeviceID == deviceID)
				return true;
		}
		return false;
	}


	int SDLInputService::getControllerNumber(int instance) const
	{
		auto it = mSystemControllers.find(instance);
		assert(it != mSystemControllers.end());
		return it->second->mDeviceID;
	}


	bool SDLInputService::isGameController(int instance) const
	{
		auto it = mSystemControllers.find(instance);
		assert(it != mSystemControllers.end());
		return !(it->second->mIsJoystick);
	}


	void SDLInputService::onConnectionChanged(const ControllerConnectionEvent& connectEvent)
	{
		// New controller
		if (connectEvent.mConnected)
		{
			addController(connectEvent.mDeviceID);
			return;
		}

		// Remove controller
		removeController(connectEvent.mDeviceID);
	}


	void SDLInputService::addController(int deviceID)
	{
		// Adding the same controller without disconnecting isn't allowed
		assert(!isConnected(deviceID));

		// Otherwise add
		std::unique_ptr<SDLController> uctrl = std::make_unique<SDLController>(deviceID);
		if (uctrl->mController == nullptr)
			return;
		mSystemControllers.emplace(std::make_pair(uctrl->mInstanceID, std::move(uctrl)));
	}


	void SDLInputService::removeController(int deviceID)
	{
		// Find controller based on device id
		auto it = std::find_if(mSystemControllers.begin(), mSystemControllers.end(), [&](auto& controller)
		{
			return controller.second->mDeviceID == deviceID;
		});

		// Delete from map
		assert(it != mSystemControllers.end());
		mSystemControllers.erase(it);
	}


	SDLInputService::SDLController::SDLController(int deviceID) : mDeviceID(deviceID)
	{
		// Open as game controller connection
		if (SDL_IsGameController(mDeviceID))
		{
			mIsJoystick = false;
			const char* controller_name = SDL_GameControllerNameForIndex(deviceID);
			nap::Logger::info("Game Controller: %d connected: %s", deviceID, controller_name);
			SDL_GameController *ctrl = SDL_GameControllerOpen(deviceID);
			if (ctrl == nullptr)
				nap::Logger::warn("Unable to open Game Controller connection: %d", deviceID);
			else
				mInstanceID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
			mController = ctrl;
			return;
		}

		// Otherwise open as joystick connection
		mIsJoystick = true;
		const char* joystick_name = SDL_JoystickNameForIndex(deviceID);
		nap::Logger::info("Joystick: %d connected: %s", deviceID, joystick_name);
		SDL_Joystick *joy = SDL_JoystickOpen(deviceID);
		if (joy == nullptr)
			nap::Logger::warn("Unable to open Joystick connection: %d", deviceID);
		else
			mInstanceID = SDL_JoystickInstanceID(joy);
		mController = joy;
	}


	void SDLInputService::SDLController::close()
	{
		// Close and be done with it
		if (mController == nullptr)
			return;

		if (mIsJoystick)
		{
			SDL_JoystickClose(reinterpret_cast<SDL_Joystick*>(mController));
			nap::Logger::info("Joystick: %d disconnected", mDeviceID);
			return;
		}

		SDL_GameControllerClose(reinterpret_cast<SDL_GameController*>(mController));
		nap::Logger::info("Game Controller: %d disconnected", mDeviceID);
	}
}
