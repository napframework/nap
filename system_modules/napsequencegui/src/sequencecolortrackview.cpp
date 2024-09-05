/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecolortrackview.h"
#include "sequencecontrollercolor.h"
#include "sequenceeditorgui.h"
#include "sequenceplayercoloroutput.h"
#include "sequencetrackcolor.h"
#include "sequenceguiservice.h"
#include "sequenceutils.h"
#include "sequenceguiutils.h"

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
        registerActionHandler(RTTI_OF(EditingColorSegment), [this] { handleEditSegmentValuePopup(); });
        registerActionHandler(RTTI_OF(AssignOutputIDToTrack), [this] { handleAssignOutputIDToTrack(); });
        registerActionHandler(RTTI_OF(DraggingSegment), [this] { handleSegmentDrag(); });
        registerActionHandler(RTTI_OF(LoadPresetPopup), [this] { handleLoadPresetPopup(); });
        registerActionHandler(RTTI_OF(EditColorCurvePopup), [this] { handleEditCurvePopup(); });
        registerActionHandler(RTTI_OF(DraggingColorCurvePoint), [this] { handleDragCurvePoint(); });
        registerActionHandler(RTTI_OF(EditColorCurvePoint), [this] { handleEditCurvePointPopup(); });
        registerActionHandler(RTTI_OF(DraggingColorCurveTanPoint), [this] { handleDragCurveTanPoint(); });

        mSegmentViews.emplace(RTTI_OF(SequenceTrackSegmentColor), std::make_unique<SequenceColorTrackSegmentView>());
    }


    void SequenceColorTrackView::onPreShowTrack()
    {
        // if state is dirty, clear cache points
        if(mState.mDirty)
        {
            mCachePoints.clear();
            mCachedPolylines.clear();
        }
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

        // track height
        const float track_height = track.mTrackHeight * mState.mScale;

        // black background color for the curve and handlers
        const ImU32 black_imu_color = mService.getColors().mDark;
        const ImU32 white_imu_color = mService.getColors().mFro4;

        float prev_segment_x = 0.0f;

        // continue drawing the segments
        int segment_count = 0;
        if(!track.mSegments.empty())
        {
            // draw first blend
            auto a_segment_color = RGBAColorFloat(0.0f, 0.0f, 0.0f, 0.0f);
            for(const auto &segment: track.mSegments)
            {
                // upcast to color segment
                assert(segment.get()->get_type() == RTTI_OF(SequenceTrackSegmentColor));
                const auto& color_segment = static_cast<const SequenceTrackSegmentColor&>(*segment.get());

                // get segment color and blend type
                auto b_segment_color = static_cast<const SequenceTrackSegmentColor*>(segment.get())->mColor;
                auto b_blend_type = static_cast<const SequenceTrackSegmentColor*>(segment.get())->mBlendMethod;

                // get segment position on screen and size
                float segment_x = (float) (segment->mStartTime) * mState.mStepSize;
                float width = segment_x - prev_segment_x;
                int steps = (int) width;
                float step_width = width / (float)steps;

                // obtain normalized mouse position, we need this information later when dragging curve points or tan handlers
                glm::vec2 normalized_mouse_pos = { (mState.mMousePos.x - trackTopLeft.x - prev_segment_x) / width,
                                                   -(mState.mMousePos.y - trackTopLeft.y - track_height) / track_height};

                // check if we're hovering the curve
                bool hovering_curve = false;
                float mouse_pos_in_curve = 0.0f;

                // only check if we're not in an action or already hovering the curve
                if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurveColorSegment>())
                {
                    if(mState.mIsWindowFocused && ImGui::IsMouseHoveringRect(
                            {trackTopLeft.x + prev_segment_x, trackTopLeft.y},
                            {trackTopLeft.x + segment_x, trackTopLeft.y + track_height}))
                    {
                        // check if we're hovering the curve
                        // get the normalized mouse x position in the segment
                        float mouse_x_in_segment_normalized = (mState.mMousePos.x - trackTopLeft.x - prev_segment_x) / width;
                        if(mouse_x_in_segment_normalized >= 0.0f && mouse_x_in_segment_normalized <= 1.0f)
                        {
                            // get the normalized mouse y position in the segment
                            float mouse_y_in_segment_normalized = (mState.mMousePos.y - trackTopLeft.y) / track_height;
                            if(mouse_y_in_segment_normalized >= 0.0f && mouse_y_in_segment_normalized <= 1.0f)
                            {
                                // get the curve value at this position
                                float h = color_segment.mCurve->evaluate(mouse_x_in_segment_normalized);

                                // check if mouse is hovering the curve
                                if(fabsf(h - (1.0f - mouse_y_in_segment_normalized)) < 0.05f)
                                {
                                    hovering_curve = true;
                                    mouse_pos_in_curve = mouse_x_in_segment_normalized;
                                }
                            }
                        }
                    }
                }

                // invoke hovering color curve action if not already active
                if(hovering_curve)
                {
                    if(mState.mAction->isAction<None>())
                    {
                        mState.mAction = createAction<HoveringCurveColorSegment>(track.mID,
                                                                                 segment->mID,
                                                                                 mState.mMousePos);
                    }

                    // if right mouse button is clicked, open edit color curve popup
                    if(mState.mAction->isAction<HoveringCurveColorSegment>())
                    {
                        if(ImGui::IsMouseDown(1))
                        {
                            mState.mAction = createAction<EditColorCurvePopup>(track.mID,
                                                                               segment->mID,
                                                                               mState.mMousePos,
                                                                               mouse_pos_in_curve);
                        }
                    }
                }else
                {
                    // if we're not hovering the curve, check if we're hovering the curve action
                    if(mState.mAction->isAction<HoveringCurveColorSegment>())
                    {
                        auto *action = mState.mAction->getDerived<HoveringCurveColorSegment>();
                        if(action->mSegmentID == segment->mID)
                        {
                            // if we're not hovering the curve, remove the action
                            mState.mAction = createAction<None>();
                        }
                    }
                }

                // create cache points if state is dirty
                // cache points are used to draw the curve and the color blend
                if(mState.mDirty)
                {
                    RGBAColorFloat a_color;
                    RGBAColorFloat b_color;
                    std::vector<ImVec2> polyline;
                    for(int i = 0; i < steps; i ++)
                    {
                        // create cache point
                        CachePoint cache_point;

                        // calculate blend values
                        float blend_a = (float)i / (float)steps;
                        float blend_b = (float)(i + 1) / (float)steps;

                        // get positions of the left and right of the curve with these bounds of the rectangle
                        float a_pos = color_segment.mCurve->evaluate(blend_a);
                        float b_pos = color_segment.mCurve->evaluate(blend_b);
                        a_pos = glm::clamp(a_pos, 0.0f, 1.0f);
                        b_pos = glm::clamp(b_pos, 0.0f, 1.0f);

                        // blend colors depending on blend method
                        a_color = utility::colorspace::blendColors(a_segment_color, b_segment_color, a_pos, b_blend_type);
                        b_color = utility::colorspace::blendColors(a_segment_color, b_segment_color, b_pos, b_blend_type);

                        // convert the a and b colors to imgui colors
                        auto a_im = utility::toImColor(a_color);
                        auto b_im = utility::toImColor(b_color);

                        // x position of the rectangle and part of curve
                        float pos = trackTopLeft.x + prev_segment_x + i * step_width;

                        // set the rectangle points and colors
                        cache_point.mRectStart = {pos, trackTopLeft.y};
                        cache_point.mRectEnd = {pos + step_width, trackTopLeft.y + track_height};
                        cache_point.mColorStart = a_im;
                        cache_point.mColorEnd = b_im;

                        // add the cache point
                        mCachePoints[segment->mID].emplace_back(cache_point);

                        // obtain the curve height for points a and b
                        float curve_height_a = track_height * a_pos;
                        float curve_height_b = track_height * b_pos;

                        // add the polyline points
                        ImVec2 curve_point_a = {pos, trackTopLeft.y + track_height - curve_height_a};
                        ImVec2 curve_point_b = {pos + step_width, trackTopLeft.y + track_height - curve_height_b};
                        polyline.emplace_back(curve_point_a);
                        polyline.emplace_back(curve_point_b);

                        // add the polyline point to cache
                        mCachedPolylines[segment->mID].emplace_back(curve_point_a);
                    }
                    a_segment_color = b_segment_color;
                }

                // draw the cached points for this segment
                if(mCachePoints.find(segment->mID) != mCachePoints.end())
                {
                    const auto& cached_points = mCachePoints[segment->mID];
                    for(const auto& cached_point : cached_points)
                    {
                        // draw the rectangle
                        draw_list->AddRectFilledMultiColor(
                                cached_point.mRectStart,
                                cached_point.mRectEnd,
                                cached_point.mColorStart,
                                cached_point.mColorEnd,
                                cached_point.mColorEnd,
                                cached_point.mColorStart);
                    }
                }

                // draw cached polyline
                if(mCachedPolylines.find(segment->mID) != mCachedPolylines.end())
                {
                    const auto& cached_polyline = mCachedPolylines[segment->mID];
                    float curve_size = hovering_curve ? 4.0f * mState.mScale : 2.0f * mState.mScale;
                    draw_list->AddPolyline(cached_polyline.data(), cached_polyline.size(), black_imu_color, false, curve_size * 2.0f);
                    draw_list->AddPolyline(cached_polyline.data(), cached_polyline.size(), white_imu_color, false, curve_size);
                }

                // draw curve points
                const auto& curve_points = color_segment.mCurve->mPoints;
                for(int i = 0; i < curve_points.size(); i++)
                {
                    // Calculate the position of the curve point
                    ImVec2 curve_point = {trackTopLeft.x + prev_segment_x + curve_points[i].mPos.mTime * width,
                                          trackTopLeft.y + track_height - curve_points[i].mPos.mValue * track_height};

                    // get curve point color
                    int point_color_cache_idx = (int)(curve_points[i].mPos.mTime * width);
                    point_color_cache_idx = math::clamp(point_color_cache_idx, 0, steps - 1);

                    // check if we're hovering the curve point, ignore the first and last points
                    const float curve_point_bounds = 5.0f * mState.mScale;
                    bool is_hovering_curve_point = ImGui::IsMouseHoveringRect(
                            {curve_point.x - curve_point_bounds, curve_point.y - curve_point_bounds},
                            {curve_point.x + curve_point_bounds, curve_point.y + curve_point_bounds}) &&
                                    i != 0 && i != curve_points.size() - 1;

                    // handle curve point interaction
                    if(is_hovering_curve_point)
                    {
                        // create the hovering curve point action
                        if(mState.mAction->isAction<None>() || mState.mAction->isAction<HoveringCurveColorSegment>() || mState.mAction->isAction<HoveringTrackExtensionHandler>())
                        {
                            mState.mAction = createAction<HoveringColorCurvePoint>(track.mID, segment->mID, i);
                        }

                        // left mouse is start dragging
                        if(ImGui::IsMouseDown(0))
                        {
                            if(!mState.mAction->isAction<DraggingColorCurvePoint>())
                            {
                                if(mState.mAction->isAction<HoveringColorCurvePoint>())
                                    mState.mAction = createAction<DraggingColorCurvePoint>(track.mID, segment->mID, i, normalized_mouse_pos);
                            }
                        }else if(ImGui::IsMouseDown(1))
                        {
                            if(mState.mAction->isAction<HoveringColorCurvePoint>())
                            {
                                mState.mAction = createAction<EditColorCurvePoint>(track.mID, segment->mID, i, mState.mMousePos);
                            }
                        }
                    }else
                    {
                        // if we're not hovering the curve point, stop the action
                        if(mState.mAction->isAction<HoveringColorCurvePoint>())
                        {
                            auto *action = mState.mAction->getDerived<HoveringColorCurvePoint>();
                            if(action->mSegmentID == segment->mID && action->mPointIndex == i)
                            {
                                mState.mAction = createAction<None>();
                            }
                        }

                        // if we're dragging the curve point, update the position
                        if(mState.mAction->isAction<DraggingColorCurvePoint>())
                        {
                            if(ImGui::IsMouseDown(0))
                            {
                                auto *action = mState.mAction->getDerived<DraggingColorCurvePoint>();
                                if(action->mSegmentID == segment->mID && action->mPointIndex == i)
                                {
                                    action->mPosition = normalized_mouse_pos;
                                    is_hovering_curve_point = true;
                                }
                            }else
                            {
                                mState.mAction = createAction<None>();
                            }
                        }
                    }

                    // draw the curve point
                    const float curve_point_size = is_hovering_curve_point ? 6.0f * mState.mScale : 4.0f * mState.mScale;
                    draw_list->AddCircleFilled(curve_point, curve_point_size * 1.5f, black_imu_color);
                    draw_list->AddCircleFilled(curve_point, curve_point_size, white_imu_color);

                    // draw tan points
                    if(color_segment.mCurveType== math::ECurveInterp::Bezier)
                    {
                        for(int tan_idx = 0; tan_idx < 2; tan_idx++)
                        {
                            // get the correct tan point
                            const math::FComplex<float, float> &tan_complex = tan_idx == 0 ? curve_points[i].mInTan : curve_points[i].mOutTan;

                            // get the offset from the tan
                            const float tan_constant_size = 200.0f * mState.mScale;
                            ImVec2 offset = {tan_complex.mTime * tan_constant_size,
                                             static_cast<float>(tan_complex.mValue) * -1.0f * tan_constant_size};
                            ImVec2 tan_point = {curve_point.x + offset.x, curve_point.y + offset.y};

                            // tan bounds represent the area around which the mouse can hover the tan point
                            const float tan_bounds = mState.mScale * 5.0f;

                            // check if we're hovering the tan point
                            bool tan_point_hovered = false;

                            // check if we are hovering this point with the mouse
                            if(mState.mAction->isAction<None>() ||
                               mState.mAction->isAction<HoveringCurveColorSegment>() ||
                               mState.mAction->isAction<HoveringSegment>() ||
                               mState.mAction->isAction<HoveringTrackExtensionHandler>())
                            {
                                tan_point_hovered = ImGui::IsMouseHoveringRect(
                                        {tan_point.x - tan_bounds, tan_point.y - tan_bounds},
                                        {tan_point.x + tan_bounds, tan_point.y + tan_bounds});

                                if(tan_point_hovered)
                                {
                                    mState.mAction = createAction<HoveringColorCurveTanPoint>(track.mID, segment->mID, i, tan_idx);
                                }
                            }else if(mState.mAction->isAction<HoveringColorCurveTanPoint>())
                            {
                                tan_point_hovered = ImGui::IsMouseHoveringRect({tan_point.x - tan_bounds, tan_point.y - tan_bounds},
                                                                               {tan_point.x + tan_bounds, tan_point.y + tan_bounds});

                                // if we are not hovering the tan point, stop the action
                                if(!tan_point_hovered)
                                {
                                    auto *action = mState.mAction->getDerived<HoveringColorCurveTanPoint>();
                                    if(action->mSegmentID == segment->mID && action->mPointIndex == i && action->mTanIndex == tan_idx)
                                    {
                                        tan_point_hovered = false;
                                        mState.mAction = createAction<None>();
                                    }
                                }
                            }

                            // if we're hovering the tan point and holding down the mouse, start dragging the tan point
                            if(tan_point_hovered)
                            {
                                if(ImGui::IsMouseDown(0))
                                {
                                    if(!mState.mAction->isAction<DraggingColorCurveTanPoint>())
                                    {
                                        glm::vec2 delta = { mState.mMouseDelta.x / tan_constant_size,
                                                            (mState.mMouseDelta.y / tan_constant_size) * -1.0f };
                                        glm::vec2 new_tan = { tan_complex.mTime + delta.x, tan_complex.mValue + delta.y };

                                        if(mState.mAction->isAction<HoveringColorCurveTanPoint>())
                                            mState.mAction = createAction<DraggingColorCurveTanPoint>(track.mID, segment->mID, i, tan_idx, new_tan);
                                    }
                                }
                            }

                            // handle dragging of tan point
                            if(mState.mAction->isAction<DraggingColorCurveTanPoint>())
                            {
                                if(ImGui::IsMouseDown(0))
                                {
                                    // obtain dragging action and set new position
                                    auto *action = mState.mAction->getDerived<DraggingColorCurveTanPoint>();
                                    if(action->mSegmentID == segment->mID && action->mPointIndex == i && action->mTanIndex == tan_idx)
                                    {
                                        glm::vec2 delta = { mState.mMouseDelta.x / tan_constant_size,
                                                            (mState.mMouseDelta.y / tan_constant_size) * -1.0f };
                                        glm::vec2 new_tan = { tan_complex.mTime + delta.x, tan_complex.mValue + delta.y };

                                        action->mPosition = new_tan;

                                        tan_point_hovered = true;
                                    }
                                }else
                                {
                                    // mouse released, stop dragging
                                    mState.mAction = createAction<None>();
                                }
                            }

                            // draw line background
                            float tan_line_size = tan_point_hovered ? 2.0f * mState.mScale : 1.0f * mState.mScale;
                            draw_list->AddLine(curve_point, tan_point, black_imu_color, tan_line_size * 2.0f);

                            // draw line
                            draw_list->AddLine(curve_point, tan_point, white_imu_color, tan_line_size);

                            // draw handler background
                            float tan_point_size = tan_point_hovered ? 4.0f * mState.mScale : 2.0f * mState.mScale;
                            draw_list->AddCircleFilled(tan_point, tan_point_size * 2.0f, black_imu_color);

                            // draw handler
                            draw_list->AddCircleFilled(tan_point, tan_point_size, white_imu_color);
                        }
                    }
                }

                // proceed with the next segment
                prev_segment_x = segment_x;
                segment_count++;
            }

            // draw segment handlers
            for(const auto &segment: track.mSegments)
            {
                float segment_x = (float) (segment->mStartTime) * mState.mStepSize;

                // draw segment handler
                showSegmentHandler(
                        track,
                        *segment,
                        trackTopLeft, segment_x,
                        draw_list);
            }
        }

        // draw line of mouse cursor in track
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
                    ImVec2 top = {mState.mMousePos.x, trackTopLeft.y};
                    ImVec2 bottom = {mState.mMousePos.x, trackTopLeft.y + track_height};

                    draw_list->AddLine(
                            top, // top
                            bottom, // bottom right
                            mService.getColors().mDark, // color
                            2.0f * mState.mScale); // thickness

                    draw_list->AddLine(
                            top, // top
                            bottom, // bottom
                            mService.getColors().mFro2, // color
                            1.0f * mState.mScale); // thickness

                    ImGui::BeginTooltip();
                    ImGui::Text(formatTimeString(mState.mMouseCursorTime).c_str());
                    ImGui::EndTooltip();

                    // right mouse down
                    if(ImGui::IsMouseClicked(1))
                    {
                        // open insert color segment popup
                        double time = mState.mMouseCursorTime;
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
                    ImVec2 top = {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y};
                    ImVec2 bottom = {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y + track_height};

                    // position of insertion in track
                    draw_list->AddLine(top, // top left
                                       bottom, // bottom right
                                       mService.getColors().mDark, // color
                                       2.0f * mState.mScale); // thickness
                    draw_list->AddLine(top, // top left
                                       bottom, // bottom right
                                       mService.getColors().mFro2, // color
                                       1.0f * mState.mScale); // thickness
                }
            }

            if(mState.mAction->isAction<InsertColorSegmentPopup>())
            {
                auto *action = mState.mAction->getDerived<InsertColorSegmentPopup>();
                if(action->mTrackID == track.mID)
                {
                    ImVec2 top = {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y};
                    ImVec2 bottom = {trackTopLeft.x + (float) action->mTime * mState.mStepSize, trackTopLeft.y + track_height};

                    // position of insertion in track
                    draw_list->AddLine(top, // top left
                                       bottom, // bottom right
                                       mService.getColors().mFro2, // color
                                       2.0f * mState.mScale); // thickness
                    draw_list->AddLine(top, // top left
                                       bottom, // bottom right
                                       mService.getColors().mFro2, // color
                                       1.0f * mState.mScale); // thickness
                }
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
            // Insert color segment, first color picker followed by insert icon on same line
            ImGui::ColorEdit4("Color", &action->mValue[0]);
            ImGui::SameLine();
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert)))
            {
                // take snapshot
                getEditor().takeSnapshot(action->get_type());

                // call function to controller
                auto &controller = getEditor().getController<SequenceControllerColor>();
                controller.insertColorSegment(action->mTrackID, action->mTime, action->mValue);

                // mark state dirty
                mState.mDirty = true;

                // exit popup
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            // show the ability to paste from clipboard if the clipboard contains color segments
            if(mState.mClipboard->isClipboard<ColorSegmentClipboard>())
            {
                if(ImGui::ImageButton(mService.getGui().getIcon(icon::paste)))
                {
                    // take snapshot
                    getEditor().takeSnapshot(action->get_type());

                    // paste from clipboard
                    utility::ErrorState error_state;
                    if(!pasteFromClipboard(action->mTrackID, action->mTime, error_state))
                    {
                        // log error and open error popup
                        Logger::error(error_state.toString());
                        ImGui::OpenPopup("Error");
                        action->mErrorString = error_state.toString();
                    }else
                    {
                        ImGui::CloseCurrentPopup();
                        mState.mAction = createAction<None>();
                    }

                    // mark state dirty
                    mState.mDirty = true;
                }
            }

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::load), "Load preset"))
            {
                if(mState.mClipboard->getTrackType() != RTTI_OF(SequenceTrackColor))
                {
                    mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(SequenceTrackColor), getEditor().mSequencePlayer->getSequenceFilename());
                }
                mState.mAction = createAction<LoadPresetPopup>(action->mTrackID, action->mTime, RTTI_OF(SequenceTrackColor));
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


    void SequenceColorTrackView::onHandleEditSegment(const nap::SequenceTrack &track,
                                                     const nap::SequenceTrackSegment &segment)
    {
        assert(segment.get_type() == RTTI_OF(SequenceTrackSegmentColor));
        auto& segment_color = static_cast<const SequenceTrackSegmentColor&>(segment);
        mState.mAction = createAction<EditingColorSegment>(track.mID,
                                                           segment.mID,
                                                           mState.mMousePos,
                                                           segment_color.mCurveType,
                                                           segment_color.mColor,
                                                           segment_color.mBlendMethod,
                                                           segment.mStartTime);
    }


    void SequenceColorTrackView::onDrawSegmentHandler(ImVec2 top, ImVec2 bottom, ImDrawList *drawList, bool inClipboard, bool bold)
    {
        drawList->AddLine(top,
                          bottom,
                          mService.getColors().mDark,
                          bold ? 6.0f * mState.mScale : (inClipboard ? 4.0f : 2.0f) * mState.mScale);
        drawList->AddLine(top,
                          bottom,
                          inClipboard ? mService.getColors().mHigh1 : mService.getColors().mFro4,
                          bold ? 3.0f * mState.mScale : (inClipboard ? 2.0f : 1.0f) * mState.mScale);
    }


    void SequenceColorTrackView::onAddSegmentToClipboard(const nap::SequenceTrack &track, const nap::SequenceTrackSegment &segment)
    {
        // if no color segment clipboard, create it or if previous clipboard is from a different sequence, create new clipboard
        if(!mState.mClipboard->isClipboard<ColorSegmentClipboard>())
            mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(ColorSegmentClipboard), getEditor().mSequencePlayer->getSequenceFilename());
        else if(mState.mClipboard->getDerived<ColorSegmentClipboard>()->getSequenceName() != getEditor().mSequencePlayer->getSequenceFilename())
            mState.mClipboard = createClipboard<ColorSegmentClipboard>(RTTI_OF(ColorSegmentClipboard), getEditor().mSequencePlayer->getSequenceFilename());

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


    void SequenceColorTrackView::handleEditSegmentValuePopup()
    {
        auto* action = mState.mAction->getDerived<EditingColorSegment>();
        assert(action!= nullptr);
        if(!action->mPopupOpened)
        {
            // invoke edit segment popup
            ImGui::OpenPopup("Edit Color Segment");
            action->mPopupOpened = true;
        }

        // handle edit segment popup
        if(ImGui::BeginPopup("Edit Color Segment"))
        {
            // get controller
            auto &controller = getEditor().getController<SequenceControllerColor>();

            // color picker
            if(ImGui::ColorEdit4("Color", &action->mValue[0]))
            {
                // Mark state dirty
                mState.mDirty = true;

                // Take snapshot
                if(action->mTakeSnapshot)
                {
                    getEditor().takeSnapshot(action->get_type());
                    action->mTakeSnapshot = false;
                }

                // change color
                controller.changeSegmentColor(action->mTrackID, action->mSegmentID, action->mValue);

                // update segment in clipboard
                updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
            }

            // blend method
            static constexpr const char *blend_methods[] = {"Linear", "Oklab"};
            if(ImGui::BeginCombo("Blend Method", blend_methods[static_cast<int>(action->mBlendMethod)]))
            {
                for(int i = 0; i < IM_ARRAYSIZE(blend_methods); i++)
                {
                    bool is_selected = (static_cast<int>(action->mBlendMethod) == i);
                    if(ImGui::Selectable(blend_methods[i], is_selected))
                    {
                        // Mark state dirty
                        mState.mDirty = true;

                        // Take snapshot
                        if(action->mTakeSnapshot)
                        {
                            getEditor().takeSnapshot(action->get_type());
                            action->mTakeSnapshot = false;
                        }

                        // change blend method
                        action->mBlendMethod = static_cast<SequenceTrackSegmentColor::EColorSpace>(i);
                        controller.changeSegmentColorBlendMethod(action->mTrackID, action->mSegmentID, action->mBlendMethod);

                        // update segment in clipboard
                        updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    }

                    if(is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // curve interp
            static constexpr const char *curve_interps[] = {"Bezier", "Linear", "Stepped"};
            if(ImGui::BeginCombo("Curve Interpolation", curve_interps[static_cast<int>(action->mCurveType)]))
            {
                for(int i = 0; i < IM_ARRAYSIZE(curve_interps); i++)
                {
                    bool is_selected = (static_cast<int>(action->mCurveType) == i);
                    if(ImGui::Selectable(curve_interps[i], is_selected))
                    {
                        // Mark state dirty
                        mState.mDirty = true;

                        // Take snapshot
                        if(action->mTakeSnapshot)
                        {
                            getEditor().takeSnapshot(action->get_type());
                            action->mTakeSnapshot = false;
                        }

                        // change curve type
                        action->mCurveType = static_cast<math::ECurveInterp>(i);
                        controller.changeSegmentCurveType(action->mTrackID, action->mSegmentID, action->mCurveType);

                        // update segment in clipboard
                        updateSegmentInClipboard(action->mTrackID, action->mSegmentID);
                    }

                    if(is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // ok button
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::SameLine();
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::del)))
            {
                getEditor().takeSnapshot(action->get_type());

                controller.deleteSegment(action->mTrackID, action->mSegmentID);

                // remove segment from clipboard
                mState.mClipboard->removeObject(action->mSegmentID);

                mState.mDirty = true;
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::EndPopup();
        }else
        {
            // click outside popup so cancel action
            ImGui::CloseCurrentPopup();
            mState.mAction = createAction<None>();
        }
    }


    bool SequenceColorTrackView::pasteFromClipboard(const std::string& trackID, double time, utility::ErrorState& errorState)
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
                deserialized_color_segments[i]->mStartTime = deserialized_color_segments[i]->mStartTime - first_segment_time;
            }

            // obtain controller
            auto &controller = getEditor().getController<SequenceControllerColor>();

            // insert deserialized color segments
            for(auto *segment : deserialized_color_segments)
            {
                // add segment to controller
                const auto* new_segment = controller.insertColorSegment(trackID, time + segment->mStartTime, segment->mColor);

                // add points
                for(int p = 0; p < segment->mCurve->mPoints.size(); p++)
                {
                    // skip first and last points, they are already added when creating the segment
                    if(p!=0 && p < segment->mCurve->mPoints.size() - 1)
                        controller.insertCurvePoint(trackID, new_segment->mID, segment->mCurve->mPoints[p].mPos.mTime);

                    controller.changeSegmentCurvePoint(trackID, new_segment->mID, p, { segment->mCurve->mPoints[p].mPos.mTime, segment->mCurve->mPoints[p].mPos.mValue });
                    controller.changeSegmentCurveTanPoint(trackID, new_segment->mID, p, 0, { segment->mCurve->mPoints[p].mInTan.mTime, segment->mCurve->mPoints[p].mInTan.mValue });
                }

                // copy blend method and curve type
                controller.changeSegmentColorBlendMethod(trackID, new_segment->mID, segment->mBlendMethod);
                controller.changeSegmentCurveType(trackID, new_segment->mID, segment->mCurveType);
            }
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
            // get action
            auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingSegment>();
            assert(action != nullptr);

            // Mark state as dirty, so that the cache is updated
            mState.mDirty = true;

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


    void SequenceColorTrackView::handleDragCurvePoint()
    {
        // get action
        auto* action = mState.mAction->getDerived<DraggingColorCurvePoint>();
        assert(action!= nullptr);

        // get editor & controller
        auto &editor = getEditor();
        auto &color_controller = editor.getController<SequenceControllerColor>();

        // take snapshot
        if(action->mTakeSnapshot)
        {
            action->mTakeSnapshot = false;
            editor.takeSnapshot(action->get_type());
        }

        // change curve point
        color_controller.changeSegmentCurvePoint(action->mTrackID,
                                                 action->mSegmentID,
                                                 action->mPointIndex,
                                                 action->mPosition);

        // update segment in clipboard
        updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

        // mark state dirty
        mState.mDirty = true;
    }


    void SequenceColorTrackView::handleEditCurvePointPopup()
    {
        auto* action = mState.mAction->getDerived<EditColorCurvePoint>();
        assert(action!= nullptr);
        if(!action->mPopupOpened)
        {
            // invoke edit segment popup
            ImGui::OpenPopup("Edit Color Curve Point");
            action->mPopupOpened = true;
        }

        // handle edit segment popup
        if(ImGui::BeginPopup("Edit Color Curve Point"))
        {
            // get controller
            auto &controller = getEditor().getController<SequenceControllerColor>();

            // delete button
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::del)))
            {
                // take snapshot
                getEditor().takeSnapshot(action->get_type());

                // delete point
                controller.deleteCurvePoint(action->mTrackID, action->mSegmentID, action->mPointIndex);

                // update segment in clipboard
                updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                // mark state dirty
                mState.mDirty = true;

                // exit popup
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            // ok button
            if(ImGui::ImageButton(mService.getGui().getIcon(icon::ok)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::EndPopup();
        }else
        {
            // click outside popup so cancel action
            ImGui::CloseCurrentPopup();
            mState.mAction = createAction<None>();
        }
    }


    void SequenceColorTrackView::handleEditCurvePopup()
    {
        auto* action = mState.mAction->getDerived<EditColorCurvePopup>();
        assert(action!= nullptr);
        if(!action->mPopupOpened)
        {
            // invoke edit segment popup
            ImGui::OpenPopup("Edit Color Curve");
            action->mPopupOpened = true;
        }

        // handle edit curve popup
        if(ImGui::BeginPopup("Edit Color Curve"))
        {
            // get controller
            auto &controller = getEditor().getController<SequenceControllerColor>();

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::insert), "insert point"))
            {
                // take snapshot
                getEditor().takeSnapshot(action->get_type());

                // insert point
                controller.insertCurvePoint(action->mTrackID,
                                            action->mSegmentID,
                                            action->mTimeInCurve);

                // update segment in clipboard
                updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

                // mark state dirty
                mState.mDirty = true;

                // exit popup
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            if(ImGui::ImageButton(mService.getGui().getIcon(icon::cancel)))
            {
                ImGui::CloseCurrentPopup();
                mState.mAction = createAction<None>();
            }

            ImGui::EndPopup();
        }else
        {
            // click outside popup so cancel action
            ImGui::CloseCurrentPopup();
            mState.mAction = createAction<None>();
        }
    }


    void SequenceColorTrackView::handleDragCurveTanPoint()
    {
        // get action
        auto* action = mState.mAction->getDerived<DraggingColorCurveTanPoint>();
        assert(action!= nullptr);

        // get editor & controller
        auto &editor = getEditor();
        auto &color_controller = editor.getController<SequenceControllerColor>();

        // take snapshot
        if(action->mTakeSnapshot)
        {
            action->mTakeSnapshot = false;
            editor.takeSnapshot(action->get_type());
        }

        // change curve point
        color_controller.changeSegmentCurveTanPoint(action->mTrackID,
                                                    action->mSegmentID,
                                                    action->mPointIndex,
                                                    action->mTanIndex,
                                                    action->mPosition);

        // update segment in clipboard
        updateSegmentInClipboard(action->mTrackID, action->mSegmentID);

        // mark state dirty
        mState.mDirty = true;
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

                            if(pasteFromClipboard(load_action->mTrackID, load_action->mTime, error_state))
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
