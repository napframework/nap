/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Internal includes
#include "sequencecurvetrackview_guiactions.h"
#include "sequenceguiutils.h"

// External includes
#include <imguiutils.h>

namespace nap
{
    template<typename T>
    void SequenceCurveTrackView::handleCurvePointActionPopup()
    {
        auto *action = mState.mAction->getDerived<sequenceguiactions::CurvePointActionPopup<T>>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Curve Point Actions");
            action->mOpened = true;
        }

        if(ImGui::BeginPopup("Curve Point Actions"))
        {
            int curveIndex = action->mCurveIndex;

            float value = action->mValue * (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]) +
                          action->mMinimum[curveIndex];
            if(ImGui::InputFloat("value", &value))
            {
                auto &curve_controller = getEditor().getController<SequenceControllerCurve>();
                curve_controller.changeCurvePoint(
                    action->mTrackID,
                    action->mSegmentID,
                    action->mControlPointIndex,
                    action->mCurveIndex,
                    action->mTime,
                    value);
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
            const auto *segment = static_cast<const SequenceTrackSegmentDuration*>(curve_controller.getSegment(action->mTrackID, action->mSegmentID));
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

            if(ImGui::ImageButton(mService.getGui().getIcon(nap::icon::del)))
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
                mState.mDirty = true;

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if(ImGui::ImageButton(mService.getGui().getIcon(nap::icon::ok)))
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


    template<typename T>
    void SequenceCurveTrackView::handleSegmentValueActionPopup()
    {
        auto *action = mState.mAction->getDerived<sequenceguiactions::EditingSegmentCurveValue<T>>();
        assert(action!= nullptr);

        if(!action->mOpened)
        {
            ImGui::OpenPopup("Segment Value Actions");
            action->mOpened = true;
        }

        if(ImGui::BeginPopup("Segment Value Actions"))
        {
            int curveIndex = action->mCurveIndex;

            if(action->mTakeSnapshot)
            {
                action->mTakeSnapshot = false;
                getEditor().takeSnapshot(action->get_type());
            }

            float value =
                action->mValue[curveIndex] * (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]) +
                action->mMinimum[curveIndex];
            if(ImGui::InputFloat("value", &value))
            {
                float translated_value = (value - action->mMinimum[curveIndex]) /
                                         (action->mMaximum[curveIndex] - action->mMinimum[curveIndex]);
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

            if(ImGui::ImageButton(mService.getGui().getIcon(nap::icon::ok)))
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


    template<typename T>
    void SequenceCurveTrackView::drawCurves(
            const SequenceTrack& track,
            const SequenceTrackSegment& segmentBase,
            const ImVec2& trackTopLeft,
            const float previousSegmentX,
            const float segmentWidth,
            const float segmentX,
            ImDrawList* drawList)
    {
        const auto &segment = static_cast<const SequenceTrackSegmentCurve<T> &>(segmentBase);
        const float points_per_pixel = 0.5f * mState.mScale;
        const float track_height = track.mTrackHeight * mState.mScale;
        bool needs_drawing = ImGui::IsRectVisible({trackTopLeft.x + previousSegmentX, trackTopLeft.y}, {
            trackTopLeft.x + previousSegmentX + segmentWidth, trackTopLeft.y + track_height});

        if(needs_drawing)
        {
            // if no cache present, create new curve
            if(mCurveCache.find(segment.mID) == mCurveCache.end())
            {
                const long point_num = (int) (points_per_pixel * segmentWidth);
                std::vector<std::vector<ImVec2>> curves;
                for(int v = 0; v < segment.mCurves.size(); v++)
                {
                    std::vector<ImVec2> curve;
                    if(point_num > 0)
                    {
                        auto start_x = math::max<float>(trackTopLeft.x, 0);
                        float end_x = start_x + mState.mWindowSize.x + mState.mWindowPos.x;

                        // start drawing at window pos
                        {
                            long i = 0;
                            for(; i <= point_num; i++)
                            {
                                float p = (float) i / (float) point_num;
                                float x = trackTopLeft.x + previousSegmentX + segmentWidth * p;
                                if(x > start_x)
                                {
                                    float value = 1.0f - segment.mCurves[v]->evaluate(p);
                                    value = math::clamp(value, 0.0f, 1.0f);
                                    ImVec2 point =
                                        {
                                            x,
                                            trackTopLeft.y + value * track_height
                                        };
                                    curve.emplace_back(point);
                                }

                                if(x > end_x)
                                {
                                    break; // no longer visible on right side, continuation of this loop is not necessary
                                }
                            }
                        }
                    }

                    curves.emplace_back(std::move(curve));
                }
                mCurveCache.emplace(segment.mID, std::move(curves));
            }
        }

        int selected_curve = -1;
        if(mState.mIsWindowFocused)
        {
            // determine if mouse is hovering curve
            if((mState.mAction->isAction<sequenceguiactions::None>() ||
                mState.mAction->isAction<sequenceguiactions::HoveringCurve>())
               && ImGui::IsMouseHoveringRect(
                {trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y}, // top left
                {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}))  // bottom right
            {
                // translate mouse position to position in curve
                ImVec2 mouse_pos = ImGui::GetMousePos();
                float x_in_segment =
                    ((mouse_pos.x - (trackTopLeft.x + segmentX - segmentWidth)) / mState.mStepSize) / segment.mDuration;
                float y_in_segment = 1.0f - ((mouse_pos.y - trackTopLeft.y) / track_height);

                for(int i = 0; i < segment.mCurves.size(); i++)
                {
                    // evaluate curve at x position
                    float y_in_curve = segment.mCurves[i]->evaluate(x_in_segment);

                    // insert curve point on click
                    const float maxDist = 0.1f;
                    if(std::abs(y_in_curve - y_in_segment) < maxDist)
                    {
                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::HoveringCurve>(
                            track.mID,
                            segment.mID,
                            i);

                        if(ImGui::IsMouseClicked(1))
                        {
                            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::InsertingCurvePoint>(
                                track.mID,
                                segment.mID,
                                i, x_in_segment);
                        }
                        selected_curve = i;
                    }
                }

                if(selected_curve == -1)
                {
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                } else
                {
                    showValue<T>(
                        track,
                        segment, x_in_segment,
                        mState.mMouseCursorTime, selected_curve);
                }
            } else
            {
                if(mState.mAction->isAction<sequenceguiactions::HoveringCurve>())
                {
                    auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringCurve>();

                    if(action->mSegmentID == segment.mID)
                    {
                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                    }
                }
            }
        }

        if(needs_drawing)
        {
            for(int i = 0; i < segment.mCurves.size(); i++)
            {
                if(mCurveCache[segment.mID][i].size() > 0)
                {
                    float thickness = selected_curve == i ? 3.0f : 1.0f;
                    thickness *= mState.mScale;
                    drawList->AddPolyline
                        (
                            &*mCurveCache[segment.mID][i].begin(),                    // points array
                            mCurveCache[segment.mID][i].size(),                        // size of points array
                            mService.getColors().mCurveColors[i],                    // color
                            false,                                                    // closed
                            thickness                                                // thickness
                        );
                }
            }
        }
    }


    template<typename T>
    void SequenceCurveTrackView::drawSegmentContent(
            const SequenceTrack& track,
            const SequenceTrackSegment& segment,
            const ImVec2& trackTopLeft,
            float previousSegmentX,
            float segmentWidth,
            float segmentX,
            ImDrawList* drawList,
            bool drawStartValue)
    {
        // TODO : do this without the elif statement
        /**
         * Draw the selection background when the mouse is hovering the segment handlers
         */
        bool draw_selection_background = false;
        if(mState.mAction->isAction<sequenceguiactions::HoveringSegmentValue>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringSegmentValue>();
            draw_selection_background = action->mSegmentID == segment.mID;
        } else if(mState.mAction->isAction<sequenceguiactions::EditingCurveSegment>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::EditingCurveSegment>();
            draw_selection_background = action->mSegmentID == segment.mID;
        } else if(mState.mAction->isAction<sequenceguiactions::HoveringSegment>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringSegment>();
            draw_selection_background = action->mSegmentID == segment.mID;
        } else if(mState.mAction->isAction<sequenceguiactions::DraggingSegment>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingSegment>();
            draw_selection_background = action->mSegmentID == segment.mID;
        } else if(mState.mAction->isAction<sequenceguiactions::DraggingSegmentValue>())
        {
            auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingSegmentValue>();
            draw_selection_background = action->mSegmentID == segment.mID;
        }

        const float track_height = track.mTrackHeight * mState.mScale;

        // draw segment color
        const auto& curve_segment = static_cast<const SequenceTrackSegmentCurve<T>&>(segment);
        drawList->AddRectFilled(
                { trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
                { trackTopLeft.x + segmentX, trackTopLeft.y + track.mTrackHeight * mState.mScale }, // top right
                ImGui::ColorConvertFloat4ToU32(curve_segment.mColor)); // color

        if(draw_selection_background)
        {
            drawList->AddRectFilled(
                {trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y}, // top left
                {trackTopLeft.x + segmentX, trackTopLeft.y + track_height}, // top right
                ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.25f))); // color
        } else
        {
            // is this segment currently serialized in the clipboard
            if(mState.mClipboard->isClipboard<sequenceguiclipboard::CurveSegmentClipboard>())
            {
                // get derived clipboard
                auto *curve_segment_clipboard = mState.mClipboard->getDerived<sequenceguiclipboard::CurveSegmentClipboard>();

                // does it contain this segment ?
                if(curve_segment_clipboard->containsObject(segment.mID, getPlayer().getSequenceFilename()))
                {
                    ImVec4 red = ImGui::ColorConvertU32ToFloat4(mService.getColors().mHigh1);
                    red.w = 0.25f;
                    drawList->AddRectFilled
                        (
                            {trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y}, // top left
                            {trackTopLeft.x + segmentX, trackTopLeft.y + track_height},  // top right
                            ImGui::ColorConvertFloat4ToU32(red) // color
                        );
                }
            }
        }

        // draw curve(s)
        drawCurves<T>(
            track,
            segment,
            trackTopLeft,
            previousSegmentX,
            segmentWidth,
            segmentX,
            drawList);


        // draw control points
        drawControlPoints<T>(
            track,
            segment,
            trackTopLeft,
            segmentX,
            segmentWidth,
            drawList);

        // if this is the first segment of the track
        // also draw a handler for the start value
        if(drawStartValue)
        {
            // draw segment value handler
            drawSegmentValue<T>(
                track,
                segment,
                trackTopLeft,
                segmentX,
                segmentWidth,
                sequencecurveenums::ESegmentValueTypes::BEGIN,
                drawList);
        }

        // draw segment value handler
        drawSegmentValue<T>(
            track,
            segment,
            trackTopLeft,
            segmentX,
            segmentWidth,
            sequencecurveenums::ESegmentValueTypes::END,
            drawList);
    }


    template<typename T>
    void SequenceCurveTrackView::drawInspectorRange(const SequenceTrack &track)
    {
        const auto &curve_track = static_cast<const SequenceTrackCurve<T> &>(track);

        T min = curve_track.mMinimum;
        T max = curve_track.mMaximum;

        //
        ImGui::PushID(track.mID.c_str());
        float offset = 5.0f * mState.mScale;
        float drag_float_x = ImGui::GetCursorPosX() + (40.0f * mState.mScale);
        ImGui::SetCursorPos({ImGui::GetCursorPosX() + offset, ImGui::GetCursorPosY() + offset});
        ImGui::Text("Min:");
        ImGui::SameLine();
        ImGui::PushID("min");
        ImGui::SetCursorPosX(drag_float_x);
        if(inputFloat<T>(min, 3))
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::ChangeMinMaxCurve<T>>(track.mID, min, max);
        }
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
        ImGui::Text("Max:");
        ImGui::SameLine();
        ImGui::PushID("max");
        ImGui::SetCursorPosX(drag_float_x);
        if(inputFloat<T>(max, 3))
        {
            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::ChangeMinMaxCurve<T>>(track.mID, min, max);
        }
        ImGui::PopID();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }


    template<typename T>
    void SequenceCurveTrackView::drawSegmentValue(
            const SequenceTrack& track,
            const SequenceTrackSegment& segmentBase,
            const ImVec2& trackTopLeft,
            const float segmentX,
            const float segmentWidth,
            const sequencecurveenums::ESegmentValueTypes segmentType,
            ImDrawList* drawList)
    {
        static std::unordered_map<rttr::type, std::function<float(const SequenceTrackSegment &, int, sequencecurveenums::ESegmentValueTypes)>> get_value_map
            {
                {RTTI_OF(float),     [](const SequenceTrackSegment &segment, int curveIndex, sequencecurveenums::ESegmentValueTypes segmentType) -> float
                                     {
                                         return static_cast<const SequenceTrackSegmentCurve<float> *>(&segment)->getValue(
                                             segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f);
                                     }},
                {RTTI_OF(glm::vec2), [](const SequenceTrackSegment &segment, int curveIndex, sequencecurveenums::ESegmentValueTypes segmentType) -> float
                                     {
                                         return static_cast<const SequenceTrackSegmentCurve<glm::vec2> *>(&segment)->getValue(
                                             segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f)[curveIndex];
                                     }},
                {RTTI_OF(glm::vec3), [](const SequenceTrackSegment &segment, int curveIndex, sequencecurveenums::ESegmentValueTypes segmentType) -> float
                                     {
                                         return static_cast<const SequenceTrackSegmentCurve<glm::vec3> *>(&segment)->getValue(
                                             segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f)[curveIndex];
                                     }},
                {RTTI_OF(glm::vec4), [](const SequenceTrackSegment &segment, int curveIndex, sequencecurveenums::ESegmentValueTypes segmentType) -> float
                                     {
                                         return static_cast<const SequenceTrackSegmentCurve<glm::vec4> *>(&segment)->getValue(
                                             segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f)[curveIndex];
                                     }}
            };
        const float track_height = track.mTrackHeight * mState.mScale;

        assert(segmentBase.get_type().template is_derived_from<SequenceTrackSegmentCurve<T>>());
        const auto &segment = static_cast<const SequenceTrackSegmentCurve<T> &>(segmentBase);

        for(int v = 0; v < segment.mCurves.size(); v++)
        {
            // calculate point of this value in the window
            ImVec2 segment_value_pos =
                {
                    trackTopLeft.x + segmentX - (segmentType == sequencecurveenums::BEGIN ? segmentWidth : 0.0f),
                    trackTopLeft.y + track_height * (1.0f - ((segmentType == sequencecurveenums::BEGIN ?
                                                              (float) segment.mCurves[v]->mPoints[0].mPos.mValue :
                                                              (float) segment.mCurves[v]->mPoints[
                                                                  segment.mCurves[v]->mPoints.size() - 1].mPos.mValue) /
                                                             1.0f))
                };

            bool hovered = false;

            if(mState.mIsWindowFocused)
            {
                // check if we are hovering this value
                if((mState.mAction->isAction<sequenceguiactions::None>() ||
                    mState.mAction->isAction<sequenceguiactions::HoveringSegmentValue>() ||
                    mState.mAction->isAction<sequenceguiactions::HoveringSegment>() ||
                    mState.mAction->isAction<sequenceguiactions::HoveringCurve>()) &&
                   ImGui::IsMouseHoveringRect(
                       {segment_value_pos.x - (12.0f * mState.mScale),
                        segment_value_pos.y - (12.0f * mState.mScale)}, // top left
                       {segment_value_pos.x + (12.0f * mState.mScale),
                        segment_value_pos.y + (12.0f * mState.mScale)}))  // bottom right
                {
                    hovered = true;
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::HoveringSegmentValue>(
                        track.mID,
                        segment.mID,
                        segmentType,
                        v);

                    if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::DraggingSegmentValue>(
                            track.mID,
                            segment.mID,
                            segmentType,
                            v,
                            get_value_map[RTTI_OF(T)](segment, v, segmentType));
                    } else if(ImGui::IsMouseDown(1))
                    {
                        const auto &curve_segment = static_cast<const SequenceTrackSegmentCurve<T> &>(segment);
                        const auto &curve_track = static_cast<const SequenceTrackCurve<T> &>(track);

                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::EditingSegmentCurveValue<T>>(
                            track.mID,
                            segment.mID,
                            segmentType,
                            v,
                            (segmentType == sequencecurveenums::ESegmentValueTypes::BEGIN)
                            ? curve_segment.getStartValue() : curve_segment.getEndValue(),
                            curve_track.mMinimum,
                            curve_track.mMaximum);
                    }

                    showValue<T>(
                        track,
                        segment,
                        segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f,
                        segmentType == sequencecurveenums::BEGIN ? segment.mStartTime : segment.mStartTime +
                                                                                        segment.mDuration,
                        v);
                } else if(!mState.mAction->isAction<sequenceguiactions::DraggingSegmentValue>())
                {
                    if(mState.mAction->isAction<sequenceguiactions::HoveringSegmentValue>())
                    {
                        auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringSegmentValue>();

                        if(action->mType == segmentType &&
                           action->mSegmentID == segment.mID &&
                           action->mCurveIndex == v)
                        {
                            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();

                            showValue<T>(
                                track,
                                segment,
                                segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f,
                                segmentType == sequencecurveenums::BEGIN ? segment.mStartTime : segment.mStartTime +
                                                                                                segment.mDuration,
                                v);
                        }
                    }
                }

                // handle dragging segment value
                if(mState.mAction->isAction<sequenceguiactions::DraggingSegmentValue>())
                {
                    auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingSegmentValue>();
                    if(action->mSegmentID == segment.mID)
                    {
                        if(action->mType == segmentType && action->mCurveIndex == v)
                        {
                            hovered = true;
                            showValue<T>(
                                track,
                                segment,
                                segmentType == sequencecurveenums::BEGIN ? 0.0f : 1.0f,
                                segmentType == sequencecurveenums::BEGIN ? segment.mStartTime : segment.mStartTime +
                                                                                                segment.mDuration,
                                v);

                            float drag_amount = (mState.mMouseDelta.y / track_height) * -1.0f;
                            float value = get_value_map[RTTI_OF(T)](segment, v, segmentType) + drag_amount;

                            action->mNewValue = value;
                        }
                    }
                }
            }

            float circle_radius = 5.0f * mState.mScale;
            if(hovered)
                drawList->AddCircleFilled(segment_value_pos, circle_radius, mService.getColors().mCurveColors[v]);
            else
                drawList->AddCircle(segment_value_pos, circle_radius, mService.getColors().mCurveColors[v]);
        }
    }


