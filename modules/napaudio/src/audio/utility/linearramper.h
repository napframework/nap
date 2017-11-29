#pragma once

// Nap includes
#include <nap/signalslot.h>

// Audo includes
#include <audio/utility/audiotypes.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Used to make linear ramps up and down of a value in steps
         */
        template <typename T>
        class LinearRamper {
        public:
            LinearRamper(T& value) : mValue(value) { }
            
            /**
             * Start a ramp
             * @param destination: the finishing value
             * @param stepCount: the number of steps
             */
            void ramp(const T& destination, int stepCount)
            {
                // if there are zero steps we reach the destination of the ramp immediately
                if (stepCount == 0)
                {
                    mStepCount = 0;
                    mValue = destination;
                    destinationReachedSignal(mValue);
                }
                
                mDestination = destination;
                mStepCount = stepCount;
                mStepCounter = 0;
                mIncrement = (mDestination - mValue) / T(mStepCount);
            }
            
            /**
             * Take the next step in the current ramp
             */
            void step()
            {
                if (mStepCount > 0)
                {
                    mValue += mIncrement;
                    mStepCounter++;
                    if (mStepCounter == mStepCount)
                    {
                        mStepCount = 0;
                        mValue = mDestination;
                        destinationReachedSignal(mValue);
                    }
                }
            }
            
            /**
             * Stop the current ramp
             */
            void stop()
            {
                mStepCount = 0;
            }
            
            /**
             * Returns true when currently playing a ramp
             */
            bool isRamping() const { return mStepCount > 0; }
            
            /**
             * Signal emitted when the destination of a ramp has been reached.
             */
            nap::Signal<T> destinationReachedSignal;
            
        private:
            T& mValue;
            T mIncrement = 0;
            T mDestination = 0;
            int mStepCounter = 0;
            int mStepCount = 0;
        };
        
        
    }
    
}
