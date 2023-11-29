/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "fftbuffer.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	namespace utility
	{
		/**
		 * @param binCount the number of frequency bins
		 * @param nyqist nyqist frequency in Hz (e.g. 44100 Hz)
		 * @return frequency interval in Hz
		 */
		float NAPAPI interval(uint binCount, float nyqist);

		/**
		 * @param bin the bin to retrieve the frequency from
		 * @param binCount the number of frequency bins
		 * @param nyqist nyqist frequency in Hz (e.g. 44100 Hz)
		 * @return frequency at bin in Hz
		 */
		float NAPAPI freq(uint bin, uint binCount, float nyqist);

		/**
		 * Negates amplitudes outside of cutoff frequency
		 * @param inAmps input amplitude spectrum
		 * @param outAmps output amplitude spectrum where amplitudes outside of cutoff frequency are negated
		 * @param minBin minimum bin to cutoff
		 * @param maxBin maximum bin to cutoff
		 */
		void NAPAPI cutoff(const FFTBuffer::AmplitudeSpectrum& inAmps, FFTBuffer::AmplitudeSpectrum& outAmps, uint minBin, uint maxBin);

		/**
		 * Computes spectral average of amplitude spectrum
		 * @param amps input amplitude spectrum
		 * @param minBin minimum bin to cutoff
		 * @param maxBin maximum bin to cutoff
		 * @return the spectrum average
		 */
		float NAPAPI average(const FFTBuffer::AmplitudeSpectrum& amps, uint minBin = 0, uint maxBin = 0);

		/**
		 * Computes spectral centroid of amplitude spectrum
		 * @param amps input amplitude spectrum
		 * @return the normalized spectral centroid
		 */
		float NAPAPI centroid(const FFTBuffer::AmplitudeSpectrum& amps);

		/**
		 * Computes spectral flux of amplitude spectrum
		 * @param current the amplitude spectrum of the current frame
		 * @param previous the amplitude spectrum of the previous frame
		 * @param minBin minimum bin to cutoff
		 * @param maxBin maximum bin to cutoff
		 * @return the spectral flux
		 */
		float NAPAPI flux(const FFTBuffer::AmplitudeSpectrum& current, const FFTBuffer::AmplitudeSpectrum& previous, uint minBin, uint maxBin);
	}
}
