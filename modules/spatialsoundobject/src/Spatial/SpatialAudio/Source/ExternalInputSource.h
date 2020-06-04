#pragma once

#include <Spatial/SpatialAudio/DistributedSourceInstance.h>

namespace nap
{

    namespace spatial
    {

        /**
         * ExternalInputSource is a DistributedSource that distributes one or more external input signals.
         */
        class ExternalInputSource : public DistributedSourceInstance {
            RTTI_ENABLE(DistributedSourceInstance)

        public:
            ExternalInputSource(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& prefix) : DistributedSourceInstance(service, channelCount, parameterComponent, prefix) { }

            bool init(utility::ErrorState& errorState);

            // Inherited from DistributedSource
            int getInputChannelCount() const override;
            float getInputChannelLevel(int index) override;

            /**
             * @return vector containing the device input channel numbers this source is listening to.
             */
            const std::vector<int> getInputChannels() const { return mInputChannels; }

        protected:
            // Inherited from DistributedSource
            audio::OutputPin* getInputChannel(int index) override;
            const SpatialTransform* getInputTransform(int index) override;

        private:
            Slot<int> mInputChannelCountChangedSlot = { this, &ExternalInputSource::inputChannelCountChanged };
            void inputChannelCountChanged(int) { updateRouting(); }

            void updateRouting();

            std::vector<int> mInputChannels = { 0 };
            SpatialTransform mTransform; // Just one default transform to pass as the input transform (as external inputs don't have a transform).

            ParameterInt* mStartChannel = nullptr;
            ParameterInt* mInputChannelCount = nullptr;
        };


    }

}