#pragma once

#include "sequencetrackview.h"

namespace nap
{
	class SequenceCurveTrackView : public SequenceTrackView
	{
	public:
		SequenceCurveTrackView(SequenceEditorGUIView& view);

		virtual void drawTrack(const SequenceTrack& track) override {}

		virtual void handleActions() override {}
	};
}
