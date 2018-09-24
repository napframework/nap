#pragma once

#include <nap/service.h>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>

namespace nap
{
	/**
	 * Service that manages connections to external input devices such as a keyboard, mouse, joystick and controllers.
	 * By default all connections to all available devices are opened automatically, ie: 
	 * all joysticks, game controllers etc. should be available to the system after initialization
	 *
	 * A Game Controller is a Joystick that makes use of a pre-defined mapping, ie: LeftTrigger etc.
	 * Most popular game controllers are supported out of the box such as the xbox live controller etc.
	 * If a controller isn't supported natively it is considered to be a Joystick where the user has to
	 * interpret the various buttons.
	 *
	 * TODO: Handle Connect / Disconnect
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
		std::vector<SDL_GameController*> mControllers;		///< All available game controllers
		std::vector<SDL_Joystick*> mJoysticks;				///< All available joysticks
	};
}
