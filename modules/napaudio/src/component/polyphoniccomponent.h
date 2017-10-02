#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/objectptr.h>

// Audio includes
#include <component/audiocomponent.h>
#include <graph/voicegraph.h>
#include <node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PolyphonicComponentInstance;
        
        
        class NAPAPI PolyphonicComponent : public AudioComponent
        {
            RTTI_ENABLE(AudioComponent)
            DECLARE_COMPONENT(PolyphonicComponent, PolyphonicComponentInstance)
            
        public:
            PolyphonicComponent() : AudioComponent() { }
            
            ObjectPtr<VoiceGraph> mGraph;
            int mVoiceCount = 1;
            bool mVoiceStealing = true;
            
        private:
        };

        
        class NAPAPI PolyphonicComponentInstance : public AudioComponentInstance
        {
            RTTI_ENABLE(AudioComponentInstance)
        public:
            PolyphonicComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            VoiceGraphInstance* findFreeVoice();
            void play(VoiceGraphInstance* voice);
            void stop(VoiceGraphInstance* voice);
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            void voiceFinished(VoiceGraphInstance& voice);
            
            std::vector<std::unique_ptr<VoiceGraphInstance>> mVoices;
            std::vector<std::unique_ptr<MixNode>> mMixNodes;
        };
        
    }
    
}
