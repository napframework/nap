#pragma once

#include <audio/utility/delay.h>
#include <audio/core/audionode.h>
#include <audio/core/nodeobject.h>

#include <mathutils.h>


namespace nap {
    
    namespace audio {
        
        /**
         * A mono diffuse late reverberation algorithm based on FreeVerb
         * Written by Jerre van der Hulst.
         */
        class NAPAPI MonoTailModel
        {
        public:

            /**
             * CorrelationOffset: the classic way of getting stereo separation, now applied to spatial audio.
             * Should increase with steps of 23 per node.
             */
            MonoTailModel(size_t correlationOffset ) : mCorrelationOffset(correlationOffset ) {}
            ~MonoTailModel() = default;
            
            // Added by Casi to be able to use a MonoTailNode as a template type in ParallelNodeObject.
            void setCorrelationOffset(int correlationOffset) { mCorrelationOffset = correlationOffset; }
            
            float processSample( float f );
            
            void setDamping( float damping );
            void setFeedback( float feedback );
            
        private:
            struct Delay2048 : public Delay
            {
                Delay2048() : Delay( 2048 ) {}
            };
            
            // based on https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
            static constexpr size_t NumCombs = 8;
            std::array< Delay2048, NumCombs > mCombs;
            std::array< float, NumCombs > mCombLasts { 0.f };
            std::array< size_t, NumCombs > mCombDelayTimes { 1557, 1617, 1491, 1422, 1277, 1358, 1188, 1116 };
            float mCombFeedback = .84f;
            float mCombDamping = .2f;
            
            static constexpr size_t NumAllPasses = 4;
            std::array< Delay2048, NumAllPasses > mAllPasses;
            std::array< size_t, NumAllPasses> mAllPassDelayTimes { 225, 556, 441, 341 };
            float mAllPassGain = 0.5f;
            
            int mCorrelationOffset; // Casi: is it ok to not keep this constant?
        };
        
        
        /**
         * Node that wraps Jerre's MonoTail freeverb adaptation.
         */
        class NAPAPI MonoTailNode : public Node
        {
        public:
            MonoTailNode(NodeManager& manager, size_t correlationOffset = 0) : Node(manager), mMonoTail(correlationOffset) { }
            
            InputPin audioInput = { this };
            
            OutputPin audioOutput = { this };
            
            /**
             * Sets correlation offset. Should not be called after init.
             */
            void setCorrelationOffset(size_t correlationOffset)
            {
                mMonoTail.setCorrelationOffset(correlationOffset);
            }
            
            /**
             * Sets damping.
             */
            void setDamping(float damping)
            {
                mMonoTail.setDamping(damping);
            }
            
            /**
             * Sets feedback.
             */
            void setFeedback(float feedback)
            {
                mMonoTail.setFeedback(feedback);
            }


        private:
            void process() override 
            {
                auto& outputBuffer = getOutputBuffer(audioOutput);
                auto& inputBuffer = *audioInput.pull();
                outputBuffer = inputBuffer;
                
                for(int i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = mMonoTail.processSample(inputBuffer[i]);
                }
            }
            
            MonoTailModel mMonoTail;
        };


        class NAPAPI MonoTail : public ParallelNodeObject<MonoTailNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
        public:

            ResourcePtr<AudioObject> mInput = nullptr;
            float mFeedback = 0.8f;
            float mDamping = 0.0f;
            std::vector<int> mCorrelationOffset = { 0 };

        private:
            virtual bool initNode(int channel, MonoTailNode& node, utility::ErrorState& errorState) override;
        };

        
    }
    
}
