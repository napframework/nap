#pragma once


// Audio includes
#include <core/audionode.h>
#include <node/delay.h>

namespace nap {
    
    namespace audio {
    
        class NAPAPI DelayNode : public Node {
        public:
            DelayNode(NodeManager& manager) : Node(manager), mDelay(65536 * 8) { }
            
            InputPin input;
            OutputPin output = { this };
            
            void setTime(int value) { mTime = value; }
            void setDryWet(ControllerValue value) { mDryWet = value; }
            void setFeedback(ControllerValue value) { mFeedback = value; }
            
            int getTime() const { return mTime; }
            ControllerValue getDryWet() const { return mDryWet; }
            ControllerValue getFeedback() const { return mFeedback; }
            
        private:
            void process() override;
            
            Delay mDelay;
            int mTime = 0;
            ControllerValue mDryWet = 0.5f;
            ControllerValue mFeedback = 0.f;
        };
        
    }
    
}
