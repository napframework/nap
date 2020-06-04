#include "GainScalingEffect.h"

// Std includes
#include <math.h>

RTTI_DEFINE_CLASS(nap::spatial::GainScalingEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GainScalingEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GainScalingEffectInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        bool GainScalingEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            mAmount = getParameterManager().addParameterFloat("amount", 0.5, 0.0, 1.0);
            mAmount->connect([&](float x){ recalculateGain(); recalculateAllParticles(); });
            
            return true;
        }
        
        
        void GainScalingEffectInstance::recalculateGain()
        {
            mGain = std::powf(1.f / float(mParticleCount), mAmount->mValue);

        }
        
        
        void GainScalingEffectInstance::update(double deltaTime)
        {
            int particleCount = getActiveParticleCount();
            if(particleCount != mParticleCount){
                mParticleCount = particleCount >= 1 ? particleCount : 1;
                recalculateGain();
                recalculateAllParticles();
            }
        }

        
        void GainScalingEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
            getProcessor(processorIndex)->getNode(particleIndex)->setGain(mGain);
        }

    }
}
