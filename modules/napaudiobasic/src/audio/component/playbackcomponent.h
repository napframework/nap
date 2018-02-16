#pragma once

// Nap includes

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/node/bufferplayernode.h>
#include <audio/node/gainnode.h>
#include <audio/node/controlnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PlaybackComponentInstance;
        
        
        class NAPAPI PlaybackComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(PlaybackComponent, PlaybackComponentInstance)
            
        public:
            PlaybackComponent() : AudioComponentBase() { }
            
            // Properties
            ObjectPtr<AudioBufferResource> mBuffer = nullptr;
            std::vector<int> mChannelRouting = { 0 };
            bool mAutoPlay = true;
            TimeValue mStartPosition = 0;
            TimeValue mFadeIn = 0;
            TimeValue mFadeOut = 0;
            ControllerValue mPitch = 1.0;
            ControllerValue mGain = 1.0;
            ControllerValue mStereoPanning = 0.5;
            
            bool isStereo() const { return mChannelRouting.size() == 2; }
            
        private:
        };

        
        class NAPAPI PlaybackComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            PlaybackComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            int getChannelCount() const override { return mGainNodes.size(); }
            OutputPin& getOutputForChannel(int channel) override { return mGainNodes[channel]->audioOutput; }
            
            bool isStereo() const { return mGainNodes.size() == 2; }
            void start();
            void stop();
            
        private:
            void applyGain();
            
            std::vector<std::unique_ptr<BufferPlayerNode>> mBufferPlayers;
            std::vector<std::unique_ptr<GainNode>> mGainNodes;
            std::vector<std::unique_ptr<ControlNode>> mGainControls;
            
            ControllerValue mGain = 0;
            ControllerValue mPanning = 0.5;
            TimeValue mStartPosition = 0;
            TimeValue mFadeIn = 0;
            TimeValue mFadeOut = 0;
            ControllerValue mPitch = 1.0;
            
            bool mStopping = false;

            PlaybackComponent* resource = nullptr;
            NodeManager* nodeManager = nullptr;
        };
        
    }
    
}
