#pragma once

// Std includes
#include <mutex>

// Nap includes
#include <rtti/objectptr.h>

// Video includes
#include <video.h>

// Audio includes
#include <audio/core/audionode.h>

namespace nap {
    
    namespace audio {
        
        /**
         * An audio node that retrieves the audiochannels from a playing video object
         */
        class NAPAPI VideoNode : public Node {
        public:
            /**
             * Constructor takes the video object and the number of requested output channels.
             */
            VideoNode(NodeManager& nodeManager, int channelCount);

            // Inherited from Node
            int getChannelCount() const { return mOutputs.size(); }
            OutputPin& getOutput(int channel) { return *mOutputs[channel]; }
            
            /**
             * Change the source video object
             */
            void setVideo(Video& video);

        private:
            // Inherited form Node
            void process() override final;
            
            nap::Slot<Video&> mVideoDestructedSlot = { this, &nap::audio::VideoNode::videoDestructed };             ///< Slot to notify the Node when the Video resource it is pointing to is being destructed
            void videoDestructed(Video&);
            
            std::vector<std::unique_ptr<OutputPin>> mOutputs; ///< The output pins for each individual channel of video audio
            Video* mVideo = nullptr; ///< Pointer to the source video object
            AudioFormat mAudioFormat; ///< This object tells the Video object what is our desired audio format
            std::vector<float> mDataBuffer; ///< The buffer that is passed to the Video object to be filled with audio data
            std::mutex mVideoMutex; ///< Mutex to protect the video pointer to be onl accessed by one thread at a time
        };
                
    }
    
}
