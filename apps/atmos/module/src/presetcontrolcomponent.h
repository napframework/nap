#pragma once

// Nap includes
#include <component.h>
#include <parameter.h>

namespace nap
{

    class SwitchPresetComponentInstance;
    class PresetControlComponentInstance;
    
    
    class NAPAPI PresetControlComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(PresetControlComponent, PresetControlComponentInstance)
        
    public:
        PresetControlComponent() : Component() { }
        void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

        ResourcePtr<ParameterGroup> mPresetParameterGroup = nullptr;
        std::vector<std::string> mPresets;
        float mAverageDuration = 0;
        float mDurationDeviation = 0;
        bool mRandomizeSequence = false;

    private:
    };

    
    class NAPAPI PresetControlComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        PresetControlComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        void update(double deltaTime) override;

    private:
        void nextPreset();

        SwitchPresetComponentInstance* mSwitchPresetComponent = nullptr;
        std::vector<int> mPresets;
        int mCurrentPresetIndex = -1;
        float mAverageDuration = 0;
        float mDurationDeviation = 0;
        float mCurrentPresetDuration = 0;
        float mCurrentPresetElapsedTime = 0;
        bool mRandomizeSequence = false;
    };
        
}
