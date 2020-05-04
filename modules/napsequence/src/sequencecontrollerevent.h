#pragma once

#include "sequencecontroller.h"

#include <nap/logger.h>

namespace nap
{
	class NAPAPI SequenceControllerEvent : public SequenceController
	{
	public:
		SequenceControllerEvent(SequencePlayer & player) : SequenceController(player) { }

		/**
		* editEventSegment
		* edits event message
		* @param trackID the trackID
		* @param segmentID the segmentID
		* @param message the new message
		*/
		void editEventSegment(const std::string& trackID, const std::string& segmentID, const std::string& eventMessage);

		void insertEventSegment(const std::string& trackID, double time);

		void segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, float amount);

		virtual void insertSegment(const std::string& trackID, double time) override;

		virtual void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

		void addNewEventTrack();
	private:
	};
}
