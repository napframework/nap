/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * Auto switch between dll import / export
 * Useful when building modules. This ensures that when building the module
 * and the NAP_SHARED_LIBRARY flag is enabled, the symbols are exported. When
 * using the library the symbols are imported
 */
#ifdef _WIN32
	#ifdef NAP_SHARED_LIBRARY
		#define NAPAPI __declspec(dllexport)	// Export the symbols
	#else
		#define NAPAPI __declspec(dllimport)	// Import the symbols
	#endif // NAP_SHARED_LIBRARY
#else
    #define NAPAPI __attribute__ ((visibility ("default")))
#endif // _WIN32
