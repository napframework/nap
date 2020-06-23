#include "audiocontrolcomponent.h"

#include <entity.h>
#include <audio/core/graphobject.h>
#include <audio/utility/audiofunctions.h>

RTTI_BEGIN_CLASS(nap::AudioControlComponent)
    RTTI_PROPERTY("AudioComponent", &nap::AudioControlComponent::mAudioComponent, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AudioLayer", &nap::AudioControlComponent::mAudioLayer, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AudioCrossFadeTime", &nap::AudioControlComponent::mAudioCrossFadeTime, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AudioVolume", &nap::AudioControlComponent::mAudioVolume, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AudioSensorControl", &nap::AudioControlComponent::mAudioSensorControl, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MasterVolume", &nap::AudioControlComponent::mMasterVolume, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Sampler", &nap::AudioControlComponent::mSampler,	nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Gain", &nap::AudioControlComponent::mGain, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("HasSensorControl", &nap::AudioControlComponent::mHasSensorControl, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioControlComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{

    bool AudioControlComponentInstance::init(utility::ErrorState& errorState)
    {
        auto resource = getComponent<AudioControlComponent>();

        auto graphObject = rtti_cast<audio::GraphObjectInstance>(mAudioComponent->getObject());
        if (graphObject == nullptr)
        {
            errorState.fail("GraphObject not found");
            return false;
        }

        if (resource->mSampler == nullptr)
        {
            errorState.fail("No sampler specified");
            return false;
        }

        auto sampler = graphObject->getObject<audio::SamplerInstance>(resource->mSampler->mID);
        if (sampler == nullptr)
        {
            errorState.fail("Sampler not found");
            return false;
        }

        if (resource->mGain == nullptr)
        {
            errorState.fail("No gain specified");
            return false;
        }

        mGain = graphObject->getObject<audio::ParallelNodeObjectInstance<audio::GainNode>>(resource->mGain->mID);
        if (mGain == nullptr)
        {
            errorState.fail("Gain not found");
            return false;
        }

        mLayerController = std::make_unique<audio::SampleLayerController>(*sampler);

        mAudioLayer = resource->mAudioLayer;
        mAudioCrossFadeTime = resource->mAudioCrossFadeTime;
        mAudioVolume = resource->mAudioVolume;
		mAudioSensorControl = resource->mHasSensorControl ? resource->mAudioSensorControl : nullptr;
		mMasterVolume = resource->mMasterVolume;

        mAudioLayer->setRange(-1, sampler->getSamplerEntries().size() - 1);
        replaceLayers({ mAudioLayer->mValue }, mAudioCrossFadeTime->mValue);
        mAudioLayer->valueChanged.connect(mAudioLayerChanged);

        mAudioVolume->valueChanged.connect(mVolumeChanged);
        mMasterVolume->valueChanged.connect(mVolumeChanged);

		if (mAudioSensorControl != nullptr)
			mAudioSensorControl->valueChanged.connect(mVolumeChanged);

        volumeChanged(0);

        return true;
    }


    void AudioControlComponentInstance::replaceLayers(const std::vector<int>& samplerEntries, audio::TimeValue crossFadeTime)
    {
        std::set<int> samplerEntrySet;
        for (auto entry : samplerEntries)
            if (entry >= 0)
                samplerEntrySet.emplace(entry);
        mLayerController->replaceLayers(samplerEntrySet, crossFadeTime, crossFadeTime);
    }


    void AudioControlComponentInstance::audioLayerChanged(int newLayer)
    {
        replaceLayers({ newLayer }, mAudioCrossFadeTime->mValue * 1000.f);
    }


    void AudioControlComponentInstance::volumeChanged(float)
    {
		auto sensorControl = 1.f;
		if (mAudioSensorControl != nullptr)
		{
			sensorControl = -48.f + mAudioSensorControl->mValue * 48;
			sensorControl = audio::dbToA(sensorControl);
		}

        for (auto channel = 0; channel < mGain->getChannelCount(); ++channel)
            mGain->getChannel(channel)->setGain(audio::dbToA(mAudioVolume->mValue) * audio::dbToA(mMasterVolume->mValue) * sensorControl, 10.f);
    }


}
