/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/core.h>
#include <apprunner.h>
#include <thread>
#include <future>
#include <nap/projectinfo.h>
#include <functional>

namespace napkin
{
	namespace exitcode
	{
		constexpr int invalid		= 2;	///< Not launched
		constexpr int failStart		= 1;	///< Failure to start
	}

	/**
	 * Wraps and runs a NAP app in a separate thread until aborted.
	 */
	template<typename APP, typename HANDLER>
	class AppLauncher
	{
	public:
		AppLauncher() = default;

		// Terminates the application if running 
		virtual ~AppLauncher()									{ abort(); }

		/**
		 * Copy is not allowed
		 */
		AppLauncher(AppLauncher&) = delete;
		AppLauncher& operator=(const AppLauncher&) = delete;

		/**
		 * Move is not allowed
		 */
		AppLauncher(AppLauncher&&) = delete;
		AppLauncher& operator=(AppLauncher&&) = delete;

		/**
		 * Launches and runs the app in a separate thread
		 * @param projectInfoFile the project info to load
		 */
		void run(const std::string& projectFilename);

		/**
		 * Aborts the running of the application.
		 * @return application exit code
		 */
		int abort();

	private:
		nap::Core mCore;													///< Unique core instance
		std::unique_ptr<nap::AppRunner<APP, HANDLER>> mRunner = nullptr;	///< Application runner
		std::future<int> mTask;												///< The client server thread with exit code
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////


	template<typename APP, typename HANDLER>
	void AppLauncher<APP, HANDLER>::run(const std::string& projectFilename)
	{
		// Create app runner
		assert(mRunner == nullptr);
		mRunner = std::make_unique<nap::AppRunner<APP, HANDLER>>(mCore, projectFilename,
			nap::ProjectInfo::EContext::Editor);

		// Create and run task
		assert(!mTask.valid());
		mTask = std::async(std::launch::async, [&]()->int
			{
				// Initialize and run application until stopped
				nap::utility::ErrorState error;
				assert(mRunner != nullptr);
				if (!mRunner->start(error))
				{
					nap::Logger::error("error: %s", error.toString().c_str());
					return napkin::exitcode::failStart;
				}

				// Return exit code
				return mRunner->exitCode();
			});
	}


	template<typename APP, typename HANDLER>
	int AppLauncher<APP, HANDLER>::abort()
	{
		// Stop application task
		auto exit_code = napkin::exitcode::invalid;
		if (mTask.valid())
		{
			assert(mRunner != nullptr);
			mRunner->stop();
			exit_code = mTask.get();
			mRunner = nullptr;
		}
		return exit_code;
	}
}
