/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencecurveenums.h"

// External Includes
#include <rtti/rtti.h>

#include <utility>

namespace nap
{
	/**
	 * Types of possible interactions with GUI
	 * Used by the gui state to handle mouse input / popups / actions
	 */
	namespace SequenceGUIActions
	{
		/**
		 * Action base class
		 */
		class Action
		{
			RTTI_ENABLE()
		public:
            virtual ~Action()= default;
            
			/**
			 * @return true of action is of type
			 */
			template<typename T>
			bool isAction()
			{
				return this->get_type() == RTTI_OF(T);
			}

			/**
			 * @return pointer to derived class. Static cast so will crash when not of derived type, use isAction<T> to check before calling this method
			 */
			template<typename T>
			T* getDerived()
			{
				assert(isAction<T>());
				return static_cast<T*>(this);
			}
		};

		// shortcut to unique ptr of action class
		using SequenceActionPtr = std::unique_ptr<Action>;

		// use this method to create an action
		template<typename T, typename... Args>
		static SequenceActionPtr createAction(Args&&... args)
		{
			return std::make_unique<T>(std::forward<Args>(args)...);
		}

		/**
		 * None is an empty action
		 * When there is currently no action in the GUI, its action is None
		 */
		class None : public Action { RTTI_ENABLE() };

		/**
		 * None pressed is an action that happens when mouse is pressed when no action is active inside
		 * the sequencer window. This is to prevent un-intented actions to happen when mouse is pressed and then
		 * dragged into the sequencer window
		 */
		class NonePressed : public Action{ RTTI_ENABLE() };

		/**
		 * TrackAction is the base class for any action that happens on a track
		 * Every track action has a track id, which identifies the track that the action applies to
		 */
		class TrackAction : public Action
		{
			RTTI_ENABLE(Action)
		public:
			explicit TrackAction(std::string trackID) : mTrackID(std::move(trackID)){}

			std::string mTrackID;
		};

		/**
		 * Action for dragging segments
		 */
		class DraggingSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackId trackID of segment being dragged
			 * @param segmentID segmentID of segment being dragged
			 */
			DraggingSegment(std::string trackId, std::string segmentID, double newDuration)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mNewDuration(newDuration) {}

			std::string mSegmentID;
			double mNewDuration;
		};

		/**
		 * Action for start dragging segments
		 */
		class StartDraggingSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			* Constructor
			* @param trackId id of track being dragged
			* @param segmentID id of segment being dragged
			*/
			StartDraggingSegment(std::string trackId, std::string segmentID, double startDuration)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mStartDuration(startDuration) {}

