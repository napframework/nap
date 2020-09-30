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

	private:
		std::unique_ptr<SDLEventConverter> mEventConverter = nullptr;
	};
}
