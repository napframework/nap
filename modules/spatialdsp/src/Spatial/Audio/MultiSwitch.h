#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <rtti/rtti.h>
#include <mathutils.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/audiotypes.h>
#include <audio/core/nodeobject.h>

namespace nap
{
    
    namespace audio
    {
        /**
         * DOC: Stijn.
         */
        class NAPAPI MultiSwitchNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            MultiSwitchNode(NodeManager& manager, unsigned int inputCount = 2) : Node(manager)
            {
                assert(inputCount > 0);
                for (auto i = 0; i < inputCount; ++i)
                    mInputs.emplace_back(std::make_unique<InputPin>(this));
            }

            OutputPin audioOutput = { this };
            
            void select(unsigned int selection)
            {
                if (selection < mInputs.size())
                    mSelection.store(selection);
            }
            
            void connect(unsigned int index, OutputPin* input)
            {
                if (index < mInputs.size())
                    mInputs[index]->connect(*input);
            }
            
            int getInputCount() const { return mInputs.size(); }
            
        private:
            void process() override
            {
                auto& outputBuffer = getOutputBuffer(audioOutput);
                auto selection = mSelection.load();
                auto inputBuffer = mInputs[selection]->pull();
                
                for (auto i = 0; i < outputBuffer.size(); ++i)
                    outputBuffer[i] = (*inputBuffer)[i];
            }
            
            std::vector<std::unique_ptr<InputPin>> mInputs;
            std::atomic<int> mSelection = { 0 };
        };
        
        
        
        class NAPAPI MultiSwitch : public ParallelNodeObject<MultiSwitchNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            std::vector<ResourcePtr<AudioObject>> mInputs;
            std::vector<int> mSelection = { 0 };
            
        private:
            virtual bool initNode(int channel, MultiSwitchNode& node, utility::ErrorState& errorState) override
            {
                for (auto i = 0; i < mInputs.size(); ++i)
                    node.connect(i, mInputs[i]->getInstance()->getOutputForChannel(channel % mInputs[i]->getInstance()->getChannelCount()));
                node.select(mSelection[channel % mSelection.size()]);

                return true;
            }

        };
        
                
    }
    
}
