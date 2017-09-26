#include "audiopin.h"

// Std includes
#include <cassert>

// Audio includes
#include <node/audionode.h>

namespace nap {
    
    namespace audio {
        
        // --- InputPin --- //
        
        InputPin::~InputPin()
        {
            disconnectAll();
        }
        
        
        SampleBufferPtr InputPin::pull()
        {
            if (mInput)
                return mInput->pull();
            else
                return nullptr;
        }
        
        
        void InputPin::connect(OutputPin& input)
        {
            // disconnect any existing connection
            disconnectAll();
            
            // make the input and output point to one another
            mInput = &input;
            input.mOutputs.emplace(this);
        }
        
        
        void InputPin::disconnect(OutputPin& input)
        {
            if (&input == mInput)
                disconnectAll();
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
        
        
        std::vector<SampleBufferPtr> MultiInputPin::pull()
        {
            std::vector<SampleBufferPtr> result;
            for (auto& input : mInputs)
                result.emplace_back(input->pull());
            return result;
        }
        
        
        void MultiInputPin::connect(OutputPin& input)
        {
            mInputs.emplace(&input);
        }

        
        void MultiInputPin::disconnect(OutputPin& input)
        {
            for (auto& aInput : mInputs)
                if (aInput == &input)
                {
                    input.mOutputs.erase(this);
                    mInputs.erase(aInput);
                    break;
                }
        }
        
        
        void MultiInputPin::disconnectAll()
        {
            while (!mInputs.empty())
            {
                (*mInputs.begin())->mOutputs.erase(this);
                mInputs.erase(mInputs.begin());
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
        
        
        SampleBufferPtr OutputPin::pull()
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
