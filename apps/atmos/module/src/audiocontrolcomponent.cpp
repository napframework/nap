#include "audiocontrolcomponent.h"

#include <entity.h>
#include <audio/core/graphobject.h>

RTTI_BEGIN_CLASS(nap::AudioControlComponent)
    RTTI_PROPERTY("AudioComponent", &nap::AudioControlComponent::mAudioComponent,			nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("DefaultAudioLayer", &nap::AudioControlComponent::mDefaultAudioLayer,			nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EnableDefaultLayer", &nap::AudioControlComponent::mEnableDefaultLayer,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioControlComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{

    bool AudioControlComponentInstance::init(utility::ErrorState& errorState)
    {
        auto graphObject = rtti_cast<audio::GraphObjectInstance>(mAudioComponent->getObject());
        if (graphObject == nullptr)
        {
            errorState.fail("GraphObject not found");
            return false;
        }

        auto sampler = graphObject->getObject<audio::SamplerInstance>("Sampler");
        if (sampler == nullptr)
        {
            errorState.fail("Sampler not found");
            return false;
        }

        mLayerController = std::make_unique<audio::SampleLayerController>(*sampler);

        auto resource = getComponent<AudioControlComponent>();
        mEnableDefaultLayer = resource->mEnableDefaultLayer;
        mDefaultAudioLayer = resource->mDefaultAudioLayer;

        if (mDefaultAudioLayer != nullptr)
        {
            mDefaultAudioLayer->setRange(-1, sampler->getSamplerEntries().size() - 1);
            if (mEnableDefaultLayer)
                replaceLayers({ mDefaultAudioLayer->mValue }, 4000.f);
            mDefaultAudioLayer->valueChanged.connect(mDefaultLayerChanged);
        }



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


    void AudioControlComponentInstance::defaultLayerChanged(int newLayer)
    {
        if (mEnableDefaultLayer)
            replaceLayers({ newLayer }, 4000.f);
    }


}
