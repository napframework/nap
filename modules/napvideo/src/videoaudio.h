#pragma once

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
            VideoNode(NodeManager& nodeManager, Video& video, int channelCount);

            // Inherited from Node
            int getChannelCount() const { return mOutputs.size(); }
            OutputPin& getOutput(int channel) { return *mOutputs[channel]; }
            
            /**
             * Change the source video object
             */
            void setVideo(Video& video) { mVideo = &video; }

			/** 
			 * We need to delete these so that the compiler doesn't try to use them. Otherwise we get compile errors on unique_ptr. Not sure why.
			 */
			VideoNode(const VideoNode&) = delete;
			VideoNode& operator=(const VideoNode&) = delete;
            
        private:
            // Inherited form Node
            void process() override final;
            
            std::vector<std::unique_ptr<OutputPin>> mOutputs; ///< The output pins for each individual channel of video audio
            rtti::ObjectPtr<Video> mVideo = nullptr; ///< Pointer to the source video object
            AudioFormat mAudioFormat; ///< This object tells the Video object what is our desired audio format
            std::vector<float> mDataBuffer; ///< The buffer that is passed to the Video object to be filled with audio data
        };
                
    }
    
}
