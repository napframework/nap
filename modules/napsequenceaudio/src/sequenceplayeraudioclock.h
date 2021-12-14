#pragma once

#include "sequenceserviceaudio.h"

#include <audio/utility/audiotypes.h>
#include <sequenceplayerclock.h>
#include <audio/core/process.h>
#include <concurrentqueue.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceServiceAudio;
    class SequencePlayerAudioClockProcess;

    /**
     * The SequencePlayerAudioClock receives updates from the AudioService thread
     * The interval depends on the Audio Buffer size, the smaller the audio buffer size, the more updates per second
     * Use this clock when you want to use audio track types with the sequencer
     */
    class NAPAPI SequencePlayerAudioClock final : public SequencePlayerClock
    {
        RTTI_ENABLE(SequencePlayerClock);
    public:
        /**
         * Constructor
         * @param service reference to SequenceServiceAudio
         */
        SequencePlayerAudioClock(SequenceServiceAudio& service);

        /**
         * onDestroy calls stop disconnecting update slot from created SequencePlayerAudioClockProcess
         */
        void onDestroy() override;

        /**
         * Called on start of the sequence player
         * @param updateSlot
         */
        void start(Slot<double>& updateSlot) override;

        /**
         * Called on stop of the sequence player
         * @param updateSlot
         */
        void stop() override;

    private:
        // reference to SequenceAudioService
        SequenceServiceAudio& mService;

        // update slot, updates sequence player
        Slot<double> mUpdateSlot;

        // the audio clock process
        audio::SafeOwner<SequencePlayerAudioClockProcess> mAudioClockProcess;
    };

    // shortcut to factory function
    using SequencePlayerAudioClockObjectCreator = rtti::ObjectCreator<SequencePlayerAudioClock, SequenceServiceAudio>;

    /**
     * SequencePlayerAudioClockProcess registers itself to the NodeManager as a root process. When process callback is
     * triggered it calculates the delta time in seconds since last call and triggers mUpdateSignal calling all registered
     * slots
     */
    class NAPAPI SequencePlayerAudioClockProcess final : public audio::Process
    {
    RTTI_ENABLE(audio::Process)
    public:
        SequencePlayerAudioClockProcess(audio::NodeManager& nodeManager);

        /*
         * We need to delete these so that the compiler doesn't try to use them. Otherwise we get compile errors on vector<unique_ptr>.
         */
        SequencePlayerAudioClockProcess(const SequencePlayerAudioClockProcess&) = delete;

        SequencePlayerAudioClockProcess& operator=(const SequencePlayerAudioClockProcess&) = delete;

        /**
         * Deconstructor, unregisters the process from node manager
         */
        virtual ~SequencePlayerAudioClockProcess();

        /**
         * Connect update slot on audio-thread.
         * Thread-Safe
         * @param slot the update slot
         */
        void connectSlot(Slot<double>& slot);

        /**
         * Disconnect update slot on audio-thread.
         * Thread-Safe
         * @param slot the update slot
         */
        void disconnectUpdateSlot(Slot<double>& slot);

    public:
        // Signals
        Signal<double> mUpdateSignal;
    protected:
        /**
         * process calculates delta-time since last call and triggers update signal
         */
        void process() override;

    private:
        // last known timestamp in samples
        audio::DiscreteTimeValue mTime;

        // the current sample rate
        float mSampleRate;

        // queue of tasks to perform on process()
        moodycamel::ConcurrentQueue<std::function<void()>> mTasks;
    };

}