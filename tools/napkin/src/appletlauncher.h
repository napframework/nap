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
		 * Initializes the applet using the given launch policy
		 * @param projectFilename the project to initialize
		 */
		void init(const std::string& projectFilename, std::launch launchPolicy);

		/**
		 * Waits until the applet has been initialized
		 * @return if initialization succeeded
		 */
		bool initialized();

		/**
		 * Runs the applet using the given launch policy
		 * @return applet exit code
		 */
		void run(std::launch launchPolicy);

		/**
		 * Aborts and waits for the application to stop running
		 * @return application exit code
		 */
		nap::uint8 stop();

		/**
		 * @return core instance
		 */
		nap::Core& getCore()					{ return mRunner.getCore(); }

		/**
		 * @return app instance
		 */
		APP& getApp()							{ return mRunner.getApp(); }

		/**
		 * @return handler
		 */
		HANDLER& getHandler()					{ return mRunner.getHandler(); }

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
		AppletRunner<APP, HANDLER> mRunner;						///< Application runner
		std::future<bool> mInitTask;							///< The initialization future contract
		std::future<nap::uint8> mRunTask;						///< The run future contract
		std::atomic<bool> mAbort = { false };					///< Aborts the application from running

		/**
		 * Runs the applet until stop is called
		 * @return applet exit code
		 */
		nap::uint8 runApplet();
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APP, typename HANDLER>
	void AppletLauncher<APP, HANDLER>::init(const std::string& projectFilename, std::launch launchPolicy)
	{
		// Create and run task
		assert(!mInitTask.valid());
		mInitTask = std::async(launchPolicy, [projectFilename, this]()-> bool
			{
				// Initialize engine and application
				nap::utility::ErrorState error;
				if (!mRunner.init(projectFilename, nap::ProjectInfo::EContext::Editor, error))
				{
					nap::Logger::error("error: %s", error.toString().c_str());
					return false;
				}
				return true;
			});
	}


	template<typename APP, typename HANDLER>
	bool napkin::AppletLauncher<APP, HANDLER>::initialized()
	{
		if (mInitTask.valid())
			return mInitTask.get();
		return false;
	}


	template<typename APP, typename HANDLER>
	void napkin::AppletLauncher<APP, HANDLER>::run(std::launch launchPolicy)
	{
		mAbort = false;
		mRunTask = std::async(launchPolicy, [&]() -> nap::uint8
		{
			mRunner.start();
			while(!mAbort)
			{
				mRunner.process();
				std::this_thread::sleep_for(nap::Milliseconds(20));
			}
			return mRunner.stop();
		});
	}


	template<typename APP, typename HANDLER>
	nap::uint8 napkin::AppletLauncher<APP, HANDLER>::stop()
	{
		if (!mRunTask.valid())
			return napkin::applet::exitcode::invalid;

		// Shutdown
		mAbort = true;
		return mRunTask.get();
	}
}

