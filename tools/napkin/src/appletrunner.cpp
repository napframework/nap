/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "appletrunner.h"
#include "imguiservice.h"
#include "apiservice.h"
#include "apievent.h"

namespace napkin
{
	AppletRunner::AppletRunner(nap::rtti::TypeInfo appletType)
	{
		assert(appletType.is_derived_from(RTTI_OF(napkin::Applet)));
		mApplet.reset(appletType.create<napkin::Applet>({ mCore }));
		assert(mApplet != nullptr);
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


	std::future<bool> AppletRunner::start(const std::string& projectFilename, nap::uint frequency)
	{
		// Create and run task
		mThread = std::thread([projectFilename, frequency, this]() mutable
		{
			// Initialize engine and application
			nap::utility::ErrorState error;
			if (!initEngine(projectFilename, error))
			{
				// Notify waiting thread init failed
				nap::Logger::error(error.toString());
				mInitPromise.set_value(false);
				return;
			}

			// Notify waiting thread init succeeded
			mInitPromise.set_value(true);

			// Start running on success
			runApplet(frequency);

			// Always clear services
			mServices = nullptr;
		});

		// Allows the calling thread to wait for (sync) with the applet
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
		auto service_handle = mCore.initializeServices(error);
		if (service_handle == nullptr)
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

		// Store services
		mServices = std::move(service_handle);
		return true;
	}


	void AppletRunner::runApplet(nap::uint frequency)
	{
		double wd = 1000.0 / static_cast<double>(nap::math::clamp<nap::uint>(frequency, 1, 1000));
		nap::Milliseconds wm(static_cast<int>(wd));
		std::queue<nap::EventPtr> event_queue;

		auto* gui_service = mApplet->getCore().getService<nap::IMGuiService>();
		auto* api_service = mApplet->getCore().getService<nap::APIService>();

		mCore.start();
		std::function<void(double)> update_call = std::bind(&Applet::update, mApplet.get(), std::placeholders::_1);
		while (!mAbort)
		{
			std::unique_lock<std::mutex> lock(mProcessMutex);
			mProcessCondition.wait_for(lock, wm, [this]
				{
					return !mEventQueue.empty();
				}
			);

			// Trade and unlock for further processing
			assert(event_queue.empty());
			event_queue.swap(mEventQueue);
			lock.unlock();

			// Forward input to running app
			while (!event_queue.empty())
			{
				auto& event_ref = event_queue.front();

				// Input (mouse, keyboard etc.) event
				if (event_ref->get_type().is_derived_from(RTTI_OF(nap::InputEvent)))
				{
					// Allow gui service to process input
					auto* gui_ctx = gui_service != nullptr ?
						gui_service->processInputEvent(static_cast<nap::InputEvent&>(*event_ref)) : nullptr;

					// Skip forwarding event if processed (captured) by gui
					if (gui_ctx != nullptr)
					{
						// Gui is capturing this keyboard event
						if (event_ref->get_type().is_derived_from(RTTI_OF(nap::KeyEvent)) &&
							gui_service->isCapturingKeyboard(gui_ctx))
						{
							event_queue.pop();
							continue;
						}

						// Gui is capturing this mouse event
						if ((event_ref->get_type().is_derived_from(RTTI_OF(nap::PointerEvent)) ||
							event_ref->get_type().is_derived_from(RTTI_OF(nap::MouseWheelEvent))) &&
							gui_service->isCapturingMouse(gui_ctx))
						{
							event_queue.pop();
							continue;
						}
					}

					// Gui not available or capturing -> forward to app
					auto* input_event = static_cast<nap::InputEvent*>(event_ref.release());
					mApplet->inputMessageReceived(std::unique_ptr<nap::InputEvent>(input_event));
				}

				// Window event
				else if (event_ref->get_type().is_derived_from(RTTI_OF(nap::WindowEvent)))
				{
					auto* window_event = static_cast<nap::WindowEvent*>(event_ref.release());
					mApplet->windowMessageReceived(std::unique_ptr<nap::WindowEvent>(window_event));
				}

				// API event
				else if (event_ref->get_type().is_derived_from(RTTI_OF(nap::APIEvent)))
				{
					NAP_ASSERT_MSG(api_service != nullptr,
						"Sending API message without API functionality enabled in Applet")

					nap::utility::ErrorState error;
					auto* api_event = static_cast<nap::APIEvent*>(event_ref.release());
					if (!api_service->sendEvent(nap::APIEventPtr(api_event), &error))
						nap::Logger::error(error.toString());
				}

				// Always remove item!
				event_queue.pop();
			}

			// Update core and application
			mCore.update(update_call);

			// Render content
			mApplet->render();
		}
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
}

