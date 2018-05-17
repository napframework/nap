#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <nap/signalslot.h>
#include <nap/logger.h>

// Audo includes
#include <audio/utility/audiotypes.h>
#include <audio/utility/dirtyflag.h>

namespace nap
{
    
    namespace audio
    {
        
        enum RampMode { Linear, Exponential };
        
        /**
         * Used to make linear or exponential ramps up and down of a value in steps.
         * The length of the ramp and the kind of ramp can be specified for each ramp.
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
                mIsDirty.set();
            }

            /**
             * Stop the current ramp.
             */
            void stop()
            {
                mNewStepCount.store(0);
                mNewDestination.store(mValue);
                mIsDirty.set();
            }
            
            /**
             * Take the next step in the current ramp.
             * Should only be called from the audio thread.
             */
            T getNextValue()
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
                    }
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
                        
            /**
             * Signal emitted when the destination of a ramp has been reached.
             */
            nap::Signal<T> destinationReachedSignal;
            
        private:
            void updateRamp()
            {
                if (mIsDirty.check())
                {
                    mDestination = mNewDestination.load();
                    mStepCount = mNewStepCount.load();
                    mRampMode = mNewRampMode.load();
                    
                    // if there are zero steps we reach the destination of the ramp immediately
                    if (mStepCount <= 0)
                    {
                        mStepCounter = 0;
                        mValue = mDestination;
                        destinationReachedSignal(mValue);
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
            
        private:
            std::atomic<T> mNewDestination = { 0 };
            std::atomic<int> mNewStepCount = { 0 };
            std::atomic<RampMode> mNewRampMode = { RampMode::Linear };
            DirtyFlag mIsDirty;

            T mValue; // Value that is being controlled by this object.
            
            union {
                T mIncrement; // Increment value per step of the current ramp when mode is linear.
                T mFactor; // Factor value per step of the current ramp when mode is exponential.
            };
            T mDestination = 0; // Destination value of the current ramp.
            int mStepCount = 0; // Number of steps in the ramp.
            int mStepCounter = { 0 }; // Current step index, 0 means at destination
            RampMode mRampMode = { RampMode::Linear }; // The mode of the current ramp
            bool mDestinationZero = false; // In case of a linear ramp this indicates wether the destination value needs to be rounded to zero.
        };
        
        
    }
    
}
