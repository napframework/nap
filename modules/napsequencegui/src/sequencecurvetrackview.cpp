#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"

#include <nap/logger.h>

namespace nap
{
	SequenceCurveTrackView::SequenceCurveTrackView(SequenceEditorGUIView& view)
		: SequenceTrackView(view)
	{
		nap::Logger::info("curve track");
	}

	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceCurveTrackView), [](SequenceEditorGUIView& view)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceCurveTrackView>(view);
	});
}
