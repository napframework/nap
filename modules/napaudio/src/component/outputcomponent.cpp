#include "outputcomponent.h"

// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>
#include "audiocomponent.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::OutputComponent)
    RTTI_PROPERTY("Input", &nap::audio::OutputComponent::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
    
        bool audio::OutputComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            OutputComponent* resource = rtti_cast<OutputComponent>(getComponent());
            
            AudioComponentInstance* input = rtti_cast<AudioComponentInstance>(resource->mInput.get());
            if (!errorState.check(input, "Input is not an audio component"))
                return false;
            
            auto& nodeManager = getEntityInstance()->getCore()->getService<AudioService>()->getNodeManager();
            
            for (auto channel = 0; channel < input->getChannelCount(); ++channel)
            {
                mOutputs.emplace_back(std::make_unique<OutputNode>(nodeManager, true));
                mOutputs[channel]->setOutputChannel(channel);
                mOutputs[channel]->audioInput.connect(input->getOutputForChannel(channel));
            }
            
            return true;
        }

    }
    
}
