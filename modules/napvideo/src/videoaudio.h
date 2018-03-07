#pragma once

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
            VideoNode(NodeManager& nodeManager, Video& video, int channelCount);
            
            // Inherited from Node
            int getChannelCount() const { return mOutputs.size(); }
            OutputPin& getOutput(int channel) { return *mOutputs[channel]; }
            
            /**
             * Change the source video object
             */
            void setVideo(Video& video) { mVideo = &video; }
            
        private:
            // Inherited form Node
            void process() override final;
            
            std::vector<std::unique_ptr<OutputPin>> mOutputs; ///< The output pins for each individual channel of video audio
            Video* mVideo = nullptr; ///< Pointer to the source video object
            AudioFormat mAudioFormat; ///< This object tells the Video object what is our desired audio format
            std::vector<float> mDataBuffer; ///< The buffer that is passed to the Video object to be filled with audio data
        };
                
    }
    
}
