
#include "DelayEffect.h"

RTTI_BEGIN_CLASS(nap::spatial::DelayEffect)
    RTTI_PROPERTY("BufferSize", &nap::spatial::DelayEffect::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DelayEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DelayEffectInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        bool DelayEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            int delayBufferSize = 65536;
            auto* resource = getResource<DelayEffect>();
            if(resource != nullptr)
                delayBufferSize = resource->mBufferSize;

            int sampleRate = getAudioService().getNodeManager().getSampleRate();
            
            float maxDelayTime = (delayBufferSize / (float)sampleRate) * 1000.0;
            mDelayTime = getParameterManager().addParameterFloat("time", maxDelayTime / 4., 0.0, maxDelayTime);
            recalculateOnChange(mDelayTime);
            
            return true;
        }
        
        void DelayEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
            getProcessor(processorIndex)->getNode(particleIndex)->setTime(mDelayTime->mValue, 25.f);
        }

    }
}
