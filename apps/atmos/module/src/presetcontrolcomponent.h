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
        PresetControlComponent() : Component() { }
        void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

        ResourcePtr<ParameterGroup> mPresetParameterGroup = nullptr; // The parametergroup that contains the preset
        std::vector<std::string> mPresets;  // The selection preset files that will be sequenced
        float mAverageDuration = 0;         // Average duration of each preset in the cycle
        float mDurationDeviation = 0;       // Random deviation of duration of each preset
        bool mRandomizeSequence = false;    // Indicates wether the order of the cycle of presets will be shuffled
        bool mEnabled = false;              // True to enable the preset cycle

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
        // Selects the next preset in the sequence
        void nextPreset();

        SwitchPresetComponentInstance* mSwitchPresetComponent = nullptr;
        std::vector<int> mPresets;
        int mCurrentPresetIndex = -1;
        float mAverageDuration = 0;
        float mDurationDeviation = 0;
        float mCurrentPresetDuration = 0;
        float mCurrentPresetElapsedTime = 0;
        bool mRandomizeSequence = false;
        bool mEnabled = false;
    };
        
}
