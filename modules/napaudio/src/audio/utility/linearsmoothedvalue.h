#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <nap/signalslot.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Used to smooth changed to a value linearly over time, using a fixed smoothing time.
         */
        template <typename T>
        class LinearSmoothedValue {
        public:

        public:
            LinearSmoothedValue(const T& initValue, int stepCount) : mValue(initValue)
            {
                mStepCount.store(stepCount);
            }
            
            /**
             * Change the number of steps the value takes to reach a new destination.
             */
            void setStepCount(int stepCount)
            {
                mStepCount.store(stepCount);
            }
            
            /**
             * Start a ramp
             * @param destination: the finishing value
             */
            void setValue(const T& destination)
            {
                mNewDestination.store(destination);
            }

            /**
             * Take the next step in the current ramp.
             * Should only be called from the audio thread.
             */
            T getNextValue()
            {
                T newDestination = mNewDestination.load();
                
                if (newDestination != mDestination)
                {
                    mDestination = newDestination;
                    mStepCounter = mStepCount.load();
                    mIncrement = (mDestination - mValue) / T(mStepCount);
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
            T getValue() const
            {
                return mValue;
            }
            
            /**
             * Returns true when currently playing a ramp.
             * Should only be called from the audio thread.
             */
            bool isRamping() const { return mStepCounter > 0; }
                        
        private:
            std::atomic<T> mNewDestination = { 0 };

            T mValue; // Value that is being controlled by this object.
            T mIncrement; // Increment value per step of the current ramp when mode is linear.
            T mDestination = 0; // Destination value of the current ramp.
            std::atomic<int> mStepCount = { 0 }; // Number of steps in the ramp.
            int mStepCounter = { 0 }; // Current step index, 0 means at destination
        };
        
        
    }
    
}
