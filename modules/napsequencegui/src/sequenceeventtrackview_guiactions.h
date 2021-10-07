#pragma once

#include "sequenceeditorguiactions.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Event Actions
	//////////////////////////////////////////////////////////////////////////

	namespace sequenceguiactions
	{
		/**
		 * TrackAction to tell the GUI to open an insert event segment popup
		 */
		class OpenInsertEventSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id of the track where the new segment should be inserted
			 * @param time the time at which to insert the track
			 */
			OpenInsertEventSegmentPopup(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
		};

		/**
		 * TrackAction that tells the GUI we're currently in an insert event segment popup
		 */
		class InsertingEventSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			/**
			 * Constructor
			 * @param trackID the track id of the track where the new segment should be inserted
			 * @param time the time at which to insert the track
			 */
			InsertingEventSegment(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
			std::string mMessage = "hello world";
		};

		/**
		 * TrackAction that tells the GUI to open a EditEvent segment popup of type T
		 * @tparam T type of event segment
		 */
		template<typename T>
		class OpenEditEventSegmentPopup : public TrackAction
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
			OpenEditEventSegmentPopup(const std::string& trackID, std::string  segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};

		/**
		 * TrackAction that tells the GUI we're currently in a EditEvent segment popup of type T
		 * @tparam T type of event segment
		 */
		template<typename T>
		class EditingEventSegment : public TrackAction
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
			EditingEventSegment(const std::string& trackID, std::string  segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};
	}
}
