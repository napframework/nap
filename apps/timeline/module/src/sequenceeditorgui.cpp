// local includes
#include "sequenceeditorgui.h"
#include "napcolors.h"

// External Includes
#include <entity.h>
#include <imgui/imgui.h>
#include <nap/logger.h>

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

		mView = std::make_unique<SequenceEditorGUIView>(
			mSequenceEditor->getSequence(),
			mSequenceEditor->getController(),
			mID);

		return true;
	}


	void SequenceEditorGUI::onDestroy()
	{
	}


	void SequenceEditorGUI::draw()
	{
		//
		mView->draw();
	}


	SequenceEditorGUIView::SequenceEditorGUIView(
		const Sequence& sequence,
		SequenceEditorController& controller,
		std::string id) : SequenceEditorView(sequence, controller) {
		mID = id;
	}


	void SequenceEditorGUIView::draw()
	{

		//
		ImVec2 newPos = ImGui::GetMousePos();
		ImVec2 mouseDelta = { newPos.x - mPreviousMousePos.x, newPos.y - mPreviousMousePos.y };
		mPreviousMousePos = newPos;

		//
		const Sequence& sequence = mSequence;

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		float stepSize = 100.0f;

		// calc width of content in timeline window
		float timelineWidth =
			stepSize * mSequence.mDuration;

		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth);

		
		// begin window
		if (ImGui::Begin(
			mID.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{

			//
			if (ImGui::Button("Save"))
			{
				mController.save();
			}

			// get current cursor pos, we will use this to position the track windows
			ImVec2 cursorPos = ImGui::GetCursorPos();

			// check if window has focus
			bool windowHasFocus = ImGui::IsWindowFocused();

			// define consts
			const float trackHeight = 100.0f;
			const float keyframeHandlerHeight = 10.0f;

			int trackCount = 0;
			for (const auto& track : sequence.mTracks)
			{
				// manually set the cursor position before drawing new track window
				cursorPos = { cursorPos.x, cursorPos.y + trackHeight * trackCount + 1 };
				ImGui::SetCursorPos(cursorPos);

				// begin track
				if (ImGui::BeginChild(
					track->mID.c_str(), // id
					{ timelineWidth + 5 , trackHeight + keyframeHandlerHeight }, // size
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

					//
					if (mState.currentAction == SequenceGUIMouseActions::NONE)
					{
						if (ImGui::IsMouseHoveringRect(
							trackTopLeft, // top left position
							{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }))
						{
							// position of mouse in track
							drawList->AddLine(
							{ newPos.x, trackTopLeft.y }, // top left
							{ newPos.x, trackTopLeft.y + trackHeight }, // bottom right
								guicolors::lightGrey, // color
								1.0f); // thickness

							// right mouse down
							if (ImGui::IsMouseClicked(1))
							{
								double time = (newPos.x - trackTopLeft.x) / stepSize;

								//
								mState.currentAction = OPEN_INSERT_SEGMENT_POPUP;
								mState.currentActionData = std::make_unique<SequenceGUIInsertSequenceData>(track->mID, time);
							}
						}
					}

					float previousSegmentX = 0.0f;

					// draw segments
					for (const auto& segment : track->mSegments)
					{
						float x = (segment->mStartTime + segment->mDuration) * stepSize;
						float segmentWidth = segment->mDuration * stepSize;

						//
						if (mState.currentAction == SequenceGUIMouseActions::NONE &&
							ImGui::IsMouseHoveringRect(
							{ trackTopLeft.x + x - 5, trackTopLeft.y - 5 }, // top left
							{ trackTopLeft.x + x + 5, trackTopLeft.y + trackHeight + 5 }))  // bottom right 
						{
							// draw handler of segment duration
							drawList->AddLine(
							{ trackTopLeft.x + x, trackTopLeft.y }, // top left
							{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
								guicolors::white, // color
								3.0f); // thickness

							if (ImGui::IsMouseDown(0))
							{
								mState.currentAction = SequenceGUIMouseActions::DRAGGING_SEGMENT;
								mState.currentObjectID = segment->mID;
							}
						}
						else if (
							mState.currentAction == SequenceGUIMouseActions::DRAGGING_SEGMENT &&
							mState.currentObjectID == segment->mID)
						{
							// draw handler of segment duration
							drawList->AddLine(
							{ trackTopLeft.x + x, trackTopLeft.y }, // top left
							{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
								guicolors::white, // color
								3.0f); // thickness

							if (ImGui::IsMouseDown(0))
							{
								float amount = mouseDelta.x / stepSize;
								mController.segmentDurationChange(segment->mID, amount);
							}
							else if (ImGui::IsMouseReleased(0))
							{
								mState.currentAction = SequenceGUIMouseActions::NONE;
							}
						}
						else
						{
							// draw handler of segment duration
							drawList->AddLine(
							{ trackTopLeft.x + x, trackTopLeft.y }, // top left
							{ trackTopLeft.x + x, trackTopLeft.y + trackHeight }, // bottom right
								guicolors::white, // color
								1.0f); // thickness
						}

						// draw curve;
						const int resolution = 20;
						std::vector<ImVec2> points;
						points.resize(resolution + 1);
						for (int i = 0; i <= resolution; i++)
						{
							float value = 1.0f - segment->mCurve->evaluate((float)i / resolution);

							points[i] = {
								trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
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

						previousSegmentX = x;

						/*
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
						*/
					}

					// pop id
					ImGui::PopID();

					ImGui::End();
				}

				
				// increment track count
				trackCount++;
			}

			// handle opening of popups
			if (mState.currentAction == OPEN_INSERT_SEGMENT_POPUP)
			{
				// invoke insert sequence popup
				ImGui::OpenPopup("Insert Segment");

				mState.currentAction = INSERTING_SEGMENT;
			}

			// handle popups
			if (mState.currentAction == INSERTING_SEGMENT)
			{
				if (ImGui::BeginPopup("Insert Segment"))
				{
					if (ImGui::Button("Insert"))
					{
						const SequenceGUIInsertSequenceData* data = dynamic_cast<SequenceGUIInsertSequenceData*>(mState.currentActionData.get());
						mController.insertSequence(data->trackID, data->time);

						ImGui::CloseCurrentPopup();
						mState.currentAction = SequenceGUIMouseActions::NONE;
						mState.currentActionData = nullptr;
					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
						mState.currentAction = SequenceGUIMouseActions::NONE;
						mState.currentActionData = nullptr;
					}

					ImGui::EndPopup();
				}
				else
				{
					// click outside popup so cancel action
					mState.currentAction = SequenceGUIMouseActions::NONE;
					mState.currentActionData = nullptr;
				}
			}
			
			ImGui::End();
		}
		
		// pop id
		ImGui::PopID();
	}


	SequenceEditorView::SequenceEditorView(const Sequence& sequence, SequenceEditorController& controller)
		: mSequence(sequence), mController(controller) {}

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
