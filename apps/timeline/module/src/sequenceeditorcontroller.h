#pragma once

// external includes
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI SequenceEditorController 
	{
		RTTI_ENABLE(Resource)
	public:
		void registerView(SequenceEditorView* sequenceEditorView);

		void unregisterView(SequenceEditorView* sequenceEditorView);
	protected:
		std::list<SequenceEditorView*> mViews;

		// slots
		Slot<const SequenceTrack&, const SequenceTrackSegment&, float> mSegmentDurationChangeSlot;
		void segmentDurationChange(const SequenceTrack& track, const SequenceTrackSegment& segment, float amount);
	};
}
