#pragma once

// Video includes
#include <video.h>

// Audio includes
#include <audio/core/audionode.h>

namespace nap {
    
    namespace audio {
        
        class NAPAPI VideoNode : public Node {
        public:
            VideoNode(NodeManager& nodeManager, Video& video, int channelCount);
            
            int getChannelCount() const { return mOutputs.size(); }
            OutputPin& getOutput(int channel) { return *mOutputs[channel]; }
            
            void setVideo(Video& video) { mVideo = &video; }
            
        private:
            void process() override final;
            
            std::vector<std::unique_ptr<OutputPin>> mOutputs;
            Video* mVideo = nullptr;
            AudioFormat mAudioFormat;
            std::vector<float> mDataBuffer;
        };
                
    }
    
}
