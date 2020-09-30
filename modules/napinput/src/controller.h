#pragma once

#include <nap/numeric.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * The most common game-controller buttons
	 */
	enum class EControllerButton : int
	{
		UNKNOWN = -1,
		A,
		B,
		X,
		Y,
		BACK,
		GUIDE,
		START,
		LEFT_STICK,
		RIGHT_STICK,
		LEFT_SHOULDER,
		RIGHT_SHOULDER,
		DPAD_UP,
		DPAD_DOWN,
		DPAD_LEFT,
		DPAD_RIGHT
	};


	/**
	 * List of axis available from a controller
	 */
	enum class EControllerAxis : int
	{
		UNKNOWN	= -1,
		LEFT_X,
		LEFT_Y,
		RIGHT_X,
		RIGHT_Y,
		TRIGGER_LEFT,
		TRIGGER_RIGHT
	};
}