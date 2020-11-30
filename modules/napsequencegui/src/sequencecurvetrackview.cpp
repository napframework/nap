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


	bool SequenceCurveTrackView::handleTanPointActionPopup()
	{
		bool handled = false;

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
				handled = true;

				auto* action = mState.mAction->getDerived<SequenceGUIActions::EditingTanPointPopup>();
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
						utility::ErrorState errorState;
						const auto* curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);
						mState.mClipboard = createClipboard<CurveSegmentClipboard>(action->mSegmentType);
						mState.mClipboard->serialize(curve_segment, errorState);

						if( errorState.hasErrors())
						{
							nap::Logger::error(errorState.toString());
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


	template<>
	bool SequenceCurveTrackView::handleCurvePointActionPopup<float>()
	{
		bool handled = false;

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
				handled = true;

				auto* action = mState.mAction->getDerived<SequenceGUIActions::CurvePointActionPopup<float>>();

				if (ImGui::Button("Delete"))
				{
					auto& curve_controller = getEditor().getController<SequenceControllerCurve>();
					curve_controller.deleteCurvePoint(
						action->mTrackID,
						action->mSegmentID,
						action->mControlPointIndex,
						action->mCurveIndex);
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
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
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

		return handled;
	}



	template<>
	bool SequenceCurveTrackView::handleSegmentValueActionPopup<float>()
	{
		bool handled = false;

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
				handled = true;

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
					mState.mDirty = true;
				}

				if (ImGui::Button("Cancel"))
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

		return handled;
	}
}
