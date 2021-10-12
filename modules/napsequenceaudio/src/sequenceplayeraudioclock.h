
#pragma once

#include "sequenceserviceaudio.h"

#include <audio/utility/audiotypes.h>
#include <sequenceplayerclock.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceServiceAudio;

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
		/**
		 * onUpdate called by audio thread
		 * @param timeValue
		 */
		void onUpdate(audio::DiscreteTimeValue timeValue);

		// the slot connected to the update signal of the node manager
		Slot<audio::DiscreteTimeValue> mSlot;

		// reference to SequenceAudioService
		SequenceServiceAudio& 	mService;

		// the previous time, used to calculate delta time
		audio::DiscreteTimeValue mStartTime;

		// the current sample rate
		float mSampleRate;

		// update slot, updates sequence player
		Slot<double> mUpdateSlot;
	};

	// shortcut to factory function
	using SequencePlayerAudioClockObjectCreator = rtti::ObjectCreator<SequencePlayerAudioClock, SequenceServiceAudio>;
}