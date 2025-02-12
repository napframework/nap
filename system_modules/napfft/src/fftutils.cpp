/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fftutils.h"

// External includes
#include <mathutils.h>

namespace nap
{
	namespace utility
	{
		float interval(uint binCount, float nyqist)
		{
			return 1.0f / binCount * nyqist;
		}


		float freq(uint bin, uint binCount, float nyqist)
		{
			assert(bin <= binCount);
			return bin * (nyqist / binCount);
		}


		void cutoff(const FFTBuffer::AmplitudeSpectrum& inAmps, FFTBuffer::AmplitudeSpectrum& outAmps, uint minBin, uint maxBin)
		{
			assert(inAmps.size() == outAmps.size());

			const uint min = minBin;
			const uint max = (maxBin == 0) ? inAmps.size() : std::clamp<uint>(inAmps.size(), min + 1, maxBin);
			assert(max > min);

			outAmps.clear();
			for (uint i = min; i < max; i++)
				outAmps[i] = inAmps[i];
		}


		float average(const FFTBuffer::AmplitudeSpectrum& amps, uint minBin, uint maxBin)
		{
			const uint min = minBin;
			const uint max = (maxBin == 0) ? amps.size() : std::clamp<uint>(amps.size(), min + 1, maxBin);
			assert(max > min);

			float sum = 0.0f;
			for (uint i = min; i < max; i++)
				sum += amps[i];

			// Compute average
			return sum / static_cast<float>(max-min);
		}


		float centroid(const FFTBuffer::AmplitudeSpectrum& amps)
		{
			float sum = 0.0f;
			float weight = 0.0f;

			for (uint i = 0; i < amps.size(); i++)
			{
				sum += amps[i];
				weight += amps[i] * (i + 1);
			}

			if (weight <= math::epsilon<float>())
				return 0.0f;

			// Compute centroid
			return sum / weight;
		}


		float flux(const FFTBuffer::AmplitudeSpectrum& current, const FFTBuffer::AmplitudeSpectrum& previous, uint minBin, uint maxBin)
		{
			assert(current.size() == previous.size());

			const uint min = minBin;
			const uint max = (maxBin == 0) ? current.size() : std::clamp<uint>(current.size(), min+1, maxBin);
			assert(max > min);

			float flux = 0.0f;
			for (uint i = min; i < max; i++)
			{
				const float x = current[i] - previous[i];
				flux += x*x;
			}
			flux = std::sqrt(flux);
			return flux;
		}
	}
}
