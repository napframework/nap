/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
		RTTI_ENABLE(SequenceController)
	public:
		/**
		 * Constructor
		 * @param player reference to player
		 * @param editor reference to the sequence editor
		 */
		SequenceControllerEvent(SequencePlayer & player, SequenceEditor& editor) : SequenceController(player, editor) { }

		/**
		 * edits event message
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param value the new message
		 */
		template<typename T>
		void editEventSegment(const std::string& trackID, const std::string& segmentID, const T& value);

		/**
		 * changes event start time
		 * @param trackID the trackID
		 * @param segmentID the segmentID
		 * @param time the new time
		 */
		double segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, double time);

		/**
		 * overloaded insert segment message
		 * @param trackID the track id
		 * @param time the time
		 */
		const SequenceTrackSegment* insertSegment(const std::string& trackID, double time) override;

		/**
		* insert event segment of type SEGMENT_TYPE
		* @param trackID the track id
		* @param time the time
		*/
		template<typename SEGMENT_TYPE>
		const SequenceTrackSegment* insertEventSegment(const std::string& trackID, double time);

		/**
		 * overloaded delete segment method
		 * @param trackID the track id
		 * @param segmentID the segment id
		 */
		void deleteSegment(const std::string& trackID, const std::string& segmentID) override;

		/**
		 *  add new event track method
		 */
		void addNewEventTrack();

		/**
		 * overloaded insert track method
		 * @param type the track type
		 */
		void insertTrack(rttr::type type) override;
	private:
	};


	template<typename SEGMENT_TYPE>
	const SequenceTrackSegment* SequenceControllerEvent::insertEventSegment(const std::string& trackID, double time)
	{
		SequenceTrackSegment* return_ptr = nullptr;

		performEditAction([this, trackID, time, &return_ptr]() mutable
		{
			// create new segment & set parameters
			std::unique_ptr<SEGMENT_TYPE> new_segment = std::make_unique<SEGMENT_TYPE>();
			new_segment->mStartTime = time;
			new_segment->mID = sequenceutils::generateUniqueID(getPlayerReadObjectIDs());
			new_segment->mDuration = 0.0;

			//
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			track->mSegments.emplace_back(ResourcePtr<SEGMENT_TYPE>(new_segment.get()));

			return_ptr = new_segment.get();
			getPlayerOwnedObjects().emplace_back(std::move(new_segment));
		});

		return return_ptr;
	}


	template<typename T>
	void SequenceControllerEvent::editEventSegment(const std::string& trackID, const std::string& segmentID, const T& value)
	{
		performEditAction([this, trackID, segmentID, value]()
		{
			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			assert(segment->get_type().template is_derived_from<SequenceTrackSegmentEvent<T>>());
			auto* event_segment = static_cast<SequenceTrackSegmentEvent<T>*>(segment);

			event_segment->mValue = value;
		});
	}
}