/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// std library includes
#include <vector>

// nap includes
#include <utility/dllexport.h>

namespace nap
{
	
	namespace audio
	{
		
		// Note: in this files typedefs are being used instead of a using statement
		// This is because the using statement cannot be used in combination with the NAPAPI macro
		
		
		/**
		 * Value of a single audio sample
		 * Change this to double to build with double precision sample calculation
		 */
		using SampleValue = float;
		
		
		/**
		 * A buffer of samples
		 */
		using SampleBuffer = std::vector<SampleValue>;
		
		
		/**
		 * A collection of sample buffers, one for each channel to represent multichannel audio.
		 */
		class NAPAPI MultiSampleBuffer
		{
		public:
			MultiSampleBuffer() = default;
			
			/**
			 * @param channelCount: number of channels in this buffer
			 * @param size: size of the buffer in samples
			 */
			MultiSampleBuffer(std::size_t channelCount, std::size_t size)
			{
				resize(channelCount, size);
			}
			
			/**
			 * Used to access the samples in the buffer
			 * example: myBuffer[channelNumber][sampleIndex]
			 */
			SampleBuffer& operator[](std::size_t index)
			{
				return channels[index];
			}
			
			/**
			 * @return: number of channels in the buffer
			 */
			std::size_t getChannelCount() const { return channels.size(); }
			
			/**
			 * @return: the size of the buffer in samples
			 */
			std::size_t getSize() const { return channels.empty() ? 0 : channels.front().size(); }
			
			/**
			 * Resize the buffer
			 * @param channelCount: new number of channels
			 * @param size: new size in samples
			 */
			void resize(std::size_t channelCount, std::size_t size)
			{
				channels.resize(channelCount);
				for (auto& channel : channels)
					channel.resize(size);
			}
			
			/**
			 * Reserve capacity of the buffer in memory to prevent repeated memory allocation
			 * @param channelCount: new number of channels capacity
			 * @param size: new size in samples capacity
			 */
			void reserve(std::size_t channelCount, std::size_t size)
			{
				channels.reserve(channelCount);
				for (auto& channel : channels)
					channel.reserve(size);
			}
			
			/**
			 * Clear the content of the buffer.
			 */
			void clear()
			{
				channels.clear();
			}
			
			std::vector<SampleBuffer> channels;
		};
		
		
		/**
		 * Value of control parameter
		 * Change this to double to build with double precision sample calculation
		 */
		using ControllerValue = float;
		
		
		/**
		 * Time value in milliseconds
		 */
		using TimeValue = float;
		
		
		/**
		 * Time value in samples
		 */
		using DiscreteTimeValue = long;
		
		
		/**
		 * Mode to calculate a ramp from one value to another in a certain amount of steps
		 */
		enum RampMode
		{
			Linear, Exponential
		};
		
		
	}
	
}
