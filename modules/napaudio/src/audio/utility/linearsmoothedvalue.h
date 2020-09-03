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
        template <typename T>
        class LinearSmoothedValue {
        public:
            
        public:
            LinearSmoothedValue(const T& initValue, int stepCount) : mNewDestination(initValue), mValue(initValue), mDestination(initValue)
            {
                mStepCount = stepCount;
            }
            
            /**
             * Change the number of steps the value takes to reach a new destination.
             */
            void setStepCount(int stepCount)
            {
                mStepCount = stepCount;
            }
            
            /**
             * Start a ramp
             * @param destination: the finishing value
             */
            void setValue(const T& destination)
            {
                mNewDestination = destination;
            }
            
            /**
             * Take the next step in the current ramp.
             * Should only be called from the audio thread.
             */
            T getNextValue()
            {
                if (mNewDestination != mDestination)
                {
                    mDestination = mNewDestination;
                    mStepCounter = mStepCount;
                    if (mStepCounter == 0)
                        mValue = mDestination;
                    else
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
            inline T getValue() const
            {
                return mValue;
            }
            
            inline T getDestination() const { return mNewDestination; }
            
            /**
             * Returns true when currently playing a ramp.
             * Should only be called from the audio thread.
             */
            inline bool isRamping() const { return mStepCounter > 0 || mDestination != mNewDestination; }
            
        private:
            T mNewDestination = 0;
            
            T mValue; // Value that is being controlled by this object.
            T mIncrement; // Increment value per step of the current ramp when mode is linear.
            T mDestination = 0; // Destination value of the current ramp.
            int mStepCount = 0; // Number of steps in the ramp.
            int mStepCounter = 0; // Current step index, 0 means at destination
        };
        
        
    }
    
}

