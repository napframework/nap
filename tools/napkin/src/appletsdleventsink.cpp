/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "appletsdleventsink.h"
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
	AppletSDLEventSink::AppletSDLEventSink(nap::uint frequency) :
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
		if (!nap::SDL::initVideo(error))
		{
			nap::Logger::error(error.toString());
			return;
		}

		if (nap::SDL::initVideo(error))
		{
			connect(&mTimer, &QTimer::timeout, this, &AppletSDLEventSink::flushEvents);
			int ms_poll = static_cast<int>(1000.0 / static_cast<double>(frequency));
			mTimer.start(ms_poll);
		}
		else
		{
			nap::Logger::error(error.toString());
		}

		// Disable interfering events
		/*
		SDL_EventState(SDL_DISPLAYEVENT, SDL_IGNORE);
		SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
		SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
		SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
		SDL_EventState(SDL_KEYUP, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
		SDL_EventState(SDL_MOUSEWHEEL, SDL_IGNORE);
		*/
	}

	
	AppletSDLEventSink::~AppletSDLEventSink()
	{
		nap::SDL::shutdownVideo();
	}



	void AppletSDLEventSink::flushEvents()
	{
		// Flush everything -> events are forward by the individual render panels
		// TODO: Remove SDL when using applets
		SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
	}
}
