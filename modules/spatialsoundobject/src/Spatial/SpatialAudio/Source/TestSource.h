#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/DistributedSourceInstance.h>

// Audio includes
#include <audio/node/oscillatornode.h>


namespace nap
{

    namespace spatial
    {

        /**
         * Source that outputs a sine wave to all particles.
         */
        class NAPAPI TestSource : public DistributedSourceInstance {
            RTTI_ENABLE(DistributedSourceInstance)

        public:
            TestSource(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& prefix) : DistributedSourceInstance(service, channelCount, parameterComponent, prefix) { }

            bool init(utility::ErrorState& errorState);

            // inherited from DistributedSource
            int getInputChannelCount() const override { return 1; }
            float getInputChannelLevel(int channel) override { return 1.f; } // amplitude is always 1.0

        protected:
            // inherited from DistributedSource
            audio::OutputPin* getInputChannel(int index) override;
            const SpatialTransform* getInputTransform(int index) override { return &mTransform; }

        private:
            std::unique_ptr<audio::NodeObjectInstance<audio::OscillatorNode>> mOscillator; // The external inputs.
            audio::SafeOwner<audio::WaveTable> mWaveTable;
            SpatialTransform mTransform; // Just one default transform to pass as the input transform.
        };

    }

}