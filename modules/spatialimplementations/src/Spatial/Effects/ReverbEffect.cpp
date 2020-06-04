#include "ReverbEffect.h"

// Spatial includes
#include <Spatial/Utility/AudioFunctions.h>

RTTI_DEFINE_CLASS(nap::spatial::DistanceDiffusionEffect)

RTTI_DEFINE_CLASS(nap::spatial::ReverbEffect)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ReverbEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ReverbEffectInstance)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistanceDiffusionEffectInstance)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {

        bool ReverbEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            
            mLength = getParameterManager().addParameterFloat("decay", 0.5, 0., 1.);
            mLength->connect([&](float value){
                mFeedbackValue = pow(value * 0.99f, 0.1f);
            });
            recalculateOnChange(mLength);
            
            mDamping = getParameterManager().addParameterFloat("damping", 0.5, 0., 1.);
            recalculateOnChange(mDamping);

            
            // dry wet parameter
            mDryWet = getParameterManager().addParameterFloat("dryWet", 0.0, 0.0, 1.0);
            recalculateOnChange(mDryWet);
            mGain = getParameterManager().addParameterFloat("gain", -6., -48., 6.);
            recalculateOnChange(mGain);
            
            return true;

        }

        
        void ReverbEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
            float dryWetValue = mDryWet->mValue;
            getProcessor(processorIndex)->setDry(1.f - dryWetValue);
            getProcessor(processorIndex)->setWet(dryWetValue * audio::dbToA(mGain->mValue));
            getProcessor(processorIndex)->getNode(particleIndex)->setDamping(mDamping->mValue);
            getProcessor(processorIndex)->getNode(particleIndex)->setFeedback(mFeedbackValue);
        }
        
        
        bool DistanceDiffusionEffectInstance::onInit(EntityInstance* entity, utility::ErrorState& errorState)
        {
            
            mDistanceThreshold = getParameterManager().addParameterFloat("threshold", 100.0, 0.001, 1000.0);
            recalculateOnChange(mDistanceThreshold);
            mDistanceCurvature = getParameterManager().addParameterFloat("curve", -0.4, -1.0, 1.0);
            recalculateOnChange(mDistanceCurvature);
            
            mLength = getParameterManager().addParameterFloat("decay", 0.5, 0., 1.);
            mLength->connect([&](float value){
                mFeedbackValue = pow(value * 0.99f, 0.1f);
            });
            recalculateOnChange(mLength);
            
            mDamping = getParameterManager().addParameterFloat("damping", 0.5, 0., 1.);
            recalculateOnChange(mDamping);
            
            
            mGain = getParameterManager().addParameterFloat("gain", -6., -48., 6.);
            recalculateOnChange(mGain);
            
            recalculateParticleOnDataChange(); // update on distance change
            
            return true;
            
        }
        
        
        void DistanceDiffusionEffectInstance::recalculate(int processorIndex, int particleIndex)
        {
         
            // threshold
            float threshold = mDistanceThreshold->mValue;
            
            // curvature
            float curvature = mDistanceCurvature->mValue;
            
            // distance to vantagepoint
            float distance = getParticleMeasurer(particleIndex)->getDistanceToVantagePoint();
            
            // 4D curve formula
            float distanceValue = math::clamp<float>(distance / threshold, 0., 1.);
            float dryWetValue = Functions::distanceCurve(distanceValue, curvature, true);
            
            getProcessor(processorIndex)->setDry(1.f - dryWetValue, particleIndex);
            getProcessor(processorIndex)->setWet(dryWetValue * audio::dbToA(mGain->mValue), particleIndex);
            
            getProcessor(processorIndex)->getNode(particleIndex)->setDamping(mDamping->mValue);
            getProcessor(processorIndex)->getNode(particleIndex)->setFeedback(mFeedbackValue);
            
        }


    }

}
