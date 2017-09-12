#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <device/audiointerface.h>
#include <resource/audiobufferresource.h>
#include <node/bufferplayer.h>

namespace nap {
    
    namespace audio {
    
        class PlayerComponentInstance;
        
        
        class NAPAPI PlayerComponent : public AudioComponent {
            RTTI_ENABLE(nap::audio::AudioComponent)
        public:
            PlayerComponent() : nap::audio::AudioComponent() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(PlayerComponentInstance);
            }
            
        public:
            // Properties
            /**
             * Pointer to the buffer file that will be played
             */
            ObjectPtr<AudioBufferResourceBase> mAudioBuffer;
            
        private:
        };

        
        class NAPAPI PlayerComponentInstance : public AudioComponentInstance {
            RTTI_ENABLE(nap::audio::AudioComponentInstance)
        public:
            PlayerComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
            OutputPin* getOutputForChannel(int channel) override { return &mPlayers[channel]->audioOutput; }
            int getChannelCount() const override { return mPlayers.size(); }
            
        private:
            std::vector<std::unique_ptr<BufferPlayer>> mPlayers;
        };
        

    }
}
