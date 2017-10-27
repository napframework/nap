#pragma once

// Local Includes
#include "app.h"
#include "appeventhandler.h"

// External Includes
#include <rtti/typeinfo.h>
#include <nap/core.h>

namespace nap
{
	/**
	 * Utility class that runs a nap::BaseApp until BaseApp::quit() is called or
	 * AppRunner::stopRunning(). The APP template argumentshould be derived from
	 * nap::BaseApp, HANDLER should be derived from nap::BaseAppEventHandler()
	 * 
	 * When creating an AppRunner with those two template arguments the app is created
	 * and invoked at the right time based on core and it's associated services.
	 * Note that the AppRunner owns the app and handler
	 */
	template<typename APP, typename HANDLER>
	class AppRunner
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 * @param core the nap core this runner uses in conjunction with the app and handler
		 */
		AppRunner(nap::Core& core);
		
		/**
		 *	Destructor
		 */
		virtual ~AppRunner();

		/**
		* Copy is not allowed
		*/
		AppRunner(AppRunner&) = delete;
		AppRunner& operator=(const AppRunner&) = delete;

		/**
		* Move is not allowed
		*/
		AppRunner(AppRunner&&) = delete;
		AppRunner& operator=(AppRunner&&) = delete;

		/**
		 * Starts the loop, if the loop could not start for some reason
		 * the error contains the message
		 * @param error the error message if the loop couldn't be started
		 */
		bool startRunning(utility::ErrorState& error);

		/**
		 *	Stops the loop and exits the application
		 */
		void stopRunning();

		/**
		 *	@return the app
		 */
		BaseApp& getApp();

		/**
		 *	@return the app handler
		 */
		BaseAppEventHandler& getHandler();

	private:
		nap::Core&					mCore;					// Core
		std::unique_ptr<APP>		mApp = nullptr;			// App this runner works with
		std::unique_ptr<HANDLER>	mHandler = nullptr;		// App handler this runner works with
		bool						mStop = false;			// If the runner should stop

	};

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APP, typename HANDLER>
	BaseApp& nap::AppRunner<APP, HANDLER>::getApp()
	{
		return static_cast<BaseApp&>(*mApp);
	}


	template<typename APP, typename HANDLER>
	BaseAppEventHandler& nap::AppRunner<APP, HANDLER>::getHandler()
	{
		return static_cast<AppEventHandler&>(*mHandler);
	}


	template<typename APP, typename HANDLER>
	nap::AppRunner<APP, HANDLER>::AppRunner(nap::Core& core) : mCore(core)
	{
		// Ensure the app is an application
		assert(RTTI_OF(APP).is_derived_from(RTTI_OF(BaseApp)));

		// Ensure the handler is an app event handler
		assert(RTTI_OF(HANDLER).is_derived_from(RTTI_OF(BaseAppEventHandler)));

		// Create 'm
		mApp = std::make_unique<APP>(core);
		mHandler = std::make_unique<HANDLER>(*mApp);
	}


	template<typename APP, typename HANDLER>
	bool nap::AppRunner<APP, HANDLER>::startRunning(utility::ErrorState& error)
	{
		// Initialize engine
		if (!mCore.initializeEngine(error))
		{
			mCore.shutdown();
			error.fail("unable to initialize engine");
			return false;
		}

		// Initialize application
		if (!getApp().init(error))
		{
			error.fail("unable to initialize application");
			return false;
		}

		// Pointer to function used inside update call by core
		std::function<void(double)> update_call = std::bind(&APP::update, mApp.get(), std::placeholders::_1);

		nap::BaseApp& running_app = getApp();

		// Start core and begin running
		mCore.start();
		while (!running_app.shouldQuit() && !mStop)
		{
			// Process app specific messages
			getHandler().process();

			// update
			mCore.update(update_call);

			// render
			getApp().render();
		}

		// Shutdown
		getApp().shutdown();

		// Shutdown core
		mCore.shutdown();

		// Message successful exit
		return true;
	}


	template<typename APP, typename HANDLER>
	void nap::AppRunner<APP, HANDLER>::stopRunning()
	{
		mStop = true;
	}


	template<typename APP, typename HANDLER>
	nap::AppRunner<APP, HANDLER>::~AppRunner()
	{
		// First clear the handler as it points to our app
		mHandler.reset();

		// Now clear the app
		mApp.reset();
	}
}