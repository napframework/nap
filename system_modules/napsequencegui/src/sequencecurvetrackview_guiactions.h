#pragma once

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // Sequence Curve Track Actions
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiactions
    {
        /**
         * TrackAction that tells the GUI we're currently hovering a curve control point
         */
        class NAPAPI HoveringControlPoint : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track ID of the track holding the curve segment containing this controlpoint
             * @param segmentID segment ID of the curve segment containing this controlpoint
             * @param controlPointIndex control point index
             * @param curveIndex curve index
             */
            HoveringControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex)
            {}


            std::string mSegmentID;
            int mControlPointIndex;
            int mCurveIndex;
        };

        /**
         * TrackAction that tells the GUI we're currently dragging a curve control point
         */
        class DraggingControlPoint : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track ID of the track holding the curve segment containing this controlpoint
             * @param segmentID segment ID of the curve segment containing this controlpoint
             * @param controlPointIndex control point index
             * @param curveIndex curve index
             * @param newTime new time
             * @param newValue new value
             */
            DraggingControlPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, float newTime, float newValue)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex),
                  mCurveIndex(curveIndex), mNewTime(newTime), mNewValue(newValue)
            {}


            std::string mSegmentID;
            int mControlPointIndex;
            int mCurveIndex;
            float mNewTime;
            float mNewValue;
        };

        /**
         * TrackAction that tells the GUI we're currently hovering a tan point
         */
        class NAPAPI HoveringTanPoint : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track holding the tan point
             * @param tanPointID id of the tan point
             */
            HoveringTanPoint(std::string trackID, std::string tanPointID)
                : TrackAction(std::move(trackID)), mTanPointID(std::move(tanPointID))
            {}


            std::string mTanPointID;
        };

        /**
         * TrackAction that tells the GUI we're currently hovering a curve
         */
        class NAPAPI HoveringCurve : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackId the track id if the track that holds the curve segment
             * @param segmentID the segment id
             * @param curveIndex the curve index
             */
            HoveringCurve(std::string trackId, std::string segmentID, int curveIndex)
                : TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mCurveIndex(curveIndex)
            {}


            std::string mSegmentID;
            int mCurveIndex;
        };


        /**
         * TrackAction that tells the GUI we're inside an insert curve point popup
         */
        class NAPAPI InsertingCurvePoint : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track that contains the segment we want to edit
             * @param segmentID the segment id we want to edit
             * @param selectedCurve the selected curve index
             * @param pos normalized position on curve
             */
            InsertingCurvePoint(std::string trackID, std::string segmentID, int selectedCurve, float pos)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSelectedIndex(selectedCurve), mPos(pos)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            int mSelectedIndex;
            float mPos;
        };


        /**
         * TrackAction that tells the GUI we're inside an insert curve point action popup for a curved segment
         * @tparam T the type of curve segment
         */
        template<typename T>
        class CurvePointActionPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param controlPointIndex the control point index
             * @param curveIndex the curve index
             * @param value the new value of the curve point
             * @param time the new time of the curve point, time is in percentage normalized between 0 and 1. F.E. 0.5 is half of segment duration
             * @param minimum minimum value of the curve point
             * @param maximum maximum value of the curve point
             */
            CurvePointActionPopup(const std::string& trackID, std::string segmentID, int controlPointIndex, int curveIndex, float value, float time, T minimum, T maximum)
                : TrackAction(trackID), mSegmentID(std::move(segmentID)),
                  mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex),
                  mValue(value), mMinimum(minimum), mMaximum(maximum), mTime(time)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            int mControlPointIndex;
            int mCurveIndex;
            float mValue;
            T mMinimum;
            T mMaximum;
            float mTime;
        };


        /**
         * TrackAction that tells the GUI we're inside a curve type popup
         */
        class NAPAPI CurveTypePopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param index index of the curve
             * @param pos normalized position on the curve
             * @param windowPos current window position
             */
            CurveTypePopup(std::string trackID, std::string segmentID, int index, float pos, ImVec2 windowPos) :
                TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mCurveIndex(index), mPos(pos), mWindowPos(windowPos)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            int mCurveIndex;
            float mPos;
            ImVec2 mWindowPos;
        };

        /**
         * TrackAction that tells the GUI we're currently dragging a tan point
         */
        class NAPAPI DraggingTanPoint : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param controlPointIndex control point index
             * @param curveIndex curve index
             * @param type tan point type, in or out
             */
            DraggingTanPoint(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, sequencecurveenums::ETanPointTypes type)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type)
            {}


            std::string mSegmentID;
            int mControlPointIndex;
            int mCurveIndex;
            sequencecurveenums::ETanPointTypes mType;

            float mNewValue = 0.0f;
            float mNewTime = 0.0f;
        };


        /**
         * TrackAction that tells the GUI we're inside a tan point edit popup
         */
        class NAPAPI EditingTanPointPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param controlPointIndex control point index
             * @param curveIndex curve index
             * @param type tan point type, in or out
             * @param value new tan point value
             * @param time new tan point time
             */
            EditingTanPointPopup(std::string trackID, std::string segmentID, int controlPointIndex, int curveIndex, sequencecurveenums::ETanPointTypes type, float value, float time)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mControlPointIndex(controlPointIndex), mCurveIndex(curveIndex), mType(type), mValue(value), mTime(time)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            int mControlPointIndex;
            int mCurveIndex;
            sequencecurveenums::ETanPointTypes mType;
            float mValue;
            float mTime;
        };


        /**
         * TrackAction that tells the GUI we're inside an edit curve segment popup
         */
        class NAPAPI EditingCurveSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param segmentType segment type info
             * @param startTime new start time of segment
             * @param duration new duration of segment
             * @param label new label of segment
             */
            EditingCurveSegment(std::string trackID, std::string segmentID, const rttr::type &segmentType, double startTime, double duration, std::string label)
                : TrackAction(trackID), mSegmentID(segmentID), mSegmentType(segmentType), mStartTime(startTime), mDuration(duration), mSegmentLabel(label)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            rttr::type mSegmentType;
            double mStartTime;
            double mDuration;
            std::string mSegmentLabel;
        };


        /**
         * TrackAction that tells the GUI we're inside an edit curve value popup
         * @tparam T value type of curve
         */
        template<typename T>
        class EditingSegmentCurveValue : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param type segment value type, begin or end
             * @param curveIndex the index of the curve
             * @param value the new value
             * @param minimum the new minimum
             * @param maximum the new maximum
             */
            EditingSegmentCurveValue(const std::string& trackId, std::string segmentID,sequencecurveenums::ESegmentValueTypes type, int curveIndex, T value, T minimum, T maximum)
                : TrackAction(trackId), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex), mValue(value), mMinimum(minimum), mMaximum(maximum)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            sequencecurveenums::ESegmentValueTypes mType;
            int mCurveIndex;
            T mValue;
            T mMinimum;
            T mMaximum;
        };

        /**
         * A TrackAction that tells the GUI to change min max of the curved track
         * @tparam T value type of the curved track
         */
        template<typename T>
        class ChangeMinMaxCurve : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param newMin the new minimum value
             * @param newMax the new maximum value
             */
            ChangeMinMaxCurve(const std::string &trackID, T newMin, T newMax)
                : TrackAction(trackID), mNewMin(newMin), mNewMax(newMax)
            {}


            T mNewMin;
            T mNewMax;
        };

        /**
         * A TrackAction that tells the GUI we're hovering a segment value
         */
        class NAPAPI HoveringSegmentValue : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param type segment value, begin or end
             * @param curveIndex the index of the curve
             */
            HoveringSegmentValue(std::string trackId, std::string segmentID, sequencecurveenums::ESegmentValueTypes type, int curveIndex)
                : TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex)
            {}


            std::string mSegmentID;
            sequencecurveenums::ESegmentValueTypes mType;
            int mCurveIndex;
        };

        /**
         * A TrackAction that tells the GUI we're dragging a segment value
         */
        class DraggingSegmentValue : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track holding the segment being edited
             * @param segmentID the segment being edited
             * @param type segment value, begin or end
             * @param curveIndex the index of the curve
             * @param newValue the new value
             */
            DraggingSegmentValue(std::string trackId, std::string segmentID, sequencecurveenums::ESegmentValueTypes type, int curveIndex, float newValue)
                : TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex), mNewValue(newValue)
            {}


            std::string mSegmentID;
            sequencecurveenums::ESegmentValueTypes mType;
            int mCurveIndex;
            float mNewValue;
        };
    }
}
