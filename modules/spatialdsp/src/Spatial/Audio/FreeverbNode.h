#pragma once


// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/nodeobject.h>


#include <Spatial/Audio/faustfreeverb/freeverb.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Mono freeverb node.
         */
        class NAPAPI FreeverbNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            FreeverbNode(NodeManager& manager) : Node(manager) {
                model.init(manager.getSampleRate());
            }
            
            InputPin audioInput = { this };
            
            OutputPin audioOutput = { this };

            // params
            void setFeedback1(float value) { model.setFeedback1(value); }
            void setFeedback2(float value) { model.setFeedback2(value); }
            void setDamping(float value) { model.setDamping(value); }
            void setSpread(int value) { model.setSpread(value); }

        private:
            void process() override;
            
            FreeverbModel model;
        };


        class NAPAPI Freeverb : public ParallelNodeObject<FreeverbNode>
        {
        RTTI_ENABLE(ParallelNodeObjectBase)
        public:

            ResourcePtr<AudioObject> mInput = nullptr;
            float mFb1 = 0.8f;
            float mFb2 = 0.7f;
            float mDamping = 0.0f;
            std::vector<int> mSpread = { 0 };

        private:
            virtual bool initNode(int channel, FreeverbNode& node, utility::ErrorState& errorState) override;
        };

        
    }
}

