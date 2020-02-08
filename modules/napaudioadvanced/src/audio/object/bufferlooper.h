#pragma once

#include <audio/object/bufferplayer.h>
#include <audio/object/gain.h>
#include <audio/core/polyphonicobject.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class BufferLooper;
        class BufferLooperInstance;
        
        
        class NAPAPI BufferLooper : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            class NAPAPI Settings
            {
            public:
                bool init(utility::ErrorState& errorState);
                
                ResourcePtr<AudioBufferResource> mBufferResource = nullptr; ///< property: 'Buffer'
                TimeValue mCrossFadeTime  = 1000.f;                         ///< property: 'CrossFadeTime'
                TimeValue mStart = 0.f;                                     ///< property: 'Start'
                TimeValue mLoopStart = 0.f;                                 ///< property: 'LoopStart'
                TimeValue mLoopEnd = 0.f;                                   ///< property: 'LoopEnd'
                ControllerValue mTranspose = 0.f;                           ///< property: 'Transpose' in semitones
                bool mLoop = true;                                          ///< property: 'Loop'
                
                TimeValue getLoopSustainDuration() const { return mLoopSustainDuration; }
                TimeValue getFirstSustainDuration() const { return mFirstSustainDuration; }
                
            private:
                TimeValue mLoopSustainDuration = 0.f;
                TimeValue mFirstSustainDuration = 0.f;
            };
            
        public:
            BufferLooper() : AudioObject() { }
            
            bool init(utility::ErrorState& errorState) override;
            
            Settings mSettings;                                         ///< property: 'Settings' All the playback settings
            int mChannelCount = 1;                                      ///< property: 'ChannelCount' Number of channels
            bool mAutoPlay = true;                                      ///< property: 'AutoPlay' Indicates wether playback will be started on init
            ResourcePtr<EqualPowerTable> mEqualPowerTable;              ///< property: 'EqualPowerTable'
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
        };
        
        
        class NAPAPI BufferLooperInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            BufferLooperInstance() : AudioObjectInstance() { }
            BufferLooperInstance(const std::string& name) : AudioObjectInstance(name) { }

            bool init(BufferLooper::Settings& settings, ResourcePtr<EqualPowerTable> equalPowerTable, int channelCount, bool autoPlay, NodeManager& nodeManager, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override { return mPolyphonicInstance->getOutputForChannel(channel); }
            int getChannelCount() const override { return mPolyphonicInstance->getChannelCount(); }

            void start();
            void start(BufferLooper::Settings& settings);
            void stop();
            
        private:
            BufferLooper::Settings mSettings;
            
            Slot<EnvelopeNode&> segmentFinishedSlot = {this, &BufferLooperInstance::segmentFinished };
            void segmentFinished(EnvelopeNode& envelope);
            
            void startVoice(bool fromStart);

            std::unique_ptr<PolyphonicObjectInstance> mPolyphonicInstance = nullptr;
            std::set<VoiceInstance*> mVoices;
            
            // private resources
            std::unique_ptr<Envelope> mEnvelope = nullptr;
            std::unique_ptr<BufferPlayer> mBufferPlayer = nullptr;
            std::unique_ptr<Gain> mGain = nullptr;
            std::unique_ptr<Voice> mVoice = nullptr;
            std::unique_ptr<PolyphonicObject> mPolyphonic = nullptr;
        };
        
        
    }
    
}
