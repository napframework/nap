// local includes
#include "sequenceeditorgui.h"
#include "napcolors.h"

// External Includes
#include <entity.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS(nap::SequenceEditorGUI)
RTTI_PROPERTY("Sequence Editor", &nap::SequenceEditorGUI::mSequenceEditor, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SequenceEditorGUI::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		mSequenceEditor->registerGUI(this);

		return true;
	}


	void SequenceEditorGUI::onDestroy()
	{
		mSequenceEditor->unregisterGUI(this);
	}


	void SequenceEditorGUI::draw()
	{
		//
		const Sequence& sequence = mSequenceEditor->getSequence();

		//
		const SequencePlayer& sequencePlayer = *mSequenceEditor->mSequencePlayer.get();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		float stepSize = 100.0f;

		// calc width of content in timeline window
		float timelineWidth =
			stepSize * sequencePlayer.getDuration();

		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth);

		// get current cursor pos, we will use this to position the track windows
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// begin window
		if (ImGui::Begin(
			mID.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			// check if window has focus
			bool windowHasFocus = ImGui::IsWindowFocused();

			int trackCount = 0;
			for (const auto& trackLink : sequence.mSequenceTrackLinks)
			{
				// push id
				ImGui::PushID(trackLink->mID.c_str());

				// define consts
				const float trackHeight = 100.0f;
				const float keyframeHandlerHeight = 10.0f;

				// manually set the cursor position before drawing new track window
				cursorPos = { cursorPos.x, cursorPos.y + trackHeight * trackCount + 1 };
				ImGui::SetCursorPos(cursorPos);

				// get child focus
				bool trackHasFocus = ImGui::IsMouseHoveringWindow();

				// get window drawlist
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// get current imgui cursor position
				ImVec2 cursorPos = ImGui::GetCursorPos();

				// get window position
				ImVec2 windowTopLeft = ImGui::GetWindowPos();

				// calc beginning of timeline graphic
				ImVec2 trackTopLeft = { windowTopLeft.x + cursorPos.x, windowTopLeft.y + cursorPos.y };

				// draw background of timeline
				drawList->AddRectFilled(
					trackTopLeft, // top left position
					{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }, // bottom right position
					guicolors::black); // color 

				// draw segments
				for (const auto& segment : trackLink->mSequenceTrack->mSegments)
				{
					float x = ( segment->mStartTime + segment->mDuration ) * stepSize;

					// draw handler of segment duration
					drawList->AddLine(
						{ trackTopLeft.x + x, trackTopLeft.y }, // top left
						{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
						guicolors::white, // color
						3.0f); // thickness
				}

				// pop id
				ImGui::PopID();

				// increment track count
				trackCount++;
			}

			ImGui::End();
		}

		// pop id
		ImGui::PopID();
	}

	/*
	void SequenceGUI::draw()
	{
		//
		Sequence& sequence = *mTimelineContainer->mTimeline.get();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		float stepSize = 100.0f;

		// calc width of content in timeline window
		float timelineWidth = stepSize * sequence.mDuration + 150.0f;

		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth);

		// begin window
		if (ImGui::Begin(
			sequence.mID.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			// check if window has focus
			bool windowHasFocus = ImGui::IsWindowFocused();

			// handle save button
			if (ImGui::Button("Save"))
			{
				utility::ErrorState errorState;
				sequence.save("test.json", errorState);
			}

			// get current cursor pos, we will use this to position the track windows
			ImVec2 cursorPos = ImGui::GetCursorPos();

			// keep track of track count, we will use this to position the track windows
			int trackCount = 0;

			// draw tracks
			for (const auto& track : sequence.mTracks)
			{
				// draw the track
				drawTrack(
					track, // track resource ptr ref
					cursorPos, // cursor position
					trackCount, // track count
					timelineWidth, // width of timeline
					stepSize); // stepsize

				// increment track count
				trackCount++;
			}
		}

		// end window
		ImGui::End();

		// pop id
		ImGui::PopID();
	}


	bool SequenceGUI::init(utility::ErrorState & errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}


	void SequenceGUI::drawTrack(
		const ResourcePtr<SequenceTrack>& track,
		ImVec2 &cursorPos,
		const int trackCount,
		const float timelineWidth,
		const float stepSize)
	{
		// push id
		ImGui::PushID(track->mID.c_str());

		// define consts
		const float trackHeight = 100.0f;
		const float keyframeHandlerHeight = 10.0f;

		// manually set the cursor position before drawing new track window
		cursorPos = { cursorPos.x, cursorPos.y + trackHeight * trackCount + 1 };
		ImGui::SetCursorPos(cursorPos);

		// begin track
		if (ImGui::BeginChild(
			track->mID.c_str(), // id
			{ timelineWidth, trackHeight + keyframeHandlerHeight }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// push id
			ImGui::PushID(track->mID.c_str());

			// get child focus
			bool trackHasFocus = ImGui::IsMouseHoveringWindow();

			// get window drawlist
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// get current imgui cursor position
			ImVec2 cursorPos = ImGui::GetCursorPos();

			// get window position
			ImVec2 windowTopLeft = ImGui::GetWindowPos();

			// calc beginning of timeline graphic
			ImVec2 trackTopLeft = { windowTopLeft.x + cursorPos.x, windowTopLeft.y + cursorPos.y };

			// draw background of timeline
			drawList->AddRectFilled(
				trackTopLeft, // top left position
				{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }, // bottom right position
				guicolors::black); // color 

			// keep track of previous position of keyframe
			float previousKeyFrameX = 0.0f;

			// draw keyframes
			for (const auto& keyFrame : track->mSegments)
			{
				drawKeyFrame(
					keyFrame, // keyframe resource ptr
					stepSize, // stepsize
					drawList, // the current window drawlist
					trackTopLeft, // the topleft position of the track in which the keyframe is located
					trackHeight, // the trackheight
					previousKeyFrameX); // previous position of keyframe on timeline
			}

			// pop id
			ImGui::PopID();
		}

		// end track
		ImGui::EndChild();

		// pop id
		ImGui::PopID();
	}


	void SequenceGUI::drawKeyFrame(
		const ResourcePtr<SequenceTrackSegment> &keyFrame,
		const float stepSize, 
		ImDrawList* drawList,
		const ImVec2 &trackTopLeft,
		const float trackHeight,
		float &previousKeyFrameX)
	{
		// position of keyframe
		float x = keyFrame->mTime * stepSize;

		// draw keyframe value handler
		drawList->AddCircle(
			{ trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue }, // position
			5.0f, // radius
			guicolors::red); // color

		// handle mouse actions for keyframe value
		if ((mMouseActionData.currentAction == SequenceGUIMouseActions::NONE || mMouseActionData.currentAction == SequenceGUIMouseActions::HOVERING_KEYFRAMEVALUE)
			&& ImGui::IsMouseHoveringRect(
			{ trackTopLeft.x + x - 5, trackTopLeft.y + trackHeight * keyFrame->mValue - 5 }, // topleft
			{ trackTopLeft.x + x + 5, trackTopLeft.y + trackHeight * keyFrame->mValue + 5 })) // bottomright
		{
			drawList->AddCircleFilled(
				ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue), // position
				5.0f, // radius
				guicolors::red); // color

			mMouseActionData.currentAction = SequenceGUIMouseActions::HOVERING_KEYFRAMEVALUE;

			// if we have a click, initiate a mouse action
			if (!mMouseActionData.mouseWasDown && ImGui::IsMouseDown(0))
			{
				mMouseActionData.mouseWasDown = true;
				mMouseActionData.previousMousePos = ImGui::GetMousePos();
				mMouseActionData.currentAction = SequenceGUIMouseActions::DRAGGING_KEYFRAMEVALUE;
				mMouseActionData.currentObject = keyFrame.get();
			}
		}
		else
		{
			// stop hovering
			if (mMouseActionData.currentAction != SequenceGUIMouseActions::DRAGGING_KEYFRAMEVALUE &&
				mMouseActionData.currentAction == SequenceGUIMouseActions::HOVERING_KEYFRAMEVALUE)
			{
				mMouseActionData.currentAction = SequenceGUIMouseActions::NONE;
			}
		}

		// process mouse action for keyframe value
		if (mMouseActionData.currentAction == SequenceGUIMouseActions::DRAGGING_KEYFRAMEVALUE &&
			keyFrame.get() == mMouseActionData.currentObject)
		{
			// draw circle filled
			drawList->AddCircleFilled(
				{ trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue }, // position
				5.0f, // radius
				guicolors::red); // color

			// calc delta
			float deltaY = ImGui::GetMousePos().y - mMouseActionData.previousMousePos.y;

			// translate delta to timeline position
			float translatedValue = keyFrame->mValue + deltaY / trackHeight;

			// clamp value
			translatedValue = math::clamp<float>(translatedValue, 0, 1);

			// set value
			keyFrame->adjustValue(translatedValue);

			//
			mMouseActionData.previousMousePos = ImGui::GetMousePos();

			// handle release of mouse, stop mouse action
			if (mMouseActionData.mouseWasDown && ImGui::IsMouseReleased(0))
			{
				mMouseActionData.mouseWasDown = false;
				mMouseActionData.previousMousePos = ImGui::GetMousePos();
				mMouseActionData.currentAction = SequenceGUIMouseActions::NONE;
				mMouseActionData.currentObject = nullptr;
			}
		}

		// draw curve
		float curveWidth = x - previousKeyFrameX;
		const int resolution = 20;
		std::vector<ImVec2> points;
		points.resize(resolution + 1);
		for (int i = 0; i <= resolution; i++)
		{
			float value = keyFrame->mCurve->evaluate((float)i / resolution);

			points[i] = {
				trackTopLeft.x + previousKeyFrameX + curveWidth * ((float)i / resolution),
				trackTopLeft.y + value * trackHeight };
		}

		// draw points of curve
		drawList->AddPolyline(
			&*points.begin(), // points array
			points.size(), // size of points array
			guicolors::red, // color
			false, // closed
			1.0f, // thickness
			true); // anti-aliased

		// keyframe handler box coordinates
		ImVec2 handlerBoxTopLeft = { trackTopLeft.x + x - 5, trackTopLeft.y };
		ImVec2 handlerBoxBottomRight = { trackTopLeft.x + x + 5, trackTopLeft.y + trackHeight };

		// check if keyframe line is being hovered
		if (ImGui::IsMouseHoveringRect(handlerBoxTopLeft, handlerBoxBottomRight) &&
			(mMouseActionData.currentAction == SequenceGUIMouseActions::NONE || mMouseActionData.currentAction == SequenceGUIMouseActions::HOVERING_KEYFRAME))
		{
			// draw keyframe line thick
			drawList->AddLine(
			{ trackTopLeft.x + x, trackTopLeft.y }, // top left
			{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			//
			mMouseActionData.currentAction = SequenceGUIMouseActions::HOVERING_KEYFRAME;

			// if we have a click, initiate a mouse action
			if (!mMouseActionData.mouseWasDown && ImGui::IsMouseDown(0))
			{
				mMouseActionData.mouseWasDown = true;
				mMouseActionData.previousMousePos = ImGui::GetMousePos();
				mMouseActionData.currentAction = SequenceGUIMouseActions::DRAGGING_KEYFRAME;
				mMouseActionData.currentObject = keyFrame.get();
			}
		}

		// handle mouse action
		if (mMouseActionData.currentAction == SequenceGUIMouseActions::DRAGGING_KEYFRAME &&
			mMouseActionData.currentObject == keyFrame.get())
		{
			// draw keyframe line thick
			drawList->AddLine(
				{ trackTopLeft.x + x, trackTopLeft.y }, // top left
				{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			// handle mouse drag
			if (mMouseActionData.mouseWasDown && ImGui::IsMouseDragging(0))
			{
				// calc delta
				float deltaX = ImGui::GetMousePos().x - mMouseActionData.previousMousePos.x;

				// translate delta to timeline position
				float timelinepos = keyFrame->mTime + deltaX / stepSize;

				// set keyframe time
				keyFrame->mTime = timelinepos;

				// set previous mouse pos
				mMouseActionData.previousMousePos = ImGui::GetMousePos();
			}

			// handle release of mouse, stop mouse action
			if (mMouseActionData.mouseWasDown && ImGui::IsMouseReleased(0))
			{
				mMouseActionData.mouseWasDown = false;
				mMouseActionData.previousMousePos = ImGui::GetMousePos();
				mMouseActionData.currentAction = SequenceGUIMouseActions::NONE;
				mMouseActionData.currentObject = nullptr;
			}
		}
		else
		{
			// draw keyframe line thin
			drawList->AddLine(
			{ trackTopLeft.x + x, trackTopLeft.y }, // top left
			{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

			// stop hovering
			if (mMouseActionData.currentAction == SequenceGUIMouseActions::HOVERING_KEYFRAME)
			{
				mMouseActionData.currentAction = SequenceGUIMouseActions::NONE;
			}
		}

		previousKeyFrameX = x;	
	}*/
}
