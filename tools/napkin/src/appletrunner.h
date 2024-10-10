#pragma once

// External includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <nap/projectinfo.h>
#include <nap/logger.h>

namespace napkin
{
	namespace applet
	{
		namespace exitcode
		{
			constexpr nap::uint8 success		= 0;
			constexpr nap::uint8 initFailure	= 1;
			constexpr nap::uint8 invalid		= 2;
		}
	}

	/**
	 * Utility class that runs a nap::BaseApp until BaseApp::quit() is called or
	 * AppRunner::stop(). The APP template argument should be derived from
	 * nap::BaseApp, HANDLER should be of type nap::BaseAppEventHandler()
	 *
	 * This class is different from a regular AppRunner because it is thread safe, ie:
	 * It can be run in a separate thread, offering an interface to communicate with the app
	 * using api events.
	 * 
	 * When creating an AppletRunner with those two template arguments the app is created
	 * and invoked at the right time based on core and it's associated services.
	 * Note that the AppRunner owns the app and handler.
	 *
	 * On start() the runner initializes nap::Core, together with all available services and
	 * loads the application data. If everything succeeds the app loop is started.
	 */

	template<typename APP, typename HANDLER>
	class AppletRunner final
	{
	public:
		// Default constructor
		AppletRunner();

		~AppletRunner();

		/**
		 * Initializes the engine and the application.
		 * @param projectInfo the project to load and run
		 * @param context project run context (relative to app or editor)
		 * @param error the error message if the app failed to initialize
		 * @return if the app initialized successfully started
		 */
		bool init(const std::string& projectInfo, nap::ProjectInfo::EContext context, nap::utility::ErrorState& error);

		/**
		 * Starts running the application
		 * @return applet exit code
		 */
		void start();

		/**
		 * Process the application
		 */
		void process();

		/**
		 * Stop the application from running
		 */
		nap::uint8 stop();

		/**
		 * @return the app
		 */
		APP& getApp()														{ assert(mApp != nullptr); return *mApp; }

		/**
		 * @return the app handler
		 */
		HANDLER& getHandler()												{ assert(mHandler != nullptr); return *mHandler; }

		/**
		 * @return core
		 */
		nap::Core& getCore()												{ return mCore; }

		/**
		 * @return exit code
		 */
		nap::uint8 exitCode() const											{ return mExitCode; }

	private:
		nap::Core					mCore;									/// Associated core instance
		nap::Core::ServicesHandle	mServices = nullptr;					/// Initialized services
		
		std::unique_ptr<APP>		mApp = nullptr;							// App to run
		std::unique_ptr<HANDLER>	mHandler = nullptr;						// Handler to use
		nap::uint8					mExitCode = applet::exitcode::success;	// Application exit code

		std::atomic<bool>			mStop = { false };						// If the runner should stop
		bool						mInitialized =  false;					// If the application is initialized
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APP, typename HANDLER>
	AppletRunner<APP, HANDLER>::AppletRunner()
	{
		// Create application and handler
		mApp = std::make_unique<APP>(mCore);
		mHandler = std::make_unique<HANDLER>(*mApp);
	}


	template<typename APP, typename HANDLER>
	bool napkin::AppletRunner<APP, HANDLER>::init(const std::string& projectInfo, nap::ProjectInfo::EContext context, nap::utility::ErrorState& error)
	{
		assert(!mInitialized);
		mInitialized = false;

		// Initialize engine
		if (!error.check(mCore.initializeEngine(projectInfo, context, error), "Unable to initialize engine"))
			return false;

		// Initialize and keep handle to the various services
		// Bail if handle is invalid, this means service initialization failed
		mServices = mCore.initializeServices(error);
		if (mServices == nullptr)
			return false;

		// Change current working directory to directory that contains the data file
		std::string data_dir = mCore.getProjectInfo()->getDataDirectory();
		nap::utility::changeDir(data_dir);
		nap::Logger::info("Current working directory: % s", data_dir.c_str());

		// Ensure project data is available
		if (!error.check(!mCore.getProjectInfo()->mDefaultData.empty(), "Missing project data, %s 'Data' field is empty",
			mCore.getProjectInfo()->getProjectDir().c_str()))
			return false;

		// Load project data
		std::string data_file = nap::utility::getFileName(mCore.getProjectInfo()->getDataFile());
		nap::Logger::info("Loading data: %s", data_file.c_str());
		if (!error.check(mCore.getResourceManager()->loadFile(data_file, error), "Failed to load data: %s", data_file.c_str()))
			return false;

		// Watch the data directory
		mCore.getResourceManager()->watchDirectory(data_dir);

		// Initialize application
		if (!error.check(mApp->init(error), "Unable to initialize application"))
			return false;

		mInitialized = true;
		return true;
	}


	template<typename APP, typename HANDLER>
	void napkin::AppletRunner<APP, HANDLER>::start()
	{
		mCore.start();
		mHandler->start();
	}



	template<typename APP, typename HANDLER>
	void napkin::AppletRunner<APP, HANDLER>::process()
	{
		// Pointer to function used inside update call by core
		auto& app = getApp();
		auto& han = getHandler();

		// Process SDL events
		// TODO: This should not be in here -> move to dedicated SDL handle thread.
		han.process();

		// Update core and app
		std::function<void(double)> update_call = std::bind(&APP::update, &app, std::placeholders::_1);
		mCore.update(update_call);

		// Render content
		app.render();
	}


	template<typename APP, typename HANDLER>
	nap::uint8 napkin::AppletRunner<APP, HANDLER>::stop()
	{
		mHandler->shutdown();
		return static_cast<nap::uint8>(mApp->shutdown());
	}


	template<typename APP, typename HANDLER>
	napkin::AppletRunner<APP, HANDLER>::~AppletRunner()
	{
		// Shutdown and destroy services before core!
		mServices.reset();
	}
}
