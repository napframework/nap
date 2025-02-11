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
			constexpr nap::uint8 success	= 0;
			constexpr nap::uint8 invalid	= 2;
		}

		namespace timing
		{
			constexpr nap::uint hz	= 60;				///< Default applet target process frequency
			constexpr double frame	= 1.0 / hz;			///< Default applet target frame time
		}
	}

	/**
	 * Utility class that runs a napkin::Applet on a seperate thread until AppletRunner::abort() is called.
	 * This class is different from a regular AppRunner because it is thread safe,
	 * and uses api events to send messages to and receive messages from the running application.
	 *
	 * On start() the runner initializes nap::Core, together with all requested services and loads the application data.
	 * If initialization succeeds the application process loop is started until suspend() or abort() is called.
	 * If the runner is started in suspended mode the application loop won't start until run() is called.
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
		 * @param projectPath full path to the project to run
		 * @pram editorInfo the project being edited in napkin
		 * @param suspend process loop from running after successful initialization
		 * @return if initialization succeeded or not
		 */
		std::future<bool> start(const std::string& projectPath, const nap::ProjectInfo& editorInfo, bool suspend);

		/**
		 * Returns if the applet launched on a separate thread and is in an active state.
		 * Note that paused applets are still considered active.
		 * @return if the applet launched and is in an active state.
		 */
		bool active() const												{ return mThread.joinable(); }

		/**
		 * Interrupts the applet process loop until run is called.
		 * You can use the future to synchronize (wait) until the process loop is suspended.
		 * Note that the returned future is invalid when the applet is not active or already suspended.
		 * @return future suspension, invalid when inactive or already suspended
		 */
		std::future<bool> suspend();

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
		nap::uint					mFrequency = applet::timing::hz;		///< Target frame process frequency (hz)
		double						mFrameTarget = applet::timing::frame;	///< Target frame process time in seconds
		std::queue<nap::EventPtr>	mEventQueue;							///< Events to forward to the running app
		std::thread					mThread;								///< Running thread
		bool						mSuspend = false;						///< If the applet is suspended from running
		std::unique_ptr<std::promise<bool>> mSuspendPromise = nullptr;		///< If the applet is requested to be suspended

		/**
		 * Initializes the engine and the application.
		 * @param projectPath the applet project to load and run
		 * @param editorInfo the project being edited in napkin
		 * @param error the error message if the app failed to initialize
		 * @return if the app initialized successfully started
		 */
		bool initEngine(const std::string& projectPath, std::unique_ptr<nap::ProjectInfo> editorInfo,  nap::utility::ErrorState& error);

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
