#include "videoaudio.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap {
    
    namespace audio {
        
        VideoNode::VideoNode(NodeManager& nodeManager, Video& video, int channelCount) : Node(nodeManager), mVideo(&video), mAudioFormat(channelCount, AudioFormat::ESampleFormat::FLT, int(nodeManager.getSampleRate()))
        {
            // Initialize the output pins
            for (auto channel = 0; channel < channelCount; ++channel)
                mOutputs.emplace_back(std::make_unique<OutputPin>(this));
            
            // Initialize the buffer to be filled by the video object
            mDataBuffer.resize(getBufferSize() * getChannelCount());
        }
        
        
        void VideoNode::process()
        {
            if (mVideo == nullptr || !mVideo->hasAudio())
            {
                // If the video has no audio channels we fill the output pins with zeros
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                {
                    auto& channelBuffer = getOutputBuffer(*mOutputs[channel]);
                    for (auto i = 0; i < getBufferSize(); ++i)
                        channelBuffer[i] = 0;
                }
                return;
            }
            
            // We tell the Video object to fill our buffer with audio data
            mVideo->OnAudioCallback((uint8_t*)(mDataBuffer.data()), mDataBuffer.size() * sizeof(float), mAudioFormat);
            
            // Deinterleaving the data for our output pins
            float* samplePtr = mDataBuffer.data();
            for (auto i = 0; i < getBufferSize(); ++i)
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    getOutputBuffer(*mOutputs[channel])[i] = *(samplePtr++);

        }

        
    }
    
}
