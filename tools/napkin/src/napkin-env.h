/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <QtEnvironmentVariables>

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
		inline bool enabled(const char* env)	{ auto v = qgetenv(env); return v != nullptr && v.toInt() == 1; }

		/**
		 * Returns if the given environment variable isn't defined or set to 0 (disabled)
		 * @param env the environment variable to check
		 * @return if the given environment variable isn't defined or set to 0 (disabled)
		 */
		inline bool disabled(const char* env)	{ auto v = qgetenv(env); return v == nullptr || v.toInt() == 0; }

		/**
		 * Set given environment variable to value
		 * @param env the name of the environment variable
		 * @param value the value of the environment variable
		 * @return if the value was set
		 */
		inline bool set(const char* env, const char* value) { return qputenv(env, value); }

		/**
		 * Get environment variable if it exists
		 * @param env the environment variable to check
		 * @return environment value, empty if it does not exist
		 */
		inline std::string get(const char* env) { auto v = qgetenv(env); return env != nullptr ? v.toStdString() : std::string(); }
	}
}
