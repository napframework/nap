#include "DistanceDampingEffect.h"

RTTI_DEFINE_CLASS(nap::spatial::DistanceDampingEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistanceDampingEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistanceDampingEffectInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        bool DistanceDampingEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            mDistanceDampingThreshold = getParameterManager().addParameterFloat("threshold", 100.0, 0.001, 1000.0);
            mDistanceDampingCurvature = getParameterManager().addParameterFloat("curve", -0.4, -1.0, 1.0);
            
            recalculateOnChange(mDistanceDampingThreshold);
            recalculateOnChange(mDistanceDampingCurvature);
            recalculateParticleOnDataChange();

            return true;
        }
        
        
        void DistanceDampingEffectInstance::recalculate(int processorIndex, int particleIndex){
            
            // threshold
            float threshold = mDistanceDampingThreshold->mValue;
            
            // curvature
            float curvature = mDistanceDampingCurvature->mValue;
            
            // distance
            float distance = getParticleMeasurer(particleIndex)->getDistanceToVantagePoint();
            
            // 4D curve formula
            float distanceDampingValue = math::clamp<float>(distance / threshold, 0., 1.);
            
            float cutoffValue = Functions::distanceCurve(distanceDampingValue, curvature, false) * 19920 + 80;
            
            // find filter chain and set frequency
            auto filterChain = getProcessor(processorIndex)->getDSP(particleIndex);

            filterChain->setFrequency(cutoffValue);
            
        }
        
        
    }
}

