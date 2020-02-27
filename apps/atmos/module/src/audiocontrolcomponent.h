#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <parameternumeric.h>

#include <audio/component/audiocomponent.h>
#include <audio/object/sampler.h>
#include <audio/utility/samplelayercontroller.h>

namespace nap
{

    class AudioControlComponentInstance;

    /**
     *	Resource parth. Allows for switching between the various camera control methods.
     */
    class NAPAPI AudioControlComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(AudioControlComponent, AudioControlComponentInstance)

    public:
        // Inherited from Component
        void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { }

        ComponentPtr<audio::AudioComponent> mAudioComponent; ///< Property: "AudioComponent"
        ResourcePtr<ParameterInt> mLayer1 = nullptr; ///< Property: 'Layer1'
        ResourcePtr<ParameterInt> mLayer2 = nullptr; ///< Property: 'Layer2'
        ResourcePtr<ParameterFloat> mCrossFadeTime = nullptr; ///< Property: 'CrossFadeTime'
    };


    /**
     *	Resource part. Allows for switching between the various camera control methods.
     */
    class NAPAPI AudioControlComponentInstance : public ComponentInstance
    {
    RTTI_ENABLE(ComponentInstance)
    public:
        AudioControlComponentInstance(EntityInstance& entity, Component& resource) :
                ComponentInstance(entity, resource)									{ }

        // Inherited from ComponentInstance
        bool init(utility::ErrorState& errorState) override;

        // Component links
        ComponentInstancePtr<audio::AudioComponent> mAudioComponent = { this, &AudioControlComponent::mAudioComponent };

        // Pointers to parameters
        ParameterInt* mLayer1 = nullptr;
        ParameterInt* mLayer2 = nullptr;
        ParameterFloat* mCrossFadeTime = nullptr;

    private:
        Slot<int> mLayerChangedSlot = { this, &AudioControlComponentInstance::layerChanged };
        void layerChanged(int);

        std::unique_ptr<audio::SampleLayerController> mLayerController = nullptr;
    };

}