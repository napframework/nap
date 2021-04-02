/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencecurveenums.h"

// External Includes
#include <rtti/rtti.h>

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
			explicit TrackAction(std::string trackID) : mTrackID(trackID)
			{
			}

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
			DraggingSegment(std::string trackId, std::string segmentID)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)) {}

			std::string mSegmentID;
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
			StartDraggingSegment(std::string trackId, std::string segmentID)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)) {}

			std::string mSegmentID;
		};

		/**
		 * Action for inserting segment
		 */
		class InsertingSegment :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			InsertingSegment(std::string id, double time, const rttr::type& type)
				: TrackAction(std::move(id)), mTime(time), mTrackType(type) {}

			double mTime;
			rttr::type mTrackType;
		};


		class OpenInsertSegmentPopup :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenInsertSegmentPopup(std::string id, double time, const rttr::type& type)
				: TrackAction(std::move(id)), mTime(time), mTrackType(type) {}

			double mTime;
			rttr::type mTrackType;
		};

		class EditingSegment : public TrackAction {
			RTTI_ENABLE(TrackAction)
		public:
			EditingSegment(std::string trackID, std::string segmentID, const rttr::type& segmentType)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType)
			{

			}

			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class OpenEditSegmentValuePopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditSegmentValuePopup(std::string trackID, std::string segmentID, const rttr::type& segmentType)
				: TrackAction(std::move(trackID)), mSegmentID(std::move(segmentID)), mSegmentType(segmentType)
			{

			}

			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class HoveringSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringSegment(std::string trackId, std::string segmentID)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)) {}

			std::string mSegmentID;
		};

		class HoveringSegmentValue : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			HoveringSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class DraggingSegmentValue :
			public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			DraggingSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex)
				: TrackAction(std::move(trackId)), mSegmentID(std::move(segmentID)), mType(type), mCurveIndex(curveIndex) {}

			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class HoveringPlayerTime : public Action { RTTI_ENABLE(Action) };

		class DraggingPlayerTime : public Action
		{
			RTTI_ENABLE(Action)
		public:
			DraggingPlayerTime(bool wasPlaying, bool wasPaused)
				: mWasPlaying(wasPlaying), mWasPaused(wasPaused) {}

			bool mWasPlaying; bool mWasPaused;
		};

		class OpenInsertTrackPopup : public Action 
		{ 
			RTTI_ENABLE(Action) 
		};

		class InsertingTrackPopup : public Action
		{
			RTTI_ENABLE(Action)
		};

		class LoadPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			int mSelectedShowIndex = 0;
			std::string mErrorString;
		};

		class SaveAsPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			int mSelectedShowIndex = 0;
			std::string mErrorString;
		};

		class HoveringSequenceDuration : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		class DraggingSequenceDuration : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		class OpenSequenceDurationPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		class EditSequenceDurationPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
		};

		class OpenEditSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditSequenceMarkerPopup(std::string id, std::string  message, double time)
				: mID(std::move(id)), mMessage(std::move(message)), mTime(time){}

			std::string mID;
			std::string mMessage;
			double mTime;
		};

		class EditingSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			EditingSequenceMarkerPopup(std::string  id, std::string  message, double time)
				: mID(std::move(id)), mMessage(std::move(message)), mTime(time){}

			std::string mID;
			std::string mMessage;
			double mTime;
		};

		class OpenInsertSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			explicit OpenInsertSequenceMarkerPopup(double time) : mTime(time){}

			double mTime;
		};

		class InsertingSequenceMarkerPopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			InsertingSequenceMarkerPopup(double time, std::string message)
				: mTime(time), mMessage(std::move(message)){}

			double mTime;
			std::string mMessage;
		};

		class DragSequenceMarker : public Action
		{
			RTTI_ENABLE(Action)
		public:
			explicit DragSequenceMarker(std::string id) : mID(std::move(id)){}

			std::string mID;
		};
	}
}
