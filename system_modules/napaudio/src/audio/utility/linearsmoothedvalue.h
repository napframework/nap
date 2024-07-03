/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/signalslot.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Used to smooth changed to a value linearly over time, using a fixed smoothing time.
		 */
		template<typename T>
		class LinearSmoothedValue
		{
		public:
		
		public:
			LinearSmoothedValue(const T& initValue, int stepCount) : mNewDestination(initValue), mValue(initValue), mDestination(initValue), mStepCount(stepCount)
			{
			}

			/**
			 * Quit all smoothing in progress and reset output to the passed value.
			 * Don't call this while getNextValue() can be called.
			 * @param initValue the new output value.
			 */
			void reset(const T& initValue)
			{
				mNewDestination.store(initValue);
				mValue = initValue;
				mDestination = initValue;
			}
			
			/**
			 * Change the number of steps the value takes to reach a new destination.
			 */
			void setStepCount(int stepCount) { mStepCount.store(stepCount); }
			
			/**
			 * Start a ramp
			 * @param destination: the finishing value
			 */
			void setValue(const T& destination) { mNewDestination.store(destination); }
			
			/**
			 * Take the next step in the current ramp.
			 * Should only be called from the audio thread.
			 */
			T getNextValue()
			{
				T newDestination = mNewDestination.load();
				int stepCount = mStepCount.load();
				
				if (newDestination != mDestination)
				{
					mDestination = newDestination;
					mStepCounter = stepCount;
					if (mStepCounter == 0)
						mValue = mDestination;
					else
						mIncrement = (mDestination - mValue) / T(stepCount);
				}
				
				if (mStepCounter > 0)
				{
					mValue = mValue + mIncrement;
					mStepCounter--;
					if (mStepCounter == 0)
						mValue = mDestination;
				}
				
				return mValue;
			}
			
			/**
			 * Returns the current value.
			 * Should only be called from the audio thread
			 */
			inline T getValue() const { return mValue; }
			
			inline T getDestination() const { return mNewDestination.load(); }
			
			/**
			 * Returns true when currently playing a ramp.
			 * Should only be called from the audio thread.
			 */
			inline bool isRamping() const { return mStepCounter > 0 || mDestination != mNewDestination.load(); }
		
		private:
			std::atomic<T> mNewDestination = 0;
			
			T mValue; // Value that is being controlled by this object.
			T mIncrement; // Increment value per step of the current ramp when mode is linear.
			T mDestination = 0; // Destination value of the current ramp.
			std::atomic<int> mStepCount = 0; // Number of steps in the ramp.
			int mStepCounter = 0; // Current step index, 0 means at destination
		};
		
		
	}
}

