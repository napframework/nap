/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/numeric.h>
#include <QTimer>
#include <sdleventconverter.h>
#include <videodriver.h>

// Local Includes
#include "appletrunner.h"

namespace nap
{
	class IMGuiService;
}

namespace napkin
{
	class Applet;

	/**
	 * Initializes the various required SDL subsystems for NAP (notably video) and disables SDL events.
	 * Applets intercept and transform QT events into NAP events, eliminating the need for SDL events entirely.
	 * This class MUST be created exactly once and running on the GUI (main) QT thread.
	 */
	class AppletSDLEventSink : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Initializes the video subsystem and creates the SDL event sink
		 * @param event flush frequency (hz)
		 * @param driver SDL video driver
		 */
		AppletSDLEventSink(nap::uint frequency, nap::EVideoDriver driver, const QApplication& qapp);

		/**
		 * Shuts down the event loop and closes video subsystem
		 */
		~AppletSDLEventSink() override;

	private:
		nap::uint mFrequency = 60;
		QTimer mTimer;
		napkin::AppletRunner* mRunner = nullptr;
		std::unique_ptr<nap::SDLEventConverter> mEventConverter = nullptr;

		void flushEvents();
	};
}
