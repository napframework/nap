#include "GridSettingsComponent.h"

// Spatial includes
#include <Spatial/MultiSpeaker/MultiSpeakerSetup.h>
#include <Spatial/MultiSpeaker/casipan2/Source.hpp> // for GridSettings
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Utility/AudioFunctions.h> // for dbToA

// Nap includes
#include <entity.h>

// GLM include
#include <glm/glm.hpp>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::GridSettingsComponent)
    RTTI_PROPERTY("MultiSpeakerSetup", &nap::spatial::GridSettingsComponent::mMultiSpeakerSetup, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::GridSettingsComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {
        void GridSettingsComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ParameterComponent));
            
        }
    
        bool GridSettingsComponentInstance::init(utility::ErrorState& errorState)
        {
            ParameterComponentInstance* parameterComponent = getEntityInstance()->findComponent<ParameterComponentInstance>();

            std::vector<std::string> gridNames = static_cast<GridSettingsComponent*>(getComponent())->mMultiSpeakerSetup->getGridNames();
            
            // create grid settings data structure
            for(int i = 0; i < gridNames.size(); i++){
                mGridSettings.push_back(casipan::GridSettings());
            }
            
            // create parameters that modify the grid settings data structure
            for(int i = 0; i < mGridSettings.size(); i++){
                mGridParams.push_back(std::make_unique<GridSettingsParams>(parameterComponent, gridNames[i], &mGridSettings[i]));
                mGridParams.back()->mDataChanged.connect([&](){ triggerGridSettingsChanged(); });
            }
            
            return true;
        }
        

        GridSettingsParams::GridSettingsParams(ParameterComponentInstance* parameterComponent, const std::string& name, casipan::GridSettings* gridSettings) : mName(name), mGridSettings(gridSettings)
        {
            std::string prefix = "grid/" + mName;
            mGainLevel = &parameterComponent->addParameterFloat(prefix + "/loudness", 0.0, -48.0, 0.0);
            mBoostLevel = &parameterComponent->addParameterFloat(prefix + "/boostLevel", 0.0, 0.0, 1.0);
            mCurvature = &parameterComponent->addParameterFloat(prefix + "/articulation", 0.0, -1.0, 1.0);
            mProjectionPoint = &parameterComponent->addParameterVec3(prefix + "/projectionPoint", glm::vec3(0.0), -9999, 9999);
            mOrthogonalProjection = &parameterComponent->addParameterBool(prefix + "/orthogonalProjection", false);
            
            mDistanceAttenuation = &parameterComponent->addParameterBool(prefix + "/distanceAttenuationEnable", false);
            mDistanceAttenuationThreshold = &parameterComponent->addParameterFloat(prefix + "/distanceAttenuationThreshold", 100.0, 0., 1000.);
            
            mDistanceAttenuationCurvature = &parameterComponent->addParameterFloat(prefix + "/distanceAttenuationCurvature", 0.0, -1., 1.);

            // set default grid settings from default parameter values
            mGridSettings->gainLevel = audio::dbToA(mGainLevel->mValue);
            mGridSettings->boostLevel = mBoostLevel->mValue;
            mGridSettings->curvature = mCurvature->mValue;
            mGridSettings->projectionPoint = mProjectionPoint->mValue;
            mGridSettings->orthogonalProjection = mOrthogonalProjection->mValue;
            mGridSettings->distanceAttenuation = mDistanceAttenuation->mValue;
            mGridSettings->distanceAttenuationThreshold = mDistanceAttenuationThreshold->mValue;
            mGridSettings->distanceAttenuationCurvature = mDistanceAttenuationCurvature->mValue;
            
            // connect valueChanged signals
            mGainLevel->valueChanged.connect([&](float x){
                mGridSettings->gainLevel = audio::dbToA(x);
                mDataChanged.trigger();
            });
            mBoostLevel->valueChanged.connect([&](float x){
                mGridSettings->boostLevel = x;
                mDataChanged.trigger();
            });
            mCurvature->valueChanged.connect([&](float x){
                mGridSettings->curvature = x;
                mDataChanged.trigger();
            });
            mProjectionPoint->valueChanged.connect([&](const glm::vec3& x){
                mGridSettings->projectionPoint = x;
                mDataChanged.trigger();
            });
            mOrthogonalProjection->valueChanged.connect([&](bool value){
                mGridSettings->orthogonalProjection = value;
                mDataChanged.trigger();
            });
            mDistanceAttenuation->valueChanged.connect([&](bool value){
                mGridSettings->distanceAttenuation = value;
                mDataChanged.trigger();
            });
            mDistanceAttenuationThreshold->valueChanged.connect([&](float value){
                mGridSettings->distanceAttenuationThreshold = value;
                mDataChanged.trigger();
            });
            mDistanceAttenuationCurvature->valueChanged.connect([&](float value){
                mGridSettings->distanceAttenuationThreshold = value;
                mDataChanged.trigger();
            });

        }
        
    }
}
