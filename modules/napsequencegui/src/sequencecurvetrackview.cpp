/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"
#include "sequenceplayercurveoutput.h"

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

	SequenceCurveTrackView::SequenceCurveTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state)
		: SequenceTrackView(view, state)
	{
		registerActionHandler(RTTI_OF(OpenInsertSegmentPopup) , std::bind(&SequenceCurveTrackView::handleInsertSegmentPopup, this));
		registerActionHandler(RTTI_OF(InsertingSegmentPopup) , std::bind(&SequenceCurveTrackView::handleInsertSegmentPopup, this));
		registerActionHandler(RTTI_OF(OpenInsertCurvePointPopup) , std::bind(&SequenceCurveTrackView::handleInsertCurvePointPopup, this));
		registerActionHandler(RTTI_OF(InsertingCurvePoint) , std::bind(&SequenceCurveTrackView::handleInsertCurvePointPopup, this));
		registerActionHandler(RTTI_OF(OpenCurveTypePopup) , std::bind(&SequenceCurveTrackView::handleCurveTypePopup, this));
		registerActionHandler(RTTI_OF(CurveTypePopup) , std::bind(&SequenceCurveTrackView::handleCurveTypePopup, this));
		registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<float>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<float>, this));
		registerActionHandler(RTTI_OF(CurvePointActionPopup<float>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<float>, this));
		registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec2>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec2>, this));
		registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec2>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec2>, this));
		registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec3>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec3>, this));
		registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec3>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec3>, this));
		registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec4>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec4>, this));
		registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec4>) , std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec4>, this));
		registerActionHandler(RTTI_OF(OpenEditSegmentCurveValuePopup<float>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<float>, this));
		registerActionHandler(RTTI_OF(EditingSegmentCurveValue<float>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<float>, this));
		registerActionHandler(RTTI_OF(OpenEditSegmentCurveValuePopup<glm::vec2>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec2>, this));
		registerActionHandler(RTTI_OF(EditingSegmentCurveValue<glm::vec2>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec2>, this));
		registerActionHandler(RTTI_OF(OpenEditSegmentCurveValuePopup<glm::vec3>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec3>, this));
		registerActionHandler(RTTI_OF(EditingSegmentCurveValue<glm::vec3>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec3>, this));
		registerActionHandler(RTTI_OF(OpenEditSegmentCurveValuePopup<glm::vec4>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec4>, this));
		registerActionHandler(RTTI_OF(EditingSegmentCurveValue<glm::vec4>), std::bind(&SequenceCurveTrackView::handleSegmentValueActionPopup<glm::vec4>, this));
		registerActionHandler(RTTI_OF(OpenEditTanPointPopup), std::bind(&SequenceCurveTrackView::handleTanPointActionPopup, this));
		registerActionHandler(RTTI_OF(EditingTanPointPopup), std::bind(&SequenceCurveTrackView::handleTanPointActionPopup, this));
		registerActionHandler(RTTI_OF(OpenEditCurveSegmentPopup), std::bind(&SequenceCurveTrackView::handleEditSegmentPopup, this));
		registerActionHandler(RTTI_OF(EditingCurveSegment), std::bind(&SequenceCurveTrackView::handleEditSegmentPopup, this));
		registerActionHandler(RTTI_OF(ChangeMinMaxCurve<float>), std::bind(&SequenceCurveTrackView::handleChangeMinMaxCurve<float>, this));
		registerActionHandler(RTTI_OF(ChangeMinMaxCurve<glm::vec2>), std::bind(&SequenceCurveTrackView::handleChangeMinMaxCurve<glm::vec2>, this));
		registerActionHandler(RTTI_OF(ChangeMinMaxCurve<glm::vec3>), std::bind(&SequenceCurveTrackView::handleChangeMinMaxCurve<glm::vec3>, this));
		registerActionHandler(RTTI_OF(ChangeMinMaxCurve<glm::vec4>), std::bind(&SequenceCurveTrackView::handleChangeMinMaxCurve<glm::vec4>, this));
		registerActionHandler(RTTI_OF(DraggingTanPoint), std::bind(&SequenceCurveTrackView::handleDragTanPoint, this));
		registerActionHandler(RTTI_OF(DraggingSegment), std::bind(&SequenceCurveTrackView::handleDragSegmentHandler, this));
		registerActionHandler(RTTI_OF(AssignOutputIDToTrack), std::bind(&SequenceCurveTrackView::handleAssignOutputIDToTrack, this));
		registerActionHandler(RTTI_OF(DraggingSegmentValue), std::bind(&SequenceCurveTrackView::handleDraggingSegmentValue, this));
		registerActionHandler(RTTI_OF(DraggingControlPoint), std::bind(&SequenceCurveTrackView::handleDraggingControlPoints, this));
	}


	std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr>& SequenceCurveTrackView::getDrawCurveSegmentsMap()
	{
		static std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr> map =
		{
			{ RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::drawSegmentContent<float> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec2), &SequenceCurveTrackView::drawSegmentContent<glm::vec2> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec3), &SequenceCurveTrackView::drawSegmentContent<glm::vec3> },
			{ RTTI_OF(SequenceTrackSegmentCurveVec4), &SequenceCurveTrackView::drawSegmentContent<glm::vec4> }
		};

		return map;
	};


	static std::unordered_map<rttr::type, std::vector<rttr::type>> parameter_types_for_curve_types
		{
			{ RTTI_OF(SequenceTrackCurveFloat), { { RTTI_OF(ParameterFloat), RTTI_OF(ParameterDouble), RTTI_OF(ParameterLong), RTTI_OF(ParameterInt) } } },
			{ RTTI_OF(SequenceTrackCurveVec2), { { RTTI_OF(ParameterVec2) } } },
			{ RTTI_OF(SequenceTrackCurveVec3), { { RTTI_OF(ParameterVec3) } } }
		};


	static bool isParameterTypeAllowed(const rttr::type& curveType, const rttr::type& parameterType)
	{
		auto it = parameter_types_for_curve_types.find(curveType);
		if(it!= parameter_types_for_curve_types.end())
		{
			for(auto& type : parameter_types_for_curve_types[curveType])
			{
				if(parameterType == type)
					return true;
			}
		}

		return false;
	}


	void SequenceCurveTrackView::handleActions()
	{
		/*
		 * if we started dragging a segment this frame, change start dragging segment action to dragging segment action
		 */
		if (mState.mAction->isAction<StartDraggingSegment>())
		{
			auto* action = mState.mAction->getDerived<StartDraggingSegment>();
			mState.mAction = createAction<DraggingSegment>(action->mTrackID, action->mSegmentID, action->mStartDuration);
		}

		SequenceTrackView::handleActions();
	}


	void SequenceCurveTrackView::showInspectorContent(const SequenceTrack &track)
	{
		// draw the assigned parameter
		ImGui::Text("Assigned Output");

		ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
		float offset = mState.mScale * 5.0f;
		inspector_cursor_pos.x += offset;
		inspector_cursor_pos.y += offset;
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

		ImGui::PushItemWidth(200.0f * mState.mScale);
		if (Combo(
			"",
			&current_item, curve_outputs))
		{
			if (current_item != 0)
				mState.mAction = SequenceGUIActions::createAction<AssignOutputIDToTrack>(track.mID, curve_outputs[current_item]);
			else
				mState.mAction = SequenceGUIActions::createAction<AssignOutputIDToTrack>(track.mID, "");
		}

		//
		ImGui::PopItemWidth();

		// map of inspectors ranges for curve types
		static std::unordered_map<rttr::type, void(SequenceCurveTrackView::*)(const SequenceTrack&)> inspectors
		{
			{ RTTI_OF(SequenceTrackCurveFloat) , &SequenceCurveTrackView::drawInspectorRange<float> },
			{ RTTI_OF(SequenceTrackCurveVec2) , &SequenceCurveTrackView::drawInspectorRange<glm::vec2> },
			{ RTTI_OF(SequenceTrackCurveVec3) , &SequenceCurveTrackView::drawInspectorRange<glm::vec3> },
			{ RTTI_OF(SequenceTrackCurveVec4) , &SequenceCurveTrackView::drawInspectorRange<glm::vec4> }
		};

		// draw inspector
		auto it = inspectors.find(track.get_type());
		assert(it!= inspectors.end()); // type not found
		if(it != inspectors.end())
		{
			(*this.*it->second)(track);
		}

		// delete track button
		ImGui::Spacing();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
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
						sequencer::colors::lightGrey, // color
						1.0f * mState.mScale); // thickness

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
						sequencer::colors::lightGrey, // color
						1.0f * mState.mScale); // thickness
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<InsertingSegmentPopup>())
			{
				auto* action = mState.mAction->getDerived<InsertingSegmentPopup>();

				if (action->mTrackID == track.mID)
				{
					// position of insertion in track
					draw_list->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState.mStepSize, trackTopLeft.y + mState.mTrackHeight }, // bottom right
						sequencer::colors::lightGrey, // color
						1.0f * mState.mScale); // thickness
				}
			}
		}

		float previous_segment_x = 0.0f;

		int segment_count = 0;
		for (const auto& segment : track.mSegments)
		{
			const auto* segment_ptr = segment.get();
			float segment_x	   = (float)(segment->mStartTime + segment->mDuration) * mState.mStepSize;
			float segment_width = (float)segment->mDuration * mState.mStepSize;

			// draw segment handlers
			drawSegmentHandler(
				track,
				*segment_ptr,
				trackTopLeft, segment_x, segment_width, draw_list);

			// draw segment content
			auto it = getDrawCurveSegmentsMap().find(segment_ptr->get_type());
			if (it != getDrawCurveSegmentsMap().end())
			{
				(*this.*it->second)(track, *segment_ptr, trackTopLeft, previous_segment_x, segment_width, segment_x,
									draw_list, (segment_count == 0));
			}

			//
			previous_segment_x = segment_x;

			//
			segment_count++;
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
		float seg_bounds = 10.0f * mState.mScale;
		if (mState.mIsWindowFocused &&
			((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringSegment>()) ||
			 (mState.mAction->isAction<StartDraggingSegment>() && mState.mAction->getDerived<StartDraggingSegment>()->mSegmentID != segment.mID))&&
			ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - seg_bounds, trackTopLeft.y - seg_bounds }, // top left
					{ trackTopLeft.x + segmentX + seg_bounds, trackTopLeft.y + mState.mTrackHeight + seg_bounds }))  // bottom right 
		{
			
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				sequencer::colors::white, // color
				3.0f * mState.mScale); // thickness

			// we are hovering this segment with the mouse
			mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime+segment.mDuration).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mState.mAction = createAction<StartDraggingSegment>(track.mID, segment.mID, segment.mDuration);
			}
			// right mouse is edit popup
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

			// if mouse is clicked, check if shift is down, if so, handle clipboard actions ( add or remove )
			if( ImGui::IsMouseClicked(0))
			{
				if( ImGui::GetIO().KeyShift )
				{
					// create new curve segment clipboard if necessary or when clipboard is from different sequence
					if( !mState.mClipboard->isClipboard<CurveSegmentClipboard>())
						mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename() );

					// is this a different track then previous clipboard ? then create a new clipboard, discarding the old clipboard
					auto* clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
					if( clipboard->getTrackID() != track.mID )
					{
						mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename());
						clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
					}

					// is clipboard from another sequence, create new clipboard
					if( clipboard->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename() )
					{
						mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename());
						clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
					}

					// see if the clipboard already contains this segment, if so, remove it, if not, add it
					if( mState.mClipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()) )
					{
						mState.mClipboard->removeObject(segment.mID);
					}else
					{
						utility::ErrorState errorState;
						mState.mClipboard->addObject(&segment, getPlayer().getSequenceFilename(), errorState);

						// log any errors
						if(errorState.hasErrors())
						{
							Logger::error(errorState.toString());
						}
					}
				}
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
						sequencer::colors::white, // color
						3.0f * mState.mScale); // thickness

				ImGui::BeginTooltip();
				ImGui::Text(formatTimeString(segment.mStartTime+segment.mDuration).c_str());
				ImGui::EndTooltip();
			}
			else
			{
				// draw handler of segment duration
				drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
					sequencer::colors::white, // color
					1.0f * mState.mScale); // thickness
			}
		}
		else
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				sequencer::colors::white, // color
				1.0f * mState.mScale); // thickness

			// release if we are not hovering this segment
			if (mState.mAction->isAction<HoveringSegment>()
				&& mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID)
			{
				mState.mAction = createAction<None>();
			}
		}
	}


	void SequenceCurveTrackView::handleInsertSegmentPopup()
	{
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

				mState.mAction = createAction<InsertingSegmentPopup>(action->mTrackID, action->mTime, action->mTrackType);
			}
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingSegmentPopup>())
		{
			auto* action = mState.mAction->getDerived<InsertingSegmentPopup>();

			if (action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ||
				action->mTrackType == RTTI_OF(SequenceTrackCurveVec4))
			{
				if (ImGui::BeginPopup("Insert Segment"))
				{
					if (ImGui::Button("Insert"))
					{
						auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
						curve_controller.insertSegment(action->mTrackID, action->mTime);
						updateSegmentsInClipboard(action->mTrackID);
						mState.mAction = createAction<None>();

						mCurveCache.clear();

						ImGui::CloseCurrentPopup();
					}

					// handle paste
					if( mState.mClipboard->isClipboard<CurveSegmentClipboard>())
					{
						if( ImGui::Button("Paste") )
						{
							// call the correct pasteClipboardSegments method for this track-type
							if( action->mTrackType == RTTI_OF(SequenceTrackCurveFloat) )
							{
								pasteClipboardSegments<SequenceTrackSegmentCurveFloat>(action->mTrackID, action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec2) ){
								pasteClipboardSegments<SequenceTrackSegmentCurveVec2>(action->mTrackID, action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec3) ){
								pasteClipboardSegments<SequenceTrackSegmentCurveVec3>(action->mTrackID, action->mTime);
							}else if( action->mTrackType == RTTI_OF(SequenceTrackCurveVec4) ){
								pasteClipboardSegments<SequenceTrackSegmentCurveVec4>(action->mTrackID, action->mTime);
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
	}


	void SequenceCurveTrackView::handleCurveTypePopup()
	{
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
				ImGui::SetWindowPos(action->mWindowPos);

				if (ImGui::Button("Linear"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Linear, action->mCurveIndex);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();

				}

				if (ImGui::Button("Bezier"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Bezier, action->mCurveIndex);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
					mCurveCache.clear();
				}

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


	void SequenceCurveTrackView::handleInsertCurvePointPopup()
	{
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
				auto* action = mState.mAction->getDerived<InsertingCurvePoint>();
				if (ImGui::Button("Insert Point"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.insertCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mPos,
						action->mSelectedIndex);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

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
	}


	void SequenceCurveTrackView::handleTanPointActionPopup()
	{
		if (mState.mAction->isAction<SequenceGUIActions::OpenEditTanPointPopup>())
		{
			auto* action = mState.mAction->getDerived<SequenceGUIActions::OpenEditTanPointPopup>();
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::EditingTanPointPopup>(
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
				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingTanPointPopup>();
				int curveIndex = action->mCurveIndex;

				bool change = false;
				if (ImGui::InputFloat("time", &action->mTime))
				{
					change = true;

					// time cannot be 0
					action->mTime = math::max<float>(0.1f, action->mTime);
				}

				if (ImGui::InputFloat("value", &action->mValue))
				{
					change = true;
				}

				if(change)
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();

					bool tangents_flipped = curve_controller.changeTanPoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex,
						action->mType,
						action->mTime,
						action->mValue);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

					if( tangents_flipped && action->mType == SequenceCurveEnums::ETanPointTypes::IN )
					{
						action->mType = SequenceCurveEnums::ETanPointTypes::OUT;
					}else if( tangents_flipped && action->mType == SequenceCurveEnums::ETanPointTypes::OUT )
					{
						action->mType = SequenceCurveEnums::ETanPointTypes::IN;
					}

					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					mState.mAction = createAction<SequenceGUIActions::None>();

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
	}


	void SequenceCurveTrackView::handleEditSegmentPopup()
	{
		if (mState.mAction->isAction<OpenEditCurveSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Segment");

			auto* action = mState.mAction->getDerived<OpenEditCurveSegmentPopup>();

			mState.mAction = createAction<EditingCurveSegment>(
				action->mTrackID,
				action->mSegmentID,
				action->mSegmentType,
				action->mStartTime,
				action->mDuration
			);
		}

		// handle edit segment popup
		if (mState.mAction->isAction<EditingCurveSegment>())
		{
			auto* action = mState.mAction->getDerived<EditingCurveSegment>();
			auto& controller = getEditor().getController<SequenceControllerCurve>();
			auto* track = controller.getTrack(action->mTrackID);

			// TODO: remove elif statement here
			if (track->get_type() == RTTI_OF(SequenceTrackCurve<float>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec2>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec3>) ||
				track->get_type() == RTTI_OF(SequenceTrackCurve<glm::vec4>) )
			{
				if(ImGui::BeginPopup("Edit Segment"))
				{
					// see if we have a curve segment in the clipboard
					bool display_copy = !mState.mClipboard->isClipboard<CurveSegmentClipboard>();
					if( !display_copy )
					{
						// yes ? see if the track is the same
						auto* clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

						// if no, display copy, which will override the existing clipboard
						if( clipboard->getTrackID() != track->mID )
						{
							display_copy = true;
						}

						// check if clipboard is from different sequence
						if( clipboard->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename() )
						{
							display_copy = true;
						}
					}

					if( display_copy )
					{
						if( ImGui::Button("Copy") )
						{
							// create new clipboard
							mState.mClipboard = createClipboard<CurveSegmentClipboard>(track->get_type(), track->mID, getEditor().mSequencePlayer->getSequenceFilename());

							// get curve segment
							const auto* curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);

							// add object to clipboard
							utility::ErrorState errorState;
							mState.mClipboard->addObject(curve_segment, getPlayer().getSequenceFilename(), errorState);

							// log any errors
							if(errorState.hasErrors())
							{
								nap::Logger::error(errorState.toString());

								// remove faulty/empty clipboard
								mState.mClipboard = createClipboard<Empty>();
							}

							// exit popup
							ImGui::CloseCurrentPopup();
							mState.mAction = createAction<None>();
						}
					}

					if( !display_copy )
					{
						// get clipboard
						auto* clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

						// get curve segment
						const auto* curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);

						// does the clipboard already contain this segment ?
						if( clipboard->containsObject(curve_segment->mID, getPlayer().getSequenceFilename()) )
						{
							if( ImGui::Button("Remove from clipboard") )
							{
								// remove the object from the clipboard
								clipboard->removeObject(curve_segment->mID);

								// if clipboard is empty, create empty clipboard
								if(clipboard->getObjectCount() == 0 )
								{
									mState.mClipboard = createClipboard<Empty>();
								}

								// exit popup
								ImGui::CloseCurrentPopup();
								mState.mAction = createAction<None>();
							}
						}else
						{
							if( ImGui::Button("Add to clipboard") )
							{
								// add object to clipboard
								utility::ErrorState errorState;
								clipboard->addObject(curve_segment, getPlayer().getSequenceFilename(), errorState);

								// log any errors
								if(errorState.hasErrors())
								{
									nap::Logger::error(errorState.toString());

									// remove faulty/empty clipboard
									mState.mClipboard = createClipboard<Empty>();
								}

								// exit popup
								ImGui::CloseCurrentPopup();
								mState.mAction = createAction<None>();
							}
						}
					}

					// see if we can replace the contents of this segment with the one from the clipboard
					// this can only happen when we have 1 segment in the clipboard
					bool display_replace = mState.mClipboard->isClipboard<CurveSegmentClipboard>();
					if( display_replace )
					{
						display_replace = mState.mClipboard->getObjectCount() == 1;
					}

					// display the replace option
					if( display_replace )
					{
						if( ImGui::Button("Paste Into This") )
						{
							// call the correct pasteClipboardSegments method for this segment-type
							if( action->mSegmentType == RTTI_OF(SequenceTrackSegmentCurveFloat) )
							{
								pasteClipboardSegmentInto<SequenceTrackSegmentCurveFloat>(action->mTrackID, action->mSegmentID);
							}else if(  action->mSegmentType == RTTI_OF(SequenceTrackSegmentCurveVec2) ){
								pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec2>(action->mTrackID, action->mSegmentID);
							}else if(  action->mSegmentType == RTTI_OF(SequenceTrackSegmentCurveVec3) ){
								pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec3>(action->mTrackID, action->mSegmentID);
							}else if(  action->mSegmentType == RTTI_OF(SequenceTrackSegmentCurveVec4) ){
								pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec4>(action->mTrackID, action->mSegmentID);
							}

							// redraw cached curves
							mState.mDirty = true;

							// exit popup
							ImGui::CloseCurrentPopup();
							mState.mAction = createAction<None>();
						}
					}

					if (ImGui::Button("Delete"))
					{
						controller.deleteSegment(
							action->mTrackID,
							action->mSegmentID);
						mCurveCache.clear();

						// remove delete object from clipboard
						if( mState.mClipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename()))
						{
							mState.mClipboard->removeObject(action->mSegmentID);
						}

						updateSegmentsInClipboard(action->mTrackID);

						// exit popup
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
						double new_duration = controller.segmentDurationChange(action->mTrackID, action->mSegmentID, (float)(new_time - action->mStartTime));

						// make the controller re-align start & end points of segments
						controller.updateCurveSegments(action->mTrackID);

						updateSegmentsInClipboard(action->mTrackID);

						action->mDuration = new_duration;
						mState.mDirty = true;
						mCurveCache.clear();
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
	}


	template<>
	void SequenceCurveTrackView::showValue<float>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<float>& segment,
		float x,
		double time,
		int curveIndex)
	{
		assert(track.get_type().is_derived_from<SequenceTrackCurve<float>>());
		const auto& curve_track = static_cast<const SequenceTrackCurve<float>&>(track);

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

		assert(track.get_type().is_derived_from<SequenceTrackCurve<glm::vec2>>());
		const auto& curve_track = static_cast<const SequenceTrackCurve<glm::vec2>&>(track);

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

		assert(track.get_type().is_derived_from<SequenceTrackCurve<glm::vec3>>());
		const auto& curve_track = static_cast<const SequenceTrackCurve<glm::vec3>&>(track);

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

		assert(track.get_type().is_derived_from<SequenceTrackCurve<glm::vec4>>());
		const auto& curve_track = static_cast<const SequenceTrackCurve<glm::vec4>&>(track);

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


	template<>
	bool SequenceCurveTrackView::inputFloat<float>(float &v, int precision)
	{
		ImGui::PushItemWidth(100.0f * mState.mScale);
		return ImGui::InputFloat("", &v, 0.0f, 0.0f, precision);
	}


	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2 &v, int precision)
	{
		ImGui::PushItemWidth(145.0f * mState.mScale);
		return ImGui::InputFloat2("", &v[0], precision);
	}


	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3 &v, int precision)
	{
		ImGui::PushItemWidth(180.0f * mState.mScale);
		return ImGui::InputFloat3("", &v[0], precision);
	}


	template<>
	bool SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4 &v, int precision)
	{
		ImGui::PushItemWidth(225.0f * mState.mScale);
		return ImGui::InputFloat4("", &v[0], precision);
	}


	template<>
	void SequenceCurveTrackView::handleCurvePointActionPopup<float>()
	{
		if (mState.mAction->isAction<SequenceGUIActions::OpenCurvePointActionPopup<float>>())
		{
			auto* action = mState.mAction->getDerived<SequenceGUIActions::OpenCurvePointActionPopup<float>>();
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::CurvePointActionPopup<float>>(
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

		if (mState.mAction->isAction<SequenceGUIActions::CurvePointActionPopup<float>>())
		{
			if (ImGui::BeginPopup("Curve Point Actions"))
			{
				auto* action = mState.mAction->getDerived<SequenceGUIActions::CurvePointActionPopup<float>>();

				if (ImGui::Button("Delete"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.deleteCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex);
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
					mCurveCache.clear();

					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();

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
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
			}
		}
	}



	template<>
	void SequenceCurveTrackView::handleSegmentValueActionPopup<float>()
	{
		if (mState.mAction->isAction<SequenceGUIActions::OpenEditSegmentCurveValuePopup<float>>())
		{
			auto* action = mState.mAction->getDerived<SequenceGUIActions::OpenEditSegmentCurveValuePopup<float>>();
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::EditingSegmentCurveValue<float>>(
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

		if (mState.mAction->isAction<SequenceGUIActions::EditingSegmentCurveValue<float>>())
		{
			if (ImGui::BeginPopup("Segment Value Actions"))
			{
				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingSegmentCurveValue<float>>();
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
					updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
					mState.mDirty = true;
				}

				if (ImGui::Button("Done"))
				{
					mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
			}
		}
	}


	void SequenceCurveTrackView::updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID)
	{
		if( mState.mClipboard->isClipboard<CurveSegmentClipboard>())
		{
			auto* curve_segment_clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

			if( mState.mClipboard->containsObject(segmentID, getPlayer().getSequenceFilename()))
			{
				mState.mClipboard->removeObject(segmentID);

				auto& controller = getEditor().getController<SequenceControllerCurve>();
				const auto* segment = controller.getSegment(trackID, segmentID);

				utility::ErrorState errorState;
				mState.mClipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);
			}
		}

	}


	void SequenceCurveTrackView::updateSegmentsInClipboard(const std::string& trackID)
	{
		if( mState.mClipboard->isClipboard<CurveSegmentClipboard>() )
		{
			auto* clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
			if(clipboard->getTrackID() == trackID )
			{
				auto segment_ids = clipboard->getObjectIDs();
				for(const auto& segment_id : segment_ids)
				{
					updateSegmentInClipboard(trackID, segment_id);
				}
			}
		}
	}


	void SequenceCurveTrackView::handleDragTanPoint()
	{
		//
		auto* action = mState.mAction->getDerived<SequenceGUIActions::DraggingTanPoint>();

		// get editor
		auto& editor = getEditor();

		// get controller for this track type
		auto* controller = editor.getControllerWithTrackID(action->mTrackID);
		assert(controller!= nullptr);

		// assume we can upcast it to a curve controller
		auto* curve_controller = rtti_cast<SequenceControllerCurve>(controller);
		assert(curve_controller != nullptr);

		bool tangents_flipped = curve_controller->changeTanPoint(
			action->mTrackID,
			action->mSegmentID,
			action->mControlPointIndex,
			action->mCurveIndex,
			action->mType,
			action->mNewTime,
			action->mNewValue);
		updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

		if( tangents_flipped && action->mType == SequenceCurveEnums::ETanPointTypes::IN )
		{
			action->mType = SequenceCurveEnums::ETanPointTypes::OUT;
		}else if( tangents_flipped && action->mType == SequenceCurveEnums::ETanPointTypes::OUT )
		{
			action->mType = SequenceCurveEnums::ETanPointTypes::IN;
		}

		//
		mState.mDirty = true;

		// discard action if mouse is released
		if (ImGui::IsMouseReleased(0))
		{
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
		}
	}


	void SequenceCurveTrackView::handleDragSegmentHandler()
	{
		auto* action = mState.mAction->getDerived<DraggingSegment>();
		assert(action!=nullptr);

		// do we have the mouse still held down ? drag the segment
		if (ImGui::IsMouseDown(0))
		{
			float amount = mState.mMouseDelta.x / mState.mStepSize;

			// get editor
			auto& editor = getEditor();

			// get controller for this track type
			auto* controller = editor.getControllerWithTrackID(action->mTrackID);
			assert(controller!= nullptr);

			// assume we can upcast it to a curve controller
			auto* curve_controler = static_cast<SequenceControllerCurve*>(controller);

			// change duration
			action->mNewDuration += amount;
			curve_controler->segmentDurationChange(action->mTrackID, action->mSegmentID, action->mNewDuration);
			updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

			// mark state as dirty
			mState.mDirty = true;
		}
			// otherwise... release!
		else if (ImGui::IsMouseReleased(0))
		{
			mState.mAction = createAction<None>();
		}
	}


	void SequenceCurveTrackView::handleAssignOutputIDToTrack()
	{
		// get action
		auto* action = mState.mAction->getDerived<AssignOutputIDToTrack>();
		assert(action!=nullptr);

		// get curve controller
		auto& curve_controller = getEditor().getController<SequenceControllerCurve>();

		// call function to controller
		curve_controller.assignNewObjectID(action->mTrackID, action->mObjectID);

		// action is done
		mState.mAction = SequenceGUIActions::createAction<None>();

		// redraw curves and caches
		mState.mDirty = true;
	}


	void SequenceCurveTrackView::handleDraggingSegmentValue()
	{
		// get action
		auto* action = mState.mAction->getDerived<DraggingSegmentValue>();
		assert(action!=nullptr);

		// get controller for this track type
		auto* controller = getEditor().getControllerWithTrackID(action->mTrackID);
		assert(controller!= nullptr);

		// assume we can upcast it to a curve controller
		assert(controller->get_type().is_derived_from<SequenceControllerCurve>());
		auto* curve_controller = static_cast<SequenceControllerCurve*>(controller);

		// tell the controller to change the curve segment value
		curve_controller->changeCurveSegmentValue(
			action->mTrackID,
			action->mSegmentID,
			action->mNewValue,
			action->mCurveIndex,
			action->mType);

		// update this segment if its in the clipboard
		updateSegmentInClipboard(action->mTrackID,action->mSegmentID);

		// set dirty to clear curve cache
		mState.mDirty = true;

		//
		if( ImGui::IsMouseReleased(0) )
		{
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
		}
	}


	void SequenceCurveTrackView::handleDraggingControlPoints()
	{
		// get the action
		auto* action = mState.mAction->getDerived<SequenceGUIActions::DraggingControlPoint>();

		// get editor
		auto& editor = getEditor();

		// get controller for this track type
		auto* controller = editor.getControllerWithTrackID(action->mTrackID);
		assert(controller!= nullptr);

		// assume we can upcast it to a curve controller
		auto* curve_controler = rtti_cast<SequenceControllerCurve>(controller);
		assert(curve_controler!= nullptr);

		curve_controler->changeCurvePoint(
			action->mTrackID,
			action->mSegmentID,
			action->mControlPointIndex,
			action->mCurveIndex,
			action->mNewTime,
			action->mNewValue);
		updateSegmentsInClipboard(action->mTrackID);

		mState.mDirty = true;

		if (ImGui::IsMouseReleased(0))
		{
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
		}
	}
}

