#include "videoaudio.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap {
    
    namespace audio {
        
        VideoNode::VideoNode(NodeManager& nodeManager, int channelCount, bool decodeAudio) : 
			Node(nodeManager), 
			mAudioFormat(channelCount, AudioFormat::ESampleFormat::FLT, int(nodeManager.getSampleRate())),
			mDecodeAudio(decodeAudio)
        {
            // Initialize the output pins
            for (auto channel = 0; channel < channelCount; ++channel)
                mOutputs.emplace_back(std::make_unique<OutputPin>(this));
            
            // Initialize the buffer to be filled by the video object
            mDataBuffer.resize(getBufferSize() * getChannelCount());
        }
        
        
        void VideoNode::process()
        {
            std::lock_guard<std::mutex> lock(mVideoMutex);
            if (mVideo == nullptr || !mVideo->audioEnabled() || !mVideo->isPlaying())
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
            if (mVideo->OnAudioCallback((uint8_t*)(mDataBuffer.data()), mDataBuffer.size() * sizeof(float), mAudioFormat))            
			{
				// Deinterleaving the data for our output pins
				float* samplePtr = mDataBuffer.data();
				for (auto i = 0; i < getBufferSize(); ++i)
					for (auto channel = 0; channel < getChannelCount(); ++channel)
						getOutputBuffer(*mOutputs[channel])[i] = *(samplePtr++);
			}
        }
        
        
        void VideoNode::setVideo(Video& video)
        {
            std::lock_guard<std::mutex> lock(mVideoMutex);
            // unregister from the old video's destruct signal
			if (mVideo != nullptr)
				mVideo->mDestructedSignal.disconnect(mVideoDestructedSlot);
            
			// Update video reference
            mVideo = &video;
            
            // connect to the new video's destruct signal and enable / disable audio decoding.
			if (mVideo != nullptr)
			{
				mVideo->mDestructedSignal.connect(mVideoDestructedSlot);
				mVideo->decodeAudioStream(mDecodeAudio);
			}
        }
        
        
        void VideoNode::videoDestructed(Video& video)
        {
            std::lock_guard<std::mutex> lock(mVideoMutex);
            if (mVideo == &video)
                mVideo = nullptr;
        }
    }
}
