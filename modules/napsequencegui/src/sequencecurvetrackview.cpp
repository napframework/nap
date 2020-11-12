/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencecurvetrackview.h"
#include "napcolors.h"
#include "sequencecontrollercurve.h"
#include "sequenceeditorgui.h"
#include "sequenceplayercurveoutput.h"
#include "sequencetrackcurve.h"

// nap includes
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>

// external includes
#include <iostream>


namespace nap
{
	using namespace SequenceGUIActions;
	using namespace SequenceGUIClipboards;

	SequenceCurveTrackView::SequenceCurveTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state)
		: SequenceTrackView(view, state)
	{
		mPopupHandlers =
		{
			[this](){ return this->handleInsertSegmentPopup(); },
			[this](){ return this->handleCurveTypePopup(); },
			[this](){ return this->handleInsertCurvePointPopup(); },
			[this](){ return this->handleDeleteSegmentPopup(); },
			[this](){ return this->handleCurvePointActionPopup<float>(); },
			[this](){ return this->handleCurvePointActionPopup<glm::vec2>(); },
			[this](){ return this->handleCurvePointActionPopup<glm::vec3>(); },
			[this](){ return this->handleCurvePointActionPopup<glm::vec4>(); },
			[this](){ return this->handleSegmentValueActionPopup<float>(); },
			[this](){ return this->handleSegmentValueActionPopup<glm::vec2>(); },
			[this](){ return this->handleSegmentValueActionPopup<glm::vec3>(); },
			[this](){ return this->handleSegmentValueActionPopup<glm::vec4>(); },
			[this](){ return this->handleTanPointActionPopup(); }
		};
	}


	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceCurveTrackView), [](SequenceEditorGUIView& view, SequenceEditorGUIState& state)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceCurveTrackView>(view, state);
	});


	static bool curveViewRegistrations[4]
	{
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<float>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec2>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec3>), RTTI_OF(SequenceCurveTrackView)),
		SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackCurve<glm::vec4>), RTTI_OF(SequenceCurveTrackView))
	};


	std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr> SequenceCurveTrackView::sDrawCurveSegmentsMap
	{
		{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::drawSegmentContent<float> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceCurveTrackView::drawSegmentContent<glm::vec2> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceCurveTrackView::drawSegmentContent<glm::vec3> },
		{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceCurveTrackView::drawSegmentContent<glm::vec4> }
	};


	static std::unordered_map<rttr::type, std::vector<rttr::type>> sParameterTypesForCurveType
		{
			{ RTTI_OF(SequenceTrackCurveFloat), { { RTTI_OF(ParameterFloat), RTTI_OF(ParameterDouble), RTTI_OF(ParameterLong), RTTI_OF(ParameterInt) } } },
			{ RTTI_OF(SequenceTrackCurveVec2), { { RTTI_OF(ParameterVec2) } } },
			{ RTTI_OF(SequenceTrackCurveVec3), { { RTTI_OF(ParameterVec3) } } }
		};


	static bool isParameterTypeAllowed(rttr::type curveType, rttr::type parameterType)
	{
		auto it = sParameterTypesForCurveType.find(curveType);
		if(it!=sParameterTypesForCurveType.end())
		{
			for(auto& type : sParameterTypesForCurveType[curveType])
			{
				if(parameterType == type)
					return true;
			}
		}

		return false;
	}


	void SequenceCurveTrackView::handleActions()
	{
		if (mState.mAction->isAction<StartDraggingSegment>())
		{
			auto* action = mState.mAction->getDerived<StartDraggingSegment>();
			mState.mAction = createAction<DraggingSegment>(action->mTrackID, action->mSegmentID);
		}
	}


	bool SequenceCurveTrackView::handlePopups()
	{
		for(auto& popup_handler : mPopupHandlers)
		{
			if( popup_handler() )
				return true;
		}

		return false;
	}


	void SequenceCurveTrackView::showInspectorContent(const SequenceTrack &track)
	{
		// draw the assigned parameter
		ImGui::Text("Assigned Output");

		ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
		inspector_cursor_pos.x += 5;
		inspector_cursor_pos.y += 5;
		ImGui::SetCursorPos(inspector_cursor_pos);

		bool assigned = false;
		std::string assigned_id;
		std::vector<std::string> curve_outputs;
		int current_item = 0;
		curve_outputs.emplace_back("none");
		int count = 0;
		const Parameter* assigned_parameter = nullptr;

		for (const auto& input : getEditor().mSequencePlayer->mOutputs)
		{
			if (input.get()->get_type() == RTTI_OF(SequencePlayerCurveOutput))
			{
				auto& curve_output = static_cast<SequencePlayerCurveOutput&>(*input.get());

				if(curve_output.mParameter != nullptr)
				{
					if(isParameterTypeAllowed(track.get_type(), curve_output.mParameter.get()->get_type()))
					{
						count++;

						if (input->mID == track.mAssignedOutputID)
						{
							assigned = true;
							assigned_id = input->mID;
							current_item = count;

							assert(input.get()->get_type() == RTTI_OF(SequencePlayerCurveOutput)); // type mismatch
							assigned_parameter = static_cast<SequencePlayerCurveOutput*>(input.get())->mParameter.get();
						}

						curve_outputs.emplace_back(input->mID);
					}
				}
			}
		}

		ImGui::PushItemWidth(200.0f);
		if (Combo(
			"",
			&current_item, curve_outputs))
		{
			SequenceControllerCurve& curve_controller = getEditor().getController<SequenceControllerCurve>();

			if (current_item != 0)
				curve_controller.assignNewObjectID(track.mID, curve_outputs[current_item]);
			else
				curve_controller.assignNewObjectID(track.mID, "");
		}

		//
		ImGui::PopItemWidth();

		// map of inspectors ranges for curve types
		static std::unordered_map<rttr::type, void(SequenceCurveTrackView::*)(const SequenceTrack&)> s_inspectors{
				{ RTTI_OF(SequenceTrackCurveFloat) , &SequenceCurveTrackView::drawInspectorRange<float> },
				{ RTTI_OF(SequenceTrackCurveVec2) , &SequenceCurveTrackView::drawInspectorRange<glm::vec2> },
				{ RTTI_OF(SequenceTrackCurveVec3) , &SequenceCurveTrackView::drawInspectorRange<glm::vec3> },
				{ RTTI_OF(SequenceTrackCurveVec4) , &SequenceCurveTrackView::drawInspectorRange<glm::vec4> }
			};

		// draw inspector
		auto it = s_inspectors.find(track.get_type());
		assert(it!= s_inspectors.end()); // type not found
		if(it != s_inspectors.end())
		{
			(*this.*it->second)(track);
		}

		// delete track button
		ImGui::Spacing();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	}


	void SequenceCurveTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
	{
		// if dirty, redraw all curves
		if (mState.mDirty)
		{
			mCurveCache.clear();
		}

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (mState.mIsWindowFocused)
		{
			// handle insertion of segment
			if (mState.mAction->isAction<None>())
			{
				if (ImGui::IsMouseHoveringRect(
					trackTopLeft, // top left position
					{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }))
				{
					// position of mouse in track
					draw_list->AddLine(
						{ mState.mMousePos.x, trackTopLeft.y }, // top left
						{ mState.mMousePos.x, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness

					ImGui::BeginTooltip();

					ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());

					ImGui::EndTooltip();

					// right mouse down
					if (ImGui::IsMouseClicked(1))
					{
						double time = mState.mMouseCursorTime;

						//
						mState.mAction = createAction<OpenInsertSegmentPopup>(track.mID, time, track.get_type());
					}
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<OpenInsertSegmentPopup>())
			{
				auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

				if (action->mTrackID == track.mID)
				{
					// position of insertion in track
					draw_list->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<InsertingSegment>())
			{
				auto* action = mState.mAction->getDerived<InsertingSegment>();

				if (action->mTrackID == track.mID)
				{
					// position of insertion in track
					draw_list->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						guicolors::lightGrey, // color
						1.0f); // thickness
				}
			}
		}

		float previous_segment_x = 0.0f;

		int segment_count = 0;
		for (const auto& segment : track.mSegments)
		{
			float segment_x	   = (segment->mStartTime + segment->mDuration) * mState.mStepSize;
			float segment_width = segment->mDuration * mState.mStepSize;

			auto it = sDrawCurveSegmentsMap.find(segment.get()->get_type());
			if (it != sDrawCurveSegmentsMap.end())
			{
				(*this.*it->second)(track, *segment.get(), trackTopLeft, previous_segment_x, segment_width, segment_x,
									draw_list, (segment_count == 0));
			}

			// draw segment handlers
			drawSegmentHandler(
				track,
				*segment.get(),
				trackTopLeft, segment_x, segment_width, draw_list);

			//
			previous_segment_x = segment_x;

			//
			segment_count++;
		}
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
				const auto& curve_point = segment.mCurves[v]->mPoints[0];
				std::ostringstream string_stream;
				string_stream << segment.mID << "_point_" << 0 << "_curve_" << v;

				ImVec2 circle_point =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
					trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curve_point.mPos.mValue) };

				// only draw tan handlers when we have a bezier
				if( segment.mCurveTypes[v] == math::ECurveInterp::Bezier )
				{
					drawTanHandler<T>(
						track,
						segment, string_stream,
						segmentWidth, curve_point, circle_point,
						0,
						v,
						SequenceCurveEnums::TanPointTypes::IN,
						drawList);

					drawTanHandler<T>(
						track,
						segment, string_stream,
						segmentWidth, curve_point, circle_point,
						0,
						v,
						SequenceCurveEnums::TanPointTypes::OUT,
						drawList);
				}
			}
		}

		// draw control points of curves
		// we ignore the first and last because they are controlled by the start & end value of the segment
		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			for (int i = 1; i < segment.mCurves[v]->mPoints.size() - 1; i++)
			{
				// get the curvepoint and generate a unique ID for the control point
				const auto& curve_point = segment.mCurves[v]->mPoints[i];
				std::ostringstream string_stream;
				string_stream << segment.mID << "_point_" << i << "_curve_" << v;
				std::string point_id = string_stream.str();

				// determine the point at where to draw the control point
				ImVec2 circle_point =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
					trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curve_point.mPos.mValue) };

				// handle mouse hovering
				bool hovered = false;
				if (mState.mIsWindowFocused)
				{
					if ((mState.mAction->isAction<None>() ||
						mState.mAction->isAction<HoveringControlPoint>() ||
						mState.mAction->isAction<HoveringCurve>())
						&& ImGui::IsMouseHoveringRect(
							{circle_point.x - 5, circle_point.y - 5 },
							{circle_point.x + 5, circle_point.y + 5 }))
					{
						hovered = true;
					}
				}

				if (hovered)
				{
					// if we are hovering this point, store ID
					mState.mAction = createAction<HoveringControlPoint>(
						track.mID,
						segment.mID,
						i,
						v);

					//
					showValue<T>(
						track,
						segment, curve_point.mPos.mTime,
						curve_point.mPos.mTime * segment.mDuration + segment.mStartTime,
						v);

					// is the mouse held down, then we are dragging
					if (ImGui::IsMouseDown(0))
					{
						mState.mAction = createAction<DraggingControlPoint>(
							track.mID,
							segment.mID,
							i,
							v);
					}
					// if we clicked right mouse button, open curve action popup
					else if (ImGui::IsMouseClicked(1))
					{
						mState.mAction = createAction<OpenCurvePointActionPopup<T>>(
							track.mID,
							segment.mID,
							i,
							v,
							curve_point.mPos.mValue,
							curve_point.mPos.mTime,
							static_cast<const SequenceTrackCurve<T>&>(track).mMinimum,
							static_cast<const SequenceTrackCurve<T>&>(track).mMaximum);
					}
				}
				else
				{
					// otherwise, if we where hovering but not anymore, stop hovering
					if (mState.mAction->isAction<HoveringControlPoint>())
					{
						auto* action = mState.mAction->getDerived<HoveringControlPoint>();
						if (action->mControlPointIndex == i && track.mID == action->mTrackID && segment.mID == action->mSegmentID && v == action->mCurveIndex)
						{
							mState.mAction = createAction<None>();
						}
					}
				}

				if (mState.mIsWindowFocused)
				{
					// handle dragging of control point
					if (mState.mAction->isAction<DraggingControlPoint>())
					{
						auto* action = mState.mAction->getDerived<DraggingControlPoint>();

						if (action->mSegmentID == segment.mID)
						{
							if (action->mControlPointIndex == i && action->mCurveIndex == v)
							{
								float time_adjust = mState.mMouseDelta.x / segmentWidth;
								float value_adjust = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;

								hovered = true;

								showValue<T>(
									track,
									segment, curve_point.mPos.mTime,
									curve_point.mPos.mTime * segment.mDuration + segment.mStartTime,
									v);


								// get editor
								auto& editor = getEditor();

								// get controller for this track type
								auto* controller = editor.getControllerWithTrackType(track.get_type());

								// assume its a curve controller
								auto* curve_controller = dynamic_cast<SequenceControllerCurve*>(controller);
								assert(curve_controller!= nullptr); // cast failed

								if( curve_controller != nullptr )
								{
									curve_controller->changeCurvePoint(
										action->mTrackID,
										action->mSegmentID,
										action->mControlPointIndex,
										action->mCurveIndex,
										curve_point.mPos.mTime + time_adjust,
										curve_point.mPos.mValue + value_adjust);
								}

								mCurveCache.clear();

								if (ImGui::IsMouseReleased(0))
								{
									mState.mAction = createAction<None>();
								}
							}
						}
					}
				}

				// draw the control point
				drawList->AddCircleFilled(circle_point,
					4.0f,
					hovered ? guicolors::white : guicolors::lightGrey);

				if( segment.mCurveTypes[v] == math::ECurveInterp::Bezier )
				{
					// draw the handlers
					drawTanHandler<T>(
						track,
						segment, string_stream,
						segmentWidth, curve_point, circle_point,
						i,
						v,
						SequenceCurveEnums::TanPointTypes::IN,
						drawList);

					drawTanHandler<T>(
						track,
						segment, string_stream,
						segmentWidth, curve_point, circle_point,
						i,
						v,
						SequenceCurveEnums::TanPointTypes::OUT,
						drawList);
				}
			}
		}

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// handle last control point
			// overlaps with endvalue so only draw tan handlers
			const int control_point_index = segment.mCurves[v]->mPoints.size() - 1;
			const auto& curve_point		  = segment.mCurves[v]->mPoints[control_point_index];

			std::ostringstream string_stream;
			string_stream << segment.mID << "_point_" << control_point_index << "_curve_" << v;
			std::string point_id = string_stream.str();

			ImVec2 circle_point =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
				trackTopLeft.y + mState.mTrackHeight * (1.0f - (float)curve_point.mPos.mValue) };

			if( segment.mCurveTypes[v] == math::ECurveInterp::Bezier )
			{
				drawTanHandler<T>(
					track,
					segment, string_stream,
					segmentWidth, curve_point, circle_point, control_point_index,
					v,
					SequenceCurveEnums::TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment, string_stream,
					segmentWidth, curve_point, circle_point, control_point_index,
					v,
					SequenceCurveEnums::TanPointTypes::OUT,
					drawList);
			}
		}

		//
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mState.mTrackHeight);
	}

	template<typename T>
	void SequenceCurveTrackView::drawSegmentValue(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const SequenceCurveEnums::SegmentValueTypes segmentType,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment = static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// calculate point of this value in the window
			ImVec2 segment_value_pos =
			{
				trackTopLeft.x + segmentX - (segmentType == SequenceCurveEnums::BEGIN ? segmentWidth : 0.0f),
				trackTopLeft.y + mState.mTrackHeight * (1.0f - ((segmentType == SequenceCurveEnums::BEGIN ?
					(float)segment.mCurves[v]->mPoints[0].mPos.mValue :
					(float)segment.mCurves[v]->mPoints[segment.mCurves[v]->mPoints.size() - 1].mPos.mValue) / 1.0f))
			};

			bool hovered = false;

			if (mState.mIsWindowFocused)
			{
				// check if we are hovering this value
				if ((mState.mAction->isAction<None>() ||
					mState.mAction->isAction<HoveringSegmentValue>() ||
					mState.mAction->isAction<HoveringSegment>() ||
					mState.mAction->isAction<HoveringCurve>()) &&
					ImGui::IsMouseHoveringRect(
						{segment_value_pos.x - 12, segment_value_pos.y - 12 }, // top left
						{segment_value_pos.x + 12, segment_value_pos.y + 12 }))  // bottom right
				{
					hovered = true;
					mState.mAction = createAction<HoveringSegmentValue>(
						track.mID,
						segment.mID,
						segmentType,
						v);

					if (ImGui::IsMouseDown(0))
					{
						mState.mAction = createAction<DraggingSegmentValue>(
							track.mID,
							segment.mID,
							segmentType,
							v);
					}
					else if (ImGui::IsMouseDown(1))
					{
						const SequenceTrackSegmentCurve<T>& curve_segment = static_cast<const SequenceTrackSegmentCurve<T>&>(segment);
						const SequenceTrackCurve<T>& curve_track = static_cast<const SequenceTrackCurve<T>&>(track);

						mState.mAction = createAction<OpenEditSegmentCurveValuePopup<T>>(
							track.mID,
							segment.mID,
							segmentType,
							v,
							(segmentType == SequenceCurveEnums::SegmentValueTypes::BEGIN) ? curve_segment.getStartValue() : curve_segment.getEndValue(),
							curve_track.mMinimum,
							curve_track.mMaximum);
					}

					showValue<T>(
						track,
						segment,
						segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f,
						segmentType == SequenceCurveEnums::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
						v);
				}
				else if (!mState.mAction->isAction<DraggingSegmentValue>())
				{
					if (mState.mAction->isAction<HoveringSegmentValue>())
					{
						auto* action = mState.mAction->getDerived<HoveringSegmentValue>();

						if (action->mType == segmentType &&
							action->mSegmentID == segment.mID &&
							action->mCurveIndex == v)
						{
							mState.mAction = createAction<None>();

							showValue<T>(
								track,
								segment,
								segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f,
								segmentType == SequenceCurveEnums::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
								v);
						}
					}
				}

				// handle dragging segment value
				if (mState.mAction->isAction<DraggingSegmentValue>())
				{
					auto* action = mState.mAction->getDerived<DraggingSegmentValue>();
					if (action->mSegmentID == segment.mID)
					{
						if (action->mType == segmentType && action->mCurveIndex == v)
						{
							hovered = true;
							showValue<T>(
								track,
								segment,
								segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f,
								segmentType == SequenceCurveEnums::BEGIN ? segment.mStartTime : segment.mStartTime + segment.mDuration,
								v);

							if (ImGui::IsMouseReleased(0))
							{
								mState.mAction = createAction<None>();
							}
							else
							{
								float drag_amount = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;
								

								static std::unordered_map<rttr::type, float(*)(const SequenceTrackSegment&, int, SequenceCurveEnums::SegmentValueTypes)> get_value_map
								{
									{ RTTI_OF(float), [](const SequenceTrackSegment& segment, int curveIndex, SequenceCurveEnums::SegmentValueTypes segmentType)->float{
										return static_cast<const SequenceTrackSegmentCurve<float>&>(segment).getValue(segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f);
									}},
									{ RTTI_OF(glm::vec2), [](const SequenceTrackSegment& segment, int curveIndex, SequenceCurveEnums::SegmentValueTypes segmentType)->float {
										return static_cast<const SequenceTrackSegmentCurve<glm::vec2>&>(segment).getValue(segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f)[curveIndex];
									} },
									{ RTTI_OF(glm::vec3), [](const SequenceTrackSegment& segment, int curveIndex, SequenceCurveEnums::SegmentValueTypes segmentType)->float {
										return static_cast<const SequenceTrackSegmentCurve<glm::vec3>&>(segment).getValue(segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f)[curveIndex];
									}},
									{ RTTI_OF(glm::vec4), [](const SequenceTrackSegment& segment, int curveIndex, SequenceCurveEnums::SegmentValueTypes segmentType)->float {
										return static_cast<const SequenceTrackSegmentCurve<glm::vec4>&>(segment).getValue(segmentType == SequenceCurveEnums::BEGIN ? 0.0f : 1.0f)[curveIndex];
									}}
								};

								SequenceControllerCurve& curve_controller = getEditor().getController<SequenceControllerCurve>();
								float value = get_value_map[RTTI_OF(T)](segment, v, segmentType);

								curve_controller.changeCurveSegmentValue(
									track.mID,
									segment.mID, 
									value + drag_amount,
									v,
									segmentType);
								mCurveCache.clear();
							}
						}
					}
				}
			}

			if (hovered)
				drawList->AddCircleFilled(segment_value_pos, 5.0f, guicolors::curvecolors[v]);
			else
				drawList->AddCircle(segment_value_pos, 5.0f, guicolors::curvecolors[v]);
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
		if (mState.mIsWindowFocused &&
			((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringSegment>()) ||
			 (mState.mAction->isAction<StartDraggingSegment>() && mState.mAction->getDerived<StartDraggingSegment>()->mSegmentID != segment.mID))&&
			ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
					{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + mState.mTrackHeight + 10 }))  // bottom right 
		{
			
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			// we are hovering this segment with the mouse
			mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime+segment.mDuration).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mState.mAction = createAction<StartDraggingSegment>(track.mID, segment.mID);
			}
			// right mouse in deletion popup
			else if (ImGui::IsMouseDown(1))
			{
				mState.mAction = createAction <OpenEditCurveSegmentPopup>(
					track.mID,
					segment.mID,
					segment.get_type(),
					segment.mStartTime,
					segment.mDuration
				);
			}
		}
		else if (mState.mAction->isAction<DraggingSegment>())
		{
			auto* action = mState.mAction->getDerived<DraggingSegment>();
			if (action->mSegmentID == segment.mID)
			{
				// draw handler of segment duration
				drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
					guicolors::white, // color
					3.0f); // thickness

				ImGui::BeginTooltip();
				ImGui::Text(formatTimeString(segment.mStartTime+segment.mDuration).c_str());
				ImGui::EndTooltip();

				// do we have the mouse still held down ? drag the segment
				if (ImGui::IsMouseDown(0))
				{
					float amount = mState.mMouseDelta.x / mState.mStepSize;

					// get editor
					auto& editor = getEditor();

					// get controller for this track type
					auto* controller = editor.getControllerWithTrackType(track.get_type());

					// assume its a curve controller
					auto* curve_controller = dynamic_cast<SequenceControllerCurve*>(controller);
					assert(curve_controller!= nullptr); // cast failed

					// change duration
					if(curve_controller!= nullptr)
					{
						curve_controller->segmentDurationChange(track.mID, segment.mID, segment.mDuration + amount);
					}

					mCurveCache.clear();
				}
				// otherwise... release!
				else if (ImGui::IsMouseReleased(0))
				{
					mState.mAction = createAction<None>();
				}
			}
			else
			{
				// draw handler of segment duration
				drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
					guicolors::white, // color
					1.0f); // thickness
			}
		}
		else
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

			// release if we are not hovering this segment
			if (mState.mAction->isAction<HoveringSegment>()
				&& mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID)
			{
				mState.mAction = createAction<None>();
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
		const SequenceCurveEnums::TanPointTypes type,
		ImDrawList* drawList)
	{
		// draw tan handlers
		{
			// create a string stream to create identifier of this object
			std::ostringstream tan_stream;
			tan_stream << stringStream.str() << ( (type == SequenceCurveEnums::TanPointTypes::IN) ? "inTan" : "outTan" );

			//
			const math::FComplex<float, float>& tan_complex = (type == SequenceCurveEnums::TanPointTypes::IN) ? curvePoint.mInTan : curvePoint.mOutTan;

			// get the offset from the tan
			ImVec2 offset =
			{ (segmentWidth * tan_complex.mTime) / (float)segment.mDuration,
				(mState.mTrackHeight *  (float)tan_complex.mValue * -1.0f) };
			ImVec2 tan_point = { circlePoint.x + offset.x, circlePoint.y + offset.y };

			// set if we are hoverting this point with the mouse
			bool tan_point_hovered = false;

			if (mState.mIsWindowFocused)
			{
				// check if hovered
				if ((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurve>())
					&& ImGui::IsMouseHoveringRect({tan_point.x - 5, tan_point.y - 5 }, {tan_point.x + 5, tan_point.y + 5 }))
				{
					mState.mAction = createAction<HoveringTanPoint>(tan_stream.str());
					tan_point_hovered = true;
				}
				else if (mState.mAction->isAction<HoveringTanPoint>())
				{
					auto* action = mState.mAction->getDerived<HoveringTanPoint>();

					// if we hare already hovering, check if its this point
					if (action->mTanPointID == tan_stream.str())
					{
						if (ImGui::IsMouseHoveringRect(
							{tan_point.x - 5, tan_point.y - 5 },
							{tan_point.x + 5, tan_point.y + 5 }))
						{
							// still hovering
							tan_point_hovered = true;

							// start dragging if mouse down
							if (ImGui::IsMouseDown(0))
							{
								mState.mAction = createAction<DraggingTanPoint>(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type);
							}else if(ImGui::IsMouseDown(1)) // open edit popup
							{
								mState.mAction = createAction<OpenEditTanPointPopup>(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type,
									(float)tan_complex.mValue,
									tan_complex.mTime);
							}
						}
						else
						{
							// otherwise, release!
							mState.mAction = createAction<None>();
						}
					}
				}

				// handle dragging of tan point
				if (mState.mAction->isAction<DraggingTanPoint>())
				{
					auto* action = mState.mAction->getDerived<DraggingTanPoint>();

					if (action->mSegmentID == segment.mID &&
						action->mControlPointIndex == controlPointIndex &&
						action->mType == type &&
						action->mCurveIndex == curveIndex)
					{
						if (ImGui::IsMouseReleased(0))
						{
							mState.mAction = createAction<None>();
						}
						else
						{
							tan_point_hovered = true;

							float delta_time = mState.mMouseDelta.x / mState.mStepSize;
							float delta_value = (mState.mMouseDelta.y / mState.mTrackHeight) * -1.0f;

							const auto& curve_segment = static_cast<const SequenceTrackSegmentCurve<T>&>(segment);

							float new_time;
							float new_value;
							if( type == SequenceCurveEnums::TanPointTypes::IN )
							{
								new_time = curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mInTan.mTime + delta_time;
								new_value= curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mInTan.mValue + delta_value;
							}
							else
							{
								new_time = curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mOutTan.mTime + delta_time;
								new_value= curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mOutTan.mValue + delta_value;
							}

							// get editor
							auto& editor = getEditor();

							// get controller for this track type
							auto* controller = editor.getControllerWithTrackType(track.get_type());

							// assume its a curve controller
							auto* curve_controller = dynamic_cast<SequenceControllerCurve*>(controller);
							assert(curve_controller!= nullptr); // cast failed

							// change duration
							if(curve_controller!= nullptr)
							{
								curve_controller->changeTanPoint(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type,
									new_time,
									new_value);
							}

							mState.mDirty = true;
						}
					}
				}
			}

			// draw line
			drawList->AddLine(circlePoint, tan_point, tan_point_hovered ? guicolors::white : guicolors::darkGrey, 1.0f);

			// draw handler
			drawList->AddCircleFilled(tan_point, 3.0f, tan_point_hovered ? guicolors::white : guicolors::darkGrey);
		}
	}


	bool SequenceCurveTrackView::handleInsertSegmentPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenInsertSegmentPopup>())
		{
			auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

			if (action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec4))
			{
				// invoke insert sequence popup
				ImGui::OpenPopup("Insert Segment");

				auto* action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

				mState.mAction = createAction<InsertingSegment>(action->mTrackID, action->mTime, action->mTrackType);
			}
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingSegment>())
		{
			auto* action = mState.mAction->getDerived<InsertingSegment>();

			if (action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec4))
			{
				if (ImGui::BeginPopup("Insert Segment"))
				{
					handled = true;

					if (ImGui::Button("Insert"))
					{
						auto* action = mState.mAction->getDerived<InsertingSegment>();

						auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
						curve_controller.insertSegment(action->mTrackID, action->mTime);
						mState.mAction = createAction<None>();

						mCurveCache.clear();

						ImGui::CloseCurrentPopup();

					}

					// handle paste
					if( mState.mClipboard->isClipboard<CurveSegmentClipboard>())
					{
						if( ImGui::Button("Paste") )
						{
							// call the correct pasteClipboardSegment method for this track-type
							if( action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) )
							{
								pasteClipboardSegment<SequenceTrackSegmentCurveFloat>(action->mTrackID,
																					  action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ){
								pasteClipboardSegment<SequenceTrackSegmentCurveVec2>(action->mTrackID,
																					 action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ){
								pasteClipboardSegment<SequenceTrackSegmentCurveVec3>(action->mTrackID,
																					 action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec4) ){
								pasteClipboardSegment<SequenceTrackSegmentCurveVec4>(action->mTrackID,
																					 action->mTime);
							}

							mState.mDirty = true;
							mState.mAction = createAction<None>();
							ImGui::CloseCurrentPopup();
						}
					}

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}

					ImGui::EndPopup();
				}
				else
				{
					// click outside popup so cancel action
					mState.mAction = createAction<None>();
				}
			}
		}

		return handled;
	}


	bool SequenceCurveTrackView::handleCurveTypePopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenCurveTypePopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Change Curve Type");

			auto* action = mState.mAction->getDerived<OpenCurveTypePopup>();
			mState.mAction = createAction<CurveTypePopup>(
				action->mTrackID, 
				action->mSegmentID,
				action->mCurveIndex, 
				action->mPos,
				action->mWindowPos);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<CurveTypePopup>())
		{
			auto* action = mState.mAction->getDerived<CurveTypePopup>();

			if (ImGui::BeginPopup("Change Curve Type"))
			{
				handled = true;

				ImGui::SetWindowPos(action->mWindowPos);

				if (ImGui::Button("Linear"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Linear, action->mCurveIndex);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();

				}

				if (ImGui::Button("Bezier"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Bezier, action->mCurveIndex);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}


	bool SequenceCurveTrackView::handleInsertCurvePointPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenInsertCurvePointPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Curve Point");

			auto* action = mState.mAction->getDerived<OpenInsertCurvePointPopup>();
			mState.mAction = createAction<InsertingCurvePoint>(
				action->mTrackID, 
				action->mSegmentID, 
				action->mSelectedIndex, 
				action->mPos);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingCurvePoint>())
		{
			if (ImGui::BeginPopup("Insert Curve Point"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<InsertingCurvePoint>();
				if (ImGui::Button("Insert Point"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.insertCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mPos,
						action->mSelectedIndex);

					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();

				}

				if (ImGui::Button("Change Curve Type"))
				{
					ImGui::CloseCurrentPopup();

					mState.mAction = createAction<OpenCurveTypePopup>(
						action->mTrackID,
						action->mSegmentID,
						action->mSelectedIndex,
						action->mPos,
						ImGui::GetWindowPos());
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}

	template<>
	bool SequenceCurveTrackView::handleCurvePointActionPopup<float>()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenCurvePointActionPopup<float>>())
		{
			auto* action = mState.mAction->getDerived<OpenCurvePointActionPopup<float>>();
			mState.mAction = createAction<CurvePointActionPopup<float>>(
					action->mTrackID,
					action->mSegmentID,
					action->mControlPointIndex,
					action->mCurveIndex,
					action->mValue,
					action->mTime,
					action->mMinimum,
					action->mMaximum
				);
			ImGui::OpenPopup("Curve Point Actions");
		}

		if (mState.mAction->isAction<CurvePointActionPopup<float>>())
		{
			if (ImGui::BeginPopup("Curve Point Actions"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<CurvePointActionPopup<float>>();

				if (ImGui::Button("Delete"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.deleteCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex);
					mCurveCache.clear();

					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				float value = action->mValue * (action->mMaximum - action->mMinimum) + action->mMinimum;
				if (ImGui::InputFloat("value", &value))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex,
						action->mTime,
						value);
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}


	template<typename T>
	bool SequenceCurveTrackView::handleCurvePointActionPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenCurvePointActionPopup<T>>())
		{
			auto* action = mState.mAction->getDerived<OpenCurvePointActionPopup<T>>();
			mState.mAction = createAction<CurvePointActionPopup<T>>(
				action->mTrackID,
				action->mSegmentID,
				action->mControlPointIndex,
				action->mCurveIndex,
				action->mValue,
				action->mTime,
				action->mMinimum,
				action->mMaximum );
			ImGui::OpenPopup("Curve Point Actions");
		}

		if (mState.mAction->isAction<CurvePointActionPopup<T>>())
		{
			if (ImGui::BeginPopup("Curve Point Actions"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<CurvePointActionPopup<T>>();
				int curveIndex = action->mCurveIndex;

				if (ImGui::Button("Delete"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.deleteCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex);
					mCurveCache.clear();

					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				float value = action->mValue * (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]) + action->mMinimum[curveIndex];
				if (ImGui::InputFloat("value", &value))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex,
						action->mTime,
						value);
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}


	bool SequenceCurveTrackView::handleTanPointActionPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenEditTanPointPopup>())
		{
			auto* action = mState.mAction->getDerived<OpenEditTanPointPopup>();
			mState.mAction = createAction<EditingTanPointPopup>(
				action->mTrackID,
				action->mSegmentID,
				action->mControlPointIndex,
				action->mCurveIndex,
				action->mType,
				action->mValue,
				action->mTime);

			ImGui::OpenPopup("Tan Point Actions");
		}

		if (mState.mAction->isAction<EditingTanPointPopup>())
		{
			if (ImGui::BeginPopup("Tan Point Actions"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<EditingTanPointPopup>();
				int curveIndex = action->mCurveIndex;

				bool change = false;
				if (ImGui::InputFloat("time", &action->mTime))
				{
					change = true;
				}

				if (ImGui::InputFloat("value", &action->mValue))
				{
					change = true;
				}

				if(change)
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeTanPoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex,
						action->mType,
						action->mTime,
						action->mValue);
					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}


	template<typename T>
	bool SequenceCurveTrackView::handleSegmentValueActionPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenEditSegmentCurveValuePopup<T>>())
		{
			auto* action = mState.mAction->getDerived<OpenEditSegmentCurveValuePopup<T>>();
			mState.mAction = createAction<EditingSegmentCurveValue<T>>(
				action->mTrackID,
				action->mSegmentID,
				action->mType,
				action->mCurveIndex,
				action->mValue,
				action->mMinimum,
				action->mMaximum
				);
			ImGui::OpenPopup("Segment Value Actions");
		}

		if (mState.mAction->isAction<EditingSegmentCurveValue<T>>())
		{
			if (ImGui::BeginPopup("Segment Value Actions"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<EditingSegmentCurveValue<T>>();
				int curveIndex = action->mCurveIndex;

				float value = action->mValue[curveIndex] * (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]) + action->mMinimum[curveIndex];
				if (ImGui::InputFloat("value", &value))
				{
					float translated_value = (value - action->mMinimum[curveIndex]) / (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]);
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveSegmentValue(
						action->mTrackID,
						action->mSegmentID,
						translated_value,
						curveIndex,
						action->mType);
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}


	template<>
	bool SequenceCurveTrackView::handleSegmentValueActionPopup<float>()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenEditSegmentCurveValuePopup<float>>())
		{
			auto* action = mState.mAction->getDerived<OpenEditSegmentCurveValuePopup<float>>();
			mState.mAction = createAction<EditingSegmentCurveValue<float>>(
				action->mTrackID,
				action->mSegmentID,
				action->mType,
				action->mCurveIndex,
				action->mValue,
				action->mMinimum,
				action->mMaximum
				);
			ImGui::OpenPopup("Segment Value Actions");
		}

		if (mState.mAction->isAction<EditingSegmentCurveValue<float>>())
		{
			if (ImGui::BeginPopup("Segment Value Actions"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<EditingSegmentCurveValue<float>>();
				int curveIndex = action->mCurveIndex;

				float value = action->mValue * (action->mMaximum - action->mMinimum) + action->mMinimum;
				if (ImGui::InputFloat("value", &value))
				{
					float translated_value = (value - action->mMinimum) / (action->mMaximum - action->mMinimum);
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveSegmentValue(
						action->mTrackID,
						action->mSegmentID,
						translated_value,
						curveIndex,
						action->mType
					);
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = createAction<None>();
			}
		}

		return handled;
	}



	bool SequenceCurveTrackView::handleDeleteSegmentPopup()
	{
		bool handled = false;

		if (mState.mAction->isAction<OpenEditCurveSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Delete Segment");

			auto* action = mState.mAction->getDerived<OpenEditCurveSegmentPopup>();

			mState.mAction = createAction<EditingCurveSegment>(
				action->mTrackID,
				action->mSegmentID,
				action->mSegmentType,
				action->mStartTime,
				action->mDuration
			);
		}

		// handle delete segment popup
		if (mState.mAction->isAction<EditingCurveSegment>())
		{
			auto* action = mState.mAction->getDerived<EditingCurveSegment>();
			auto& controller = getEditor().getController<SequenceControllerCurve>();
			auto* track = controller.getTrack(action->mTrackID);

			if (track->get_type() == RTTI_OF(SequenceTrackCurve<float>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec2>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec3>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec4>) )
			{
				if(ImGui::BeginPopup("Delete Segment"))
				{
					handled = true;

					auto& controller = getEditor().getController<SequenceControllerCurve>();
					auto* action = mState.mAction->getDerived<EditingCurveSegment>();

					if( ImGui::Button("Copy") )
					{
						const auto* curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);
						mState.mClipboard = createClipboard<CurveSegmentClipboard>(action->mSegmentType);
						if( !mState.mClipboard->serialize(curve_segment) )
						{
							nap::Logger::error("Error serializing curve segment");
							mState.mClipboard = createClipboard<Empty>();
						}
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}

					if (ImGui::Button("Delete"))
					{
						controller.deleteSegment(
							action->mTrackID,
							action->mSegmentID);
						mCurveCache.clear();

						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}

					int time_milseconds = (int) ( ( action->mStartTime + action->mDuration ) * 100.0 ) % 100;
					int time_seconds = (int) ( action->mStartTime + action->mDuration ) % 60;
					int time_minutes = (int) ( action->mStartTime + action->mDuration ) / 60;

					bool edit_time = false;

					ImGui::Separator();

					ImGui::PushItemWidth(100.0f);

					int time_array[3] =
						{
							time_minutes,
							time_seconds,
							time_milseconds
						};

					edit_time = ImGui::InputInt3("Time (mm:ss:ms)", &time_array[0]);
					time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
					time_array[1] = math::clamp<int>(time_array[1], 0, 59);
					time_array[2] = math::clamp<int>(time_array[2], 0, 99);

					if( edit_time )
					{
						double new_time = ( ( (double) time_array[2] )  / 100.0 ) + (double) time_array[1] + ( (double) time_array[0] * 60.0 );
						double new_duration = controller.segmentDurationChange(action->mTrackID, action->mSegmentID, new_time - action->mStartTime);
						action->mDuration = new_duration;
						mState.mDirty = true;
					}

					ImGui::PopItemWidth();

					ImGui::Separator();

					if (ImGui::Button("Done"))
					{
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}

					ImGui::EndPopup();
				}
				else
				{
					// click outside popup so cancel action
					mState.mAction = createAction<None>();
				}
			}
		}

		return handled;
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

		const float points_per_pixel = 0.5f;

		bool needs_drawing = ImGui::IsRectVisible({ trackTopLeft.x + previousSegmentX, trackTopLeft.y }, { trackTopLeft.x + previousSegmentX + segmentWidth, trackTopLeft.y + mState.mTrackHeight });

		if (needs_drawing)
		{
			// if no cache present, create new curve
			if (mCurveCache.find(segment.mID) == mCurveCache.end())
			{
				const long point_num = (int) (points_per_pixel * segmentWidth );
				std::vector<std::vector<ImVec2>> curves;
				for (int v = 0; v < segment.mCurves.size(); v++)
				{
					std::vector<ImVec2> curve;
					if(point_num>0)
					{
						float start_x = math::max<float>(trackTopLeft.x, 0);
						float end_x = start_x + mState.mWindowSize.x + mState.mWindowPos.x;

						// start drawing at window pos
						{
							long i = 0;
							for (; i <= point_num; i++)
							{
								float p = (float)i / point_num;
								float x = trackTopLeft.x + previousSegmentX + segmentWidth * p;
								if (x > start_x)
								{
									float value = 1.0f - segment.mCurves[v]->evaluate(p);
									ImVec2 point =
									{
										x,
										trackTopLeft.y + value * mState.mTrackHeight
									};
									curve.emplace_back(point);
								}

								if (x > end_x)
								{
									break; // no longer visible on right side, continuation of this loop is not necessary
								}								
							}
						}
					}

					curves.emplace_back(curve);
				}
				mCurveCache.emplace(segment.mID, curves);
			}
		}

		int selected_curve = -1;
		if (mState.mIsWindowFocused)
		{
			// determine if mouse is hovering curve
			if ((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurve>())
				&& ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
					{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }))  // bottom right 
			{
				// translate mouse position to position in curve
				ImVec2 mouse_pos = ImGui::GetMousePos();
				float x_in_segment = ((mouse_pos.x - (trackTopLeft.x + segmentX - segmentWidth)) / mState.mStepSize) / segment.mDuration;
				float y_in_segment = 1.0f - ((mouse_pos.y - trackTopLeft.y) / mState.mTrackHeight);

				for (int i = 0; i < segment.mCurves.size(); i++)
				{
					// evaluate curve at x position
					float y_in_curve = segment.mCurves[i]->evaluate(x_in_segment);

					// insert curve point on click
					const float maxDist = 0.1f;
					if (abs(y_in_curve - y_in_segment) < maxDist)
					{
						mState.mAction = createAction<HoveringCurve>(
							track.mID,
							segment.mID,
							i);

						if (ImGui::IsMouseClicked(1))
						{
							mState.mAction = createAction<OpenInsertCurvePointPopup>(
								track.mID,
								segment.mID,
								i, x_in_segment);
						}
						selected_curve = i;
					}
				}

				if (selected_curve == -1)
				{
					mState.mAction = createAction<None>();
				}
				else
				{
					showValue<T>(
						track,
						segment, x_in_segment,
						mState.mMouseCursorTime, selected_curve);
				}
			}
			else
			{
				if (mState.mAction->isAction<HoveringCurve>())
				{
					auto* action = mState.mAction->getDerived<HoveringCurve>();

					if (action->mSegmentID == segment.mID)
					{
						mState.mAction = createAction<None>();
					}
				}
			}
		}

		if (needs_drawing)
		{
			for (int i = 0; i < segment.mCurves.size(); i++)
			{
				if (mCurveCache[segment.mID][i].size() > 0)
				{
					// draw points of curve
					drawList->AddPolyline(
						&*mCurveCache[segment.mID][i].begin(), // points array
						mCurveCache[segment.mID][i].size(),	 // size of points array
						guicolors::curvecolors[i],  // color
						false, // closed
						selected_curve == i ? 3.0f : 1.0f); // thickness
				}
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
				SequenceCurveEnums::SegmentValueTypes::BEGIN,
				drawList);
		}

		// draw segment value handler
		drawSegmentValue<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
							SequenceCurveEnums::SegmentValueTypes::END,
			drawList);
	}


	template<typename T>
	void SequenceCurveTrackView::drawInspectorRange(const SequenceTrack& track)
	{
		const SequenceTrackCurve<T>& curve_track = static_cast<const SequenceTrackCurve<T>&>(track);

		T min = curve_track.mMinimum;
		T max = curve_track.mMaximum;

		//
		ImGui::PushID(track.mID.c_str());

		float drag_float_x = ImGui::GetCursorPosX() + 40;
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() + 5, ImGui::GetCursorPosY() + 5 });
		ImGui::Text("Min:"); ImGui::SameLine();
		ImGui::PushID("min");
		ImGui::SetCursorPosX(drag_float_x);
		if (inputFloat<T>(min, 3))
		{
			auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
			curve_controller.changeMinMaxCurveTrack<T>(track.mID, min, max);
			mState.mDirty = true;
		}
		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::Text("Max:"); ImGui::SameLine();
		ImGui::PushID("max");
		ImGui::SetCursorPosX(drag_float_x);
		if (inputFloat<T>(max, 3))
		{
			auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
			curve_controller.changeMinMaxCurveTrack<T>(track.mID, min, max);
			mState.mDirty = true;
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
		const SequenceTrackCurve<float>& curve_track = static_cast<const SequenceTrackCurve<float>&>(track);

		ImGui::BeginTooltip();

		ImGui::Text(formatTimeString(time).c_str());
		ImGui::Text("%.3f", segment.getValue(x) * (curve_track.mMaximum - curve_track.mMinimum) + curve_track.mMinimum);

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

		const SequenceTrackCurve<glm::vec2>& curve_track = static_cast<const SequenceTrackCurve<glm::vec2>&>(track);

		ImGui::BeginTooltip();

		glm::vec2 value = segment.getValue(x) * (curve_track.mMaximum - curve_track.mMinimum) + curve_track.mMinimum;

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

		const SequenceTrackCurve<glm::vec3>& curve_track = static_cast<const SequenceTrackCurve<glm::vec3>&>(track);

		ImGui::BeginTooltip();

		glm::vec3 value = segment.getValue(x) * (curve_track.mMaximum - curve_track.mMinimum) + curve_track.mMinimum;

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

		const SequenceTrackCurve<glm::vec4>& curve_track = static_cast<const SequenceTrackCurve<glm::vec4>&>(track);

		ImGui::BeginTooltip();

		glm::vec4 value = segment.getValue(x) * (curve_track.mMaximum - curve_track.mMinimum) + curve_track.mMinimum;

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
}
