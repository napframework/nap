#pragma once

// internal includes
#include "sequenceeditor.h"
#include "sequenceeditorview.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	
	enum SequenceGUIMouseActions
	{
		// HOVERING
		HOVERING_SEGMENT,
		// ACTIONS
		DRAGGING_SEGMENT,
		NONE
	};

	struct SequenceGUIState
	{
	public:
		SequenceGUIMouseActions currentAction	= SequenceGUIMouseActions::NONE;
		std::string currentObjectID		= "";
	};
	

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
	protected:
		std::unique_ptr<SequenceEditorView> mView = nullptr;

		SequenceGUIState mState;

		ImVec2 mPreviousMousePos = { 0,0 };
	};
}
