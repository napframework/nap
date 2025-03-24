/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencecontroller.h"
#include "sequencecurveenums.h"
#include "sequencetrackcurve.h"

// External Includes
#include <mathutils.h>

namespace nap {
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class SequenceService;

    /**
     * Controller class for curve tracks
     */
    class NAPAPI SequenceControllerCurve : public SequenceController
    {
        RTTI_ENABLE(SequenceController)
    public:
        /**
         * Constructor
         * @param service reference to service
         * @param player reference to player
         * @param editor reference to editor
         */
        SequenceControllerCurve(SequenceService& service, SequencePlayer& player, SequenceEditor& editor);

        /**
         * @param trackID the id of the track
         * @param segmentID the id of the segment we need to edit
         * @param duration the new duration
         * @param adjustFollowingSegments if true, following segments will be moved
         * @return new duration of segment
          */
         double segmentDurationChange(const std::string& trackID, const std::string& segmentID, float duration, bool adjustFollowingSegments);

         /**
          * Sets the segment locked property
          * @param trackID the track id
          * @param segmentID the segment id
          * @param locked the locked value
          */
         void setSegmentLocked(const std::string& trackID, const std::string& segmentID, bool locked);

        /**
         * adds a new curve track of type T ( float, vec2, vec3, vec4 )
         */
        template<typename T>
        void addNewCurveTrack();

        /**
         * changes start or end value of segment of type T
         * @param trackID the track id
         * @param segmentID id of segment
         * @param newValue the new value
         * @param curveIndex the curve index of the value
         * @param valueType the segment value type ( first or last value )
         */
        void changeCurveSegmentValue(const std::string& trackID, const std::string& segmentID, float newValue,
                                     int curveIndex, sequencecurveenums::ESegmentValueTypes valueType);

        /**
         * insert point in curve of segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param pos the position at which to insert the curvepoint in curve ( range 0-1 )
         * @param curveIndex the index of the curve
         */
        void insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos, int curveIndex);

        /**
         * deletes point from curve
         * @param trackID the trackID
         * @param segmentID the segmentID
         * @param index the point index
         * @param curveIndex the curveIndex
         */
        void deleteCurvePoint(const std::string& trackID, const std::string& segmentID, int index, int curveIndex);

        /**
         * changes a curvepoint value and time / position
         * @param trackID the trackID
         * @param segmentID the segmentID
         * @param pointIndex the point index
         * @param curveIndex the curve index
         * @param time new time in percentage of segment duration normalized between 0 and 1
         * @param value new value
         */
        void changeCurvePoint(const std::string& trackID, const std::string& segmentID, int pointIndex, int curveIndex,
                              float time, float value);

        /**
         * changes tangent of curve point. Tangents are always aligned
         * @param trackID the trackID
         * @param segmentID the segmentID
         * @param pointIndex the point index
         * @param curveIndex the curve index
         * @param tanType in or out tangent
         * @param time offset for new time
         * @param value offset for new value
         * @return if true, tangents have been flipped
         */
        bool changeTanPoint(const std::string& trackID, const std::string& segmentID, int pointIndex, int curveIndex,
                            sequencecurveenums::ETanPointTypes tanType, float time, float value);

        /**
         * changes minimum and maximum value of track
         * @param trackID the trackID
         * @param minimum new minimum
         * @param maximum new maximum
         */
        template<typename T>
        void changeMinMaxCurveTrack(const std::string& trackID, T minimum, T maximum);

        /**
         * changes curvetype ( linear or bezier )
         * @param segment the segment
         * @param type the new curve type
         * @param curveIndex index of the curve
         */
        template<typename T>
        void changeCurveType(SequenceTrackSegment& segment, math::ECurveInterp type, int curveIndex);

        /**
         * overloaded insert segment function
         * @param trackID the track id
         * @param time time
         * @return const pointer to newly created segment
         */
        const SequenceTrackSegment* insertSegment(const std::string& trackID, double time) override;

        /**
         * overloaded delete segment function
         * @param trackID track id
         * @param segmentID segment id
         */
        void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

        /**
         * overloaded insert track function
         * @param type track type
         */
        void insertTrack(rtti::TypeInfo type) override;

        /**
         * updates curve segments values to be continuous ( segment 1 end value == segment 2 start value etc )
         * @param trackID the track id of the track that we want to update
         */
        void updateCurveSegments(const std::string& trackID);

