#pragma once

// Std includes
#include <math.h>

// Audio includes
#include <audio/utility/audiotypes.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Used to ramp a value up and down in steps according to the formula destinationValue = startValue^stepCount
         */
        template <typename T>
        class ExponentialRamper {
        public:
            static constexpr T smallestValue = 100000.0;
            
        public:            
            ExponentialRamper(T& value) : mValue(value)
            {
//                mUpToDateFlag.test_and_set();
            }

            /**
             * Start a ramp
             * @param destination: the finishing value
             * @param stepCount: the number of steps
             */
            void ramp(const T& destination, int stepCount)
            {
                assert(stepCount >= 0);
                
                mNewStepCount.store(stepCount);
                mNewDestination.store(destination);
//                mUpToDateFlag.clear();
                mUpToDateFlag = false;
            }
                
            void updateRamp()
            {
                if (mUpToDateFlag == false)
                {
                    mUpToDateFlag = true;
                    mDestination = mNewDestination.load();
                    mStepCount = mNewStepCount.load();
                    
                    // if there are zero steps we reach the destination of the ramp immediately
                    if (mStepCount <= 0)
                    {
                        mStepCount = 0;
                        mValue = mDestination;
                        if (mIsRamping)
                        {
                            destinationReachedSignal(mValue);
                            mIsRamping = false;
                        }
                        return;
                    }
                    
                    mDestination = mDestination;
                    mStepCounter = mStepCount;
                    
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
            }
            
            /**
             * Take the next step in the current ramp
             */
            void step()
            {
                updateRamp();
                
                if (mStepCounter > 0)
                {
                    mValue *= mFactor;
                    mStepCounter--;
                    if (mStepCounter == 0)
                    {
                        if (mDestinationZero)
                            mValue = 0;
                        else
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
                mNewStepCount.store(0);
                mNewDestination.store(mValue);
//                mUpToDateFlag.clear();
                mUpToDateFlag = false;
                mIsRamping = false;
            }
            
            /**
             * Returns true when currently playing a ramp
             */
            bool isRamping() const { return mIsRamping; }

            /**
             * Signal emitted when the destination of a ramp has been reached.
             */
            nap::Signal<T> destinationReachedSignal;

        private:
            std::atomic<T> mNewDestination = { 0 };
            std::atomic<int> mNewStepCount = { 0 };
            std::atomic<bool> mUpToDateFlag = { true };
            std::atomic<bool> mIsRamping = { false };
//            std::atomic_flag mUpToDateFlag = ATOMIC_FLAG_INIT;
            
            T& mValue; // Value that is being controlled by this object.
            T mFactor = 0; // Multiplication factor per step of the current ramp
            T mDestination = 0; // Destination value of the current ramp
            int mStepCounter = 0; // Index of the current step in the ramp, 0 means at destination or not ramping
            int mStepCount = 0;
            bool mDestinationZero = false; // Indicates wether the destination of the current ramp will be rounded to zero
        };
        
        
    }
    
}

