#pragma once

// Audio includes
#include <core/audioobject.h>
#include <core/voice.h>
#include <node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class PolyphonicObjectInstance;
        
        
        /**
         * This object manages a pool of voices for polyphonic playback.
         * The most common use would be musical instrument emulation.
         * For each "note" played on the isntrument one voice in the pool will be used.
         * When the playback of the note is finished the voice will be disconnected from the DSP processing graph.
         */
        class NAPAPI PolyphonicObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
            
        public:
            PolyphonicObject() : AudioObject() { }
            
            /**
             * This points to the voice graph resource defining the patch for a single voice in the polyphonic system.
             */
            ObjectPtr<Voice> mVoice;
            
            /**
             * Number of voices in the voice pool. This indicates the maximum number of voices playing at the same time.
             */
            int mVoiceCount = 1;
            
            /**
             * If set to true, every time the user tries to play more voices than there are present in the pool,
             * the voice that has been playing for the longest time will be "stolen" and used to perform the new play command.
             */
            bool mVoiceStealing = true;
            
        private:
            std::unique_ptr<AudioObjectInstance> createInstance() override;
        };

        
        /**
         * Instance of object manages a pool of voices for polyphonic playback.
         */
        class NAPAPI PolyphonicObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
            
        public:
            PolyphonicObjectInstance(PolyphonicObject& resource) : AudioObjectInstance(resource) { }
            
            // Initialize the component
            bool init(NodeManager& nodeManager, utility::ErrorState& errorState) override;
            
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
            void play(VoiceInstance* voice);
            
            /**
             * Stops playing the voice by telling it to fade out it's envelope
             * Once the envelope is faded out this will trigger the voice to be disconnected from this object's mixers
             */
            void stop(VoiceInstance* voice);
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            void voiceFinished(VoiceInstance& voice);
            
            std::vector<std::unique_ptr<VoiceInstance>> mVoices;
            std::vector<std::unique_ptr<MixNode>> mMixNodes;
            
            NodeManager* mNodeManager = nullptr;
        };
        
    }
    
}