        /**
         * changes curvetype ( linear or bezier )
         * @param trackID the trackID
         * @param segmentID the segmentID
         * @param type the new curve type
         * @param curveIndex the index of the curve
         */
        void changeCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp type,
                             int curveIndex);

        /**
        * changes the color of a curve segment
        * @param trackID the trackID
        * @param segmentID the segmentID
        * @param newColor the new color
        */
        void changeSegmentColor(const std::string& trackID, const std::string& segmentID, const RGBAColorFloat& newColor);

    protected:
        /**
         * updates curve segments values to be continuous ( segment 1 end value == segment 2 start value etc )
         * @param track reference to sequence track
         */
        template<typename T>
        void updateCurveSegments(SequenceTrack& track);

        /**
         * insertCurveSegment
         * inserts a new curvesegment of type T ( vec2, vec3, vec4, float )
         * @param trackID the track in which to insert the segment
         * @param time the time at when to insert segment
         */
        template<typename T>
        const SequenceTrackSegment* insertCurveSegment(const std::string& trackID, double time);

        /**
         * changes tangent of curve point. Tangents are always aligned
         * @param segment the segment
         * @param trackID the trackID
         * @param pointIndex the point index
         * @param curveIndex the curve index
         * @param tanType in or out tangent
         * @param time offset for new time
         * @param value offset for new value
         * @return if tangents have been flipped
         */
        template<typename T>
        bool changeTanPoint(SequenceTrackSegment& segment, const std::string& trackID, int pointIndex, int curveIndex,
                            sequencecurveenums::ETanPointTypes tanType, float time, float value);

        /**
         * changes a curvepoint value and time / position
         * @param segment the segmentID
         * @param pointIndex the point index
         * @param curveIndex the curve index
         * @param time offset for new time
         * @param value offset for new value
         */
        template<typename T>
        void changeCurvePoint(SequenceTrackSegment& segment, int pointIndex, int curveIndex, float time, float value);

        template<typename T>
        void changeLastCurvePoint(SequenceTrackSegment& segment, int curveIndex, float time, float value);

        /**
         * deletes point from curve
         * @param segment the segmentID
         * @param index the point index
         * @param curveIndex the curveIndex
         */
        template<typename T>
        void deleteCurvePoint(SequenceTrackSegment& segment, int index, int curveIndex);

        /**
         * insert point in curve of segment
         * @param segment the segment id
         * @param pos the position at which to insert the curvepoint in curve ( range 0-1 )
         * @param curveIndex the index of the curve
         */
        template<typename T>
        void insertCurvePoint(SequenceTrackSegment& segment, float pos, int curveIndex);

        /**
         * changes start or end value of segment of type T
         * @param track the track id
         * @param segment id of segment
         * @param newValue new value
         * @param curveIndex the curve index of the value
         * @param valueType the segment value type ( first or last value )
         */
        template<typename T>
        void changeCurveSegmentValue(SequenceTrack& track, SequenceTrackSegment& segment, float newValue, int curveIndex, sequencecurveenums::ESegmentValueTypes valueType);

        // map for updating segments
        std::unordered_map<rttr::type, std::function<void(SequenceTrack&)>> mUpdateSegmentFunctionMap;

        // map for inserting segments
        std::unordered_map<rttr::type, std::function<const SequenceTrackSegment*(const std::string&,double)>> mInsertSegmentFunctionMap;

        // maps containing function pointers to templated functions
        // the reason for this is that actions are not templated and when handled by the gui contain rtti type information
        // in order to call the right templated controller function, function calls are stored in a map with type information as key value
        // this makes it also possible to add other curved tracks when extending the controller
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceControllerCurve*)>> mInsertTrackFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<bool(SequenceTrackSegment&, const std::string&, const int, const int, sequencecurveenums::ETanPointTypes, float, float)>> mChangeTanPointFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrackSegment&, const int, const int, float, float)>> mChangeCurvePointFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrackSegment&, const int, float, float)>> mChangeLastCurvePointFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrackSegment&, const int, int)>> mDeleteCurvePointFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrackSegment&, float, int)>> mInsertCurvePointFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrack&, SequenceTrackSegment& segment, float, int, sequencecurveenums::ESegmentValueTypes)>> mChangeSegmentValueFunctionMap;
        std::unordered_map<rtti::TypeInfo, std::function<void(SequenceTrackSegment&, math::ECurveInterp type, int curveIndex)>> mChangeCurveTypeFunctionMap;
    };

    //////////////////////////////////////////////////////////////////////////
    // Forward declarations
    //////////////////////////////////////////////////////////////////////////

    template<>
    void NAPAPI SequenceControllerCurve::changeMinMaxCurveTrack<float>(const std::string& trackID, float minimum, float maximum);
}

#include "sequencecontrollercurve_template.h"
