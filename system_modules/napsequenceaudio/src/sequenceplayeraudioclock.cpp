#include "sequenceplayeraudioclock.h"

#include <nap/core.h>
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioClock, "Updates a sequence using the audio thread")
RTTI_END_CLASS

namespace nap
{
    SequencePlayerAudioClock::SequencePlayerAudioClock(SequenceServiceAudio& service)
            :mService(service)
    {
    }


    void SequencePlayerAudioClock::start(Slot<double>& updateSlot)
    {
        mUpdateSlot = updateSlot;

        auto* audio_service = mService.getCore().getService<audio::AudioService>();
        assert(audio_service!=nullptr);

        auto& node_manager = audio_service->getNodeManager();
        mAudioClockProcess = node_manager.makeSafe<SequencePlayerAudioClockProcess>(node_manager);

        // Register the clock to be processed directly by the NodeManager
        node_manager.registerRootProcess(mAudioClockProcess.get());

        mAudioClockProcess->connectSlot(mUpdateSlot);
    }


    void SequencePlayerAudioClock::onDestroy()
    {
        auto* audio_service = mService.getCore().getService<audio::AudioService>();
        assert(audio_service!=nullptr);

        // Unregister the clock from being processed directly by the NodeManager
        audio_service->getNodeManager().unregisterRootProcess(mAudioClockProcess.get());
        
        stop();
    }


    void SequencePlayerAudioClock::stop()
    {
        mAudioClockProcess->disconnectUpdateSlot(mUpdateSlot);
    }


    SequencePlayerAudioClockProcess::SequencePlayerAudioClockProcess(audio::NodeManager& nodeManager)
            :audio::Process(nodeManager)
    {
        mTime = nodeManager.getSampleTime();
    }


    void SequencePlayerAudioClockProcess::connectSlot(Slot<double>& slot)
    {
        mTasks.enqueue([this, &slot]()
        {
            mUpdateSignal.connect(slot);
        });
    }


    void SequencePlayerAudioClockProcess::disconnectUpdateSlot(Slot<double>& slot)
    {
        mTasks.enqueue([this, &slot]()
        {
            mUpdateSignal.disconnect(slot);
        });
    }


    void SequencePlayerAudioClockProcess::process()
    {
        // execute any connecting or disconnecting slots on this thread
        while (mTasks.size_approx()>0)
        {
            std::function<void()>task;
            if (mTasks.try_dequeue(task))
            {
                task();
            }
        }

        // calculate delta time in seconds
        const auto& now = getNodeManager().getSampleTime();
        audio::DiscreteTimeValue elapsed_samples = now-mTime;
        double delta_time = 0.0;
        if(elapsed_samples > 0)
            delta_time = static_cast<double>(elapsed_samples)/static_cast<double>(getNodeManager().getSampleRate());
        mTime = now;

        // dispatch delta time
        mUpdateSignal.trigger(delta_time);
    }
}
