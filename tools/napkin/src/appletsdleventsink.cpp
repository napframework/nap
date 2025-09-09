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

#include "appcontext.h"

namespace napkin
{
	AppletSDLEventSink::AppletSDLEventSink(nap::uint frequency, nap::EVideoDriver driver, const QApplication& app) :
		mFrequency(nap::math::min<nap::uint>(frequency, 1))
	{
		// SDL event loop must run on the QT GUI thread
		NAP_ASSERT_MSG(QThread::currentThread() == QCoreApplication::instance()->thread(),
			"SDL event loop must be created and running on the QT GUI thread");

		// Video subsystem must be initialized by the loop on the GUI thread
		NAP_ASSERT_MSG(!nap::SDL::videoInitialized(),
			"Video subsystem already initialized");

		// Wayland windows and surfaces are more intrinsically tied to the client library than other windowing systems,
		// therefore, when importing surfaces, it is necessary for both SDL and the application or toolkit to use the
		// same wl_display object, see: https://wiki.libsdl.org/SDL3/README-wayland
		switch (driver)
		{
#ifdef __linux__
		case nap::EVideoDriver::Wayland:
			{
				auto* wl_app = app.nativeInterface<QNativeInterface::QWaylandApplication>();
				assert(wl_app != nullptr);
				auto* wl_display = wl_app->display();
				if (wl_display == nullptr)
				{
					nap::Logger::error("Unable to get wayland display handle");
					break;
				}

				// Set wayland video display pointer for SDL
				if (!SDL_SetPointerProperty(SDL_GetGlobalProperties(),
					SDL_PROP_GLOBAL_VIDEO_WAYLAND_WL_DISPLAY_POINTER, wl_display))
				{
					nap::Logger::error("Unable to set wayland display handle");
					break;
				}

				// Handle set -> continue to default initialization
				[[fallthrough]];
			}
#endif
		default:
			{
				// Initialize SDL video subsystem
				nap::utility::ErrorState error;
				if (nap::SDL::initVideo(driver, error))
				{
					connect(&mTimer, &QTimer::timeout, this, &AppletSDLEventSink::flushEvents);
					int ms_poll = static_cast<int>(1000.0 / static_cast<double>(frequency));
					mTimer.start(ms_poll);
				}
				else
				{
					nap::Logger::error(error.toString());
				}
			}
		}
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
