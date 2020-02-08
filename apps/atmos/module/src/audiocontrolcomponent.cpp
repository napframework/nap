#include "audiocontrolcomponent.h"

#include <entity.h>
#include <audio/core/graphobject.h>

RTTI_BEGIN_CLASS(nap::AudioControlComponent)
    RTTI_PROPERTY("AudioComponent", &nap::AudioControlComponent::mAudioComponent,			nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Layer1", &nap::AudioControlComponent::mLayer1, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Layer2", &nap::AudioControlComponent::mLayer2, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioControlComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{

    bool AudioControlComponentInstance::init(utility::ErrorState& errorState)
    {
        auto component = getComponent<AudioControlComponent>();

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

        mLayer1 = component->mLayer1.get();
        mLayer1->setRange(-1, sampler->getSamplerEntries().size() - 1);
        mLayer1->valueChanged.connect(mLayerChangedSlot);
        mLayer2 = component->mLayer2.get();
        mLayer2->setRange(-1, sampler->getSamplerEntries().size() - 1);
        mLayer2->valueChanged.connect(mLayerChangedSlot);

        return true;
    }


    void AudioControlComponentInstance::layerChanged(int)
    {
        std::set<int> samplerEntries;

        if (mLayer1->mValue > -1)
            samplerEntries.emplace(mLayer1->mValue);
        if (mLayer2->mValue > -1)
            samplerEntries.emplace(mLayer2->mValue);
        mLayerController->replaceLayers({ mLayer1->mValue, mLayer2->mValue }, 5000, 5000);
    }


}
