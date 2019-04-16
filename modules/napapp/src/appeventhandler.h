#pragma once

// Local Includes
#include "app.h"

// External Includes
#include <rtti/typeinfo.h>

namespace nap
{
	/**
	 * Helper object that allows for custom app processing behavior inside the AppRunner update loop.
	 * By default this object does nothing. Override the various methods to add process logic.
	 */
	class NAPAPI AppEventHandler
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 * @param app the app this event handler works with when processing messages
		 */
		AppEventHandler(BaseApp& app);

		// Default Destructor
		virtual ~AppEventHandler() = default;

		/**
		 * Copy is not allowed
		 */
		AppEventHandler(AppEventHandler&) = delete;
		AppEventHandler& operator=(const AppEventHandler&) = delete;

		/**
		 * Move is not allowed
		 */
		AppEventHandler(AppEventHandler&&) = delete;
		AppEventHandler& operator=(AppEventHandler&&) = delete;

		/**
		 * Called before running the app loop but after all services have been initialized
		 * Use this call to initialize functionality before starting the process loop
		 */
		virtual void start() 				{ }

		/**
		 * This is called within the main loop of the app runner to specify
		 * specific event process behavior. For example, when working with a window
		 * you might want to check for window or input messages and forward those to the application
		 * This function is invoked at the beginning of the app loop
		 */
		virtual void process()				{ }

		/**
		 * Called before shutting down all services and exiting the app
		 */
		virtual void shutdown()				{ }

		/**
		 *	Returns the application as an object of type T
		 */
		template<typename T>
		T& getApp();

	protected:
		BaseApp& mApp;						// The app associated with this app event handler
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T& nap::AppEventHandler::getApp()
	{
		assert(mApp.get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<T&>(mApp);
	}
}