/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <audio/utility/audiotypes.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Utility class representing a single delay that can be written and read from.
		 * Supports interpolation between samples while reading.
		 */
		class NAPAPI Delay
		{
		public:
			/**
			 * The buffer size has to be a power of 2
			 */
			Delay(unsigned int bufferSize);
			
			~Delay() = default;
			
			/**
			 * Write a sample to the delay line at the current write position
			 */
			void write(SampleValue sample);
			
			/**
			 * Read a sample from the delay line at @time samples behind the write position.
			 * Non interpolating.
			 */
			SampleValue read(unsigned int time);
			
			/**
			 * Same as @read() but with interpolation between samples
			 */
			SampleValue readInterpolating(float sampleTime);
			
			/**
			 * Clear the delay line by flushing its buffer.
			 */
			void clear();
			
			/**
			 * @return: return the maximum delay. (equalling the size of the buffer)
			 */
			unsigned int getMaxDelay() { return mBuffer.size(); }
			
			/**
			 * Operator to read from the delay line without interpolation at @index before the write position
			 */
			inline SampleValue operator[](unsigned int index) { return read(index); }
		
		private:
			
			SampleBuffer mBuffer;
			unsigned int mWriteIndex = 0;
		};
		
	}
}
