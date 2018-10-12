#pragma once

#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Enum describing a list of all possible mouse buttons
	 */
	enum class NAPAPI EMouseButton : int
	{
		UNKNOWN		= -1,
		LEFT		= 0,
		MIDDLE		= 2,
		RIGHT		= 3
	};
}