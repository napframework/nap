#include "InputDistanceIntensityEffect.h"

RTTI_DEFINE_CLASS(nap::spatial::InputDistanceIntensityEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InputDistanceIntensityEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::InputDistanceIntensityEffectInstance)
RTTI_END_CLASS


namespace nap
{
    
    namespace spatial
    {
        
        
        bool InputDistanceIntensityEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            mInputDistanceIntensityThreshold = getParameterManager().addParameterFloat("threshold", 100.0, 0.001, 1000.0);
            mInputDistanceIntensityCurvature = getParameterManager().addParameterFloat("curve", -0.4, -1.0, 1.0);

            recalculateOnChange(mInputDistanceIntensityThreshold);
            recalculateOnChange(mInputDistanceIntensityCurvature);
            recalculateParticleOnDataChange();

            return true;
        }

        
        void InputDistanceIntensityEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
            // threshold
            float threshold = mInputDistanceIntensityThreshold->mValue;

            // curvature
            float curvature = mInputDistanceIntensityCurvature->mValue;

            // distance to input
            float distance = getParticleMeasurer(particleIndex)->getDistanceToInput(processorIndex);

            // 4D curve formula
            float distanceIntensityValue = math::clamp<float>(distance / threshold, 0., 1.);
            float gainValue = Functions::distanceCurve(distanceIntensityValue, curvature, false);

            // set node
            // set gain
            auto processor = getProcessor(processorIndex);
            auto gain = processor->getNode(particleIndex);
            gain->setGain(gainValue);
        }
        
        
    }
}
