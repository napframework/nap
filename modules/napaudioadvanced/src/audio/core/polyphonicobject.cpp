#include "polyphonicobject.h"

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::PolyphonicObject)
    RTTI_PROPERTY("Voice", &nap::audio::PolyphonicObject::mVoice, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("VoiceCount", &nap::audio::PolyphonicObject::mVoiceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("VoiceStealing", &nap::audio::PolyphonicObject::mVoiceStealing, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ChannelCount", &nap::audio::PolyphonicObject::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::PolyphonicObjectInstance)
    RTTI_FUNCTION("findFreeVoice", &nap::audio::PolyphonicObjectInstance::findFreeVoice)
    RTTI_FUNCTION("play", &nap::audio::PolyphonicObjectInstance::play)
    RTTI_FUNCTION("playOnChannels", &nap::audio::PolyphonicObjectInstance::playOnChannels)
    RTTI_FUNCTION("stop", &nap::audio::PolyphonicObjectInstance::stop)
    RTTI_FUNCTION("getBusyVoiceCount", &nap::audio::PolyphonicObjectInstance::getBusyVoiceCount)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> PolyphonicObject::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<PolyphonicObjectInstance>();
            if (!instance->init(*mVoice, mVoiceCount, mVoiceStealing, mChannelCount, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }


        bool PolyphonicObjectInstance::init(Voice& voice, int voiceCount, bool voiceStealing, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mNodeManager = &nodeManager;

            for (auto i = 0; i < voiceCount; ++i)
            {
                mVoices.emplace_back(std::make_unique<VoiceInstance>());
                if (!mVoices.back()->init(voice, nodeManager, errorState))
                    return false;
                mVoices.back()->finishedSignal.connect(voiceFinishedSlot);
            }

            // Create the mix nodes to mix output of all the voices
            for (auto i = 0; i < channelCount; ++i)
                mMixNodes.emplace_back(mNodeManager->makeSafe<MixNode>(*mNodeManager));
            
            mVoiceStealing = voiceStealing;

            return true;
        }


        VoiceInstance* PolyphonicObjectInstance::findFreeVoice()
        {
            for (auto& voice : mVoices)
                if (voice->try_use())
                    return voice.get();

            if (mVoiceStealing)
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


        void PolyphonicObjectInstance::play(VoiceInstance* voice, TimeValue duration)
        {
            if (!voice)
                return;

            voice->play(duration);

            // We cache the channel numbers of the output mixer that the voice will be connected to within the voice object.
            // We do that here already and not in the enqueued task to avoid allocations on the audio thread.
            voice->mConnectedToChannels.clear();
            for (auto channel = 0; channel < std::min<int>(mMixNodes.size(), voice->getOutput()->getChannelCount()); ++channel)
                voice->mConnectedToChannels.emplace_back(channel);

            mNodeManager->enqueueTask([&, voice](){
                for (auto i = 0; i < voice->mConnectedToChannels.size(); ++i)
                    mMixNodes[voice->mConnectedToChannels[i]]->inputs.connect(*voice->getOutput()->getOutputForChannel(i));
            });
        }


        void PolyphonicObjectInstance::playOnChannels(VoiceInstance* voice, std::vector<unsigned int> channels, TimeValue duration)
        {
            if (!voice)
                return;

            voice->play(duration);

            // We cache the channel numbers of the output mixer that the voice will be connected to within the voice object.
            // We do that here already and not in the enqueued task to avoid allocations on the audio thread.
            voice->mConnectedToChannels.clear();
            for (auto channel : channels)
                if (channel < mMixNodes.size())
                    voice->mConnectedToChannels.emplace_back(channel);

            mNodeManager->enqueueTask([&, voice](){
                for (auto i = 0; i < voice->mConnectedToChannels.size(); ++i)
                    mMixNodes[voice->mConnectedToChannels[i]]->inputs.connect(*voice->getOutput()->getOutputForChannel(i % voice->getOutput()->getChannelCount()));
            });
        }


        void PolyphonicObjectInstance::stop(VoiceInstance* voice, TimeValue fadeOutTime)
        {
            if (!voice)
                return;

            voice->stop(fadeOutTime);
        }


        int PolyphonicObjectInstance::getBusyVoiceCount() const
        {
            int result = 0;
            for (auto& voice : mVoices)
                if (voice->isBusy())
                    result++;
            return result;
        }


        OutputPin* PolyphonicObjectInstance::getOutputForChannel(int channel)
        {
            return &mMixNodes[channel]->audioOutput;
        }


        int PolyphonicObjectInstance::getChannelCount() const
        {
            return mMixNodes.size();
        }


        void PolyphonicObjectInstance::voiceFinished(VoiceInstance& voice)
        {
            assert(voice.getEnvelope().getValue() == 0);

            for (auto channel = 0; channel < voice.mConnectedToChannels.size(); ++channel)
            {
                // TODO: Crashed so called enqueueDisconnect() instead, doesn't fix the issue.
                // It crashes because the mix nodes have been deleted on realtime-edit.
                // Means the envelope calling voiceFinished() is probably in the deletion queue and this Polyphonic already dead? Why does the slot not disconnect itself though..
                
                // this function is called from the audio thread, so we don't have to call AudioService::enqueueTask() to schedule disconnection on the audio thread
                mMixNodes[voice.mConnectedToChannels[channel]]->inputs.disconnect(*voice.getOutput()->getOutputForChannel(channel % voice.getOutput()->getChannelCount()));
            }
            voice.free();            
        }


    }

}
