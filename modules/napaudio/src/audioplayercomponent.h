#pragma once

// nap includes
#include <nap/component.h>

// audio includes
#include "audionode.h"
#include "audionodemanager.h"
#include "audiointerface.h"
#include "audiofileresource.h"
#include "nodes/bufferplayer.h"
#include "nodes/gain.h"
#include "nodes/stereopanner.h"

namespace nap {
    
    namespace audio {
        
        // Forward declarations
        class AudioPlayerComponentInstance;
        
        /**
         * A component that plays back a mono or stereo audio file on first 2 channels (stereo) of the system.
         */
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
            /**
             * Pointer to the audio interface the audio file will be played back on
             */
            ObjectPtr<AudioInterface> mAudioInterface;
            
            /**
             * Pointer to the audio file that will be played
             */
            ObjectPtr<AudioFileResource> mAudioFile;
            
            /**
             * Gain of the playback between 0 and 1.
             */
            float mGain = 1.f;
            
            /**
             * Panning of the playback, 0 is left 1 is right.
             */
            float mPanning = 0.5f;
        };
        
        
        class NAPAPI AudioPlayerComponentInstance : public nap::ComponentInstance {          
            RTTI_ENABLE(nap::ComponentInstance)
            
        public:
            AudioPlayerComponentInstance(EntityInstance& entity) : nap::ComponentInstance(entity) { }
            
            // Initialize the component
            bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            std::vector<std::unique_ptr<BufferPlayer>> mPlayers;
            std::vector<std::unique_ptr<Gain>> mGains;
            std::unique_ptr<StereoPanner> mPanner = nullptr;
            std::vector<std::unique_ptr<AudioOutputNode>> mOutputs;
        };
        
    }
    
}
