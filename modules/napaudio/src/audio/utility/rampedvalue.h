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
        
        enum RampMode { Linear, Exponential };
        
        /**
         * Used to make linear ramps up and down of a value in steps
         */
        template <typename T>
        class RampedValue {
        public:
            static constexpr T smallestFactor = 0.00001f;

        public:
            RampedValue(const T& initValue) : mValue(initValue)
            {
            }
            
            /**
             * Start a ramp
             * @param destination: the finishing value
             * @param stepCount: the number of steps
             */
            void ramp(const T& destination, int stepCount, RampMode mode = RampMode::Linear)
            {
                assert(stepCount >= 0);
                
                mNewStepCount.store(stepCount);
                mNewDestination.store(destination);
                mNewRampMode.store(mode);
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
                    mRampMode = mNewRampMode.load();
                    
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
                    
                    switch (mRampMode) {
                        case RampMode::Linear:
                            mIncrement = (mDestination - mValue) / T(mStepCount);
                            break;
                            
                        case RampMode::Exponential:
                            // avoid divisions by zero by avoiding mValue = 0
                            if (mValue == 0)
                                mValue = mDestination * smallestFactor; // this is a 140dB ramp up from mValue to mDestination
                            
                            // avoid divisions by zero by avoiding mDestination = 0
                            if (mDestination == 0)
                            {
                                mDestination = mValue * smallestFactor; // this is a 140 dB ramp down from mValue to mDestination
                                mDestinationZero = true;
                            }
                            else
                                mDestinationZero = false;
                            
                            // calculate the increment factor
                            mFactor = pow(double(mDestination / mValue), double(1.0 / mStepCount));
                            break;
                    }
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
                    switch (mRampMode) {
                        case RampMode::Linear:
                            mValue = mValue + mIncrement;
                            break;
                        case RampMode::Exponential:
                            mValue = mValue * mFactor;
                            break;
                    }
                    mStepCounter--;
                    if (mStepCounter == 0)
                    {
                        if (mRampMode == RampMode::Exponential && mDestinationZero)
                            mValue = 0;
                        else
                            mValue = mDestination;
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
                mUpToDate = false;
                mIsRamping = false;
            }
            
            /**
             * Returns true when currently playing a ramp
             */
            bool isRamping() const { return mIsRamping; }
            
            /**
             * Sets the value immediately if not ramping, otherwise stop the ramp and set in on next step() call.
             */
            void setValue(const T& value)
            {
                if (mIsRamping)
                    ramp(value, 0);
                else
                    mValue = value;
            }
            
            /**
             * Returns the current value.
             */
            T getValue() const { return mValue; }
            
            /**
             * Signal emitted when the destination of a ramp has been reached.
             */
            nap::Signal<T> destinationReachedSignal;
            
        private:
            std::atomic<T> mNewDestination = { 0 };
            std::atomic<int> mNewStepCount = { 0 };
            std::atomic<RampMode> mNewRampMode = { RampMode::Linear };
            std::atomic<bool> mUpToDate = { true };
            std::atomic<bool> mIsRamping =  { false };
            
            std::atomic<T> mValue; // Value that is being controlled by this object.
            union {
                T mIncrement; // Increment value per step of the current ramp.
                T mFactor; // Factor value per step of the current ramp
            };
            T mDestination = 0; // Destination value of the current ramp.
            int mStepCount = 0; // Number of steps in the ramp.
            int mStepCounter = { 0 }; // Current step index, 0 means at destination
            RampMode mRampMode = { RampMode::Linear }; // The mode of the current ramp
            bool mDestinationZero = false; // In case of a linear ramp this indicates wether the destination value needs to be rounded to zero.
        };
        
        
    }
    
}
