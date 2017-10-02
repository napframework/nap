#pragma once

// RTTI includes
#include <rtti/rtti.h>

// Audio includes
#include <utility/linearramper.h>
#include <utility/exponentialramper.h>
#include <utility/translator.h>
#include <node/audionode.h>
#include <node/audionodemanager.h>

namespace nap {
    
    namespace audio {
        
        class NAPAPI ControlNode : public Node {
            RTTI_ENABLE(Node)
            
        public:
            enum class RampMode { LINEAR, EXPONENTIAL };
            
        public:
            ControlNode(NodeManager& manager);
            
            OutputPin output = { this };
            
            void setValue(ControllerValue value);
            ControllerValue getValue() const;
            ControllerValue getRawValue() const { return mValue; }

            void ramp(ControllerValue destination, TimeValue time, RampMode mode = RampMode::LINEAR);
            bool isRamping() const;
            void stop();
            
            void setTranslator(Translator<ControllerValue>& translator) { mTranslator = &translator; }
            bool hasTranslator() const { return mTranslator != nullptr; }
            
            nap::Signal<ControlNode&> rampFinishedSignal;
            
        private:
            void process() override;
            
            nap::Slot<ControllerValue> mDestinationReachedSlot = { this, &ControlNode::destinationReached };
            void destinationReached(ControllerValue value) { rampFinishedSignal(*this); }
            
            ControllerValue mValue = 0;
            LinearRamper<ControllerValue> mLinearRamper = { mValue };
            ExponentialRamper<ControllerValue> mExponentialRamper = { mValue };
            Translator<ControllerValue>* mTranslator = nullptr;
        };
        
    }
    
}
