#pragma once

// internal includes
#include "timelineholder.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>
#include <imgui/imgui.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	enum TimelineGUIMouseActions
	{
		DRAGGING_KEYFRAME,
		DRAGGING_KEYFRAMEVALUE,
		NONE
	};

	struct TimelineGUIMouseActionData
	{
	public:
		bool	mouseWasDown					= false;
		ImVec2	previousMousePos				= ImVec2(0, 0);
		TimelineGUIMouseActions currentAction	= TimelineGUIMouseActions::NONE;
		rtti::Object* currentObject				= nullptr;
	};

	/**
	 */
	class NAPAPI TimelineGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		void draw();

		std::string getName() const;

		virtual bool init(utility::ErrorState& errorState) override;
	public:
		ResourcePtr<TimelineHolder>						mTimelineHolder;
	protected:
		TimelineGUIMouseActionData							mMouseActionData;
	};
}
