#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackcurve.h"

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

	static bool curveViewRegistrations[4]
	{
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec4>), RTTI_OF(SequenceCurveTrackView))
	};
}
