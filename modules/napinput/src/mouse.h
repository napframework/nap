#pragma once

#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Enum describing a list of all possible mouse buttons
	 */
	enum class EMouseButton : int
	{
		UNKNOWN		= -1,
		LEFT		= 0,
		MIDDLE		= 1,
		RIGHT		= 2
	};
}