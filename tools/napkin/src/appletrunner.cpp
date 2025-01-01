/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "appletrunner.h"
#include "imguiservice.h"
#include "apiservice.h"
#include "apievent.h"
#include "appcontext.h"
#include "naputils.h"

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
			// Notify thread we're done!
			mAbort = true;
			mProcessCondition.notify_one();
			mThread.join();
			return::napkin::applet::exitcode::success;
		}
		return::napkin::applet::exitcode::invalid;
	}


	std::future<bool> AppletRunner::start(const std::string& projectFilename, nap::uint frequency)
	{
		// Create and run task
		assert(!active());
		mFrequency = frequency; 

		// Create the promise used to signal initialization
		std::promise<bool> init_promise;
		auto init_future = init_promise.get_future();

		mThread = std::thread([projectFilename, initPromise = std::move(init_promise), this]() mutable
		{
			// Initialize engine and application
			nap::utility::ErrorState error;
			if (!initEngine(projectFilename, error))
			{
				// Notify waiting thread init failed
				nap::Logger::error(error.toString());
				initPromise.set_value(false);
				return;
			}

			// Notify waiting thread init succeeded
			initPromise.set_value(true);

			// Start running on success
			runApplet();

			// Always clear services
			mServices = nullptr;
		});

		// Allows the calling thread to wait for (sync) with the applet
		return init_future;
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

		// Ensure project data is available
		if (!error.check(!mCore.getProjectInfo()->mDefaultData.empty(), "Missing project data, %s 'Data' field is empty",
			mCore.getProjectInfo()->getProjectDir().c_str()))
			return false;

		// Change current working directory to directory that contains the data file and load
		{
			napkin::CWDHandle cwd(mCore.getProjectInfo()->getDataDirectory());
			std::string data_file = nap::utility::getFileName(mCore.getProjectInfo()->getDataFile());
			nap::Logger::info("Loading data: %s", data_file.c_str());
			if (!error.check(mCore.getResourceManager()->loadFile(data_file, error), "Failed to load data: %s", data_file.c_str()))
				return false;
		}

		// Initialize application
		mApplet->mEditorInfo = std::move(AppContext::get().getProjectInfo()->clone());
		if (!error.check(mApplet->init(error), "Unable to initialize applet"))
			return false;

		// Store services
		mServices = std::move(service_handle);

		return true;
	}


	std::shared_future<bool> AppletRunner::pause()
	{
		// Create a promise that the applet will be suspended -> notify the listener when that occurs using the returned future.
		// Note that it is possible that a different thread calls run before the thread is suspended,
		// which continues execution and could break the promise.
		// The promise therefore returns IF the thread actually suspended and is always called, regardless of inter-frame changes.
		{
			assert(active());
			std::unique_lock<std::mutex> lock(mProcessMutex);
			mSuspend = true;
			if (mSuspendPromise == nullptr)
				mSuspendPromise = std::make_unique<std::promise<bool>>();
		}
		mProcessCondition.notify_one();
		return mSuspendPromise->get_future();
	}


	void AppletRunner::run()
	{
		{
			std::unique_lock<std::mutex> lock(mProcessMutex);
			mSuspend = false;
		}
		mProcessCondition.notify_one();
	}


	void AppletRunner::runApplet()
	{
		auto* gui_service = mApplet->getCore().getService<nap::IMGuiService>();
		auto* api_service = mApplet->getCore().getService<nap::APIService>();
	
		std::queue<nap::EventPtr> event_queue;
		std::function<void(double)> update_call = std::bind(&Applet::update, mApplet.get(), std::placeholders::_1);

		// Run until abort
		mCore.start();
		while (true)
		{
			// Thread safe section! 
			// Wait until ready for processing -> unlocks when exiting this block
			{
				std::unique_lock<std::mutex> lock(mProcessMutex);
				nap::Milliseconds wmss(mFrequency > 0 ? 1000 / mFrequency : 0);
				if (wmss.count() == 0)
				{
					// Wait indefinitely until event is received or frequency is positive.
					// We also proceed if a request to suspend or abort is received.
					mProcessCondition.wait(lock, [this] {
							return mFrequency > 0 || !mEventQueue.empty() || mSuspendPromise != nullptr || mAbort;
						});
				}
				else
				{
					// Wait until a event is received or x amount of time has passed
					// We also proceed if a request to suspend or abort is received
					mProcessCondition.wait_for(lock, wmss, [this] {
							return !mEventQueue.empty() || mAbort || mSuspendPromise != nullptr;
						}
					);
				}

				// Handle applet suspension:
				// This one is a tad tricky because the user could call run() between suspension and waiting for the suspension to occur using the returned future.
				// Although this sequence isn't likely it can't be ruled out, especially from multiple threads. We therefore must
				// always handle the suspension promise and notify the user if it remains suspended or continues operation, for that
				// we track an addition suspended boolean that can be flipped regardless of a previous suspension request.
				// This prevents the app from throwing an exception when the future is invalidated because the promise has been broken (removed).
				if (mSuspendPromise != nullptr)
				{
					mSuspendPromise->set_value(mSuspend);
					mSuspendPromise.reset(nullptr);
					mProcessCondition.wait(lock, [this] {
							return !mSuspend || mAbort;
						});
				}

				// Bail if we're told to stop (unlocks the handle)
				if (mAbort)
				{
					auto code = mApplet->shutdown();
					if (code != applet::exitcode::success)
					{
						nap::Logger::error("Applet '%s' failed to shut down gracefully, exit code: %d",
							mApplet->get_type().get_name().data());
					}
					mAbort = false;
					return;
				}

				// Trade events and unlock for further processing
				assert(event_queue.empty());
				event_queue.swap(mEventQueue);
			}

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
							// Convert keyboard event into utf8 character and queue in the gui.
							// We explicitly forward them here because the QT process takes ownership of the keyboard.
							//
							// TODO: we can (should?) store the utf char encoding directly in the key press event, both QT and SDL
							// support conversion of a key event into a utf character, allowing us to bypass the nap key mapping.
							if (event_ref->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
							{
								auto utf_char = static_cast<const nap::KeyPressEvent&>(*event_ref).toUtf8();
								if(utf_char != 0x00)
									gui_service->addInputCharachter(gui_ctx, utf_char);
							}
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


	void AppletRunner::setFrequency(nap::uint frequency)
	{
		// Swap and notify handling thread
		{
			std::lock_guard lk(mProcessMutex);
			mFrequency = frequency;
		}
		mProcessCondition.notify_one();
	}
}
