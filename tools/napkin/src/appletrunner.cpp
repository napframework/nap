/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "appletrunner.h"

namespace napkin
{
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

		// Watch the data directory
		mCore.getResourceManager()->watchDirectory(data_dir);

		// Initialize application
		if (!error.check(mApplet->init(error), "Unable to initialize application"))
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


	void AppletRunner::init(const std::string& projectFilename, std::launch launchPolicy)
	{
		// Create and run task
		assert(!mInitTask.valid());
		mInitTask = std::async(launchPolicy, [projectFilename, this]()-> bool
			{
				// Initialize engine and application
				nap::utility::ErrorState error;
				if (!initEngine(projectFilename, error))
				{
					nap::Logger::error("error: %s", error.toString().c_str());
					return false;
				}
				return true;
			});
	}


	bool AppletRunner::initialized()
	{
		if (mInitTask.valid())
			return mInitTask.get();
		return false;

	}


	void AppletRunner::run(std::launch launchPolicy, nap::uint frequency)
	{
		mAbort = false;
		mRunTask = std::async(launchPolicy, [frequency, this]() -> nap::uint8
			{
				mCore.start();
				double wd = 1000.0 / static_cast<double>(nap::math::clamp<nap::uint>(frequency, 1, 1000));
				nap::Milliseconds wm(static_cast<int>(wd));

				std::function<void(double)> update_call = std::bind(&Applet::update, mApplet.get(), std::placeholders::_1);
				while (!mAbort)
				{
					std::unique_lock<std::mutex> lk(mProcessMutex);
					mProcessCondition.wait_for(lk, wm, [this] 
						{
							return !mEvents.empty();
						}
					);

					// Forward input and window events to application (thread safe)
					for (auto& event : mEvents)
					{
						if(event->get_type().is_derived_from(RTTI_OF(nap::InputEvent)))
						{
							auto* input_event = static_cast<nap::InputEvent*>(event.release());
							mApplet->inputMessageReceived(std::unique_ptr<nap::InputEvent>(input_event));
							continue;
						}

						if (event->get_type().is_derived_from(RTTI_OF(nap::WindowEvent)))
						{
							auto* input_event = static_cast<nap::WindowEvent*>(event.release());
							mApplet->windowMessageReceived(std::unique_ptr<nap::WindowEvent>(input_event));
							continue;
						}
					}

					// Clear and unlock
					mEvents.clear();
					lk.unlock();

					// Update core and application
					mCore.update(update_call);

					// Render content
					mApplet->render();
				}

				return static_cast<nap::uint8>(mApplet->shutdown());
			});
	}


	void AppletRunner::sendEvents(nap::EventPtrList& events)
	{
		// Swap and notify handling thread
		{
			std::lock_guard lk(mProcessMutex);
			mEvents.swap(events);
		}
		mProcessCondition.notify_one();
	}


	nap::uint8 AppletRunner::abort()
	{
		if (!mRunTask.valid())
			return napkin::applet::exitcode::invalid;

		// Shutdown
		mAbort = true;
		return mRunTask.get();
	}
}
