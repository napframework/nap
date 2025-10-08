/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <QTEnvironmentVariables>

namespace napkin
{
	namespace env
	{
		namespace option
		{
			// Disable Vulkan applets inside the editor
			constexpr const char* NAPKIN_DISABLE_APPLETS = "NAPKIN_DISABLE_APPLETS";

			// QT Platform (wayland, xcb etc..)
			constexpr const char* QT_QPA_PLATFORM = "QT_QPA_PLATFORM";
		}

		/**
		 * Returns if the given environment variable is defined and set to 1 (enabled)
		 * @param env the environment variable to check
		 * @return if the given environment variable is defined and set to 1 (enabled)
		 */
		bool enabled(const char* env)	{ auto v = qgetenv(env); return v != nullptr && v.toInt() == 1; }

		/**
		 * Returns if the given environment variable isn't defined or set to 0 (disabled)
		 * @param env the environment variable to check
		 * @return if the given environment variable isn't defined or set to 0 (disabled)
		 */
		bool disabled(const char* env)	{ auto v = qgetenv(env); return v == nullptr || v.toInt() == 0; }

		/**
		 * Set given environment variable to value
		 * @param env the name of the environment variable
		 * @param value the value of the environment variable
		 * @return if the value was set
		 */
		bool set(const char* env, const char* value) { qputenv(env, value); }
	}
}
