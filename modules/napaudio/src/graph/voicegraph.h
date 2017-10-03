#pragma once

#include <atomic>

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
            RTTI_ENABLE(GraphInstance)
            
        public:
            bool init(VoiceGraph& resource, utility::ErrorState& errorState);
            
            EnvelopeInstance& getEnvelope() { return *mEnvelope; }
            const EnvelopeInstance& getEnvelope() const { return *mEnvelope; }
            
            void setBusy(bool busy) { mBusy = busy; }
            bool isBusy() const { return mBusy; }

            void play(TimeValue duration = 0);
            void stop(TimeValue rampTime = 0);
            
            DiscreteTimeValue getStartTime() const { return mStartTime; }
            
            nap::Signal<VoiceGraphInstance&> finishedSignal;
            
        private:
            void envelopeFinished(EnvelopeGenerator&);
            
            EnvelopeInstance* mEnvelope = nullptr;
            std::atomic<bool> mBusy = { false };
            DiscreteTimeValue mStartTime = 0;
        };
        
        
        using VoiceGraphObjectCreator = rtti::ObjectCreator<VoiceGraph, NodeManager>;

        
    }
    
}
