#pragma once

// External Inlcludes
#include <appeventhandler.h>

namespace nap
{
	/**
	 * Application event handler that is designed to work with applications that host a graphical user interface.
	 * This class checks if the user is interacting with a GUI element, if so, no input events are forwarded to the application.
	 */
	class EmographyAppEventHandler : public AppEventHandler
	{
		RTTI_ENABLE(AppEventHandler)
	public:
		EmographyAppEventHandler(App& app);

		/**
		 * Starts the application event handler
		 */
		virtual void start() override;

		/**
		 * Process incoming signals
		 */
		virtual void process() override;

		/**
		 * Called after the app has been terminated
		 */
		virtual void shutdown() override;
	};
}