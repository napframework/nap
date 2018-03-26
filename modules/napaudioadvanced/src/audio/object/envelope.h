#pragma once

// Nap includes
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/envelopenode.h>

namespace nap
{
    
    namespace audio
    {
        
        class EnvelopeInstance;
        
        /**
         * Audio object resource for an envelope generator.
         */
        class Envelope : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            Envelope() = default;
            
            EnvelopeGenerator::Envelope mSegments; ///< The segments that define the envelope's shape.
            
            bool mAutoTrigger = false; ///< If true the envelope will be triggered automatically on initialization
            
        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance();
        };
        
        
        
        /**
         * Instance of an envelope generator.
         */
        class EnvelopeInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            EnvelopeInstance(Envelope& resource) : AudioObjectInstance(resource) { }
            
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override
            {
                mEnvelopeGenerator = make_node<EnvelopeGenerator>(nodeManager);
                auto resource = rtti_cast<Envelope>(&getResource());
                if (resource->mAutoTrigger)
                    mEnvelopeGenerator->trigger(resource->mSegments);
                return true;
            }                        
            
            OutputPin& getOutputForChannel(int channel) override { return mEnvelopeGenerator->output; }
            int getChannelCount() const override { return 1; }
            
            void trigger(TimeValue totalDuration = 0)
            {
                auto& envelope = rtti_cast<Envelope>(&getResource())->mSegments;
                mEnvelopeGenerator->trigger(envelope, totalDuration);
            }
            
            void stop(TimeValue rampTime) { mEnvelopeGenerator->stop(rampTime); }
            
            ControllerValue getValue() const { return mEnvelopeGenerator->getValue(); }
            
            nap::Signal<EnvelopeGenerator&>& getEnvelopeFinishedSignal() { return mEnvelopeGenerator->envelopeFinishedSignal; }
            
        private:
            NodePtr<EnvelopeGenerator> mEnvelopeGenerator = nullptr;
        };
        
    }
    
}
