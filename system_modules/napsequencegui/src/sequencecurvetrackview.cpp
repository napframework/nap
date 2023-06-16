/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencecurvetrackview.h"
#include "sequenceeditorgui.h"
#include "sequenceplayercurveoutput.h"
#include "sequenceguiutils.h"

// nap includes
#include <nap/logger.h>
#include <parametervec.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <imguiutils.h>

// external includes
#include <iostream>


namespace nap
{
    using namespace sequenceguiactions;
    using namespace sequenceguiclipboard;


    SequenceCurveTrackView::SequenceCurveTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state)
        : SequenceTrackView(view, state)
    {
        registerActionHandler(RTTI_OF(OpenInsertSegmentPopup), std::bind(&SequenceCurveTrackView::handleInsertSegmentPopup, this));
        registerActionHandler(RTTI_OF(InsertingSegmentPopup), std::bind(&SequenceCurveTrackView::handleInsertSegmentPopup, this));
        registerActionHandler(RTTI_OF(OpenInsertCurvePointPopup), std::bind(&SequenceCurveTrackView::handleInsertCurvePointPopup, this));
        registerActionHandler(RTTI_OF(InsertingCurvePoint), std::bind(&SequenceCurveTrackView::handleInsertCurvePointPopup, this));
        registerActionHandler(RTTI_OF(OpenCurveTypePopup), std::bind(&SequenceCurveTrackView::handleCurveTypePopup, this));
        registerActionHandler(RTTI_OF(CurveTypePopup), std::bind(&SequenceCurveTrackView::handleCurveTypePopup, this));
        registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<float>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<float>, this));
        registerActionHandler(RTTI_OF(CurvePointActionPopup<float>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<float>, this));
        registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec2>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec2>, this));
        registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec2>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec2>, this));
        registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec3>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec3>, this));
        registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec3>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec3>, this));
        registerActionHandler(RTTI_OF(OpenCurvePointActionPopup<glm::vec4>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec4>, this));
        registerActionHandler(RTTI_OF(CurvePointActionPopup<glm::vec4>), std::bind(&SequenceCurveTrackView::handleCurvePointActionPopup<glm::vec4>, this));
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
        registerActionHandler(RTTI_OF(ShowLoadPresetPopup), std::bind(&SequenceCurveTrackView::handleLoadPresetPopup, this));
    }


    std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr>& SequenceCurveTrackView::getDrawCurveSegmentsMap()
    {
        static std::unordered_map<rttr::type, SequenceCurveTrackView::DrawSegmentMemFunPtr> map =
            {
                {RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::drawSegmentContent<float>},
                {RTTI_OF(SequenceTrackSegmentCurveVec2),  &SequenceCurveTrackView::drawSegmentContent<glm::vec2>},
                {RTTI_OF(SequenceTrackSegmentCurveVec3),  &SequenceCurveTrackView::drawSegmentContent<glm::vec3>},
                {RTTI_OF(SequenceTrackSegmentCurveVec4),  &SequenceCurveTrackView::drawSegmentContent<glm::vec4>}
            };

        return map;
    };


    static std::unordered_map<rttr::type, std::vector<rttr::type>> parameter_types_for_curve_types
        {
            {RTTI_OF(SequenceTrackCurveFloat), {{RTTI_OF(ParameterFloat), RTTI_OF(ParameterDouble), RTTI_OF(ParameterLong), RTTI_OF(ParameterInt), RTTI_OF(ParameterBool)}}},
            {RTTI_OF(SequenceTrackCurveVec2),  {{RTTI_OF(ParameterVec2)}}},
            {RTTI_OF(SequenceTrackCurveVec3),  {{RTTI_OF(ParameterVec3)}}},
			{RTTI_OF(SequenceTrackCurveVec4),  {{RTTI_OF(ParameterVec4)}}}
        };


    static bool isParameterTypeAllowed(const rttr::type& curveType, const rttr::type& parameterType)
    {
        auto it = parameter_types_for_curve_types.find(curveType);
        if(it != parameter_types_for_curve_types.end())
        {
            for(auto &type: parameter_types_for_curve_types[curveType])
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
        if(mState.mAction->isAction<StartDraggingSegment>())
        {
            auto *action = mState.mAction->getDerived<StartDraggingSegment>();
            mState.mAction = createAction<DraggingSegment>(action->mTrackID, action->mSegmentID, action->mStartDuration);
        }

        SequenceTrackView::handleActions();
    }


    void SequenceCurveTrackView::showInspectorContent(const SequenceTrack& track)
    {
        // draw the assigned parameter
        ImGui::Text("Assigned Output");

        // give the inspector cursor a small offset
        ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
        float inspector_offset = mState.mScale * 5.0f;
        inspector_cursor_pos.x += inspector_offset;
        inspector_cursor_pos.y += inspector_offset;
        ImGui::SetCursorPos(inspector_cursor_pos);

        // declare local variables for output assignment operations
        std::string assigned_id;
        std::vector<std::string> curve_outputs;
        int current_item = 0;
        int count = 0;

        // track height
        const float track_height = track.mTrackHeight * mState.mScale;

        curve_outputs.emplace_back("none");

        // gather all available outputs for this track type
        for(const auto &input: getEditor().mSequencePlayer->mOutputs)
        {
            if(input.get()->get_type() == RTTI_OF(SequencePlayerCurveOutput))
            {
                auto &curve_output = static_cast<SequencePlayerCurveOutput &>(*input.get());

                if(curve_output.mParameter != nullptr)
                {
                    if(isParameterTypeAllowed(track.get_type(), curve_output.mParameter.get()->get_type()))
                    {
                        count++;

                        if(input->mID == track.mAssignedOutputID)
                        {
                            assigned_id = input->mID;
                            current_item = count;

                            assert(input.get()->get_type() == RTTI_OF(SequencePlayerCurveOutput)); // type mismatch
                        }

                        curve_outputs.emplace_back(input->mID);
                    }
                }
            }
        }

        // create dropdown of collected outputs and create action if new output is assigned
        const float inspector_item_width = 200.0f;
        ImGui::PushItemWidth(inspector_item_width * mState.mScale);
        if(Combo(
            "",
            &current_item, curve_outputs))
        {
            if(current_item != 0)
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, curve_outputs[current_item]);
            else
                mState.mAction = sequenceguiactions::createAction<AssignOutputIDToTrack>(track.mID, "");
        }

        //
        ImGui::PopItemWidth();

        // map of inspectors ranges for curve types
        static std::unordered_map<rttr::type, void (SequenceCurveTrackView::*)(const SequenceTrack &)> inspectors
            {
                {RTTI_OF(SequenceTrackCurveFloat), &SequenceCurveTrackView::drawInspectorRange<float>},
                {RTTI_OF(SequenceTrackCurveVec2),  &SequenceCurveTrackView::drawInspectorRange<glm::vec2>},
                {RTTI_OF(SequenceTrackCurveVec3),  &SequenceCurveTrackView::drawInspectorRange<glm::vec3>},
                {RTTI_OF(SequenceTrackCurveVec4),  &SequenceCurveTrackView::drawInspectorRange<glm::vec4>}
            };

        // draw inspector
        auto it = inspectors.find(track.get_type());
        assert(it != inspectors.end()); // type not found
        if(it != inspectors.end())
        {
            (*this.*it->second)(track);
        }

        // delete track button
        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + inspector_offset);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + inspector_offset);
    }


    void SequenceCurveTrackView::showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft)
    {
        // if dirty, redraw all curves
        if(mState.mDirty)
        {
            mCurveCache.clear();
        }

        // track height
        const float track_height = track.mTrackHeight * mState.mScale;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        if(mState.mIsWindowFocused)
        {
            // handle insertion of segment
            if(mState.mAction->isAction<None>())
            {
                if(ImGui::IsMouseHoveringRect(
                    trackTopLeft, // top left position
                    {trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + track_height}))
                {
                    // position of mouse in track
                    draw_list->AddLine
                        (
                            {mState.mMousePos.x, trackTopLeft.y}, // top left
                            {mState.mMousePos.x, trackTopLeft.y + track_height}, // bottom right
                            mService.getColors().mFro2, // color
                            1.0f * mState.mScale // thickness
                        );

                    ImGui::BeginTooltip();

                    // display segment label
                    for(const auto &segment: track.mSegments)
                    {
                        float segment_start = (float) segment->mStartTime * mState.mStepSize;
                        float segment_width = (float) segment->mDuration * mState.mStepSize;

                        if(mState.mMousePos.x > trackTopLeft.x + segment_start &&
                           mState.mMousePos.x < trackTopLeft.x + segment_start + segment_width)
                        {
                            ImGui::Text(segment->mLabel.c_str());
                            break;
                        }
                    }

                    // display cursor time
                    ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());

                    ImGui::EndTooltip();

                    // right mouse down
                    if(ImGui::IsMouseClicked(1))
                    {
                        double time = mState.mMouseCursorTime;

                        //
                        mState.mAction = createAction<OpenInsertSegmentPopup>(track.mID, time, track.get_type());
                    }
                }
            }

            // draw line in track while in inserting segment popup
            if(mState.mAction->isAction<OpenInsertSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

                if(action->mTrackID == track.mID)
                {
                    // position of insertion in track
                    draw_list->AddLine
                        (
                            {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y}, // top left
                            {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                             trackTopLeft.y + track_height}, // bottom right
                            mService.getColors().mFro2, // color
                            1.0f * mState.mScale // thickness
                        );
                }
            }

            // draw line in track while in inserting segment popup
            if(mState.mAction->isAction<InsertingSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertingSegmentPopup>();

                if(action->mTrackID == track.mID)
                {
                    // position of insertion in track
                    draw_list->AddLine
                        (
                            {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y}, // top left
                            {trackTopLeft.x + (float) action->mTime * mState.mStepSize,
                             trackTopLeft.y + track_height}, // bottom right
                            mService.getColors().mFro2, // color
                            1.0f * mState.mScale  // thickness
                        );
                }
            }
        }

        float previous_segment_x = 0.0f;
        int segment_count = 0;
        for(const auto &segment: track.mSegments)
        {
            const auto *segment_ptr = segment.get();
            float segment_x = (float) (segment->mStartTime + segment->mDuration) * mState.mStepSize;
            float segment_width = (float) segment->mDuration * mState.mStepSize;

            // draw segment handlers
            drawSegmentHandler(
                track,
                *segment_ptr,
                trackTopLeft, segment_x, segment_width, draw_list);

            // draw segment content
            auto it = getDrawCurveSegmentsMap().find(segment_ptr->get_type());
            if(it != getDrawCurveSegmentsMap().end())
            {
                (*this.*
                 it->second)(track, *segment_ptr, trackTopLeft, previous_segment_x, segment_width, segment_x, draw_list, (
                    segment_count == 0));
            }

            previous_segment_x = segment_x;
            segment_count++;
        }
    }


    void SequenceCurveTrackView::drawSegmentHandler(
            const SequenceTrack& track,
            const SequenceTrackSegment& segment,
            const ImVec2& trackTopLeft,
            const float segmentX,
            const float segmentWidth,
            ImDrawList* drawList)
    {
        // const values
        const float seg_bounds = 10.0f * mState.mScale;
        const float line_thickness_regular = 1.0f * mState.mScale;
        const float line_thickness_active = 3.0f * mState.mScale;
        const float track_height = track.mTrackHeight * mState.mScale;

        // check if user is hovering or dragging the handler of this segment
        if(mState.mIsWindowFocused
           && ((mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringSegment>()) ||
               (mState.mAction->isAction<StartDraggingSegment>() &&
                mState.mAction->getDerived<StartDraggingSegment>()->mSegmentID != segment.mID))
           && ImGui::IsMouseHoveringRect(
            {trackTopLeft.x + segmentX - seg_bounds, trackTopLeft.y - seg_bounds}, // top left
            {trackTopLeft.x + segmentX + seg_bounds, trackTopLeft.y + track_height + seg_bounds}))  // bottom right
        {

            // draw handler of segment duration
            drawList->AddLine
                (
                    {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                    {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                    mService.getColors().mFro4, // color
                    3.0f * mState.mScale // thickness
                );

            // we are hovering this segment with the mouse
            mState.mAction = createAction<HoveringSegment>(track.mID, segment.mID);

            // show timestamp
            ImGui::BeginTooltip();
            ImGui::Text(formatTimeString(segment.mStartTime + segment.mDuration).c_str());
            ImGui::EndTooltip();

            // left mouse is start dragging
            if(ImGui::IsMouseDown(0))
            {
                mState.mAction = createAction<StartDraggingSegment>(track.mID, segment.mID, segment.mDuration);
            }
                // right mouse is edit popup
            else if(ImGui::IsMouseDown(1))
            {
                mState.mAction = createAction<OpenEditCurveSegmentPopup>(
                    track.mID,
                    segment.mID,
                    segment.get_type(),
                    segment.mStartTime,
                    segment.mDuration,
                    segment.mLabel
                );
            }

            // if mouse is clicked, check if shift is down, if so, handle clipboard actions (add or remove)
            if(ImGui::IsMouseClicked(0))
            {
                if(ImGui::GetIO().KeyShift)
                {
                    // create new curve segment clipboard if necessary or when clipboard is from different sequence
                    if(!mState.mClipboard->isClipboard<CurveSegmentClipboard>())
                        mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename());

                    // is this a different track then previous clipboard ? then create a new clipboard, discarding the old clipboard
                    auto *clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
                    if(clipboard->getTrackID() != track.mID)
                    {
                        mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename());
                        clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
                    }

                    // is clipboard from another sequence, create new clipboard
                    if(clipboard->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename())
                    {
                        mState.mClipboard = createClipboard<CurveSegmentClipboard>(track.get_type(), track.mID, getEditor().mSequencePlayer->getSequenceFilename());
                        clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
                    }

                    // see if the clipboard already contains this segment, if so, remove it, if not, add it
                    if(mState.mClipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()))
                    {
                        mState.mClipboard->removeObject(segment.mID);
                    } else
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
        } else if(mState.mAction->isAction<DraggingSegment>())
        {
            auto *action = mState.mAction->getDerived<DraggingSegment>();
            if(action->mSegmentID == segment.mID)
            {
                // draw handler of segment duration
                drawList->AddLine
                    (
                        {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                        {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                        mService.getColors().mFro4, // color
                        line_thickness_active // thickness
                    );
                ImGui::BeginTooltip();
                ImGui::Text(segment.mLabel.c_str());
                ImGui::Text(formatTimeString(segment.mStartTime + segment.mDuration).c_str());
                ImGui::EndTooltip();
            } else
            {
                // draw handler of segment duration
                drawList->AddLine
                    (
                        {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                        {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                        mService.getColors().mFro4, // color
                        line_thickness_regular // thickness
                    );
            }
        } else
        {
            // draw handler of segment duration
            drawList->AddLine
                (
                    {trackTopLeft.x + segmentX, trackTopLeft.y}, // top left
                    {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // bottom right
                    mService.getColors().mFro4, // color
                    line_thickness_regular // thickness
                );

            // release if we are not hovering this segment
            if(mState.mAction->isAction<HoveringSegment>()
               && mState.mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID)
            {
                mState.mAction = createAction<None>();
            }
        }
    }


    void SequenceCurveTrackView::handleInsertSegmentPopup()
    {
        static const std::vector<rtti::TypeInfo> allowed_types =
                {RTTI_OF(SequenceTrackCurveFloat),
                 RTTI_OF(SequenceTrackCurveVec2),
                 RTTI_OF(SequenceTrackCurveVec3),
                 RTTI_OF(SequenceTrackCurveVec4)};

        if(mState.mAction->isAction<OpenInsertSegmentPopup>())
        {
            auto *action = mState.mAction->getDerived<OpenInsertSegmentPopup>();

            auto it = std::find(allowed_types.begin(), allowed_types.end(), action->mTrackType);
            if(it != allowed_types.end())
            {
                // invoke insert sequence popup
                ImGui::OpenPopup("Insert Segment");

                mState.mAction = createAction<InsertingSegmentPopup>(action->mTrackID, action->mTime, action->mTrackType);
            }
        }

        // handle insert segment popup
        if(mState.mAction->isAction<InsertingSegmentPopup>())
        {
            auto *action = mState.mAction->getDerived<InsertingSegmentPopup>();

            auto it = std::find(allowed_types.begin(), allowed_types.end(), action->mTrackType);
            if(it != allowed_types.end())
            {
                if(ImGui::BeginPopup("Insert Segment"))
                {
                    std::string btn_msg =
                        "Insert " + utility::stripNamespace(action->mTrackType.get_name().to_string());
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert), btn_msg.data()))
                    {
                        auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                        curve_controller.insertSegment(action->mTrackID, action->mTime);
                        updateSegmentsInClipboard(action->mTrackID);
                        mState.mAction = createAction<None>();

                        mCurveCache.clear();

                        ImGui::CloseCurrentPopup();
                    }

                    // handle paste
                    if(mState.mClipboard->isClipboard<CurveSegmentClipboard>() &&
                       mState.mClipboard->getObjectCount() > 0)
                    {
                        ImGui::SameLine();
                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::paste)))
                        {
                            static const std::unordered_map<rtti::TypeInfo, bool (SequenceCurveTrackView::*)(const std::string &, double, utility::ErrorState &)> paste_map =
                                {{RTTI_OF(SequenceTrackCurveFloat), &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveFloat>},
                                 {RTTI_OF(SequenceTrackCurveVec2),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec2>},
                                 {RTTI_OF(SequenceTrackCurveVec3),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec3>},
                                 {RTTI_OF(SequenceTrackCurveVec4),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec4>}};

                            // call the correct pasteClipboardSegments method for this track-type
                            utility::ErrorState error_state;
                            auto paste_map_it = paste_map.find(action->mTrackType);
                            assert(paste_map_it != paste_map.end()); // type not found
                            if((*this.*paste_map_it->second)(action->mTrackID, action->mTime, error_state))
                            {
                                mState.mDirty = true;
                                mState.mAction = createAction<None>();
                                ImGui::CloseCurrentPopup();
                            } else
                            {
                                ImGui::OpenPopup("Error");
                                action->mErrorString = error_state.toString();
                            }
                        }
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::load), "Load preset"))
                    {
                        if(mState.mClipboard->getTrackType() != action->mTrackType)
                        {
                            mState.mClipboard = createClipboard<CurveSegmentClipboard>(action->mTrackType,
                                                                                       action->mTrackID,
                                                                                       getEditor().mSequencePlayer->getSequenceFilename());
                        }
                        mState.mAction = createAction<ShowLoadPresetPopup>(action->mTrackID, action->mTime, action->mTrackType);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::cancel)))
                    {
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

                    ImGui::EndPopup();
                } else
                {
                    // click outside popup so cancel action
                    mState.mAction = createAction<None>();
                }
            }
        }
    }


    void SequenceCurveTrackView::handleCurveTypePopup()
    {
        if(mState.mAction->isAction<OpenCurveTypePopup>())
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Change Curve Type");

            auto *action = mState.mAction->getDerived<OpenCurveTypePopup>();
            mState.mAction = createAction<CurveTypePopup>(
                action->mTrackID,
                action->mSegmentID,
                action->mCurveIndex,
                action->mPos,
                action->mWindowPos);
        }

        // handle insert segment popup
        if(mState.mAction->isAction<CurveTypePopup>())
        {
            auto *action = mState.mAction->getDerived<CurveTypePopup>();

            if(ImGui::BeginPopup("Change Curve Type"))
            {
                ImGui::SetWindowPos(action->mWindowPos);

                if(ImGui::Button("Linear"))
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Linear, action->mCurveIndex);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                    mCurveCache.clear();
                }
                ImGui::SameLine();
                if(ImGui::Button("Bezier"))
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Bezier, action->mCurveIndex);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                    mCurveCache.clear();
                }
                ImGui::SameLine();
                if(ImGui::Button("Stepped"))
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.changeCurveType(action->mTrackID, action->mSegmentID, math::ECurveInterp::Stepped, action->mCurveIndex);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                    ImGui::CloseCurrentPopup();
                    mState.mAction = createAction<None>();
                    mCurveCache.clear();
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


    void SequenceCurveTrackView::handleInsertCurvePointPopup()
    {
        if(mState.mAction->isAction<OpenInsertCurvePointPopup>())
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Insert Curve Point");

            auto *action = mState.mAction->getDerived<OpenInsertCurvePointPopup>();
            mState.mAction = createAction<InsertingCurvePoint>(
                action->mTrackID,
                action->mSegmentID,
                action->mSelectedIndex,
                action->mPos);
        }

        // handle insert segment popup
        if(mState.mAction->isAction<InsertingCurvePoint>())
        {
            if(ImGui::BeginPopup("Insert Curve Point"))
            {
                auto *action = mState.mAction->getDerived<InsertingCurvePoint>();
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::add), "Insert Point"))
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
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

                ImGui::SameLine();
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::change), "Change Curve Type"))
                {
                    ImGui::CloseCurrentPopup();

                    mState.mAction = createAction<OpenCurveTypePopup>(
                        action->mTrackID,
                        action->mSegmentID,
                        action->mSelectedIndex,
                        action->mPos,
                        ImGui::GetWindowPos());
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


    void SequenceCurveTrackView::handleTanPointActionPopup()
    {
        if(mState.mAction->isAction<sequenceguiactions::OpenEditTanPointPopup>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::OpenEditTanPointPopup>();
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::EditingTanPointPopup>(
                action->mTrackID,
                action->mSegmentID,
                action->mControlPointIndex,
                action->mCurveIndex,
                action->mType,
                action->mValue,
                action->mTime);

            ImGui::OpenPopup("Tan Point Actions");
        }

        if(mState.mAction->isAction<EditingTanPointPopup>())
        {
            if(ImGui::BeginPopup("Tan Point Actions"))
            {
                auto *action = mState.mAction->getDerived<sequenceguiactions::EditingTanPointPopup>();
                int curveIndex = action->mCurveIndex;

                bool change = false;
                if(ImGui::InputFloat("time", &action->mTime))
                {
                    change = true;

                    // time cannot be 0
                    action->mTime = math::max<float>(0.1f, action->mTime);
                }

                if(ImGui::InputFloat("value", &action->mValue))
                {
                    change = true;
                }

                if(change)
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();

                    bool tangents_flipped = curve_controller.changeTanPoint(
                        action->mTrackID,
                        action->mSegmentID,
                        action->mControlPointIndex,
                        action->mCurveIndex,
                        action->mType,
                        action->mTime,
                        action->mValue);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                    if(tangents_flipped && action->mType == sequencecurveenums::ETanPointTypes::IN)
                    {
                        action->mType = sequencecurveenums::ETanPointTypes::OUT;
                    } else if(tangents_flipped && action->mType == sequencecurveenums::ETanPointTypes::OUT)
                    {
                        action->mType = sequencecurveenums::ETanPointTypes::IN;
                    }

                    mState.mDirty = true;
                }

                if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
                {
                    mState.mAction = createAction<sequenceguiactions::None>();

                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            } else
            {
                // click outside popup so cancel action
                mState.mAction = createAction<None>();
            }
        }
    }


    void SequenceCurveTrackView::handleEditSegmentPopup()
    {
        if(mState.mAction->isAction<OpenEditCurveSegmentPopup>())
        {
            // invoke insert sequence popup
            ImGui::OpenPopup("Edit Segment");

            auto *action = mState.mAction->getDerived<OpenEditCurveSegmentPopup>();

            mState.mAction = createAction<EditingCurveSegment>(
                action->mTrackID,
                action->mSegmentID,
                action->mSegmentType,
                action->mStartTime,
                action->mDuration,
                action->mSegmentLabel
            );
        }

        // handle edit segment popup
        if(mState.mAction->isAction<EditingCurveSegment>())
        {
            auto *action = mState.mAction->getDerived<EditingCurveSegment>();
            auto &controller = getEditor().getController<SequenceControllerCurve>();
            auto *track = controller.getTrack(action->mTrackID);

            static const std::vector<rtti::TypeInfo> allowed_types =
                {RTTI_OF(SequenceTrackCurveFloat),
                 RTTI_OF(SequenceTrackCurveVec2),
                 RTTI_OF(SequenceTrackCurveVec3),
                 RTTI_OF(SequenceTrackCurveVec4)};

            auto allowed_types_it = std::find(allowed_types.begin(), allowed_types.end(), track->get_type());
            if(allowed_types_it != allowed_types.end())
            {
                if(ImGui::BeginPopup("Edit Segment"))
                {
                    // see if we have a curve segment in the clipboard
                    bool display_copy = !mState.mClipboard->isClipboard<CurveSegmentClipboard>();
                    if(!display_copy)
                    {
                        // yes ? see if the track is the same
                        auto *clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

                        // if no, display copy, which will override the existing clipboard
                        if(clipboard->getTrackID() != track->mID)
                        {
                            display_copy = true;
                        }

                        // check if clipboard is from different sequence
                        if(clipboard->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename())
                        {
                            display_copy = true;
                        }
                    }

                    if(display_copy)
                    {
                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::copy), "Copy"))
                        {
                            // create new clipboard
                            mState.mClipboard = createClipboard<CurveSegmentClipboard>(track->get_type(), track->mID, getEditor().mSequencePlayer->getSequenceFilename());

                            // get curve segment
                            const auto *curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);

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
                        ImGui::SameLine();
                    }

                    if(!display_copy)
                    {
                        // get clipboard
                        auto *clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

                        // get curve segment
                        const auto *curve_segment = controller.getSegment(action->mTrackID, action->mSegmentID);

                        // does the clipboard already contain this segment ?
                        if(clipboard->containsObject(curve_segment->mID, getPlayer().getSequenceFilename()))
                        {
                            if(ImGui::ImageButton(mService.getGui().getIcon(icon::remove), "Remove from clipboard"))
                            {
                                // remove the object from the clipboard
                                clipboard->removeObject(curve_segment->mID);

                                // if clipboard is empty, create empty clipboard
                                if(clipboard->getObjectCount() == 0)
                                {
                                    mState.mClipboard = createClipboard<Empty>();
                                }

                                // exit popup
                                ImGui::CloseCurrentPopup();
                                mState.mAction = createAction<None>();
                            }
                        } else
                        {
                            if(ImGui::ImageButton(mService.getGui().getIcon(icon::copy), "Add to clipboard"))
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
                        ImGui::SameLine();
                    }

                    // see if we can replace the contents of this segment with the one from the clipboard
                    // this can only happen when we have 1 segment in the clipboard
                    bool display_replace = mState.mClipboard->isClipboard<CurveSegmentClipboard>() && mState.mClipboard->getObjectCount();
                    if(display_replace)
                    {
                        display_replace = mState.mClipboard->getObjectCount() == 1;
                    }

                    // display the replace option
                    if(display_replace)
                    {
                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::paste), "Paste into this"))
                        {
                            static const std::unordered_map<rtti::TypeInfo, void (SequenceCurveTrackView::*)(const std::string &, const std::string &)> paste_map =
                                {{RTTI_OF(SequenceTrackSegmentCurveFloat), &SequenceCurveTrackView::pasteClipboardSegmentInto<SequenceTrackSegmentCurveFloat>},
                                 {RTTI_OF(SequenceTrackSegmentCurveVec2),  &SequenceCurveTrackView::pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec2>},
                                 {RTTI_OF(SequenceTrackSegmentCurveVec3),  &SequenceCurveTrackView::pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec3>},
                                 {RTTI_OF(SequenceTrackSegmentCurveVec4),  &SequenceCurveTrackView::pasteClipboardSegmentInto<SequenceTrackSegmentCurveVec4>}};

                            // call the correct pasteClipboardSegments method for this segment-type
                            auto paste_map_it = paste_map.find(action->mSegmentType);
                            assert(paste_map_it != paste_map.end());
                            (*this.*paste_map_it->second)(action->mTrackID, action->mSegmentID);

                            // redraw cached curves
                            mState.mDirty = true;

                            // exit popup
                            ImGui::CloseCurrentPopup();
                            mState.mAction = createAction<None>();
                        }
                        ImGui::SameLine();
                    }

                    if(ImGui::ImageButton(mService.getGui().getIcon(icon::del), "Delete"))
                    {
                        controller.deleteSegment(
                            action->mTrackID,
                            action->mSegmentID);
                        mCurveCache.clear();

                        // remove delete object from clipboard
                        if(mState.mClipboard->containsObject(action->mSegmentID, getPlayer().getSequenceFilename()))
                        {
                            mState.mClipboard->removeObject(action->mSegmentID);
                        }
                        updateSegmentsInClipboard(action->mTrackID);

                        // exit popup
                        ImGui::CloseCurrentPopup();
                        mState.mAction = createAction<None>();
                    }

                    // Ensure the action is valid
                    if(!mState.mAction->isAction<None>())
                    {
                        ImGui::Separator();
                        ImGui::PushItemWidth(100.0f * mState.mScale);

                        // show segment label
                        std::string label_copy = action->mSegmentLabel;
                        label_copy.resize(32);

                        bool edit_label = ImGui::InputText("Label", &label_copy[0], 32);

                        if(edit_label)
                        {
                            controller.changeSegmentLabel(action->mTrackID, action->mSegmentID, label_copy);

                            // update any segments we have in the clipboard
                            updateSegmentsInClipboard(action->mTrackID);

                            action->mSegmentLabel = label_copy;
                            mState.mDirty = true;
                        }

                        std::vector<int> time_array = convertTimeToMMSSMSArray(action->mDuration + action->mStartTime);
                        bool edit_time = false;

                        edit_time = ImGui::InputInt3("End time (mm:ss:ms)", &time_array[0]);
                        time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
                        time_array[1] = math::clamp<int>(time_array[1], 0, 59);
                        time_array[2] = math::clamp<int>(time_array[2], 0, 99);

                        if(edit_time)
                        {
                            double new_time = convertMMSSMSArrayToTime(time_array);
                            double new_duration = controller.segmentDurationChange(action->mTrackID, action->mSegmentID, (float) (
                                new_time - action->mStartTime));

                            // make the controller re-align start & end points of segments
                            controller.updateCurveSegments(action->mTrackID);

                            updateSegmentsInClipboard(action->mTrackID);

                            action->mDuration = new_duration;
                            mState.mDirty = true;
                            mCurveCache.clear();
                        }

                        ImGui::PopItemWidth();
                        ImGui::Separator();
                        if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok), "Done"))
                        {
                            ImGui::CloseCurrentPopup();
                            mState.mAction = createAction<None>();
                        }
                    }
                    ImGui::EndPopup();
                } else
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
        const auto &curve_track = static_cast<const SequenceTrackCurve<float> &>(track);

        ImGui::BeginTooltip();

        ImGui::Text(formatTimeString(time).c_str());
        ImGui::Text("%.3f", segment.getValue(x) * (curve_track.mMaximum - curve_track.mMinimum) + curve_track.mMinimum);

        ImGui::EndTooltip();
    }


    template<>
    void SequenceCurveTrackView::showValue<glm::vec2>(
        const SequenceTrack &track,
        const SequenceTrackSegmentCurve<glm::vec2> &segment,
        float x,
        double time,
        int curveIndex)
    {
        assert(curveIndex >= 0);
        assert(curveIndex < 2);

        assert(track.get_type().is_derived_from<SequenceTrackCurve<glm::vec2>>());
        const auto &curve_track = static_cast<const SequenceTrackCurve<glm::vec2> &>(track);

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
        const auto &curve_track = static_cast<const SequenceTrackCurve<glm::vec3> &>(track);

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
        const auto &curve_track = static_cast<const SequenceTrackCurve<glm::vec4> &>(track);

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
    bool SequenceCurveTrackView::inputFloat<float>(float& v, int precision)
    {
        ImGui::PushItemWidth(100.0f * mState.mScale);
        return ImGui::InputFloat("", &v, 0.0f, 0.0f, precision);
    }


    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2& v, int precision)
    {
        ImGui::PushItemWidth(145.0f * mState.mScale);
        return ImGui::InputFloat2("", &v[0], precision);
    }


    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3& v, int precision)
    {
        ImGui::PushItemWidth(180.0f * mState.mScale);
        return ImGui::InputFloat3("", &v[0], precision);
    }


    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4& v, int precision)
    {
        ImGui::PushItemWidth(225.0f * mState.mScale);
        return ImGui::InputFloat4("", &v[0], precision);
    }


    template<>
    void SequenceCurveTrackView::handleCurvePointActionPopup<float>()
    {
        if(mState.mAction->isAction<sequenceguiactions::OpenCurvePointActionPopup<float>>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::OpenCurvePointActionPopup<float>>();
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::CurvePointActionPopup<float>>(
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

        if(mState.mAction->isAction<sequenceguiactions::CurvePointActionPopup<float>>())
        {
            if(ImGui::BeginPopup("Curve Point Actions"))
            {
                auto *action = mState.mAction->getDerived<sequenceguiactions::CurvePointActionPopup<float>>();

                float value = action->mValue * (action->mMaximum - action->mMinimum) + action->mMinimum;
                if(ImGui::InputFloat("value", &value))
                {
                    float new_value = (value - action->mMinimum) / (action->mMaximum - action->mMinimum);

                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.changeCurvePoint(
                        action->mTrackID,
                        action->mSegmentID,
                        action->mControlPointIndex,
                        action->mCurveIndex,
                        action->mTime,
                        new_value);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    mState.mDirty = true;
                }

                /**
                 * Handle adjusting time of point
                 * Calculate mTime value to time in sequence, show InputInt3 (mm::ss::ms).
                 * On edit : validate input and call controller
                 */
                // obtain segment
                auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                const auto *segment = curve_controller.getSegment(action->mTrackID, action->mSegmentID);
                assert(segment != nullptr);

                double time = action->mTime * segment->mDuration + segment->mStartTime;
                double min_time = segment->mStartTime;
                double max_time = segment->mStartTime + segment->mDuration;

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
                    new_time = math::clamp(new_time, min_time, max_time);

                    float perc = (new_time - segment->mStartTime) / segment->mDuration;
                    action->mTime = perc;
                    curve_controller.changeCurvePoint(
                        action->mTrackID,
                        action->mSegmentID,
                        action->mControlPointIndex,
                        action->mCurveIndex,
                        perc,
                        action->mValue);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    mState.mDirty = true;
                }

                // delete button
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::del)))
                {
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.deleteCurvePoint(
                        action->mTrackID,
                        action->mSegmentID,
                        action->mControlPointIndex,
                        action->mCurveIndex);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    mCurveCache.clear();

                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
                {
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            } else
            {
                // click outside popup so cancel action
                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
            }
        }
    }


    template<>
    void SequenceCurveTrackView::handleSegmentValueActionPopup<float>()
    {
        if(mState.mAction->isAction<sequenceguiactions::OpenEditSegmentCurveValuePopup<float>>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::OpenEditSegmentCurveValuePopup<float>>();
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::EditingSegmentCurveValue<float>>(
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

        if(mState.mAction->isAction<sequenceguiactions::EditingSegmentCurveValue<float>>())
        {
            if(ImGui::BeginPopup("Segment Value Actions"))
            {
                auto *action = mState.mAction->getDerived<sequenceguiactions::EditingSegmentCurveValue<float>>();
                int curveIndex = action->mCurveIndex;

                float value = action->mValue * (action->mMaximum - action->mMinimum) + action->mMinimum;
                if(ImGui::InputFloat("value", &value))
                {
                    float translated_value = (value - action->mMinimum) / (action->mMaximum - action->mMinimum);
                    auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                    curve_controller.changeCurveSegmentValue(
                        action->mTrackID,
                        action->mSegmentID,
                        translated_value,
                        curveIndex,
                        action->mType);
                    updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    mState.mDirty = true;
                }

                if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
                {
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            } else
            {
                // click outside popup so cancel action
                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
            }
        }
    }


    void SequenceCurveTrackView::updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID)
    {
        if(mState.mClipboard->isClipboard<CurveSegmentClipboard>())
        {
            auto *curve_segment_clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();

            if(mState.mClipboard->containsObject(segmentID, getPlayer().getSequenceFilename()))
            {
                mState.mClipboard->removeObject(segmentID);

                auto &controller = getEditor().getController<SequenceControllerCurve>();
                const auto *segment = controller.getSegment(trackID, segmentID);

                utility::ErrorState errorState;
                mState.mClipboard->addObject(segment, getPlayer().getSequenceFilename(), errorState);
            }
        }

    }


    void SequenceCurveTrackView::updateSegmentsInClipboard(const std::string& trackID)
    {
        if(mState.mClipboard->isClipboard<CurveSegmentClipboard>())
        {
            auto *clipboard = mState.mClipboard->getDerived<CurveSegmentClipboard>();
            if(clipboard->getTrackID() == trackID)
            {
                auto segment_ids = clipboard->getObjectIDs();
                for(const auto &segment_id: segment_ids)
                {
                    updateSegmentInClipboard(trackID, segment_id);
                }
            }
        }
    }


    void SequenceCurveTrackView::handleDragTanPoint()
    {
        //
        auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingTanPoint>();

        // get editor
        auto &editor = getEditor();

        // get controller for this track type
        auto *controller = editor.getControllerWithTrackID(action->mTrackID);
        assert(controller != nullptr);

        // assume we can upcast it to a curve controller
        auto *curve_controller = rtti_cast<SequenceControllerCurve>(controller);
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

        if(tangents_flipped && action->mType == sequencecurveenums::ETanPointTypes::IN)
        {
            action->mType = sequencecurveenums::ETanPointTypes::OUT;
        } else if(tangents_flipped && action->mType == sequencecurveenums::ETanPointTypes::OUT)
        {
            action->mType = sequencecurveenums::ETanPointTypes::IN;
        }

        //
        mState.mDirty = true;

        // discard action if mouse is released
        if(ImGui::IsMouseReleased(0))
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    void SequenceCurveTrackView::handleDragSegmentHandler()
    {
        auto *action = mState.mAction->getDerived<DraggingSegment>();
        assert(action != nullptr);

        // do we have the mouse still held down ? drag the segment
        if(ImGui::IsMouseDown(0))
        {
            float amount = mState.mMouseDelta.x / mState.mStepSize;

            // get editor
            auto &editor = getEditor();

            // get controller for this track type
            auto *controller = editor.getControllerWithTrackID(action->mTrackID);
            assert(controller != nullptr);

            // assume we can upcast it to a curve controller
            auto *curve_controler = static_cast<SequenceControllerCurve *>(controller);

            // change duration
            action->mNewDuration += amount;
            curve_controler->segmentDurationChange(action->mTrackID, action->mSegmentID, action->mNewDuration);
            updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

            // mark state as dirty
            mState.mDirty = true;
        }
            // otherwise... release!
        else if(ImGui::IsMouseReleased(0))
        {
            mState.mAction = createAction<None>();
        }
    }


    void SequenceCurveTrackView::handleAssignOutputIDToTrack()
    {
        // get action
        auto *action = mState.mAction->getDerived<AssignOutputIDToTrack>();
        assert(action != nullptr);

        // get curve controller
        auto &curve_controller = getEditor().getController<SequenceControllerCurve>();

        // call function to controller
        curve_controller.assignNewOutputID(action->mTrackID, action->mOutputID);

        // action is done
        mState.mAction = sequenceguiactions::createAction<None>();

        // redraw curves and caches
        mState.mDirty = true;
    }


    void SequenceCurveTrackView::handleDraggingSegmentValue()
    {
        // get action
        auto *action = mState.mAction->getDerived<DraggingSegmentValue>();
        assert(action != nullptr);

        // get controller for this track type
        auto *controller = getEditor().getControllerWithTrackID(action->mTrackID);
        assert(controller != nullptr);

        // assume we can upcast it to a curve controller
        assert(controller->get_type().is_derived_from<SequenceControllerCurve>());
        auto *curve_controller = static_cast<SequenceControllerCurve *>(controller);

        // tell the controller to change the curve segment value
        curve_controller->changeCurveSegmentValue(
            action->mTrackID,
            action->mSegmentID,
            action->mNewValue,
            action->mCurveIndex,
            action->mType);

        // update this segment if its in the clipboard
        updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

        // set dirty to clear curve cache
        mState.mDirty = true;

        //
        if(ImGui::IsMouseReleased(0))
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    void SequenceCurveTrackView::handleDraggingControlPoints()
    {
        // get the action
        auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingControlPoint>();

        // get editor
        auto &editor = getEditor();

        // get controller for this track type
        auto *controller = editor.getControllerWithTrackID(action->mTrackID);
        assert(controller != nullptr);

        // assume we can upcast it to a curve controller
        auto *curve_controler = rtti_cast<SequenceControllerCurve>(controller);
        assert(curve_controler != nullptr);

        curve_controler->changeCurvePoint(
            action->mTrackID,
            action->mSegmentID,
            action->mControlPointIndex,
            action->mCurveIndex,
            action->mNewTime,
            action->mNewValue);
        updateSegmentsInClipboard(action->mTrackID);

        mState.mDirty = true;

        if(ImGui::IsMouseReleased(0))
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
        }
    }


    void CurveSegmentClipboard::changeTrackID(const std::string &newTrackID)
    {
        mTrackID = newTrackID;
    }


    void SequenceCurveTrackView::handleLoadPresetPopup()
    {
        if(mState.mAction->isAction<ShowLoadPresetPopup>())
        {
            auto *load_action = mState.mAction->getDerived<ShowLoadPresetPopup>();
            auto *controller = getEditor().getControllerWithTrackID(load_action->mTrackID);

            if(controller->get_type().is_derived_from<SequenceControllerCurve>())
            {
                const std::string popup_name = "Load preset";
                if(!ImGui::IsPopupOpen(popup_name.c_str()))
                    ImGui::OpenPopup(popup_name.c_str());

                //
                if(ImGui::BeginPopupModal(
                    popup_name.c_str(),
                    nullptr,
                    ImGuiWindowFlags_AlwaysAutoResize))
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

                    SequenceTrackView::Combo("Presets",
                                             &load_action->mSelectedPresetIndex,
                                             presets);

                    utility::ErrorState error_state;
                    auto &gui = mService.getGui();
                    if(ImGui::ImageButton(gui.getIcon(icon::ok)))
                    {
                        if(mState.mClipboard->load(preset_files[load_action->mSelectedPresetIndex], error_state))
                        {
                            static const std::unordered_map<rtti::TypeInfo, bool (SequenceCurveTrackView::*)(const std::string &, double, utility::ErrorState &)> paste_map =
                                {{RTTI_OF(SequenceTrackCurveFloat), &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveFloat>},
                                 {RTTI_OF(SequenceTrackCurveVec2),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec2>},
                                 {RTTI_OF(SequenceTrackCurveVec3),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec3>},
                                 {RTTI_OF(SequenceTrackCurveVec4),  &SequenceCurveTrackView::pasteClipboardSegments<SequenceTrackSegmentCurveVec4>}};

                            // call the correct pasteClipboardSegments method for this track-type
                            auto paste_map_it = paste_map.find(load_action->mTrackType);
                            assert(paste_map_it != paste_map.end()); // type not found
                            if((*this.*paste_map_it->second)(load_action->mTrackID, load_action->mTime, error_state))
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

