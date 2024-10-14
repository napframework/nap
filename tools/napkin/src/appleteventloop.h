/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/numeric.h>
#include <QTimer>

namespace napkin
{
	/**
	 * Initializes the various required SDL subsystems for NAP (notably video) and polls events.
	 * This class MUST be created and running on the GUI (main) QT thread.
	 * The events are forwarded (thread safe) to the various NAP applets running in Napkin.
	 */
	class AppletEventLoop : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Creates the event loop that polls SDL for events at the given frequency.
		 * @param poll frequency (hz)
		 */
		AppletEventLoop(nap::uint frequency);

		/**
		 * Shuts down the event loop and close
		 */
		virtual ~AppletEventLoop();

	private:
		nap::uint mFrequency = 60;
		bool mInitialized = false;

		QTimer mTimer;
		void pollEvent();
	};
}
