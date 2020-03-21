#pragma once

// internal includes
#include "sequenceeditor.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/*
	enum SequenceGUIMouseActions
	{
		// HOVERING
		HOVERING_KEYFRAME,
		HOVERING_KEYFRAMEVALUE,
		// ACTIONS
		DRAGGING_KEYFRAME,
		DRAGGING_KEYFRAMEVALUE,
		NONE
	};

	struct SequenceGUIMouseActionData
	{
	public:
		bool	mouseWasDown					= false;
		ImVec2	previousMousePos				= ImVec2(0, 0);
		SequenceGUIMouseActions currentAction	= SequenceGUIMouseActions::NONE;
		rtti::Object* currentObject				= nullptr;
	};
	*/

	/**
	 */
	class NAPAPI SequenceEditorGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);

		virtual void onDestroy();

		void draw();
	public:
		// Signals
		Signal<const SequenceTrack&, const SequenceTrackSegment&, float> mSegmentDurationChange;
	public:
		ResourcePtr<SequenceEditor> mSequenceEditor = nullptr;
	};
}
