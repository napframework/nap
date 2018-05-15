#pragma once

// Std includes
#include <atomic>

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
            LinearRamper(T& value) : mValue(value)
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
                mUpToDate = false;
                mIsRamping = true;
            }
            
            void updateRamp()
            {
                if (mUpToDate == false)
                {
                    mUpToDate = true;
                    mDestination = mNewDestination.load();
                    mStepCount = mNewStepCount.load();
                    
                    // if there are zero steps we reach the destination of the ramp immediately
                    if (mStepCount <= 0)
                    {
                        mStepCounter = 0;
                        mValue = mDestination;
                        if (mIsRamping)
                        {
                            destinationReachedSignal(mValue);
                            mIsRamping = false;
                        }
                        return;
                    }
                    
                    mStepCounter = mStepCount;
                    
                    //--- specific for linear
                    mIncrement = (mDestination - mValue) / T(mStepCount);
                    //---
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
                    mValue += mIncrement; // specific for linear
                    mStepCounter--;
                    if (mStepCounter == 0)
                    {
                        mValue = mDestination; // specific for linear
                        destinationReachedSignal(mValue);
                        mIsRamping = false;
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
                mUpToDate = false;
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
            std::atomic<bool> mUpToDate = { true };
            std::atomic<bool> mIsRamping =  { false };
            
            T& mValue; // Value that is being controlled by this object.
            T mIncrement = 0; // Increment value per step of the current ramp.
            T mDestination = 0; // Destination value of the current ramp.
            int mStepCount = 0; // Number of steps in the ramp.
            std::atomic<int> mStepCounter = { 0 }; // Current step index, 0 means at destination
        };
        
        
    }
    
}
