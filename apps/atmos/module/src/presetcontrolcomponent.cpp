#include "presetcontrolcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

#include <switchpresetcomponent.h>

// RTTI
RTTI_BEGIN_STRUCT(nap::PresetControlComponent::PresetInfo)
    RTTI_PROPERTY("Preset", &nap::PresetControlComponent::PresetInfo::mPreset, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AverageDuration", &nap::PresetControlComponent::PresetInfo::mAverageDuration, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DurationDeviation", &nap::PresetControlComponent::PresetInfo::mDurationDeviation, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("TransitionTime", &nap::PresetControlComponent::PresetInfo::mTransitionTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::PresetControlComponent)
    RTTI_PROPERTY("PresetParameterGroup", &nap::PresetControlComponent::mPresetParameterGroup, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Presets", &nap::PresetControlComponent::mPresets, nap::rtti::EPropertyMetaData::Default)
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
                mPresets.emplace_back(PresetInfo(i));
        }
        else {
            for (auto &preset : resource->mPresets)
            {
                bool found = false;
                for (auto i = 0; i < availablePresets.size(); ++i)
                {
                    if (availablePresets[i] == preset.mPreset)
                    {
                        mPresets.emplace_back(PresetInfo(i, preset));
                        found = true;
                    }
                }
                if (!found)
                {
                    errorState.fail("preset %s does not exist", preset.mPreset.c_str());
                    return false;
                }
            }
        }

        for (auto& preset : mPresets)
            mPermutedPresets.emplace_back(&preset);
        permute(mPermutedPresets);

        mRandomizeSequence = resource->mRandomizeSequence;
        mEnabled = resource->mEnabled;

        mCurrentPresetDuration = mPresets[0].mAverageDuration;
        mCurrentPresetElapsedTime = 0;
        mCurrentPresetIndex = 0;
        if (mEnabled)
            mSwitchPresetComponent->selectPresetByIndex(mPresets[mCurrentPresetIndex].mPresetIndex, 0.f);

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


    PresetControlComponentInstance::PresetInfo::PresetInfo(int index, const PresetControlComponent::PresetInfo& resource)
    {
        mPresetIndex = index;
        mAverageDuration = resource.mAverageDuration;
        mDurationDeviation = resource.mDurationDeviation;
        mTransitionTime = resource.mTransitionTime;
    }


    void PresetControlComponentInstance::nextPreset()
    {
        mCurrentPresetIndex++;
        if (mCurrentPresetIndex >= mPresets.size())
        {
            permute(mPermutedPresets);
            mCurrentPresetIndex = 0;
        }

        auto preset = &mPresets[mCurrentPresetIndex];
        if (mRandomizeSequence)
            preset = mPermutedPresets[mCurrentPresetIndex];

        mCurrentPresetDuration = preset->mAverageDuration + math::random(-preset->mDurationDeviation / 2.f, preset->mDurationDeviation / 2.f);
        mCurrentPresetElapsedTime = 0;
        mSwitchPresetComponent->selectPresetByIndex(preset->mPresetIndex, preset->mTransitionTime);
    }


    void PresetControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.push_back(RTTI_OF(nap::SwitchPresetComponent));
    }


    void PresetControlComponentInstance::permute(std::vector<PresetControlComponentInstance::PresetInfo*>& list)
    {
        for (auto i = 0; i < list.size(); ++i)
        {
            auto swapIndex = math::random<int>(0, list.size() - 1);
            auto temp = list[swapIndex];
            list[swapIndex] = list[i];
            list[i] = temp;
        }
    }

}
