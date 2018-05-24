#include "audiopin.h"

// Std includes
#include <cassert>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        // --- InputPin --- //
        
        InputPin::~InputPin()
        {
            disconnectAll();
        }
        
        
        SampleBuffer* InputPin::pull()
        {
            if (mInput)
                return mInput->pull();
            else
                return nullptr;
        }
        
        
        void InputPin::connect(OutputPin& input)
        {
            // remove old connection
            if (mInput)
                mInput->mOutputs.erase(this);

            // make the input and output point to one another
            mInput = &input;
            mInput->mOutputs.emplace(this);
        }
        
        
        void InputPin::disconnect(OutputPin& input)
        {
            if (&input == mInput)
            {
                mInput->mOutputs.erase(this);
                mInput = nullptr;
            }
        }

        
        void InputPin::disconnectAll()
        {
            if (mInput)
            {
                mInput->mOutputs.erase(this);
                mInput = nullptr;
            }
        }
        
        
        // --- MultiInputPin ---- //
        
        
        MultiInputPin::~MultiInputPin()
        {
            disconnectAll();
        }
        
        
        std::vector<SampleBuffer*> MultiInputPin::pull()
        {
            std::vector<SampleBuffer*> result;
            
            auto inputs = mInputs; // we make a copy of mInputs because its contents can be changed while traversing the loop!
            for (auto& input : inputs)
                result.emplace_back(input->pull());
            
            return result;
        }
        
        
        void MultiInputPin::connect(OutputPin& input)
        {
            mInputs.emplace(&input);
            input.mOutputs.emplace(this);
        }

        
        void MultiInputPin::disconnect(OutputPin& input)
        {
            mInputs.erase(&input);
            input.mOutputs.erase(this);
        }
        
        
        void MultiInputPin::disconnectAll()
        {
            while (!mInputs.empty())
            {
                auto input = *mInputs.begin();
                input->mOutputs.erase(this);
                mInputs.erase(input);
            }
        }
        
        
        // --- OutputPin --- //
        
        
        OutputPin::OutputPin(Node* node)
        {
            node->mOutputs.emplace(this);
            mNode = node;
            setBufferSize(mNode->getBufferSize());
        }
        
        
        OutputPin::~OutputPin()
        {
            mNode->mOutputs.erase(this);
            disconnectAll();
        }
        
        
        void OutputPin::disconnectAll()
        {
            while (!mOutputs.empty())
                (*mOutputs.begin())->disconnect(*this);
        }
        
        
        SampleBuffer* OutputPin::pull()
        {
            mNode->update();
            return &mBuffer;
        }
        
        
        void OutputPin::setBufferSize(int bufferSize)
        {
            mBuffer.resize(bufferSize);
        }
        
    }
    
}
