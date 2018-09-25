#pragma once

#include <nap/service.h>
#include <inputservice.h>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>

namespace nap
{
	/**
	 * Service that manages connections to external input devices such as a keyboard, mouse, joystick and controllers.
	 * By default all connections to all available devices are opened automatically, ie: 
	 * all joysticks, game controllers etc. should be available to the system after initialization
	 * When a controller disconnects it is removed from the system until connected again. 
	 * This ensures that controllers can be connected / disconnected during sessions.
	 *
	 * A Game Controller is a Joystick that makes use of a pre-defined mapping, ie: LeftTrigger etc.
	 * Most popular game controllers are supported out of the box such as the xbox live controller etc.
	 * If a controller isn't supported natively it is considered to be a Joystick where the user has to
	 * interpret the various buttons.
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
		std::vector<SDL_Joystick*> mJoysticks;				///< All available joystick controllers
		std::vector<SDL_GameController*> mControllers;		///< All available game controllers
		nap::InputService* mInputService = nullptr;			///< Input service that deals with controller events


		// Adds a new controller
		void addController(int deviceID);

		// Removes a controller if registered
		void removeController(int deviceID);

		/**
		 * Called by the slot when a controller connection changes, ie: 
		 * a new controller is added or removed
		 */
		void onConnectionChanged(const ControllerConnectionEvent& event);

		// Slot that is called when a controller connection changes
		NSLOT(mConnectionChanged, const ControllerConnectionEvent&, onConnectionChanged);
	};
}
