#include "polyphoniccomponent.h"

// Nap includes
#include <nap/entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::PolyphonicComponent)
    RTTI_PROPERTY("Graph", &nap::audio::PolyphonicComponent::mGraph, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("VoiceCount", &nap::audio::PolyphonicComponent::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceStealing", &nap::audio::PolyphonicComponent::mVoiceStealing, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PolyphonicComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("findFreeVoice", &nap::audio::PolyphonicComponentInstance::findFreeVoice)
    RTTI_FUNCTION("play", &nap::audio::PolyphonicComponentInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::PolyphonicComponentInstance::stop)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool PolyphonicComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            auto resource = rtti_cast<PolyphonicComponent>(getComponent());
            
            for (auto i = 0; i < resource->mVoiceCount; ++i)
            {
                mVoices.emplace_back(std::make_unique<VoiceGraphInstance>());
                if (!mVoices.back()->init(*resource->mGraph, errorState))
                    return false;
                mVoices.back()->finishedSignal.connect(this, &PolyphonicComponentInstance::voiceFinished);
            }
            
            // Create the mix nodes to mix output of all the voices
            for (auto i = 0; i < resource->mGraph->mOutput->getInstance()->getChannelCount(); ++i)
                mMixNodes.emplace_back(std::make_unique<MixNode>(resource->mGraph->getNodeManager()));
            
            return true;
        }
        
        
        VoiceGraphInstance* PolyphonicComponentInstance::findFreeVoice()
        {
            for (auto& voice : mVoices)
                if (!voice->isBusy())
                {
                    voice->setBusy(true);
                    return voice.get();
                }
            
            if (rtti_cast<PolyphonicComponent>(getComponent())->mVoiceStealing)
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
        
        
        void PolyphonicComponentInstance::play(VoiceGraphInstance* voice)
        {
            if (!voice)
                return;
            
            voice->play();
            
            getNodeManager().execute([&, voice](){
                for (auto channel = 0; channel < std::min<int>(mMixNodes.size(), voice->getOutput().getChannelCount()); ++channel)
                    mMixNodes[channel]->inputs.connect(voice->getOutput().getOutputForChannel(channel));
            });
        }
        
        
        void PolyphonicComponentInstance::stop(VoiceGraphInstance* voice)
        {
            if (!voice)
                return;
            
            voice->stop();
        }
        

        OutputPin& PolyphonicComponentInstance::getOutputForChannel(int channel)
        {
            return mMixNodes[channel]->audioOutput;
            
        }
        
        
        int PolyphonicComponentInstance::getChannelCount() const
        {
            return mMixNodes.size();
        }
        
        
        void PolyphonicComponentInstance::voiceFinished(VoiceGraphInstance& voice)
        {
            VoiceGraphInstance* voicePtr = &voice;
            getNodeManager().execute([&, voicePtr](){
                for (auto channel = 0; channel < std::min<int>(mMixNodes.size(), voicePtr->getOutput().getChannelCount()); ++channel)
                    mMixNodes[channel]->inputs.disconnect(voicePtr->getOutput().getOutputForChannel(channel));
                voicePtr->setBusy(false);
            });
        }


    }
    
}
