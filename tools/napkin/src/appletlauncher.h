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
	template<typename APPLET, typename HANDLER>
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
		 * @param launchPolicy deferred or a-synchronous
		 */
		void init(const std::string& projectFilename, std::launch launchPolicy);

		/**
		 * Waits until the applet has been initialized
		 * @return if initialization succeeded
		 */
		bool initialized();

		/**
		 * Runs the applet using the given launch policy
		 * @param launchPolicy deferred or a-synchronous
		 * @parawm frequency process frequency (hz)
		 * @return applet exit code
		 */
		void run(std::launch launchPolicy, nap::uint frequency);

		/**
		 * @return if the applet is running
		 */
		bool running() const						{ return mRunTask.valid(); }

		/**
		 * Aborts and waits for the application to stop running
		 * @return application exit code
		 */
		nap::uint8 abort();

		/**
		 * @return core instance
		 */
		nap::Core& getCore()						{ return mRunner.getCore(); }

		/**
		 * @return app instance
		 */
		APPLET& getApplet()							{ return mRunner.getApplet(); }

		/**
		 * @return handler
		 */
		HANDLER& getHandler()						{ return mRunner.getHandler(); }

	private:
		AppletRunner<APPLET, HANDLER> mRunner;		///< Application runner
		std::future<bool> mInitTask;				///< The initialization future contract
		std::future<nap::uint8> mRunTask;			///< The run future contract
		std::atomic<bool> mAbort = { false };		///< Aborts the application from running
		std::atomic<nap::uint> mFrequency = 60;		///< Processing frequency (hz)
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename APPLET, typename HANDLER>
	void AppletLauncher<APPLET, HANDLER>::init(const std::string& projectFilename, std::launch launchPolicy)
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


	template<typename APPLET, typename HANDLER>
	bool napkin::AppletLauncher<APPLET, HANDLER>::initialized()
	{
		if (mInitTask.valid())
			return mInitTask.get();
		return false;
	}


	template<typename APPLET, typename HANDLER>
	void napkin::AppletLauncher<APPLET, HANDLER>::run(std::launch launchPolicy, nap::uint frequency)
	{
		mAbort = false;
		mRunTask = std::async(launchPolicy, [frequency, this]() -> nap::uint8
		{
			nap::SteadyTimer process_timer;
			mRunner.start();
			float ffreq = static_cast<float>(frequency);

			while(!mAbort)
			{
				// Process frame
				process_timer.start();
				mRunner.process();

				// Sleep a bit
				if (ffreq > nap::math::epsilon<float>())
				{
					auto tick = nap::math::min<float>((float)(process_timer.getTicks()), 1000.0f);
					auto wait = static_cast<nap::uint32>((1000.0f - tick) / ffreq);
					std::this_thread::sleep_for(nap::Milliseconds(wait));
				}
			}
			return mRunner.stop();
		});
	}


	template<typename APPLET, typename HANDLER>
	nap::uint8 napkin::AppletLauncher<APPLET, HANDLER>::abort()
	{
		if (!mRunTask.valid())
			return napkin::applet::exitcode::invalid;

		// Shutdown
		mAbort = true;
		return mRunTask.get();
	}
}

