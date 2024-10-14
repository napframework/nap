/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "appleteventloop.h"

// External includes
#include <QThread>
#include <QCoreApplication>
#include <nap/assert.h>
#include <sdlhelpers.h>
#include <nap/logger.h>
#include <mathutils.h>

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


	void AppletEventLoop::pollEvent()
	{
		//nap::Logger::info("SDL process callback");
	}
}
