/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceaudiotrackview.h"
#include "sequenceplayeraudiooutput.h"
#include "sequenceguiservice.h"
#include "sequenceplayeraudioclock.h"
#include "sequenceaudioguiservice.h"

// nap includes
#include <parametervec.h>
#include <sequenceeditorgui.h>
#include <sequencetracksegmentaudio.h>
#include <sequencecontrolleraudio.h>
#include <sequencetrackaudio.h>
#include <sequenceguiutils.h>
#include <imguiutils.h>

// external includes
#include <iostream>

namespace nap
{
    using namespace audio;
    using namespace sequenceguiactions;
    using namespace sequenceguiclipboard;


    SequenceAudioTrackView::SequenceAudioTrackView(SequenceGUIService& service, SequenceEditorGUIView& view,
                                                   SequenceEditorGUIState& state)
        : SequenceTrackView(view, state)
    {
        mAudioGUIService = mService.getCore().getService<SequenceAudioGUIService>();

        registerActionHandler(RTTI_OF(AssignOutputIDToTrack), [this]
        {
            handleAssignOutputIDToTrack();
        });
        registerActionHandler(RTTI_OF(StartDraggingSegment), [this]
        {
            handleSegmentDrag();
        });
        registerActionHandler(RTTI_OF(DraggingSegment), [this]
        {
            handleSegmentDrag();
        });
        registerActionHandler(RTTI_OF(InsertingSegmentPopup), [this]
        {
            handleSegmentInsert();
        });
        registerActionHandler(RTTI_OF(InsertAudioSegmentPopup), [this]
        {
            handleSegmentInsert();
        });
        registerActionHandler(RTTI_OF(EditAudioSegmentPopup), [this]
        {
            handleEditSegment();
        });
        registerActionHandler(RTTI_OF(DraggingLeftAudioSegmentHandler), [this]
        {
            handleLeftHandlerDrag();
        });
        registerActionHandler(RTTI_OF(DraggingRightAudioSegmentHandler), [this]
        {
            handleRightHandlerDrag();
        });
        registerActionHandler(RTTI_OF(LoadPresetPopup), [this]
        {
            handleLoadPresetPopup();
        });
    }


