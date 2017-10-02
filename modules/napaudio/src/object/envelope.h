#pragma once

// Nap includes
#include <nap/objectptr.h>

// Audio includes
#include <graph/audioobject.h>
#include <node/envelopenode.h>

namespace nap {
    
    namespace audio {
        
        class EnvelopeInstance;
        
        class Envelope : public AudioObject {
            RTTI_ENABLE(AudioObject)
        public:
            Envelope() = default;
            
            EnvelopeGenerator::Envelope mSegments;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance();
        };
        
        
        class EnvelopeInstance : public AudioObjectInstance {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            EnvelopeInstance(Envelope& resource) : AudioObjectInstance(resource) { }
            
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override
            {
                mEnvelopeGenerator = std::make_unique<EnvelopeGenerator>(nodeManager);
                mEnvelopeGenerator->trigger(rtti_cast<Envelope>(&getResource())->mSegments);
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
            
            nap::Signal<EnvelopeGenerator&>& getEnvelopeFinishedSignal() { return mEnvelopeGenerator->envelopeFinishedSignal; }
            
        private:
            std::unique_ptr<EnvelopeGenerator> mEnvelopeGenerator = nullptr;
        };
        
    }
    
}
