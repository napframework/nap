#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackcurve.h"
#include "napcolors.h"
#include "sequencecontrollercurve.h"
#include "sequenceplayercurveinput.h"

#include <nap/logger.h>
#include <iostream>

namespace nap
{
	SequenceCurveTrackView::SequenceCurveTrackView(SequenceEditorGUIView& view)
		: SequenceTrackView(view)
	{
		nap::Logger::info("curve track");
	}

	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceCurveTrackView), [](SequenceEditorGUIView& view)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceCurveTrackView>(view);
	});

	static bool curveViewRegistrations[4]
	{
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec4>), RTTI_OF(SequenceCurveTrackView))
	};

	std::unordered_map<rttr::type, SequenceCurveTrackView::DrawTrackMemFunPtr> SequenceCurveTrackView::sDrawTracksMap
	{
		{ RTTI_OF(SequenceTrackCurveFloat), &SequenceCurveTrackView::drawCurveTrack<float> },
		{ RTTI_OF(SequenceTrackCurveVec2), &SequenceCurveTrackView::drawCurveTrack<glm::vec2> },
		{ RTTI_OF(SequenceTrackCurveVec3), &SequenceCurveTrackView::drawCurveTrack<glm::vec3> },
		{ RTTI_OF(SequenceTrackCurveVec4), &SequenceCurveTrackView::drawCurveTrack<glm::vec4> }
	};


	std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr> SequenceCurveTrackView::sDrawCurveSegmentsMap
	{
		{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::drawSegmentContent<float> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceCurveTrackView::drawSegmentContent<glm::vec2> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceCurveTrackView::drawSegmentContent<glm::vec3> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceCurveTrackView::drawSegmentContent<glm::vec4> }
	};


	void SequenceCurveTrackView::drawTrack(const SequenceTrack& track, SequenceEditorGUIState& state)
	{
		std::string deleteTrackID;
		bool deleteTrack = false;

		mState = &state;

		auto it = sDrawTracksMap.find(track.get_type());
		assert(it != sDrawTracksMap.end()); // track type not found
		if (it != sDrawTracksMap.end())
		{
			(*this.*it->second)(track, mState->mCursorPos, 10.0f, getPlayer(), deleteTrack, deleteTrackID);
		}

		if (deleteTrack)
		{
			auto& controller = getEditor().getController<SequenceControllerCurve>();
			controller.deleteTrack(track.mID);
		}
	}


	void SequenceCurveTrackView::handlePopups(SequenceEditorGUIState& state)
	{
		handleInsertSegmentPopup();

		handleCurveTypePopup();

		handleInsertCurvePointPopup();

		handleDeleteSegmentPopup();

		handleCurvePointActionPopup();
	}


	template<typename T>
	void SequenceCurveTrackView::drawCurveTrack(const SequenceTrack &track, ImVec2 &cursorPos, const float marginBetweenTracks, const SequencePlayer &sequencePlayer, bool &deleteTrack, std::string &deleteTrackID)
	{
		// begin inspector
		std::ostringstream inspectorIDStream;
		inspectorIDStream << track.mID << "inspector";
		std::string inspectorID = inspectorIDStream.str();

		// manually set the cursor position before drawing new track window
		cursorPos =
		{
			cursorPos.x ,
			mState->mTrackHeight + marginBetweenTracks + cursorPos.y
		};

		// manually set the cursor position before drawing inspector
		ImVec2 inspectorCursorPos = { cursorPos.x , cursorPos.y };
		ImGui::SetCursorPos(inspectorCursorPos);

		// draw inspector window
		if (ImGui::BeginChild(
			inspectorID.c_str(), // id
			{ mState->mInspectorWidth , mState->mTrackHeight + 5 }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// obtain drawlist
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// store window size and position
			const ImVec2 windowPos = ImGui::GetWindowPos();
			const ImVec2 windowSize = ImGui::GetWindowSize();

			// draw background & box
			drawList->AddRectFilled(
				windowPos,
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState->mTrackHeight },
				guicolors::black);

			drawList->AddRect(
				windowPos,
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState->mTrackHeight },
				guicolors::white);

			// 
			ImVec2 inspectorCursorPos = ImGui::GetCursorPos();
			inspectorCursorPos.x += 5;
			inspectorCursorPos.y += 5;
			ImGui::SetCursorPos(inspectorCursorPos);

			// scale down everything
			float scale = 0.25f;
			ImGui::GetStyle().ScaleAllSizes(scale);

			// draw the assigned parameter
			ImGui::Text("Assigned Parameter");

			inspectorCursorPos = ImGui::GetCursorPos();
			inspectorCursorPos.x += 5;
			inspectorCursorPos.y += 5;
			ImGui::SetCursorPos(inspectorCursorPos);

			bool assigned = false;
			std::string assignedID;
			std::vector<std::string> curveInputs;
			int currentItem = 0;
			curveInputs.emplace_back("none");
			int count = 0;
			const Parameter* assignedParameter = nullptr;

			for (const auto& input : sequencePlayer.mInputs)
			{
				if (input.get()->get_type() == RTTI_OF(SequencePlayerCurveInput))
				{
					count++;

					if (input->mID == track.mAssignedObjectIDs)
					{
						assigned = true;
						assignedID = input->mID;
						currentItem = count;

						assert(input.get()->get_type() == RTTI_OF(SequencePlayerCurveInput)); // type mismatch
						assignedParameter = static_cast<SequencePlayerCurveInput*>(input.get())->mParameter.get();
					}

					curveInputs.emplace_back(input->mID);
				}
			}

			ImGui::PushItemWidth(140.0f);
			if (Combo(
				"",
				&currentItem,
				curveInputs))
			{
				SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

				if (currentItem != 0)
					curveController.assignNewObjectID(track.mID, curveInputs[currentItem]);
				else
					curveController.assignNewObjectID(track.mID, "");
			}

			//
			ImGui::PopItemWidth();

			//
			drawInspectorRange<T>(track);

			// delete track button
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

			// when we delete a track, we don't immediately call the controller because we are iterating track atm
			if (ImGui::SmallButton("Delete"))
			{
				deleteTrack = true;
				deleteTrackID = track.mID;
			}

			// pop scale
			ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
		}
		ImGui::EndChild();

		const ImVec2 windowCursorPos = { cursorPos.x + mState->mInspectorWidth + 5, cursorPos.y };
		ImGui::SetCursorPos(windowCursorPos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mState->mTimelineWidth + 5 , mState->mTrackHeight + 5 }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// push id
			ImGui::PushID(track.mID.c_str());

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
				{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }, // bottom right position
				guicolors::black); // color 

			 // draw border of track
			drawList->AddRect(
				trackTopLeft, // top left position
				{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }, // bottom right position
				guicolors::white); // color 

		   //
			mState->mMouseCursorTime = (mState->mMousePos.x - trackTopLeft.x) / mState->mStepSize;

			if (mState->mIsWindowFocused)
			{
				// handle insertion of segment
				if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::None>())
				{
					if (ImGui::IsMouseHoveringRect(
						trackTopLeft, // top left position
						{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }))
					{
						// position of mouse in track
						drawList->AddLine(
							{ mState->mMousePos.x, trackTopLeft.y }, // top left
							{ mState->mMousePos.x, trackTopLeft.y + mState->mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness

						ImGui::BeginTooltip();

						ImGui::Text(formatTimeString(mState->mMouseCursorTime).c_str());

						ImGui::EndTooltip();

						// right mouse down
						if (ImGui::IsMouseClicked(1))
						{
							double time = mState->mMouseCursorTime;

							//
							mState->mAction.currentAction = SequenceGUIActions::OpenInsertSegmentPopup();
							mState->mAction.currentActionData = std::make_unique<SequenceGUIInsertSegmentData>(track.mID, time, track.get_type());
						}
					}
				}

				// draw line in track while in inserting segment popup
				if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenInsertSegmentPopup>() ||
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::InsertingSegment>())
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mState->mAction.currentActionData.get());
					if (data->mID == track.mID)
					{
						// position of insertion in track
						drawList->AddLine(
							{ trackTopLeft.x + (float)data->mTime * mState->mStepSize, trackTopLeft.y }, // top left
							{ trackTopLeft.x + (float)data->mTime * mState->mStepSize, trackTopLeft.y + mState->mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}
			}

			float previousSegmentX = 0.0f;

			int segmentCount = 0;
			for (const auto& segment : track.mSegments)
			{
				float segmentX = (segment->mStartTime + segment->mDuration) * mState->mStepSize;
				float segmentWidth = segment->mDuration * mState->mStepSize;

				auto it = sDrawCurveSegmentsMap.find(segment.get()->get_type());
				if (it != sDrawCurveSegmentsMap.end())
				{
					(*this.*it->second)(track, *segment.get(), trackTopLeft, previousSegmentX, segmentWidth, segmentX, drawList, (segmentCount == 0));
				}

				// draw segment handlers
				drawSegmentHandler(
					track,
					*segment.get(),
					trackTopLeft,
					segmentX,
					segmentWidth,
					drawList);

				//
				previousSegmentX = segmentX;

				//
				segmentCount++;
			}

			// pop id
			ImGui::PopID();

		}

		ImGui::End();

		//
		ImGui::SetCursorPos(cursorPos);
	}

	template<typename T>
	void SequenceCurveTrackView::drawControlPoints(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{

		const SequenceTrackSegmentCurve<T>& segment
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		// draw first control point(s) handlers IF this is the first segment of the track
		if (track.mSegments[0]->mID == segment.mID)
		{
			for (int v = 0; v < segment.mCurves.size(); v++)
			{
				const auto& curvePoint = segment.mCurves[v]->mPoints[0];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << 0 << "_curve_" << v;

				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mState->mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					SequenceEditorTypes::TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					SequenceEditorTypes::TanPointTypes::OUT,
					drawList);
			}
		}

		// draw control points of curves
		// we ignore the first and last because they are controlled by the start & end value of the segment
		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			for (int i = 1; i < segment.mCurves[v]->mPoints.size() - 1; i++)
			{
				// get the curvepoint and generate a unique ID for the control point
				const auto& curvePoint = segment.mCurves[v]->mPoints[i];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << i << "_curve_" << v;
				std::string pointID = stringStream.str();

				// determine the point at where to draw the control point
				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mState->mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				// handle mouse hovering
				bool hovered = false;
				if (mState->mIsWindowFocused)
				{
					if ((mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::None>() ||
						mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringControlPoint>() ||
						mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringCurve>())
						&& ImGui::IsMouseHoveringRect(
							{ circlePoint.x - 5, circlePoint.y - 5 },
							{ circlePoint.x + 5, circlePoint.y + 5 }))
					{
						hovered = true;
					}
				}

				if (hovered)
				{
					// if we are hovering this point, store ID
					mState->mAction.currentAction = SequenceGUIActions::HoveringControlPoint();
					mState->mAction.currentObjectID = pointID;

					//
					showValue<T>(
						track,
						segment,
						curvePoint.mPos.mTime,
						curvePoint.mPos.mTime * segment.mDuration + segment.mStartTime,
						v);

					// is the mouse held down, then we are dragging
					if (ImGui::IsMouseDown(0))
					{
						mState->mAction.currentAction = SequenceGUIActions::DraggingControlPoint();
						mState->mAction.currentActionData = std::make_unique<SequenceGUIControlPointData>(
							track.mID,
							segment.mID,
							i,
							v);
						mState->mAction.currentObjectID = segment.mID;
					}
					// if we clicked right mouse button, open curve action popup
					else if (ImGui::IsMouseClicked(1))
					{
						mState->mAction.currentAction = SequenceGUIActions::OpenCurvePointActionPopup();
						mState->mAction.currentActionData = std::make_unique<SequenceGUIControlPointActionData>(
							track.mID,
							segment.mID,
							i,
							v);
						mState->mAction.currentObjectID = segment.mID;
					}
				}
				else
				{
					// otherwise, if we where hovering but not anymore, stop hovering
					if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringControlPoint>() &&
						pointID == mState->mAction.currentObjectID)
					{
						mState->mAction.currentAction = SequenceGUIActions::None();
					}
				}

				if (mState->mIsWindowFocused)
				{
					// handle dragging of control point
					if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::DraggingControlPoint>() &&
						segment.mID == mState->mAction.currentObjectID)
					{
						const SequenceGUIControlPointData* data
							= dynamic_cast<SequenceGUIControlPointData*>(mState->mAction.currentActionData.get());

						if (data->mControlIndex == i && data->mCurveIndex == v)
						{
							float timeAdjust = mState->mMouseDelta.x / segmentWidth;
							float valueAdjust = (mState->mMouseDelta.y / mState->mTrackHeight) * -1.0f;

							hovered = true;

							showValue<T>(
								track,
								segment,
								curvePoint.mPos.mTime,
								curvePoint.mPos.mTime * segment.mDuration + segment.mStartTime,
								v);


							SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

							curveController.changeCurvePoint(
								data->mTrackID,
								data->mSegmentID,
								data->mControlIndex,
								data->mCurveIndex,
								timeAdjust,
								valueAdjust);

							mCurveCache.clear();

							if (ImGui::IsMouseReleased(0))
							{
								mState->mAction.currentAction = SequenceGUIActions::None();
								mState->mAction.currentActionData = nullptr;
							}
						}
					}
				}


				// draw the control point
				drawList->AddCircleFilled(
					circlePoint,
					4.0f,
					hovered ? guicolors::white : guicolors::lightGrey);

				// draw the handlers
				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					SequenceEditorTypes::TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					SequenceEditorTypes::TanPointTypes::OUT,
					drawList);
			}
		}

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// handle last control point
			// overlaps with endvalue so only draw tan handlers
			const int controlPointIndex = segment.mCurves[v]->mPoints.size() - 1;
			const auto& curvePoint = segment.mCurves[v]->mPoints[controlPointIndex];

			std::ostringstream stringStream;
			stringStream << segment.mID << "_point_" << controlPointIndex << "_curve_" << v;
			std::string pointID = stringStream.str();

			ImVec2 circlePoint =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
				trackTopLeft.y + mState->mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				SequenceEditorTypes::TanPointTypes::IN,
				drawList);

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				SequenceEditorTypes::TanPointTypes::OUT,
				drawList);
		}

		//
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mState->mTrackHeight);
	}

	template<typename T>
	void SequenceCurveTrackView::drawSegmentValue(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const SequenceEditorTypes::SegmentValueTypes segmentType,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment =
			static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// calculate point of this value in the window
			ImVec2 segmentValuePos =
			{
				trackTopLeft.x + segmentX - (segmentType == SequenceEditorTypes::BEGIN ? segmentWidth : 0.0f),
				trackTopLeft.y + mState->mTrackHeight * (1.0f - ((segmentType == SequenceEditorTypes::BEGIN ?
					(float)segment.mCurves[v]->mPoints[0].mPos.mValue :
					(float)segment.mCurves[v]->mPoints[segment.mCurves[v]->mPoints.size() - 1].mPos.mValue) / 1.0f))
			};

			bool hovered = false;

			if (mState->mIsWindowFocused)
			{
				// check if we are hovering this value
				if ((mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::None>() ||
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringSegmentValue>() ||
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringSegment>() ||
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringCurve>())
					&& ImGui::IsMouseHoveringRect(
						{ segmentValuePos.x - 12, segmentValuePos.y - 12 }, // top left
						{ segmentValuePos.x + 12, segmentValuePos.y + 12 }))  // bottom right 
				{
					hovered = true;
					mState->mAction.currentAction = SequenceGUIActions::HoveringSegmentValue();
					mState->mAction.currentActionData = std::make_unique<SequenceGUIDragSegmentData>(
						track.mID,
						segment.mID,
						segmentType,
						v);

					if (ImGui::IsMouseDown(0))
					{
						mState->mAction.currentAction = SequenceGUIActions::DraggingSegmentValue();
						mState->mAction.currentObjectID = segment.mID;
					}

					showValue<T>(
						track,
						segment,
						segmentType == SequenceEditorTypes::BEGIN ? 0.0f : 1.0f,
						segmentType == SequenceEditorTypes::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
						v);
				}
				else if (!mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::DraggingSegmentValue>())
				{
					if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringSegmentValue>())
					{
						const SequenceGUIDragSegmentData* data = dynamic_cast<SequenceGUIDragSegmentData*>(mState->mAction.currentActionData.get());

						if (data->mType == segmentType &&
							data->mSegmentID == segment.mID &&
							data->mCurveIndex == v)
						{
							mState->mAction.currentAction = SequenceGUIActions::None();
						}
					}
				}

				// handle dragging segment value
				if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::DraggingSegmentValue>() &&
					mState->mAction.currentObjectID == segment.mID)
				{
					const SequenceGUIDragSegmentData* data = dynamic_cast<SequenceGUIDragSegmentData*>(mState->mAction.currentActionData.get());

					if (data->mType == segmentType && data->mCurveIndex == v)
					{
						hovered = true;
						showValue<T>(
							track,
							segment,
							segmentType == SequenceEditorTypes::BEGIN ? 0.0f : 1.0f,
							segmentType == SequenceEditorTypes::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
							v);

						if (ImGui::IsMouseReleased(0))
						{
							mState->mAction.currentAction = SequenceGUIActions::None();
						}
						else
						{
							float dragAmount = (mState->mMouseDelta.y / mState->mTrackHeight) * -1.0f;

							SequenceControllerCurve& curveController = getEditor().getController<SequenceControllerCurve>();

							curveController.changeCurveSegmentValue(
								track.mID,
								segment.mID,
								dragAmount,
								v,
								segmentType);
							mCurveCache.clear();
						}
					}
				}
			}

			if (hovered)
				drawList->AddCircleFilled(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
			else
				drawList->AddCircle(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
		}
	}

	void SequenceCurveTrackView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{
		// segment handler
		if (mState->mIsWindowFocused &&
			(mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenInsertSegmentPopup>() ||
			(mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringSegment>() && mState->mAction.currentObjectID == segment.mID)) &&
			ImGui::IsMouseHoveringRect(
				{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
				{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + mState->mTrackHeight + 10 }))  // bottom right 
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

					   // we are hovering this segment with the mouse
			mState->mAction.currentAction = SequenceGUIActions::HoveringSegment();
			mState->mAction.currentObjectID = segment.mID;

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mState->mAction.currentAction = SequenceGUIActions::DraggingSegment();
				mState->mAction.currentObjectID = segment.mID;
			}
			// right mouse in deletion popup
			else if (ImGui::IsMouseDown(1))
			{
				std::unique_ptr<SequenceGUIEditSegmentData> editSegmentData = std::make_unique<SequenceGUIEditSegmentData>(
					track.mID,
					segment.mID,
					segment.get_type());
				mState->mAction.currentAction = SequenceGUIActions::OpenEditSegmentPopup();
				mState->mAction.currentObjectID = segment.mID;
				mState->mAction.currentActionData = std::move(editSegmentData);
			}
		}
		else if (
			mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::DraggingSegment>() &&
			mState->mAction.currentObjectID == segment.mID)
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime).c_str());
			ImGui::EndTooltip();

			// do we have the mouse still held down ? drag the segment
			if (ImGui::IsMouseDown(0))
			{
				float amount = mState->mMouseDelta.x / mState->mStepSize;

				auto& editor = getEditor();
				SequenceControllerCurve& curveController = editor.getController<SequenceControllerCurve>();
				curveController.segmentDurationChange(track.mID, segment.mID, amount);
				
				mCurveCache.clear();
			}
			// otherwise... release!
			else if (ImGui::IsMouseReleased(0))
			{
				mState->mAction.currentAction = SequenceGUIActions::None();
			}
		}
		else
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

					   // release if we are not hovering this segment
			if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringSegment>()
				&& mState->mAction.currentObjectID == segment.mID)
			{
				mState->mAction.currentAction = SequenceGUIActions::None();
			}
		}
	}

	template<typename T>
	void SequenceCurveTrackView::drawTanHandler(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		std::ostringstream &stringStream,
		const float segmentWidth,
		const math::FCurvePoint<float, float> &curvePoint,
		const ImVec2 &circlePoint,
		const int controlPointIndex,
		const int curveIndex,
		const SequenceEditorTypes::TanPointTypes type,
		ImDrawList* drawList)
	{
		// draw tan handlers
		{
			// create a string stream to create identifier of this object
			std::ostringstream tanStream;
			tanStream << stringStream.str() << (type == SequenceEditorTypes::TanPointTypes::IN) ? "inTan" : "outTan";

			//
			const math::FComplex<float, float>& tanComplex
				= (type == SequenceEditorTypes::TanPointTypes::IN) ? curvePoint.mInTan : curvePoint.mOutTan;

			// get the offset from the tan
			ImVec2 offset =
			{ (segmentWidth * tanComplex.mTime) / (float)segment.mDuration,
				(mState->mTrackHeight *  (float)tanComplex.mValue * -1.0f) };
			ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

			// set if we are hoverting this point with the mouse
			bool tanPointHovered = false;

			if (mState->mIsWindowFocused)
			{
				// check if hovered
				if ((mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::None>() ||
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringCurve>())
					&& ImGui::IsMouseHoveringRect(
				{ tanPoint.x - 5, tanPoint.y - 5 },
				{ tanPoint.x + 5, tanPoint.y + 5 }))
				{
					mState->mAction.currentAction = SequenceGUIActions::HoveringTanPoint();
					mState->mAction.currentObjectID = tanStream.str();
					tanPointHovered = true;
				}
				else if (
					mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringTanPoint>())
				{
					// if we hare already hovering, check if its this point
					if (mState->mAction.currentObjectID == tanStream.str())
					{
						if (ImGui::IsMouseHoveringRect(
						{ tanPoint.x - 5, tanPoint.y - 5 },
						{ tanPoint.x + 5, tanPoint.y + 5 }))
						{
							// still hovering
							tanPointHovered = true;

							// start dragging if mouse down
							if (ImGui::IsMouseDown(0) && mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringTanPoint>())
							{
								mState->mAction.currentAction = SequenceGUIActions::DraggingTanPoint();

								mState->mAction.currentActionData = std::make_unique<SequenceGUIDragTanPointData>(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type);
							}
						}
						else
						{
							// otherwise, release!
							mState->mAction.currentAction = SequenceGUIActions::None();
						}
					}
				}

				// handle dragging of tan point
				if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::DraggingTanPoint>())
				{
					const SequenceGUIDragTanPointData* data =
						dynamic_cast<SequenceGUIDragTanPointData*>(mState->mAction.currentActionData.get());

					if (data->mSegmentID == segment.mID &&
						data->mControlPointIndex == controlPointIndex &&
						data->mType == type &&
						data->mCurveIndex == curveIndex)
					{
						if (ImGui::IsMouseReleased(0))
						{
							mState->mAction.currentAction = SequenceGUIActions::None();
							mState->mAction.currentActionData = nullptr;
						}
						else
						{
							tanPointHovered = true;

							float time = mState->mMouseDelta.x / mState->mStepSize;
							float value = (mState->mMouseDelta.y / mState->mTrackHeight) * -1.0f;

							auto& curveController = getEditor().getController<SequenceControllerCurve>();
							curveController.changeTanPoint(
								track.mID,
								segment.mID,
								controlPointIndex,
								curveIndex,
								type,
								time,
								value);
							mCurveCache.clear();
						}
					}
				}
			}

			// draw line
			drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

			// draw handler
			drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);
		}
	}


	void SequenceCurveTrackView::handleInsertSegmentPopup()
	{
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenInsertSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Segment");

			mState->mAction.currentAction = SequenceGUIActions::InsertingSegment();
		}

		// handle insert segment popup
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::InsertingSegment>())
		{
			if (ImGui::BeginPopup("Insert Segment"))
			{
				if (ImGui::Button("Insert"))
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mState->mAction.currentActionData.get());

					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.insertSegment(data->mID, data->mTime);
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;

					mCurveCache.clear();

					ImGui::CloseCurrentPopup();

				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction.currentAction = SequenceGUIActions::None();
				mState->mAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceCurveTrackView::handleCurveTypePopup()
	{
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenCurveTypePopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Change Curve Type");

			mState->mAction.currentAction = SequenceGUIActions::CurveTypePopup();
		}

		// handle insert segment popup
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::CurveTypePopup>())
		{
			SequenceGUIChangeCurveData* data = dynamic_cast<SequenceGUIChangeCurveData*>(mState->mAction.currentActionData.get());
			assert(data != nullptr);

			if (ImGui::BeginPopup("Change Curve Type"))
			{
				ImGui::SetWindowPos(data->mWindowPos);

				if (ImGui::Button("Linear"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.changeCurveType(data->mTrackID, data->mSegmentID, math::ECurveInterp::Linear);

					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
					mCurveCache.clear();

				}

				if (ImGui::Button("Bezier"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.changeCurveType(data->mTrackID, data->mSegmentID, math::ECurveInterp::Bezier);

					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
					mCurveCache.clear();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction.currentAction = SequenceGUIActions::None();
				mState->mAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceCurveTrackView::handleInsertCurvePointPopup()
	{
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenInsertCurvePointPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Curve Point");

			mState->mAction.currentAction = SequenceGUIActions::InsertingCurvePoint();
		}

		// handle insert segment popup
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::InsertingCurvePoint>())
		{
			if (ImGui::BeginPopup("Insert Curve Point"))
			{
				SequenceGUIInsertCurvePointData& data = static_cast<SequenceGUIInsertCurvePointData&>(*mState->mAction.currentActionData.get());

				if (ImGui::Button("Insert Point"))
				{
					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.insertCurvePoint(
						data.mTrackID,
						data.mSegmentID,
						data.mPos,
						data.mSelectedIndex);

					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;

				}

				if (ImGui::Button("Change Curve Type"))
				{
					ImGui::CloseCurrentPopup();

					SequenceGUIInsertCurvePointData& data = static_cast<SequenceGUIInsertCurvePointData&>(*mState->mAction.currentActionData.get());

					mState->mAction.currentActionData = std::make_unique<SequenceGUIChangeCurveData>(
						data.mTrackID,
						data.mSegmentID,
						data.mSelectedIndex,
						ImGui::GetWindowPos());
					mState->mAction.currentAction = SequenceGUIActions::OpenCurveTypePopup();

				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction.currentAction = SequenceGUIActions::None();
				mState->mAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceCurveTrackView::handleCurvePointActionPopup()
	{
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenCurvePointActionPopup>())
		{
			mState->mAction.currentAction = SequenceGUIActions::CurvePointActionPopup();
			ImGui::OpenPopup("Curve Point Actions");
		}

		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::CurvePointActionPopup>())
		{
			if (ImGui::BeginPopup("Curve Point Actions"))
			{
				if (ImGui::Button("Delete"))
				{
					const SequenceGUIControlPointActionData* data
						= dynamic_cast<SequenceGUIControlPointActionData*>(mState->mAction.currentActionData.get());

					assert(data != nullptr);

					auto& curveController = getEditor().getController<SequenceControllerCurve>();
					curveController.deleteCurvePoint(
						data->mTrackId,
						data->mSegmentID,
						data->mControlPointIndex,
						data->mCurveIndex);
					mCurveCache.clear();

					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::Button("Cancel"))
				{
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction.currentAction = SequenceGUIActions::None();
				mState->mAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceCurveTrackView::handleDeleteSegmentPopup()
	{
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::OpenEditSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Delete Segment");

			mState->mAction.currentAction = SequenceGUIActions::EditingSegment();
		}

		// handle delete segment popup
		if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::EditingSegment>())
		{
			if (ImGui::BeginPopup("Delete Segment"))
			{
				const SequenceGUIEditSegmentData* data = dynamic_cast<SequenceGUIEditSegmentData*>(mState->mAction.currentActionData.get());
				assert(data != nullptr);

				if (ImGui::Button("Delete"))
				{
					getEditor().getController<SequenceControllerCurve>().deleteSegment(data->mTrackID, data->mSegmentID);
					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction.currentAction = SequenceGUIActions::None();
				mState->mAction.currentActionData = nullptr;
			}
		}
	}


	template<typename T>
	void SequenceCurveTrackView::drawCurves(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float previousSegmentX,
		const float segmentWidth,
		const float segmentX,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		const int resolution = 40;
		bool curveSelected = false;

		bool needsDrawing = ImGui::IsRectVisible({ trackTopLeft.x + previousSegmentX, trackTopLeft.y }, { trackTopLeft.x + previousSegmentX + segmentWidth, trackTopLeft.y + mState->mTrackHeight });

		if (needsDrawing)
		{
			if (mCurveCache.find(segment.mID) == mCurveCache.end())
			{
				std::vector<ImVec2> points;
				points.resize((resolution + 1)*segment.mCurves.size());
				for (int v = 0; v < segment.mCurves.size(); v++)
				{
					for (int i = 0; i <= resolution; i++)
					{
						float value = 1.0f - segment.mCurves[v]->evaluate((float)i / resolution);

						points[i + v * (resolution + 1)] =
						{
							trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
							trackTopLeft.y + value * mState->mTrackHeight
						};
					}
				}
				mCurveCache.emplace(segment.mID, points);
			}
		}


		int selectedCurve = -1;
		if (mState->mIsWindowFocused)
		{
			// determine if mouse is hovering curve
			if ((mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::None>() ||
				mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringCurve>())
				&& ImGui::IsMouseHoveringRect(
			{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }))  // bottom right 
			{
				// translate mouse position to position in curve
				ImVec2 mousePos = ImGui::GetMousePos();
				float xInSegment = ((mousePos.x - (trackTopLeft.x + segmentX - segmentWidth)) / mState->mStepSize) / segment.mDuration;
				float yInSegment = 1.0f - ((mousePos.y - trackTopLeft.y) / mState->mTrackHeight);

				for (int i = 0; i < segment.mCurves.size(); i++)
				{
					// evaluate curve at x position
					float yInCurve = segment.mCurves[i]->evaluate(xInSegment);

					// insert curve point on click
					const float maxDist = 0.1f;
					if (abs(yInCurve - yInSegment) < maxDist)
					{
						mState->mAction.currentAction = SequenceGUIActions::HoveringCurve();
						mState->mAction.currentObjectID = segment.mID;
						mState->mAction.currentActionData = std::make_unique<SequenceGUIHoveringCurveData>(i);

						if (ImGui::IsMouseClicked(1))
						{
							mState->mAction.currentActionData = std::make_unique<SequenceGUIInsertCurvePointData>(
								track.mID,
								segment.mID,
								i,
								xInSegment);
							mState->mAction.currentAction = SequenceGUIActions::OpenInsertCurvePointPopup();
						}
						selectedCurve = i;
					}
				}

				if (selectedCurve == -1)
				{
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}
				else
				{
					showValue<T>(
						track,
						segment,
						xInSegment,
						mState->mMouseCursorTime,
						selectedCurve);
				}
			}
			else
			{
				if (mState->mAction.currentAction.get_type().is_derived_from<SequenceGUIActions::HoveringCurve>() &&
					mState->mAction.currentObjectID == segment.mID)
				{
					mState->mAction.currentAction = SequenceGUIActions::None();
					mState->mAction.currentActionData = nullptr;
				}
			}
		}

		if (needsDrawing)
		{
			for (int i = 0; i < segment.mCurves.size(); i++)
			{
				// draw points of curve
				drawList->AddPolyline(&*mCurveCache[segment.mID].begin() + i * (resolution + 1), // points array
					mCurveCache[segment.mID].size() / segment.mCurves.size(),	 // size of points array
					guicolors::curvecolors[i],								 // color
					false,													 // closed
					selectedCurve == i ? 3.0f : 1.0f,							 // thickness
					true);													 // anti-aliased
			}
		}
	}

	template<typename T>
	void SequenceCurveTrackView::drawSegmentContent(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		const ImVec2& trackTopLeft,
		float previousSegmentX,
		float segmentWidth,
		float segmentX,
		ImDrawList* drawList,
		bool drawStartValue)
	{
		// curve
		drawCurves<T>(
			track,
			segment,
			trackTopLeft,
			previousSegmentX,
			segmentWidth,
			segmentX,
			drawList);


		// draw control points
		drawControlPoints<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
			drawList);

		// if this is the first segment of the track
		// also draw a handler for the start value
		if (drawStartValue)
		{
			// draw segment value handler
			drawSegmentValue<T>(
				track,
				segment,
				trackTopLeft,
				segmentX,
				segmentWidth,
				SequenceEditorTypes::SegmentValueTypes::BEGIN,
				drawList);
		}

		// draw segment value handler
		drawSegmentValue<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
			SequenceEditorTypes::SegmentValueTypes::END,
			drawList);
	}


	template<typename T>
	void SequenceCurveTrackView::drawInspectorRange(const SequenceTrack& track)
	{
		const SequenceTrackCurve<T>& curveTrack = static_cast<const SequenceTrackCurve<T>&>(track);

		T min = curveTrack.mMinimum;
		T max = curveTrack.mMaximum;

		//
		ImGui::PushID(track.mID.c_str());

		float dragFloatX = ImGui::GetCursorPosX() + 40;
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() + 5, ImGui::GetCursorPosY() + 5 });
		ImGui::Text("Min:"); ImGui::SameLine();
		ImGui::PushID("min");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(min, 3))
		{
			auto& curveController = getEditor().getController<SequenceControllerCurve>();
			curveController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::Text("Max:"); ImGui::SameLine();
		ImGui::PushID("max");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(max, 3))
		{
			auto& curveController = getEditor().getController<SequenceControllerCurve>();
			curveController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();

		ImGui::PopID();
	}

	template<typename T>
	void SequenceCurveTrackView::showValue(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<T>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(false);
	}

	template<>
	void SequenceCurveTrackView::showValue<float>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<float>& segment,
		float x,
		double time,
		int curveIndex)
	{
		const SequenceTrackCurve<float>& curveTrack = static_cast<const SequenceTrackCurve<float>&>(track);

		ImGui::BeginTooltip();

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%.3f", segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec2>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec2>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 2);

		const SequenceTrackCurve<glm::vec2>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec2>&>(track);

		ImGui::BeginTooltip();

		glm::vec2 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[2] =
		{
			"x",
			"y"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec3>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec3>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 3);

		const SequenceTrackCurve<glm::vec3>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec3>&>(track);

		ImGui::BeginTooltip();

		glm::vec3 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[3] =
		{
			"x",
			"y",
			"z"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceCurveTrackView::showValue<glm::vec4>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec4>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 4);

		const SequenceTrackCurve<glm::vec4>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec4>&>(track);

		ImGui::BeginTooltip();

		glm::vec4 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[4] =
		{
			"x",
			"y",
			"z",
			"w"
		};

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<typename T>
	bool SequenceCurveTrackView::inputFloat(T &v, int precision)
	{
		assert(true);
		return false;
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<float>(float &v, int precision)
	{
		ImGui::PushItemWidth(100.0f);
		return ImGui::InputFloat("", &v, 0.0f, 0.0f, precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2 &v, int precision)
	{
		ImGui::PushItemWidth(145.0f);
		return ImGui::InputFloat2("", &v[0], precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3 &v, int precision)
	{
		ImGui::PushItemWidth(180.0f);
		return ImGui::InputFloat3("", &v[0], precision);
	}

	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4 &v, int precision)
	{
		ImGui::PushItemWidth(225.0f);
		return ImGui::InputFloat4("", &v[0], precision);
	}

	/**
	* drawSegmentContent
	* draws the contents of a segment
	* @tparam T the type of this segment
	* @param track reference to track
	* @param segment reference to segment
	* @param trackTopLeft topleft position of track
	* @param previousSegmentX the x position of the previous segment
	* @param segmentWidth the width of this segment
	* @param segmentX the x position of this segment
	* @param drawList pointer to drawlist of this track window
	* @param drawStartValue should we draw the start value ? only used in first segment of track
	*/
	template<typename T>
	void drawSegmentContent(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);

	/**
	* drawSegmentValue
	* draws a segments value
	* @tparam T type of segment
	* @param track reference to track
	* @param segment reference to segment
	* @param trackTopLeft tracks topleft position
	* @param segmentX segment x position
	* @param segmentWidth width of segment
	* @param segmentType type of segment
	* @param drawList pointer to window drawlist
	*/
	template<typename T>
	void drawSegmentValue(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, const SequenceEditorTypes::SegmentValueTypes segmentType, ImDrawList* drawList);

	/**
	* drawSegmentHandler
	* draws segment handler
	* @param track reference to track
	* @param segment reference to segment
	* @param trackTopLeft tracks topleft position
	* @param segmentX segment x position
	* @param segmentWidth width of segment
	* @param drawList pointer to window drawlist
	*/
	void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

	/**
	* drawControlPoints
	* draws control points of curve segment
	* @param track reference to track
	* @param segment reference to segment
	* @param trackTopLeft tracks topleft position
	* @param segmentX segment x position
	* @param segmentWidth width of segment
	* @param drawList pointer to window drawlist
	*/
	template<typename T>
	void drawControlPoints(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float segmentX, const float segmentWidth, ImDrawList* drawList);

	/**
	* drawCurves
	* draws curves of segment
	* @tparam T the type of this segment
	* @param track reference to track
	* @param segment reference to segment
	* @param trackTopLeft topleft position of track
	* @param previousSegmentX the x position of the previous segment
	* @param segmentWidth the width of this segment
	* @param segmentX the x position of this segment
	* @param drawList pointer to drawlist of this track window
	*/
	template<typename T>
	void drawCurves(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2 &trackTopLeft, const float previousSegmentX, const float segmentWidth, const float segmentX, ImDrawList* drawList);

	/**
	* drawTanHandler
	* draws handlers of curve point
	* @tparam T type of segment
	* @param track reference to track
	* @param segment reference to segment
	* @param stringStream stringstream, used to keep track of object id
	* @param segmentWidth width of segment
	* @param curvePoint reference to curvePoint
	* @param circlePoint circlePoint position
	* @param controlPointIndex index of control point
	* @param curveIndex index of curve
	* @param type tangent type ( in or out )
	* @param drawList pointer to window drawlist
	*/
	template<typename T>
	void drawTanHandler(const SequenceTrack &track, const SequenceTrackSegment &segment, std::ostringstream &stringStream, const float segmentWidth, const math::FCurvePoint<float, float> &curvePoint, const ImVec2 &circlePoint, const int controlPointIndex, const int curveIndex, const SequenceEditorTypes::TanPointTypes type, ImDrawList* drawList);

	/**
	* handleInsertSegmentPopup
	* handles insert segment popup
	*/
	void handleInsertSegmentPopup();

	/**
	* handleDeleteSegmentPopup
	* handles delete segment popup
	*/
	void handleDeleteSegmentPopup();

	/**
	* handleInsertCurvePointPopup
	* handles insert curve point popup
	*/
	void handleInsertCurvePointPopup();

	/**
	* handleCurvePointActionPopup
	* handles curvepoint action popup
	*/
	void handleCurvePointActionPopup();

	/**
	* handleCurveTypePopup
	* handles curve type popup
	*/
	void handleCurveTypePopup();

	/**
	* drawInspectorRange
	* Draws min/max range of inspector
	* @tparam T type
	* @param track reference to track
	*/
	template<typename T>
	void drawInspectorRange(const SequenceTrack& track);

	/**
	* showValue
	* @tparam T type of value
	* @param track reference to track
	* @param segment reference to segment
	* @param x value to display
	* @param time time in segment
	* @param curveIndex curve index
	*/
	template<typename T>
	void showValue(const SequenceTrack& track, const SequenceTrackSegmentCurve<T>& segment, float x, double time, int curveIndex);

	/**
	* inputFloat
	* input float that takes type T as input
	* @tparam T type of inputFloat
	* @param precision decimal precision
	* @return true if dragged
	*/
	template<typename T>
	bool inputFloat(T &, int precision);
}
