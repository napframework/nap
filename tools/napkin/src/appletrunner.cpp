/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "appletrunner.h"

namespace napkin
{
	std::future<bool> AppletRunner::run(const std::string& projectFilename, nap::uint frequency, std::future<bool> syncTask)
	{
		// Create and run task
		mThread = std::thread([task = std::move(syncTask), projectFilename, frequency, this]() mutable
		{
			// Initialize engine and application
			nap::utility::ErrorState error;
			if (!initEngine(projectFilename, error))
			{
				mInitPromise.set_value(false);
				return;
			}

			// Notify other threads we initialized
			mInitPromise.set_value(true);

			// Wait for other thread to initialize
			if (task.get())
			{
				runApplet(frequency);
			}
		});
		return mInitPromise.get_future();
	}


	bool napkin::AppletRunner::initEngine(const std::string& projectInfo, nap::utility::ErrorState& error)
	{
		// Initialize engine
		if (!error.check(mCore.initializeEngine(projectInfo, nap::ProjectInfo::EContext::Editor, error),
			"Unable to initialize engine"))
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

		// Initialize application
		if (!error.check(mApplet->init(error), "Unable to initialize applet"))
			return false;

		return true;
	}


	AppletRunner::AppletRunner(nap::rtti::TypeInfo appletType)
	{
		assert(appletType.is_derived_from(RTTI_OF(napkin::Applet)));
		auto* applet = appletType.create<napkin::Applet>({ mCore });
		assert(applet != nullptr);
		mApplet.reset(applet);
	}


	void AppletRunner::sendEvent(nap::EventPtr event)
	{
		// Swap and notify handling thread
		{
			std::lock_guard lk(mProcessMutex);
			mEventQueue.emplace(std::move(event));
		}
		mProcessCondition.notify_one();
	}


	nap::uint8 AppletRunner::abort()
	{
		if (mThread.joinable())
		{
			mAbort = true;
			mThread.join();
			return::napkin::applet::exitcode::success;
		}
		return::napkin::applet::exitcode::invalid;
	}


	void AppletRunner::runApplet(nap::uint frequency)
	{
		mCore.start();
		double wd = 1000.0 / static_cast<double>(nap::math::clamp<nap::uint>(frequency, 1, 1000));
		nap::Milliseconds wm(static_cast<int>(wd));
		std::queue<nap::EventPtr> event_queue;

		std::function<void(double)> update_call = std::bind(&Applet::update, mApplet.get(), std::placeholders::_1);
		while (!mAbort)
		{
			std::unique_lock<std::mutex> lk(mProcessMutex);
			mProcessCondition.wait_for(lk, wm, [this]
				{
					return !mEventQueue.empty();
				}
			);

			// Trade and unlock for further processing
			assert(event_queue.empty());
			event_queue.swap(mEventQueue);
			lk.unlock();

			// Forward input to running app
			while (!event_queue.empty())
			{
				auto& event_ref = event_queue.front();
				if (event_ref->get_type().is_derived_from(RTTI_OF(nap::InputEvent)))
				{
					auto* input_event = static_cast<nap::InputEvent*>(event_ref.release());
					mApplet->inputMessageReceived(std::unique_ptr<nap::InputEvent>(input_event));
				}

				else if (event_ref->get_type().is_derived_from(RTTI_OF(nap::WindowEvent)))
				{
					auto* window_event = static_cast<nap::WindowEvent*>(event_ref.release());
					mApplet->windowMessageReceived(std::unique_ptr<nap::WindowEvent>(window_event));
				}
				event_queue.pop();
			}

			// Update core and application
			mCore.update(update_call);

			// Render content
			mApplet->render();
		}
		mServices = nullptr;
	}
}


