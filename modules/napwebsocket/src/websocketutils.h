/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <utility/dllexport.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Enum that describes the web-socket log level. 
	 * The levels can be bit-masked together.
	 */
	enum class EWebSocketLogLevel : uint32
	{
		None			= 0x0,				///< Log nothing
		Debug			= 0x1,				///< Low level debugging information
		Library			= 0x2,				///< Usual system states or small internal library problems
		Info			= 0x4,				///< Minor configuration problems or additional warning information
		Warning			= 0x8,				///< Information about important problems not severe enough to close connection.
		Error			= 0x10,				///< Recoverable error. Solution might be closing the connection with an appropiate error code
		Fatal			= 0x20,				///< Unrecoverable error. Will cause endpoint to be terminated unclean.
		All				= 0xffffffff		///< Log all levels
	};

	enum class EWebSocketOPCode : uint32
	{
		Continuation	= 0x0,
		Text			= 0x1,
		Binary			= 0x2,
		RSV3			= 0x3,
		RSV4			= 0x4,
		RSV5			= 0x5,
		RSV6			= 0x6,
		RSV7			= 0x7,
		Close			= 0x8,
		Ping			= 0x9,
		Pong			= 0xA,
		ControlRSVB		= 0xB,
		ControlRSVC		= 0xC,
		ControlRSVD		= 0xD,
		ControlRSVE		= 0xE,
		ControlRSVF		= 0xF
	};

	/**
	 * Utility function that combines the various log levels into a single bitmasked value.
	 * All log levels equal to and above the given level are logged.
	 * @param level the lowest log level to combine into a single field.
	 * @return the bitmasked (combined) log level
	 */
	NAPAPI uint32 computeWebSocketLogLevel(EWebSocketLogLevel level);
}
