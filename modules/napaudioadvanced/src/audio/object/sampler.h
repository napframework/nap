#pragma once

#include <audio/object/bufferlooper.h>

namespace nap
{
    
    namespace audio
    {
        
        class NAPAPI Sampler : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            using SamplerEntries = std::vector<BufferLooper::Settings>;
            
        public:
            Sampler() : AudioObject() { }
            
            bool init(utility::ErrorState& errorState) override;
            
            SamplerEntries mSampleEntries;                              ///< property: 'SampleEntries' Default set of different playback settings
            EnvelopeNode::Envelope mEnvelopeData;                  ///< property: 'Envelope' Default envelope settings
            int mChannelCount = 1;                                      ///< property: 'ChannelCount' Number of channels
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        
        
        class NAPAPI SamplerInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            SamplerInstance() : AudioObjectInstance() { }
            SamplerInstance(const std::string& name) : AudioObjectInstance(name) { }
            
            bool init(Sampler::SamplerEntries& sampleEntries, EnvelopeNode::Envelope& envelopeData, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }
            
            VoiceInstance* play(unsigned int samplerEntryIndex, TimeValue duration);
            void stop(VoiceInstance* voice, TimeValue release = 0);
            
            const Sampler::SamplerEntries& getSamplerEntries() const { return mSamplerEntries; }
            EnvelopeNode::Envelope& getEnvelopeData() { return mEnvelopeData; }
            
        private:
            Sampler::SamplerEntries mSamplerEntries;
            EnvelopeNode::Envelope mEnvelopeData;
            
            std::unique_ptr<PolyphonicObjectInstance> mPolyphonicInstance = nullptr;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferLooper> mBufferLooper = nullptr;
            std::unique_ptr<Gain> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<PolyphonicObject> mPolyphonic = nullptr;
        };
        
        
    }
    
}
