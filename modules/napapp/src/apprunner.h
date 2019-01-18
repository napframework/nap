#pragma once

// Local Includes
#include "app.h"
#include "appeventhandler.h"

// External Includes
#include <rtti/typeinfo.h>
#include <nap/core.h>
#include <nap/datetime.h>
#include <thread>

namespace nap
{

	/**
	 * Utility class that runs a nap::BaseApp until BaseApp::quit() is called or
	 * AppRunner::stop(). The APP template argumentshould be derived from
	 * nap::BaseApp, HANDLER should be of type nap::BaseAppEventHandler()
	 * 
	 * When creating an AppRunner with those two template arguments the app is created
	 * and invoked at the right time based on core and it's associated services.
	 * Note that the AppRunner owns the app and handler.
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
		 * Starts the app loop, if the loop could not start for some reason
		 * the error contains the reason. This call will initialize core
		 * and the application and run the application loop until AppRunner::stop()
		 * or BaseApp::quit() is invoked.
		 * @param error the error message if the loop couldn't be started
		 * @return if the app loop has successfully started
		 */
		bool start(utility::ErrorState& error);

		/**
		 * Stops the loop and exits the application
		 */
		void stop();

		/**
		 * @return the app
		 */
		APP& getApp();

		/**
		 * @return the app handler
		 */
		HANDLER& getHandler();

		/**
		 * @return the application exit code
		 */
		int exitCode() const								{ return mExitCode; }

		/**
		 * Limits execution speed of the application. 
		 * This causes the application to sleep in between calls when execution speed exceeds the set framerate.
		 * The final execution speed may vary from platform to platform, based on the accuracy of the timers,
		 * but will always be less than the given framerate.
		 * Not setting any framerate allows the app to run at full speed (default)
		 * @param fps the maximum allowed refresh rate in frames per second
		 */
		void setFramerate(float fps);

	private:
		nap::Core&					mCore;					// Core
		std::unique_ptr<APP>		mApp = nullptr;			// App this runner works with
		std::unique_ptr<HANDLER>	mHandler = nullptr;		// App handler this runner works with
		bool						mStop = false;			// If the runner should stop
		int							mExitCode = 0;			// Application exit code
		MicroSeconds				mWaitTime;				// Time to wait in milliseconds based on FPS
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APP, typename HANDLER>
	APP& nap::AppRunner<APP, HANDLER>::getApp()
	{
		return *mApp;
	}


	template<typename APP, typename HANDLER>
	HANDLER& nap::AppRunner<APP, HANDLER>::getHandler()
	{
		return *mHandler;
	}


	template<typename APP, typename HANDLER>
	nap::AppRunner<APP, HANDLER>::AppRunner(nap::Core& core) : mCore(core)
	{
		// Ensure the app is an application
		assert(RTTI_OF(APP).is_derived_from(RTTI_OF(BaseApp)));

		// Ensure the handler is an app event handler
		assert(RTTI_OF(HANDLER).is_derived_from(RTTI_OF(AppEventHandler)));

		// Create 'm
		mApp = std::make_unique<APP>(core);
		mHandler = std::make_unique<HANDLER>(*mApp);
		mWaitTime = MicroSeconds(0);
	}


	template<typename APP, typename HANDLER>
	bool nap::AppRunner<APP, HANDLER>::start(utility::ErrorState& error)
	{
		nap::BaseApp& app = getApp();
		nap::AppEventHandler& app_event_handler = getHandler();

		// Initialize engine
		if (!mCore.initializeEngine(error))
		{
			error.fail("unable to initialize engine");
			return false;
		}
		// Initialize the various services
		if (!mCore.initializeServices(error))
		{
			mCore.shutdownServices();
			error.fail("Failed to initialize services");
			return false;
		}

#ifdef NAP_ENABLE_PYTHON
		if (!mCore.initializePython(error))
			return false;
#endif

		// Initialize application
		if(!error.check(app.init(error), "unable to initialize application"))
			return false;

		// Start event handler
		app_event_handler.start();

		// Pointer to function used inside update call by core
		std::function<void(double)> update_call = std::bind(&APP::update, mApp.get(), std::placeholders::_1);

		// Start core
		mCore.start();

		// Begin running
		HighResolutionTimer timer;
		MicroSeconds frame_time;
		MicroSeconds delay_time;
		while (!app.shouldQuit() && !mStop)
		{
			// Get time point for next frame
			frame_time = timer.getMicros() + mWaitTime;
			 
			// Process app specific messages
			app_event_handler.process();

			// update
			mCore.update(update_call);

			// render
			app.render();

			// Only sleep when there is at least 1 millisecond that needs to be compensated for
			// The actual outcome of the sleep call can vary greatly from system to system
			// And is more accurate with lower framerate limitations
			delay_time = frame_time - timer.getMicros();
			if(std::chrono::duration_cast<Milliseconds>(delay_time).count() > 0)
				std::this_thread::sleep_for(delay_time);
		}

		// Stop handling events
		app_event_handler.shutdown();

		// Shutdown
		mExitCode = app.shutdown();

		// Shutdown core
		mCore.shutdownServices();

		// Message successful exit
		return true;
	}


	template<typename APP, typename HANDLER>
	void nap::AppRunner<APP, HANDLER>::stop()
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


	template<typename APP, typename HANDLER>
	void nap::AppRunner<APP, HANDLER>::setFramerate(float fps)
	{
		mWaitTime = MicroSeconds(static_cast<long>(1000000.0 / static_cast<double>(fps)));
	}
}