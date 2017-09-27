#pragma once

// Std includes
#include <math.h>

// Audio includes
#include <utility/audiotypes.h>

namespace nap {
    
    namespace audio {
        
        /**
         * Used to ramp a value up and down in steps according to the formula destinationValue = startValue^stepCount
         */
        template <typename T>
        class ExponentialRamper {
        public:
            static constexpr T smallestValue = 100000.0;
            
        public:            
            ExponentialRamper(T& value) : mValue(value) { }

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
                
                // avoid divisions by zero by avoiding mValue = 0
                if (mValue == 0)
                    mValue = mDestination / smallestValue; // this is a 140dB ramp up from mValue to mDestination
                
                // avoid divisions by zero by avoiding mDestination = 0
                if (mDestination == 0)
                {
                    mDestination = mValue / smallestValue; // this is a 140 dB tamp down from mValue to mDestination
                    mDestinationZero = true;
                }
                else
                    mDestinationZero = false;
                
                // calculate the increment factor
                mFactor = pow(double(mDestination / mValue), double(1.0 / mStepCount));
            }
            
            /**
             * Take the next step in the current ramp
             */
            void step()
            {
                if (mStepCount > 0)
                {
                    mValue *= mFactor;
                    mStepCounter++;
                    if (mStepCounter == mStepCount)
                    {
                        mStepCount = 0;
                        if (mDestinationZero)
                            mValue = 0;
                        else
                            mValue = mDestination;
                        
                        destinationReachedSignal(mValue);
                    }
                }
            }
            
            void stop()
            {
                mStepCount = 0;
            }
            
            bool isRamping() const { return mStepCount > 0; }

            nap::Signal<T> destinationReachedSignal;

        private:
            T& mValue;
            T mFactor = 0;
            T mDestination = 0;
            int mStepCounter = 0;
            int mStepCount = 0;
            bool mDestinationZero = false;
        };
        
        
    }
    
}

