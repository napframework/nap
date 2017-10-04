#include "polyphoniccomponent.h"

// Nap includes
#include <nap/entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::PolyphonicObject)
    RTTI_PROPERTY("Graph", &nap::audio::PolyphonicObject::mGraph, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("VoiceCount", &nap::audio::PolyphonicObject::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceStealing", &nap::audio::PolyphonicObject::mVoiceStealing, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PolyphonicObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::PolyphonicObject&)
    RTTI_FUNCTION("findFreeVoice", &nap::audio::PolyphonicObjectInstance::findFreeVoice)
    RTTI_FUNCTION("play", &nap::audio::PolyphonicObjectInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::PolyphonicObjectInstance::stop)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> PolyphonicObject::createInstance()
        {
            return std::make_unique<PolyphonicObjectInstance>(*this);
        }

    
        bool PolyphonicObjectInstance::init(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto resource = rtti_cast<PolyphonicObject>(&getResource());
            mNodeManager = &nodeManager;
            
            for (auto i = 0; i < resource->mVoiceCount; ++i)
            {
                mVoices.emplace_back(std::make_unique<VoiceGraphInstance>());
                if (!mVoices.back()->init(*resource->mGraph, errorState))
                    return false;
                mVoices.back()->finishedSignal.connect(this, &PolyphonicObjectInstance::voiceFinished);
            }
            
            // Create the mix nodes to mix output of all the voices
            for (auto i = 0; i < resource->mGraph->mOutput->getInstance()->getChannelCount(); ++i)
                mMixNodes.emplace_back(std::make_unique<MixNode>(resource->mGraph->getNodeManager()));
            
            return true;
        }
        
        
        VoiceGraphInstance* PolyphonicObjectInstance::findFreeVoice()
        {
            std::unique_lock<std::mutex> lock(mNodeManager->getProcessingMutex());
            
            for (auto& voice : mVoices)
                if (!voice->isBusy())
                {
                    voice->setBusy(true);
                    return voice.get();
                }
            
            if (rtti_cast<PolyphonicObject>(&getResource())->mVoiceStealing)
            {
                DiscreteTimeValue time = mVoices[0]->getStartTime();
                auto result = mVoices[0].get();
                for (auto& voice : mVoices)
                    if (voice->getStartTime() < time)
                        result = voice.get();
                
                return result;
            }
            
            return nullptr;
        }
        
        
        void PolyphonicObjectInstance::play(VoiceGraphInstance* voice)
        {
            if (!voice)
                return;
            
            std::unique_lock<std::mutex> lock(mNodeManager->getProcessingMutex());
            
            voice->play();
            
            for (auto channel = 0; channel < std::min<int>(mMixNodes.size(), voice->getOutput().getChannelCount()); ++channel)
                mMixNodes[channel]->inputs.connect(voice->getOutput().getOutputForChannel(channel));
        }
        
        
        void PolyphonicObjectInstance::stop(VoiceGraphInstance* voice)
        {
            if (!voice)
                return;
            
            std::unique_lock<std::mutex> lock(mNodeManager->getProcessingMutex());
            voice->stop();
        }
        

        OutputPin& PolyphonicObjectInstance::getOutputForChannel(int channel)
        {
            return mMixNodes[channel]->audioOutput;
            
        }
        
        
        int PolyphonicObjectInstance::getChannelCount() const
        {
            return mMixNodes.size();
        }
        
        
        void PolyphonicObjectInstance::voiceFinished(VoiceGraphInstance& voice)
        {
            assert(voice.getEnvelope().getValue() == 0);
            for (auto channel = 0; channel < std::min<int>(mMixNodes.size(), voice.getOutput().getChannelCount()); ++channel)
                mMixNodes[channel]->inputs.disconnect(voice.getOutput().getOutputForChannel(channel));
            voice.setBusy(false);
        }


    }
    
}
