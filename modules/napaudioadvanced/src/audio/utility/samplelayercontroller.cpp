#include "samplelayercontroller.h"

namespace nap
{

    namespace audio
    {

        SampleLayerController::SampleLayerController(SamplerInstance& sampler) : mSampler(sampler), mEnvelopeData(sampler.getEnvelopeData())
        {
        }


        void SampleLayerController::startLayer(int samplerEntryIndex, TimeValue attack)
        {
            // If the layer is already playing, exit
            if (mLayerVoices.find(samplerEntryIndex) != mLayerVoices.end())
                return;

            auto& envelope = mSampler.getEnvelopeData();
            envelope.resize(1);
            envelope[0].mTranslate = false;
            envelope[0].mDestination = 1.f;
            envelope[0].mDuration = attack;
            auto voice = mSampler.play(samplerEntryIndex, 0);
            mLayerVoices[samplerEntryIndex] = voice;
        }


        void SampleLayerController::play(int samplerEntryIndex, TimeValue attack, TimeValue sustain, TimeValue release)
        {
            auto& envelope = mSampler.getEnvelopeData();
            envelope.resize(3);
            envelope[0].mTranslate = false;
            envelope[0].mDestination = 1.f;
            envelope[0].mDuration = attack;
            envelope[1].mTranslate = false;
            envelope[1].mDestination = 1.f;
            envelope[1].mDuration = sustain;
            envelope[2].mTranslate = false;
            envelope[2].mDestination = 0.f;
            envelope[2].mDuration = release;
            mSampler.play(samplerEntryIndex, 0);
        }


        void SampleLayerController::stopLayer(int samplerEntryIndex, TimeValue fadeOutTime)
        {
            auto it = mLayerVoices.find(samplerEntryIndex);
            if (it != mLayerVoices.end())
            {
                mSampler.stop(it->second, fadeOutTime);
            }
            mLayerVoices.erase(it);
        }


        void SampleLayerController::stopAllLayers(TimeValue release)
        {
            for (auto& pair : mLayerVoices)
                mSampler.stop(pair.second, release);
            mLayerVoices.clear();
        }


        void SampleLayerController::replaceLayers(std::set<int> samplerEntryIndices, TimeValue attack, TimeValue release)
        {
            for (auto& samplerEntryIndex : samplerEntryIndices)
            {
                startLayer(samplerEntryIndex, attack);
            }

            std::set<int> toBeStopped;
            for (auto& pair : mLayerVoices)
            {
                if (samplerEntryIndices.find(pair.first) == samplerEntryIndices.end())
                    toBeStopped.emplace(pair.first);
            }

            for (auto& entry : toBeStopped)
                stopLayer(entry, release);
        }


        std::set<int> SampleLayerController::getLayers()
        {
            std::set<int> result;
            for (auto& pair : mLayerVoices)
                result.emplace(pair.first);
            return result;
        }


    }

}