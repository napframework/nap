/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility>

#include "sequencetrackview.h"
#include "sequencetracksegmentcurve.h"
#include "sequencecontrollercurve.h"
#include "sequenceguiservice.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * SequenceCurveTrackView shows and handles contents for curve tracks
     */
    class NAPAPI SequenceCurveTrackView final : public SequenceTrackView
    {
    RTTI_ENABLE(SequenceTrackView)
    public:
        /**
         * Constructor
         * @param service sequence gui service handle
         * @param view reference to editor view
         * @param state reference to editor state
         */
        SequenceCurveTrackView(SequenceGUIService& service, SequenceEditorGUIView& view, SequenceEditorGUIState& state);

        // make this class explicitly non-copyable
        SequenceCurveTrackView(const SequenceCurveTrackView&) = delete;
        SequenceCurveTrackView& operator=(const SequenceCurveTrackView&) = delete;

        /**
         * Handles any actions
         */
        void handleActions() override;

    protected:
        /**
         * draws the contents of a segment
         * @tparam T the type of this segment
         * @param track reference to track
         * @param segment reference to segment
         * @param trackTopLeft topleft position of track
         * @param previousSegmentX the x position of the previous segment
         * @param segmentWidth the width of this segment
         * @param segmentX the x position of this segment
         * @param drawList pointer to drawlist of this track window
         * @param drawStartValue should we draw the start value ? only used in first segment of track
         */
        template<typename T>
        void drawSegmentContent(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList, bool drawStartValue);

        /**
         * draws a segments value
         * @tparam T type of segment
         * @param track reference to track
         * @param segment reference to segment
         * @param trackTopLeft tracks topleft position
         * @param segmentX segment x position
         * @param segmentWidth width of segment
         * @param segmentType type of segment
         * @param drawList pointer to window drawlist
         */
        template<typename T>
        void drawSegmentValue(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2& trackTopLeft, float segmentX, float segmentWidth, sequencecurveenums::ESegmentValueTypes segmentType, ImDrawList* drawList);

        /**
         * draws segment handler
         * @param track reference to track
         * @param segment reference to segment
         * @param trackTopLeft tracks topleft position
         * @param segmentX segment x position
         * @param segmentWidth width of segment
         * @param drawList pointer to window drawlist
         */
        void drawSegmentHandler(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2& trackTopLeft, float segmentX, float segmentWidth, ImDrawList* drawList);

        /**
         * draws control points of curve segment
         * @param track reference to track
         * @param segment reference to segment
         * @param trackTopLeft tracks topleft position
         * @param segmentX segment x position
         * @param segmentWidth width of segment
         * @param drawList pointer to window drawlist
         */
        template<typename T>
        void drawControlPoints(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2& trackTopLeft, float segmentX, float segmentWidth, ImDrawList* drawList);

        /**
         * draws curves of segment
         * @tparam T the type of this segment
         * @param track reference to track
         * @param segment reference to segment
         * @param trackTopLeft topleft position of track
         * @param previousSegmentX the x position of the previous segment
         * @param segmentWidth the width of this segment
         * @param segmentX the x position of this segment
         * @param drawList pointer to drawlist of this track window
         */
        template<typename T>
        void drawCurves(const SequenceTrack& track, const SequenceTrackSegment& segment, const ImVec2& trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList* drawList);

        /**
         * draws handlers of curve point
         * @tparam T type of segment
         * @param track reference to track
         * @param segment reference to segment
         * @param stringStream stringstream, used to keep track of object id
         * @param segmentWidth width of segment
         * @param curvePoint reference to curvePoint
         * @param circlePoint circlePoint position
         * @param controlPointIndex index of control point
         * @param curveIndex index of curve
         * @param type tangent type ( in or out )
         * @param drawList pointer to window drawlist
         */
        template<typename T>
        void drawTanHandler(const SequenceTrack& track,
                            const SequenceTrackSegment& segment,
                            std::ostringstream& stringStream,
                            float segmentWidth,
                            const math::FCurvePoint<float, float>& curvePoint,
                            const ImVec2& circlePoint, int controlPointIndex,
                            int curveIndex,
                            sequencecurveenums::ETanPointTypes type,
                            ImDrawList* drawList);

        /**
         * handles insert segment popup
         */
        void handleInsertSegmentPopup();

        /**
         * handles delete segment popup
         */
        void handleEditSegmentPopup();

        /**
         * handles insert curve point popup
         */
        void handleInsertCurvePointPopup();

        /**
         * handles curvepoint action popup
         */
        template<typename T>
        void handleCurvePointActionPopup();

        /**
         * handles segment value actions
         */
        template<typename T>
        void handleSegmentValueActionPopup();

        /**
         * handles changing of min max values of curve track
         */
        template<typename T>
        void handleChangeMinMaxCurve();

        /**
         * handles dragging of tan points
         */
        void handleDragTanPoint();

        /**
         * handles assigning new object id to track
         */
        void handleAssignOutputIDToTrack();

        /**
         * handle dragging of segment handlers
         */
        void handleDragSegmentHandler();

        /**
         * handles curve type popup
         */
        void handleCurveTypePopup();

        /**
         * handles tanpoint action popup
         */
        void handleTanPointActionPopup();

        /**
         * handles dragging of segment values
         */
        void handleDraggingSegmentValue();

        /**
         * handles dragging of control points
         */
        void handleDraggingControlPoints();

        /**
         * handles loading of popups
         */
        void handleLoadPresetPopup();

        /**
         * Draws min/max range of inspector
         * @tparam T type
         * @param track reference to track
         */
        template<typename T>
        void drawInspectorRange(const SequenceTrack& track);

        /**
         * @tparam T type of value
         * @param track reference to track
         * @param segment reference to segment
         * @param x value to display
         * @param time time in segment
         * @param curveIndex curve index
         */
        template<typename T>
        void showValue(const SequenceTrack& track, const SequenceTrackSegmentCurve<T>& segment, float x, double time, int curveIndex);

        /**
         * input float that takes type T as input
         * @tparam T type of inputFloat
         * @param precision decimal precision
         * @return true if dragged
         */
        template<typename T>
        bool inputFloat(T&, int precision);

        /**
         * show inspector content
         * @param track reference to track
         */
        void showInspectorContent(const SequenceTrack& track) override;

        /**
         * shows track content
         * @param track reference to track
         * @param trackTopLeft orientation
         */
        void showTrackContent(const SequenceTrack& track, const ImVec2& trackTopLeft) override;

        /**
         * pastes current clipboard as new segments at given time
         * @param trackId the track id of the track to insert into
         * @param time the time at which to create new segment
         * @param errorState contains any errors
         * @return true on success
         */
        template<typename T>
        bool pasteClipboardSegments(const std::string& trackId, double time, utility::ErrorState& errorState);

        /**
         * pastes content of clipboard segment into another segment
         * @param trackID the track id of the track to insert into
         * @param segmentID the segment id of the segment to replace
         */
        template<typename T>
        void pasteClipboardSegmentInto(const std::string& trackID, const std::string& segmentID);

        /**
         * updates segment in clipboard with contents of segment in track
         * @param trackID the track id of the segment in clipboard
         * @param segmentID the segment id of the segment in clipboard
         */
        void updateSegmentInClipboard(const std::string& trackID, const std::string& segmentID);

        /**
         * iterates of all segments in clipboard of specified track and updates them with the ones in the specified track
         * @param trackID the track id containing segments in clipboard
         */
        void updateSegmentsInClipboard(const std::string& trackID);

        // curve cache holds evaluated curves, needs to be cleared when view changes and curves need to be redrawn
        std::unordered_map<std::string, std::vector<std::vector<ImVec2>>> mCurveCache;

        // short curt to member function drawSegmentContent
        using DrawSegmentMemFunPtr = void (SequenceCurveTrackView::*)(const SequenceTrack &track, const SequenceTrackSegment &segment, const ImVec2 &trackTopLeft, float previousSegmentX, float segmentWidth, float segmentX, ImDrawList *drawList, bool drawStartValue);

        // static map of member function pointers
        static std::unordered_map<rttr::type, DrawSegmentMemFunPtr> &getDrawCurveSegmentsMap();
    };

    //////////////////////////////////////////////////////////////////////////
    // Curve Clipboards
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiclipboard
    {
        /**
         * CurveSegmentClipboard contains serialized curve segments
         */
        class NAPAPI CurveSegmentClipboard final : public Clipboard
        {
        RTTI_ENABLE(Clipboard)
        public:
            /**
             * Constructor
             * @param segmentType the segment type that needs to be serialized
             * @param trackID the track id of track that contains the segment
             * @param sequenceName the name of the current loaded sequence
             */
            CurveSegmentClipboard(const rttr::type &segmentType, std::string trackID, std::string sequenceName)
                : Clipboard(segmentType), mTrackID(std::move(trackID)), mSequenceName(std::move(sequenceName))
            {}


            /**
             * Change track id
             * @param newTrackID the new track id
             */
            void changeTrackID(const std::string &newTrackID);


            /**
             * returns track id
             * @return the track id that contains the segment
             */
            const std::string& getTrackID() const{ return mTrackID; }


            /**
             * returns sequence name that contains the segment
             * @return the sequence name that contains the segment
             */
            const std::string& getSequenceName() const { return mSequenceName; }


        private:
            std::string mTrackID;
            std::string mSequenceName;
        };
    }


    //////////////////////////////////////////////////////////////////////////
    // Forward declarations
    //////////////////////////////////////////////////////////////////////////

    template<>
    void SequenceCurveTrackView::handleCurvePointActionPopup<float>();

    template<>
    void SequenceCurveTrackView::handleSegmentValueActionPopup<float>();

    template<>
    void SequenceCurveTrackView::showValue<float>(const SequenceTrack& track,
                                                  const SequenceTrackSegmentCurve<float>& segment, float x, double time,
                                                  int curveIndex);

    template<>
    void SequenceCurveTrackView::showValue<glm::vec2>(const SequenceTrack& track, const SequenceTrackSegmentCurve<glm::vec2>& segment, float x, double time, int curveIndex);

    template<>
    void SequenceCurveTrackView::showValue<glm::vec3>(const SequenceTrack& track, const SequenceTrackSegmentCurve<glm::vec3>& segment, float x, double time, int curveIndex);

    template<>
    void SequenceCurveTrackView::showValue<glm::vec4>(const SequenceTrack& track, const SequenceTrackSegmentCurve<glm::vec4>& segment, float x, double time, int curveIndex);

    template<>
    bool SequenceCurveTrackView::inputFloat<float>(float& v, int precision);

    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec2>(glm::vec2& v, int precision);

    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec3>(glm::vec3& v, int precision);

    template<>
    bool SequenceCurveTrackView::inputFloat<glm::vec4>(glm::vec4& v, int precision);
}

// Include all template definitions
#include "sequencecurvetrackview_template.h"


