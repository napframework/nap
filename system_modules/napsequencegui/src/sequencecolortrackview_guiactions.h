#pragma once

#include <utility>

#include "sequenceeditorguiactions.h"
#include "sequencetracksegmentcolor.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // Color Actions
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiactions
    {
        /**
         * TrackAction that tells the GUI we're currently in an insert color segment popup
         */
        class NAPAPI InsertColorSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track where the new segment should be inserted
             * @param time the time at which to insert the track
             */
            InsertColorSegmentPopup(const std::string& trackID, double time)
                    : TrackAction(trackID), mTime(time)
            {}

            bool mOpened = false;
            double mTime;
            RGBAColorFloat mValue;
            std::string mMessage = "hello world";
            std::string mErrorString;
        };


        class OpenEditColorSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that holds the segment being edited
             * @param segmentID the segment id being edited
             * @param windowPos current window position
             * @param value the new value of the event segment
             * @param startTime the new start time of the event segment
             */
            OpenEditColorSegmentPopup(const std::string& trackID, std::string segmentID, ImVec2 windowPos, RGBAColorFloat  value, double startTime)
                    : TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(std::move(value)), mStartTime(startTime)
            {}

            std::string mSegmentID;
            ImVec2 mWindowPos;
            RGBAColorFloat mValue;
            double mStartTime;
        };


        class EditingColorSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that holds the segment being edited
             * @param segmentID the segment id being edited
             * @param windowPos current window position
             * @param value the new value of the event segment
             * @param startTime the new start time of the event segment
             */
            EditingColorSegment(const std::string& trackID, std::string segmentID, ImVec2 windowPos, RGBAColorFloat value, SequenceTrackSegmentColor::EColorSpace blendMethod, double startTime)
                    : TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(std::move(value)), mBlendMethod(blendMethod), mStartTime(startTime)
            {}

            bool mPopupOpened = false;
            bool mTakeSnapshot = true;
            std::string mSegmentID;
            ImVec2 mWindowPos;
            RGBAColorFloat mValue;
            SequenceTrackSegmentColor::EColorSpace mBlendMethod = SequenceTrackSegmentColor::EColorSpace::OKLAB;
            double mStartTime;
        };

        class HoveringCurveColorSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            HoveringCurveColorSegment(const std::string& trackID, std::string segmentID, ImVec2 windowPos)
                    : TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos)
            {}

            std::string mSegmentID;
            ImVec2 mWindowPos;
        };

        class EditColorCurvePopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            EditColorCurvePopup(const std::string& trackID, std::string segmentID, ImVec2 windowPos, float timeInCurve)
                    : TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mTimeInCurve(timeInCurve)
            {}

            bool mPopupOpened = false;
            float mTimeInCurve = 0.0f;
            std::string mSegmentID;
            ImVec2 mWindowPos;
        };
    }
}
