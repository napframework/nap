#pragma once

// nap includes
#include <nap/component.h>

// audio includes
#include "audionode.h"
#include "audionodemanager.h"
#include "audiodevice.h"
#include "nodes/bufferplayer.h"

namespace nap {
    
    namespace audio {
        
        // Forward declarations
        class AudioPlayerComponentInstance;
        
        
        class NAPAPI AudioPlayerComponent : public nap::Component {
            RTTI_ENABLE(nap::Component)
            
        public:
            AudioPlayerComponent() : nap::Component() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(AudioPlayerComponentInstance);
            }
            
        public:
            // Properties
            ObjectPtr<AudioInterface> mAudioInterface;
            std::string mAudioFilePath = "";
        };
        
        
        class NAPAPI AudioPlayerComponentInstance : public nap::ComponentInstance {          
            RTTI_ENABLE(nap::ComponentInstance)
            
        public:
            AudioPlayerComponentInstance(EntityInstance& entity) : nap::ComponentInstance(entity) { }
            
            // Initialize the component
            bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            // Audio buffer
            nap::audio::MultiSampleBuffer audioFileBuffer;
            float fileSampleRate;
            
            // One player and one output for every channel of the audio file
            std::vector<std::unique_ptr<BufferPlayer>> mPlayers;
            std::vector<std::unique_ptr<AudioOutputNode>> mOutputs;
        };
        
    }
    
}
