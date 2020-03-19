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
	class NAPAPI SequenceGUI : public Resource
	{
		RTTI_ENABLE(Resource)
		/*
	public:
		
		void draw();

		virtual bool init(utility::ErrorState& errorState) override;
	public:
		ResourcePtr<SequenceEditor>						mTimelineContainer;
	protected:
		SequenceGUIMouseActionData						mMouseActionData;
	private:

		void drawTrack(
			const ResourcePtr<SequenceTrack>& track,
			ImVec2 &cursorPos,
			const int trackCount,
			const float timelineWidth,
			const float stepSize);

		void drawKeyFrame(
			const ResourcePtr<SequenceTrackSegment> &keyFrame,
			const float stepSize,
			ImDrawList* drawList,
			const ImVec2 &trackTopLeft,
			const float trackHeight,
			float &previousKeyFrameX);
			*/
	};
}
