#include "FilterBank.h"

#include <cmath>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FilterBankNode)
    RTTI_PROPERTY("input", &nap::audio::FilterBankNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("output", &nap::audio::FilterBankNode::output, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setFilterCount", &nap::audio::FilterBankNode::setFilterCount)
    RTTI_FUNCTION("setParameters", &nap::audio::FilterBankNode::setParameters)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::audio::FilterBank)

namespace nap
{
    
    namespace audio
    {
        
        float8 makeFloat8(const std::vector<float>& list)
        {
            float8 result;
            for (auto i = 0; i < 8; ++i)
                result[i] = list[i % list.size()];
            return result;
        }
        
        
        void FilterBankNode::setFilterCount(unsigned int count)
        {
            if (count <= 8)
                mFilterCount = count;
            else
                mFilterCount = 8;
        }
		
		
        FilterBankNode::~FilterBankNode()
        {
			UpdateFunction * functionToDelete = nullptr;
            while (mDeletionQueue.try_dequeue(functionToDelete))
                delete functionToDelete;
		}
        
        
        void FilterBankNode::setParameters(const std::vector<ControllerValue>& aCenterFrequency, const std::vector<ControllerValue>& aBandWidth, const std::vector<ControllerValue>& aGain)
        {
            float8 centerFrequency = makeFloat8(aCenterFrequency);
            float8 bandWidth = makeFloat8(aBandWidth);
            float8 gain = makeFloat8(aGain);
            float8 scaledGain = gain * powVec(float8(10000.0) / bandWidth, float8(0.5));
            
            float8 sampleRate = float8(getNodeManager().getSampleRate());
            float8 zero = float8(0);
            float8 one = float8(1);
            float8 two = float8(2);

            float8 c = one / tanVec(float8(M_PI) * bandWidth / sampleRate);
            float8 d = two * cosVec(float8(2 * M_PI) * centerFrequency / sampleRate);
            float8 a0 = one / (one + c);
            float8 a1 = zero;
            float8 a2 = zero - a0;
            float8 b1 = a2 * c * d;
            float8 b2 = a0 * (c - one);
            auto biquadPtr = &mFilter;
            
            auto updateFunction = new std::function<void()>(
                [a0, a1, a2, b1, b2, scaledGain, biquadPtr](){
                  biquadPtr->setCoefficients(a0, a1, a2, b1, b2, scaledGain);
                }
            );
			
            updateFunction = mUpdateFunction.exchange(updateFunction);
			
            delete updateFunction;
            updateFunction = nullptr;

            UpdateFunction * functionToDelete = nullptr;
            while (mDeletionQueue.try_dequeue(functionToDelete))
                delete functionToDelete;
        }
        
        
        void FilterBankNode::process()
        {
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(output);
            auto filterCount = mFilterCount.load();
			
            UpdateFunction * updateFunction = mUpdateFunction.exchange(nullptr);
            if (updateFunction)
            {
                (*updateFunction)();
                mDeletionQueue.enqueue(updateFunction);
            }
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                const float8 inputValue(inputBuffer[i]);
                const float8 outputValue = mFilter.process(inputValue);
                float result = 0;
                for (auto filterIndex = 0; filterIndex < filterCount; ++filterIndex)
                    result += outputValue[filterIndex];
                outputBuffer[i] = result;
            }
        }
        
        
    }
    
}
