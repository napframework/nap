#pragma once

#include "sequencetrackview.h"

namespace nap
{
	class SequenceEventTrackView : public SequenceTrackView
	{
	public:
		SequenceEventTrackView(SequenceEditorGUIView& view);

		virtual void drawTrack(SequenceTrack& track) override {}

		virtual void handleActions() override {}
	};
}