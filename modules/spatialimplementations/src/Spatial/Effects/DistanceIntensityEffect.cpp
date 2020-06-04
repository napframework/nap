#include "DistanceIntensityEffect.h"

RTTI_DEFINE_CLASS(nap::spatial::DistanceIntensityEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistanceIntensityEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistanceIntensityEffectInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace spatial
    {
        
        
        bool DistanceIntensityEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            mDistanceIntensityThreshold = getParameterManager().addParameterFloat("threshold", 100.0, 0.001, 1000.0);
            mDistanceIntensityCurvature = getParameterManager().addParameterFloat("curve", -0.4, -1.0, 1.0);
            
            recalculateOnChange(mDistanceIntensityThreshold);
            recalculateOnChange(mDistanceIntensityCurvature);
            recalculateParticleOnDataChange();

            return true;
        }
        
        
        void DistanceIntensityEffectInstance::recalculate(int processorIndex, int particleIndex){
            
            // threshold
            float threshold = mDistanceIntensityThreshold->mValue;
            
            // curvature
            float curvature = mDistanceIntensityCurvature->mValue;
            
            // distance
            float distance = getParticleMeasurer(particleIndex)->getDistanceToVantagePoint();
            
            // 4D curve formula
            float distanceIntensityValue = math::clamp<float>(distance / threshold, 0., 1.);
            float gainValue = Functions::distanceCurve(distanceIntensityValue, curvature, false);
            
            // set gain
            auto processor = getProcessor(processorIndex);
            auto gain = processor->getNode(particleIndex);
            gain->setGain(gainValue);
            
        }

        
    }
}
