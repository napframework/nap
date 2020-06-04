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
            EnvelopeNode::Envelope mEnvelopeData;                       ///< property: 'Envelope' Default envelope settings
            int mChannelCount = 1;                                      ///< property: 'ChannelCount' Number of channels
            int mVoiceCount = 10;                                       ///< property: 'VoiceCount' Number of voices in the pool.
            ResourcePtr<EqualPowerTable> mEqualPowerTable = nullptr;    ///< property: 'EqualPowerTable'
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        
        
        class NAPAPI SamplerInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            SamplerInstance() : AudioObjectInstance() { }
            SamplerInstance(const std::string& name) : AudioObjectInstance(name) { }

            /**
             * @param sampleEntries sample entries that can be played by this sampler
             * @param envelopeData default envelope data
             * @param channelCount number of output channels of the sampler
             * @param nodeManager node manager this sampler runs on
             * @param errorState contains error information if the init() fails
             * @return true on success
             */
            bool init(Sampler::SamplerEntries& sampleEntries, ResourcePtr<EqualPowerTable> equalPowerTable, EnvelopeNode::Envelope& envelopeData, int channelCount, int voiceCount, NodeManager& nodeManager, utility::ErrorState& errorState);

            // Inhrited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }

            /**
             * Plays back sampler entry with given index for given duration.
             * Uses envelope data that can be accessed through @getEnvelopeData()
             * Returns a voice that is playing back the entry.
             */
            VoiceInstance* play(unsigned int samplerEntryIndex, TimeValue duration);

            VoiceInstance* playSection(unsigned int samplerEntryIndex, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Fades out the given voice over @releaseTime and stops it.
             */
            void stop(VoiceInstance* voice, TimeValue release = 0);

            /**
             * @return all the available sampler entries. Non editable.
             */
            const Sampler::SamplerEntries& getSamplerEntries() const { return mSamplerEntries; }

            /**
             * @return the envelope data that will be passed to the next voice to be played. Result can be edited to affect the next play() statement.
             */
            EnvelopeNode::Envelope& getEnvelopeData() { return mEnvelopeData; }
            
        private:
            Sampler::SamplerEntries mSamplerEntries;
            EnvelopeNode::Envelope mEnvelopeData;
            
            std::unique_ptr<PolyphonicInstance> mPolyphonicInstance = nullptr;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferLooper> mBufferLooper = nullptr;
            std::unique_ptr<Gain> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<Polyphonic> mPolyphonic = nullptr;
        };
        
        
    }
    
}
