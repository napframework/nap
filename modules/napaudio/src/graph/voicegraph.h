#pragma once

#include <graph/audiograph.h>
#include <object/envelope.h>

namespace nap {
    
    namespace audio {
        
        class VoiceGraph : public Graph
        {
            RTTI_ENABLE(Graph)
            
        public:
            VoiceGraph() = default;
            VoiceGraph(NodeManager& nodeManager) : Graph(nodeManager)  { }
            
            ObjectPtr<Envelope> mEnvelope;
            
        private:
        };
        
        
        class VoiceGraphInstance : public GraphInstance
        {
        public:
            bool init(VoiceGraph& resource, utility::ErrorState& errorState);
            
            EnvelopeInstance& getEnvelope() { return *mEnvelope; }
            const EnvelopeInstance& getEnvelope() const { return *mEnvelope; }
            
        private:
            EnvelopeInstance* mEnvelope = nullptr;
            bool mBusy = false;
        };
        
    }
    
}
