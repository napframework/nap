#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <resource/audiobufferresource.h>
#include <node/bufferplayer.h>

namespace nap {
    
    namespace audio {
    
        class PlayerComponentInstance;
        
        
        /**
         * Component that plays back an audio buffer
         */
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
            
            ObjectPtr<AudioBufferResourceBase> mAudioBuffer; /**< Pointer to the buffer file that will be played */
            
            /**
             * The size of this array represents the number of playback channels that this component outputs.
             * The value of each element represents the channel of the source buffer that will be played back on this channel.
             */
            std::vector<int> mChannelsToPlay = { 0 };
            
            float mSpeed = 1.0; /**< Playback speed. 1.0 = original speed, 2.0 is double speed. */
            
        private:
        };

        
        /**
         * Instance of component that plays back an audio buffer
         */
        class NAPAPI PlayerComponentInstance : public AudioComponentInstance {
            RTTI_ENABLE(nap::audio::AudioComponentInstance)
        public:
            PlayerComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
            OutputPin& getOutputForChannel(int channel) override final { return mPlayers[channel]->audioOutput; }
            int getChannelCount() const override final { return mPlayers.size(); }
            
        private:
            std::vector<std::unique_ptr<BufferPlayer>> mPlayers;
        };
        

    }
}
