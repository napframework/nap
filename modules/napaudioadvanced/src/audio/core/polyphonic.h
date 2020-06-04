#pragma once

// Std includes
#include <mutex>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h>
#include <audio/core/voice.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PolyphonicInstance;
        
        
        /**
         * This object manages a pool of voices for polyphonic playback.
         * The most common use would be musical instrument emulation.
         * For each "note" played on the isntrument one voice in the pool will be used.
         * When the playback of the note is finished the voice will be disconnected from the DSP processing graph.
         */
        class NAPAPI Polyphonic : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            Polyphonic() : AudioObject() { }

            /**
             * This points to the voice graph resource defining the patch for a single voice in the polyphonic system.
             */
            ResourcePtr<Voice> mVoice;
            
            /**
             * Number of voices in the voice pool. This indicates the maximum number of voices playing at the same time.
             */
            int mVoiceCount = 1;
            
            /**
             * If set to true, every time the user tries to play more voices than there are present in the pool,
             * the voice that has been playing for the longest time will be "stolen" and used to perform the new play command.
             */
            bool mVoiceStealing = true;
            
            /**
             * The number of channels that the object outputs.
             * Beware that this dos not to be equal to the number of channels of the voice, as it is possible to play a voice on a specific set of output channels of the polyphonic object. See also @PolyphonicObjectInstance::playOnChannels().
             */
            int mChannelCount = 1;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };

        
        /**
         * Instance of object manages a pool of voices for polyphonic playback.
         */
        class NAPAPI PolyphonicInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            PolyphonicInstance() : AudioObjectInstance() { }
            PolyphonicInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Initialize the object
            bool init(Voice& voice, int voiceCount, bool voiceStealing, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            /**
             * Returns the first voice in the pool that is not being used (Voice::isBusy() == false) for playback.
             * Before a voice is returned by this method it will already be marked as busy.
             * Once the envelope of the voice has been played and finished the voice will be freed again.
             */
            VoiceInstance* findFreeVoice();
            
            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be aqcuired and reserved for use using findFreeVoice().
             */
            void play(VoiceInstance* voice, TimeValue duration = 0);

            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be aqcuired and reserved for use using findFreeVoice().
             */
            void playSection(VoiceInstance* voice, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Starts playing a voice by calling it's play() method and connecting it's output to this object's mixer.
             * Before being passed to this method a voice has te be aqcuired and reserved for use using findFreeVoice().
             * As opposed to @play() this method connects the voice to the specified @channels of this polyphic object's output mixer.
             */
            void playOnChannels(VoiceInstance* voice, std::vector<unsigned int> channels, TimeValue duration = 0);
            
            /**
             * Stops playing the voice by telling it to fade out it's envelope
             * Once the envelope is faded out this will trigger the voice to be disconnected from this object's mixers
             */
            void stop(VoiceInstance* voice, TimeValue fadeOutTime = 0);
            
            /**
             * Counts the number of voices that are currently playing.
             */
            int getBusyVoiceCount() const;
            
        private:
            void connectVoice(VoiceInstance* voice);

            Slot<VoiceInstance&> voiceFinishedSlot = { this, &PolyphonicInstance::voiceFinished };
            void voiceFinished(VoiceInstance& voice);
            
            std::vector<std::unique_ptr<VoiceInstance>> mVoices;
            std::vector<SafeOwner<MixNode>> mMixNodes;
            
            NodeManager* mNodeManager = nullptr;
            bool mVoiceStealing = true;
        };
        
    }
    
}
