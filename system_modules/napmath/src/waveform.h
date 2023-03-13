/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <rtti/rtti.h>

namespace nap
{
	namespace math
	{
		/**
		 *	Various available waveforms
		 */
		enum class EWaveform : int
		{
			SINE			= 0,			///< Sine
			SQUARE,							///< Square
			SAW,							///< Saw
			TRIANGLE						///< Triangle
		};

		/**
		 * @return a normalized value (0-1) associated with a specific type of waveform
		 * @param type the waveform type to query
		 * @param time point in time to sample the waveform
		 * @param frequency the waveform frequency
		 * @return a normalized (0-1) waveform value
		 */
		float NAPAPI waveform(EWaveform type, float time, float frequency);
	}
}
