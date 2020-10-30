/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <inputservice.h>
#include <unordered_map>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>

namespace nap
{
	/**
	 * Service that manages connections to external input devices such as a keyboard, mouse, joystick and controller.
	 * By default all connections to all available devices are opened automatically, ie: 
	 * all joysticks, game controllers etc. should be available to the system after initialization.
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

		/**
		 * Struct used for keeping track of controllers inside the system
		 * The SDL controller, when constructed, will try to open a connection
		 * On destruction the controller will close it's connection (if opened)
		 */
		class NAPAPI SDLController final
		{
		public:
			SDLController(int deviceID);
			~SDLController()					{ close(); }

			int		mDeviceID = -1;				///< Physical device id
			int		mInstanceID = -1;			///< Instance id (uuid)
			void*	mController = nullptr;		///< Pointer to the controller or joystick
			bool	mIsJoystick = false;		///< If the controller is a joystick or actual controller

		private:
			void close();
		};

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

		/**
		 * Checks if a physical controller with the given number is connected.
		 * The number is always associated with a physical joystick or game control device
		 * @param controllerNumber the number of the controller
		 * @return if the controller is online
		 */
		bool isConnected(int controllerNumber) const;

		/**
		 * This call can be used to ensure a controller is known to the system using a unique controller id.
		 * The unique controller id is part of SDL controller and joystick events.
		 * @return if a controller with the given unique id exists
		 */
		bool controllerInstanceExists(int instance) const;

		/**
		 * Returns the physical controller number based on the given unique identifier.
		 * This call assumes that a controller with the unique identifier exists!
		 * @param instance the unique controller instance, part of an SDL controller / joystick event
		 * @return the physical controller index for the given instance identifier, fails when not available
		 */
		int getControllerNumber(int instance) const;

		/**
		 * Checks if the controller is a game controller or joystick.
		 * Note that this call assumes the instance exists!
		 * @param instance the unique controller instance, part of an sdl controller / joystick event
		 * @return if the instance is a controller. If not the controller is considered by SDL to be a joystick. 
		 */
		bool isGameController(int instance) const;

	private:
		std::unordered_map<int, std::unique_ptr<SDLController>> mSystemControllers;
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
