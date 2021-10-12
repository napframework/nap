/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "app.h"
#include "appeventhandler.h"

// External Includes
#include <rtti/typeinfo.h>
#include <nap/core.h>
#include <nap/datetime.h>
#include <nap/logger.h>
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
	 * 
	 * On start() the runner initializes nap::Core, together with all available services and
	 * loads the application data. If everything succeeds the app loop is started.
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

	private:
		nap::Core&					mCore;					// Core
		std::unique_ptr<APP>		mApp = nullptr;			// App this runner works with
		std::unique_ptr<HANDLER>	mHandler = nullptr;		// App handler this runner works with
		bool						mStop = false;			// If the runner should stop
		int							mExitCode = 0;			// Application exit code* Call update() to force an update.
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
	}


	template<typename APP, typename HANDLER>
	bool nap::AppRunner<APP, HANDLER>::start(utility::ErrorState& error)
	{
		nap::BaseApp& app = getApp();
		nap::AppEventHandler& app_event_handler = getHandler();

		// Initialize engine
		if (!mCore.initializeEngine(error))
		{
			error.fail("Unable to initialize engine");
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
		// Change current working directory to directory that contains the data file
		std::string data_dir = mCore.getProjectInfo()->getDataDirectory();
		utility::changeDir(data_dir);
		nap::Logger::info("Current working directory: % s", data_dir.c_str());

		// Ensure project data is available
		if(!error.check(!mCore.getProjectInfo()->mDefaultData.empty(), "Missing project data, %s 'Data' field is empty",
			mCore.getProjectInfo()->getProjectDir().c_str()))
			return false;

		// Load project data
		std::string data_file = utility::getFileName(mCore.getProjectInfo()->getDataFile());
		nap::Logger::info("Loading data: %s", data_file.c_str());
		if (!error.check(mCore.getResourceManager()->loadFile(data_file, error),
			"Failed to load data: %s", data_file.c_str()))
			return false;

		// Watch the data directory
		mCore.getResourceManager()->watchDirectory(data_dir);

		// Initialize application
		if(!error.check(app.init(error), "Unable to initialize application"))
		{
			mCore.shutdownServices();
			return false;
		}

		// Start event handler
		app_event_handler.start();

		// Pointer to function used inside update call by core
		std::function<void(double)> update_call = std::bind(&APP::update, mApp.get(), std::placeholders::_1);

		// Start core
		mCore.start();

		// Begin running
		HighResolutionTimer timer;
		Milliseconds frame_time, delay_time, zero_delay(0);
		while (!app.shouldQuit() && !mStop)
		{
			// Get point in time when frame is requested to be completed
			frame_time = timer.getMillis() + (app.framerateCapped() ? 
				Milliseconds(static_cast<long>(1000.0 / static_cast<double>(app.getRequestedFramerate()))) :
				zero_delay);
			 
			// Process app specific messages
			app_event_handler.process();

			// update
			mCore.update(update_call);

			// render
			app.render();

			// Only sleep when there is at least 1 millisecond that needs to be compensated for
			// The actual outcome of the sleep call can vary greatly from system to system
			// And is more accurate with lower framerate limitations
			delay_time = frame_time - timer.getMillis();
			if(delay_time.count() > 0)
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
}
