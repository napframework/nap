#pragma once

#include <Spatial/SpatialAudio/DistributedSourceInstance.h>

namespace nap
{

    namespace spatial
    {

        /**
         * @SoundObjectSource is a @DistributedSource is that distributes a given mono mixdown (for now just mono).
         */
        class NAPAPI SoundObjectSource : public DistributedSourceInstance {
            RTTI_ENABLE(DistributedSourceInstance)

        public:
            SoundObjectSource(SpatialService& service, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& prefix, EntityInstance* entityInstance) : DistributedSourceInstance(service, channelCount, parameterComponent, prefix), mEntity(entityInstance) { }

            bool init(MixdownComponentInstance* mixdown, utility::ErrorState& errorState);

            // Inherited from DistributedSource
            int getInputChannelCount() const override;
            float getInputChannelLevel(int channel) override;
            float getInputChannelGain(int inputChannel) const override;
            
            /**
             * @return the source sound object.
             */
            EntityInstance* getSourceEntity() const { return mEntity; };
            
        protected:
            // Inherited from SpatialSourceInstance
            const SpatialTransform* getInputTransform(int index) override;

            // Inherited from DistributedSource
            audio::OutputPin* getInputChannel(int index) override;

        private:
            EntityInstance* mEntity;
            MixdownComponentInstance* mMixdown = nullptr;
            std::unique_ptr<audio::ParallelNodeObjectInstance<audio::FastGainNode>> mGain;
            ParameterFloat* mReceiveAmountParameter = nullptr;
        };



    }

}
