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
		for (auto& controller : mSystemControllers)
		{
			SDLController& ctrl = *(controller.second);
			if (ctrl.mIsJoystick)
				SDL_JoystickClose(reinterpret_cast<SDL_Joystick*>(ctrl.mController));
			else
				SDL_GameControllerClose(reinterpret_cast<SDL_GameController*>(ctrl.mController));
		}

		mSystemControllers.clear();
		mInputService = nullptr;
	}


	bool SDLInputService::isOnline(int deviceID)
	{
		for (auto& controller : mSystemControllers)
		{
			if (controller.second->mDeviceID == deviceID)
				return true;
		}
		return false;
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
		std::unique_ptr<SDLController> uctrl = nullptr;
		int instance_id = -1;

		if (SDL_IsGameController(deviceID))
		{
			const char* controller_name = SDL_GameControllerNameForIndex(deviceID);
			nap::Logger::info("Game Controller connected: %s, number: %d", controller_name, deviceID);
			SDL_GameController *ctrl = SDL_GameControllerOpen(deviceID);
			instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctrl));
			uctrl = std::make_unique<SDLController>(deviceID, reinterpret_cast<void*>(ctrl), false);
		}
		else
		{
			const char* joystick_name = SDL_JoystickNameForIndex(deviceID);
			nap::Logger::info("Joystick connected: %s, number: %d", joystick_name, deviceID);
			SDL_Joystick *joy = SDL_JoystickOpen(deviceID);
			instance_id = SDL_JoystickInstanceID(joy);
			uctrl = std::make_unique<SDLController>(deviceID, reinterpret_cast<void*>(joy), true);
		}

		mSystemControllers.emplace(std::make_pair(instance_id, std::move(uctrl)));
	}


	void SDLInputService::removeController(int deviceID)
	{
		// Find controller to close
		auto it = mSystemControllers.find(deviceID);
		if (it == mSystemControllers.end())
		{
			assert(false);
			return;
		}

		SDLController& ctrl = *(it->second);
		if (ctrl.mIsJoystick)
		{
			SDL_JoystickClose(reinterpret_cast<SDL_Joystick*>(ctrl.mController));
			nap::Logger::info("Joystick: %d disconnected", ctrl.mDeviceID);
		}
		else
		{
			SDL_GameControllerClose(reinterpret_cast<SDL_GameController*>(ctrl.mController));
			nap::Logger::info("Game Controller: %d disconnected", ctrl.mDeviceID);
		}

		// Delete from map
		mSystemControllers.erase(it);
	}

}