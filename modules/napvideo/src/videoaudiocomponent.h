#pragma once

// Video includes
#include <video.h>
#include <videoaudio.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>

namespace nap
{
    
    namespace audio
    {
    
        class VideoAudioComponentInstance;
        
        
        class NAPAPI VideoAudioComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(VideoAudioComponent, VideoAudioComponentInstance)
            
        public:
            VideoAudioComponent() : AudioComponentBase() { }
            
            // Properties
            rtti::ObjectPtr<Video> mVideo = nullptr;
            int mChannelCount = 2;
        };

        
        class NAPAPI VideoAudioComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            VideoAudioComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            int getChannelCount() const override { return mNode->getChannelCount(); }
            OutputPin& getOutputForChannel(int channel) override { return mNode->getOutput(channel); }
            
            void setVideo(Video& video);
            
        private:
            std::unique_ptr<VideoNode> mNode = nullptr;
        };
        
    }
    
}
