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
            
            mDataBuffer.resize(getBufferSize() * getChannelCount());
        }
        
        
        void VideoNode::process()
        {
            if (!mVideo->hasAudio())
            {
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                {
                    auto& channelBuffer = getOutputBuffer(*mOutputs[channel]);
                    for (auto i = 0; i < getBufferSize(); ++i)
                        channelBuffer[i] = 0;
                }
                return;
            }
            
            mVideo->OnAudioCallback((uint8_t*)(mDataBuffer.data()), mDataBuffer.size() * sizeof(float), mAudioFormat);
            
            float* samplePtr = mDataBuffer.data();
            for (auto i = 0; i < getBufferSize(); ++i)
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                    getOutputBuffer(*mOutputs[channel])[i] = *(samplePtr++);

        }

        
    }
    
}
