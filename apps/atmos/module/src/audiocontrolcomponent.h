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

        void replaceLayers(const std::vector<int>& samplerEntries, audio::TimeValue crossFadeTime);

        // Component links
        ComponentInstancePtr<audio::AudioComponent> mAudioComponent = { this, &AudioControlComponent::mAudioComponent };

    private:
        std::unique_ptr<audio::SampleLayerController> mLayerController = nullptr;
    };

}