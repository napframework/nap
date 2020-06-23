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
     *	Component that controls the sampler in the linked audio component.
     */
    class NAPAPI AudioControlComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(AudioControlComponent, AudioControlComponentInstance)

    public:
        // Inherited from Component
        void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { }

        ComponentPtr<audio::AudioComponent> mAudioComponent; ///< Property: "AudioComponent"
        ResourcePtr<audio::Sampler> mSampler = nullptr;
        ResourcePtr<audio::Gain> mGain = nullptr;

        ResourcePtr<ParameterNumeric<int>> mAudioLayer = nullptr; ///< Property: 'AudioLayer' index in the sampler entries
        ResourcePtr<ParameterNumeric<float>> mAudioCrossFadeTime = nullptr; ///< Property: 'AudioCrossFadeTime' in seconds
        ResourcePtr<ParameterNumeric<float>> mAudioVolume = nullptr; ///< Property: 'AudioVolume' in dB
		ResourcePtr<ParameterNumeric<float>> mAudioSensorControl = nullptr; ///< Property: Sensor input normalized between 0 and 1
		ResourcePtr<ParameterNumeric<float>> mMasterVolume = nullptr; ///< Property: 'MasterVolume' in dB
		bool mHasSensorControl = false; ///< Property: 'HasSensorControl' Indicates wether the AudioSensorControl parameter will be used
    };


    /**
     *	Instance 0f @AudioControlComponent
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
        audio::ParallelNodeObjectInstance<audio::GainNode>* mGain = nullptr;

        ResourcePtr<ParameterNumeric<int>> mAudioLayer = nullptr;
        ResourcePtr<ParameterNumeric<float>> mAudioCrossFadeTime = nullptr;
        ResourcePtr<ParameterNumeric<float>> mAudioVolume = nullptr;
        ResourcePtr<ParameterNumeric<float>> mMasterVolume = nullptr;
		ResourcePtr<ParameterNumeric<float>> mAudioSensorControl = nullptr;

        Slot<int> mAudioLayerChanged = { this, &AudioControlComponentInstance::audioLayerChanged };
        void audioLayerChanged(int newLayer);

        Slot<float> mVolumeChanged = { this, &AudioControlComponentInstance::volumeChanged };
        void volumeChanged(float);
	};

}