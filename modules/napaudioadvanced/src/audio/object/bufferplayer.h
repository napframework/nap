#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * AudioObject to play back audio contained by an AudioBufferResource.
         */
        class BufferPlayer : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            BufferPlayer() = default;
            
            int mChannelCount = 1; /**< Number of channels that will be played back from the source buffer */
            ResourcePtr<AudioBufferResource> mBufferResource = nullptr; /**< Resource containing the buffer that will be played. */
            bool mAutoPlay = true; /**<  If true, the object will start playing back immediately after initialization. */
            
        private:
            // Inherited from MultiChannelObject
            SafeOwner<Node> createNode(int channel, AudioService& audioService) override;
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

