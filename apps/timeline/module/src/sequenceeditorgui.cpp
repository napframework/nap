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
		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 mouseDelta = { mousePos.x - mPreviousMousePos.x, mousePos.y - mPreviousMousePos.y };
		mPreviousMousePos = mousePos;

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

			// draw tracks
			drawTracks(
				sequence, 
				timelineWidth, 
				mousePos,
				stepSize, 
				mouseDelta);

			// handle opening of popups
			if (mState.currentAction == OPEN_INSERT_SEGMENT_POPUP)
			{
				// invoke insert sequence popup
				ImGui::OpenPopup("Insert Segment");

				mState.currentAction = INSERTING_SEGMENT;
			}

			// handle insert segment popup
			if (mState.currentAction == INSERTING_SEGMENT)
			{
				if (ImGui::BeginPopup("Insert Segment"))
				{
					if (ImGui::Button("Insert"))
					{
						const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mState.currentActionData.get());
						mController.insertSegment(data->trackID, data->time);

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

			// handle opening of popups
			if (mState.currentAction == OPEN_DELETE_SEGMENT_POPUP)
			{
				// invoke insert sequence popup
				ImGui::OpenPopup("Delete Segment");

				mState.currentAction = DELETING_SEGMENT;
			}

			// handle delete segment popup
			if (mState.currentAction == DELETING_SEGMENT)
			{
				if (ImGui::BeginPopup("Delete Segment"))
				{
					if (ImGui::Button("Delete"))
					{
						const SequenceGUIDeleteSegmentData* data = dynamic_cast<SequenceGUIDeleteSegmentData*>(mState.currentActionData.get());
						mController.deleteSegment(data->trackID, data->segmentID);

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


	void SequenceEditorGUIView::drawTracks(
		const Sequence &sequence, 
		const float timelineWidth,
		const ImVec2 &mousePos, 
		const float stepSize, 
		const ImVec2 &mouseDelta)
	{
		// get current cursor pos, we will use this to position the track windows
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// check if window has focus
		bool windowHasFocus = ImGui::IsWindowFocused();

		// define consts
		const float trackHeight = 100.0f;

		int trackCount = 0;
		for (const auto& track : sequence.mTracks)
		{
			// manually set the cursor position before drawing new track window
			cursorPos = { cursorPos.x, cursorPos.y + trackHeight * trackCount + 1 };
			ImGui::SetCursorPos(cursorPos);

			// begin track
			if (ImGui::BeginChild(
				track->mID.c_str(), // id
				{ timelineWidth + 5 , trackHeight + 5 }, // size
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

				// handle insertion of segment
				if (mState.currentAction == SequenceGUIMouseActions::NONE)
				{
					if (ImGui::IsMouseHoveringRect(
						trackTopLeft, // top left position
						{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }))
					{
						// position of mouse in track
						drawList->AddLine(
						{ mousePos.x, trackTopLeft.y }, // top left
						{ mousePos.x, trackTopLeft.y + trackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness

								   // right mouse down
						if (ImGui::IsMouseClicked(1))
						{
							double time = (mousePos.x - trackTopLeft.x) / stepSize;

							//
							mState.currentAction = OPEN_INSERT_SEGMENT_POPUP;
							mState.currentActionData = std::make_unique<SequenceGUIInsertSegmentData>(track->mID, time);
						}
					}
				}

				// draw line in track while in inserting segment popup
				if (mState.currentAction == SequenceGUIMouseActions::OPEN_INSERT_SEGMENT_POPUP || mState.currentAction == INSERTING_SEGMENT)
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mState.currentActionData.get());
					if (data->trackID == track->mID)
					{
						// position of insertion in track
						drawList->AddLine(
						{ trackTopLeft.x + (float)data->time * stepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)data->time * stepSize, trackTopLeft.y + trackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}

				float previousSegmentX = 0.0f;

				for (const auto& segment : track->mSegments)
				{
					float segmentX = (segment->mStartTime + segment->mDuration) * stepSize;
					float segmentWidth = segment->mDuration * stepSize;

					// curve
					const int resolution = 40;
					bool curveSelected = false;
					std::vector<ImVec2> points;
					points.resize(resolution + 1);
					for (int i = 0; i <= resolution; i++)
					{
						float value = 1.0f - segment->mCurve->evaluate((float)i / resolution);

						points[i] = {
							trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
							trackTopLeft.y + value * trackHeight };
					}

					// determine if mouse is hovering curve
					if (mState.currentAction == SequenceGUIMouseActions::NONE
						&& ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
					{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }))  // bottom right 
					{
						// translate mouse position to position in curve
						ImVec2 mousePos = ImGui::GetMousePos();
						float xInSegment = ((mousePos.x - (trackTopLeft.x + segmentX - segmentWidth)) / stepSize) / segment->mDuration;
						float yInSegment = 1.0f - ((mousePos.y - trackTopLeft.y) / trackHeight);

						// evaluate curve at x position
						float yInCurve = segment->mCurve->evaluate(xInSegment);

						// insert curve point on click
						const float maxDist = 0.05f;
						if (abs(yInCurve - yInSegment) < maxDist)
						{
							curveSelected = true;
							if (ImGui::IsMouseClicked(0))
							{
								mController.insertCurvePoint(track->mID, segment->mID, xInSegment);
							}
						}
					}

					// draw points of curve
					drawList->AddPolyline(
						&*points.begin(), // points array
						points.size(), // size of points array
						guicolors::red, // color
						false, // closed
						curveSelected ? 3.0f : 1.0f, // thickness
						true); // anti-aliased

							   // draw control points of curve
					for (int i = 1; i < segment->mCurve->mPoints.size() - 1; i++)
					{
						const auto& curvePoint = segment->mCurve->mPoints[i];
						std::ostringstream stringStream;
						stringStream << segment->mID << "_point_" << i;
						std::string pointID = stringStream.str();

						ImVec2 circlePoint =
						{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
							trackTopLeft.y + trackHeight * (1.0f - curvePoint.mPos.mValue) };

						bool hovered = false;
						if ((mState.currentAction == NONE || mState.currentAction == HOVERING_CONTROL_POINT) &&
							ImGui::IsMouseHoveringRect(
						{ circlePoint.x - 5, circlePoint.y - 5 },
						{ circlePoint.x + 5, circlePoint.y + 5 }))
						{
							hovered = true;
						}

						drawList->AddCircleFilled(
							circlePoint,
							4.0f,
							hovered ? guicolors::white : guicolors::lightGrey);

						if (hovered)
						{
							mState.currentAction = HOVERING_CONTROL_POINT;
							mState.currentObjectID = pointID;

							if (ImGui::IsMouseDown(0))
							{
								mState.currentAction = DRAGGING_CONTROL_POINT;
								mState.currentActionData = std::make_unique<SequenceGUIDragControlPointData>(track->mID, segment->mID, i);
								mState.currentObjectID = segment->mID;
							}
						}
						else
						{
							if (mState.currentAction == HOVERING_CONTROL_POINT &&
								pointID == mState.currentObjectID)
							{
								mState.currentAction = NONE;
							}
						}

						// draw tan handlers
						{
							// create a string stream to create identifier of this object
							std::ostringstream tanStream;
							tanStream << stringStream.str() << "inTan";

							// get the offset from the tan
							ImVec2 offset =
							{	( segmentWidth * curvePoint.mInTan.mTime ) / (float) segment->mDuration,
								( trackHeight *  curvePoint.mInTan.mValue * -1.0f ) / (float) segment->mDuration};
							ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

							// set if we are hoverting this point with the mouse
							bool tanPointHovered = false;

							// check if hovered
							if (mState.currentAction == NONE  
								&& ImGui::IsMouseHoveringRect(
									{ tanPoint.x - 5, tanPoint.y - 5 },
									{ tanPoint.x + 5, tanPoint.y + 5 }))
							{
								mState.currentAction = HOVERING_TAN_POINT;
								mState.currentObjectID = tanStream.str();
								tanPointHovered = true;
							}
							else if (mState.currentAction == HOVERING_TAN_POINT)
							{
								// if we hare already hovering, check if its this point
								if (mState.currentObjectID == tanStream.str())
								{
									if (ImGui::IsMouseHoveringRect(
									{ tanPoint.x - 5, tanPoint.y - 5 },
									{ tanPoint.x + 5, tanPoint.y + 5 }))
									{
										// still hovering
										tanPointHovered = true;

										// start dragging if mouse down
										if (ImGui::IsMouseDown(0))
										{
											mState.currentAction = DRAGGING_TAN_POINT;
											mState.currentActionData = std::make_unique<SequenceGUIDragTanPointData>(
												track->mID,
												segment->mID,

												)
										}
									}
									else
									{
										// otherwise, release!
										mState.currentAction = NONE;
									}
								}
							}

							// draw line
							drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

							// draw handler
							drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);

						}
						
						/*
						{
							//
							std::ostringstream tanStream;
							tanStream << stringStream.str() << "outTan";

							// out tan
							ImVec2 offset =
							{ (segmentWidth * curvePoint.mOutTan.mTime) / (float) segment->mDuration,
								(trackHeight *  curvePoint.mOutTan.mValue * -1.0f) / (float) segment->mDuration };
							ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

							bool tanPointHovered = false;

							// check if hovered
							if (mState.currentAction == NONE || mState.currentAction == HOVERING_TAN_POINT
								&& ImGui::IsMouseHoveringRect(
							{ circlePoint.x - 5, circlePoint.y - 5 },
							{ tanPoint.x + 5, tanPoint.y + 5 }))
							{
								mState.currentAction = HOVERING_TAN_POINT;
								mState.currentObjectID = tanStream.str();
								tanPointHovered = true;
							}
							else if (mState.currentAction == HOVERING_TAN_POINT)
							{
								if (mState.currentObjectID == tanStream.str())
								{
									mState.currentAction = NONE;
								}
							}

							// draw line
							drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

							// draw handler
							drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);

						}*/
					}

					// handle dragging of control point
					if (mState.currentAction == DRAGGING_CONTROL_POINT && segment->mID == mState.currentObjectID)
					{
						const SequenceGUIDragControlPointData* data = dynamic_cast<SequenceGUIDragControlPointData*>(mState.currentActionData.get());
						float timeAdjust = mouseDelta.x / segmentWidth;
						float valueAdjust = (mouseDelta.y / trackHeight) * -1.0f;

						mController.changeCurvePoint(
							data->trackID,
							data->segmentID,
							data->controlPointIndex,
							timeAdjust,
							valueAdjust);

						if (ImGui::IsMouseReleased(0))
						{
							mState.currentAction = NONE;
							mState.currentActionData = nullptr;
						}
					}

					// end value handler
					ImVec2 endValuePos = { trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight * (1.0f - (segment->mEndValue / 1.0f)) };
					if ((mState.currentAction == SequenceGUIMouseActions::NONE || mState.currentAction == HOVERING_SEGMENT) &&
						ImGui::IsMouseHoveringRect(
					{ endValuePos.x - 7, endValuePos.y - 7 }, // top left
					{ endValuePos.x + 7, endValuePos.y + 7 }))  // bottom right 
					{
						drawList->AddCircleFilled(endValuePos, 5.0f, guicolors::red);

						mState.currentAction = HOVERING_SEGMENT_VALUE;

						if (ImGui::IsMouseDown(0))
						{
							mState.currentAction = DRAGGING_SEGMENT_VALUE;
							mState.currentObjectID = segment->mID;
						}
					}
					else if (mState.currentAction != DRAGGING_SEGMENT_VALUE)
					{
						drawList->AddCircle(endValuePos, 5.0f, guicolors::red);

						if (mState.currentAction == HOVERING_SEGMENT_VALUE)
						{
							mState.currentAction = SequenceGUIMouseActions::NONE;
						}
					}

					// handle dragging segment value
					if (mState.currentAction == DRAGGING_SEGMENT_VALUE && mState.currentObjectID == segment->mID)
					{
						drawList->AddCircleFilled(endValuePos, 5.0f, guicolors::red);

						if (ImGui::IsMouseReleased(0))
						{
							mState.currentAction = NONE;
						}
						else
						{
							float dragAmount = (mouseDelta.y / trackHeight) * -1.0f;
							mController.changeSegmentEndValue(track->mID, segment->mID, dragAmount);
						}
					}

					// segment handler
					if ((mState.currentAction == SequenceGUIMouseActions::NONE ||
						(mState.currentAction == HOVERING_SEGMENT && mState.currentObjectID == segment->mID)) &&
						ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - 5, trackTopLeft.y - 5 }, // top left
					{ trackTopLeft.x + segmentX + 5, trackTopLeft.y + trackHeight + 5 }))  // bottom right 
					{
						// draw handler of segment duration
						drawList->AddLine(
						{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
						{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }, // bottom right
							guicolors::white, // color
							3.0f); // thickness

						mState.currentAction = HOVERING_SEGMENT;
						mState.currentObjectID = segment->mID;

						// left mouse is start dragging
						if (ImGui::IsMouseDown(0))
						{
							mState.currentAction = SequenceGUIMouseActions::DRAGGING_SEGMENT;
							mState.currentObjectID = segment->mID;
						}
						// right mouse in deletion popup
						else if (ImGui::IsMouseDown(1))
						{
							std::unique_ptr<SequenceGUIDeleteSegmentData> deleteSegmentData = std::make_unique<SequenceGUIDeleteSegmentData>(track->mID, segment->mID);
							mState.currentAction = SequenceGUIMouseActions::OPEN_DELETE_SEGMENT_POPUP;
							mState.currentObjectID = segment->mID;
							mState.currentActionData = std::move(deleteSegmentData);
						}
					}
					else if (
						mState.currentAction == SequenceGUIMouseActions::DRAGGING_SEGMENT &&
						mState.currentObjectID == segment->mID)
					{
						// draw handler of segment duration
						drawList->AddLine(
						{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
						{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }, // bottom right
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
						{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
						{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }, // bottom right
							guicolors::white, // color
							1.0f); // thickness

						if (mState.currentAction == HOVERING_SEGMENT && mState.currentObjectID == segment->mID)
						{
							mState.currentAction = NONE;
						}
					}

					previousSegmentX = segmentX;
				}

				// pop id
				ImGui::PopID();

				ImGui::End();
			}


			// increment track count
			trackCount++;
		}
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
