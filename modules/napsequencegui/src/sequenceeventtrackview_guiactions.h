#pragma once

#include "sequenceeditorguiactions.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Event Actions
	//////////////////////////////////////////////////////////////////////////

	namespace SequenceGUIActions
	{
		class OpenInsertEventSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenInsertEventSegmentPopup(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
		};

		class InsertingEventSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			InsertingEventSegment(const std::string& trackID, double time)
				: TrackAction(trackID), mTime(time) {}

			double mTime;
			std::string mMessage = "hello world";
		};

		template<typename T>
		class OpenEditEventSegmentPopup : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			OpenEditEventSegmentPopup(const std::string& trackID, std::string  segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};

		template<typename T>
		class EditingEventSegment : public TrackAction
		{
			RTTI_ENABLE(TrackAction)
		public:
			EditingEventSegment(const std::string& trackID, std::string  segmentID, ImVec2 windowPos, T value, double startTime)
				: TrackAction(trackID), mSegmentID(std::move(segmentID)), mWindowPos(windowPos), mValue(value), mStartTime(startTime) {}

			std::string mSegmentID;
			ImVec2 mWindowPos;
			T mValue;
			double mStartTime;
		};
	}
}
