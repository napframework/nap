/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencecurveenums.h"

// External Includes
#include <rtti/rtti.h>
#include <imgui/imgui.h>
#include <utility>

namespace nap
{
    /**
     * Types of possible interactions with GUI
     * Used by the gui state to handle mouse input / popups / actions
     */
    namespace sequenceguiactions
    {
        /**
         * Action base class
         */
        class NAPAPI Action
        {
        RTTI_ENABLE()
        public:
            virtual ~Action() = default;

            /**
             * @return true of action is of type
             */
            template<typename T>
            bool isAction(){ return this->get_type() == RTTI_OF(T); }

            /**
             * @return pointer to derived class. Static cast so will crash when not of derived type, use isAction<T> to check before calling this method
             */
            template<typename T>
            T* getDerived()
            {
                assert(isAction<T>());
                return static_cast<T *>(this);
            }

            // bool that tells the gui to take a snapshot of the current state
            bool mTakeSnapshot = true;
        };

        // shortcut to unique ptr of action class
        using SequenceActionPtr = std::unique_ptr<Action>;


        // use this method to create an action
        template<typename T, typename... Args>
        static SequenceActionPtr createAction(Args &&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }


        /**
         * None is an empty action
         * When there is currently no action in the GUI, its action is None
         */
        class NAPAPI None : public Action
        {
        RTTI_ENABLE()
        };


        /**
         * NonePressed is an action that happens when mouse is pressed when no action is active inside
         * the sequencer window. This is to prevent unintended actions to happen when mouse is pressed and then
         * dragged into the sequencer window
         */
        class NAPAPI NonePressed : public Action
        {
        RTTI_ENABLE()
        };


        /**
         * TrackAction is the base class for any action that happens on a track
         * Every track action has a track id, which identifies the track that the action applies to
         */
        class NAPAPI TrackAction : public Action
        {
        RTTI_ENABLE(Action)
        public:
            explicit TrackAction(std::string trackID) : mTrackID(std::move(trackID))
            {}

            std::string mTrackID;
        };


        /**
         * Action for dragging segments
         */
        class NAPAPI DraggingSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackId trackID of segment being dragged
             * @param segmentID segmentID of segment being dragged
             * @param newDuration segment duration
             * @param moveNextSegments if true, move next segments according to new duration
             */
            DraggingSegment(std::string trackId, std::string segmentID, double newDuration, bool moveNextSegments)
                : TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mNewDuration(newDuration), mMoveNextSegments(moveNextSegments)
            {}


            std::string mSegmentID;
            double mNewDuration;
            bool mMoveNextSegments = false;
        };


        /**
         * Action for start dragging segments
         */
        class NAPAPI StartDraggingSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
            * Constructor
            * @param trackId id of track being dragged
            * @param segmentID id of segment being dragged
            * @param startDuration drag start point
            * @param moveNextSegments if true, move next segments according to new duration
            */
            StartDraggingSegment(std::string trackId, std::string segmentID, double startDuration, bool moveNextSegments)
                : TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mStartDuration(startDuration), mMoveNextSegments(moveNextSegments)
            {}


