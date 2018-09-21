// Local includes
#include "sdlinputservice.h"

// External includes
#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>
#include <inputservice.h>
#include <sdl.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SDLInputService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	SDLInputService::SDLInputService(ServiceConfiguration* configuration) : Service(configuration)
	{

	}

	void SDLInputService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(nap::InputService));
	}

	bool SDLInputService::init(utility::ErrorState& error)
	{
		// Initialize game controller layer from SDL
		SDL_Init(SDL_INIT_GAMECONTROLLER);

		SDL_GameController *ctrl =nullptr;
		SDL_Joystick *joy = nullptr;
		for (int i = 0; i < SDL_NumJoysticks(); ++i) 
		{
			if (SDL_IsGameController(i)) 
			{
				const char* controller_name = SDL_GameControllerNameForIndex(i);
				nap::Logger::info("found compatible game controller: %s", controller_name);
				ctrl = SDL_GameControllerOpen(i);
				joy  = SDL_GameControllerGetJoystick(ctrl);
			}
		}
		return true;
	}

	void SDLInputService::shutdown()
	{

	}
}