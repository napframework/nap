#include "SoundObjectSource.h"

// Spatial includes
#include <Spatial/SpatialAudio/MixdownComponent.h>
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/service/audioservice.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SoundObjectSource)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        bool SoundObjectSource::init(MixdownComponentInstance* mixdown, utility::ErrorState& errorState)
        {
            mMixdown = mixdown;

            if (mMixdown->getChannelCount() != 1)
            {
                errorState.fail("only mono sound object sources are currently supported");
                return false;
            }

            // for now we are just using a mono mixdown, so we create only one gain.
            mGain = std::make_unique<audio::ParallelNodeObjectInstance<audio::FastGainNode>>();
            if (!mGain->init(getChannelCount(), getSpatialService().getAudioService().getNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize gain for SoundObjectSource");
                return false;
            }

            auto channel = mGain->getChannel(0);
            channel->audioInput.connect(*mMixdown->getOutput(0)->getOutputPin());
            channel->setGain(0.f);

            // trigger data changed signal when mixdown transform changed
            mMixdown->getOutput(0)->getTransformChangedSignal()->connect([&, this](const auto& x){ getDataChangedSignal()->trigger(*this); });

            // distribute mixdown to particles
            redistribute();

            // Add receive amount parameter.
            mReceiveAmountParameter = getParameterManager().addParameterFloat("amount", 0.0, 0.0, 1.0);
            mReceiveAmountParameter->valueChanged.connect([&](float value){
                mGain->getChannel(0)->setGain(value);
            });

            return true;
        }


        audio::OutputPin* SoundObjectSource::getInputChannel(int index)
        {
            if(index == 0)
                return &mGain->getChannel(0)->audioOutput;
            else
                return nullptr;
        }


        const SpatialTransform* SoundObjectSource::getInputTransform(int index)
        {
            return &mMixdown->getOutput(index)->getTransform();
        }


        int SoundObjectSource::getInputChannelCount() const
        {
            return 1;
        }

        float SoundObjectSource::getInputChannelLevel(int channel)
        {
            return mMixdown->getMeasuredLevel();
        }


        float SoundObjectSource::getInputChannelGain(int inputChannel) const
        {
            return mReceiveAmountParameter->mValue;
        }



    }

}