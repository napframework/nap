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
#include <thread>

namespace nap
{
	/**
	 * Utility class that runs a nap::BaseApp as a service.
	 * This is useful when a different environment embeds a NAP application as a service that is run in the background.
	 * When running an app as a service the app is initialized, updated and stopped from that external environment.
	 * Call init() to initialize core, all the services and the app.
	 * Call update() to update all services and the app.
	 * Call shutdown() on exit to make sure all processes are stopped correctly.
	 * 
	 * The APP template argument should be derived from nap::BaseApp. 
	 * The HANDLER template argument should be of type nap::AppEventHandler()
	 * 
	 * When creating a ServiceRunner with those two template arguments the app is created
	 * and invoked at the right time based on core and it's associated services.
	 * Note that this object owns the app and handler.
	 */
	template<typename APP, typename HANDLER>
	class ServiceRunner
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 * @param core the nap core this runner uses in conjunction with the app and handler
		 */
		ServiceRunner(nap::Core& core);
		
		/**
		 *	Destructor
		 */
		virtual ~ServiceRunner();

		/**
		* Copy is not allowed
		*/
		ServiceRunner(ServiceRunner&) = delete;
		ServiceRunner& operator=(const ServiceRunner&) = delete;

		/**
		* Move is not allowed
		*/
		ServiceRunner(ServiceRunner&&) = delete;
		ServiceRunner& operator=(ServiceRunner&&) = delete;

		/**
		 * This call will initialize core and the application
		 * @param error the error message if the initialization failed
		 * @return if the initialization was successful
		 */
		bool init(utility::ErrorState& error);

		/**
		 * Call this from an external environment.
		 * On update all events are processed, after that update on core is called.
		 */
		void update();

		/**
		 * Call this before exiting your application.
		 * Ensures the app is exited and running services are stopped appropiately.
		 */
		int shutdown();

		/**
		 * Returns the application exit code, available after shutdown().
		 * @return the application exit code.
		 */
		int exitCode() const								{ return mExitCode; }

		/**
		 * @return the app
		 */
		APP& getApp();

		/**
		 * @return core	
		 */
		Core& getCore()										{ return mCore; }

		/**
		 * @return core	
		 */
		const Core& getCore() const							{ return mCore; }

		/**
		 * @return the app handler
		 */
		HANDLER& getHandler();

	private:
		nap::Core&					mCore;						// Core
		std::unique_ptr<APP>		mApp = nullptr;				// App this runner works with
		std::unique_ptr<HANDLER>	mHandler = nullptr;			// App handler this runner works with
		std::function<void(double)>	mUpdateCall;				// Callback to the application update call
		int							mExitCode = 0;				// Application exit code, available after shutdown
		Core::ServicesHandle		mServicesHandle = nullptr;	// Handle to initialized and running services
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APP, typename HANDLER>
	APP& nap::ServiceRunner<APP, HANDLER>::getApp()
	{
		return *mApp;
	}


	template<typename APP, typename HANDLER>
	HANDLER& nap::ServiceRunner<APP, HANDLER>::getHandler()
	{
		return *mHandler;
	}


	template<typename APP, typename HANDLER>
	nap::ServiceRunner<APP, HANDLER>::ServiceRunner(nap::Core& core) : mCore(core)
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
	bool nap::ServiceRunner<APP, HANDLER>::init(utility::ErrorState& error)
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
		// Bail if handle is invalid, this means service initialization failed
		mServicesHandle = mCore.initializeServices(error);
		if (mServicesHandle == nullptr)
			return false;

#ifdef NAP_ENABLE_PYTHON
		if (!mCore.initializePython(error))
			return false;
#endif

		// Initialize application
		if(!error.check(app.init(error), "Unable to initialize application"))
			return false;

		// Start event handler
		app_event_handler.start();

		// Pointer to function used inside update call by core
		mUpdateCall = std::bind(&APP::update, mApp.get(), std::placeholders::_1);		

		// Start core
		mCore.start();

		// Message successful start
		return true;
	}


	template<typename APP, typename HANDLER>
	void nap::ServiceRunner<APP, HANDLER>::update()
	{
		// Process app specific messages
		nap::AppEventHandler& app_event_handler = getHandler();
		app_event_handler.process();

		// update
		mCore.update(mUpdateCall);
	}


	template<typename APP, typename HANDLER>
	int nap::ServiceRunner<APP, HANDLER>::shutdown()
	{
		// Stop handling events
		nap::AppEventHandler& app_event_handler = getHandler();
		app_event_handler.shutdown();

		// Shutdown
		nap::BaseApp& app = getApp();
		mExitCode = app.shutdown();

		// Shut down services
		mServicesHandle.reset(nullptr);

		// Shutdown core
		return mExitCode;
	}


	template<typename APP, typename HANDLER>
	nap::ServiceRunner<APP, HANDLER>::~ServiceRunner()
	{
		// First clear the handler as it points to our app
		mHandler.reset();

		// Now clear the app
		mApp.reset();
	}
}
