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

	std::vector<rttr::type>& SequenceEventTrackView::getEventTypesMap()
	{
		static std::vector<rttr::type> eventTypes
			{
				RTTI_OF(SequenceTrackSegmentEventString),
				RTTI_OF(SequenceTrackSegmentEventFloat),
				RTTI_OF(SequenceTrackSegmentEventInt),
				RTTI_OF(SequenceTrackSegmentEventVec2),
				RTTI_OF(SequenceTrackSegmentEventVec3)
			};
		return eventTypes;
	}


	std::unordered_map<rttr::type, void(*)(SequenceControllerEvent&, std::string&, double)>& SequenceEventTrackView::getInsertSegmentMap()
	{
		static std::unordered_map<rttr::type, void(*)(SequenceControllerEvent&, std::string&, double)> insertSegmentMap
			{
				{ RTTI_OF(SequenceTrackSegmentEventString), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<std::string>(trackID, time); }},
				{ RTTI_OF(SequenceTrackSegmentEventFloat), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<float>(trackID, time); }},
				{ RTTI_OF(SequenceTrackSegmentEventInt), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<int>(trackID, time); }},
				{ RTTI_OF(SequenceTrackSegmentEventVec2), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<glm::vec2>(trackID, time); }},
				{ RTTI_OF(SequenceTrackSegmentEventVec3), [](SequenceControllerEvent& controller, std::string& trackID, double time){ controller.insertEventSegment<glm::vec3>(trackID, time); }}
			};
		return insertSegmentMap;
	}

	std::unordered_map<rttr::type, bool (SequenceEventTrackView::*)()>& SequenceEventTrackView::getHandlePopupMap()
	{
		static std::unordered_map<rttr::type, bool(SequenceEventTrackView::*)()> handlePopupsMap
			{
				{ RTTI_OF(SequenceTrackSegmentEventString), &SequenceEventTrackView::handleEditEventSegmentPopup<std::string> },
				{ RTTI_OF(SequenceTrackSegmentEventFloat), 	&SequenceEventTrackView::handleEditEventSegmentPopup<float> },
				{ RTTI_OF(SequenceTrackSegmentEventInt), 	&SequenceEventTrackView::handleEditEventSegmentPopup<int> },
				{ RTTI_OF(SequenceTrackSegmentEventVec2), 	&SequenceEventTrackView::handleEditEventSegmentPopup<glm::vec2> },
				{ RTTI_OF(SequenceTrackSegmentEventVec3), 	&SequenceEventTrackView::handleEditEventSegmentPopup<glm::vec3> }
			};
		return handlePopupsMap;
	}

	std::unordered_map<rttr::type, void(SequenceEventTrackView::*)(const SequenceTrackSegmentEventBase*, const std::string&, const std::string&)>& SequenceEventTrackView::getEditActionMap()
	{
		static std::unordered_map<rttr::type, void(SequenceEventTrackView::*)(const SequenceTrackSegmentEventBase*, const std::string&, const std::string&)> actionMap
			{
				{ RTTI_OF(SequenceTrackSegmentEventString), &SequenceEventTrackView::createEditAction<std::string> },
				{ RTTI_OF(SequenceTrackSegmentEventFloat), &SequenceEventTrackView::createEditAction<float> },
				{ RTTI_OF(SequenceTrackSegmentEventInt), &SequenceEventTrackView::createEditAction<int> },
				{ RTTI_OF(SequenceTrackSegmentEventVec2), &SequenceEventTrackView::createEditAction<glm::vec2> },
				{ RTTI_OF(SequenceTrackSegmentEventVec3), &SequenceEventTrackView::createEditAction<glm::vec3> }
			};

		return actionMap;
	}

	std::unordered_map<rttr::type, void(*)(const SequenceTrackSegment& segment, ImDrawList*, const ImVec2&, const float)>& SequenceEventTrackView::getDrawEventsMap()
	{
		static std::unordered_map<rttr::type, void(*)(const SequenceTrackSegment& segment, ImDrawList*, const ImVec2&, const float)>drawMap
		{
			{
				RTTI_OF(SequenceTrackSegmentEventFloat), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
				{
				  const auto& segment_event = static_cast<const SequenceTrackSegmentEventFloat&>(segment);

				  std::ostringstream string_stream;
				  string_stream << segment_event.mValue;

				  drawList->AddText(
					  { topLeft.x + x + 5, topLeft.y + 5 },
					  guicolors::red,
					  string_stream.str().c_str());
				}
			},
			{
				RTTI_OF(SequenceTrackSegmentEventInt), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
				{
			  		const auto& segment_event = static_cast<const SequenceTrackSegmentEventInt&>(segment);

			  		std::ostringstream string_stream;
			  		string_stream << segment_event.mValue;

			  		drawList->AddText(
				  	{ topLeft.x + x + 5, topLeft.y + 5 },
				  		guicolors::red,
				  		string_stream.str().c_str());
				}
			},
			{
				RTTI_OF(SequenceTrackSegmentEventString), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
				{
			  		const auto& segment_event = static_cast<const SequenceTrackSegmentEventString&>(segment);

			 	 	std::ostringstream string_stream;
			  		string_stream << "\"" << segment_event.mValue << "\"" ;

			  		drawList->AddText(
				  		{ topLeft.x + x + 5, topLeft.y + 5 },
				  		guicolors::red, string_stream.str().c_str());
				}
			},
			{
				RTTI_OF(SequenceTrackSegmentEventVec2), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
				{
			  		const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec2&>(segment);

			  		std::ostringstream string_stream;
			  		string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ")";

			  		drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },guicolors::red,string_stream.str().c_str());
				}
			},
			{
				RTTI_OF(SequenceTrackSegmentEventVec3), [](const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
				{
			  		const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec3&>(segment);

			  		std::ostringstream string_stream;
			  		string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ", " << segment_event.mValue.z << ")";

			  		drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },guicolors::red,
										string_stream.str().c_str());
				}
			}
		};

		return drawMap;
	}


	std::unordered_map<rttr::type, void(*)(SequenceGUIActions::Action&)>& SequenceEventTrackView::getHandlePopupContentMap()
	{
		static std::unordered_map<rttr::type, void(*)(SequenceGUIActions::Action&)> handlePopupsMap
		{
			{
				RTTI_OF(std::string), [](SequenceGUIActions::Action& action)
				{
				  auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<std::string>>();
				  std::string& message = static_cast<std::string&>(edit_action->mValue);

				  char buffer[256];
				  strcpy(buffer, message.c_str());

				  if (ImGui::InputText("message", buffer, 256))
				  {
					  message = std::string(buffer);
				  }
				}
			},
			{
				RTTI_OF(int), [](SequenceGUIActions::Action& action)
				{
					auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<int>>();
					int& value = static_cast<int&>(edit_action->mValue);

					ImGui::InputInt("Value", &value);
				}
			},
			{
				RTTI_OF(float), [](SequenceGUIActions::Action& action)
				{
					auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<float>>();
					float& value = static_cast<float&>(editAction->mValue);

					ImGui::InputFloat("Value", &value);
				}
			},
			{
				RTTI_OF(glm::vec2), [](SequenceGUIActions::Action& action)
				{
					auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec2>>();
					glm::vec2& value = static_cast<glm::vec2&>(edit_action->mValue);

					ImGui::InputFloat2("Value", &value.x);
				}
			},
			{
				RTTI_OF(glm::vec3), [](SequenceGUIActions::Action& action)
				{
					auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec3>>();
					glm::vec3& value = static_cast<glm::vec3&>(edit_action->mValue);

					ImGui::InputFloat3("Value", &value.x);
				}
			}
		};

		return handlePopupsMap;
	}


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


			//
			auto type = segment.get()->get_type();
			auto& draw_map = getDrawEventsMap();
			auto it = draw_map.find(type);
			assert(it != draw_map.end()); // type not found
			if( it != draw_map.end())
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

		for(auto& type : getEventTypesMap())
		{
			auto& handle_popup_map = getHandlePopupMap();
			auto it = handle_popup_map.find(type);
			assert(it != handle_popup_map.end()); // type not found
			if( it != handle_popup_map.end())
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

				for(auto& type : getEventTypesMap())
				{
					std::string buttonString = "Insert " + type.get_name().to_string();
					if( ImGui::Button(buttonString.c_str()))
					{
						auto& insert_segment_map = getInsertSegmentMap();
						auto it = insert_segment_map.find(type);
						assert(it!=insert_segment_map.end()); // type not found
						if( it != insert_segment_map.end() )
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

								auto& action_map = getEditActionMap();
								auto it = action_map.find(type);
								assert(it!=action_map.end()); // type not found
								if(it!=action_map.end())
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