    void SequenceAudioTrackView::showInspectorContent(const SequenceTrack& track)
    {
        ImGui::Text("Assigned Audio Output");

        const float offset = 5 * mState.mScale;
        ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
        inspector_cursor_pos.x += offset;
        inspector_cursor_pos.y += offset;
        ImGui::SetCursorPos(inspector_cursor_pos);

        bool assigned = false;
        std::string assigned_id;
        std::vector<std::string> audio_outputs;
        int current_item = 0;
        audio_outputs.emplace_back("none");

        int count = 0;

        for(const auto& output: getEditor().mSequencePlayer->mOutputs)
        {
            if(output.get()->get_type().is_derived_from<SequencePlayerAudioOutput>())
            {
                count++;

                if(output->mID == track.mAssignedOutputID)
                {
                    assigned = true;
                    assigned_id = output->mID;
                    current_item = count;

                    assert(output.get()->get_type().is_derived_from<SequencePlayerAudioOutput>()); // type mismatch
                }

                audio_outputs.emplace_back(output->mID);
            }
        }

        const float item_width = 200.0f * mState.mScale;
        ImGui::PushItemWidth(item_width);
        if(Combo("", &current_item, audio_outputs))
        {
            if(current_item != 0)
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, audio_outputs[current_item]);
            else
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, "");

        }

        // draw warning if necessary
        if(!getPlayer().mClock.get()->get_type().is_derived_from<SequencePlayerAudioClock>())
        {
            inspector_cursor_pos = ImGui::GetCursorPos();
            inspector_cursor_pos.x += offset;
            inspector_cursor_pos.y += offset;
            ImGui::SetCursorPos(inspector_cursor_pos);

            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh2), "No AudioClock used by player!\nAudioSegments will not play!");

        }
        ImGui::PopItemWidth();
    }


    void SequenceAudioTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
    {
        // if dirty, clear waveform cache
        if(mState.mDirty)
        {
            mWaveformCache.clear();
        }

        const float track_height = track.mTrackHeight * mState.mScale;

        // get drawlist
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // handle insertion of segment
        if(mState.mAction->isAction<None>())
        {
            if(ImGui::IsMouseHoveringRect(trackTopLeft, {trackTopLeft.x + mState.mTimelineWidth,
                                                         trackTopLeft.y + track_height}))
            {
                // position of mouse in track
                draw_list->AddLine({mState.mMousePos.x, trackTopLeft.y}, // top left
                                   {mState.mMousePos.x, trackTopLeft.y + track_height}, // bottom right
                                   mService.getColors().mFro2, // color
                                   1.0f); // thickness

                // draw tooltip
                ImGui::BeginTooltip();
                ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());
                ImGui::EndTooltip();

                // right mouse down, create InsertSegmentPopup
                if(ImGui::IsMouseClicked(1))
                {
                    double time = mState.mMouseCursorTime;
                    mState.mAction = createAction<InsertingSegmentPopup>(track.mID, time, track.get_type());
                }
            }
        }

        // draw line in track while in inserting segment popup
        if(mState.mAction->isAction<InsertingSegmentPopup>())
        {
            auto *action = mState.mAction->getDerived<InsertingSegmentPopup>();

            if(action->mTrackID == track.mID)
            {
                // position of insertion in track
                const float line_thickness = 1.0f * mState.mScale;
                draw_list->AddLine({trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                                    trackTopLeft.y}, // top left
                                   {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                                    trackTopLeft.y + track_height}, // bottom right
                                   mService.getColors().mFro2, // color
                                   line_thickness); // thickness
            }
        }

        // draw line in track while in inserting segment popup
        if(mState.mAction->isAction<InsertingSegmentPopup>())
        {
            auto *action = mState.mAction->getDerived<InsertingSegmentPopup>();

            if(action->mTrackID == track.mID)
            {
                // position of insertion in track
                const float line_thickness = 1.0f * mState.mScale;
                draw_list->AddLine({trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                                    trackTopLeft.y}, // top left
                                   {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                                    trackTopLeft.y + track_height}, // bottom right
                                   mService.getColors().mFro2, // color
                                   line_thickness); // thickness
            }
        }

        // draw segments
        int segment_count = 0;
        for(const auto &segment: track.mSegments)
        {
            // upcast to duration segment
            assert(segment.get()->get_type().is_derived_from<SequenceTrackSegmentAudio>()); // type mismatch
            const auto* segment_audio = static_cast<const SequenceTrackSegmentAudio*>(segment.get());

            // obtain width & position
            float segment_x = (float) (segment_audio->mStartTime) * mState.mStepSize;
            float segment_width = (float) (segment_audio->mDuration) * mState.mStepSize;

            // calc segment top left and bottom right
            const float one_pixel_offset = 1.0f * mState.mScale;
            ImVec2 segment_top_left = {trackTopLeft.x + segment_x, trackTopLeft.y + one_pixel_offset};
            ImVec2 segment_bottom_right = {trackTopLeft.x + segment_x + segment_width,
                                           trackTopLeft.y + track_height - one_pixel_offset};

            /**
             * Handle hovering and dragging of left handler
             */
            bool hovering_segment = false;
            bool dragging_segment = false;
            bool hovering_left_handler = false;
            bool dragging_left_handler = false;
            bool hovering_right_handler = false;
            bool dragging_right_handler = false;

            const float handler_margin = 10.0f * mState.mScale;
            if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringSegment>())
            {
                if(ImGui::IsMouseHoveringRect({segment_top_left.x - handler_margin, segment_top_left.y}, {
                    segment_top_left.x + handler_margin, segment_bottom_right.y}))
                {
                    mState.mAction = createAction<HoveringLeftAudioSegmentHandler>(track.mID, segment->mID);
                }
            }
            if(mState.mAction->isAction<HoveringLeftAudioSegmentHandler>())
            {
                auto *action = mState.mAction->getDerived<HoveringLeftAudioSegmentHandler>();
                if(action->mSegmentID == segment->mID)
                {
                    hovering_left_handler = true;
                    if(!ImGui::IsMouseHoveringRect({segment_top_left.x - handler_margin, segment_top_left.y}, {
                        segment_top_left.x + handler_margin, segment_bottom_right.y}))
                    {
                        hovering_left_handler = false;
                        mState.mAction = createAction<None>();
                    } else if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = createAction<DraggingLeftAudioSegmentHandler>(track.mID, segment->mID);
                    }
                }
            }
            if(mState.mAction->isAction<DraggingLeftAudioSegmentHandler>())
            {
                auto *action = mState.mAction->getDerived<DraggingLeftAudioSegmentHandler>();
                dragging_left_handler = action->mSegmentID == segment->mID;
            }

            /**
            * Handle hovering and dragging of right handler
            */
            if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringSegment>())
            {
                if(ImGui::IsMouseHoveringRect({segment_bottom_right.x - handler_margin, segment_top_left.y}, {
                    segment_bottom_right.x + handler_margin, segment_bottom_right.y}))
                {
                    mState.mAction = createAction<HoveringRightAudioSegmentHandler>(track.mID, segment->mID);
                }
            }
            if(mState.mAction->isAction<HoveringRightAudioSegmentHandler>())
            {
                auto *action = mState.mAction->getDerived<HoveringRightAudioSegmentHandler>();
                if(action->mSegmentID == segment->mID)
                {
                    hovering_right_handler = true;
                    if(!ImGui::IsMouseHoveringRect({segment_bottom_right.x - handler_margin, segment_top_left.y}, {
                        segment_bottom_right.x + handler_margin, segment_bottom_right.y}))
                    {
                        hovering_right_handler = false;
                        mState.mAction = createAction<None>();
                    } else if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = createAction<DraggingRightAudioSegmentHandler>(track.mID, segment->mID);
                    }
                }
            }
            if(mState.mAction->isAction<DraggingRightAudioSegmentHandler>())
            {
                auto *action = mState.mAction->getDerived<DraggingRightAudioSegmentHandler>();
                dragging_right_handler = action->mSegmentID == segment->mID;
            }

            /**
             * Handle hovering and dragging of segment
             */

            if(mState.mAction->isAction<None>() && ImGui::IsMouseHoveringRect(segment_top_left, segment_bottom_right))
            {
                hovering_segment = true;
                mState.mAction = createAction<HoveringSegment>(track.mID, segment->mID);
            }
            if(mState.mAction->isAction<HoveringSegment>())
            {
                bool inside_this_segment = mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment->mID;

                if(inside_this_segment && !ImGui::IsMouseHoveringRect(segment_top_left, segment_bottom_right))
                {
                    mState.mAction = createAction<None>();
                } else
                {
                    hovering_segment = inside_this_segment;
                }
            }
            if(mState.mAction->isAction<DraggingSegment>())
            {
                auto *action = mState.mAction->getDerived<DraggingSegment>();
                if(action->mSegmentID == segment->mID)
                {
                    hovering_segment = true;
                    dragging_segment = true;
                }
            }
            if(hovering_segment)
            {
                if(ImGui::IsMouseDown(0))
                {
                    if(mState.mAction->isAction<HoveringSegment>())
                    {
                        bool move_next_segments = ImGui::GetIO().KeyCtrl;
                        mState.mAction = createAction<StartDraggingSegment>(track.mID, segment->mID, segment->mStartTime, move_next_segments);
                    }
                } else if(ImGui::IsMouseDown(1))
                {
                    if(mState.mAction->isAction<HoveringSegment>())
                    {
                        mState.mAction = createAction<EditAudioSegmentPopup>(track.mID, segment->mID, mState.mMousePos);
                    }
                }
            }
            if(dragging_segment)
            {
                ImGui::BeginTooltip();
                ImGui::Text("Clip start: ");
                ImGui::SameLine();
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(mService.getColors().mFro4), formatTimeString(segment->mStartTime).c_str());
                ImGui::EndTooltip();
            }

            // handled shift click for add/remove to clipboard
            if(ImGui::IsMouseClicked(0) && hovering_segment)
            {
                if(ImGui::GetIO().KeyShift)
                {
                    // if no audio segment clipboard, create it or if previous clipboard is from a different sequence, create new clipboard
                    if(!mState.mClipboard->isClipboard<AudioSegmentClipboard>())
                        mState.mClipboard = createClipboard<AudioSegmentClipboard>(RTTI_OF(SequenceTrackAudio), getEditor().mSequencePlayer->getSequenceFilename());
                    else if(mState.mClipboard->getDerived<AudioSegmentClipboard>()->getSequenceName() !=
                            getEditor().mSequencePlayer->getSequenceFilename())
                        mState.mClipboard = createClipboard<AudioSegmentClipboard>(RTTI_OF(SequenceTrackAudio), getEditor().mSequencePlayer->getSequenceFilename());

                    // get derived clipboard
                    auto *clipboard = mState.mClipboard->getDerived<AudioSegmentClipboard>();

                    // if the clipboard contains this segment or is a different sequence, remove it
                    if(clipboard->containsObject(segment->mID, getPlayer().getSequenceFilename()))
                    {
                        clipboard->removeObject(segment->mID);
                    } else
                    {
                        // if not, serialize it into clipboard
                        utility::ErrorState errorState;
                        clipboard->addObject(segment.get(), getPlayer().getSequenceFilename(), errorState);

                        // log any errors
                        if(errorState.hasErrors())
                        {
                            Logger::error(errorState.toString());
                        }
                    }
                }
            }


            /**
             * Proceed drawing contents of segment
             * Look in the cache if waveform is present, if not create the waveform & store it in cache
             */
            bool segment_in_clipboard = false;
            if(mState.mClipboard->containsObject(segment->mID, getPlayer().getSequenceFilename()))
            {
                segment_in_clipboard = true;
            }

            // draw background
            draw_list->AddRectFilled(segment_top_left, // top left
                                     segment_bottom_right, // bottom right
                                     segment_in_clipboard ? (hovering_segment
                                                             ? mAudioGUIService->getColors().mAudioSegmentBackgroundClipboardHovering
                                                             : mAudioGUIService->getColors().mAudioSegmentBackgroundClipboard)
                                                          : (hovering_segment
                                                             ? mAudioGUIService->getColors().mAudioSegmentBackgroundHovering
                                                             : mAudioGUIService->getColors().mAudioSegmentBackground)); // color

            // get sequence player
            const SequencePlayer *sequence_player = getEditor().mSequencePlayer.get();

            // get assigned output
            SequencePlayerOutput *output = nullptr;
            for(const auto &player_output: sequence_player->mOutputs)
            {
                if(player_output->mID == track.mAssignedOutputID)
                {
                    output = player_output.get();
                }
            }

            // upcast to audio output
            SequencePlayerAudioOutput *audio_output = nullptr;
            if(output != nullptr)
            {
                assert(output->get_type().is_derived_from<SequencePlayerAudioOutput>()); // type mismatch
                audio_output = static_cast<SequencePlayerAudioOutput *>(output);
            }

            // if audio output present, get audio buffer assigned for this segment
            if(audio_output != nullptr)
            {
                AudioBufferResource *audio_buffer = nullptr;
                for(const auto &buffer: audio_output->getBuffers())
                {
                    if(buffer->mID == segment_audio->mAudioBufferID)
                    {
                        audio_buffer = buffer.get();
                        break;
                    }
                }

                // determine if we need drawing
                const float points_per_pixel = 2.0f * mState.mScale;
                bool needs_drawing = ImGui::IsRectVisible(segment_top_left, segment_bottom_right);

                if(needs_drawing)
                {
                    if(audio_buffer != nullptr)
                    {
                        // if no cache present, create new polyline
                        if(mWaveformCache.find(segment->mID) == mWaveformCache.end())
                        {
                            // get channels
                            int channels = audio_buffer->getChannelCount();

                            // get number of points
                            const long point_num = (int) (points_per_pixel * segment_width);

                            // polyline per channel
                            std::vector<std::vector<ImVec2>> points;
                            points.resize(channels);

                            auto start_x = math::max<float>(trackTopLeft.x, 0);
                            float end_x = start_x + mState.mWindowSize.x + mState.mWindowPos.x;

                            // start drawing at window pos
                            long i = 0;
                            for(; i <= point_num; i++)
                            {
                                float p = (float) i / (float) point_num;
                                float x = trackTopLeft.x + segment_x + segment_width * p;
                                if(x > start_x)
                                {
                                    // get position in segment
                                    double time =
                                        p * segment_audio->mDuration + segment_audio->mStartTimeInAudioSegment;
                                    int64 position = time * audio_buffer->getSampleRate();

                                    // create polyline for each channel
                                    for(int channel = 0; channel < channels; channel++)
                                    {
                                        if(position < audio_buffer->getBuffer().get()->channels[channel].size())
                                        {
                                            float part_of_height = 1.0f / static_cast<float>(channels);
                                            float sample_value = audio_buffer->getBuffer().get()->channels[channel][position];

                                            float value = 1.0f - ((sample_value / 2.0f) + 0.5f) * part_of_height -
                                                          part_of_height * static_cast<float>(channel);
                                            ImVec2 point = {x, trackTopLeft.y + value * track_height};
                                            points[channel].emplace_back(point);
                                        }
                                    }
                                }

                                if(x > end_x)
                                {
                                    break; // no longer visible on right side, continuation of this loop is not necessary
                                }
                            }

                            mWaveformCache.emplace(segment->mID, std::move(points));
                        }

                        // draw cached waveform
                        if(!mWaveformCache[segment->mID].empty())
                        {
                            const auto &waveforms = mWaveformCache[segment->mID];
                            for(const auto &waveform: waveforms)
                            {
                                if(!waveform.empty())
                                {
                                    // draw points of curve
                                    draw_list->AddPolyline(&*waveform.begin(), // points array
                                                           waveform.size(), // size of points array
                                                           mService.getColors().mHigh1, // color
                                                           false, // closed
                                                           2.0f * mState.mScale); // thickness
                                }
                            }
                        }
                    } else
                    {
                        // create error message
                        std::stringstream error_message_stream;
                        error_message_stream << "Error displaying audio buffer : " << segment_audio->mAudioBufferID
                                             << std::endl << "Missing source!" << std::endl;

                        // draw error
                        ImVec2 text_offset = {5 * mState.mScale, 3 * mState.mScale};
                        ImVec2 text_position = {trackTopLeft.x + segment_x + text_offset.x,
                                                trackTopLeft.y + text_offset.y};
                        draw_list->AddText(text_position, mService.getColors().mHigh4, error_message_stream.str().c_str());
                    }

                    // draw left handler
                    float line_thickness = (hovering_left_handler || dragging_left_handler) ? 3.0f * mState.mScale :
                                           1.0f * mState.mScale;
                    ImU32 line_color = (hovering_left_handler || dragging_left_handler) ? mService.getColors().mFro4
                                                                                        : mService.getColors().mFro1;
                    draw_list->AddLine(segment_top_left, {segment_top_left.x,
                                                          segment_bottom_right.y}, line_color, line_thickness);

                    // draw right handler
                    line_thickness = (hovering_right_handler || dragging_right_handler) ? 3.0f * mState.mScale : 1.0f *
                                                                                                                 mState.mScale;
                    line_color = (hovering_right_handler || dragging_right_handler) ? mService.getColors().mFro4
                                                                                    : mService.getColors().mFro1;
                    draw_list->AddLine({segment_bottom_right.x,
                                        segment_top_left.y}, segment_bottom_right, line_color, line_thickness);
                }
            } else
            {
                // create error message
                std::stringstream error_message_stream;
                error_message_stream << "Error displaying audio buffer : " << segment_audio->mAudioBufferID << std::endl
                                     << "No audio output assigned!" << std::endl;

                // draw error
                ImVec2 text_offset = {5 * mState.mScale, 3 * mState.mScale};
                ImVec2 text_position = {trackTopLeft.x + segment_x + text_offset.x, trackTopLeft.y + text_offset.y};
                draw_list->AddText(text_position, mService.getColors().mHigh4, error_message_stream.str().c_str());
            }

            //
            segment_count++;
        }
    }


    void SequenceAudioTrackView::handleAssignOutputIDToTrack()
    {
        // get action
        auto *action = mState.mAction->getDerived<AssignOutputIDToTrack>();
        assert(action != nullptr);

        // take snapshot
        getEditor().takeSnapshot(action->get_type());

        // get audio controller
        auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

        // call function to controller
        audio_controller.assignNewOutputID(action->mTrackID, action->mOutputID);

        // action is done
        mState.mAction = sequenceguiactions::createAction<None>();
    }


    void SequenceAudioTrackView::handleSegmentDrag()
    {
        if(mState.mAction->isAction<StartDraggingSegment>())
        {
            auto *action = mState.mAction->getDerived<StartDraggingSegment>();
            getEditor().takeSnapshot(action->get_type());
            mState.mAction = createAction<DraggingSegment>(action->mTrackID,
                                                           action->mSegmentID,
                                                           action->mStartDuration,
                                                           action->mMoveNextSegments);

        } else
        {
            auto *action = mState.mAction->getDerived<DraggingSegment>();
            assert(action != nullptr);

            // take snapshot
            if(action->mTakeSnapshot)
            {
                getEditor().takeSnapshot(action->get_type());
                action->mTakeSnapshot = false;
            }

            // get audio controller
            auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

            float amount = mState.mMouseDelta.x / mState.mStepSize;
            action->mNewDuration = audio_controller.segmentAudioStartTimeChange(action->mTrackID, action->mSegmentID,
                                                                                action->mNewDuration + amount);

            mState.mDirty = true;
        }

        if(ImGui::IsMouseReleased(0))
        {
            mState.mAction = createAction<None>();
        }
    }


    void SequenceAudioTrackView::handleSegmentInsert()
    {
        if(mState.mAction->isAction<InsertingSegmentPopup>())
        {
            auto *action = mState.mAction->getDerived<InsertingSegmentPopup>();

            // invoke insert sequence popup
            ImGui::OpenPopup("Insert Audio Segment");

            mState.mAction = createAction<InsertAudioSegmentPopup>(action->mTrackID, action->mTime, 0);
        }

        // handle insert segment popup
        if(mState.mAction->isAction<InsertAudioSegmentPopup>())
        {
            auto &audio_controller = getEditor().getController<SequenceControllerAudio>();
            auto *action = mState.mAction->getDerived<InsertAudioSegmentPopup>();

            if(ImGui::BeginPopup("Insert Audio Segment"))
            {
                /**
                 * handle clipboard paste
                 */
                if(mState.mClipboard->isClipboard<AudioSegmentClipboard>())
                {
                    // get clipboard
                    auto *clipboard = mState.mClipboard->getDerived<AudioSegmentClipboard>();
                    if(clipboard->getObjectCount() > 0)
                    {
                        // if we have multiple segments in clipboard, continue with paste option
                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::paste)))
                        {
                            // deserialize objects
                            utility::ErrorState error_state;

                            getEditor().takeSnapshot(action->get_type());
                            if(pasteClipboard(action->mTrackID, action->mTime, error_state))
                            {
                                ImGui::CloseCurrentPopup();
                                mState.mAction = createAction<None>();

                                ImGui::EndPopup();
                                return;
                            } else
                            {
                                ImGui::OpenPopup("Error");
                                action->mErrorString = error_state.toString();
                            }
                        }
                    }
                }

                /**
                 * handle selection of audio buffers
                 */
                SequencePlayerAudioOutput *audio_output = getAudioOutputForTrack(action->mTrackID);
                if(audio_output != nullptr)
                {
                    auto audio_buffers = getAudioBuffersForTrack(action->mTrackID);

                    if(!audio_buffers.empty())
                    {
                        ImGui::Text("Select audio buffer");
                        if(action->mCurrentItem < audio_buffers.size())
                        {
                            int selection = action->mCurrentItem;
                            if(Combo("Audio Buffers", &selection, audio_buffers))
                            {
                                action->mCurrentItem = selection;
                            }
                        }

                        bool valid_selection = action->mCurrentItem < audio_buffers.size();

                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert)))
                        {
                            if(valid_selection)
                            {
                                getEditor().takeSnapshot(action->get_type());
                                audio_controller.insertAudioSegment(action->mTrackID, action->mTime, audio_buffers[action->mCurrentItem]);
                                mState.mAction = createAction<None>();
                                mState.mDirty = true;
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    } else
                    {
                        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh2), "No audio buffers found!");
                    }
                } else
                {
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh2), "No audio output assigned!");
                }

                /**
                 * handle load preset
                 */
                ImGui::SameLine();
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::load), "Load preset"))
                {
                    mState.mAction = createAction<LoadPresetPopup>(action->mTrackID, action->mTime, RTTI_OF(SequenceTrackAudio));
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }
                ImGui::SameLine();

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
    }


    void SequenceAudioTrackView::handleEditSegment()
    {
        auto* action = mState.mAction->getDerived<EditAudioSegmentPopup>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Edit Audio Segment");
            ImGui::SetNextWindowPos(action->mWindowPos);
            action->mOpened = true;
        }

        if(ImGui::BeginPopup("Edit Audio Segment"))
        {
            auto take_snapshot = [this, action]
            {
                if(action->mTakeSnapshot)
                    action->mTakeSnapshot = false;
                getEditor().takeSnapshot(action->get_type());
            };

            /**
             * obtain controller & segment
             */
            auto &audio_controller = getEditor().getController<SequenceControllerAudio>();
            const auto *segment = audio_controller.getSegment(action->mTrackID, action->mSegmentID);
            assert(segment->get_type() == RTTI_OF(SequenceTrackSegmentAudio));
            const auto *audio_segment = static_cast<const SequenceTrackSegmentAudio *>(segment);

            ImGui::Separator();

            /**
             * handle start time in track
             */
            std::vector<int> time_array = convertTimeToMMSSMSArray(audio_segment->mStartTime);
            bool edit_time = false;

            ImGui::PushItemWidth(100.0f * mState.mScale);

            edit_time = ImGui::InputInt3("Start Time (mm:ss:ms)", &time_array[0]);
            time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
            time_array[1] = math::clamp<int>(time_array[1], 0, 59);
            time_array[2] = math::clamp<int>(time_array[2], 0, 99);

            ImGui::PopItemWidth();

            if(edit_time)
            {
                take_snapshot();

                double new_time = convertMMSSMSArrayToTime(time_array);
                audio_controller.segmentAudioStartTimeChange(action->mTrackID, action->mSegmentID, new_time);
                mState.mDirty = true;
            }

            /**
             * handle start time in audio segment
             */
            time_array = convertTimeToMMSSMSArray(audio_segment->mStartTimeInAudioSegment);

            ImGui::PushItemWidth(100.0f * mState.mScale);

            edit_time = ImGui::InputInt3("Start Time in audio segment (mm:ss:ms)", &time_array[0]);
            time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
            time_array[1] = math::clamp<int>(time_array[1], 0, 59);
            time_array[2] = math::clamp<int>(time_array[2], 0, 99);

            ImGui::PopItemWidth();

            if(edit_time)
            {
                take_snapshot();

                double new_time = convertMMSSMSArrayToTime(time_array);
                audio_controller.segmentAudioStartTimeInSegmentChange(action->mTrackID, action->mSegmentID, new_time);
                mState.mDirty = true;
            }

            /**
             * handle duration
             */
            time_array = convertTimeToMMSSMSArray(audio_segment->mDuration);

            ImGui::PushItemWidth(100.0f * mState.mScale);

            edit_time = ImGui::InputInt3("Audio segment duration (mm:ss:ms)", &time_array[0]);
            time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
            time_array[1] = math::clamp<int>(time_array[1], 0, 59);
            time_array[2] = math::clamp<int>(time_array[2], 0, 99);

            ImGui::PopItemWidth();

            if(edit_time)
            {
                take_snapshot();

                double new_time = convertMMSSMSArrayToTime(time_array);
                audio_controller.segmentAudioDurationChange(action->mTrackID, action->mSegmentID, new_time);
                mState.mDirty = true;
            }

            ImGui::Separator();

            auto audio_buffers = getAudioBuffersForTrack(action->mTrackID);
            int selection = 0;
            for(int i = 0; i < audio_buffers.size(); i++)
            {
                if(audio_segment->mAudioBufferID == audio_buffers[i])
                {
                    selection = i;
                }
            }

            /**
             * Select audio buffer available in output
             */
            if(Combo("Audio Buffer", &selection, audio_buffers))
            {
                take_snapshot();

                audio_controller.changeAudioSegmentAudioBuffer(action->mTrackID, action->mSegmentID, audio_buffers[selection]);
                mState.mDirty = true;
            }

            /**
             * Handle delete
             */
            auto &gui = mService.getGui();
            if(ImGui::ImageButton(gui.getIcon(icon::del)))
            {
                take_snapshot();

                audio_controller.deleteSegment(action->mTrackID, action->mSegmentID);
                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                mState.mDirty = true;
                ImGui::CloseCurrentPopup();
            }

            /**
             * Handle exit
             */
            ImGui::SameLine();
            if(ImGui::ImageButton(gui.getIcon(icon::ok)))
            {
                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                ImGui::CloseCurrentPopup();
            }

            action->mWindowPos = ImGui::GetWindowPos();

            ImGui::EndPopup();
        } else
        {
            // click outside popup so cancel action
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    SequencePlayerAudioOutput* SequenceAudioTrackView::getAudioOutputForTrack(const std::string& trackID)
    {
        // get audio controller
        auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

        // get assigned output
        const auto &sequence_player = getPlayer();
        SequencePlayerOutput *output = nullptr;
        for(const auto &player_output: sequence_player.mOutputs)
        {
            const auto *track = audio_controller.getTrack(trackID);
            assert(track != nullptr);
            if(player_output->mID == track->mAssignedOutputID)
            {
                output = player_output.get();
            }
        }

        // upcast to audio output
        SequencePlayerAudioOutput *audio_output = nullptr;
        if(output != nullptr)
        {
            assert(output->get_type().is_derived_from<SequencePlayerAudioOutput>()); // type mismatch
            audio_output = static_cast<SequencePlayerAudioOutput *>(output);
        }

        return audio_output;
    }


    std::vector<std::string> SequenceAudioTrackView::getAudioBuffersForTrack(const std::string& trackID)
    {
        // get audio controller
        auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

        auto *audio_output = getAudioOutputForTrack(trackID);
        std::vector<std::string> audio_buffers;
        if(audio_output != nullptr)
        {
            for(const auto &audio_buffer: audio_output->getBuffers())
            {
                audio_buffers.emplace_back(audio_buffer->mID);
            }
        }

        return audio_buffers;
    }


    void SequenceAudioTrackView::handleLeftHandlerDrag()
    {
        if(mState.mAction->isAction<DraggingLeftAudioSegmentHandler>())
        {
            // get the derived action
            auto *action = mState.mAction->getDerived<DraggingLeftAudioSegmentHandler>();

            // take snapshot
            if(action->mTakeSnapshot)
            {
                getEditor().takeSnapshot(action->get_type());
                action->mTakeSnapshot = false;
            }

            // get audio controller
            auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

            // obtain the audio segment
            const auto *segment = audio_controller.getSegment(action->mTrackID, action->mSegmentID);
            assert(segment->get_type().is_derived_from<SequenceTrackSegmentAudio>()); // type mismatch
            const auto *segment_audio = static_cast<const SequenceTrackSegmentAudio *>(segment);

            // adjust time in segment & start time
            float amount = mState.mMouseDelta.x / mState.mStepSize;
            double new_start_time_in_segment = segment_audio->mStartTimeInAudioSegment + amount;
            double adjusted_new_time_in_segment = audio_controller.segmentAudioStartTimeInSegmentChange(action->mTrackID, action->mSegmentID, new_start_time_in_segment);

            // if time is adjusted, also adjust the start time
            if(adjusted_new_time_in_segment == new_start_time_in_segment)
            {
                double new_start_time = segment_audio->mStartTime + amount;
                audio_controller.segmentAudioStartTimeChange(action->mTrackID, action->mSegmentID, new_start_time);
            }

            // mark state as dirty
            mState.mDirty = true;

            // release
            if(ImGui::IsMouseReleased(0))
            {
                mState.mAction = createAction<None>();
            }
        }
    }


    void SequenceAudioTrackView::handleRightHandlerDrag()
    {
        if(mState.mAction->isAction<DraggingRightAudioSegmentHandler>())
        {
            // get the derived action
            auto *action = mState.mAction->getDerived<DraggingRightAudioSegmentHandler>();

            // take snapshot
            if(action->mTakeSnapshot)
            {
                getEditor().takeSnapshot(action->get_type());
                action->mTakeSnapshot = false;
            }

            // get audio controller
            auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

            // obtain the audio segment
            const auto *segment = audio_controller.getSegment(action->mTrackID, action->mSegmentID);
            assert(segment->get_type().is_derived_from<SequenceTrackSegmentAudio>()); // type mismatch
            const auto *segment_audio = static_cast<const SequenceTrackSegmentAudio *>(segment);

            // adjust time in segment & start time
            float amount = mState.mMouseDelta.x / mState.mStepSize;
            float new_duration = segment_audio->mDuration + amount;
            audio_controller.segmentAudioDurationChange(action->mTrackID, action->mSegmentID, new_duration);

            // mark state as dirty
            mState.mDirty = true;

            // release
            if(ImGui::IsMouseReleased(0))
            {
                mState.mAction = createAction<None>();
            }
        }
    }


    void SequenceAudioTrackView::handleLoadPresetPopup()
    {
        if(mState.mAction->isAction<LoadPresetPopup>())
        {
            auto *load_action = mState.mAction->getDerived<LoadPresetPopup>();
            auto *controller = getEditor().getControllerWithTrackType(load_action->mTrackType);

            if(controller->get_type().is_derived_from<SequenceControllerAudio>())
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
                    if(ImGui::ImageButton(gui.getIcon(icon::ok), "Done"))
                    {
                        getEditor().takeSnapshot(load_action->get_type()); 
                        if(mState.mClipboard->load(preset_files[load_action->mSelectedPresetIndex], error_state))
                        {
                            if(pasteClipboard(load_action->mTrackID, load_action->mTime, error_state))
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


    bool SequenceAudioTrackView::pasteClipboard(const std::string& trackID, double time, utility::ErrorState& errorState)
    {
        auto &audio_controller = getEditor().getController<SequenceControllerAudio>();

        std::vector<std::unique_ptr<rtti::Object>> deserialized_objects;
        auto deserialized_audio_segments = mState.mClipboard->deserialize<SequenceTrackSegmentAudio>(deserialized_objects, errorState);
        if(!errorState.hasErrors())
        {
            // if success on deserialization, get the first segment start time
            double first_segment_time = math::max<double>();

            for (const auto* deserialized_audio_segment: deserialized_audio_segments)
            {
                if(deserialized_audio_segment->mStartTime < first_segment_time)
                {
                    first_segment_time = deserialized_audio_segment->mStartTime;
                }
            }

            // do appropriate calls to the controller to copy deserialized segments
            for (const auto* deserialized_audio_segment: deserialized_audio_segments)
            {
                double time_offset = deserialized_audio_segment->mStartTime - first_segment_time;
                assert(time_offset >= 0.0);

                auto new_segment_id = audio_controller.insertAudioSegment(trackID, time +
                                                                                   time_offset, deserialized_audio_segment->mAudioBufferID, deserialized_audio_segment->mDuration, deserialized_audio_segment->mStartTimeInAudioSegment);

                audio_controller.segmentAudioDurationChange(trackID, new_segment_id, deserialized_audio_segment->mDuration);
                audio_controller.segmentAudioStartTimeInSegmentChange(trackID, new_segment_id, deserialized_audio_segment->mStartTimeInAudioSegment);
                mState.mDirty = true;
            }
        } else
        {
            return false;
        }

        return true;
    }
}

