#pragma once

// external includes
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class SequenceEditorView
	{
	protected:
		// Signals
		Signal<const SequenceTrack&, const SequenceTrackSegment&, float> mSegmentDurationChange;
		Signal<> mSave;
	public:
		// events
		void dispatchSegmentDurationChange(const SequenceTrack& track, const SequenceTrackSegment& segment, float amount)
		{
			mSegmentDurationChange.trigger(track, segment, amount);
		}

		void dispatchSave()
		{
			mSave.trigger();
		}
	public:
		// registration
		void registerSegmentDurationChangeSlot(Slot<const SequenceTrack&, const SequenceTrackSegment&, float>& slot)
		{
			mSegmentDurationChange.connect(slot);
		}

		void registerSaveSlot(Slot<>& slot)
		{
			mSave.connect(slot);
		}

		// unregistration
		void unregisterSegmentDurationChangeSlot(Slot<const SequenceTrack&, const SequenceTrackSegment&, float>& slot)
		{
			mSegmentDurationChange.disconnect(slot);
		}

		void unregisterSaveSlot(Slot<> slot)
		{
			mSave.disconnect(slot);
		}
	};
}
