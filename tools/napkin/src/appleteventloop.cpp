/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "appleteventloop.h"
#include "applet.h"

// External includes
#include <QThread>
#include <QCoreApplication>
#include <nap/assert.h>
#include <sdlhelpers.h>
#include <nap/logger.h>
#include <mathutils.h>
#include <SDL_events.h>

namespace napkin
{
	AppletEventLoop::AppletEventLoop(nap::uint frequency) :
		mFrequency(nap::math::min<nap::uint>(frequency, 1))
	{
		// SDL event loop must run on the QT GUI thread
		NAP_ASSERT_MSG(QThread::currentThread() == QCoreApplication::instance()->thread(),
			"SDL event loop must be created and running on the QT GUI thread");

		// Video subsystem must be initialized by the loop on the GUI thread
		NAP_ASSERT_MSG(!nap::SDL::videoInitialized(),
			"Video subsystem already initialized");

		// Initialize SDL video subsystem
		nap::utility::ErrorState error;
		if (nap::SDL::initVideo(error))
		{
			connect(&mTimer, &QTimer::timeout, this, &AppletEventLoop::pollEvent);
			int ms_poll = static_cast<int>(1000.0 / static_cast<double>(frequency));
			mTimer.start(ms_poll);
		}
		else
		{
			nap::Logger::error(error.toString());
		}
	}

	
	AppletEventLoop::~AppletEventLoop()
	{
		nap::SDL::shutdownVideo();
	}


	void AppletEventLoop::setApplet(napkin::AppletRunner& applet)
	{
		mRunner = &applet;
		auto* sdl_service = mRunner->getCore().getService<nap::SDLInputService>();
		assert(sdl_service != nullptr);
		mEventConverter = std::make_unique<nap::SDLEventConverter>(*sdl_service);
	}


	void AppletEventLoop::pollEvent()
	{
		// Flush everything if we're not targeting a specific applet
		if (mRunner == nullptr)
		{
			SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			return;
		}

		assert(mRunner != nullptr);
		SDL_Event event; 
		nap::EventPtrList events;
		while (SDL_PollEvent(&event) > 0)
		{
			// Check if we are dealing with an input event (mouse / keyboard)
			if (mEventConverter->isInputEvent(event))
			{
				nap::InputEventPtr input_event = mEventConverter->translateInputEvent(event);
				if (input_event != nullptr)
				{
					mRunner->sendEvent(std::move(input_event));
				}
			}

			// Check if we're dealing with a window event
			else if (mEventConverter->isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = mEventConverter->translateWindowEvent(event);
				if (window_event != nullptr)
				{
					mRunner->sendEvent(std::move(window_event));
				}
			}
		}
	}
}
