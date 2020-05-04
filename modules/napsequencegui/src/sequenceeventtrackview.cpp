#include "sequenceeventtrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackevent.h"

#include <nap/logger.h>

namespace nap
{
	SequenceEventTrackView::SequenceEventTrackView(SequenceEditorGUIView& view)
		: SequenceTrackView(view)
	{
		nap::Logger::info("event track");
	}


	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceEventTrackView), [](SequenceEditorGUIView& view)->std::unique_ptr<SequenceTrackView>
	{
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceEventTrackView));

		return std::make_unique<SequenceEventTrackView>(view);
	});
}
