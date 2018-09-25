// Local includes
#include "sdlinputservice.h"

// External includes
#include <SDL_joystick.h>
#include <inputservice.h>
#include <SDL.h>
#include <nap/logger.h>
#include <nap/core.h>

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
		for (auto& joystick : mJoysticks)
			SDL_JoystickClose(joystick);

		for (auto& controller : mControllers)
			SDL_GameControllerClose(controller);

		mInputService = nullptr;
	}


	void SDLInputService::onConnectionChanged(const ControllerConnectionEvent& connectEvent)
	{
		// New controller
		if (connectEvent.mStatus)
		{
			addController(connectEvent.mDeviceID);
			return;
		}

		// Remove controller
		removeController(connectEvent.mDeviceID);

	}


	void SDLInputService::addController(int deviceID)
	{
		// Otherwise add
		if (SDL_IsGameController(deviceID))
		{
			const char* controller_name = SDL_GameControllerNameForIndex(deviceID);
			nap::Logger::info("game controller: %d, %s connected", deviceID, controller_name);
			SDL_GameController *ctrl = SDL_GameControllerOpen(deviceID);
			mControllers.emplace_back(ctrl);
		}
		else
		{
			const char* joystick_name = SDL_JoystickNameForIndex(deviceID);
			nap::Logger::info("joystick: %d, %s connected", deviceID, joystick_name);
			SDL_Joystick *joy = SDL_JoystickOpen(deviceID);
			mJoysticks.emplace_back(joy);
		}
	}


	void SDLInputService::removeController(int deviceID)
	{
		/*
		auto found_it = std::find_if(mControllers.begin(), mControllers.end(), [&](auto& controller) {
			return deviceID == SDL_JoystickInstanceID(controller);
		});

		// Erase if found
		if (found_it != mControllers.end())
		{
			if (SDL_IsGameController(deviceID))
			{
				SDL_GameController* ctrl = SDL_GameControllerFromInstanceID(deviceID);
				SDL_GameControllerClose(ctrl);
				const char* controller_name = SDL_GameControllerNameForIndex(deviceID);
				nap::Logger::info("game controller disconnected", controller_name);
			}
			else
			{
				SDL_JoystickClose(*found_it);
				const char* joystick_name = SDL_JoystickNameForIndex(deviceID);
				nap::Logger::info("joystick disconnected", joystick_name);
			}

			// Erase from map
			mControllers.erase(found_it);
		}
		*/
	}

}