			std::string mSegmentID;
			double mStartDuration;
		};

		/**
		 * Action for when inside insert segment popup
		 */
		class InsertingSegmentPopup :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id
			 * @param time the time at which a segment is being inserted
			 * @param trackType trackType information about the track trackType
			 */
			InsertingSegmentPopup(std::string trackID, double time, const rttr::type& trackType)
				: TrackAction(std::move(trackID)), mTime(time), mTrackType(trackType) {}

			double mTime;
			rttr::type mTrackType;
		};

		/**
		 * Action for opening the popup to insert a segment into a track of a certain type
		 */
		class OpenInsertSegmentPopup :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id
			 * @param time time at which to insert new segment
			 * @param trackType type information about the track type
			 */
			OpenInsertSegmentPopup(std::string trackID, double time, const rttr::type& trackType)
				: TrackAction(std::move(trackID)), mTime(time), mTrackType(trackType) {}

			double mTime;
			rttr::type mTrackType;
		};

		/**
		 * Action when inside edit segment popup
		 */
		class EditingSegmentPopup : public TrackAction {
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id of the track containing the segment being edited
			 * @param segmentID the segment id being edited
			 * @param segmentType type info about the segment being edited
			 */
			EditingSegmentPopup(std::string trackID, std::string segmentID, const rttr::type& segmentType)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType){}

			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		/**
		 * Action when edit segment popup needs to be opened
		 */
		class OpenEditSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id of the track containing the segment being edited
			 * @param segmentID the segment id being edited
			 * @param segmentType type info about the segment being edited
			 */
			OpenEditSegmentPopup(std::string trackID, std::string segmentID, const rttr::type& segmentType)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType){}

			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		/**
		 * Action when hovering a segment
		 */
		class HoveringSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID track id of the track containing the segment we are hovering
			 * @param segmentID segment id
			 */
			HoveringSegment(std::string trackID, std::string segmentID)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)) {}

			std::string mSegmentID;
		};

		/**
		 * When hovering mouse over player scrub bar
		 */
		class HoveringPlayerTime : public Action
		{
			RTTI_ENABLE(Action)
		};

		/**
		 * Action when dragging player controller
		 */
		class DraggingPlayerTime : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param wasPlaying was the sequence player playing when we started dragging
			 * @param wasPaused was the sequence player paused when we started dragging
			 */
			DraggingPlayerTime(bool wasPlaying, bool wasPaused)
				: mWasPlaying(wasPlaying), mWasPaused(wasPaused) {}

			bool mWasPlaying;
			bool mWasPaused;
		};

		/**
		 * Action when a insert track popup needs to be opened
		 */
		class OpenInsertTrackPopup : public Action 
		{ 
			RTTI_ENABLE(Action) 
		};

		/**
		 * Action when inside insert track popup
		 */
		class InsertingTrackPopup : public Action
		{
			RTTI_ENABLE(Action)
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
		class SaveAsPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			int mSelectedShowIndex = 0;
			std::string mErrorString;
		};

		/**
		 * Action when hovering sequence duration handler
		 */
		class HoveringSequenceDuration : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		/**
		 * Action when dragging sequence duration handler
		 */
		class DraggingSequenceDuration : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		/**
		 * Action when open sequence duration popup needs to be opened
		 */
		class OpenSequenceDurationPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		/**
		 * Action when inside sequence duration popup
		 */
		class EditSequenceDurationPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		/**
		 * Action when sequence marker popup needs to be openend
		 */
		class OpenEditSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param id the id of the marker
			 * @param message the marker message
			 * @param time time at which to insert the marker in sequence ( seconds )
			 */
			OpenEditSequenceMarkerPopup(std::string id, std::string  message, double time)
				: mID(std::move(id)), mMessage(std::move(message)), mTime(time){}

			std::string mID;
			std::string mMessage;
			double mTime;
		};

		/**
		 * Action when inside edit marker popup
		 */
		class EditingSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param id the id of the marker
			 * @param message the marker message
			 * @param time time at which to insert the marker in sequence ( seconds )
			 */
			EditingSequenceMarkerPopup(std::string  id, std::string  message, double time)
				: mID(std::move(id)), mMessage(std::move(message)), mTime(time){}

			std::string mID;
			std::string mMessage;
			double mTime;
		};

		/**
		 * Action when insert marker popup needs to be opened
		 */
		class OpenInsertSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param time point in sequence where to insert marker ( seconds )
			 */
			explicit OpenInsertSequenceMarkerPopup(double time) : mTime(time){}

			double mTime;
		};

		/**
		 * Action when inside insert marker popup
		 */
		class InsertingSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param time point in sequence where to insert marker ( seconds )
			 * @param message message of marker
			 */
			InsertingSequenceMarkerPopup(double time, std::string message)
				: mTime(time), mMessage(std::move(message)){}

			double mTime;
			std::string mMessage;
		};

		/**
		 * Action when dragging marker
		 */
		class DragSequenceMarker : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param id the id of the marker
			 */
			explicit DragSequenceMarker(std::string id) : mID(std::move(id)){}

			std::string mID;
		};

		/**
		 * Action when assigned a new output id to track
		 */
		class AssignOutputIDToTrack :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id
			 * @param outputID the output id
			 */
			AssignOutputIDToTrack(const std::string& trackID, std::string outputID)
				: TrackAction(trackID), mObjectID(std::move(outputID)){}

			std::string mObjectID;
			
		};

		/**
		 * Action that tells the gui to open help popup
		 */
		class OpenHelpPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		/**
		 * Action that tells the gui its inside the help popup
		 */
		class ShowHelpPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};
	}
}
