#pragma once

// Nap includes
#include <component.h>
#include <parameter.h>

namespace nap
{

    class SwitchPresetComponentInstance;
    class PresetControlComponentInstance;


    /**
     * Component that automatically selects presets on the SwitchPresetComponent
     * It cycles through a sequence of preset files.
     * The order of the sequence can be shuffled and the duration of each preset can be randomized.
     */
    class NAPAPI PresetControlComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(PresetControlComponent, PresetControlComponentInstance)
        
    public:
        struct PresetInfo
        {
            std::string mPreset = "";
            float mAverageDuration = 5.f;
            float mDurationDeviation = 0.f;
            float mTransitionTime = 3.f;
        };

    public:
        PresetControlComponent() : Component() { }
        void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

        ResourcePtr<ParameterGroup> mPresetParameterGroup = nullptr; // The parametergroup that contains the preset
        std::vector<PresetInfo> mPresets;
        bool mRandomizeSequence = false;        // Indicates wether the order of the cycle of presets will be shuffled
        bool mEnabled = false;                  // True to enable the preset cycle

    private:
    };


    /**
     * Instance of @PresetControlComponent
     */
    class NAPAPI PresetControlComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        PresetControlComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;

        /**
         * Checks wether it is time to switch to the next preset and tells the @SwitchPresetComponent to switch.
         */
        void update(double deltaTime) override;
        
    private:
        struct PresetInfo
        {
            PresetInfo(int index) { mPresetIndex = index; }
            PresetInfo(int index, const PresetControlComponent::PresetInfo& resource);
            int mPresetIndex = 0;
            float mAverageDuration = 5.f;
            float mDurationDeviation = 0.f;
            float mTransitionTime = 3.f;
        };

    private:
        // Selects the next preset in the sequence
        void nextPreset();

        // Permutes a list of presets. Helper method.
        void permute(std::vector<PresetControlComponentInstance::PresetInfo*>& list);
        
        SwitchPresetComponentInstance* mSwitchPresetComponent = nullptr;
        std::vector<PresetInfo> mPresets;
        std::vector<PresetInfo*> mPermutedPresets;
        int mCurrentPresetIndex = -1;
        float mCurrentPresetDuration = 0;
        float mCurrentPresetElapsedTime = 0;
        bool mRandomizeSequence = false;
        bool mEnabled = false;
    };
        
}
