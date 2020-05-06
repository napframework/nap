#pragma once

// Local Includes
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"

// External Includes
#include <imgui/imgui.h>

namespace nap
{
	struct SequenceEditorGUIState
	{
	public:
		// action 
		SequenceGUIActions::SequenceActionPtr mAction = nullptr;

		//
		bool mDirty = false;

		// previous mouse pos
		ImVec2 mPreviousMousePos;

		// window focused
		bool mIsWindowFocused = false;

		// mouse delte
		ImVec2 mMouseDelta;

		// current mouse position
		ImVec2 mMousePos;

		// current window position
		ImVec2 mWindowPos;

		// current window size
		ImVec2 mWindowSize;

		// current timeline controller position
		ImVec2 mTimelineControllerPos;

		// current timelinewidth
		float mTimelineWidth;

		// stepsize ( how many pixels per second ? )
		float mStepSize;

		// current trackheight / vertical resolution
		float mTrackHeight;

		// width of inspector box
		float mInspectorWidth;

		// previous window position
		ImVec2 mPrevWindowPos;

		// previous window scroll
		ImVec2 mPrevScroll;

		// vertical resolution
		float mVerticalResolution = 150.0f;

		// horizontal resolution
		float mHorizontalResolution = 100.0f;

		// current time in sequence of mouse cursor
		double mMouseCursorTime;

		// current cursor pos
		ImVec2 mCursorPos;
	};
}