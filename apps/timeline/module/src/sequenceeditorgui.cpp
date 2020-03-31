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
		const float timelineWidth =
			stepSize * mSequence.mDuration;

		const float trackInspectorWidth = 200.0f;
		
		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth + trackInspectorWidth + 10);


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

			// store position of next window ( player controller ), we need it later to draw the timelineplayer position 
			const ImVec2 timelineControllerWindowPosition = ImGui::GetCursorPos();
			drawPlayerController(
				trackInspectorWidth + 5,
				timelineWidth, 
				mouseDelta);

			// move a little bit more up to align tracks nicely with timelinecontroller
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10);

			// draw tracks
			drawTracks(
				sequence,
				trackInspectorWidth,
				timelineWidth,
				mousePos,
				stepSize,
				mouseDelta);

			// on top of everything, draw time line player position
			drawTimelinePlayerPosition(
				timelineControllerWindowPosition,
				trackInspectorWidth,
				timelineWidth);

			// handle insert segment popup
			handleInsertSegmentPopup();

			// handle delete segment popup
			handleDeleteSegmentPopup();

			ImGui::End();
		}

		// pop id
		ImGui::PopID();
	}


	void SequenceEditorGUIView::drawTracks(
		const Sequence &sequence,
		const float inspectorWidth,
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
		const float margin = 10.0f;

		int trackCount = 0;
		for (const auto& track : sequence.mTracks)
		{
			// begin inspector
			std::ostringstream inspectorIDStream;
			inspectorIDStream << track->mID << "inspector";
			std::string inspectorID = inspectorIDStream.str();

			// manually set the cursor position before drawing new track window
			cursorPos = { cursorPos.x , cursorPos.y + trackHeight * trackCount + margin };

			// manually set the cursor position before drawing inspector
			ImVec2 inspectorCursorPos = { cursorPos.x + 5 , cursorPos.y };
			ImGui::SetCursorPos(inspectorCursorPos);

			if (ImGui::BeginChild(
				inspectorID.c_str(), // id
				{ inspectorWidth , trackHeight + 5 }, // size
				false, // no border
				ImGuiWindowFlags_NoMove)) // window flags
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				const ImVec2 windowPos = ImGui::GetWindowPos();
				const ImVec2 windowSize = ImGui::GetWindowSize();
				drawList->AddRectFilled(
					windowPos,
					{windowPos.x + windowSize.x - 5, windowPos.y + trackHeight},
					guicolors::black);

				drawList->AddRect(
					windowPos,
					{ windowPos.x + windowSize.x - 5, windowPos.y + trackHeight },
					guicolors::white);

				ImGui::EndChild();
			}

			const ImVec2 windowCursorPos = { cursorPos.x + inspectorWidth + 5, cursorPos.y };
			ImGui::SetCursorPos(windowCursorPos);

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

				// draw background of track
				drawList->AddRectFilled(
					trackTopLeft, // top left position
					{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }, // bottom right position
					guicolors::black); // color 

				// draw border of track
				drawList->AddRect(
					trackTopLeft, // top left position
					{ trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight }, // bottom right position
					guicolors::white); // color 

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

				int segmentCount = 0;
				for (const auto& segment : track->mSegments)
				{
					float segmentX = (segment->mStartTime + segment->mDuration) * stepSize;
					float segmentWidth = segment->mDuration * stepSize;

					// curve
					drawCurve(
						*track.get(),
						*segment.get(),
						trackTopLeft,
						previousSegmentX,
						segmentWidth,
						trackHeight,
						segmentX,
						stepSize,
						drawList);

					// draw control points
					drawControlPoints(
						*track.get(),
						*segment.get(),
						trackTopLeft,
						segmentX,
						segmentWidth,
						trackHeight,
						mouseDelta,
						stepSize,
						drawList);

					// if this is the first segment of the track
					// also draw a handler for the start value
					if (segmentCount == 0)
					{
						// draw segment value handler
						drawSegmentValue(
							*track.get(),
							*segment.get(),
							trackTopLeft,
							segmentX,
							segmentWidth,
							trackHeight,
							mouseDelta,
							stepSize,
							SegmentValueTypes::BEGIN,
							drawList);
					}

					// draw segment value handler
					drawSegmentValue(
						*track.get(),
						*segment.get(),
						trackTopLeft,
						segmentX,
						segmentWidth,
						trackHeight,
						mouseDelta,
						stepSize,
						SegmentValueTypes::END,
						drawList);

					// draw segment handlers
					drawSegmentHandler(
						*track.get(),
						*segment.get(),
						trackTopLeft,
						segmentX,
						segmentWidth,
						trackHeight,
						mouseDelta,
						stepSize,
						drawList);

					//
					previousSegmentX = segmentX;

					//
					segmentCount++;
				}

				// pop id
				ImGui::PopID();

				ImGui::End();
			}

			//
			ImGui::SetCursorPos(cursorPos);

			// increment track count
			trackCount++;
		}
	}


	void SequenceEditorGUIView::drawControlPoints(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const float trackHeight,
		const ImVec2 mouseDelta,
		const int stepSize,
		ImDrawList* drawList)
	{
		// draw first control point handlers IF this is the first segment of the track
		if (track.mSegments[0]->mID == segment.mID)
		{
			const auto& curvePoint = segment.mCurve->mPoints[0];
			std::ostringstream stringStream;
			stringStream << segment.mID << "_point_" << 0;

			ImVec2 circlePoint =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
				trackTopLeft.y + trackHeight * (1.0f - curvePoint.mPos.mValue) };

			drawTanHandler(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				trackHeight,
				circlePoint,
				0,
				TanPointTypes::IN,
				mouseDelta,
				stepSize,
				drawList);

			drawTanHandler(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				trackHeight,
				circlePoint,
				0,
				TanPointTypes::OUT,
				mouseDelta,
				stepSize,
				drawList);
		}

		// draw control points of curve
		// we ignore the first and last because they are controlled by the start & end value of the segment
		for (int i = 1; i < segment.mCurve->mPoints.size() - 1; i++)
		{
			// get the curvepoint and generate a unique ID for the control point
			const auto& curvePoint = segment.mCurve->mPoints[i];
			std::ostringstream stringStream;
			stringStream << segment.mID << "_point_" << i;
			std::string pointID = stringStream.str();

			// determine the point at where to draw the control point
			ImVec2 circlePoint =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
				trackTopLeft.y + trackHeight * (1.0f - curvePoint.mPos.mValue) };

			// handle mouse hovering
			bool hovered = false;
			if ((mState.currentAction == NONE ||
				mState.currentAction == HOVERING_CONTROL_POINT ||
				mState.currentAction == HOVERING_CURVE)
				&& ImGui::IsMouseHoveringRect(
			{ circlePoint.x - 5, circlePoint.y - 5 },
			{ circlePoint.x + 5, circlePoint.y + 5 }))
			{
				hovered = true;
			}

			if (hovered)
			{
				// if we are hovering this point, store ID
				mState.currentAction = HOVERING_CONTROL_POINT;
				mState.currentObjectID = pointID;

				// is the mouse held down, then we are dragging
				if (ImGui::IsMouseDown(0))
				{
					mState.currentAction = DRAGGING_CONTROL_POINT;
					mState.currentActionData = std::make_unique<SequenceGUIDragControlPointData>(track.mID, segment.mID, i);
					mState.currentObjectID = segment.mID;
				}
				// if we clicked right mouse button, delete control point
				else if( ImGui::IsMouseClicked(1))
				{
					mState.currentAction = DELETE_CONTROL_POINT;
					mState.currentActionData = std::make_unique<SequenceGUIDeleteControlPointData>(track.mID, segment.mID, i);
					mState.currentObjectID = segment.mID;
				}
			}
			else
			{
				// otherwise, if we where hovering but not anymore, stop hovering
				if (mState.currentAction == HOVERING_CONTROL_POINT &&
					pointID == mState.currentObjectID)
				{
					mState.currentAction = NONE;
				}
			}

			// handle dragging of control point
			if (mState.currentAction == DRAGGING_CONTROL_POINT &&
				segment.mID == mState.currentObjectID)
			{
				const SequenceGUIDragControlPointData* data
					= dynamic_cast<SequenceGUIDragControlPointData*>(mState.currentActionData.get());

				if (data->controlPointIndex == i)
				{
					float timeAdjust = mouseDelta.x / segmentWidth;
					float valueAdjust = (mouseDelta.y / trackHeight) * -1.0f;

					hovered = true;

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
			}

			// handle deletion of control point
			if (mState.currentAction == DELETE_CONTROL_POINT &&
				segment.mID == mState.currentObjectID)
			{
				const SequenceGUIDeleteControlPointData* data
					= dynamic_cast<SequenceGUIDeleteControlPointData*>(mState.currentActionData.get());

				if (data->controlPointIndex == i)
				{
					mController.deleteCurvePoint(
						data->trackID,
						data->segmentID,
						data->controlPointIndex);

					mState.currentAction = NONE;
					mState.currentActionData = nullptr;
					
				}
			}

			// draw the control point
			drawList->AddCircleFilled(
				circlePoint,
				4.0f,
				hovered ? guicolors::white : guicolors::lightGrey);

			// draw the handlers
			drawTanHandler(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				trackHeight,
				circlePoint,
				i,
				TanPointTypes::IN,
				mouseDelta,
				stepSize,
				drawList);

			drawTanHandler(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				trackHeight,
				circlePoint,
				i,
				TanPointTypes::OUT,
				mouseDelta,
				stepSize,
				drawList);
		}

		// handle last control point
		// overlaps with endvalue so only draw tan handlers
		const int controlPointIndex = segment.mCurve->mPoints.size() - 1;
		const auto& curvePoint = segment.mCurve->mPoints[controlPointIndex];

		std::ostringstream stringStream;
		stringStream << segment.mID << "_point_" << controlPointIndex;
		std::string pointID = stringStream.str();

		ImVec2 circlePoint =
		{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
		   trackTopLeft.y + trackHeight * (1.0f - curvePoint.mPos.mValue) };

		drawTanHandler(
			track,
			segment,
			stringStream,
			segmentWidth,
			curvePoint,
			trackHeight,
			circlePoint,
			controlPointIndex,
			TanPointTypes::IN,
			mouseDelta,
			stepSize,
			drawList);

		drawTanHandler(
			track,
			segment,
			stringStream,
			segmentWidth,
			curvePoint,
			trackHeight,
			circlePoint,
			controlPointIndex,
			TanPointTypes::OUT,
			mouseDelta,
			stepSize,
			drawList);

	}


	void SequenceEditorGUIView::drawCurve(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float previousSegmentX,
		const float segmentWidth,
		const float trackHeight,
		const float segmentX,
		const float stepSize,
		ImDrawList* drawList)
	{
		const int resolution = 40;
		bool curveSelected = false;
		std::vector<ImVec2> points;
		points.resize(resolution + 1);
		for (int i = 0; i <= resolution; i++)
		{
			float value = 1.0f - segment.mCurve->evaluate((float)i / resolution);

			points[i] = {
				trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
				trackTopLeft.y + value * trackHeight };
		}

		// determine if mouse is hovering curve
		if ((mState.currentAction == NONE || mState.currentAction == HOVERING_CURVE)
			&& ImGui::IsMouseHoveringRect(
		{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
		{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }))  // bottom right 
		{
			// translate mouse position to position in curve
			ImVec2 mousePos = ImGui::GetMousePos();
			float xInSegment = ((mousePos.x - (trackTopLeft.x + segmentX - segmentWidth)) / stepSize) / segment.mDuration;
			float yInSegment = 1.0f - ((mousePos.y - trackTopLeft.y) / trackHeight);

			// evaluate curve at x position
			float yInCurve = segment.mCurve->evaluate(xInSegment);

			// insert curve point on click
			const float maxDist = 0.1f;
			if (abs(yInCurve - yInSegment) < maxDist)
			{
				curveSelected = true;
				mState.currentAction = HOVERING_CURVE;
				mState.currentObjectID = segment.mID;

				if (ImGui::IsMouseClicked(0))
				{
					mController.insertCurvePoint(track.mID, segment.mID, xInSegment);
				}
			}
			else
			{
				if (mState.currentAction == HOVERING_CURVE &&
					mState.currentObjectID == segment.mID)
				{
					mState.currentAction = NONE;
				}
			}
		}
		else
		{
			if (mState.currentAction == HOVERING_CURVE &&
				mState.currentObjectID == segment.mID)
			{
				mState.currentAction = NONE;
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
	}


	void SequenceEditorGUIView::drawSegmentValue(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const float trackHeight,
		const ImVec2 &mouseDelta,
		const float stepSize,
		const SegmentValueTypes segmentType,
		ImDrawList* drawList
	)
	{
		// calculate point of this value in the window
		ImVec2 segmentValuePos =
		{
			trackTopLeft.x + segmentX - (segmentType == BEGIN ? segmentWidth : 0.0f),
			trackTopLeft.y + trackHeight * (1.0f - ((segmentType == BEGIN ? segment.mStartValue : segment.mEndValue) / 1.0f))
		};

		bool hovered = false;

		// check if we are hovering this value
		if ((mState.currentAction == SequenceGUIMouseActions::NONE ||
			mState.currentAction == HOVERING_SEGMENT_VALUE ||
			mState.currentAction == HOVERING_SEGMENT ||
			mState.currentAction == HOVERING_CURVE)
			&& ImGui::IsMouseHoveringRect(
		{ segmentValuePos.x - 12, segmentValuePos.y - 12 }, // top left
		{ segmentValuePos.x + 12, segmentValuePos.y + 12 }))  // bottom right 
		{
			hovered = true;
			mState.currentAction = HOVERING_SEGMENT_VALUE;
			mState.currentActionData = std::make_unique<SequenceGUIDragSegmentData>(
				track.mID,
				segment.mID,
				segmentType);

			if (ImGui::IsMouseDown(0))
			{
				mState.currentAction = DRAGGING_SEGMENT_VALUE;
				mState.currentObjectID = segment.mID;
			}
		}
		else if (mState.currentAction != DRAGGING_SEGMENT_VALUE)
		{
			if (mState.currentAction == HOVERING_SEGMENT_VALUE)
			{
				const SequenceGUIDragSegmentData* data =
					dynamic_cast<SequenceGUIDragSegmentData*>(mState.currentActionData.get());

				if (data->type == segmentType && data->segmentID == segment.mID)
				{
					mState.currentAction = SequenceGUIMouseActions::NONE;
				}
			}
		}

		// handle dragging segment value
		if (mState.currentAction == DRAGGING_SEGMENT_VALUE &&
			mState.currentObjectID == segment.mID)
		{
			const SequenceGUIDragSegmentData* data =
				dynamic_cast<SequenceGUIDragSegmentData*>(mState.currentActionData.get());

			if (data->type == segmentType)
			{
				hovered = true;

				if (ImGui::IsMouseReleased(0))
				{
					mState.currentAction = NONE;
				}
				else
				{
					float dragAmount = (mouseDelta.y / trackHeight) * -1.0f;
					mController.changeSegmentValue(track.mID, segment.mID, dragAmount, segmentType);
				}
			}
		}

		if (hovered)
			drawList->AddCircleFilled(segmentValuePos, 5.0f, guicolors::red);
		else
			drawList->AddCircle(segmentValuePos, 5.0f, guicolors::red);
	}

	void SequenceEditorGUIView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const float trackHeight,
		const ImVec2 &mouseDelta,
		const float stepSize,
		ImDrawList* drawList)
	{
		// segment handler
		if ((mState.currentAction == SequenceGUIMouseActions::NONE ||
			(mState.currentAction == HOVERING_SEGMENT && mState.currentObjectID == segment.mID)) &&
			ImGui::IsMouseHoveringRect(
		{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
		{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + trackHeight + 10 }))  // bottom right 
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

					   // we are hovering this segment with the mouse
			mState.currentAction = HOVERING_SEGMENT;
			mState.currentObjectID = segment.mID;

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mState.currentAction = SequenceGUIMouseActions::DRAGGING_SEGMENT;
				mState.currentObjectID = segment.mID;
			}
			// right mouse in deletion popup
			else if (ImGui::IsMouseDown(1))
			{
				std::unique_ptr<SequenceGUIDeleteSegmentData> deleteSegmentData = std::make_unique<SequenceGUIDeleteSegmentData>(track.mID, segment.mID);
				mState.currentAction = SequenceGUIMouseActions::OPEN_DELETE_SEGMENT_POPUP;
				mState.currentObjectID = segment.mID;
				mState.currentActionData = std::move(deleteSegmentData);
			}
		}
		else if (
			mState.currentAction == SequenceGUIMouseActions::DRAGGING_SEGMENT &&
			mState.currentObjectID == segment.mID)
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + trackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			// do we have the mouse still held down ? drag the segment
			if (ImGui::IsMouseDown(0))
			{
				float amount = mouseDelta.x / stepSize;
				mController.segmentDurationChange(segment.mID, amount);
			}
			// otherwise... release!
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

					   // release if we are not hovering this segment
			if (mState.currentAction == HOVERING_SEGMENT
				&& mState.currentObjectID == segment.mID)
			{
				mState.currentAction = NONE;
			}
		}
	}


	void SequenceEditorGUIView::drawTanHandler(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		std::ostringstream &stringStream,
		const float segmentWidth,
		const math::FCurvePoint<float, float> &curvePoint,
		const float trackHeight,
		const ImVec2 &circlePoint,
		const int controlPointIndex,
		const TanPointTypes type,
		const ImVec2& mouseDelta,
		const int stepSize,
		ImDrawList* drawList)
	{
		// draw tan handlers
		{
			// create a string stream to create identifier of this object
			std::ostringstream tanStream;
			tanStream << stringStream.str() << (type == TanPointTypes::IN) ? "inTan" : "outTan";

			//
			const math::FComplex<float, float>& tanComplex
				= (type == TanPointTypes::IN) ? curvePoint.mInTan : curvePoint.mOutTan;

			// get the offset from the tan
			ImVec2 offset =
			{ (segmentWidth * tanComplex.mTime) / (float)segment.mDuration,
				(trackHeight *  tanComplex.mValue * -1.0f) };
			ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

			// set if we are hoverting this point with the mouse
			bool tanPointHovered = false;

			// check if hovered
			if ((mState.currentAction == NONE ||
				mState.currentAction == HOVERING_CURVE)
				&& ImGui::IsMouseHoveringRect(
			{ tanPoint.x - 5, tanPoint.y - 5 },
			{ tanPoint.x + 5, tanPoint.y + 5 }))
			{
				mState.currentAction = HOVERING_TAN_POINT;
				mState.currentObjectID = tanStream.str();
				tanPointHovered = true;
			}
			else if (
				mState.currentAction == HOVERING_TAN_POINT)
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
						if (ImGui::IsMouseDown(0) && mState.currentAction == HOVERING_TAN_POINT)
						{
							mState.currentAction = DRAGGING_TAN_POINT;

							mState.currentActionData = std::make_unique<SequenceGUIDragTanPointData>(
								track.mID,
								segment.mID,
								controlPointIndex,
								type);
						}
					}
					else
					{
						// otherwise, release!
						mState.currentAction = NONE;
					}
				}
			}

			// handle dragging of tan point
			if (mState.currentAction == DRAGGING_TAN_POINT)
			{
				const SequenceGUIDragTanPointData* data = dynamic_cast<SequenceGUIDragTanPointData*>(mState.currentActionData.get());
				if (data->segmentID == segment.mID && data->controlPointIndex == controlPointIndex && data->type == type)
				{
					if (ImGui::IsMouseReleased(0))
					{
						mState.currentAction = NONE;
						mState.currentActionData = nullptr;
					}
					else
					{
						tanPointHovered = true;

						float time = mouseDelta.x / stepSize;
						float value = (mouseDelta.y / trackHeight) * -1.0f;

						mController.changeTanPoint(
							track.mID,
							segment.mID,
							controlPointIndex,
							type,
							time,
							value);
					}
				}
			}

			// draw line
			drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

			// draw handler
			drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);
		}
	}


	void SequenceEditorGUIView::handleInsertSegmentPopup()
	{
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
	}


	void SequenceEditorGUIView::handleDeleteSegmentPopup()
	{
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
	}


	void SequenceEditorGUIView::drawPlayerController(
		const float startOffsetX,
		const float timelineWidth, 
		const ImVec2 &mouseDelta)
	{
		const float timelineControllerHeight = 15.0f;

		std::ostringstream stringStream;
		stringStream << mID << "timelinecontroller";
		std::string idString = stringStream.str();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + startOffsetX);

		// draw timeline controller
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ timelineWidth + 5 , timelineControllerHeight }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			ImGui::PushID(idString.c_str());

			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 windowTopLeft = ImGui::GetWindowPos();
			ImVec2 startPos =
			{
				windowTopLeft.x + cursorPos.x,
				windowTopLeft.y + cursorPos.y,
			};

			cursorPos.y += 5;

			// get window drawlist
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// draw backgroundbox of controller
			drawList->AddRectFilled(
				startPos,
				{
					startPos.x + timelineWidth,
					startPos.y + timelineControllerHeight
				}, guicolors::black);

			// draw box of controller
			drawList->AddRect(
				startPos,
				{
					startPos.x + timelineWidth,
					startPos.y + timelineControllerHeight
				}, guicolors::white);

			// draw handler of player position
			const double playerTime = mController.getPlayerPosition();
			const ImVec2 playerTimeRectTopLeft =
			{
				startPos.x + (float)(playerTime / mSequence.mDuration) * timelineWidth - 5,
				startPos.y
			};
			const ImVec2 playerTimeRectBottomRight =
			{
				startPos.x + (float)(playerTime / mSequence.mDuration) * timelineWidth + 5,
				startPos.y + timelineControllerHeight,
			};

			drawList->AddRectFilled(
				playerTimeRectTopLeft,
				playerTimeRectBottomRight,
				guicolors::red);

			if (mState.currentAction == NONE || mState.currentAction == HOVERING_PLAYER_TIME)
			{
				if (ImGui::IsMouseHoveringRect(playerTimeRectTopLeft, playerTimeRectBottomRight))
				{
					mState.currentAction = HOVERING_PLAYER_TIME;

					if (ImGui::IsMouseDown(0))
					{
						mState.currentAction = DRAGGING_PLAYER_TIME;
					}
				}
				else
				{
					mState.currentAction = NONE;
				}
			}

			if (mState.currentAction == DRAGGING_PLAYER_TIME)
			{
				if (ImGui::IsMouseDown(0))
				{
					double delta = (mouseDelta.x / timelineWidth) * mSequence.mDuration;
					mController.setPlayerPosition(playerTime + delta);
				}
				else
				{
					if (ImGui::IsMouseReleased(0))
					{
						mState.currentAction = NONE;
					}
				}
			}

			ImGui::PopID();

			ImGui::EndChild();
		}
	}


	void SequenceEditorGUIView::drawTimelinePlayerPosition(
		const ImVec2 &timelineControllerWindowPosition, 
		const float trackInspectorWidth,
		const float timelineWidth)
	{
		std::ostringstream stringStream;
		stringStream << mID << "timelineplayerposition";
		std::string idString = stringStream.str();

		ImGui::SetCursorPos(
		{
			timelineControllerWindowPosition.x 
				+ trackInspectorWidth + 5 
				+ timelineWidth * (float)(mController.getPlayerPosition() / mSequence.mDuration) - 1,
			timelineControllerWindowPosition.y
		});
		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, guicolors::red);
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ 1.0f, mSequence.mTracks.size() * 110.0f + 10.0f }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{

			ImGui::End();
		}
		ImGui::PopStyleColor();
	}


	SequenceEditorView::SequenceEditorView(const Sequence& sequence, SequenceEditorController& controller)
		: mSequence(sequence), mController(controller) {}
}