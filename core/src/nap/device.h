/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "resource.h"

namespace nap
{
	/**
	 * Special type of Resource that represents a device. Only 1 instance of a device is allowed to run at the same time.
	 * For example: If you have a device that opens a specific port, that port can't be opened twice. Initialization should fail because of this reason. 
	 * The solution is to start() and stop() the device when 2 (or more) instances of that same device exist during the real-time edit fase. 
	 * In this specific case the port is closed when the device is stopped (first instance) before being opened on start (second instance).
	 *
	 * The Device class deals with this case by providing an explicit start / stop virtual, which are called at appropriate times by the ResourceManager.
	 * It's important that both start & stop can be called multiple times. start is always called before stop. stop is only called when start succeeded.
	 */
	class NAPAPI Device : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		 * Start the device. Called after initialization.
		 * When called it is safe to assume that all dependencies have been resolved up to this point.
		 * @param errorState The error state
		 * @return: true on success
		 */
		virtual bool start(utility::ErrorState& errorState) { return true; }

		/**
		 * Called when the device needs to be stopped, but only if start has previously been called on this Device. 
		 * It is safe to assume that when stop is called the device is in a 'started' state. Called in reverse init order.
		 */
		virtual void stop() {}
	};
}
