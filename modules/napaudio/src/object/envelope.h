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
            
            virtual bool init(NodeManager& nodeManager, utility::ErrorState& errorState)
            {
                mEnvelopeGenerator = std::make_unique<EnvelopeGenerator>(nodeManager);
                mEnvelopeGenerator->trigger(rtti_cast<Envelope>(&getResource())->mSegments);
                return true;
            }
            
            virtual OutputPin& getOutputForChannel(int channel) { return mEnvelopeGenerator->output; }
            virtual int getChannelCount() const { return 1; }
            
        private:
            std::unique_ptr<EnvelopeGenerator> mEnvelopeGenerator = nullptr;
        };
        
    }
    
}
