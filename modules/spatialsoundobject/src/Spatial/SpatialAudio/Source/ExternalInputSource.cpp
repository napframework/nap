#include "ExternalInputSource.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ExternalInputSource)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        bool ExternalInputSource::init(utility::ErrorState& errorState)
        {
            // sets the channel count
            mInputChannelCount = getParameterManager().addParameterInt("externalInputChannelCount", 1, 1, 64);
            mInputChannelCount->valueChanged.connect([&](int){
                updateRouting();
            });

            // sets the first channel of the externalinput. All next channels will count up from this channel.
            mStartChannel = getParameterManager().addParameterInt("externalInputStartChannel", 1, 1, 128);
            mStartChannel->valueChanged.connect([&](int){
                updateRouting();
            });

            // Respond to a change of available ADC input channels
            getSpatialService().mInputChannelCountChangedSignal.connect(mInputChannelCountChangedSlot);

            updateRouting();

            return true;
        }


        audio::OutputPin* ExternalInputSource::getInputChannel(int index)
        {
            auto channel = mInputChannels[index];
            if (channel < getSpatialService().getInputChannelCount())
                return &getSpatialService().getInput(mInputChannels[index]);
            else
                return nullptr;
        }


        float ExternalInputSource::getInputChannelLevel(int index)
        {
            auto channel = mInputChannels[index];
            if (channel < getSpatialService().getInputChannelCount())
                return getSpatialService().getInputLevel(mInputChannels[index]);
            else
                return 0.f;
        }


        const SpatialTransform* ExternalInputSource::getInputTransform(int index)
        {
            return &mTransform;
        }


        int ExternalInputSource::getInputChannelCount() const
        {
            return mInputChannels.size();
        }


        void ExternalInputSource::updateRouting()
        {
            std::vector<int> inputChannels;
            for(int i = 0; i < mInputChannelCount->mValue; i++){

                int channel = mStartChannel->mValue - 1 + i;
                inputChannels.push_back(channel);
            }

            mInputChannels = inputChannels;
            redistribute();
        }



    }

}