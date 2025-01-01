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
	 * AppletRunner::abort().
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
		/**
		 * Creates the runner for the given applet type
		 * @param appletType the type of applet to create and run
		 */
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
		 * Initializes and runs the applet a-synchronous on it's own thread.
		 * Use the future to synchronize (wait) until initialized succeeded or failed.
		 * @param projectFilename full path to the project to run
		 * @param frequency update frequency in hz
		 * @return if initialization succeeded or not
		 */
		std::future<bool> start(const std::string& projectFilename, nap::uint frequency);

		/**
		 * Returns if the applet launched on a separate thread and is in an active state.
		 * Note that paused applets are still considered active.
		 * @return if the applet launched and is in an active state.
		 */
		bool active() const												{ return mThread.joinable(); }

		/**
		 * Interrupts the applet process loop until run is called.
		 * Only call this function when the applet is in an active state!
		 * You can use the future to synchronize (wait) until the process loop is suspended.
		 * @return if the process is suspended
		 */
		std::shared_future<bool> suspend();

		/**
		 * Resume applet process loop.
		 */
		void run();

		/** 
		 * Sends an event to the app for processing, thread safe
		 * @param event the event to send
		 */
		void sendEvent(nap::EventPtr event);

		/**
		 * Sets the application update frequency in hz.
		 * A frequency of 0 stalls the app indefinitely until an event is received
		 * @param frequency update frequency in hz
		 */
		void setFrequency(nap::uint frequency);

		/**
		 * Aborts and waits for the application to stop running
		 * @return application exit code
		 */
		nap::uint8 abort();

		/**
		 * @return core
		 */
		nap::Core& getCore()												{ return mCore; }

	private:
		nap::Core mCore;													/// Associated core instance
		nap::Core::ServicesHandle mServices = nullptr;						/// Initialized services
		std::unique_ptr<napkin::Applet> mApplet;							/// Applet to run

		mutable std::mutex			mProcessMutex;							///< Process related mutex
		std::condition_variable		mProcessCondition;						///< Process condition variable
		bool						mAbort = false;							///< Aborts the application from running
		nap::uint					mFrequency = 60;						///< Processing frequency (hz)
		std::queue<nap::EventPtr>	mEventQueue;							///< Events to forward to the running app
		std::thread					mThread;								///< Running thread
		bool						mSuspend = false;						///< If the applet is paused
		std::unique_ptr<std::promise<bool>> mSuspendPromise = nullptr;		///< If the applet is requested to be suspended

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
		void runApplet();
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

