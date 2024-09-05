/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "sequencecontroller.h"
#include "sequencetracksegmentevent.h"
#include "color.h"
#include "sequencetracksegmentcolor.h"

#include <nap/logger.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class SequenceService;

    /**
     * Controller class for color tracks and segments
     */
    class NAPAPI SequenceControllerColor : public SequenceController
    {
    RTTI_ENABLE(SequenceController)
    public:
        /**
         * Constructor
         * @param service reference to SequenceService
         * @param player reference to SequencePlayer
         * @param editor reference to SequenceEditor
         */
        SequenceControllerColor(SequenceService& service, SequencePlayer& player, SequenceEditor& editor)
                : SequenceController(service, player, editor)
        {
        }

        /**
         * Call this when changing the start time of a segment
         * @param trackID track id
         * @param segmentID segment id
         * @param time the time to change to
         * @return new start time
         */
        double segmentColorStartTimeChange(const std::string& trackID, const std::string& segmentID, double time);

        /**
         * Call this to insert a new segment
         * @param trackID track id
         * @param time the time to insert the segment
         * @param color the color of the segment
         * @return pointer to the new segment
         */
        const SequenceTrackSegment* insertColorSegment(const std::string& trackID, double time, const RGBAColorFloat& color);

        /**
         * Call this to change the color of a segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param color the new color
         */
        void changeSegmentColor(const std::string& trackID, const std::string& segmentID, const RGBAColorFloat& color);

        /**
         * Call this to change the blend method of a segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param blendMethod the new blend method
         */
        void changeSegmentColorBlendMethod(const std::string& trackID, const std::string& segmentID, SequenceTrackSegmentColor::EColorSpace blendMethod);

        /**
         * Call this to change the curve point of a segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param pointIndex the point index
         * @param value the new value
         */
        void changeSegmentCurvePoint(const std::string& trackID, const std::string& segmentID, int pointIndex, const glm::vec2& value);

        /**
         * Call this to change the curve type of a segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param curveType the new curve type
         */
        void changeSegmentCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp curveType);

        /**
         * Call this to change the curve tan point of a segment
         * @param trackID the track id
         * @param segmentID the segment id
         * @param pointIndex the point index
         * @param tanIndex the tan index (0 = in, 1 = out)
         * @param value the new value
         */
        void changeSegmentCurveTanPoint(const std::string& trackID, const std::string& segmentID, int pointIndex, int tanIndex, const glm::vec2& value);

        /**
         * Call this to delete a curve point
         * @param trackID the track id
         * @param segmentID the segment id
         * @param pointIndex the point index
         */
        void deleteCurvePoint(const std::string& trackID, const std::string& segmentID, int pointIndex);

        /**
         * Call this to insert a curve point
         * @param trackID the track id
         * @param segmentID the segment id
         * @param pos the new position
         */
        void insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos);

        /**
         * Call this to delete a segment
         * @param trackID the track id
         * @param segmentID the segment id
         */
        void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

        /**
         * Call this to add a new color track
         */
        void addNewColorTrack();

        /**
         * Call this to insert a new track
         * @param type the type of track to insert
         */
        void insertTrack(rttr::type type) override;
    };
}