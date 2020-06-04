#pragma once

#include <Spatial/SpatialAudio/SpatialSource.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainNode.h>
#include <Spatial/Audio/FastMixNode.h>

// Audio includes
#include <audio/core/process.h>
#include <audio/utility/safeptr.h>
#include <audio/core/nodeobject.h>


namespace nap
{

    namespace spatial
    {

        /**
         * A DistributedSource is a SpatialSource that distributes an x-channel external input / soundobject input to a particlecount-channel output.
         */
        class NAPAPI DistributedSourceInstance : public SpatialSourceInstance {
            RTTI_ENABLE(SpatialSourceInstance)

        public:
            enum DistributionMode { Alternating, Random };

        public:
            DistributedSourceInstance(SpatialService& spatialService, int particleCount, ParameterComponentInstance& parameterComponent, const std::string& prefix);
            ~DistributedSourceInstance();

            /**
            * Has to be overwritten to return number of input/source channels
            */
            virtual int getInputChannelCount() const = 0;

            /**
             * Has to be overwritten to return the level of the incoming single into the source.
             * This function cannot be defined const as in most cases it has to call @LevelMeterNode::getLevel(), which performs internal calculations on its data.
             * The returnvalue is the level before the gain is applied that is returned by @getInputChannelGain().
             */
            virtual float getInputChannelLevel(int inputChannel) = 0;

            /**
             * This function can be overwritten to return the current value of a gain applied to the input for @inputChannel
             */
            virtual float getInputChannelGain(int inputChannel) const { return 1.f; }

            /**
            * Returns the input transform of the input that is set to this particle channel by the distribution algorithm.
            * Inherited from SpatialSource.
            */
            const SpatialTransform* getInputTransformForChannel(int channel) const override {
                return mTransforms[channel];
            };

             audio::OutputPin* getOutputForChannel(int channel) override;

             /**
             * Sets the distribution mode.
             */
            void setDistributionMode(DistributionMode distributionMode);

        protected:
            /**
             * Should return the input channel (from externalinput or soundobjectinput) for the given input index.
             */
            virtual audio::OutputPin* getInputChannel(int index) = 0;

            /**
             * Should return the transform for the given input index.
             */
            virtual const SpatialTransform* getInputTransform(int index) = 0;

            /**
             * Routes the given inputs to the particle outputs according to the current distribution mode.
             * Should be called on initialization
             */
            void redistribute();

        private:
            /**
             * Helper function. Sets the transform and outputpin for a particle to the given ones for the inputIndex.
             */
            void setInputForParticle(int inputIndex, int particleIndex, bool disconnectOthers = true);

            std::unique_ptr<audio::ParallelNodeObjectInstance<audio::FastMixNode>> mMixer = nullptr; // mixer of particlecount channels.
            std::vector<const SpatialTransform*> mTransforms; // vector of pointers to input transform per particle.

            DistributionMode mDistributionMode = Alternating; // the current distribution mode

            audio::SafeOwner<audio::ParentProcess> mProcess = nullptr;

            std::unique_ptr<audio::IMultiChannelOutput> mInput = nullptr;
        };


    }

}