    template<typename T>
    void SequenceCurveTrackView::drawControlPoints(
            const SequenceTrack& track,
            const SequenceTrackSegment& segmentBase,
            const ImVec2& trackTopLeft,
            const float segmentX,
            const float segmentWidth,
            ImDrawList* drawList)
    {

        const auto &segment = static_cast<const SequenceTrackSegmentCurve<T> &>(segmentBase);
        const float track_height = track.mTrackHeight * mState.mScale;

        // draw first control point(s) handlers IF this is the first segment of the track
        if(track.mSegments[0]->mID == segment.mID)
        {
            for(int v = 0; v < segment.mCurves.size(); v++)
            {
                const auto &curve_point = segment.mCurves[v]->mPoints[0];
                std::ostringstream string_stream;
                string_stream << segment.mID << "_point_" << 0 << "_curve_" << v;

                ImVec2 circle_point =
                    {(trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
                     trackTopLeft.y + track_height * (1.0f - (float) curve_point.mPos.mValue)};

                // only draw tan handlers when we have a bezier
                if(segment.mCurveTypes[v] == math::ECurveInterp::Bezier)
                {
                    drawTanHandler<T>(
                        track,
                        segment, string_stream,
                        segmentWidth, curve_point, circle_point,
                        0,
                        v,
                        sequencecurveenums::ETanPointTypes::IN,
                        drawList);

                    drawTanHandler<T>(
                        track,
                        segment, string_stream,
                        segmentWidth, curve_point, circle_point,
                        0,
                        v,
                        sequencecurveenums::ETanPointTypes::OUT,
                        drawList);
                }
            }
        }

        // draw control points of curves
        // we ignore the first and last because they are controlled by the start & end value of the segment
        for(int v = 0; v < segment.mCurves.size(); v++)
        {
            for(int i = 1; i < segment.mCurves[v]->mPoints.size() - 1; i++)
            {
                // get the curvepoint and generate a unique ID for the control point
                const auto &curve_point = segment.mCurves[v]->mPoints[i];
                std::ostringstream string_stream;
                string_stream << segment.mID << "_point_" << i << "_curve_" << v;
                std::string point_id = string_stream.str();

                // determine the point at where to draw the control point
                ImVec2 circle_point =
                    {(trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
                     trackTopLeft.y + track_height * (1.0f - (float) curve_point.mPos.mValue)};

                // handle mouse hovering
                bool hovered = false;
                float offset = 5.0f * mState.mScale;
                if(mState.mIsWindowFocused)
                {
                    if((mState.mAction->isAction<sequenceguiactions::None>() ||
                        mState.mAction->isAction<sequenceguiactions::HoveringControlPoint>() ||
                        mState.mAction->isAction<sequenceguiactions::HoveringCurve>())
                       && ImGui::IsMouseHoveringRect({circle_point.x - offset, circle_point.y - offset},
                                                     {circle_point.x + offset, circle_point.y + offset}))
                    {
                        hovered = true;
                    }
                }

                if(hovered)
                {
                    // if we are hovering this point, store ID
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::HoveringControlPoint>(
                        track.mID,
                        segment.mID,
                        i,
                        v);

                    // show value of current control point as tooltip
                    showValue<T>(
                        track,
                        segment, curve_point.mPos.mTime,
                        curve_point.mPos.mTime * segment.mDuration + segment.mStartTime,
                        v);

                    // is the mouse held down, then we are dragging
                    if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::DraggingControlPoint>(
                            track.mID,
                            segment.mID,
                            i,
                            v,
                            curve_point.mPos.mTime,
                            curve_point.mPos.mValue);
                    }
                        // if we clicked right mouse button, open curve action popup
                    else if(ImGui::IsMouseClicked(1))
                    {
                        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::CurvePointActionPopup<T>>(
                            track.mID,
                            segment.mID,
                            i,
                            v,
                            curve_point.mPos.mValue,
                            curve_point.mPos.mTime,
                            static_cast<const SequenceTrackCurve<T> &>(track).mMinimum,
                            static_cast<const SequenceTrackCurve<T> &>(track).mMaximum);
                    }
                } else
                {
                    // otherwise, if we where hovering but not anymore, stop hovering
                    if(mState.mAction->isAction<sequenceguiactions::HoveringControlPoint>())
                    {
                        auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringControlPoint>();
                        if(action->mControlPointIndex == i && track.mID == action->mTrackID &&
                           segment.mID == action->mSegmentID && v == action->mCurveIndex)
                        {
                            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                        }
                    }
                }

                if(mState.mIsWindowFocused)
                {
                    // handle dragging of control point
                    if(mState.mAction->isAction<sequenceguiactions::DraggingControlPoint>())
                    {
                        auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingControlPoint>();

                        if(action->mSegmentID == segment.mID)
                        {
                            if(action->mControlPointIndex == i && action->mCurveIndex == v)
                            {
                                float time_adjust = mState.mMouseDelta.x / segmentWidth;
                                float value_adjust = (mState.mMouseDelta.y / track_height) * -1.0f;

                                hovered = true;

                                showValue<T>(
                                    track,
                                    segment, curve_point.mPos.mTime,
                                    curve_point.mPos.mTime * segment.mDuration + segment.mStartTime,
                                    v);

                                action->mNewValue += value_adjust;
                                action->mNewTime += time_adjust;
                            }
                        }
                    }
                }

                // draw the control point
                drawList->AddCircleFilled
                    (
                        circle_point, 4.0f * mState.mScale,
                        hovered ? mService.getColors().mFro4 : mService.getColors().mFro2
                    );

                if(segment.mCurveTypes[v] == math::ECurveInterp::Bezier)
                {
                    // draw the handlers
                    drawTanHandler<T>(
                        track,
                        segment, string_stream,
                        segmentWidth, curve_point, circle_point,
                        i,
                        v,
                        sequencecurveenums::ETanPointTypes::IN,
                        drawList);

                    drawTanHandler<T>(
                        track,
                        segment, string_stream,
                        segmentWidth, curve_point, circle_point,
                        i,
                        v,
                        sequencecurveenums::ETanPointTypes::OUT,
                        drawList);
                }
            }
        }

        for(int v = 0; v < segment.mCurves.size(); v++)
        {
            // handle last control point
            // overlaps with endvalue so only draw tan handlers
            const int control_point_index = segment.mCurves[v]->mPoints.size() - 1;
            const auto &curve_point = segment.mCurves[v]->mPoints[control_point_index];

            std::ostringstream string_stream;
            string_stream << segment.mID << "_point_" << control_point_index << "_curve_" << v;
            std::string point_id = string_stream.str();

            ImVec2 circle_point =
                {(trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curve_point.mPos.mTime,
                 trackTopLeft.y + track_height * (1.0f - (float) curve_point.mPos.mValue)};

            if(segment.mCurveTypes[v] == math::ECurveInterp::Bezier)
            {
                drawTanHandler<T>(
                    track,
                    segment, string_stream,
                    segmentWidth, curve_point, circle_point, control_point_index,
                    v,
                    sequencecurveenums::ETanPointTypes::IN,
                    drawList);

                drawTanHandler<T>(
                    track,
                    segment, string_stream,
                    segmentWidth, curve_point, circle_point, control_point_index,
                    v,
                    sequencecurveenums::ETanPointTypes::OUT,
                    drawList);
            }
        }

        //
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - track_height);
    }


    template<typename T>
    void SequenceCurveTrackView::drawTanHandler(
            const SequenceTrack& track,
            const SequenceTrackSegment& segment,
            std::ostringstream& stringStream,
            const float segmentWidth,
            const math::FCurvePoint<float, float>& curvePoint,
            const ImVec2& circlePoint,
            const int controlPointIndex,
            const int curveIndex,
            const sequencecurveenums::ETanPointTypes type,
            ImDrawList* drawList)
    {
        const float track_height = track.mTrackHeight * mState.mScale;

        // draw tan handlers
        {
            // create a string stream to create identifier of this object
            std::ostringstream tan_stream;
            tan_stream << stringStream.str() << ((type == sequencecurveenums::ETanPointTypes::IN) ? "inTan" : "outTan");

            //
            const math::FComplex<float, float> &tan_complex = (type == sequencecurveenums::ETanPointTypes::IN)
                                                              ? curvePoint.mInTan : curvePoint.mOutTan;

            // get the offset from the tan
            const float tan_constant_size = 100.0f * mState.mScale;
            ImVec2 offset = {tan_complex.mTime * tan_constant_size,
                             static_cast<float>(tan_complex.mValue) * -1.0f * tan_constant_size};
            ImVec2 tan_point = {circlePoint.x + offset.x, circlePoint.y + offset.y};

            // set if we are hoverting this point with the mouse
            bool tan_point_hovered = false;
            float tan_bounds = mState.mScale * 5.0f;

            if(mState.mIsWindowFocused)
            {
                // check if hovered
                if((mState.mAction->template isAction<sequenceguiactions::None>() ||
                    mState.mAction->template isAction<sequenceguiactions::HoveringCurve>() ||
                    mState.mAction->template isAction<sequenceguiactions::HoveringSegment>())
                   && ImGui::IsMouseHoveringRect
                       (
                           {tan_point.x - tan_bounds, tan_point.y - tan_bounds},
                           {tan_point.x + tan_bounds, tan_point.y + tan_bounds})
                    )
                {
                    mState.mAction = sequenceguiactions::createAction<sequenceguiactions::HoveringTanPoint>(track.mID, tan_stream.str());
                    tan_point_hovered = true;
                } else if(mState.mAction->isAction<sequenceguiactions::HoveringTanPoint>())
                {
                    auto *action = mState.mAction->getDerived<sequenceguiactions::HoveringTanPoint>();

                    // if we hare already hovering, check if its this point
                    if(action->mTanPointID == tan_stream.str())
                    {
                        if(ImGui::IsMouseHoveringRect(
                            {tan_point.x - tan_bounds, tan_point.y - tan_bounds},
                            {tan_point.x + tan_bounds, tan_point.y + tan_bounds}))
                        {
                            // still hovering
                            tan_point_hovered = true;

                            // start dragging if mouse down
                            if(ImGui::IsMouseDown(0))
                            {
                                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::DraggingTanPoint>(
                                    track.mID,
                                    segment.mID,
                                    controlPointIndex,
                                    curveIndex,
                                    type);
                            } else if(ImGui::IsMouseDown(1)) // open edit popup
                            {
                                mState.mAction = sequenceguiactions::createAction<sequenceguiactions::EditingTanPointPopup>(
                                    track.mID,
                                    segment.mID,
                                    controlPointIndex,
                                    curveIndex,
                                    type,
                                    (float) tan_complex.mValue,
                                    tan_complex.mTime);
                            }
                        } else
                        {
                            // otherwise, release!
                            mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
                        }
                    }
                }

                // handle dragging of tan point
                if(mState.mAction->isAction<sequenceguiactions::DraggingTanPoint>())
                {
                    auto *action = mState.mAction->getDerived<sequenceguiactions::DraggingTanPoint>();

                    if(action->mSegmentID == segment.mID &&
                       action->mControlPointIndex == controlPointIndex &&
                       action->mType == type &&
                       action->mCurveIndex == curveIndex)
                    {
                        tan_point_hovered = true;

                        float delta_time = mState.mMouseDelta.x / tan_constant_size;
                        float delta_value = (mState.mMouseDelta.y / tan_constant_size) * -1.0f;

                        const auto &curve_segment = static_cast<const SequenceTrackSegmentCurve<T> &>(segment);

                        float new_time;
                        float new_value;
                        if(type == sequencecurveenums::ETanPointTypes::IN)
                        {
                            new_time =
                                curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mInTan.mTime + delta_time;
                            new_value = curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mInTan.mValue +
                                        delta_value;
                        } else
                        {
                            new_time = curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mOutTan.mTime +
                                       delta_time;
                            new_value = curve_segment.mCurves[curveIndex]->mPoints[controlPointIndex].mOutTan.mValue +
                                        delta_value;
                        }

                        action->mNewTime = new_time;
                        action->mNewValue = new_value;
                    }
                }
            }

            // draw line
            drawList->AddLine(circlePoint, tan_point, tan_point_hovered ? mService.getColors().mFro4
                                                                        : mService.getColors().mFro1,
                              1.0f * mState.mScale);

            // draw handler
            drawList->AddCircleFilled(tan_point, 3.0f * mState.mScale, tan_point_hovered ? mService.getColors().mFro4
                                                                                         : mService.getColors().mFro1);
        }
    }


    template<typename T>
    bool SequenceCurveTrackView::pasteClipboardSegments(const std::string &trackId, double time, utility::ErrorState &errorState)
    {
        // get clipboard action
        auto *curve_segment_clipboard = mState.mClipboard->getDerived<sequenceguiclipboard::CurveSegmentClipboard>();

        // create vector & object ptr to be filled by de-serialization
        std::vector<std::unique_ptr<rtti::Object>> read_objects;

        // continue upon succesfull de-serialization
        std::vector<T *> curve_segments = curve_segment_clipboard->deserialize<T>(read_objects, errorState);

        if(errorState.hasErrors())
        {
            nap::Logger::error(errorState.toString());
            return false;
        } else
        {
            assert(curve_segments.size() > 0); // no curve segments deserialized

            // sort curve segments by start time
            std::sort(curve_segments.begin(), curve_segments.end(), [](T *a, T *b)
            {
                return a->mStartTime < b->mStartTime;
            });

            // adjust start times by duration and start from 0.0
            if(curve_segments.size() > 0)
            {
                curve_segments[0]->mStartTime = 0.0;
            }
            for(int i = 1; i < curve_segments.size(); i++)
            {
                curve_segments[i]->mStartTime = curve_segments[i - 1]->mStartTime + curve_segments[i - 1]->mDuration;
            }

            //
            for(auto curve_segment: curve_segments)
            {
                // obtain controller
                auto *base_controller = getEditor().getControllerWithTrackID(trackId);

                // controller for track id not found
                assert(base_controller != nullptr);

                // upcast
                assert(base_controller->get_type().template is_derived_from<SequenceControllerCurve>());
                auto *curve_controller = static_cast<SequenceControllerCurve *>(base_controller);

                // insert new segment
                const auto *new_segment = curve_controller->insertSegment(trackId, time + curve_segment->mStartTime);

                // copy label
                curve_controller->changeSegmentLabel(trackId, new_segment->mID, curve_segment->mLabel);

                // change duration
                curve_controller->segmentDurationChange(trackId, new_segment->mID, curve_segment->mDuration, true);

                // update any segments that could be changed
                updateSegmentsInClipboard(trackId);

                // copy curve points & curve type
                for(int c = 0; c < curve_segment->mCurves.size(); c++)
                {
                    for(int i = 1; i < curve_segment->mCurves[c]->mPoints.size() - 1; i++)
                    {
                        curve_controller->insertCurvePoint(trackId, new_segment->mID, curve_segment->mCurves[c]->mPoints[i].mPos.mTime, c);
                    }

                    curve_controller->changeCurveType(trackId, new_segment->mID, curve_segment->mCurveTypes[c], c);
                }

                // change all curvepoints to match the copied clipboard curve segment
                // note that the first point is always determined by the previous segment
                for(int c = 0; c < curve_segment->mCurves.size(); c++)
                {
                    for(int i = 0; i < curve_segment->mCurves[c]->mPoints.size(); i++)
                    {
                        curve_controller->changeCurvePoint(trackId, new_segment->mID, i, c,
                                                           curve_segment->mCurves[c]->mPoints[i].mPos.mTime,
                                                           curve_segment->mCurves[c]->mPoints[i].mPos.mValue);

                        curve_controller->changeTanPoint(trackId, new_segment->mID, i, c, sequencecurveenums::IN,
                                                         curve_segment->mCurves[c]->mPoints[i].mInTan.mTime,
                                                         curve_segment->mCurves[c]->mPoints[i].mInTan.mValue);
                    }
                }

                // make the controller re-align start & end points of segments
                curve_controller->updateCurveSegments(trackId);

                // update any segments we have in the clipboard
                updateSegmentsInClipboard(trackId);
            }
        }

        return true;
    }


    template<typename T>
    void SequenceCurveTrackView::pasteClipboardSegmentInto(const std::string& trackId, const std::string& segmentId)
    {
        // get clipboard action
        auto *curve_segment_clipboard = mState.mClipboard->getDerived<sequenceguiclipboard::CurveSegmentClipboard>();

        // expect 1 object
        assert(curve_segment_clipboard->getObjectCount() == 1);

        // create vector & object ptr to be filled by de-serialization
        std::vector<std::unique_ptr<rtti::Object>> read_objects;
        std::vector<T *> curve_segments;

        // continue upon successful de-serialization
        utility::ErrorState errorState;
        curve_segments = curve_segment_clipboard->deserialize<T>(read_objects, errorState);

        if(!errorState.hasErrors())
        {
            T *curve_segment = curve_segments[0];

            assert(curve_segment != nullptr); // curve segment cannot be null here

            // obtain controller
            auto &curve_controller = getEditor().getController<SequenceControllerCurve>();

            // insert new segment
            const auto *target_segment = curve_controller.getSegment(trackId, segmentId);

            // upcast target segment to type of T
            assert(target_segment->get_type().template is_derived_from<T>()); // cannot upcast
            const T *target_segment_upcast = static_cast<const T *>(target_segment);

            // delete all points except the first and last one
            for(size_t c = 0; c < target_segment_upcast->mCurves.size(); c++)
            {
                for(size_t p = 1; p < target_segment_upcast->mCurves[c]->mPoints.size() - 1; p++)
                {
                    curve_controller.deleteCurvePoint(trackId, segmentId, p, c);
                }
            }

            // change duration
            curve_controller.segmentDurationChange(trackId, target_segment_upcast->mID, curve_segment->mDuration, true);

            // copy curve points & curve type
            for(int c = 0; c < curve_segment->mCurves.size(); c++)
            {
                for(int i = 1; i < curve_segment->mCurves[c]->mPoints.size() - 1; i++)
                {
                    curve_controller.insertCurvePoint(trackId, target_segment_upcast->mID, curve_segment->mCurves[c]->mPoints[i].mPos.mTime, c);
                }

                curve_controller.changeCurveType(trackId, target_segment_upcast->mID, curve_segment->mCurveTypes[c], c);
            }

            // change all curvepoints to match the copied clipboard curve segment
            // note that the first point is always determined by the previous segment
            for(int c = 0; c < curve_segment->mCurves.size(); c++)
            {
                for(int i = 0; i < curve_segment->mCurves[c]->mPoints.size(); i++)
                {
                    curve_controller.changeCurvePoint(trackId, target_segment_upcast->mID, i, c,
                                                      curve_segment->mCurves[c]->mPoints[i].mPos.mTime,
                                                      curve_segment->mCurves[c]->mPoints[i].mPos.mValue);

                    curve_controller.changeTanPoint(trackId, target_segment_upcast->mID, i, c, sequencecurveenums::IN,
                                                    curve_segment->mCurves[c]->mPoints[i].mInTan.mTime,
                                                    curve_segment->mCurves[c]->mPoints[i].mInTan.mValue);
                }
            }

            // make the controller re-align start & end points of segments
            curve_controller.updateCurveSegments(trackId);

            // update any segments we have in the clipboard
            updateSegmentsInClipboard(trackId);
        } else
        {
            nap::Logger::error(errorState.toString());
        }
    }


    template<typename T>
    void SequenceCurveTrackView::handleChangeMinMaxCurve()
    {
        // get action
        auto *action = mState.mAction->template getDerived<sequenceguiactions::ChangeMinMaxCurve<T>>();

        // take snapshot
        getEditor().takeSnapshot(action->get_type());

        // perform action
        auto &controller = getEditor().template getController<SequenceControllerCurve>();
        controller.template changeMinMaxCurveTrack<T>(action->mTrackID, action->mNewMin, action->mNewMax);

        // state is dirty
        mState.mDirty = true;

        // stop action
        mState.mAction = sequenceguiactions::createAction<sequenceguiactions::None>();
    }
}
