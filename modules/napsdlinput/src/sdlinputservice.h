#pragma once

#include <nap/service.h>
#include <SDL_gamecontroller.h>

namespace nap
{
	/**
	 * Service that manages connections to external input devices such as keyboard, mouse and joystick
	 * By default all connections to all available devices are opened automatically, ie: 
	 * all joysticks, game controllers etc. should be available to the system after initialization
	 */
	class NAPAPI SDLInputService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		// Default constructor
		SDLInputService(ServiceConfiguration* configuration);

		/**
		 * This service depends on the default input service
		 * @param dependencies rtti information of the services this service depends on
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Initializes the SDL input service
		 * When initialized the system is able to receive keyboard, mouse and joystick events
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * Shuts down the SDL input service
		 * Closes all open connections to external devices
		 */
		virtual void shutdown() override;

	private:
		std::vector<SDL_GameController*> mControllers;
	};
}
