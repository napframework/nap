#include "DistributedSourceInstance.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::DistributedSourceInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {

        DistributedSourceInstance::DistributedSourceInstance(SpatialService& spatialService, int particleCount, ParameterComponentInstance& parameterComponent, const std::string& prefix) : SpatialSourceInstance(spatialService, particleCount, parameterComponent, prefix)
        {
            auto& audioService = spatialService.getAudioService();
            auto& rootProcess = spatialService.getRootProcess();

            utility::ErrorState errorState;
            mMixer = std::make_unique<audio::ParallelNodeObjectInstance<audio::FastMixNode>>();
            mMixer->init(particleCount, audioService.getNodeManager(), errorState);

            // Register each mixer channel as a 'source' at the root process.
            mProcess = audioService.getNodeManager().makeSafe<audio::ParentProcess>(audioService.getNodeManager(), rootProcess.getThreadPool(), rootProcess.getAsyncObserver());
            for (auto channel = 0; channel < particleCount; ++channel)
                mProcess->addChild(*mMixer->getChannel(channel));
            rootProcess.registerSource(*mProcess);

            // resize transforms vector..
            mTransforms.resize(particleCount);

            // Add shared distribution mode parameter
            std::vector<std::string> inputDistributions = {"consecutive", "random"};
            auto distributionMode = getParameterManager().addParameterOptionList("inputDistribution", "random", inputDistributions, true);
            distributionMode->valueChanged.connect([&, inputDistributions](int i){
                if (i == 0)
                    setDistributionMode(DistributedSourceInstance::DistributionMode::Alternating);
                else
                    setDistributionMode(DistributedSourceInstance::DistributionMode::Random);
            });
            mDistributionMode = DistributionMode ::Random;
        }


        DistributedSourceInstance::~DistributedSourceInstance()
        {
            // Unregister the source with the root process
            getSpatialService().getRootProcess().unregisterSource(*mProcess);
        }


        void DistributedSourceInstance::setDistributionMode(DistributionMode distributionMode)
        {
            if(distributionMode != mDistributionMode)
            {
                mDistributionMode = distributionMode;
                redistribute();
            }
        }


        void DistributedSourceInstance::redistribute()
        {
            int inputChannelCount = getInputChannelCount();

            if (inputChannelCount > 0)
            {
                if(mDistributionMode == Alternating){
                    for(int i = 0; i < getChannelCount(); i++){
                        setInputForParticle(i % inputChannelCount, i);
                    }
                }
                else if(mDistributionMode == Random){
                    for(int i = 0; i < getChannelCount(); i++){
                        int randomInputIndex = nap::math::random<int>(0, inputChannelCount - 1);
                        setInputForParticle(randomInputIndex, i);
                    }
                }
            }

            getDataChangedSignal()->trigger(*this);
        }


        audio::OutputPin* DistributedSourceInstance::getOutputForChannel(int channel)
        {
            return &mMixer->getChannel(channel)->audioOutput;
        }



        void DistributedSourceInstance::setInputForParticle(int inputIndex, int particleIndex, bool disconnectOthers)
        {

            auto& mixerInputPin = mMixer->getChannel(particleIndex)->inputs;

            if(disconnectOthers)
                mixerInputPin.disconnectAll();

            auto channel = getInputChannel(inputIndex);
            if (channel != nullptr)
                mixerInputPin.connect(*channel);

            mTransforms[particleIndex] = getInputTransform(inputIndex);
        }


    }
}
