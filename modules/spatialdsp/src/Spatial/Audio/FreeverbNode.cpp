//
//  FreeverbNode.cpp
//  theworks
//
//  Created by Casimir Geelhoed on 19/02/2019.
//
//

#include "FreeverbNode.h"


RTTI_BEGIN_CLASS(nap::audio::Freeverb)
    RTTI_PROPERTY("Input", &nap::audio::Freeverb::mInput, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Fb1", &nap::audio::Freeverb::mFb1, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Fb2", &nap::audio::Freeverb::mFb2, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Damping", &nap::audio::Freeverb::mDamping, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Spread", &nap::audio::Freeverb::mSpread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FreeverbNode)
    RTTI_PROPERTY("audioInput", &nap::audio::FreeverbNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::FreeverbNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::audio::Freeverb)


namespace nap {
    
    namespace audio {
        
        void FreeverbNode::process(){
            
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(audioOutput);

            // convert vector to float array by pointing to first element (why are buffers vectors?..)
            float* inputArray = &inputBuffer[0];
            float* outputArray = &outputBuffer[0];

            model.compute(getBufferSize(), &inputArray, &outputArray);
        }


        bool Freeverb::initNode(int channel, FreeverbNode& node, utility::ErrorState& errorState)
        {
            if (mInput != nullptr)
                (*node.getInputs().begin())->connect(*mInput->getInstance()->getOutputForChannel(0));
            node.setFeedback1(mFb1);
            node.setFeedback2(mFb2);
            node.setDamping(mDamping);
            node.setSpread(mSpread[channel % mSpread.size()]);
            return true;
        }

        
    }
    
}
