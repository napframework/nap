/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/core.h>
#include <nap/numeric.h>
#include <nap/projectinfo.h>
#include <nap/logger.h>
#include <future>
#include <condition_variable>

// Local includes
#include "applet.h"

namespace napkin
{
	namespace applet
	{
		namespace exitcode
		{
			constexpr nap::uint8 success		= 0;
			constexpr nap::uint8 invalid		= 2;
		}
	}

	/**
	 * Utility class that runs a napkin::Applet until Applet::quit() is called or
	 * AppletRunner::abort(). The APP template argument should be derived from
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
	class AppletRunner
	{
	public:
		// Default constructor
		AppletRunner(nap::rtti::TypeInfo appletType);

		/**
		 * Copy is not allowed
		 */
		AppletRunner(AppletRunner&) = delete;
		AppletRunner& operator=(const AppletRunner&) = delete;

		/**
		 * Move is not allowed
		 */
		AppletRunner(AppletRunner&&) = delete;
		AppletRunner& operator=(AppletRunner&&) = delete;

		/**
		 * Initializes and runs the applet a-synchronous.
		 * Initialization and processing happens on the same thread!
		 * @param projectFilename full path to the project to run
		 * @param process frequency (hz)
		 * @param syncTask task to wait for (sync with) after initialization
		 * @return if initialization succeeded or not
		 */
		std::future<bool> start(const std::string& projectFilename, nap::uint frequency);

		/**
		 * Sends an event to the app for processing, thread safe
		 * @param events the events to send
		 */
		void sendEvent(nap::EventPtr event);

		/**
		 * @return if the applet is running
		 */
		bool running() const												{ return mThread.joinable(); }

		/**
		 * Aborts and waits for the application to stop running
		 * @return application exit code
		 */
		nap::uint8 abort();

		/**
		 * @return the applet
		 */
		napkin::Applet& getApplet()											{ assert(mApplet != nullptr); return *mApplet; }

		/**
		 * @return the applet 
		 */
		const napkin::Applet& getApplet() const								{ assert(mApplet != nullptr); return *mApplet; }

		/**
		 * @return core
		 */
		nap::Core& getCore()												{ return mCore; }

	private:
		nap::Core mCore;													/// Associated core instance
		nap::Core::ServicesHandle mServices = nullptr;						/// Initialized services
		std::unique_ptr<napkin::Applet> mApplet;							/// Applet to run

		std::mutex					mProcessMutex;							///< Process related mutex
		std::condition_variable		mProcessCondition;						///< Process condition variable
		std::atomic<bool>			mAbort = { false };						///< Aborts the application from running
		std::atomic<nap::uint>		mFrequency = 60;						///< Processing frequency (hz)
		std::queue<nap::EventPtr>	mEventQueue;							///< Events to forward to the running app
		std::thread					mThread;								///< Running thread
		std::promise<bool>			mInitPromise;							///< If runner initialized correctly

		/**
		 * Initializes the engine and the application.
		 * @param projectInfo the project to load and run
		 * @param error the error message if the app failed to initialize
		 * @return if the app initialized successfully started
		 */
		bool initEngine(const std::string& projectInfo, nap::utility::ErrorState& error);

		/**
		 * Runs the applet until stopped
		 */
		void runApplet(nap::uint frequency);
	};


	//////////////////////////////////////////////////////////////////////////
	// Typed template runner 
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	class TypedAppletRunner : public AppletRunner
	{
	public:
		TypedAppletRunner() : AppletRunner(RTTI_OF(T)) { }
	};
}

