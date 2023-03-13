/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the yoctopuce library.
	 * This service doesn't initialize the library explicitly. 
	 * Initialization of the yoctopuce lib happens when connecting to a hub for the first time.
	 * This service does however explicitly shut the yoctopuce lib down to prevent memory leaks.
	 */
	class NAPAPI YoctoService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		YoctoService(ServiceConfiguration* configuration);

	protected:

		/**
		 * Explicitly free Yoctopuce library
		 */
		virtual void shutdown() override;
	};
}
