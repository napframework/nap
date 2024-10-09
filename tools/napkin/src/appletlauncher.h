/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "appletrunner.h"

// External Includes
#include <nap/core.h>
#include <thread>
#include <future>

namespace napkin
{
	/**
	 * Wraps and runs a NAP applet in a separate thread until aborted.
	 */
	template<typename APP, typename HANDLER>
	class AppletLauncher
	{
	public:
		AppletLauncher() = default;

		// Terminates the application if running 
		virtual ~AppletLauncher()											{ }

		/**
		 * Copy is not allowed
		 */
		AppletLauncher(AppletLauncher&) = delete;
		AppletLauncher& operator=(const AppletLauncher&) = delete;

		/**
		 * Move is not allowed
		 */
		AppletLauncher(AppletLauncher&&) = delete;
		AppletLauncher& operator=(AppletLauncher&&) = delete;

		/**
		 * Launches and runs the app in a separate thread
		 * @param projectInfoFile the project info to load
		 */
		// void run(const std::string& projectFilename);

		/**
		 * Aborts and joins the running application if it runs (= valid)
		 * @return application exit code
		 */
		// int abort();

		/**
		 * Initializes the applet
		 * @param projectFilename the project to initialize
		 * @param error holds the error if initialization fails
		 */
		bool init(const std::string& projectFilename, nap::utility::ErrorState error);

		/**
		 * Runs the applet until stop is called
		 * @return applet exit code
		 */
		nap::uint8 run();

		/**
		 * @return core instance
		 */
		nap::Core& getCore()					{ assert(mRunner != nullptr); return mRunner->getCore(); }

		/**
		 * @return app instance
		 */
		APP& getApp()							{ assert(mRunner != nullptr); return mRunner->getApp(); }

		/**
		 * @return handler
		 */
		HANDLER& getHandler()					{ assert(mRunner != nullptr); return mRunner->getHandler(); }

		//////////////////////////////////////////////////////////////////////////
		// Signals
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Called after the applet, including engine and required services, has been initialized
		 */
		nap::Signal<APP&> appletInitialized;

		/**
		 * Called when the applet started running
		 */
		nap::Signal<APP&> appletStarted;

		/**
		 * Called when the applet stops running
		 */
		nap::Signal<APP&> appletStopped;

	private:
		std::unique_ptr<AppletRunner<APP, HANDLER>> mRunner = nullptr;		///< Application runner
		std::future<nap::uint8> mTask;										///< The client server thread with exit code
	};



	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////


	template<typename APP, typename HANDLER>
	bool napkin::AppletLauncher<APP, HANDLER>::init(const std::string& projectFilename, nap::utility::ErrorState error)
	{
		// Initialize engine and application
		assert(mRunner == nullptr);
		mRunner = std::make_unique<napkin::AppletRunner<APP, HANDLER>>();
		if (!mRunner->init(projectFilename, nap::ProjectInfo::EContext::Editor, error))
		{
			nap::Logger::error("error: %s", error.toString().c_str());
			return false;
		}
		return true;
	}


	template<typename APP, typename HANDLER>
	nap::uint8 napkin::AppletLauncher<APP, HANDLER>::run()
	{
		// Start core
		assert(mRunner != nullptr);
		return mRunner->run();
	}

	/*
	template<typename APP, typename HANDLER>
	void AppletLauncher<APP, HANDLER>::run(const std::string& projectFilename)
	{
		// Create app runner
		assert(mRunner == nullptr);
		mRunner = std::make_unique<napkin::AppletRunner<APP, HANDLER>>();

		// Create and run task
		assert(!mTask.valid());
		mTask = std::async(std::launch::deferred, [projectFilename, this]()->nap::uint8
			{
				// Initialize engine and application
				nap::utility::ErrorState error;
				assert(mRunner != nullptr);
				if (!mRunner->init(projectFilename, nap::ProjectInfo::EContext::Editor, error))
				{
					nap::Logger::error("error: %s", error.toString().c_str());
					return napkin::applet::exitcode::initFailure;
				}

				// Return exit code
				return mRunner->exitCode();
			});
	}


	template<typename APP, typename HANDLER>
	int AppletLauncher<APP, HANDLER>::abort()
	{
		// Stop application task
		auto exit_code = napkin::applet::exitcode::invalid;
		if (mTask.valid())
		{
			assert(mRunner != nullptr);
			exit_code = mTask.get();
		}
		return exit_code;
	}
	*/
}
