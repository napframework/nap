#include "presetcontrolcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

#include <switchpresetcomponent.h>

// RTTI
RTTI_BEGIN_CLASS(nap::PresetControlComponent)
    RTTI_PROPERTY("PresetParameterGroup", &nap::PresetControlComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Presets", &nap::PresetControlComponent::mPresets, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AverageDuration", &nap::PresetControlComponent::mAverageDuration, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("DurationDeviation", &nap::PresetControlComponent::mDurationDeviation, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RandomizeSequence", &nap::PresetControlComponent::mRandomizeSequence, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Enabled", &nap::PresetControlComponent::mEnabled, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PresetControlComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    bool PresetControlComponentInstance::init(utility::ErrorState& errorState)
    {
        auto resource = getComponent<PresetControlComponent>();
        mSwitchPresetComponent = &getEntityInstance()->getComponent<SwitchPresetComponentInstance>();

        auto presetGroup = resource->mPresetParameterGroup;
        auto parameterService = this->getEntityInstance()->getCore()->getService<nap::ParameterService>();
        std::vector<std::string> availablePresets = parameterService->getPresets(*presetGroup);

        if (!errorState.check(availablePresets.size() > 0, "No presets available"))
            return false;

        if (resource->mPresets.empty())
        {
            for (auto i = 0; i < availablePresets.size(); ++i)
                mPresets.emplace_back(i);
        }
        else {
            for (auto &preset : resource->mPresets) {
                for (auto i = 0; i < availablePresets.size(); ++i) {
                    if (availablePresets[i] == preset)
                        mPresets.emplace_back(i);
                    if (i == availablePresets.size())
                    {
                        errorState.fail("preset %s does not exist", preset.c_str());
                        return false;
                    }
                }
            }
        }

        mAverageDuration = resource->mAverageDuration;
        mDurationDeviation = resource->mDurationDeviation;
        mRandomizeSequence = resource->mRandomizeSequence;
        mEnabled = resource->mEnabled;

        mCurrentPresetDuration = mAverageDuration;
        mCurrentPresetElapsedTime = 0;
        mCurrentPresetIndex = 0;
        if (mEnabled)
            mSwitchPresetComponent->selectPresetByIndex(mPresets[mCurrentPresetIndex]);

        return true;
    }


    void PresetControlComponentInstance::update(double deltaTime)
    {
        if (!mEnabled)
            return;

        mCurrentPresetElapsedTime += deltaTime;
        if (mCurrentPresetElapsedTime >= mCurrentPresetDuration)
            nextPreset();
    }


    void PresetControlComponentInstance::nextPreset()
    {
        mCurrentPresetIndex++;
        if (mCurrentPresetIndex >= mPresets.size())
            mCurrentPresetIndex = 0;
        mCurrentPresetDuration = mAverageDuration;
        mCurrentPresetElapsedTime = 0;
        mSwitchPresetComponent->selectPresetByIndex(mPresets[mCurrentPresetIndex]);

    }


    void PresetControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.push_back(RTTI_OF(nap::SwitchPresetComponent));
    }

        
}
