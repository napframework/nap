#include "MixdownOutputComponent.h"

// Spatial includes
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/ParameterComponent.h>

// Multispeaker includes
#include <Spatial/MultiSpeaker/MultiSpeakerService.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Std includes
#include <string>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::MixdownOutputComponent)
    RTTI_PROPERTY("MixdownComponent", &nap::spatial::MixdownOutputComponent::mMixdownComponent, nap::rtti::EPropertyMetaData::Required);
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MixdownOutputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        void MixdownOutputComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ParameterComponent));
            components.push_back(RTTI_OF(SpatializationComponent)); // to access root process
        }

        
        bool MixdownOutputComponentInstance::init(utility::ErrorState& errorState)
        {
            mMultiSpeakerService = getEntityInstance()->getCore()->getService<MultiSpeakerService>();
            auto& audioService = mMultiSpeakerService->getSpatialService().getAudioService();
            auto& nodeManager = audioService.getNodeManager();
            
            // create parameters
            auto& parameterComponent = getEntityInstance()->getComponent<ParameterComponentInstance>();
            
            mOutputChannel = &parameterComponent.addParameterInt("externalSend/channel", 256, 1, 256);
            mOutputChannel->valueChanged.connect([&](float x){
                updateChannelAndEnable(); } );
            
            mEnable = &parameterComponent.addParameterBool("externalSend/enable", false);
            mEnable->valueChanged.connect([&](bool value){ updateChannelAndEnable(); } );
            
            
            mVolume = &parameterComponent.addParameterFloat("externalSend/amount", 0.0, 0.0, 1.0);
            mVolume->valueChanged.connect([&](float value){ setGain(value); });

            // initialise gain node
            mGainNode = nodeManager.makeSafe<audio::FastGainNode>(nodeManager);
            mGainNode->setGain(0.);
            
            // connect mixdowncomponent output to gain node (we only support mono for now)
            mGainNode->audioInput.connect(*mMixdownComponent->getOutput(0)->getOutputPin());

            // initialise output node and connect gain node to output node
            mOutputNode = nodeManager.makeSafe<audio::OutputNode>(nodeManager, false); // false: don't register as root process
            mOutputNode->audioInput.connect(mGainNode->audioOutput);
            mOutputNode->setOutputChannel(-1);
            
            // get root process
            mRootProcess = &getEntityInstance()->getComponent<SpatializationComponentInstance>().getSpatialService().getRootProcess();

            // connect spatial service outputcountchanged signal to updateEnable().
            mMultiSpeakerService->mVacantChannelsChangedSignal.connect(mVacantChannelsChangedSlot);
            
            updateChannelAndEnable();

            return true;
        }
        
        void MixdownOutputComponentInstance::updateChannelAndEnable()
        {
            if(mEnable == nullptr)
                return;
            
            int selectedChannel = mOutputChannel->mValue - 1;
            
            std::vector<int> vacantChannels = { };
            if(mMultiSpeakerService)
                vacantChannels = mMultiSpeakerService->getVacantChannels();

            if(std::find(vacantChannels.begin(), vacantChannels.end(), selectedChannel) != vacantChannels.end())
                mValidChannel = true;
            else
                mValidChannel = false;

            // update channel if it is a valid channel
            if(mValidChannel)
                mOutputNode->setOutputChannel(selectedChannel);
            else
                mOutputNode->setOutputChannel(-1); // set to invalid channel to prevent short audio burst still going to a speaker channel (because unregistering is scheduled)
            
            bool newEnabled = mEnable->mValue && mValidChannel;
            
            if(newEnabled != mEnabled)
            {
                mEnabled = newEnabled;
                
                if(mEnabled)
                    mRootProcess->registerPostMixdownProcess(*mOutputNode.getRaw());
                else
                    mRootProcess->unregisterPostMixdownProcess(*mOutputNode.getRaw());
            }
            
        }
        
        void MixdownOutputComponentInstance::setGain(float gain)
        {
            mGainNode->setGain(gain);
        }
        
    }
    
}
