/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceeditorgui.h"
#include "sequenceguiutils.h"

// External Includes
#include <entity.h>
#include <iomanip>
#include <utility>
#include <nap/modulemanager.h>
#include <nap/logger.h>
#include <imguiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEditorGUI)
        RTTI_PROPERTY("Sequence Editor", &nap::SequenceEditorGUI::mSequenceEditor, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Render Window", &nap::SequenceEditorGUI::mRenderWindow, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Draw Full Window", &nap::SequenceEditorGUI::mDrawFullWindow, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Hide Marker Labels", &nap::SequenceEditorGUI::mHideMarkerLabels, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::sequenceguiactions;
using namespace nap::sequencecurveenums;
using namespace nap::sequenceguiclipboard;

namespace nap
{
    SequenceEditorGUI::SequenceEditorGUI(SequenceGUIService& service) : mService(service)
    {}


    bool SequenceEditorGUI::init(utility::ErrorState& errorState)
    {
        // Initialize base
        if(!Resource::init(errorState))
            return false;

        // Create and initialize view
        mView = std::make_unique<SequenceEditorGUIView>(mService, *mSequenceEditor.get(), mID, mRenderWindow.get(), mDrawFullWindow);
        mView->hideMarkerLabels(mHideMarkerLabels);

        auto &sequence_player = mSequenceEditor->mSequencePlayer;
        sequence_player->sequenceLoaded.connect([this](SequencePlayer& player, std::string sequence_name)
                                                {
                                                    mRenderWindow->setTitle(sequence_name);
                                                });

        // TODO: Dont show name if invalid file was loaded
        mRenderWindow->setTitle(sequence_player->getSequenceFilename());

        return true;
    }


    void SequenceEditorGUI::onDestroy()
    {}


    void SequenceEditorGUI::show(bool newWindow)
    {
        mView->show(newWindow);
    }


    SequenceEditorGUIView::SequenceEditorGUIView(SequenceGUIService& service, SequenceEditor& editor, std::string id, RenderWindow* renderWindow, bool drawFullWindow)
        : mService(service), mEditor(editor), mID(std::move(id)), mRenderWindow(renderWindow), mDrawFullWindow(drawFullWindow)
    {
        // start with empty clipboard and empty action
        mState.mAction = createAction<None>();
        mState.mClipboard = createClipboard<Empty>();

        // call registerActionHandlers to register all actions handled by the SequenceEditorGUIView
        registerActionHandlers();

        // create views for all registered track types
        const auto &track_types = mService.getAllTrackTypes();
        for(const auto &track_type: track_types)
        {
            // get view type
            auto view_type = mService.getViewTypeForTrackType(track_type);

            // create the track view
            auto track_view = mService.invokeTrackViewFactory(view_type, *this, mState);

            // move & store the unique pointer
            mViews.emplace(view_type, std::move(track_view));
        }
    }


    void SequenceEditorGUIView::show(bool newWindow)
    {
        // Store scale
        mState.mScale = mService.getGui().getScale();
        bool reset_dirty_flag = mState.mDirty;
        mState.mInspectorWidth = 350.0f * mState.mScale;
        mState.mMousePos = ImGui::GetMousePos();
        mState.mMouseDelta = ImGui::GetIO().MouseDelta;

        // obtain sequence & sequence player
        const Sequence &sequence = mEditor.mSequencePlayer->getSequenceConst();
        SequencePlayer &sequence_player = *mEditor.mSequencePlayer.get();

        // calc track spacing
        const float track_spacing = 20.0f * mState.mScale;

        // push id
        ImGui::PushID(mID.c_str());

        // Set as amount of pixels per second
        mState.mStepSize = mState.mHorizontalResolution * mState.mScale;

        // calc width of content in timeline window
        mState.mTimelineWidth = (float) (mState.mStepSize * sequence.mDuration);

        // calc total tracksheight
        mState.mTotalTracksHeight = 0.0f;
        for(const auto &track: sequence.mTracks)
        {
            mState.mTotalTracksHeight += track->mTrackHeight * mState.mScale;
        }
        mState.mTotalTracksHeight += track_spacing * sequence.mTracks.size();

        // set content width of next window
        ImGui::SetNextWindowContentSize(ImVec2(mState.mTimelineWidth + mState.mInspectorWidth, 0.0f));

        // set window flags
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
        if(mDrawFullWindow)
            window_flags = window_flags | ImGuiWindowFlags_NoResize;

        // Influences window size handling
        if(mDrawFullWindow)
        {
            ImGui::SetNextWindowPos({0, 0});
            ImGui::SetNextWindowSize
                (
                    {static_cast<float>(mRenderWindow->getBufferSize().x),
                     static_cast<float>(mRenderWindow->getBufferSize().y)}
                );
        }

        bool visible = false;
        if(newWindow)
        {
            visible = ImGui::Begin(mID.c_str(), // id
                                   (bool *) 0, // open
                                   window_flags); // window flags;
        } else
        {
            visible = ImGui::BeginChild(mID.c_str(), // id
                                        {0, 0}, // size
                                        (bool *) 0, // open
                                        window_flags); // window flags;
        }

        // begin window
        if(visible)
        {
            // handle hotkeys
            handleHotKeys();

            // if the sequence has changed, mark the state dirty so caches can get cleared
            if(mState.mSequenceName!=sequence_player.getSequenceFilename())
            {
                mState.mSequenceName=sequence_player.getSequenceFilename();
                mState.mDirty = true;
            }

            // check if window is resized
            ImVec2 windowSize = ImGui::GetWindowSize();
            if(windowSize.x != mState.mWindowSize.x || windowSize.y != mState.mWindowSize.y)
            {
                mState.mDirty = true;
                mState.mWindowSize = windowSize;
            }

            // we want to know if this window is focused in order to handle mouseinput
            // in child windows or not
            mState.mIsWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            // clear curve cache if we move the window
            mState.mWindowPos = ImGui::GetWindowPos();
            if(mState.mWindowPos.x != mState.mPrevWindowPos.x ||
               mState.mWindowPos.y != mState.mPrevWindowPos.y)
            {
                mState.mDirty = true;
            }
            mState.mPrevWindowPos = mState.mWindowPos;

            // clear curve cache if we scroll inside the window
            ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            if(scroll.x != mState.mScroll.x || scroll.y != mState.mScroll.y)
            {
                mState.mDirty = true;
            }
            mState.mScroll = scroll;

            // move controls according to scroll, so they will alway be visible
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetScrollY());

            IMGuiService &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::save), "Save sequence"))
            {
                mEditor.save(mEditor.mSequencePlayer->mSequenceFileName);
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::saveAs), "Save sequence as ..."))
            {
                ImGui::OpenPopup("Save As");
                mState.mAction = createAction<SaveAsPopup>();
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::load), "Load sequence"))
            {
                ImGui::OpenPopup("Load");
                mState.mAction = createAction<LoadPopup>();
            }

            ImGui::SameLine();
            if(sequence_player.getIsPlaying())
            {
                if(ImGui::ImageButton(gui.getIcon(icon::sequencer::stop), "Stop playback"))
                {
                    sequence_player.setIsPlaying(false);
                }
            } else
            {
                if(ImGui::ImageButton(gui.getIcon(icon::sequencer::play), "Start playback"))
                {
                    sequence_player.setIsPlaying(true);
                }
            }

            ImGui::SameLine();
            if(sequence_player.getIsPaused())
            {
                if(ImGui::ImageButton(gui.getIcon(icon::sequencer::unpause), "Un-Pause playback"))
                {
                    sequence_player.setIsPaused(false);
                }
            } else
            {
                if(ImGui::ImageButton(gui.getIcon(icon::sequencer::pause), "Pause playback"))
                {
                    sequence_player.setIsPaused(true);
                    sequence_player.setIsPaused(true);
                }
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::sequencer::rewind), "Move marker to beginning"))
            {
                sequence_player.setPlayerTime(0.0);
            }

            ImGui::SameLine();
            bool isLooping = sequence_player.getIsLooping();
            if(ImGui::Checkbox("Loop", &isLooping))
            {
                sequence_player.setIsLooping(isLooping);
            }

            ImGui::SameLine();
            float playback_speed = sequence_player.getPlaybackSpeed();

            float item_width = 50.0f * mState.mScale;
            ImGui::PushItemWidth(item_width);
            if(ImGui::DragFloat("speed", &playback_speed, 0.01f, -10.0f, 10.0f, "%.1f"))
            {
                playback_speed = math::clamp(playback_speed, -10.0f, 10.0f);
                sequence_player.setPlaybackSpeed(playback_speed);
            }
            ImGui::PopItemWidth();

            ImGui::SameLine();

            if(ImGui::Checkbox("Follow", &mState.mFollow))
            {
            }

            if(mState.mFollow)
            {
                float scroll_x =
                    (float) (sequence_player.getPlayerTime() / sequence_player.getDuration()) * mState.mTimelineWidth;
                ImGui::SetScrollX(scroll_x);
            }

            ImGui::SameLine();

            if(mState.mClipboard->getObjectCount() > 0)
            {
                if(ImGui::Button("Save Preset"))
                {
                    mState.mAction = createAction<SavePresetPopup>("segments");
                }
            }

            ImGui::SameLine();

            // only show undo/redo when editor contains snapshots
            if(mEditor.getHistorySize() > 0)
            {
                // if history index has reached the limit, don't show undo option
                if(mEditor.getHistoryIndex() > 0)
                {
                    if(ImGui::ImageButton(gui.getIcon(icon::sequencer::undo), "undo"))
                    {
                        mState.mAction = createAction<PerformUndo>();
                    }
                }

                // if history index is not at at the most recent history, show redo
                ImGui::SameLine();
                if(mEditor.getHistoryIndex() < mEditor.getHistorySize() - 1)
                {
                    if(ImGui::ImageButton(gui.getIcon(icon::sequencer::redo), "redo"))
                    {
                        mState.mAction = createAction<PerformRedo>();
                    }
                    ImGui::SameLine();
                }

                //
                if(ImGui::ImageButton(gui.getIcon(icon::sequencer::history), "History"))
                {
                    mState.mAction = createAction<OpenHistoryPopup>();
                }
                ImGui::SameLine();
            }

            ImGui::SameLine();

            if(ImGui::ImageButton(gui.getIcon(icon::help)))
            {
                mState.mAction = createAction<HelpPopup>();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // allow mouse zoom-in with mouse wheel and ctrl
            if(ImGui::GetIO().KeyCtrl)
            {
                float scroll = ImGui::GetIO().MouseWheel;
                if(scroll != 0.0f)
                {
                    float new_resolution = mState.mHorizontalResolution + ImGui::GetIO().MouseWheel * 5.0f;
                    new_resolution = math::max<float>(new_resolution, 2.5f);
                    mState.mAction = createAction<ChangeHorizontalResolution>(new_resolution);
                }
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());

            item_width = 200.0f * mState.mScale;
            ImGui::PushItemWidth(item_width);
            float horizontal_resolution = mState.mHorizontalResolution;
            if(ImGui::SliderFloat("Horizontal Zoom", &horizontal_resolution, 2.5, 500, ""))
            {
                if(mState.mAction->isAction<None>() || mState.mAction->isAction<NonePressed>())
                {
                    float new_resolution = horizontal_resolution;
                    new_resolution = math::max<float>(new_resolution, 2.5f);
                    mState.mAction = createAction<ChangeHorizontalResolution>(new_resolution);
                }
            }

            ImGui::PopItemWidth();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetScrollY());

            // store position of next window (player controller), we need it later to draw the timelineplayer position
            mState.mTimelineControllerPos = ImGui::GetCursorPos();

            float top_size = 70.0f * mState.mScale; // area of markers and player controller combined
            float clip_start_y = ImGui::GetWindowPos().y + ImGui::GetCursorPosY() +
                                 top_size; // clipping area starts at current cursor position plus top size, which is the area of comments, playercontroller and error messages
            float end_clip_y = ImGui::GetWindowPos().y + ImGui::GetWindowHeight() -
                               10.0f * mState.mScale; // bottom scrollbar overlaps clipping area
            float end_clip_x = ImGui::GetWindowPos().x + ImGui::GetWindowWidth() -
                               10.0f * mState.mScale; // right scrollbar overlaps clipping area

            // get total height
            float total_height = 0;
            for(const auto &track: sequence.mTracks)
            {
                total_height += track->mTrackHeight * mState.mScale + track_spacing;
            }

            // timeline window properties
            ImVec2 timeline_window_pos =
                {
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + mState.mInspectorWidth - mState.mScroll.x,
                    ImGui::GetCursorPos().y + ImGui::GetWindowPos().y - mState.mScroll.y
                };
            ImVec2 timeline_window_size =
                {
                    mState.mTimelineWidth + (50.0f * mState.mScale),
                    total_height + top_size
                };

            // inspector window properties
            ImVec2 inspector_window_pos =
                {
                    ImGui::GetCursorPos().x + ImGui::GetWindowPos().x,
                    ImGui::GetCursorPos().y + ImGui::GetWindowPos().y - mState.mScroll.y
                };
            ImVec2 inspector_window_size =
                {
                    mState.mInspectorWidth,
                    timeline_window_size.y
                };

            // setup up position for next window, which contains all tracks
            ImGui::SetNextWindowPos(timeline_window_pos);

            // begin timeline window
            if(ImGui::BeginChild(std::string(mID + "_timeline_window").c_str(),
                                 timeline_window_size,
                                 false,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoMouseInputs))
            {
                /**
                 * First the player controller and markers are drawn, after that, the tracks are drawn below the player controller
                 */
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mState.mScroll.y);

                ImVec2 clip_rect_min_top = {inspector_window_pos.x + inspector_window_size.x,
                                            timeline_window_pos.y > clip_start_y - top_size ? timeline_window_pos.y :
                                            clip_start_y - top_size};
                ImVec2 clip_rect_max_top = {
                    timeline_window_pos.x + timeline_window_size.x < end_clip_x ? timeline_window_pos.x +
                                                                                  timeline_window_size.x : end_clip_x,
                    timeline_window_pos.y + timeline_window_size.y < end_clip_y ? timeline_window_pos.y +
                                                                                  timeline_window_size.y : end_clip_y};

                ImGui::PushClipRect(clip_rect_min_top, clip_rect_max_top, false);

                // draw markers
                drawMarkers(sequence_player, sequence);

                // draw player controller
                drawPlayerController(sequence_player);

                ImGui::PopClipRect();

                // move the cursor below player and marker area
                ImGui::SetCursorPosY(top_size);

                // calc clipping rectangle
                ImVec2 clip_rect_min_tracks = {inspector_window_pos.x + inspector_window_size.x,
                                               timeline_window_pos.y > clip_start_y ? timeline_window_pos.y
                                                                                    : clip_start_y};
                ImVec2 clip_rect_max_tracks = {
                    timeline_window_pos.x + timeline_window_size.x < end_clip_x ? timeline_window_pos.x +
                                                                                  timeline_window_size.x : end_clip_x,
                    timeline_window_pos.y + timeline_window_size.y < end_clip_y ? timeline_window_pos.y +
                                                                                  timeline_window_size.y : end_clip_y};


                // push it
                ImGui::PushClipRect(clip_rect_min_tracks, clip_rect_max_tracks, false);

                // draw tracks
                drawTracks(sequence_player, sequence);

                // pop clip rect, we slightly adjust it in Y start when drawing markers and playerposition line
                ImGui::PopClipRect();

                const float player_position_y_offset = 5.0f * mState.mScale;
                ImGui::PushClipRect({clip_rect_min_tracks.x,
                                     clip_rect_min_tracks.y - player_position_y_offset}, clip_rect_max_tracks, false);

                // draw time line position line
                drawTimelinePlayerPosition(sequence, sequence_player);

                ImGui::PopClipRect();

                const float marker_lines_y_offset = 50.0f * mState.mScale;
                ImGui::PushClipRect({clip_rect_min_tracks.x,
                                     clip_rect_min_tracks.y - marker_lines_y_offset}, clip_rect_max_tracks, false);

                // draw marker lines
                drawMarkerLines(sequence, sequence_player);

                ImGui::PopClipRect();
            }
            ImGui::EndChild();

            // reset cursor
            ImGui::SetCursorPos(mState.mTimelineControllerPos);

            // setup up position for next window, which contains all inspectors
            // inspectors will be draw on top of tracks
            ImGui::SetNextWindowPos(inspector_window_pos);

            if(ImGui::BeginChild(std::string(mID + "_timeline_inspectors").c_str(),
                                 inspector_window_size,
                                 false,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                                 ImGuiWindowFlags_NoMouseInputs))
            {
                const float inspector_clip_start_y = clip_start_y - 25.0f * mState.mScale;
                ImGui::PushClipRect({inspector_window_pos.x,
                                     inspector_window_pos.y > inspector_clip_start_y ? inspector_window_pos.y
                                                                                     : inspector_clip_start_y},
                                    {inspector_window_pos.x + inspector_window_size.x < end_clip_x ?
                                     inspector_window_pos.x + inspector_window_size.x : end_clip_x,
                                     inspector_window_pos.y + inspector_window_size.y < end_clip_y ?
                                     inspector_window_pos.y + inspector_window_size.y : end_clip_y}, false);

                // align inspectors with track views
                ImGui::SetCursorPosY(top_size);

                drawInspectors(sequence_player, sequence);

                ImGui::PopClipRect();
            }
            ImGui::EndChild();

            // move the cursor below the tracks
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + mState.mScroll.x);
            if(ImGui::ImageButton(gui.getIcon(icon::add)))
            {
                mState.mAction = createAction<InsertTrackPopup>();
            }
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + mState.mScroll.x);
            ImGui::Text("Add Track");

            // handle actions
            for(auto &it: mViews)
            {
                it.second->handleActions();
            }

            // handle actions for popups
            auto it = mActionHandlers.find(mState.mAction.get()->get_type());
            if(it != mActionHandlers.end())
            {
                it->second();
            }
        }

        // pop id
        if(newWindow)
            ImGui::End();
        else
            ImGui::EndChild();
        ImGui::PopID();

        if(reset_dirty_flag)
        {
            mState.mDirty = false;
        }
    }


    void SequenceEditorGUIView::drawTracks(const SequencePlayer& sequencePlayer, const Sequence& sequence)
    {
        // define consts
        const float track_spacing = 20.0f * mState.mScale;

        // get cursor pos
        auto cursor_pos = ImGui::GetCursorPos();

        // draw tracks
        for(int i = 0; i < sequence.mTracks.size(); i++)
        {
            const auto &track = sequence.mTracks[i];
            float height = 0;
            if(i > 0)
            {
                for(int j = 0; j < i; j++)
                {
                    height += sequence.mTracks[j]->mTrackHeight * mState.mScale + track_spacing;
                }
            }
            ImGui::SetCursorPos({cursor_pos.x, cursor_pos.y + height});
            mState.mCursorPos = ImGui::GetCursorPos();

            auto track_type = sequence.mTracks[i].get()->get_type();
            auto view_type = mService.getViewTypeForTrackType(track_type);

            auto it = mViews.find(view_type);
            assert(it != mViews.end()); // no view class created for this view type
            it->second->showTrack(*sequence.mTracks[i].get());
        }
    }


    void SequenceEditorGUIView::drawInspectors(const SequencePlayer& sequencePlayer, const Sequence& sequence)
    {
        // define consts
        const float track_spacing = 20.0f * mState.mScale;

        // get cursor pos
        auto cursor_pos = ImGui::GetCursorPos();

        // draw tracks
        for(int i = 0; i < sequence.mTracks.size(); i++)
        {
            const auto &track = sequence.mTracks[i];
            float height = 0;
            if(i > 0)
            {
                for(int j = 0; j < i; j++)
                {
                    height += sequence.mTracks[j]->mTrackHeight * mState.mScale + track_spacing;
                }
            }
            ImGui::SetCursorPos({cursor_pos.x, cursor_pos.y + height});
            mState.mCursorPos = ImGui::GetCursorPos();

            auto track_type = sequence.mTracks[i].get()->get_type();
            auto view_type = mService.getViewTypeForTrackType(track_type);

            auto it = mViews.find(view_type);
            assert(it != mViews.end()); // no view class created for this view type

            it->second->showInspector(*sequence.mTracks[i].get());
        }
    }


    void SequenceEditorGUIView::drawMarkers(const SequencePlayer& sequencePlayer, const Sequence& sequence)
    {
        const float sequence_controller_height = 30.0f * mState.mScale;

        std::ostringstream string_stream;
        string_stream << mID << "markers";
        std::string id_string = string_stream.str();

        const float marker_offset = 5.0f * mState.mScale;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + marker_offset);
        ImGui::PushID(id_string.c_str());

        // draw timeline controller
        if(ImGui::BeginChild(id_string.c_str(), // id
                             {mState.mTimelineWidth, sequence_controller_height}, // size
                             false, // no border
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_ChildWindow)) // window flags
        {
            // get current window and cursor position
            ImVec2 cursor_pos = ImGui::GetCursorPos();
            ImVec2 window_top_left = ImGui::GetWindowPos();

            // get start position for drawing markers
            ImVec2 start_pos = {
                window_top_left.x + cursor_pos.x,
                window_top_left.y + cursor_pos.y,
            };

            // get window drawlist
            cursor_pos.y += marker_offset;
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            // draw marker background
            draw_list->AddRectFilled(window_top_left,
                                     {window_top_left.x + ImGui::GetWindowWidth(),
                                      window_top_left.y + ImGui::GetWindowHeight()},
                                     ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]));

            // store clip rect
            const ImVec2 cliprect_min_cached = ImGui::GetWindowDrawList()->GetClipRectMin();
            const ImVec2 cliprect_max_cached = ImGui::GetWindowDrawList()->GetClipRectMax();

            // draw markers
            for(const auto &marker: sequence.mMarkers)
            {
                double marker_pos = marker->mTime;

                // draw handler of player position

                // top left of circle hit area
                const float player_time_top_rect_left_x_offset = -10.0f * mState.mScale;
                const ImVec2 player_time_top_rect_left =
                    {
                        (start_pos.x + (float) (marker_pos / sequencePlayer.getDuration()) * mState.mTimelineWidth) +
                        player_time_top_rect_left_x_offset,
                        start_pos.y + marker_offset
                    };

                // bottom right of circle hit area
                const ImVec2 player_time_bottom_right_offset = {10.0f * mState.mScale, 25.0f * mState.mScale};
                const ImVec2 player_time_rect_bottom_right =
                    {
                        (start_pos.x + (float) (marker_pos / sequencePlayer.getDuration()) * mState.mTimelineWidth) +
                        player_time_bottom_right_offset.x,
                        start_pos.y + player_time_bottom_right_offset.y,
                    };

                // center position of circle
                const ImVec2 player_time_rect_center =
                    {
                        (player_time_top_rect_left.x + player_time_rect_bottom_right.x) * 0.5f,
                        (player_time_top_rect_left.y + player_time_rect_bottom_right.y) * 0.5f
                    };

                // is hit area hovered ?
                bool hovered = false;
                if(ImGui::IsMouseHoveringRect(player_time_top_rect_left, player_time_rect_bottom_right))
                    hovered = true;

                // is marker line hit area hovered ?
                bool show_marker_label = mHideMarkerLabels ? hovered : true;
                if(!show_marker_label)
                {
                    const float hover_margin = 8.0f;

                    // if player position is inside the sequencer window, draw it
                    if(player_time_rect_center.x < mState.mWindowPos.x + mState.mWindowSize.x - 15.0f * mState.mScale &&
                       player_time_rect_center.x > mState.mWindowPos.x)
                    {
                        ImGui::PopClipRect();
                        const float bottom = player_time_rect_center.y + math::min<float>(
                            mState.mTotalTracksHeight + 55.0f * mState.mScale,
                            mState.mScroll.y + mState.mWindowSize.y - mState.mTimelineControllerPos.y);
                        if(ImGui::IsMouseHoveringRect(
                            {player_time_rect_center.x - hover_margin, player_time_rect_center.y - hover_margin},
                            {player_time_rect_center.x + hover_margin, bottom + hover_margin}))
                        {
                            show_marker_label = true;
                        }
                        ImGui::PushClipRect(cliprect_min_cached, cliprect_max_cached, false);
                    }
                }

                // draw text
                if(show_marker_label)
                {
                    const ImVec2 text_offset = {5.0f * mState.mScale, -10.0f * mState.mScale};
                    draw_list->AddText({player_time_rect_bottom_right.x + text_offset.x,
                                        player_time_rect_center.y + text_offset.y}, // position
                                       mService.getColors().mFro4, // color
                                       marker->mMessage.c_str()); // text
                }

                // no action and are we hovering the hit action ?
                if(mState.mAction->isAction<None>() && hovered)
                {
                    // start dragging marker if mouse 1 is down, otherwise open edit popup
                    if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = createAction<DragSequenceMarker>(marker->mID);
                    } else if(ImGui::IsMouseDown(1))
                    {
                        mState.mAction = createAction<EditSequenceMarkerPopup>(marker->mID, marker->mMessage, marker->mTime);
                    }
                }

                // are we dragging a marker ?
                if(mState.mAction->isAction<DragSequenceMarker>())
                {
                    // is it this marker ?
                    auto *action = mState.mAction->getDerived<DragSequenceMarker>();
                    if(action->mID == marker->mID)
                    {
                        // continue dragging
                        if(ImGui::IsMouseDown(0))
                        {
                            double time = ((ImGui::GetMousePos().x - window_top_left.x) / mState.mTimelineWidth) *
                                          sequencePlayer.getDuration();
                            mEditor.changeMarkerTime(marker->mID, time);

                            hovered = true;
                        } else
                        {
                            // release
                            mState.mAction = createAction<None>();
                        }
                    }
                }

                float radius = 10.0f * mState.mScale;
                const float circle_thickness = 2.0f * mState.mScale;
                int segments = static_cast<int>(12.0f * mState.mScale);
                if(hovered)
                    draw_list->AddCircleFilled(player_time_rect_center, radius, mService.getColors().mFro4, segments);
                else
                    draw_list->AddCircle(player_time_rect_center, radius, mService.getColors().mFro3, segments, circle_thickness);
            }

            if(mState.mAction->isAction<None>())
            {
                if(ImGui::IsMouseHoveringRect(window_top_left, {window_top_left.x + ImGui::GetWindowWidth(),
                                                                window_top_left.y + ImGui::GetWindowHeight()}))
                {
                    if(ImGui::IsMouseDown(1))
                    {
                        double time = ((ImGui::GetMousePos().x - window_top_left.x) / mState.mTimelineWidth) *
                                      sequencePlayer.getDuration();
                        mState.mAction = createAction<InsertingSequenceMarkerPopup>(time);
                    }
                }
            }
        }

        ImGui::EndChild();

        ImGui::PopID();
    }


    void SequenceEditorGUIView::drawPlayerController(SequencePlayer& player)
    {
        const float sequence_controller_height = 30.0f * mState.mScale;

        std::ostringstream string_stream;
        string_stream << mID << "sequencecontroller";
        std::string id_string = string_stream.str();

        const float player_controller_offset = 5.0f * mState.mScale;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + player_controller_offset);
        ImGui::PushID(id_string.c_str());

        // used for culling ( is stuff inside the parent window ??? )
        ImVec2 parent_window_size = mState.mWindowSize;

        // draw timeline controller
        if(ImGui::BeginChild(id_string.c_str(), // id
                             {mState.mTimelineWidth + player_controller_offset, sequence_controller_height}, // size
                             false, // no border
                             ImGuiWindowFlags_NoMove)) // window flags
        {
            ImVec2 cursor_pos = ImGui::GetCursorPos();
            ImVec2 window_top_left = ImGui::GetWindowPos();

            ImVec2 start_pos_offset = {0.0f, 15.0f * mState.mScale};
            ImVec2 start_pos =
                {
                    window_top_left.x + cursor_pos.x + start_pos_offset.x,
                    window_top_left.y + cursor_pos.y + start_pos_offset.y,
                };

            cursor_pos.y += player_controller_offset;

            // get window drawlist
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            ImGui::PushStyleColor(ImGuiCol_WindowBg, mService.getColors().mBack);

            // draw backgroundbox of controller
            const ImVec2 background_box_bottom_right = {start_pos.x + mState.mTimelineWidth,
                                                        start_pos.y + sequence_controller_height -
                                                        15.0f * mState.mScale};

            // draw box of controller
            draw_list->AddRect(start_pos, background_box_bottom_right, mService.getColors().mFro1);

            ImGui::PopStyleColor();

            // draw handler of player position
            const double player_time = player.getPlayerTime();
            const float player_handler_half_width = player_controller_offset;
            const ImVec2 player_time_top_rect_left =
                {
                    start_pos.x + (float) (player_time / player.getDuration()) * mState.mTimelineWidth -
                    player_handler_half_width, start_pos.y
                };
            const ImVec2 player_time_rect_bottom_right =
                {
                    start_pos.x + (float) (player_time / player.getDuration()) * mState.mTimelineWidth +
                    player_handler_half_width,
                    start_pos.y + sequence_controller_height,
                };

            // draw box
            draw_list->AddRectFilled(player_time_top_rect_left, player_time_rect_bottom_right, mService.getColors().mHigh1);

            // define consts
            const float timestamp_line_height = 18.0f * mState.mScale;
            const float timestamp_line_end_offset = 2.0f * mState.mScale;

            // draw timestamp text every ~100 pixels
            const float timestamp_interval = 100.0f * mState.mScale;
            int steps = (int) (mState.mTimelineWidth / timestamp_interval);
            int step_start = (int) (math::max<float>(mState.mScroll.x - start_pos.x, start_pos.x) /
                                    mState.mTimelineWidth);
            for(int i = step_start; i < steps; i++)
            {
                ImVec2 timestamp_pos =
                    {
                        (float) i * timestamp_interval + start_pos.x,
                        start_pos.y - timestamp_line_height
                    };

                if(timestamp_pos.x < parent_window_size.x &&
                   timestamp_pos.x >= 0)
                {
                    if(timestamp_pos.y >= 0 &&
                       timestamp_pos.y < parent_window_size.y)
                    {
                        double time_in_player = ((float) i * timestamp_interval) / mState.mStepSize;
                        std::string formatted_time_string = SequenceTrackView::formatTimeString(time_in_player);
                        draw_list->AddText(timestamp_pos, mService.getColors().mFro3, formatted_time_string.c_str());

                        if(i != 0)
                        {
                            draw_list->AddLine({timestamp_pos.x, timestamp_pos.y + timestamp_line_height},
                                               {timestamp_pos.x, timestamp_pos.y + sequence_controller_height +
                                                                 timestamp_line_end_offset},
                                               mService.getColors().mFro1,
                                               1.0f * mState.mScale);
                        }
                    }
                } else
                {
                    if(timestamp_pos.x > parent_window_size.x)
                        break; // right side of window, break out of loop
                }
            }

            // handle hovering and dragging of player position
            if(mState.mIsWindowFocused)
            {
                if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringPlayerTime>())
                {
                    if(ImGui::IsMouseHoveringRect(start_pos, {start_pos.x + mState.mTimelineWidth,
                                                              start_pos.y + sequence_controller_height}))
                    {
                        mState.mAction = createAction<HoveringPlayerTime>();

                        if(ImGui::IsMouseDown(0))
                        {
                            //
                            bool player_was_playing = player.getIsPlaying();
                            bool player_was_paused = player.getIsPaused();

                            mState.mAction = createAction<DraggingPlayerTime>(player_was_playing, player_was_paused);
                            if(player_was_playing)
                            {
                                player.setIsPaused(true);
                            }

                            // snap to mouse position
                            double time =
                                ((ImGui::GetMousePos().x - start_pos.x) / mState.mTimelineWidth) * player.getDuration();
                            player.setPlayerTime(time);
                        }
                    } else
                    {
                        mState.mAction = createAction<None>();
                    }
                } else if(mState.mAction->isAction<DraggingPlayerTime>())
                {
                    if(ImGui::IsMouseDown(0))
                    {
                        double delta = (mState.mMouseDelta.x / mState.mTimelineWidth) * player.getDuration();
                        player.setPlayerTime(player_time + delta);
                    } else
                    {
                        if(ImGui::IsMouseReleased(0))
                        {
                            const auto *drag_action = mState.mAction->getDerived<DraggingPlayerTime>();
                            assert(drag_action != nullptr);
                            if(drag_action->mWasPlaying && !drag_action->mWasPaused)
                            {
                                player.setIsPlaying(true);
                            }

                            mState.mAction = createAction<None>();
                        }
                    }
                }
            }

            // handle dragging of timeline duration
            const double player_duration = player.getDuration();

            const ImVec2 player_duration_size =
                {
                    5.0f * mState.mScale,
                    15.0f * mState.mScale
                };
            const ImVec2 player_duration_top_rect_left =
                {
                    start_pos.x + mState.mTimelineWidth - player_duration_size.x,
                    start_pos.y - player_duration_size.y
                };
            const ImVec2 player_duration_rect_bottom_right =
                {
                    start_pos.x + mState.mTimelineWidth + player_duration_size.x,
                    start_pos.y,
                };

            bool draw_filled = false;
            if(mState.mIsWindowFocused)
            {
                if(mState.mAction->isAction<None>())
                {
                    if(ImGui::IsMouseHoveringRect(player_duration_top_rect_left, player_duration_rect_bottom_right))
                    {
                        draw_filled = true;
                        mState.mAction = createAction<HoveringSequenceDuration>();
                    }
                } else if(mState.mAction->isAction<HoveringSequenceDuration>())
                {
                    if(ImGui::IsMouseHoveringRect(player_duration_top_rect_left, player_duration_rect_bottom_right))
                    {
                        draw_filled = true;

                        if(ImGui::IsMouseDown(0))
                        {
                            mState.mAction = createAction<DraggingSequenceDuration>();
                        }

                        if(ImGui::IsMouseDown(1))
                        {
                            mState.mAction = createAction<EditSequenceDurationPopup>();
                        }
                    } else
                    {
                        mState.mAction = createAction<None>();
                    }
                } else if(mState.mAction->isAction<DraggingSequenceDuration>())
                {
                    if(ImGui::IsMouseDown(0))
                    {
                        draw_filled = true;
                        float amount = mState.mMouseDelta.x / (mState.mHorizontalResolution * mState.mScale);
                        double new_duration = player_duration + amount;
                        mEditor.changeSequenceDuration(new_duration);
                    } else
                    {
                        mState.mAction = createAction<None>();
                    }

                }
            }

            if(!draw_filled)
            {
                draw_list->AddRect(player_duration_top_rect_left, player_duration_rect_bottom_right, mService.getColors().mFro3);
            } else
            {
                draw_list->AddRectFilled(player_duration_top_rect_left, player_duration_rect_bottom_right, mService.getColors().mFro3);
            }
        }

        ImGui::EndChild();

        ImGui::PopID();
    }


    void SequenceEditorGUIView::drawTimelinePlayerPosition(const Sequence& sequence, SequencePlayer& player) const
    {
        const float line_thickness = 2.0f * mState.mScale;
        const ImVec2 player_offset = {5.0f * mState.mScale, 50.0f * mState.mScale};
        const float line_x_offset = 25.0f * mState.mScale;
        const float line_y_end = mState.mTotalTracksHeight;

        ImVec2 pos =
            {
                mState.mWindowPos.x + mState.mTimelineControllerPos.x - mState.mScroll.x
                + mState.mInspectorWidth + player_offset.x
                + mState.mTimelineWidth * (float) (player.getPlayerTime() / player.getDuration()) - 1,
                mState.mWindowPos.y + mState.mTimelineControllerPos.y + player_offset.y - mState.mScroll.y
            };

        // if player position in inside the sequencer window, draw it
        if(pos.x < mState.mWindowPos.x + mState.mWindowSize.x && pos.x > mState.mWindowPos.x)
        {
            ImVec2 line_begin = {pos.x, math::max<float>(
                mState.mWindowPos.y + line_x_offset, pos.y)}; // clip line to top of window
            ImVec2 line_end = {pos.x, pos.y + math::min<float>(line_y_end, mState.mScroll.y + mState.mWindowSize.y -
                                                                           mState.mTimelineControllerPos.y)}; // clip the line to bottom of window

            ImGui::SetNextWindowPos(line_begin);
            if(ImGui::BeginChild("PlayerPosition", {line_thickness, line_end.y - line_begin.y}, false,
                                 ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove))
            {
                auto *drawlist = ImGui::GetWindowDrawList();
                drawlist->AddLine(line_begin, line_end, mService.getColors().mHigh1, line_thickness);
            }
            ImGui::EndChild();

        }
    }


    void SequenceEditorGUIView::drawMarkerLines(const Sequence& sequence, SequencePlayer& player) const
    {
        const float line_thickness = 2.0f * mState.mScale;
        const ImVec4 white_color = ImGui::ColorConvertU32ToFloat4(mService.getColors().mFro3);
        const ImU32 color = ImGui::ColorConvertFloat4ToU32({white_color.x, white_color.y, white_color.z, 1.0f});
        const float marker_width = 5.0f * mState.mScale;
        const float line_y_start = 25.0f * mState.mScale;
        const float line_stop = mState.mTotalTracksHeight - mState.mScroll.y + line_y_start;

        for(const auto &marker: sequence.mMarkers)
        {
            ImVec2 pos =
                {
                    mState.mWindowPos.x + mState.mTimelineControllerPos.x - mState.mScroll.x
                    + mState.mInspectorWidth + marker_width
                    + mState.mTimelineWidth * (float) (marker->mTime / player.getDuration()),
                    mState.mWindowPos.y + mState.mTimelineControllerPos.y + line_y_start
                };

            // if player position in inside the sequencer window, draw it
            if(pos.x < mState.mWindowPos.x + mState.mWindowSize.x && pos.x > mState.mWindowPos.x)
            {
                ImVec2 line_begin =
                    {
                        pos.x,
                        math::max<float>(mState.mWindowPos.y + line_y_start, pos.y) // clip line to top of window
                    };

                ImVec2 line_end =
                    {
                        pos.x,
                        pos.y + math::min<float>(line_stop, // clip the line to bottom of window
                                                 mState.mScroll.y + mState.mWindowSize.y -
                                                 mState.mTimelineControllerPos.y)
                    };

                ImGui::SetNextWindowPos(line_begin);
                if(ImGui::BeginChild(("marker" + marker->mID).c_str(), {line_thickness,
                                                                        line_end.y - line_begin.y}, false,
                                     ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoMove))
                {
                    auto *drawlist = ImGui::GetWindowDrawList();
                    drawlist->AddLine(line_begin, line_end, color, line_thickness);
                }
                ImGui::EndChild();
            }
        }
    }


    void SequenceEditorGUIView::drawEndOfSequence(const Sequence& sequence, SequencePlayer& player)
    {
        std::ostringstream string_stream;
        string_stream << mID << "timelinesequenceduration";
        std::string id_string = string_stream.str();

        // store cursorpos
        ImVec2 cursor_pos = ImGui::GetCursorPos();

        ImVec2 offset = {5.0f * mState.mScale, 15.0f * mState.mScale};
        const float bottom_y = mState.mTotalTracksHeight + (30.0f * mState.mScale);

        ImGui::SetCursorPos(
            {
                mState.mTimelineControllerPos.x
                + mState.mInspectorWidth + offset.x
                + mState.mTimelineWidth,
                mState.mTimelineControllerPos.y + offset.y
            });

        ImGui::PushStyleColor(ImGuiCol_ChildBg, mService.getColors().mFro3);
        if(ImGui::BeginChild(id_string.c_str(), // id
                             {1.0f, bottom_y}, // size
                             false, // no border
                             ImGuiWindowFlags_NoMove)) // window flags
        {
            // empty
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // pop cursorpos
        ImGui::SetCursorPos(cursor_pos);
    }


    void SequenceEditorGUIView::handleLoadPopup()
    {
        if(mState.mAction->isAction<LoadPopup>())
        {
            auto *load_action = mState.mAction->getDerived<LoadPopup>();

            //
            if(ImGui::BeginPopupModal(
                "Load",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                //
                const std::string show_dir = "sequences/";

                // Find all files in the preset directory
                std::vector<std::string> files_in_directory;
                utility::listDir(show_dir.c_str(), files_in_directory);

                std::vector<std::string> shows;
                std::vector<std::string> show_files;
                for(const auto &filename: files_in_directory)
                {
                    // Ignore directories
                    if(utility::dirExists(filename))
                        continue;

                    if(utility::getFileExtension(filename) == "json")
                    {
                        shows.emplace_back(utility::getFileName(filename));
                        show_files.emplace_back(filename);
                    }
                }

                SequenceTrackView::Combo("Sequences",
                                         &load_action->mSelectedShowIndex,
                                         shows);

                utility::ErrorState error_state;
                auto &gui = mService.getGui();
                if(ImGui::ImageButton(gui.getIcon(icon::ok), "load"))
                {
                    if(mEditor.mSequencePlayer->load(utility::getFileName(show_files[load_action->mSelectedShowIndex]),
                                                     error_state))
                    {
                        mState.mAction = createAction<None>();
                        mState.mDirty = true;
                        ImGui::CloseCurrentPopup();
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
                        mState.mAction = createAction<LoadPopup>();
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                ImGui::EndPopup();
            }
        }
    }


    void SequenceEditorGUIView::handleSaveAsPopup()
    {
        if(mState.mAction->isAction<SaveAsPopup>())
        {
            auto *save_as_action = mState.mAction->getDerived<SaveAsPopup>();

            // save as popup
            if(ImGui::BeginPopupModal(
                "Save As",
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                //
                const std::string show_dir = "sequences";

                // Find all files in the preset directory
                std::vector<std::string> files_in_directory;
                utility::listDir(show_dir.c_str(), files_in_directory);

                std::vector<std::string> shows;
                for(const auto &filename: files_in_directory)
                {
                    // Ignore directories
                    if(utility::dirExists(filename))
                        continue;

                    if(utility::getFileExtension(filename) == "json")
                    {
                        shows.push_back(utility::getFileName(filename));
                    }
                }
                shows.emplace_back("<New...>");

                if(SequenceTrackView::Combo("Shows",
                                            &save_as_action->mSelectedShowIndex,
                                            shows))
                {
                    if(save_as_action->mSelectedShowIndex == shows.size() - 1)
                    {
                        ImGui::OpenPopup("New");
                    } else
                    {
                        ImGui::OpenPopup("Overwrite");
                    }
                }

                // new show popup
                std::string new_show_filename;
                bool done = false;
                if(ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    static char name[256] = {0};
                    ImGui::InputText("Name", name, 256);

                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)) && strlen(name) != 0)
                    {
                        new_show_filename = std::string(name, strlen(name));
                        new_show_filename += ".json";

                        ImGui::CloseCurrentPopup();
                        done = true;
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }
                if(done)
                {
                    // Insert before the '<new...>' item
                    shows.insert(shows.end() - 1, new_show_filename);

                    utility::ErrorState error_state;

                    if(mEditor.mSequencePlayer->save(utility::getFileName(new_show_filename), error_state))
                    {
                        save_as_action->mSelectedShowIndex = (int) shows.size() - 2;
                        mState.mDirty = true;
                    } else
                    {
                        mState.mDirty = true;
                        save_as_action->mErrorString = error_state.toString();
                        ImGui::OpenPopup("Error");
                    }
                }

                if(ImGui::BeginPopupModal("Overwrite"))
                {
                    utility::ErrorState error_state;
                    ImGui::Text(("Are you sure you want to overwrite " +
                                 shows[save_as_action->mSelectedShowIndex] + " ?").c_str());

                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                    {
                        if(mEditor.mSequencePlayer->save(
                            utility::getFileName(shows[save_as_action->mSelectedShowIndex]), error_state))
                        {
                        } else
                        {
                            save_as_action->mErrorString = error_state.toString();
                            ImGui::OpenPopup("Error");
                        }

                        mState.mDirty = true;

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if(ImGui::BeginPopupModal("Error"))
                {
                    ImGui::Text(save_as_action->mErrorString.c_str());
                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                    {
                        mState.mDirty = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                auto &gui = mService.getGui();
                if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                {
                    // update title and close popup
                    mRenderWindow->setTitle(mEditor.mSequencePlayer->getSequenceFilename());
                    mState.mAction = createAction<None>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }
    }


    void SequenceEditorGUIView::handleSaveClipboardPopup()
    {
        if(mState.mAction->isAction<SavePresetPopup>())
        {
            const std::string popup_name = "Save segments in clipboard as preset..";
            auto *save_as_action = mState.mAction->getDerived<SavePresetPopup>();

            if(!ImGui::IsPopupOpen(popup_name.c_str()))
                ImGui::OpenPopup(popup_name.c_str());

            // save as popup
            if(ImGui::BeginPopupModal(
                popup_name.c_str(),
                nullptr,
                ImGuiWindowFlags_AlwaysAutoResize))
            {
                // create presets dir
                const std::string presets_dir = utility::joinPath({"sequences", "presets"});

                // Find all files in the preset directory
                std::vector<std::string> files_in_directory;
                utility::listDir(presets_dir.c_str(), files_in_directory);

                std::vector<std::string> presets;
                for(const auto &filename: files_in_directory)
                {
                    // Ignore directories
                    if(utility::dirExists(filename))
                        continue;

                    if(utility::getFileExtension(filename) == "json")
                    {
                        presets.push_back(utility::getFileName(filename));
                    }
                }
                presets.emplace_back("<New...>");

                if(SequenceTrackView::Combo("Presets",
                                            &save_as_action->mSelectedPresetIndex,
                                            presets))
                {
                    if(save_as_action->mSelectedPresetIndex == presets.size() - 1)
                    {
                        ImGui::OpenPopup("New");
                    } else
                    {
                        ImGui::OpenPopup("Overwrite");
                    }
                }

                // new preset popup
                std::string new_show_filename;
                bool done = false;
                if(ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    static char name[256] = {0};
                    ImGui::InputText("Name", name, 256);

                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)) && strlen(name) != 0)
                    {
                        new_show_filename = std::string(name, strlen(name));
                        new_show_filename += ".json";

                        ImGui::CloseCurrentPopup();
                        done = true;
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
                        ImGui::CloseCurrentPopup();

                    ImGui::EndPopup();
                }
                if(done)
                {
                    // Insert before the '<new...>' item
                    presets.insert(presets.end() - 1, new_show_filename);

                    utility::ErrorState error_state;

                    if(mState.mClipboard->save(utility::joinPath({presets_dir, new_show_filename}), error_state))
                    {
                        save_as_action->mSelectedPresetIndex = (int) presets.size() - 2;
                        mState.mDirty = true;
                    } else
                    {
                        mState.mDirty = true;
                        save_as_action->mErrorString = error_state.toString();
                        ImGui::OpenPopup("Error");
                    }
                }

                if(ImGui::BeginPopupModal("Overwrite"))
                {
                    utility::ErrorState error_state;
                    ImGui::Text(("Are you sure you want to overwrite " +
                                 presets[save_as_action->mSelectedPresetIndex] + " ?").c_str());

                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                    {
                        if(mState.mClipboard->save(utility::joinPath({presets_dir,
                                                                      presets[save_as_action->mSelectedPresetIndex]}), error_state))
                        {
                        } else
                        {
                            save_as_action->mErrorString = error_state.toString();
                            ImGui::OpenPopup("Error");
                        }

                        mState.mDirty = true;

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if(ImGui::BeginPopupModal("Error"))
                {
                    ImGui::Text(save_as_action->mErrorString.c_str());
                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                    {
                        mState.mDirty = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                auto &gui = mService.getGui();
                if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                {
                    mState.mAction = createAction<None>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }
    }


    void SequenceEditorGUIView::handleInsertTrackPopup()
    {
        auto* action = mState.mAction->getDerived<sequenceguiactions::InsertTrackPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Insert New Track");
        }

        if(ImGui::BeginPopup("Insert New Track"))
        {
            const auto &track_types = mService.getAllTrackTypes();
            for(const auto &track_type: track_types)
            {
                const auto &name = track_type.get_name().to_string();
                if(ImGui::Button(name.c_str()))
                {
                    auto *controller = mEditor.getControllerWithTrackType(track_type);
                    assert(controller != nullptr);
                    if(controller != nullptr)
                    {
                        controller->insertTrack(track_type);
                        mState.mAction = createAction<None>();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            auto &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
            {
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        } else
        {
            // clicked outside so exit popup
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEditorGUIView::handleEditMarkerPopup()
    {
        auto* action = mState.mAction->getDerived<EditSequenceMarkerPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Edit Sequence Marker");
            action->mOpened = true;
        }

        if(ImGui::BeginPopup("Edit Sequence Marker"))
        {
            auto *action = mState.mAction->getDerived<EditSequenceMarkerPopup>();

            double time = action->mTime;

            std::vector<int> time_array = convertTimeToMMSSMSArray(time);

            bool edit_time = false;

            ImGui::Separator();
            ImGui::PushItemWidth(100.0f * mState.mScale);

            edit_time = ImGui::InputInt3("Time (mm:ss:ms)", &time_array[0]);
            time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
            time_array[1] = math::clamp<int>(time_array[1], 0, 59);
            time_array[2] = math::clamp<int>(time_array[2], 0, 99);

            if(edit_time)
            {
                double new_time = convertMMSSMSArrayToTime(time_array);
                action->mTime = new_time;
                mEditor.changeMarkerTime(action->mID, new_time);
            }

            char buffer[256];
            strcpy(buffer, action->mMessage.c_str());
            if(ImGui::InputText("Message", buffer, 256))
            {
                action->mMessage = std::string(buffer);
                mEditor.changeMarkerMessage(action->mID, action->mMessage);
            }

            auto &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::del), "Delete"))
            {
                mEditor.deleteMarker(action->mID);
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::ok)))
            {
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        } else
        {
            // clicked outside so exit popup
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEditorGUIView::handleInsertMarkerPopup()
    {
        auto *action = mState.mAction->getDerived<InsertingSequenceMarkerPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Insert Sequence Marker");
            action->mOpened = true;
        }

        if(ImGui::BeginPopup("Insert Sequence Marker"))
        {
            char buffer[256];
            strcpy(buffer, action->mMessage.c_str());
            if(ImGui::InputText("Message", buffer, 256))
            {
                action->mMessage = std::string(buffer);
            }

            auto &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::ok), "Insert Marker"))
            {
                mEditor.insertMarker(action->mTime, action->mMessage);
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::cancel)))
            {
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        } else
        {
            // clicked outside so exit popup
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEditorGUIView::handleHorizontalZoom()
    {
        // get sequence player
        const auto &sequence_player = *mEditor.mSequencePlayer.get();

        // get percentage of scroll of total time line width
        float scroll_perc = mState.mScroll.x / mState.mTimelineWidth;

        // total timeline width represents total sequence duration, so get the leftmost time stamp
        double time_start = scroll_perc * sequence_player.getDuration();

        // get the rightmost timestamp visible
        double time_end = (scroll_perc + ((mState.mWindowSize.x - mState.mInspectorWidth) / mState.mTimelineWidth)) *
                          sequence_player.getDuration();

        // this is the time that is in the middle
        float time_focus = (float) (time_start + time_end) * 0.5f;

        // calc new timeline width
        mState.mTimelineWidth = mState.mHorizontalResolution * mState.mScale * (float) sequence_player.getDuration();

        // calc the new scroll keeping time_focus in the middle
        mState.mScroll.x = (float) (time_focus / sequence_player.getDuration()) * mState.mTimelineWidth -
                           ((mState.mWindowSize.x - mState.mInspectorWidth) * 0.5f);

        // finally set the new scroll value
        ImGui::SetScrollX(mState.mScroll.x);

        // mark dirty so curves can be redrawn
        mState.mDirty = true;
    }


    void SequenceEditorGUIView::handleSequenceDurationPopup()
    {
        auto* action = mState.mAction->getDerived<EditSequenceDurationPopup>();
        assert(action!= nullptr);
        if(!action->mOpened)
        {
            action->mOpened = true;
            ImGui::OpenPopup("Edit Sequence Duration");
        }

        if(ImGui::BeginPopup("Edit Sequence Duration"))
        {
            double duration = mEditor.mSequencePlayer->getDuration();

            std::vector<int> time_array = convertTimeToMMSSMSArray(duration);

            bool edit_time = false;

            ImGui::Separator();

            ImGui::PushItemWidth(100.0f);

            edit_time = ImGui::InputInt3("Duration (mm:ss:ms)", &time_array[0]);
            time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
            time_array[1] = math::clamp<int>(time_array[1], 0, 59);
            time_array[2] = math::clamp<int>(time_array[2], 0, 99);

            if(edit_time)
            {
                double new_duration = convertMMSSMSArrayToTime(time_array);
                mEditor.changeSequenceDuration(new_duration);
                mState.mDirty = true;
            }

            ImGui::PopItemWidth();

            ImGui::Separator();

            auto &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::ok)))
            {
                mState.mAction = createAction<None>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        } else
        {
            // clicked outside so exit popup
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEditorGUIView::registerActionHandler(const rttr::type& actionType, const std::function<void()>& action)
    {
        auto it = mActionHandlers.find(actionType);
        assert(it == mActionHandlers.end()); // key already present
        if(it == mActionHandlers.end())
        {
            mActionHandlers.insert({actionType, action});
        }
    }


    void SequenceEditorGUIView::handleHelpPopup()
    {
        auto* action = mState.mAction->getDerived<sequenceguiactions::HelpPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Help");
            action->mOpened = true;
        }

        if(ImGui::BeginPopupModal("Help", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const float width = 300.0f * mState.mScale;
            auto color = ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh2);
            ImGui::Text("Start / Stop playback :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Spacebar");
            ImGui::Text("Select & drag :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Left mouse button");
            ImGui::Text("Select & drag + move next segments :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Control + Left mouse button");
            ImGui::Text("Select & open edit popup :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Right mouse button");
            ImGui::Text("Copy segment :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Shift + Left click segment handler");
            ImGui::Text("Zoom in & out :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Control + Scroll Wheel");
            ImGui::Text("Horizontal Scroll :");
            ImGui::SameLine(width);
            ImGui::TextColored(color, "Shift + Scroll Wheel");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }
            ImGui::EndPopup();
        } else
        {
            // clicked outside so exit popup
            mState.mAction = createAction<None>();
        }
    }


    void SequenceEditorGUIView::handleHistoryPopup()
    {
        static auto strip_class_prefix_and_namespaces = [](std::string& type_string)
        {
            /**
             * Remove "class " prefix if present
             */
            std::string class_prefix = "class ";
            auto npos = type_string.find(class_prefix);
            if(npos != std::string::npos)
            {
                type_string.erase(npos, class_prefix.length());
            }

            /**
             * Remove templated class names "<>"
             */
            size_t first = type_string.find_first_of("<");
            size_t last = type_string.find_last_of(">");
            while(first != std::string::npos && last != std::string::npos)
            {
                type_string.erase(first,last-first+1);

                first = type_string.find_first_of("<");
                last = type_string.find_last_of(">");
            }

            /**
             * Remove all namespaces
             */
            npos = type_string.find("::");
            while(npos != std::string::npos)
            {
                type_string.erase(0, npos + 2);
                npos = type_string.find("::");
            }
        };

        if(mState.mAction->isAction<OpenHistoryPopup>())
        {
            mState.mAction = createAction<SelectHistoryPopup>();
            ImGui::OpenPopup("History");
        }

        IMGuiService &gui = mService.getGui();

        if(mState.mAction->isAction<SelectHistoryPopup>())
        {
            if(ImGui::BeginPopupModal("History", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                // widths of table
                const float widths[3] =
                        {
                                300.0f * mState.mScale,
                                500.0f * mState.mScale,
                                600.0f * mState.mScale
                        };

                // highlight color
                const auto highlight_color = ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh2);

                ImGui::Text("Action:");
                ImGui::SameLine(widths[0]);
                ImGui::Text("Date & Time:");
                ImGui::SameLine(widths[1]);
                ImGui::Text("Size:");
                ImGui::Separator();

                const auto& history = mEditor.getHistory();
                int history_index = mEditor.getHistoryIndex();

                double total_kilobyte = 0.0;

                int idx = history.size() - 1;
                bool jump_to_history_point = false;
                int jump_to_history_point_idx = -1;
                for (auto it = history.rbegin(); it != history.rend(); ++it)
                {
                    const auto& history_point = *it;

                    ImGui::PushID(idx);
                    if(history_index == idx)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, highlight_color);
                    }
                    std::string type_name = history_point->mActionType.get_raw_type().get_name().to_string();

                    strip_class_prefix_and_namespaces(type_name);
                    ImGui::Text(type_name.c_str());
                    ImGui::SameLine(widths[0]);
                    ImGui::Text(history_point->mDateTime.toString().c_str());
                    ImGui::SameLine(widths[1]);
                    float kilobyte = (static_cast<float>(history_point->mBinaryWriter.getBuffer().size()) / 1024.0f);
                    total_kilobyte += kilobyte;
                    std::string size_string = utility::stringFormat("%.2fkb", kilobyte);
                    ImGui::Text(size_string.c_str());
                    if(history_index == idx)
                    {
                        ImGui::PopStyleColor();
                    }
                    ImGui::SameLine(widths[2]);

                    // jump to different history point outside this loop because it
                    // modifies the history deque
                    if(ImGui::ImageButton(gui.getIcon(icon::load), "select"))
                    {
                        jump_to_history_point = true;
                        jump_to_history_point_idx = idx;
                    }
                    idx--;
                    ImGui::PopID();
                }

                // jump to history point if user pressed load on a history point
                if(jump_to_history_point)
                {
                    if(mEditor.getHistoryIndex() == mEditor.getHistorySize())
                        mEditor.takeSnapshot(RTTI_OF(sequenceguiactions::PerformUndo));

                    mEditor.jumpToHistoryPointIndex(jump_to_history_point_idx);
                    mState.mDirty = true;
                }

                ImGui::Spacing();
                ImGui::Separator();
                std::string total_size_string = utility::stringFormat("Total history size %.2fmb", total_kilobyte / 1024.0);
                ImGui::Text(total_size_string.c_str());
                ImGui::Spacing();

                if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
                {
                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                }
                ImGui::EndPopup();
            } else
            {
                // clicked outside so exit popup
                mState.mAction = createAction<None>();
            }
        }
    }


    void SequenceEditorGUIView::handleUndo()
    {
        assert(mState.mAction->isAction<PerformUndo>());
        auto* action = mState.mAction->getDerived<PerformUndo>();

        // clear clipboard
        mState.mClipboard = createClipboard<Empty>();

        // if history index is currently at the end of the history size, take a snapshot to store the current
        // state of the sequence and add it to the history
        if(mEditor.getHistoryIndex() == mEditor.getHistorySize())
        {
            mEditor.takeSnapshot(action->get_type());
            mEditor.undo();
        }
        mEditor.undo();
        mState.mAction = createAction<None>();
        mState.mDirty = true;
    }


    void SequenceEditorGUIView::handleRedo()
    {
        // clear clipboard
        mState.mClipboard = createClipboard<Empty>();

        mEditor.redo();
        mState.mAction = createAction<None>();
        mState.mDirty = true;
    }


    void SequenceEditorGUIView::handleHotKeys()
    {
        // if window is focused
        if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive())
        {
            // if ctrl is pressed, check for undo/redo actions
            if(ImGui::GetIO().KeyCtrl)
            {
                // redo
                if(ImGui::GetIO().KeyShift)
                {
                    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
                    {
                        if(mEditor.getHistoryIndex() < mEditor.getHistorySize()-1)
                        {
                            mState.mAction = createAction<PerformRedo>();
                        }
                    }
                }else
                {
                    // undo
                    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z)))
                    {
                        mState.mAction = createAction<PerformUndo>();
                    }
                }
            }else
            {
                // Start stop player with keyboard
                if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                {
                    mEditor.mSequencePlayer->setIsPlaying(!mEditor.mSequencePlayer->getIsPlaying());
                }
            }

            // allow mouse zoom-in with mouse wheel and ctrl
            if(ImGui::GetIO().KeyCtrl)
            {
                float scroll = ImGui::GetIO().MouseWheel;
                if(scroll != 0.0f)
                {
                    float new_resolution = mState.mHorizontalResolution + ImGui::GetIO().MouseWheel * 5.0f;
                    new_resolution = math::max<float>(new_resolution, 2.5f);
                    mState.mAction = createAction<ChangeHorizontalResolution>(new_resolution);
                }
            }
        }
    }


    void SequenceEditorGUIView::registerActionHandlers()
    {
        // register handlers for popups
        registerActionHandler(RTTI_OF(EditSequenceMarkerPopup), [this] { handleEditMarkerPopup(); });
        registerActionHandler(RTTI_OF(InsertingSequenceMarkerPopup), [this] { handleInsertMarkerPopup(); });
        registerActionHandler(RTTI_OF(InsertTrackPopup), [this] { handleInsertTrackPopup(); });
        registerActionHandler(RTTI_OF(EditSequenceDurationPopup), [this] { handleSequenceDurationPopup(); });
        registerActionHandler(RTTI_OF(LoadPopup), [this] { handleLoadPopup(); });
        registerActionHandler(RTTI_OF(SaveAsPopup), [this] { handleSaveAsPopup(); });
        registerActionHandler(RTTI_OF(HelpPopup), [this] { handleHelpPopup(); });
        registerActionHandler(RTTI_OF(SavePresetPopup), [this] { handleSaveClipboardPopup(); });
        registerActionHandler(RTTI_OF(OpenHistoryPopup), [this] { handleHistoryPopup(); });
        registerActionHandler(RTTI_OF(SelectHistoryPopup), [this] { handleHistoryPopup(); });
        registerActionHandler(RTTI_OF(PerformUndo), [this] { handleUndo(); });
        registerActionHandler(RTTI_OF(PerformRedo), [this] { handleRedo(); });

        /**
         * action handlers for changing horizontal resolution (zoom)
         */
        registerActionHandler(RTTI_OF(ChangeHorizontalResolution), [this]
        {
            assert(mState.mAction->isAction<ChangeHorizontalResolution>());
            auto *action = mState.mAction->getDerived<ChangeHorizontalResolution>();
            mState.mHorizontalResolution = action->mHorizontalResolution;
            handleHorizontalZoom();
            mState.mAction = createAction<None>();
        });

        registerActionHandler(RTTI_OF(ChangeTrackName), [this]
        {
            assert(mState.mAction->isAction<ChangeTrackName>());
            auto *action = mState.mAction->getDerived<ChangeTrackName>();
            auto *controller = mEditor.getControllerWithTrackID(action->mTrackID);
            assert(controller != nullptr); // controller not found
            controller->changeTrackName(action->mTrackID, action->mNewTrackName);
            mState.mDirty = true;
            mState.mAction = createAction<None>();
        });
        registerActionHandler(RTTI_OF(DraggingTrackExtensionHandler), [this]
        {
            assert(mState.mAction->isAction<DraggingTrackExtensionHandler>());
            auto *action = mState.mAction->getDerived<DraggingTrackExtensionHandler>();
            if(ImGui::IsMouseDown(0))
            {
                // obtain controller
                auto *controller = mEditor.getControllerWithTrackID(action->mTrackID);
                assert(controller != nullptr);

                // obtain track
                const auto *track = controller->getTrack(action->mTrackID);

                // add delta move to track height
                float move = mState.mMouseDelta.y / mState.mScale;
                float new_track_height = track->mTrackHeight + move;

                // clip the track height to minimum track height
                if(new_track_height < track->getMinimumTrackHeight())
                    new_track_height = track->getMinimumTrackHeight();

                controller->changeTrackHeight(action->mTrackID, new_track_height);

                // mark dirty
                mState.mDirty = true;
            } else
            {
                // mouse release so cancel action
                mState.mAction = createAction<None>();
            }
        });

        /**
         * When mouse is pressed but no actions are taken, switch to NonePressed action so no actions are triggered when
         * mouse is being dragged into one of the tracks in the sequencer window
         */
        registerActionHandler(RTTI_OF(None), [this]
        {
            if(ImGui::IsMouseDown(0))
            {
                mState.mAction = createAction<NonePressed>();
            }
        });
        registerActionHandler(RTTI_OF(NonePressed), [this]
        {
            if(!ImGui::IsMouseDown(0))
            {
                mState.mAction = createAction<None>();
            }
        });
    }
}