            std::string mSegmentID;
            double mStartDuration;
            bool mMoveNextSegments = false;
        };


        /**
         * Action for when inside insert segment popup
         */
        class NAPAPI InsertingSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id
             * @param time the time at which a segment is being inserted
             * @param trackType trackType information about the track trackType
             */
            InsertingSegmentPopup(std::string trackID, double time, const rttr::type &trackType)
                : TrackAction(std::move(trackID)), mTime(time), mTrackType(trackType)
            {}

            bool mOpened = false;
            double mTime;
            rttr::type mTrackType;
            std::string mErrorString;
        };


        /**
         * Action when inside edit segment popup
         */
        class NAPAPI EditSegmentPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id of the track containing the segment being edited
             * @param segmentID the segment id being edited
             * @param segmentType type info about the segment being edited
             */
            EditSegmentPopup(std::string trackID, std::string segmentID, const rttr::type &segmentType)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType)
            {}

            bool mOpened = false;
            std::string mSegmentID;
            rttr::type mSegmentType;
        };


        /**
         * Action when hovering a segment
         */
        class NAPAPI HoveringSegment : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID track id of the track containing the segment we are hovering
             * @param segmentID segment id
             */
            HoveringSegment(std::string trackID, std::string segmentID)
                : TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID))
            {}


            std::string mSegmentID;
        };


        /**
         * When hovering mouse over player scrub bar
         */
        class NAPAPI HoveringPlayerTime : public Action
        {
        RTTI_ENABLE(Action)
        };


        /**
         * Action when dragging player controller
         */
        class NAPAPI DraggingPlayerTime : public Action
        {
        RTTI_ENABLE(Action)
        public:
            /**
             * Constructor
             * @param wasPlaying was the sequence player playing when we started dragging
             * @param wasPaused was the sequence player paused when we started dragging
             */
            DraggingPlayerTime(bool wasPlaying, bool wasPaused)
                : mWasPlaying(wasPlaying), mWasPaused(wasPaused)
            {}


            bool mWasPlaying;
            bool mWasPaused;
        };


        /**
         * Action of insert track popup
         */
        class NAPAPI InsertTrackPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            // indicates whether the handler should open the popup
            bool mOpened = false;
        };


        /**
         * Action when inside insert load popup
         */
        class LoadPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            int mSelectedShowIndex = 0;
            std::string mErrorString;
        };


        /**
         * Action when inside save as popup
         */
        class NAPAPI SaveAsPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            int mSelectedShowIndex = 0;
            std::string mErrorString;
        };


        /**
         * Action when hovering sequence duration handler
         */
        class NAPAPI HoveringSequenceDuration : public Action
        {
        RTTI_ENABLE(Action)
        public:
        };


        /**
         * Action when dragging sequence duration handler
         */
        class NAPAPI DraggingSequenceDuration : public Action
        {
        RTTI_ENABLE(Action)
        public:
        };


        /**
         * Action when inside sequence duration popup
         */
        class NAPAPI EditSequenceDurationPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            // indicates whether the handler should open the popup
            bool mOpened = false;
        };


        /**
         * Action when inside edit marker popup
         */
        class NAPAPI EditSequenceMarkerPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            /**
             * Constructor
             * @param id the id of the marker
             * @param message the marker message
             * @param time time at which to insert the marker in sequence ( seconds )
             */
            EditSequenceMarkerPopup(std::string id, std::string message, double time)
                : mID(std::move(id)), mMessage(std::move(message)), mTime(time)
            {}

            // marker id
            std::string mID;
            // indicates whether the handler should open the popup
            bool mOpened = false;
            // time of the marker
            double mTime;
            // marker message content
            std::string mMessage = "Message";
        };


        /**
         * Action of insert marker popup
         */
        class NAPAPI InsertingSequenceMarkerPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            /**
             * Constructor
             * @param time point in sequence where to insert marker ( seconds )
             */
            InsertingSequenceMarkerPopup(double time)
                : mTime(time)
            {}

            // indicates whether the handler should open the popup
            bool mOpened = false;
            // time of the marker
            double mTime;
            // marker message content
            std::string mMessage = "Message";
        };


        /**
         * Action when dragging marker
         */
        class NAPAPI DragSequenceMarker : public Action
        {
        RTTI_ENABLE(Action)
        public:
            /**
             * Constructor
             * @param id the id of the marker
             */
            explicit DragSequenceMarker(std::string id) : mID(std::move(id))
            {}


            std::string mID;
        };


        /**
         * Action when assigned a new output id to track
         */
        class NAPAPI AssignOutputIDToTrack : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackID the track id
             * @param outputID the output id
             */
            AssignOutputIDToTrack(const std::string& trackID, std::string outputID)
                : TrackAction(trackID), mOutputID(std::move(outputID))
            {}


            std::string mOutputID;

        };


        /**
         * Action that tells the gui its inside the help popup
         */
        class NAPAPI HelpPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            // indicates whether the handler should open the popup
            bool mOpened = false;
        };

        /**
         * Action that tells the gui to change horizontal resolution
         */
        class ChangeHorizontalResolution : public Action
        {
        RTTI_ENABLE(Action)
        public:
            /**
             * Constructor
             * @param newResolution the new horizontal resolution
             */
            ChangeHorizontalResolution(float newResolution) : mHorizontalResolution(newResolution)
            {}


            float mHorizontalResolution;
        };

        /**
         * Action that tells the gui to change the name of a track
         */
        class ChangeTrackName : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            ChangeTrackName(const std::string& id, const std::string& newName) : TrackAction(id), mNewTrackName(newName)
            {}


            std::string mNewTrackName;
        };

        /**
         * A TrackAction that tells the GUI we're hovering the bottom extension handler for trackheight
         */
        class HoveringTrackExtensionHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackId track id of the track holding the segment being edited
             */
            HoveringTrackExtensionHandler(std::string trackId) : TrackAction(std::move(trackId))
            {}
        };

        /**
         * A TrackAction that tells the GUI we're dragging the bottom extension handler for trackheight
         */
        class DraggingTrackExtensionHandler : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            /**
             * Constructor
             * @param trackId track id of the track holding the segment being edited
             */
            DraggingTrackExtensionHandler(std::string trackId) : TrackAction(std::move(trackId))
            {}
        };

        /**
         * An action that tells the GUI to show the save clipboard contents popup (presets)
         */
        class SavePresetPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
            SavePresetPopup(const std::string& filePath)
                : mFilePath(filePath)
            {}


            std::string mFilePath;
            int mSelectedPresetIndex = 0;
            std::string mErrorString;
        };

        /**
         * An action that tells the GUI to show to load clipboard contents popup (presets)
         */
        class LoadPresetPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            LoadPresetPopup(const std::string& id, double time, rtti::TypeInfo trackType)
                : TrackAction(id), mTime(time), mTrackType(trackType)
            {}


            std::string mFilePath;
            rtti::TypeInfo mTrackType;
            int mSelectedPresetIndex = 0;
            std::string mErrorString;
            double mTime;
        };

        /**
         * Track Options Popup
         */
        class TrackOptionsPopup : public TrackAction
        {
        RTTI_ENABLE(TrackAction)
        public:
            TrackOptionsPopup(const std::string& trackID, ImVec2 pos)
                    : TrackAction(trackID), mPos(std::move(pos))
            {}

            // indicates whether the handler should open the popup
            bool mPopupOpened = false;
            // position of the popup in screen coordinates
            ImVec2 mPos;
            // scroll at the moment of opening, popup closes when the user scrolls within the sequence editor gui
            float mScrollY = 0.0f;
        };


        /**
         * Action that tells the gui to open history popup
         */
        class NAPAPI OpenHistoryPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
        };

        /**
         * Action that tells the gui its inside the history popup
         */
        class NAPAPI SelectHistoryPopup : public Action
        {
        RTTI_ENABLE(Action)
        public:
        };

        /**
         * Action that tells the gui to perform an undo
         */
        class PerformUndo : public Action
        {
        RTTI_ENABLE(Action)
        public:
            PerformUndo(){}
        };

        /**
         * Action that tells the gui to perform a redo
         */
        class PerformRedo : public Action
        {
        RTTI_ENABLE(Action)
        public:
            PerformRedo(){}
        };
    }
}
