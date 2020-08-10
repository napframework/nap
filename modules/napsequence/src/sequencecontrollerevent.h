#pragma once

#include "sequencecontroller.h"
#include "sequencetracksegmentevent.h"
#include "sequenceutils.h"

#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Controller class for event tracks
	 */
	class NAPAPI SequenceControllerEvent : public SequenceController
	{
	public:
		/**
		 * Constructor
		 * @param player reference to player
		 */
		SequenceControllerEvent(SequencePlayer & player, SequenceEditor& editor) : SequenceController(player, editor) { }

		/**
		 * edits event message
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param message the new message
		 */
		 template<typename T>
		void editEventSegment(const std::string& trackID, const std::string& segmentID, const T& value);

		/**
		 * changes event start time
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param time the new time
		 */
		 double segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, float time);

		/**
		 * overloaded insert segment message
		 * @param trackID the track id
		 * @param time the time
		 */
		virtual void insertSegment(const std::string& trackID, double time) override;

		template<typename T>
		void insertEventSegment(const std::string& trackID, double time);

		/**
		 * overloaded delete segment method
		 * @param trackID the track id
		 * @param segmentID the segment id
		 */
		virtual void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

		/**
		 *  add new event track method
		 */
		void addNewEventTrack();

		/**
		 * overloaded insert track method
		 * @param type the track type
		 */
		virtual void insertTrack(rttr::type type) override;
	private:
	};


	template<typename T>
	void SequenceControllerEvent::insertEventSegment(const std::string& trackID, double time)
	{
		performEditAction([this, trackID, time]()
		{
			// create new segment & set parameters
			std::unique_ptr<SequenceTrackSegmentEvent<T>> newSegment = std::make_unique<SequenceTrackSegmentEvent<T>>();
			newSegment->mStartTime = time;
			newSegment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
			newSegment->mDuration = 0.0;

			//
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
				track->mSegments.emplace_back(ResourcePtr<SequenceTrackSegmentEvent<T>>(newSegment.get()));

				getPlayerOwnedObjects().emplace_back(std::move(newSegment));
			}
		});
	}


	template<typename T>
	void SequenceControllerEvent::editEventSegment(const std::string& trackID, const std::string& segmentID, const T& value)
	{
		performEditAction([this, trackID, segmentID, value]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
				SequenceTrackSegment* segment = findSegment(trackID, segmentID);
				assert(segment != nullptr); // segment not found

				if (segment != nullptr)
				{
					SequenceTrackSegmentEvent<T>* event_segment = dynamic_cast<SequenceTrackSegmentEvent<T>*>(segment);
					assert(event_segment != nullptr); // type mismatch

					if (event_segment != nullptr)
					{
						event_segment->mValue = value;
					}
				}
			}
		});
	}
}
