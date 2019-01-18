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
		 * @param error the error message if the initialisation failed
		 * @return if the initialisation was successful
		 */
		bool init(utility::ErrorState& error);

		void update();

		int shutdown();
		
		/**
		 * @return the app
		 */
		APP& getApp();

		/**
		 * @return the app handler
		 */
		HANDLER& getHandler();

	private:
		nap::Core&					mCore;					// Core
		std::unique_ptr<APP>		mApp = nullptr;			// App this runner works with
		std::unique_ptr<HANDLER>	mHandler = nullptr;		// App handler this runner works with
		std::function<void(double)>	mUpdateCall;

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
		if(!error.check(app.init(error), "Unable to initialize application"))
			return false;

		// Start event handler
		app_event_handler.start();

		// Pointer to function used inside update call by core
		// std::function<void(double)> update_call = std::bind(&APP::update, mApp.get(), std::placeholders::_1);
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
		int exit_code = app.shutdown();

		// Shutdown core
		mCore.shutdownServices();

		return exit_code;
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
