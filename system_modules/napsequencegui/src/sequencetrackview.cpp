/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "sequencetrackview.h"
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"
#include "sequencetracksegmentduration.h"

// External Includes
#include <imgui/imgui.h>
#include <iomanip>
#include <imguiutils.h>

using namespace nap::sequenceguiactions;

namespace nap
{
    SequenceTrackView::SequenceTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state) :
            mView(view), mState(state), mService(view.getService())
    {
        registerActionHandler(RTTI_OF(TrackOptionsPopup), [this]{ handleTrackOptionsPopup(); });
    }


    static bool vector_getter(void* vec, int idx, const char** out_text)
    {
        auto &vector = *static_cast<std::vector<std::string> *>(vec);
        if(idx < 0 || idx >= static_cast<int>(vector.size())){ return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    }


    bool SequenceTrackView::Combo(const char* label, int* currIndex, std::vector<std::string>& values)
    {
        if(values.empty()){ return false; }
        return ImGui::Combo(label, currIndex, vector_getter,
                            static_cast<void *>(&values), values.size());
    }


    bool SequenceTrackView::ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
    {
        if(values.empty()){ return false; }
        return ImGui::ListBox(label, currIndex, vector_getter,
                              static_cast<void *>(&values), values.size());
    }


    std::string SequenceTrackView::formatTimeString(double time)
    {
        int hours = (int) (time / 3600.0f);
        int minutes = (int) (time / 60.0f) % 60;
        int seconds = (int) time % 60;
        int milliseconds = (int) (time * 100.0f) % 100;

        return hours == 0 ? utility::stringFormat("%.02d:%.02d:%.02d", minutes, seconds, milliseconds) :
               utility::stringFormat("%.02d:%.02d:%.02d:%.02d", hours, minutes, seconds, milliseconds);
    }


    void SequenceTrackView::showInspector(const SequenceTrack& track)
    {
        const float track_height = track.mTrackHeight * mState.mScale;

        auto &gui = mService.getGui();

        // begin inspector
        std::ostringstream inspector_id_stream;
        inspector_id_stream << track.mID << "inspector";
        std::string inspector_id = inspector_id_stream.str();

        // manually set the cursor position before drawing new track window
        ImVec2 cursor_pos = ImGui::GetCursorPos();

        // manually set the cursor position before drawing inspector
        ImVec2 inspector_cursor_pos = cursor_pos;

        // draw inspector window
        float x_offset = 5.0f * mState.mScale;
        float y_offset = 0.5f * mState.mScale;

        // Push Style Color Child Border
        ImGui::PushStyleColor(ImGuiCol_Border, mService.getColors().mFro1);

        // Push Style Child Background
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);

        ImGui::SetCursorPos(cursor_pos);
        if(ImGui::BeginChild(inspector_id.c_str(), // id
                             {mState.mInspectorWidth, track_height + y_offset}, // size
                             true, // border
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) // window flags
        {
            // push track id
            ImGui::PushID(track.mID.c_str());

            // obtain drawlist
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            // store window size and position
            const ImVec2 window_pos = ImGui::GetWindowPos();
            const ImVec2 window_size = ImGui::GetWindowSize();

            // push clipping rectangle
            ImGui::PushClipRect(window_pos, {window_pos.x + window_size.x - x_offset, window_pos.y + track_height}, true);

            /**
             * Helper function for offsetting inspector
             */
            static std::function<ImVec2(float)> offset_inspector = [](float offset)
            {
                ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
                inspector_cursor_pos.x += offset;
                inspector_cursor_pos.y += offset;
                ImGui::SetCursorPos(inspector_cursor_pos);

                return inspector_cursor_pos;
            };

            // offset inspector cursor
            inspector_cursor_pos = offset_inspector(x_offset);

            // scale down everything
            float global_scale = 0.25f;
            ImGui::GetStyle().ScaleAllSizes(global_scale);

            // Remove background
            ImVec4 frame_bg = {0.0f, 0.0f, 0.0f, 0.0f};
            ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, frame_bg);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, frame_bg);

            // show name label
            std::string name_copy = track.mName;
            name_copy.resize(32);
            ImGui::PushID("name_label");
            if(ImGui::InputText("", &name_copy[0], 32))
            {
                mState.mAction = createAction<ChangeTrackName>(track.mID, name_copy);
            }
            ImGui::PopID();

            // draw move and delete controls aligned to upper right corner of inspector
            const float upper_right_alignment = ImGui::GetWindowWidth() - 30.0f * mState.mScale;
            ImGui::SameLine(upper_right_alignment);
            float track_options_popup_x = ImGui::GetCursorScreenPos().x; // store x position for track settings popup placement

            if(ImGui::ImageButton(gui.getIcon(icon::settings), "Track Settings"))
            {
                mState.mAction = createAction<TrackOptionsPopup>(track.mID, ImVec2(track_options_popup_x, ImGui::GetCursorScreenPos().y));
            }

            // offset inspector cursor
            inspector_cursor_pos = offset_inspector(x_offset);

            // show inspector content
            showInspectorContent(track);

            ImGui::Spacing();

            // Pop gui style elements
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::GetStyle().ScaleAllSizes(1.0f / global_scale);

            // pop clipping rectangle
            ImGui::PopClipRect();

            // pop id
            ImGui::PopID();
        }
        ImGui::EndChild();

        // Pop background style
        ImGui::PopStyleColor();

        // Pop border style
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(cursor_pos);
    }


    void SequenceTrackView::showTrack(const SequenceTrack& track)
    {
        ImVec2 cursor_pos = ImGui::GetCursorPos();
        float offset = 5.0f * mState.mScale;
        const ImVec2 window_cursor_pos = {cursor_pos.x + offset, cursor_pos.y};

        ImGui::SetCursorPos(window_cursor_pos);

        // begin track
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.0f, 0.0f, 0.0f, 0.0f});
        if(ImGui::BeginChild(
            track.mID.c_str(), // id
            {mState.mTimelineWidth + offset, (track.mTrackHeight * mState.mScale) + (10.0f * mState.mScale)}, // size
            false, // no border
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) // window flags
        {
            // push id
            ImGui::PushID(track.mID.c_str());

            // get window drawlist
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            // get current imgui cursor position
            cursor_pos = ImGui::GetCursorPos();

            // get window position
            ImVec2 window_top_left = ImGui::GetWindowPos();

            // calc beginning of timeline graphic
            ImVec2 track_top_left = {window_top_left.x + cursor_pos.x, window_top_left.y + cursor_pos.y};
            ImVec2 track_bottom_right = {track_top_left.x + mState.mTimelineWidth,
                                         track_top_left.y + (track.mTrackHeight * mState.mScale)};

            // draw background of track
            draw_list->AddRectFilled(track_top_left, track_bottom_right, ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]));

            // draw border of track
            draw_list->AddRect(track_top_left, track_bottom_right, mService.getColors().mFro1);

            // draw timestamp every 100 pixels
            const float timestamp_interval = 100.0f * mState.mScale;
            int steps = mState.mTimelineWidth / timestamp_interval;
            int i = (math::max<int>(mState.mScroll.x - mState.mInspectorWidth + timestamp_interval, 0) /
                     timestamp_interval);
            for(; i < steps; i++)
            {
                if(i == 0) // ignore first timestamp since it will hide window left border
                    continue;

                ImVec2 pos = {track_top_left.x + i * timestamp_interval, track_top_left.y};

                if(pos.x > 0.0f)
                {
                    draw_list->AddLine(pos, {pos.x,
                                             pos.y + (track.mTrackHeight * mState.mScale)}, mService.getColors().mBack);
                    if(pos.x > mState.mWindowPos.x + mState.mScroll.x + mState.mWindowSize.x)
                        break;
                }
            }

            // store time of current mouse position in track
            mState.mMouseCursorTime = (mState.mMousePos.x - track_top_left.x) / mState.mStepSize;

            // show the track content
            showTrackContent(track, track_top_left);

            // extension handler of track height
            const float extension_handler_margin = 10.0f * mState.mScale;
            const float extension_handler_line_thickness = 5.0f * mState.mScale;
            if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringTrackExtensionHandler>())
            {
                // are we hovering bottom of the track ?
                if(ImGui::IsMouseHoveringRect({track_top_left.x, track_bottom_right.y - extension_handler_margin},
                                              {track_bottom_right.x, track_bottom_right.y + extension_handler_margin}))
                {
                    // create hovering action
                    mState.mAction = createAction<HoveringTrackExtensionHandler>(track.mID);

                    // draw line to indicate we are hovering
                    draw_list->AddLine({track_top_left.x, track_bottom_right.y}, {track_bottom_right.x,
                                                                                  track_bottom_right.y}, mService.getColors().mFro1, extension_handler_line_thickness);

                    // if click, continue dragging
                    if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = createAction<DraggingTrackExtensionHandler>(track.mID);
                    }
                } else
                {
                    // are we not hovering this track extension handler
                    if(mState.mAction->isAction<HoveringTrackExtensionHandler>())
                    {
                        // is it the track we are currently drawing ?
                        if(mState.mAction->getDerived<HoveringTrackExtensionHandler>()->mTrackID == track.mID)
                        {
                            // if so, action is no longer applicable
                            mState.mAction = createAction<None>();
                        }
                    }
                }
            } else if(mState.mAction->isAction<DraggingTrackExtensionHandler>())
            {
                auto *action = mState.mAction->getDerived<DraggingTrackExtensionHandler>();
                if(action->mTrackID == track.mID)
                {
                    // draw line to indicate we are dragging
                    draw_list->AddLine({track_top_left.x, track_bottom_right.y}, {track_bottom_right.x,
                                                                                  track_bottom_right.y}, mService.getColors().mFro1, extension_handler_line_thickness);
                }
            }

            // pop id
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::SetCursorPos({cursor_pos.x, cursor_pos.y});
    }


    void SequenceTrackView::handleActions()
    {
        // check if there is a track action
        if(mState.mAction.get()->get_type().is_derived_from<TrackAction>())
        {
            // get derived track action
            assert(mState.mAction.get()->get_type().is_derived_from<TrackAction>());
            const auto *track_action = static_cast<const TrackAction *>(mState.mAction.get());

            // find the track this track action belongs to
            const auto &sequence = getEditor().mSequencePlayer->getSequenceConst();
            for(const auto &track: sequence.mTracks)
            {
                const auto *track_ptr = track.get();
                if(track_ptr->mID == track_action->mTrackID)
                {
                    // does the track type match for this view?
                    rtti::TypeInfo view_type_for_track = mService.getViewTypeForTrackType(track_ptr->get_type());
                    rtti::TypeInfo current_view_type = get_type();

                    if(view_type_for_track == current_view_type)
                    {
                        // handle any action if necessary
                        auto it = mActionHandlers.find(mState.mAction.get()->get_type());
                        if(it != mActionHandlers.end())
                        {
                            it->second();
                        }
                    }

                    break;
                }
            }
        }
    }


    void SequenceTrackView::handleTrackOptionsPopup()
    {
        assert(mState.mAction.get()->get_type().is_derived_from<sequenceguiactions::TrackOptionsPopup>());
        auto* action = static_cast<sequenceguiactions::TrackOptionsPopup*>(mState.mAction.get());

        if(!action->mPopupOpened)
        {
            ImGui::OpenPopup("TrackOptions");
            action->mPopupOpened = true;
            action->mScrollY = mState.mScroll.y; // store scroll, close popup when user begins scrolling
            const float window_offset = 3.0f * mState.mScale;
            ImGui::SetNextWindowPos({ action->mPos.x - window_offset, action->mPos.y } );
        }

        // scale down everything
        float global_scale = 0.25f;
        ImGui::GetStyle().ScaleAllSizes(global_scale);

        // indicate whether we should close the popup at the end of this function
        bool close_popup = false;

        if(ImGui::BeginPopup("TrackOptions"))
        {
            // fetch controller and track
            auto* controller = getEditor().getControllerWithTrackID(action->mTrackID);
            assert(controller!= nullptr);
            auto *track = controller->getTrack(action->mTrackID);
            assert(track!= nullptr);

            // gui is needed to draw necessary icons
            auto &gui = mService.getGui();

            // bool indicating we performed an edit and we should end the action and close the popup
            bool performed_edit = false;

            if(ImGui::ImageButton(gui.getIcon(icon::sequencer::up), "Move track up"))
            {
                controller->moveTrackUp(action->mTrackID);
                performed_edit = true;
            }

            if(ImGui::ImageButton(gui.getIcon(icon::sequencer::down), "Move track down"))
            {
                controller->moveTrackDown(action->mTrackID);
                performed_edit = true;
            }

            if(ImGui::ImageButton(gui.getIcon(icon::subtract), "Minimize"))
            {
                controller->changeTrackHeight(action->mTrackID, track->getMinimumTrackHeight());
                performed_edit = true;
            }

            if(ImGui::ImageButton(gui.getIcon(icon::add), "Extend"))
            {
                controller->changeTrackHeight(action->mTrackID, track->getExtendedTrackHeight());
                performed_edit = true;
            }

            if(ImGui::ImageButton(gui.getIcon(icon::del), "Delete track"))
            {
                controller->deleteTrack(action->mTrackID);
                performed_edit = true;
            }

            // if edit is performed, close popup and end action
            if(performed_edit)
            {
                mState.mDirty = true;
                close_popup = true;
            }

            // close popup on scroll
            close_popup = close_popup || mState.mScroll.y != action->mScrollY;

            ImGui::EndPopup();
        }else
        {
            // click outside popup so cancel action and close popup
            close_popup = true;
        }

        // scale up
        ImGui::GetStyle().ScaleAllSizes(1.0f / global_scale);

        if(close_popup)
        {
            mState.mAction = sequenceguiactions::createAction<None>();
            ImGui::CloseCurrentPopup();
        }
    }


    void SequenceTrackView::showSegmentHandler(const SequenceTrack& track,
                                               const SequenceTrackSegment& segment,
                                               const ImVec2& trackTopLeft,
                                               float segmentX,
                                               ImDrawList* drawList,
                                               bool movable)
    {
        // obtain track height and handler bounds
        const float track_height = track.mTrackHeight * mState.mScale;
        const float handler_bounds = 10.0f * mState.mScale;

        // line thickness depends on actions performed on the segment handler
        bool bold_handler = false;

        // check if window is focused
        if(mState.mIsWindowFocused)
        {
            // check if we are currently hovering this segment
            if(mState.mAction->isAction<HoveringSegment>())
            {
                auto *action = mState.mAction->getDerived<HoveringSegment>();
                if(action->mSegmentID == segment.mID)
                {
                    // hovering this segment, if outside bounds, end action
                    if(!ImGui::IsMouseHoveringRect(
                            {trackTopLeft.x + segmentX - handler_bounds, trackTopLeft.y - handler_bounds},
                            {trackTopLeft.x + segmentX + handler_bounds,
                             trackTopLeft.y + track_height + handler_bounds}))
                    {
                        mState.mAction = createAction<None>();
                    }else
                    {
                        // otherwise, bold handler
                        bold_handler = true;
                    }

                    // if we clicked on segment and hold shift, add segment to clipboard
                    if(ImGui::IsMouseClicked(0))
                    {
                        // add segment to clipboard
                        if(ImGui::GetIO().KeyShift)
                        {
                            onAddSegmentToClipboard(track, segment);
                        }
                    }

                    // if we clicked on handler and not holding shift, start dragging segment
                    if(ImGui::IsMouseDown(0) && !ImGui::GetIO().KeyShift)
                    {
                        // if we are and segment is movable, start dragging segment
                        if(movable)
                        {
                            // if we are holding ctrl, move to next segments on track accordingly
                            bool move_next_segment = ImGui::GetIO().KeyCtrl;

                            // duration Segments work a little bit different since their handlers are at the end
                            // of the segment, so we need to calculate the position accordingly
                            double position = segment.mStartTime;
                            if(segment.get_type().is_derived_from<SequenceTrackSegmentDuration>())
                            {
                                const auto& duration_segment = static_cast<const SequenceTrackSegmentDuration&>(segment);
                                position = duration_segment.mDuration;
                            }

                            // create dragging segment action
                            mState.mAction = createAction<DraggingSegment>(track.mID,
                                                                           segment.mID,
                                                                           position,
                                                                           move_next_segment);

                        }
                    }else if(ImGui::IsMouseDown(1))
                    {
                        // if we are right clicking, open edit segment popup
                        onHandleEditSegment(track, segment);
                    }

                    // show tooltip with time
                    ImGui::BeginTooltip();
                    ImGui::Text(formatTimeString(segment.mStartTime).c_str());
                    ImGui::EndTooltip();
                }
            }else
            {
                // if we are not dragging a segment, check if we are hovering a segment
                if(!mState.mAction->isAction<DraggingSegment>())
                {
                    if(ImGui::IsMouseHoveringRect(
                            {trackTopLeft.x + segmentX - handler_bounds, trackTopLeft.y - handler_bounds},
                            {trackTopLeft.x + segmentX + handler_bounds,
                             trackTopLeft.y + track_height + handler_bounds}))
                    {
                        bold_handler = true;
                        mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);
                    }
                }else
                {
                    // if we are dragging a segment, check if we are dragging this segment
                    auto *action = mState.mAction->getDerived<DraggingSegment>();
                    if(action->mSegmentID == segment.mID)
                    {
                        // if so, bold handler
                        bold_handler = true;

                        // if we are releasing the mouse, end dragging
                        if(ImGui::IsMouseReleased(0))
                        {
                            mState.mAction = createAction<None>();
                        }
                    }
                }
            }
        }

        // draw handler of segment
        onDrawSegmentHandler({trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                             {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                             drawList, // draw list
                             mState.mClipboard->containsObject(segment.mID, getPlayer().mSequenceFileName), // in clipboard
                             bold_handler); // bold
    }


    void SequenceTrackView::onDrawSegmentHandler(ImVec2 top, ImVec2 bottom, ImDrawList *drawList, bool inClipboard, bool bold)
    {
        drawList->AddLine(top,
                          bottom,
                          inClipboard ? mService.getColors().mHigh1 : mService.getColors().mFro4,
                          bold ? 3.0f * mState.mScale : (inClipboard ? 2.0f : 1.0f) * mState.mScale);
    }


    const SequencePlayer &SequenceTrackView::getPlayer(){ return *mView.mEditor.mSequencePlayer.get(); }


    SequenceEditor &SequenceTrackView::getEditor(){ return mView.mEditor; }


    void SequenceTrackView::registerActionHandler(const rttr::type& type, const std::function<void()>& handler)
    {
        // Assert is triggered when element with same key already exists
        auto it = mActionHandlers.emplace(std::make_pair(type, handler));
        assert(it.second);
    }
}
