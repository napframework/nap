#pragma once

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // Sequence Audio Track Actions
    //////////////////////////////////////////////////////////////////////////

    namespace sequenceguiactions
    {
        /**
         * Action is invoked when user wants to insert a new audio segment
         */
        class InsertingAudioSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track where segment needs to be inserted
             * @param time the time at which to insert the segment
             * @param currentSelectedItem current selected item in popup combo box
             */
            InsertingAudioSegmentPopup(std::string trackID, double time, int currentSelectedItem)
                    : TrackAction(std::move(trackID)), mTime(time), mCurrentItem(currentSelectedItem)
            {}

        public:
            double mTime = 0.0;
            int mCurrentItem = 0;
        };

        /**
         * Action is invoked when user wants to edit an audio segment
         */
        class OpenEditAudioSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             * @param windowPos the window position the popup needs to get
             */
            OpenEditAudioSegmentPopup(std::string trackID, std::string segmentID, ImVec2 windowPos)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mWindowPos(windowPos)
            {}

        public:
            std::string mSegmentID;
            ImVec2 mWindowPos;
        };

        /**
         * Action is being invoked when user is inside an Edit Audio Segment popup
         */
        class EditingAudioSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             * @param windowPos the window position the popup needs to get
             */
            EditingAudioSegmentPopup(std::string trackID, std::string segmentID, ImVec2 windowPos)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mWindowPos(windowPos)
            {}

        public:
            std::string mSegmentID;
            ImVec2 mWindowPos;
        };

        /**
         * Action is invoked when user is hovering the left handler of the audio segment
         */
        class HoveringLeftAudioSegmentHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             */
            HoveringLeftAudioSegmentHandler(std::string trackID, std::string segmentID)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID))
            {}

        public:
            std::string mSegmentID;
        };

        /**
         * Action is invoked when user is dragging the left handler of the audio segment
         */
        class DraggingLeftAudioSegmentHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             */
            DraggingLeftAudioSegmentHandler(std::string trackID, std::string segmentID)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID))
            {}

        public:
            std::string mSegmentID;
        };

        /**
         * Action is invoked when user is hovering the right handler of the audio segment
         */
        class HoveringRightAudioSegmentHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             */
            HoveringRightAudioSegmentHandler(std::string trackID, std::string segmentID)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID))
            {}

        public:
            std::string mSegmentID;
        };

        /**
         * Action is invoked when user is dragging the right handler of the audio segment
         */
        class DraggingRightAudioSegmentHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track that contains the segment being edited
             * @param segmentID the segment id of the segment being edited
             */
            DraggingRightAudioSegmentHandler(std::string trackID, std::string segmentID)
                    : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID))
            {}

        public:
            std::string mSegmentID;
        };
    }
}
