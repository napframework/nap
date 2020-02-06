#pragma once

#include <atomic>

#include <audio/core/graph.h>
#include <audio/object/envelope.h>


namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class PolyphonicObjectInstance;
        
        
        /**
         * A specific type of graph that is used to define a DSP network for one voice of processing in a polyphonic system.
         */
        class Voice : public Graph
        {
            RTTI_ENABLE(Graph)
            
        public:
            Voice() : Graph()  { }
            
            /**
             * Points to an envelope within the graph that controls the amplitude of a single audio event processed by the voice.
             * When the voice is played this envelope will be triggered. When it has finished it emits a signal that will cause the voice to be disconnected and enter idle state again.
             */
            ResourcePtr<Envelope> mEnvelope = nullptr;
            
        private:
        };
        
        
        /**
         * Instance of a Voice graph representing one voice within a polyphonic system.
         */
        class VoiceInstance : public GraphInstance
        {
            RTTI_ENABLE(GraphInstance)
            
            friend class PolyphonicObjectInstance;
            
        public:
            bool init(Voice& resource, NodeManager& nodeManager, utility::ErrorState& errorState);
            
            /**
             * @return: the envelope controlling the overall amplitude of the voice
             */
            EnvelopeInstance& getEnvelope() { return *mEnvelope; }
            
            /**
             * @return: the envelope controlling the overall amplitude of the voice
             */
            const EnvelopeInstance& getEnvelope() const { return *mEnvelope; }

            /**
             * Starts playback of the voice by triggering the envelope
             */
            void play(TimeValue duration = 0);
            
            /**
             * Stops playback of the voice by forcing the envelope to fade out
             */
            void stop(TimeValue rampTime = 0);
            
            /**
             * @return: wether this voice is currently playing or reserved for usage.
             */
            bool isBusy() const { return mBusy; }
            
            /**
             * @return: when the voice is busy, the time the voice started playing
             */
            DiscreteTimeValue getStartTime() const { return mStartTime; }

            /**
             * Signal that is emitted when the voice has finished playing.
             */
            nap::Signal<VoiceInstance&> finishedSignal;
            
            /**
             * Function to retrieve the @finishedSignal using RTTR
             */
            nap::Signal<VoiceInstance&>* getFinishedSignal() { return &finishedSignal; }
            
        private:
            // Used internally by PolyphincObjectInstance to try to reserve the voice for usage
            bool try_use();
            void free();
            
            // Responds to the signal emitted by the envelope generator of the main envelope by emitting the finishedSignal.
            Slot<EnvelopeNode&> envelopeFinishedSlot = {this, &VoiceInstance::envelopeFinished };
            void envelopeFinished(EnvelopeNode&);

            EnvelopeInstance* mEnvelope = nullptr;
            std::atomic<bool> mBusy = { false };
            DiscreteTimeValue mStartTime = 0;
            
            // This set caches the channels of the output mixer of the polyphonic object that this voice is connected to before it was started to play. When playing is done the polyphonic object will take care of disconnecting the voice from these channels.
            std::vector<int> mConnectedToChannels = { };
        };
        
    }
    
}
