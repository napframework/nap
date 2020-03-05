#pragma once

// Std includes
#include <atomic>

// RTTI includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/rampedvalue.h>
#include <audio/utility/translator.h>
#include <audio/utility/safeptr.h>
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Used to generate a control signal by ramping between different values.
         * Ramps can be either linear or exponential.
         * Optionally a lookup table can be used to shape the output signal.
         */
        class NAPAPI ControlNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            ControlNode(NodeManager& manager);
            
            /**
             * The output signal pin
             */
            OutputPin output = { this };
            
            /**
             * Set the output value immediately.
             */
            void setValue(ControllerValue value);
            
            /**
             * Return the output value, optionally shaped by the lookup translator.
             */
            ControllerValue getValue() const;
            
            /**
             * Return the output value bypassing the loopkup translator.
             */
            ControllerValue getRawValue() const { return mCurrentValue; }

            /**
             * Start ramping to @destination over a period of @time, using mode to indicate the type of ramp.
             */
            void ramp(ControllerValue destination, TimeValue time, RampMode mode = RampMode::Linear);
            
            /**
             * @return: wether the object is currently ramping to a new value.
             */
            bool isRamping() const { return mValue.isRamping(); }
            
            /**
             * Stops the current ramp (if any) and stays on the current value.
             */
            void stop();
            
            /**
             * Assign a translator to this node to shape the output value.
             */
            void setTranslator(SafePtr<Translator<ControllerValue>>& translator) { mTranslator = translator; }
            
            /**
             * @return: wether this node uses a translator lookup table to shape it's output values.
             */
            bool hasTranslator() const { return mTranslator != nullptr; }
            
            /**
             * Signal that is emitted when the destination of a ramp has been reached.
             */
            nap::Signal<ControlNode&> rampFinishedSignal;
            
        private:
            void process() override;
            void update();
            
            // Slot called internally when the destination of a ramp has been reached.
            nap::Slot<ControllerValue> mDestinationReachedSlot = { this, &ControlNode::destinationReached };
            void destinationReached(ControllerValue value) {
                mCurrentValue = value;
                rampFinishedSignal(*this);
            }
            
            std::atomic<ControllerValue> mNewDestination = { 0.f };
            std::atomic<int> mNewStepCount = { 0 };
            std::atomic<RampMode> mNewMode = { RampMode::Linear };
            DirtyFlag mIsDirty;
            
            RampedValue<ControllerValue> mValue = { 0.f }; // Current output value of the node.
            std::atomic<ControllerValue> mCurrentValue = { 0.f };
            SafePtr<Translator<ControllerValue>> mTranslator = nullptr; // Helper object to apply a translation to the output value.
        };
        
    }
    
}
