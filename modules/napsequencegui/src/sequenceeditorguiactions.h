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
            virtual ~Action(){}
            
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
			return std::make_unique<T>(args...);
		}

		/**
		 * None is an empty action
		 * When there is currently no action in the GUI, its action is None
		 */
		class None : public Action { RTTI_ENABLE() };

		/**
		 * Action for dragging segments
		 */
		class DraggingSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			 * Constructor
			 * @param trackId trackID of segment being dragged
			 * @param segmentID segmentID of segment being dragged
			 */
			DraggingSegment(std::string trackId, std::string segmentID)
				: mTrackID(trackId), mSegmentID(segmentID) {}

			std::string mTrackID;
			std::string mSegmentID;
		};

		/**
		* Action for start dragging segments
		*/
		class StartDraggingSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			/**
			* Constructor
			* @param trackId id of track being dragged
			* @param segmentID id of segment being dragged
			*/
			StartDraggingSegment(std::string trackId, std::string segmentID)
				: mTrackID(trackId), mSegmentID(segmentID) {}

			std::string mTrackID;
			std::string mSegmentID;
		};

		/**
		 * Action for inserting segment
		 */
		class InsertingSegment :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			InsertingSegment(std::string id, double time, rttr::type type)
				: mTrackID(id), mTime(time), mTrackType(type) {}

			std::string mTrackID;
			double mTime;
			rttr::type mTrackType;
		};


		class OpenInsertSegmentPopup :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenInsertSegmentPopup(std::string id, double time, rttr::type type)
				: mTrackID(id), mTime(time), mTrackType(type) {}

			std::string mTrackID;
			double mTime;
			rttr::type mTrackType;
		};

		class EditingSegment : public Action {
			RTTI_ENABLE(Action)
		public:
			EditingSegment(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class OpenEditSegmentValuePopup : public Action
		{
			RTTI_ENABLE(Action)
		public:
			OpenEditSegmentValuePopup(std::string trackID, std::string segmentID, rttr::type segmentType)
				: mTrackID(trackID), mSegmentID(segmentID), mSegmentType(segmentType)
			{

			}

			std::string mTrackID;
			std::string mSegmentID;
			rttr::type mSegmentType;
		};

		class HoveringSegment : public Action
		{
			RTTI_ENABLE(Action)
		public:
			HoveringSegment(std::string trackId, std::string segmentID)
				: mTrackID(trackId), mSegmentID(segmentID) {}

			std::string mTrackID;
			std::string mSegmentID;
		};

		class HoveringSegmentValue : public Action
		{
			RTTI_ENABLE(Action)
		public:
			HoveringSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex)
				: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex) {}

			std::string mTrackID;
			std::string mSegmentID;
			SequenceCurveEnums::SegmentValueTypes mType;
			int mCurveIndex;
		};

		class DraggingSegmentValue :
			public Action
		{
			RTTI_ENABLE(Action)
		public:
			DraggingSegmentValue(std::string trackId, std::string segmentID, SequenceCurveEnums::SegmentValueTypes type, int curveIndex)
				: mTrackID(trackId), mSegmentID(segmentID), mType(type), mCurveIndex(curveIndex) {}

			std::string mTrackID;
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
	}
}
