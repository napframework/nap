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
#include <imguiutils.h>

namespace nap
{
    using namespace sequenceguiactions;
    using namespace sequenceguiclipboard;


    SequenceEventTrackView::SequenceEventTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state)
        : SequenceTrackView(view, state)
    {
        // register applicable action handlers
        registerActionHandler(RTTI_OF(InsertEventSegmentPopup), [this] { handleInsertEventSegmentPopup(); });
        registerActionHandler(RTTI_OF(EditSegmentPopup), [this] { handleEditSegmentValuePopup(); });
        registerActionHandler(RTTI_OF(AssignOutputIDToTrack), [this] { handleAssignOutputIDToTrack(); });
        registerActionHandler(RTTI_OF(DraggingSegment), [this] { handleSegmentDrag(); });
        registerActionHandler(RTTI_OF(LoadPresetPopup), [this] { handleLoadPresetPopup(); });

        /**
         * register all edit popups for all registered views for possible event actions
         * these views and actions are registered at initialization of sequence gui service, since we know all possible actions and
         * their corresponding views at that time
         */
        const auto &event_actions = mService.getAllRegisteredEventActions();
        for(const auto &action_type: event_actions)
        {
            registerActionHandler(action_type, [this, action_type]() { mService.invokeEditEventHandler(action_type, *this); });
        }

        /**
         * create views for all event segments registered
         */
        const auto &event_types = mService.getRegisteredSegmentEventTypes();
        for(const auto &event_type: event_types)
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
        inspector_cursor_pos.x += (5.0f * mState.mScale);
        inspector_cursor_pos.y += (5.0f * mState.mScale);
        ImGui::SetCursorPos(inspector_cursor_pos);

        bool assigned = false;
        std::string assigned_id;
        std::vector<std::string> event_outputs;
        int current_item = 0;
        event_outputs.emplace_back("none");
        int count = 0;
        const SequencePlayerEventOutput *event_output = nullptr;

        for(const auto &output: getEditor().mSequencePlayer->mOutputs)
        {
            if(output.get()->get_type() == RTTI_OF(SequencePlayerEventOutput))
            {
                count++;

                if(output->mID == track.mAssignedOutputID)
                {
                    assigned = true;
                    assigned_id = output->mID;
                    current_item = count;

                    assert(output.get()->get_type() == RTTI_OF(SequencePlayerEventOutput)); // type mismatch
                }

                event_outputs.emplace_back(output->mID);
            }
        }

        ImGui::PushItemWidth(200.0f * mState.mScale);
        if(Combo(
            "",
            &current_item, event_outputs))
        {
            if(current_item != 0)
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, event_outputs[current_item]);
            else
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, "");

        }
        ImGui::PopItemWidth();
    }


    void SequenceEventTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        if(mState.mIsWindowFocused)
        {
            const float track_height = track.mTrackHeight * mState.mScale;
            // handle insertion of segment
            if(mState.mAction->isAction<None>())
            {
                if(ImGui::IsMouseHoveringRect(
                    trackTopLeft, // top left position
                    {trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + track_height}))
                {
                    // position of mouse in track
                    draw_list->AddLine(
                        {mState.mMousePos.x, trackTopLeft.y}, // top left
                        {mState.mMousePos.x, trackTopLeft.y + track_height}, // bottom right
                        mService.getColors().mFro2, // color
                        1.0f * mState.mScale); // thickness

                    ImGui::BeginTooltip();
                    ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());
                    ImGui::EndTooltip();

                    // right mouse down
                    if(ImGui::IsMouseClicked(1))
                    {
                        double time = mState.mMouseCursorTime;

                        //
                        mState.mAction = createAction<InsertEventSegmentPopup>(track.mID, time);
                    }
                }
            }

            // draw line in track while in inserting segment popup
            if(mState.mAction->isAction<InsertEventSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertEventSegmentPopup>();
                if(action->mTrackID == track.mID)
                {
                    // position of insertion in track
                    draw_list->AddLine(
                        {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y}, // top left
                        {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                         trackTopLeft.y + track_height}, // bottom right
                        mService.getColors().mFro2, // color
                        1.0f * mState.mScale); // thickness
                }
            }

            if(mState.mAction->isAction<InsertEventSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertEventSegmentPopup>();
                if(action->mTrackID == track.mID)
                {
                    // position of insertion in track
                    draw_list->AddLine(
                        {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y}, // top left
                        {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                         trackTopLeft.y + track_height}, // bottom right
                        mService.getColors().mFro2, // color
                        1.0f * mState.mScale); // thickness
                }
            }
        }

        float prev_segment_x = 0.0f;

        int segment_count = 0;
        for(const auto &segment: track.mSegments)
        {
            float segment_x = (float) (segment->mStartTime) * mState.mStepSize;

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
            it->second->drawEvent(*(segment.get()), draw_list, trackTopLeft,
                                  segment_x + (5.0f * mState.mScale), mService.getColors().mFro4);

            prev_segment_x = segment_x;
            segment_count++;
        }
    }


    void SequenceEventTrackView::handleInsertEventSegmentPopup()
    {
        auto* action = mState.mAction->getDerived<InsertEventSegmentPopup>();
        assert(action!= nullptr);
        
        if(!action->mOpened)
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Insert Event");
            action->mOpened = true;
        }

        // handle insert segment popup
        if(ImGui::BeginPopup("Insert Event"))
        {
            auto &event_map = mService.getRegisteredSegmentEventTypes();
            for(auto &type: event_map)
            {
                std::string type_str = utility::stripNamespace(type.get_name().to_string());
                std::string btn_str = "Insert " + type_str;
                ImGui::PushID(type_str.c_str());
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert), btn_str.c_str()))
                {
                    auto it = mSegmentViews.find(type);
                    assert(it != mSegmentViews.end()); // type not found

                    // take snapshot
                    getEditor().takeSnapshot(action->get_type());

                    it->second->insertSegment(getEditor().getController<SequenceControllerEvent>(), action->mTrackID, action->mTime);
                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                }
                ImGui::SameLine();
                ImGui::Text(type_str.c_str());
                ImGui::PopID();
            }

            // handle paste if event segment is in clipboard
            if(mState.mClipboard->isClipboard<EventSegmentClipboard>())
            {
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::paste)))
                {
                    // take snapshot
                    getEditor().takeSnapshot(action->get_type());

                    // call appropriate paste method
                    utility::ErrorState error_state;
                    if(!pasteEventsFromClipboard(action->mTrackID, action->mTime, error_state))
                    {
                        ImGui::OpenPopup("Error");
                        action->mErrorString = error_state.toString();
                    } else
                    {
                        // exit popup
                        ImGui::CloseCurrentPopup();
                        mState.mAction = createAction<None>();
                    }
                }
                ImGui::SameLine();
            }

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::load), "Load preset"))
            {
                if(mState.mClipboard->getTrackType() != RTTI_OF(SequenceTrackEvent))
                {
                    mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent),
                                                                               getEditor().mSequencePlayer->getSequenceFilename());
                }
                mState.mAction = createAction<LoadPresetPopup>(action->mTrackID, action->mTime, RTTI_OF(SequenceTrackEvent));
                ImGui::CloseCurrentPopup();
            }

            if(ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(action->mErrorString.c_str());
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
                {
                    mState.mDirty = true;
                    mState.mAction = createAction<None>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::cancel)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::EndPopup();
        } else
        {
            // click outside popup so cancel action
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEventTrackView::drawSegmentHandler(
            const SequenceTrack& track,
            const SequenceTrackSegment& segment,
            const ImVec2& trackTopLeft,
            const float segmentX,
            const float segmentWidth,
            ImDrawList* drawList)
    {
        const float track_height = track.mTrackHeight * mState.mScale;
        float seg_bounds = 10.0f * mState.mScale;

        // segment handler
        if(((mState.mIsWindowFocused && ImGui::IsMouseHoveringRect(
            {trackTopLeft.x + segmentX - seg_bounds, trackTopLeft.y - seg_bounds},
            {trackTopLeft.x + segmentX + seg_bounds, trackTopLeft.y + track_height + seg_bounds})) &&
            (mState.mAction->isAction<None>() || (mState.mAction->isAction<HoveringSegment>() &&
                                                  mState.mAction->getDerived<HoveringSegment>()->mSegmentID ==
                                                  segment.mID)))
           ||
           (mState.mAction->isAction<DraggingSegment>() &&
            mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID))
        {
            bool isAlreadyHovering = false;
            if(mState.mAction->isAction<HoveringSegment>())
            {
                isAlreadyHovering = mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID;
            }

            bool isAlreadyDragging = false;
            if(mState.mAction->isAction<DraggingSegment>())
            {
                isAlreadyDragging = mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID;
            }

            // draw handler of segment
            drawList->AddLine(
                {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                mService.getColors().mFro4, // color
                3.0f * mState.mScale); // thickness

            if(!isAlreadyHovering && !isAlreadyDragging)
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
            if(!isAlreadyDragging)
            {
                if(!mState.mAction->isAction<DraggingSegment>())
                {
                    if(ImGui::IsMouseDown(0))
                    {
                        bool move_next_segment = ImGui::GetIO().KeyCtrl;
                        mState.mAction = createAction<DraggingSegment>(track.mID, segment.mID, segment.mStartTime, move_next_segment);
                    }
                }
            }

            // right mouse in deletion popup
            if(ImGui::IsMouseDown(1))
            {
                mState.mAction = createAction<EditSegmentPopup>(track.mID, segment.mID, segment.get_type());
            }

            // handled shift click for add/remove to clipboard
            if(ImGui::IsMouseClicked(0))
            {
                if(ImGui::GetIO().KeyShift)
                {
                    // if no event segment clipboard, create it or if previous clipboard is from a different sequence, create new clipboard
                    if(!mState.mClipboard->isClipboard<EventSegmentClipboard>())
                        mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());
                    else if(mState.mClipboard->getDerived<EventSegmentClipboard>()->getSequenceName() !=
                            getEditor().mSequencePlayer->getSequenceFilename())
                        mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());

                    // get derived clipboard
                    auto *clipboard = mState.mClipboard->getDerived<EventSegmentClipboard>();

                    // if the clipboard contains this segment or is a different sequence, remove it
                    if(clipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()))
                    {
                        clipboard->removeObject(segment.mID);
                    } else
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
        } else
        {
            ImU32 line_color = mService.getColors().mFro3;

            // if segment is in clipboard, line is red
            if(mState.mClipboard->isClipboard<EventSegmentClipboard>())
            {
                if(mState.mClipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()))
                {
                    line_color = mService.getColors().mHigh1;
                }
            }

            // draw handler of segment duration
            drawList->AddLine(
                {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                line_color, // color
                1.0f * mState.mScale); // thickness

            if(mState.mAction->isAction<HoveringSegment>())
            {
                auto *action = mState.mAction->getDerived<HoveringSegment>();
                if(action->mSegmentID == segment.mID)
                {
                    mState.mAction = createAction<None>();
                }
            }
        }

        if(ImGui::IsMouseReleased(0))
        {
            if(mState.mAction->isAction<DraggingSegment>())
            {
                if(mState.mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID)
                {
                    mState.mAction = createAction<None>();
                }
            }
        }
    }


    void SequenceEventTrackView::handleEditSegmentValuePopup()
    {
        auto* action = mState.mAction->getDerived<EditSegmentPopup>();
        assert(action!= nullptr);
        if(!action->mOpened)
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Edit Segment");
            action->mOpened = true;
        }

        // handle edit segment popup
        if(ImGui::BeginPopup("Edit Segment"))
        {
            auto *action = mState.mAction->getDerived<EditSegmentPopup>();

            bool display_copy = !mState.mClipboard->isClipboard<EventSegmentClipboard>();

            // if clipboard is from different sequence, create new clipboard, so display copy
            if(!display_copy)
            {
                if(mState.mClipboard->getDerived<EventSegmentClipboard>()->getSequenceName() !=
                   getEditor().mSequencePlayer->getSequenceFilename())
                    mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());
            }

            if(display_copy)
            {
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::copy)))
                {
                    // create clipboard
                    mState.mClipboard = createClipboard<EventSegmentClipboard>(RTTI_OF(SequenceTrackEvent), getEditor().mSequencePlayer->getSequenceFilename());

                    // serialize segment
                    utility::ErrorState errorState;
                    auto &controller = getEditor().getController<SequenceControllerEvent>();
                    const auto *event_segment = controller.getSegment(action->mTrackID, action->mSegmentID);
                    mState.mClipboard->addObject(event_segment, getPlayer().getSequenceFilename(), errorState);

                    // exit popup
                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                    ImGui::EndPopup();

                    return;
                }
            } else
            {
                // obtain derived clipboard
                auto *clipboard = mState.mClipboard->getDerived<EventSegmentClipboard>();

                // is event contained by clipboard ? if so, add remove button
                bool display_remove_from_clipboard = clipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename());

                if(display_remove_from_clipboard)
                {
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::remove), "Remove from clipboard"))
                    {
                        clipboard->removeObject(action->mSegmentID);

                        // if clipboard is empty, remove current clipboard
                        if(clipboard->getObjectCount() == 0)
                        {
                            mState.mClipboard = createClipboard<Empty>();
                        }

                        // exit popup
                        ImGui::CloseCurrentPopup();
                        mState.mAction = createAction<None>();
                        ImGui::EndPopup();
                        return;
                    }
                } else
                {
                    // clipboard is of correct type, but does not contain this segment, present the user with an add button
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::copy), "Add to clipboard"))
                    {
                        // obtain controller
                        auto &controller = getEditor().getController<SequenceControllerEvent>();

                        // fetch segment
                        const auto *segment = controller.getSegment(action->mTrackID, action->mSegmentID);

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
            ImGui::SameLine();

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::del)))
            {
                getEditor().takeSnapshot(action->get_type());

                auto &controller = getEditor().getController<SequenceControllerEvent>();
                controller.deleteSegment(action->mTrackID, action->mSegmentID);

                // remove segment from clipboard
                if(mState.mClipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename()))
                {
                    mState.mClipboard->removeObject(action->mSegmentID);
                }

                mState.mDirty = true;
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            } else
            {
                if(action->mSegmentType.is_derived_from<SequenceTrackSegmentEventBase>())
                {
                    ImGui::SameLine();
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::edit)))
                    {
                        auto &eventController = getEditor().getController<SequenceControllerEvent>();
                        const auto *eventSegment = dynamic_cast<const SequenceTrackSegmentEventBase *>(eventController.getSegment(action->mTrackID, action->mSegmentID));

                        assert(eventSegment != nullptr);

                        if(eventSegment != nullptr)
                        {
                            rttr::type type = eventSegment->get_type();

                            auto it = mSegmentViews.find(type);
                            assert(it != mSegmentViews.end()); // type not found
                            mState.mAction = it->second->createEditAction(eventSegment, action->mTrackID, action->mSegmentID);
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::cancel)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::EndPopup();
        } else
        {
            // click outside popup so cancel action
            mState.mAction = createAction<None>();
        }
    }


    bool SequenceEventTrackView::pasteEventsFromClipboard(const std::string& trackID, double time, utility::ErrorState& errorState)
    {
        auto *paste_clipboard = mState.mClipboard->getDerived<sequenceguiclipboard::EventSegmentClipboard>();

        // create vector & object ptr to be filled by de-serialization
        std::vector<std::unique_ptr<rtti::Object>> read_objects;
        SequenceTrackSegmentEventBase *event_segment = nullptr;

        // continue upon successful de-serialization
        std::vector<SequenceTrackSegmentEventBase *> deserialized_event_segments = paste_clipboard->deserialize<SequenceTrackSegmentEventBase>(read_objects, errorState);

        // no errors ? continue
        if(!errorState.hasErrors() && !deserialized_event_segments.empty())
        {
            // sort event segments by start time
            std::sort(deserialized_event_segments.begin(), deserialized_event_segments.end(), [](SequenceTrackSegmentEventBase *a, SequenceTrackSegmentEventBase *b)
            {
                return a->mStartTime < b->mStartTime;
            });

            // adjust start times by duration and start from 0.0
            double first_segment_time = 0.0;
            if(!deserialized_event_segments.empty())
            {
                first_segment_time = deserialized_event_segments[0]->mStartTime;
                deserialized_event_segments[0]->mStartTime = 0.0;
            }
            for(int i = 1; i < deserialized_event_segments.size(); i++)
            {
                deserialized_event_segments[i]->mStartTime =
                    deserialized_event_segments[i]->mStartTime - first_segment_time;
            }

            // obtain controller
            auto &controller = getEditor().getController<SequenceControllerEvent>();

            for(const auto *event: deserialized_event_segments)
            {
                mService.invokePasteEvent(event->get_type(), *this, trackID, *event, time);
            }
        } else
        {
            if(!errorState.hasErrors())
            {
                if(errorState.check(!deserialized_event_segments.empty(), "No events de-serialized"))
                    return false;
            }

            return false;
        }

        return true;
    }


    void SequenceEventTrackView::updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID)
    {
        if(mState.mClipboard->isClipboard<EventSegmentClipboard>())
        {
            if(mState.mClipboard->containsObject(segmentID, getPlayer().getSequenceFilename()))
            {
                mState.mClipboard->removeObject(segmentID);

                auto &controller = getEditor().getController<SequenceControllerEvent>();
                const auto *segment = controller.getSegment(trackID, segmentID);

                utility::ErrorState errorState;
                mState.mClipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);
            }
        }
    }


    void SequenceEventTrackView::handleAssignOutputIDToTrack()
    {
        // get action
        auto *action = mState.mAction->getDerived<AssignOutputIDToTrack>();
        assert(action != nullptr);

        // get curve controller
        auto &event_controller = getEditor().getController<SequenceControllerEvent>();

        // take snapshot
        getEditor().takeSnapshot(action->get_type());

        // call function to controller
        event_controller.assignNewOutputID(action->mTrackID, action->mOutputID);

        // action is done
        mState.mAction = sequenceguiactions::createAction<None>();
    }


    void SequenceEventTrackView::handleSegmentDrag()
    {
        if(ImGui::IsMouseDown(0))
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingSegment>();
            assert(action != nullptr);

            // calc new time
            float amount = mState.mMouseDelta.x / mState.mStepSize;
            action->mNewDuration += amount;

            // get editor & controller
            auto &editor = getEditor();
            auto &event_controller = editor.getController<SequenceControllerEvent>();

            // take snapshot
            if(action->mTakeSnapshot)
            {
                action->mTakeSnapshot = false;
                editor.takeSnapshot(action->get_type());
            }

            // change start time of segment
            event_controller.segmentEventStartTimeChange(action->mTrackID, action->mSegmentID, action->mNewDuration);

            // update segment in clipboard
            updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
        } else
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    void SequenceEventTrackView::handleLoadPresetPopup()
    {
        if(mState.mAction->isAction<LoadPresetPopup>())
        {
            auto *load_action = mState.mAction->getDerived<LoadPresetPopup>();
            auto *controller = getEditor().getControllerWithTrackType(load_action->mTrackType);

            if(controller->get_type().is_derived_from<SequenceControllerEvent>())
            {
                const std::string popup_name = "Load preset";
                if(!ImGui::IsPopupOpen(popup_name.c_str()))
                    ImGui::OpenPopup(popup_name.c_str());

                //
                if(ImGui::BeginPopupModal(popup_name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    //
                    const std::string presets_dir = utility::joinPath({"sequences", "presets"});

                    // Find all files in the preset directory
                    std::vector<std::string> files_in_directory;
                    utility::listDir(presets_dir.c_str(), files_in_directory);

                    std::vector<std::string> presets;
                    std::vector<std::string> preset_files;
                    for(const auto &filename: files_in_directory)
                    {
                        // Ignore directories
                        if(utility::dirExists(filename))
                            continue;

                        if(utility::getFileExtension(filename) == "json")
                        {
                            presets.emplace_back(utility::getFileName(filename));
                            preset_files.emplace_back(filename);
                        }
                    }

                    SequenceTrackView::Combo("Presets", &load_action->mSelectedPresetIndex, presets);

                    utility::ErrorState error_state;
                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok), "load preset"))
                    {
                        if(mState.mClipboard->load(preset_files[load_action->mSelectedPresetIndex], error_state))
                        {
                            // take snapshot
                            getEditor().takeSnapshot(load_action->get_type());

                            if(pasteEventsFromClipboard(load_action->mTrackID, load_action->mTime, error_state))
                            {
                                mState.mAction = createAction<None>();
                                mState.mDirty = true;
                                mState.mClipboard = createClipboard<Empty>();
                                ImGui::CloseCurrentPopup();
                            } else
                            {
                                mState.mClipboard = createClipboard<Empty>();
                                ImGui::OpenPopup("Error");
                                load_action->mErrorString = error_state.toString();
                            }
                        } else
                        {
                            ImGui::OpenPopup("Error");
                            load_action->mErrorString = error_state.toString();
                        }
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
                    {
                        mState.mAction = createAction<None>();
                        ImGui::CloseCurrentPopup();
                    }

                    if(ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        ImGui::Text(load_action->mErrorString.c_str());
                        if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                        {
                            mState.mDirty = true;
                            mState.mAction = createAction<None>();
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }
                    ImGui::EndPopup();
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // std::string event segment view specialization
    //////////////////////////////////////////////////////////////////////////

    template<>
    bool SequenceEventTrackSegmentView<std::string>::handleEditPopupContent(sequenceguiactions::Action& action)
    {
        auto *edit_action = action.getDerived<sequenceguiactions::EditingEventSegment<std::string>>();
        auto &message = static_cast<std::string &>(edit_action->mValue);

        char buffer[256];
        strcpy(buffer, message.c_str());

        bool edit = false;
        if(ImGui::InputText("message", buffer, 256))
        {
            edit = true;
            message = std::string(buffer);
        }

        return edit;
    }


    template<>
    void
    SequenceEventTrackSegmentView<std::string>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
    {
        assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventString>());
        const auto &segment_event = static_cast<const SequenceTrackSegmentEventString &>(segment);
        std::ostringstream string_stream;
        string_stream << "\"" << segment_event.mValue << "\"";
        drawList->AddText({topLeft.x + x, topLeft.y}, color, string_stream.str().c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    // float event segment view specialization
    //////////////////////////////////////////////////////////////////////////


    template<>
    bool SequenceEventTrackSegmentView<float>::handleEditPopupContent(sequenceguiactions::Action& action)
    {
        auto *editAction = action.getDerived<sequenceguiactions::EditingEventSegment<float>>();
        auto &value = static_cast<float &>(editAction->mValue);

        return ImGui::InputFloat("Value", &value);
    }


    template<>
    void SequenceEventTrackSegmentView<float>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
    {
        assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventFloat>());
        const auto &segment_event = static_cast<const SequenceTrackSegmentEventFloat &>(segment);

        std::ostringstream string_stream;
        string_stream << segment_event.mValue;

        drawList->AddText(
            {topLeft.x + x, topLeft.y},
            color,
            string_stream.str().c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    // int event segment view specialization
    //////////////////////////////////////////////////////////////////////////


    template<>
    bool SequenceEventTrackSegmentView<int>::handleEditPopupContent(sequenceguiactions::Action& action)
    {
        auto *edit_action = action.getDerived<sequenceguiactions::EditingEventSegment<int>>();
        int &value = static_cast<int &>(edit_action->mValue);

        return ImGui::InputInt("Value", &value);
    }


    template<>
    void SequenceEventTrackSegmentView<int>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
    {
        assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventInt>());
        const auto &segment_event = static_cast<const SequenceTrackSegmentEventInt &>(segment);

        std::ostringstream string_stream;
        string_stream << segment_event.mValue;

        drawList->AddText
            (
                {topLeft.x + x, topLeft.y},
                color,
                string_stream.str().c_str()
            );
    }

    //////////////////////////////////////////////////////////////////////////
    // glm::vec2 event segment view specialization
    //////////////////////////////////////////////////////////////////////////


    template<>
    bool SequenceEventTrackSegmentView<glm::vec2>::handleEditPopupContent(sequenceguiactions::Action& action)
    {
        auto *edit_action = action.getDerived<sequenceguiactions::EditingEventSegment<glm::vec2>>();
        auto &value = static_cast<glm::vec2 &>(edit_action->mValue);
        return ImGui::InputFloat2("Value", &value.x);
    }


    template<>
    void SequenceEventTrackSegmentView<glm::vec2>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
    {
        assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventVec2>());
        const auto &segment_event = static_cast<const SequenceTrackSegmentEventVec2 &>(segment);
        std::ostringstream string_stream;
        string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ")";
        drawList->AddText({topLeft.x + x, topLeft.y}, color, string_stream.str().c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    // glm::vec3 event segment view specialization
    //////////////////////////////////////////////////////////////////////////


    template<>
    bool SequenceEventTrackSegmentView<glm::vec3>::handleEditPopupContent(sequenceguiactions::Action& action)
    {
        auto *edit_action = action.getDerived<sequenceguiactions::EditingEventSegment<glm::vec3>>();
        auto &value = static_cast<glm::vec3 &>(edit_action->mValue);

        return ImGui::InputFloat3("Value", &value.x);
    }


    template<>
    void SequenceEventTrackSegmentView<glm::vec3>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
    {
        assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventVec3>());
        const auto &segment_event = static_cast<const SequenceTrackSegmentEventVec3 &>(segment);
        std::ostringstream string_stream;
        string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ", " << segment_event.mValue.z << ")";
        drawList->AddText({topLeft.x + x, topLeft.y}, color, string_stream.str().c_str());
    }


	//////////////////////////////////////////////////////////////////////////
	// glm::vec4 event segment view specialization
	//////////////////////////////////////////////////////////////////////////


	template<>
	bool SequenceEventTrackSegmentView<glm::vec4>::handleEditPopupContent(sequenceguiactions::Action& action)
	{
		auto* edit_action = action.getDerived<sequenceguiactions::EditingEventSegment<glm::vec4>>();
		auto& value = static_cast<glm::vec4&>(edit_action->mValue);

		return ImGui::InputFloat4("Value", &value.x);
	}


	template<>
	void SequenceEventTrackSegmentView<glm::vec4>::drawEvent(const SequenceTrackSegment& segment, ImDrawList* drawList, const ImVec2& topLeft, float x, ImU32 color)
	{
		assert(segment.get_type().is_derived_from<SequenceTrackSegmentEventVec4>());
		const auto& segment_event = static_cast<const SequenceTrackSegmentEventVec4&>(segment);
		std::ostringstream string_stream;
		string_stream << "(" << segment_event.mValue.x << ", " << segment_event.mValue.y << ", " << segment_event.mValue.z << ", " << segment_event.mValue.w << ")";
		drawList->AddText({ topLeft.x + x, topLeft.y }, color, string_stream.str().c_str());
	}
}
