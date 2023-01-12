/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "sdleventconverter.h"

// External includes
#include <appeventhandler.h>

namespace nap
{
	/**
	 * Default application event handler.
	 * Forwards key, mouse and window events to the running application
	 * By default touch input generates mouse events.
	 */
	class NAPAPI SDLAppEventHandler : public AppEventHandler
	{
		RTTI_ENABLE(AppEventHandler)
	public:
		SDLAppEventHandler(App& app);

		/**
		* This call creates the SDL Input Converter
		*/
		virtual void start() override;

		/**
		* This call polls SDL for various messages, translates those messages
		* in nap events and forwards those to the default nap application
		*/
		virtual void process() override;

		/**
		* This call deletes the input converter
		*/
		virtual void shutdown() override;

		/**
		 * Tells the input system if touch input also generates mouse events, next to touch events.
		 * On most systems touch input is coupled to mouse input. This is controlled on an operating system level.
		 * This call tries to explicitly tell the input system to couple or decouple both events.
		 * @param value if touch input generates mouse events.
		 * @return if the system accepted the hint.
		 */
		bool setTouchGeneratesMouseEvents(bool value);

	private:
		std::unique_ptr<SDLEventConverter> mEventConverter = nullptr;
	};
}
