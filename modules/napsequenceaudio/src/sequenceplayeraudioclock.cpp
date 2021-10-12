#include "sequenceplayeraudioclock.h"

#include <nap/core.h>
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioClock)
RTTI_END_CLASS

namespace nap
{
	SequencePlayerAudioClock::SequencePlayerAudioClock(SequenceServiceAudio& service) : mService(service)
	{
		mSlot = Slot<audio::DiscreteTimeValue>(this, &SequencePlayerAudioClock::onUpdate);
	}


	void SequencePlayerAudioClock::start(Slot<double>& updateSlot)
	{
		mUpdateSlot = updateSlot;

		auto* audio_service = mService.getCore().getService<audio::AudioService>();
		assert(audio_service!= nullptr);

		audio_service->enqueueTask([this, audio_service]()
		{
			auto& node_manager = audio_service->getNodeManager();

			mStartTime = node_manager.getSampleTime();
			mSampleRate = node_manager.getSampleRate();
			node_manager.mUpdateSignal.connect(mSlot);
		});
	}


	void SequencePlayerAudioClock::stop()
	{
		auto* audio_service = mService.getCore().getService<audio::AudioService>();
		assert(audio_service!= nullptr);

		audio_service->enqueueTask([this, audio_service]()
		{
			auto& node_manager = audio_service->getNodeManager();
			node_manager.mUpdateSignal.disconnect(mSlot);
		});
	}


	void SequencePlayerAudioClock::onUpdate(audio::DiscreteTimeValue timeValue)
	{
		audio::DiscreteTimeValue elapsed_samples = timeValue - mStartTime;
		double delta_time = static_cast<double>(elapsed_samples) / (double)mSampleRate;
		mStartTime = timeValue;
		mUpdateSlot.trigger(delta_time);
	}
}
