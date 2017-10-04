#pragma once

// Audio includes
#include <graph/audioobject.h>
#include <graph/voicegraph.h>
#include <node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PolyphonicObjectInstance;
        
        
        class NAPAPI PolyphonicObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            PolyphonicObject() : AudioObject() { }
            
            ObjectPtr<VoiceGraph> mGraph;
            int mVoiceCount = 1;
            bool mVoiceStealing = true;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };

        
        class NAPAPI PolyphonicObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            PolyphonicObjectInstance(PolyphonicObject& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the component
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            VoiceGraphInstance* findFreeVoice();
            void play(VoiceGraphInstance* voice);
            void stop(VoiceGraphInstance* voice);
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            void voiceFinished(VoiceGraphInstance& voice);
            
            std::vector<std::unique_ptr<VoiceGraphInstance>> mVoices;
            std::vector<std::unique_ptr<MixNode>> mMixNodes;
            
            NodeManager* mNodeManager = nullptr;
        };
        
    }
    
}
