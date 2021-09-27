/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"
#include "sequenceeditorguiclipboard.h"

// External Includes
#include <imgui/imgui.h>

namespace nap
{
	/**
	 * Holds information about the state of the editor
	 * Shared between GUI editor and view classes
	 */
	struct SequenceEditorGUIState
	{
	public:
		// action 
		SequenceGUIActions::SequenceActionPtr mAction = nullptr;

		// clipboard
		SequenceGUIClipboards::SequenceClipboardPtr mClipboard = nullptr;

		// dirty means view has changed, and content of tracks need to be redrawn and cached
		bool mDirty = false;

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

		// stepsize amount pixels per second
		float mStepSize;

		// current trackheight / vertical resolution
		float mTrackHeight;

		// width of inspector box
		float mInspectorWidth;

		// previous window position
		ImVec2 mPrevWindowPos;

		// previous window scroll
		ImVec2 mScroll;

		// vertical resolution (not scaled)
		float mVerticalResolution = 150.0f;

		// horizontal resolution (not scaled)
		float mHorizontalResolution = 100.0f;

		// current time in sequence of mouse cursor
		double mMouseCursorTime;

		// current cursor pos
		ImVec2 mCursorPos;

		// scroll follows player position
		bool mFollow = false;

		// current gui scaling factor 
		float mScale = 1.0f;

		// current sequence name
		std::string mSequenceName;
	};
}
