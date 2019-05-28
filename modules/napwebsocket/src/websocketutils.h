#pragma once

// External Includes
#include <utility/dllexport.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Enum describing the levels for logging web-socket errors.
	 * The levels can be bit-masked together.
	 */
	enum class EWebSocketLogLevel : uint32
	{
		None		= 0x0,				///< Log nothing
		Debug		= 0x1,				///< Low level debugging information
		Library		= 0x2,				///< Usual system states or small internal library problems
		Info		= 0x4,				///< Minor configuration problems or additional warning information
		Warning		= 0x8,				///< Information about important problems not severe enough to close connection.
		Error		= 0x10,				///< Recoverable error. Solution might be closing the connection with an appropiate error code
		Fatal		= 0x20,				///< Unrecoverable error. Will cause endpoint to be terminated unclean.
		All			= 0xffffffff		///< Log all levels
	};

	/**
	 * Utility function that combines the various log levels into a single bitmasked value.
	 * All log levels equal to and above the given level are logged.
	 * @param level the lowest log level to combine into a single field.
	 * @return the bitmasked (combined) log level
	 */
	NAPAPI uint32 computeWebSocketLogLevel(EWebSocketLogLevel level);
}
