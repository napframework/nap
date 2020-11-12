/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceeventtrackview.h"
#include "napcolors.h"
#include "sequencecontrollerevent.h"
#include "sequenceeditorgui.h"
#include "sequenceplayereventoutput.h"
#include "sequencetrackevent.h"

#include <nap/logger.h>
#include <iostream>

namespace nap
{
	using namespace SequenceGUIActions;

	static bool trackViewRegistration = SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceEventTrackView));

	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceEventTrackView), [](SequenceEditorGUIView& view, SequenceEditorGUIState& state)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceEventTrackView>(view, state);
	});

	static std::vector<rttr::type> sEventTypes
		{
			RTTI_OF(SequenceTrackSegmentEventString),
			RTTI_OF(SequenceTrackSegmentEventFloat),
			RTTI_OF(SequenceTrackSegmentEventInt),
			RTTI_OF(SequenceTrackSegmentEventVec2),
			RTTI_OF(SequenceTrackSegmentEventVec3)
		};

	static std::unordered_map<rttr::type, void(*)(SequenceControllerEvent&, std::string&, double)> sInsertSegmentMap
		{
			{ RTTI_OF(SequenceTrackSegmentEventString), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<std::string>(trackID, time); }},
			{ RTTI_OF(SequenceTrackSegmentEventFloat), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<float>(trackID, time); }},
			{ RTTI_OF(SequenceTrackSegmentEventInt), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<int>(trackID, time); }},
			{ RTTI_OF(SequenceTrackSegmentEventVec2), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<glm::vec2>(trackID, time); }},
			{ RTTI_OF(SequenceTrackSegmentEventVec3), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<glm::vec3>(trackID, time); }}
		};

	std::unordered_map<rttr::type, bool(SequenceEventTrackView::*)()> SequenceEventTrackView::sHandlePopupsMap
		{
			{ RTTI_OF(SequenceTrackSegmentEventString), &SequenceEventTrackView::handleEditEventSegmentPopup<std::string> },
			{ RTTI_OF(SequenceTrackSegmentEventFloat), &SequenceEventTrackView::handleEditEventSegmentPopup<float> },
			{ RTTI_OF(SequenceTrackSegmentEventInt), &SequenceEventTrackView::handleEditEventSegmentPopup<int> },
			{ RTTI_OF(SequenceTrackSegmentEventVec2), &SequenceEventTrackView::handleEditEventSegmentPopup<glm::vec2>},
			{ RTTI_OF(SequenceTrackSegmentEventVec3), &SequenceEventTrackView::handleEditEventSegmentPopup<glm::vec3> }
		};

	std::unordered_map<rttr::type, void(SequenceEventTrackView::*)(const SequenceTrackSegmentEventBase*, const std::string&, const std::string&)> SequenceEventTrackView::sEditActionMap
		{
			{ RTTI_OF(SequenceTrackSegmentEventString), &SequenceEventTrackView::createEditAction<std::string> },
			{ RTTI_OF(SequenceTrackSegmentEventFloat), &SequenceEventTrackView::createEditAction<float> },
			{ RTTI_OF(SequenceTrackSegmentEventInt), &SequenceEventTrackView::createEditAction<int> },
			{ RTTI_OF(SequenceTrackSegmentEventVec2), &SequenceEventTrackView::createEditAction<glm::vec2>},
			{ RTTI_OF(SequenceTrackSegmentEventVec3), &SequenceEventTrackView::createEditAction<glm::vec3> }
		};

	SequenceEventTrackView::SequenceEventTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state)
		: SequenceTrackView(view, state)
	{
	}


	void SequenceEventTrackView::showInspectorContent(const SequenceTrack& track)
	{
		// draw the assigned receiver
		ImGui::Text("Assigned Output");

		ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
		inspector_cursor_pos.x += 5;
		inspector_cursor_pos.y += 5;
		ImGui::SetCursorPos(inspector_cursor_pos);

		bool assigned = false;
		std::string assigned_id;
		std::vector<std::string> event_outputs;
		int current_item = 0;
		event_outputs.emplace_back("none");
		int count = 0;
		const SequencePlayerEventOutput* event_output = nullptr;

		for (const auto& output : getEditor().mSequencePlayer->mOutputs)
		{
			if (output.get()->get_type() == RTTI_OF(SequencePlayerEventOutput))
			{
				count++;

				if (output->mID == track.mAssignedOutputID)
				{
					assigned = true;
					assigned_id = output->mID;
					current_item = count;

					assert(output.get()->get_type() == RTTI_OF(SequencePlayerEventOutput)); // type mismatch
					event_output = static_cast<SequencePlayerEventOutput*>(output.get());
				}

				event_outputs.emplace_back(output->mID);
			}
		}

		ImGui::PushItemWidth(200.0f);
		if (Combo(
			"",
			&current_item, event_outputs))
		{
			auto& event_controller = getEditor().getController<SequenceControllerEvent>();

			if (current_item != 0)
				event_controller.assignNewObjectID(track.mID, event_outputs[current_item]);
			else
				event_controller.assignNewObjectID(track.mID, "");

		}
		ImGui::PopItemWidth();
	}


	void SequenceEventTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
	{
		ImDrawList * draw_list = ImGui::GetWindowDrawList();

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
						mState.mAction = createAction<OpenInsertEventSegmentPopup>(
							track.mID,
							time);
					}
				}
			}

			// draw line in track while in inserting segment popup
			if (mState.mAction->isAction<OpenInsertEventSegmentPopup>())
			{
				auto* action = mState.mAction->getDerived<OpenInsertEventSegmentPopup>();
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

			if ( mState.mAction->isAction<InsertingEventSegment>())
			{
				auto* action = mState.mAction->getDerived<InsertingEventSegment>();
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

		float prev_segment_x = 0.0f;

		int segment_count = 0;
		for (const auto& segment : track.mSegments)
		{
			float segment_x	   = (segment->mStartTime) * mState.mStepSize;

			// draw segment handlers
			drawSegmentHandler(
				track,
				*segment.get(),
				trackTopLeft, segment_x,
				0.0f, draw_list);

			// static map of draw functions for different event types
			static std::unordered_map<rttr::type, void(*)(const SequenceTrackSegment& segment, ImDrawList*, const ImVec2&, const float)>
				s_draw_map{
					{ RTTI_OF(SequenceTrackSegmentEventFloat), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x){
					  const auto& segment_event = static_cast<const SequenceTrackSegmentEventFloat&>(segment);

					  std::ostringstream string_stream;
					  string_stream << segment_event.mValue;

					  drawList->AddText(
						  { topLeft.x + x + 5, topLeft.y + 5 },
						  guicolors::red,
										string_stream.str().c_str());
					} },
					{ RTTI_OF(SequenceTrackSegmentEventInt), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x){
					  const auto& segment_event = static_cast<const SequenceTrackSegmentEventInt&>(segment);

					  std::ostringstream string_stream;
					  string_stream << segment_event.mValue;

					  drawList->AddText(
						  { topLeft.x + x + 5, topLeft.y + 5 },
						  guicolors::red,
										string_stream.str().c_str());
					} },
					{ RTTI_OF(SequenceTrackSegmentEventString), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x){

					  const auto& segment_event = static_cast<const SequenceTrackSegmentEventString&>(segment);

					  std::ostringstream string_stream;
					  string_stream << "\"" << segment_event.mValue << "\"" ;

					  drawList->AddText(
						  { topLeft.x + x + 5, topLeft.y + 5 },
						  guicolors::red, string_stream.str().c_str());
					} },
					{ RTTI_OF(SequenceTrackSegmentEventVec2), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x){
					  const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec2&>(segment);

					  std::ostringstream string_stream;
					  string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ")";

					  drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },guicolors::red,string_stream.str().c_str());
					} },
					{ RTTI_OF(SequenceTrackSegmentEventVec3), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x){
					  const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec3&>(segment);

					  std::ostringstream string_stream;
					  string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ", " << segment_event.mValue.z << ")";

					  drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },guicolors::red,
										string_stream.str().c_str());
					} }
				};

			//
			auto it = s_draw_map.find(segment.get()->get_type());
			assert(it != s_draw_map.end()); // type not found
			if( it != s_draw_map.end())
			{
				it->second(*segment.get(), draw_list, trackTopLeft, segment_x);
			}

			//
			prev_segment_x = segment_x;

			//
			segment_count++;
		}
	}


	bool SequenceEventTrackView::handlePopups()
	{
		if( handleInsertEventSegmentPopup() )
			return true;

		if( handleDeleteSegmentPopup() )
			return true;

		for(auto& type : sEventTypes)
		{
			auto it = sHandlePopupsMap.find(type);
			assert(it != sHandlePopupsMap.end()); // type not found
			if( it != sHandlePopupsMap.end())
			{
				if( (*this.*it->second)() )
				{
					return true;
				}
			}
		}

		return false;
	}


	bool SequenceEventTrackView::handleInsertEventSegmentPopup()
	{
		// popup handled
		bool handled = false;

		if (mState.mAction->isAction<OpenInsertEventSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Event");

			auto* action = mState.mAction->getDerived<OpenInsertEventSegmentPopup>();
			mState.mAction = createAction<InsertingEventSegment>(action->mTrackID, action->mTime);
		}

		// handle insert segment popup
		if (mState.mAction->isAction<InsertingEventSegment>())
		{
			if (ImGui::BeginPopup("Insert Event"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<InsertingEventSegment>();

				for(auto& type : sEventTypes)
				{
					std::string buttonString = "Insert " + type.get_name().to_string();
					if( ImGui::Button(buttonString.c_str()))
					{
						auto it = sInsertSegmentMap.find(type);
						assert(it!=sInsertSegmentMap.end()); // type not found
						if( it != sInsertSegmentMap.end() )
						{
							it->second(getEditor().getController<SequenceControllerEvent>(), action->mTrackID, action->mTime);
							ImGui::CloseCurrentPopup();
							mState.mAction = createAction<None>();
						}
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

		return handled;
	}


	void SequenceEventTrackView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{
		// segment handler
        if (((mState.mIsWindowFocused && ImGui::IsMouseHoveringRect(
            { trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 },
            { trackTopLeft.x + segmentX + 10, trackTopLeft.y + mState.mTrackHeight + 10 })) &&
             ( mState.mAction->isAction<None>() || ( mState.mAction->isAction<HoveringSegment>() && mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID)))
			||
			( mState.mAction->isAction<DraggingSegment>() &&  mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID))
		{
			bool isAlreadyHovering = false;
			if (mState.mAction->isAction<HoveringSegment>())
			{
				isAlreadyHovering = mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID;
			}

			bool isAlreadyDragging = false;
			if (mState.mAction->isAction<DraggingSegment>())
			{
				isAlreadyDragging = mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID;
			}

			// draw handler of segment duration
			drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			if (!isAlreadyHovering && !isAlreadyDragging)
			{
				if(!ImGui::IsMouseDragging(0))
				{
					// we are hovering this segment with the mouse
					mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);
				}

			}

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (!isAlreadyDragging)
			{
				if(  !mState.mAction->isAction<DraggingSegment>() )
				{
					if (ImGui::IsMouseDown(0))
					{
						mState.mAction = createAction<DraggingSegment>(track.mID, segment.mID);
					}
				}
			}
			else
			{
				if (ImGui::IsMouseDown(0))
				{
					float amount = mState.mMouseDelta.x / mState.mStepSize;

					auto& editor = getEditor();
					SequenceControllerEvent& eventController = editor.getController<SequenceControllerEvent>();
					eventController.segmentEventStartTimeChange(track.mID, segment.mID, segment.mStartTime + amount);
				}
			}

			// right mouse in deletion popup
			if (ImGui::IsMouseDown(1))
			{
				mState.mAction = createAction<OpenEditSegmentValuePopup>(track.mID, segment.mID, segment.get_type());
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

			if (mState.mAction->isAction<HoveringSegment>())
			{
				auto* action = mState.mAction->getDerived<HoveringSegment>();
				if (action->mSegmentID == segment.mID)
				{
					mState.mAction = createAction<None>();
				}
			}
		}
		
		if (ImGui::IsMouseReleased(0))
		{
			if (mState.mAction->isAction<DraggingSegment>())
			{
				if (mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID)
				{
					mState.mAction = createAction<None>();
				}
			}
		}
	}


	bool SequenceEventTrackView::handleDeleteSegmentPopup()
	{
		//
		bool handled = false;

		if (mState.mAction->isAction<OpenEditSegmentValuePopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Segment");

			auto* action = mState.mAction->getDerived<OpenEditSegmentValuePopup>();
			mState.mAction = createAction<EditingSegment>(action->mTrackID, action->mSegmentID, action->mSegmentType);
		}

		// handle delete segment popup
		if (mState.mAction->isAction<EditingSegment>())
		{
			if (ImGui::BeginPopup("Edit Segment"))
			{
				handled = true;

				auto* action = mState.mAction->getDerived<EditingSegment>();
				if (ImGui::Button("Delete"))
				{
					auto& controller = getEditor().getController<SequenceControllerEvent>();
					controller.deleteSegment(action->mTrackID, action->mSegmentID);
					mState.mDirty = true;

					ImGui::CloseCurrentPopup();
					mState.mAction = createAction<None>();
				}
				else
				{
					if (action->mSegmentType.is_derived_from<SequenceTrackSegmentEventBase>())
					{
						if (ImGui::Button("Edit"))
						{
							auto& eventController = getEditor().getController<SequenceControllerEvent>();
							const SequenceTrackSegmentEventBase *eventSegment = dynamic_cast<const SequenceTrackSegmentEventBase*>(eventController.getSegment(action->mTrackID, action->mSegmentID));
							assert(eventSegment != nullptr);

							if( eventSegment != nullptr)
							{
								rttr::type type = eventSegment->get_type();

								auto it = sEditActionMap.find(type);
								assert(it!=sEditActionMap.end()); // type not found
								if(it!=sEditActionMap.end())
								{
									(*this.*it->second)(eventSegment, action->mTrackID, action->mSegmentID);
								}
							}

							ImGui::CloseCurrentPopup();
						}
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

		return handled;
	}
}
