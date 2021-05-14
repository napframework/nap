/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceeventtrackview.h"
#include "sequencecontrollerevent.h"
#include "sequenceeditorgui.h"
#include "sequenceplayereventoutput.h"
#include "sequencetrackevent.h"
#include "sequenceguiservice.h"

#include <nap/logger.h>
#include <iostream>

namespace nap
{
	using namespace SequenceGUIActions;
	using namespace SequenceGUIClipboards;

	SequenceEventTrackView::SequenceEventTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state)
		: SequenceTrackView(view, state)
	{
		// register applicable action handlers
		registerActionHandler(RTTI_OF(OpenInsertEventSegmentPopup), [this] { handleInsertEventSegmentPopup(); });
		registerActionHandler(RTTI_OF(InsertingEventSegment), [this] { handleInsertEventSegmentPopup(); });
		registerActionHandler(RTTI_OF(OpenEditSegmentPopup), [this] { handleEditSegmentValuePopup(); });
		registerActionHandler(RTTI_OF(EditingSegmentPopup), [this] { handleEditSegmentValuePopup(); });
		registerActionHandler(RTTI_OF(AssignOutputIDToTrack), [this]{ handleAssignOutputIDToTrack(); });
		registerActionHandler(RTTI_OF(DraggingSegment), [this]{ handleSegmentDrag(); });

		/**
		 * register all edit popups for all registered views for possible event actions
		 * these views and actions are registered at initialization of sequence gui service, since we know all possible actions and
		 * their corresponding views at that time
		 */
		const auto& event_actions = mService.getAllRegisteredEventActions();
		for(const auto& action_type : event_actions)
		{
			registerActionHandler(action_type, [this, action_type](){ mService.invokeEditEventHandler(action_type, *this); });
		}

		/**
		 * create views for all event segments registered
		 */
		const auto& event_types = mService.getRegisteredSegmentEventTypes();
		for(const auto& event_type : event_types)
		{
			// create segment view
			auto segment_view = mService.invokeEventTrackSegmentViewFactory(event_type);

			// move ownership
			mSegmentViews.emplace(event_type, std::move(segment_view));
		}
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
				}

				event_outputs.emplace_back(output->mID);
			}
		}

		ImGui::PushItemWidth(200.0f);
		if (Combo(
			"",
			&current_item, event_outputs))
		{
			if (current_item != 0)
				mState.mAction = SequenceGUIActions::createAction<AssignOutputIDToTrack>(track.mID, event_outputs[current_item]);
			else
				mState.mAction = SequenceGUIActions::createAction<AssignOutputIDToTrack>(track.mID, "");

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
						SequenceGUIService::lightGrey, // color
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
						SequenceGUIService::lightGrey, // color
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
						SequenceGUIService::lightGrey, // color
						1.0f); // thickness
				}
			}
		}

		float prev_segment_x = 0.0f;

		int segment_count = 0;
		for (const auto& segment : track.mSegments)
		{
			float segment_x	   = (float)(segment->mStartTime) * mState.mStepSize;

			// draw segment handlers
			drawSegmentHandler(
				track,
				*(segment.get()),
				trackTopLeft, segment_x,
				0.0f, draw_list);

			// static map of draw functions for different event types
			auto type = (segment.get())->get_type();
			auto it = mSegmentViews.find(type);
			assert(it != mSegmentViews.end()); // type not found
			it->second->drawEvent(*(segment.get()), draw_list, trackTopLeft, segment_x);

			//
			prev_segment_x = segment_x;

			//
			segment_count++;
		}
	}


	void SequenceEventTrackView::handleInsertEventSegmentPopup()
	{
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
				auto* action = mState.mAction->getDerived<InsertingEventSegment>();

				auto& event_map = mService.getRegisteredSegmentEventTypes();
				for(auto& type : event_map)
				{
					std::string buttonString = "Insert " + type.get_name().to_string();
					if( ImGui::Button(buttonString.c_str()))
					{
						auto it = mSegmentViews.find(type);
						assert(it!=mSegmentViews.end()); // type not found

						it->second->insertSegment(getEditor().getController<SequenceControllerEvent>(), action->mTrackID, action->mTime);
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
					}
				}

				// handle paste if event segment is in clipboard
				if( mState.mClipboard->isClipboard<EventSegmentClipboard>())
				{
					if( ImGui::Button("Paste") )
					{
						// call appropriate paste method
						pasteEventsFromClipboard(action->mTrackID, action->mTime);

						// exit popup
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
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

			// draw handler of segment
			drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				SequenceGUIService::white, // color
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
						mState.mAction = createAction<DraggingSegment>(track.mID, segment.mID, segment.mStartTime);
					}
				}
			}

			// right mouse in deletion popup
			if (ImGui::IsMouseDown(1))
			{
				mState.mAction = createAction<OpenEditSegmentPopup>(track.mID, segment.mID, segment.get_type());
			}

			// handled shift click for add/remove to clipboard
			if( ImGui::IsMouseClicked(0) )
			{
				if( ImGui::GetIO().KeyShift )
				{
					// if no event segment clipboard, create it or if previous clipboard is from a different sequence, create new clipboard
					if (!mState.mClipboard->isClipboard<EventSegmentClipboard>() )
						mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());
					else if( mState.mClipboard->getDerived<EventSegmentClipboard>()->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename() )
						mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());

					// get derived clipboard
					auto* clipboard = mState.mClipboard->getDerived<EventSegmentClipboard>();

					// if the clipboard contains this segment or is a different sequence, remove it
					if (clipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()))
					{
						clipboard->removeObject(segment.mID);
					}else
					{
						// if not, serialize it into clipboard


						utility::ErrorState errorState;
						clipboard->addObject(&segment, getPlayer().getSequenceFilename(), errorState);

						// log any errors
						if(errorState.hasErrors())
						{
							Logger::error(errorState.toString());
						}
					}
				}
			}
		}
		else 
		{
			ImU32 line_color = SequenceGUIService::white;

			// if segment is in clipboard, line is red
			if( mState.mClipboard->isClipboard<EventSegmentClipboard>() )
			{
				if( mState.mClipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()) )
				{
					line_color = SequenceGUIService::red;
				}
			}

			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState.mTrackHeight }, // bottom right
				line_color, // color
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


	void SequenceEventTrackView::handleEditSegmentValuePopup()
	{
		if (mState.mAction->isAction<OpenEditSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Segment");

			auto* action = mState.mAction->getDerived<OpenEditSegmentPopup>();
			mState.mAction = createAction<EditingSegmentPopup>(action->mTrackID, action->mSegmentID, action->mSegmentType);
		}

		// handle delete segment popup
		if (mState.mAction->isAction<EditingSegmentPopup>())
		{
			if (ImGui::BeginPopup("Edit Segment"))
			{
				auto* action = mState.mAction->getDerived<EditingSegmentPopup>();

				bool display_copy = !mState.mClipboard->isClipboard<EventSegmentClipboard>();

				// if clipboard is from different sequence, create new clipboard, so display copy
				if( !display_copy )
				{
					if( mState.mClipboard->getDerived<EventSegmentClipboard>()->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename() )
						mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());
				}

				if( display_copy )
				{
					if(ImGui::Button("Copy"))
					{
						// create clipboard
						mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());

						// serialize segment
						utility::ErrorState errorState;
						auto& controller = getEditor().getController<SequenceControllerEvent>();
						const auto* event_segment = controller.getSegment(action->mTrackID, action->mSegmentID);
						mState.mClipboard->addObject(event_segment, getPlayer().getSequenceFilename(), errorState);

						// exit popup
						ImGui::CloseCurrentPopup();
						mState.mAction = createAction<None>();
						ImGui::EndPopup();

						return;
					}
				}else
				{
					// obtain derived clipboard
					auto* clipboard = mState.mClipboard->getDerived<EventSegmentClipboard>();

					// is event contained by clipboard ? if so, add remove button
					bool display_remove_from_clipboard = clipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename());

					if( display_remove_from_clipboard )
					{
						if( ImGui::Button("Remove from clipboard") )
						{
							clipboard->removeObject(action->mSegmentID);

							// if clipboard is empty, remove current clipboard
							if( clipboard->getObjectCount() == 0 )
							{
								mState.mClipboard = createClipboard<Empty>();
							}

							// exit popup
							ImGui::CloseCurrentPopup();
							mState.mAction = createAction<None>();
							ImGui::EndPopup();

							return;
						}
					}else
					{
						// clipboard is of correct type, but does not contain this segment, present the user with an add button
						if( ImGui::Button("Add to clipboard") )
						{
							// obtain controller
							auto& controller = getEditor().getController<SequenceControllerEvent>();

							// fetch segment
							const auto* segment = controller.getSegment(action->mTrackID, action->mSegmentID);

							// serialize object into clipboard
							utility::ErrorState errorState;
							clipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);

							// log any errors
							if(errorState.hasErrors())
							{
								nap::Logger::error(errorState.toString());
							}

							// exit popup
							ImGui::CloseCurrentPopup();
							mState.mAction = createAction<None>();
							ImGui::EndPopup();

							return;
						}
					}
				}

				if (ImGui::Button("Delete"))
				{
					auto& controller = getEditor().getController<SequenceControllerEvent>();
					controller.deleteSegment(action->mTrackID, action->mSegmentID);

					// remove segment from clipboard
					if(mState.mClipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename()))
					{
						mState.mClipboard->removeObject(action->mSegmentID);
					}

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
							const auto *eventSegment = dynamic_cast<const SequenceTrackSegmentEventBase*>(eventController.getSegment(action->mTrackID, action->mSegmentID));

							assert(eventSegment != nullptr);

							if( eventSegment != nullptr)
							{
								rttr::type type = eventSegment->get_type();

								auto it = mSegmentViews.find(type);
								assert(it!=mSegmentViews.end()); // type not found
								mState.mAction = it->second->createEditAction(eventSegment, action->mTrackID, action->mSegmentID);
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
	}


	void SequenceEventTrackView::pasteEventsFromClipboard(const std::string& trackID, double time)
	{
		auto* paste_clipboard = mState.mClipboard->getDerived<SequenceGUIClipboards::EventSegmentClipboard>();

		// create vector & object ptr to be filled by de-serialization
		std::vector<std::unique_ptr<rtti::Object>> read_objects;
		SequenceTrackSegmentEventBase* event_segment = nullptr;

		// continue upon successful de-serialization
		utility::ErrorState errorState;
		std::vector<SequenceTrackSegmentEventBase*> deserialized_event_segments = paste_clipboard->deserialize<SequenceTrackSegmentEventBase>(read_objects, errorState);

		assert(!deserialized_event_segments.empty() ); // no event segments de serialized

		// no errors ? continue
		if(!errorState.hasErrors() && !deserialized_event_segments.empty())
		{
			// sort event segments by start time
			std::sort(deserialized_event_segments.begin(), deserialized_event_segments.end(), [](SequenceTrackSegmentEventBase* a, SequenceTrackSegmentEventBase* b)
			{
			  return a->mStartTime < b->mStartTime;
			});

			// adjust start times by duration and start from 0.0
			double first_segment_time = 0.0;
			if( !deserialized_event_segments.empty() )
			{
				first_segment_time = deserialized_event_segments[0]->mStartTime;
				deserialized_event_segments[0]->mStartTime = 0.0;
			}
			for(int i = 1 ; i < deserialized_event_segments.size(); i++)
			{
				deserialized_event_segments[i]->mStartTime = deserialized_event_segments[i]->mStartTime - first_segment_time;
			}

			// obtain controller
			auto& controller = getEditor().getController<SequenceControllerEvent>();

			for(const auto* event : deserialized_event_segments)
			{
				mService.invokePasteEvent(event->get_type(), *this, trackID, *event, time);
			}
		}
	}


	void SequenceEventTrackView::updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID)
	{
		if( mState.mClipboard->isClipboard<EventSegmentClipboard>() )
		{
			if( mState.mClipboard->containsObject(segmentID, getPlayer().getSequenceFilename()) )
			{
				mState.mClipboard->removeObject(segmentID);

				auto& controller = getEditor().getController<SequenceControllerEvent>();
				const auto* segment = controller.getSegment(trackID, segmentID);

				utility::ErrorState errorState;
				mState.mClipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);
			}
		}
	}


	void SequenceEventTrackView::handleAssignOutputIDToTrack()
	{
		// get action
		auto* action = mState.mAction->getDerived<AssignOutputIDToTrack>();
		assert(action!=nullptr);

		// get curve controller
		auto& curve_controller = getEditor().getController<SequenceControllerEvent>();

		// call function to controller
		curve_controller.assignNewObjectID(action->mTrackID, action->mObjectID);

		// action is done
		mState.mAction = SequenceGUIActions::createAction<None>();
	}


	void SequenceEventTrackView::handleSegmentDrag()
	{
		if (ImGui::IsMouseDown(0))
		{
			auto* action = mState.mAction->getDerived<SequenceGUIActions::DraggingSegment>();
			assert(action!= nullptr);

			// calc new time
			float amount = mState.mMouseDelta.x / mState.mStepSize;
			action->mNewDuration += amount;

			// get editor & controller
			auto& editor = getEditor();
			auto& event_controller = editor.getController<SequenceControllerEvent>();

			// change start time of segment
			event_controller.segmentEventStartTimeChange(action->mTrackID, action->mSegmentID, action->mNewDuration);

			// update segment in clipboard
			updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
		}else
		{
			mState.mAction = SequenceGUIActions::createAction<SequenceGUIActions::None>();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// std::string event segment view specialization
	//////////////////////////////////////////////////////////////////////////

	template<>
	void SequenceEventTrackSegmentView<std::string>::handleEditPopupContent(SequenceGUIActions::Action& action)
	{
		auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<std::string>>();
		auto& message = static_cast<std::string&>(edit_action->mValue);

		char buffer[256];
		strcpy(buffer, message.c_str());

		if (ImGui::InputText("message", buffer, 256))
		{
			message = std::string(buffer);
		}
	}


	template<>
	void SequenceEventTrackSegmentView<std::string>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventString>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventString&>(segment);

		std::ostringstream string_stream;
		string_stream << "\"" << segment_event.mValue << "\"" ;

		drawList->AddText(
			{ topLeft.x + x + 5, topLeft.y + 5 },
			SequenceGUIService::red, string_stream.str().c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// float event segment view specialization
	//////////////////////////////////////////////////////////////////////////


	template<>
	void SequenceEventTrackSegmentView<float>::handleEditPopupContent(SequenceGUIActions::Action& action)
	{
		auto* editAction = action.getDerived<SequenceGUIActions::EditingEventSegment<float>>();
		auto& value = static_cast<float&>(editAction->mValue);

		ImGui::InputFloat("Value", &value);
	}


	template<>
	void SequenceEventTrackSegmentView<float>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventFloat>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventFloat&>(segment);

		std::ostringstream string_stream;
		string_stream << segment_event.mValue;

		drawList->AddText(
			{ topLeft.x + x + 5, topLeft.y + 5 },
			SequenceGUIService::red,
			string_stream.str().c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// int event segment view specialization
	//////////////////////////////////////////////////////////////////////////


	template<>
	void SequenceEventTrackSegmentView<int>::handleEditPopupContent(SequenceGUIActions::Action& action)
	{
		auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<int>>();
		int& value = static_cast<int&>(edit_action->mValue);

		ImGui::InputInt("Value", &value);
	}


	template<>
	void SequenceEventTrackSegmentView<int>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventInt>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventInt&>(segment);

		std::ostringstream string_stream;
		string_stream << segment_event.mValue;

		drawList->AddText(
			{ topLeft.x + x + 5, topLeft.y + 5 },
			SequenceGUIService::red,
			string_stream.str().c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// glm::vec2 event segment view specialization
	//////////////////////////////////////////////////////////////////////////


	template<>
	void SequenceEventTrackSegmentView<glm::vec2>::handleEditPopupContent(SequenceGUIActions::Action& action)
	{
		auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec2>>();
		auto& value = static_cast<glm::vec2&>(edit_action->mValue);

		ImGui::InputFloat2("Value", &value.x);
	}


	template<>
	void SequenceEventTrackSegmentView<glm::vec2>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventVec2>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec2&>(segment);

		std::ostringstream string_stream;
		string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ")";

		drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },SequenceGUIService::red,string_stream.str().c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// glm::vec3 event segment view specialization
	//////////////////////////////////////////////////////////////////////////


	template<>
	void SequenceEventTrackSegmentView<glm::vec3>::handleEditPopupContent(SequenceGUIActions::Action& action)
	{
		auto* edit_action = action.getDerived<SequenceGUIActions::EditingEventSegment<glm::vec3>>();
		auto& value = static_cast<glm::vec3&>(edit_action->mValue);

		ImGui::InputFloat3("Value", &value.x);
	}


	template<>
	void SequenceEventTrackSegmentView<glm::vec3>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, const float x)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventVec3>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec3&>(segment);

		std::ostringstream string_stream;
		string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ", " << segment_event.mValue.z << ")";

		drawList->AddText({ topLeft.x + x + 5, topLeft.y + 5 },SequenceGUIService::red,
						  string_stream.str().c_str());
	}
}
