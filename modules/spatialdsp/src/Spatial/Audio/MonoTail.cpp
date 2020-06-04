#include "MonoTail.h"

// from freeverb
// http://www.dreampoint.co.uk
#define freeverb_undenormalise(sample) if(((*(unsigned int*)&sample)&0x7f800000)==0) sample=0.0f

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MonoTailNode)
    RTTI_PROPERTY("audioInput", &nap::audio::MonoTailNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::MonoTailNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::audio::MonoTail)
        RTTI_PROPERTY("Input", &nap::audio::MonoTail::mInput, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Feedback", &nap::audio::MonoTail::mFeedback, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Damping", &nap::audio::MonoTail::mDamping, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("CorrelationOffset", &nap::audio::MonoTail::mCorrelationOffset, nap::rtti::EPropertyMetaData::Default)

RTTI_END_CLASS

namespace nap
{
    namespace audio
    {
        
        float MonoTailModel::processSample(float f )
        {
            const float inCopy = f;
            
            // parallel combs
            float combCollector = 0.f;
            for( size_t c = 0; c < NumCombs; c++ )
            {
                const float output = mCombs[c].read(mCombDelayTimes[ c ] + mCorrelationOffset);
                mCombLasts[c] = output * ( 1.f - mCombDamping ) + mCombLasts[c] * mCombDamping;
                freeverb_undenormalise(mCombLasts[c]);
                mCombs[c].write(inCopy + mCombLasts[c] * mCombFeedback);
                combCollector += output;
            }
            
            // serial allpasses
            float currentAllPassInput = combCollector;
            float nextAllpassInput;
            for( size_t a = 0; a < NumAllPasses; a++ )
            {
                const float hist = mAllPasses[a].read( mAllPassDelayTimes[a] + mCorrelationOffset);
                nextAllpassInput = -currentAllPassInput + hist;
                auto newHist = hist * mAllPassGain;
                freeverb_undenormalise( newHist );
                mAllPasses[ a ].write( currentAllPassInput + newHist );
                currentAllPassInput = nextAllpassInput;
            }
            
            return currentAllPassInput;
        }
        
        void MonoTailModel::setFeedback(float feedback)
        {
            assert(feedback >= 0.f);
            mCombFeedback =  math::min(feedback, 0.999f);
        }
        
        void MonoTailModel::setDamping(float damping)
        {
            mCombDamping = damping;
        }


        bool MonoTail::initNode(int channel, MonoTailNode& node, utility::ErrorState& errorState)
        {
            if (mInput != nullptr)
                (*node.getInputs().begin())->connect(*mInput->getInstance()->getOutputForChannel(0));
            node.setFeedback(mFeedback);
            node.setDamping(mDamping);
            node.setCorrelationOffset(mCorrelationOffset[channel % mCorrelationOffset.size()]);
            return true;
        }

        
    }
    
}
