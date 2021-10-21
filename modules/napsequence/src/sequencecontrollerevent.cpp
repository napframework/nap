/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencecontrollerevent.h"
#include "sequencetrackevent.h"
#include "sequenceeditor.h"

namespace nap
{
	double SequenceControllerEvent::segmentEventStartTimeChange(const std::string& trackID, const std::string& segmentID, double time)
	{
		double return_time = time;
		performEditAction([this, trackID, segmentID, time, &return_time]()
		{
			auto* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr); // segment not found

			if (segment != nullptr)
			{
				assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase))); // type mismatch

				if (segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)))
				{
					auto& segment_event = static_cast<SequenceTrackSegmentEventBase&>(*segment);
					segment_event.mStartTime = time;
					return_time = segment_event.mStartTime;
				}

			}

			updateTracks();
		});

		return return_time;
	}


	const SequenceTrackSegment* SequenceControllerEvent::insertSegment(const std::string& trackID, double time)
	{
		nap::Logger::warn("insertSegment not used, use insertEventSegment instead");
		return nullptr;
	}


	void SequenceControllerEvent::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		performEditAction([this, trackID, segmentID]()
		{
			//
			auto* track = findTrack(trackID);
			assert(track != nullptr); // track not found

			if (track != nullptr)
			{
				int segment_index = 0;
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						// erase it from the list
						track->mSegments.erase(track->mSegments.begin() + segment_index);

						deleteObjectFromSequencePlayer(segmentID);

						break;
					}

					updateTracks();
					segment_index++;
				}
			}
		});
	}


	void SequenceControllerEvent::addNewEventTrack()
	{
		performEditAction([this]()
		{
			// create sequence track
			std::unique_ptr<SequenceTrackEvent> sequence_track = std::make_unique<SequenceTrackEvent>();
			sequence_track->mID = mService.generateUniqueID(getPlayerReadObjectIDs());

            // name is id
            sequence_track->mName = sequence_track->mID;

			// create resource ptr
			getSequence().mTracks.emplace_back(ResourcePtr<SequenceTrackEvent>(sequence_track.get()));

			// move ownership of unique ptrs
			getPlayerOwnedObjects().emplace_back(std::move(sequence_track));
		});
	}


	void SequenceControllerEvent::insertTrack(rttr::type type)
	{
		addNewEventTrack();
	}
}