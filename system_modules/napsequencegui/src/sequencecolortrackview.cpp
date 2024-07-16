/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecolortrackview.h"
#include "sequencecontrollercolor.h"
#include "sequenceeditorgui.h"
#include "sequenceplayercoloroutput.h"
#include "sequencetrackcolor.h"
#include "sequenceguiservice.h"

#include <nap/logger.h>
#include <iostream>
#include <imguiutils.h>

namespace nap
{
    using namespace sequenceguiactions;
    using namespace sequenceguiclipboard;


    SequenceColorTrackView::SequenceColorTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state)
            : SequenceTrackView(view, state)
    {
        // register applicable action handlers
        registerActionHandler(RTTI_OF(InsertColorSegmentPopup), [this] { handleInsertColorSegmentPopup(); });
        registerActionHandler(RTTI_OF(EditSegmentPopup), [this] { handleEditSegmentValuePopup(); });
        registerActionHandler(RTTI_OF(AssignOutputIDToTrack), [this] { handleAssignOutputIDToTrack(); });
        registerActionHandler(RTTI_OF(DraggingSegment), [this] { handleSegmentDrag(); });
        registerActionHandler(RTTI_OF(LoadPresetPopup), [this] { handleLoadPresetPopup(); });

        mSegmentViews.emplace(RTTI_OF(SequenceTrackSegmentColor), std::make_unique<SequenceColorTrackSegmentView>());
    }


    void SequenceColorTrackView::showInspectorContent(const SequenceTrack& track)
    {
        // draw the assigned receiver
        ImGui::Text("Assigned Output");

        ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
        inspector_cursor_pos.x += (5.0f * mState.mScale);
        inspector_cursor_pos.y += (5.0f * mState.mScale);
        ImGui::SetCursorPos(inspector_cursor_pos);

        bool assigned = false;
        std::string assigned_id;
        std::vector<std::string> color_outputs;
        int current_item = 0;
        color_outputs.emplace_back("none");
        int count = 0;
        const SequencePlayerColorOutput *color_output = nullptr;

        for(const auto &output: getEditor().mSequencePlayer->mOutputs)
        {
            if(output.get()->get_type() == RTTI_OF(SequencePlayerColorOutput))
            {
                count++;

                if(output->mID == track.mAssignedOutputID)
                {
                    assigned = true;
                    assigned_id = output->mID;
                    current_item = count;

                    assert(output.get()->get_type() == RTTI_OF(SequencePlayerColorOutput)); // type mismatch
                }

                color_outputs.emplace_back(output->mID);
            }
        }

        ImGui::PushItemWidth(200.0f * mState.mScale);
        if(Combo("", &current_item, color_outputs))
        {
            if(current_item != 0)
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, color_outputs[current_item]);
            else
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, "");

        }
        ImGui::PopItemWidth();
    }


    void SequenceColorTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
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
                        mState.mAction = createAction<InsertColorSegmentPopup>(track.mID, time);
                    }
                }
            }

            // draw line in track while in inserting segment popup
            if(mState.mAction->isAction<InsertColorSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertColorSegmentPopup>();
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

            if(mState.mAction->isAction<InsertColorSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertColorSegmentPopup>();
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
        if(!track.mSegments.empty())
        {
            static std::function<ImU32(const RGBAColorFloat)> to_im_color = [](const RGBAColorFloat& color)
            {
                return IM_COL32(
                        static_cast<int>(color.getRed() * 255.0f),
                        static_cast<int>(color.getGreen() * 255.0f),
                        static_cast<int>(color.getBlue() * 255.0f),
                        static_cast<int>(color.getAlpha()* 255.0f));
            };

            // draw first blend
            auto a = to_im_color(RGBAColorFloat( 0.0f, 0.0f, 0.0f, 0.0f ));
            for(const auto &segment: track.mSegments)
            {
                float segment_x = (float) (segment->mStartTime) * mState.mStepSize;

                auto b = to_im_color(static_cast<const SequenceTrackSegmentColor*>(segment.get())->mColor);
                draw_list->AddRectFilledMultiColor(
                        {trackTopLeft.x + prev_segment_x, trackTopLeft.y},
                        {trackTopLeft.x + segment_x, trackTopLeft.y + track.mTrackHeight * mState.mScale},
                        a, b, b, a);
                a = b;

                prev_segment_x = segment_x;
                segment_count++;
            }

            // draw segment handlers
            for(const auto &segment: track.mSegments)
            {
                float segment_x = (float) (segment->mStartTime) * mState.mStepSize;
                // draw segment handlers
                drawSegmentHandler(
                        track,
                        *(segment.get()),
                        trackTopLeft, segment_x,
                        0.0f, draw_list);
            }
        }

    }


    void SequenceColorTrackView::handleInsertColorSegmentPopup()
    {
        auto* action = mState.mAction->getDerived<InsertColorSegmentPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Insert Color");
            action->mOpened = true;
        }

        // handle insert segment popup
        if(ImGui::BeginPopup("Insert Color"))
        {
            ImGui::ColorEdit4("Color", &action->mValue[0]);

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert)))
            {
                // take snapshot
                getEditor().takeSnapshot(action->get_type());

                // call function to controller
                auto &controller = getEditor().getController<SequenceControllerColor>();
                controller.insertColorSegment(action->mTrackID, action->mTime, action->mValue);

                // exit popup
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
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


    void SequenceColorTrackView::drawSegmentHandler(
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
                    if(!mState.mClipboard->isClipboard<ColorSegmentClipboard>())
                        mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(SequenceTrackColor), getEditor().mSequencePlayer->getSequenceFilename());
                    else if(mState.mClipboard->getDerived<ColorSegmentClipboard>()->getSequenceName() !=
                            getEditor().mSequencePlayer->getSequenceFilename())
                        mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(SequenceTrackColor), getEditor().mSequencePlayer->getSequenceFilename());

                    // get derived clipboard
                    auto *clipboard = mState.mClipboard->getDerived<ColorSegmentClipboard>();

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
            if(mState.mClipboard->isClipboard<ColorSegmentClipboard>())
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


    void SequenceColorTrackView::handleEditSegmentValuePopup()
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

            bool display_copy = !mState.mClipboard->isClipboard<ColorSegmentClipboard>();

            // if clipboard is from different sequence, create new clipboard, so display copy
            if(!display_copy)
            {
                if(mState.mClipboard->getDerived<ColorSegmentClipboard>()->getSequenceName() !=
                   getEditor().mSequencePlayer->getSequenceFilename())
                    mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(SequenceTrackColor), getEditor().mSequencePlayer->getSequenceFilename());
            }

            if(display_copy)
            {
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::copy)))
                {
                    // create clipboard
                    mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(SequenceTrackColor), getEditor().mSequencePlayer->getSequenceFilename());

                    // serialize segment
                    utility::ErrorState errorState;
                    auto &controller = getEditor().getController<SequenceControllerColor>();
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
                auto *clipboard = mState.mClipboard->getDerived<ColorSegmentClipboard>();

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
                        auto &controller = getEditor().getController<SequenceControllerColor>();

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

                auto &controller = getEditor().getController<SequenceControllerColor>();
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
                        auto &color_controller = getEditor().getController<SequenceControllerColor>();
                        const auto *color_segment = dynamic_cast<const SequenceTrackSegmentColor *>(color_controller.getSegment(action->mTrackID, action->mSegmentID));

                        assert(color_segment != nullptr);

                        if(color_segment != nullptr)
                        {
                            rttr::type type = color_segment->get_type();

                            // TODO : do something
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


    bool SequenceColorTrackView::pasteEventsFromClipboard(const std::string& trackID, double time, utility::ErrorState& errorState)
    {
        auto *paste_clipboard = mState.mClipboard->getDerived<sequenceguiclipboard::ColorSegmentClipboard>();

        // create vector & object ptr to be filled by de-serialization
        std::vector<std::unique_ptr<rtti::Object>> read_objects;
        SequenceTrackSegmentColor *color_segment = nullptr;

        // continue upon successful de-serialization
        std::vector<SequenceTrackSegmentColor *> deserialized_color_segments = paste_clipboard->deserialize<SequenceTrackSegmentColor>(read_objects, errorState);

        // no errors ? continue
        if(!errorState.hasErrors() && !deserialized_color_segments.empty())
        {
            // sort event segments by start time
            std::sort(deserialized_color_segments.begin(), deserialized_color_segments.end(), [](SequenceTrackSegmentColor *a, SequenceTrackSegmentColor *b)
            {
                return a->mStartTime < b->mStartTime;
            });

            // adjust start times by duration and start from 0.0
            double first_segment_time = 0.0;
            if(!deserialized_color_segments.empty())
            {
                first_segment_time = deserialized_color_segments[0]->mStartTime;
                deserialized_color_segments[0]->mStartTime = 0.0;
            }
            for(int i = 1; i < deserialized_color_segments.size(); i++)
            {
                deserialized_color_segments[i]->mStartTime =
                        deserialized_color_segments[i]->mStartTime - first_segment_time;
            }

            // obtain controller
            auto &controller = getEditor().getController<SequenceControllerColor>();

            // TODO
            /*
            for(const auto *color: deserialized_color_segments)
            {
                mService.invokePasteEvent(event->get_type(), *this, trackID, *event, time);
            }*/
        } else
        {
            if(!errorState.hasErrors())
            {
                if(errorState.check(!deserialized_color_segments.empty(), "No colors de-serialized"))
                    return false;
            }

            return false;
        }

        return true;
    }


    void SequenceColorTrackView::updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID)
    {
        if(mState.mClipboard->isClipboard<ColorSegmentClipboard>())
        {
            if(mState.mClipboard->containsObject(segmentID, getPlayer().getSequenceFilename()))
            {
                mState.mClipboard->removeObject(segmentID);

                auto &controller = getEditor().getController<SequenceControllerColor>();
                const auto *segment = controller.getSegment(trackID, segmentID);

                utility::ErrorState errorState;
                mState.mClipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);
            }
        }
    }


    void SequenceColorTrackView::handleAssignOutputIDToTrack()
    {
        // get action
        auto *action = mState.mAction->getDerived<AssignOutputIDToTrack>();
        assert(action != nullptr);

        // get curve controller
        auto &event_controller = getEditor().getController<SequenceControllerColor>();

        // take snapshot
        getEditor().takeSnapshot(action->get_type());

        // call function to controller
        event_controller.assignNewOutputID(action->mTrackID, action->mOutputID);

        // action is done
        mState.mAction = sequenceguiactions::createAction<None>();
    }


    void SequenceColorTrackView::handleSegmentDrag()
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
            auto &color_controller = editor.getController<SequenceControllerColor>();

            // take snapshot
            if(action->mTakeSnapshot)
            {
                action->mTakeSnapshot = false;
                editor.takeSnapshot(action->get_type());
            }

            // change start time of segment
            color_controller.segmentColorStartTimeChange(action->mTrackID, action->mSegmentID, action->mNewDuration);

            // update segment in clipboard
            updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
        } else
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    void SequenceColorTrackView::handleLoadPresetPopup()
    {
        if(mState.mAction->isAction<LoadPresetPopup>())
        {
            auto *load_action = mState.mAction->getDerived<LoadPresetPopup>();
            auto *controller = getEditor().getControllerWithTrackType(load_action->mTrackType);

            if(controller->get_type().is_derived_from<SequenceControllerColor>())
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
}
