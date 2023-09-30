/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mathutils.h>

namespace nap
{
	namespace audio
	{

		/**
		 * Wraps index value within the range of a buffer size.
		 * @param index value to be wrapped
		 * @param bufferSize size to be wrapped within, needs to be a power of two!
		 */
		inline unsigned int wrap(unsigned int index, unsigned int bufferSize)
		{
			unsigned int bitMask = bufferSize - 1;
			return index & bitMask;
		}


		/**
		 * Linear interpolation between two values.
		 * @param v0 start value of the interpolation which is returned when t = 0
		 * @param v1 end value of the interpolation which is returned when t = 1.
		 * @param t value between 0 and zero
		 * @return the result of the interpolation
		 */
		template<typename T>
		inline T lerp(const T& v0, const T& v1, const T& t)
		{
			return v0 + t * (v1 - v0);
		}

		/**
		 * Stereo equal power panning function.
		 * @param panning: value between 0 and 1.0, 0 meaning far left, 0.5 center and 1.0 far right.
		 * @param left: left channel gain will be stored in this variable
		 * @param right: right channel gain will be stored in this variable
		 */
		template<typename T>
		inline void equalPowerPan(const T& panning, T& left, T& right)
		{
			left = cos(panning * 0.5 * math::PI);
			right = sin(panning * 0.5 * math::PI);
		}


		/**
		 * Convert a midi notenumber format pitch (floating point for microtonal precision) to a frequency in Herz.
		 * @param pitch in semitones. A pitch of 57 equals 220Hz.
		 * @return frequency in Hz.
		 */
		inline float mtof(float pitch)
		{
			auto res = pitch - 57;
			res /= 12.0;
			res = pow(2.0, res);
			res *= 220.0;
			return res;
		}


		/**
		 * Convert amplitude to decibel value.
		 * @param amplitude Amplitude scaling factor. A value of 1.0 results in 0dB.
		 * @return intensity in dB.
		 */
		inline float toDB(float amplitude)
		{
			return 20 * log10(amplitude);
		}


		/**
		 * Convert decibel value to amplitude.
		 * @param db in dB.
		 * @param zero specifies the lowest possible dB input value, which will result in a zero return value.
		 * @return amplitude scaling factor.
		 */
		inline float dbToA(float db, float zero = -48)
		{
			if (db <= zero)
				return 0;

			return powf(10, db / 20.0);
		}

	}
}
