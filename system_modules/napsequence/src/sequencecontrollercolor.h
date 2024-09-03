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
     * Controller class for event tracks
     */
    class NAPAPI SequenceControllerColor : public SequenceController
    {
    RTTI_ENABLE(SequenceController)
    public:
        SequenceControllerColor(SequenceService& service, SequencePlayer& player, SequenceEditor& editor)
                : SequenceController(service, player, editor)
        {
        }

        double segmentColorStartTimeChange(const std::string& trackID, const std::string& segmentID, double time);

        const SequenceTrackSegment* insertSegment(const std::string& trackID, double time) override;

        const SequenceTrackSegment* insertColorSegment(const std::string& trackID, double time, const RGBAColorFloat& color);

        void changeSegmentColor(const std::string& trackID, const std::string& segmentID, const RGBAColorFloat& color);

        void changeSegmentColorBlendMethod(const std::string& trackID, const std::string& segmentID, SequenceTrackSegmentColor::EColorSpace blendMethod);

        void changeSegmentCurvePoint(const std::string& trackID, const std::string& segmentID, int pointIndex, const glm::vec2& value);

        void changeSegmentCurveType(const std::string& trackID, const std::string& segmentID, math::ECurveInterp curveType);

        void changeSegmentCurveTanPoint(const std::string& trackID, const std::string& segmentID, int pointIndex, int tanIndex, const glm::vec2& value);

        void deleteCurvePoint(const std::string& trackID, const std::string& segmentID, int pointIndex);

        void insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos);

        void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

        void addNewColorTrack();

        void insertTrack(rttr::type type) override;
    };
}