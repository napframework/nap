// Local includes
#include "sdlinputservice.h"

// External includes
#include <SDL_joystick.h>
#include <inputservice.h>
#include <SDL.h>
#include <nap/logger.h>

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

		SDL_GameController *ctrl =nullptr;
		SDL_Joystick *joy = nullptr;
		for (int i = 0; i < SDL_NumJoysticks(); ++i) 
		{
			if (SDL_IsGameController(i)) 
			{
				const char* controller_name = SDL_GameControllerNameForIndex(i);
				nap::Logger::info("found compatible game controller: %s", controller_name);
				ctrl = SDL_GameControllerOpen(i);
				mControllers.emplace_back(ctrl);
			}
			else
			{
				const char* joystick_name = SDL_JoystickNameForIndex(i);
				nap::Logger::info("found compatible joystick device: %s", joystick_name);
				joy = SDL_JoystickOpen(i);
				mJoysticks.emplace_back(joy);
			}
		}
		return true;
	}


	void SDLInputService::shutdown()
	{
		for (auto& controller : mControllers)
			SDL_GameControllerClose(controller);
		for (auto& joystick : mJoysticks)
			SDL_JoystickClose(joystick);
	}
}